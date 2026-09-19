// SPDX-License-Identifier: MIT
#include "SAWSTAR.h"
#include "plugin/EngineControls.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "gui/Controls/Keyboard.h"
#include "plugin/Parameters.h"
#include "plugin/State.h"
#include "gui/Controls/PageButton.h"
#include "gui/Controls/PerformanceWheel.h"
#include "gui/Controls/PresetControls.h"
#include "gui/Controls/WaveformControl.h"
#include "gui/Controls/NoiseSelector.h"
#include "gui/Layout.h"
#include <algorithm>
#include <chrono>

using namespace iplug;
using namespace igraphics;

SAWSTAR::SAWSTAR(const InstanceInfo& info)
: Plugin(info, MakeConfig(static_cast<int>(sawstar::kParameters.size()), 1)) {
  for (const auto& spec : sawstar::kParameters) {
    auto* param = GetParam(static_cast<int>(spec.id));
    const auto prefix=spec.key.substr(0,spec.key.find('.'));
    const char* group=prefix=="amp"?"Amp Envelope":prefix=="filter"?"Filter":prefix=="output"?"Output":prefix=="performance"?"Performance":prefix=="mixer"||prefix=="noise"?"Mixer":prefix=="sub"?"Sub":prefix=="saw"||prefix=="osc1"?"Oscillator 1":prefix=="osc2"?"Oscillator 2":prefix=="lfo"?"LFO 1":prefix=="lfo2"?"LFO 2":prefix=="chorus"?"Chorus":prefix=="delay"?"Delay":prefix=="reverb"?"Reverb":prefix=="arp"?"Arpeggiator":"Modulation";
    const int id=static_cast<int>(spec.id);
    if(id>=71 && id<=82 && (id-71)%3==0)
      param->InitEnum(spec.name.data(),0,6,"",IParam::kFlagsNone,group,"Off","LFO 1","LFO 2","Mod Wheel","Velocity","Aftertouch");
    else if(id>=71 && id<=82 && (id-71)%3==1)
      param->InitEnum(spec.name.data(),0,5,"",IParam::kFlagsNone,group,"Filter Cutoff","Pitch","Amp Level","Pan","Noise Color");
    else if(id==90)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,group,"Off","On");
    else if(id==83||id==89)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,group,"Off","On");
    else if(id==84)
      param->InitEnum(spec.name.data(),0,5,"",IParam::kFlagsNone,group,"Up","Down","Up/Down","Random","Played");
    else if(id==85)
      param->InitEnum(spec.name.data(),2,8,"",IParam::kFlagsNone,group,"1/4","1/8","1/16","1/32","1/8 Triplet","1/16 Triplet","1/8 Dotted","1/16 Dotted");
    else if(id==87)
      param->InitInt(spec.name.data(),1,1,4,"oct",IParam::kFlagsNone,group);
    else if(spec.id==sawstar::ParameterId::Osc1Wave||spec.id==sawstar::ParameterId::Osc2Wave)
      param->InitEnum(spec.name.data(),0,4,"",IParam::kFlagsNone,group,"Saw","Square","Triangle","Sine");
    else if(spec.id==sawstar::ParameterId::VoiceMode)
      param->InitEnum(spec.name.data(),0,3,"",IParam::kFlagsNone,group,"Poly","Mono","Legato");
    else if(spec.id==sawstar::ParameterId::GlideMode)
      param->InitEnum(spec.name.data(),1,2,"",IParam::kFlagsNone,group,"Always","Overlap only");
    else if(spec.id==sawstar::ParameterId::ReverbEnabled)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,group,"Off","On");
    else if(spec.id==sawstar::ParameterId::DelayEnabled)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,group,"Off","On");
    else if(spec.id==sawstar::ParameterId::DelayMode)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,group,"Stereo","Ping-pong");
    else if(spec.id==sawstar::ParameterId::DelaySync)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,group,"Free ms","Tempo Sync");
    else if(spec.id==sawstar::ParameterId::DelayDivision)
      param->InitEnum(spec.name.data(),3,7,"",IParam::kFlagsNone,group,"1/16","1/8","1/8 dotted","1/4","1/4 dotted","1/2","1/1");
    else if(spec.id==sawstar::ParameterId::ChorusEnabled)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,group,"Off","On");
    else if(spec.id==sawstar::ParameterId::LfoShape||spec.id==sawstar::ParameterId::Lfo2Shape)
      param->InitEnum(spec.name.data(),0,4,"",IParam::kFlagsNone,group,"Sine","Triangle","Ramp","Square");
    else if(spec.id==sawstar::ParameterId::LfoTarget||spec.id==sawstar::ParameterId::Lfo2Target)
      param->InitEnum(spec.name.data(),0,4,"",IParam::kFlagsNone,group,"Filter Cutoff","Pitch","Amp Level","Pan");
    else if(spec.id==sawstar::ParameterId::LfoSync||spec.id==sawstar::ParameterId::Lfo2Sync)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,group,"Free Hz","Tempo Sync");
    else if(spec.id==sawstar::ParameterId::LfoDivision||spec.id==sawstar::ParameterId::Lfo2Division)
      param->InitEnum(spec.name.data(),2,6,"",IParam::kFlagsNone,group,"1/1","1/2","1/4","1/8","1/16","1/32");
    else if(spec.id==sawstar::ParameterId::LfoRetrigger||spec.id==sawstar::ParameterId::Lfo2Retrigger)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,group,"Free phase","Retrigger first key");
    else if(spec.id==sawstar::ParameterId::SubWave)
      param->InitEnum(spec.name.data(),0,3,"",IParam::kFlagsNone,group,"Sine","Triangle","Square");
    else if (spec.id == sawstar::ParameterId::FilterMode)
      param->InitEnum(spec.name.data(),0,4,"",IParam::kFlagsNone,group,"Low Pass 12","Low Pass 24","High Pass 12","Band Pass 12");
    else if (spec.id == sawstar::ParameterId::NoiseSource)
      param->InitEnum(spec.name.data(),0,4,"",IParam::kFlagsNone,group,"Legacy White/Dark","White Noise","Dark Noise","Pink Noise");
    else if (spec.id == sawstar::ParameterId::NoiseType)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,group,"White Noise","Dark Noise");
    else if (spec.id == sawstar::ParameterId::Osc1Octave || spec.id == sawstar::ParameterId::Osc2Octave || spec.id == sawstar::ParameterId::SubOctave)
      param->InitInt(spec.name.data(),static_cast<int>(spec.initial),static_cast<int>(spec.minimum),static_cast<int>(spec.maximum),"oct",IParam::kFlagsNone,group);
    else if (spec.mapping == sawstar::Mapping::Logarithmic)
      param->InitDouble(spec.name.data(), spec.initial, spec.minimum, spec.maximum, 0.01,
                        spec.unit.data(), IParam::kFlagsNone, group, IParam::ShapeExp());
    else
      param->InitDouble(spec.name.data(), spec.initial, spec.minimum, spec.maximum, 0.001,
                        spec.unit.data(),IParam::kFlagsNone,group);
  }
  MakeDefaultPreset("Init", 1);
#if IPLUG_EDITOR
  mMakeGraphicsFunc = [this]() {
    if(mGuiScale<=0.f)mGuiScale=GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT);
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS,mGuiScale);
  };
  mLayoutFunc = [this](IGraphics* g) {
    SyncRestoredPreset();
#if IPLUG_DSP
    // The new keyboard has no highlighted keys, regardless of the old editor.
    mDisplayed.fill(false);
#endif
    auto apply = [this,g](const sawstar::Snapshot& values) {
      for (const auto& spec : sawstar::kParameters) {
        const int id=static_cast<int>(spec.id);
        BeginInformHostOfParamChangeFromUI(id);
        const double value=sawstar::Normalize(spec,values[id]);
        SendParameterValueFromUI(id,value);
        EndInformHostOfParamChangeFromUI(id);
        SendParameterValueFromDelegate(id,value,true);
      }
      mArpReset.store(true);
      mFactoryIndex=sawstar::MatchFactoryPreset(values);g->SetAllControlsDirty();
    };
    auto loadFactory=[this,apply](int index){if(index<0||index>=int(sawstar::FactoryPresets().size()))return;apply(sawstar::FactoryPresets()[index].values);mUserPreset.active=false;};
    auto snapshot=[this]{sawstar::Snapshot values{};for(size_t i=0;i<values.size();++i)values[i]=GetParam(int(i))->Value();return values;};
    sawstar::Snapshot current{};
    for(size_t i=0;i<current.size();++i)current[i]=GetParam(static_cast<int>(i))->Value();
    mFactoryIndex=sawstar::MatchFactoryPreset(current);
    sawstar::gui::BuildLayout(g,mGuiScale,mPage,mLfoPage,mFxPage,mFactoryIndex,loadFactory,mMeter,mCpu,mRate,mVoiceCount,mUserPreset,snapshot,apply);
  };
#endif
}
#if IPLUG_EDITOR
void SAWSTAR::SyncRestoredPreset() {
  // Host state callbacks may run away from the GUI thread. Consume only here.
  if(!mStateRestored.exchange(false))return;
  mUserPreset={};
  if(auto* g=GetUI()) {
    if(auto* browser=dynamic_cast<sawstar::gui::PresetBrowser*>(g->GetControlWithTag(9102)))
      browser->SyncToSound();
    g->SetAllControlsDirty();
  }
}
#endif
#if IPLUG_DSP
void SAWSTAR::OnReset() {
  mScope.Reset(GetSampleRate());
  mRate.store(static_cast<int>(GetSampleRate()));mMeter.Reset();mCpu.store(0);mVoiceCount.store(0);
  mSynth.Reset(GetSampleRate());mArp.Init(GetSampleRate());mArpReset.store(false);
  mEvents.Clear();mMidiVoiceMode=0;
  mBend.store(8192);mMod.store(0);
  for (auto& held : mHeld) held.store(false, std::memory_order_relaxed);
}
void SAWSTAR::ProcessBlock(sample**, sample** outputs, int frames) {
  const auto started=std::chrono::steady_clock::now();float peakL=0,peakR=0;
  auto send=[this](int status,int note,int value){mSynth.Midi(status,note,value);};
  const int mode=GetParam(59)->Int();
  if(mArpReset.exchange(false)||mode!=mMidiVoiceMode)mArp.Clear(send);
  mMidiVoiceMode=mode;
  sawstar::ApplyEngineControls(mSynth,mArp,
    [this](sawstar::ParameterId id){return GetParam(static_cast<int>(id))->Value();},
    [this](sawstar::ParameterId id){return GetParam(static_cast<int>(id))->Int();},
    GetTempo(),GetTransportIsRunning());
  const int channels=NOutChansConnected();
  mEvents.Process(frames,[&](const sawstar::BlockMidiEvent& msg){
    mArp.Midi(msg.status,msg.data1,msg.data2,send);
  },[&](int i){
    mArp.Process(send);
    const auto value=mSynth.ProcessStereo();
    const auto pre=mSynth.PreFX();mScope.Push((pre.left+pre.right)*.5f);
    peakL=std::max(peakL,std::abs(value.left));peakR=std::max(peakR,std::abs(value.right));
    sawstar::WriteHostOutput(value,outputs,channels,i);
  },[&]{sawstar::RecoverMidiOverflow(mArp,mSynth);});
  mMeter.Publish(peakL,peakR,uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(started.time_since_epoch()).count()));mVoiceCount.store(mSynth.ActiveVoices());
  if(frames>0){float used=100.f*std::chrono::duration<float>(std::chrono::steady_clock::now()-started).count()*GetSampleRate()/frames;mCpu.store(mCpu.load()*.9f+used*.1f);}
  mBend.store(mSynth.PitchBend(0),std::memory_order_relaxed);
  mMod.store(mSynth.ModWheel(0),std::memory_order_relaxed);
  for (int note = 0; note < 128; ++note)
    mHeld[note].store(mSynth.Held(note), std::memory_order_relaxed);
}
void SAWSTAR::ProcessMidiMsg(const IMidiMsg& msg) {
  mEvents.Push({msg.mOffset,msg.mStatus,msg.mData1,msg.mData2});
}
void SAWSTAR::OnIdle() {
#if IPLUG_EDITOR
  SyncRestoredPreset();
  if(GetUI()){if(auto* meter=dynamic_cast<sawstar::gui::Meter*>(GetUI()->GetControlWithTag(9100)))meter->Update(mPage==0);}
  else mMeter.Take();
  if(GetUI())if(auto* c=dynamic_cast<sawstar::gui::ScopeControl*>(GetUI()->GetControlWithTag(9104)))c->Update(mScope,mPage==0);
  if(GetUI()){sawstar::gui::StyleEntry(GetUI());for(int tag:{9103})if(auto* c=GetUI()->GetControlWithTag(tag))c->SetDirty(false);}
  if(GetUI())if(auto* c=dynamic_cast<sawstar::gui::Status*>(GetUI()->GetControlWithTag(9101)))c->Update();
  sawstar::Snapshot current{};
  for(size_t i=0;i<current.size();++i)current[i]=GetParam(static_cast<int>(i))->Value();
  const int match=sawstar::MatchFactoryPreset(current);
  if(match!=mFactoryIndex){mFactoryIndex=match;if(GetUI())GetUI()->SetAllControlsDirty();}
#endif
  // A bounded snapshot also synchronizes wheels when the editor is reopened.
  IMidiMsg wheel;const int bend=mBend.load(std::memory_order_relaxed);
  wheel.mStatus=0xe0;wheel.mData1=bend&127;wheel.mData2=bend>>7;wheel.mOffset=0;
  SendMidiMsgFromDelegate(wheel);
  wheel.mStatus=0xb0;wheel.mData1=1;wheel.mData2=mMod.load(std::memory_order_relaxed);
  SendMidiMsgFromDelegate(wheel);
  // Do not acknowledge note display changes while no editor exists.
#if IPLUG_EDITOR
  if(GetUI())for (int note = 0; note < 128; ++note) {
    const bool held = mHeld[note].load(std::memory_order_relaxed);
    if (held != mDisplayed[note]) {
      IMidiMsg msg;
      if (held) msg.MakeNoteOnMsg(note, 100, 0); else msg.MakeNoteOffMsg(note, 0);
      SendMidiMsgFromDelegate(msg); mDisplayed[note] = held;
    }
  }
#endif
}
#endif

bool SAWSTAR::SerializeState(IByteChunk& chunk) const {
  sawstar::Snapshot values{};
  for (size_t i=0;i<values.size();++i) values[i]=GetParam(static_cast<int>(i))->Value();
  const auto bytes=sawstar::EncodeState(values);
  return chunk.PutBytes(bytes.data(), static_cast<int>(bytes.size())) > 0;
}
int SAWSTAR::UnserializeState(const IByteChunk& chunk, int startPos) {
  if(startPos<0 || startPos>=chunk.Size() || !chunk.GetData()) return -1;
  sawstar::Snapshot values{};
  const auto consumed=sawstar::DecodeState(chunk.GetData()+startPos,
    static_cast<size_t>(chunk.Size()-startPos),values);
  if(!consumed) return -1;
  // Validate the entire payload first, then use framework locking and reset hooks.
  IByteChunk params;
  for(double value:values) if(params.Put(&value)<0) return -1;
  if(UnserializeParams(params,0)<0) return -1;
  mArpReset.store(true);
  mStateRestored.store(true);
  return startPos+static_cast<int>(consumed);
}
