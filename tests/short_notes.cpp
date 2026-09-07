// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cstdlib>
#include <iostream>
#include <cmath>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
int main(){
 for(float sr:{44100.f,48000.f,96000.f})for(int mode=0;mode<3;++mode){
  for(int samples:{0,1})for(int ending:{0,1,2,3}){
   sawstar::Synth s;s.Reset(sr);s.SetVoiceMode(mode,0,true);s.SetParameters(0,1,1,.8,10);
   s.SetFilterEnvelope(36,0,1,1,.7,10);
   // Entire mouse glissando arrives between audio samples; also exceeds polyphony.
   for(int note=36;note<97;++note){s.Midi(0x90,note,100);for(int i=0;i<samples;++i)s.Process();
    if(ending==0)s.Midi(0x80,note,0);
    if(ending==1)s.Midi(0x90,note,0);
   }
   if(ending==2)s.Midi(0xb0,123,0);
   if(ending==3){s.Midi(0xb0,64,127);for(int n=36;n<97;++n)s.Midi(0x80,n,0);s.Midi(0xb0,64,0);}
   for(int i=0;i<int(sr/2);++i)check(std::isfinite(s.Process()),"finite short-note output");
   check(s.ActiveVoices()==0,"zero-sample note sequence must release every voice");
   check(s.Process()==0,"short-note sequence must become silent");
   // A real held note must still sustain, followed by a normal release.
   s.Midi(0x90,60,100);for(int i=0;i<int(sr/10);++i)s.Process();
   check(s.ActiveVoices()==1&&s.Held(60),"held note must sustain");
   s.Midi(0x80,60,0);for(int i=0;i<int(sr/2);++i)s.Process();
   check(s.ActiveVoices()==0,"normal release after short notes");
  }
 }
}
