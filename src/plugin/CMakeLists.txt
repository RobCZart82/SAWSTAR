set(IPLUG2_DIR "${PROJECT_SOURCE_DIR}/third_party/iPlug2")
if(NOT EXISTS "${IPLUG2_DIR}/iPlug2.cmake")
  message(FATAL_ERROR "Run python scripts/bootstrap-plugin.py before configuring the plugin")
endif()
if(NOT EXISTS "${IPLUG2_DIR}/Dependencies/IPlug/VST3_SDK/public.sdk/source/main/pluginfactory.cpp")
  message(FATAL_ERROR "The pinned VST3 SDK is missing. Run python scripts/bootstrap-plugin.py")
endif()
set(IPLUG_DEPLOY_PLUGINS OFF CACHE BOOL "Do not install plugins during build")
set(IGRAPHICS_BACKEND NANOVG)
if(APPLE)
  set(IGRAPHICS_RENDERER METAL)
else()
  set(IGRAPHICS_RENDERER GL2)
endif()
include(${IPLUG2_DIR}/iPlug2.cmake)
find_package(iPlug2 REQUIRED)
iplug_add_plugin(SAWSTAR
  FORMATS VST3
  SOURCES SAWSTAR.cpp SAWSTAR.h config.h ../gui/Controls/PageButton.h
  LINK SAWSTAR::Foundation sawstar_engine)
set_target_properties(SAWSTAR-vst3 PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/out"
  LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/out")
# Keep the SDK's Windows bundle layout (the generic properties above are macOS only).
if(WIN32)
  set_target_properties(SAWSTAR-vst3 PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/out/SAWSTAR.vst3/Contents/x86_64-win"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/out/SAWSTAR.vst3/Contents/x86_64-win")
endif()

# Embed the original creator PNG so About needs no external asset installation.
set(_logo "${PROJECT_SOURCE_DIR}/assets/branding/GYR-Logo1.png")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_logo}")
file(READ "${_logo}" _logo_hex HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," _logo_bytes "${_logo_hex}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/CreatorLogo.h"
  "#pragma once\nnamespace sawstar::gui { inline constexpr unsigned char CreatorLogo[]={${_logo_bytes}}; }\n")
target_include_directories(SAWSTAR-vst3 PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

if(APPLE)
  target_sources(SAWSTAR-vst3 PRIVATE ../gui/NativeImport.mm)
else()
  target_sources(SAWSTAR-vst3 PRIVATE ../gui/NativeImport.cpp)
  target_link_libraries(SAWSTAR-vst3 PRIVATE comdlg32)
endif()
