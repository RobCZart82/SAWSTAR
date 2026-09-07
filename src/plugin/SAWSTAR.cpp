// SPDX-License-Identifier: MIT
#include "SAWSTAR.h"
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
    const int id=static_cast<int>(spec.id);
    if(id>=71 && id<=82 && (id-71)%3==0)
      param->InitEnum(spec.name.data(),0,6,"",IParam::kFlagsNone,"Modulation","Off","LFO 1","LFO 2","Mod Wheel","Velocity","Aftertouch");
    else if(id>=71 && id<=82 && (id-71)%3==1)
      param->InitEnum(spec.name.data(),0,5,"",IParam::kFlagsNone,"Modulation","Filter Cutoff","Pitch","Amp Level","Pan","Noise Color");
    else if(id==83||id==89)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,"Arpeggiator","Off","On");
    else if(id==84)
      param->InitEnum(spec.name.data(),0,5,"",IParam::kFlagsNone,"Arpeggiator","Up","Down","Up/Down","Random","Played");
    else if(id==85)
      param->InitEnum(spec.name.data(),2,8,"",IParam::kFlagsNone,"Arpeggiator","1/4","1/8","1/16","1/32","1/8 Triplet","1/16 Triplet","1/8 Dotted","1/16 Dotted");
    else if(id==87)
      param->InitInt(spec.name.data(),1,1,4,"oct");
    else if(spec.id==sawstar::ParameterId::Osc1Wave||spec.id==sawstar::ParameterId::Osc2Wave)
      param->InitEnum(spec.name.data(),0,4,"",IParam::kFlagsNone,"Oscillator","Saw","Square","Triangle","Sine");
    else if(spec.id==sawstar::ParameterId::VoiceMode)
      param->InitEnum(spec.name.data(),0,3,"",IParam::kFlagsNone,"Performance","Poly","Mono","Legato");
    else if(spec.id==sawstar::ParameterId::GlideMode)
      param->InitEnum(spec.name.data(),1,2,"",IParam::kFlagsNone,"Performance","Always","Overlap only");
    else if(spec.id==sawstar::ParameterId::ReverbEnabled)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,"Reverb","Off","On");
    else if(spec.id==sawstar::ParameterId::DelayEnabled)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,"Delay","Off","On");
    else if(spec.id==sawstar::ParameterId::DelayMode)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,"Delay","Stereo","Ping-pong");
    else if(spec.id==sawstar::ParameterId::DelaySync)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,"Delay","Free ms","Tempo Sync");
    else if(spec.id==sawstar::ParameterId::DelayDivision)
      param->InitEnum(spec.name.data(),3,7,"",IParam::kFlagsNone,"Delay","1/16","1/8","1/8 dotted","1/4","1/4 dotted","1/2","1/1");
    else if(spec.id==sawstar::ParameterId::ChorusEnabled)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,"Effects","Off","On");
    else if(spec.id==sawstar::ParameterId::LfoShape||spec.id==sawstar::ParameterId::Lfo2Shape)
      param->InitEnum(spec.name.data(),0,4,"",IParam::kFlagsNone,"LFO","Sine","Triangle","Ramp","Square");
    else if(spec.id==sawstar::ParameterId::LfoTarget||spec.id==sawstar::ParameterId::Lfo2Target)
      param->InitEnum(spec.name.data(),0,4,"",IParam::kFlagsNone,"LFO","Filter Cutoff","Pitch","Amp Level","Pan");
    else if(spec.id==sawstar::ParameterId::LfoSync||spec.id==sawstar::ParameterId::Lfo2Sync)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,"LFO","Free Hz","Tempo Sync");
    else if(spec.id==sawstar::ParameterId::LfoDivision||spec.id==sawstar::ParameterId::Lfo2Division)
      param->InitEnum(spec.name.data(),2,6,"",IParam::kFlagsNone,"LFO","1/1","1/2","1/4","1/8","1/16","1/32");
    else if(spec.id==sawstar::ParameterId::LfoRetrigger||spec.id==sawstar::ParameterId::Lfo2Retrigger)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,"LFO","Free phase","Retrigger first key");
    else if (spec.id == sawstar::ParameterId::FilterMode)
      param->InitEnum(spec.name.data(),0,4,"",IParam::kFlagsNone,"Filter","Low Pass 12","Low Pass 24","High Pass 12","Band Pass 12");
    else if (spec.id == sawstar::ParameterId::NoiseSource)
      param->InitEnum(spec.name.data(),0,4,"",IParam::kFlagsNone,"Mixer","Legacy White/Dark","White Noise","Dark Noise","Pink Noise");
    else if (spec.id == sawstar::ParameterId::NoiseType)
      param->InitEnum(spec.name.data(),0,2,"",IParam::kFlagsNone,"Mixer","White Noise","Dark Noise");
    else if (spec.id == sawstar::ParameterId::Osc1Octave || spec.id == sawstar::ParameterId::Osc2Octave || spec.id == sawstar::ParameterId::SubOctave)
      param->InitInt(spec.name.data(),static_cast<int>(spec.initial),static_cast<int>(spec.minimum),static_cast<int>(spec.maximum),"oct");
    else if (spec.mapping == sawstar::Mapping::Logarithmic)
      param->InitDouble(spec.name.data(), spec.initial, spec.minimum, spec.maximum, 0.01,
                        spec.unit.data(), IParam::kFlagsNone, static_cast<int>(spec.id) >= 8 ? "Filter" : "Amp", IParam::ShapeExp());
    else
      param->InitDouble(spec.name.data(), spec.initial, spec.minimum, spec.maximum, 0.001,
                        spec.unit.data());
  }
  MakeDefaultPreset("Init", 1);
#if IPLUG_EDITOR
  mMakeGraphicsFunc = [this]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS,
                        GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  mLayoutFunc = [this](IGraphics* g) {
    auto loadFactory = [this,g](int index) {
      if(index<0 || index>=static_cast<int>(sawstar::FactoryPresets().size()))return;
      const auto& values=sawstar::FactoryPresets()[index].values;
      for (const auto& spec : sawstar::kParameters) {
        const int id=static_cast<int>(spec.id);
        BeginInformHostOfParamChangeFromUI(id);
        const double value=sawstar::Normalize(spec,values[id]);
        SendParameterValueFromUI(id,value);
        EndInformHostOfParamChangeFromUI(id);
        SendParameterValueFromDelegate(id,value,true);
      }
      mArpReset.store(true);
      mFactoryIndex=index;g->SetAllControlsDirty();
    };
    sawstar::Snapshot current{};
    for(size_t i=0;i<current.size();++i)current[i]=GetParam(static_cast<int>(i))->Value();
    mFactoryIndex=sawstar::MatchFactoryPreset(current);
    sawstar::gui::BuildLayout(g,mPage,mLfoPage,mFxPage,mFactoryIndex,loadFactory,mPeakL,mPeakR,mCpu,mRate,mVoiceCount);
  };
#endif
}
#if IPLUG_DSP
void SAWSTAR::OnReset() {
  mRate.store(static_cast<int>(GetSampleRate()));mPeakL.store(0);mPeakR.store(0);mCpu.store(0);mVoiceCount.store(0);
  mSynth.Reset(GetSampleRate());mArp.Init(GetSampleRate());mArpReset.store(false);
  mEventCount = 0; mOverflow = false;
  mBend.store(8192);mMod.store(0);
  for (auto& held : mHeld) held.store(false, std::memory_order_relaxed);
}
void SAWSTAR::ProcessBlock(sample**, sample** outputs, int frames) {
  const auto started=std::chrono::steady_clock::now();float peakL=0,peakR=0;
  auto send=[this](int status,int note,int value){mSynth.Midi(status,note,value);};
  if(mArpReset.exchange(false))mArp.Clear(send);
  mArp.Set(GetParam(83)->Int()!=0,GetParam(84)->Int(),GetParam(85)->Int(),GetParam(86)->Value(),GetParam(87)->Int(),GetParam(88)->Value(),GetParam(89)->Int()!=0,GetTempo(),GetTransportIsRunning(),send);
  mSynth.SetLfo2(GetParam(64)->Value(),GetParam(65)->Value(),GetParam(66)->Int(),GetParam(67)->Int(),GetParam(68)->Int()!=0,GetParam(69)->Int(),GetTempo(),GetParam(70)->Int()!=0);
  for(int row=0;row<4;++row){const int id=71+row*3;mSynth.SetModulation(row,GetParam(id)->Int(),GetParam(id+1)->Int(),GetParam(id+2)->Value());}
  mSynth.SetVoiceMode(GetParam(59)->Int(),GetParam(60)->Value(),GetParam(61)->Int()!=0);
  mSynth.SetParameters(GetParam(0)->Value(), GetParam(1)->Value(), GetParam(2)->Value(),
                       GetParam(3)->Value(), GetParam(4)->Value());
  mSynth.SetMixer(GetParam(20)->Value(),GetParam(21)->Value(),GetParam(22)->Value(),GetParam(23)->Value(),
    GetParam(24)->Int(),GetParam(25)->Int(),GetParam(62)->Int()==0?GetParam(26)->Int():GetParam(62)->Int()-1,GetParam(30)->Int());
  mSynth.SetNoiseColor(GetParam(63)->Value());
  mSynth.SetOsc2(GetParam(27)->Value(),GetParam(28)->Value(),GetParam(29)->Value());
  mSynth.SetOutputBoost(static_cast<float>(GetParam(19)->Value()));
  mSynth.SetSaw(static_cast<float>(GetParam(5)->Value()), static_cast<float>(GetParam(6)->Value()),
                static_cast<float>(GetParam(7)->Value()));
  mSynth.SetReverb(GetParam(54)->Int()!=0,GetParam(55)->Value(),GetParam(56)->Value(),GetParam(57)->Value(),GetParam(58)->Value());
  mSynth.SetDelay(GetParam(46)->Int()!=0,GetParam(47)->Value(),GetParam(48)->Value(),GetParam(49)->Value(),GetParam(50)->Value(),GetParam(51)->Int()!=0,GetParam(52)->Int()!=0,GetParam(53)->Int(),GetTempo());
  mSynth.SetChorus(GetParam(42)->Int()!=0,GetParam(43)->Value(),GetParam(44)->Value(),GetParam(45)->Value());
  mSynth.SetWaveforms(GetParam(33)->Int(),GetParam(34)->Int());
  mSynth.SetLfo(GetParam(35)->Value(),GetParam(36)->Value(),GetParam(37)->Int(),GetParam(38)->Int(),
    GetParam(39)->Int()!=0,GetParam(40)->Int(),GetTempo(),GetParam(41)->Int()!=0);
  mSynth.SetFilterCharacter(static_cast<float>(GetParam(31)->Value()),GetParam(32)->Int());
  mSynth.SetFilter(static_cast<float>(GetParam(8)->Value()), static_cast<float>(GetParam(9)->Value()),
                   static_cast<float>(GetParam(10)->Value()));
  mSynth.SetFilterEnvelope(static_cast<float>(GetParam(11)->Value()), static_cast<float>(GetParam(12)->Value()),
    static_cast<float>(GetParam(13)->Value()), static_cast<float>(GetParam(14)->Value()),
    static_cast<float>(GetParam(15)->Value()), static_cast<float>(GetParam(16)->Value()));
  mSynth.SetPerformance(static_cast<float>(GetParam(17)->Value()),static_cast<float>(GetParam(18)->Value()));
  if (mOverflow) {
    mArp.Clear(send);
    for (int ch = 0; ch < 16; ++ch) mSynth.Midi(0xB0 | ch, 120, 0);
    mEventCount = 0; mOverflow = false;
  mBend.store(8192);mMod.store(0);
  }
  int event = 0;
  for (int i = 0; i < frames; ++i) {
    while (event < mEventCount && mEvents[event].mOffset <= i) {
      const auto& msg = mEvents[event++];
      mArp.Midi(msg.mStatus, msg.mData1, msg.mData2,send);
    }
    mArp.Process(send);
    const auto value=mSynth.ProcessStereo();
    peakL=std::max(peakL,std::abs(value.left));peakR=std::max(peakR,std::abs(value.right));
    const int channels=NOutChansConnected();
    if(channels==1) outputs[0][i]=static_cast<sample>((value.left+value.right)*0.5f);
    else for(int ch=0;ch<channels;++ch) outputs[ch][i]=static_cast<sample>(ch%2?value.right:value.left);
  }
  mPeakL.store(peakL);mPeakR.store(peakR);mVoiceCount.store(mSynth.ActiveVoices());
  if(frames>0){float used=100.f*std::chrono::duration<float>(std::chrono::steady_clock::now()-started).count()*GetSampleRate()/frames;mCpu.store(mCpu.load()*.9f+used*.1f);}
  for (int i = event; i < mEventCount; ++i) {
    mEvents[i - event] = mEvents[i]; mEvents[i - event].mOffset -= frames;
  }
  mEventCount -= event;
  mBend.store(mSynth.PitchBend(0),std::memory_order_relaxed);
  mMod.store(mSynth.ModWheel(0),std::memory_order_relaxed);
  for (int note = 0; note < 128; ++note)
    mHeld[note].store(mSynth.Held(note), std::memory_order_relaxed);
}
void SAWSTAR::ProcessMidiMsg(const IMidiMsg& msg) {
  if (mEventCount == static_cast<int>(mEvents.size())) { mOverflow = true; return; }
  int pos = mEventCount++;
  while (pos > 0 && mEvents[pos - 1].mOffset > msg.mOffset) {
    mEvents[pos] = mEvents[pos - 1]; --pos;
  }
  mEvents[pos] = msg;
}
void SAWSTAR::OnIdle() {
#if IPLUG_EDITOR
  if(GetUI()){for(int tag:{9100,9101})if(auto* c=GetUI()->GetControlWithTag(tag))c->SetDirty(false);}
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
  for (int note = 0; note < 128; ++note) {
    const bool held = mHeld[note].load(std::memory_order_relaxed);
    if (held != mDisplayed[note]) {
      IMidiMsg msg;
      if (held) msg.MakeNoteOnMsg(note, 100, 0); else msg.MakeNoteOffMsg(note, 0);
      SendMidiMsgFromDelegate(msg); mDisplayed[note] = held;
    }
  }
}
#endif

bool SAWSTAR::SerializeState(IByteChunk& chunk) const {
  sawstar::Snapshot values{};
  for (size_t i=0;i<values.size();++i) values[i]=GetParam(static_cast<int>(i))->Value();
  const auto bytes=sawstar::EncodeState(values);
  return chunk.PutBytes(bytes.data(), static_cast<int>(bytes.size())) > 0;
}
int SAWSTAR::UnserializeState(const IByteChunk& chunk, int startPos) {
  if(startPos<0 || startPos>chunk.Size()) return -1;
  sawstar::Snapshot values{};
  const auto consumed=sawstar::DecodeState(chunk.GetData()+startPos,
    static_cast<size_t>(chunk.Size()-startPos),values);
  if(!consumed) return -1;
  // Validate the entire payload first, then use framework locking and reset hooks.
  IByteChunk params;
  for(double value:values) if(params.Put(&value)<0) return -1;
  if(UnserializeParams(params,0)<0) return -1;
  mArpReset.store(true);
  return startPos+static_cast<int>(consumed);
}
