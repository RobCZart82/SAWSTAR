// SPDX-License-Identifier: MIT
#include "SynthTestRig.h"
#include <memory>
#include <iostream>
#include <cstdlib>
int main(){
 for(float rate:{44100.f,48000.f,96000.f}){
  auto a=std::make_unique<Rig>(),b=std::make_unique<Rig>();
  auto p=DefaultSnapshot();p[20]=70;p[21]=50;p[22]=20;p[23]=10;p[63]=50;p[90]=1;p[91]=80;
  p[5]=35;p[6]=80;p[27]=25;p[28]=60;p[42]=1;p[46]=1;p[54]=1;p[92]=2;
  a->init(44100);a->apply(p);a->midi(0x90,60,100);
  for(int i=0;i<10000;++i)a->process();
  a->init(rate);b->init(rate);a->apply(p);b->apply(p);
  a->midi(0x90,60,100);b->midi(0x90,60,100);
  for(int i=0;i<10000;++i){auto x=a->process(),y=b->process();
   if(x.left!=y.left||x.right!=y.right){std::cerr<<"Reset history changes first phrase at "<<rate<<" sample "<<i<<'\n';return 1;}}
 }
}
