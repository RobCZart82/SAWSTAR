# SPDX-License-Identifier: MIT
"""Compile the production parameter-registration loop with a framework recorder."""
from pathlib import Path
import re
import sys


def extract(source: str) -> str:
    constructor = source.split("SAWSTAR::SAWSTAR(", 1)
    if len(constructor) != 2:
        raise ValueError("Cannot find the production SAWSTAR constructor")
    registration = re.search(
        r"(?P<loop>for\s*\(const auto& spec\s*:\s*sawstar::kParameters\)\s*\{"
        r".*?)\n\s*MakeDefaultPreset\(\"Init\",\s*1\);",
        constructor[1],
        re.DOTALL,
    )
    if registration is None:
        raise ValueError("Cannot extract the complete production registration loop")
    loop = registration.group("loop").rstrip()
    if not loop.endswith("}") or loop.count("{") != loop.count("}"):
        raise ValueError("Production registration loop is incomplete")
    if not all(token in loop for token in (
        "GetParam(", "param->InitEnum(", "param->InitInt(", "param->InitDouble(",
    )):
        raise ValueError("Production parameter-registration contract changed")
    return loop + "\n"


def main() -> None:
    if len(sys.argv) != 2:
        raise ValueError("Expected the generated registration include path")
    root = Path(__file__).resolve().parents[1]
    source = (root / "src/plugin/SAWSTAR.cpp").read_text(encoding="utf-8")
    Path(sys.argv[1]).write_text(extract(source), encoding="utf-8")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError) as error:
        raise SystemExit(f"Cannot verify production parameter registration: {error}")
