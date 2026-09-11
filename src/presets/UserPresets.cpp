// SPDX-License-Identifier: MIT
#include "presets/UserPresets.h"
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#include <share.h>
#else
#include <unistd.h>
#endif

namespace sawstar {
static void SaveUserPresetUnlocked(const fs::path& path,const Snapshot& values) {
  const auto bytes=EncodeState(values);
  if(!path.parent_path().empty())fs::create_directories(path.parent_path());
#ifdef _WIN32
  int fd=-1;
  ::_wsopen_s(&fd,path.c_str(),_O_WRONLY|_O_CREAT|_O_EXCL|_O_BINARY,_SH_DENYNO,_S_IREAD|_S_IWRITE);
  auto closeFile=[](int handle){return ::_close(handle);};
#else
  int fd=::open(path.c_str(),O_WRONLY|O_CREAT|O_EXCL,0666);
  auto closeFile=[](int handle){return ::close(handle);};
#endif
  if(fd<0) {
    if(errno==EEXIST)throw std::runtime_error("That name already exists. Choose a new name.");
    throw std::runtime_error("Cannot create preset.");
  }
  try {
    size_t written=0;
    while(written<bytes.size()) {
#ifdef _WIN32
      const auto count=::_write(fd,bytes.data()+written,static_cast<unsigned>(bytes.size()-written));
#else
      const auto count=::write(fd,bytes.data()+written,bytes.size()-written);
#endif
      if(count<0&&errno==EINTR)continue;
      if(count<=0)throw std::runtime_error("Preset write failed.");
      written+=static_cast<size_t>(count);
    }
#ifdef _WIN32
    if(::_commit(fd)!=0)throw std::runtime_error("Cannot flush preset.");
#else
    if(::fsync(fd)!=0)throw std::runtime_error("Cannot flush preset.");
#endif
    const int result=closeFile(fd);
    fd=-1; // Never retry close: the descriptor may already have been released.
    if(result!=0)throw std::runtime_error("Preset write failed.");
  }catch(...) {
    if(fd>=0)closeFile(fd);
    // Only the writer that created this file may remove its incomplete output.
    std::error_code ignored;
    fs::remove(path,ignored);
    throw;
  }
}
}

#include "presets/Library.h"
#include <atomic>
#include <chrono>
#include <mutex>
#include <sstream>
#include <thread>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/file.h>
#endif
namespace sawstar {
bool ValidPresetName(const std::string& s) {
  if(s.empty()||s.size()>240||s.back()=='.'||s.back()==' '||s.find_first_of("/\\:*?\"<>|")!=std::string::npos)return false;
  size_t chars=0;
  for(size_t i=0;i<s.size();){
    const auto c=static_cast<unsigned char>(s[i]);unsigned length=0,cp=0;
    if(c<128){length=1;cp=c;}else if(c>=0xc2&&c<=0xdf){length=2;cp=c&31;}else if(c>=0xe0&&c<=0xef){length=3;cp=c&15;}else if(c>=0xf0&&c<=0xf4){length=4;cp=c&7;}else return false;
    if(i+length>s.size())return false;
    for(unsigned j=1;j<length;++j){auto b=static_cast<unsigned char>(s[i+j]);if((b&0xc0)!=0x80)return false;cp=(cp<<6)|(b&63);}
    if(cp<32||cp==127||(length==2&&cp<128)||(length==3&&cp<2048)||(length==4&&cp<65536)||cp>0x10ffff||(cp>=0xd800&&cp<=0xdfff))return false;
    i+=length;if(++chars>80)return false;
  }
  auto base=Fold(s.substr(0,s.find('.')));
  if(base=="con"||base=="prn"||base=="aux"||base=="nul"||base=="conin$"||base=="conout$")return false;
  if(base.rfind("com",0)==0||base.rfind("lpt",0)==0){auto n=base.substr(3);if((n.size()==1&&n[0]>='1'&&n[0]<='9')||n==u8"\u00b9"||n==u8"\u00b2"||n==u8"\u00b3")return false;}
  return true;
}
namespace {
class FavoritesLock {
#ifdef _WIN32
  HANDLE file_=INVALID_HANDLE_VALUE;OVERLAPPED offset_{};
#else
  int file_=-1;
#endif
public:
  explicit FavoritesLock(const fs::path& path){
#ifdef _WIN32
    file_=CreateFileW(path.c_str(),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file_==INVALID_HANDLE_VALUE)throw std::runtime_error("Cannot lock preset library.");
#else
    file_=::open(path.c_str(),O_RDWR|O_CREAT,0600);if(file_<0)throw std::runtime_error("Cannot lock preset library.");
#endif
    const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(2);
    do {
#ifdef _WIN32
      if(LockFileEx(file_,LOCKFILE_EXCLUSIVE_LOCK|LOCKFILE_FAIL_IMMEDIATELY,0,1,0,&offset_))return;
#else
      if(::flock(file_,LOCK_EX|LOCK_NB)==0)return;
#endif
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }while(std::chrono::steady_clock::now()<until);
#ifdef _WIN32
    CloseHandle(file_);
#else
    ::close(file_);
#endif
    throw std::runtime_error("Preset library is busy. Please try again.");
  }
  ~FavoritesLock(){
#ifdef _WIN32
    UnlockFileEx(file_,0,1,0,&offset_);CloseHandle(file_);
#else
    ::flock(file_,LOCK_UN);::close(file_);
#endif
  }
  FavoritesLock(const FavoritesLock&)=delete;
  FavoritesLock& operator=(const FavoritesLock&)=delete;
};
void ReplaceFavorites(const fs::path& root,const std::set<std::string>& values){
  static std::atomic<unsigned long long> serial{0};
#ifdef _WIN32
  const auto pid=GetCurrentProcessId();
#else
  const auto pid=getpid();
#endif
  fs::path temp;int fd=-1;
  for(int tries=0;tries<100&&fd<0;++tries){temp=root/("favorites.tmp-"+std::to_string(pid)+"-"+std::to_string(serial++));
#ifdef _WIN32
    ::_wsopen_s(&fd,temp.c_str(),_O_WRONLY|_O_CREAT|_O_EXCL|_O_BINARY,_SH_DENYNO,_S_IREAD|_S_IWRITE);
#else
    fd=::open(temp.c_str(),O_WRONLY|O_CREAT|O_EXCL,0600);
#endif
    if(fd<0&&errno!=EEXIST)break;
  }
  if(fd<0)throw std::runtime_error("Cannot create favorites update.");
  auto closeFile=[](int f){
#ifdef _WIN32
    return ::_close(f);
#else
    return ::close(f);
#endif
  };
  try{
    std::ostringstream out;for(const auto& k:values)out<<std::quoted(k)<<'\n';auto bytes=out.str();size_t pos=0;
    while(pos<bytes.size()){
#ifdef _WIN32
      auto n=::_write(fd,bytes.data()+pos,static_cast<unsigned>(bytes.size()-pos));
#else
      auto n=::write(fd,bytes.data()+pos,bytes.size()-pos);
#endif
      if(n<0&&errno==EINTR)continue;if(n<=0)throw std::runtime_error("Cannot write favorites.");pos+=static_cast<size_t>(n);
    }
#ifdef _WIN32
    if(::_commit(fd)!=0)throw std::runtime_error("Cannot flush favorites.");
#else
    if(::fsync(fd)!=0)throw std::runtime_error("Cannot flush favorites.");
#endif
    const int result=closeFile(fd);fd=-1;if(result!=0)throw std::runtime_error("Cannot close favorites update.");
    const auto target=root/"favorites.txt";
#ifdef _WIN32
    if(!MoveFileExW(temp.c_str(),target.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("Cannot replace favorites.");
#else
    fs::rename(temp,target);
#endif
  }catch(...){if(fd>=0)closeFile(fd);std::error_code ignored;fs::remove(temp,ignored);throw;}
}
}
namespace {
class PresetMutationLock {
  inline static std::mutex mutex_;
  std::unique_lock<std::mutex> local_{mutex_};
  FavoritesLock file_;
public:
  explicit PresetMutationLock(const fs::path& path)
    :file_((path.parent_path().empty()?fs::path("."):path.parent_path())/".sawstar-save.lock"){}
};
}
ImportReport ImportPresets(const std::vector<fs::path>& files,const fs::path& root,bool allowIdentical){
 if(root.empty())throw std::runtime_error("User preset folder is unavailable.");fs::create_directories(root);PresetMutationLock lock(root/".import");ImportReport report;
 for(const auto& p:files)try{
  if(Fold(p.extension().u8string())!=".sawstar"||!ValidPresetName(p.stem().u8string()))throw std::runtime_error("Invalid preset filename.");
  auto values=ReadUserPreset(p);auto target=root/fs::u8path(p.stem().u8string()+".sawstar");
  // Case-insensitive collision policy is consistent on macOS and Windows.
  bool collision=false;for(const auto& entry:ListUserPresets(root))if(Fold(entry.filename().u8string())==Fold(target.filename().u8string())){collision=true;break;}
  if(collision){++report.skipped;report.details.push_back(p.filename().u8string()+": already in library; not overwritten.");continue;}
  if(!allowIdentical){std::string match;for(const auto& entry:ListUserPresets(root)){try{if(ReadUserPreset(entry)==values){match=entry.stem().u8string();break;}}catch(const std::exception&){/* A damaged existing file must not block valid imports. */}}if(!match.empty()){++report.skipped;report.duplicates.push_back(p);report.details.push_back(p.filename().u8string()+": same settings as "+match);continue;}}
  SaveUserPresetUnlocked(target,values);++report.imported;
 }catch(const std::exception& e){++report.failed;report.details.push_back(p.filename().u8string()+": "+e.what());}
 return report;
}

void SaveUserPreset(const fs::path& path,const Snapshot& values){
  if(!path.parent_path().empty())fs::create_directories(path.parent_path());
  PresetMutationLock lock(path);
  SaveUserPresetUnlocked(path,values);
}
fs::path RenameUserPreset(const fs::path& source,const std::string& name) {
  PresetMutationLock lock(source);
  if(!ValidPresetName(name))throw std::runtime_error("Use a valid preset name of up to 80 characters. Shorten very long names.");
  auto target=source.parent_path()/fs::u8path(name+".sawstar");if(target==source)return source;
  if(fs::exists(target)){
    if(Fold(target.filename().u8string())!=Fold(source.filename().u8string())||!fs::equivalent(source,target))throw std::runtime_error("That name already exists.");
    // On case-insensitive volumes this is the same file, not an overwrite.
    fs::rename(source,target);
  }else{
    // Link creation is exclusive: a concurrent writer cannot be overwritten.
    fs::create_hard_link(source,target);
    try{if(!fs::remove(source))throw std::runtime_error("Cannot remove old preset name.");}
    catch(...){std::error_code ignored;fs::remove(target,ignored);throw;}
  }
  return target;
}
fs::path ArchiveUserPreset(const fs::path& source){
  PresetMutationLock lock(source);
  auto target=source;target+=".deleted";
  for(int i=1;fs::exists(target);++i){target=source;target+=".deleted-"+std::to_string(i);}
  fs::rename(source,target);
  return target;
}
fs::path OverwriteUserPreset(const fs::path& path,const Snapshot& expected,const Snapshot& values){
  if(!PresetExtension(path)||!ValidPresetName(path.stem().u8string()))throw std::runtime_error("Invalid user preset path.");
  PresetMutationLock lock(path);
  const auto parent=path.parent_path().empty()?fs::path("."):path.parent_path();
  if(!fs::is_regular_file(fs::symlink_status(path)))throw std::runtime_error("Preset is missing or is not a regular file. Use Save As.");
  if(ReadUserPreset(path)!=expected)throw std::runtime_error("Preset changed on disk. Reload it or use Save As to keep your edits.");
  static std::atomic<unsigned long long> serial{0};
#ifdef _WIN32
  const auto pid=GetCurrentProcessId();
#else
  const auto pid=getpid();
#endif
  const auto token=std::to_string(std::chrono::system_clock::now().time_since_epoch().count())+"-"+std::to_string(pid)+"-"+std::to_string(serial++);
  const auto temp=parent/(".sawstar-save-"+token+".tmp");
  const auto backupDir=parent/".sawstar-backups";
  const auto backup=backupDir/token/path.filename();
  SaveUserPresetUnlocked(temp,values);
  try{
    fs::create_directories(backup.parent_path());
    if(!fs::copy_file(path,backup,fs::copy_options::none))throw std::runtime_error("Cannot create preset backup.");
    // Check again after preparing the replacement; never overwrite known stale edits.
    if(ReadUserPreset(path)!=expected)throw std::runtime_error("Preset changed on disk. Use Save As.");
#ifdef _WIN32
    if(!MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("Cannot replace preset. The previous version is preserved.");
#else
    fs::rename(temp,path);
#endif
  }catch(...){std::error_code ignored;fs::remove(temp,ignored);throw;}
  return backup;
}
static std::set<std::string> ReadFavoritesUnlocked(const fs::path& root){
  std::set<std::string> values;if(root.empty())return values;
  const auto path=root/"favorites.txt";std::ifstream in(path);if(!in){if(!fs::exists(path))return values;throw std::runtime_error("Cannot read favorites.");}
  std::string line;while(std::getline(in,line)){if(line.empty())continue;std::istringstream row(line);std::string key;row>>std::ws;if(row.peek()!='"'||!(row>>std::quoted(key)))throw std::runtime_error("Favorites file is damaged; it has been preserved.");row>>std::ws;if(!row.eof())throw std::runtime_error("Favorites file is damaged; it has been preserved.");values.insert(key);}
  if(in.bad())throw std::runtime_error("Cannot read favorites.");return values;
}
std::set<std::string> ReadFavorites(const fs::path& root){
  if(root.empty()||!fs::exists(root/"favorites.txt"))return {};
  // Windows readers must finish before atomic replacement of the open file.
  FavoritesLock lock(root/"favorites.lock");return ReadFavoritesUnlocked(root);
}
std::set<std::string> ChangeFavorite(const fs::path& root,const std::string& key,const std::string* moveTo){
  if(root.empty())throw std::runtime_error("User folder unavailable.");
  // No filesystem work or locking on the audio callback.
  static std::mutex mutex;std::lock_guard<std::mutex> local(mutex);
  fs::create_directories(root);FavoritesLock lock(root/"favorites.lock");auto next=ReadFavoritesUnlocked(root);
  if(moveTo){if(!next.erase(key))return next;if(!moveTo->empty())next.insert(*moveTo);}
  else if(!next.erase(key))next.insert(key);
  ReplaceFavorites(root,next);return next;
}
}
