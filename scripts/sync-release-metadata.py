#!/usr/bin/env python3
"""Regenerate framework-required literal metadata; --check is CI-safe."""
import argparse, json, pathlib, re, sys
p=argparse.ArgumentParser();p.add_argument('--check',action='store_true');args=p.parse_args()
root=pathlib.Path(__file__).resolve().parents[1]
m=json.loads((root/'release.json').read_text());version=m['version']
if not re.fullmatch(r'\d+\.\d+\.\d+',version):raise ValueError('Invalid version')
if not re.fullmatch(r'(rc\d+)?',m['candidate']):raise ValueError('Invalid candidate')
major,minor,patch=map(int,version.split('.'))
if max(major,minor,patch)>255:raise ValueError('Version exceeds framework byte fields')
config=root/'src/plugin/config.h';s=config.read_text()
s=re.sub(r'(#define PLUG_VERSION_HEX\s+)\S+',lambda x:x[1]+f'0x{major:04x}{minor:02x}{patch:02x}',s)
s=re.sub(r'(#define PLUG_VERSION_STR\s+)"[^"]*"',lambda x:x[1]+json.dumps(version),s)
s=re.sub(r'(#define PLUG_COPYRIGHT_STR\s+)"[^"]*"',lambda x:x[1]+json.dumps(m['copyright'],ensure_ascii=False),s)
plist=root/'src/plugin/resources/SAWSTAR-VST3-Info.plist';t=plist.read_text()
from xml.sax.saxutils import escape
for key,value in {'CFBundleShortVersionString':version,'CFBundleVersion':version,'LSMinimumSystemVersion':m['macos_minimum'],'NSHumanReadableCopyright':m['copyright'],'CFBundleGetInfoString':f"SAWSTAR v{version} {m['copyright']}"}.items():
 t=re.sub(r'(<key>'+key+r'</key>\s*<string>)[^<]*(</string>)',lambda x:x[1]+escape(value)+x[2],t)
changed=[]
for path,new in [(config,s),(plist,t)]:
 if path.read_text()!=new:
  changed.append(str(path.relative_to(root)))
  if not args.check:path.write_text(new)
if args.check and changed:sys.exit('Run scripts/sync-release-metadata.py: '+', '.join(changed))
print('Release metadata consistent.' if not changed else 'Updated: '+', '.join(changed))
