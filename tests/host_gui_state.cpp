// SPDX-License-Identifier: MIT
#include "plugin/State.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <vector>

void check(bool ok,const char* message) {
  if(!ok){std::cerr<<message<<'\n';std::exit(1);}
}
int main() {
  using namespace sawstar;
  auto sound=DefaultSnapshot();sound[0]=-23.5;sound[8]=1234.;
  const auto preset=EncodeState(sound);
  for(float scale:{0.f,.75f,1.f,1.25f}) {
    const auto host=EncodeHostState(sound,scale);Snapshot restored{};float gui=-1.f;
    check(DecodeHostState(host.data(),host.size(),restored,gui)==host.size() &&
          restored==sound && gui==scale,"host sound and scale roundtrip");
    check(DecodeState(host.data(),host.size(),restored)==host.size() && restored==sound,
          "old decoder ignores GUI record and retains sound");
    auto bytes=std::vector<uint8_t>(host.begin(),host.end());
    bytes.insert(bytes.end(),{1,0,0,0});
    check(DecodeHostState(bytes.data(),bytes.size(),restored,gui)==host.size() &&
          gui==scale && bytes[host.size()]==1,"VST3 bypass trailer remains unconsumed");
    for(size_t n=0;n<host.size();++n) {
      // A complete old sound-only payload is a valid legacy host state.
      restored=sound;gui=1.25f;
      check(!DecodeHostState(host.data(),n,restored,gui) && restored==sound && gui==1.25f,
            "truncated payload cannot mutate outputs");
    }
  }
  Snapshot restored{};float gui=.75f;
  check(DecodeHostState(preset.data(),preset.size(),restored,gui)==preset.size() &&
        gui==0.f && restored==sound,"old v1 host state uses automatic GUI size");
  const std::vector<uint8_t> old(40,0);
  gui=1.25f;
  check(DecodeHostState(old.data(),old.size(),restored,gui)==40 && gui==0.f,
        "headerless legacy host state remains supported");
  for(float invalid:{-.75f,2.f,std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}) {
    const auto bytes=EncodeHostState(sound,invalid);
    check(DecodeHostState(bytes.data(),bytes.size(),restored,gui) && gui==0.f,
          "invalid encoded editor preference falls back to auto");
  }
  auto host=EncodeHostState(sound,.75f);
  // Corrupt only the optional GUI value, preserving the valid sound payload.
  std::fill(host.end()-8,host.end(),0xff);
  gui=1.f;
  check(DecodeHostState(host.data(),host.size(),restored,gui) && restored==sound && gui==0.f,
        "invalid stored GUI value does not discard the sound");
  const auto valid=EncodeHostState(sound,.75f);
  auto duplicate=std::vector<uint8_t>(valid.begin(),valid.end());
  duplicate.insert(duplicate.end(),valid.end()-12,valid.end());
  const auto length=duplicate.size()-16;
  for(int i=0;i<4;++i)duplicate[12+i]=static_cast<uint8_t>(length>>(8*i));
  restored=sound;gui=1.25f;
  check(!DecodeHostState(duplicate.data(),duplicate.size(),restored,gui) && restored==sound && gui==1.25f,
        "duplicate editor record rejected transactionally");
  // Two independent instances/preset applications never share an editor preference.
  auto first=EncodeHostState(sound,.75f),second=EncodeHostState(sound,1.25f);
  float a=0.f,b=0.f;
  check(DecodeHostState(first.data(),first.size(),restored,a) &&
        DecodeHostState(second.data(),second.size(),restored,b) && a==.75f && b==1.25f,
        "instance states remain independent");
  check(EncodeState(sound)==preset,"sound preset bytes remain unchanged");
  check(DecodePresetFile(preset.data(),preset.size(),restored) && restored==sound,
        "sound preset decode retains the original sound contract");
}
