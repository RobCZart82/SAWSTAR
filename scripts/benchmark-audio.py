#!/usr/bin/env python3
"""Compare the current audio benchmark against a fixed baseline and fail CI on regressions."""

import csv
import argparse
import hashlib
import io
import json
import math
import pathlib
import platform
import statistics
import subprocess
import os
import tempfile
import zipfile

root = pathlib.Path(__file__).resolve().parents[1]
base = "0873064487e02b2e1da49b42d0fd12ca83e11ec7"

REPEATS = 8
EXPECTED_SCENARIOS = frozenset(
    (scene, voices) for scene in ("dry_fx", "full_fx") for voices in (1, 8, 16)
)

# A CPU regression fails only when it exceeds BOTH limits:
# more than +25% relative and more than +1.0 realtime percentage point.
CPU_RELATIVE_LIMIT = 1.25
CPU_ABSOLUTE_LIMIT = 1.0

# Audio-level changes larger than 1 dB are treated as suspicious.
LEVEL_DB_LIMIT = 1.0

# The current render must not contain more than 1% full-scale DC.
MAX_CURRENT_DC = 0.01


class EvidenceRecorder:
    """Preserve every attempt before validation; never reuse old evidence."""
    def __init__(self, directory, metadata):
        self.directory = pathlib.Path(directory)
        self.directory.mkdir(parents=True, exist_ok=False)
        self.manifest = dict(schema=1, status="incomplete", metadata=metadata, records=[])
        self.write_manifest()

    def write_manifest(self):
        temporary = self.directory / "manifest.json.tmp"
        temporary.write_text(json.dumps(self.manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        temporary.replace(self.directory / "manifest.json")

    def record(self, label, phase, repeat, output, returncode=0):
        name = f"{len(self.manifest['records']):03d}-{label}-{phase}-{repeat}.csv"
        data = output.encode("utf-8")
        with (self.directory / name).open("xb") as stream:
            stream.write(data)
        self.manifest["records"].append(dict(file=name, label=label, phase=phase,
            repeat=repeat, returncode=returncode, sha256=hashlib.sha256(data).hexdigest()))
        self.write_manifest()

    def finish(self, status, error=None):
        self.manifest.update(status=status, error=error)
        self.write_manifest()


def build_identity(build, binary):
    """Record binaries and effective CMake flags without dumping environment secrets."""
    keys = ("CMAKE_CXX_COMPILER:", "CMAKE_CXX_FLAGS", "CMAKE_BUILD_TYPE:",
            "CMAKE_GENERATOR:", "CMAKE_OSX_ARCHITECTURES:", "CMAKE_OSX_DEPLOYMENT_TARGET:")
    cache = build / "CMakeCache.txt"
    selected = [line for line in cache.read_text(encoding="utf-8").splitlines()
                if line.startswith(keys)] if cache.exists() else []
    flags = {path.relative_to(build).as_posix(): path.read_text(encoding="utf-8")
             for path in sorted(build.rglob("flags.make"))
             if any(part in path.parts for part in
                    ("sawstar_engine.dir", "sawstar_daisy_primitives.dir", "sawstar_audio_benchmark.dir"))}
    compiler = {path.relative_to(build).as_posix(): [line for line in
                path.read_text(encoding="utf-8").splitlines()
                if line.startswith(("set(CMAKE_CXX_COMPILER ", "set(CMAKE_CXX_COMPILER_ID ",
                                    "set(CMAKE_CXX_COMPILER_VERSION ", "set(CMAKE_CXX_COMPILER_ARCHITECTURE_ID "))]
                for path in sorted(build.rglob("CMakeCXXCompiler.cmake"))}
    return dict(binary_sha256=hashlib.sha256(binary.read_bytes()).hexdigest(),
                cmake_cache=selected, target_flags=flags, compiler=compiler)


def cache_values(build):
    values = {}
    for line in (build / "CMakeCache.txt").read_text(encoding="utf-8").splitlines():
        if not line.startswith(("//", "#")) and ":" in line and "=" in line:
            name, value = line.split("=", 1)
            values[name.split(":", 1)[0]] = value
    return values


def baseline_platform_options(current_build):
    """The archived source must target the same macOS/architecture as current."""
    cache = cache_values(current_build)
    return [f"-D{key}:STRING={cache[key]}" for key in
            ("CMAKE_OSX_DEPLOYMENT_TARGET", "CMAKE_OSX_ARCHITECTURES") if cache.get(key)]


def verify_build_policy(baseline_build, current_build):
    """Reject unequal build policy before any timing; thresholds stay unchanged."""
    baseline, current = cache_values(baseline_build), cache_values(current_build)
    keys = ("CMAKE_BUILD_TYPE", "CMAKE_GENERATOR", "CMAKE_CXX_FLAGS",
            "CMAKE_CXX_FLAGS_RELEASE", "CMAKE_OSX_DEPLOYMENT_TARGET", "CMAKE_OSX_ARCHITECTURES")
    differences = {key: dict(baseline=baseline.get(key, ""), current=current.get(key, ""))
                   for key in keys if baseline.get(key, "") != current.get(key, "")}
    if differences:
        raise RuntimeError(f"Benchmark build policy mismatch: {differences}")


def run(*args, cwd=root):
    return subprocess.check_output(args, cwd=cwd, text=True)


def executable(build):
    matches = [
        p
        for p in build.rglob("sawstar_audio_benchmark*")
        if p.is_file() and (p.suffix == ".exe" or not p.suffix)
    ]
    if len(matches) != 1:
        raise RuntimeError(
            f"Expected exactly one sawstar_audio_benchmark executable in {build}, "
            f"found: {matches}"
        )
    return matches[0]


def parse_report(text):
    reader = csv.DictReader(io.StringIO(text.strip()))
    required = {
        "scene",
        "voices",
        "realtime_percent",
        "peak_dbfs",
        "rms_dbfs",
        "dc",
    }

    if (not reader.fieldnames or set(reader.fieldnames) != required
            or len(reader.fieldnames) != len(required)):
        raise RuntimeError(
            f"Unexpected benchmark CSV header: {reader.fieldnames}"
        )

    report = {}

    for row in reader:
        if None in row or any(value is None for value in row.values()):
            raise RuntimeError("Malformed benchmark CSV row")
        try:
            key = (row["scene"], int(row["voices"]))
            metrics = {
                metric: float(row[metric])
                for metric in ("realtime_percent", "peak_dbfs", "rms_dbfs", "dc")
            }
        except (ValueError, TypeError) as error:
            raise RuntimeError("Invalid benchmark numeric value") from error

        if key in report:
            raise RuntimeError(f"Duplicate benchmark row: {key}")

        report[key] = metrics

        for metric, value in report[key].items():
            if not math.isfinite(value):
                raise RuntimeError(
                    f"Non-finite benchmark value for {key}, {metric}: {value}"
                )
        if metrics["realtime_percent"] <= 0:
            raise RuntimeError(f"Non-positive benchmark CPU time for {key}")

    if not report:
        raise RuntimeError("Benchmark produced no data rows")
    if set(report) != EXPECTED_SCENARIOS:
        raise RuntimeError(
            "Incomplete or unexpected benchmark scenarios: "
            f"missing={sorted(EXPECTED_SCENARIOS - set(report))}, "
            f"unexpected={sorted(set(report) - EXPECTED_SCENARIOS)}"
        )

    return report


def median_report(reports):
    keys = set(reports[0])

    for report in reports[1:]:
        if set(report) != keys:
            raise RuntimeError("Benchmark scenario set changed between repeats")

    result = {}

    for key in keys:
        result[key] = {}
        for metric in ("realtime_percent", "peak_dbfs", "rms_dbfs", "dc"):
            result[key][metric] = statistics.median(
                report[key][metric] for report in reports
            )

    return result


def check_current_dc(report):
    """A single unsafe current render must not disappear in a median."""
    for key, metrics in report.items():
        if abs(metrics["dc"]) > MAX_CURRENT_DC:
            raise RuntimeError(
                f"{key}: current DC={metrics['dc']:+.6f} exceeds "
                f"±{MAX_CURRENT_DC:.3f} in an individual render"
            )


def collect_reports(binaries, repeats=REPEATS, evidence=None):
    """Warm both executables, then balance measurement order across pairs."""
    if repeats < 2 or repeats % 2:
        raise ValueError("Use a positive even number of measurement pairs")
    labels = ("baseline", "current")
    def capture(label, phase, repeat):
        try:
            output = run(str(binaries[label]))
        except subprocess.CalledProcessError as error:
            if evidence is not None:
                evidence.record(label, phase, repeat, error.output or "", error.returncode)
            raise
        if evidence is not None:
            evidence.record(label, phase, repeat, output)
        return output
    # Validate warm-up output too; invalid audio must never be silently ignored.
    for label in labels:
        report = parse_report(capture(label, "warmup", 0))
        if label == "current":
            check_current_dc(report)
    reports = {label: [] for label in labels}
    for repeat in range(repeats):
        order = labels if repeat % 2 == 0 else labels[::-1]
        for label in order:
            output = capture(label, "timed", repeat + 1)
            print(f"{label},repeat={repeat + 1}", flush=True)
            print(output, flush=True)
            report = parse_report(output)
            if label == "current":
                check_current_dc(report)
            reports[label].append(report)
    return reports


def main(evidence=None):
    with tempfile.TemporaryDirectory(prefix="sawstar-benchmark-") as temp:
        source = pathlib.Path(temp) / "baseline"
        source.mkdir()

        data = subprocess.check_output(
            ["git", "archive", "--format=zip", base],
            cwd=root,
        )

        with zipfile.ZipFile(io.BytesIO(data)) as archive:
            archive.extractall(source)

        # Use the exact same pinned dependency and workload for both engines.
        cmake = (source / "CMakeLists.txt").read_text().replace(
            "${CMAKE_CURRENT_SOURCE_DIR}/third_party/DaisySP",
            (root / "third_party/DaisySP").as_posix(),
        )

        cmake += (
            "\nadd_executable(sawstar_audio_benchmark tests/audio_benchmark.cpp)\n"
            "target_link_libraries(sawstar_audio_benchmark PRIVATE sawstar_engine)\n"
        )

        (source / "CMakeLists.txt").write_text(cmake)

        (source / "tests/audio_benchmark.cpp").write_text(
            (root / "tests/audio_benchmark.cpp").read_text()
        )

        build = pathlib.Path(temp) / "build"

        run(
            "cmake",
            "-S",
            str(source),
            "-B",
            str(build),
            "-DCMAKE_BUILD_TYPE=Release",
            "-DSAWSTAR_CHECK_DAISYSP=ON",
            "-DBUILD_TESTING=OFF",
            *baseline_platform_options(root / "build"),
        )

        run(
            "cmake",
            "--build",
            str(build),
            "--config",
            "Release",
            "--target",
            "sawstar_audio_benchmark",
            "--parallel",
            "2",
        )

        binaries = {
            "baseline": executable(build),
            "current": executable(root / "build"),
        }
        if evidence is not None:
            evidence.manifest["builds"] = {
                "baseline": build_identity(build, binaries["baseline"]),
                "current": build_identity(root / "build", binaries["current"]),
            }
            evidence.write_manifest()
        verify_build_policy(build, root / "build")
        runs = collect_reports(binaries, evidence=evidence)

        baseline = median_report(
            runs["baseline"]
        )

        current = median_report(
            runs["current"]
        )

        if set(baseline) != set(current):
            raise RuntimeError(
                "Baseline and current benchmark scenarios do not match"
            )

        failures = []

        print(
            "\nMedian benchmark comparison"
        )

        print(
            "scene,voices,baseline_cpu,current_cpu,cpu_change_pct,"
            "peak_delta_db,rms_delta_db,current_dc"
        )

        for key in sorted(baseline):
            b = baseline[key]
            c = current[key]

            base_cpu = b["realtime_percent"]
            current_cpu = c["realtime_percent"]

            cpu_change_pct = (
                100.0 * (current_cpu / base_cpu - 1.0)
                if base_cpu > 0.0
                else float("inf")
            )

            peak_delta = (
                c["peak_dbfs"] - b["peak_dbfs"]
            )

            rms_delta = (
                c["rms_dbfs"] - b["rms_dbfs"]
            )

            print(
                f"{key[0]},{key[1]},"
                f"{base_cpu:.6f},{current_cpu:.6f},"
                f"{cpu_change_pct:+.2f},"
                f"{peak_delta:+.4f},"
                f"{rms_delta:+.4f},"
                f"{c['dc']:+.8f}"
            )

            cpu_limit = max(
                base_cpu * CPU_RELATIVE_LIMIT,
                base_cpu + CPU_ABSOLUTE_LIMIT,
            )

            if current_cpu > cpu_limit:
                failures.append(
                    f"{key}: CPU regression: "
                    f"baseline={base_cpu:.4f}, "
                    f"current={current_cpu:.4f}, "
                    f"limit={cpu_limit:.4f}"
                )

            if abs(peak_delta) > LEVEL_DB_LIMIT:
                failures.append(
                    f"{key}: peak level changed by "
                    f"{peak_delta:+.3f} dB "
                    f"(limit ±{LEVEL_DB_LIMIT:.1f} dB)"
                )

            if abs(rms_delta) > LEVEL_DB_LIMIT:
                failures.append(
                    f"{key}: RMS level changed by "
                    f"{rms_delta:+.3f} dB "
                    f"(limit ±{LEVEL_DB_LIMIT:.1f} dB)"
                )

            if abs(c["dc"]) > MAX_CURRENT_DC:
                failures.append(
                    f"{key}: current DC="
                    f"{c['dc']:+.6f} exceeds "
                    f"±{MAX_CURRENT_DC:.3f}"
                )

        if failures:
            if evidence is not None:
                evidence.manifest["failures"] = failures
                evidence.write_manifest()
            print(
                "\nBENCHMARK REGRESSION DETECTED:"
            )

            for failure in failures:
                print(
                    f"  - {failure}"
                )

            raise SystemExit(1)

        print(
            "\nBenchmark regression check passed."
        )



if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--evidence-dir", type=pathlib.Path,
                        help="New directory for raw reports and build identity (also on failure)")
    args = parser.parse_args()
    evidence = None
    if args.evidence_dir is not None:
        evidence = EvidenceRecorder(args.evidence_dir, dict(
            current_commit=run("git", "rev-parse", "HEAD").strip(), baseline_commit=base,
            platform=platform.platform(), machine=platform.machine(),
            python=platform.python_version(), repeats=REPEATS,
            thresholds=dict(cpu_relative=CPU_RELATIVE_LIMIT, cpu_absolute=CPU_ABSOLUTE_LIMIT,
                            level_db=LEVEL_DB_LIMIT, dc=MAX_CURRENT_DC),
            github={key: os.environ[key] for key in
                    ("GITHUB_RUN_ID", "GITHUB_RUN_ATTEMPT", "GITHUB_SHA", "RUNNER_OS", "RUNNER_ARCH")
                    if key in os.environ},
            workload_sha256=hashlib.sha256((root / "tests/audio_benchmark.cpp").read_bytes()).hexdigest()))
    try:
        main(evidence)
    except BaseException as error:
        if evidence is not None:
            evidence.finish("failed", f"{type(error).__name__}: {error}")
        raise
    else:
        if evidence is not None:
            evidence.finish("success")
