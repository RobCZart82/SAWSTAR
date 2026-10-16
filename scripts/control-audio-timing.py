#!/usr/bin/env python3
"""Run a fixed A/A timing control; never replace the production regression gate."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import platform
import re

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("production_benchmark", ROOT / "scripts/benchmark-audio.py")
benchmark = importlib.util.module_from_spec(spec)
spec.loader.exec_module(benchmark)


def run_control(binary, output, source_commit=None, build_dir=None):
    binary = Path(binary).resolve(strict=True)
    if not binary.is_file():
        raise ValueError("Expected one benchmark executable file")
    if source_commit is not None and not re.fullmatch(r"[0-9a-f]{40}", source_commit):
        raise ValueError("source_commit must be a full lowercase Git SHA")
    digest = hashlib.sha256(binary.read_bytes()).hexdigest()
    metadata = dict(mode="identical-binary-control-v1", source_commit=source_commit,
        binary_sha256=digest, platform=platform.platform(), machine=platform.machine(),
        python=platform.python_version(), repeats=benchmark.REPEATS,
        thresholds=dict(cpu_relative=benchmark.CPU_RELATIVE_LIMIT,
                        cpu_absolute=benchmark.CPU_ABSOLUTE_LIMIT),
        github={k: os.environ[k] for k in
                ("GITHUB_RUN_ID", "GITHUB_RUN_ATTEMPT", "GITHUB_SHA", "RUNNER_OS", "RUNNER_ARCH")
                if k in os.environ})
    evidence = benchmark.EvidenceRecorder(output, metadata)
    try:
        if build_dir is not None:
            evidence.manifest["build"] = benchmark.build_identity(Path(build_dir), binary)
            evidence.write_manifest()
        # Both labels invoke the same absolute file, not separately built copies.
        reports = benchmark.collect_reports(dict(baseline=binary, current=binary), evidence=evidence)
        if hashlib.sha256(binary.read_bytes()).hexdigest() != digest:
            raise RuntimeError("Control executable changed during measurement")
        # The same reset workload must also produce the same audio, not just finite CSV.
        reference = reports["baseline"][0]
        for label in ("baseline", "current"):
            for report in reports[label]:
                for scene in reference:
                    if any(report[scene][metric] != reference[scene][metric]
                           for metric in ("peak_dbfs", "rms_dbfs", "dc")):
                        raise RuntimeError(f"Control audio differs for {label}, {scene}")
        baseline = benchmark.median_report(reports["baseline"])
        current = benchmark.median_report(reports["current"])
        scenes = []
        for key in sorted(baseline):
            old, new = baseline[key]["realtime_percent"], current[key]["realtime_percent"]
            limit = max(old * benchmark.CPU_RELATIVE_LIMIT, old + benchmark.CPU_ABSOLUTE_LIMIT)
            scenes.append(dict(scene=key[0], voices=key[1], baseline_cpu=old,
                current_cpu=new, change_percent=100 * (new / old - 1),
                limit=limit, threshold_exceeded=new > limit))
        failed = any(scene["threshold_exceeded"] for scene in scenes)
        result = dict(schema=1, mode="identical-binary-control-v1", binary_sha256=digest,
            classification="control_threshold_exceeded" if failed else "no_threshold_exceeded_in_this_control",
            production_regression_accepted=False, native_cpu_accepted=False, scenes=scenes)
        (evidence.directory / "control-result.json").write_text(
            json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        evidence.finish("failed" if failed else "success")
        return result
    except BaseException as error:
        evidence.finish("failed", f"{type(error).__name__}: {error}")
        raise


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--binary", type=Path)
    group.add_argument("--build-dir", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--source-commit")
    args = parser.parse_args()
    binary = args.binary if args.binary is not None else benchmark.executable(args.build_dir)
    result = run_control(binary, args.output, args.source_commit, args.build_dir)
    print(json.dumps(result, indent=2))
    raise SystemExit(1 if any(s["threshold_exceeded"] for s in result["scenes"]) else 0)
