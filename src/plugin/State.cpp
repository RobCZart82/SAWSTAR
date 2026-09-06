// SPDX-License-Identifier: MIT
#include "plugin/State.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
namespace sawstar {
namespace {
constexpr std::array<uint8_t, 8> magic{{'S','A','W','S','T','A','R',0}};
uint64_t Read(const uint8_t* p, size_t n) {
  uint64_t value = 0; for (size_t i=0;i<n;++i) value |= uint64_t(p[i]) << (8*i); return value;
}
void Write(uint8_t* p, uint64_t value, size_t n) {
  for (size_t i=0;i<n;++i) p[i] = static_cast<uint8_t>(value >> (8*i));
}
double ReadDouble(const uint8_t* p) {
  const auto bits = Read(p,8); double value; std::memcpy(&value,&bits,8); return value;
}
}
Snapshot DefaultSnapshot() {
  Snapshot values{}; for(size_t i=0;i<values.size();++i) values[i]=kParameters[i].initial; return values;
}
StateBytes EncodeState(const Snapshot& values) {
  static_assert(sizeof(double)==8 && std::numeric_limits<double>::is_iec559, "IEEE-754 required");
  StateBytes bytes{}; std::copy(magic.begin(),magic.end(),bytes.begin());
  Write(bytes.data()+8,1,4); Write(bytes.data()+12,bytes.size()-16,4);
  for(size_t i=0;i<values.size();++i) {
    Write(bytes.data()+16+i*12,static_cast<uint32_t>(kParameters[i].id),4);
    double value=Sanitize(kParameters[i],values[i]); uint64_t bits;
    std::memcpy(&bits,&value,8); Write(bytes.data()+20+i*12,bits,8);
  }
  return bytes;
}
size_t DecodeState(const uint8_t* data, size_t size, Snapshot& output) {
  if (!data) return 0;
  Snapshot values=DefaultSnapshot(); size_t consumed=0;
  if(size>=8 && std::equal(magic.begin(),magic.end(),data)) {
    if(size<16 || Read(data+8,4)!=1) return 0;
    const size_t payload=static_cast<size_t>(Read(data+12,4));
    if(payload>64*12 || payload%12 || payload>size-16) return 0;
    consumed=16+payload;
    if(size!=consumed && size!=consumed+4) return 0;
    std::array<bool,kParameters.size()> seen{};
    for(size_t pos=16;pos<consumed;pos+=12) {
      const auto* spec=FindParameter(static_cast<uint32_t>(Read(data+pos,4)));
      if(!spec) continue;
      const auto id=static_cast<size_t>(spec->id);
      const double value=ReadDouble(data+pos+4);
      if(seen[id] || !std::isfinite(value)) return 0;
      seen[id]=true; values[id]=Sanitize(*spec,value);
    }
  } else {
    // Legacy 0.1.0: five little-endian physical doubles, optionally VST3 bypass.
    constexpr size_t legacySize=5*8;
    if(size!=legacySize && size!=legacySize+4) return 0;
    consumed=legacySize;
    for(size_t i=0;i<values.size();++i) {
      const double value=ReadDouble(data+i*8);
      if(!std::isfinite(value)) return 0;
      values[i]=Sanitize(kParameters[i],value);
    }
  }
  output=values; return consumed;
}
}
