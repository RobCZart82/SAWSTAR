// SPDX-License-Identifier: MIT
#pragma once
#include "gui/Controls/Theme.h"
#include "gui/Controls/PageButton.h"
#include "gui/Controls/PresetControls.h"
#include "gui/Controls/WaveformControl.h"
#include "gui/Controls/NoiseSelector.h"
#include "gui/Controls/PerformanceWheel.h"
#include "gui/Controls/Keyboard.h"
#include <functional>
#include "gui/Controls/UserPresetPanel.h"
#include <string>
namespace sawstar::gui {
class LibraryRow final:public IControl{
 int index_;const int& selected_;std::function<void(int)> load_;
public:LibraryRow(IRECT r,int i,const int& selected,std::function<void(int)> load):IControl(r),index_(i),selected_(selected),load_(load){}
 void Draw(IGraphics& g)override{const auto& p=FactoryPresets()[index_];g.FillRoundRect(index_==selected_?IColor(255,25,65,88):PanelColor,mRECT,2);g.DrawText(IText(16,Text).WithAlign(EAlign::Near),p.name,mRECT.GetPadded(-12));g.DrawText(IText(11,Blue),p.category,mRECT.GetFromRight(95));}
 void OnMouseDown(float,float,const IMouseMod&)override{load_(index_);}
};
class LibraryInfo final:public IControl{
 const int& selected_;const UserPresetSelection& user_;
public:LibraryInfo(IRECT r,const int& selected,const UserPresetSelection& user):IControl(r),selected_(selected),user_(user){SetIgnoreMouse(true);}
 void Draw(IGraphics& g)override{const bool custom=selected_<0;const auto& p=FactoryPresets()[custom?0:selected_];g.DrawText(IText(24,Text).WithAlign(EAlign::Near),user_.active?user_.path.stem().u8string().c_str():custom?"Custom sound":p.name,mRECT.GetFromTop(45));
 std::string lesson=user_.active?"User preset. Edit the name below to save a new copy or rename the selected file.":custom?"Your edited sound. Save it with your host's preset menu or in the project.":p.lesson;float y=mRECT.T+70;
 while(!lesson.empty()){size_t end=lesson.size()<=54?lesson.size():lesson.rfind(' ',54);if(end==std::string::npos||end==0)end=std::min<size_t>(54,lesson.size());auto line=lesson.substr(0,end);g.DrawText(IText(15,Text).WithAlign(EAlign::Near),line.c_str(),IRECT(mRECT.L,y,mRECT.R,y+22));lesson.erase(0,end+(end<lesson.size()?1:0));y+=23;}
 g.DrawText(IText(13,Blue).WithAlign(EAlign::Near),"SIGNAL FLOW",IRECT(mRECT.L,mRECT.T+190,mRECT.R,mRECT.T+220));
 const char* labels[]={"OSC","MIXER","FILTER","AMP","FX"};const IColor colors[]={IColor(255,70,206,237),IColor(255,144,173,246),IColor(255,220,179,86),IColor(255,84,213,186),IColor(255,178,143,231)};
 for(int i=0;i<5;++i){IRECT box(mRECT.L+i*95,mRECT.T+235,mRECT.L+i*95+80,mRECT.T+295);g.DrawRoundRect(colors[i],box,3);g.DrawText(IText(13,colors[i]),labels[i],box);if(i<4)g.DrawText(IText(14,Text),">",IRECT(box.R,box.T,box.R+15,box.B));}
 g.DrawText(IText(13,Text).WithAlign(EAlign::Near),"Sources > balance > tone > loudness envelope > space",IRECT(mRECT.L,mRECT.T+315,mRECT.R,mRECT.T+345));
 }
};
inline void BuildLayout(IGraphics* g,int& page,int& lfoPage,int& fxPage,const int& preset,std::function<void(int)> load,const std::atomic<float>& peakL,const std::atomic<float>& peakR,const std::atomic<float>& cpu,const std::atomic<int>& rate,const std::atomic<int>& voices,UserPresetSelection& user,std::function<Snapshot()> current,std::function<void(const Snapshot&)> apply){
 g->AttachPanelBackground(IColor(255,10,15,18));g->EnableMouseOver(true);g->AttachPopupMenuControl();g->AttachTextEntryControl();g->LoadFont("Roboto-Regular","Arial",ETextStyle::Normal);
 auto text=[&](IRECT r,const char* label,int size,const char* group=""){g->AttachControl(new ITextControl(r,label,IText(size,Text).WithAlign(EAlign::Near)),iplug::kNoTag,group);};
 auto section=[&](IRECT r,const char* name,const char* group,IColor color=Blue,bool tint=false){g->AttachControl(new Section(r,name,color,tint),iplug::kNoTag,group);};
 auto knob=[&](float x,float y,float w,int id,const char* label,const char* group){g->AttachControl(new Knob(IRECT(x,y,x+w,y+79),id,label),iplug::kNoTag,group);};
 auto menu=[&](IRECT r,int id,const char* label,const char* group){if(id==42||id==46||id==54||id==83||id==89)g->AttachControl(new Toggle(r,id,label),iplug::kNoTag,group);else g->AttachControl(new IVMenuButtonControl(r,id,label,Style()),iplug::kNoTag,group);};
 auto fader=[&](IRECT r,int id,const char* label,const char* group,bool horizontal=false){g->AttachControl(new Fader(r,id,label,horizontal?EDirection::Horizontal:EDirection::Vertical),iplug::kNoTag,group);};
 text(IRECT(20,17,272,61),"S A W S T A R",30);text(IRECT(285,20,427,38),"Simple Synth",13);text(IRECT(285,38,427,56),"B i g  S o u n d",13);
 auto select=[g,&page,&lfoPage,&fxPage](int next){page=next;const char* groups[]={"main","advanced","presets"};for(int i=0;i<3;++i)g->ForControlInGroup(groups[i],[i,next](IControl* c){c->Hide(i!=next);});for(int i=0;i<2;++i)g->ForControlInGroup(i?"lfo2":"lfo1",[i,next,&lfoPage](IControl* c){c->Hide(next!=1||lfoPage!=i);});const char* fx[]={"chorus","delay","reverb"};for(int i=0;i<3;++i)g->ForControlInGroup(fx[i],[i,next,&fxPage](IControl* c){c->Hide(next!=1||fxPage!=i);});g->SetAllControlsDirty();};
 const char* pages[]={"MAIN","ADVANCED","PRESETS"};for(int i=0;i<3;++i)g->AttachControl(new PageButton(IRECT(440+i*98,20,534+i*98,60),pages[i],i,page,[select,i]{select(i);}));
 g->AttachControl(new PresetSelector(IRECT(752,20,1268,60),preset,load,[&user,current]{return user.active?user.path.stem().u8string()+(current()!=user.saved?" *":""):std::string{};}));
 // MAIN: signal flow, with color restricted to section headers.
 section(IRECT(12,82,292,636),"OSCILLATORS","main",IColor(255,70,206,237),true);section(IRECT(300,82,464,636),"MIXER","main",IColor(255,144,173,246),true);section(IRECT(472,82,648,636),"FILTER","main",IColor(255,220,179,86),true);section(IRECT(656,82,826,636),"FILTER ENV","main",IColor(255,102,220,192),true);section(IRECT(834,82,1004,636),"AMP ENV","main",IColor(255,84,213,186),true);section(IRECT(1012,82,1118,636),"FX","main",IColor(255,178,143,231),true);section(IRECT(1126,82,1268,636),"OUTPUT","main",IColor(255,156,179,240),true);
 for(int osc=0;osc<2;++osc){float y=126+osc*173;g->AttachControl(new WaveformControl(IRECT(23,y,145,y+146),33+osc,osc?"OSC 2":"OSC 1"),iplug::kNoTag,"main");int ids[4]={osc?24:30,osc?27:5,osc?28:6,osc?29:7};const char* labels[]={"OCT","DETUNE","UNISON","WIDTH"};for(int j=0;j<4;++j)knob(150+(j%2)*66,y+(j/2)*80,63,ids[j],labels[j],"main");}
 text(IRECT(24,485,145,515),"SUB / SINE",15,"main");knob(150,479,63,25,"OCT","main");knob(216,479,63,22,"LEVEL","main");text(IRECT(24,572,276,616),"Balance sources in the mixer",12,"main");
 const char* sources[]={"OSC1","OSC2","SUB","NOISE"};for(int i=0;i<4;++i)fader(IRECT(308+i*37,126,344+i*37,457),20+i,sources[i],"main");fader(IRECT(312,480,452,553),63,"NOISE COLOR","main",true);g->AttachControl(new NoiseSelector(IRECT(311,586,453,621)),iplug::kNoTag,"main");
 g->AttachControl(new Curve(IRECT(482,128,638,259),{8,32},false),iplug::kNoTag,"main");knob(478,280,55,8,"CUTOFF","main");knob(533,280,55,9,"RES","main");knob(588,280,55,31,"DRIVE","main");fader(IRECT(484,386,636,449),10,"FILTER MIX","main",true);fader(IRECT(484,489,636,554),12,"KEY TRACK","main",true);menu(IRECT(483,582,637,623),32,"","main");
 const char* adsr[]={"A","D","S","R"};for(int e=0;e<2;++e){float x=e?834:656;g->AttachControl(new Curve(IRECT(x+10,128,x+160,259),e?std::initializer_list<int>{1,2,3,4}:std::initializer_list<int>{13,14,15,16},true),iplug::kNoTag,"main");for(int j=0;j<4;++j)knob(x+3+j*41,280,40,(e?1:13)+j,adsr[j],"main");if(!e)knob(x+42,464,85,11,"AMOUNT","main");}
 for(int i=0;i<3;++i){knob(1021,143+i*141,88,i==0?43:i==1?47:55,i==0?"CHORUS":i==1?"DELAY":"REVERB","main");menu(IRECT(1021,225+i*141,1109,261+i*141),i==0?42:i==1?46:54,"","main");}
 fader(IRECT(1137,126,1227,421),0,"VOLUME","main");g->AttachControl(new Meter(IRECT(1236,147,1252,396),peakL,peakR),9100,"main");knob(1147,430,100,19,"BOOST dB","main");knob(1147,514,100,91,"AMOUNT WIDE","main");g->AttachControl(new Toggle(IRECT(1137,595,1257,623),90,"WIDE"),iplug::kNoTag,"main");
 // ADVANCED uses a single blue palette throughout.
 section(IRECT(12,82,260,421),"PERFORMANCE","advanced");section(IRECT(268,82,585,421),"ARPEGGIATOR","advanced");section(IRECT(593,82,879,421),"LFO","advanced");section(IRECT(887,82,1268,421),"MODULATION","advanced");
 menu(IRECT(24,133,247,180),59,"VOICE MODE","advanced");knob(23,202,105,60,"GLIDE ms","advanced");menu(IRECT(136,207,246,275),61,"GLIDE MODE","advanced");knob(23,316,105,17,"BEND RANGE","advanced");knob(136,316,105,18,"WHEEL DEPTH","advanced");
 menu(IRECT(279,130,359,178),83,"ARP","advanced");menu(IRECT(369,130,574,178),84,"ORDER","advanced");menu(IRECT(279,200,421,250),85,"RATE","advanced");menu(IRECT(432,200,574,250),89,"HOLD","advanced");knob(281,292,90,86,"GATE","advanced");knob(382,292,90,87,"OCTAVES","advanced");knob(481,292,90,88,"SWING","advanced");
 for(int bank=0;bank<2;++bank){g->AttachControl(new PageButton(IRECT(674+bank*97,89,766+bank*97,116),bank?"LFO 2":"LFO 1",bank,lfoPage,[&lfoPage,&page,select,bank]{lfoPage=bank;select(page);}),iplug::kNoTag,"advanced");const char* group=bank?"lfo2":"lfo1";int base=bank?64:35;
 menu(IRECT(605,132,867,175),base+2,"SHAPE",group);knob(600,190,86,base,"RATE Hz",group);knob(693,190,86,base+1,"AMOUNT",group);menu(IRECT(785,193,867,263),base+4,"SYNC",group);menu(IRECT(605,280,730,331),base+5,"DIVISION",group);menu(IRECT(740,280,867,331),base+6,"PHASE",group);menu(IRECT(605,347,867,401),base+3,"DESTINATION",group);}
 for(int row=0;row<4;++row){float y=132+row*67;int id=71+row*3;menu(IRECT(899,y,1006,y+51),id,row==0?"SOURCE":"","advanced");menu(IRECT(1014,y,1163,y+51),id+1,row==0?"DESTINATION":"","advanced");g->AttachControl(new Knob(IRECT(1173,y-7,1256,y+56),id+2,row==0?"AMOUNT":" "),iplug::kNoTag,"advanced");}
 section(IRECT(12,433,1268,636),"EFFECT DETAILS","advanced");const char* effects[]={"CHORUS","DELAY","REVERB"};for(int i=0;i<3;++i)g->AttachControl(new PageButton(IRECT(24+i*112,478,129+i*112,515),effects[i],i,fxPage,[&fxPage,&page,select,i]{fxPage=i;select(page);}),iplug::kNoTag,"advanced");text(IRECT(24,539,350,601),"Mix and enable are also on MAIN",12,"advanced");
 menu(IRECT(386,474,513,529),42,"CHORUS","chorus");knob(551,492,165,43,"MIX","chorus");knob(739,492,165,44,"RATE","chorus");knob(927,492,165,45,"DEPTH","chorus");
 const int dm[]={46,51,52,53};for(int i=0;i<4;++i)menu(IRECT(386+i*215,474,587+i*215,527),dm[i],i==0?"DELAY":i==1?"MODE":i==2?"CLOCK":"DIVISION","delay");for(int i=0;i<4;++i)knob(386+i*215,544,198,47+i,i==0?"MIX":i==1?"TIME ms":i==2?"FEEDBACK":"TONE Hz","delay");
 menu(IRECT(386,476,527,531),54,"REVERB","reverb");for(int i=0;i<4;++i)knob(541+i*179,520,165,55+i,i==0?"MIX":i==1?"SIZE":i==2?"DECAY s":"DAMPING Hz","reverb");
 // Factory selection is real; custom user presets continue through host saving.
 section(IRECT(12,82,570,636),"FACTORY LIBRARY","presets");section(IRECT(580,82,1268,636),"PRESET INFO / LEARNING","presets");for(int i=0;i<int(FactoryPresets().size());++i)g->AttachControl(new LibraryRow(IRECT(24,129+i*53,558,177+i*53),i,preset,load),iplug::kNoTag,"presets");g->AttachControl(new LibraryInfo(IRECT(604,134,1245,551),preset,user),iplug::kNoTag,"presets");g->AttachControl(new UserPresetPanel(IRECT(604,487,1245,623),user,current,apply,[load]{load(0);}),iplug::kNoTag,"presets");
 section(IRECT(12,646,1268,724),"","",Blue);g->AttachControl(new PerformanceWheel(IRECT(23,654,54,702),true));g->AttachControl(new PerformanceWheel(IRECT(65,654,96,702),false));text(IRECT(23,703,60,721),"PITCH",9);text(IRECT(66,703,101,721),"MOD",9);
 g->AttachControl(new Keyboard(IRECT(115,654,1258,715),36,96,false,IColor(255,181,187,187),IColor(255,19,24,27),Blue,PanelColor,Text));g->AttachControl(new Status(IRECT(20,729,1260,750),cpu,rate,voices),9101);
 select(page);
}
}
