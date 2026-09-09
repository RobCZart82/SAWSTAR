// SPDX-License-Identifier: MIT
#include "plugin/State.h"
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cstring>
void check(bool ok,const char* name){if(!ok){std::cerr<<name<<"\n";std::exit(1);}}
int main(){
 using namespace sawstar;
 Snapshot wanted=DefaultSnapshot();
 const double fixture[]={-23.5,23.24,780,0.42,1234, 23, 64, 82, 1900, 35, 100, 36, 80, 25, 450, .35, 800, 12, 36, 18};
 for(size_t i=0;i<20;++i)wanted[i]=fixture[i];
 wanted[21]=42;wanted[26]=1;wanted[31]=12;wanted[32]=2;wanted[33]=3;wanted[34]=1;wanted[36]=65;wanted[39]=1;wanted[42]=1;wanted[43]=31;wanted[44]=.7;wanted[45]=45;wanted[46]=1;wanted[47]=40;wanted[48]=250;wanted[49]=55;wanted[50]=3500;wanted[51]=1;wanted[52]=1;wanted[53]=2;wanted[54]=1;wanted[55]=30;wanted[56]=70;wanted[57]=4.2;wanted[58]=2500;wanted[59]=2;wanted[60]=230;wanted[61]=0;wanted[62]=3;wanted[63]=-35;wanted[64]=3.5;wanted[65]=42;wanted[71]=2;wanted[72]=4;wanted[73]=-36;wanted[80]=5;wanted[81]=1;wanted[82]=29;wanted[83]=1;wanted[84]=4;wanted[85]=5;wanted[86]=67;wanted[87]=3;wanted[88]=33;wanted[89]=1;
 Snapshot out{};
 wanted[90]=1;wanted[91]=73;wanted[92]=2;
 const auto state=EncodeState(wanted);
 check(DecodeState(state.data(),state.size(),out)==state.size() && out==wanted,"roundtrip");
 check(state[8]==1 && state[12]==80 && state[13]==4 && state[16]==0,"wire fixture");
 // Old v1 payload with only the original five records must keep new defaults.
 auto oldV1=std::vector<uint8_t>(state.begin(),state.begin()+76);oldV1[12]=60;oldV1[13]=0;
 check(DecodeState(oldV1.data(),oldV1.size(),out)==76 && out[1]==wanted[1] &&
       out[5]==20 && out[6]==0 && out[7]==75,"five-record v1 migration");
 auto eightV1=std::vector<uint8_t>(state.begin(),state.begin()+112);eightV1[12]=96;eightV1[13]=0;
 check(DecodeState(eightV1.data(),eightV1.size(),out)==112 && out[6]==wanted[6] &&
       out[8]==12000 && out[9]==0 && out[10]==0,"eight-record v1 migration");
 auto elevenV1=std::vector<uint8_t>(state.begin(),state.begin()+148);elevenV1[12]=132;elevenV1[13]=0;
 check(DecodeState(elevenV1.data(),elevenV1.size(),out)==148 && out[10]==100 &&
       out[11]==0 && out[12]==0 && out[13]==10 && out[14]==200 && out[15]==0 && out[16]==250,
       "eleven-record v1 migration");
 auto seventeenV1=std::vector<uint8_t>(state.begin(),state.begin()+220);seventeenV1[12]=204;seventeenV1[13]=0;
 check(DecodeState(seventeenV1.data(),seventeenV1.size(),out)==220 && out[16]==800 &&
       out[17]==2 && out[18]==24,"seventeen-record v1 migration");
 auto nineteenV1=std::vector<uint8_t>(state.begin(),state.begin()+244);nineteenV1[12]=228;nineteenV1[13]=0;
 check(DecodeState(nineteenV1.data(),nineteenV1.size(),out)==244 && out[19]==0,
       "old nineteen-record project keeps original loudness");
 auto thirtyOne=std::vector<uint8_t>(state.begin(),state.begin()+388);thirtyOne[12]=116;thirtyOne[13]=1;
 check(DecodeState(thirtyOne.data(),thirtyOne.size(),out)==388 && out[19]==18 && out[31]==0 && out[32]==0,
       "old mixer project keeps LP12 without drive");
 auto thirtyThree=std::vector<uint8_t>(state.begin(),state.begin()+412);thirtyThree[12]=140;thirtyThree[13]=1;
 check(DecodeState(thirtyThree.data(),thirtyThree.size(),out)==412 && out[33]==0 && out[34]==0 && out[36]==0,"old filter state has saws and no LFO");
 auto fortyTwo=std::vector<uint8_t>(state.begin(),state.begin()+520);fortyTwo[12]=248;fortyTwo[13]=1;
 check(DecodeState(fortyTwo.data(),fortyTwo.size(),out)==520 && out[42]==0 && out[43]==25 && out[44]==.3 && out[45]==35,"old LFO and One presets keep chorus off");
 auto fortySix=std::vector<uint8_t>(state.begin(),state.begin()+568);fortySix[12]=40;fortySix[13]=2;
 check(DecodeState(fortySix.data(),fortySix.size(),out)==568 && out[42]==1 && out[46]==0 && out[47]==20 && out[48]==350,"old chorus state keeps delay off");
 auto fiftyFour=std::vector<uint8_t>(state.begin(),state.begin()+664);fiftyFour[12]=136;fiftyFour[13]=2;
 check(DecodeState(fiftyFour.data(),fiftyFour.size(),out)==664 && out[46]==1 && out[54]==0 && out[55]==20 && out[57]==2.5,"old delay state keeps reverb off");
 auto fiftyNine=std::vector<uint8_t>(state.begin(),state.begin()+724);fiftyNine[12]=196;fiftyNine[13]=2;
 check(DecodeState(fiftyNine.data(),fiftyNine.size(),out)==724 && out[54]==1 && out[59]==0 && out[60]==0 && out[61]==1,"old reverb state keeps poly without glide");
 auto sixtyTwo=std::vector<uint8_t>(state.begin(),state.begin()+760);sixtyTwo[12]=232;sixtyTwo[13]=2;
 check(DecodeState(sixtyTwo.data(),sixtyTwo.size(),out)==760 && out[26]==1 && out[62]==0 && out[63]==0,"old noise stays Dark with neutral color");
 auto sixtyFour=std::vector<uint8_t>(state.begin(),state.begin()+784);sixtyFour[12]=0;sixtyFour[13]=3;
 check(DecodeState(sixtyFour.data(),sixtyFour.size(),out)==784 && out[63]==-35 && out[65]==0 && out[71]==0 && out[82]==0,"old noise preset disables new modulation");
 auto eightyThree=std::vector<uint8_t>(state.begin(),state.begin()+1012);eightyThree[12]=228;eightyThree[13]=3;
 check(DecodeState(eightyThree.data(),eightyThree.size(),out)==1012 && out[82]==29 && out[83]==0 && out[89]==0,"old modulation preset keeps arp and hold off");
 // Fixture for old physical doubles: -12, 10, 100, .7, 250, little endian.
 auto ninety=std::vector<uint8_t>(state.begin(),state.begin()+1096);ninety[12]=56;ninety[13]=4;
 check(DecodeState(ninety.data(),ninety.size(),out)==1096 && out[90]==0 && out[91]==50,"old GUI state keeps master width bypassed");
 const uint8_t old[]={0,0,0,0,0,0,40,192,0,0,0,0,0,0,36,64,0,0,0,0,0,0,89,64,
   102,102,102,102,102,102,230,63,0,0,0,0,0,64,111,64};
 auto legacyDefault=DefaultSnapshot();legacyDefault[19]=0;
 check(DecodeState(old,sizeof(old),out)==40 && out==legacyDefault,"legacy fixture");
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
