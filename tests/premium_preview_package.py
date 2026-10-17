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
        self.info = dict(CFBundleIdentifier='io.github.robczart82.sawstarpremiumpreview', CFBundleSignature='SwPP', CFBundleExecutable='SAWSTARPremiumPreview', CFBundleName='SAWSTARPremiumPreview')
        (self.bundle / 'Contents/Info.plist').write_bytes(plistlib.dumps(self.info))
        for name in ('LICENSE','THIRD_PARTY_NOTICES.md','assets/branding/LICENSE.txt','docs/PREMIUM_PREVIEW.md','docs/NATIVE_PREVIEW_PAIR_QUALIFICATION.md','docs/PREMIUM_PREVIEW_LEVELS.md','docs/measurements/premium-preview-levels-2026-10-15-local-arm64.csv','third_party/licenses/notice.txt'):
            path = self.root / name;path.parent.mkdir(parents=True, exist_ok=True);path.write_text('test notice')
        self.output = self.root / 'preview.zip'

    def run_package(self, role='reference'):
        with patch.object(package, 'ROOT', self.root), patch.object(package.subprocess, 'check_output', return_value='a'*40):
            return package.package(self.bundle, self.output, role=role)

    def test_package_hashes_identity_and_executable_permissions(self):
        self.run_package()
        with zipfile.ZipFile(self.output) as archive:
            manifest = json.loads(archive.read('PACKAGE-MANIFEST.json'))
            self.assertEqual(manifest['pair_role'],'reference')
            self.assertEqual(manifest['oscillator'],'SevenSaw')
            self.assertFalse(manifest['native_host_acceptance'])
            self.assertEqual(manifest['binary_sha256'],hashlib.sha256(self.binary.read_bytes()).hexdigest())
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

    def test_candidate_has_distinct_identity_binary_and_manifest(self):
        original = self.bundle
        self.bundle = self.root / 'SAWSTARPremiumCandidate.vst3'
        original.rename(self.bundle)
        self.binary = self.bundle / 'Contents/MacOS/SAWSTARPremiumCandidate'
        self.binary.with_name('SAWSTARPremiumPreview').rename(self.binary)
        info = dict(CFBundleIdentifier='io.github.robczart82.sawstarpremiumcandidate', CFBundleSignature='SwPC', CFBundleExecutable='SAWSTARPremiumCandidate', CFBundleName='SAWSTARPremiumCandidate')
        (self.bundle / 'Contents/Info.plist').write_bytes(plistlib.dumps(info))
        with self.assertRaisesRegex(ValueError,'Expected'): self.run_package()
        self.run_package('candidate')
        with zipfile.ZipFile(self.output) as archive:
            manifest = json.loads(archive.read('PACKAGE-MANIFEST.json'))
            self.assertEqual(manifest['pair_role'],'candidate')
            self.assertEqual(manifest['plugin_unique_id'],'SwPC')
            self.assertEqual(manifest['oscillator'],'SawDispatchSevenSaw')
            self.assertFalse(manifest['native_host_acceptance'])
            self.assertFalse(manifest['production_promotion_allowed'])
            self.assertEqual(manifest['binary_sha256'],hashlib.sha256(self.binary.read_bytes()).hexdigest())
            for name, digest in manifest['files_sha256'].items():
                self.assertEqual(hashlib.sha256(archive.read(name)).hexdigest(),digest)
            self.assertFalse(any(name.startswith('SAWSTARPremiumPreview.vst3/') for name in archive.namelist()))
        # A renamed reference must not be accepted as a Candidate.
        (self.bundle / 'Contents/Info.plist').write_bytes(plistlib.dumps(self.info))
        with self.assertRaisesRegex(ValueError,'identity'): self.run_package('candidate')

    def test_wrong_role_and_symlink_rejected_before_packaging(self):
        with self.assertRaisesRegex(ValueError,'Unknown'): self.run_package('production')
        with self.assertRaisesRegex(ValueError,'Expected'): self.run_package('candidate')
        (self.bundle / 'Contents/link').symlink_to(self.binary)
        with self.assertRaisesRegex(ValueError,'symlink'): self.run_package()
        self.assertFalse(self.output.exists())

    def test_rejects_production_identity(self):
        for key, value in [('CFBundleIdentifier','io.github.robczart82.sawstar'),('CFBundleSignature','SwSt'),('CFBundleExecutable','SAWSTAR'),('CFBundleName','SAWSTAR')]:
            info = dict(self.info);info[key]=value
            (self.bundle / 'Contents/Info.plist').write_bytes(plistlib.dumps(info))
            with self.subTest(key=key), self.assertRaisesRegex(ValueError,'identity'): self.run_package()
        self.assertFalse(self.output.exists())

    def test_rejects_missing_binary(self):
        self.binary.unlink()
        with self.assertRaisesRegex(ValueError,'executable'): self.run_package()
        self.assertFalse(self.output.exists())


if __name__ == '__main__': unittest.main()
