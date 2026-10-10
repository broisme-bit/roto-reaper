#undef NDEBUG
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
struct In:midi_Input {Events q;void start()override{}void stop()override{}void SwapBufs(unsigned int)override{}MIDI_eventlist* GetReadBuf()override{return &q;}void Destroy()override{}} input;
struct Out:midi_Output {std::vector<roto::Bytes> sent;void SendMsg(MIDI_event_t* e,int)override{sent.emplace_back(e->midi_message,e->midi_message+e->size);}void Send(unsigned char,unsigned char,unsigned char,int)override{}void Destroy()override{}} output;
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
int main(){
 assert(reg.ShowConfig != nullptr);
 assert(reg.ShowConfig(reg.type_string,nullptr,"") == nullptr);
 time_precise=+[](){return clockTime;};GetResourcePath=+[]()->const char*{return "/tmp";};
 GetNumMIDIInputs=GetNumMIDIOutputs=+[](){return 1;};
 GetMIDIInputName=GetMIDIOutputName=+[](int,char* b,int){std::strcpy(b,"Roto-Control");return true;};
 CreateMIDIInput=+[](int)->midi_Input*{return &input;};CreateMIDIOutput=+[](int,bool,int*)->midi_Output*{return &output;};
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
 TrackFX_GetNamedConfigParm=+[](MediaTrack*,int index,const char*,char* b,int){std::strcpy(b,index?"other-fx":"reaeq");return true;};
 TrackFX_GetEnabled=+[](MediaTrack*,int){return true;};TrackFX_SetEnabled=+[](MediaTrack*,int,bool){};
 TrackFX_GetNumParams=+[](MediaTrack*,int){return 10;};
 TrackFX_GetParamName=+[](MediaTrack*,int,int p,char* b,int size){std::snprintf(b,size,"Param %d",p);return true;};
 TrackFX_GetParamIdent=+[](MediaTrack*,int,int p,char* b,int size){std::snprintf(b,size,"param%d",p);return true;};
 TrackFX_GetParamNormalized=+[](MediaTrack*,int f,int p){return fxValues[f][p];};
 TrackFX_SetParamNormalized=+[](MediaTrack* t,int f,int p,double v){lastFxWrite=t;++fxChanges;fxValues[f][p]=v;return true;};
 TrackFX_EndParamEdit=+[](MediaTrack*,int,int){++editEnds;return true;};
 TrackFX_GetParameterStepSizes=+[](MediaTrack*,int,int,double* step,double*,double*,bool* toggle){*step=0;*toggle=toggleParam;return toggleParam;};
 TrackFX_FormatParamValueNormalized=+[](MediaTrack*,int,int,double v,char* b,int){std::strcpy(b,v>.5?"On":"Off");return true;};
 TrackFX_Show=+[](MediaTrack*,int f,int){selectedFx=f;};
 SetOnlyTrackSelected=+[](MediaTrack* t){selected=t;};
 GetTouchedOrFocusedFX=+[](int mode,int* ti,int* item,int* take,int* f,int* p){*ti=mode?focusedTrack:0;*item=-1;*take=-1;*f=mode?focusedFx:lastFx;*p=mode?0:lastParam;return mode?focusActive:lastTouched;};
 int err=0;Surface s(&err);assert(err==0);assert(output.sent.back()==roto::sysex(10,1));
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
 for(auto b:output.sent)if(b.size()>=8 && b[0]==240 && b[5]==10)batch.push_back(b);
 roto::Bytes ninth;roto::word(ninth,8);roto::text(ninth,"Drums");ninth.push_back(70);ninth.push_back(0);
 roto::Bytes tenth=ninth;tenth[1]=9;
 const std::vector<roto::Bytes> expected={roto::sysex(10,4,{0,10}),roto::sysex(10,5,{0,8}),roto::sysex(10,7,ninth),roto::sysex(10,7,tenth),roto::sysex(10,8)};
 assert(batch==expected);
 output.sent.clear();clockTime+=3;pump(s);
 for(auto b:output.sent)assert(b.size()<7 || b[0]!=240 || b[5]!=10 || b[6]!=5);
 // Color-only refresh must preserve the exact partial-bank sequence.
 trackColor=0x1000000|0xFF0000;output.sent.clear();clockTime+=3;pump(s);
 batch.clear();for(auto b:output.sent)if(b.size()>=8 && b[0]==240 && b[5]==10)batch.push_back(b);
 auto colored=expected;colored[2][22]=71;colored[3][22]=71;assert(batch==colored);
 trackColor=0;output.sent.clear();clockTime+=3;pump(s);
 batch.clear();for(auto b:output.sent)if(b.size()>=8 && b[0]==240 && b[5]==10)batch.push_back(b);assert(batch==expected);
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
 const auto id=roto::pluginIdentity("reaeq");
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
 auto mapped=[&](int param,int type,int slot,const std::string& pluginId){roto::Bytes b;roto::word(b,param);auto h=roto::paramHash(pluginId,"param"+std::to_string(param));b.insert(b.end(),h.begin(),h.end());b.push_back(type);b.push_back(slot);b.push_back(0);return roto::sysex(11,11,b);};
 // Learn requires a fresh host touch/change after entering learn.
 lastTouched=true;lastParam=0;input.q.push(roto::sysex(11,9,{1}));output.sent.clear();pump(s);
 for(auto b:output.sent)assert(b.size()<7 || b[0]!=240 || b[5]!=11 || b[6]!=10);
 fxValues[0][0]=.4;output.sent.clear();pump(s);
 assert(has(roto::learnParam(0,roto::paramHash(id,"param0"),6553,"Param 0")));
 input.q.push(roto::sysex(11,17,{0,0}));input.q.push(roto::sysex(11,9,{0}));input.q.push(mapped(0,0,0,id));output.sent.clear();pump(s);
 assert(has(roto::learnParam(0,roto::paramHash(id,"param0"),6553,"Param 0")));
 input.q.push({191,12,64});input.q.push({191,44,0});pump(s);assert(fxChanges==1 && std::abs(fxValues[0][0]-8192/16383.0)<1e-9);
 fxValues[0][0]=.75;output.sent.clear();pump(s);int value=(int)std::lround(.75*16383);
 assert(has({191,12,(unsigned char)(value>>7)}) && has({191,44,(unsigned char)(value&127)}));
 input.q.push({191,52,127});pump(s);fxValues[0][0]=.25;output.sent.clear();pump(s);
 for(auto b:output.sent)assert(b.size()!=3 || b[1]!=12);
 input.q.push({191,52,0});output.sent.clear();pump(s);value=(int)std::lround(.25*16383);assert(has({191,12,(unsigned char)(value>>7)}));
 // A new mapping page clears stale slots, including missing controls.
 input.q.push(mapped(8,0,1,id));output.sent.clear();pump(s);
 for(auto b:output.sent)assert(b.size()!=3 || b[1]!=12);
 input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(fxChanges==1);
 input.q.push({191,13,127});input.q.push({191,45,127});pump(s);assert(fxChanges==2 && fxValues[0][8]==1);
 // Unknown hashes leave the controller's stored mapping intact and ignore input.
 auto unknown=mapped(0,0,0,id);unknown[9]^=1;input.q.push(unknown);pump(s);
 input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(fxChanges==2);
 // Button mappings accept their actual seven-bit value.
 input.q.push(mapped(1,1,0,id));pump(s);input.q.push({191,20,127});pump(s);assert(fxValues[0][1]==1 && fxChanges==3);
 input.q.push({191,20,0});pump(s);assert(fxValues[0][1]==0 && fxChanges==4);
 // Device selection clears old mappings and follows the selected instance.
 input.q.push(roto::sysex(11,7,{1}));pump(s);assert(selectedFx==1);
 input.q.push({191,12,127});input.q.push({191,44,127});pump(s);assert(fxChanges==4);
 input.q.push(mapped(0,0,0,roto::pluginIdentity("other-fx")));pump(s);
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
 assert(has(roto::learnParam(2,roto::paramHash(id,"param2"),(int)std::lround(fxValues[0][2]*16383),"Param 2",{"Off","On"})));assert(fxChanges==5);
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
 std::cout<<"Surface mock: unity indent and touch chatter, MIX unity event, stereo meters, mixer automation callbacks, mixer regressions, plugin learn/recall, page isolation, motor/touch, instance selection/lock, removal, buttons and mixer return passed\n";
}
