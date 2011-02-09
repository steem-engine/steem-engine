#ifndef MYMISC_H
#define MYMISC_H

#include <stdio.h>
#ifdef WIN32
#include <io.h>
#endif

#ifndef SLASH
#ifdef WIN32
#define SLASH "\\"
#else
#define SLASH "/"
#endif
#endif

#ifdef MYMISC_CPP
bool _no_ints=0;
#else
extern bool _no_ints;
#endif

#define NO_SLASH(path) if (path[0]) \
                         if (path[strlen(path)-1]=='/' \
                   WIN_ONLY( || path[strlen(path)-1]=='\\' )  \
                            ) path[strlen(path)-1]=0;

#define CenterWindow CentreWindow

#define bound(Val,Min,Max) ( ((Val)<(Min))?(Min):( ((Val)>(Max))?(Max):(Val) ) )

#define REMOVE_LAST_SLASH true
#define REMOVE_SLASH true
#define WITH_SLASH 0
#define KEEP_SLASH 0
#define WITHOUT_SLASH true

typedef struct{
  int Left,Top;
  int Width,Height;
  bool Maximized,Minimized;
}WINPOSITIONDATA;

extern unsigned long HexToVal(char *);
//---------------------------------------------------------------------------
extern bool LoadBool(FILE *f);
extern void SaveBool(bool b,FILE *f);
extern int LoadInt(FILE *f);
extern void SaveInt(int i,FILE *f);
extern void LoadChars(char *buf,FILE *f);
extern void SaveChars(char *buf,FILE *f);
extern long GetFileLength(FILE *f);
extern char *GetFileNameFromPath(char *);
extern void RemoveFileNameFromPath(char *fil,bool rem);
extern bool has_extension_list(char *Filename,char *Ext,...);
extern bool has_extension(char *Filename,char *Ext);
extern bool MatchesAnyString(char *StrToCompare,char *Str,...);
extern bool MatchesAnyString_I(char *StrToCompare,char *Str,...);
#if defined(_INC_TIME) || defined(_SYS_TIME_H)
extern DWORD TMToDOSDateTime(struct tm *lpTime);
#endif
extern int log_to_base_2(unsigned long x);

#ifdef EASYSTR_H
extern EasyStr GetUniquePath(EasyStr path,EasyStr name);
#endif

#ifdef WIN32

#define SetPropI(w,s,dw) SetProp(w,s,(HANDLE)(dw))
#define GetPropI(w,s) DWORD(GetProp(w,s))

extern void RemoveProps(HWND Win,char *Prop1,...);
extern void Border3D(HDC dc,int x,int y,int w,int h,
              DWORD col0,DWORD col1,DWORD col2,DWORD col3);
extern void Box3D(HDC dc,int x,int y,int w,int h,bool d);
extern void CentreTextOut(HDC dc,int x,int y,int w,int h,
    char *text,int len);
extern void GetLongPathName(char *src,char *dest,int maxlen);
extern void SetWindowAndChildrensFont(HWND Win,HFONT fnt);
extern void RemoveAllMenuItems(HMENU),DeleteAllMenuItems(HMENU);
extern void CentreWindow(HWND Win,bool Redraw);
#define RegValueExists(Key,Name) (RegQueryValueEx(Key,Name,NULL,NULL,NULL,NULL)==ERROR_SUCCESS)
extern bool RegKeyExists(HKEY Key,char *Name);
extern bool WindowIconsAre256();
extern void DisplayLastError(char *TitleText=NULL);
extern HFONT MakeFont(char *Typeface,int Height,int Width=0,int Boldness=FW_NORMAL,
                        bool Italic=0,bool Underline=0,bool Strikeout=0);
extern COLORREF GetMidColour(COLORREF RGB1,COLORREF RGB2);
extern COLORREF DimColour(COLORREF Col,double DimAmount);

#ifndef DSTRING_H
#ifdef EASYSTR_H
extern EasyStr GetCurrentDir();
extern EasyStr GetEXEDir();
extern EasyStr GetEXEFileName();
#endif
#endif

extern bool GetWindowPositionData(HWND Win,WINPOSITIONDATA *wpd);

#ifdef EASYSTR_H
extern EasyStr GetPPEasyStr(char *SectionName,char *KeyName,char *Default,char *FileName);
extern EasyStr FileSelect(HWND Owner,char *Title,char *DefaultDir,char *Types,int InitType,int LoadFlag,EasyStr DefExt="",char *DefFile="");

#ifdef _SHLOBJ_H_
extern EasyStr GetLinkDest(EasyStr LinkFileName,WIN32_FIND_DATA *wfd,HWND UIParent=NULL,
                     IShellLink *Link=NULL,IPersistFile* File=NULL);
extern HRESULT CreateLink(char *LinkFileName,char *TargetFileName,char *Description=NULL,
                    IShellLink *Link=NULL,IPersistFile* File=NULL,
                    char *IconPath=NULL,int IconIdx=0,
                    bool NoOverwrite=0);
#endif
#endif
extern void DeleteDirAndContents(char *Dir);

#ifdef _INC_COMMCTRL
extern void CentreLVItem(HWND LV,int iItem,int State=-1);
extern void GetTabControlPageSize(HWND Tabs,RECT *rc);
#endif
typedef struct{
  LONG Width;
  LONG Height;
}WIDTHHEIGHT;
extern WIDTHHEIGHT GetTextSize(HFONT Font,char *Text);
extern WIDTHHEIGHT GetCheckBoxSize(HFONT Font=NULL,char *Text=NULL);
#ifdef _INC_TOOLHELP32
typedef bool (WINAPI *LPTOOLHELPMODULEWALK)(HANDLE,LPMODULEENTRY32);
typedef HANDLE (WINAPI *LPTOOLHELPCREATESNAPSHOT)(DWORD,DWORD);
extern void GetWindowExePaths(HWND Win,char *Buf,int BufLen);
#endif
extern char *RemoveIllegalFromPath(char *Path,bool DriveIncluded,bool RemoveWild=true,char ReplaceChar='-',bool STPath=0);
extern char *RemoveIllegalFromName(char *Name,bool RemoveWild=true,char ReplaceChar='-');
LPARAM lParamPointsToParent(HWND Win,LPARAM lPar);
extern int CBAddString(HWND Combo,char *String);
extern int CBAddString(HWND Combo,char *String,DWORD Data);
extern int CBFindItemWithData(HWND Combo,DWORD Data);
extern int CBSelectItemWithData(HWND Combo,DWORD Data);
extern int CBGetSelectedItemData(HWND Combo);
extern void MoveWindowClient(HWND Win,int x,int y,int w,int h);
extern void GetWindowRectRelativeToParent(HWND Win,RECT *pRc);
#define IsCachedPrivateProfile() (0)
#define UnCachePrivateProfile()
#define CachePrivateProfile(a)


#ifdef _INC_COMMCTRL
extern void ToolsDeleteWithIDs(HWND,HWND,DWORD,...);
extern void ToolsDeleteAllChildren(HWND,HWND);
extern void ToolAddWindow(HWND,HWND,char*);

extern HTREEITEM TreeSelectItemWithData(HWND,long,HTREEITEM=TVI_ROOT);
extern int TreeGetMaxItemWidth(HWND,HTREEITEM=TVI_ROOT,int=0);

#define PAD_ALIGN_CENTRE b0000
#define PAD_ALIGN_LEFT   b0001
#define PAD_ALIGN_RIGHT  b0010
#define PAD_ALIGN_TOP    b0100
#define PAD_ALIGN_BOTTOM b1000
#define PAD_NAMES    b10000000

extern void ImageList_AddPaddedIcons(HIMAGELIST,int,char *,...);
extern void ImageList_AddPaddedIcons(HIMAGELIST,int,int,...);
extern void ImageList_AddPaddedIcons(HIMAGELIST,int,HICON,...);

extern int LVGetSelItem(HWND);
#ifdef EASYSTR_H
EasyStr LVGetItemText(HWND,int);
#endif

#endif

#ifdef EASYSTR_H
extern EasyStr GetWindowTextStr(HWND Win);
extern EasyStr LoadWholeFileIntoStr(char *File);
extern bool SaveStrAsFile(EasyStr &s,char *File);
extern EasyStr ShortenPath(EasyStr,HFONT,int);
#endif

typedef struct{
  bool LShift,RShift;
  bool LCtrl,RCtrl;
  bool LAlt,RAlt;
}MODIFIERSTATESTRUCT;

extern MODIFIERSTATESTRUCT GetLRModifierStates();

extern HDC CreateScreenCompatibleDC();
extern HBITMAP CreateScreenCompatibleBitmap(int,int);
//---------------------------------------------------------------------------
#else

#include "notwin_mymisc.h"

#endif

#ifdef UNIX
#include "x/x_mymisc.h"
#endif

#ifdef BEOS
#include "beos/be_mymisc.h"
#endif

#endif

