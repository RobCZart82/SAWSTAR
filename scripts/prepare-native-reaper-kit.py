#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Verify a matching test-package pair and prepare empty native REAPER records.

Read-only input ZIPs; no plugin loading, installation, CPU measurement or acceptance.
"""
import argparse
import csv
import hashlib
import io
import json
from pathlib import Path, PurePosixPath
import re
import stat
import struct
import zipfile

IDENTITIES = {
    'reference': ('SAWSTAR Premium Preview', 'SAWSTARPremiumPreview', 'SwPP', 'SevenSaw'),
    'candidate': ('SAWSTAR Premium Candidate', 'SAWSTARPremiumCandidate', 'SwPC', 'SawDispatchSevenSaw'),
}
MODES = ('LP12', 'LP24', 'HP12', 'BP12')
NOTES = (48, 50, 52, 53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74)
POLICY = 'wet-output-3db-factory-trim-v1'


def require(condition, message):
    if not condition:
        raise ValueError(message)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def verify_package(path, role):
    path = Path(path)
    require(path.stat().st_size <= 64 * 1024 * 1024, 'Test ZIP exceeds 64 MiB limit')
    data = path.read_bytes()
    checksum = path.with_suffix(path.suffix + '.sha256').read_text(encoding='utf-8').split()
    require(checksum == [sha(data), path.name], 'ZIP checksum/name mismatch: ' + path.name)
    product, stem, unique_id, oscillator = IDENTITIES[role]
    with zipfile.ZipFile(io.BytesIO(data)) as archive:
        entries = archive.infolist()
        names = [entry.filename for entry in entries]
        require(len(set(names)) == len(names), 'Duplicate ZIP member')
        require(sum(entry.file_size for entry in entries) <= 128 * 1024 * 1024,
                'Test package exceeds 128 MiB uncompressed limit')
        for entry in entries:
            name = entry.filename
            parts = PurePosixPath(name).parts
            require(parts and not name.startswith('/') and '\\' not in name and
                    ':' not in name and '..' not in parts and not entry.is_dir() and
                    not stat.S_ISLNK(entry.external_attr >> 16), 'Unsafe ZIP member: ' + name)
        manifest = json.loads(archive.read('PACKAGE-MANIFEST.json'))
        require(isinstance(manifest, dict), 'Invalid package manifest object')
        hashes = manifest.get('files_sha256', {})
        require(isinstance(hashes, dict) and set(names) == set(hashes) | {'PACKAGE-MANIFEST.json'},
                'Incomplete or unlisted package files')
        for name, digest in hashes.items():
            require(sha(archive.read(name)) == digest, 'Package member hash mismatch: ' + name)
        require(manifest.get('pair_role') == role and manifest.get('product') == product and
                manifest.get('plugin_unique_id') == unique_id and manifest.get('oscillator') == oscillator,
                'Wrong Reference/Candidate identity')
        require(manifest.get('experimental') is True and
                manifest.get('native_host_acceptance') is False and
                manifest.get('production_promotion_allowed') is False,
                'Package does not retain experimental/unaccepted state')
        require(manifest.get('engine') == 'RateScaledLookupEnginePremiumFilter' and
                manifest.get('latency_samples') == 32 and manifest.get('level_policy') == POLICY,
                'Different filter/latency/level policy')
        require(re.fullmatch(r'[0-9a-f]{40}', manifest.get('source_commit', '')),
                'Missing exact source SHA')
        platform = manifest.get('platform')
        require(platform in ('macos', 'windows-x64'), 'Unsupported test package platform')
        binary = stem + '.vst3/Contents/' + (
            'MacOS/' + stem if platform == 'macos' else 'x86_64-win/' + stem + '.vst3')
        require(binary in hashes and hashes[binary] == manifest.get('binary_sha256'),
                'Missing or mismatched plugin executable')
        require(all(not name.split('/')[0].endswith('.vst3') or
                    name.startswith(stem + '.vst3/') for name in names), 'Mixed plugin bundles')
        if platform == 'macos':
            require(manifest.get('bundle_identifier') == 'io.github.robczart82.' + stem.lower(),
                    'Wrong macOS bundle identity')
        else:
            require(manifest.get('vst3_class_id') ==
                    'F2AEE70D00DE4F4E52433832' + unique_id.encode('ascii').hex().upper(),
                    'Wrong Windows VST3 class identity')
    return dict(role=role, platform=platform, source_commit=manifest['source_commit'],
                product=product, zip_name=path.name, zip_sha256=sha(data),
                binary_relative_path=binary, binary_sha256=manifest['binary_sha256'])


def cases():
    result = []
    def add(family, buffer, voices, mode, cutoff, gui, preset, early=False):
        key = (f'{family}-{buffer}-{mode}-{cutoff}' if family == 'C' else
               f'V-128-{voices}-{mode}' if family == 'V' else
               f'G-{buffer}-LP24-OPEN' if family == 'G' else f'F-{buffer}-P1')
        result.append(dict(case_id=key, family=family, rate_hz=48000, buffer_samples=buffer,
                           held_notes=voices, filter_mode=mode, cutoff=cutoff, gui=gui,
                           preset=preset, early_screen=early))
    for buffer in (128, 64):
        # Early screen first; the /128 block finishes before /64 starts.
        for cutoff in ('AUTO', 'STATIC'):
            for mode in MODES:
                add('C', buffer, 16, mode, cutoff, 'closed', 'P0', buffer == 128 and cutoff == 'AUTO')
        if buffer == 128:
            for voices in (1, 8):
                for mode in MODES:
                    add('V', buffer, voices, mode, 'AUTO', 'closed', 'P0')
        add('G', buffer, 16, 'LP24', 'AUTO', 'open', 'P0')
        add('F', buffer, 16, 'P1_PREDECLARE', 'AUTO', 'open', 'P1')
    return result


def windows(plan):
    result = []
    for case in plan:
        for order, roles in (('AB', ('reference', 'candidate')), ('BA', ('candidate', 'reference'))):
            for slot, role in enumerate(roles, 1):
                row = dict(case, window_id=f"{case['case_id']}-{order}-{slot}", order=order,
                           slot=slot, role=role, warmup_seconds=10, observation_seconds=60,
                           observation_status='not_measured')
                for key in ('started_at', 'finished_at', 'actual_voices_verified', 'meter_name',
                            'measurement_method', 'fx_cpu_mean', 'fx_cpu_max', 'fx_cpu_observed_min',
                            'fx_cpu_observed_max', 'rt_cpu_peak', 'xrun_availability', 'xrun_start',
                            'xrun_end', 'audible_issues', 'clipping', 'crash', 'state_gui_issues',
                            'evidence_paths', 'notes'):
                    row[key] = ''
                result.append(row)
    return result


def vlq(value):
    encoded = [value & 127]
    value >>= 7
    while value:
        encoded.insert(0, (value & 127) | 128)
        value >>= 7
    return bytes(encoded)


def midi(voices):
    # SMF0, 480 PPQ, 120 BPM: held notes 0..80 s, end marker at 82 s.
    notes = NOTES[:voices]
    events = bytearray(b'\0\xff\x51\x03\x07\xa1\x20\0\xff\x58\x04\x04\x02\x18\x08')
    for note in notes:
        events.extend(bytes((0, 0x90, note, 100)))
    for index, note in enumerate(notes):
        events.extend(vlq(76800 if index == 0 else 0) + bytes((0x80, note, 0)))
    events.extend(vlq(1920) + b'\xff\x2f\0')
    return b'MThd' + struct.pack('>IHHH', 6, 0, 1, 480) + b'MTrk' + struct.pack('>I', len(events)) + events


def write_csv(path, rows):
    with path.open('w', encoding='utf-8', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def prepare(reference, candidate, output, session_id):
    output = Path(output)
    require(re.fullmatch(r'[A-Za-z0-9_-]{1,64}', session_id), 'Use a unique plain session ID')
    require(not output.exists(), 'Existing session directory is never overwritten')
    builds = [verify_package(reference, 'reference'), verify_package(candidate, 'candidate')]
    require(builds[0]['source_commit'] == builds[1]['source_commit'], 'Different package source commits')
    require(builds[0]['platform'] == builds[1]['platform'], 'Different package platforms')
    plan = cases()
    records = windows(plan)
    output.mkdir(parents=True, exist_ok=False)
    write_csv(output / 'case-plan.csv', plan)
    write_csv(output / 'observations.csv', records)
    for count in (1, 8, 16):
        (output / f'held-{count}-notes.mid').write_bytes(midi(count))
    curve = [dict(time_seconds=0, cutoff_normalized=0.2), dict(time_seconds=10, cutoff_normalized=0.2)]
    curve += [dict(time_seconds=t, cutoff_normalized=0.8 if t % 10 else 0.2) for t in range(15, 71, 5)]
    write_csv(output / 'automation-plan.csv', curve)
    (output / 'READ-ME.txt').write_text(
        'PREPARED, NOT MEASURED. No plugin was installed or loaded.\n'
        'Use a new project at 120 BPM; import the matching 1/8/16-note MIDI at time zero.\n'
        'Sustained Poly sound, ARP off; verify actual sounding voices after the 10 s warmup.\n'
        'The notes hold for 80 s; observe only 10..70 s. MIDI ends at 82 s, not a tail guarantee.\n'
        'P0: two Saw banks, filter wet 100%, no chorus/delay/reverb; save full settings.\n'
        'P1: predeclare a sustained Poly FX sound and filter mode. Save identical settings for both roles.\n'
        'Set and save headroom before testing; do not adjust level during observations.\n'
        'automation-plan.csv is a normalized cutoff plan, NOT an imported REAPER envelope.\n'
        'Create one linear envelope from those points, save it in RPP and clone it exactly to the other role.\n'
        'STATIC cases use constant normalized cutoff 0.5; do not leave AUTO enabled.\n'
        'One instrument online at a time; the other OFFLINE, not just muted.\n'
        'AB = Reference then Candidate; BA = Candidate then Reference. Never erase a bad window.\n'
        'Fill session.json environment/preset/project fields before measurements.\n'
        'Blank/N/A xrun and RT data are unknown, never zero or automatic acceptance.\n'
        'Stress, full tail/idle, GUI/state, quality and final RC gates remain separate.\n'
        'See docs/NATIVE_REAPER_CPU_ACCEPTANCE.md and NEXT_RELEASE_DEVELOPMENT_PLAN.md in the repository.\n',
        encoding='utf-8')
    session = dict(schema=1, session_id=session_id, state='prepared_not_measured', builds=builds,
                   case_count=len(plan), observation_count=len(records), net_seconds=7840,
                   native_host_acceptance=False, production_promotion_allowed=False,
                   environment={key: None for key in ('date', 'machine_cpu_ram_os', 'power_profile_ac',
                       'background_load', 'reaper_version_arch', 'audio_device_driver', 'anticipative_fx',
                       'multiprocessing', 'live_monitoring', 'rt_meter_definition', 'reserve_criterion')},
                   inputs={key: None for key in ('P0_preset_path_sha256', 'P1_preset_path_sha256',
                       'P1_filter_mode', 'headroom_settings', 'project_path_sha256', 'envelope_path_sha256')},
                   generated_inputs_sha256={p.name: sha(p.read_bytes()) for p in sorted(output.iterdir())
                       if p.name != 'observations.csv'})
    (output / 'session.json').write_text(json.dumps(session, indent=2) + '\n', encoding='utf-8')
    return session


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference', required=True, type=Path)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--session-id', required=True)
    args = parser.parse_args()
    try:
        result = prepare(args.reference, args.candidate, args.output, args.session_id)
    except (OSError, ValueError, KeyError, zipfile.BadZipFile) as error:
        parser.exit(1, str(error) + '\n')
    print(f"Prepared {result['case_count']} cases / {result['observation_count']} empty windows; NOT MEASURED")
