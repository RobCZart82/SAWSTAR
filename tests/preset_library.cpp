// SPDX-License-Identifier: MIT
#include "presets/Library.h"
#include "presets/QuickPresetList.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <atomic>
int main(){using namespace sawstar;auto root=fs::temp_directory_path()/("sawstar-library-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));try{
 auto check=[](bool b){if(!b)throw std::runtime_error("Preset library regression failed");};auto source=root/"source",dest=root/"library";auto values=DefaultSnapshot();values[91]=88;
 // Quick-menu display order must retain the exact factory ID or user path.
 const std::vector<fs::path> quickFiles{source/"zebra.sawstar",source/"Alpha.sawstar",source/"alpha.sawstar",source/(std::string(FactoryPresets()[1].name)+".sawstar"),source/fs::u8path(u8"\u0150r.sawstar")};
 const auto quick=BuildQuickPresetList(quickFiles);check(quick.size()==FactoryPresets().size()-1+quickFiles.size());
 for(size_t i=1;i<quick.size();++i)check(Fold(quick[i-1].name)<=Fold(quick[i].name));
 auto reversed=quickFiles;std::reverse(reversed.begin(),reversed.end());const auto repeat=BuildQuickPresetList(reversed);
 for(size_t i=0;i<quick.size();++i)check(quick[i].factory==repeat[i].factory&&quick[i].path==repeat[i].path);
 for(int id=1;id<int(FactoryPresets().size());++id){auto position=FindQuickPreset(quick,id,false,{});check(position>=0&&quick[position].factory==id&&quick[position].name==FactoryPresets()[id].name&&quick[position].Label()==std::string(FactoryPresets()[id].name)+" ["+FactoryPresets()[id].category+"]");}
 for(const auto& path:quickFiles){auto position=FindQuickPreset(quick,1,true,path);check(position>=0&&quick[position].factory==-1&&quick[position].path==path&&quick[position].Label()==path.stem().u8string()+" [User]");}
 check(FindQuickPreset(quick,0,false,{})==-1&&FindQuickPreset(quick,-1,false,{})==-1&&FindQuickPreset(quick,1,true,source/"external.sawstar")==-1);
 const int count=int(quick.size());check(StepQuickPreset(0,-1,count)==count-1&&StepQuickPreset(count-1,1,count)==0&&StepQuickPreset(-1,1,count)==0&&StepQuickPreset(-1,-1,count)==count-1&&StepQuickPreset(0,1,0)==-1);
 for(int start=0;start<count;++start){auto position=start;for(int i=0;i<count;++i)position=StepQuickPreset(position,1,count);check(position==start);}
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
