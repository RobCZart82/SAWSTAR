#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Validate isolated Drive pairs; wall time is descriptive, never a CPU gate."""
import copy
import csv
import json
import math
from pathlib import Path
import statistics
import sys

FIELDS = ("factor", "rate", "voices", "drive_db", "pair",
          "reference_seconds", "simd_seconds")
EXPECTED = {(factor, rate, pair) for factor in (2, 4)
            for rate in (48000, 96000, 192000) for pair in range(4)}

def validate(rows):
    pairs = {}
    for row in rows:
        if set(row) != set(FIELDS):
            raise ValueError("Unexpected Drive CSV columns")
        key = (int(row["factor"]), int(row["rate"]), int(row["pair"]))
        if key not in EXPECTED or key in pairs:
            raise ValueError("Unexpected or duplicate Drive pair")
        if (int(row["voices"]), float(row["drive_db"])) != (16, 20.):
            raise ValueError("Unexpected Drive fixture")
        a, b = float(row["reference_seconds"]), float(row["simd_seconds"])
        if not all(math.isfinite(x) and x > 0 for x in (a, b)):
            raise ValueError("Invalid Drive timing")
        ratio = b / a
        if not math.isfinite(ratio) or ratio <= 0:
            raise ValueError("Invalid Drive ratio")
        pairs[key] = ratio
    if set(pairs) != EXPECTED:
        raise ValueError("Incomplete Drive grid")
    return pairs

def self_test():
    rows = [dict(zip(FIELDS, (str(f), str(r), "16", "20", str(p), "2", "1")))
            for f, r, p in sorted(EXPECTED)]
    if set(validate(rows).values()) != {.5}:
        raise RuntimeError("Incorrect paired ratio")
    invalid = [rows[:-1], rows + [rows[0]]]
    for field, value in (("factor", "3"), ("rate", "44100"),
                         ("pair", "4"), ("voices", "8"), ("drive_db", "0"),
                         ("reference_seconds", "0"), ("simd_seconds", "-1"),
                         ("reference_seconds", "nan"), ("simd_seconds", "inf"),
                         ("reference_seconds", "1e-300")):
        case = copy.deepcopy(rows)
        case[0][field] = value
        if value == "1e-300":
            case[0]["simd_seconds"] = "1e300"
        invalid.append(case)
    case = copy.deepcopy(rows)
    case[0]["extra"] = "1"
    invalid.append(case)
    for case in invalid:
        try:
            validate(case)
        except ValueError:
            continue
        raise RuntimeError("Invalid Drive fixture accepted")
    print(f"Drive reporter controls passed: valid grid and {len(invalid)} rejected fixtures.")

def report(folder):
    folder = Path(folder)
    with (folder / "drive.csv").open(newline="") as f:
        pairs = validate(list(csv.DictReader(f)))
    metadata = json.loads((folder / "metadata.json").read_text(encoding="utf-8"))
    if metadata["study_kind"] != "rate-simd-fir":
        raise ValueError("Isolated Drive data requires rate SIMD study provenance")
    lines = ["# Isolated premium Drive cost", "",
             f"Source: {metadata['source_sha']}; backend: {metadata['fir_backend']}; Release.",
             "16 independent Drive instances; 20 dB; 16384 timed stereo frames per instance.",
             "1024 warmup frames per instance, outside timing; four alternating pairs.",
             "Checksum accumulation remains inside timing. No synth, lowpass, FX or host wrapper.",
             "Each ratio is SIMD/scalar; table reports the median of four pair ratios.",
             "This isolates whole Drive cost, not FIR instructions or the cause of engine slowdown.",
             "Shared CI wall time; no portable realtime acceptance or timing threshold.", "",
             "| Factor | Rate | Median paired ratio |",
             "| --- | --- | ---: |"]
    for factor in (2, 4):
        for rate in (48000, 96000, 192000):
            ratio = statistics.median(pairs[(factor, rate, p)] for p in range(4))
            lines.append(f"| {factor} | {rate} | {ratio:.6f} |")
    text = "\n".join(lines) + "\n"
    (folder / "drive-report.md").write_text(text, encoding="utf-8")
    print(text)
    print("Validated 24 isolated Drive pairs.")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("Usage: report-premium-drive.py [--self-test | measurement-directory]")
    if sys.argv[1] == "--self-test":
        self_test()
    else:
        report(sys.argv[1])
