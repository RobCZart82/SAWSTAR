// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <type_traits>

namespace sawstar {
// Offline-only schema. Pointer values/padding are never compared or printed.
// Names are string literals; index identifies channel/phase/tap, not an address.
template<size_t Capacity> struct NamedStateTrace {
  struct Field { const char* name=nullptr; uint64_t bits=0; unsigned index=0; char kind=0; };
  std::array<Field,Capacity> fields{};
  size_t size=0;
  void Clear(){size=0;}
  template<class T> void Add(const char* name,unsigned index,T value){
    if(size==Capacity)throw std::runtime_error("Filter state trace capacity exceeded");
    auto& f=fields[size++];f.name=name;f.index=index;f.bits=0;
    if constexpr(std::is_same_v<T,double>){f.kind='d';std::memcpy(&f.bits,&value,8);}
    else if constexpr(std::is_same_v<T,float>){f.kind='f';uint32_t bits;std::memcpy(&bits,&value,4);f.bits=bits;}
    else {static_assert(std::is_integral_v<T>);f.kind='i';f.bits=static_cast<uint64_t>(value);}
  }
  template<size_t Other> void Append(const NamedStateTrace<Other>& other){
    for(size_t i=0;i<other.size;++i){
      if(size==Capacity)throw std::runtime_error("Filter state trace capacity exceeded");
      const auto& f=other.fields[i];fields[size++]={f.name,f.bits,f.index,f.kind};
    }
  }
  // Copy only defined entries, including when a voice stops sounding.
  void CopyFrom(const NamedStateTrace& other){
    size=other.size;for(size_t i=0;i<size;++i)fields[i]=other.fields[i];
  }
};
using FilterStateTrace=NamedStateTrace<2304>;
struct FilterStateFrame {
  std::array<FilterStateTrace,16> voices{};
  void Clear(){for(auto& voice:voices)voice.Clear();}
  void CopyFrom(const FilterStateFrame& other){
    for(size_t v=0;v<voices.size();++v)voices[v].CopyFrom(other.voices[v]);
  }
};
}
