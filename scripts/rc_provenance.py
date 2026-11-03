# SPDX-License-Identifier: MIT
"""Verify the pinned, deliberately patched dependency tree used by RC builds."""
import ast
import hashlib
import importlib.util
from pathlib import Path
import subprocess
import tempfile


def git(root, *args):
    return subprocess.check_output(['git', *args], cwd=root)


def normalize(data):
    return data.replace(b'\r\n', b'\n')


def require_bytes(actual, expected, label):
    if normalize(actual) != normalize(expected):
        raise ValueError('Unexpected dependency content: ' + label)


def require_paths(actual, allowed, label):
    unexpected = set(actual) - set(allowed)
    if unexpected:
        raise ValueError(f'Unexpected {label} changes: {sorted(unexpected)}')


def checkout_provenance(root):
    root = Path(root).resolve()
    status = git(root, 'status', '--porcelain=v1', '-z', '--untracked-files=all').decode().split('\0')
    entries = [x for x in status if x]
    # Nothing in project code/docs may drift from the checkout being packaged.
    require_paths([x[3:] for x in entries], {'third_party/iPlug2'}, 'checkout')
    proof = dependency_provenance(root)
    proof.update(source_checkout_clean=True, checkout_status=entries)
    return proof


def dependency_provenance(root):
    root = Path(root).resolve()
    iplug = root / 'third_party/iPlug2'
    pin = git(root, 'rev-parse', 'HEAD:third_party/iPlug2').decode().strip()
    if git(iplug, 'rev-parse', 'HEAD').decode().strip() != pin:
        raise ValueError('iPlug2 revision does not match the checkout gitlink')
    spec = importlib.util.spec_from_file_location('rc_pinned_patch', root / 'scripts/patch-iplug2-midi-overflow.py')
    patch = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(patch)
    paths = sorted({x[0] for x in patch.PATCHES})
    changed = git(iplug, 'diff', '--name-only', 'HEAD', '-z').decode().split('\0')
    require_paths([x for x in changed if x], set(paths) | {'Dependencies/IPlug/VST3_SDK/README.md'}, 'iPlug2')
    files = {}
    with tempfile.TemporaryDirectory(prefix='sawstar-expected-patch-') as temp:
        expected = Path(temp)
        for name in paths:
            p = expected / name
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_bytes(git(iplug, 'show', 'HEAD:' + name))
        patch.apply(expected)
        for name in paths:
            data = (iplug / name).read_bytes()
            require_bytes(data, (expected / name).read_bytes(), name)
            files[name] = dict(sha256=hashlib.sha256(data).hexdigest(),
                              normalized_sha256=hashlib.sha256(normalize(data)).hexdigest())
    sdk = iplug / 'Dependencies/IPlug/VST3_SDK'
    tree = ast.parse((root / 'scripts/bootstrap-plugin.py').read_text())
    sdk_pin = next(ast.literal_eval(x.value) for x in tree.body
                   if isinstance(x, ast.Assign) and any(isinstance(t, ast.Name) and t.id == 'SDK_COMMIT' for t in x.targets))
    if git(sdk, 'rev-parse', 'HEAD').decode().strip() != sdk_pin:
        raise ValueError('VST3 SDK revision does not match bootstrap pin')
    if git(sdk, 'status', '--porcelain', '--untracked-files=all').strip():
        raise ValueError('VST3 SDK tree is modified')
    require_bytes((sdk / 'README.md').read_bytes(), git(sdk, 'show', 'HEAD:README.md'), 'SDK README')
    children = {}
    for name in ('base', 'pluginterfaces', 'public.sdk', 'cmake'):
        child_pin = git(sdk, 'rev-parse', 'HEAD:' + name).decode().strip()
        if git(sdk / name, 'rev-parse', 'HEAD').decode().strip() != child_pin:
            raise ValueError('VST3 SDK child revision mismatch: ' + name)
        if git(sdk / name, 'status', '--porcelain', '--untracked-files=all').strip():
            raise ValueError('VST3 SDK child tree is modified: ' + name)
        children[name] = child_pin
    daisy = root / 'third_party/DaisySP'
    daisy_pin = git(root, 'rev-parse', 'HEAD:third_party/DaisySP').decode().strip()
    if git(daisy, 'rev-parse', 'HEAD').decode().strip() != daisy_pin or git(daisy, 'status', '--porcelain', '--untracked-files=all').strip():
        raise ValueError('DaisySP differs from the pinned clean dependency')
    return dict(schema=1, managed_dependency_changes_verified=True,
                iplug2_commit=pin,
                patched_files=files,
                patch_script_sha256=hashlib.sha256((root / 'scripts/patch-iplug2-midi-overflow.py').read_bytes()).hexdigest(),
                sdk_commit=sdk_pin, sdk_children=children, daisysp_commit=daisy_pin,
                note='Raw source_dirty may remain true for the verified iPlug2 patch and SDK placeholder replacement; this is not a pristine upstream dependency tree.')
