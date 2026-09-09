// SPDX-License-Identifier: MIT
#include "presets/UserPresets.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
int main(){using namespace sawstar;auto root=fs::temp_directory_path()/("sawstar-preset-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));try{
 auto check=[](bool ok){if(!ok)throw std::runtime_error("User preset test failed");};
 check(!ValidPresetName("../test")&&!ValidPresetName("")&&!ValidPresetName("foo/bar"));auto values=DefaultSnapshot();values[90]=1;values[91]=83;auto path=root/"My Sound.sawstar";SaveUserPreset(path,values);check(ReadUserPreset(path)==values);check(ListUserPresets(root).size()==1);
 bool rejected=false;try{SaveUserPreset(path,DefaultSnapshot());}catch(...){rejected=true;}check(rejected&&ReadUserPreset(path)==values);
 path=RenameUserPreset(path,"New Sound");check(ReadUserPreset(path)==values&&!fs::exists(root/"My Sound.sawstar"));auto backup=ArchiveUserPreset(path);check(ListUserPresets(root).empty()&&ReadUserPreset(backup)==values);
 // Independent writers must never both report success for the same filename.
 for(int round=0;round<32;++round){
  auto target=root/fs::u8path("Shared Á sound "+std::to_string(round)+".sawstar");
  std::atomic<int> ready{0};std::atomic<bool> go{false};std::array<bool,8> saved{};
  std::vector<std::thread> writers;
  for(int i=0;i<8;++i)writers.emplace_back([&,i]{auto v=values;v[0]=-double(i+1);++ready;while(!go.load())std::this_thread::yield();try{SaveUserPreset(target,v);saved[i]=true;}catch(const std::exception&){} });
  while(ready.load()!=8)std::this_thread::yield();go=true;
  for(auto& writer:writers)writer.join();
  check(std::count(saved.begin(),saved.end(),true)==1);
  auto expected=values;expected[0]=-double(std::find(saved.begin(),saved.end(),true)-saved.begin()+1);
  check(ReadUserPreset(target)==expected);
 }
 auto bad=root/"broken.sawstar";{std::ofstream out(bad);out<<"not a preset";}rejected=false;try{ReadUserPreset(bad);}catch(...){rejected=true;}check(rejected);fs::remove_all(root);std::cout<<"User preset lifecycle passed\n";
 }catch(const std::exception& e){fs::remove_all(root);std::cerr<<e.what()<<'\n';return 1;} }
