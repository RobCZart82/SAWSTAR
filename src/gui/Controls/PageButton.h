// SPDX-License-Identifier: MIT
#pragma once
#include "IControl.h"
#include <functional>
#include <utility>

namespace sawstar::gui {
class PageButton final : public iplug::igraphics::IControl {
public:
  PageButton(const iplug::igraphics::IRECT& bounds, const char* label, int page,
             const int& selected, std::function<void()> action)
  : IControl(bounds), mLabel(label), mPage(page), mSelected(selected), mAction(std::move(action)) {}
  void Draw(iplug::igraphics::IGraphics& g) override {
    using namespace iplug::igraphics;
    const bool active = mSelected == mPage;
    g.FillRoundRect(active ? IColor(255, 43, 121, 180) : IColor(255, 20, 26, 29), mRECT, 5.f);
    g.DrawText(IText(13.f, active ? IColor(255, 5, 24, 33) : IColor(255, 210, 237, 245)), mLabel, mRECT);
  }
  void OnMouseDown(float, float, const iplug::igraphics::IMouseMod&) override { mAction(); }
private:
  const char* mLabel;
  int mPage;
  const int& mSelected;
  std::function<void()> mAction;
};
}
