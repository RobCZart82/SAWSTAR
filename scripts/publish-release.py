#!/usr/bin/env python3
"""Prepare a verified draft from successful builds of this exact commit."""
import hashlib,json,os,pathlib,subprocess,time,zipfile
from release_validation import matching_runs, workflows_ready, validate_package
from release_draft import release_for_tag, write_verified_draft
root=pathlib.Path.cwd();m=json.loads((root/'release.json').read_text())
if m['candidate']:
 print('Release preparation skipped: development candidate '+m['candidate'])
 raise SystemExit(0)
version=m['version'];sha=os.environ['RELEASE_SHA'];repo=os.environ['GH_REPO'];tag='v'+version
if not m.get('release_date'):
 raise ValueError('Final release date required')
notes=root/f'docs/RELEASE_NOTES_{version}.md'
if not notes.is_file():
 raise ValueError('Version-specific release notes required')
def gh(*args):return subprocess.check_output(['gh',*args],text=True)
def api(path):return json.loads(gh('api',path))
# Main documentation may change after publication. A published version is an
# intentional no-op here; the mutation guard below still rejects replacement
# if a draft is published concurrently during preparation. API errors fail closed.
existing=release_for_tag(gh,repo,tag)
if existing is not None and not existing['draft']:
 print(f'Release preparation skipped: {tag} is already published; no assets or tags changed')
 raise SystemExit(0)
deadline=time.monotonic()+2100
while True:
 runs=api(f'repos/{repo}/actions/runs?head_sha={sha}&event=push&per_page=100')['workflow_runs']
 selected=matching_runs(runs,sha)
 if workflows_ready(selected):break
 if time.monotonic()>deadline:raise RuntimeError('Build timeout; release not published')
 time.sleep(20)
if not all(x['conclusion']=='success' for x in selected.values()):
 raise ValueError('Build failed; release not published')
assets=root/'release-assets';assets.mkdir()
expected={'SAWSTAR-macos-universal-candidate':('macOS-Universal','SAWSTAR-macos.zip'),
'SAWSTAR-windows-x64-candidate':('Windows-x64','SAWSTAR-windows.zip'),
'SAWSTAR-windows-ARM64-candidate':('Windows-ARM64','SAWSTAR-windows.zip')}
found=set()
for run in selected.values():
 for artifact in api(f"repos/{repo}/actions/runs/{run['id']}/artifacts")['artifacts']:
  name=artifact['name']
  if name not in expected:continue
  if artifact['expired']:
   raise ValueError('Expired artifact: '+name)
  dest=root/'downloaded'/name
  gh('run','download',str(run['id']),'--name',name,'--dir',str(dest))
  platform,archive=expected[name];z=dest/archive
  with zipfile.ZipFile(z) as package:
   validate_package(package,sha,version)
  (assets/f'SAWSTAR-{version}-{platform}-Manual.zip').write_bytes(z.read_bytes())
  extensions={'.dmg','.pkg'} if platform.startswith('macOS') else {'.exe'}
  installers=[p for p in dest.rglob('*') if p.suffix in extensions]
  if any(sum(p.suffix==extension for p in installers)!=1 for extension in extensions):
   raise ValueError('Missing or extra installer type for '+platform)
  for p in installers:
   if not p.name.startswith(f'SAWSTAR-{version}-') or '-rc' in p.name:
    raise ValueError('Installer version mismatch: '+p.name)
   (assets/p.name).write_bytes(p.read_bytes())
  found.add(name)
if found!=set(expected):
 raise ValueError('Incomplete platform coverage')
checks=''.join(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+p.name+'\n' for p in sorted(assets.iterdir()))
(assets/'SHA256SUMS.txt').write_text(checks)
# An unpublished draft may be refreshed; public releases and existing tags
# pointing elsewhere are protected. Partial uploads stay visibly unverified.
write_verified_draft(gh,repo,tag,sha,notes,assets)
print(f'Verified draft ready for final review: {tag}. Publication is a separate explicit step.')
