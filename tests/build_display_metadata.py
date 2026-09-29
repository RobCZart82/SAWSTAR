# SPDX-License-Identifier: MIT
"""Exercise the same CMake metadata used in the actual generated plugin header."""
import json
from pathlib import Path
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[1]
cmake = sys.argv[1] if len(sys.argv) > 1 else 'cmake'
cases = [
    ('dev', '', '1.0.3 dev', 'Build'),
    ('rc2', '', '1.0.3 rc2', 'Build'),
    ('', '2026.09.19.', '1.0.3', 'Release'),
    ('', '', '1.0.3 Pre Release', 'Build'),
    ('dev', '2026.09.19.', None, None),
    ('invalid', '', None, None),
    ('', '2026-09-19', None, None),
]
for candidate, date, display, label in cases:
    with tempfile.TemporaryDirectory() as folder:
        folder = Path(folder)
        metadata = json.dumps(dict(candidate=candidate, release_date=date))
        script = folder / 'check.cmake'
        output = folder / 'BuildVersion.h'
        script.write_text(f'''set(PROJECT_VERSION "1.0.3")
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 3)
set(SAWSTAR_BUILD_ID "test123")
set(SAWSTAR_METADATA [==[{metadata}]==])
include("{root.as_posix()}/cmake/BuildDisplayMetadata.cmake")
configure_file("{root.as_posix()}/src/plugin/BuildVersion.h.in" "{output.as_posix()}" @ONLY)
''')
        result = subprocess.run([cmake, '-P', str(script)], capture_output=True, text=True)
        if display is None:
            assert result.returncode != 0, (candidate, date)
            continue
        assert result.returncode == 0, result.stderr
        header = output.read_text()
        assert f'#define SAWSTAR_DISPLAY_VERSION "{display}"' in header, header
        assert f'Version: {display} - {label}:' in header, header
        if label == 'Release':
            assert date in header, header
        else:
            assert 'Release:' not in header, header
print('Build display metadata: all seven release/dev/RC cases passed.')
