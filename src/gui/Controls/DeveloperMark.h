// SPDX-License-Identifier: MIT
#pragma once
#include "gui/Controls/Theme.h"
namespace sawstar::gui {
// Compact vector rendition of Gyuricza Róbert's supplied GYR monogram.
// Native paths avoid bitmap scaling artifacts and external image dependencies.
inline void DrawDeveloperMark(IGraphics& g,const IRECT& r){
 const float scale=std::min(r.W()/280.f,r.H()/430.f);
 const float ox=r.MW()-140*scale,oy=r.MH()-215*scale;
 auto move=[&](float x,float y){g.PathMoveTo(ox+x*scale,oy+y*scale);};
 auto line=[&](float x,float y){g.PathLineTo(ox+x*scale,oy+y*scale);};
 auto curve=[&](float a,float b,float c,float d,float e,float f){g.PathCubicBezierTo(ox+a*scale,oy+b*scale,ox+c*scale,oy+d*scale,ox+e*scale,oy+f*scale);};
 auto fill=[&]{g.PathClose();g.PathFill(Text);};
 g.PathClear();move(25,0);line(45,0);line(140,135);line(235,0);line(255,0);line(149,156);line(149,420);line(140,430);line(131,420);line(131,156);fill();
 g.PathClear();move(62,66);line(72,82);curve(-20,111,6,201,93,201);line(93,218);curve(-17,220,-34,104,62,66);fill();
 g.PathClear();move(40,151);line(115,151);line(128,168);line(120,168);line(120,264);curve(120,324,49,341,20,301);curve(-8,263,23,226,50,234);curve(76,240,70,275,54,271);curve(40,268,48,253,44,249);curve(23,248,18,278,35,293);curve(65,321,103,293,103,264);line(103,168);line(49,168);fill();
 g.PathClear();move(220,64);curve(306,84,291,187,232,208);curve(270,231,269,260,270,279);curve(270,296,272,300,277,307);line(264,321);curve(252,309,252,289,252,277);curve(252,242,238,222,197,216);line(197,201);curve(275,198,285,111,213,80);fill();
 g.PathClear();move(186,125);line(186,381);line(168,400);line(168,144);fill();
 g.PathClear();move(120,319);line(120,400);line(103,381);line(103,336);fill();
}
}
