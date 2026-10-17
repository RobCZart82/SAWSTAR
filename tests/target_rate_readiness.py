# SPDX-License-Identifier: MIT
import copy
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('readiness',
    Path(__file__).resolve().parents[1] / 'scripts/report-target-rate-readiness.py')
readiness = importlib.util.module_from_spec(spec)
spec.loader.exec_module(readiness)


class ReadinessTest(unittest.TestCase):
    def setUp(self):
        self.inputs = {name: readiness.load(path)[0] for name, path in readiness.INPUTS.items()}

    def mac_rows(self):
        return self.inputs['M1']['paired_results']

    def test_actual_two_targets_keep_all_rounds_and_known_windows_failures(self):
        result = readiness.report(self.inputs)
        self.assertEqual(result['input_cells'], 576)
        self.assertEqual(len(result['rows']), 36)
        self.assertFalse(result['raw_reverification_performed'])
        self.assertFalse(result['cpu_acceptance'])
        self.assertFalse(result['native_host_acceptance'])
        self.assertFalse(result['production_promotion_allowed'])
        for row in result['rows']:
            self.assertEqual(row['native_status'], 'not_accepted')
            self.assertEqual(row['support_policy'], 'undecided')
            for group in row['comparisons'].values():
                self.assertEqual(group['cells'], 8)
                self.assertEqual(group['blocks_per_path_total'], 32768)
            if row['platform'] == 'Windows' and row['rate'] >= 96000:
                candidate = row['comparisons']['candidate']
                self.assertEqual(candidate['reference_over_budget_blocks'], 32768)
                self.assertEqual(candidate['study_over_budget_blocks'], 32768)
                self.assertEqual(candidate['study_all_blocks_over_budget_cells'], 8)

    def test_missing_or_duplicate_cell_even_with_288_records_rejected(self):
        rows = self.mac_rows()
        rows[-1] = copy.deepcopy(rows[0])
        with self.assertRaisesRegex(ValueError, 'duplicate'): readiness.report(self.inputs)
        rows.pop()
        with self.assertRaisesRegex(ValueError, '288'): readiness.report(self.inputs)

    def test_unknown_grid_coordinate_rejected(self):
        for key, value in [('waveform', 4), ('round', 3), ('rate', 44100),
                           ('buffer', 256), ('comparison', 'new-candidate'), ('workload', 'unknown')]:
            original = self.mac_rows()[0][key]
            self.mac_rows()[0][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError): readiness.report(self.inputs)
            self.mac_rows()[0][key] = original

    def test_nonfinite_zero_and_boolean_ratios_rejected(self):
        row = self.mac_rows()[0]
        for value in (float('nan'), float('inf'), -1, 0, True):
            row['median_paired_p99_ratio'] = value
            with self.subTest(value=value), self.assertRaises(ValueError): readiness.report(self.inputs)

    def test_invalid_block_counts_and_boolean_coordinates_rejected(self):
        row = self.mac_rows()[0]
        for key, value in [('study_over_budget_blocks', 4097), ('study_over_budget_blocks', -1),
                           ('reference_over_budget_blocks', True), ('blocks_per_path', 2048),
                           ('paired_observations', 8), ('waveform', False)]:
            old = row[key]
            row[key] = value
            with self.subTest(key=key), self.assertRaises(ValueError): readiness.report(self.inputs)
            row[key] = old

    def test_rounds_and_control_are_never_averaged_or_subtracted(self):
        for row in self.mac_rows():
            row['median_paired_p99_ratio'] = 1.4 if row['comparison'] == 'reference-repeat' else (1.2 if row['round'] == 1 else .8)
        result = readiness.report(self.inputs)
        for row in result['rows']:
            if row['platform'] != 'M1': continue
            self.assertEqual(row['comparisons']['candidate']['p99_ratio_max'], 1.2 if row['round'] == 1 else .8)
            self.assertEqual(row['comparisons']['reference-repeat']['p99_ratio_max'], 1.4)

    def test_worst_ties_keep_both_workloads_and_negative_counts(self):
        selected = []
        for row in self.mac_rows():
            if row['round'] == 1 and row['rate'] == 48000 and row['buffer'] == 128 and row['comparison'] == 'candidate' and row['waveform'] == 0:
                row['median_paired_p99_ratio'] = 3
                row['study_over_budget_blocks'] = 4096
                selected.append(row)
        self.assertEqual(len(selected), 2)
        result = readiness.report(self.inputs)
        group = next(r for r in result['rows'] if (r['platform'], r['round'], r['rate'], r['buffer']) == ('M1', 1, 48000, 128))['comparisons']['candidate']
        self.assertEqual(len(group['worst_p99_cells']), 2)
        self.assertEqual({r['workload'] for r in group['worst_p99_cells']}, {'stationary-v1', 'modulated-v1'})
        self.assertTrue(all(r['study_over_budget_blocks'] == 4096 for r in group['worst_p99_cells']))

    def test_favourable_synthetic_data_still_cannot_grant_acceptance(self):
        for row in self.mac_rows() + self.inputs['Windows']['rows']:
            row['reference_over_budget_blocks'] = row['study_over_budget_blocks'] = 0
            row['median_paired_p99_ratio'] = .9
        result = readiness.report(self.inputs)
        self.assertFalse(result['native_host_acceptance'])
        self.assertFalse(result['production_promotion_allowed'])
        self.assertTrue(all(r['support_policy'] == 'undecided' for r in result['rows']))

    def test_source_and_dependency_identity_mismatch_rejected(self):
        for key, value in [('source_sha', 'a' * 40), ('DaisySP_sha', 'b' * 40)]:
            old = self.inputs['M1'][key]
            self.inputs['M1'][key] = value
            with self.assertRaises(ValueError): readiness.report(self.inputs)
            self.inputs['M1'][key] = old
        key = next(iter(self.inputs['Windows']['source_file_sha256']))
        self.inputs['Windows']['source_file_sha256'][key] = '0' * 64
        with self.assertRaisesRegex(ValueError, '20 source hashes differ'): readiness.report(self.inputs)

    def test_incomplete_or_unreviewed_evidence_rejected(self):
        for key, value in [('state', 'incomplete'), ('raw_measurements_verified', False),
                           ('native_host_acceptance', True), ('cells', 287), ('verifier_exit_code', 1)]:
            old = self.inputs['M1'][key]
            self.inputs['M1'][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError): readiness.report(self.inputs)
            self.inputs['M1'][key] = old

    def test_missing_or_swapped_platform_rejected(self):
        with self.assertRaises(ValueError): readiness.report({'M1': self.inputs['M1']})
        with self.assertRaises(ValueError): readiness.report({'M1': self.inputs['Windows'], 'Windows': self.inputs['M1']})

    def test_shuffled_inputs_do_not_change_worst_case_report(self):
        expected = readiness.report(self.inputs)
        self.mac_rows().reverse()
        self.inputs['Windows']['rows'].reverse()
        self.assertEqual(expected, readiness.report(self.inputs))

    def test_report_does_not_change_original_evidence(self):
        before = copy.deepcopy(self.inputs)
        readiness.report(self.inputs)
        self.assertEqual(before, self.inputs)

    def test_json_duplicate_keys_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / 'input.json'
            path.write_text('{"source_sha":"a", "source_sha":"b"}')
            with self.assertRaisesRegex(ValueError, 'Duplicate'): readiness.load(path)


if __name__ == '__main__':
    unittest.main()
