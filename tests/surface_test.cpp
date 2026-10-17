#undef NDEBUG
#define ROTO_SURFACE_TEST
#include "../src/extension.cpp"
#include <cassert>
#include <iostream>
struct Events: MIDI_eventlist {
 std::vector<std::vector<unsigned char>> items;
 void AddItem(MIDI_event_t* e)override {std::vector<unsigned char> b(sizeof(MIDI_event_t)+e->size);std::memcpy(b.data(),e,sizeof(MIDI_event_t));std::memcpy(((MIDI_event_t*)b.data())->midi_message,e->midi_message,e->size);items.push_back(b);}
 void push(roto::Bytes b){std::vector<unsigned char> v(sizeof(MIDI_event_t)+b.size());auto* e=(MIDI_event_t*)v.data();e->size=b.size();std::memcpy(e->midi_message,b.data(),b.size());items.push_back(v);}
 MIDI_event_t* EnumItems(int* p)override {if(*p>=(int)items.size()){items.clear();return nullptr;}return (MIDI_event_t*)items[(*p)++].data();}
 void DeleteItem(int)override{} int GetSize()override{return items.size();}void Empty()override{items.clear();}
};
static bool autoReply=true,openInput=true,openOutput=true;
static int portCount=1,portIndex=0,inputCreates=0,outputCreates=0,inputDestroys=0,outputDestroys=0;
struct In:midi_Input {Events q;void start()override{}void stop()override{q.Empty();}void SwapBufs(unsigned int)override{}MIDI_eventlist* GetReadBuf()override{return &q;}void Destroy()override{++inputDestroys;}} input;
struct Out:midi_Output {std::vector<roto::Bytes> sent;void SendMsg(MIDI_event_t* e,int)override{
 sent.emplace_back(e->midi_message,e->midi_message+e->size);
 if(autoReply && sent.back()==roto::sysex(10,15))input.q.push(roto::sysex(10,16,{1,6}));
 }void Send(unsigned char,unsigned char,unsigned char,int)override{}void Destroy()override{++outputDestroys;}} output;
static double peakLeft=0,peakRight=0;static int peakReads=0;
static double clockTime=0,vol=1,pan=0;static int changes=0;static int fakeTrack=0;static int trackCount=1;static int trackColor=0;
static std::string firstName="Drums",secondName="Bass";
static int secondTrack=0,fxCount=2,fxChanges=0,editEnds=0,selectedFx=0;
static bool targetValid=true,focusActive=false,lastTouched=false,toggleParam=false;
static int lastParam=0,lastFx=0,focusedFx=0,focusedTrack=0;
static MediaTrack* lastFxWrite=nullptr;
static MediaTrack* selected=(MediaTrack*)&fakeTrack;
static double fxValues[2][10]={{.2,.3,.4,.5,.6,.7,.8,.9,.1,.25},{.6}};
static GUID fxGuids[2]={{1},{2}},secondFxGuids[2]={{11},{12}};
static void pump(Surface& s){for(int i=0;i<20;++i){clockTime+=.034;s.Run();}}
static int sendTracks[10];
static double trackVolumes[10]={1,.3},trackPans[10]={0,-.4};
struct TestSend {MediaTrack* destination;double value;};
static std::vector<TestSend> routes[10];
static int sendWrites=0,sendEnds=0,sendInstants=0,lastSendIndex=-1;
static MediaTrack* lastSendTrack=nullptr;
static int sourceIndex(MediaTrack* t){for(int i=0;i<10;++i)if(t==(MediaTrack*)&sendTracks[i])return i;return -1;}
static bool formatFails=false;static std::string displaySuffix;
static bool liveMode=false,auMode=false,vst2Mode=false;
int main(int argc,char** argv){
 liveMode=argc>1;auMode=liveMode && std::strcmp(argv[1],"au")==0;vst2Mode=liveMode && std::strcmp(argv[1],"vst2")==0;
 assert(reg.ShowConfig != nullptr);
 assert(reg.ShowConfig(reg.type_string,nullptr,"") == nullptr);
 time_precise=+[](){return clockTime;};GetResourcePath=+[]()->const char*{return ".";};
 GetNumMIDIInputs=GetNumMIDIOutputs=+[](){return portCount;};
 GetMIDIInputName=GetMIDIOutputName=+[](int i,char* b,int){std::strcpy(b,i==portIndex || (portCount==2 && portIndex==0)?"Roto-Control":"Other MIDI");return true;};
 CreateMIDIInput=+[](int)->midi_Input*{++inputCreates;return openInput?&input:nullptr;};CreateMIDIOutput=+[](int,bool,int*)->midi_Output*{++outputCreates;return openOutput?&output:nullptr;};
 CountTracks=+[](ReaProject*){return trackCount;};GetTrack=+[](ReaProject*,int n)->MediaTrack*{return n>=0 && n<trackCount?(MediaTrack*)&fakeTrack:nullptr;};GetMasterTrack=+[](ReaProject*)->MediaTrack*{return (MediaTrack*)&fakeTrack;};
 GetTrackName=+[](MediaTrack* t,char* b,int size){std::snprintf(b,size,"%s",(t==(MediaTrack*)&secondTrack?secondName:firstName).c_str());return true;};GetTrackColor=+[](MediaTrack*){return trackColor;};
 ColorFromNative=+[](int c,int* r,int* g,int* b){*r=(c>>16)&255;*g=(c>>8)&255;*b=c&255;};
 GetMediaTrackInfo_Value=+[](MediaTrack* t,const char* key){if(std::strcmp(key,"IP_TRACKNUMBER")==0)return t==(MediaTrack*)&secondTrack?2.0:1.0;return std::strcmp(key,"D_VOL")==0?vol:std::strcmp(key,"D_PAN")==0?pan:0.0;};
 Track_GetPeakInfo=+[](MediaTrack*,int ch){++peakReads;return ch?peakRight:peakLeft;};
 CSurf_OnVolumeChange=+[](MediaTrack*,double v,bool){++changes;return vol=v;};CSurf_OnPanChange=+[](MediaTrack*,double v,bool){++changes;return pan=v;};
 CSurf_SetSurfaceVolume=CSurf_SetSurfacePan=+[](MediaTrack*,double,IReaperControlSurface*){};
 GetSelectedTrack2=+[](ReaProject*,int,bool)->MediaTrack*{return selected;};
 ValidatePtr2=+[](ReaProject*,void* p,const char*){return targetValid && (p==&fakeTrack || p==&secondTrack);};
 TrackFX_GetCount=+[](MediaTrack*){return fxCount;};
 TrackFX_GetFXGUID=+[](MediaTrack* t,int index)->GUID*{return index>=0 && index<2?&(t==(MediaTrack*)&secondTrack?secondFxGuids:fxGuids)[index]:nullptr;};
 TrackFX_GetFXName=+[](MediaTrack*,int index,char* b,int){std::strcpy(b,index?"Other FX":"ReaEQ");return true;};
 TrackFX_GetNamedConfigParm=+[](MediaTrack*,int index,const char* key,char* b,int){
   if(liveMode && !std::strcmp(key,"fx_type"))std::strcpy(b,auMode?"AU":vst2Mode?"VST":"VST3");
   else if(liveMode && (!std::strcmp(key,"original_name") || !std::strcmp(key,"fx_name")))std::snprintf(b,512,"%s: %s",auMode?"AU":vst2Mode?"VST":"VST3",index?"Other FX (Vendor)":"UADx Pultec EQP-1A EQ (Universal Audio (UADx))");
   else std::strcpy(b,index?"other-fx":"reaeq");return true;
 };
 TrackFX_GetEnabled=+[](MediaTrack*,int){return true;};TrackFX_SetEnabled=+[](MediaTrack*,int,bool){};
 TrackFX_GetNumParams=+[](MediaTrack*,int){return 10;};
 TrackFX_GetParamName=+[](MediaTrack*,int,int p,char* b,int size){std::snprintf(b,size,"Param %d",p);return true;};
 TrackFX_GetParamIdent=+[](MediaTrack*,int,int p,char* b,int size){std::snprintf(b,size,"param%d",p);return true;};
 TrackFX_GetParamNormalized=+[](MediaTrack*,int f,int p){return fxValues[f][p];};
 TrackFX_SetParamNormalized=+[](MediaTrack* t,int f,int p,double v){lastFxWrite=t;++fxChanges;fxValues[f][p]=v;return true;};
 TrackFX_EndParamEdit=+[](MediaTrack*,int,int){++editEnds;return true;};
 TrackFX_GetParameterStepSizes=+[](MediaTrack*,int,int,double* step,double*,double*,bool* toggle){*step=0;*toggle=toggleParam;return toggleParam;};
 TrackFX_FormatParamValueNormalized=+[](MediaTrack*,int,int,double v,char* b,int size){if(formatFails)return false;std::snprintf(b,size,"%s%s",v>.5?"On":"Off",displaySuffix.c_str());return true;};
 TrackFX_Show=+[](MediaTrack*,int f,int){selectedFx=f;};
 SetOnlyTrackSelected=+[](MediaTrack* t){selected=t;};
 GetTouchedOrFocusedFX=+[](int mode,int* ti,int* item,int* take,int* f,int* p){*ti=mode?focusedTrack:0;*item=-1;*take=-1;*f=mode?focusedFx:lastFx;*p=mode?0:lastParam;return mode?focusActive:lastTouched;};
 GetTrackNumSends=+[](MediaTrack*,int){return 0;};
 int err=0;Surface s(&err,liveMode?"source=ableton":"0");assert(err==0);assert(std::string(s.GetConfigString())==(liveMode?"source=ableton":"0"));assert(output.sent.back()==roto::sysex(10,1));
 input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(changes==0);
 input.q.push(roto::sysex(10,2));pump(s);assert(output.sent.back()==roto::sysex(10,3,{2}));
 input.q.push(roto::sysex(10,12));pump(s);assert(output.sent.back()==roto::sysex(10,13));
 input.q.push(roto::sysex(12,1,{0,0,0,0}));pump(s);
 bool motor=false;for(auto b:output.sent)if(b.size()==3 && b[1]==12)motor=true;assert(motor);
 // Stereo meters use separate channel-16 CCs, independent of touch/motor values.
 assert(std::find(output.sent.begin(),output.sent.end(),roto::sysex(12,11,{92,115}))!=output.sent.end());
 auto hasMeter=[&](int cc,int v){return std::find(output.sent.begin(),output.sent.end(),roto::Bytes{191,(unsigned char)cc,(unsigned char)v})!=output.sent.end();};
 peakLeft=1;peakRight=.1;s.SetTrackListChange();output.sent.clear();pump(s);assert(hasMeter(65,115) && hasMeter(66,77));
 // Unused screens are explicitly zeroed; stable readings send no repeated meter traffic.
 assert(hasMeter(67,0) && hasMeter(80,0));output.sent.clear();pump(s);
 for(auto b:output.sent)assert(b.size()!=3 || b[1]<65 || b[1]>80);
 input.q.push({191,52,127});pump(s);peakLeft=.01;output.sent.clear();pump(s);
 assert(hasMeter(65,38));for(auto b:output.sent)assert(b.size()!=3 || b[1]!=12);
 input.q.push({191,52,0});pump(s);peakLeft=peakRight=0;output.sent.clear();pump(s);
 assert(hasMeter(65,0) && hasMeter(66,0));
 // Read-mode automation changes the host fader while D_VOL remains fixed.
 auto motorValue=[&](double position){int v=(int)std::lround(16383*position);return std::find(output.sent.begin(),output.sent.end(),roto::Bytes{191,12,(unsigned char)(v>>7)})!=output.sent.end() && std::find(output.sent.begin(),output.sent.end(),roto::Bytes{191,44,(unsigned char)(v&127)})!=output.sent.end();};
 output.sent.clear();s.SetSurfaceVolume((MediaTrack*)&fakeTrack,.1);pump(s);
 assert(vol==1 && motorValue(roto::position(.1)));
 output.sent.clear();s.SetSurfaceVolume((MediaTrack*)&fakeTrack,.8);pump(s);
 assert(vol==1 && motorValue(roto::position(.8)));
 input.q.push({191,52,127});pump(s);output.sent.clear();
 s.SetSurfaceVolume((MediaTrack*)&fakeTrack,.2);pump(s);
 for(auto b:output.sent)assert(b.size()!=3 || b[1]!=12);
 input.q.push({191,52,0});output.sent.clear();pump(s);assert(motorValue(roto::position(.2)));
 // Pan callbacks and mixer input use the same authoritative host feedback.
 input.q.push(roto::sysex(12,1,{0,1,0,0}));pump(s);output.sent.clear();
 s.SetSurfacePan((MediaTrack*)&fakeTrack,.6);pump(s);assert(pan==0 && motorValue(.8));
 input.q.push(roto::sysex(12,1,{0,0,0,0}));pump(s);
 s.SetTrackListChange();pump(s); // Discard old callback values after track/project changes.
 input.q.push({191,52,127});pump(s);assert(s.GetTouchState((MediaTrack*)&fakeTrack,0));assert(!s.GetTouchState((MediaTrack*)&fakeTrack,1));
 output.sent.clear();vol=.5;pump(s);for(auto b:output.sent)assert(b.size()!=3 || b[1]!=12);
 input.q.push({191,12,64});input.q.push({191,44,0});pump(s);assert(changes==1);
 input.q.push({191,52,0});pump(s);assert(!s.GetTouchState((MediaTrack*)&fakeTrack,0));
 // Repeat the user's touch/release/re-touch sequence, including host refreshes.
 input.q.push({191,52,127});pump(s);s.SetTrackListChange();
 input.q.push(roto::sysex(12,1,{0,0,0,0}));output.sent.clear();vol=.25;pump(s);
 assert(s.GetTouchState((MediaTrack*)&fakeTrack,0));
 for(auto b:output.sent)assert(b.size()!=3 || b[1]!=12);
 clockTime+=3;output.sent.clear();pump(s);
 for(auto b:output.sent)assert(b.size()!=3 || b[1]!=12);
 input.q.push({191,52,0});pump(s);
 // More than one bank; check navigation survives periodic display refresh.
 trackCount=10;s.SetTrackListChange();pump(s);
 input.q.push(roto::sysex(10,6,{0,8}));output.sent.clear();pump(s);
 bool bankEight=false;for(auto b:output.sent)if(b==roto::sysex(10,5,{0,8}))bankEight=true;
 assert(bankEight);
 std::vector<roto::Bytes> batch;
 for(auto b:output.sent)if(b.size()>=8 && b[0]==240 && b[5]==10 && b[6]!=15)batch.push_back(b);
 roto::Bytes ninth;roto::word(ninth,8);roto::text(ninth,"Drums");ninth.push_back(70);ninth.push_back(0);
 roto::Bytes tenth=ninth;tenth[1]=9;
 const std::vector<roto::Bytes> expected={roto::sysex(10,4,{0,10}),roto::sysex(10,5,{0,8}),roto::sysex(10,7,ninth),roto::sysex(10,7,tenth),roto::sysex(10,8)};
 assert(batch==expected);
 output.sent.clear();clockTime+=3;pump(s);
 for(auto b:output.sent)assert(b.size()<7 || b[0]!=240 || b[5]!=10 || b[6]!=5);
 // Color-only refresh must preserve the exact partial-bank sequence.
 trackColor=0x1000000|0xFF0000;output.sent.clear();clockTime+=3;pump(s);
 batch.clear();for(auto b:output.sent)if(b.size()>=8 && b[0]==240 && b[5]==10 && b[6]!=15)batch.push_back(b);
 auto colored=expected;colored[2][22]=71;colored[3][22]=71;assert(batch==colored);
 trackColor=0;output.sent.clear();clockTime+=3;pump(s);
 batch.clear();for(auto b:output.sent)if(b.size()>=8 && b[0]==240 && b[5]==10 && b[6]!=15)batch.push_back(b);assert(batch==expected);
 input.q.push(roto::sysex(10,6,{0,0}));output.sent.clear();pump(s);
 bool bankZero=false;for(auto b:output.sent)if(b==roto::sysex(10,5,{0,0}))bankZero=true;assert(bankZero);
 input.q.push(roto::sysex(12,1,{0,2,0,0}));pump(s);input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(changes==1);
 input.q.push(roto::sysex(11,1));pump(s);input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(changes==1);
 // Plugin mode emits no mixer meter updates or peak reads.
 int readsBefore=peakReads;output.sent.clear();pump(s);assert(peakReads==readsBefore);
 for(auto b:output.sent)assert(b.size()!=3 || b[1]<65 || b[1]>80);
 // Plugin list, learning, recall, input/output and return-to-mixer regression.
 input.q.push(roto::sysex(11,1));output.sent.clear();pump(s);
 auto has=[&](const roto::Bytes& target){return std::find(output.sent.begin(),output.sent.end(),target)!=output.sent.end();};
 const auto id=liveMode?roto::abletonIdentity("UADx Pultec EQP-1A EQ",auMode,vst2Mode):roto::pluginIdentity("reaeq");
 assert(has(roto::sysex(11,2,{2})) && has(roto::pluginDetails(0,id,true,"ReaEQ")) && has(roto::sysex(11,8,{0,0,0})));
 assert(has(roto::selectedTrack(0,"Drums",70)));
 auto labelBeforeSelection=[&](int index,const std::string& name){
   auto track=std::find(output.sent.begin(),output.sent.end(),roto::selectedTrack(index,name,70));
   auto explicitName=std::find(output.sent.begin(),output.sent.end(),roto::currentTrackName(name));
   auto select=std::find(output.sent.begin(),output.sent.end(),roto::sysex(11,8,{0,0,0}));
   assert(track!=output.sent.end() && explicitName!=output.sent.end() && select!=output.sent.end());
   assert(track<explicitName && explicitName<select);
 };
 labelBeforeSelection(0,"Drums");

 // Track-only edits refresh the header without reselecting the plugin.
 firstName="Drums renamed";output.sent.clear();s.SetTrackTitle((MediaTrack*)&fakeTrack,firstName.c_str());pump(s);
 assert(has(roto::selectedTrack(0,"Drums renamed",70)) && has(roto::currentTrackName("Drums renamed")));
 for(auto b:output.sent)assert(b.size()<7 || b[0]!=240 || b[5]!=11 || b[6]!=8);
 output.sent.clear();pump(s);assert(!has(roto::selectedTrack(0,"Drums renamed",70)));
 trackColor=0x1000000|0xFF0000;output.sent.clear();pump(s);assert(has(roto::selectedTrack(0,"Drums renamed",71)));trackColor=0;pump(s);

 input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(fxChanges==0 && changes==1);
 auto mapped=[&](int param,int type,int slot,const std::string& pluginId){roto::Bytes b;roto::word(b,param);auto h=roto::mappingParamHash(pluginId,"param"+std::to_string(param),"Param "+std::to_string(param));b.insert(b.end(),h.begin(),h.end());b.push_back(type);b.push_back(slot);b.push_back(0);return roto::sysex(11,11,b);};
 // Learn requires a fresh host touch/change after entering learn.
 lastTouched=true;lastParam=0;input.q.push(roto::sysex(11,9,{1}));output.sent.clear();pump(s);
 for(auto b:output.sent)assert(b.size()<7 || b[0]!=240 || b[5]!=11 || b[6]!=10);
 fxValues[0][0]=.4;output.sent.clear();pump(s);
 assert(has(roto::learnParam(0,roto::mappingParamHash(id,"param0","Param 0"),6553,"Param 0")));
 input.q.push(roto::sysex(11,17,{0,0}));input.q.push(roto::sysex(11,9,{0}));input.q.push(mapped(0,0,0,id));output.sent.clear();pump(s);
 assert(has(roto::learnParam(0,roto::mappingParamHash(id,"param0","Param 0"),6553,"Param 0")));
 // Initial recall shows value once; idle does not keep resetting the display timeout.
 assert(has(roto::showControlValue(0,0,"Off")));
 output.sent.clear();pump(s);assert(!has(roto::showControlValue(0,0,"Off")));
 // A stationary touch redisplays the value without moving the motor.
 input.q.push({191,52,127});output.sent.clear();pump(s);
 assert(has(roto::showControlValue(0,0,"Off")));
 for(auto b:output.sent)assert(b.size()!=3 || b[1]!=12);
 input.q.push({191,52,0});pump(s);
 // Refresh formatted text even if only units/format change, not normalized value.
 displaySuffix=" Hz";output.sent.clear();pump(s);assert(has(roto::showControlValue(0,0,"Off Hz")));
 displaySuffix.clear();pump(s);
 formatFails=true;output.sent.clear();pump(s);assert(has(roto::showControlValue(0,0,"40.0%")));
 formatFails=false;pump(s);
 input.q.push({191,12,64});input.q.push({191,44,0});pump(s);assert(fxChanges==1 && std::abs(fxValues[0][0]-8192/16383.0)<1e-9);assert(has(roto::showControlValue(0,0,"On")));
 fxValues[0][0]=.75;output.sent.clear();pump(s);int value=(int)std::lround(.75*16383);
 assert(has({191,12,(unsigned char)(value>>7)}) && has({191,44,(unsigned char)(value&127)}));
 assert(!has(roto::showControlValue(0,0,"On"))); // unchanged formatted text is suppressed
 input.q.push({191,52,127});pump(s);fxValues[0][0]=.25;output.sent.clear();pump(s);
 for(auto b:output.sent)assert(b.size()!=3 || b[1]!=12);
 assert(has(roto::showControlValue(0,0,"Off")));
 input.q.push({191,52,0});output.sent.clear();pump(s);value=(int)std::lround(.25*16383);assert(has({191,12,(unsigned char)(value>>7)}));
 // A new mapping page clears stale slots, including missing controls.
 input.q.push(mapped(8,0,1,id));output.sent.clear();pump(s);
 for(auto b:output.sent)assert(b.size()!=3 || b[1]!=12);
 for(const auto& b:output.sent)assert(b.size()!=23 || b[5]!=10 || b[6]!=24 || b[7]!=0 || b[8]!=0);
 input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(fxChanges==1);
 input.q.push({191,13,127});input.q.push({191,45,127});pump(s);assert(fxChanges==2 && fxValues[0][8]==1);
 // Unknown hashes leave the controller's stored mapping intact and ignore input.
 auto unknown=mapped(0,0,0,id);unknown[9]^=1;input.q.push(unknown);pump(s);
 input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(fxChanges==2);
 // Button mappings accept their actual seven-bit value.
 input.q.push(mapped(1,1,0,id));pump(s);output.sent.clear();input.q.push({191,20,127});pump(s);assert(has(roto::showControlValue(1,0,"On")));assert(fxValues[0][1]==1 && fxChanges==3);
 input.q.push({191,20,0});pump(s);assert(fxValues[0][1]==0 && fxChanges==4);
 // Device selection clears old mappings and follows the selected instance.
 input.q.push(roto::sysex(11,7,{1}));pump(s);assert(selectedFx==1);
 input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(fxChanges==4);
 input.q.push(mapped(0,0,0,liveMode?roto::abletonIdentity("Other FX",auMode,vst2Mode):roto::pluginIdentity("other-fx")));pump(s);
 input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(fxValues[1][0]==1 && fxChanges==5);
 // A replaced/deleted instance invalidates cached control bindings.
 fxGuids[1].Data1=3;input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(fxChanges==5);
 fxCount=0;output.sent.clear();pump(s);assert(has(roto::sysex(11,2,{0})));
 output.sent.clear();pump(s);for(auto b:output.sent)assert(b.size()<7 || b[0]!=240 || b[5]!=11);
 // Empty/deleted track does not dereference an invalid pointer.
 targetValid=false;selected=nullptr;output.sent.clear();pump(s);assert(has(roto::selectedTrack(0,"",70)));targetValid=true;selected=(MediaTrack*)&fakeTrack;fxCount=2;
 input.q.push(roto::sysex(11,1));pump(s);
 // A toggle's labels are obtained without changing its value.
 toggleParam=true;input.q.push(mapped(2,1,0,id));output.sent.clear();pump(s);
 assert(has(roto::learnParam(2,roto::mappingParamHash(id,"param2","Param 2"),(int)std::lround(fxValues[0][2]*16383),"Param 2",{"Off","On"})));assert(fxChanges==5);
 toggleParam=false;
 // Exact reported trigger: focus another track's plugin without selecting its track first.
 auto previousGetTrack=GetTrack;
 GetTrack=+[](ReaProject*,int n)->MediaTrack*{return n==0?(MediaTrack*)&fakeTrack:n==1?(MediaTrack*)&secondTrack:nullptr;};
 selected=(MediaTrack*)&fakeTrack;pump(s);focusActive=true;focusedTrack=1;focusedFx=0;
 output.sent.clear();pump(s);assert(selected==(MediaTrack*)&secondTrack);labelBeforeSelection(1,"Bass");
 // Both label and target are ready before plugin selection reaches the device.
 focusActive=false;selected=(MediaTrack*)&fakeTrack;output.sent.clear();pump(s);labelBeforeSelection(0,firstName);
 GetTrack=previousGetTrack;
 // Recall uses the same plugin/parameter identity on another track instance.
 selected=(MediaTrack*)&secondTrack;output.sent.clear();pump(s);assert(has(roto::selectedTrack(1,"Bass",70)));input.q.push(mapped(0,0,0,id));pump(s);
 input.q.push({191,12,64});input.q.push({191,44,0});pump(s);assert(lastFxWrite==(MediaTrack*)&secondTrack && fxChanges==6);
 // Lock keeps the current instance despite host track selection changes.
 input.q.push(roto::sysex(11,13,{1}));pump(s);selected=(MediaTrack*)&fakeTrack;output.sent.clear();pump(s);
 assert(!has(roto::selectedTrack(0,firstName,70)));
 secondName="Bass renamed";output.sent.clear();pump(s);assert(has(roto::selectedTrack(1,"Bass renamed",70)));
 input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(lastFxWrite==(MediaTrack*)&secondTrack && fxChanges==7);
 input.q.push(roto::sysex(11,13,{0}));output.sent.clear();pump(s);assert(has(roto::selectedTrack(0,firstName,70)));
 input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(fxChanges==7);
 input.q.push(mapped(0,0,0,id));pump(s);
 input.q.push({191,12,64});input.q.push({191,44,0});pump(s);assert(lastFxWrite==(MediaTrack*)&fakeTrack && fxChanges==8);
 // Rename while a plugin-selection batch is still queued uses only the latest label.
 selected=(MediaTrack*)&secondTrack;output.sent.clear();clockTime+=.034;s.Run();
 secondName="Bass latest";pump(s);
 auto latestName=std::find(output.sent.begin(),output.sent.end(),roto::currentTrackName(secondName));
 auto pendingSelection=std::find(output.sent.begin(),output.sent.end(),roto::sysex(11,8,{0,0,0}));
 assert(latestName!=output.sent.end() && pendingSelection!=output.sent.end() && latestName<pendingSelection);
 selected=(MediaTrack*)&fakeTrack;pump(s);
 // A fresh connection republishes the label even if host selection is unchanged.
 input.q.push(roto::sysex(10,2));pump(s);input.q.push(roto::sysex(10,12));output.sent.clear();pump(s);
 assert(has(roto::selectedTrack(0,firstName,70)));
 // A saved mapping can be recalled again after leaving and reentering PLUGIN.
 input.q.push(roto::sysex(12,1,{0,0,0,0}));pump(s);
 input.q.push(roto::sysex(11,1));pump(s);input.q.push(mapped(0,0,0,id));pump(s);
 input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(fxChanges==9);
 input.q.push(roto::sysex(12,1,{0,0,0,0}));output.sent.clear();pump(s);assert(has(roto::sysex(10,5,{0,0})));
 input.q.push({191,12,64});input.q.push({191,44,0});pump(s);assert(changes==2 && fxChanges==9);
 // Stereo routing follows actual track pointers after a bank change, and clears gaps.
 trackCount=10;GetTrack=+[](ReaProject*,int n)->MediaTrack*{return n==0?(MediaTrack*)&fakeTrack:n==8?(MediaTrack*)&secondTrack:nullptr;};
 Track_GetPeakInfo=+[](MediaTrack* t,int ch){return t==(MediaTrack*)&secondTrack?(ch?.5:.25):(ch?.1:1.0);};
 s.SetTrackListChange();input.q.push(roto::sysex(10,6,{0,8}));output.sent.clear();pump(s);
 assert(hasMeter(65,roto::meterLevel(.25)) && hasMeter(66,roto::meterLevel(.5)) && hasMeter(67,0) && hasMeter(80,0));
 input.q.push(roto::sysex(10,6,{0,0}));output.sent.clear();pump(s);
 assert(hasMeter(65,115) && hasMeter(66,77));
 // Limit changing meter traffic and retain a transient between update windows.
 clockTime+=.06;output.sent.clear();s.Run();output.sent.clear();
 Track_GetPeakInfo=+[](MediaTrack*,int ch){return ch?0.0:2.0;};clockTime+=.01;s.Run();
 for(auto b:output.sent)assert(b.size()!=3 || b[1]<65 || b[1]>80);
 Track_GetPeakInfo=+[](MediaTrack*,int){return 0.0;};clockTime+=.05;s.Run();assert(hasMeter(65,127));
 clockTime+=.06;output.sent.clear();s.Run();assert(hasMeter(65,0));
 // Master view maps only screen 1; all remaining screens are cleared.
 Track_GetPeakInfo=+[](MediaTrack*,int){return 1.0;};
 input.q.push(roto::sysex(12,1,{1,0,0,0}));output.sent.clear();pump(s);
 assert(hasMeter(65,115) && hasMeter(66,115) && hasMeter(67,0) && hasMeter(80,0));
 // Returning to the measured indent produces exact unity via ordinary 14-bit CC.
 input.q.push({191,12,(unsigned char)(13004>>7)});input.q.push({191,44,(unsigned char)(13004&127)});pump(s);assert(vol==1);
 // Touch chatter alone must not repeatedly transmit an unchanged motor target.
 output.sent.clear();input.q.push({191,52,127});pump(s);input.q.push({191,52,0});pump(s);
 for(auto b:output.sent)assert(b.size()!=3 || (b[1]!=12 && b[1]!=44));
 // CC 64 is a zero-based knob index, including value 0; restore exact unity.
 int beforeUnity=changes;vol=.4;input.q.push({191,64,0});output.sent.clear();pump(s);
 assert(changes==beforeUnity+1 && vol==1 && motorValue(roto::position(1)));
 input.q.push({191,64,8});input.q.push({191,64,127});input.q.push({191,64,1});pump(s);assert(changes==beforeUnity+1);
 // All-track banking addresses the indexed track rather than track 1.
 input.q.push(roto::sysex(12,1,{0,0,0,0}));input.q.push(roto::sysex(10,6,{0,8}));pump(s);
 MediaTrack* unityTrack=nullptr;
 CSurf_OnVolumeChange=+[](MediaTrack* t,double v,bool){lastFxWrite=t;++changes;return vol=v;};
 input.q.push({191,64,0});pump(s);unityTrack=lastFxWrite;
 assert(unityTrack==(MediaTrack*)&secondTrack && vol==1 && changes==beforeUnity+2);
 input.q.push(roto::sysex(12,1,{0,1,0,0}));input.q.push({191,64,0});pump(s);assert(changes==beforeUnity+2);
 input.q.push(roto::sysex(12,1,{0,2,0,0}));input.q.push({191,64,0});pump(s);assert(changes==beforeUnity+2);
 input.q.push(roto::sysex(11,1));input.q.push({191,64,0});pump(s);assert(changes==beforeUnity+2);
 // Disconnect releases host touch and old bindings; reopen changed port indices.
 input.q.push(roto::sysex(10,6,{0,0}));
 input.q.push(roto::sysex(12,1,{0,0,0,0}));pump(s);
 input.q.push({191,52,127});pump(s);assert(s.GetTouchState((MediaTrack*)&fakeTrack,0));
 int created=inputCreates,destroyed=inputDestroys,oldChanges=changes;
 portCount=0;output.sent.clear();pump(s);
 assert(inputDestroys==destroyed+1 && !s.GetTouchState((MediaTrack*)&fakeTrack,0));
 assert(output.sent.empty());pump(s);assert(inputCreates==created);
 portCount=2;portIndex=1;output.sent.clear();pump(s);
 assert(inputCreates==created+1 && has(roto::sysex(10,1)));
 input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(changes==oldChanges);
 input.q.push(roto::sysex(10,2));input.q.push(roto::sysex(10,12));input.q.push(roto::sysex(12,1,{0,0,0,0}));output.sent.clear();pump(s);
 assert(has(roto::sysex(10,4,{0,10})) && has(roto::sysex(10,8)) && hasMeter(65,115));
 input.q.push({191,12,64});input.q.push({191,44,0});pump(s);assert(changes==oldChanges+1);
 // A valid idle version reply leaves ports untouched. No MIDI activity needed.
 created=inputCreates;for(int i=0;i<30;++i)pump(s);assert(inputCreates==created);
 // Same-name/index stale handles: unanswered health query triggers reopening.
 autoReply=false;clockTime+=2.1;s.Run();clockTime+=5.1;s.Run();
 assert(inputDestroys==destroyed+2 && !s.GetTouchState((MediaTrack*)&fakeTrack,0));
 clockTime+=1.1;s.Run();assert(inputCreates==created+1);autoReply=true;
 input.q.push(roto::sysex(10,2));input.q.push(roto::sysex(10,12));input.q.push(roto::sysex(11,1));output.sent.clear();pump(s);
 assert(has(roto::pluginDetails(0,id,true,"ReaEQ")) && has(roto::selectedTrack(0,firstName,70)));
 int oldFxChanges=fxChanges;input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(fxChanges==oldFxChanges);
 input.q.push(mapped(0,0,0,id));pump(s);input.q.push({191,12,64});input.q.push({191,44,0});pump(s);assert(fxChanges==oldFxChanges+1);
 // Ambiguous device names never select an arbitrary controller.
 portIndex=0;created=inputCreates;pump(s);assert(inputCreates==created);
 portIndex=1;pump(s);assert(inputCreates==created+1);
 input.q.push(roto::sysex(10,2));input.q.push(roto::sysex(10,12));pump(s);
 // Partial open failures destroy the successful handle and retry later.
 portCount=0;pump(s);openOutput=false;portCount=1;portIndex=0;
 destroyed=inputDestroys;pump(s);assert(inputDestroys==destroyed+1);
 openOutput=true;clockTime+=2.1;pump(s);
 input.q.push(roto::sysex(10,2));input.q.push(roto::sysex(10,12));pump(s);
 // Shared send destinations must resolve by target, never by source-local slot.
 trackCount=10;GetTrack=+[](ReaProject*,int i)->MediaTrack*{return i>=0 && i<trackCount?(MediaTrack*)&sendTracks[i]:nullptr;};
 GetMasterTrack=+[](ReaProject*)->MediaTrack*{return (MediaTrack*)&sendTracks[9];};
 ValidatePtr2=+[](ReaProject*,void* p,const char*){return sourceIndex((MediaTrack*)p)>=0 && sourceIndex((MediaTrack*)p)<trackCount;};
 GetTrackName=+[](MediaTrack* t,char* b,int size){int i=sourceIndex(t);std::snprintf(b,size,"%s",i==2?"Reverb":i==3?"Delay":"Source");return true;};
 auto* reverb=(MediaTrack*)&sendTracks[2];auto* delay=(MediaTrack*)&sendTracks[3];
 routes[0]={{delay,.2},{reverb,.4}};routes[1]={{reverb,.6}};routes[8]={{reverb,.8}};
 GetTrackNumSends=+[](MediaTrack* t,int category){int i=sourceIndex(t);return i<0?0:category==1?2:(int)routes[i].size();};
 GetSetTrackSendInfo=+[](MediaTrack* t,int category,int index,const char* key,void*)->void*{
   int i=sourceIndex(t);return category==0 && i>=0 && index>=0 && index<(int)routes[i].size() && std::strcmp(key,"P_DESTTRACK")==0?routes[i][index].destination:nullptr;
 };
 GetTrackSendUIVolPan=+[](MediaTrack* t,int index,double* value,double* pan){int i=sourceIndex(t);index-=2;
   if(i<0 || index<0 || index>=(int)routes[i].size())return false;*value=routes[i][index].value;*pan=0;return true;
 };
 SetTrackSendUIVol=+[](MediaTrack* t,int index,double value,int end){int i=sourceIndex(t);lastSendIndex=index;index-=2;
   if(i<0 || index<0 || index>=(int)routes[i].size())return false;
   ++sendWrites;sendEnds+=end==1;sendInstants+=end==-1;lastSendTrack=t;routes[i][index].value=value;return true;
 };
 input.q.push(roto::sysex(10,6,{0,0}));input.q.push(roto::sysex(12,1,{0,2,0,0}));s.SetTrackListChange();output.sent.clear();pump(s);
 assert(std::find(output.sent.begin(),output.sent.end(),roto::sysex(12,3,{2}))!=output.sent.end());
 roto::Bytes sendLabels{0};roto::text(sendLabels,"Reverb");roto::text(sendLabels,"Delay");for(int i=2;i<8;++i)roto::text(sendLabels,"");
 assert(std::find(output.sent.begin(),output.sent.end(),roto::sysex(12,8,sendLabels))!=output.sent.end());
 assert(motorValue(roto::position(.4)));
 input.q.push({191,12,127});input.q.push({191,44,127});pump(s);
 assert(lastSendTrack==(MediaTrack*)&sendTracks[0] && lastSendIndex==3 && routes[0][1].value==roto::volume(1) && routes[0][0].value==.2);
 input.q.push({191,13,0});input.q.push({191,45,0});pump(s);assert(routes[1][0].value==0 && lastSendIndex==2);
 // No route: no send creation and no accidental track-volume edit.
 int priorWrites=sendWrites,priorChanges=changes;
 input.q.push({191,14,100});input.q.push({191,46,0});pump(s);assert(sendWrites==priorWrites && changes==priorChanges);
 input.q.push({191,64,0});pump(s);assert(sendWrites==priorWrites);
 input.q.push({191,12,101});input.q.push({191,44,76});pump(s);assert(routes[0][1].value==1 && sendInstants>0);
 // REAPER changes feed back; held knobs receive no motor target. Release ends edit.
 routes[0][1].value=.1;output.sent.clear();pump(s);assert(motorValue(roto::position(.1)));
 input.q.push({191,52,127});pump(s);assert(!s.GetTouchState((MediaTrack*)&sendTracks[0],0));
 input.q.push({191,12,64});input.q.push({191,44,0});pump(s);int ends=sendEnds;
 routes[0][1].value=.7;output.sent.clear();pump(s);for(auto b:output.sent)assert(b.size()!=3 || (b[1]!=12 && b[1]!=44));
 input.q.push({191,52,0});pump(s);assert(sendEnds==ends+1);
 // Hardware outputs are skipped and local send reordering preserves destination.
 std::swap(routes[0][0],routes[0][1]);input.q.push({191,12,101});input.q.push({191,44,76});pump(s);assert(routes[0][0].destination==reverb && routes[0][0].value==1 && lastSendIndex==2);
 // Changing send choice addresses Delay, while buttons remain track controls.
 input.q.push(roto::sysex(12,1,{0,2,0,1}));pump(s);input.q.push({191,12,101});input.q.push({191,44,76});pump(s);assert(routes[0][1].destination==delay && routes[0][1].value==1);
 CSurf_OnMuteChange=+[](MediaTrack* t,int){lastSendTrack=t;return true;};CSurf_SetSurfaceMute=+[](MediaTrack*,bool,IReaperControlSurface*){};
 input.q.push({191,20,127});pump(s);assert(lastSendTrack==(MediaTrack*)&sendTracks[0]);
 // Request names beyond the end: eight zero-padded strings, correct block start.
 input.q.push(roto::sysex(12,7,{9}));output.sent.clear();pump(s);roto::Bytes blankNames{8};for(int i=0;i<8;++i)roto::text(blankNames,"");
 assert(std::find(output.sent.begin(),output.sent.end(),roto::sysex(12,8,blankNames))!=output.sent.end());
 // Bank 9-16 resolves track 9's send independently.
 input.q.push(roto::sysex(12,1,{0,2,0,0}));input.q.push(roto::sysex(10,6,{0,8}));pump(s);input.q.push({191,12,101});input.q.push({191,44,76});pump(s);
 assert(lastSendTrack==(MediaTrack*)&sendTracks[8] && routes[8][0].value==1);
 // A second names page includes destinations 9/10, then six empty names.
 routes[0].clear();for(int i=1;i<10;++i)routes[0].push_back({(MediaTrack*)&sendTracks[i],.2});routes[1]={{(MediaTrack*)&sendTracks[0],.3}};
 s.SetTrackListChange();pump(s);input.q.push(roto::sysex(12,7,{9}));output.sent.clear();pump(s);
 roto::Bytes pageNames{8};roto::text(pageNames,"Source");roto::text(pageNames,"Source");for(int i=2;i<8;++i)roto::text(pageNames,"");
 assert(std::find(output.sent.begin(),output.sent.end(),roto::sysex(12,8,pageNames))!=output.sent.end());
 // Removal clears send choices and does not create or write another route.
 for(auto& list:routes)list.clear();s.SetTrackListChange();output.sent.clear();pump(s);priorWrites=sendWrites;
 assert(std::find(output.sent.begin(),output.sent.end(),roto::sysex(12,3,{0}))!=output.sent.end());
 input.q.push({191,64,0});input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(sendWrites==priorWrites);
 // Returning to Level restores volume controls after the Send mode.
 input.q.push(roto::sysex(12,1,{0,0,0,0}));pump(s);input.q.push({191,12,100});input.q.push({191,44,0});pump(s);assert(changes==priorChanges+1);
 // Single-track MIX uses Level/Pan then sends, never adjacent track controls.
 GetMediaTrackInfo_Value=+[](MediaTrack* t,const char* key){int i=sourceIndex(t);if(i<0)return 0.0;
   if(std::strcmp(key,"IP_TRACKNUMBER")==0)return double(i+1);
   if(std::strcmp(key,"D_VOL")==0)return trackVolumes[i];if(std::strcmp(key,"D_PAN")==0)return trackPans[i];return 0.0;
 };
 CSurf_OnVolumeChange=+[](MediaTrack* t,double v,bool){lastFxWrite=t;++changes;return trackVolumes[sourceIndex(t)]=v;};
 CSurf_OnPanChange=+[](MediaTrack* t,double v,bool){lastFxWrite=t;++changes;return trackPans[sourceIndex(t)]=v;};
 selected=(MediaTrack*)&sendTracks[0];routes[0]={{delay,.2},{reverb,.4}};routes[1]={{reverb,.6}};
 input.q.push(roto::sysex(12,2,{0}));s.SetTrackListChange();output.sent.clear();pump(s);
 for(const auto& b:output.sent)assert(b.size()<8 || b[0]!=240 || !((b[5]==10 && b[6]==7) || (b[5]==10 && b[6]==0x16)));
 auto slotValue=[&](int slot,double position){int v=(int)std::lround(16383*position);
   return has({191,(unsigned char)(12+slot),(unsigned char)(v>>7)}) && has({191,(unsigned char)(44+slot),(unsigned char)(v&127)});
 };
 assert(has(roto::selectedTrack(0,"Source",70)) && slotValue(0,roto::position(1)) && slotValue(1,.5));
 // Bitwig publishes send metadata before selected-track metadata.
 auto selectPacket=std::find(output.sent.begin(),output.sent.end(),roto::selectedTrack(0,"Source",70));
 auto singleSendLabels=std::find_if(output.sent.begin(),output.sent.end(),[](const roto::Bytes& b){return b.size()>8 && b[0]==240 && b[5]==12 && b[6]==8;});
 assert(singleSendLabels!=output.sent.end() && singleSendLabels<selectPacket);
 assert(slotValue(2,roto::position(.4)) && slotValue(3,roto::position(.2)) && slotValue(7,0));
 assert(hasMeter(65,115) && hasMeter(67,0));
 oldChanges=changes;priorWrites=sendWrites;
 input.q.push({191,12,64});input.q.push({191,44,0});pump(s);assert(changes==oldChanges+1 && lastFxWrite==selected && trackVolumes[1]==.3);
 input.q.push({191,13,127});input.q.push({191,45,127});pump(s);assert(changes==oldChanges+2 && trackPans[0]==1 && trackPans[1]==-.4);
 input.q.push({191,14,101});input.q.push({191,46,76});pump(s);assert(sendWrites==priorWrites+1 && lastSendTrack==selected && routes[0][1].value==1);
 priorWrites=sendWrites;input.q.push({191,19,127});input.q.push({191,51,127});pump(s);assert(sendWrites==priorWrites);
 // Unity applies only to the Level control, not Pan or a Send.
 input.q.push({191,64,0});pump(s);assert(trackVolumes[0]==1 && changes==oldChanges+3);
 input.q.push({191,64,1});input.q.push({191,64,2});pump(s);assert(changes==oldChanges+3 && sendWrites==priorWrites);
 // Ordinary buttons are inactive in the official single-track layout.
 lastSendTrack=nullptr;input.q.push({191,20,127});pump(s);assert(lastSendTrack==nullptr);
 // Separate Level/Pan touch states and host automation feedback.
 input.q.push({191,52,127});input.q.push({191,53,127});pump(s);
 assert(s.GetTouchState(selected,0) && s.GetTouchState(selected,1));
 s.SetSurfaceVolume(selected,.1);s.SetSurfacePan(selected,-.6);output.sent.clear();pump(s);
 for(auto b:output.sent)assert(b.size()!=3 || (b[1]!=12 && b[1]!=44 && b[1]!=13 && b[1]!=45));
 // Selection during a hold defers retargeting until release.
 selected=(MediaTrack*)&sendTracks[1];input.q.push({191,12,64});input.q.push({191,44,0});pump(s);assert(lastFxWrite==(MediaTrack*)&sendTracks[0]);
 input.q.push({191,52,0});input.q.push({191,53,0});output.sent.clear();pump(s);
 assert(has(roto::selectedTrack(1,"Source",70)) && slotValue(0,roto::position(.3)) && slotValue(1,.3));
 assert(!s.GetTouchState((MediaTrack*)&sendTracks[0],0));
 // All-tracks name packets must never overwrite single-track control labels,
 // including periodic refreshes, selection changes and host title/list callbacks.
 auto noTrackNames=[&](){for(const auto& b:output.sent)assert(b.size()<8 || b[0]!=240 || !((b[5]==10 && b[6]==7) || (b[5]==10 && b[6]==0x16)));};
 noTrackNames();
 output.sent.clear();s.SetTrackTitle(selected,"Source renamed");s.SetTrackListChange();clockTime+=2.1;pump(s);noTrackNames();
 selected=(MediaTrack*)&sendTracks[3];output.sent.clear();pump(s);noTrackNames();assert(has(roto::selectedTrack(3,"Delay",70)));
 selected=(MediaTrack*)&sendTracks[1];output.sent.clear();pump(s);noTrackNames();
 // No route on selected track: do not write/create one or edit its volume.
 priorWrites=sendWrites;oldChanges=changes;input.q.push({191,15,127});input.q.push({191,47,127});pump(s);assert(sendWrites==priorWrites && changes==oldChanges);
 input.q.push({191,14,101});input.q.push({191,46,76});pump(s);assert(lastSendTrack==selected && routes[1][0].value==1);
 s.SetSurfaceVolume(selected,.2);s.SetSurfacePan(selected,.4);output.sent.clear();pump(s);assert(slotValue(0,roto::position(.2)) && slotValue(1,.7));
 // Page 2 begins at send index 6; page 3 begins at index 14 (API v1.6).
 selected=(MediaTrack*)&sendTracks[0];routes[0].clear();for(int i=1;i<10;++i)routes[0].push_back({(MediaTrack*)&sendTracks[i],.2});routes[1]={{(MediaTrack*)&sendTracks[0],.3}};
 s.SetTrackListChange();input.q.push(roto::sysex(12,2,{8}));output.sent.clear();pump(s);
 roto::Bytes singleNames{6};for(int i=0;i<4;++i)roto::text(singleNames,"Source");for(int i=4;i<8;++i)roto::text(singleNames,"");
 assert(has(roto::sysex(12,8,singleNames)) && slotValue(0,roto::position(.2)) && slotValue(7,0));
 oldChanges=changes;input.q.push({191,12,101});input.q.push({191,44,76});pump(s);
 assert(lastSendTrack==selected && lastSendIndex==7 && routes[0][5].destination==(MediaTrack*)&sendTracks[6] && routes[0][5].value==1 && changes==oldChanges);
 input.q.push({191,64,0});pump(s);assert(changes==oldChanges);
 input.q.push(roto::sysex(12,2,{16}));output.sent.clear();pump(s);
 roto::Bytes emptySingleNames{14};for(int i=0;i<8;++i)roto::text(emptySingleNames,"");assert(has(roto::sysex(12,8,emptySingleNames)) && slotValue(0,0));
 priorWrites=sendWrites;input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(sendWrites==priorWrites && changes==oldChanges);
 // Page payload must be a multiple of eight; malformed/short packets do nothing.
 input.q.push(roto::sysex(12,2,{1}));input.q.push(roto::sysex(12,2,{128}));input.q.push(roto::sysex(12,2));pump(s);
 input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(sendWrites==priorWrites && changes==oldChanges);
 // Deleted selected tracks clear the label/controls without stale pointer writes.
 trackCount=0;output.sent.clear();s.SetTrackListChange();pump(s);assert(has(roto::selectedTrack(0,"",70)) && slotValue(0,0));
 input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(sendWrites==priorWrites && changes==oldChanges);
 trackCount=10;selected=(MediaTrack*)&sendTracks[0];input.q.push(roto::sysex(12,2,{0}));pump(s);
 // Return to all-tracks restores its knob assignment and requested bank.
 input.q.push(roto::sysex(10,6,{0,8}));
 input.q.push(roto::sysex(12,1,{0,0,0,0}));pump(s);input.q.push({191,12,64});input.q.push({191,44,0});pump(s);
 assert(lastFxWrite==(MediaTrack*)&sendTracks[8] && changes==oldChanges+1);
 // Disconnect while editing a send ends its gesture exactly once.
 for(auto& list:routes)list.clear();
 routes[8]={{reverb,.5}};s.SetTrackListChange();
 input.q.push(roto::sysex(12,1,{0,2,0,0}));pump(s);
 input.q.push({191,52,127});input.q.push({191,12,64});input.q.push({191,44,0});pump(s);
 ends=sendEnds;portCount=0;pump(s);assert(sendEnds==ends+1);pump(s);assert(sendEnds==ends+1);
 // REAPER can retain/create the surface with hardware absent at startup.
 {
   Surface waiting(&err);assert(err==0);created=inputCreates;pump(waiting);assert(inputCreates==created);
   portCount=1;output.sent.clear();pump(waiting);assert(inputCreates==created+1 && has(roto::sysex(10,1)));
   // An opened but silent endpoint is periodically reopened, not abandoned.
   clockTime+=8.1;waiting.Run();clockTime+=1.1;waiting.Run();assert(inputCreates==created+2);
   input.q.push(roto::sysex(10,2));input.q.push(roto::sysex(10,12));input.q.push(roto::sysex(12,1,{0,0,0,0}));
   output.sent.clear();pump(waiting);assert(has(roto::sysex(10,8)));
   input.q.push(roto::sysex(12,2,{0}));output.sent.clear();pump(waiting);assert(has(roto::selectedTrack(0,"Source",70)));
   portCount=0;pump(waiting);portCount=1;pump(waiting);
   input.q.push(roto::sysex(10,2));input.q.push(roto::sysex(10,12));input.q.push(roto::sysex(12,2,{0}));
   output.sent.clear();pump(waiting);assert(has(roto::selectedTrack(0,"Source",70)));
 }
 std::cout<<"Surface mock: single-track Level/Pan/sends, pages, selection/hold, missing routes, deletion and return; reconnect, idle health, stale handles, ambiguous ports, partial failures, mapping recall; send destination names, routing, hardware offsets, banking, motor/touch, removal and mixer return; unity indent and touch chatter, MIX unity event, stereo meters, mixer automation callbacks, mixer regressions, plugin learn/recall, page isolation, motor/touch, instance selection/lock, removal, buttons and mixer return passed\n";
}
