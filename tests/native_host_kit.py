# SPDX-License-Identifier: MIT
import csv
import importlib.util
import json
from pathlib import Path
import plistlib
import struct
import tempfile
import unittest
from unittest.mock import patch
import warnings
import zipfile

ROOT = Path(__file__).resolve().parents[1]


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT / path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


kit = module('host_kit', 'scripts/prepare-native-host-kit.py')
package = module('preview_package', 'scripts/package-premium-preview.py')
SOURCE = 'a' * 40


def universal(fat64=False):
    image = bytearray(256)
    image[:8] = struct.pack('>II', 0xcafebabf if fat64 else 0xcafebabe, 2)
    for index, cpu in enumerate((0x01000007, 0x0100000c)):
        offset = 128 + index * 64
        if fat64:
            struct.pack_into('>IIQQII', image, 8 + index * 32, cpu, 0, offset, 32, 6, 0)
        else:
            struct.pack_into('>IIIII', image, 8 + index * 20, cpu, 0, offset, 32, 6)
        struct.pack_into('<IIIIIIII', image, offset, 0xfeedfacf, cpu, 0, 8, 0, 0, 0, 0)
    return bytes(image)


class NativeHostKit(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        for name in ('LICENSE', 'THIRD_PARTY_NOTICES.md', 'assets/branding/LICENSE.txt',
                     'docs/PREMIUM_PREVIEW.md', 'docs/NATIVE_PREVIEW_PAIR_QUALIFICATION.md',
                     'docs/PREMIUM_PREVIEW_LEVELS.md',
                     'docs/measurements/premium-preview-levels-2026-10-15-local-arm64.csv'):
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('fixture', encoding='utf-8')
        self.paths = {}
        for role, (_, stem, signature, _) in kit.IDENTITIES.items():
            bundle = self.root / (stem + '.vst3')
            binary = bundle / 'Contents/MacOS' / stem
            binary.parent.mkdir(parents=True)
            binary.write_bytes(universal())
            binary.chmod(0o755)
            (bundle / 'Contents/Info.plist').write_bytes(plistlib.dumps(dict(
                CFBundleIdentifier='io.github.robczart82.' + stem.lower(),
                CFBundleSignature=signature, CFBundleName=stem, CFBundleExecutable=stem)))
            self.paths[role] = self.root / (role + '.zip')
            with patch.object(package, 'ROOT', self.root), patch.object(package.subprocess, 'check_output', return_value=SOURCE):
                package.package(bundle, self.paths[role], role=role)

    def rewrite(self, role='candidate', change=None, extra=None, refresh_hashes=False):
        path = self.paths[role]
        with zipfile.ZipFile(path) as archive:
            items = [(info, archive.read(info)) for info in archive.infolist()]
        manifest = json.loads(dict((info.filename, data) for info, data in items)['PACKAGE-MANIFEST.json'])
        if change:
            change(manifest, items)
        if refresh_hashes:
            manifest['files_sha256'] = {info.filename: kit.sha(data) for info, data in items
                                        if info.filename != 'PACKAGE-MANIFEST.json'}
        with warnings.catch_warnings(), zipfile.ZipFile(path, 'w') as archive:
            warnings.simplefilter('ignore', UserWarning)
            for info, data in items:
                archive.writestr(info, json.dumps(manifest).encode() if info.filename == 'PACKAGE-MANIFEST.json' else data)
            if extra:
                archive.writestr(*extra)
        Path(str(path) + '.sha256').write_text(kit.sha(path.read_bytes()) + '  ' + path.name + '\n')

    def prepare(self, **kwargs):
        return kit.prepare(self.paths['reference'], self.paths['candidate'],
                           self.root / 'output', kwargs.get('source', SOURCE), kwargs.get('fx_mode', 'HP12'))

    def test_real_packager_contract_and_exact_unmeasured_protocol(self):
        before = {role: path.read_bytes() for role, path in self.paths.items()}
        output = self.prepare()
        with (output / 'measurements.csv').open(newline='') as stream:
            rows = list(csv.DictReader(stream))
        self.assertEqual(len(rows), 112)
        self.assertEqual(len({row['case_id'] for row in rows}), 28)
        self.assertEqual(len({row['window_id'] for row in rows}), 112)
        self.assertEqual([row['case_id'] for row in rows[::4]][:4],
                         ['C-128-' + mode + '-AUTO' for mode in kit.MODES])
        self.assertEqual([row['role'] for row in rows[:4]], ['reference', 'candidate', 'candidate', 'reference'])
        self.assertEqual([row['order'] for row in rows[:4]], ['AB', 'AB', 'BA', 'BA'])
        self.assertEqual(sum(int(row['warmup_seconds']) + int(row['observation_seconds']) for row in rows), 7840)
        self.assertEqual({family: sum(row['case_id'].startswith(family) for row in rows) // 4
                          for family in 'CVGF'}, dict(C=16, V=8, G=2, F=2))
        buffers = [row['buffer_samples'] for row in rows]
        self.assertEqual(buffers, sorted(buffers, key=lambda value: value != '128'))
        self.assertTrue(all(row['status'] == 'not_run' and row['cpu_average'] == '' and row['xrun_start'] == '' for row in rows))
        self.assertTrue(all(row['filter_mode'] == 'HP12' for row in rows if row['preset'] == 'P1'))
        provenance = json.loads((output / 'provenance.json').read_text())
        self.assertFalse(provenance['native_host_acceptance'])
        self.assertFalse(provenance['ci_success_verified'])
        self.assertTrue(all(value == '' for value in json.loads((output / 'environment.json').read_text()).values()))
        for role, path in self.paths.items():
            self.assertEqual(path.read_bytes(), before[role])
            with zipfile.ZipFile(path) as archive:
                self.assertEqual(provenance['packages'][role]['package_manifest_sha256'], kit.sha(archive.read('PACKAGE-MANIFEST.json')))
        with self.assertRaises(FileExistsError):
            self.prepare()

    def test_existing_empty_directory_is_not_overwritten(self):
        (self.root / 'output').mkdir()
        with self.assertRaises(FileExistsError):
            self.prepare()
        self.assertEqual(list((self.root / 'output').iterdir()), [])

    def test_wrong_source_role_and_policy_rejected_before_output(self):
        for key, value in [('source_commit', 'b' * 40), ('pair_role', 'reference'),
                           ('native_host_acceptance', True), ('latency_samples', True),
                           ('platform', 'windows-x64'), ('plugin_unique_id', 'SwSt')]:
            original = self.paths['candidate'].read_bytes()
            self.rewrite(change=lambda manifest, items: manifest.update({key: value}))
            with self.subTest(key=key), self.assertRaisesRegex(ValueError, 'mismatch'):
                self.prepare()
            self.assertFalse((self.root / 'output').exists())
            self.paths['candidate'].write_bytes(original)

    def test_checksum_and_member_hash_fail_independently(self):
        path = self.paths['candidate']
        path.write_bytes(path.read_bytes() + b'changed')
        with self.assertRaisesRegex(ValueError, 'checksum'):
            self.prepare()
        self.rewrite(change=lambda manifest, items: manifest['files_sha256'].update({'LICENSE': '0' * 64}))
        with self.assertRaisesRegex(ValueError, 'Member hash'):
            self.prepare()

    def test_bad_inventory_unsafe_names_duplicates_and_links(self):
        original = self.paths['candidate'].read_bytes()
        for name in ('../escape', '/absolute', 'a\\b', 'a//b', 'extra', 'LICENSE', 'license'):
            self.paths['candidate'].write_bytes(original)
            self.rewrite(extra=(name, b'bad'))
            with self.subTest(name=name), self.assertRaises(ValueError):
                self.prepare()
        self.paths['candidate'].write_bytes(original)
        link = zipfile.ZipInfo('link')
        link.create_system = 3
        link.external_attr = 0o120777 << 16
        self.rewrite(extra=(link, b'target'))
        with self.assertRaisesRegex(ValueError, 'non-regular'):
            self.prepare()

    def test_valid_hash_cannot_hide_wrong_plist_or_binary(self):
        original = self.paths['candidate'].read_bytes()
        for suffix, data in [('Info.plist', plistlib.dumps({'CFBundleSignature': 'SwSt'})),
                             ('MacOS/SAWSTARPremiumCandidate', b'not universal')]:
            self.paths['candidate'].write_bytes(original)

            def change(manifest, items):
                for index, (info, content) in enumerate(items):
                    if info.filename.endswith('/Contents/' + suffix):
                        items[index] = (info, data)
                        if suffix.startswith('MacOS/'):
                            manifest['binary_sha256'] = kit.sha(data)

            self.rewrite(change=change, refresh_hashes=True)
            with self.subTest(suffix=suffix), self.assertRaises(ValueError):
                self.prepare()

    def test_macho_bounds_overlap_and_cpu_headers(self):
        self.assertEqual(kit.universal_architectures(universal(True)), ['arm64', 'x86_64'])
        for offset, value in [(16, 99999), (36, 128), (132, 7)]:
            data = bytearray(universal())
            struct.pack_into('>I' if offset < 128 else '<I', data, offset, value)
            with self.subTest(offset=offset), self.assertRaises(ValueError):
                kit.universal_architectures(bytes(data))

    def test_explicit_source_and_fx_selection_required(self):
        for source, mode in [('main', 'HP12'), (SOURCE, 'unknown')]:
            with self.assertRaises(ValueError):
                self.prepare(source=source, fx_mode=mode)
        self.assertFalse((self.root / 'output').exists())

    def test_duplicate_json_keys_rejected(self):
        with self.assertRaisesRegex(ValueError, 'Duplicate JSON'):
            json.loads('{"pair_role":"reference","pair_role":"candidate"}', object_pairs_hook=kit.unique_object)


if __name__ == '__main__':
    unittest.main()
