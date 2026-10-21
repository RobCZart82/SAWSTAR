#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Verify macOS or Windows x64 Preview ZIPs and prepare unmeasured REAPER worksheets.

No extraction, installation, plugin execution or DAW modification is performed.
Checksums establish consistency, not authenticity or successful CI/host testing.
"""
import argparse
import csv
import hashlib
import io
import importlib.util
import json
from pathlib import Path, PurePosixPath
import plistlib
import re
import stat
import struct
import zipfile

IDENTITIES = {
    'reference': ('SAWSTAR Premium Preview', 'SAWSTARPremiumPreview', 'SwPP', 'SevenSaw'),
    'candidate': ('SAWSTAR Premium Candidate', 'SAWSTARPremiumCandidate', 'SwPC', 'SawDispatchSevenSaw'),
}
MODES = ('LP12', 'LP24', 'HP12', 'BP12')
MAX_MEMBER = 128 * 1024 * 1024
MAX_TOTAL = 512 * 1024 * 1024

_spec = importlib.util.spec_from_file_location('preview_package_identity',
    Path(__file__).with_name('package-premium-preview.py'))
_packager = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_packager)

_midi_spec = importlib.util.spec_from_file_location('native_host_midi',
    Path(__file__).with_name('native-host-midi.py'))
_midi = importlib.util.module_from_spec(_midi_spec)
_midi_spec.loader.exec_module(_midi)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError('Duplicate JSON key: ' + key)
        result[key] = value
    return result


def universal_architectures(data):
    """Check fat table bounds and each 64-bit Mach-O CPU header, not loadability."""
    formats = {b'\xca\xfe\xba\xbe': ('>', False), b'\xbe\xba\xfe\xca': ('<', False),
               b'\xca\xfe\xba\xbf': ('>', True), b'\xbf\xba\xfe\xca': ('<', True)}
    if len(data) < 8 or data[:4] not in formats:
        raise ValueError('Expected Universal Mach-O')
    endian, fat64 = formats[data[:4]]
    count = struct.unpack_from(endian + 'I', data, 4)[0]
    size = 32 if fat64 else 20
    table_end = 8 + count * size
    if count != 2 or table_end > len(data):
        raise ValueError('Expected exactly arm64 and x86_64 slices')
    cpus, spans = [], []
    for i in range(count):
        fields = struct.unpack_from(endian + ('IIQQII' if fat64 else 'IIIII'), data, 8 + i * size)
        cpu, subtype, offset, length, alignment = fields[:5]
        if (offset < table_end or length < 32 or offset + length > len(data) or
                alignment > 31 or offset % (1 << alignment)):
            raise ValueError('Invalid Mach-O slice bounds/alignment')
        if any(offset < end and start < offset + length for start, end in spans):
            raise ValueError('Overlapping Mach-O slices')
        spans.append((offset, offset + length))
        if data[offset:offset + 4] != b'\xcf\xfa\xed\xfe':
            raise ValueError('Expected little-endian 64-bit Mach-O slice')
        if struct.unpack_from('<II', data, offset + 4) != (cpu, subtype):
            raise ValueError('Mach-O slice CPU header disagrees with fat table')
        cpus.append(cpu)
    if sorted(cpus) != [0x01000007, 0x0100000c]:
        raise ValueError('Expected arm64 and x86_64 CPU headers')
    return ['arm64', 'x86_64']


def verify_package(path, role, expected_source, platform='macos'):
    if platform not in ('macos', 'windows-x64'):
        raise ValueError('Unsupported package platform')
    path = Path(path)
    raw = path.read_bytes()
    checksum = Path(str(path) + '.sha256').read_text(encoding='utf-8').strip()
    if checksum != sha(raw) + '  ' + path.name:
        raise ValueError('ZIP checksum/filename mismatch: ' + path.name)
    with zipfile.ZipFile(io.BytesIO(raw)) as archive:
        entries = archive.infolist()
        names, folded, total = set(), set(), 0
        for item in entries:
            name = item.filename
            parts = PurePosixPath(name).parts
            mode = item.external_attr >> 16
            if (not parts or name.startswith('/') or '\\' in name or ':' in name or
                    any(p in ('', '.', '..') for p in name.split('/')) or
                    name != PurePosixPath(name).as_posix() or item.is_dir() or
                    stat.S_IFMT(mode) not in (0, stat.S_IFREG) or item.flag_bits & 1):
                raise ValueError('Unsafe/non-regular ZIP member: ' + name)
            if name in names or name.casefold() in folded:
                raise ValueError('Duplicate/case-colliding ZIP member: ' + name)
            names.add(name)
            folded.add(name.casefold())
            total += item.file_size
            if item.file_size > MAX_MEMBER or total > MAX_TOTAL or len(names) > 10000:
                raise ValueError('Package exceeds inspection size limits')
        if 'PACKAGE-MANIFEST.json' not in names:
            raise ValueError('Missing package manifest')
        manifest_data = archive.read('PACKAGE-MANIFEST.json')
        manifest = json.loads(manifest_data, object_pairs_hook=unique_object)
        if not isinstance(manifest, dict):
            raise ValueError('Package manifest must be a JSON object')
        product, stem, signature, oscillator = IDENTITIES[role]
        identity = dict(product=product, pair_role=role, experimental=True,
                        source_commit=expected_source, platform=platform,
                        bundle_identifier='io.github.robczart82.' + stem.lower() if platform == 'macos' else None,
                        plugin_unique_id=signature, oscillator=oscillator,
                        engine='RateScaledLookupEnginePremiumFilter', latency_samples=32,
                        level_policy='wet-output-3db-factory-trim-v1',
                        native_host_acceptance=False, production_promotion_allowed=False)
        for key, value in identity.items():
            if manifest.get(key) != value or type(manifest.get(key)) is not type(value):
                raise ValueError('Package identity/source/policy mismatch: ' + key)
        hashes = manifest.get('files_sha256')
        if not isinstance(hashes, dict) or set(hashes) != names - {'PACKAGE-MANIFEST.json'}:
            raise ValueError('Manifest file inventory mismatch')
        for name, digest in hashes.items():
            if not isinstance(digest, str) or not re.fullmatch('[0-9a-f]{64}', digest) or sha(archive.read(name)) != digest:
                raise ValueError('Member hash mismatch: ' + name)
        bundles = {name.split('/')[0] for name in names if '.vst3' in name.split('/')[0].lower()}
        if bundles != {stem + '.vst3'}:
            raise ValueError('Unexpected plugin bundle inventory')
        prefix = stem + '.vst3/Contents/'
        if platform == 'macos':
            info = plistlib.loads(archive.read(prefix + 'Info.plist'))
            for key, value in dict(CFBundleIdentifier=identity['bundle_identifier'],
                                   CFBundleSignature=signature, CFBundleExecutable=stem, CFBundleName=stem).items():
                if info.get(key) != value:
                    raise ValueError('Bundle plist identity mismatch: ' + key)
        binary_path = prefix + ('MacOS/' + stem if platform == 'macos' else 'x86_64-win/' + stem + '.vst3')
        binary = archive.read(binary_path)
        if sha(binary) != manifest.get('binary_sha256'):
            raise ValueError('Executable hash mismatch')
        if platform == 'macos':
            if not archive.getinfo(binary_path).external_attr >> 16 & 0o111:
                raise ValueError('Executable permissions missing')
            architectures = universal_architectures(binary)
        else:
            proof = json.loads(archive.read('evidence/moduleinfo.provenance.json'), object_pairs_hook=unique_object)
            cid = _packager.windows_identity_bytes(binary, binary_path[len(stem + '.vst3/'):],
                stem, product, signature, archive.read('evidence/moduleinfo.json'), proof, expected_source)
            if manifest.get('vst3_class_id') != cid:
                raise ValueError('Manifest VST3 class ID mismatch')
            architectures = ['x86_64']
    return dict(package_filename=path.name, package_sha256=sha(raw),
                package_manifest_sha256=sha(manifest_data),
                manifest=manifest, binary_path=binary_path, architectures=architectures)


def cases(fx_mode):
    if fx_mode not in MODES:
        raise ValueError('Select P1 filter mode before preparing the test')
    result = []

    def add(identifier, buffer, voices, mode, cutoff, gui, preset):
        result.append(dict(case_id=identifier, sample_rate=48000, buffer_samples=buffer,
                           voices=voices, filter_mode=mode, cutoff=cutoff, gui=gui, preset=preset))

    # Initial four-case prescreen is a subset of C, not additional acceptance rows.
    for buffer in (128, 64):
        for cutoff in ('AUTO', 'STATIC'):
            for mode in MODES:
                add(f'C-{buffer}-{mode}-{cutoff}', buffer, 16, mode, cutoff, 'closed', 'P0')
    for voices in (1, 8):
        for mode in MODES:
            add(f'V-128-{voices}-{mode}', 128, voices, mode, 'AUTO', 'closed', 'P0')
    for buffer in (128, 64):
        add(f'G-{buffer}-LP24-OPEN', buffer, 16, 'LP24', 'AUTO', 'open', 'P0')
    for buffer in (128, 64):
        add(f'F-{buffer}-P1', buffer, 16, fx_mode, 'AUTO', 'open', 'P1')
    # Keep /128 before /64 across families; retain the four AUTO prescreen cases first.
    return sorted(result, key=lambda row: row['buffer_samples'] != 128)


def measurement_rows(fx_mode):
    rows = []
    for case in cases(fx_mode):
        for index, (order, role) in enumerate((('AB', 'reference'), ('AB', 'candidate'),
                                                ('BA', 'candidate'), ('BA', 'reference')), 1):
            row = dict(case, window_id=case['case_id'] + '-' + str(index), order=order, role=role,
                       warmup_seconds=10, observation_seconds=60, status='not_run')
            row['midi_fixture'] = _midi.filename(case['voices']) if case['preset'] == 'P0' else ''
            for key in ('started_at', 'ended_at', 'cpu_meter', 'cpu_method', 'cpu_average',
                        'cpu_maximum', 'cpu_observed_min', 'cpu_observed_max', 'rt_meter',
                        'rt_observation', 'xrun_start', 'xrun_end', 'audible_issue', 'clipping',
                        'peak', 'rms', 'crash_state_gui_issue', 'evidence', 'notes'):
                row[key] = ''
            rows.append(row)
    return rows


def prepare(reference, candidate, output, expected_source, fx_mode, platform='macos'):
    if not re.fullmatch('[0-9a-f]{40}', expected_source):
        raise ValueError('Expected source must be a full lowercase commit SHA')
    rows = measurement_rows(fx_mode)
    packages = {role: verify_package(path, role, expected_source, platform)
                for role, path in [('reference', reference), ('candidate', candidate)]}
    output = Path(output)
    # mkdir is exclusive: existing evidence is never overwritten, even an empty directory.
    output.mkdir(parents=True, exist_ok=False)
    inputs = {}
    for voices in (1, 8, 16):
        relative_path = _midi.filename(voices)
        data = _midi.held_notes(voices)
        path = output / relative_path
        path.parent.mkdir(exist_ok=True)
        path.write_bytes(data)
        inputs[relative_path] = dict(sha256=sha(data), specification=_midi.specification(voices))
    provenance = dict(schema=1, source_commit=expected_source, platform=platform,
                      packages=packages, fx_filter_mode=fx_mode, cases=28, windows=112,
                      generated_inputs=inputs,
                      native_host_acceptance=False, production_promotion_allowed=False,
                      status='not_run', ci_success_verified=False)
    (output / 'provenance.json').write_text(json.dumps(provenance, indent=2) + '\n', encoding='utf-8')
    with (output / 'measurements.csv').open('w', newline='', encoding='utf-8') as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    environment = dict.fromkeys(('date', 'machine_cpu_ram_os', 'power_background_load',
        'reaper_version', 'actual_reaper_process_architecture', 'audio_device_driver',
        'actual_rate_buffers', 'anticipative_fx_multiprocessing_monitoring',
        'other_tracks_fx', 'P0_preset_parameters_path_sha256', 'P1_preset_parameters_path_sha256',
        'project_rpp_path_sha256', 'midi_path_sha256', 'cutoff_automation_path_sha256',
        'fixed_headroom', 'cpu_meter_method', 'rt_meter_semantics_and_predeclared_target',
        'xrun_counter_availability', 'workflow_run_urls_and_verified_success',
        'installed_reference_binary_sha256', 'installed_candidate_binary_sha256',
        'installation_rescan_smoke_evidence', 'manual_acceptance_robert_date_scope_reason'), '')
    (output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n', encoding='utf-8')
    (output / 'README.md').write_text(
        '# Native REAPER measurement kit — NOT RUN\n\n'
        'ZIP consistency and platform identity checked; no plugin was installed or executed.\n'
        'Checksums are not authenticity, CI success, validator or host acceptance.\n'
        'Verify both exact-source workflow jobs succeeded and record their URLs.\n'
        'After installation compare actual executable hashes with provenance.json.\n'
        'macOS: native arm64 REAPER on Apple Silicon, native x86_64 on Intel.\n'
        'Windows x64: native x64 REAPER; no Windows ARM64 qualification.\n'
        'Record actual process architecture; Universal headers do not prove execution.\n\n'
        'Complete environment.json before playback. Save identical P0/P1 presets, MIDI,\n'
        'cutoff automation and headroom for both roles. P0 has chorus/delay/reverb off;\n'
        'P1 is a preset chosen before the run with fixed FX. Check actual 1/8/16 voices.\n'
        'P0 MIDI files in midi/: 120 BPM, channel 1, velocity 80, held notes 48 upward.\n'
        'Import at project time zero and keep project tempo 120 BPM with no item stretch/loop.\n'
        'Start each window at zero: note-on at 0s, observe 10–70s, note-off at 70s.\n'
        'Use Poly mode and a sustaining P0 envelope; MIDI note count is not proof of active voices.\n'
        'P1 musical MIDI/automation must still be chosen and saved before measurement.\n'
        'Let playback continue after note-off for the full release/FX tail; do not loop.\n'
        'Exactly one instrument online; the other offline, not merely muted.\n'
        'Run the first four C-128 AUTO cases as prescreen, then the remaining rows.\n'
        'Each case: Reference, Candidate, Candidate, Reference; 10s warmup + 60s each.\n'
        '28 cases / 112 windows / 130m40s, excluding setup/tails.\n'
        'Record every result; repeats need separate IDs and a reason. Preserve failures.\n'
        'CPU average/max only if the meter supplies them; otherwise observed range.\n'
        'No p99 from visual readings. RT metrics are separate from FX CPU.\n'
        'Unavailable xrun counters are N/A, never zero. Keep audio/clipping/crash evidence.\n'
        'Keep the accepted 0.44 dB RMS difference; no new gain/limiter to hide issues.\n'
        'Tail/idle, stress, state/GUI, high rates and 32 samples are separate tests.\n'
        'Robert must explicitly accept the measured build/machine/scope; no automatic PASS.\n'
        'See docs/NEXT_RELEASE_DEVELOPMENT_PLAN.md sections 4–7 and\n'
        'docs/NATIVE_REAPER_CPU_ACCEPTANCE.md. Final production RC needs separate validation.\n',
        encoding='utf-8')
    return output


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference', type=Path, required=True, help='Inner package ZIP, with adjacent .zip.sha256')
    parser.add_argument('--candidate', type=Path, required=True)
    parser.add_argument('--platform', choices=('macos', 'windows-x64'), default='macos')
    parser.add_argument('--expected-source', required=True, help='Exact successful workflow checkout SHA')
    parser.add_argument('--fx-mode', choices=MODES, required=True, help='Preselected P1 filter mode')
    parser.add_argument('--output', type=Path, required=True, help='New directory; existing paths refused')
    args = parser.parse_args()
    try:
        print(prepare(args.reference, args.candidate, args.output, args.expected_source, args.fx_mode, args.platform))
    except (ValueError, OSError, KeyError, zipfile.BadZipFile, plistlib.InvalidFileException) as error:
        parser.exit(1, 'Preflight failed: ' + str(error) + '\n')
