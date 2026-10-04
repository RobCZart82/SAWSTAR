# SPDX-License-Identifier: MIT
"""Validate research WAV output and a constant, rather than dynamic, RMS match."""
import array
import math
from pathlib import Path
import struct
import subprocess
import sys
import tempfile


def read(path):
    data = path.read_bytes()
    assert data[:4] == b'RIFF' and struct.unpack_from('<I', data, 4)[0] + 8 == len(data)
    assert data[8:12] == b'WAVE' and struct.unpack_from('<HHI', data, 20) == (3, 2, 48000)
    assert data[48:52] == b'data' and struct.unpack_from('<I', data, 52)[0] == len(data) - 56
    result = array.array('f')
    result.frombytes(data[56:])
    if sys.byteorder != 'little':
        result.byteswap()
    assert len(result) == 12 * 48000 * 2
    assert all(math.isfinite(x) and abs(x) < 1 for x in result)
    return result


def validate(prefix, mode='LP24'):
    a = read(Path(prefix + f'-A-legacy-{mode}.wav'))
    raw = read(Path(prefix + f'-B-candidate-{mode}.wav'))
    matched = read(Path(prefix + f'-B-RMS-matched-{mode}.wav'))
    start, end = 48000, 48000 * 23  # stereo 0.5..11.5 s
    energy_a = sum(float(x) * x for x in a[start:end])
    energy_raw = sum(float(x) * x for x in raw[start:end])
    energy_matched = sum(float(x) * x for x in matched[start:end])
    assert energy_a > 0 and energy_raw > 0
    assert abs(10 * math.log10(energy_matched / energy_a)) < 1e-5
    gain = math.sqrt(energy_a / energy_raw)
    assert all(abs(out - source * gain) < 1e-7 for source, out in zip(raw, matched))
    assert a[0] == raw[0] == matched[0] == 0


with tempfile.TemporaryDirectory() as folder:
    default = str(Path(folder) / 'default')
    subprocess.run([sys.argv[1], default], check=True)
    validate(default)
    for scene in ('lead', 'pluck', 'pad'):
        for resonance in (10, 50, 90):
            prefix = str(Path(folder) / f'{scene}-{resonance}')
            subprocess.run([sys.argv[1], prefix, str(resonance), scene], check=True)
            validate(prefix)
    reference = None
    for db in (0, 12, 20, 24):
        for mode in ('host', '4x'):
            prefix = str(Path(folder) / f'drive-{db}-{mode}')
            subprocess.run([sys.argv[1], prefix, '50', 'lead', str(db), mode], check=True)
            validate(prefix)
            data = Path(prefix + '-A-legacy-LP24.wav').read_bytes()
            if reference is None:
                reference = data
            assert data == reference  # Identical clean A source, gain and aligned delay.
    for mode in ('LP12', 'LP24', 'HP12', 'BP12'):
        prefix = str(Path(folder) / f'four-mode-{mode}')
        subprocess.run([sys.argv[1], prefix, '50', 'lead', '20', '4x', mode], check=True)
        validate(prefix, mode)
        if mode == 'LP24':
            for suffix in ('A-legacy', 'B-candidate', 'B-RMS-matched'):
                assert Path(prefix + f'-{suffix}-LP24.wav').read_bytes() == Path(
                    str(Path(folder) / 'drive-20-4x') + f'-{suffix}-LP24.wav').read_bytes()
    for index, arguments in enumerate((('nan',), ('inf',), ('-1',), ('101',),
                                      ('50oops',), ('50', 'unknown'),
                                      ('50', 'lead', 'nan'), ('50', 'lead', '-1'),
                                      ('50', 'lead', '25'), ('50', 'lead', '12', 'bad'),
                                      ('50', 'lead', '20', '4x', 'bad'))):
        prefix = str(Path(folder) / f'bad-{index}')
        result = subprocess.run([sys.argv[1], prefix, *arguments], capture_output=True)
        assert result.returncode != 0
        assert not list(Path(folder).glob(f'bad-{index}*'))
