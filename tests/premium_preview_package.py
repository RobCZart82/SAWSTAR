# SPDX-License-Identifier: MIT
import hashlib
import importlib.util
import json
from pathlib import Path
import plistlib
import tempfile
import unittest
from unittest.mock import patch
import zipfile

spec = importlib.util.spec_from_file_location('preview_package', Path(__file__).resolve().parents[1] / 'scripts/package-premium-preview.py')
package = importlib.util.module_from_spec(spec)
spec.loader.exec_module(package)


class PreviewPackage(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.bundle = self.root / 'SAWSTARPremiumPreview.vst3'
        self.binary = self.bundle / 'Contents/MacOS/SAWSTARPremiumPreview'
        self.binary.parent.mkdir(parents=True)
        self.binary.write_bytes(b'test binary');self.binary.chmod(0o755)
        self.info = dict(CFBundleIdentifier='io.github.robczart82.sawstarpremiumpreview', CFBundleSignature='SwPP')
        (self.bundle / 'Contents/Info.plist').write_bytes(plistlib.dumps(self.info))
        for name in ('LICENSE','THIRD_PARTY_NOTICES.md','assets/branding/LICENSE.txt','docs/PREMIUM_PREVIEW.md','docs/PREMIUM_PREVIEW_LEVELS.md','docs/measurements/premium-preview-levels-2026-10-15-local-arm64.csv','third_party/licenses/notice.txt'):
            path = self.root / name;path.parent.mkdir(parents=True, exist_ok=True);path.write_text('test notice')
        self.output = self.root / 'preview.zip'

    def run_package(self):
        with patch.object(package, 'ROOT', self.root), patch.object(package.subprocess, 'check_output', return_value='a'*40):
            return package.package(self.bundle, self.output)

    def test_package_hashes_identity_and_executable_permissions(self):
        self.run_package()
        with zipfile.ZipFile(self.output) as archive:
            manifest = json.loads(archive.read('PACKAGE-MANIFEST.json'))
            self.assertTrue(manifest['experimental'])
            self.assertFalse(manifest['production_promotion_allowed'])
            self.assertEqual(manifest['latency_samples'],32)
            self.assertEqual(manifest['level_policy'],'wet-output-3db-factory-trim-v1')
            self.assertIn('PREMIUM_PREVIEW_LEVELS.md',archive.namelist())
            self.assertIn('measurements/premium-preview-levels-2026-10-15-local-arm64.csv',archive.namelist())
            for name, digest in manifest['files_sha256'].items():
                self.assertEqual(hashlib.sha256(archive.read(name)).hexdigest(),digest)
            self.assertTrue(archive.getinfo('SAWSTARPremiumPreview.vst3/Contents/MacOS/SAWSTARPremiumPreview').external_attr >> 16 & 0o111)
            self.assertFalse(any(name.startswith('SAWSTAR.vst3/') for name in archive.namelist()))
        self.assertEqual(self.output.with_suffix('.zip.sha256').read_text().split()[0],hashlib.sha256(self.output.read_bytes()).hexdigest())

    def test_rejects_production_identity(self):
        for key, value in [('CFBundleIdentifier','io.github.robczart82.sawstar'),('CFBundleSignature','SwSt')]:
            info = dict(self.info);info[key]=value
            (self.bundle / 'Contents/Info.plist').write_bytes(plistlib.dumps(info))
            with self.subTest(key=key), self.assertRaisesRegex(ValueError,'identity'): self.run_package()
        self.assertFalse(self.output.exists())

    def test_rejects_missing_binary(self):
        self.binary.unlink()
        with self.assertRaisesRegex(ValueError,'executable'): self.run_package()
        self.assertFalse(self.output.exists())


if __name__ == '__main__': unittest.main()
