#!/usr/bin/env python3
"""Prepare a verified draft from successful builds of this exact commit."""
import hashlib,json,os,pathlib,subprocess,time,zipfile
root=pathlib.Path.cwd();m=json.loads((root/'release.json').read_text())
if m['candidate']:raise SystemExit('Candidate: no public release')
version=m['version'];sha=os.environ['RELEASE_SHA'];repo=os.environ['GH_REPO'];tag='v'+version
assert m.get('release_date'), 'Final release date required'
notes=root/f'docs/RELEASE_NOTES_{version}.md'
assert notes.is_file(), 'Version-specific release notes required'
def gh(*args):return subprocess.check_output(['gh',*args],text=True)
def api(path):return json.loads(gh('api',path))
workflows={'build-macos.yml','build-windows.yml'}
deadline=time.monotonic()+2100
while True:
 runs=api(f'repos/{repo}/actions/runs?head_sha={sha}&event=push&per_page=100')['workflow_runs']
 selected={}
 for run in runs:
  name=run['path'].split('/')[-1]
  if name in workflows and name not in selected:selected[name]=run
 if len(selected)==2 and all(x['status']=='completed' for x in selected.values()):break
 if time.monotonic()>deadline:raise RuntimeError('Build timeout; release not published')
 time.sleep(20)
assert all(x['conclusion']=='success' for x in selected.values()),'Build failed; release not published'
assets=root/'release-assets';assets.mkdir()
expected={'SAWSTAR-macos-universal-candidate':('macOS-Universal','SAWSTAR-macos.zip'),
'SAWSTAR-windows-x64-candidate':('Windows-x64','SAWSTAR-windows.zip'),
'SAWSTAR-windows-ARM64-candidate':('Windows-ARM64','SAWSTAR-windows.zip')}
found=set()
for run in selected.values():
 for artifact in api(f"repos/{repo}/actions/runs/{run['id']}/artifacts")['artifacts']:
  name=artifact['name']
  if name not in expected:continue
  assert not artifact['expired']
  dest=root/'downloaded'/name
  gh('run','download',str(run['id']),'--name',name,'--dir',str(dest))
  platform,archive=expected[name];z=dest/archive
  with zipfile.ZipFile(z) as package:
   assert package.testzip() is None
   manifest=json.loads(package.read('PACKAGE-MANIFEST.json'))
   assert manifest['source_commit']==sha, 'Artifact commit mismatch'
   assert manifest['version']==version, 'Artifact version mismatch'
   for entry,digest in manifest['files_sha256'].items():
    assert hashlib.sha256(package.read(entry)).hexdigest()==digest, 'Artifact file checksum mismatch'
   assert any(n.endswith('SAWSTAR-User-Manual-EN.pdf') for n in package.namelist())
   assert any(n.endswith('SAWSTAR-User-Manual-HU.pdf') for n in package.namelist())
  (assets/f'SAWSTAR-{version}-{platform}-Manual.zip').write_bytes(z.read_bytes())
  extensions={'.dmg','.pkg'} if platform.startswith('macOS') else {'.exe'}
  installers=[p for p in dest.rglob('*') if p.suffix in extensions]
  assert len(installers)==len(extensions),'Missing or extra installer'
  for p in installers:
   assert f'SAWSTAR-{version}-' in p.name and '-rc' not in p.name
   (assets/p.name).write_bytes(p.read_bytes())
  found.add(name)
assert found==set(expected),'Incomplete platform coverage'
checks=''.join(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+p.name+'\n' for p in sorted(assets.iterdir()))
(assets/'SHA256SUMS.txt').write_text(checks)
# A draft keeps partial uploads private; existing releases are never overwritten.
gh('release','create',tag,'--target',sha,'--draft','--title',f'SAWSTAR {version}','--notes-file',str(notes))
gh('release','upload',tag,*[str(p) for p in sorted(assets.iterdir())])
release=json.loads(gh('release','view',tag,'--json','assets'))
assert {a['name']:a['size'] for a in release['assets']}=={p.name:p.stat().st_size for p in assets.iterdir()}
print(f'Verified draft ready for final review: {tag}. Publication is a separate explicit step.')
