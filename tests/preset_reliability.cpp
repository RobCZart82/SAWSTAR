// SPDX-License-Identifier: MIT
#include "presets/Library.h"
#include "presets/RenameNoReplace.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#ifndef _WIN32
#include <sys/wait.h>
#include <sys/resource.h>
#include <csignal>
#include <unistd.h>
#endif
using namespace sawstar;
void check(bool b,const char* why){if(!b)throw std::runtime_error(why);}
int main(){

// Optional external-volume run, e.g. an exFAT USB drive. Default CI stays isolated.
const auto* external=std::getenv("SAWSTAR_PRESET_TEST_ROOT");
auto parent=external&&*external?fs::u8path(external):fs::temp_directory_path();
auto root=parent/("sawstar-reliable-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));try{
 fs::create_directories(root);
 auto readBytesAt=[](const fs::path& path){std::ifstream in(path,std::ios::binary);check(bool(in),"cannot read rename fixture");return std::string((std::istreambuf_iterator<char>(in)),{});};
 auto writeBytesAt=[](const fs::path& path,const std::string& bytes){std::ofstream out(path,std::ios::binary);out.write(bytes.data(),static_cast<std::streamsize>(bytes.size()));check(bool(out),"cannot write rename fixture");};
 // The native operation must move raw bytes, without decoding/re-encoding state.
 const std::string raw("raw\0preset\xff",11);
 auto sourceFile=root/fs::u8path(u8"Rename \u0151.sawstar");auto targetFile=root/fs::u8path(u8"Moved \u00e9.sawstar");
 writeBytesAt(sourceFile,raw);detail::RenameNoReplace(sourceFile,targetFile);
 check(!fs::exists(sourceFile)&&readBytesAt(targetFile)==raw,"exclusive rename changed bytes");
 writeBytesAt(sourceFile,"source");bool collision=false;
 try{detail::RenameNoReplace(sourceFile,targetFile);}catch(const fs::filesystem_error&){collision=true;}
 check(collision&&readBytesAt(sourceFile)=="source"&&readBytesAt(targetFile)==raw,"rename overwrote existing destination");
 auto missing=root/"Missing.sawstar";auto absentTarget=root/"Absent.sawstar";bool missingRejected=false;
 try{detail::RenameNoReplace(missing,absentTarget);}catch(const fs::filesystem_error&){missingRejected=true;}
 check(missingRejected&&!fs::exists(absentTarget),"missing rename source created destination");
 // Bypass cooperating-writer locks to test the primitive's filesystem exclusion.
 for(int round=0;round<32;++round){
  auto dir=root/("exclusive-"+std::to_string(round));fs::create_directory(dir);
  std::array<fs::path,2> sources{dir/"A.sawstar",dir/"B.sawstar"};auto target=dir/"Shared.sawstar";
  writeBytesAt(sources[0],"first");writeBytesAt(sources[1],"second");
  std::atomic<int> ready{0};std::atomic<bool> go{false};std::array<bool,2> won{};std::vector<std::thread> movers;
  for(int i=0;i<2;++i)movers.emplace_back([&,i]{++ready;while(!go.load())std::this_thread::yield();try{detail::RenameNoReplace(sources[i],target);won[i]=true;}catch(const fs::filesystem_error&){} });
  while(ready.load()!=2)std::this_thread::yield();go=true;for(auto& thread:movers)thread.join();
  check(won[0]!=won[1],"exclusive rename did not have exactly one winner");
  const int winner=won[0]?0:1;const int loser=1-winner;
  check(!fs::exists(sources[winner])&&readBytesAt(target)==(winner==0?"first":"second")&&readBytesAt(sources[loser])==(loser==0?"first":"second"),"racing rename lost or replaced data");
 }
#if defined(__APPLE__) || defined(_WIN32)
 if(sawstar::Fold(u8"\u00c9CHO \u0150R")!=sawstar::Fold(u8"e\u0301cho \u0151r"))throw std::runtime_error("Unicode canonical lowercase regression");
 if(sawstar::Fold(u8"\u0151r")==sawstar::Fold("or"))throw std::runtime_error("Accents must be preserved");
#endif
 for(auto name:{"CON","con.txt","PRN","AUX","NUL","COM1","LPT9","CONIN$"})check(!ValidPresetName(name),"reserved filename accepted");
 std::string accents;for(int i=0;i<80;++i)accents+=u8"\u0151";check(ValidPresetName(accents),"80 Unicode letters");check(!ValidPresetName(accents+"a"),"81 letters accepted");check(!ValidPresetName(std::string("bad\xc0\xaf")),"invalid UTF-8 accepted");
 auto values=DefaultSnapshot();auto source=root/"external"/"Sound.SAWSTAR";auto saved=SavePresetSelection(source,values,root/"library");check(saved.active&&saved.path==root/"library"/"Sound.sawstar","uppercase save import");
 auto changed=values;changed[20]=33;auto second=SavePresetSelection(root/"elsewhere"/"Sound.sawstar",changed,root/"library");check(second.active&&second.saved==changed&&second.path.parent_path()==root/"elsewhere","external collision lost active state");check(ReadUserPreset(saved.path)==values,"existing library overwritten");
 auto renamed=RenameUserPreset(saved.path,"SOUND");check(ReadUserPreset(renamed)==values,"case-only rename lost preset");check(ListUserPresets(root/"external").size()==1,"uppercase not listed");
 std::atomic<bool> start{false};std::atomic<int> failures{0};std::vector<std::thread> threads;
 for(int i=0;i<16;++i)threads.emplace_back([&,i]{try{PresetLibrary lib(root/"library");while(!start.load())std::this_thread::yield();lib.ToggleFavorite("thread:"+std::to_string(i));}catch(...){++failures;}});
 start=true;for(auto& t:threads)t.join();check(failures==0&&ReadFavorites(root/"library").size()==16,"parallel favorites lost updates");
#ifndef _WIN32
 std::vector<pid_t> children;for(int i=0;i<6;++i){auto pid=fork();check(pid>=0,"fork failed");if(pid==0){try{ChangeFavorite(root/"library","process:"+std::to_string(i));_exit(0);}catch(...){_exit(1);}}children.push_back(pid);}
 for(auto pid:children){int status=0;waitpid(pid,&status,0);check(WIFEXITED(status)&&WEXITSTATUS(status)==0,"process write failed");}check(ReadFavorites(root/"library").size()==22,"process updates lost");
#endif
#ifndef _WIN32
 // A failed partial write must leave the previous complete file byte-for-byte intact.
 auto readBytes=[&]{std::ifstream in(root/"library"/"favorites.txt",std::ios::binary);return std::string((std::istreambuf_iterator<char>(in)),{});};auto before=readBytes();
 auto child=fork();check(child>=0,"failure-test fork failed");if(child==0){signal(SIGXFSZ,SIG_IGN);rlimit limit{64,64};if(setrlimit(RLIMIT_FSIZE,&limit)!=0)_exit(2);try{ChangeFavorite(root/"library",std::string(1000,'x'));_exit(3);}catch(...){_exit(0);}}
 int status=0;waitpid(child,&status,0);check(WIFEXITED(status)&&WEXITSTATUS(status)==0&&readBytes()==before,"failed write damaged previous favorites");
#endif
 auto readFavoritesBytes=[&]{std::ifstream f(root/"library"/"favorites.txt",std::ios::binary);return std::string((std::istreambuf_iterator<char>(f)),{});};
 auto favoritesBefore=readFavoritesBytes();bool newlineRejected=false;
 try{ChangeFavorite(root/"library",std::string("external:/tmp/preset\nname.sawstar"));}
 catch(...){newlineRejected=true;}
 check(newlineRejected&&readFavoritesBytes()==favoritesBefore,
       "line-break favorite key was accepted or changed the existing file");
 auto file=root/"library"/"favorites.txt";{std::ofstream out(file);out<<"\"broken";}
 bool rejected=false;try{ChangeFavorite(root/"library","new");}catch(...){rejected=true;}check(rejected,"malformed favorites overwritten");std::ifstream in(file);std::string preserved((std::istreambuf_iterator<char>(in)),{});check(preserved=="\"broken","damaged file not preserved");in.close();
 PresetLibrary damaged(root/"library");check(!damaged.warning.empty(),"read failure not surfaced");
 for(const auto& f:fs::directory_iterator(root/"library"))check(f.path().filename().string().rfind("favorites.tmp-",0)!=0,"temporary update leaked");
 fs::remove_all(root);std::cout<<"Concurrent favorites, Unicode names, case rename and external Save As passed\n";
 }catch(const std::exception& e){fs::remove_all(root);std::cerr<<e.what()<<'\n';return 1;}}
