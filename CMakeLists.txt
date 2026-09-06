cmake_minimum_required(VERSION 3.21)
project(SAWSTAR VERSION 0.1.0 LANGUAGES CXX)

option(SAWSTAR_BUILD_PLUGIN "Build the iPlug2 VST3 development shell" OFF)
if(SAWSTAR_BUILD_PLUGIN)
  enable_language(C)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

include(CTest)
option(SAWSTAR_CHECK_DAISYSP "Compile and test the selected DaisySP primitives" OFF)

add_library(sawstar_foundation STATIC src/plugin/Parameters.cpp src/plugin/State.cpp)
add_library(SAWSTAR::Foundation ALIAS sawstar_foundation)
target_include_directories(sawstar_foundation PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_features(sawstar_foundation PUBLIC cxx_std_17)
set_target_properties(sawstar_foundation PROPERTIES
  CXX_EXTENSIONS OFF POSITION_INDEPENDENT_CODE ON)
if(MSVC)
  target_compile_options(sawstar_foundation PRIVATE /W4 /permissive-)
else()
  target_compile_options(sawstar_foundation PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
  add_executable(sawstar_state_tests tests/state.cpp)
  target_link_libraries(sawstar_state_tests PRIVATE SAWSTAR::Foundation)
  add_test(NAME state_compatibility COMMAND sawstar_state_tests)
  add_executable(sawstar_parameter_tests tests/parameters.cpp)
  target_link_libraries(sawstar_parameter_tests PRIVATE SAWSTAR::Foundation)
  add_test(NAME parameter_contract COMMAND sawstar_parameter_tests)
endif()

if(SAWSTAR_CHECK_DAISYSP OR SAWSTAR_BUILD_PLUGIN)
  set(_daisy "${CMAKE_CURRENT_SOURCE_DIR}/third_party/DaisySP")
  if(NOT EXISTS "${_daisy}/Source/Synthesis/oscillator.cpp")
    message(FATAL_ERROR
      "DaisySP is missing. Run: git submodule update --init third_party/DaisySP")
  endif()
  # Deliberately avoid the omnibus header and optional DaisySP-LGPL sources.
  add_library(sawstar_daisy_primitives STATIC
    "${_daisy}/Source/Synthesis/oscillator.cpp"
    "${_daisy}/Source/Control/adsr.cpp")
  target_include_directories(sawstar_daisy_primitives SYSTEM PUBLIC
    "${_daisy}/Source"
    PRIVATE "${_daisy}/Source/Synthesis" "${_daisy}/Source/Control"
            "${_daisy}/Source/Utility")
  target_compile_features(sawstar_daisy_primitives PUBLIC cxx_std_17)
  if(MSVC)
    # DaisySP ADSR uses M_E; MSVC exposes it only with this opt-in.
    target_compile_definitions(sawstar_daisy_primitives PRIVATE _USE_MATH_DEFINES)
  endif()
  set_target_properties(sawstar_daisy_primitives PROPERTIES
    CXX_EXTENSIONS OFF POSITION_INDEPENDENT_CODE ON)
  add_library(sawstar_engine STATIC src/engine/Synth.cpp)
  target_link_libraries(sawstar_engine PUBLIC sawstar_foundation sawstar_daisy_primitives)
  set_target_properties(sawstar_engine PROPERTIES POSITION_INDEPENDENT_CODE ON)
  if(BUILD_TESTING)
    add_executable(sawstar_engine_tests tests/engine.cpp)
    target_link_libraries(sawstar_engine_tests PRIVATE sawstar_engine)
    add_test(NAME voice_engine COMMAND sawstar_engine_tests)
    add_executable(sawstar_dsp_tests tests/daisy_primitives.cpp)
    target_link_libraries(sawstar_dsp_tests PRIVATE sawstar_daisy_primitives)
    add_test(NAME daisy_primitives COMMAND sawstar_dsp_tests)
  endif()
endif()

if(SAWSTAR_BUILD_PLUGIN)
  add_subdirectory(src/plugin)
endif()
