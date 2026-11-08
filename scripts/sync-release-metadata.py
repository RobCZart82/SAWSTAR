#!/usr/bin/env python3
"""Regenerate framework-required literal metadata; --check is CI-safe."""
import argparse, json, pathlib, re, sys
p=argparse.ArgumentParser();p.add_argument('--check',action='store_true');args=p.parse_args()
root=pathlib.Path(__file__).resolve().parents[1]
m=json.loads((root/'release.json').read_text());version=m['version']
if not re.fullmatch(r'\d+\.\d+\.\d+',version):raise ValueError('Invalid version')
if not re.fullmatch(r'(dev|rc\d+)?',m['candidate']):raise ValueError('Invalid candidate')
if m['candidate'] and m.get('release_date'):raise ValueError('Development/candidate builds must not carry a release date')
major,minor,patch=map(int,version.split('.'))
if max(major,minor,patch)>255:raise ValueError('Version exceeds framework byte fields')
def replace_once(text,pattern,replacement,label):
 text,count=re.subn(pattern,replacement,text)
 if count!=1:raise ValueError(f'Expected exactly one {label}, found {count}')
 return text
config=root/'src/plugin/config.h';s=config.read_text()
s=replace_once(s,r'(#define PLUG_VERSION_HEX\s+)\S+',lambda x:x[1]+f'0x{major:04x}{minor:02x}{patch:02x}','PLUG_VERSION_HEX')
s=replace_once(s,r'(#define PLUG_VERSION_STR\s+)"[^"]*"',lambda x:x[1]+json.dumps(version),'PLUG_VERSION_STR')
s=replace_once(s,r'(#define PLUG_COPYRIGHT_STR\s+)"[^"]*"',lambda x:x[1]+json.dumps(m['copyright'],ensure_ascii=False),'PLUG_COPYRIGHT_STR')
from xml.sax.saxutils import escape
outputs=[(config,s)]
for product in ('SAWSTAR','SAWSTARPremiumPreview','SAWSTARPremiumCandidate'):
 plist=root/f'src/plugin/resources/{product}-VST3-Info.plist';t=plist.read_text()
 for key,value in {'CFBundleShortVersionString':version,'CFBundleVersion':version,'LSMinimumSystemVersion':m['macos_minimum'],'NSHumanReadableCopyright':m['copyright'],'CFBundleGetInfoString':f"{product} v{version} {m['copyright']}"}.items():
  t=replace_once(t,r'(<key>'+re.escape(key)+r'</key>\s*<string>)[^<]*(</string>)',lambda x:x[1]+escape(value)+x[2],f'{product} metadata field {key}')
 outputs.append((plist,t))
changed=[]
for path,new in outputs:
 if path.read_text()!=new:
  changed.append(str(path.relative_to(root)))
  if not args.check:path.write_text(new)
if args.check and changed:sys.exit('Run scripts/sync-release-metadata.py: '+', '.join(changed))
print('Release metadata consistent.' if not changed else 'Updated: '+', '.join(changed))
