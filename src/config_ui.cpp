#define NOMINMAX
#include "../vendor/sdk/reaper_plugin.h"
#undef min
#undef max
#include "plugin_protocol.h"
#include <cstdio>
#ifndef _WIN32
#include "../vendor/WDL/swell/swell-dlggen.h"
SWELL_DEFINE_DIALOG_RESOURCE_BEGIN2(1001,SWELL_DLG_WS_CHILD,"",268,90)
BEGIN
 LTEXT "VST3 mapping source:", -1, 8, 9, 96, 12
 COMBOBOX 1002, 110, 7, 145, 70, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP
 LTEXT "REAPER uses its own mappings. Ableton reuses Live VST3 mappings.", -1, 8, 32, 248, 22
 LTEXT "Other plugin formats use REAPER mappings.", -1, 8, 58, 248, 22
END
SWELL_DEFINE_DIALOG_RESOURCE_END2(1001)
#undef BEGIN
#undef END
#endif
static INT_PTR CALLBACK configProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
 if(msg==WM_INITDIALOG){
   SendDlgItemMessage(hwnd,1002,CB_ADDSTRING,0,(LPARAM)"REAPER");
   SendDlgItemMessage(hwnd,1002,CB_ADDSTRING,0,(LPARAM)"Ableton");
   SendDlgItemMessage(hwnd,1002,CB_SETCURSEL,roto::parseMappingSource((const char*)lp)==roto::MappingSource::Ableton?1:0,0);
   return 1;
 }
 if(msg==WM_USER+1024 && wp>1 && lp){
   const auto source=SendDlgItemMessage(hwnd,1002,CB_GETCURSEL,0,0)==1?roto::MappingSource::Ableton:roto::MappingSource::Reaper;
   std::snprintf((char*)lp,(size_t)wp,"%s",roto::mappingSourceConfig(source));
 }
 return 0;
}
HWND rotoShowConfig(HWND parent,const char* config,REAPER_PLUGIN_HINSTANCE instance){
 return CreateDialogParam(instance,MAKEINTRESOURCE(1001),parent,configProc,(LPARAM)config);
}
