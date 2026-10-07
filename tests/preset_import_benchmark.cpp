// SPDX-License-Identifier: MIT
// Manual filesystem/UI-preparation benchmark. Fixture setup and cleanup are untimed.
#include "presets/Library.h"
#include "presets/QuickPresetList.h"
#include <chrono>
#include <iostream>
#include <iomanip>
using namespace sawstar;
namespace {
using Clock=std::chrono::steady_clock;
void Check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
Snapshot Sound(int i){auto v=DefaultSnapshot();v[0]=-30.+i*.001;return v;}
void Write(const fs::path& path,int i,bool invalid=false){
 std::ofstream out(path,std::ios::binary);Check(bool(out),"Cannot create fixture.");
 if(invalid)out<<"not a preset";else{auto bytes=EncodeState(Sound(i));out.write(reinterpret_cast<const char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()));}
 out.close();Check(bool(out),"Cannot finish fixture.");
}
struct Scratch {
 fs::path path;
 explicit Scratch(const fs::path& parent):path(parent/("import-profile-"+std::to_string(Clock::now().time_since_epoch().count()))){
  // Own only a newly created child of the explicitly supplied scratch folder.
  Check(fs::create_directory(path),"Measurement directory already exists.");
 }
 ~Scratch(){std::error_code ignored;fs::remove_all(path,ignored);}
};
template<typename F> double Measure(F&& operation){
 auto start=Clock::now();operation();return std::chrono::duration<double,std::milli>(Clock::now()-start).count();
}
void Run(const fs::path& parent,int existing,int batch,const std::string& scenario,int repetition){
 Scratch scratch(parent);auto library=scratch.path/"library",source=scratch.path/"source";
 fs::create_directory(library);fs::create_directory(source);
 for(int i=0;i<existing;++i)Write(library/("Existing "+std::to_string(i)+".sawstar"),i);
 std::vector<fs::path> files;
 for(int i=0;i<batch;++i){
  auto p=source/((scenario=="collision"?"Existing ":"New ")+std::to_string(i)+".sawstar");
  Write(p,scenario=="duplicate"?i:existing+i,scenario=="invalid");files.push_back(p);
 }
 PresetLibrary view(library); // Initial construction is untimed.
 ImportReport report;const auto importMs=Measure([&]{report=ImportPresets(files,library);});
 const int imported=scenario=="new"?batch:0;
 const int skipped=scenario=="duplicate"||scenario=="collision"?batch:0;
 const int failed=scenario=="invalid"?batch:0;
 Check(report.imported==imported&&report.skipped==skipped&&report.failed==failed,"Import report differs from fixture.");
 Check(report.duplicates.size()==static_cast<size_t>(scenario=="duplicate"?batch:0),"Duplicate report differs from fixture.");
 const auto refreshMs=Measure([&]{view.Refresh();});Check(view.warning.empty(),"Library refresh failed.");
 std::vector<int> filtered;const auto filterMs=Measure([&]{filtered=view.Filter("User","");});
 std::vector<QuickPresetEntry> quick;const auto quickMs=Measure([&]{quick=BuildQuickPresetList(ListUserPresets(library));});
 Snapshot preview{};const auto previewMs=Measure([&]{preview=ReadUserPreset(library/"Existing 0.sawstar");});
 Check(preview==Sound(0)&&ReadUserPreset(library/("Existing "+std::to_string(existing-1)+".sawstar"))==Sound(existing-1),"Existing preset changed.");
 Check(filtered.size()==static_cast<size_t>(existing+imported)&&quick.size()==FactoryPresets().size()-1+filtered.size(),"Library count differs from fixture.");
 if(imported)for(int i=0;i<batch;++i)Check(ReadUserPreset(library/("New "+std::to_string(i)+".sawstar"))==Sound(existing+i),"Imported sound changed.");
 std::cout<<existing<<','<<batch<<','<<scenario<<','<<repetition<<','<<importMs<<','<<refreshMs<<','<<filterMs<<','<<quickMs<<','<<previewMs<<','<<report.imported<<','<<report.skipped<<','<<report.failed<<'\n';
}
}
int main(int argc,char** argv){try{
 if(argc<2||argc>3)throw std::runtime_error("Usage: sawstar_preset_import_benchmark SCRATCH_DIR [REPETITIONS=3]");
 int repeats=3;if(argc==3){size_t consumed=0;repeats=std::stoi(argv[2],&consumed);Check(consumed==std::string(argv[2]).size()&&repeats>=1&&repeats<=20,"Repetitions must be 1..20.");}
 auto parent=fs::absolute(fs::u8path(argv[1])).lexically_normal();Check(fs::is_directory(parent),"Scratch folder must already exist.");
 std::cout<<"existing,batch,scenario,repetition,import_ms,refresh_ms,filter_ms,quick_list_ms,preview_ms,imported,skipped,failed\n"<<std::fixed<<std::setprecision(6);
 for(int existing:{100,1000})for(int repetition=1;repetition<=repeats;++repetition){
  for(int batch:{10,100})for(const auto* scenario:{"new","duplicate","collision","invalid"})Run(parent,existing,batch,scenario,repetition);
  Run(parent,existing,0,"empty",repetition);
 }
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
