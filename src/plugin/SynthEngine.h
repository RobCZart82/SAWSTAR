// SPDX-License-Identifier: MIT
#pragma once
#include "engine/Synth.h"
#if defined(SAWSTAR_PREMIUM_NATIVE_CANDIDATE)
#include "PremiumNativeCandidateSynth.h"
#elif defined(SAWSTAR_PREMIUM_PREVIEW) || defined(SAWSTAR_PREMIUM_PRODUCTION)
#include "PremiumPreviewSynth.h"
#endif
namespace sawstar {
#if defined(SAWSTAR_PREMIUM_NATIVE_CANDIDATE)
using PluginSynth = premium_native_candidate_engine::Synth;
#elif defined(SAWSTAR_PREMIUM_PREVIEW) || defined(SAWSTAR_PREMIUM_PRODUCTION)
using PluginSynth = premium_preview_engine::Synth;
#else
using PluginSynth = Synth;
#endif
}
