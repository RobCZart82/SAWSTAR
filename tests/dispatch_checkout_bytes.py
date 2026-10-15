# SPDX-License-Identifier: MIT
"""Exercise real Git LF source checkout under simulated Windows settings."""
import hashlib
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ('CMakeLists.txt', 'src/engine/example.cpp', 'experiments/premium_filter/example.h')
SOURCE_BYTES = b'// source fixture\nint value = 1;\n'
EVIDENCE = 'experiments/oscillator/measurements/2026-10-09-saw-dispatch-ci-summary/evidence.csv'
EVIDENCE_BYTES = b'column,value\r\noriginal,1\r\n'


def git(folder, *args, windows=False):
    command = ['git', '-c', 'core.autocrlf=' + ('true' if windows else 'false'),
               '-c', 'core.eol=' + ('crlf' if windows else 'lf'),
               '-c', 'user.name=SAWSTAR checkout test',
               '-c', 'user.email=checkout-test@example.invalid', '-C', str(folder), *args]
    return subprocess.check_output(command, stderr=subprocess.STDOUT,
                                   env=dict(os.environ, GIT_CONFIG_GLOBAL=os.devnull,
                                            GIT_CONFIG_NOSYSTEM='1'))


class CheckoutBytes(unittest.TestCase):
    def checkout(self, attributes):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        repo = Path(temp.name)
        git(repo, 'init')
        (repo / '.gitattributes').write_bytes(attributes.encode('utf-8'))
        for name, data in [(p, SOURCE_BYTES) for p in SOURCES] + [(EVIDENCE, EVIDENCE_BYTES)]:
            target = repo / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        git(repo, 'add', '.')
        git(repo, 'commit', '-m', 'fixture')
        for name in SOURCES + (EVIDENCE,):
            (repo / name).unlink()
        git(repo, 'checkout', '--', '.', windows=True)
        return repo

    def test_source_bytes_stay_lf_and_archived_evidence_stays_crlf(self):
        repo = self.checkout((ROOT / '.gitattributes').read_text(encoding='utf-8'))
        expected = hashlib.sha256(SOURCE_BYTES).digest()
        for name in SOURCES:
            with self.subTest(path=name):
                self.assertEqual(hashlib.sha256((repo / name).read_bytes()).digest(), expected)
                self.assertEqual(git(repo, 'show', 'HEAD:' + name), SOURCE_BYTES)
        self.assertEqual((repo / EVIDENCE).read_bytes(), EVIDENCE_BYTES)

    def test_negative_control_unpinned_checkout_changes_raw_hash(self):
        # Keep archival protection but deliberately omit the new source rules.
        repo = self.checkout(EVIDENCE.rsplit('/', 1)[0] + '/** -text\n')
        for name in SOURCES:
            with self.subTest(path=name):
                self.assertEqual((repo / name).read_bytes(), SOURCE_BYTES.replace(b'\n', b'\r\n'))
                self.assertNotEqual(hashlib.sha256((repo / name).read_bytes()).digest(),
                                    hashlib.sha256(SOURCE_BYTES).digest())
        self.assertEqual((repo / EVIDENCE).read_bytes(), EVIDENCE_BYTES)


if __name__ == '__main__':
    unittest.main()
