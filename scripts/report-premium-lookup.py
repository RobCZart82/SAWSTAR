#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Validate isolated lookup saturation pairs; timings never gate CI."""
import argparse
import copy
import csv
import hashlib
import json
import math
from pathlib import Path
import platform
import re
import statistics

SCALAR_FIELDS = {"drive_db", "pair", "order", "reference_seconds", "candidate_seconds"}
DRIVE_FIELDS = {"factor", "rate", "voices", "drive_db", "pair", "reference_seconds", "study_seconds"}
SCALAR_KEYS = {(db, pair) for db in (0, 12, 20, 24) for pair in range(8)}
DRIVE_KEYS = {(factor, rate, pair) for factor in (2, 4)
              for rate in (48000, 96000, 192000) for pair in range(8)}


def validate(rows, scalar):
    expected = SCALAR_KEYS if scalar else DRIVE_KEYS
    fields = SCALAR_FIELDS if scalar else DRIVE_FIELDS
    result = {}
    for row in rows:
        if set(row) != fields:
            raise ValueError("Unexpected lookup CSV columns")
        pair = int(row["pair"])
        if scalar:
            key = (int(row["drive_db"]), pair)
            if row["order"] != ("study-first" if pair % 2 else "reference-first"):
                raise ValueError("Wrong scalar pair order")
        else:
            key = (int(row["factor"]), int(row["rate"]), pair)
            if (int(row["voices"]), float(row["drive_db"])) != (16, 20.):
                raise ValueError("Wrong Drive fixture")
        if key not in expected or key in result:
            raise ValueError("Unexpected or duplicate lookup pair")
        a = float(row["reference_seconds"])
        b = float(row["candidate_seconds" if scalar else "study_seconds"])
        if not all(math.isfinite(x) and x > 0 for x in (a, b)):
            raise ValueError("Invalid lookup timing")
        ratio = b / a
        if not math.isfinite(ratio) or ratio <= 0:
            raise ValueError("Invalid lookup ratio")
        result[key] = ratio
    if set(result) != expected:
        raise ValueError("Incomplete lookup grid")
    return result


def self_test():
    rejected = 0
    for scalar in (True, False):
        if scalar:
            rows = [dict(drive_db=str(db), pair=str(pair),
                         order="study-first" if pair % 2 else "reference-first",
                         reference_seconds="2", candidate_seconds="1")
                    for db, pair in sorted(SCALAR_KEYS)]
        else:
            rows = [dict(factor=str(f), rate=str(r), voices="16", drive_db="20",
                         pair=str(p), reference_seconds="2", study_seconds="1")
                    for f, r, p in sorted(DRIVE_KEYS)]
        if set(validate(rows, scalar).values()) != {.5}:
            raise RuntimeError("Wrong lookup paired ratio")
        cases = [rows[:-1], rows + [rows[0]]]
        changes = [("pair", "8"), ("drive_db", "19"), ("reference_seconds", "0"),
                   ("reference_seconds", "nan"), ("reference_seconds", "-1"),
                   ("candidate_seconds" if scalar else "study_seconds", "inf")]
        changes += [("order", "study-first")] if scalar else [("factor", "3"), ("rate", "44100"), ("voices", "8")]
        for field, value in changes:
            case = copy.deepcopy(rows); case[0][field] = value; cases.append(case)
        case = copy.deepcopy(rows); case[0]["extra"] = "1"; cases.append(case)
        case = copy.deepcopy(rows); case[0]["reference_seconds"] = "1e-300"
        case[0]["candidate_seconds" if scalar else "study_seconds"] = "1e300"
        cases.append(case)
        for case in cases:
            try:
                validate(case, scalar)
            except ValueError:
                rejected += 1
            else:
                raise RuntimeError("Invalid lookup pairs accepted")
    print(f"Lookup reporter: two valid grids; {rejected} malformed controls rejected.")


def report(folder, source):
    if not re.fullmatch(r"[0-9a-f]{40}", source):
        raise ValueError("Exact source commit required")
    folder = Path(folder)
    with (folder / "scalar.csv").open(newline="") as file:
        scalar = validate(list(csv.DictReader(file)), True)
    with (folder / "drive.csv").open(newline="") as file:
        drive = validate(list(csv.DictReader(file)), False)
    backend = dict(part.split("=", 1) for part in (folder / "backend.txt").read_text().strip().split(";"))
    if backend != {"fir_backend": backend.get("fir_backend"), "shared_lookup_bytes": "65536"} or backend["fir_backend"] not in ("SSE2", "NEON", "scalar-fallback"):
        raise ValueError("Wrong lookup/backend record")
    root = Path(__file__).resolve().parents[1]
    sources = ("experiments/premium_filter/LookupTanh.h", "experiments/premium_filter/PremiumDrive.h",
               "experiments/premium_filter/FirLanes4.h", "tests/premium_lookup_tanh.cpp",
               "tests/premium_drive_tanh.cpp", "scripts/report-premium-lookup.py")
    metadata = {"study_kind": "lookup-tanh", "source_sha": source,
                "platform": platform.platform(), "machine": platform.machine(),
                "configuration": "Release", **backend, "scalar_pairs": 32,
                "drive_pairs": 48, "native_host_acceptance": False,
                "source_sha256": {p: hashlib.sha256((root / p).read_bytes()).hexdigest() for p in sources},
                "measurement_sha256": {p: hashlib.sha256((folder / p).read_bytes()).hexdigest()
                                        for p in ("scalar.csv", "drive.csv", "backend.txt")},
                "compiler_sha256": {p.name: hashlib.sha256(p.read_bytes()).hexdigest()
                                    for p in sorted((folder / "compiler").glob("*")) if p.is_file()}}
    lines = ["# Isolated lookup saturation study", "", f"Source: {source}; backend: {backend['fir_backend']}; Release.",
             "Eight alternating pairs per scene; median of candidate/reference time ratios.",
             "Shared 64 KiB immutable coefficients, prepared on Init, plus one pointer per opted-in Drive.",
             "Scalar: 65536 precomputed samples, sixteen timed passes; no FIR or engine.",
             "Drive: sixteen independent instances, 20 dB, 16384 stereo frames and 1024 warmup frames.",
             "Both Drive paths use the same SIMD FIR and reciprocal normalization; only tanh differs.",
             "Init/allocation/warmup excluded; checksum accumulation included. No synth, filter, FX or host.",
             "A shared-runner wall-clock observation, not portable realtime acceptance or an audibility claim.", "",
             "| Scalar gain dB | Median paired ratio |", "| ---: | ---: |"]
    for db in (0, 12, 20, 24):
        lines.append(f"| {db} | {statistics.median(scalar[(db, p)] for p in range(8)):.6f} |")
    lines += ["", "The real Drive bypasses nonlinear shaping at unity gain; scalar 0 dB timing does not imply Drive savings.",
              "", "| Drive factor | Rate | Median paired ratio |", "| ---: | ---: | ---: |"]
    for f in (2, 4):
        for r in (48000, 96000, 192000):
            lines.append(f"| {f} | {r} | {statistics.median(drive[(f, r, p)] for p in range(8)):.6f} |")
    (folder / "metadata.json").write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")
    text = "\n".join(lines) + "\n"
    (folder / "report.md").write_text(text, encoding="utf-8")
    print(text)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("folder", nargs="?")
    parser.add_argument("--source-sha")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    if args.self_test:
        self_test()
    elif args.folder and args.source_sha:
        report(args.folder, args.source_sha)
    else:
        parser.error("folder and --source-sha are required unless --self-test")
