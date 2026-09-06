// SPDX-License-Identifier: MIT
#pragma once
#include "IPlug_include_in_plug_hdr.h"

class SAWSTAR final : public iplug::Plugin {
public:
  explicit SAWSTAR(const iplug::InstanceInfo& info);
#if IPLUG_DSP
  void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int frames) override;
  void ProcessMidiMsg(const iplug::IMidiMsg& msg) override;
#endif
private:
  int mPage = 0; // Editor-only state; never read by the audio callback.
};
