# SPDX-License-Identifier: MIT
import contextlib
import csv
import hashlib
import importlib.util
import io
import json
import statistics
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('report', Path(__file__).resolve().parents[1] / 'scripts/report-oscillator-cache.py')
reporter = importlib.util.module_from_spec(spec)
spec.loader.exec_module(reporter)

class ReportTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.variant = "held-saw-tuning-v2"
        self.campaign = "short-v1"
        self.comparison = "candidate"
        self.rows = []
        for path in ('reference', 'study'):
            for rate in (48000, 96000, 192000):
                for wave in range(4):
                    for moving in range(2):
                        for pair in range(4):
                            self.rows.append(dict(path=path, rate=rate, waveform=wave, modulated=moving,
                                pair=pair, order='study-first' if pair % 2 else 'reference-first',
                                oscillators=32, frames=8192,
                                seconds=((2, 8, 30, 100) if path == 'study' else (1, 2, 10, 20))[pair], energy=10,
                                variant='held-saw-tuning-v2'))
    def tearDown(self):
        self.temp.cleanup()
    def run_report(self, source_sha='0' * 40):
        with (self.root / 'oscillators.csv').open('w', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=self.rows[0].keys())
            writer.writeheader(); writer.writerows(self.rows)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            result = reporter.report(self.root, source_sha, True, self.variant, self.campaign, self.comparison)
        self.output = output.getvalue()
        return result
    def test_pairs_and_provenance(self):
        result = self.run_report()
        self.assertEqual(len(result['summary']), 24)
        self.assertEqual({row['median_paired_seconds_ratio'] for row in result['summary']}, {3.5})
        self.assertEqual(result['summary'][0]['paired_ratios'], [2, 4, 3, 5])
        self.assertFalse(result['metadata']['native_host_acceptance'])
        self.assertFalse(result['metadata']['production_activation'])
        self.assertEqual(result['metadata']['ratio_direction'], 'study/reference')
        self.assertEqual(result['metadata']['candidate_variant'], 'held-saw-tuning-v2')
        self.assertEqual(len(result['metadata']['raw_sha256']), 64)
        self.assertEqual(len(result['metadata']['source_file_sha256']), 9)
        self.assertEqual(json.loads((self.root / 'report.json').read_text()), result)
        line = next(line for line in self.output.splitlines() if line.startswith('OSCILLATOR_CI_REPORT='))
        self.assertEqual(json.loads(line.split('=', 1)[1]), result)
    def test_missing_and_duplicate_pairs(self):
        removed = self.rows.pop()
        with self.assertRaisesRegex(ValueError, 'Incomplete'): self.run_report()
        self.rows.append(removed); self.rows.append(self.rows[0])
        with self.assertRaisesRegex(ValueError, 'duplicate'): self.run_report()
    def test_bad_signal_order_and_duration(self):
        original = self.rows[0].copy()
        for key, value, message in [('seconds', 0, 'measurement'), ('seconds', 'nan', 'measurement'),
                                    ('energy', 'inf', 'measurement'), ('order', 'study-first', 'order'),
                                    ('frames', 1, 'frame count'), ('oscillators', 1, 'bank'),
                                    ('variant', 'all-waveforms-v1', 'variant')]:
            with self.subTest(key=key, value=value):
                self.rows[0] = dict(original, **{key: value})
                with self.assertRaisesRegex(ValueError, message): self.run_report()
        self.rows[0] = original
        self.rows[-1]['energy'] = 11
        with self.assertRaisesRegex(ValueError, 'energy differs'): self.run_report()
    def test_shared_variant_is_distinct(self):
        self.variant = 'shared-frequency-v1'
        with self.assertRaisesRegex(ValueError, 'variant'): self.run_report()
        for row in self.rows:
            row['variant'] = self.variant
        result = self.run_report()
        self.assertEqual(result['metadata']['candidate_variant'], self.variant)
        sources = result['metadata']['source_file_sha256']
        self.assertIn('experiments/oscillator/SharedFrequencySevenSaw.cpp', sources)
        self.assertNotIn('experiments/oscillator/CachedSevenSaw.cpp', sources)
        self.assertFalse(result['qualification']['production_promotion_allowed'])
        self.variant = 'held-saw-tuning-v2'
        with self.assertRaisesRegex(ValueError, 'variant'): self.run_report()
        self.variant = 'unknown'
        with self.assertRaisesRegex(ValueError, 'variant'): self.run_report()

    def test_requires_exact_source(self):
        with self.assertRaisesRegex(ValueError, 'source SHA'): self.run_report('main')

    def extended_rows(self):
        self.variant = 'shared-frequency-v1'
        self.campaign = 'extended-v1'
        original = self.rows[:]
        self.rows = []
        for extra in (0, 4):
            for row in original:
                self.rows.append(dict(row, pair=row['pair'] + extra, variant=self.variant,
                                      frames=131072, campaign=self.campaign,
                                      comparison=self.comparison, warmup_frames=row['rate'] // 4))

    def test_extended_pairs_and_ranges(self):
        self.extended_rows()
        result = self.run_report()
        self.assertEqual(result['schema_version'], 3)
        self.assertEqual(result['metadata']['frames_per_path_and_pair'], 131072)
        self.assertEqual(result['metadata']['pairs_per_cell'], 8)
        self.assertEqual(result['metadata']['timed_study_type'], 'SharedFrequencySevenSaw')
        row = result['summary'][0]
        self.assertEqual(row['paired_ratios'], [2, 4, 3, 5] * 2)
        self.assertEqual(row['min_paired_seconds_ratio'], 2)
        self.assertEqual(row['max_paired_seconds_ratio'], 5)
        self.assertEqual(row['paired_observations'], 8)
        self.rows.pop()
        with self.assertRaisesRegex(ValueError, 'Incomplete'): self.run_report()

    def test_extended_cannot_masquerade_as_short_or_other_comparison(self):
        self.extended_rows()
        self.campaign = 'short-v1'
        with self.assertRaisesRegex(ValueError, 'columns'): self.run_report()
        self.campaign = 'extended-v1'
        original = self.rows[0].copy()
        for key, value in [('campaign', 'short-v1'), ('comparison', 'reference-repeat'),
                           ('warmup_frames', 0), ('frames', 8192)]:
            self.rows[0] = dict(original, **{key: value})
            with self.subTest(key=key), self.assertRaises(ValueError): self.run_report()
        self.rows[0] = original
        self.variant = 'held-saw-tuning-v2'
        with self.assertRaisesRegex(ValueError, 'Incompatible'): self.run_report()

    def test_reference_repeat_is_a_control_and_never_promotes(self):
        self.comparison = 'reference-repeat'
        with self.assertRaisesRegex(ValueError, 'Incompatible'): self.run_report()
        self.extended_rows()
        for row in self.rows:
            row['seconds'] = 1
        result = self.run_report()
        self.assertEqual(result['metadata']['comparison'], 'reference-repeat')
        self.assertEqual(result['metadata']['timed_study_type'], 'SevenSaw')
        self.assertFalse(result['metadata']['noise_correction_applied'])
        self.assertFalse(result['metadata']['background_load_controlled'])
        self.assertFalse(result['qualification']['production_promotion_allowed'])
        self.assertEqual(result['qualification']['cpu_acceptance'], 'not-established')
        self.assertEqual({x['median_paired_seconds_ratio'] for x in result['summary']}, {1})
        self.comparison = 'candidate'
        with self.assertRaisesRegex(ValueError, 'comparison'): self.run_report()

    def test_archived_platform_bytes_and_pairs(self):
        archive = Path(__file__).resolve().parents[1] / 'experiments/oscillator/measurements/2026-10-08-shared-frequency-ci'
        manifest = json.loads((archive / 'provenance.json').read_text())
        for platform, provenance in manifest['platforms'].items():
            folder = archive / platform
            for name, digest in provenance['files_sha256'].items():
                self.assertEqual(hashlib.sha256((folder / name).read_bytes()).hexdigest(), digest)
            result = json.loads((folder / 'report.json').read_bytes())
            self.assertEqual(result['metadata']['source_sha'], manifest['source_sha'])
            self.assertEqual(result['metadata']['raw_sha256'], provenance['files_sha256']['oscillators.csv'])
            with (folder / 'oscillators.csv').open(newline='') as stream:
                rows = list(csv.DictReader(stream))
            samples = {(r['path'], int(r['rate']), int(r['waveform']), int(r['modulated']), int(r['pair'])):
                       (float(r['seconds']), float(r['energy'])) for r in rows}
            self.assertEqual(len(rows), 192)
            self.assertEqual(len(samples), 192)
            for row in result['summary']:
                ratios = []
                for pair in range(4):
                    key = row['rate'], row['waveform'], int(row['modulated']), pair
                    reference, study = samples[('reference',) + key], samples[('study',) + key]
                    self.assertEqual(reference[1], study[1])
                    ratios.append(study[0] / reference[0])
                self.assertEqual(row['paired_ratios'], ratios)
                self.assertEqual(row['median_paired_seconds_ratio'], statistics.median(ratios))
            self.assertEqual(reporter.qualification(result['summary']), result['qualification'])

    def test_slowdown_observations_include_modulation(self):
        result = self.run_report()
        decision = result['qualification']
        self.assertEqual(result['schema_version'], 2)
        self.assertEqual(len(decision['median_slowdown_cells']), 24)
        self.assertEqual(len(decision['all_pairs_slowdown_cells']), 24)
        self.assertEqual(sum(cell['modulated'] for cell in decision['median_slowdown_cells']), 12)
        self.assertFalse(decision['production_promotion_allowed'])
        self.assertIn('CPU acceptance: not established', self.output)

    def test_archived_extended_ci_summaries(self):
        archive = Path(__file__).resolve().parents[1] / 'experiments/oscillator/measurements/2026-10-09-shared-frequency-extended-ci-summary'
        files = list(archive.glob('*.json'))
        self.assertEqual(len(files), 4)
        for path in files:
            result = json.loads(path.read_bytes())
            self.assertEqual(result['schema_version'], 3)
            metadata = result['metadata']
            self.assertEqual(metadata['source_sha'], 'c09785828b8ec19c7daef22447b30480cf655fad')
            self.assertEqual(metadata['candidate_variant'], 'shared-frequency-v1')
            self.assertEqual(metadata['frames_per_path_and_pair'], 131072)
            self.assertEqual(metadata['pairs_per_cell'], 8)
            self.assertIn(metadata['comparison'], path.stem)
            self.assertEqual(len(result['summary']), 24)
            cells = set()
            for row in result['summary']:
                cells.add((row['rate'], row['waveform'], row['modulated']))
                ratios = row['paired_ratios']
                self.assertEqual(len(ratios), 8)
                self.assertEqual(row['median_paired_seconds_ratio'], statistics.median(ratios))
                self.assertEqual(row['min_paired_seconds_ratio'], min(ratios))
                self.assertEqual(row['max_paired_seconds_ratio'], max(ratios))
            self.assertEqual(len(cells), 24)
            self.assertEqual(reporter.qualification(result['summary']), result['qualification'])

    def test_favorable_or_mixed_pairs_never_promote(self):
        for row in self.rows:
            row['seconds'] = 1 if row['path'] == 'reference' else .5
        result = self.run_report()
        self.assertEqual(result['qualification']['median_slowdown_cells'], [])
        self.assertEqual(result['qualification']['all_pairs_slowdown_cells'], [])
        self.assertFalse(result['qualification']['production_promotion_allowed'])
        self.assertEqual(result['qualification']['cpu_acceptance'], 'not-established')
        for row in self.rows:
            if row['path'] == 'study':
                row['seconds'] = (.5, 1.1, 1.2, 1.3)[row['pair']]
        mixed = self.run_report()['qualification']
        self.assertEqual(len(mixed['median_slowdown_cells']), 24)
        self.assertEqual(mixed['all_pairs_slowdown_cells'], [])

if __name__ == '__main__':
    unittest.main()
