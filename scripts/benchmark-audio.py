#!/usr/bin/env python3
"""Compare the current audio benchmark against a fixed baseline and fail CI on regressions."""

import csv
import io
import pathlib
import statistics
import subprocess
import tempfile
import zipfile

root = pathlib.Path(__file__).resolve().parents[1]
base = "0873064487e02b2e1da49b42d0fd12ca83e11ec7"

REPEATS = 3

# A CPU regression fails only when it exceeds BOTH limits:
# more than +25% relative and more than +1.0 realtime percentage point.
CPU_RELATIVE_LIMIT = 1.25
CPU_ABSOLUTE_LIMIT = 1.0

# Audio-level changes larger than 1 dB are treated as suspicious.
LEVEL_DB_LIMIT = 1.0

# The current render must not contain more than 1% full-scale DC.
MAX_CURRENT_DC = 0.01


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

    if not reader.fieldnames or not required.issubset(reader.fieldnames):
        raise RuntimeError(
            f"Unexpected benchmark CSV header: {reader.fieldnames}"
        )

    report = {}

    for row in reader:
        key = (row["scene"], int(row["voices"]))

        if key in report:
            raise RuntimeError(f"Duplicate benchmark row: {key}")

        report[key] = {
            "realtime_percent": float(row["realtime_percent"]),
            "peak_dbfs": float(row["peak_dbfs"]),
            "rms_dbfs": float(row["rms_dbfs"]),
            "dc": float(row["dc"]),
        }

    if not report:
        raise RuntimeError("Benchmark produced no data rows")

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

    runs = {
        "baseline": [],
        "current": [],
    }

    for repeat in range(REPEATS):
        for label, folder in (
            ("baseline", build),
            ("current", root / "build"),
        ):
            output = run(str(executable(folder)))

            print(
                f"{label},repeat={repeat + 1}",
                flush=True,
            )

            print(
                output,
                flush=True,
            )

            runs[label].append(
                parse_report(output)
            )

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
    
