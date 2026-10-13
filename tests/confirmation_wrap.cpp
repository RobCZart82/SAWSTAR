// SPDX-License-Identifier: MIT
#include "gui/WrapText.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>

void Check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
float Width(const std::string& text){
  float result=0;
  for(unsigned char c:text)if((c&0xc0)!=0x80)result+=c==' '?4.f:10.f;
  return result;
}
std::string Compact(std::string text){text.erase(std::remove(text.begin(),text.end(),' '),text.end());return text;}
int main(){
  using namespace sawstar::gui;
  const std::string message="Overwrite the saved settings of \""+std::string(80,'W')+"\"? A recovery copy will be kept.";
  auto lines=WrapText(message,522.f,Width);
  std::string joined;
  for(const auto& line:lines){Check(Width(line)<=522.f,"confirmation line spills outside dialog");joined+=line;}
  Check(Compact(joined)==Compact(message),"confirmation text lost characters");
  Check(66.f+24.f*static_cast<float>(lines.size())<=ConfirmationHeight(lines.size())-63.f,"message overlaps confirmation buttons");
  std::string unicode;for(int i=0;i<80;++i)unicode+=u8"\u0151";
  lines=WrapText(unicode,100.f,Width);joined.clear();
  for(const auto& line:lines){Check(!line.empty()&&static_cast<unsigned char>(line.front())==0xc5,"split UTF-8 start");Check(line.size()%2==0&&static_cast<unsigned char>(line.back())==0x91,"split UTF-8 tail");Check(Width(line)<=100.f,"Unicode line too wide");joined+=line;}
  Check(joined==unicode,"Unicode name changed");
  std::string emoji;for(int i=0;i<40;++i)emoji+=u8"\U0001f3b9";
  lines=WrapText(emoji,70.f,Width);joined.clear();
  for(const auto& line:lines){Check(line.size()%4==0,"split four-byte UTF-8 character");Check(Width(line)<=70.f,"emoji line too wide");joined+=line;}
  Check(joined==emoji,"four-byte characters changed");
  Check(WrapText("one two three",70.f,Width)==std::vector<std::string>({"one two","three"}),"word boundary regression");
  Check(WrapText("first\n\nlast\n",522.f,Width)==std::vector<std::string>({"first","","last",""}),"explicit line breaks lost");
  Check(WrapText("",522.f,Width)==std::vector<std::string>({""}),"empty message regression");
  Check(WrapText(u8"\u0151",1.f,Width)==std::vector<std::string>({u8"\u0151"}),"oversize glyph does not advance");
  Check(ConfirmationHeight(3)==230.f,"short confirmation size changed");
  Check(ConfirmationHeight(8)>=66.f+24.f*8+63.f+16.f,"expanded dialog lacks button clearance");
  std::cout<<"Confirmation wrapping and button clearance passed.\n";
}
