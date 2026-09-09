// SPDX-License-Identifier: MIT
#include "presets/UserPresets.h"
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace sawstar {
void SaveUserPreset(const fs::path& path,const Snapshot& values) {
  const auto bytes=EncodeState(values);
  if(!path.parent_path().empty())fs::create_directories(path.parent_path());
#ifdef _WIN32
  int fd=::_wopen(path.c_str(),_O_WRONLY|_O_CREAT|_O_EXCL|_O_BINARY,_S_IREAD|_S_IWRITE);
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
