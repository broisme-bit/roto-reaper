#undef NDEBUG
#include "../src/plugin_protocol.h"
#include <cassert>
#include <iostream>
int main(){
 const auto devil=roto::abletonIdentity(roto::abletonPluginName("VST: Devil-Loc Deluxe (Soundtoys)",false,true),false,true);
 assert(roto::mappingPluginHash(devil)==roto::Bytes({0x31,0x4f,0x0d,0x54,0x4d,0x74,0x1c,0x6a}));
 assert(roto::mappingParamHash(devil,"1","Crush")==roto::Bytes({0x43,0x77,0x0b,0x1d,0x0f,0x43}));
 assert(roto::mappingParamHash(devil,"2","Crunch")==roto::Bytes({1,0x4d,0x37,0x5f,0x6b,0x22}));
 assert(roto::mappingParamHash(devil,"4","Darkness")==roto::Bytes({0x33,0x29,0x59,0x38,0x63,0x62}));
 assert(roto::abletonPluginName("VSTi: FabFilter Twin 3 (FabFilter)",false,true)=="FabFilter Twin 3");
 assert(roto::mappingPluginHash(roto::abletonIdentity("FabFilter Twin 3",false,true))!=roto::mappingPluginHash(roto::abletonIdentity("Twin 3",false,true)));
 assert(roto::abletonPluginName("VST3: Synth (Vendor)",false,true).empty());

 const auto au=roto::abletonIdentity("UADx Pultec EQP-1A EQ",true);
 assert(roto::mappingPluginHash(au)==roto::Bytes({0x11,0x62,0x67,8,0x72,0x25,0x21,0x58}));
 assert(roto::mappingPluginHash(au)!=roto::mappingPluginHash(roto::abletonIdentity("UADx Pultec EQP-1A EQ")));
 assert(roto::mappingParamHash(au,"1:49","LF Boost")==roto::Bytes({0x16,6,0x65,0x70,8,0x7c}));
 assert(roto::abletonPluginName("AU: UADx Pultec EQP-1A EQ (Universal Audio (UADx))",true)=="UADx Pultec EQP-1A EQ");
 assert(roto::abletonPluginName("AUi: Synth (Vendor)",true)=="Synth");
 assert(roto::abletonPluginName("VST3i: Synth (Vendor)",true).empty());
 assert(roto::abletonPluginName("AUi: Synth (Vendor)").empty());
 const auto moog=roto::abletonIdentity("UADx Moog Minimoog Synth");
 assert(roto::mappingPluginHash(moog)==roto::Bytes({0x57,0x61,0x0b,0x52,0x59,0x18,0x51,0x53}));

 assert(roto::parseMappingSource(nullptr)==roto::MappingSource::Reaper);
 assert(roto::parseMappingSource("0")==roto::MappingSource::Reaper);
 assert(roto::parseMappingSource("source=ableton")==roto::MappingSource::Ableton);
 assert(roto::parseMappingSource("source=unknown")==roto::MappingSource::Reaper);
 assert(roto::parseMappingSource(roto::mappingSourceConfig(roto::MappingSource::Ableton))==roto::MappingSource::Ableton);
 assert(roto::abletonPluginName("VST3: UADx Pultec EQP-1A EQ (Universal Audio (UADx))")=="UADx Pultec EQP-1A EQ");
 assert(roto::abletonPluginName("VST3i: Synth (mono) (Vendor)")=="Synth (mono)");
 assert(roto::abletonPluginName("VST3: Synth")=="Synth");
 assert(roto::abletonPluginName("AU: Synth (Vendor)").empty());
 const auto live=roto::abletonIdentity("UADx Pultec EQP-1A EQ");
 assert(roto::mappingPluginHash(live)==roto::Bytes({0x3e,0x0c,3,0x36,0x22,0x63,2,0x7b}));
 assert(roto::mappingParamHash(live,"1:49","LF Boost")==roto::Bytes({0x16,6,0x65,0x70,8,0x7c}));
 assert(roto::mappingParamHash(live,"2:50","LF Atten")==roto::Bytes({0x51,0x4a,0x22,0x2d,0x4d,0x74}));
 assert(roto::mappingParamHash(live,"0:48","Low Freq")==roto::Bytes({0x21,0x7d,0x16,0x75,0x27,0x5d}));
 assert(roto::mappingParamHash(roto::pluginIdentity("reaeq"),"0","Frequency")==roto::paramHash(roto::pluginIdentity("reaeq"),"0"));

 const std::string cid="ABCDEF019182FAEB554144785533414E";
 const auto stable=roto::mappingIdentity("VST3","/Library/Audio/Plug-Ins/VST3/uaudio_pultec_eqp-1a.vst3<468507556{"+cid);
 assert(stable=="ROTOREAPER:v2:VST3:"+cid);
 assert(stable==roto::mappingIdentity("VST3","/some/other/plugin.vst3<123{"+cid+"}"));
 assert(stable==roto::mappingIdentity("VST3i","C:\\Plugins\\renamed.vst3<123{abcdef019182faeb554144785533414e"));
 assert(roto::hash(stable,8)==roto::hash(roto::mappingIdentity("VST3","renamed.vst3<999{"+cid),8));
 assert(roto::paramHash(stable,"0:48")==roto::paramHash(roto::mappingIdentity("VST3","renamed.vst3<999{"+cid),"0:48"));
 assert(stable!=roto::mappingIdentity("VST3","p.vst3<123{BBCDEF019182FAEB554144785533414E"));
 assert(roto::mappingIdentity("AU","vendor: plugin")==roto::pluginIdentity("vendor: plugin"));
 assert(roto::mappingIdentity("CLAP","p{"+cid)==roto::pluginIdentity("p{"+cid));
 for(const auto& raw: {std::string("no-class-id"),std::string("p{ABC"),std::string("p{")+cid+"extra",std::string("p{ZBCDEF019182FAEB554144785533414E"),std::string("p{")+std::string(32,'0')})
   assert(roto::mappingIdentity("VST3",raw)==roto::pluginIdentity(raw));

 assert(roto::hash("abc",8)==roto::Bytes({0x29,0x19,0x3e,0x36,0x47,0x06,0x01,0x6a}));
 roto::Bytes track;roto::word(track,8);roto::text(track,"Track Nine");track.push_back(71);track.push_back(0);
 assert(roto::selectedTrack(8,"Track Nine",71)==roto::sysex(12,4,track));
 assert(roto::selectedTrack(0,"",70).size()==25);
 roto::Bytes name;roto::text(name,"Track Nine");assert(roto::currentTrackName("Track Nine")==roto::sysex(10,0x16,name));
 assert(roto::currentTrackName("").size()==21);
 const auto id=roto::pluginIdentity("VST:ReaEQ");auto h=roto::paramHash(id,"freq1");
 assert(h==roto::paramHash(id,"freq1") && h!=roto::paramHash(id,"gain1"));
 auto learned=roto::learnParam(130,h,8192,"Frequency");
 assert(learned.size()==34 && learned[7]==1 && learned[8]==2 && learned[15]==0 && learned[18]==64 && learned[19]==0 && learned[33]==247);
 auto device=roto::pluginDetails(9,id,true,"ReaEQ");assert(device.size()==33 && device[7]==9 && device[16]==1 && device[30]==0 && device[31]==0);
 assert(roto::sysex(10,3,{2})==roto::Bytes({240,0,34,3,2,10,3,2,247}));
 for(int n=0;n<16384;++n){roto::Bytes b;roto::word(b,n);assert(roto::word(b.data())==n);assert(b[0]<128 && b[1]<128);}
 roto::Bytes b;roto::text(b,"123456789012345");assert(b.size()==13 && b[12]==0 && b[11]=='2');
 for(int n=1;n<=16383;++n){double x=n/16383.0;assert(std::abs(roto::position(roto::volume(x))-x)<1e-10);}
 for(int i=0;i<83;++i){auto c=roto::palette[i];assert(roto::nearestColor(c>>16,(c>>8)&255,c&255)==i);}
 assert(roto::nearestColor(255,0,0)==71 && roto::nearestColor(0,255,0)==72);
 assert(roto::nearestColor(0,0,255)==74);
 assert(roto::meterLevel(0)==0 && roto::meterLevel(-1)==0);
 assert(roto::meterLevel(std::nan(""))==0 && roto::meterLevel(INFINITY)==0);
 assert(roto::meterLevel(.001)==0 && roto::meterLevel(1)==115 && roto::meterLevel(2)==127);
 assert(roto::meterLevel(.01)<roto::meterLevel(.1) && roto::meterLevel(.1)<roto::meterLevel(1));
 assert(roto::volume(13004.0/16383)==1.0);
 assert(std::lround(roto::position(1)*16383)==13004);
 assert(roto::volume(13003.0/16383)<1 && roto::volume(13005.0/16383)>1);
 assert(std::abs(20*std::log10(roto::volume(1))-12)<1e-10);
 assert(roto::volume(0)==0 && roto::position(0)==0);
 // API section 4.10: track index and each RGB component are two 7-bit bytes.
 std::cout<<"Handshake bytes, full 14-bit range, text limits, and volume feedback round-trip passed\n";
}
