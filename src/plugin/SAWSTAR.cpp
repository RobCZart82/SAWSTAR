// SPDX-License-Identifier: MIT
#include "SAWSTAR.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "IVKeyboardControl.h"
#include "plugin/Parameters.h"
#include "plugin/State.h"
#include "gui/Controls/PageButton.h"
#include "gui/Controls/PerformanceWheel.h"
#include "gui/Controls/PresetControls.h"
#include <algorithm>

using namespace iplug;
using namespace igraphics;

SAWSTAR::SAWSTAR(const InstanceInfo& info)
: Plugin(info, MakeConfig(static_cast<int>(sawstar::kParameters.size()), 1)) {
  for (const auto& spec : sawstar::kParameters) {
    auto* param = GetParam(static_cast<int>(spec.id));
    if (spec.mapping == sawstar::Mapping::Logarithmic)
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
    auto selectPage = [this, g](int page) {
      mPage = page;
      for (int i = 0; i < 3; ++i)
        g->ForControlInGroup(groups[i], [i, page](IControl* c) { c->Hide(i != page); });
      g->SetAllControlsDirty();
    };
    for (int i = 0; i < 3; ++i)
      g->AttachControl(new sawstar::gui::PageButton(IRECT(382.f+i*102.f, 25, 479.f+i*102.f, 70),
                         titles[i], i, mPage, [selectPage, i]() { selectPage(i); }));
    g->AttachControl(new sawstar::gui::PresetSelector(IRECT(701,25,1004,70),mFactoryIndex,loadFactory));
    g->AttachControl(new ITextControl(IRECT(28, 105, 996, 139),
      "7-SAW  >  LOW-PASS  >  AMP ENVELOPE  >  OUTPUT", IText(22, accent)), kNoTag, "main");
    g->AttachControl(new ITextControl(IRECT(28, 140, 996, 169),
      "Filter envelope: raise Filter Mix, lower Cutoff, then add Env Amount.", IText(16, light)), kNoTag, "main");
    const auto knobStyle=DEFAULT_STYLE.WithLabelText(IText(13, light))
      .WithValueText(IText(12, light).WithVAlign(EVAlign::Bottom));
    for(int i=0;i<6;++i)
      g->AttachControl(new IVKnobControl(IRECT(28.f+i*165, 172, 168.f+i*165, 250),
        5+i, sawstar::kParameters[5+i].name.data(), knobStyle, true), kNoTag, "main");
    for(int i=0;i<6;++i)
      g->AttachControl(new IVKnobControl(IRECT(28.f+i*165, 262, 168.f+i*165, 340),
        11+i, sawstar::kParameters[11+i].name.data(), knobStyle, true), kNoTag, "main");
    const int order[] = {1, 2, 3, 4, 0};
    for (int i = 0; i < 5; ++i) {
      const auto& spec = sawstar::kParameters[order[i]];
      g->AttachControl(new IVKnobControl(IRECT(40.f+i*195.f, 352, 204.f+i*195.f, 430),
                        order[i], spec.name.data(), knobStyle, true), kNoTag, "main");
    }
    g->AttachControl(new ITextControl(IRECT(35, 180, 989, 230),
      "PERFORMANCE  /  ARPEGGIATOR  /  MODULATION", IText(23, accent)), kNoTag, "advanced");
    g->AttachControl(new IVKnobControl(IRECT(290,245,460,350),17,"Bend Range",knobStyle,true),kNoTag,"advanced");
    g->AttachControl(new IVKnobControl(IRECT(560,245,730,350),18,"Mod > Cutoff",knobStyle,true),kNoTag,"advanced");
    g->AttachControl(new ITextControl(IRECT(35,365,989,398),
      "MOD opens the filter. Raise Filter Mix to hear it. PITCH returns to center.",IText(16,light)),kNoTag,"advanced");
    g->AttachControl(new ITextControl(IRECT(35,403,989,427),
      "Arpeggiator and flexible modulation routing are planned.",IText(13,light)),kNoTag,"advanced");
    g->AttachControl(new ITextControl(IRECT(35,100,989,133),
      "FACTORY LIBRARY + LEARNING",IText(22,accent)),kNoTag,"presets");
    for(int i=0;i<static_cast<int>(sawstar::FactoryPresets().size());++i)
      g->AttachControl(new sawstar::gui::PresetRow(IRECT(24,145.f+i*44,1000,185.f+i*44),
        i,mFactoryIndex,loadFactory),kNoTag,"presets");
    g->AttachControl(new ITextControl(IRECT(24,412,1000,435),
      "Click a sound to load it. Edits show as Custom; save your sound in the DAW project.",IText(13,light)),kNoTag,"presets");
    g->AttachControl(new ITextControl(IRECT(20, 523, 1004, 553),
      "0.1.0-dev  |  7-SAW  |  16 VOICES  |  WHEELS: MIDI CH 1", IText(14, light)));
    g->AttachControl(new sawstar::gui::PerformanceWheel(IRECT(20,450,54,497),true));
    g->AttachControl(new sawstar::gui::PerformanceWheel(IRECT(62,450,96,497),false));
    g->AttachControl(new ITextControl(IRECT(17,497,57,514),"PITCH",IText(10,light)));
    g->AttachControl(new ITextControl(IRECT(59,497,99,514),"MOD",IText(10,light)));
    g->AttachControl(new IVKeyboardControl(IRECT(104, 450, 1004, 514), 36, 96, false,
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
  mSynth.SetParameters(GetParam(0)->Value(), GetParam(1)->Value(), GetParam(2)->Value(),
                       GetParam(3)->Value(), GetParam(4)->Value());
  mSynth.SetSaw(static_cast<float>(GetParam(5)->Value()), static_cast<float>(GetParam(6)->Value()),
                static_cast<float>(GetParam(7)->Value()));
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
