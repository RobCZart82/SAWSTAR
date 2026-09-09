// SPDX-License-Identifier: MIT
#pragma once
#include "gui/Controls/Theme.h"
#include "gui/Controls/About.h"
#include "gui/Controls/PageButton.h"
#include "gui/Controls/PresetControls.h"
#include "gui/Controls/WaveformControl.h"
#include "gui/Controls/NoiseSelector.h"
#include "gui/Controls/PerformanceWheel.h"
#include "gui/Controls/Keyboard.h"
#include <functional>
#include "gui/Controls/PresetBrowser.h"
#include <string>
namespace sawstar::gui {
inline void BuildLayout(IGraphics* g,int& page,int& lfoPage,int& fxPage,const int& preset,std::function<void(int)> load,const std::atomic<float>& peakL,const std::atomic<float>& peakR,const std::atomic<float>& cpu,const std::atomic<int>& rate,const std::atomic<int>& voices,UserPresetSelection& user,std::function<Snapshot()> current,std::function<void(const Snapshot&)> apply){
 auto* confirm=new ConfirmAction(IRECT(0,0,1280,760));
 g->AttachPanelBackground(IColor(255,10,15,18));g->EnableMouseOver(true);g->AttachPopupMenuControl();g->AttachTextEntryControl();g->LoadFont("Roboto-Regular","Arial",ETextStyle::Normal);
 auto text=[&](IRECT r,const char* label,int size,const char* group=""){g->AttachControl(new ITextControl(r,label,IText(size,Text).WithAlign(EAlign::Near)),iplug::kNoTag,group);};
 auto section=[&](IRECT r,const char* name,const char* group,IColor color=Blue,bool tint=false){g->AttachControl(new Section(r,name,color,tint),iplug::kNoTag,group);};
 auto knob=[&](float x,float y,float w,int id,const char* label,const char* group){g->AttachControl(new Knob(IRECT(x,y,x+w,y+79),id,label),iplug::kNoTag,group);};
 auto menu=[&](IRECT r,int id,const char* label,const char* group){if(id==42||id==46||id==54||id==83||id==89)g->AttachControl(new Toggle(r,id,label),iplug::kNoTag,group);else g->AttachControl(new Dropdown(r,id,label),iplug::kNoTag,group);};
 auto fader=[&](IRECT r,int id,const char* label,const char* group,bool horizontal=false){g->AttachControl(new Fader(r,id,label,horizontal?EDirection::Horizontal:EDirection::Vertical),iplug::kNoTag,group);};
 bool brandFont=g->LoadFont("SAWSTAR-Orbitron",AboutFont,sizeof(AboutFont));g->AttachControl(new ITextControl(IRECT(20,17,272,61),"SAWSTAR",IText(30,Text).WithFont(brandFont?"SAWSTAR-Orbitron":"Roboto-Regular").WithAlign(EAlign::Near)));text(IRECT(285,20,427,38),"Simple Synth",13);text(IRECT(285,38,427,56),"B i g  S o u n d",13);
 auto select=[g,&page,&lfoPage,&fxPage](int next){page=next;const char* groups[]={"main","advanced","presets"};for(int i=0;i<3;++i)g->ForControlInGroup(groups[i],[i,next](IControl* c){c->Hide(i!=next);});for(int i=0;i<2;++i)g->ForControlInGroup(i?"lfo2":"lfo1",[i,next,&lfoPage](IControl* c){c->Hide(next!=1||lfoPage!=i);});const char* fx[]={"chorus","delay","reverb"};for(int i=0;i<3;++i)g->ForControlInGroup(fx[i],[i,next,&fxPage](IControl* c){c->Hide(next!=1||fxPage!=i);});if(next==2)g->ForControlInGroup("presets",[](IControl* c){if(auto* b=dynamic_cast<PresetBrowser*>(c))b->SyncToSound();});g->SetAllControlsDirty();};
 const char* pages[]={"MAIN","ADVANCED","PRESETS"};for(int i=0;i<3;++i)g->AttachControl(new PageButton(IRECT(440+i*98,20,534+i*98,60),pages[i],i,page,[select,i]{select(i);}));
 g->AttachControl(new PresetSelector(IRECT(752,20,1214,60),preset,load,user,current,apply,[g]{g->ForControlInGroup("presets",[](IControl* c){if(auto* b=dynamic_cast<PresetBrowser*>(c))b->SyncToSound();});},confirm));
 // MAIN: signal flow, with color restricted to section headers.
 section(IRECT(12,82,292,636),"OSCILLATORS","main",IColor(255,70,206,237),true);section(IRECT(300,82,464,636),"MIXER","main",IColor(255,144,173,246),true);section(IRECT(472,82,648,636),"FILTER","main",IColor(255,220,179,86),true);section(IRECT(656,82,826,636),"FILTER ENV","main",IColor(255,102,220,192),true);section(IRECT(834,82,1004,636),"AMP ENV","main",IColor(255,84,213,186),true);section(IRECT(1012,82,1118,636),"FX","main",IColor(255,178,143,231),true);section(IRECT(1126,82,1268,636),"OUTPUT","main",IColor(255,156,179,240),true);
 for(int osc=0;osc<2;++osc){float y=126+osc*173;g->AttachControl(new WaveformControl(IRECT(23,y,145,y+146),33+osc,osc?"OSC 2":"OSC 1"),iplug::kNoTag,"main");int ids[4]={osc?24:30,osc?27:5,osc?28:6,osc?29:7};const char* labels[]={"OCT","DETUNE","UNISON","WIDTH"};for(int j=0;j<4;++j)knob(150+(j%2)*66,y+(j/2)*80,63,ids[j],labels[j],"main");}
 text(IRECT(24,485,145,515),"SUB / SINE",15,"main");knob(150,479,63,25,"OCT","main");knob(216,479,63,22,"LEVEL","main");text(IRECT(24,572,276,616),"Balance sources in the mixer",12,"main");
 const char* sources[]={"OSC1","OSC2","SUB","NOISE"};for(int i=0;i<4;++i)fader(IRECT(308+i*37,126,344+i*37,457),20+i,sources[i],"main");fader(IRECT(312,480,452,553),63,"NOISE COLOR","main",true);g->AttachControl(new NoiseSelector(IRECT(311,586,453,621)),iplug::kNoTag,"main");
 g->AttachControl(new Curve(IRECT(482,128,638,259),{8,32},false),iplug::kNoTag,"main");knob(478,280,55,8,"CUTOFF","main");knob(533,280,55,9,"RES","main");knob(588,280,55,31,"DRIVE","main");fader(IRECT(484,386,636,449),10,"FILTER MIX","main",true);fader(IRECT(484,489,636,554),12,"KEY TRACK","main",true);menu(IRECT(483,582,637,623),32,"","main");
 const char* adsr[]={"A","D","S","R"};for(int e=0;e<2;++e){float x=e?834:656;g->AttachControl(new Curve(IRECT(x+10,128,x+160,259),e?std::initializer_list<int>{1,2,3,4}:std::initializer_list<int>{13,14,15,16},true),iplug::kNoTag,"main");for(int j=0;j<4;++j)knob(x+3+j*41,280,40,(e?1:13)+j,adsr[j],"main");if(!e)knob(x+42,464,85,11,"AMOUNT","main");}
 for(int i=0;i<3;++i){knob(1021,143+i*141,88,i==0?43:i==1?47:55,i==0?"CHORUS":i==1?"DELAY":"REVERB","main");menu(IRECT(1021,225+i*141,1109,261+i*141),i==0?42:i==1?46:54,"","main");}
 auto* volume=new Fader(IRECT(1137,126,1227,421),0,"VOLUME");g->AttachControl(volume,iplug::kNoTag,"main");g->AttachControl(new Meter(IRECT(1236,126,1252,421),peakL,peakR,volume),9100,"main");knob(1147,430,100,19,"BOOST dB","main");knob(1147,514,100,91,"AMOUNT WIDE","main");g->AttachControl(new Toggle(IRECT(1137,595,1257,623),90,"WIDE"),iplug::kNoTag,"main");
 // ADVANCED uses a single blue palette throughout.
 section(IRECT(12,82,260,421),"PERFORMANCE","advanced");section(IRECT(268,82,585,421),"ARPEGGIATOR","advanced");section(IRECT(593,82,879,421),"LFO","advanced");section(IRECT(887,82,1268,421),"MODULATION","advanced");
 menu(IRECT(24,133,247,180),59,"VOICE MODE","advanced");knob(23,202,105,60,"GLIDE ms","advanced");menu(IRECT(136,207,246,275),61,"GLIDE MODE","advanced");knob(23,316,105,17,"BEND RANGE","advanced");knob(136,316,105,18,"WHEEL DEPTH","advanced");
 menu(IRECT(279,147,421,179),83,"ARP","advanced");menu(IRECT(432,147,574,179),89,"HOLD","advanced");menu(IRECT(279,200,421,250),85,"RATE","advanced");menu(IRECT(432,200,574,250),84,"ORDER","advanced");knob(281,292,90,86,"GATE","advanced");knob(382,292,90,87,"OCTAVES","advanced");knob(481,292,90,88,"SWING","advanced");
 for(int bank=0;bank<2;++bank){g->AttachControl(new PageButton(IRECT(674+bank*97,89,766+bank*97,116),bank?"LFO 2":"LFO 1",bank,lfoPage,[&lfoPage,&page,select,bank]{lfoPage=bank;select(page);}),iplug::kNoTag,"advanced");const char* group=bank?"lfo2":"lfo1";int base=bank?64:35;
 menu(IRECT(605,132,867,175),base+2,"SHAPE",group);knob(600,190,86,base,"RATE Hz",group);knob(693,190,86,base+1,"AMOUNT",group);menu(IRECT(785,193,867,263),base+4,"SYNC",group);menu(IRECT(605,280,730,331),base+5,"DIVISION",group);menu(IRECT(740,280,867,331),base+6,"PHASE",group);menu(IRECT(605,347,867,401),base+3,"DESTINATION",group);}
 for(int row=0;row<4;++row){float y=132+row*67;int id=71+row*3;menu(IRECT(899,y,1006,y+51),id,row==0?"SOURCE":"","advanced");menu(IRECT(1014,y,1163,y+51),id+1,row==0?"DESTINATION":"","advanced");g->AttachControl(new Knob(IRECT(1173,y-7,1256,y+56),id+2,row==0?"AMOUNT":" "),iplug::kNoTag,"advanced");}
 section(IRECT(12,433,1268,636),"EFFECT DETAILS","advanced");const char* effects[]={"CHORUS","DELAY","REVERB"};for(int i=0;i<3;++i)g->AttachControl(new PageButton(IRECT(24,478+i*45,174,512+i*45),effects[i],i,fxPage,[&fxPage,&page,select,i]{fxPage=i;select(page);}),iplug::kNoTag,"advanced");
 // Equal-width columns: enable shares the knob row; delay choices sit above it.
 menu(IRECT(220,558,390,590),42,"CHORUS","chorus");knob(420,540,170,43,"MIX","chorus");knob(620,540,170,44,"RATE","chorus");knob(820,540,170,45,"DEPTH","chorus");
 const int dm[]={51,52,53};for(int i=0;i<3;++i)menu(IRECT(420+i*200,475,590+i*200,527),dm[i],i==0?"MODE":i==1?"CLOCK":"DIVISION","delay");menu(IRECT(220,558,390,590),46,"DELAY","delay");for(int i=0;i<4;++i)knob(420+i*200,540,170,47+i,i==0?"MIX":i==1?"TIME ms":i==2?"FEEDBACK":"TONE Hz","delay");
 menu(IRECT(220,558,390,590),54,"REVERB","reverb");for(int i=0;i<4;++i)knob(420+i*200,540,170,55+i,i==0?"MIX":i==1?"SIZE":i==2?"DECAY s":"DAMPING Hz","reverb");
 // Quiet separators group related controls without changing their hit areas.
 auto divider=[&](float l,float y,float rr,const char* group){g->AttachControl(new Divider(IRECT(l,y,rr,y+1)),iplug::kNoTag,group);};
 divider(24,294,280,"main");divider(24,470,280,"main");
 divider(312,470,452,"main");divider(484,376,636,"main");
 divider(1021,272,1109,"main");divider(1021,413,1109,"main");
 divider(1137,425,1257,"main");divider(1137,510,1257,"main");
 divider(24,293,247,"advanced");
 // Shared factory/user library follows the approved four-column concept.
 g->AttachControl(new PresetBrowser(user,current,apply,load,confirm),9102,"presets");
 section(IRECT(12,646,1268,724),"","",Blue);g->AttachControl(new PerformanceWheel(IRECT(23,654,54,702),true));g->AttachControl(new PerformanceWheel(IRECT(65,654,96,702),false));text(IRECT(23,703,60,721),"PITCH",9);text(IRECT(66,703,101,721),"MOD",9);
 g->AttachControl(new Keyboard(IRECT(115,654,1258,715),36,96,false,IColor(255,181,187,187),IColor(255,19,24,27),Blue,PanelColor,Text));g->AttachControl(new Status(IRECT(20,729,1260,750),cpu,rate,voices),9101);
 auto* about=new AboutWindow(IRECT(0,0,1280,760),brandFont);
 g->AttachControl(new SettingsMenu(IRECT(1228,20,1268,60),about));
 g->AttachControl(about);
 g->AttachControl(confirm);
 select(page);
}
}
