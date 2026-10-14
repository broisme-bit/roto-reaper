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
// REAPER reports VST3 fx_ident as path<legacy-number{32-hex-class-ID.
// Accept only a complete terminal class ID, optionally followed by a closing brace.
// Unknown encodings retain the legacy identity instead of guessing.
inline std::string vst3ClassId(const std::string& ident){
 const auto marker=ident.rfind('{');
 if(marker==std::string::npos)return {};
 const auto length=ident.size()-marker-1;
 if(length!=32 && !(length==33 && ident.back()=='}'))return {};
 std::string id=ident.substr(marker+1,32);
 for(char& c:id){
   if(c>='a' && c<='f')c=char(c-'a'+'A');
   else if(!((c>='0' && c<='9') || (c>='A' && c<='F')))return {};
 }
 if(id==std::string(32,'0'))return {};
 return id;
}
inline std::string mappingIdentity(const std::string& type,const std::string& ident){
 if(type=="VST3" || type=="VST3i"){
   const auto id=vst3ClassId(ident);
   if(!id.empty())return "ROTOREAPER:v2:VST3:"+id;
 }
 return pluginIdentity(ident);
}
enum class MappingSource { Reaper, Ableton };
inline MappingSource parseMappingSource(const char* config){
 return config && std::string(config)=="source=ableton"?MappingSource::Ableton:MappingSource::Reaper;
}
inline const char* mappingSourceConfig(MappingSource source){return source==MappingSource::Ableton?"source=ableton":"0";}
inline std::string abletonVst3Name(const std::string& original){
 size_t start=original.compare(0,6,"VST3: ")==0?6:original.compare(0,7,"VST3i: ")==0?7:0;
 if(!start)return {}; // Do not guess at an unknown REAPER name encoding.
 std::string name=original.substr(start);
 if(!name.empty() && name.back()==')'){
   int depth=0;
   for(size_t i=name.size();i-->0;){
     if(name[i]==')')++depth;
     else if(name[i]=='(' && --depth==0){
       if(i>0 && name[i-1]==' ')name.resize(i-1);
       break;
     }
   }
 }
 return name;
}
inline std::string abletonIdentity(const std::string& name){return "ROTOABLETON:v1:VST3:"+name;}
inline bool isAbletonIdentity(const std::string& id){return id.compare(0,20,"ROTOABLETON:v1:VST3:")==0;}
inline Bytes mappingPluginHash(const std::string& id){
 if(!isAbletonIdentity(id))return hash(id,8);
 const auto name=id.substr(20);auto b=hash("PluginDevice"+name,4),tail=hash(name+"PluginDevice",4);
 b.insert(b.end(),tail.begin(),tail.end());return b;
}
inline Bytes mappingParamHash(const std::string& id,const std::string& ident,const std::string& name){
 return isAbletonIdentity(id)?hash(name,6):hash(id+"/param/"+ident,6);
}
inline Bytes paramHash(const std::string& plugin,const std::string& ident){return hash(plugin+"/param/"+ident,6);}
inline Bytes pluginDetails(int index,const std::string& id,bool enabled,const std::string& name){
 Bytes b={(unsigned char)index};auto h=mappingPluginHash(id);b.insert(b.end(),h.begin(),h.end());b.push_back(enabled?1:0);text(b,name);b.push_back(0);b.push_back(0);return sysex(11,5,b);
}
inline Bytes currentTrackName(const std::string& name){Bytes b;text(b,name);return sysex(10,0x16,b);}
inline Bytes selectedTrack(int index,const std::string& name,int color){
 Bytes b;word(b,index);text(b,name);b.push_back((unsigned char)color);b.push_back(0);return sysex(12,4,b);
}
inline Bytes learnParam(int wireIndex,const Bytes& h,int position,const std::string& name,const std::vector<std::string>& labels={}){
 Bytes b;word(b,wireIndex);b.insert(b.end(),h.begin(),h.end());b.push_back(0);b.push_back(0);b.push_back((unsigned char)labels.size());word(b,position);text(b,name);for(const auto& label:labels)text(b,label);return sysex(11,10,b);
}
}
