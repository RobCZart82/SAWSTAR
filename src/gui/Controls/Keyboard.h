// SPDX-License-Identifier: MIT
#pragma once
#include "IVKeyboardControl.h"
namespace sawstar::gui {
class Keyboard final:public iplug::igraphics::IVKeyboardControl {
public:
 using IVKeyboardControl::IVKeyboardControl;
 void OnMouseOut()override {
   // The base hover cleanup forgets the touched key without sending note-off.
   // Release first so leaving the keyboard cannot orphan a mouse-held note.
   IVKeyboardControl::OnMouseUp(0,0,iplug::igraphics::IMouseMod{});
   IVKeyboardControl::OnMouseOut();
 }
};
}
