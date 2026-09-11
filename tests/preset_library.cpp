// SPDX-License-Identifier: MIT
#include "presets/Library.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <atomic>
int main(){using namespace sawstar;auto root=fs::temp_directory_path()/("sawstar-library-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));try{
 auto check=[](bool b){if(!b)throw std::runtime_error("Preset library regression failed");};auto source=root/"source",dest=root/"library";auto values=DefaultSnapshot();values[91]=88;
 std::vector<fs::path> files;for(int i=0;i<30;++i){auto p=source/("Sound "+std::to_string(i)+".sawstar");auto unique=values;unique[91]=88-i;SaveUserPreset(p,unique);files.push_back(p);}
 auto result=ImportPresets(files,dest);check(result.imported==30&&result.failed==0&&ListUserPresets(dest).size()==30);check(ReadUserPreset(dest/"Sound 0.sawstar")==values);
 auto modified=DefaultSnapshot();SaveUserPreset(root/"collision"/"Sound 0.sawstar",modified);{std::ofstream out(source/"broken.sawstar");out<<"bad";}
 result=ImportPresets({files[1],root/"collision"/"Sound 0.sawstar",source/"broken.sawstar",source/"missing.sawstar"},dest);check(result.skipped==2&&result.failed==2&&ReadUserPreset(dest/"Sound 0.sawstar")==values);
 SaveUserPreset(source/"Copy.sawstar",values);auto duplicate=ImportPresets({source/"Copy.sawstar"},dest);check(duplicate.skipped==1&&duplicate.duplicates.size()==1&&!fs::exists(dest/"Copy.sawstar"));check(ImportPresets(duplicate.duplicates,dest,true).imported==1);fs::remove(dest/"Copy.sawstar");
 PresetLibrary lib(dest);check(lib.Filter("All","").size()==FactoryPresets().size()+30&&lib.Filter("User","").size()==30);check(lib.Filter("User","SOUND 29").size()==1);check(lib.Filter("Lead","").size()==4&&lib.Filter("Templates","").size()==7);check(lib.Filter("FX","").empty());check(!lib.Filter("All","Trance").empty());check(lib.Filter("Bass","Sine").size()==1);
 PresetLibrary second(dest);lib.ToggleFavorite("user:Sound 0.sawstar");second.ToggleFavorite("factory:init");PresetLibrary restored(dest);check(restored.Filter("Favorites","").size()==2);restored.MoveFavorite("user:Sound 0.sawstar","user:Renamed.sawstar");RenameUserPreset(dest/"Sound 0.sawstar","Renamed");restored.Refresh();check(restored.Filter("Favorites","").size()==2);ArchiveUserPreset(dest/"Renamed.sawstar");restored.Refresh();check(restored.Filter("Favorites","").size()==1);check(ImportPresets({},dest).imported==0);
 // Concurrent imports with different names but identical settings must deduplicate.
 auto concurrent=root/"concurrent";std::atomic<int> imported{0},failed{0};
 std::vector<std::thread> workers;
 for(int i=0;i<12;++i){auto file=source/("Concurrent "+std::to_string(i)+".sawstar");SaveUserPreset(file,values);
 workers.emplace_back([&,file]{try{auto report=ImportPresets({file},concurrent);imported+=report.imported;failed+=report.failed;}catch(...){++failed;}});}
 for(auto& worker:workers)worker.join();
 check(imported==1&&failed==0&&ListUserPresets(concurrent).size()==1);
 check(ReadUserPreset(ListUserPresets(concurrent).front())==values);
 fs::remove_all(root);std::cout<<"Batch import, collision safety, filters and favorite persistence passed\n";
 }catch(const std::exception& e){fs::remove_all(root);std::cerr<<e.what()<<'\n';return 1;}}
