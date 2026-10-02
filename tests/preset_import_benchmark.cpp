// SPDX-License-Identifier: MIT
// Manual benchmark; fixture preparation is excluded from timing.
#include "presets/Library.h"
#include <chrono>
#include <iostream>
using namespace sawstar;
void write(const fs::path& path,int i){auto v=DefaultSnapshot();v[0]=-30.+i*.001;auto bytes=EncodeState(v);std::ofstream out(path,std::ios::binary);out.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());}
int main(int argc,char** argv){if(argc!=2)return 2;const fs::path parent=argv[1];
 for(int n:{100,200,400}){
  auto root=parent/("import-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
  auto library=root/"library",source=root/"source";fs::create_directories(library);fs::create_directories(source);
  std::vector<fs::path> files;
  for(int i=0;i<n;++i){write(library/("Existing "+std::to_string(i)+".sawstar"),i);auto p=source/("New "+std::to_string(i)+".sawstar");write(p,n+i);files.push_back(p);}
  const auto start=std::chrono::steady_clock::now();auto result=ImportPresets(files,library);
  const auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
  std::cout<<n<<" existing + "<<n<<" imported: "<<ms<<" ms, imported="<<result.imported<<", failed="<<result.failed<<'\n';
  if(result.imported!=n||result.failed)return 1;
  // Generated child of the explicitly supplied scratch measurement directory.
  fs::remove_all(root);
 }
}
