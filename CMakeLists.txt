cmake_minimum_required(VERSION 3.21)

# ---------------------------------------------------------------------------
# Project metadata
# ---------------------------------------------------------------------------

file(READ
  "${CMAKE_CURRENT_SOURCE_DIR}/release.json"
  SAWSTAR_METADATA
)

set_property(
  DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/release.json"
)

string(JSON SAWSTAR_VERSION
  GET "${SAWSTAR_METADATA}" version
)

string(JSON SAWSTAR_MACOS_MINIMUM
  GET "${SAWSTAR_METADATA}" macos_minimum
)

if(APPLE AND NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
  set(
    CMAKE_OSX_DEPLOYMENT_TARGET
    "${SAWSTAR_MACOS_MINIMUM}"
    CACHE STRING "Minimum macOS version"
  )
endif()

project(
  SAWSTAR
  VERSION ${SAWSTAR_VERSION}
  LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# Build options
# ---------------------------------------------------------------------------

option(
  SAWSTAR_BUILD_PLUGIN
  "Build the iPlug2 VST3 development shell"
  OFF
)

option(
  SAWSTAR_CHECK_DAISYSP
  "Compile and test the selected DaisySP primitives"
  OFF
)

if(SAWSTAR_BUILD_PLUGIN)
  enable_language(C)

  set(
    CMAKE_MSVC_RUNTIME_LIBRARY
    "MultiThreaded$<$<CONFIG:Debug>:Debug>"
  )
endif()

# ---------------------------------------------------------------------------
# Testing and common dependencies
# ---------------------------------------------------------------------------

include(CTest)

find_package(Threads REQUIRED)

# ---------------------------------------------------------------------------
# Foundation library
# ---------------------------------------------------------------------------

add_library(
  sawstar_foundation
  STATIC
    src/plugin/Parameters.cpp
    src/plugin/State.cpp
    src/presets/FactoryPresets.cpp
    src/presets/UserPresets.cpp
)

add_library(
  SAWSTAR::Foundation
  ALIAS
  sawstar_foundation
)

target_include_directories(
  sawstar_foundation
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_features(
  sawstar_foundation
  PUBLIC
    cxx_std_17
)

target_link_libraries(
  sawstar_foundation
  PUBLIC
    Threads::Threads
)

if(APPLE)
  target_link_libraries(
    sawstar_foundation
    PUBLIC
      "-framework CoreFoundation"
  )
elseif(WIN32)
  target_link_libraries(
    sawstar_foundation
    PUBLIC
      Normaliz
  )
endif()

set_target_properties(
  sawstar_foundation
  PROPERTIES
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

if(MSVC)
  target_compile_options(
    sawstar_foundation
    PRIVATE
      /W4
      /permissive-
  )
else()
  target_compile_options(
    sawstar_foundation
    PRIVATE
      -Wall
      -Wextra
      -Wpedantic
  )
endif()

# ---------------------------------------------------------------------------
# Foundation and utility tests
# ---------------------------------------------------------------------------

if(BUILD_TESTING)

  add_executable(
    sawstar_meter_tests
    tests/meter.cpp
  )

  target_include_directories(
    sawstar_meter_tests
    PRIVATE
      src
  )

  target_compile_features(
    sawstar_meter_tests
    PRIVATE
      cxx_std_17
  )

  add_test(
    NAME meter_ballistics
    COMMAND sawstar_meter_tests
  )


  add_executable(
    sawstar_display_tests
    tests/display_state.cpp
  )

  target_include_directories(
    sawstar_display_tests
    PRIVATE
      src
  )

  target_compile_features(
    sawstar_display_tests
    PRIVATE
      cxx_std_17
  )

  add_test(
    NAME display_timing
    COMMAND sawstar_display_tests
  )


  add_executable(
    sawstar_scope_tests
    tests/scope.cpp
  )

  target_include_directories(
    sawstar_scope_tests
    PRIVATE
      src
  )

  target_compile_features(
    sawstar_scope_tests
    PRIVATE
      cxx_std_17
  )

  target_link_libraries(
    sawstar_scope_tests
    PRIVATE
      Threads::Threads
  )

  add_test(
    NAME realtime_scope
    COMMAND sawstar_scope_tests
  )


  add_executable(
    sawstar_preset_reliability
    tests/preset_reliability.cpp
  )

  target_link_libraries(
    sawstar_preset_reliability
    PRIVATE
      SAWSTAR::Foundation
      Threads::Threads
  )

  add_test(
    NAME preset_reliability
    COMMAND sawstar_preset_reliability
  )


  add_executable(
    sawstar_sub_tests
    tests/sub_oscillator.cpp
  )

  target_link_libraries(
    sawstar_sub_tests
    PRIVATE
      SAWSTAR::Foundation
  )

  add_test(
    NAME sub_waveform_transitions
    COMMAND sawstar_sub_tests
  )


  add_executable(
    sawstar_library_tests
    tests/preset_library.cpp
  )

  target_link_libraries(
    sawstar_library_tests
    PRIVATE
      SAWSTAR::Foundation
  )

  add_test(
    NAME preset_library_import
    COMMAND sawstar_library_tests
  )

  add_executable(sawstar_preset_import_index_tests tests/preset_import_index.cpp)
  target_link_libraries(sawstar_preset_import_index_tests PRIVATE SAWSTAR::Foundation)
  add_test(NAME preset_import_index COMMAND sawstar_preset_import_index_tests)

  # Manual filesystem benchmark; timing thresholds do not belong in CTest.
  add_executable(sawstar_preset_import_benchmark tests/preset_import_benchmark.cpp)
  target_link_libraries(sawstar_preset_import_benchmark PRIVATE SAWSTAR::Foundation)
  add_executable(sawstar_preset_import_job_tests tests/preset_import_job.cpp)
  target_link_libraries(sawstar_preset_import_job_tests PRIVATE SAWSTAR::Foundation)
  add_test(NAME preset_import_job COMMAND sawstar_preset_import_job_tests)


  add_executable(
    sawstar_user_preset_tests
    tests/user_presets.cpp
  )

  target_link_libraries(
    sawstar_user_preset_tests
    PRIVATE
      SAWSTAR::Foundation
      Threads::Threads
  )

  add_test(
    NAME user_preset_files
    COMMAND sawstar_user_preset_tests
  )


  add_executable(
    sawstar_state_tests
    tests/state.cpp
  )

  target_link_libraries(
    sawstar_state_tests
    PRIVATE
      SAWSTAR::Foundation
  )

  add_test(
    NAME state_compatibility
    COMMAND sawstar_state_tests
  )


  add_executable(
    sawstar_state_roundtrip_tests
    tests/state_roundtrip.cpp
  )

  target_link_libraries(
    sawstar_state_roundtrip_tests
    PRIVATE
      SAWSTAR::Foundation
  )

  add_test(
    NAME state_roundtrip
    COMMAND sawstar_state_roundtrip_tests
  )

  add_executable(sawstar_preset_canonicalization_tests tests/preset_canonicalization.cpp)
  target_link_libraries(sawstar_preset_canonicalization_tests PRIVATE SAWSTAR::Foundation)
  add_test(NAME preset_discrete_canonicalization COMMAND sawstar_preset_canonicalization_tests)

  add_executable(sawstar_strict_preset_tests tests/strict_preset_files.cpp)
  target_link_libraries(sawstar_strict_preset_tests PRIVATE SAWSTAR::Foundation)
  add_test(NAME strict_preset_files COMMAND sawstar_strict_preset_tests)


  add_executable(
    sawstar_parameter_tests
    tests/parameters.cpp
  )

  target_link_libraries(
    sawstar_parameter_tests
    PRIVATE
      SAWSTAR::Foundation
  )

  add_test(
    NAME parameter_contract
    COMMAND sawstar_parameter_tests
  )

endif()

# ---------------------------------------------------------------------------
# DaisySP primitives and audio engine
# ---------------------------------------------------------------------------

if(SAWSTAR_CHECK_DAISYSP OR SAWSTAR_BUILD_PLUGIN)

  set(
    _daisy
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/DaisySP"
  )

  if(NOT EXISTS "${_daisy}/Source/Synthesis/oscillator.cpp")
    message(
      FATAL_ERROR
      "DaisySP is missing. Run: git submodule update --init third_party/DaisySP"
    )
  endif()

  # Deliberately avoid the omnibus header and optional DaisySP-LGPL sources.
  add_library(
    sawstar_daisy_primitives
    STATIC
      "${_daisy}/Source/Synthesis/oscillator.cpp"
      "${_daisy}/Source/Control/adsr.cpp"
  )

  target_include_directories(
    sawstar_daisy_primitives
    SYSTEM
    PUBLIC
      "${_daisy}/Source"
    PRIVATE
      "${_daisy}/Source/Synthesis"
      "${_daisy}/Source/Control"
      "${_daisy}/Source/Utility"
  )

  target_compile_features(
    sawstar_daisy_primitives
    PUBLIC
      cxx_std_17
  )

  if(MSVC)
    # DaisySP ADSR uses M_E; MSVC exposes it only with this opt-in.
    target_compile_definitions(
      sawstar_daisy_primitives
      PRIVATE
        _USE_MATH_DEFINES
    )
  endif()

  set_target_properties(
    sawstar_daisy_primitives
    PROPERTIES
      CXX_EXTENSIONS OFF
      POSITION_INDEPENDENT_CODE ON
  )


  add_library(
    sawstar_engine
    STATIC
      src/engine/Synth.cpp
      src/dsp/SevenSaw.cpp
      src/dsp/LowPass.cpp
  )

  target_link_libraries(
    sawstar_engine
    PUBLIC
      sawstar_foundation
      sawstar_daisy_primitives
  )

  set_target_properties(
    sawstar_engine
    PROPERTIES
      POSITION_INDEPENDENT_CODE ON
  )

  if(MSVC)
    target_compile_options(
      sawstar_engine
      PRIVATE
        /W4
        /permissive-
    )
  else()
    target_compile_options(
      sawstar_engine
      PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
  endif()

# ---------------------------------------------------------------------------
# Audio engine and DSP tests
# ---------------------------------------------------------------------------

  if(BUILD_TESTING)

    add_executable(
      sawstar_idle_start_tests
      tests/idle_start.cpp
    )

    target_link_libraries(
      sawstar_idle_start_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME idle_voice_start
      COMMAND sawstar_idle_start_tests
    )


    add_executable(
      sawstar_dsp_safety_tests
      tests/dsp_safety.cpp
    )

    target_link_libraries(
      sawstar_dsp_safety_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME dsp_safety
      COMMAND sawstar_dsp_safety_tests
    )


    add_executable(
      sawstar_reset_lifecycle_tests
      tests/reset_lifecycle.cpp
    )

    target_link_libraries(
      sawstar_reset_lifecycle_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME reset_lifecycle
      COMMAND sawstar_reset_lifecycle_tests
    )


    add_executable(
      sawstar_overlap_tests
      tests/overlapping_notes.cpp
    )

    target_link_libraries(
      sawstar_overlap_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME overlapping_notes
      COMMAND sawstar_overlap_tests
    )


    add_executable(
      sawstar_engine_audit
      tests/engine_audit.cpp
    )

    target_link_libraries(
      sawstar_engine_audit
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME engine_audit
      COMMAND sawstar_engine_audit
    )


    add_executable(
      sawstar_parameter_transition_tests
      tests/parameter_transitions.cpp
    )

    target_link_libraries(
      sawstar_parameter_transition_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME parameter_transitions
      COMMAND sawstar_parameter_transition_tests
    )


    add_executable(
      sawstar_voice_transition_tests
      tests/voice_transitions.cpp
    )

    target_link_libraries(
      sawstar_voice_transition_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME voice_transitions
      COMMAND sawstar_voice_transition_tests
    )


    add_executable(
      sawstar_source_quality_tests
      tests/source_quality.cpp
    )

    target_link_libraries(
      sawstar_source_quality_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME source_quality
      COMMAND sawstar_source_quality_tests
    )


    # Benchmark executable. It is intentionally not registered with CTest.
    add_executable(
      sawstar_audio_benchmark
      tests/audio_benchmark.cpp
    )

    target_link_libraries(
      sawstar_audio_benchmark
      PRIVATE
        sawstar_engine
    )


    add_executable(
      sawstar_arp_tests
      tests/arpeggiator.cpp
    )

    target_link_libraries(
      sawstar_arp_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME arpeggiator
      COMMAND sawstar_arp_tests
    )


    add_executable(
      sawstar_modulation_tests
      tests/modulation.cpp
    )

    target_link_libraries(
      sawstar_modulation_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME dual_lfo_modulation
      COMMAND sawstar_modulation_tests
    )


    add_executable(
      sawstar_short_note_tests
      tests/short_notes.cpp
    )

    target_link_libraries(
      sawstar_short_note_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME short_note_release
      COMMAND sawstar_short_note_tests
    )


    add_executable(
      sawstar_noise_tests
      tests/noise.cpp
    )

    target_link_libraries(
      sawstar_noise_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME noise_sources_color
      COMMAND sawstar_noise_tests
    )


    add_executable(
      sawstar_mono_tests
      tests/mono.cpp
    )

    target_link_libraries(
      sawstar_mono_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME mono_legato_glide
      COMMAND sawstar_mono_tests
    )


    add_executable(
      sawstar_reverb_tests
      tests/reverb.cpp
    )

    target_link_libraries(
      sawstar_reverb_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME stereo_reverb
      COMMAND sawstar_reverb_tests
    )


    add_executable(
      sawstar_delay_tests
      tests/delay.cpp
    )

    target_link_libraries(
      sawstar_delay_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME stereo_delay
      COMMAND sawstar_delay_tests
    )


    add_executable(
      sawstar_width_tests
      tests/width.cpp
    )

    target_link_libraries(
      sawstar_width_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME master_width
      COMMAND sawstar_width_tests
    )


    add_executable(
      sawstar_chorus_tests
      tests/chorus.cpp
    )

    target_link_libraries(
      sawstar_chorus_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME stereo_chorus
      COMMAND sawstar_chorus_tests
    )


    add_executable(
      sawstar_lfo_wave_tests
      tests/lfo_wave.cpp
    )

    target_link_libraries(
      sawstar_lfo_wave_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME lfo_waveforms
      COMMAND sawstar_lfo_wave_tests
    )


    # Research candidate only: not linked into the plugin or Synth signal path.
    add_executable(sawstar_premium_filter_tests tests/premium_filter.cpp)
    target_link_libraries(sawstar_premium_filter_tests PRIVATE sawstar_engine)
    add_test(NAME premium_filter_response COMMAND sawstar_premium_filter_tests)
    add_executable(sawstar_premium_mode_tests tests/premium_filter_modes.cpp)
    target_link_libraries(sawstar_premium_mode_tests PRIVATE sawstar_engine)
    add_test(NAME premium_filter_modes COMMAND sawstar_premium_mode_tests)
    add_executable(sawstar_premium_cache_tests tests/premium_filter_cache.cpp)
    target_link_libraries(sawstar_premium_cache_tests PRIVATE sawstar_engine)
    add_test(NAME premium_filter_cache_identity COMMAND sawstar_premium_cache_tests)
    add_executable(sawstar_premium_drive_tests tests/premium_drive.cpp)
    target_link_libraries(sawstar_premium_drive_tests PRIVATE sawstar_engine)
    add_test(NAME premium_drive_alias_and_latency COMMAND sawstar_premium_drive_tests)
    add_executable(sawstar_premium_polyphase_tests tests/premium_drive_polyphase.cpp)
    target_link_libraries(sawstar_premium_polyphase_tests PRIVATE sawstar_engine)
    add_test(NAME premium_drive_polyphase_identity COMMAND sawstar_premium_polyphase_tests)
    add_executable(sawstar_premium_cost_tests tests/premium_drive_cost.cpp)
    target_link_libraries(sawstar_premium_cost_tests PRIVATE sawstar_engine)
    add_test(NAME premium_drive_cost_controls COMMAND sawstar_premium_cost_tests)
    add_executable(sawstar_premium_tanh_tests tests/premium_tanh.cpp)
    target_link_libraries(sawstar_premium_tanh_tests PRIVATE sawstar_engine)
    add_test(NAME premium_tanh_scalar_contract COMMAND sawstar_premium_tanh_tests)
    add_executable(sawstar_premium_drive_tanh_tests tests/premium_drive_tanh.cpp)
    target_link_libraries(sawstar_premium_drive_tanh_tests PRIVATE sawstar_engine)
    add_test(NAME premium_drive_tanh_study COMMAND sawstar_premium_drive_tanh_tests)
    add_executable(sawstar_premium_drive_spectrum_tests tests/premium_drive_spectrum.cpp)
    target_link_libraries(sawstar_premium_drive_spectrum_tests PRIVATE sawstar_engine)
    add_test(NAME premium_drive_spectral_grid COMMAND sawstar_premium_drive_spectrum_tests)
    add_executable(sawstar_premium_drive_benchmark experiments/premium_filter/benchmark_drive.cpp)
    target_link_libraries(sawstar_premium_drive_benchmark PRIVATE sawstar_engine)
    # Compile the current Synth in a separate namespace, replacing only its
    # per-voice filter type. Production files and library remain unchanged.
    # Configure dependencies ensure future Synth edits regenerate this probe.
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/Synth.h"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/Synth.cpp")
    set(_premium_engine "${CMAKE_CURRENT_BINARY_DIR}/premium_engine")
    file(MAKE_DIRECTORY "${_premium_engine}")
    foreach(_part IN ITEMS h cpp)
      file(READ "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/Synth.${_part}" _synth_probe)
      if(NOT _synth_probe MATCHES "namespace sawstar [{]")
        message(FATAL_ERROR "Premium probe cannot find Synth namespace")
      endif()
      string(REPLACE "namespace sawstar {" "namespace sawstar::experimental_engine {" _synth_probe "${_synth_probe}")
      if(_part STREQUAL "h")
        if(NOT _synth_probe MATCHES "LowPass filter;")
          message(FATAL_ERROR "Premium probe cannot find per-voice filter")
        endif()
        string(REPLACE "#include \"dsp/LowPass.h\"" "#include \"EnginePremiumFilter.h\"" _synth_probe "${_synth_probe}")
        string(REPLACE "LowPass filter;" "experimental::EnginePremiumFilter filter;" _synth_probe "${_synth_probe}")
      else()
        string(REPLACE "#include \"engine/Synth.h\"" "#include \"PremiumSynth.h\"" _synth_probe "${_synth_probe}")
      endif()
      file(WRITE "${_premium_engine}/PremiumSynth.${_part}" "${_synth_probe}")
    endforeach()
    add_library(sawstar_premium_engine_probe STATIC "${_premium_engine}/PremiumSynth.cpp")
    target_include_directories(sawstar_premium_engine_probe PUBLIC "${_premium_engine}"
      "${CMAKE_CURRENT_SOURCE_DIR}/experiments/premium_filter")
    target_link_libraries(sawstar_premium_engine_probe PUBLIC sawstar_engine)
    add_executable(sawstar_premium_engine_benchmark experiments/premium_filter/benchmark_engine.cpp)
    target_link_libraries(sawstar_premium_engine_benchmark PRIVATE sawstar_premium_engine_probe)
    add_test(NAME premium_engine_probe_contract COMMAND sawstar_premium_engine_benchmark --smoke)
    add_test(NAME premium_engine_modulation COMMAND sawstar_premium_engine_benchmark --modulation)
    # Keep both probe class definitions distinct even if a later tool links
    # both libraries: different per-voice layouts must not share Synth symbols.
    foreach(_part IN ITEMS h cpp)
      file(READ "${_premium_engine}/PremiumSynth.${_part}" _rate_probe)
      string(REPLACE "sawstar::experimental_engine" "sawstar::experimental_rate_engine" _rate_probe "${_rate_probe}")
      string(REPLACE "experimental::EnginePremiumFilter" "experimental::RateScaledEnginePremiumFilter" _rate_probe "${_rate_probe}")
      string(REPLACE "PremiumSynth.h" "RatePremiumSynth.h" _rate_probe "${_rate_probe}")
      file(WRITE "${_premium_engine}/RatePremiumSynth.${_part}" "${_rate_probe}")
    endforeach()
    add_library(sawstar_premium_rate_probe STATIC "${_premium_engine}/RatePremiumSynth.cpp")
    target_include_directories(sawstar_premium_rate_probe PUBLIC "${_premium_engine}"
      "${CMAKE_CURRENT_SOURCE_DIR}/experiments/premium_filter")
    target_compile_definitions(sawstar_premium_rate_probe PUBLIC SAWSTAR_PREMIUM_HIGH_RATE_STUDY=1)
    target_link_libraries(sawstar_premium_rate_probe PUBLIC sawstar_engine)
    add_executable(sawstar_premium_rate_benchmark experiments/premium_filter/benchmark_engine.cpp)
    target_link_libraries(sawstar_premium_rate_benchmark PRIVATE sawstar_premium_rate_probe)
    add_test(NAME premium_rate_engine_contract COMMAND sawstar_premium_rate_benchmark --smoke)
    add_test(NAME premium_rate_engine_modulation COMMAND sawstar_premium_rate_benchmark --modulation)
    foreach(_part IN ITEMS h cpp)
      file(READ "${_premium_engine}/PremiumSynth.${_part}" _tanh_probe)
      string(REPLACE "sawstar::experimental_engine" "sawstar::experimental_tanh_engine" _tanh_probe "${_tanh_probe}")
      string(REPLACE "experimental::EnginePremiumFilter" "experimental::TanhStudyEnginePremiumFilter" _tanh_probe "${_tanh_probe}")
      string(REPLACE "PremiumSynth.h" "TanhPremiumSynth.h" _tanh_probe "${_tanh_probe}")
      file(WRITE "${_premium_engine}/TanhPremiumSynth.${_part}" "${_tanh_probe}")
    endforeach()
    add_library(sawstar_premium_tanh_probe STATIC "${_premium_engine}/TanhPremiumSynth.cpp")
    target_include_directories(sawstar_premium_tanh_probe PUBLIC "${_premium_engine}"
      "${CMAKE_CURRENT_SOURCE_DIR}/experiments/premium_filter")
    target_link_libraries(sawstar_premium_tanh_probe PUBLIC sawstar_engine)
    add_executable(sawstar_premium_tanh_engine_tests tests/premium_tanh_engine.cpp)
    target_link_libraries(sawstar_premium_tanh_engine_tests PRIVATE sawstar_premium_engine_probe sawstar_premium_tanh_probe)
    add_test(NAME premium_tanh_engine_study COMMAND sawstar_premium_tanh_engine_tests)
    add_executable(sawstar_premium_tanh_deadline experiments/premium_filter/benchmark_tanh_deadline.cpp)
    target_link_libraries(sawstar_premium_tanh_deadline PRIVATE sawstar_premium_engine_probe sawstar_premium_tanh_probe)
    add_test(NAME premium_tanh_deadline_fixture COMMAND sawstar_premium_tanh_deadline)
    add_executable(sawstar_premium_simd_tests tests/premium_drive_simd.cpp)
    target_link_libraries(sawstar_premium_simd_tests PRIVATE sawstar_engine)
    add_test(NAME premium_drive_simd_identity COMMAND sawstar_premium_simd_tests)
    foreach(_part IN ITEMS h cpp)
      file(READ "${_premium_engine}/PremiumSynth.${_part}" _simd_probe)
      string(REPLACE "sawstar::experimental_engine" "sawstar::experimental_simd_engine" _simd_probe "${_simd_probe}")
      string(REPLACE "experimental::EnginePremiumFilter" "experimental::SimdStudyEnginePremiumFilter" _simd_probe "${_simd_probe}")
      string(REPLACE "PremiumSynth.h" "SimdPremiumSynth.h" _simd_probe "${_simd_probe}")
      file(WRITE "${_premium_engine}/SimdPremiumSynth.${_part}" "${_simd_probe}")
    endforeach()
    add_library(sawstar_premium_simd_probe STATIC "${_premium_engine}/SimdPremiumSynth.cpp")
    target_include_directories(sawstar_premium_simd_probe PUBLIC "${_premium_engine}"
      "${CMAKE_CURRENT_SOURCE_DIR}/experiments/premium_filter")
    target_link_libraries(sawstar_premium_simd_probe PUBLIC sawstar_engine)
    add_executable(sawstar_premium_simd_deadline experiments/premium_filter/benchmark_tanh_deadline.cpp)
    target_compile_definitions(sawstar_premium_simd_deadline PRIVATE SAWSTAR_SIMD_FIR_STUDY=1)
    target_link_libraries(sawstar_premium_simd_deadline PRIVATE sawstar_premium_engine_probe sawstar_premium_simd_probe)
    add_test(NAME premium_simd_deadline_fixture COMMAND sawstar_premium_simd_deadline)

    foreach(_part IN ITEMS h cpp)
      file(READ "${_premium_engine}/SimdPremiumSynth.${_part}" _gain_probe)
      if(_part STREQUAL "h" AND NOT _gain_probe MATCHES "experimental::SimdStudyEnginePremiumFilter filter;")
        message(FATAL_ERROR "Gain probe cannot find SIMD/4x per-voice filter")
      endif()
      string(REPLACE "sawstar::experimental_simd_engine" "sawstar::experimental_gain_engine" _gain_probe "${_gain_probe}")
      string(REPLACE "experimental::SimdStudyEnginePremiumFilter" "experimental::GainStudyEnginePremiumFilter" _gain_probe "${_gain_probe}")
      string(REPLACE "SimdPremiumSynth.h" "GainPremiumSynth.h" _gain_probe "${_gain_probe}")
      file(WRITE "${_premium_engine}/GainPremiumSynth.${_part}" "${_gain_probe}")
    endforeach()
    add_library(sawstar_premium_gain_probe STATIC "${_premium_engine}/GainPremiumSynth.cpp")
    target_include_directories(sawstar_premium_gain_probe PUBLIC "${_premium_engine}"
      "${CMAKE_CURRENT_SOURCE_DIR}/experiments/premium_filter")
    target_link_libraries(sawstar_premium_gain_probe PUBLIC sawstar_engine)
    add_executable(sawstar_premium_gain_deadline experiments/premium_filter/benchmark_tanh_deadline.cpp)
    target_compile_definitions(sawstar_premium_gain_deadline PRIVATE SAWSTAR_GAIN_NORMALIZATION_STUDY=1)
    target_link_libraries(sawstar_premium_gain_deadline PRIVATE sawstar_premium_simd_probe sawstar_premium_gain_probe)
    add_test(NAME premium_gain_deadline_fixture COMMAND sawstar_premium_gain_deadline)


    add_executable(sawstar_premium_rate_simd_tests tests/premium_drive_rate_simd.cpp)
    target_link_libraries(sawstar_premium_rate_simd_tests PRIVATE sawstar_engine)
    add_test(NAME premium_drive_rate_simd_identity COMMAND sawstar_premium_rate_simd_tests)
    foreach(_part IN ITEMS h cpp)
      file(READ "${_premium_engine}/RatePremiumSynth.${_part}" _rate_simd_probe)
      if(_part STREQUAL "h" AND NOT _rate_simd_probe MATCHES "experimental::RateScaledEnginePremiumFilter filter;")
        message(FATAL_ERROR "Rate SIMD probe cannot find rate-scaled per-voice filter")
      endif()
      string(REPLACE "sawstar::experimental_rate_engine" "sawstar::experimental_rate_simd_engine" _rate_simd_probe "${_rate_simd_probe}")
      string(REPLACE "experimental::RateScaledEnginePremiumFilter" "experimental::RateScaledSimdEnginePremiumFilter" _rate_simd_probe "${_rate_simd_probe}")
      string(REPLACE "RatePremiumSynth.h" "RateSimdPremiumSynth.h" _rate_simd_probe "${_rate_simd_probe}")
      file(WRITE "${_premium_engine}/RateSimdPremiumSynth.${_part}" "${_rate_simd_probe}")
    endforeach()
    add_library(sawstar_premium_rate_simd_probe STATIC "${_premium_engine}/RateSimdPremiumSynth.cpp")
    target_include_directories(sawstar_premium_rate_simd_probe PUBLIC "${_premium_engine}"
      "${CMAKE_CURRENT_SOURCE_DIR}/experiments/premium_filter")
    target_link_libraries(sawstar_premium_rate_simd_probe PUBLIC sawstar_engine)
    add_executable(sawstar_premium_rate_simd_deadline experiments/premium_filter/benchmark_tanh_deadline.cpp)
    target_compile_definitions(sawstar_premium_rate_simd_deadline PRIVATE SAWSTAR_RATE_SIMD_FIR_STUDY=1)
    target_link_libraries(sawstar_premium_rate_simd_deadline PRIVATE sawstar_premium_rate_probe sawstar_premium_rate_simd_probe)
    add_test(NAME premium_rate_simd_deadline_fixture COMMAND sawstar_premium_rate_simd_deadline)

    foreach(_part IN ITEMS h cpp)
      file(READ "${_premium_engine}/RateSimdPremiumSynth.${_part}" _rate_gain_probe)
      if(_part STREQUAL "h" AND NOT _rate_gain_probe MATCHES "experimental::RateScaledSimdEnginePremiumFilter filter;")
        message(FATAL_ERROR "Rate gain probe cannot find rate-scaled SIMD filter")
      endif()
      string(REPLACE "sawstar::experimental_rate_simd_engine" "sawstar::experimental_rate_gain_engine" _rate_gain_probe "${_rate_gain_probe}")
      string(REPLACE "experimental::RateScaledSimdEnginePremiumFilter" "experimental::RateScaledGainEnginePremiumFilter" _rate_gain_probe "${_rate_gain_probe}")
      string(REPLACE "RateSimdPremiumSynth.h" "RateGainPremiumSynth.h" _rate_gain_probe "${_rate_gain_probe}")
      file(WRITE "${_premium_engine}/RateGainPremiumSynth.${_part}" "${_rate_gain_probe}")
    endforeach()
    add_library(sawstar_premium_rate_gain_probe STATIC "${_premium_engine}/RateGainPremiumSynth.cpp")
    target_include_directories(sawstar_premium_rate_gain_probe PUBLIC "${_premium_engine}"
      "${CMAKE_CURRENT_SOURCE_DIR}/experiments/premium_filter")
    target_link_libraries(sawstar_premium_rate_gain_probe PUBLIC sawstar_engine)
    add_executable(sawstar_premium_rate_gain_deadline experiments/premium_filter/benchmark_tanh_deadline.cpp)
    target_compile_definitions(sawstar_premium_rate_gain_deadline PRIVATE SAWSTAR_RATE_GAIN_STUDY=1)
    target_link_libraries(sawstar_premium_rate_gain_deadline PRIVATE sawstar_premium_rate_simd_probe sawstar_premium_rate_gain_probe)
    add_test(NAME premium_rate_gain_deadline_fixture COMMAND sawstar_premium_rate_gain_deadline)
    add_executable(sawstar_premium_rate_gain_tests tests/premium_drive_rate_gain.cpp)
    target_link_libraries(sawstar_premium_rate_gain_tests PRIVATE sawstar_engine)
    add_test(NAME premium_drive_rate_gain_contract COMMAND sawstar_premium_rate_gain_tests)

    # Lookup study differs only in nonlinear evaluation; separate Synth symbols.
    foreach(_part IN ITEMS h cpp)
      file(READ "${_premium_engine}/RateGainPremiumSynth.${_part}" _lookup_probe)
      if(_part STREQUAL "h" AND NOT _lookup_probe MATCHES "experimental::RateScaledGainEnginePremiumFilter filter;")
        message(FATAL_ERROR "Lookup probe cannot find rate-scaled gain filter")
      endif()
      string(REPLACE "sawstar::experimental_rate_gain_engine" "sawstar::experimental_lookup_engine" _lookup_probe "${_lookup_probe}")
      string(REPLACE "experimental::RateScaledGainEnginePremiumFilter" "experimental::RateScaledLookupEnginePremiumFilter" _lookup_probe "${_lookup_probe}")
      string(REPLACE "RateGainPremiumSynth.h" "LookupPremiumSynth.h" _lookup_probe "${_lookup_probe}")
      file(WRITE "${_premium_engine}/LookupPremiumSynth.${_part}" "${_lookup_probe}")
    endforeach()
    add_library(sawstar_premium_lookup_probe STATIC "${_premium_engine}/LookupPremiumSynth.cpp")
    target_include_directories(sawstar_premium_lookup_probe PUBLIC "${_premium_engine}"
      "${CMAKE_CURRENT_SOURCE_DIR}/experiments/premium_filter")
    target_link_libraries(sawstar_premium_lookup_probe PUBLIC sawstar_engine)
    # Opt-in loop-dispatch study, with distinct Synth/filter symbols.
    foreach(_part IN ITEMS h cpp)
      file(READ "${_premium_engine}/LookupPremiumSynth.${_part}" _unrolled_probe)
      if(_part STREQUAL "h" AND NOT _unrolled_probe MATCHES "experimental::RateScaledLookupEnginePremiumFilter filter;")
        message(FATAL_ERROR "Unrolled probe cannot find lookup per-voice filter")
      endif()
      string(REPLACE "sawstar::experimental_lookup_engine" "sawstar::experimental_unrolled_engine" _unrolled_probe "${_unrolled_probe}")
      string(REPLACE "experimental::RateScaledLookupEnginePremiumFilter" "experimental::RateScaledUnrolledEnginePremiumFilter" _unrolled_probe "${_unrolled_probe}")
      string(REPLACE "LookupPremiumSynth.h" "UnrolledPremiumSynth.h" _unrolled_probe "${_unrolled_probe}")
      file(WRITE "${_premium_engine}/UnrolledPremiumSynth.${_part}" "${_unrolled_probe}")
    endforeach()
    add_library(sawstar_premium_unrolled_probe STATIC "${_premium_engine}/UnrolledPremiumSynth.cpp")
    target_include_directories(sawstar_premium_unrolled_probe PUBLIC "${_premium_engine}"
      "${CMAKE_CURRENT_SOURCE_DIR}/experiments/premium_filter")
    target_link_libraries(sawstar_premium_unrolled_probe PUBLIC sawstar_engine)
    add_executable(sawstar_premium_unrolled_deadline experiments/premium_filter/benchmark_tanh_deadline.cpp)
    target_compile_definitions(sawstar_premium_unrolled_deadline PRIVATE SAWSTAR_UNROLLED_FIR_STUDY=1)
    target_link_libraries(sawstar_premium_unrolled_deadline PRIVATE sawstar_premium_lookup_probe sawstar_premium_unrolled_probe)
    add_test(NAME premium_unrolled_deadline_fixture COMMAND sawstar_premium_unrolled_deadline)
    add_executable(sawstar_premium_unrolled_engine_tests tests/premium_lookup_engine.cpp)
    target_compile_definitions(sawstar_premium_unrolled_engine_tests PRIVATE SAWSTAR_UNROLLED_FIR_STUDY=1)
    target_link_libraries(sawstar_premium_unrolled_engine_tests PRIVATE sawstar_premium_lookup_probe sawstar_premium_unrolled_probe)
    add_test(NAME premium_unrolled_engine_modulation COMMAND sawstar_premium_unrolled_engine_tests)
    add_executable(sawstar_premium_unrolled_complex_tests tests/premium_drive_complex.cpp)
    target_compile_definitions(sawstar_premium_unrolled_complex_tests PRIVATE SAWSTAR_COMPLEX_LOOKUP_STUDY=1 SAWSTAR_UNROLLED_FIR_STUDY=1)
    target_link_libraries(sawstar_premium_unrolled_complex_tests PRIVATE sawstar_engine)
    # Independent 8x qualification runs in the dedicated Release workflow.
    # Existing global 8x controls are retained without duplicating slow Debug work.
    # Independent descriptive component workloads, never a timing pass/fail gate.
    add_executable(sawstar_premium_lookup_components experiments/premium_filter/benchmark_engine.cpp)
    target_compile_definitions(sawstar_premium_lookup_components PRIVATE SAWSTAR_PREMIUM_LOOKUP_COMPONENT_STUDY=1)
    target_link_libraries(sawstar_premium_lookup_components PRIVATE sawstar_premium_lookup_probe)
    add_executable(sawstar_premium_lookup_deadline experiments/premium_filter/benchmark_tanh_deadline.cpp)
    target_compile_definitions(sawstar_premium_lookup_deadline PRIVATE SAWSTAR_RATE_LOOKUP_STUDY=1)
    target_link_libraries(sawstar_premium_lookup_deadline PRIVATE sawstar_premium_rate_gain_probe sawstar_premium_lookup_probe)
    add_test(NAME premium_lookup_deadline_fixture COMMAND sawstar_premium_lookup_deadline)
    add_test(NAME premium_lookup_modulated_deadline_fixture COMMAND sawstar_premium_lookup_deadline --modulation-contract)
    add_executable(sawstar_premium_lookup_engine_tests tests/premium_lookup_engine.cpp)
    target_link_libraries(sawstar_premium_lookup_engine_tests PRIVATE sawstar_premium_rate_gain_probe sawstar_premium_lookup_probe)
    add_test(NAME premium_lookup_engine_modulation COMMAND sawstar_premium_lookup_engine_tests)
    add_executable(sawstar_premium_lookup_rate_tests tests/premium_drive_rate_gain.cpp)
    target_compile_definitions(sawstar_premium_lookup_rate_tests PRIVATE SAWSTAR_RATE_LOOKUP_STUDY=1)
    target_link_libraries(sawstar_premium_lookup_rate_tests PRIVATE sawstar_engine)
    add_test(NAME premium_lookup_rate_contract COMMAND sawstar_premium_lookup_rate_tests)
    add_executable(sawstar_premium_lookup_complex_tests tests/premium_drive_complex.cpp)
    target_compile_definitions(sawstar_premium_lookup_complex_tests PRIVATE SAWSTAR_COMPLEX_LOOKUP_STUDY=1)
    target_link_libraries(sawstar_premium_lookup_complex_tests PRIVATE sawstar_engine)
    add_test(NAME premium_lookup_complex_controls COMMAND sawstar_premium_lookup_complex_tests)

    add_executable(sawstar_premium_rate_tests tests/premium_drive_rate.cpp)
    target_link_libraries(sawstar_premium_rate_tests PRIVATE sawstar_engine)
    add_test(NAME premium_drive_high_rate_controls COMMAND sawstar_premium_rate_tests)
    add_executable(sawstar_premium_complex_tests tests/premium_drive_complex.cpp)
    target_link_libraries(sawstar_premium_complex_tests PRIVATE sawstar_engine)
    add_test(NAME premium_drive_complex_controls COMMAND sawstar_premium_complex_tests)
    add_executable(sawstar_premium_gain_complex_tests tests/premium_drive_complex.cpp)
    target_compile_definitions(sawstar_premium_gain_complex_tests PRIVATE SAWSTAR_COMPLEX_GAIN_STUDY=1)
    target_link_libraries(sawstar_premium_gain_complex_tests PRIVATE sawstar_engine)
    add_test(NAME premium_gain_complex_controls COMMAND sawstar_premium_gain_complex_tests)



    add_executable(sawstar_premium_filter_preview experiments/premium_filter/render.cpp)
    target_link_libraries(sawstar_premium_filter_preview PRIVATE sawstar_engine)

    add_executable(
      sawstar_filter_character_tests
      tests/filter_character.cpp
    )

    target_link_libraries(
      sawstar_filter_character_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME filter_character
      COMMAND sawstar_filter_character_tests
    )


    add_executable(
      sawstar_mixer_tests
      tests/mixer.cpp
    )

    target_link_libraries(
      sawstar_mixer_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME source_mixer
      COMMAND sawstar_mixer_tests
    )


    add_executable(
      sawstar_output_tests
      tests/output.cpp
    )

    target_link_libraries(
      sawstar_output_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME output_level
      COMMAND sawstar_output_tests
    )


    add_executable(
      sawstar_factory_tests
      tests/factory_presets.cpp
    )

    target_link_libraries(
      sawstar_factory_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME factory_presets
      COMMAND sawstar_factory_tests
    )


    add_executable(
      sawstar_performance_tests
      tests/performance.cpp
    )

    target_link_libraries(
      sawstar_performance_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME performance_wheels
      COMMAND sawstar_performance_tests
    )


    add_executable(
      sawstar_filter_modulation_tests
      tests/filter_modulation.cpp
    )

    target_link_libraries(
      sawstar_filter_modulation_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME filter_modulation
      COMMAND sawstar_filter_modulation_tests
    )


    add_executable(
      sawstar_filter_tests
      tests/filter.cpp
    )

    target_link_libraries(
      sawstar_filter_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME low_pass
      COMMAND sawstar_filter_tests
    )


    add_executable(
      sawstar_sevensaw_tests
      tests/sevensaw.cpp
    )

    target_link_libraries(
      sawstar_sevensaw_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME seven_saw
      COMMAND sawstar_sevensaw_tests
    )


    add_executable(
      sawstar_engine_tests
      tests/engine.cpp
    )

    target_link_libraries(
      sawstar_engine_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME voice_engine
      COMMAND sawstar_engine_tests
    )


    add_executable(
      sawstar_dsp_tests
      tests/daisy_primitives.cpp
    )

    target_link_libraries(
      sawstar_dsp_tests
      PRIVATE
        sawstar_daisy_primitives
    )

    add_test(
      NAME daisy_primitives
      COMMAND sawstar_dsp_tests
    )

    add_executable(
      sawstar_daisy_adsr_convergence_tests
      tests/daisy_adsr_convergence.cpp
    )

    target_link_libraries(
      sawstar_daisy_adsr_convergence_tests
      PRIVATE
        sawstar_engine
    )

    add_test(
      NAME daisy_adsr_long_time_convergence
      COMMAND sawstar_daisy_adsr_convergence_tests
    )

  endif()

endif()

# ---------------------------------------------------------------------------
# iPlug2 VST3 plugin
# ---------------------------------------------------------------------------

if(SAWSTAR_BUILD_PLUGIN)
  add_subdirectory(src/plugin)
endif()

# ---------------------------------------------------------------------------
# Stress and torture tests
# ---------------------------------------------------------------------------

if(
  BUILD_TESTING
  AND (SAWSTAR_CHECK_DAISYSP OR SAWSTAR_BUILD_PLUGIN)
)

  add_executable(
    sawstar_torture
    tests/torture.cpp
  )

  target_link_libraries(
    sawstar_torture
    PRIVATE
      sawstar_engine
  )

  add_test(
    NAME torture
    COMMAND sawstar_torture
      --steps 1200
      --seed 0x53415753544152
  )

  set_tests_properties(
    torture
    PROPERTIES
      LABELS "stress"
      TIMEOUT 180
  )


  add_test(
    NAME torture_reject_negative
    COMMAND sawstar_torture --steps -1
  )

  add_test(
    NAME torture_reject_suffix
    COMMAND sawstar_torture --steps 1200junk
  )

  add_test(
    NAME torture_reject_excess
    COMMAND sawstar_torture --steps 1000001
  )

  set_tests_properties(
    torture_reject_negative
    torture_reject_suffix
    torture_reject_excess
    PROPERTIES
      WILL_FAIL TRUE
      TIMEOUT 5
      LABELS "stress;cli"
  )

endif()

if(BUILD_TESTING AND (SAWSTAR_CHECK_DAISYSP OR SAWSTAR_BUILD_PLUGIN))
  add_executable(sawstar_editor_midi_tracker_tests tests/editor_midi_tracker.cpp)
  target_link_libraries(sawstar_editor_midi_tracker_tests PRIVATE sawstar_engine)
  target_include_directories(sawstar_editor_midi_tracker_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
  target_compile_features(sawstar_editor_midi_tracker_tests PRIVATE cxx_std_17)
  add_test(NAME editor_midi_tracker_counts_and_resets COMMAND sawstar_editor_midi_tracker_tests)

  add_executable(sawstar_editor_wheel_mailbox_tests tests/editor_wheel_mailbox.cpp)
  target_link_libraries(sawstar_editor_wheel_mailbox_tests PRIVATE sawstar_engine Threads::Threads)
  add_test(NAME editor_wheel_mailbox COMMAND sawstar_editor_wheel_mailbox_tests)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  add_test(NAME premium_filter_preview_contract
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/premium_filter_preview.py"
      $<TARGET_FILE:sawstar_premium_filter_preview>)
  execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/host_midi_hook.py"
    "${CMAKE_CURRENT_BINARY_DIR}/host_midi_hook.inc" RESULT_VARIABLE host_midi_hook_result)
  if(NOT host_midi_hook_result EQUAL 0)
    message(FATAL_ERROR "Cannot extract/verify the production VST3 MIDI controller method")
  endif()
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    tests/host_midi_hook.py tests/fixtures/iplug2_parameter_changes.inc)
  add_executable(sawstar_host_midi_points_tests tests/host_midi_points.cpp)
  target_compile_features(sawstar_host_midi_points_tests PRIVATE cxx_std_17)
  target_include_directories(sawstar_host_midi_points_tests PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
  add_test(NAME host_midi_points COMMAND sawstar_host_midi_points_tests)
  add_executable(sawstar_editor_transport_tests tests/editor_transport.cpp)
  target_link_libraries(sawstar_editor_transport_tests PRIVATE sawstar_engine)
  add_test(NAME editor_transport COMMAND sawstar_editor_transport_tests)
  execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/editor_overflow_hook.py"
    "${CMAKE_CURRENT_BINARY_DIR}/editor_overflow_hook.inc" RESULT_VARIABLE overflow_hook_result)
  if(NOT overflow_hook_result EQUAL 0)
    message(FATAL_ERROR "Cannot extract production overflow hook")
  endif()
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS tests/editor_overflow_hook.py)
  add_executable(sawstar_editor_overflow_lifecycle_tests tests/editor_overflow_lifecycle.cpp)
  target_link_libraries(sawstar_editor_overflow_lifecycle_tests PRIVATE sawstar_engine)
  target_include_directories(sawstar_editor_overflow_lifecycle_tests PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
  add_test(NAME editor_overflow_lifecycle COMMAND sawstar_editor_overflow_lifecycle_tests)

  add_executable(sawstar_editor_wheel_gesture_tests tests/editor_wheel_gesture.cpp)
  target_link_libraries(sawstar_editor_wheel_gesture_tests PRIVATE sawstar_engine)
  target_include_directories(sawstar_editor_wheel_gesture_tests BEFORE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests/gui_stubs")
  add_test(NAME editor_wheel_gesture COMMAND sawstar_editor_wheel_gesture_tests)

  execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/tail_report_hook.py"
    "${CMAKE_CURRENT_BINARY_DIR}/tail_report_hook.inc" RESULT_VARIABLE tail_hook_result)
  if(NOT tail_hook_result EQUAL 0)
    message(FATAL_ERROR "Cannot extract the production host tail report")
  endif()
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS tests/tail_report_hook.py src/plugin/SAWSTAR.cpp)
  add_executable(sawstar_tail_contract_tests tests/tail_contract.cpp)
  target_link_libraries(sawstar_tail_contract_tests PRIVATE sawstar_engine)
  target_include_directories(sawstar_tail_contract_tests PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
  option(SAWSTAR_EXTENDED_TAIL_TESTS "Run complete tail residual renders in Debug tests" OFF)
  target_compile_definitions(sawstar_tail_contract_tests PRIVATE
    "$<$<OR:$<BOOL:${SAWSTAR_EXTENDED_TAIL_TESTS}>,$<NOT:$<CONFIG:Debug>>>:SAWSTAR_EXTENDED_TAIL_RENDER>")
  add_test(NAME tail_contract COMMAND sawstar_tail_contract_tests)
  set_tests_properties(tail_contract PROPERTIES LABELS "tail;render")

  foreach(case glide_history release_order idle_voice_reuse output_gain detune_continuity delay_initial_time block_midi_queue engine_controls controller_lifecycle performance_policy)
    add_executable(sawstar_${case}_tests tests/${case}.cpp)
    target_link_libraries(sawstar_${case}_tests PRIVATE sawstar_engine)
    add_test(NAME ${case} COMMAND sawstar_${case}_tests)
  endforeach()
endif()

# Exercise the actual benchmark CSV parser without building its baseline engine.
if(BUILD_TESTING)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/parameter_registration_hook.py"
    "${CMAKE_CURRENT_BINARY_DIR}/parameter_registration_hook.inc" RESULT_VARIABLE registration_hook_result)
  if(NOT registration_hook_result EQUAL 0)
    message(FATAL_ERROR "Cannot extract the production parameter registration")
  endif()
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    tests/parameter_registration_hook.py src/plugin/SAWSTAR.cpp)
  add_executable(sawstar_parameter_registration_tests tests/parameter_registration.cpp)
  target_link_libraries(sawstar_parameter_registration_tests PRIVATE SAWSTAR::Foundation)
  target_include_directories(sawstar_parameter_registration_tests PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
  add_test(NAME parameter_registration_contract COMMAND sawstar_parameter_registration_tests)
  execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/editor_reset_hook.py"
    "${CMAKE_CURRENT_BINARY_DIR}/editor_reset_hook.inc" RESULT_VARIABLE reset_hook_result)
  if(NOT reset_hook_result EQUAL 0)
    message(FATAL_ERROR "Cannot extract/verify the production editor reset hook")
  endif()
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    scripts/patch-iplug2-midi-overflow.py tests/editor_reset_hook.py src/plugin/SAWSTAR.cpp)
  add_executable(sawstar_editor_reset_tests tests/editor_reset.cpp)
  target_include_directories(sawstar_editor_reset_tests PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
  target_compile_features(sawstar_editor_reset_tests PRIVATE cxx_std_17)
  add_test(NAME editor_reset_queue COMMAND sawstar_editor_reset_tests)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/iPlug2/IPlug/IPlugQueue.h")
    add_executable(sawstar_editor_reset_iplug_tests tests/editor_reset_iplug.cpp)
    target_compile_features(sawstar_editor_reset_iplug_tests PRIVATE cxx_std_17)
    target_include_directories(sawstar_editor_reset_iplug_tests PRIVATE
      "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/third_party/iPlug2/IPlug"
      "${CMAKE_CURRENT_SOURCE_DIR}/third_party/iPlug2/WDL")
    add_test(NAME editor_reset_iplug_queue COMMAND sawstar_editor_reset_iplug_tests)
  endif()
  add_test(NAME iplug2_midi_patch
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/iplug2_midi_patch.py")
  add_test(NAME build_display_metadata
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/build_display_metadata.py" "${CMAKE_COMMAND}")
  add_test(NAME release_guard
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/release_guard.py")
  add_test(NAME benchmark_report
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/benchmark_report.py")
endif()

if(BUILD_TESTING)
  # Header-only ring study also works without DaisySP or the plugin wrapper.
  add_executable(sawstar_premium_unrolled_tests tests/premium_drive_unrolled.cpp)
  target_link_libraries(sawstar_premium_unrolled_tests PRIVATE sawstar_foundation)
  add_test(NAME premium_unrolled_bit_identity COMMAND sawstar_premium_unrolled_tests)
  add_executable(sawstar_premium_deadline_timeline_tests tests/premium_deadline_timeline.cpp)
  target_link_libraries(sawstar_premium_deadline_timeline_tests PRIVATE sawstar_foundation)
  add_test(NAME premium_deadline_timeline COMMAND sawstar_premium_deadline_timeline_tests)
  add_executable(sawstar_premium_lookup_tests tests/premium_lookup_tanh.cpp)
  target_link_libraries(sawstar_premium_lookup_tests PRIVATE sawstar_foundation)
  add_test(NAME premium_lookup_scalar_contract COMMAND sawstar_premium_lookup_tests)
  add_executable(sawstar_premium_lookup_drive_tests tests/premium_drive_tanh.cpp)
  target_compile_definitions(sawstar_premium_lookup_drive_tests PRIVATE SAWSTAR_LOOKUP_TANH_STUDY=1)
  target_link_libraries(sawstar_premium_lookup_drive_tests PRIVATE sawstar_foundation)
  add_test(NAME premium_lookup_drive_contract COMMAND sawstar_premium_lookup_drive_tests)
  add_test(NAME premium_lookup_report
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/report-premium-lookup.py" --self-test)
  add_executable(sawstar_premium_ring_study experiments/premium_filter/benchmark_ring_storage.cpp)
  target_link_libraries(sawstar_premium_ring_study PRIVATE sawstar_foundation)
  add_test(NAME premium_drive_ring_identity COMMAND sawstar_premium_ring_study)
  add_test(NAME premium_drive_ring_report
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/report-premium-ring.py" --self-test)
  add_executable(sawstar_premium_gain_tests tests/premium_gain_normalization.cpp)
  target_link_libraries(sawstar_premium_gain_tests PRIVATE sawstar_foundation)
  if(TARGET sawstar_engine)
    target_link_libraries(sawstar_premium_gain_tests PRIVATE sawstar_engine)
    target_compile_definitions(sawstar_premium_gain_tests PRIVATE SAWSTAR_GAIN_ADAPTER_CONTRACT=1)
  endif()
  add_test(NAME premium_gain_normalization COMMAND sawstar_premium_gain_tests)
  add_test(NAME premium_gain_report
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/report-premium-ring.py" --self-test --normalization)
endif()

if(BUILD_TESTING)
  add_test(NAME premium_deadline_report
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/premium_deadline_report.py")
  add_test(NAME premium_components_report
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/report-premium-components.py" --self-test)
endif()

