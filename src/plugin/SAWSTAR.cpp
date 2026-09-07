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
#include <algorithm>

using namespace iplug;
using namespace igraphics;

SAWSTAR::SAWSTAR(const InstanceInfo& info)
: Plugin(info, MakeConfig(static_cast<int>(sawstar::kParameters.size()), 1)) {
  for (const auto& spec : sawstar::kParameters) {
    auto* param = GetParam(static_cast<int>(spec.id));
    const int id=static_cast<int>(spec.id);
    if(id>=71 && (id-71)%3==0)
      param->InitEnum(spec.name.data(),0,6,"",IParam::kFlagsNone,"Modulation","Off","LFO 1","LFO 2","Mod Wheel","Velocity","Aftertouch");
    else if(id>=71 && (id-71)%3==1)
      param->InitEnum(spec.name.data(),0,5,"",IParam::kFlagsNone,"Modulation","Filter Cutoff","Pitch","Amp Level","Pan","Noise Color");
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
    g->AttachPanelBackground(IColor(255, 9, 26, 38));
    g->EnableMouseOver(true);
    g->AttachPopupMenuControl();
    g->AttachTextEntryControl();
    // Use an installed system font; no external font asset is redistributed.
    g->LoadFont("Roboto-Regular", "Arial", ETextStyle::Normal);
    const IColor light(255, 210, 238, 246), accent(255, 61, 200, 239);
    g->AttachControl(new ITextControl(IRECT(20, 22, 233, 65), "S A W S T A R", IText(24, light)));
    g->AttachControl(new ITextControl(IRECT(246, 24, 370, 44), "Simple Synth", IText(13, light).WithAlign(EAlign::Near)));
    g->AttachControl(new ITextControl(IRECT(246, 44, 370, 65), "Big Sound", IText(15, accent).WithAlign(EAlign::Near)));
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
      mFactoryIndex=index;g->SetAllControlsDirty();
    };
    sawstar::Snapshot current{};
    for(size_t i=0;i<current.size();++i)current[i]=GetParam(static_cast<int>(i))->Value();
    mFactoryIndex=sawstar::MatchFactoryPreset(current);
    static const char* titles[] = {"MAIN", "ADVANCED", "PRESETS"};
    static const char* groups[] = {"main", "advanced", "presets"};
    static const char* fxGroups[] = {"fx-chorus", "fx-delay", "fx-reverb", "modulation"};
    auto selectPage = [this, g](int page) {
      mPage = page;
      for (int i = 0; i < 3; ++i)
        g->ForControlInGroup(groups[i], [i, page](IControl* c) { c->Hide(i != page); });
      for(int i=0;i<4;++i)g->ForControlInGroup(fxGroups[i],[this,i,page](IControl* c){c->Hide(page!=1||mFxPage!=i);});
      g->ForControlInGroup("lfo1",[this,page](IControl* c){c->Hide(page!=1||mLfoPage!=0);});
      g->ForControlInGroup("lfo2",[this,page](IControl* c){c->Hide(page!=1||mLfoPage!=1);});
      g->SetAllControlsDirty();
    };
    for (int i = 0; i < 3; ++i)
      g->AttachControl(new sawstar::gui::PageButton(IRECT(382.f+i*102.f, 25, 479.f+i*102.f, 70),
                         titles[i], i, mPage, [selectPage, i]() { selectPage(i); }));
    g->AttachControl(new sawstar::gui::PresetSelector(IRECT(701,25,1004,70),mFactoryIndex,loadFactory));
    g->AttachControl(new sawstar::gui::WaveformControl(IRECT(20,86,245,166),33,"OSC1 - click to select"),kNoTag,"main");
    g->AttachControl(new sawstar::gui::WaveformControl(IRECT(260,86,485,166),34,"OSC2 - click to select"),kNoTag,"main");

    const auto knobStyle=DEFAULT_STYLE.WithLabelText(IText(13, light))
      .WithValueText(IText(12, light).WithVAlign(EVAlign::Bottom));
    const auto menuStyle=knobStyle.WithValueText(IText(13,IColor(255,9,26,38)));
    g->AttachControl(new IVMenuButtonControl(IRECT(510,108,725,140),32,"",menuStyle),kNoTag,"main");
    g->AttachControl(new IVSliderControl(IRECT(750,108,995,140),31,"Drive",knobStyle,true,EDirection::Horizontal),kNoTag,"main");
    g->AttachControl(new ITextControl(IRECT(510,140,995,166),"Raise Filter Mix to hear",IText(12,light)),kNoTag,"main");
    g->AttachControl(new ITextControl(IRECT(20,170,248,192),"MIXER",IText(15,accent)),kNoTag,"main");
    for(int i=0;i<4;++i)
      g->AttachControl(new IVSliderControl(IRECT(20.f+i*58,197,74.f+i*58,344),20+i,
        sawstar::kParameters[20+i].name.data(),knobStyle,true),kNoTag,"main");
    g->AttachControl(new IVSliderControl(IRECT(25,347,240,390),63,"NOISE COLOR",knobStyle, true, EDirection::Horizontal),kNoTag,"main");
    g->AttachControl(new sawstar::gui::NoiseSelector(IRECT(25,395,240,432)),kNoTag,"main");
    const int order[]={30,5,6,7,24,27,28,29,25,8,9,10,11,12,13,14,15,16,1,2,3,4,19,0};
    for(int i=0;i<24;++i) {
      const int id=order[i];const float x=260.f+(i%6)*124.f,y=170.f+(i/6)*66.f;
      const char* label=id==5?"OSC1 Detune":id==6?"OSC1 Unison":id==7?"OSC1 Width":sawstar::kParameters[id].name.data();
      g->AttachControl(new IVKnobControl(IRECT(x,y,x+118,y+64),id,label,knobStyle,true),kNoTag,"main");
    }
    for(int bank=0;bank<2;++bank){
      g->AttachControl(new sawstar::gui::PageButton(IRECT(25.f+bank*235,92,245.f+bank*235,121),bank?"LFO 2":"LFO 1",bank,mLfoPage,[this,selectPage,bank](){mLfoPage=bank;selectPage(mPage);}),kNoTag,"advanced");
      const char* group=bank?"lfo2":"lfo1";
      const int base=bank?64:35;
      const int menus[]={base+2,base+3,base+4,base+5};
      for(int i=0;i<4;++i){float x=25.f+(i%2)*235,y=125.f+(i/2)*48;
        g->AttachControl(new IVMenuButtonControl(IRECT(x,y,x+220,y+44),menus[i],sawstar::kParameters[menus[i]].name.data(),menuStyle),kNoTag,group);}
      for(int i=0;i<2;++i){float x=25.f+i*235;
        g->AttachControl(new IVKnobControl(IRECT(x,223,x+220,279),base+i,i?"Amount":"Rate",knobStyle,true),kNoTag,group);}
      g->AttachControl(new IVMenuButtonControl(IRECT(25,345,245,381),base+6,"LFO Phase",menuStyle),kNoTag,group);
    }
    for(int i=0;i<2;++i){float x=25.f+i*235;
      g->AttachControl(new IVKnobControl(IRECT(x,282,x+220,338),17+i,sawstar::kParameters[17+i].name.data(),knobStyle,true),kNoTag,"advanced");}
    g->AttachControl(new IVMenuButtonControl(IRECT(260,345,480,381),59,"",menuStyle),kNoTag,"advanced");
    g->AttachControl(new IVSliderControl(IRECT(25,385,245,437),60,"Glide (Mono/Legato)",knobStyle,true,EDirection::Horizontal),kNoTag,"advanced");
    g->AttachControl(new IVMenuButtonControl(IRECT(260,395,480,435),61,"",menuStyle),kNoTag,"advanced");
    const char* fxTitles[]={"CHORUS","DELAY","REVERB","MODULATION"};
    for(int i=0;i<4;++i)g->AttachControl(new sawstar::gui::PageButton(IRECT(520.f+i*120,92,636.f+i*120,127),fxTitles[i],i,mFxPage,[this,selectPage,i](){mFxPage=i;selectPage(mPage);}),kNoTag,"advanced");
    g->AttachControl(new IVMenuButtonControl(IRECT(520,150,665,195),42,"",menuStyle),kNoTag,"fx-chorus");
    g->AttachControl(new IVSliderControl(IRECT(685,145,995,205),43,"Chorus Mix",knobStyle,true,EDirection::Horizontal),kNoTag,"fx-chorus");
    for(int i=0;i<2;++i)g->AttachControl(new IVSliderControl(IRECT(520.f+i*245,225,750.f+i*245,285),44+i,sawstar::kParameters[44+i].name.data(),knobStyle,true,EDirection::Horizontal),kNoTag,"fx-chorus");
    g->AttachControl(new ITextControl(IRECT(520,310,995,345),"Gentle movement: Mix 25%, Rate 0.3 Hz, Depth 35%",IText(13,light)),kNoTag,"fx-chorus");
    const int delayMenus[]={46,51,52,53};
    for(int i=0;i<4;++i)g->AttachControl(new IVMenuButtonControl(IRECT(520.f+i*121,150,634.f+i*121,195),delayMenus[i],"",menuStyle),kNoTag,"fx-delay");
    for(int i=0;i<4;++i){float x=520.f+(i%2)*245,y=215.f+(i/2)*80;
      g->AttachControl(new IVSliderControl(IRECT(x,y,x+230,y+60),47+i,sawstar::kParameters[47+i].name.data(),knobStyle,true,EDirection::Horizontal),kNoTag,"fx-delay");}
    g->AttachControl(new ITextControl(IRECT(520,380,995,420),"Tempo Sync replaces Time (maximum 2 seconds).",IText(13,light)),kNoTag,"fx-delay");
    g->AttachControl(new IVMenuButtonControl(IRECT(520,150,665,195),54,"",menuStyle),kNoTag,"fx-reverb");
    for(int i=0;i<4;++i){float x=520.f+(i%2)*245,y=215.f+(i/2)*80;
      g->AttachControl(new IVSliderControl(IRECT(x,y,x+230,y+60),55+i,sawstar::kParameters[55+i].name.data(),knobStyle,true,EDirection::Horizontal),kNoTag,"fx-reverb");}
    g->AttachControl(new ITextControl(IRECT(520,380,995,420),"Lower Damping Hz makes the reverb darker.",IText(13,light)),kNoTag,"fx-reverb");
    g->AttachControl(new ITextControl(IRECT(520,135,995,158),"SOURCE                 DESTINATION                 AMOUNT",IText(12,light)),kNoTag,"modulation");
    for(int row=0;row<4;++row){const int id=71+row*3;const float y=165.f+row*62;
      g->AttachControl(new IVMenuButtonControl(IRECT(520,y,670,y+52),id,"",menuStyle),kNoTag,"modulation");
      g->AttachControl(new IVMenuButtonControl(IRECT(679,y,864,y+52),id+1,"",menuStyle),kNoTag,"modulation");
      g->AttachControl(new IVKnobControl(IRECT(880,y,995,y+56),id+2,"",knobStyle,true),kNoTag,"modulation");}
    g->AttachControl(new ITextControl(IRECT(520,415,995,438),"Off disables a row. Negative amounts invert its effect.",IText(12,light)),kNoTag,"modulation");
    g->AttachControl(new ITextControl(IRECT(35,100,989,133),
      "FACTORY LIBRARY + LEARNING",IText(22,accent)),kNoTag,"presets");
    for(int i=0;i<static_cast<int>(sawstar::FactoryPresets().size());++i)
      g->AttachControl(new sawstar::gui::PresetRow(IRECT(24,140.f+i*33,1000,170.f+i*33),
        i,mFactoryIndex,loadFactory),kNoTag,"presets");
    g->AttachControl(new ITextControl(IRECT(24,412,1000,435),
      "Click a sound to load it. Edits show as Custom; save your sound in the DAW project.",IText(13,light)),kNoTag,"presets");
    g->AttachControl(new ITextControl(IRECT(20, 523, 1004, 553),
      "0.1.0-dev  |  7-SAW  |  16 VOICES  |  WHEELS: MIDI CH 1", IText(14, light)));
    g->AttachControl(new sawstar::gui::PerformanceWheel(IRECT(20,450,54,497),true));
    g->AttachControl(new sawstar::gui::PerformanceWheel(IRECT(62,450,96,497),false));
    g->AttachControl(new ITextControl(IRECT(17,497,57,514),"PITCH",IText(10,light)));
    g->AttachControl(new ITextControl(IRECT(59,497,99,514),"MOD",IText(10,light)));
    g->AttachControl(new sawstar::gui::Keyboard(IRECT(104, 450, 1004, 514), 36, 96, false,
      IColor(255, 117, 137, 147), IColor(255, 22, 40, 50), accent,
      IColor(255, 9, 26, 38), light));
    selectPage(mPage);
  };
#endif
}
#if IPLUG_DSP
void SAWSTAR::OnReset() {
  mSynth.Reset(GetSampleRate());
  mEventCount = 0; mOverflow = false;
  mBend.store(8192);mMod.store(0);
  for (auto& held : mHeld) held.store(false, std::memory_order_relaxed);
}
void SAWSTAR::ProcessBlock(sample**, sample** outputs, int frames) {
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
    for (int ch = 0; ch < 16; ++ch) mSynth.Midi(0xB0 | ch, 120, 0);
    mEventCount = 0; mOverflow = false;
  mBend.store(8192);mMod.store(0);
  }
  int event = 0;
  for (int i = 0; i < frames; ++i) {
    while (event < mEventCount && mEvents[event].mOffset <= i) {
      const auto& msg = mEvents[event++];
      mSynth.Midi(msg.mStatus, msg.mData1, msg.mData2);
    }
    const auto value=mSynth.ProcessStereo();
    const int channels=NOutChansConnected();
    if(channels==1) outputs[0][i]=static_cast<sample>((value.left+value.right)*0.5f);
    else for(int ch=0;ch<channels;++ch) outputs[ch][i]=static_cast<sample>(ch%2?value.right:value.left);
  }
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
  return startPos+static_cast<int>(consumed);
}
