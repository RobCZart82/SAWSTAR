"""Pure validation shared by the release publisher and offline regression tests."""
import hashlib
import json
import posixpath
import re
from urllib.parse import unquote, urlsplit
from pathlib import Path
from pathlib import PurePosixPath

REQUIRED_WORKFLOWS = frozenset(('build-macos.yml', 'build-windows.yml', 'quality.yml', 'workflow-lint.yml'))


def matching_runs(runs, sha):
    """Choose newest exact-commit push runs; exclude the draft preparer itself."""
    selected = {}
    for run in sorted(runs, key=lambda item: item['id'], reverse=True):
        name = run['path'].split('/')[-1]
        if (name != 'release.yml' and run['head_sha'] == sha
                and run['event'] == 'push' and name not in selected):
            selected[name] = run
    return selected


def workflows_ready(selected):
    """Missing/running checks wait; any terminal non-success blocks publication."""
    for run in selected.values():
        if run['status'] == 'completed' and run['conclusion'] != 'success':
            raise ValueError('Required workflow failed: ' + run['path'])
    return (REQUIRED_WORKFLOWS <= set(selected)
            and all(run['status'] == 'completed' for run in selected.values()))


MARKDOWN_LINK = re.compile(r'!?\[[^\]]*\]\(\s*(?:<([^>]+)>|([^\s)]+))(?:\s+[^)]*)?\)')

MARKDOWN_REFERENCE = re.compile(r'(?m)^ {0,3}\[[^\]]+\]:[ \t]*(?:<([^>]+)>|([^\s]+))')

def validate_markdown_payload_links(payloads):
    """Reject relative Markdown links whose targets are absent from the package."""
    names = set(payloads)
    missing = []
    for name, data in payloads.items():
        if not name.lower().endswith('.md'):
            continue
        text = data.decode('utf-8')
        matches = list(MARKDOWN_LINK.finditer(text)) + list(MARKDOWN_REFERENCE.finditer(text))
        for match in matches:
            target = match.group(1) or match.group(2)
            parsed = urlsplit(target)
            if parsed.scheme or parsed.netloc or not parsed.path:
                continue
            path = unquote(parsed.path)
            resolved = posixpath.normpath(posixpath.join(posixpath.dirname(name), path))
            if resolved == '..' or resolved.startswith('../') or (
                    resolved not in names and
                    not any(member.startswith(resolved.rstrip('/') + '/') for member in names)):
                missing.append(f'{name} -> {target}')
    if missing:
        raise ValueError('Broken relative Markdown link(s) in package: ' + '; '.join(missing))


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
    validate_markdown_payload_links({name: package.read(name) if name.lower().endswith('.md') else b''
                                     for name in files})
    return manifest
