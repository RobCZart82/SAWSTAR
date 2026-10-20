// SPDX-License-Identifier: MIT
#pragma once
// One shared API timeline for the engine grid and the separate setup probe.
namespace sawstar_test {
template<class S> void ConfigureDispatchFixture(S& s, double rate, int filter, int voice, bool fx, int waveform) {
  s.Reset(rate); s.SetParameters(-12, 2, 50, .7, 10); s.SetOutputBoost(12);
  s.SetSaw(25, 70, 70); s.SetOsc2(19, 60, 80);
  s.SetMixer(70, 50, 20, 5, 0, -1, 0, 0);
  s.SetFilter(1800, 50, 100); s.SetFilterCharacter(20, filter);
  s.SetFilterEnvelope(60, 100, 1, 30, .3f, 40);
  s.SetVoiceMode(voice, 15, true); s.SetPerformance(24, 24);
  s.SetLfo(20, 70, 3, 0, false, 0, 136, true);
  s.SetLfo2(17, 50, 2, 1, false, 0, 136, false);
  s.SetModulation(0, 3, 0, 100); s.SetModulation(1, 4, 0, -50);
  s.SetModulation(2, 5, 1, 20);
  if (fx) {
    s.SetChorus(true, 20, .3, 30);
    s.SetDelay(true, 15, 250, 30, 2000, true, false, 2, 136);
    s.SetReverb(true, 15, 50, 2, 6000);
  }
  s.SetWaveforms(waveform,waveform);
  for (int note = 48; note < (voice == 0 ? 64 : 51); ++note) s.Midi(0x90, note, 100);
}
}
