# SPDX-License-Identifier: MIT
import importlib.util
from pathlib import Path
import tempfile
import unittest
import zipfile,json,hashlib,subprocess
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('rc',ROOT/'scripts/package-premium-rc.py')
rc=importlib.util.module_from_spec(spec);spec.loader.exec_module(rc)
class CacheTests(unittest.TestCase):
    def test_rc_supplement_matches_current_candidate_and_factory_count(self):
        meta=json.loads((ROOT/'release.json').read_text())
        supplement=(ROOT/'docs/PREMIUM_PRODUCTION_RC.md').read_text()
        self.assertIn(f"{meta['version']} {meta['candidate']}-premium-integration",supplement)
        self.assertIn('30 preset × 2 ráta × 3 hangszám = 180 alaphelyzetben',supplement)
        self.assertIn('összesen 252 esetet architektúránként',supplement)
        self.assertNotIn('1.2.0 rc1-premium-integration',supplement)
    def test_wrong_engine_or_identity_is_rejected(self):
        base={'SAWSTAR_PREMIUM_PRODUCTION':'ON','SAWSTAR_BUILD_PLUGIN':'ON',
              'SAWSTAR_PREMIUM_PREVIEW':'OFF','SAWSTAR_PREMIUM_NATIVE_CANDIDATE':'OFF',
              'CMAKE_HOME_DIRECTORY':str(ROOT)}
        with tempfile.TemporaryDirectory() as folder:
            p=Path(folder)/'CMakeCache.txt'
            def write(values):p.write_text(''.join(f'{k}:STRING={v}\n' for k,v in values.items()))
            write(base);rc.integration_cache(p)
            for key,value in [('SAWSTAR_PREMIUM_PRODUCTION','OFF'),('SAWSTAR_BUILD_PLUGIN','OFF'),
                              ('SAWSTAR_PREMIUM_PREVIEW','ON'),('SAWSTAR_PREMIUM_NATIVE_CANDIDATE','ON'),
                              ('CMAKE_HOME_DIRECTORY',folder)]:
                with self.subTest(key=key):
                    write(dict(base,**{key:value}))
                    with self.assertRaises(ValueError):rc.integration_cache(p)
    def test_rc_payloads_are_hashed_without_duplicate_manifest(self):
        with tempfile.TemporaryDirectory() as folder:
            p=Path(folder)/'rc.zip'
            with zipfile.ZipFile(p,'w') as z:
                z.writestr('SAWSTAR.vst3/binary',b'fixture')
                z.writestr('PACKAGE-MANIFEST.json',json.dumps({'files_sha256':{}}))
            rc.finalize_rc_archive(p,{'publication_allowed':False,'native_host_acceptance':False})
            with zipfile.ZipFile(p) as z:
                self.assertEqual(len(z.namelist()),len(set(z.namelist())))
                hashes=json.loads(z.read('PACKAGE-MANIFEST.json'))['files_sha256']
                self.assertEqual(set(hashes),set(z.namelist())-{'PACKAGE-MANIFEST.json'})
                for name,digest in hashes.items():
                    self.assertEqual(hashlib.sha256(z.read(name)).hexdigest(),digest)
                self.assertFalse(json.loads(z.read('PREMIUM-RC-MANIFEST.json'))['publication_allowed'])
            self.assertEqual(p.with_suffix('.zip.sha256').read_text().split()[0],hashlib.sha256(p.read_bytes()).hexdigest())
    def test_packaged_markdown_links_must_resolve_in_package_or_be_remote(self):
        valid={
            'docs/README.md': b'[Requirements](SYSTEM_REQUIREMENTS.md) [history](https://example.test/history)',
            'docs/SYSTEM_REQUIREMENTS.md': b'Available in this package.',
        }
        rc.validate_markdown_payload_links(valid)
        invalid=dict(valid, **{'docs/PREMIUM_PRODUCTION_RC.md':
                               b'[old release](RELEASE_NOTES_1.2.0.md) '
                               b'[factory evidence](EXPRESSIVE_FACTORY_PRESETS_2026_11_03.md)'})
        with self.assertRaisesRegex(ValueError, 'Broken relative Markdown link'):
            rc.validate_markdown_payload_links(invalid)
    def test_rejected_rc_archive_never_appears_under_final_names(self):
        with tempfile.TemporaryDirectory() as folder:
            output=Path(folder)/'candidate.zip'
            broken=b'[missing](NOT-SHIPPED.md)'
            def package(_bundle_parent, staged):
                with zipfile.ZipFile(staged,'w') as archive:
                    archive.writestr('docs/BROKEN.md',broken)
                    archive.writestr('PACKAGE-MANIFEST.json',json.dumps({'files_sha256':{}}))
                staged.with_suffix(staged.suffix+'.sha256').write_text('temporary')
            with self.assertRaisesRegex(ValueError, 'Broken relative Markdown link'):
                rc.package_rc_atomically(package,None,output,{'publication_allowed':False})
            self.assertFalse(output.exists())
            self.assertFalse(output.with_suffix(output.suffix+'.sha256').exists())
            self.assertEqual(list(Path(folder).iterdir()),[])
    def test_provenance_rejects_unknown_changes_and_partial_patches(self):
        from rc_provenance import require_paths, require_bytes
        require_paths(['third_party/iPlug2'],{'third_party/iPlug2'},'checkout')
        with self.assertRaises(ValueError):
            require_paths(['third_party/iPlug2','src/engine/Synth.cpp'],{'third_party/iPlug2'},'checkout')
        require_bytes(b'whole\r\nfile\r\n',b'whole\nfile\n','line endings')
        with self.assertRaises(ValueError):
            require_bytes(b'whole\nfile\nextra drift',b'whole\nfile\n','unexpected bytes')
    def test_provenance_rejects_actual_checkout_or_gitlink_drift(self):
        from rc_provenance import checkout_provenance, dependency_provenance
        with patch('rc_provenance.git',return_value=b' M src/engine/Synth.cpp\0'):
            with self.assertRaisesRegex(ValueError,'checkout changes'):
                checkout_provenance(ROOT)
        with patch('rc_provenance.git',side_effect=[b'pinned',b'different']):
            with self.assertRaisesRegex(ValueError,'gitlink'):
                dependency_provenance(ROOT)
    def test_untracked_iplug_source_is_rejected_with_only_sdk_exempted(self):
        from rc_provenance import require_managed_untracked
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)
            subprocess.run(['git','init','--quiet',folder],check=True)
            sdk=root/'Dependencies/IPlug/VST3_SDK';sdk.mkdir(parents=True)
            (sdk/'README.md').write_text('separately verified SDK fixture')
            require_managed_untracked(root)
            unexpected=root/'unexpected.h';unexpected.write_text('unexplained source')
            with self.assertRaisesRegex(ValueError,'untracked iPlug2'):
                require_managed_untracked(root)
            unexpected.unlink()
            require_managed_untracked(root)
if __name__=='__main__':unittest.main()
