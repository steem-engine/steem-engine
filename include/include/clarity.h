#ifndef CLARITY_H
#define CLARITY_H

#define LOOP                 for(;;)
#define IsSameStr(pc1,pc2)   (!strcmp(pc1,pc2))
#define StringsMatch(pc1,pc2)   (!strcmp(pc1,pc2))
#define NotSameStr(pc1,pc2)  ((bool)strcmp(pc1,pc2))

#define IsSameStr_I(pc1,pc2)   (!strcmpi(pc1,pc2))
#define StringsMatch_I(pc1,pc2)   (!strcmpi(pc1,pc2))
#define NotSameStr_I(pc1,pc2)  ((bool)strcmpi(pc1,pc2))

#define Exists(pc)           (access(pc,0)==0)
#define NOT                  !
#define IS_TRUE              !=0
#define IS_FALSE             ==0
#define DID_NOT_FAIL         ==0
#define COMMENT(String)

#ifdef WIN32
#define SendWMCommandMessage(Win,ID,Code) SendMessage(Win,WM_COMMAND,MAKEWPARAM(ID,Code),(LPARAM)GetDlgItem(Win,ID))
#define HideWindow(h)        (ShowWindow(h,SW_HIDE))
#define SUCCESS              ERROR_SUCCESS
#define VK_PAGEUP 33
#define VK_PAGEDOWN 34
#define ClassBkColor(Col) ((HBRUSH)((Col)+1))
#define ClassBkColour(Col) ((HBRUSH)((Col)+1))
#endif

#define RGB_TO_BGR(RGB) ((BYTE(RGB) << 16) | (RGB & 0x00ff00) | ((RGB & 0xff0000) >> 16))

#ifdef STRICT
#define WINDOWPROC WNDPROC
#else
#define WINDOWPROC FARPROC
#endif

#endif

