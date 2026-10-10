#pragma once
#include "protocol.h"
#include "../vendor/WDL/sha.h"
namespace roto {
// Match the documented Bitwig hash encoding; namespace identities to this host.
inline Bytes hash(const std::string& id,int size){
 WDL_SHA1 sha;sha.add(id.data(),(int)id.size());unsigned char digest[20];sha.result(digest);
 Bytes b;for(int i=0;i<size;++i)b.push_back(digest[i]&127);return b;
}
inline std::string pluginIdentity(const std::string& id){return "ROTOREAPER:v1:"+id;}
inline Bytes paramHash(const std::string& plugin,const std::string& ident){return hash(plugin+"/param/"+ident,6);}
inline Bytes pluginDetails(int index,const std::string& id,bool enabled,const std::string& name){
 Bytes b={(unsigned char)index};auto h=hash(id,8);b.insert(b.end(),h.begin(),h.end());b.push_back(enabled?1:0);text(b,name);b.push_back(0);b.push_back(0);return sysex(11,5,b);
}
inline Bytes currentTrackName(const std::string& name){Bytes b;text(b,name);return sysex(10,0x16,b);}
inline Bytes selectedTrack(int index,const std::string& name,int color){
 Bytes b;word(b,index);text(b,name);b.push_back((unsigned char)color);b.push_back(0);return sysex(12,4,b);
}
inline Bytes learnParam(int wireIndex,const Bytes& h,int position,const std::string& name,const std::vector<std::string>& labels={}){
 Bytes b;word(b,wireIndex);b.insert(b.end(),h.begin(),h.end());b.push_back(0);b.push_back(0);b.push_back((unsigned char)labels.size());word(b,position);text(b,name);for(const auto& label:labels)text(b,label);return sysex(11,10,b);
}
}
