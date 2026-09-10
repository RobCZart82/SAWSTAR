// SPDX-License-Identifier: MIT
#include "presets/Library.h"
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
int main(){auto root=fs::temp_directory_path()/("sawstar-reliable-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));try{
 for(auto name:{"CON","con.txt","PRN","AUX","NUL","COM1","LPT9","CONIN$"})check(!ValidPresetName(name),"reserved filename accepted");
 std::string accents;for(int i=0;i<80;++i)accents+=u8"ő";check(ValidPresetName(accents),"80 Unicode letters");check(!ValidPresetName(accents+"a"),"81 letters accepted");check(!ValidPresetName(std::string("bad\xc0\xaf")),"invalid UTF-8 accepted");
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
 auto file=root/"library"/"favorites.txt";{std::ofstream out(file);out<<"\"broken";}
 bool rejected=false;try{ChangeFavorite(root/"library","new");}catch(...){rejected=true;}check(rejected,"malformed favorites overwritten");std::ifstream in(file);std::string preserved((std::istreambuf_iterator<char>(in)),{});check(preserved=="\"broken","damaged file not preserved");in.close();
 PresetLibrary damaged(root/"library");check(!damaged.warning.empty(),"read failure not surfaced");
 for(const auto& f:fs::directory_iterator(root/"library"))check(f.path().filename().string().rfind("favorites.tmp-",0)!=0,"temporary update leaked");
 fs::remove_all(root);std::cout<<"Concurrent favorites, Unicode names, case rename and external Save As passed\n";
 }catch(const std::exception& e){fs::remove_all(root);std::cerr<<e.what()<<'\n';return 1;}}
