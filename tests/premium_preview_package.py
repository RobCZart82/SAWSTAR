# SPDX-License-Identifier: MIT
import hashlib
import importlib.util
import json
from pathlib import Path
import plistlib
import struct
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
        for name in ('LICENSE','THIRD_PARTY_NOTICES.md','assets/branding/LICENSE.txt','docs/PREMIUM_PREVIEW.md','docs/PREMIUM_PREVIEW_WINDOWS.md','docs/NATIVE_PREVIEW_PAIR_QUALIFICATION.md','docs/PREMIUM_PREVIEW_LEVELS.md','docs/measurements/premium-preview-levels-2026-10-15-local-arm64.csv','third_party/licenses/notice.txt'):
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


class WindowsPreviewPackage(unittest.TestCase):
    def setUp(self):
        PreviewPackage.setUp(self)
        self.binary.unlink()
        self.binary.parent.rmdir()
        (self.bundle / 'Contents/Info.plist').unlink()
        self.binary = self.bundle / 'Contents/x86_64-win/SAWSTARPremiumPreview.vst3'
        self.binary.parent.mkdir()
        image = bytearray(256)
        image[:2] = b'MZ'
        struct.pack_into('<I', image, 60, 128)
        image[128:132] = b'PE\0\0'
        struct.pack_into('<H', image, 132, 0x8664)
        struct.pack_into('<H', image, 152, 0x20b)
        self.binary.write_bytes(image)
        self.module_info = self.root / 'moduleinfo.json'
        self.info = {'Name':'SAWSTARPremiumPreview', 'Classes':[
            {'CID':'F2AEE70D00DE4F4E5243383253775050', 'Name':'SAWSTAR Premium Preview',
             'Category':'Audio Module Class', 'Vendor':'RobCZart82'}]}
        self.write_evidence()

    def write_evidence(self):
        self.module_info.write_text(json.dumps(self.info), encoding='utf-8')
        self.proof = dict(schema=1, source_commit='a'*40,
            sdk_commit='3cdf9ca5d1f5b1b21e0a86832aa4abe55607bd96',
            binary_relative_path=self.binary.relative_to(self.bundle).as_posix(),
            binary_sha256=hashlib.sha256(self.binary.read_bytes()).hexdigest(),
            module_info_sha256=hashlib.sha256(self.module_info.read_bytes()).hexdigest(),
            module_info_tool_sha256='d'*64, validator_sha256='e'*64,
            validator_passed=True, native_host_acceptance=False, production_promotion_allowed=False)
        self.save_proof()

    def save_proof(self):
        Path(str(self.module_info)+'.provenance.json').write_text(json.dumps(self.proof), encoding='utf-8')

    def run_package(self, role='reference'):
        with patch.object(package, 'ROOT', self.root), patch.object(package.subprocess, 'check_output', return_value='a'*40):
            return package.package(self.bundle, self.output, role=role,
                                   platform='windows-x64', module_info=self.module_info)

    def test_both_roles_have_binary_factory_identity_and_all_file_hashes(self):
        for role,stem,signature in [('reference','SAWSTARPremiumPreview','SwPP'),
                                    ('candidate','SAWSTARPremiumCandidate','SwPC')]:
            if role == 'candidate':
                previous = self.bundle
                self.bundle = self.root / (stem+'.vst3')
                previous.rename(self.bundle)
                old = self.bundle / 'Contents/x86_64-win/SAWSTARPremiumPreview.vst3'
                self.binary = old.with_name(stem+'.vst3');old.rename(self.binary)
                self.info['Name']=stem
                self.info['Classes'][0].update(Name='SAWSTAR Premium Candidate',
                                              CID='F2AEE70D00DE4F4E5243383253775043')
                self.write_evidence()
            with self.subTest(role=role):
                self.run_package(role)
                with zipfile.ZipFile(self.output) as archive:
                    manifest=json.loads(archive.read('PACKAGE-MANIFEST.json'))
                    self.assertEqual(manifest['platform'],'windows-x64')
                    self.assertEqual(manifest['pair_role'],role)
                    self.assertEqual(manifest['plugin_unique_id'],signature)
                    self.assertEqual(manifest['vst3_class_id'],self.info['Classes'][0]['CID'])
                    self.assertEqual(manifest['binary_sha256'],self.proof['binary_sha256'])
                    self.assertFalse(manifest['native_host_acceptance'])
                    self.assertFalse(manifest['production_promotion_allowed'])
                    self.assertIn('evidence/moduleinfo.provenance.json',archive.namelist())
                    for name,digest in manifest['files_sha256'].items():
                        self.assertEqual(hashlib.sha256(archive.read(name)).hexdigest(),digest)

    def test_renamed_classic_or_candidate_factory_is_rejected(self):
        for cid in ['F2AEE70D00DE4F4E5243383253775374','F2AEE70D00DE4F4E5243383253775043']:
            self.info['Classes'][0]['CID']=cid;self.write_evidence()
            with self.subTest(cid=cid),self.assertRaisesRegex(ValueError,'factory identity'):
                self.run_package()
        self.assertFalse(self.output.exists())

    def test_wrong_architecture_and_truncated_pe_rejected(self):
        image=self.binary.read_bytes()
        for data in [b'',b'MZ',image[:150],image[:132]+bytes.fromhex('64aa')+image[134:]]:
            self.binary.write_bytes(data);self.write_evidence()
            with self.subTest(length=len(data)),self.assertRaisesRegex(ValueError,'PE'):
                self.run_package()
        self.assertFalse(self.output.exists())

    def test_stale_source_binary_module_info_or_unvalidated_evidence_rejected(self):
        for key,value in [('source_commit','b'*40),('binary_sha256','0'*64),
                          ('binary_relative_path','Contents/x86_64-win/SAWSTAR.vst3'),
                          ('module_info_sha256','1'*64),('sdk_commit','c'*40),
                          ('validator_passed',False),('module_info_tool_sha256',''),('validator_sha256',''),
                          ('native_host_acceptance',True),
                          ('production_promotion_allowed',True)]:
            self.write_evidence();self.proof[key]=value;self.save_proof()
            with self.subTest(key=key),self.assertRaisesRegex(ValueError,'provenance'):
                self.run_package()
        self.assertFalse(self.output.exists())

    def test_sdk_trailing_commas_do_not_change_quoted_strings_or_raw_evidence(self):
        raw=br'{"Name":"comma,} and escaped \" quote", "Classes":[{"CID":"ABC",},],}'
        self.assertEqual(package.read_module_info(raw)['Name'],'comma,} and escaped " quote')
        original=self.module_info.read_bytes()
        # Match the pinned SDK writer: trailing object and array commas.
        data=original.replace(b'}]',b'},]').replace(b']}',b'],}')
        self.module_info.write_bytes(data)
        self.proof['module_info_sha256']=hashlib.sha256(data).hexdigest();self.save_proof()
        self.run_package()
        with zipfile.ZipFile(self.output) as archive:
            self.assertEqual(archive.read('evidence/moduleinfo.json'),data)
        with self.assertRaises(json.JSONDecodeError):package.read_module_info(b'{unquoted:1,}')


if __name__ == '__main__': unittest.main()
