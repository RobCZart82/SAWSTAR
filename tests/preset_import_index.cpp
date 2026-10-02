// SPDX-License-Identifier: MIT
#include "presets/Library.h"
#include <chrono>
#include <iostream>
using namespace sawstar;
void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){
 const auto root=fs::temp_directory_path()/("sawstar-import-index-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 try{
  auto library=root/"library",source=root/"source";auto initial=DefaultSnapshot();initial[0]=-20;
  auto unique=initial;unique[0]=-19;auto close=unique;close[0]+=1e-12;
  SaveUserPreset(library/"Alpha.sawstar",initial);SaveUserPreset(library/"Beta.sawstar",initial);
  fs::create_directories(source);
  {std::ofstream broken(library/"Damaged.sawstar");broken<<"broken";}
  auto input=[&](const char* name,const Snapshot& values){auto path=source/name;SaveUserPreset(path,values);return path;};
  auto collision=input("ALPHA.sawstar",unique);
  auto damagedName=input("Damaged.sawstar",unique);
  auto existingCopy=input("Existing Copy.sawstar",initial);
  auto first=input("New.sawstar",unique),copy=input("New Copy.sawstar",unique);
  auto near=input("Near.sawstar",close);
  {std::ofstream broken(source/"Broken.sawstar");broken<<"broken";}
  auto report=ImportPresets({collision,damagedName,existingCopy,first,copy,near,source/"Broken.sawstar",source/"Missing.sawstar"},library);
  check(report.imported==2&&report.skipped==4&&report.failed==2,"Mixed batch report changed");
  check(report.duplicates==std::vector<fs::path>({existingCopy,copy}),"Duplicate retry list changed");
  check(report.details[2].find("same settings as Alpha")!=std::string::npos,"Existing duplicate chose a different first sorted name");
  check(ReadUserPreset(library/"Alpha.sawstar")==initial,"Case-insensitive collision overwrote a preset");
  check(ReadUserPreset(library/"New.sawstar")==unique&&ReadUserPreset(library/"Near.sawstar")==close,"Exact snapshot comparison became approximate");
  check(!fs::exists(library/"New Copy.sawstar"),"Batch did not index newly imported settings");
  check(ImportPresets(report.duplicates,library,true).imported==2,"Explicit identical-copy retry failed");
  check(ImportPresets({copy},library,true).skipped==1,"Copy retry bypassed filename collision protection");
  // Failed exclusive creation must not reserve a sound in the batch index.
  auto fresh=initial;fresh[0]=-18;
  fs::create_directory(library/"Blocked.sawstar");
  auto blocked=input("Blocked.sawstar",fresh),recovery=input("Recovered.sawstar",fresh);
  auto recoveryCopy=input("Recovered Copy.sawstar",fresh);
  report=ImportPresets({blocked,recovery,recoveryCopy},library);
  check(report.failed==1&&report.imported==1&&report.skipped==1,"Failed save poisoned subsequent duplicate detection");
  check(report.duplicates==std::vector<fs::path>({recoveryCopy})&&ReadUserPreset(library/"Recovered.sawstar")==fresh,"Recovery batch contents changed");
  // A new call rebuilds its index and observes changes made between imports.
  ArchiveUserPreset(library/"Recovered.sawstar");
  check(ImportPresets({recoveryCopy},library).imported==1,"Index leaked across import calls");
  auto sameA=root/"a"/"Same.sawstar",sameB=root/"b"/"same.sawstar";
  SaveUserPreset(sameA,fresh);SaveUserPreset(sameB,initial);
  auto names=root/"names";report=ImportPresets({sameA,sameB},names,true);
  check(report.imported==1&&report.skipped==1&&ReadUserPreset(names/"Same.sawstar")==fresh,"Same-batch folded filename collision changed");
#if defined(__APPLE__) || defined(_WIN32)
  auto unicodeA=root/"unicode-a"/fs::u8path(u8"\u00c9cho \u0150r.sawstar");
  auto unicodeB=root/"unicode-b"/fs::u8path(u8"e\u0301cho \u0151r.sawstar");
  SaveUserPreset(unicodeA,fresh);SaveUserPreset(unicodeB,initial);
  auto unicodeLibrary=root/"unicode-library";report=ImportPresets({unicodeA,unicodeB},unicodeLibrary,true);
  check(report.imported==1&&report.skipped==1&&ListUserPresets(unicodeLibrary).size()==1,"Batch index lost Unicode name collision policy");
#endif
  auto empty=ImportPresets({},library);check(empty.imported==0&&empty.skipped==0&&empty.failed==0,"Empty batch changed");
  fs::remove_all(root);std::cout<<"Batch index preserves collisions, exact duplicates, failures and retry semantics.\n";
 }catch(const std::exception& e){fs::remove_all(root);std::cerr<<e.what()<<'\n';return 1;}
}
