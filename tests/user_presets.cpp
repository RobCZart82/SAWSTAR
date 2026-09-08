// SPDX-License-Identifier: MIT
#include "presets/UserPresets.h"
#include <iostream>
#include <chrono>
int main(){using namespace sawstar;auto root=fs::temp_directory_path()/("sawstar-preset-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));try{
 auto check=[](bool ok){if(!ok)throw std::runtime_error("User preset test failed");};
 check(!ValidPresetName("../test")&&!ValidPresetName("")&&!ValidPresetName("foo/bar"));auto values=DefaultSnapshot();values[90]=1;values[91]=83;auto path=root/"My Sound.sawstar";SaveUserPreset(path,values);check(ReadUserPreset(path)==values);check(ListUserPresets(root).size()==1);
 bool rejected=false;try{SaveUserPreset(path,DefaultSnapshot());}catch(...){rejected=true;}check(rejected&&ReadUserPreset(path)==values);
 path=RenameUserPreset(path,"New Sound");check(ReadUserPreset(path)==values&&!fs::exists(root/"My Sound.sawstar"));auto backup=ArchiveUserPreset(path);check(ListUserPresets(root).empty()&&ReadUserPreset(backup)==values);
 auto bad=root/"broken.sawstar";{std::ofstream out(bad);out<<"not a preset";}rejected=false;try{ReadUserPreset(bad);}catch(...){rejected=true;}check(rejected);fs::remove_all(root);std::cout<<"User preset lifecycle passed\n";
 }catch(const std::exception& e){fs::remove_all(root);std::cerr<<e.what()<<'\n';return 1;} }
