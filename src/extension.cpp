#define NOMINMAX
#include "../vendor/sdk/reaper_plugin.h"
#undef min
#undef max
#define REAPERAPI_IMPLEMENT
#define REAPERAPI_MINIMAL
#define REAPERAPI_WANT_CreateMIDIInput
#define REAPERAPI_WANT_CreateMIDIOutput
#define REAPERAPI_WANT_GetMIDIInputName
#define REAPERAPI_WANT_GetMIDIOutputName
#define REAPERAPI_WANT_GetNumMIDIInputs
#define REAPERAPI_WANT_GetNumMIDIOutputs
#define REAPERAPI_WANT_CountTracks
#define REAPERAPI_WANT_GetTrack
#define REAPERAPI_WANT_GetMasterTrack
#define REAPERAPI_WANT_GetTrackNumSends
#define REAPERAPI_WANT_GetSetTrackSendInfo
#define REAPERAPI_WANT_GetTrackSendUIVolPan
#define REAPERAPI_WANT_SetTrackSendUIVol
#define REAPERAPI_WANT_GetTrackName
#define REAPERAPI_WANT_Track_GetPeakInfo
#define REAPERAPI_WANT_GetTrackColor
#define REAPERAPI_WANT_ColorFromNative
#define REAPERAPI_WANT_GetMediaTrackInfo_Value
#define REAPERAPI_WANT_CSurf_OnVolumeChange
#define REAPERAPI_WANT_CSurf_OnPanChange
#define REAPERAPI_WANT_CSurf_OnMuteChange
#define REAPERAPI_WANT_CSurf_OnSoloChange
#define REAPERAPI_WANT_CSurf_OnRecArmChange
#define REAPERAPI_WANT_CSurf_SetSurfaceVolume
#define REAPERAPI_WANT_CSurf_SetSurfacePan
#define REAPERAPI_WANT_CSurf_SetSurfaceMute
#define REAPERAPI_WANT_CSurf_SetSurfaceSolo
#define REAPERAPI_WANT_CSurf_SetSurfaceRecArm
#define REAPERAPI_WANT_SetOnlyTrackSelected
#define REAPERAPI_WANT_GetPlayState
#define REAPERAPI_WANT_GetSetRepeat
#define REAPERAPI_WANT_CSurf_OnPlay
#define REAPERAPI_WANT_CSurf_OnStop
#define REAPERAPI_WANT_CSurf_OnRecord
#define REAPERAPI_WANT_time_precise
#define REAPERAPI_WANT_GetResourcePath
#define REAPERAPI_WANT_GetSelectedTrack2
#define REAPERAPI_WANT_GetTouchedOrFocusedFX
#define REAPERAPI_WANT_ValidatePtr2
#define REAPERAPI_WANT_TrackFX_GetCount
#define REAPERAPI_WANT_TrackFX_GetFXName
#define REAPERAPI_WANT_TrackFX_GetFXGUID
#define REAPERAPI_WANT_TrackFX_GetNamedConfigParm
#define REAPERAPI_WANT_TrackFX_GetNumParams
#define REAPERAPI_WANT_TrackFX_GetParamIdent
#define REAPERAPI_WANT_TrackFX_GetParamName
#define REAPERAPI_WANT_TrackFX_GetParamNormalized
#define REAPERAPI_WANT_TrackFX_SetParamNormalized
#define REAPERAPI_WANT_TrackFX_GetParameterStepSizes
#define REAPERAPI_WANT_TrackFX_FormatParamValueNormalized
#define REAPERAPI_WANT_TrackFX_GetEnabled
#define REAPERAPI_WANT_TrackFX_SetEnabled
#define REAPERAPI_WANT_TrackFX_Show
#define REAPERAPI_WANT_TrackFX_EndParamEdit
#include "../vendor/sdk/reaper_plugin_functions.h"
#include "plugin_protocol.h"
#include <array>
#include <cstdio>
#include <cstring>
#include <memory>
#include <deque>
#include <unordered_map>
#ifndef ROTO_SURFACE_TEST
HWND rotoShowConfig(HWND parent,const char* config,REAPER_PLUGIN_HINSTANCE instance);
static REAPER_PLUGIN_HINSTANCE rotoInstance=nullptr;
#endif
class Surface:public IReaperControlSurface {
 roto::MappingSource mappingSource=roto::MappingSource::Reaper;
 midi_Input* in=nullptr; midi_Output* out=nullptr;
 int inputPort=-1,outputPort=-1;double portCheck=0,openedAt=0,healthNext=0,healthDeadline=0;
 bool waitingLogged=false;
 bool plugin=false,pluginDirty=true,pluginControls=false,learning=false,pluginLocked=false;
 MediaTrack* fxTrack=nullptr;int fx=-1,firstFx=0,learnParam=-1;
 roto::Bytes publishedPluginTrack;bool pluginTrackPublished=false;
 std::string fxToken,focusToken,pluginSnapshot;double pluginPoll=0,mappingTime=-100,learnTime=0;
 struct Param {std::string name;roto::Bytes hash;};std::vector<Param> params;
 std::array<int,8> knobParams{{-1,-1,-1,-1,-1,-1,-1,-1}},switchParams{{-1,-1,-1,-1,-1,-1,-1,-1}};
 int baselineTrack=-2,baselineFx=-1,baselineParam=-1;double baselineValue=0;
 bool singleMix=false;int singlePage=0;MediaTrack* singleTrack=nullptr;
 roto::Bytes publishedSingleTrack;bool singleTrackPublished=false;
 bool connected=false, mix=false, supported=false, dirty=true, master=false;
 int sendIndex=0,sendNameStart=0;double sendPoll=0;bool sendDirty=true;
 std::vector<MediaTrack*> sendDestinations;std::vector<std::string> sendNames;
 std::array<MediaTrack*,8> sendEditSource{},sendEditDestination{},touchSendDestination{};
 std::array<double,8> sendEditValue{};
 int bank=0,knobMode=0,buttonMode=0,publishedCount=-1,publishedBank=-1; bool publishedMaster=false;
 std::array<std::string,8> names{}; std::array<int,8> colors{}; double retry=0,refresh=0;
 std::array<int,8> buttons{{-1,-1,-1,-1,-1,-1,-1,-1}};
 std::array<int,8> hi{},last{}; std::array<bool,8> haveHi{}; std::array<bool,8> touch{};
 std::array<MediaTrack*,8> touched{};
 std::deque<roto::Bytes> metadata; double metadataNext=0,motorReady=0;
 // Host callbacks carry the displayed automation value, unlike D_VOL/D_PAN.
 std::unordered_map<MediaTrack*,double> hostVolumes,hostPans;
 std::array<int,16> meterLast{};std::array<double,16> meterPeaks{};double meterNext=0;
 FILE* log=nullptr;
 void note(const char* s){if(log){std::fprintf(log,"%.3f %s\n",time_precise(),s);std::fflush(log);}}
 void closePorts(const char* reason){
   note(reason);endPluginEdits();clearTouch();clearBindings();metadata.clear();
   connected=false;mix=false;singleMix=false;singleTrack=nullptr;singleTrackPublished=false;plugin=false;supported=false;learning=false;pluginLocked=false;
   pluginControls=false;pluginTrackPublished=false;pluginDirty=true;focusToken.clear();
   pluginSnapshot.clear();params.clear();fxTrack=nullptr;fx=-1;fxToken.clear();
   publishedCount=publishedBank=-1;sendDirty=true;reset();healthDeadline=0;
   if(in){in->stop();in->Destroy();in=nullptr;}
   if(out){out->Destroy();out=nullptr;}
   inputPort=outputPort=-1;
 }
 void checkPorts(double now){
   if(now<portCheck)return;
   portCheck=now+.5;
   int a=-1,b=-1,ac=0,bc=0;char name[256];
   for(int i=0;i<GetNumMIDIInputs();++i)if(GetMIDIInputName(i,name,sizeof(name)) && std::strstr(name,"Roto-Control")){a=i;++ac;}
   for(int i=0;i<GetNumMIDIOutputs();++i)if(GetMIDIOutputName(i,name,sizeof(name)) && std::strstr(name,"Roto-Control")){b=i;++bc;}
   if(in && out && ac==1 && bc==1 && a==inputPort && b==outputPort)return;
   if(in || out)closePorts("MIDI ports changed or unavailable; releasing connection");
   if(ac!=1 || bc!=1){
     if(!waitingLogged)note("Waiting for exactly one Roto-Control input/output");
     waitingLogged=true;return;
   }
   in=CreateMIDIInput(a);out=CreateMIDIOutput(b,false,nullptr);
   if(!in || !out){closePorts("Port open failed; will retry");portCheck=now+2;return;}
   inputPort=a;outputPort=b;waitingLogged=false;in->start();
   openedAt=retry=now;healthDeadline=0;sx(10,1);
   note("v0.4.0; Ports opened; DAW STARTED sent");
 }
 int controlMode(int slot){return singleMix?(singlePage+slot==0?0:singlePage+slot==1?1:2):knobMode;}
 MediaTrack* controlTrack(int slot){return singleMix?(singleTrack && ValidatePtr2(nullptr,singleTrack,"MediaTrack*")?singleTrack:nullptr):track(slot);}
 MediaTrack* track(int i){return master?(i==0?GetMasterTrack(nullptr):nullptr):GetTrack(nullptr,bank+i);}
 void send(roto::Bytes b){if(!out)return;std::vector<unsigned char> mem(sizeof(MIDI_event_t)+b.size());auto* e=(MIDI_event_t*)mem.data();e->frame_offset=0;e->size=(int)b.size();std::memcpy(e->midi_message,b.data(),b.size());out->SendMsg(e,-1);}
 void sx(int g,int c,roto::Bytes b={}){send(roto::sysex(g,c,b));}
 void clearTouch(){endSendEdits();touch.fill(false);touched.fill(nullptr);touchSendDestination.fill(nullptr);}
 void resetMeters(){meterLast.fill(-1);meterPeaks.fill(0);meterNext=0;}
 void reset(){resetMeters();dirty=true;last.fill(-1);buttons.fill(-1);haveHi.fill(false);}
 void details(){
   const int count=master?1:std::min(CountTracks(nullptr),16383);
   bank=master?0:std::min(bank,count?((count-1)/8)*8:0);
   bool changed=publishedCount!=count || publishedBank!=bank || publishedMaster!=master;
   for(int i=0;i<8;++i){char name[512]={};auto* t=track(i);int col=0;
     if(t){GetTrackName(t,name,sizeof(name));col=GetTrackColor(t);}
     if(names[i]!=name || colors[i]!=col)changed=true;
     names[i]=name;colors[i]=col;
   }
   if(changed){
     metadata.clear();singleTrackPublished=false;resetMeters();sendDirty=true;
     roto::Bytes b;roto::word(b,count);metadata.push_back(roto::sysex(10,4,b));
     b.clear();roto::word(b,bank);metadata.push_back(roto::sysex(10,5,b));
     for(int i=0;i<8;++i){if(!track(i))continue;
       b.clear();roto::word(b,bank+i);roto::text(b,names[i]);
       // Restore color in the existing batch without adding protocol messages.
       int paletteColor=70;
       if(colors[i]){int r=0,g=0,blue=0;ColorFromNative(colors[i],&r,&g,&blue);paletteColor=roto::nearestColor(r,g,blue);}
       b.push_back(paletteColor);b.push_back(0);metadata.push_back(roto::sysex(10,7,b));
     }
     metadata.push_back(roto::sysex(10,8));
     char msg[160];std::snprintf(msg,sizeof(msg),"Track batch queued: start=%d count=%d existing=%d messages=%zu",bank+1,count,master?1:std::max(0,std::min(8,count-bank)),metadata.size());note(msg);
     last.fill(-1);buttons.fill(-1);metadataNext=time_precise();
   }
   publishedCount=count;publishedBank=bank;publishedMaster=master;dirty=false;
 }

 MediaTrack* sendDestination(int slot=0){int index=singleMix?singlePage+slot-2:sendIndex;return index>=0 && index<(int)sendDestinations.size()?sendDestinations[index]:nullptr;}
 int route(MediaTrack* source,MediaTrack* destination){
   if(!source || !destination || !ValidatePtr2(nullptr,source,"MediaTrack*") || !ValidatePtr2(nullptr,destination,"MediaTrack*"))return -1;
   for(int i=0;i<GetTrackNumSends(source,0);++i)if(GetSetTrackSendInfo(source,0,i,"P_DESTTRACK",nullptr)==destination)return i;
   return -1;
 }
 void endSendEdit(int slot){
   int index=route(sendEditSource[slot],sendEditDestination[slot]);
   if(index>=0)SetTrackSendUIVol(sendEditSource[slot],GetTrackNumSends(sendEditSource[slot],1)+index,sendEditValue[slot],1);
   sendEditSource[slot]=sendEditDestination[slot]=nullptr;
 }
 void endSendEdits(){for(int i=0;i<8;++i)if(sendEditSource[i])endSendEdit(i);}
 void writeSend(int slot,double value,bool instant=false){
   auto* source=controlTrack(slot);auto* destination=sendDestination(slot);
   if(touch[slot] && (touched[slot]!=source || touchSendDestination[slot]!=destination))return;
   int index=route(source,destination);if(index<0)return;
   if(SetTrackSendUIVol(source,GetTrackNumSends(source,1)+index,value,instant || !touch[slot]?-1:0)){
     if(touch[slot] && !instant){sendEditSource[slot]=source;sendEditDestination[slot]=destination;sendEditValue[slot]=value;}
     char msg[140];std::snprintf(msg,sizeof(msg),"Send level: knob=%d destination=%d route=%d volume=%.9f",slot+1,sendIndex,index,value);note(msg);
   }
 }
 void queueSendNames(int start){
   start=std::clamp(start,0,127);roto::Bytes b{(unsigned char)start};
   for(int i=0;i<8;++i)roto::text(b,start+i<(int)sendNames.size()?sendNames[start+i]:"");
   metadata.erase(std::remove_if(metadata.begin(),metadata.end(),[](const roto::Bytes& b){return b.size()>=8 && b[5]==12 && b[6]==8;}),metadata.end());
   auto selection=singleMix?std::find_if(metadata.begin(),metadata.end(),[](const roto::Bytes& p){return p.size()>=8 && p[5]==12 && p[6]==4;}):metadata.end();
   metadata.insert(selection,roto::sysex(12,8,b));metadataNext=time_precise();
   if(singleMix){std::string labels="Single-track send labels: start="+std::to_string(start);
     for(int i=0;i<8;++i)labels+=" | "+(start+i<(int)sendNames.size()?sendNames[start+i]:"<empty>");
     note(labels.c_str());
   }
 }
 void syncSends(double now){
   if(!sendDirty && now<sendPoll)return;
   sendPoll=now+.25;
   // Bitwig uses shared destination buses, not each source's local send slot.
   std::vector<MediaTrack*> destinations;std::vector<std::string> labels;
   std::vector<MediaTrack*> used;
   for(int i=0;i<CountTracks(nullptr);++i){auto* source=GetTrack(nullptr,i);
     for(int j=0;j<GetTrackNumSends(source,0);++j){auto* destination=(MediaTrack*)GetSetTrackSendInfo(source,0,j,"P_DESTTRACK",nullptr);
       if(destination && std::find(used.begin(),used.end(),destination)==used.end())used.push_back(destination);
     }
   }
   for(int i=0;i<CountTracks(nullptr) && destinations.size()<127;++i){auto* t=GetTrack(nullptr,i);
     if(std::find(used.begin(),used.end(),t)==used.end())continue;
     char name[512]={};GetTrackName(t,name,sizeof(name));destinations.push_back(t);labels.emplace_back(name);
   }
   bool topology=destinations!=sendDestinations;
   if(topology)endSendEdits();
   if(sendDirty || topology || labels!=sendNames){
     sendDestinations=std::move(destinations);sendNames=std::move(labels);
     metadata.erase(std::remove_if(metadata.begin(),metadata.end(),[](const roto::Bytes& b){return b.size()>=8 && b[5]==12 && b[6]==3;}),metadata.end());
     auto selection=singleMix?std::find_if(metadata.begin(),metadata.end(),[](const roto::Bytes& p){return p.size()>=8 && p[5]==12 && p[6]==4;}):metadata.end();
     metadata.insert(selection,roto::sysex(12,3,{(unsigned char)sendNames.size()}));queueSendNames(sendNameStart);
     if(topology)last.fill(-1);
     char msg[100];std::snprintf(msg,sizeof(msg),"Send destinations queued: count=%zu selected=%d",sendNames.size(),sendIndex);note(msg);
   }
   sendDirty=false;
 }

 void syncSingle(){
   auto* desired=GetSelectedTrack2(nullptr,0,true);
   if(desired && !ValidatePtr2(nullptr,desired,"MediaTrack*"))desired=nullptr;
   if(desired!=singleTrack){
     // Keep edits on the original valid track until physical release.
     if(held() && controlTrack(0))return;
     clearTouch();singleTrack=desired;singleTrackPublished=false;sendDirty=true;reset();
   }
   char name[512]={};int index=0,r=16,g=16,b=16;
   if(controlTrack(0)){
     GetTrackName(singleTrack,name,sizeof(name));index=std::max(0,(int)GetMediaTrackInfo_Value(singleTrack,"IP_TRACKNUMBER")-1);
     int native=GetTrackColor(singleTrack);
     if(native)ColorFromNative(native,&r,&g,&b);
   }
   // Match Bitwig TrackState.toSysExUpdateSel. The general name command
   // is not part of Bitwig MIX selection and can overwrite a control label.
   int color=controlTrack(0) && GetTrackColor(singleTrack)?roto::nearestColor(r,g,b):70;
   auto packet=roto::selectedTrack(index,name,color);
   auto snapshot=packet;
   if(!singleTrackPublished || snapshot!=publishedSingleTrack){
     metadata.erase(std::remove_if(metadata.begin(),metadata.end(),[](const roto::Bytes& b){return b.size()>=8 && ((b[5]==12 && (b[6]==4 || b[6]==0x0A)) || (b[5]==10 && b[6]==0x16));}),metadata.end());
     metadata.push_back(packet);
     publishedSingleTrack=std::move(snapshot);singleTrackPublished=true;metadataNext=time_precise();
     char msg[160];std::snprintf(msg,sizeof(msg),"Single-track MIX label: index=%d page=%d name=%.100s",index,singlePage,name);note(msg);
   }
 }

 bool validFxTrack(){return fxTrack && ValidatePtr2(nullptr,fxTrack,"MediaTrack*");}
 bool held(){return std::any_of(touch.begin(),touch.end(),[](bool t){return t;});}
 std::string identity(MediaTrack* t,int index){
   char id[2048]={};if(!TrackFX_GetNamedConfigParm(t,index,"fx_ident",id,sizeof(id)) || !id[0]){
     if(!TrackFX_GetNamedConfigParm(t,index,"fx_name",id,sizeof(id)) || !id[0])TrackFX_GetFXName(t,index,id,sizeof(id));
   }
   char type[64]={};TrackFX_GetNamedConfigParm(t,index,"fx_type",type,sizeof(type));
   if(mappingSource==roto::MappingSource::Ableton && (std::strcmp(type,"VST3")==0 || std::strcmp(type,"VST3i")==0)){
     char original[2048]={};
     if(!TrackFX_GetNamedConfigParm(t,index,"original_name",original,sizeof(original)) || !original[0])
       TrackFX_GetNamedConfigParm(t,index,"fx_name",original,sizeof(original));
     const auto name=roto::abletonVst3Name(original);
     if(!name.empty())return roto::abletonIdentity(name);
     note("Ableton VST3 name unavailable; using REAPER mapping identity");
   }
   return roto::mappingIdentity(type,id);
 }
 std::string instance(MediaTrack* t,int index){
   auto* guid=TrackFX_GetFXGUID(t,index);if(!guid)return {};
   return std::string((const char*)guid,sizeof(GUID));
 }
 bool validFx(){return validFxTrack() && fx>=0 && fx<std::min(TrackFX_GetCount(fxTrack),127) && instance(fxTrack,fx)==fxToken;}
 void clearBindings(){knobParams.fill(-1);switchParams.fill(-1);last.fill(-1);buttons.fill(-1);haveHi.fill(false);mappingTime=-100;}
 void endPluginEdits(){if(validFx())for(int i:knobParams)if(i>=0 && i<(int)params.size())TrackFX_EndParamEdit(fxTrack,fx,i);}
 void selectFx(int index){
   endPluginEdits();metadata.clear();pluginTrackPublished=false;clearBindings();learning=false;learnParam=-1;params.clear();
   const int count=validFxTrack()?std::min(TrackFX_GetCount(fxTrack),127):0;
   fx=index>=0 && index<count?index:-1;fxToken=fx>=0?instance(fxTrack,fx):"";
   if(fx>=0){
     const auto id=identity(fxTrack,fx);const int count=std::min(TrackFX_GetNumParams(fxTrack,fx),16384);
     for(int p=0;p<count;++p){char name[512]={},ident[512]={};TrackFX_GetParamName(fxTrack,fx,p,name,sizeof(name));
       if(!TrackFX_GetParamIdent(fxTrack,fx,p,ident,sizeof(ident)) || !ident[0])std::snprintf(ident,sizeof(ident),"index:%d",p);
       const std::string parameterId=std::strncmp(ident,"index:",6)==0?std::string(ident)+"/name:"+name:std::string(ident);
       params.push_back({name,roto::mappingParamHash(id,parameterId,name)});
     }
     firstFx=(fx/8)*8;
   }
   pluginControls=fx>=0;pluginDirty=true;
   char msg[160];std::snprintf(msg,sizeof(msg),"Plugin selected: fx=%d parameters=%zu",fx,params.size());note(msg);
 }
 void syncPlugin(){
   if(!plugin)return;
   // Follow host selection at each frame; changing targets while held is deferred.
   if(held() && validFx())return;
   MediaTrack* desired=GetSelectedTrack2(nullptr,0,true);int desiredFx=-1;
   int ti=-2,item=-1,take=-1,focused=-1,status=0;
   if(!pluginLocked && !learning && GetTouchedOrFocusedFX(1,&ti,&item,&take,&focused,&status) && item==-1 && !(status&1)){
     auto* t=ti==-1?GetMasterTrack(nullptr):GetTrack(nullptr,ti);
     if(t && ValidatePtr2(nullptr,t,"MediaTrack*") && focused>=0 && focused<std::min(TrackFX_GetCount(t),127)){
       const auto token=instance(t,focused);
       if(token!=focusToken){focusToken=token;desired=t;desiredFx=focused;SetOnlyTrackSelected(t);}
     }
   }
   if(pluginLocked && validFx())return;
   if(learning && validFx())return;
   if(desired!=fxTrack){endPluginEdits();fxTrack=desired;firstFx=0;selectFx(desiredFx>=0?desiredFx:0);pluginSnapshot.clear();}
   else if(desiredFx>=0 && desiredFx!=fx)selectFx(desiredFx);
   else if((fx>=0 && !validFx()) || (fx<0 && validFxTrack() && TrackFX_GetCount(fxTrack)>0))selectFx(0);
   else if(validFx() && std::min(TrackFX_GetNumParams(fxTrack,fx),16384)!=(int)params.size())selectFx(fx);
   if(time_precise()-pluginPoll>.25 || pluginDirty){
     pluginPoll=time_precise();std::string snapshot;
     if(validFxTrack())for(int i=0;i<std::min(TrackFX_GetCount(fxTrack),127);++i){char name[512]={};TrackFX_GetFXName(fxTrack,i,name,sizeof(name));snapshot+=instance(fxTrack,i)+identity(fxTrack,i)+name+(TrackFX_GetEnabled(fxTrack,i)?"1":"0");}
     if(snapshot!=pluginSnapshot){pluginSnapshot=snapshot;pluginDirty=true;}
   }
 }
 void pluginDetails(){
   metadata.clear();pluginTrackPublished=false;pluginTrackDetails();
   const int count=validFxTrack()?std::min(TrackFX_GetCount(fxTrack),127):0;
   firstFx=std::clamp(firstFx,0,count?((count-1)/8)*8:0);
   metadata.push_back(roto::sysex(11,2,{(unsigned char)count}));metadata.push_back(roto::sysex(11,3,{(unsigned char)firstFx}));
   for(int i=firstFx;i<std::min(firstFx+8,count);++i){char name[512]={};TrackFX_GetFXName(fxTrack,i,name,sizeof(name));metadata.push_back(roto::pluginDetails(i,identity(fxTrack,i),TrackFX_GetEnabled(fxTrack,i),name));}
   metadata.push_back(roto::sysex(11,6));
   if(pluginControls && validFx() && fx>=firstFx && fx<firstFx+8)metadata.push_back(roto::sysex(11,8,{(unsigned char)fx,0,0}));
   pluginDirty=false;metadataNext=time_precise();note("Plugin list queued; awaiting controller mappings");
 }
 // Official Bitwig selection packet also supplies the track label in PLUGIN.
 void pluginTrackDetails(){
   char name[512]={};int index=0,color=70;
   if(validFxTrack()){
     GetTrackName(fxTrack,name,sizeof(name));
     index=std::max(0,(int)GetMediaTrackInfo_Value(fxTrack,"IP_TRACKNUMBER")-1);
     int native=GetTrackColor(fxTrack);
     if(native){int r=0,g=0,b=0;ColorFromNative(native,&r,&g,&b);color=roto::nearestColor(r,g,b);}
   }
   auto packet=roto::selectedTrack(index,name,color);
   if(!pluginTrackPublished || packet!=publishedPluginTrack){
     // Replace an obsolete queued label if selection/rename arrives mid-batch.
     metadata.erase(std::remove_if(metadata.begin(),metadata.end(),[](const roto::Bytes& b){return b.size()>=8 && ((b[5]==12 && b[6]==4) || (b[5]==10 && b[6]==0x16));}),metadata.end());
     // The new track context must precede DAW SELECT PLUGIN. Use the
     // explicit name command too, so renames update an already visible header.
     auto select=std::find_if(metadata.begin(),metadata.end(),[](const roto::Bytes& b){return b.size()>=8 && b[5]==11 && b[6]==8;});
     auto next=metadata.insert(select,packet);++next;
     metadata.insert(next,roto::currentTrackName(name));
     publishedPluginTrack=packet;pluginTrackPublished=true;
     char msg[160];std::snprintf(msg,sizeof(msg),"Plugin track label queued: index=%d name=%.100s",index,name);note(msg);
   }
 }
 roto::Bytes learnMessage(int p,int wireIndex){
   std::vector<std::string> labels;double step=0,smallStep=0,largeStep=0;bool toggle=false;
   if(TrackFX_GetParameterStepSizes(fxTrack,fx,p,&step,&smallStep,&largeStep,&toggle)){
     int steps=toggle?2:0;
     if(!toggle && std::isfinite(step) && step>0 && step<=1){double n=1/step;if(n>=1 && n<=15 && std::abs(n-std::round(n))<1e-5)steps=(int)std::round(n)+1;}
     for(int i=0;i<steps;++i){char text[512]={};if(!TrackFX_FormatParamValueNormalized(fxTrack,fx,p,double(i)/(steps-1),text,sizeof(text))){labels.clear();break;}labels.emplace_back(text);}
   }
   double value=TrackFX_GetParamNormalized(fxTrack,fx,p);if(!std::isfinite(value))value=0;
   return roto::learnParam(wireIndex,params[p].hash,(int)std::lround(16383*std::clamp(value,0.0,1.0)),params[p].name,labels);
 }
 void captureLearnBaseline(){
   int item=-1,take=-1;baselineTrack=-2;baselineFx=baselineParam=-1;
   if(GetTouchedOrFocusedFX(0,&baselineTrack,&item,&take,&baselineFx,&baselineParam) && item==-1){
     auto* t=baselineTrack==-1?GetMasterTrack(nullptr):GetTrack(nullptr,baselineTrack);
     if(t==fxTrack && validFx() && baselineFx==fx && baselineParam>=0 && baselineParam<(int)params.size())baselineValue=TrackFX_GetParamNormalized(t,fx,baselineParam);
   }
 }
 void pollLearn(){
   if(!learning || !validFx() || time_precise()-learnTime<.1)return;
   int ti=-2,item=-1,take=-1,index=-1,p=-1;
   if(!GetTouchedOrFocusedFX(0,&ti,&item,&take,&index,&p) || item!=-1 || index!=fx || p<0 || p>=(int)params.size())return;
   auto* t=ti==-1?GetMasterTrack(nullptr):GetTrack(nullptr,ti);if(t!=fxTrack)return;
   double value=TrackFX_GetParamNormalized(t,fx,p);
   if(ti==baselineTrack && index==baselineFx && p==baselineParam && value==baselineValue)return;
   baselineTrack=ti;baselineFx=index;baselineParam=p;baselineValue=value;learnParam=p;learnTime=time_precise();
   metadata.push_back(learnMessage(p,p));char msg[120];std::snprintf(msg,sizeof(msg),"Learn candidate: parameter=%d name=%s",p,params[p].name.c_str());note(msg);
 }
 void mapped(const unsigned char* p){
   if(!validFx() || p[15]>1 || p[16]>7 || p[17]!=0)return;
   const double now=time_precise();
   if(!learning && now-mappingTime>.25){clearBindings();note("Mapping recall batch started");}
   mappingTime=now;motorReady=now+.25;
   int found=-1,matches=0;const int hint=roto::word(p+7);
   for(int i=0;i<(int)params.size();++i)if(std::equal(params[i].hash.begin(),params[i].hash.end(),p+9)){found=i;++matches;}
   if(matches>1)found=hint<(int)params.size() && std::equal(params[hint].hash.begin(),params[hint].hash.end(),p+9)?hint:-1;
   auto& slots=p[15]==0?knobParams:switchParams;slots[p[16]]=found;haveHi[p[16]]=false;last[p[16]]=-1;buttons[p[16]]=-1;
   if(found>=0){metadata.push_back(learnMessage(found,hint));pluginControls=true;}
   char msg[120];std::snprintf(msg,sizeof(msg),"Control mapped: type=%u slot=%u parameter=%d hashMatches=%d",p[15],p[16],found,matches);note(msg);
 }
 bool pluginEvent(int c,const unsigned char* p,int n){
   if(c==1){metadata.clear();plugin=true;mix=false;singleMix=false;pluginLocked=false;pluginControls=false;learning=false;focusToken.clear();clearBindings();pluginSnapshot.clear();pluginDirty=true;syncPlugin();selectFx(fx>=0?fx:0);note("PLUGIN mode active: native learned mappings");return true;}
   if(!plugin)return false;
   char msg[80];std::snprintf(msg,sizeof(msg),"Plugin RX: command=%02X size=%d",c,n);note(msg);
   if(c==4 && n>=9){firstFx=(p[7]/8)*8;pluginDirty=true;clearBindings();pluginControls=false;}
   if(c==7 && n>=9 && !held()){selectFx(p[7]);if(validFx())TrackFX_Show(fxTrack,fx,3);}
   if(c==9 && n>=9){learning=p[7]!=0;learnParam=-1;captureLearnBaseline();if(learning)endPluginEdits();clearBindings();note(learning?"Plugin learn ON; move a parameter in REAPER":"Plugin learn OFF; awaiting mapping recall");}
   if(c==11 && n>=19)mapped(p);
   if(c==12 && n>=10 && validFxTrack() && p[7]<std::min(TrackFX_GetCount(fxTrack),127)){TrackFX_SetEnabled(fxTrack,p[7],p[8]!=0);pluginDirty=true;}
   if(c==13 && n>=9){pluginLocked=p[7]!=0;}
   if(c==0x17 || c==0x16 || c==0x15){endPluginEdits();clearBindings();pluginControls=false;}
   if(c==0x17)pluginDirty=true;
   if(c==0x11 && n>=10){learnParam=-1;captureLearnBaseline();note("Control learned acknowledgment received");}
   return true;
 }
 void pluginFeedback(){
   if(!pluginControls || learning || !validFx())return;
   for(int i=0;i<8;++i){int p=knobParams[i];
     if(p>=0 && p<(int)params.size()){double x=TrackFX_GetParamNormalized(fxTrack,fx,p);if(std::isfinite(x)){int v=(int)std::lround(16383*std::clamp(x,0.0,1.0));
       if(!touch[i] && last[i]!=v){send({191,(unsigned char)(12+i),(unsigned char)(v>>7)});send({191,(unsigned char)(44+i),(unsigned char)(v&127)});last[i]=v;}
     }}
     p=switchParams[i];if(p>=0 && p<(int)params.size()){double x=TrackFX_GetParamNormalized(fxTrack,fx,p);if(!std::isfinite(x))continue;int v=(int)std::lround(127*std::clamp(x,0.0,1.0));if(buttons[i]!=v){send({191,(unsigned char)(20+i),(unsigned char)v});buttons[i]=v;}}
   }
 }
 void meterFeedback(double now){
   if(plugin || !mix || !supported)return;
   for(int i=0;i<8;++i){auto* t=singleMix?(i==0 && singlePage==0?controlTrack(0):nullptr):track(i);
     for(int ch=0;ch<2;++ch){int slot=2*i+ch;double peak=t?Track_GetPeakInfo(t,ch):0;
       if(std::isfinite(peak) && peak>meterPeaks[slot])meterPeaks[slot]=peak;
     }
   }
   // Metadata establishes the bank first; hold maximum peaks between 20 Hz updates.
   if(!metadata.empty() || now<motorReady || now<meterNext)return;
   for(int slot=0;slot<16;++slot){int v=roto::meterLevel(meterPeaks[slot]);
     if(meterLast[slot]!=v){send({191,(unsigned char)(65+slot),(unsigned char)v});meterLast[slot]=v;}
   }
   meterPeaks.fill(0);meterNext=now+0.05;
 }
 void event(MIDI_event_t* e){auto* p=e->midi_message;int n=e->size;
   if(n>=8 && p[0]==240 && p[1]==0 && p[2]==34 && p[3]==3 && p[4]==2 && p[n-1]==247){
     int g=p[5],c=p[6];
     if(g==10 && c==2){connected=false;endPluginEdits();metadata.clear();clearTouch();clearBindings();pluginTrackPublished=false;openedAt=retry=time_precise();healthDeadline=0;sx(10,3,{2});note("Ping received; Bitwig ID 02 response sent");return;}
     if(g==10 && c==12){connected=true;healthDeadline=0;healthNext=time_precise()+2;clearTouch();clearBindings();pluginDirty=true;pluginTrackPublished=false;singleTrackPublished=false;publishedCount=-1;publishedBank=-1;reset();sx(10,15);sx(10,13);note("ROTO-DAW CONNECTED");return;}
     if(!connected)return;
     if(g==10 && c==16 && n>=10){healthDeadline=0;char v[80];std::snprintf(v,sizeof(v),"SysEx API %u.%u",p[7],p[8]);note(v);}
     if(g==10 && c==14 && n>=11){char v[80];std::snprintf(v,sizeof(v),"Firmware %u.%u.%u",p[7],p[8],p[9]);note(v);}
     if(g==10 && c==6 && n>=10){bank=(roto::word(p+7)/8)*8;reset();char msg[100];std::snprintf(msg,sizeof(msg),"Bank requested: tracks %d-%d",bank+1,bank+8);note(msg);}
     if(g==10 && c==9 && n>=10){auto* t=master?GetMasterTrack(nullptr):GetTrack(nullptr,roto::word(p+7));if(t)SetOnlyTrackSelected(t);if(singleMix)syncSingle();}
     if(g==11){endSendEdits();pluginEvent(c,p,n);return;}
     if(g==12 && c==1 && n>=12){endSendEdits();endPluginEdits();clearBindings();plugin=false;learning=false;metadata.clear();singleMix=false;singleTrackPublished=false;publishedCount=-1;mix=true;master=p[7]!=0;knobMode=p[8];buttonMode=p[9];sendIndex=p[10];sendNameStart=(sendIndex/8)*8;sendDirty=true;char msg[140];std::snprintf(msg,sizeof(msg),"MIX payload: master=%u knob=%u button=%u send=%u",p[7],p[8],p[9],p[10]);note(msg);supported=knobMode<=2;reset();if(supported){sx(12,11,{(unsigned char)roto::meterLevel(std::pow(10.0,-12.0/20)),(unsigned char)roto::meterLevel(1.0)});note("Stereo MIX meters enabled: 20 Hz, -60 to +6 dBFS; yellow -12, red 0 dBFS");}note(supported?"MIX all-tracks active":"Unknown knob mode; controls ignored");}
     if(g==12 && c==7 && n>=9){sendNameStart=(p[7]/8)*8;if(mix && supported){syncSends(time_precise());queueSendNames(sendNameStart);}return;}
     if(g==12 && c==2 && n>=9){
       if(p[7]>120 || p[7]%8)return;
       endPluginEdits();clearTouch();clearBindings();plugin=false;learning=false;pluginLocked=false;
       metadata.clear();mix=true;singleMix=true;supported=true;singlePage=p[7];singleTrackPublished=false;
       publishedCount=-1;sendNameStart=std::max(0,singlePage-2);sendDirty=true;reset();
       sx(12,11,{(unsigned char)roto::meterLevel(std::pow(10.0,-12.0/20)),(unsigned char)roto::meterLevel(1.0)});
       note("Single-track MIX active: Level, Pan and sends; buttons inactive");
     }
     if(g==12 && c==5 && n>=9){if(singleMix)clearTouch();singleMix=false;master=p[7]!=0;reset();}
     if(g==10 && c==10){int s=GetPlayState();sx(10,11,{(unsigned char)((s&1)!=0),(unsigned char)((s&2)!=0),(unsigned char)((s&4)!=0),0,(unsigned char)(GetSetRepeat(-1)!=0),0,0,0});}
     return;
   }
   if(!connected || n!=3 || p[0]!=191)return;
   int cc=p[1],v=p[2];
   if(cc>=28 && cc<=35 && v){if(cc==28)CSurf_OnPlay();if(cc==29)CSurf_OnStop();if(cc==30)CSurf_OnRecord();return;}
   if(cc>=52 && cc<=59){int i=cc-52;if(!v && sendEditSource[i])endSendEdit(i);touch[i]=v!=0;touchSendDestination[i]=touch[i] && !plugin && controlMode(i)==2?sendDestination(i):nullptr;touched[i]=touch[i] && !plugin?controlTrack(i):nullptr;
     if(plugin && !touch[i] && validFx() && knobParams[i]>=0)TrackFX_EndParamEdit(fxTrack,fx,knobParams[i]);
     char msg[100];std::snprintf(msg,sizeof(msg),"Knob %d touch %s (RX CC=%d value=%d)",i+1,touch[i]?"ON":"OFF",cc,v);note(msg);if(singleMix && !touch[i])syncSingle();return;}
   if(plugin){
     if(!pluginControls || learning || !validFx())return;
     if(cc>=12 && cc<=19){hi[cc-12]=v;haveHi[cc-12]=true;return;}
     if(cc>=44 && cc<=51){int i=cc-44,p=knobParams[i];if(!haveHi[i] || p<0 || p>=(int)params.size())return;
       TrackFX_SetParamNormalized(fxTrack,fx,p,((hi[i]<<7)|v)/16383.0);if(!touch[i])TrackFX_EndParamEdit(fxTrack,fx,p);return;}
     if(cc>=20 && cc<=27){int p=switchParams[cc-20];if(p>=0 && p<(int)params.size()){TrackFX_SetParamNormalized(fxTrack,fx,p,v/127.0);TrackFX_EndParamEdit(fxTrack,fx,p);}return;}
     return;
   }
   if(!mix || !supported)return;
   // Appendix B.1 CC 64: knob index requesting the MIX unity/default level.
   // The official Ableton integration treats the value as an index, not a level.
   if(cc==64){
     if(v>=8)return;
     if(controlMode(v)!=0)return;
     auto* t=controlTrack(v);if(!t)return;
     double val=CSurf_OnVolumeChange(t,1.0,false);hostVolumes[t]=val;
     CSurf_SetSurfaceVolume(t,val,this);last[v]=-1;
     char msg[120];std::snprintf(msg,sizeof(msg),"MIX unity event: knob=%d track=%d volume=%.9f",v+1,master?0:bank+v+1,val);note(msg);return;
   }
   if(cc>=12 && cc<=19){hi[cc-12]=v;haveHi[cc-12]=true;return;}
   if(cc>=44 && cc<=51){int i=cc-44;if(!haveHi[i])return;auto* t=controlTrack(i);if(!t)return;if(singleMix && touch[i] && touched[i]!=t)return;double x=((hi[i]<<7)|v)/16383.0;
     if(controlMode(i)==2){writeSend(i,roto::volume(x));return;}
     if(controlMode(i)==1){double val=CSurf_OnPanChange(t,2*x-1,false);hostPans[t]=val;CSurf_SetSurfacePan(t,val,this);}else{double val=CSurf_OnVolumeChange(t,roto::volume(x),false);hostVolumes[t]=val;CSurf_SetSurfaceVolume(t,val,this);}return;}
   if(cc>=20 && cc<=27 && v && !singleMix){auto* t=track(cc-20);if(!t)return;
     if(buttonMode==0)CSurf_SetSurfaceMute(t,CSurf_OnMuteChange(t,-1),this);
     if(buttonMode==1)CSurf_SetSurfaceSolo(t,CSurf_OnSoloChange(t,-1),this);
     if(buttonMode==2)CSurf_SetSurfaceRecArm(t,CSurf_OnRecArmChange(t,-1),this);
   }
 }
public:
 Surface(int* err,const char* config=nullptr):mappingSource(roto::parseMappingSource(config)){
   last.fill(-1);std::string path=std::string(GetResourcePath())+"/roto-reaper.log";log=std::fopen(path.c_str(),"w");
   // Keep the surface installed even when hardware is absent; Run retries.
   if(err)*err=0;
   checkPorts(time_precise());
 }
 ~Surface(){closePorts("Surface closed");if(log)std::fclose(log);}
 const char* GetTypeString()override{return "ROTOREAPER";}
 const char* GetDescString()override{return "Roto-Control REAPER v0.4.0";}
 // Keep a configuration token so REAPER can reload the saved surface entry.
 const char* GetConfigString()override{return roto::mappingSourceConfig(mappingSource);}
 bool GetTouchState(MediaTrack* t,int pan)override{if(plugin || (pan!=0 && pan!=1))return false;for(int i=0;i<8;++i)if(touch[i] && touched[i]==t && controlMode(i)<2 && ((pan==1)==(controlMode(i)==1)))return true;return false;}
 void SetSurfaceVolume(MediaTrack* t,double value)override{if(t)hostVolumes[t]=value;}
 void SetSurfacePan(MediaTrack* t,double value)override{if(t)hostPans[t]=value;}
 void SetTrackListChange()override{sendDirty=true;hostVolumes.clear();hostPans.clear();reset();}
 void SetTrackTitle(MediaTrack*,const char*)override{dirty=true;sendDirty=true;}
 void Run()override{
   double now=time_precise();checkPorts(now);if(!in || !out)return;
   if(connected && plugin)syncPlugin();if(connected && singleMix)syncSingle();in->SwapBufsPrecise((unsigned int)(now*1000),now);int pos=0;auto* list=in->GetReadBuf();while(auto* e=list->EnumItems(&pos))event(e);
   if(!connected){
     if(now-openedAt>8){closePorts("Handshake timed out; reopening MIDI ports");portCheck=now+1;return;}
     if(now-retry>3){sx(10,1);retry=now;}return;
   }
   // Input/output handles have no disconnect/error API. A version reply
   // confirms both directions still work, even when the controller is idle.
   if(healthDeadline && now>=healthDeadline){
     closePorts("Controller response timed out; reopening MIDI ports");portCheck=now+1;return;
   }
   if(!healthDeadline && now>=healthNext){healthDeadline=now+5;healthNext=now+2;sx(10,15);}
   if(plugin){syncPlugin();if(pluginDirty)pluginDetails();pluginTrackDetails();pollLearn();}
   else {
     if(!mix || !supported)return;
     // TRACK DETAILS names address individual mixer screens. In single-track
     // mode those screens are Level/Pan/send controls, not banked track names.
     if(singleMix){syncSingle();dirty=false;}
     else if(dirty || now-refresh>2){details();refresh=now;}
     syncSends(now);
   }
   meterFeedback(now);
   if(!metadata.empty()){
     if(now>=metadataNext){auto b=metadata.front();metadata.pop_front();send(b);metadataNext=now+0.03;
       char msg[160];std::snprintf(msg,sizeof(msg),"Metadata TX: group=%02X command=%02X size=%zu remaining=%zu",b[5],b[6],b.size(),metadata.size());note(msg);
       if(metadata.empty()){motorReady=std::max(motorReady,now+0.05);note("Metadata batch sent; motor feedback resumes after settling");}
     }
     return;
   }
   if(now<motorReady)return;
   if(plugin){pluginFeedback();return;}
   for(int i=0;i<8;++i){auto* t=controlTrack(i);if(!t && !singleMix)continue;int mode=controlMode(i);
     auto& hostValues=mode==1?hostPans:hostVolumes;
     auto value=hostValues.find(t);
     double val=t?(value!=hostValues.end()?value->second:GetMediaTrackInfo_Value(t,mode==1?"D_PAN":"D_VOL")):0;
     if(mode==2){double sendPan=0;int index=route(t,sendDestination(i));val=0;
       if(index>=0 && !GetTrackSendUIVolPan(t,GetTrackNumSends(t,1)+index,&val,&sendPan))val=0;
     }
     if(!std::isfinite(val))val=0;
     int v=(int)std::lround(16383*(mode==1?std::clamp((val+1)/2,0.0,1.0):roto::position(val)));
     if(!touch[i] && last[i]!=v){send({191,(unsigned char)(12+i),(unsigned char)(v>>7)});send({191,(unsigned char)(44+i),(unsigned char)(v&127)});last[i]=v;char msg[120];std::snprintf(msg,sizeof(msg),"Motor TX: knob=%d value=%d touch=%d track=%d",i+1,v,touch[i],bank+i+1);note(msg);}
     const char* key=buttonMode==1?"I_SOLO":buttonMode==2?"I_RECARM":"B_MUTE";
     int state=!singleMix && t && GetMediaTrackInfo_Value(t,key)>0?127:0;
     if(buttons[i]!=state){send({191,(unsigned char)(20+i),(unsigned char)state});buttons[i]=state;}
   }
 }
};
static IReaperControlSurface* create(const char*,const char* config,int* err){return new Surface(err,config);}
// REAPER calls this callback unconditionally when selecting the surface type.
static HWND showConfig(const char*,HWND parent,const char* config) {
#ifdef ROTO_SURFACE_TEST
 return nullptr;
#else
 return rotoShowConfig(parent,config,rotoInstance);
#endif
}
static reaper_csurf_reg_t reg={"ROTOREAPER","Roto-Control REAPER v0.4.0",create,showConfig};
extern "C" REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE instance,reaper_plugin_info_t* rec){
#ifndef ROTO_SURFACE_TEST
rotoInstance=instance;
#endif
if(!rec)return 0;if(rec->caller_version!=REAPER_PLUGIN_VERSION || REAPERAPI_LoadAPI(rec->GetFunc))return 0;return rec->Register("csurf",&reg);}
