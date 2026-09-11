// SPDX-License-Identifier: MIT
#pragma once
#include <string>
#include "plugin/State.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include <memory>
namespace sawstar {
namespace fs=std::filesystem;
inline fs::path UserPresetFolder(){
#ifdef _WIN32
 wchar_t* value=nullptr;
 if(::_wdupenv_s(&value,nullptr,L"APPDATA")!=0)return {};
 std::unique_ptr<wchar_t,decltype(&std::free)> home(value,&std::free);
 return home?fs::path(home.get())/"SAWSTAR"/"Presets":fs::path{};
#else
 const char* home=std::getenv("HOME");
 return home?fs::path(home)/"Library"/"Application Support"/"SAWSTAR"/"Presets":fs::path{};
#endif
}
inline std::string Fold(std::string s){for(auto& c:s)if(c>='A'&&c<='Z')c=char(c-'A'+'a');return s;}
bool ValidPresetName(const std::string& name);
inline bool PresetExtension(const fs::path& p){return Fold(p.extension().u8string())==".sawstar";}
inline Snapshot ReadUserPreset(const fs::path& path){
 std::ifstream in(path,std::ios::binary|std::ios::ate);if(!in)throw std::runtime_error("Cannot open preset.");auto size=in.tellg();if(size<=0||size>4096)throw std::runtime_error("Invalid preset size.");
 std::vector<uint8_t> bytes(static_cast<size_t>(size));in.seekg(0);if(!in.read(reinterpret_cast<char*>(bytes.data()),size))throw std::runtime_error("Cannot read preset.");Snapshot result{};if(!DecodeState(bytes.data(),bytes.size(),result))throw std::runtime_error("Not a supported SAWSTAR preset.");return result;
}
// Exclusive creation also protects against other plugin instances saving at once.
void SaveUserPreset(const fs::path& path,const Snapshot& values);
// UI thread only; rejects stale edits and returns the recovery copy path.
fs::path OverwriteUserPreset(const fs::path& path,const Snapshot& expected,const Snapshot& values);
inline std::vector<fs::path> ListUserPresets(const fs::path& folder){std::vector<fs::path> result;if(!fs::exists(folder))return result;for(const auto& f:fs::directory_iterator(folder))if(f.is_regular_file()&&PresetExtension(f.path()))result.push_back(f.path());std::sort(result.begin(),result.end());return result;}
fs::path RenameUserPreset(const fs::path& source,const std::string& name);
fs::path ArchiveUserPreset(const fs::path& source);
struct UserPresetSelection {fs::path path;std::string name="My Sound",status;Snapshot saved{};bool active=false;};
}
