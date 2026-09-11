// SPDX-License-Identifier: MIT
#include "visual/Meter.h"
#include <iostream>
#include <stdexcept>
#include <limits>
int main(){using namespace sawstar;
 auto check=[](bool b){if(!b)throw std::runtime_error("meter regression");};
 MeterMailbox mailbox;mailbox.Publish(.9f,.2f);mailbox.Publish(.1f,.6f);
 auto peak=mailbox.Take();check(peak[0]==.9f&&peak[1]==.6f);check(mailbox.Take()[0]==0);
 mailbox.Publish(1.f,1.2f);mailbox.Take();check(mailbox.Clipped(0)&&mailbox.Clipped(1));
 mailbox.ClearClip(0);check(!mailbox.Clipped(0)&&mailbox.Clipped(1));mailbox.Reset();check(!mailbox.Clipped(1));
 mailbox.Publish(std::numeric_limits<float>::quiet_NaN(),-1);check(mailbox.Take()[0]==0);
 mailbox.Publish(.8f,.7f,1000);check(mailbox.Take(1300)[0]==0);
 mailbox.Publish(.8f,.7f,1000);mailbox.Publish(.2f,.1f,1300);check(mailbox.Take(1300)[0]==.2f);
 mailbox.Publish(.6f,.4f,0xfffffff0u);check(mailbox.Take(10)[0]==.6f);
 auto start=MeterDisplay::Clock::time_point{};MeterDisplay a,b;
 a.Update(1,start);b.Update(1,start);check(a.HoldCell()==19);
 for(int i=1;i<=120;++i)a.Update(0,start+std::chrono::milliseconds(i*10));
 b.Update(0,start+std::chrono::milliseconds(1200));
 check(std::abs(a.LevelDb()-b.LevelDb())<.001f);check(std::abs(a.HoldDb()-b.HoldDb())<.001f);
 check(std::abs(b.HoldDb()+3.6f)<.001f);
 b.Update(0,start+std::chrono::seconds(10));check(b.LevelDb()==-60&&b.HoldCell()==-1);
 MeterDisplay quiet;quiet.Update(0,start);check(quiet.Brightness(0)==0&&quiet.HoldCell()==-1);
 std::cout<<"Meter transient capture, channel independence, clip latch and elapsed-time ballistics passed\n";
}
