# SPDX-License-Identifier: MIT
import importlib.util
from pathlib import Path
import tempfile
import unittest
import zipfile,json,hashlib
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('rc',ROOT/'scripts/package-premium-rc.py')
rc=importlib.util.module_from_spec(spec);spec.loader.exec_module(rc)
class CacheTests(unittest.TestCase):
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
if __name__=='__main__':unittest.main()
