#!/usr/bin/env python3
"""Fail if the VST3 imports a separate MSVC runtime or has the wrong architecture."""
from pathlib import Path
import struct,sys
b=Path(sys.argv[1]).read_bytes();expected={'x64':0x8664,'ARM64':0xaa64}[sys.argv[2]]
pe=struct.unpack_from('<I',b,60)[0]
if b[pe:pe+4]!=b'PE\0\0':raise SystemExit('Not a PE image')
machine,sections=struct.unpack_from('<HH',b,pe+4)
if machine!=expected:raise SystemExit(f'Wrong PE architecture: {machine:#x}')
opt=pe+24;opt_size=struct.unpack_from('<H',b,pe+20)[0]
if struct.unpack_from('<H',b,opt)[0]!=0x20b:raise SystemExit('Expected PE32+')
section_start=opt+opt_size
regions=[]
for i in range(sections):
 p=section_start+i*40;virtual_size,rva,raw_size,raw=struct.unpack_from('<IIII',b,p+8)
 regions.append((rva,max(virtual_size,raw_size),raw))
def offset(rva):
 for start,size,raw in regions:
  if start<=rva<start+size:return raw+rva-start
 raise ValueError(f'Unmapped RVA {rva:#x}')
def name(rva):
 p=offset(rva);return b[p:b.index(b'\0',p)].decode('ascii')
imports=[]
for directory,stride,name_index in [(1,20,3),(13,32,1)]:
 rva,size=struct.unpack_from('<II',b,opt+112+directory*8)
 if not rva:continue
 start=offset(rva)
 for p in range(start,start+size,stride):
  fields=struct.unpack_from('<'+'I'*(stride//4),b,p)
  if not any(fields):break
  if directory==13 and not fields[0]&1:raise SystemExit('Unsupported delay import VA mode')
  imports.append(name(fields[name_index]))
print('Imported DLLs:',', '.join(imports))
bad=[n for n in imports if n.lower().startswith(('vcruntime','msvcp','msvcr'))]
if bad:raise SystemExit('Unexpected separate VC++ runtime imports: '+', '.join(bad))
print('Architecture and static MSVC runtime import audit: PASS')
