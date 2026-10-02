// SPDX-License-Identifier: MIT
// Minimal VST3 interfaces compile the actual pinned and patched adapter method.
#include <vector>
#include <utility>
#include <iostream>
#include <cstdlib>
using int32=int;
constexpr int kResultTrue=0,kBypassParam=99,kMIDICCParamStartIdx=1000;
constexpr int kCountCtrlNumber=130,kAfterTouch=128,kPitchBend=129,kHost=0;
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
struct IMidiMsg {
 enum EControlChangeMsg { Sustain=64 };
 int status=0,note=0,offset=0;double value=0;
 void MakeControlChangeMsg(EControlChangeMsg n,double v,int ch,int o){status=0xb0|ch;note=int(n);value=v;offset=o;}
 void MakeChannelATMsg(int v,int o,int ch){status=0xd0|ch;value=v;offset=o;}
 void MakePitchWheelMsg(double v,int ch,int o){status=0xe0|ch;value=v;offset=o;}
};
template<class T> struct IPlugQueue {std::vector<T> events;void Push(T x){events.push_back(x);}};
struct IParamValueQueue {
 int id;std::vector<std::pair<int,double>> points;int failed=-1;bool invalidRead=false;
 int getPointCount(){return int(points.size());}
 int getParameterId(){return id;}
 int getPoint(int i,int& offset,double& value){
  if(i<0||i>=getPointCount()){invalidRead=true;return 1;}
  if(i==failed)return 1;
  offset=points[size_t(i)].first;value=points[size_t(i)].second;return kResultTrue;
 }
};
struct IParameterChanges {
 std::vector<IParamValueQueue> queues;
 int getParameterCount(){return int(queues.size());}
 IParamValueQueue* getParameterData(int i){return &queues[size_t(i)];}
};
struct ProcessData {IParameterChanges* inputParameterChanges;};
struct Param {double value=0;void SetNormalized(double v){value=v;}};
struct Plug {
 Param p;int changes=0,lastOffset=-1;
 int NParams(){return 1;}Param* GetParam(int){return &p;}
 void OnParamChange(int,int,int offset){++changes;lastOffset=offset;}
};
class IPlugVST3ProcessorBase {
public:
 Plug mPlug;bool bypassed=false;std::vector<IMidiMsg> delivered;
 bool GetBypassed(){return bypassed;}void SetBypassed(bool v){bypassed=v;}
 void ProcessMidiMsg(IMidiMsg x){delivered.push_back(x);}
 void ProcessParameterChanges(ProcessData&,IPlugQueue<IMidiMsg>&);
};
#include "host_midi_hook.inc"
int main(){
 for(int channel : {0,7,15})for(int control : {1,64,120,121,123,kAfterTouch,kPitchBend}){
  IParameterChanges changes{{{kMIDICCParamStartIdx+channel*kCountCtrlNumber+control,{{0,1},{32,0},{32,.5},{127,.25}}}}};
  ProcessData data{&changes};IPlugQueue<IMidiMsg> feedback;IPlugVST3ProcessorBase adapter;
  adapter.ProcessParameterChanges(data,feedback);
  check(adapter.delivered.size()==4&&feedback.events.size()==4,"Controller points were coalesced or duplicated");
  for(size_t i=0;i<4;++i){const auto& event=adapter.delivered[i];
   check(event.offset==changes.queues[0].points[i].first,"Controller sample offset changed");
   check((event.status&15)==channel,"Controller MIDI channel changed");
   const double v=changes.queues[0].points[i].second;
   const double expected=control==kPitchBend?v*2-1:(control==kAfterTouch?int(v*127):v);
   check(event.value==expected,"Controller value or equal-offset ordering changed");
  }
 }
 IParameterChanges changes{{{0,{{0,.2},{16,.8}}},{kBypassParam,{{0,0},{32,1}}},
                            {kMIDICCParamStartIdx+64,{}},
                            {kMIDICCParamStartIdx+64,{{0,1},{16,.5},{32,0}},1}}};
 ProcessData data{&changes};IPlugQueue<IMidiMsg> feedback;IPlugVST3ProcessorBase adapter;
 adapter.ProcessParameterChanges(data,feedback);
 check(adapter.mPlug.changes==1&&adapter.mPlug.p.value==.8&&adapter.mPlug.lastOffset==16,"Regular parameter contract changed");
 check(adapter.bypassed,"Final bypass parameter was lost");
 check(!changes.queues[2].invalidRead,"Empty parameter queue read a negative point");
 check(adapter.delivered.size()==2&&adapter.delivered[0].offset==0&&adapter.delivered[1].offset==32,"Invalid controller point hid valid edges");
 changes.queues.back().failed=2;adapter.delivered.clear();feedback.events.clear();
 adapter.ProcessParameterChanges(data,feedback);
 check(adapter.delivered.size()==2&&adapter.delivered[1].offset==16,"Invalid final point hid earlier controller edges");
 data.inputParameterChanges=nullptr;adapter.ProcessParameterChanges(data,feedback);
 check(adapter.delivered.size()==2,"Null parameter changes emitted MIDI");
}
