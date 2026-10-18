# SPDX-License-Identifier: MIT
import contextlib
import csv
import importlib.util
import io
import itertools
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('slices',ROOT/'scripts/qualify-preview-slices.py')
slices=importlib.util.module_from_spec(spec);spec.loader.exec_module(slices)


class SliceQualification(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name)
        self.executable=self.root/'pair';self.executable.write_bytes(b'fixed binary')
        self.cache=self.root/'CMakeCache.txt';self.cache.write_bytes(b'fixed cache')
        self.output=self.root/'evidence'
        self.calls=[];self.counts={}

    def diagnostic(self,stream,changed=False,incomplete=False,within_pair=False):
        writer=csv.writer(stream);writer.writerow(slices.FIELDS)
        for index,(rate,mode,voice,fx) in enumerate(itertools.product(
                (48000,96000,176400,192000,384000),range(4),range(3),range(2))):
            if incomplete and index==119:continue
            value='0000000000000002' if changed and index==0 else '0000000000000001'
            writer.writerow([2,rate,mode,voice,fx,0,0,0,0,value,
                             '0000000000000003' if within_pair and index==0 else value,value,value])

    def invoke(self,kind=None):
        def run(command,stdout,stderr):
            arch,flag,wave=command[1],command[-2],command[-1]
            key=(arch,wave);cold=self.counts.get(key,0);self.counts[key]=cold+1
            self.calls.append((arch,flag,wave,cold))
            if wave=='2':
                self.assertEqual(flag,'--diagnostic-waveform')
                self.diagnostic(stdout,
                    changed=kind=='changed' and arch=='-x86_64' and cold==1,
                    incomplete=kind=='incomplete' and arch=='-arm64' and cold==0,
                    within_pair=kind=='within_pair' and arch=='-x86_64' and cold==0)
            else:self.assertEqual(flag,'--waveform');stdout.write('ordinary fixture log\n')
            if kind=='binary_changed' and len(self.calls)==14:self.executable.write_bytes(b'changed binary')
            return subprocess.CompletedProcess(command,1 if kind=='exit_failure' and arch=='-x86_64' and wave=='2' and cold==0 else 0)
        def git_head(command, **kwargs):
            # Reject accidental interception of platform.py's binary `file`
            # probe: only the declared text-mode Git query belongs to this mock.
            self.assertEqual(command,['git','rev-parse','HEAD'])
            self.assertEqual(kwargs,{'text':True})
            return 'a'*40
        with patch.object(slices.platform,'system',return_value='Darwin'),patch.object(slices.platform,'machine',return_value='arm64'),patch.object(slices.platform,'platform',return_value='synthetic-Darwin-arm64'),patch.object(slices.subprocess,'check_output',side_effect=git_head),patch.object(slices.subprocess,'run',side_effect=run),contextlib.redirect_stdout(io.StringIO()):
            return slices.qualify(self.executable,self.cache,self.output)

    def assert_evidence(self,result):
        self.assertEqual(result['runtime_platform'],'synthetic-Darwin-arm64')
        self.assertEqual(len(result['results']),14)
        self.assertEqual(len(self.calls),14)
        self.assertEqual(len(set(self.calls)),14)
        for arch in ('-arm64','-x86_64'):
            self.assertEqual(self.counts[(arch,'2')],4)
        self.assertEqual((self.output/'pair').read_bytes(),b'fixed binary')
        self.assertEqual(result['executable_sha256'],slices.sha(self.output/'pair'))
        self.assertEqual(result['cmake_cache_sha256'],slices.sha(self.output/'CMakeCache.txt'))
        self.assertFalse(result['native_host_acceptance']);self.assertFalse(result['production_promotion_allowed'])
        self.assertEqual(json.loads((self.output/'manifest.json').read_text())['state'],result['state'])
        for r in result['results']:
            self.assertEqual(r['log_sha256'],slices.sha(self.output/r['log_file']))

    def test_stable_signals_four_cold_processes_per_architecture(self):
        result=self.invoke();self.assert_evidence(result)
        self.assertEqual(result['state'],'complete')
        self.assertFalse(any(r.get('validation_error') for r in result['results']))

    def test_changed_signal_fails_even_when_every_process_exits_successfully(self):
        result=self.invoke('changed');self.assert_evidence(result)
        self.assertEqual(result['state'],'failed')
        self.assertTrue(all(r['returncode']==0 for r in result['results']))
        errors=[r for r in result['results'] if r.get('validation_error')]
        self.assertEqual(len(errors),1)
        self.assertEqual(errors[0]['cold_run'],1)
        self.assertIn('differs',errors[0]['validation_error'])

    def test_failed_first_process_is_not_replaced_by_later_success(self):
        result=self.invoke('exit_failure');self.assert_evidence(result)
        self.assertEqual(result['state'],'failed')
        errors=[r for r in result['results'] if r.get('validation_error')]
        self.assertEqual(len(errors),3)
        self.assertTrue(all('baseline' in r['validation_error'] for r in errors))

    def test_incomplete_grid_and_within_pair_mismatch_cannot_pass(self):
        for kind in ('incomplete','within_pair'):
            with self.subTest(kind=kind):
                if self.output.exists():
                    import shutil
                    shutil.rmtree(self.output)
                self.calls=[];self.counts={}
                result=self.invoke(kind);self.assert_evidence(result)
                self.assertEqual(result['state'],'failed')
                self.assertTrue(any(r.get('validation_error') for r in result['results']))

    def test_changed_executable_blocks_and_keeps_original_copy(self):
        result=self.invoke('binary_changed');self.assert_evidence(result)
        self.assertEqual(result['state'],'failed')
        self.assertIn('changed',result['error'])

    def test_existing_evidence_never_overwritten(self):
        self.output.mkdir();p=self.output/'keep';p.write_bytes(b'keep original')
        with self.assertRaisesRegex(ValueError,'never overwritten'):self.invoke()
        self.assertEqual(p.read_bytes(),b'keep original')
        self.assertEqual(self.calls,[])

    def test_non_native_runtime_rejected_before_execution(self):
        with patch.object(slices.platform,'system',return_value='Linux'):
            with self.assertRaisesRegex(ValueError,'Apple Silicon'):
                slices.qualify(self.executable,self.cache,self.output)
        self.assertFalse(self.output.exists())


if __name__=='__main__':unittest.main()
