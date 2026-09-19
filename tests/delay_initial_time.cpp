// SPDX-License-Identifier: MIT
#include "dsp/Effects/Delay.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<"\n";std::exit(1);}}
int main(){for(float sr:{44100.f,48000.f,96000.f})for(bool sync:{false,true}){
 sawstar::Delay d;d.Init(sr);
 // Multiple setup calls before audio starts: the last time must be used.
 d.Set(true,100,150,0,16000,false,false,0,120);
 d.Set(true,100,10,0,16000,false,sync,0,120);
 int first=-1;const int expected=int(std::round(sr*(sync?.125:.010)));
 for(int i=0;i<int(sr*.4);++i){auto x=d.Process({i==0?1.f:0.f,0});
  check(std::isfinite(x.left)&&std::isfinite(x.right),"invalid delay output");
  if(i>0&&std::abs(x.left)>1.e-7f&&first<0)first=i;
 }
 std::cout<<sr<<" sync="<<sync<<" expected="<<expected<<" actual="<<first<<std::endl;
 check(std::abs(first-expected)<=1,"fresh delay does not start at configured time");
 d.Clear();for(int i=0;i<int(sr*.3);++i){auto x=d.Process({});check(x.left==0&&x.right==0,"clear leaks old history");}
}}
