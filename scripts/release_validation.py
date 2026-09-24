"""Pure validation shared by the release publisher and offline regression tests."""
import hashlib
import json
from pathlib import Path
from pathlib import PurePosixPath

REQUIRED_WORKFLOWS = frozenset(('build-macos.yml', 'build-windows.yml', 'quality.yml'))


def matching_runs(runs, sha):
    """Choose the newest push run of each required workflow for this exact commit."""
    selected = {}
    for run in sorted(runs, key=lambda item: item['id'], reverse=True):
        name = run['path'].split('/')[-1]
        if (name in REQUIRED_WORKFLOWS and run['head_sha'] == sha
                and run['event'] == 'push' and name not in selected):
            selected[name] = run
    return selected


def workflows_ready(selected):
    """Missing/running checks wait; any terminal non-success blocks publication."""
    for run in selected.values():
        if run['status'] == 'completed' and run['conclusion'] != 'success':
            raise ValueError('Required workflow failed: ' + run['path'])
    return (set(selected) == REQUIRED_WORKFLOWS
            and all(run['status'] == 'completed' for run in selected.values()))


def validate_package(package, sha, version):
    """Accept exactly the hashed file set, with no duplicate or unsafe members."""
    names = package.namelist()
    if len(names) != len(set(names)):
        raise ValueError('Duplicate archive member')
    for name in names:
        path = PurePosixPath(name)
        if (path.is_absolute() or '..' in path.parts or '\\' in name
                or ':' in name or str(path) != name or name.endswith('/')):
            raise ValueError('Unsafe or non-file archive member: ' + name)
    if package.testzip() is not None:
        raise ValueError('Archive CRC mismatch')
    def unique_pairs(pairs):
        result = {}
        for key, value in pairs:
            if key in result:
                raise ValueError('Duplicate manifest key: ' + key)
            result[key] = value
        return result
    manifest = json.loads(package.read('PACKAGE-MANIFEST.json'), object_pairs_hook=unique_pairs)
    if manifest['source_commit'] != sha or manifest['version'] != version:
        raise ValueError('Artifact commit/version mismatch')
    files = manifest['files_sha256']
    if not isinstance(files, dict) or 'PACKAGE-MANIFEST.json' in files:
        raise ValueError('Invalid manifest file map')
    if set(names) != set(files) | {'PACKAGE-MANIFEST.json'}:
        raise ValueError('Archive and manifest file sets differ')
    for name, digest in files.items():
        if hashlib.sha256(package.read(name)).hexdigest() != digest:
            raise ValueError('Artifact file checksum mismatch: ' + name)
    bundle_prefix = 'SAWSTAR.vst3/Contents/'
    bundle_files = [name for name in files if name.startswith(bundle_prefix)]
    if not bundle_files or not any(package.getinfo(name).file_size for name in bundle_files):
        raise ValueError('Missing non-empty SAWSTAR VST3 bundle')
    required_docs = {
        'LICENSE',
        'THIRD_PARTY_NOTICES.md',
        'docs/THIRD_PARTY.md',
        'docs/INSTALLATION.md',
        f'docs/RELEASE_NOTES_{version}.md',
        'docs/SYSTEM_REQUIREMENTS.md',
        'docs/manuals/README.md',
    }
    missing_docs = required_docs - set(files)
    if missing_docs:
        raise ValueError('Missing required distribution documents: ' + ', '.join(sorted(missing_docs)))
    for language in ('EN', 'HU'):
        manual = f'docs/manuals/SAWSTAR-User-Manual-{language}.pdf'
        if manual not in files:
            raise ValueError('Missing user manual: ' + language)
        if not package.read(manual).startswith(b'%PDF-'):
            raise ValueError('Invalid user manual PDF: ' + language)
    licenses_dir = Path(__file__).resolve().parents[1] / 'third_party' / 'licenses'
    expected_licenses = {
        'licenses/' + item.relative_to(licenses_dir).as_posix()
        for item in licenses_dir.rglob('*') if item.is_file()
    }
    expected_licenses.add('licenses/SAWSTAR-BRANDING-LICENSE.txt')
    missing_licenses = expected_licenses - set(files)
    if missing_licenses:
        raise ValueError('Missing required license notices: ' + ', '.join(sorted(missing_licenses)))
    return manifest
