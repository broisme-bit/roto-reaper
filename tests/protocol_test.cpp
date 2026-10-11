#undef NDEBUG
#include "../src/plugin_protocol.h"
#include <cassert>
#include <iostream>
int main(){
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
