#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Bounded numerical reproducibility probe, never a CPU acceptance campaign."""
import argparse
import csv
import hashlib
import itertools
import json
from pathlib import Path
import platform
import re
import shutil
import subprocess

FIELDS = ('waveform','rate','filter_mode','voice_mode','fx','output_peak_error',
          'output_relative_rms','prefx_peak_error','prefx_relative_rms',
          'output_reference_digest','output_candidate_digest','prefx_reference_digest','prefx_candidate_digest')
COLD_RUNS = 4  # Declared before execution; never retry until a successful result.


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def fingerprints(path):
    with path.open(encoding='utf-8',newline='') as stream:
        reader=csv.DictReader(stream)
        if tuple(reader.fieldnames or ()) != FIELDS:
            raise ValueError('Diagnostic fingerprint header mismatch')
        rows=list(reader)
    if any(None in row or any(row.get(field) is None for field in FIELDS) for row in rows):
        raise ValueError('Malformed diagnostic fingerprint row')
    expected=set(itertools.product(('2',),('48000','96000','176400','192000','384000'),
                                  ('0','1','2','3'),('0','1','2'),('0','1')))
    keys=[tuple(row[field] for field in FIELDS[:5]) for row in rows]
    if len(rows)!=120 or set(keys)!=expected:
        raise ValueError('Incomplete/duplicate diagnostic fingerprint grid')
    table=[]
    for key,row in zip(keys,rows):
        if any(row[field] != '0' for field in FIELDS[5:9]):
            raise ValueError('Nonzero bit-identity error in diagnostic table')
        values=[row[field] for field in FIELDS[9:]]
        if any(not re.fullmatch('[0-9a-f]{16}',v or '') for v in values):
            raise ValueError('Invalid signal fingerprint')
        if values[0]!=values[1] or values[2]!=values[3]:
            raise ValueError('Reference/Candidate signal fingerprints differ')
        table.append(key+tuple(values))
    # This hashes an FNV fingerprint table, NOT raw audio bytes or CPU data.
    return hashlib.sha256(json.dumps(sorted(table),separators=(',',':')).encode()).hexdigest()


def qualify(executable, cache, output):
    executable,cache,output=Path(executable).resolve(),Path(cache).resolve(),Path(output).resolve()
    if platform.system()!='Darwin' or platform.machine()!='arm64':
        raise ValueError('Expected Apple Silicon for explicit arm64/Rosetta qualification')
    if output.exists():
        raise ValueError('Existing qualification evidence is never overwritten')
    output.mkdir(parents=True,exist_ok=False)
    shutil.copy2(executable,output/executable.name)
    shutil.copy2(cache,output/'CMakeCache.txt')
    manifest=dict(schema=2,state='incomplete',
        source_commit=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),
        executable_sha256=sha(executable),cmake_cache_sha256=sha(cache),
        runtime_platform=platform.platform(),cold_runs_per_architecture=COLD_RUNS,
        purpose='numerical reproducibility; not CPU measurement or native host acceptance',
        native_host_acceptance=False,production_promotion_allowed=False,results=[])
    baseline={}
    def save():
        (output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
    save()
    def run(wave,architecture,cold_run=0):
        diagnostic=wave==2
        suffix='' if cold_run==0 else '-cold-'+str(cold_run)
        log=output/(architecture+'-waveform-'+str(wave)+suffix+'.txt')
        command=['arch','-'+architecture,str(executable),
                 '--diagnostic-waveform' if diagnostic else '--waveform',str(wave)]
        with log.open('w',encoding='utf-8') as stream:
            result=subprocess.run(command,stdout=stream,stderr=subprocess.STDOUT)
        entry=dict(waveform=wave,architecture=architecture,translated=architecture=='x86_64',
                   cold_run=cold_run,returncode=result.returncode,log_sha256=sha(log),log_file=log.name)
        if result.returncode==0 and diagnostic:
            try:
                digest=fingerprints(log)
                entry['fingerprint_table_sha256']=digest
                if cold_run==0:
                    baseline[architecture]=digest
                elif architecture not in baseline:
                    raise ValueError('No successful declared cold-run-0 baseline')
                elif digest!=baseline[architecture]:
                    raise ValueError('Cold-process signal fingerprint differs from declared baseline')
            except (ValueError,TypeError,KeyError) as error:
                entry['validation_error']=str(error)
        manifest['results'].append(entry)
        save()
        print(architecture,'waveform',wave,'cold',cold_run,'exit',result.returncode,
              entry.get('validation_error',''),flush=True)
    try:
        for wave in range(4):
            for architecture in ('arm64','x86_64'):
                run(wave,architecture)
        # Three additional cold processes + the declared baseline = four per
        # architecture. Preserve every result; do not select a later good baseline.
        for cold_run in range(1,COLD_RUNS):
            for architecture in ('arm64','x86_64'):
                run(2,architecture,cold_run)
        failed=any(r['returncode'] or r.get('validation_error') for r in manifest['results'])
        if sha(executable)!=manifest['executable_sha256'] or sha(cache)!=manifest['cmake_cache_sha256']:
            manifest['error']='Executable or CMake cache changed during qualification'
            failed=True
        manifest['state']='failed' if failed else 'complete'
    except BaseException as error:
        manifest['state']='failed';manifest['error']=str(error)
        raise
    finally:
        save()
    return manifest


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable',required=True,type=Path)
    parser.add_argument('--cache',required=True,type=Path)
    parser.add_argument('--output',required=True,type=Path)
    args=parser.parse_args()
    result=qualify(args.executable,args.cache,args.output)
    if result['state']!='complete':
        parser.exit(1,'Numerical reproducibility failed; inspect preserved evidence\n')
