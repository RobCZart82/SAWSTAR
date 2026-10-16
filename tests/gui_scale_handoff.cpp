// SPDX-License-Identifier: MIT
#include "plugin/State.h"
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

void check(bool ok,const char* name){if(!ok){std::cerr<<name<<'\n';std::exit(1);}}
struct HostHandler {int dirtyCalls=0;void setDirty(bool value){if(value)++dirtyCalls;}};
namespace Steinberg {
namespace Vst {using IComponentHandler2=HostHandler;}
template<class T> struct FUnknownPtr {
  T* ptr;explicit FUnknownPtr(T* p):ptr(p){}explicit operator bool()const{return ptr!=nullptr;}
  T* operator->()const{return ptr;}
};
}
class IByteChunk {
  std::vector<uint8_t> bytes;
public:
  int PutBytes(const void* p,int n){auto* b=static_cast<const uint8_t*>(p);bytes.insert(bytes.end(),b,b+n);return Size();}
  template<class T> int Put(const T* p){return PutBytes(p,sizeof(T));}
  int Size()const{return static_cast<int>(bytes.size());}
  const uint8_t* GetData()const{return bytes.data();}
};
struct Param {double value=0.;double Value()const{return value;}};
struct UI {
  int resizes=0;float scale=0.f;std::thread::id lastThread;
  void Resize(int,int,float s){++resizes;scale=s;lastThread=std::this_thread::get_id();}
};
#define VST3_API 1
#define PLUG_WIDTH 1280
#define PLUG_HEIGHT 760
class SAWSTAR {
public:
  std::array<Param,sawstar::kParameters.size()> params{};
  std::atomic<float> mSavedGuiScale{0.f};std::atomic<bool> mGuiScaleRestored{false};
  std::atomic<bool> mArpReset{false},mStateRestored{false};
  float mGuiScale=1.f;UI ui;HostHandler host;bool open=true,hasHandler=true,rejectParams=false;
  const Param* GetParam(int n)const{return &params[n];}
  int UnserializeParams(const IByteChunk& chunk,int pos){
    if(rejectParams)return -1;
    if(chunk.Size()!=static_cast<int>(params.size()*sizeof(double)))return -1;
    for(size_t i=0;i<params.size();++i)std::memcpy(&params[i].value,chunk.GetData()+i*sizeof(double),sizeof(double));
    return pos+chunk.Size();
  }
  HostHandler* GetComponentHandler(){return hasHandler?&host:nullptr;}
  UI* GetUI(){return open?&ui:nullptr;}
  float GetScaleForScreen(int,int){return 1.f;}
  void SaveGuiScale(float);
  void SyncRestoredGuiScale();
  bool SerializeState(IByteChunk&)const;
  int UnserializeState(const IByteChunk&,int);
};
#include "gui_scale_hooks.inc"
int main(){
  SAWSTAR first,second;const auto initial=sawstar::DefaultSnapshot();
  for(size_t i=0;i<initial.size();++i)first.params[i].value=initial[i];
  first.SaveGuiScale(.75f);check(first.host.dirtyCalls==1,"scale-only edit marks host dirty");
  first.SaveGuiScale(.75f);check(first.host.dirtyCalls==1,"unchanged scale does not dirty host twice");
  IByteChunk saved;check(first.SerializeState(saved),"host serialization");
  std::thread hostThread([&]{check(second.UnserializeState(saved,0)==saved.Size(),"restore from host thread");});
  hostThread.join();
  check(second.ui.resizes==0 && second.mGuiScale==1.f,"host restore never touches live GUI");
  second.SyncRestoredGuiScale();
  check(second.ui.resizes==1 && second.ui.scale==.75f &&
        second.ui.lastThread==std::this_thread::get_id(),"GUI idle applies restored size");
  check(second.host.dirtyCalls==0,"restore does not dirty project");
  second.SyncRestoredGuiScale();check(second.ui.resizes==1,"idle consumes restore once");
  second.SaveGuiScale(1.25f);check(first.mSavedGuiScale.load()==.75f,"independent instance preference");
  second.open=false;second.UnserializeState(saved,0);second.SyncRestoredGuiScale();
  check(second.mGuiScale==.75f && second.ui.resizes==1,"closed editor retains size for next creation");
  const auto old=sawstar::EncodeState(initial);IByteChunk legacy;legacy.PutBytes(old.data(),old.size());
  second.open=true;check(second.UnserializeState(legacy,0)>0,"old sound-only host state");
  second.SyncRestoredGuiScale();check(second.mGuiScale==1.f,"old project uses automatic scale");
  IByteChunk bad;const uint8_t zero=0;bad.Put(&zero);
  second.SaveGuiScale(1.25f);
  check(second.UnserializeState(bad,0)==-1 && second.mSavedGuiScale.load()==1.25f &&
        !second.mGuiScaleRestored.load(),"invalid restore cannot alter scale or queue GUI work");
  second.hasHandler=false;second.SaveGuiScale(.75f);
  check(second.mSavedGuiScale.load()==.75f,"missing optional host handler still saves state");
  IByteChunk prefixed;const int marker=123;prefixed.Put(&marker);
  check(first.SerializeState(prefixed),"append state after host prefix");
  check(second.UnserializeState(prefixed,sizeof(marker))==prefixed.Size(),"nonzero host chunk offset");
  second.SyncRestoredGuiScale();second.SaveGuiScale(1.25f);second.rejectParams=true;
  check(second.UnserializeState(saved,0)==-1 && second.mSavedGuiScale.load()==1.25f &&
        !second.mGuiScaleRestored.load(),"failed parameter restore does not publish GUI preference");
}
