// SPDX-License-Identifier: MIT
#include "SAWSTAR.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "plugin/Parameters.h"
#include "gui/Controls/PageButton.h"
#include <algorithm>

using namespace iplug;
using namespace igraphics;

SAWSTAR::SAWSTAR(const InstanceInfo& info)
: Plugin(info, MakeConfig(static_cast<int>(sawstar::kParameters.size()), 1)) {
  for (const auto& spec : sawstar::kParameters) {
    auto* param = GetParam(static_cast<int>(spec.id));
    if (spec.mapping == sawstar::Mapping::Logarithmic)
      param->InitDouble(spec.name.data(), spec.initial, spec.minimum, spec.maximum, 0.01,
                        spec.unit.data(), IParam::kFlagsNone, "Amp", IParam::ShapeExp());
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
    g->AttachControl(new ITextControl(IRECT(24, 15, 280, 65), "S A W S T A R", IText(27, light)));
    g->AttachControl(new ITextControl(IRECT(24, 64, 360, 92), "SIMPLE SYNTH. REAL SOUNDS.", IText(13, accent)));
    static const char* titles[] = {"MAIN", "ADVANCED", "PRESETS"};
    static const char* groups[] = {"main", "advanced", "presets"};
    auto selectPage = [this, g](int page) {
      mPage = page;
      for (int i = 0; i < 3; ++i)
        g->ForControlInGroup(groups[i], [i, page](IControl* c) { c->Hide(i != page); });
      g->SetAllControlsDirty();
    };
    for (int i = 0; i < 3; ++i)
      g->AttachControl(new sawstar::gui::PageButton(IRECT(550.f+i*148.f, 25, 690.f+i*148.f, 70),
                         titles[i], i, mPage, [selectPage, i]() { selectPage(i); }));
    g->AttachControl(new ITextControl(IRECT(28, 128, 996, 168),
      "SAW  >  AMP ENVELOPE  >  OUTPUT", IText(22, accent)), kNoTag, "main");
    g->AttachControl(new ITextControl(IRECT(28, 177, 996, 213),
      "Shape the starting patch. The sound engine is the next development step.", IText(16, light)), kNoTag, "main");
    const int order[] = {1, 2, 3, 4, 0};
    for (int i = 0; i < 5; ++i) {
      const auto& spec = sawstar::kParameters[order[i]];
      g->AttachControl(new IVKnobControl(IRECT(40.f+i*195.f, 245, 204.f+i*195.f, 410),
                        order[i], spec.name.data()), kNoTag, "main");
    }
    g->AttachControl(new ITextControl(IRECT(35, 180, 989, 230),
      "PERFORMANCE  /  ARPEGGIATOR  /  MODULATION", IText(23, accent)), kNoTag, "advanced");
    g->AttachControl(new ITextControl(IRECT(35, 255, 989, 310),
      "These controls will arrive after the first playable voice engine.", IText(18, light)), kNoTag, "advanced");
    g->AttachControl(new ITextControl(IRECT(35, 170, 989, 220),
      "LIBRARY + LEARNING", IText(23, accent)), kNoTag, "presets");
    g->AttachControl(new ITextControl(IRECT(35, 240, 989, 290),
      "Start from Init. An annotated sound library is planned for a later milestone.", IText(17, light)), kNoTag, "presets");
    g->AttachControl(new IVButtonControl(IRECT(405, 325, 619, 380), [this, g](IControl*) {
      for (const auto& spec : sawstar::kParameters) {
        const int id = static_cast<int>(spec.id);
        BeginInformHostOfParamChangeFromUI(id);
        const double value = sawstar::Normalize(spec, spec.initial);
        SendParameterValueFromUI(id, value);
        EndInformHostOfParamChangeFromUI(id);
        SendParameterValueFromDelegate(id, value, true);
      }
    }, "Load Init"), kNoTag, "presets");
    g->AttachControl(new ITextControl(IRECT(20, 495, 1004, 540),
      "0.1.0-dev  |  SILENT DEVELOPMENT SHELL  |  AUDIO ENGINE NOT CONNECTED", IText(14, light)));
    selectPage(mPage);
  };
#endif
}
#if IPLUG_DSP
void SAWSTAR::ProcessBlock(sample**, sample** outputs, int frames) {
  for (int channel = 0; channel < NOutChansConnected(); ++channel)
    std::fill_n(outputs[channel], frames, sample(0));
}
void SAWSTAR::ProcessMidiMsg(const IMidiMsg&) {
  // The host MIDI input is declared; event handling starts with the voice engine.
}
#endif
