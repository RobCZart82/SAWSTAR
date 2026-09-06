// SPDX-License-Identifier: MIT
#include "plugin/State.h"
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cstring>
void check(bool ok,const char* name){if(!ok){std::cerr<<name<<"\n";std::exit(1);}}
int main(){
 using namespace sawstar;
 Snapshot wanted{{-23.5,23.24,780,0.42,1234, 23, 64, 82}}, out{};
 const auto state=EncodeState(wanted);
 check(DecodeState(state.data(),state.size(),out)==state.size() && out==wanted,"roundtrip");
 check(state[8]==1 && state[12]==96 && state[16]==0,"wire fixture");
 // Old v1 payload with only the original five records must keep new defaults.
 auto oldV1=std::vector<uint8_t>(state.begin(),state.begin()+76);oldV1[12]=60;
 check(DecodeState(oldV1.data(),oldV1.size(),out)==76 && out[1]==wanted[1] &&
       out[5]==20 && out[6]==0 && out[7]==75,"five-record v1 migration");
 // Fixture for old physical doubles: -12, 10, 100, .7, 250, little endian.
 const uint8_t old[]={0,0,0,0,0,0,40,192,0,0,0,0,0,0,36,64,0,0,0,0,0,0,89,64,
   102,102,102,102,102,102,230,63,0,0,0,0,0,64,111,64};
 check(DecodeState(old,sizeof(old),out)==40 && out==DefaultSnapshot(),"legacy fixture");
 auto reject=[&](std::vector<uint8_t> b){out=wanted;check(!DecodeState(b.data(),b.size(),out)&&out==wanted,"invalid state mutated values");};
 for(size_t n=0;n<state.size();++n) reject({state.begin(),state.begin()+n});
 auto b=std::vector<uint8_t>(state.begin(),state.end()); b[8]=2;reject(b);
 b.assign(state.begin(),state.end());b[0]='X';reject(b);
 b.assign(state.begin(),state.end());b[28]=0;reject(b); // duplicate known ID
 b.assign(state.begin(),state.end());b[12]=255;reject(b);
 b.assign(state.begin(),state.end());for(int i=20;i<28;++i)b[i]=255;reject(b);
 b.assign(state.begin(),state.end());b.resize(b.size()+4);check(DecodeState(b.data(),b.size(),out)==state.size(),"bypass trailer");
 b.assign(state.begin(),state.end());b[16]=99;check(DecodeState(b.data(),b.size(),out)&&out[0]==-12,"unknown ID/default");
 wanted[0]=10;auto clamped=EncodeState(wanted);check(DecodeState(clamped.data(),clamped.size(),out)&&out[0]==0,"clamp");
}
