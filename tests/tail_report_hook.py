# SPDX-License-Identifier: MIT
"""Compile the real host-reporting calls, not just the standalone estimator."""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
source = (root / 'src/plugin/SAWSTAR.cpp').read_text()
constructor = source.split('SAWSTAR::SAWSTAR(', 1)[1].split('void SAWSTAR::SyncRestoredPreset', 1)[0]
reset = source.split('void SAWSTAR::OnReset() {', 1)[1].split('void SAWSTAR::ProcessBlock', 1)[0]
calls = []
for region in (constructor, reset):
    matches = re.findall(r'SetTailSize\(sawstar::TailSamples\([^;]+;', region)
    assert len(matches) == 1, 'Tail must be reported at construction and each sample-rate reset'
    calls.append(matches[0])
assert 'GetSampleRate()' in calls[1], 'Reset must refresh the sample-count report'
Path(sys.argv[1]).write_text('void initialize() {' + calls[0] + '}\n'
                           'void reset() {' + calls[1] + '}\n')
