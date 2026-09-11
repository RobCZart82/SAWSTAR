#!/usr/bin/env python3
"""Build an unsigned candidate PKG/DMG, or sign/notarize with supplied identities."""
import argparse, pathlib, shutil, subprocess, tempfile, hashlib
p=argparse.ArgumentParser()
p.add_argument('bundle',type=pathlib.Path)
p.add_argument('--application-identity');p.add_argument('--installer-identity');p.add_argument('--notary-profile')
a=p.parse_args()
credentials=[a.application_identity,a.installer_identity,a.notary_profile]
if any(credentials) and not all(credentials):p.error('Signing requires both identities and a Keychain notary profile')
root=pathlib.Path(__file__).resolve().parents[1];out=root/'dist';out.mkdir(exist_ok=True)
def run(*args):subprocess.run([str(x) for x in args],check=True)
run('lipo',a.bundle/'Contents/MacOS/SAWSTAR','-verify_arch','arm64','x86_64')
with tempfile.TemporaryDirectory() as temp:
 t=pathlib.Path(temp);payload=t/'payload';dest=payload/'Library/Audio/Plug-Ins/VST3/SAWSTAR.vst3'
 shutil.copytree(a.bundle,dest)
 docs=payload/'Library/Application Support/SAWSTAR/Documentation';docs.mkdir(parents=True)
 for name in ['LICENSE','THIRD_PARTY_NOTICES.md','docs/INSTALLATION.md','docs/SYSTEM_REQUIREMENTS.md']:
  shutil.copy2(root/name,docs/pathlib.Path(name).name)
 shutil.copytree(root/'docs/manuals',docs/'manuals');shutil.copytree(root/'third_party/licenses',docs/'licenses')
 if all(credentials):run('codesign','--force','--options','runtime','--timestamp','--sign',a.application_identity,dest)
 run('pkgbuild','--root',payload,'--identifier','io.github.robczart82.sawstar','--version','1.0.0','--install-location','/',t/'component.pkg')
 distribution=t/'distribution.xml'
 distribution.write_text('''<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="2"><title>SAWSTAR 1.0.0 RC2</title>
<options customize="never" rootVolumeOnly="true" hostArchitectures="x86_64,arm64"/>
<allowed-os-versions><os-version min="11.0"/></allowed-os-versions>
<choices-outline><line choice="main"/></choices-outline>
<choice id="main" visible="false"><pkg-ref id="io.github.robczart82.sawstar"/></choice>
<pkg-ref id="io.github.robczart82.sawstar" version="1.0.0">component.pkg</pkg-ref></installer-gui-script>''')
 image=t/'image';image.mkdir();pkg=image/'SAWSTAR-1.0.0-rc2-macOS-Universal.pkg'
 args=['productbuild','--distribution',distribution,'--package-path',t]
 if all(credentials):args+=['--sign',a.installer_identity,'--timestamp']
 run(*args,pkg)
 if all(credentials):
  run('xcrun','notarytool','submit',pkg,'--keychain-profile',a.notary_profile,'--wait')
  run('xcrun','stapler','staple',pkg)
 shutil.copy2(root/'docs/INSTALLATION.md',image/'INSTALLATION.md')
 shutil.copy2(pkg,out/pkg.name)
 dmg=out/'SAWSTAR-1.0.0-rc2-macOS-Universal.dmg'
 run('hdiutil','create','-volname','SAWSTAR 1.0.0 RC2','-srcfolder',image,'-format','UDZO',dmg)
 if all(credentials):
  run('codesign','--timestamp','--sign',a.application_identity,dmg)
  run('xcrun','notarytool','submit',dmg,'--keychain-profile',a.notary_profile,'--wait')
  run('xcrun','stapler','staple',dmg)
 for file in [out/pkg.name,dmg]:file.with_suffix(file.suffix+'.sha256').write_text(hashlib.sha256(file.read_bytes()).hexdigest()+'  '+file.name+'\n')
