#pragma once
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
namespace roto {
using Bytes=std::vector<unsigned char>;
inline void word(Bytes& b,int v){v=std::clamp(v,0,16383);b.push_back(v>>7);b.push_back(v&127);}
inline int word(const unsigned char* p){return (p[0]<<7)|p[1];}
inline void text(Bytes& b,const std::string& s){for(int i=0;i<13;++i)b.push_back(i<12 && i<(int)s.size() ? ((unsigned char)s[i]<128?(unsigned char)s[i]:'?'):0);}
inline Bytes sysex(int group,int command,Bytes data={}){Bytes b={240,0,34,3,2,(unsigned char)group,(unsigned char)command};b.insert(b.end(),data.begin(),data.end());b.push_back(247);return b;}
// SysEx palette IDs from Melbourne Instruments Serial API palette table.
inline constexpr unsigned int palette[]={
0xFF94A0,0xFFA527,0xD39925,0xFFF477,0xC6FB00,0x1BFF2D,0x26FFA2,0x5FFFDF,0x90C5F6,0x5780DB,0x97A7F6,0xE06CDB,0xED539A,0xFFFFF6,
0xFF3634,0xFF6C02,0x9E6248,0xFFF032,0x8CFF63,0x3FC300,0x00BFA8,0x19E9F6,0x10A4E5,0x007DB9,0x8D6CDB,0xBD77BF,0xFF39CC,0xD8D0C8,
0xEC6856,0xFFA36F,0xDBAD6D,0xF6FFA7,0xDAE492,0xC1D06F,0xA1C488,0xDCFDD9,0xD4F1EF,0xC0C1DA,0xD4BBDB,0xB498DC,0xEDDCD9,0xAFA9A3,
0xEE9286,0xBE8252,0x9E8366,0xC6BA65,0xACBE00,0x81B04A,0x8DC2B3,0xA1B3BD,0x8AA5BF,0x8893C4,0xAB95AE,0xC69FB7,0xC37190,0x7F7B76,
0xB53331,0xAF512F,0x764F3E,0xE3C300,0x8A961D,0x569F2F,0x0A9C88,0x24637F,0x1B2F90,0x30529C,0x654BA6,0xA94BA6,0xD32E6A,0x3E3C39,
0x101010,0xFF0000,0x00FF00,0xFFFF00,0x0000F6,0xFF00F6,0x00FFF6,0x840000,0x848000,0x008000,0x00807B,0x00007B,0x84007B};
inline int nearestColor(int r,int g,int b){
 int best=70,distance=3*255*255+1;
 for(int i=0;i<83;++i){const auto c=palette[i];
   const int dr=r-int(c>>16),dg=g-int((c>>8)&255),db=b-int(c&255);
   const int d=dr*dr+dg*dg+db*db;
   if(d<distance){best=i;distance=d;}
 }
 return best;
}
// Stereo meter display: -60..+6 dBFS, silence=0; reserve headroom above 0 dBFS.
inline int meterLevel(double peak){
 if(!std::isfinite(peak) || peak<=0)return 0;
 return (int)std::lround(127*std::clamp((20*std::log10(peak)+60)/66,0.0,1.0));
}
// Bitwig-personality unity indent observed on firmware 3.2.1/API 1.6.
inline constexpr double unityPosition=13004.0/16383.0;
// Silence, then -60..0 dB below the indent and 0..+12 dB above it.
inline double volume(double n){
 if(n<=0)return 0;
 n=std::clamp(n,0.0,1.0);
 double db=n<=unityPosition?-60+60*n/unityPosition:12*(n-unityPosition)/(1-unityPosition);
 return std::pow(10.0,db/20);
}
inline double position(double v){
 if(v<=0)return 0;
 double db=20*std::log10(v);
 return std::clamp(db<=0?unityPosition*(db+60)/60:unityPosition+(1-unityPosition)*db/12,0.0,1.0);
}
}
