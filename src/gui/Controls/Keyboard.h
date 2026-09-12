// SPDX-License-Identifier: MIT
#pragma once
#include "IVKeyboardControl.h"
namespace sawstar::gui {
class Keyboard final:public iplug::igraphics::IVKeyboardControl {
public:
 using IVKeyboardControl::IVKeyboardControl;
 void Draw(iplug::igraphics::IGraphics& g)override {
   using namespace iplug::igraphics;
   mPK_COLOR=IColor(255,65,195,248);
   IVKeyboardControl::Draw(g);
   for(int i=0;i<mPressedKeys.GetSize();++i)if(mPressedKeys.Get()[i]){
     const bool black=mIsBlackKeyList.Get()[i];const float x=mKeyXPos.Get()[i];
     // White-key glow stays below the black keys; never paint over neighbours.
     const IRECT b(x+1,black?mRECT.T+2:mRECT.T+mRECT.H()*mBKHeightRatio+1,
                   x+(black?mWKWidth*mBKWidthRatio:mWKWidth)-1,black?mRECT.T+mRECT.H()*mBKHeightRatio-1:mRECT.B-1);
     g.DrawRect(IColor(75,125,225,255),b.GetPadded(-1),nullptr,3);
     g.DrawRect(IColor(210,148,237,255),b.GetPadded(-1),nullptr,1);
   }
   // One logical pixel at 100% GUI scale, inside the existing keyboard bounds.
   g.FillRect(IColor(255,255,24,24),IRECT(mRECT.L,mRECT.T,mRECT.R,mRECT.T+1));
 }
 void OnMouseOut()override {
   // The base hover cleanup forgets the touched key without sending note-off.
   // Release first so leaving the keyboard cannot orphan a mouse-held note.
   IVKeyboardControl::OnMouseUp(0,0,iplug::igraphics::IMouseMod{});
   IVKeyboardControl::OnMouseOut();
 }
};
}
