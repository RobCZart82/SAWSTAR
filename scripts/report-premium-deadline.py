#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Validate paired offline research measurements and report descriptive ratios."""
import csv
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import statistics
import subprocess
import sys

def report(folder):
    folder = Path(folder)
    workload = (folder / "workload.txt").read_text(encoding="utf-8").strip()
    if workload not in ("stationary-v1", "modulated-v1"):
        raise ValueError("Unknown compiled workload")
    expected_workload = os.environ.get("SAWSTAR_PREMIUM_WORKLOAD", "stationary-v1")
    if workload != expected_workload:
        raise ValueError("Compiled workload disagrees with requested workload")
    with (folder / "summary.csv").open(newline="") as f:
        rows = list(csv.DictReader(f))
    with (folder / "blocks.csv").open(newline="") as f:
        raw = list(csv.DictReader(f))
    groups = {}
    for row in raw:
        key = (row["engine"], int(row["rate"]), int(row["filter_mode"]), int(row["buffer"]), int(row["pair"]))
        block, value = int(row["block"]), float(row["block_percent"])
        if not math.isfinite(value) or value <= 0:
            raise ValueError("Invalid raw timing")
        group = groups.setdefault(key, {})
        if block in group:
            raise ValueError("Duplicate raw block")
        group[block] = value
    expected = {(engine, rate, mode, buffer, pair)
                for engine in ("reference", "study") for rate in (48000, 96000, 192000)
                for mode in range(4) for buffer in (32, 64, 128) for pair in range(4)}
    if set(groups) != expected or len(raw) != 73728 or len(rows) != 288:
        raise ValueError("Incomplete measurement grid")
    summaries = {}
    for row in rows:
        key = (row["engine"], int(row["rate"]), int(row["filter_mode"]), int(row["buffer"]), int(row["pair"]))
        if key not in expected or key in summaries:
            raise ValueError("Unexpected or duplicate summary")
        times = groups[key]
        if set(times) != set(range(256)) or int(row["blocks"]) != 256:
            raise ValueError("Incomplete block sequence")
        if (int(row["voices"]), float(row["drive_db"]), int(row["fx"])) != (16, 20., 1):
            raise ValueError("Unexpected fixture")
        if row["order"] != ("study-first" if key[-1] % 2 else "reference-first"):
            raise ValueError("Unexpected alternating order")
        ordered = sorted(times.values())
        controls = {"median_block_percent": statistics.median(ordered),
                    "p95_block_percent": ordered[math.ceil(.95 * 256) - 1],
                    "p99_block_percent": ordered[math.ceil(.99 * 256) - 1],
                    "worst_block_percent": ordered[-1]}
        for name, value in controls.items():
            if not math.isclose(float(row[name]), value, rel_tol=1e-12, abs_tol=1e-10):
                raise ValueError(f"Summary/raw mismatch: {name}")
        if int(row["over_budget_blocks"]) != sum(t > 100 for t in ordered):
            raise ValueError("Incorrect strict deadline count")
        for name in ("peak", "rms", "checksum"):
            if not math.isfinite(float(row[name])):
                raise ValueError("Invalid signal diagnostic")
        if not 0 < float(row["peak"]) <= .98001 or float(row["rms"]) <= 0:
            raise ValueError("Silent or unprotected signal")
        summaries[key] = row
    if set(summaries) != expected:
        raise ValueError("Incomplete summary grid")
    study_kind = os.environ.get("SAWSTAR_PREMIUM_STUDY", "tanh")
    if study_kind not in ("tanh", "simd-fir", "rate-simd-fir", "gain-normalization", "rate-gain-normalization", "rate-lookup", "rate-unrolled-fir", "rate-saw-dispatch"):
        raise ValueError("Unknown study kind")
    if study_kind == "rate-unrolled-fir" and (folder / "study.txt").read_text(encoding="utf-8").strip() != '2x-interpolation-unroll-v1':
        raise ValueError("Compiled FIR study disagrees with requested label")
    if study_kind == "rate-saw-dispatch":
        # Older candidate archives predate this compiled marker. A reference
        # repeat always requires it; an environment label cannot create one.
        comparison_file = folder / 'comparison.txt'
        comparison = comparison_file.read_text(encoding='utf-8').strip() if comparison_file.exists() else 'candidate'
        if comparison not in ('candidate', 'reference-repeat') or comparison != os.environ.get('SAWSTAR_PREMIUM_COMPARISON', 'candidate'):
            raise ValueError('Compiled comparison disagrees with requested comparison')
        if (folder / "study.txt").read_text(encoding="utf-8").strip() != 'saw-dispatch-v1':
            raise ValueError("Compiled oscillator study disagrees with requested label")
        waveform = (folder / "waveform.txt").read_text(encoding="utf-8").strip()
        if waveform not in ('0', '1', '2', '3') or waveform != os.environ.get('SAWSTAR_OSC_WAVEFORM'):
            raise ValueError("Compiled waveform disagrees with requested waveform")
        for rate in (48000, 96000, 192000):
            for mode in range(4):
                for buffer in (32, 64, 128):
                    for pair in range(4):
                        a = summaries['reference', rate, mode, buffer, pair]
                        b = summaries['study', rate, mode, buffer, pair]
                        if any(float(a[name]) != float(b[name]) for name in ('peak', 'rms', 'checksum')):
                            raise ValueError("Dispatch paired signal differs")
    backend = (folder / "backend.txt").read_text(encoding="utf-8").strip()
    if backend not in ("SSE2", "NEON", "scalar-fallback", "scalar-source"):
        raise ValueError("Unknown FIR backend")
    if study_kind == "tanh" and backend != "scalar-source":
        raise ValueError("Incorrect tanh backend")
    if study_kind in ("simd-fir", "rate-simd-fir", "gain-normalization", "rate-gain-normalization", "rate-lookup", "rate-unrolled-fir", "rate-saw-dispatch") and backend == "scalar-source":
        raise ValueError("Incorrect SIMD study backend")
    adaptive = study_kind in ("rate-simd-fir", "rate-gain-normalization", "rate-lookup", "rate-unrolled-fir", "rate-saw-dispatch")
    factors = {str(rate): (2 if adaptive and rate >= 176400 else 4)
               for rate in (48000, 96000, 192000)}
    with (folder / "factors.csv").open(newline="") as f:
        recorded = list(csv.DictReader(f))
    if len(recorded) != 3 or {row["rate"] for row in recorded} != set(factors):
        raise ValueError("Incomplete compiled routing record")
    for row in recorded:
        if (int(row["reference_factor"]), int(row["study_factor"])) != (factors[row["rate"]],) * 2:
            raise ValueError("Compiled routing disagrees with study label")
    metadata = {"study_kind": study_kind, "workload": workload,
                "timed_control_events": workload == "modulated-v1",
                "setup_drive_db": 20,
                "drive_targets_db": [0, 12, 24] if workload == "modulated-v1" else [20],
                "fir_backend": backend,"source_sha": subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip(),
                "platform": platform.platform(), "machine": platform.machine(),
                "runner_os": os.environ.get("RUNNER_OS"), "runner_arch": os.environ.get("RUNNER_ARCH"),
                "configuration": "Release", "blocks_per_path": 36864, "pairs_per_scene": 4,
                "warmup_seconds_per_path": .25, "timed_blocks_per_path_and_pair": 256,
                "drive_factor": None if adaptive else 4, "drive_factors_by_rate": factors,
                "rate_policy": "normalized-176400-boundary" if adaptive else "fixed-4x",
                "clock": "C++ steady_clock wall time",
                "measurement_sha256": {name: hashlib.sha256((folder / name).read_bytes()).hexdigest()
                                        for name in ("summary.csv", "blocks.csv", "factors.csv", "backend.txt", "workload.txt")},
                "native_host_acceptance": False}
    if study_kind == "rate-unrolled-fir":
        metadata['reference_variant'] = 'rate-lookup-loop'
        metadata['study_variant'] = 'rate-lookup-unrolled'
        metadata['fir_unroll_scope'] = '2x-interpolation-only-v1'
        metadata['measurement_sha256']['study.txt'] = hashlib.sha256((folder / 'study.txt').read_bytes()).hexdigest()
    if study_kind == "rate-saw-dispatch":
        metadata.update(reference_variant='rate-lookup-seven-saw', study_variant='rate-lookup-saw-dispatch-v1',
                        oscillator_waveform=int(waveform), comparison=comparison, production_activation=False)
        if comparison == 'reference-repeat':
            metadata['study_variant'] = metadata['reference_variant']
        if comparison_file.exists():
            metadata['measurement_sha256']['comparison.txt'] = hashlib.sha256(comparison_file.read_bytes()).hexdigest()
        for name in ('study.txt', 'waveform.txt'):
            metadata['measurement_sha256'][name] = hashlib.sha256((folder / name).read_bytes()).hexdigest()
        root = Path(__file__).resolve().parents[1]
        sources = ('CMakeLists.txt', 'src/engine/Synth.h', 'src/engine/Synth.cpp',
                   'src/dsp/SevenSaw.h', 'src/dsp/SevenSaw.cpp',
                   'experiments/oscillator/SawDispatchSevenSaw.h', 'experiments/oscillator/SawDispatchSevenSaw.cpp',
                   'experiments/oscillator/benchmark_dispatch_engine.cpp', 'tests/oscillator_dispatch_engine.cpp',
                   'experiments/premium_filter/DeadlineModulation.h')
        metadata['source_file_sha256'] = {name: hashlib.sha256((root / name).read_bytes()).hexdigest() for name in sources}
        for file in sorted((root / 'experiments/premium_filter').glob('*.h')):
            metadata['source_file_sha256'][file.relative_to(root).as_posix()] = hashlib.sha256(file.read_bytes()).hexdigest()
        compiler_files = sorted((folder / 'compiler').glob('*.cmake'))
        if not compiler_files:
            raise ValueError('Missing dispatch compiler provenance')
        metadata['compiler_files_sha256'] = {file.relative_to(folder).as_posix(): hashlib.sha256(file.read_bytes()).hexdigest() for file in compiler_files}
    (folder / "metadata.json").write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")
    lines = ["# Offline paired premium engine deadline study", "",
             f"Source: {metadata['source_sha']}; {metadata['platform']}; {metadata['machine']}; Release.", "",
             f"Candidate: {study_kind}; explicit FIR backend: {backend}.",
             f"Compiled workload: {workload}; timed control setters: {metadata['timed_control_events']}.",
             f"Both paths use Drive factors by rate: {factors}.",
             "16 Poly voices, FX, four filter modes. Ratios are study/reference.",
             f"Setup Drive: 20 dB; workload targets: {metadata['drive_targets_db']} dB. CSV drive_db identifies setup, not the changing timeline.",
             "Each table row uses 16 paired scene observations (four modes x four pairs).",
             "Percentages use host audio time; counts combine the measured scenes only.",
             "Offline wall time, buffer stores included, output checks outside timing.",
             "No native host or portable realtime acceptance; no timing pass/fail threshold.", "",
             "| Rate | Buffer | Median paired p50 ratio | Median paired p99 ratio | Reference over/4096 | Study over/4096 |",
             "| --- | --- | --- | --- | --- | --- |"]
    if study_kind == "rate-saw-dispatch":
        lines.insert(6, f"OSC1/OSC2 waveform: {waveform}; waveforms and workloads remain separate grids.")
        lines.insert(7, f"Comparison: {comparison}; reference repeats are separate controls, not noise corrections.")
    paired_results = []
    for rate in (48000, 96000, 192000):
        for buffer in (32, 64, 128):
            ratios = {name: [] for name in ("median_block_percent", "p99_block_percent")}
            counts = {"reference": 0, "study": 0}
            for mode in range(4):
                for pair in range(4):
                    a = summaries[("reference", rate, mode, buffer, pair)]
                    b = summaries[("study", rate, mode, buffer, pair)]
                    for name in ratios:
                        ratios[name].append(float(b[name]) / float(a[name]))
                    for engine, row in (("reference", a), ("study", b)):
                        counts[engine] += int(row["over_budget_blocks"])
            p50_ratio = statistics.median(ratios['median_block_percent'])
            p99_ratio = statistics.median(ratios['p99_block_percent'])
            paired_results.append({"rate": rate, "buffer": buffer,
                                   "paired_observations": 16, "blocks_per_path": 4096,
                                   "median_paired_p50_ratio": p50_ratio,
                                   "median_paired_p99_ratio": p99_ratio,
                                   "reference_over_budget_blocks": counts['reference'],
                                   "study_over_budget_blocks": counts['study']})
            lines.append(f"| {rate} | {buffer} | {p50_ratio:.6f} | "
                         f"{p99_ratio:.6f} | "
                         f"{counts['reference']} | {counts['study']} |")
    text = "\n".join(lines) + "\n"
    (folder / "report.md").write_text(text, encoding="utf-8")
    payload = {"schema_version": 1, "metadata": metadata,
               "ratio_direction": "study/reference", "native_host_acceptance": False,
               "paired_results": paired_results}
    (folder / "paired-results.json").write_text(json.dumps(payload, indent=2, allow_nan=False) + "\n", encoding="utf-8")
    print(text)
    print("Validated 288 summary rows and 73728 raw blocks.")
    if study_kind in ("rate-unrolled-fir", "rate-saw-dispatch"):
        print("DEADLINE_CI_REPORT=" + json.dumps(payload, separators=(",", ":"), allow_nan=False))

if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("Usage: report-premium-deadline.py measurement-directory")
    report(sys.argv[1])

