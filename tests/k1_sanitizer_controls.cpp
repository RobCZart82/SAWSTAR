// SPDX-License-Identifier: MIT
// Deliberately invalid, separate executable: verifies sanitizer instrumentation.
// Never linked to the engine or plugin and never a K1 reproduction.
#include <cstring>
#include <iostream>
#include <limits>
int main(int argc,char** argv){
  if(argc!=2)return 2;
  if(std::strcmp(argv[1],"--asan")==0){
    int* values=new int[4]{};
    volatile int index=argc+6;
    volatile int observed=values[index]; // intentional heap buffer overflow
    std::cout<<observed<<'\n';
    delete[] values;
    return 0;
  }
  if(std::strcmp(argv[1],"--ubsan")==0){
    volatile int maximum=std::numeric_limits<int>::max();
    volatile int one=1;
    volatile int observed=maximum+one; // intentional signed integer overflow
    std::cout<<observed<<'\n';
    return 0;
  }
  return 2;
}
