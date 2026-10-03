"""Update only private drafts after package validation; never publish or move tags."""
import hashlib
import json


def release_for_tag(gh, repo, tag):
    pages = json.loads(gh('api', f'repos/{repo}/releases?per_page=100',
                          '--paginate', '--slurp'))
    matches = [release for page in pages for release in page
               if release['tag_name'] == tag]
    if len(matches) > 1:
        raise ValueError('Ambiguous release tag')
    return matches[0] if matches else None


def draft_for_tag(gh, repo, tag, sha):
    release = release_for_tag(gh, repo, tag)
    if release is not None and not release['draft']:
        raise ValueError('Published releases cannot be replaced')
    # target_commitish does not override an existing tag. Reject a stale tag,
    # including annotated tags; never force it to a different commit.
    refs = json.loads(gh('api', f'repos/{repo}/git/matching-refs/tags/{tag}'))
    exact = [ref for ref in refs if ref['ref'] == 'refs/tags/' + tag]
    if len(exact) > 1:
        raise ValueError('Ambiguous tag reference')
    if exact:
        obj = exact[0]['object']
        for _ in range(10):
            if obj['type'] != 'tag':
                break
            obj = json.loads(gh('api', f"repos/{repo}/git/tags/{obj['sha']}"))['object']
        if obj['type'] != 'commit' or obj['sha'] != sha:
            raise ValueError('Existing release tag does not point to the tested commit')
    return release


def write_verified_draft(gh, repo, tag, sha, notes, assets):
    """Called only after all exact-commit workflows and local packages pass."""
    files = sorted(assets.iterdir())
    expected = {path.name: path.stat().st_size for path in files}
    release = draft_for_tag(gh, repo, tag, sha)
    if release is not None:
        # Unknown old files need deliberate review instead of silent deletion.
        if {asset['name'] for asset in release['assets']} - set(expected):
            raise ValueError('Draft has unexpected assets; review them before refreshing')
        # Mark it incomplete BEFORE the first replacement. Failed uploads remain
        # private and visibly unverified, rather than claiming a complete draft.
        gh('release', 'edit', tag, '--draft', '--target', sha,
           '--title', tag + ' draft refresh in progress',
           '--notes', 'Draft refresh in progress. Do not publish until verification succeeds.')
    else:
        gh('release', 'create', tag, '--target', sha, '--draft',
           '--title', tag + ' draft refresh in progress',
           '--notes', 'Draft refresh in progress. Do not publish until verification succeeds.')
    args = ['release', 'upload', tag, *[str(path) for path in files]]
    if release is not None:
        args.append('--clobber')
    gh(*args)
    # Re-read the API, including draft status and target, after upload. A partial
    # upload cannot reach the final title/notes or the success message.
    current = draft_for_tag(gh, repo, tag, sha)
    if current is None or current['target_commitish'] != sha:
        raise ValueError('Draft target mismatch')
    actual = {asset['name']: asset['size'] for asset in current['assets']}
    if len(current['assets']) != len(expected) or actual != expected:
        raise ValueError('Draft asset list/size mismatch')
    for asset in current['assets']:
        digest = asset.get('digest')
        if digest and digest != 'sha256:' + hashlib.sha256((assets / asset['name']).read_bytes()).hexdigest():
            raise ValueError('Uploaded asset checksum mismatch: ' + asset['name'])
    gh('release', 'edit', tag, '--draft', '--target', sha,
       '--title', 'SAWSTAR ' + tag.removeprefix('v'), '--notes-file', str(notes))
