#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Deterministic held-note MIDI inputs for native P0 CPU controls, not audio results."""
import struct
import argparse
import hashlib
import json
from pathlib import Path
import re

PPQ = 480
TEMPO_US = 500000
VELOCITY = 80
WARMUP_SECONDS = 10
OBSERVATION_SECONDS = 60


def vlq(value):
    if not isinstance(value, int) or not 0 <= value <= 0x0fffffff:
        raise ValueError('MIDI delta must fit a four-byte VLQ')
    result = [value & 0x7f]
    value >>= 7
    while value:
        result.insert(0, (value & 0x7f) | 0x80)
        value >>= 7
    return bytes(result)


def held_notes(voices):
    """Format 0, channel 1; note-off at 70s, without CC/program/plugin changes."""
    if type(voices) is not int or voices not in (1, 8, 16):
        raise ValueError('Supported MIDI input counts: 1, 8, 16')
    notes = list(range(48, 48 + voices))
    ticks_per_second = PPQ * 1000000 // TEMPO_US
    start = WARMUP_SECONDS * ticks_per_second
    end = (WARMUP_SECONDS + OBSERVATION_SECONDS) * ticks_per_second
    track = bytearray(b'\x00\xff\x51\x03' + TEMPO_US.to_bytes(3, 'big'))
    track.extend(b'\x00\xff\x58\x04\x04\x02\x18\x08')
    for note in notes:
        track.extend(bytes((0, 0x90, note, VELOCITY)))
    for delta, label in ((start, b'OBSERVE_START'), (end - start, b'OBSERVE_END')):
        track.extend(vlq(delta) + b'\xff\x06' + vlq(len(label)) + label)
    for note in notes:
        track.extend(bytes((0, 0x80, note, 0)))
    track.extend(b'\x00\xff\x2f\x00')
    return (b'MThd' + struct.pack('>IHHH', 6, 0, 1, PPQ) +
            b'MTrk' + struct.pack('>I', len(track)) + track)


def filename(voices):
    return f'midi/P0-held-{voices:02d}-notes.mid'


def specification(voices):
    return dict(format=0, ppq=PPQ, tempo_bpm=120, channel=1,
                note_numbers=list(range(48, 48 + voices)), velocity=VELOCITY,
                note_on_seconds=0, observation_start_seconds=WARMUP_SECONDS,
                note_off_seconds=WARMUP_SECONDS + OBSERVATION_SECONDS,
                expected_input_notes=voices, actual_synth_voices_verified=False,
                intended_cases='P0 C/V/G only; P1 musical input chosen separately')


def write_inputs(output, source_commit):
    if not re.fullmatch('[0-9a-f]{40}', source_commit):
        raise ValueError('Source must be a full lowercase commit SHA')
    output = Path(output)
    output.mkdir(parents=True, exist_ok=False)
    manifest = dict(schema=1, generator_source_commit=source_commit,
                    native_host_acceptance=False, status='not_run', files={})
    for voices in (1, 8, 16):
        name = filename(voices)
        data = held_notes(voices)
        path = output / name
        path.parent.mkdir(exist_ok=True)
        path.write_bytes(data)
        manifest['files'][name] = dict(sha256=hashlib.sha256(data).hexdigest(),
                                     specification=specification(voices))
    (output / 'MIDI-INPUTS.json').write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
    (output / 'README.txt').write_text(
        'P0 held-note technical MIDI inputs; NOT a plugin, preset, project or test result.\n'
        '120 BPM, channel 1, velocity 80; note numbers 48 upward.\n'
        'Import at project time zero. Keep 120 BPM, playrate 1, no item stretch or loop.\n'
        'Each window starts at 0s: note-on at 0s, observe 10–70s, note-off at 70s.\n'
        'Poly mode and a sustaining envelope required. Check actual active voices.\n'
        'Keep playback running for the full release/FX tail after 70s; do not auto-stop.\n'
        'Use identical input for Reference and Candidate; only one instrument online.\n'
        'Set safe, fixed headroom before measuring. No volume changes within A/B.\n'
        'These chromatic held notes are CPU controls for P0 C/V/G, not musical P1 input.\n'
        'No cutoff automation is encoded; save identical host automation separately.\n'
        'No sound is produced until a synth is loaded. MIDI note count is not voice acceptance.\n'
        'See docs/NATIVE_HOST_MIDI_INPUTS.md and docs/NATIVE_REAPER_CPU_ACCEPTANCE.md.\n',
        encoding='utf-8')
    return output


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--source', required=True, help='Generator checkout SHA, not a plugin source claim')
    args = parser.parse_args()
    try:
        print(write_inputs(args.output, args.source))
    except (ValueError, OSError) as error:
        parser.exit(1, str(error) + '\n')
