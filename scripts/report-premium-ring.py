#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Validate ring-capacity pairs and retain provenance; timings never gate CI."""
import argparse
import copy
import csv
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import re
import statistics

REFERENCE_SHA = "c5f5d6351b7dbed9e30af329c99934dcda5cc7f6"
FIELDS = ("factor", "implementation", "rate", "voices", "drive_db", "pair",
          "reference_bytes", "candidate_bytes", "reference_seconds", "candidate_seconds")
EXPECTED = {(f, i, r, p) for f in (2, 4) for i in ("scalar", "simd")
            for r in (48000, 96000, 192000) for p in range(8)}
SOURCES = ("experiments/premium_filter/PremiumDrive.h",
           "experiments/premium_filter/FirLanes4.h",
           "experiments/premium_filter/ResearchTanh.h",
           "experiments/premium_filter/benchmark_ring_storage.cpp",
           "tests/fixtures/premium_drive_large_ring_reference.h",
           "scripts/report-premium-ring.py")


def validate(rows, normalization=False):
    ratios, sizes = {}, {}
    for row in rows:
        if set(row) != set(FIELDS):
            raise ValueError("Unexpected ring CSV columns")
        key = (int(row["factor"]), row["implementation"], int(row["rate"]), int(row["pair"]))
        if key not in EXPECTED or key in ratios:
            raise ValueError("Unexpected or duplicate ring pair")
        if (int(row["voices"]), float(row["drive_db"])) != (16, 20.):
            raise ValueError("Unexpected ring fixture")
        old, new = int(row["reference_bytes"]), int(row["candidate_bytes"])
        reduction = 4096 if key[0] == 2 and not normalization else 0
        if new <= 0 or old - new != reduction:
            raise ValueError("Unexpected ring storage reduction")
        size_key = key[:2]
        if size_key in sizes and sizes[size_key] != (old, new):
            raise ValueError("Inconsistent object sizes")
        sizes[size_key] = (old, new)
        a, b = float(row["reference_seconds"]), float(row["candidate_seconds"])
        if not all(math.isfinite(x) and x > 0 for x in (a, b)):
            raise ValueError("Invalid ring timing")
        ratio = b / a
        if not math.isfinite(ratio) or ratio <= 0:
            raise ValueError("Invalid paired ratio")
        ratios[key] = ratio
    if set(ratios) != EXPECTED:
        raise ValueError("Incomplete ring grid")
    for factor in (2, 4):
        if sizes[(factor, "scalar")] != sizes[(factor, "simd")]:
            raise ValueError("SIMD changed persistent storage")
    return ratios


def self_test(normalization=False):
    rows = [dict(zip(FIELDS, map(str, (f, i, r, 16, 20, p,
                                     (7248 if normalization else 11344) if f == 2 else 12384,
                                     7248 if f == 2 else 12384, 2, 1))))
            for f, i, r, p in sorted(EXPECTED)]
    if set(validate(rows, normalization).values()) != {.5}:
        raise RuntimeError("Incorrect paired ratios")
    invalid = [rows[:-1], rows + [rows[0]]]
    for field, value in (("factor", "3"), ("implementation", "avx"), ("rate", "44100"),
                         ("pair", "8"), ("voices", "8"), ("drive_db", "0"),
                         ("candidate_bytes", "7247"), ("candidate_bytes", "0"),
                         ("reference_seconds", "0"), ("candidate_seconds", "-1"),
                         ("reference_seconds", "nan"), ("candidate_seconds", "inf"),
                         ("reference_seconds", "1e-300")):
        case = copy.deepcopy(rows)
        case[0][field] = value
        if value == "1e-300":
            case[0]["candidate_seconds"] = "1e300"
        invalid.append(case)
    case = copy.deepcopy(rows)
    case[0]["extra"] = "1"
    invalid.append(case)
    case = copy.deepcopy(rows)
    case[0]["reference_bytes"] = str(int(case[0]["reference_bytes"]) + 8)
    case[0]["candidate_bytes"] = str(int(case[0]["candidate_bytes"]) + 8)
    invalid.append(case)
    case = copy.deepcopy(rows)
    for row in case:
        if row["factor"] == "2" and row["implementation"] == "simd":
            row["reference_bytes"] = str(int(row["reference_bytes"]) + 8)
            row["candidate_bytes"] = str(int(row["candidate_bytes"]) + 8)
    invalid.append(case)
    for case in invalid:
        try:
            validate(case, normalization)
        except ValueError:
            continue
        raise RuntimeError("Malformed ring data was accepted")
    print(f"Ring reporter: {len(invalid)} rejecting controls passed")


def report(directory, source_sha, normalization=False):
    if source_sha is not None and not re.fullmatch(r"[0-9a-f]{40}", source_sha):
        raise ValueError("Source SHA must be the actual checked-out 40-digit commit")
    path = directory / "ring.csv"
    with path.open(newline="", encoding="utf-8") as file:
        rows = list(csv.DictReader(file))
    ratios = validate(rows, normalization)
    root = Path(__file__).resolve().parents[1]
    summaries = []
    for factor in (2, 4):
        for implementation in ("scalar", "simd"):
            for rate in (48000, 96000, 192000):
                values = [ratios[(factor, implementation, rate, p)] for p in range(8)]
                row = dict(factor=factor, implementation=implementation, rate=rate,
                           median_ratio=statistics.median(values),
                           minimum_ratio=min(values), maximum_ratio=max(values))
                summaries.append(row)
                print(f"{factor}x {implementation} {rate}: median candidate/reference={row['median_ratio']:.6f}, range={min(values):.6f}..{max(values):.6f}")
    compiler_files = sorted((directory / "compiler").glob("CMakeCXXCompiler.cmake"))
    compiler_text = "\n".join(p.read_text(encoding="utf-8") for p in compiler_files)
    compiler = {}
    for name in ("ID", "VERSION", "ARCHITECTURE_ID"):
        match = re.search(r'set\(CMAKE_CXX_COMPILER_' + name + r' "([^"]*)"\)', compiler_text)
        if match:
            compiler[name.lower()] = match.group(1)
    sources = SOURCES + (("tests/premium_gain_normalization.cpp",) if normalization else ())
    metadata = dict(source_sha=source_sha, reference_sha=None if normalization else REFERENCE_SHA,
                    study="gain-normalization" if normalization else "ring-capacity",
                    reference_policy="same-source default division branch" if normalization else "frozen large-ring fixture",
                    source_state="checked-out CI commit" if source_sha else "uncommitted local working files",
                    platform=platform.platform(), machine=platform.machine(),
                    run_id=os.getenv("GITHUB_RUN_ID"), attempt=os.getenv("GITHUB_RUN_ATTEMPT"),
                    rows=len(rows), pairs_per_case=8, voices=16, drive_db=20,
                    frames_per_pair=16384, warmup_frames=1024,
                    vector_backend=(directory / "backend.txt").read_text(encoding="utf-8").strip(),
                    csv_sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                    source_sha256={p: hashlib.sha256((root / p).read_bytes()).hexdigest() for p in sources},
                    compiler=compiler,
                    compiler_sha256={p.name: hashlib.sha256(p.read_bytes()).hexdigest()
                                     for p in sorted((directory / "compiler").glob("*")) if p.is_file()},
                    summaries=summaries, timing_is_acceptance_gate=False)
    if metadata["vector_backend"] not in ("vector_backend=SSE2", "vector_backend=NEON", "vector_backend=scalar-fallback"):
        raise ValueError("Unexpected vector backend")
    (directory / "provenance.json").write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", nargs="?", type=Path)
    parser.add_argument("--source-sha")
    parser.add_argument("--self-test", action="store_true")
    parser.add_argument("--normalization", action="store_true")
    args = parser.parse_args()
    if args.self_test:
        self_test(args.normalization)
    elif args.directory:
        report(args.directory, args.source_sha, args.normalization)
    else:
        parser.error("provide a results directory or --self-test")
