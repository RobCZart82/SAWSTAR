# SPDX-License-Identifier: MIT
"""Compile the production GUI state handoff methods against a thread-observing host."""
from pathlib import Path
import sys

source = (Path(__file__).resolve().parents[1] / 'src/plugin/SAWSTAR.cpp').read_text()
functions = []
for signature in ('void SAWSTAR::SaveGuiScale(', 'void SAWSTAR::SyncRestoredGuiScale(',
                  'bool SAWSTAR::SerializeState(', 'int SAWSTAR::UnserializeState('):
    start = source.index(signature)
    end = source.index('\n}', start) + 2
    functions.append(source[start:end])
Path(sys.argv[1]).write_text('\n\n'.join(functions) + '\n', encoding='utf-8')
