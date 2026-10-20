#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Replay the archived K1 executable without rebuilding it or changing FP modes."""
import argparse
import csv
import hashlib
import io
import itertools
import json
import os
from pathlib import Path, PurePosixPath
import platform
import re
import shutil
import subprocess
import zipfile

SOURCE = "fb3114eb829442223083e103929a15ad25d3efeb"
EXECUTABLE_SHA = "3228d4d226e7e5f6b213773831406b326a74a260340ed738e012efa1009cccd0"
FAILED_LOG_SHA = "d893ac670bfe10981e3ae55c34593c433363ec7f38ecfd34cd875908c24f2c09"
ORIGINS = {
    "candidate": (12186151118, "247fe73cec4300ff828cd529993fdcb9572def3edb9388c9f0918165814fda6c", "failed"),
    "reference": (12185332231, "cdcbb17305ddec26ae15174062fc19627e068240df35b37d08836b7355877a72", "complete"),
}
FIELDS = ("waveform", "rate", "filter_mode", "voice_mode", "fx",
          "output_peak_error", "output_relative_rms", "prefx_peak_error", "prefx_relative_rms")
DIGEST_FIELDS = ("output_reference_digest", "output_candidate_digest",
                 "prefx_reference_digest", "prefx_candidate_digest")
PROCESS_TIMEOUT = 120


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def plan():
    # Matches fb3114e scripts/qualify-preview-slices.py: eight wave/slice
    # processes, then three further Triangle pairs. No warm-up or retry.
    return ([(wave, arch, 0) for wave in range(4) for arch in ("arm64", "x86_64")] +
            [(2, arch, cold) for cold in range(1, 4) for arch in ("arm64", "x86_64")])


def execution_plan(mode):
    if mode == "apple-silicon":
        return plan()
    if mode == "native-intel":
        return [item for item in plan() if item[1] == "x86_64"]
    raise ValueError("Unknown execution mode")


def arguments(wave):
    # The original failure used signal digests, not plain --waveform 2.
    return ["--diagnostic-waveform" if wave == 2 else "--waveform", str(wave)]


def validate_table(text, wave, diagnostic=None):
    if diagnostic is None:
        diagnostic = wave == 2
    fields = FIELDS + (DIGEST_FIELDS if diagnostic else ())
    reader = csv.DictReader(io.StringIO(text))
    if tuple(reader.fieldnames or ()) != fields:
        raise ValueError("Unexpected CSV header")
    rows = list(reader)
    expected = list(itertools.product((str(wave),),
                    ("48000", "96000", "176400", "192000", "384000"),
                    ("0", "1", "2", "3"), ("0", "1", "2"), ("0", "1")))
    if len(rows) != len(expected):
        raise ValueError("Incomplete grid")
    keys = []
    table = []
    for row in rows:
        if None in row or any(row.get(field) is None for field in fields):
            raise ValueError("Malformed CSV row")
        key = tuple(row[field] for field in FIELDS[:5])
        keys.append(key)
        if any(row[field] != "0" for field in FIELDS[5:]):
            raise ValueError("Nonzero or invalid error metric")
        if diagnostic:
            values = tuple(row[field] for field in DIGEST_FIELDS)
            if any(not re.fullmatch("[0-9a-f]{16}", value) for value in values):
                raise ValueError("Invalid signal fingerprint")
            if values[0] != values[1] or values[2] != values[3]:
                raise ValueError("Reference/Candidate fingerprints differ")
            table.append(key + values)
    if keys != expected:
        raise ValueError("Reordered, duplicate or missing grid case")
    if diagnostic:
        # Same table algorithm as the archived qualifier; not raw audio SHA.
        return hashlib.sha256(json.dumps(sorted(table), separators=(",", ":")).encode()).hexdigest()
    return None


def unpack_verified(archive, origin, output):
    artifact_id, archive_sha, state = ORIGINS[origin]
    if sha(archive) != archive_sha:
        raise ValueError("Original artifact ZIP SHA256 mismatch")
    with zipfile.ZipFile(archive) as z:
        infos = z.infolist()
        names = [i.filename for i in infos]
        if len(set(names)) != len(names):
            raise ValueError("Duplicate ZIP member")
        for info in infos:
            path = PurePosixPath(info.filename)
            if path.is_absolute() or ".." in path.parts or "\\" in info.filename:
                raise ValueError("Unsafe ZIP member")
            if info.file_size > 16 * 1024 * 1024:
                raise ValueError("Oversized ZIP member")

        def read_one(name):
            found = [i for i in infos if not i.is_dir() and PurePosixPath(i.filename).name == name]
            if len(found) != 1:
                raise ValueError("Missing or ambiguous archive member: " + name)
            return z.read(found[0])

        manifest_bytes = read_one("manifest.json")
        manifest = json.loads(manifest_bytes)
        if (manifest.get("schema") != 2 or manifest.get("source_commit") != SOURCE or
                manifest.get("state") != state or
                manifest.get("executable_sha256") != EXECUTABLE_SHA):
            raise ValueError("Original manifest provenance mismatch")
        exe = read_one("sawstar_premium_native_pair")
        cache = read_one("CMakeCache.txt")
        if hashlib.sha256(exe).hexdigest() != EXECUTABLE_SHA:
            raise ValueError("Original executable SHA256 mismatch")
        if hashlib.sha256(cache).hexdigest() != manifest.get("cmake_cache_sha256"):
            raise ValueError("Original CMake cache SHA256 mismatch")
        results = manifest.get("results", [])
        actual = [(r["waveform"], r["architecture"], r["cold_run"]) for r in results]
        if actual != plan():
            raise ValueError("Original process plan mismatch")
        original_logs = {}
        for entry in results:
            name = entry["log_file"]
            if PurePosixPath(name).name != name or "\\" in name:
                raise ValueError("Unsafe original log name")
            data = read_one(name)
            if hashlib.sha256(data).hexdigest() != entry["log_sha256"]:
                raise ValueError("Original log SHA256 mismatch")
            original_logs[name] = data
        if origin == "candidate":
            first = next(r for r in results if (r["waveform"], r["architecture"], r["cold_run"]) == (2, "x86_64", 0))
            if first["returncode"] != 1 or first["log_sha256"] != FAILED_LOG_SHA:
                raise ValueError("Original negative evidence mismatch")
        elif any(r["returncode"] or r.get("validation_error") for r in results):
            raise ValueError("Original successful control is not complete")

    # Do not use extractall, executable ZIP mode bits, or archive paths.
    output.mkdir(exist_ok=False)
    (output / "manifest.json").write_bytes(manifest_bytes)
    (output / "CMakeCache.txt").write_bytes(cache)
    for name, data in original_logs.items():
        (output / name).write_bytes(data)
    executable = output / "sawstar_premium_native_pair"
    executable.write_bytes(exe)
    executable.chmod(0o755)
    return executable, manifest


def replay(archive, origin, output, execution_mode="apple-silicon"):
    archive, output = Path(archive).resolve(), Path(output).resolve()
    if output.exists():
        raise ValueError("Existing replay evidence is never overwritten")
    output.mkdir(parents=True, exist_ok=False)
    planned = execution_plan(execution_mode)
    manifest = dict(schema=1, state="incomplete", origin=origin, execution_mode=execution_mode,
        artifact_id=ORIGINS[origin][0], original_source_commit=SOURCE,
        expected_executable_sha256=EXECUTABLE_SHA, process_timeout_seconds=PROCESS_TIMEOUT,
        process_count=len(planned), rebuild=False, fp_override=False, results=[],
        purpose="Original K1 binary replay; not root cause, CPU or native host acceptance",
        production_promotion_allowed=False, native_host_acceptance=False)

    def save():
        (output / "replay-manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    save()
    try:
        expected_arch = "x86_64" if execution_mode == "native-intel" else "arm64"
        if platform.system() != "Darwin" or platform.machine() != expected_arch:
            raise ValueError("Unexpected host architecture for execution mode")
        translated = subprocess.run(["sysctl", "-in", "sysctl.proc_translated"],
            capture_output=True, text=True)
        manifest["translation_probe"] = dict(returncode=translated.returncode,
            stdout=translated.stdout.strip(), stderr=translated.stderr.strip())
        if execution_mode == "native-intel":
            if translated.returncode == 0 and translated.stdout.strip() != "0":
                raise ValueError("Native Intel campaign must not run through Rosetta")
            vendor = subprocess.check_output(["sysctl", "-n", "machdep.cpu.vendor"], text=True).strip()
            if vendor != "GenuineIntel":
                raise ValueError("Native Intel campaign requires an Intel CPU")
            manifest["cpu_vendor"] = vendor
            manifest["cpu_brand"] = subprocess.check_output(
                ["sysctl", "-n", "machdep.cpu.brand_string"], text=True).strip()
            manifest["runner_arch_env"] = os.environ.get("RUNNER_ARCH")
            if os.environ.get("RUNNER_ARCH") != "X64":
                raise ValueError("Expected the native X64 runner label")
        manifest["execution_plan"] = [dict(waveform=w, architecture=a, cold_run=c) for w,a,c in planned]
        manifest.update(runner_source_commit=subprocess.check_output(
            ["git", "rev-parse", "HEAD"], text=True).strip(),
            runtime_platform=platform.platform(),
            os_version=subprocess.check_output(["sw_vers"], text=True),
            runner_image=os.environ.get("ImageVersion"), runner_architecture=platform.machine())
        shutil.copy2(archive, output / "original-artifact.zip")
        manifest["original_zip_sha256"] = sha(archive)
        exe, original = unpack_verified(archive, origin, output / "original")
        manifest["executable_sha256"] = sha(exe)
        manifest["cmake_cache_sha256"] = sha(output / "original/CMakeCache.txt")
        manifest["slices"] = subprocess.check_output(["lipo", "-archs", str(exe)], text=True).strip()
        if set(manifest["slices"].split()) != {"arm64", "x86_64"}:
            raise ValueError("Expected both original Universal slices")
        save()
        baseline = {}
        child_env = os.environ.copy()
        # The download step alone receives GH_TOKEN; never pass it to archived code.
        for key in ("GH_TOKEN", "GITHUB_TOKEN"):
            child_env.pop(key, None)
        manifest["observed_malloc_environment"] = {
            k: child_env[k] for k in ("MallocScribble", "MallocPreScribble", "MallocNanoZone") if k in child_env}
        for wave, arch, cold in planned:
            suffix = "" if cold == 0 else "-cold-" + str(cold)
            name = arch + "-waveform-" + str(wave) + suffix + ".txt"
            log = output / name
            entry = dict(waveform=wave, architecture=arch, cold_run=cold,
                         arguments=arguments(wave), log_file=name)
            with log.open("w", encoding="utf-8") as stream:
                try:
                    result = subprocess.run(["arch", "-" + arch, str(exe)] + arguments(wave),
                        stdout=stream, stderr=subprocess.STDOUT, timeout=PROCESS_TIMEOUT, env=child_env)
                    entry["returncode"] = result.returncode
                except subprocess.TimeoutExpired:
                    entry.update(returncode=124, timeout=True)
                    stream.write("\nReplay process timeout\n")
                except OSError as error:
                    entry.update(returncode=125, launch_error=str(error))
            text = log.read_text(encoding="utf-8", errors="replace")
            entry["log_sha256"] = sha(log)
            if entry["returncode"]:
                entry["numerical_divergence_reported"] = bool(re.search(
                    r"Dispatch engine bit identity waveform=\d+ rate=", text))
            else:
                try:
                    fingerprint = validate_table(text, wave)
                    entry["validated_cases"] = 120
                    if wave == 2:
                        entry["fingerprint_table_sha256"] = fingerprint
                        if cold == 0:
                            baseline[arch] = fingerprint
                        elif arch not in baseline:
                            raise ValueError("No successful declared cold-run-0 baseline")
                        elif fingerprint != baseline[arch]:
                            raise ValueError("Cold-process fingerprint differs from declared baseline")
                except ValueError as error:
                    entry["validation_error"] = str(error)
            manifest["results"].append(entry)
            save()
            print(arch, "wave", wave, "cold", cold, "exit", entry["returncode"],
                  entry.get("validation_error", ""), flush=True)
        if sha(exe) != EXECUTABLE_SHA or sha(output / "original/CMakeCache.txt") != manifest["cmake_cache_sha256"]:
            raise ValueError("Original executable/cache changed during replay")
        failed = any(r["returncode"] or r.get("validation_error") for r in manifest["results"])
        manifest["state"] = "failed" if failed else "complete"
        manifest["interpretation"] = (
            "Inspect every process result; any numerical failure remains a failed campaign. "
            "An all-success replay does not fix or close the original K1 failure.")
    except BaseException as error:
        manifest.update(state="failed", error=str(error))
        raise
    finally:
        save()
    return manifest


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--archive", type=Path, required=True)
    parser.add_argument("--origin", choices=tuple(ORIGINS), required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--execution-mode", choices=("apple-silicon", "native-intel"),
                        default="apple-silicon")
    args = parser.parse_args()
    result = replay(args.archive, args.origin, args.output, args.execution_mode)
    if result["state"] != "complete":
        parser.exit(1, "K1 original replay failed; inspect retained evidence\n")
