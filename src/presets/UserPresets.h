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
namespace sawstar {
namespace fs=std::filesystem;
inline fs::path UserPresetFolder(){
#ifdef _WIN32
 const char* home=std::getenv("APPDATA");
 return home?fs::path(home)/"SAWSTAR"/"Presets":fs::path{};
#else
 const char* home=std::getenv("HOME");
 return home?fs::path(home)/"Library"/"Application Support"/"SAWSTAR"/"Presets":fs::path{};
#endif
}
inline bool ValidPresetName(const std::string& s){return !s.empty()&&s.size()<=80&&s!="."&&s!=".."&&s.back()!='.'&&s.back()!=' '&&s.find_first_of("/\\:*?\"<>|")==std::string::npos&&std::none_of(s.begin(),s.end(),[](unsigned char c){return c<32;});}
inline Snapshot ReadUserPreset(const fs::path& path){
 std::ifstream in(path,std::ios::binary|std::ios::ate);if(!in)throw std::runtime_error("Cannot open preset.");auto size=in.tellg();if(size<=0||size>4096)throw std::runtime_error("Invalid preset size.");
 std::vector<uint8_t> bytes(static_cast<size_t>(size));in.seekg(0);if(!in.read(reinterpret_cast<char*>(bytes.data()),size))throw std::runtime_error("Cannot read preset.");Snapshot result{};if(!DecodeState(bytes.data(),bytes.size(),result))throw std::runtime_error("Not a supported SAWSTAR preset.");return result;
}
inline void SaveUserPreset(const fs::path& path,const Snapshot& values){
 if(fs::exists(path))throw std::runtime_error("That name already exists. Choose a new name.");
 fs::create_directories(path.parent_path());auto bytes=EncodeState(values);std::ofstream out(path,std::ios::binary);if(!out)throw std::runtime_error("Cannot create preset.");out.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());out.close();if(!out){fs::remove(path);throw std::runtime_error("Preset write failed.");}
}
inline std::vector<fs::path> ListUserPresets(const fs::path& folder){std::vector<fs::path> result;if(!fs::exists(folder))return result;for(const auto& f:fs::directory_iterator(folder))if(f.is_regular_file()&&f.path().extension()==".sawstar")result.push_back(f.path());std::sort(result.begin(),result.end());return result;}
inline fs::path RenameUserPreset(const fs::path& source,const std::string& name){if(!ValidPresetName(name))throw std::runtime_error("Use a name of 1-80 characters without path symbols.");auto target=source.parent_path()/fs::u8path(name+".sawstar");if(target==source)return source;if(fs::exists(target))throw std::runtime_error("That name already exists.");fs::rename(source,target);return target;}
inline fs::path ArchiveUserPreset(const fs::path& source){auto target=source;target+=".deleted";for(int i=1;fs::exists(target);++i){target=source;target+=".deleted-"+std::to_string(i);}fs::rename(source,target);return target;}
struct UserPresetSelection {fs::path path;std::string name="My Sound",status;Snapshot saved{};bool active=false;};
}
