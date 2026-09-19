// SPDX-License-Identifier: MIT
#include "SynthTestRig.h"
#include "LegacyControlReference.h"
#include <memory>
#include <iostream>
#include <cstdlib>

void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
int main(){
 // Independent pre-extraction mapping protects against sharing the same bug.
 // Each parameter is exercised at both endpoints, plus whole-state fixtures.
 for(float sr:{44100.f,48000.f,96000.f})
 for(size_t fixture=0;fixture<4+2*kParameters.size();++fixture){
  auto values=DefaultSnapshot();
  for(size_t i=0;i<values.size();++i){
   if(fixture==1)values[i]=kParameters[i].minimum;
   if(fixture==2)values[i]=kParameters[i].maximum;
  }
  if(fixture>=4){const size_t id=(fixture-4)/2;
   values[id]=(fixture%2)?kParameters[id].maximum:kParameters[id].minimum;
  }
  if(fixture==3){values[20]=25;values[21]=70;values[22]=35;values[23]=10;values[19]=3;values[0]=-20;values[48]=37;values[46]=1;}
  const auto bytes=EncodeState(values);Snapshot restored{};
  check(DecodeState(bytes.data(),bytes.size(),restored)==bytes.size(),"state decode failed");
  check(restored==values,"physical parameter values changed in state roundtrip");
  auto actual=std::make_unique<Rig>();auto reference=std::make_unique<LegacyControlReference>();
  actual->init(sr);reference->init(sr);actual->apply(restored);reference->apply(values);
  actual->midi(144,60,100);reference->midi(144,60,100);
  for(int frame=0;frame<2048;++frame){
   if(frame==512){actual->midi(144,67,90);reference->midi(144,67,90);}
   if(frame==1024){actual->midi(128,67,0);reference->midi(128,67,0);}
   if(frame==1536){actual->midi(128,60,0);reference->midi(128,60,0);}
   const auto a=actual->process(),b=reference->process();
   if(a.left!=b.left||a.right!=b.right){std::cerr<<"sr="<<sr<<" fixture="<<fixture<<" frame="<<frame<<'\n';check(false,"production mapping differs from independent reference");}
  }
 }
}
