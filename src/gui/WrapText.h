// SPDX-License-Identifier: MIT
#pragma once
#include <string>
#include <vector>

namespace sawstar::gui {
// UI thread only. Prefer word boundaries, but split long words at UTF-8 code
// point boundaries. A point wider than the available width is emitted alone so the
// loop always advances. Explicit line breaks, including blank lines, survive.
template<class Measure>
std::vector<std::string> WrapText(const std::string& text, float width, Measure measure) {
  std::vector<std::string> lines;
  std::size_t paragraphStart=0;
  for (;;) {
    const auto newline=text.find('\n',paragraphStart);
    const auto paragraph=text.substr(paragraphStart,newline==std::string::npos?newline:newline-paragraphStart);
    if(paragraph.empty())lines.emplace_back();
    std::size_t start=0;
    while(start<paragraph.size()) {
      std::size_t fit=start,first=start;
      for(std::size_t end=start;end<paragraph.size();) {
        ++end;
        while(end<paragraph.size()&&(static_cast<unsigned char>(paragraph[end])&0xc0)==0x80)++end;
        if(first==start)first=end;
        if(measure(paragraph.substr(start,end-start))>width)break;
        fit=end;
      }
      if(fit==start)fit=first;
      std::size_t end=fit;
      if(fit<paragraph.size()) {
        const auto space=paragraph.rfind(' ',fit);
        if(space!=std::string::npos&&space>start)end=space;
      }
      lines.push_back(paragraph.substr(start,end-start));
      start=end;
      while(start<paragraph.size()&&paragraph[start]==' ')++start;
    }
    if(newline==std::string::npos)break;
    paragraphStart=newline+1;
  }
  return lines;
}
inline float ConfirmationHeight(std::size_t lines) {
  // 66 px before the message, 24 px per line, then clear space above buttons.
  const float needed=146.f+24.f*static_cast<float>(lines);
  return needed>230.f?needed:230.f;
}
}
