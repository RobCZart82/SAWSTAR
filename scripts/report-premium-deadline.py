#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Validate paired offline research measurements and report descriptive ratios."""
import csv
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
    metadata = {"source_sha": subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip(),
                "platform": platform.platform(), "machine": platform.machine(),
                "runner_os": os.environ.get("RUNNER_OS"), "runner_arch": os.environ.get("RUNNER_ARCH"),
                "configuration": "Release", "blocks_per_path": 36864, "pairs_per_scene": 4,
                "warmup_seconds_per_path": .25, "timed_blocks_per_path_and_pair": 256,
                "drive_factor": 4, "clock": "C++ steady_clock wall time",
                "native_host_acceptance": False}
    (folder / "metadata.json").write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")
    lines = ["# Offline paired saturation deadline study", "",
             f"Source: {metadata['source_sha']}; {metadata['platform']}; {metadata['machine']}; Release.", "",
             "16 Poly voices, 20 dB, FX, four filter modes. Ratios are study/reference.",
             "Each table row uses 16 paired scene observations (four modes x four pairs).",
             "Percentages use host audio time; counts combine the measured scenes only.",
             "Shared CI wall time, buffer stores included, output checks outside timing.",
             "No native host or portable realtime acceptance; no timing pass/fail threshold.", "",
             "| Rate | Buffer | Median paired p50 ratio | Median paired p99 ratio | Reference over/4096 | Study over/4096 |",
             "| --- | --- | --- | --- | --- | --- |"]
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
            lines.append(f"| {rate} | {buffer} | {statistics.median(ratios['median_block_percent']):.6f} | "
                         f"{statistics.median(ratios['p99_block_percent']):.6f} | "
                         f"{counts['reference']} | {counts['study']} |")
    text = "\n".join(lines) + "\n"
    (folder / "report.md").write_text(text, encoding="utf-8")
    print(text)
    print("Validated 288 summary rows and 73728 raw blocks.")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("Usage: report-premium-deadline.py measurement-directory")
    report(sys.argv[1])
