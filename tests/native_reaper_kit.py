# SPDX-License-Identifier: MIT
import csv
import hashlib
import importlib.util
import itertools
import json
from pathlib import Path
import struct
import tempfile
import unittest
import zipfile

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('kit', ROOT / 'scripts/prepare-native-reaper-kit.py')
kit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(kit)


class NativeKit(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)

    def package(self, role, platform='windows-x64', source='a'*40, edits=None, damage=None):
        product, stem, unique_id, oscillator = kit.IDENTITIES[role]
        binary = stem + '.vst3/Contents/' + ('MacOS/' + stem if platform == 'macos' else
                                            'x86_64-win/' + stem + '.vst3')
        content = (role + '-fixture-not-a-loadable-plugin').encode()
        manifest = dict(product=product, pair_role=role, plugin_unique_id=unique_id,
            oscillator=oscillator, platform=platform, source_commit=source,
            experimental=True, native_host_acceptance=False, production_promotion_allowed=False,
            engine='RateScaledLookupEnginePremiumFilter', latency_samples=32,
            level_policy='wet-output-3db-factory-trim-v1', binary_sha256=kit.sha(content),
            files_sha256={binary:kit.sha(content)},
            bundle_identifier='io.github.robczart82.'+stem.lower(),
            vst3_class_id='F2AEE70D00DE4F4E52433832'+unique_id.encode().hex().upper())
        if edits:manifest.update(edits)
        path = self.root / (role + '-' + platform + '.zip')
        with zipfile.ZipFile(path,'w') as archive:
            archive.writestr(binary, damage if damage is not None else content)
            archive.writestr('PACKAGE-MANIFEST.json',json.dumps(manifest))
        self.checksum(path)
        return path

    def checksum(self, path):
        path.with_suffix('.zip.sha256').write_text(kit.sha(path.read_bytes())+'  '+path.name+'\n')

    def test_full_schedule_is_exact_and_observations_stay_empty(self):
        plan = kit.cases()
        c = {f'C-{b}-{m}-{a}' for b,m,a in itertools.product((128,64),
            ('LP12','LP24','HP12','BP12'),('AUTO','STATIC'))}
        v = {f'V-128-{n}-{m}' for n,m in itertools.product((1,8),('LP12','LP24','HP12','BP12'))}
        g = {'G-128-LP24-OPEN','G-64-LP24-OPEN'}
        f = {'F-128-P1','F-64-P1'}
        self.assertEqual({p['case_id'] for p in plan},c|v|g|f)
        self.assertEqual(len(plan),28)
        self.assertEqual([p['buffer_samples'] for p in plan], [128]*18+[64]*10)
        self.assertEqual({p['case_id'] for p in plan[:4]}, {f'C-128-{m}-AUTO' for m in ('LP12','LP24','HP12','BP12')})
        records=kit.windows(plan)
        self.assertEqual(len({r['window_id'] for r in records}),112)
        self.assertEqual(sum(r['warmup_seconds']+r['observation_seconds'] for r in records),7840)
        for start in range(0,112,4):
            self.assertEqual([r['role'] for r in records[start:start+4]], ['reference','candidate','candidate','reference'])
            self.assertEqual([r['order'] for r in records[start:start+4]], ['AB','AB','BA','BA'])
        for r in records:
            self.assertEqual(r['observation_status'],'not_measured')
            for key in ('fx_cpu_mean','fx_cpu_max','rt_cpu_peak','xrun_start','xrun_end','audible_issues','crash'):
                self.assertEqual(r[key],'')

    def test_smf_decodes_to_exact_notes_tempo_and_duration(self):
        # Independent SMF reader: verify event semantics, not a writer round-trip.
        for count in (1,8,16):
            data=kit.midi(count)
            self.assertEqual(data[:14],b'MThd'+struct.pack('>IHHH',6,0,1,480))
            self.assertEqual(data[14:18],b'MTrk')
            self.assertEqual(struct.unpack('>I',data[18:22])[0],len(data)-22)
            pos=22;tick=0;ons=[];offs=[];tempo=None;end=None
            def variable():
                nonlocal pos
                value=0
                for _ in range(4):
                    byte=data[pos];pos+=1;value=(value<<7)|(byte&127)
                    if byte<128:return value
                self.fail('Invalid MIDI variable length quantity')
            while pos<len(data):
                tick+=variable();status=data[pos];pos+=1
                if status==255:
                    kind=data[pos];pos+=1;size=variable();payload=data[pos:pos+size];pos+=size
                    if kind==81:tempo=int.from_bytes(payload,'big')
                    if kind==47:end=tick
                elif status in (0x90,0x80):
                    note,velocity=data[pos:pos+2];pos+=2
                    (ons if status==0x90 else offs).append((tick,note,velocity))
                else:self.fail(f'Unexpected status {status}')
            expected=[48,50,52,53,55,57,59,60,62,64,65,67,69,71,72,74][:count]
            self.assertEqual(tempo,500000)
            self.assertEqual(ons,[(0,n,100) for n in expected])
            self.assertEqual(offs,[(76800,n,0) for n in expected])
            self.assertEqual(end,78720)  # 82 seconds at the declared tempo.

    def test_both_platforms_prepare_hash_bound_unmeasured_sessions(self):
        for platform in ('windows-x64','macos'):
            out=self.root/platform
            result=kit.prepare(self.package('reference',platform),self.package('candidate',platform),out,'check-1')
            self.assertEqual(result['state'],'prepared_not_measured')
            self.assertFalse(result['native_host_acceptance'])
            self.assertFalse(result['production_promotion_allowed'])
            self.assertTrue(all(v is None for v in result['environment'].values()))
            with (out/'observations.csv').open(encoding='utf-8',newline='') as stream:
                self.assertEqual(len(list(csv.DictReader(stream))),112)
            for name,digest in result['generated_inputs_sha256'].items():
                self.assertEqual(hashlib.sha256((out/name).read_bytes()).hexdigest(),digest)
            with self.assertRaisesRegex(ValueError,'never overwritten'):
                kit.prepare(self.root/'absent',self.root/'absent',out,'check-1')

    def test_mixed_source_or_platform_rejected_without_output(self):
        ref=self.package('reference')
        for cand in (self.package('candidate',source='b'*40),self.package('candidate','macos')):
            out=self.root/'not-created'
            with self.assertRaisesRegex(ValueError,'Different package'):
                kit.prepare(ref,cand,out,'reject')
            self.assertFalse(out.exists())

    def test_corrupt_member_checksum_and_wrong_role_rejected(self):
        path=self.package('reference',damage=b'changed')
        with self.assertRaisesRegex(ValueError,'member hash'):kit.verify_package(path,'reference')
        path=self.package('reference')
        path.with_suffix('.zip.sha256').write_text('0'*64+' '+path.name)
        with self.assertRaisesRegex(ValueError,'checksum'):kit.verify_package(path,'reference')
        path=self.package('reference')
        with self.assertRaisesRegex(ValueError,'identity'):kit.verify_package(path,'candidate')

    def test_changed_policy_identity_acceptance_and_missing_executable_rejected(self):
        for key,value in [('latency_samples',0),('level_policy','new-gain'),('engine','Classic'),
                ('native_host_acceptance',True),('production_promotion_allowed',True),
                ('experimental',False),('source_commit','main'),('binary_sha256','f'*64),
                ('vst3_class_id','Classic')]:
            with self.subTest(key=key):
                path=self.package('reference',edits={key:value})
                with self.assertRaises(ValueError):kit.verify_package(path,'reference')

    def test_unlisted_duplicate_and_traversal_entries_rejected(self):
        import warnings
        for name in ('not-listed.txt','../escape','PACKAGE-MANIFEST.json'):
            path=self.package('reference')
            with warnings.catch_warnings():
                warnings.simplefilter('ignore',UserWarning)
                with zipfile.ZipFile(path,'a') as archive:archive.writestr(name,b'bad')
            self.checksum(path)
            with self.subTest(name=name),self.assertRaises(ValueError):kit.verify_package(path,'reference')
        self.assertFalse((self.root/'escape').exists())


if __name__=='__main__':unittest.main()
