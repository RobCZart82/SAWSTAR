# SPDX-License-Identifier: MIT
"""The release guard must skip candidates without creating or querying releases."""
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / 'scripts/publish-release.py'

class ReleaseGuard(unittest.TestCase):
    def run_guard(self, candidate):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder)
            (path / 'release.json').write_text(json.dumps({
                'version': '1.0.3', 'candidate': candidate, 'release_date': ''}))
            # No credentials, RELEASE_SHA or gh executable: candidate must stop first.
            result = subprocess.run([sys.executable, str(SCRIPT)], cwd=folder,
                                    env={}, capture_output=True, text=True)
            self.assertEqual(sorted(p.name for p in path.iterdir()), ['release.json'])
            return result

    def test_candidate_is_successful_skip(self):
        result = self.run_guard('rc1')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('skipped', result.stdout)

    def test_final_missing_release_context_still_fails(self):
        self.assertNotEqual(self.run_guard('').returncode, 0)

if __name__ == '__main__':
    unittest.main()
