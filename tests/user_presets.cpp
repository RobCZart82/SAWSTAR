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
 // Save overwrites only the loaded version and preserves a recoverable original.
 auto overwrite=root/fs::u8path("Mentett ő.sawstar");SaveUserPreset(overwrite,values);
 auto changed=values;changed[8]=1234;auto recovery=OverwriteUserPreset(overwrite,values,changed);
 check(ReadUserPreset(overwrite)==changed&&ReadUserPreset(recovery)==values);
 rejected=false;try{OverwriteUserPreset(overwrite,values,DefaultSnapshot());}catch(...){rejected=true;}
 check(rejected&&ReadUserPreset(overwrite)==changed);
 // Two instances loaded the same version; exactly one may replace it.
 std::atomic<int> readySave{0};std::atomic<bool> goSave{false};std::array<bool,2> won{};std::vector<std::thread> savers;
 for(int i=0;i<2;++i)savers.emplace_back([&,i]{auto next=changed;next[8]=2000+i*1000;++readySave;while(!goSave.load())std::this_thread::yield();try{OverwriteUserPreset(overwrite,changed,next);won[i]=true;}catch(const std::exception&){} });
 while(readySave.load()!=2)std::this_thread::yield();goSave=true;for(auto& t:savers)t.join();check(won[0]!=won[1]);
 check(ReadUserPreset(overwrite)[8]==(won[0]?2000:3000));
 // Backup preparation failure must leave the target and no temporary update.
 auto blocked=root/"blocked";SaveUserPreset(blocked/"Sound.sawstar",values);{std::ofstream f(blocked/".sawstar-backups");f<<"blocked";}
 rejected=false;try{OverwriteUserPreset(blocked/"Sound.sawstar",values,changed);}catch(...){rejected=true;}check(rejected&&ReadUserPreset(blocked/"Sound.sawstar")==values);
 for(const auto& f:fs::directory_iterator(blocked))check(f.path().extension()!=".tmp");
 rejected=false;try{OverwriteUserPreset(root/"missing.sawstar",values,changed);}catch(...){rejected=true;}check(rejected&&!fs::exists(root/"missing.sawstar"));
 // Save/Rename/Delete share one transaction order; no resurrected old name.
 for(bool archive:{false,true})for(int round=0;round<32;++round){
  auto dir=root/(std::string(archive?"delete":"rename")+std::to_string(round));
  auto source=dir/"Old.sawstar";SaveUserPreset(source,values);
  std::atomic<bool> go{false};bool saved=false,moved=false;fs::path destination;
  std::thread save([&]{while(!go.load())std::this_thread::yield();try{OverwriteUserPreset(source,values,changed);saved=true;}catch(const std::exception&){} });
  std::thread move([&]{while(!go.load())std::this_thread::yield();try{destination=archive?ArchiveUserPreset(source):RenameUserPreset(source,"New");moved=true;}catch(const std::exception&){} });
  go=true;save.join();move.join();check(moved&&!fs::exists(source));
  check(ReadUserPreset(destination)==(saved?changed:values));
 }
 auto bad=root/"broken.sawstar";{std::ofstream out(bad);out<<"not a preset";}rejected=false;try{ReadUserPreset(bad);}catch(...){rejected=true;}check(rejected);fs::remove_all(root);std::cout<<"User preset lifecycle passed\n";
 }catch(const std::exception& e){fs::remove_all(root);std::cerr<<e.what()<<'\n';return 1;} }
