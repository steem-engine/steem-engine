/*---------------------------------------------------------------------------
FILE: mymisc.cpp
MODULE: helper
DESCRIPTION: Many miscellaneous functions from all areas of programming that
just refuse to be categorized.
---------------------------------------------------------------------------*/

#ifndef MYMISC_CPP
#define MYMISC_CPP

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <binary.h>
#include <clarity.h>
#include <string.h>

#ifndef WIN_ONLY
#ifdef WIN32
#define WIN_ONLY(s) s
#else
#define WIN_ONLY(s)
#endif
#endif

#include <mymisc.h>

#ifndef HEXTOVAL_FUNC
#define HEXTOVAL_FUNC
unsigned long HexToVal(char *HexStr)
{
  if (strlen(HexStr)>100) return 0;
  char *numstring=new char[strlen(HexStr)+1];
  char *hex=numstring;
  strcpy(hex,HexStr);
  strupr(hex);

  // Get rid of 0x or $
  if (hex[0]=='$') hex++;
  else if (hex[1]=='X') hex+=2;

  // Search from end to get first allowable character
  char *l;
  for (l=hex+strlen(hex);l>=hex;l--){
    if ((*l>='0' && *l<='9') || (*l>='A' && *l<='F')) break;
  }
  if (l<hex){  // No allowable characters
    delete[] numstring;
    return 0;
  }
  unsigned long ret=0,val;
  char numdid=(char)min((int)(l-hex)+1,8);
  char *let;
  for (int n=0;n<numdid;n++){
    let=l-n;
    if (*let>='0' && *let<='9'){
      val=(*let)-'0';
    }else if (*let>='A' && *let<='F'){
      val=(*let)-'A';
      val+=10;
    }else
      break;

    if (n>0) val*=(unsigned long)pow(16,n);
    ret+=val;
  }
  delete[] numstring;
  return ret;
}
#endif
//---------------------------------------------------------------------------
bool LoadBool(FILE *f)
{
  static char b;
  fread(&b,1,1,f);
  return b!=0;
}
//---------------------------------------------------------------------------
void SaveBool(bool b,FILE *f)
{
  static char buf;
  buf=b;
  fwrite(&buf,1,1,f);
}
//---------------------------------------------------------------------------
int LoadInt(FILE *f)
{
  static int i;
  fread(&i,sizeof(i),1,f);
  return i;
}
//---------------------------------------------------------------------------
void SaveInt(int i,FILE *f)
{
  fwrite(&i,sizeof(i),1,f);
}
//---------------------------------------------------------------------------
void LoadChars(char *buf,FILE *f)
{
  static int p;
  p=0;
  do{
    fread(buf+p,1,1,f);
  }while(buf[p++]);
}
//---------------------------------------------------------------------------
void SaveChars(char *buf,FILE *f)
{
  static int p;
  static char c;
  p=0;
  do{
    c=buf[p++];
    fwrite(&c,1,1,f);
  }while(c);
}
//---------------------------------------------------------------------------
long GetFileLength(FILE *f)
{
  long pos=ftell(f);
  fseek(f,0,SEEK_END);
  long len=ftell(f);
  fseek(f,pos,SEEK_SET);
  return len;
}
//---------------------------------------------------------------------------
void RemoveFileNameFromPath(char *fil,bool rem)
{
  if (fil[0]) *(GetFileNameFromPath(fil)-rem)=0;
}
//---------------------------------------------------------------------------
bool has_extension_list(char *Filename,char *Ext,...)
{
  char **ExtList=&Ext;
  char *FileExt=strrchr(GetFileNameFromPath(Filename),'.');
  if (FileExt==NULL) return 0;

  FileExt++;
  char *Extension=*ExtList;
  while (Extension){
    if (Extension[0]=='.') Extension++;
    if (IsSameStr_I(FileExt,Extension)) return true;
    ExtList++;
    Extension=*ExtList;
  }
  return 0;
}
//---------------------------------------------------------------------------
bool has_extension(char *Filename,char *Ext)
{
  return has_extension_list(Filename,Ext,NULL);
}
//---------------------------------------------------------------------------
bool MatchesAnyString(char *StrToCompare,char *Str,...)
{
  char **StrList=&Str;
  while (*StrList){
    if (IsSameStr(StrToCompare,*(StrList++))) return true;
  }
  return 0;
}
//---------------------------------------------------------------------------
bool MatchesAnyString_I(char *StrToCompare,char *Str,...)
{
  char **StrList=&Str;
  while (*StrList){
    if (IsSameStr_I(StrToCompare,*(StrList++))) return true;
  }
  return 0;
}
//---------------------------------------------------------------------------
#ifdef EASYSTR_H

#include <sys/stat.h>

EasyStr GetUniquePath(EasyStr path,EasyStr name)
{
  NO_SLASH(path);

  EasyStr ext;
  char *p=strrchr(name,'.');
  if (p){
    ext=p;
    *p=0;
  }
  EasyStr ret=path+SLASH+name+ext;

  struct stat s;
  int i=2;
  while (stat(ret,&s)==0) ret=path+SLASH+name+" ("+(i++)+")"+ext;
  return ret;
}

#endif
//---------------------------------------------------------------------------
#if defined(_INC_TIME) || defined(_SYS_TIME_H)
DWORD TMToDOSDateTime(struct tm *lpTime)
{
  DWORD Ret=(lpTime->tm_sec/2) & 0x1f;
  Ret |= (lpTime->tm_min & 0x3f) << 5;
  Ret |= (lpTime->tm_hour & 0x1f) << 11;
  Ret |= (lpTime->tm_mday & 0x1f) << 16;
  Ret |= ((lpTime->tm_mon+1) & 0xf) << 21;
  Ret |= ((lpTime->tm_year+1900-1980) & 0x3f) << 25;
  return Ret;
}
#endif
//---------------------------------------------------------------------------
int log_to_base_2(unsigned long x)
{
  int n;
  for (n=0;x>>n;n++);
  return n-1;
}
//---------------------------------------------------------------------------
char *GetFileNameFromPath(char *fil)
{
  int Len=strlen(fil);
  if (Len==0) return fil;

  char *pos=fil+Len-1;
  for (;pos>=fil;pos--){
    if (*pos=='\\' || *pos=='/' || *pos==':') break;
  }
  return pos+1;
}
//---------------------------------------------------------------------------
#ifdef WIN32

void RemoveProps(HWND Win,char *Prop1,...)
{
  char* *lpProp=&Prop1;
  while (*lpProp) RemoveProp(Win,*(lpProp++));
}
//---------------------------------------------------------------------------
void Border3D(HDC dc,int x,int y,int w,int h,
              DWORD col0,DWORD col1,DWORD col2,DWORD col3)
{
  int x1=(x+w)-1,y1=(y+h)-1;
  HPEN o_pen=(HPEN)SelectObject(dc,CreatePen(PS_SOLID,1,col0));
  MoveToEx(dc,x+1,y1-1,0);
  LineTo(dc,x+1,y+1);
  LineTo(dc,x1-1,y+1);
  DeleteObject(SelectObject(dc,o_pen));

  o_pen=(HPEN)SelectObject(dc,CreatePen(PS_SOLID,1,col2));
  MoveToEx(dc,x+1,y1-1,0);
  LineTo(dc,x1-1,y1-1);
  LineTo(dc,x1-1,y);
  DeleteObject(SelectObject(dc,o_pen));

  o_pen=(HPEN)SelectObject(dc,CreatePen(PS_SOLID,1,col1));
  MoveToEx(dc,x,y1,0);
  LineTo(dc,x,y);
  LineTo(dc,x1,y);
  DeleteObject(SelectObject(dc,o_pen));

  o_pen=(HPEN)SelectObject(dc,CreatePen(PS_SOLID,1,col3));
  MoveToEx(dc,x,y1,0);
  LineTo(dc,x1,y1);
  LineTo(dc,x1,y-1);
  DeleteObject(SelectObject(dc,o_pen));
}
//---------------------------------------------------------------------------
void Box3D(HDC dc,int x,int y,int w,int h,bool d)
{
  RECT rc={x,y,x+w+1,y+h+1};
  DrawEdge(dc,&rc,d ? EDGE_SUNKEN:EDGE_RAISED,BF_RECT);
}
//---------------------------------------------------------------------------
void CentreTextOut(HDC dc,int x,int y,int w,int h,
    char *text,int len)
{
  if (len==-1) len=strlen(text);

  SIZE sz;GetTextExtentPoint32(dc,text,len,&sz);
  TextOut(dc,x+(w/2)-(sz.cx/2),y+(h/2)-(sz.cy/2),text,len);
}
//---------------------------------------------------------------------------
void GetLongPathName(char *src,char *dest,int maxlen)
{
  if (src[0]=='\\'){
    if (src[1]=='\\'){
      // Network file, leave alone
      int i=0;
      while (i<maxlen){
        dest[i]=src[i];
        if (src[i]==0) break;
        i++;
      }
      dest[i]=0; // make sure null-terminated
      return;
    }
  }

  bool TooLong=0,FileName=true;
  int Mov;
  char *longpath=new char[maxlen+1];longpath[0]=0;
  char *shortpath=new char[strlen(src)+1];strcpy(shortpath,src);
  char *spp=shortpath+strlen(shortpath)-1;
  WIN32_FIND_DATA *wfd=new WIN32_FIND_DATA;
  for (;;){
    do{
      spp--;
    }while (*spp!='\\' && *spp!='/' && spp>shortpath);
    if (spp<=shortpath) break;

    HANDLE fh=FindFirstFile(shortpath,wfd);
    if (fh!=INVALID_HANDLE_VALUE){
      FindClose(fh);
    }else{
      wfd->cFileName[0]=0;
      if (FileName) strcpy(wfd->cFileName,GetFileNameFromPath(shortpath));
    }
    if (wfd->cFileName[0]){
      Mov=strlen(wfd->cFileName);
      if ((int)strlen(longpath)+Mov+1>=maxlen){
        TooLong=true;break;
      }
      memmove(longpath+Mov+1,longpath,strlen(longpath)+1);
      longpath[0]=*spp;
      memcpy(longpath+1,wfd->cFileName,Mov);
    }
    *spp=0;
    FileName=0;
  }
  delete wfd;

  Mov=strlen(shortpath);
  if ((int)strlen(longpath)+Mov>=maxlen){
    TooLong=true;
  }else{
    memmove(longpath+Mov,longpath,strlen(longpath)+1);
    memcpy(longpath,shortpath,Mov);
  }
  if (TooLong==0){
    strcpy(dest,longpath);
  }else if ((int)strlen(src)<maxlen){
    strcpy(dest,src);
  }else{
    dest[0]=0;
  }
  delete[] shortpath;
  delete[] longpath;
}
//---------------------------------------------------------------------------
void SetWindowAndChildrensFont(HWND Win,HFONT fnt)
{
  if (Win==NULL) return;
  SendMessage(Win,WM_SETFONT,(WPARAM)fnt,0);
  HWND Child=GetWindow(Win,GW_CHILD);
  while (Child){
    SendMessage(Child,WM_SETFONT,(WPARAM)fnt,0);
    Child=GetWindow(Child,GW_HWNDNEXT);
  }
}
//---------------------------------------------------------------------------
void RemoveAllMenuItems(HMENU menu)
{
  int n=GetMenuItemCount(menu);
  for (int i=0;i<n;i++) RemoveMenu(menu,0,MF_BYPOSITION);
}
//---------------------------------------------------------------------------
void DeleteAllMenuItems(HMENU menu)
{
  int n=GetMenuItemCount(menu);
  for (int i=0;i<n;i++) DeleteMenu(menu,0,MF_BYPOSITION);
}
//---------------------------------------------------------------------------
void CentreWindow(HWND Win,bool Redraw)
{
  RECT rc;GetWindowRect(Win,&rc);
  int W=rc.right-rc.left, H=rc.bottom-rc.top;
  MoveWindow(Win,(GetSystemMetrics(SM_CXSCREEN)-W)/2,(GetSystemMetrics(SM_CYSCREEN)-H)/2,W,H,Redraw);
}
//---------------------------------------------------------------------------
bool RegKeyExists(HKEY Key,char *Name)
{
  HKEY K;
  if (RegOpenKey(Key,Name,&K)==ERROR_SUCCESS){
    RegCloseKey(K);
    return 1;
  }
  return 0;
}
//---------------------------------------------------------------------------
bool WindowIconsAre256()
{
  HKEY Key;
  if (RegOpenKey(HKEY_CURRENT_USER,"Control Panel\\desktop\\WindowMetrics",&Key)==ERROR_SUCCESS){
    char BPP[200]={0,0,0,0,0,0,0};
    DWORD Size=200;
    if (RegQueryValueEx(Key,"Shell Icon BPP",NULL,NULL,(BYTE*)BPP,&Size)!=ERROR_SUCCESS) BPP[0]='4';
    RegCloseKey(Key);
    return atoi(BPP)>4;
  }
  return 0;
}
//---------------------------------------------------------------------------
void DisplayLastError(char *TitleText)
{
  HLOCAL lpMsgBuf;
  DWORD Err;
  char Title[50];

  Err=GetLastError();

  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,GetLastError(),MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (char*) &lpMsgBuf,0,NULL);

  if (TitleText==NULL){
    strcpy(Title,"Error #");
    ultoa(Err,Title+7,10);
  }else{
    strcpy(Title,TitleText);
  }
  MessageBox(NULL,(char*)lpMsgBuf,Title,MB_OK | MB_ICONINFORMATION);

  LocalFree(lpMsgBuf);
}
//---------------------------------------------------------------------------
HFONT MakeFont(char *Typeface,int Height,int Width,int Boldness,
                        bool Italic,bool Underline,bool Strikeout)
{
  return CreateFont(Height,Width,0,0,Boldness,Italic,Underline,Strikeout,ANSI_CHARSET,
            OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE,Typeface);
}
//---------------------------------------------------------------------------
COLORREF GetMidColour(COLORREF RGB1,COLORREF RGB2)
{
  int C1=GetRValue(RGB1),C2=GetRValue(RGB2);
  int R=(max(C1,C2)-min(C1,C2))/2 + min(C1,C2);

  C1=GetGValue(RGB1),C2=GetGValue(RGB2);
  int G=(max(C1,C2)-min(C1,C2))/2 + min(C1,C2);

  C1=GetBValue(RGB1),C2=GetBValue(RGB2);
  int B=(max(C1,C2)-min(C1,C2))/2 + min(C1,C2);

  return RGB(R,G,B);
}
//---------------------------------------------------------------------------
COLORREF DimColour(COLORREF Col,double DimAmount)
{
  if (DimAmount<0) return Col;

  if (DimAmount<1){
    return RGB( double(GetRValue(Col))*DimAmount,
                double(GetGValue(Col))*DimAmount,
                double(GetBValue(Col))*DimAmount
                  );
  }else{
    return RGB( max(GetRValue(Col)-BYTE(DimAmount),0),
                max(GetGValue(Col)-BYTE(DimAmount),0),
                max(GetBValue(Col)-BYTE(DimAmount),0)
                  );
  }
}
//---------------------------------------------------------------------------
#ifndef DSTRING_H
#ifdef EASYSTR_H
EasyStr GetCurrentDir()
{
  EasyStr Path;
  Path.SetLength(MAX_PATH);
  GetCurrentDirectory(MAX_PATH,Path);
  return Path;
}
EasyStr GetEXEDir()
{
  EasyStr Path;
  Path.SetLength(MAX_PATH);
  GetModuleFileName(NULL,Path,MAX_PATH);
  RemoveFileNameFromPath(Path,REMOVE_SLASH);
  GetLongPathName(Path,Path,MAX_PATH);
  return Path;
}
EasyStr GetEXEFileName()
{
  EasyStr Path;
  Path.SetLength(MAX_PATH);
  GetModuleFileName(NULL,Path,MAX_PATH);
  GetLongPathName(Path,Path,MAX_PATH);
  return Path;
}
#endif
#endif
//---------------------------------------------------------------------------
bool GetWindowPositionData(HWND Win,WINPOSITIONDATA *wpd)
{
  if (IsWindow(Win)==0) return 1;

  RECT rc;
  SystemParametersInfo(SPI_GETWORKAREA,0,&rc,0);

  WINDOWPLACEMENT wp;
  wp.length=sizeof(WINDOWPLACEMENT);
  GetWindowPlacement(Win,&wp);

  wpd->Left=rc.left+wp.rcNormalPosition.left;
  wpd->Top=rc.top+wp.rcNormalPosition.top;
  wpd->Width=wp.rcNormalPosition.right-wp.rcNormalPosition.left;
  wpd->Height=wp.rcNormalPosition.bottom-wp.rcNormalPosition.top;

  long l=GetWindowLong(Win,GWL_STYLE);

  wpd->Maximized=bool(l & WS_MAXIMIZE);
  if (wp.showCmd==SW_SHOWMINIMIZED && (wp.flags & WPF_RESTORETOMAXIMIZED)){
    wpd->Maximized=true;
  }

  wpd->Minimized=bool(l & WS_MINIMIZE);

  return 0;
}
//---------------------------------------------------------------------------
#ifdef EASYSTR_H

EasyStr GetPPEasyStr(char *SectionName,char *KeyName,char *Default,char *FileName)
{
  EasyStr Temp;
  Temp.SetLength(5000);
  GetPrivateProfileString(SectionName,KeyName,Default,Temp,5000,FileName);
  return Temp;
}
//---------------------------------------------------------------------------
EasyStr FileSelect(HWND Owner,char *Title,char *DefaultDir,char *Types,int InitType,int LoadFlag,EasyStr DefExt,char *DefFile)
{
  char fil[MAX_PATH+1];
  if (DefFile[0]){
    strcpy(fil,DefFile);
  }else{
    fil[0]=0;
  }

  OPENFILENAME ofn;
  ZeroMemory(&ofn,sizeof(OPENFILENAME));

  ofn.lStructSize=sizeof(OPENFILENAME);
  ofn.hwndOwner=Owner;
  ofn.hInstance=(HINSTANCE)GetModuleHandle(NULL);
  ofn.lpstrFilter=Types;
  ofn.lpstrCustomFilter=NULL;
  ofn.nMaxCustFilter=0;
  ofn.nFilterIndex=InitType;
  ofn.lpstrFile=fil;
  ofn.nMaxFile=MAX_PATH;
  ofn.lpstrFileTitle=NULL;
  ofn.nMaxFileTitle=0;
  ofn.lpstrInitialDir=DefaultDir;
  ofn.lpstrTitle=Title;
  ofn.Flags=OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
  if (LoadFlag==1){
    ofn.Flags|=OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
  }else if (LoadFlag==0){
    ofn.Flags|=OFN_OVERWRITEPROMPT;
  }
  ofn.lpstrDefExt=DefExt.IsEmpty() ? NULL:DefExt.Text;
  ofn.lpfnHook=0;
  ofn.lpTemplateName=0;

  if ((LoadFlag ? GetOpenFileName(&ofn):GetSaveFileName(&ofn))==0) fil[0]=0;

  EasyStr a=fil;
  return a;
}
//---------------------------------------------------------------------------
#ifdef _SHLOBJ_H_
EasyStr GetLinkDest(EasyStr LinkFileName,WIN32_FIND_DATA *wfd,HWND UIParent,
                     IShellLink *Link,IPersistFile* File)
{
  HRESULT hres;
  bool ReleaseLink=true,ReleaseFile=true;
  EasyStr Path;

  // Get a pointer to the IShellLink interface.
  if (Link==NULL){
    hres=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&Link);
    if (SUCCEEDED(hres)==0) Link=NULL;
  }else{
    ReleaseLink=0;
  }
  if (Link){
    if (File==NULL){
      // Query IShellLink for the IPersistFile interface for saving the shortcut in persistent storage.
      hres=Link->QueryInterface(IID_IPersistFile,(void**)&File);
      if (SUCCEEDED(hres)==0) File=NULL;
    }else{
      ReleaseFile=0;
    }
    if (File){
      // Ensure that the path is Unicode.
      wchar_t WideName[MAX_PATH+1];
      MultiByteToWideChar(CP_ACP,0,LinkFileName,-1,WideName,MAX_PATH);

      // Load the shortcut.
      hres=File->Load(WideName,STGM_READ);
      if (SUCCEEDED(hres)){
        // Resolve the link.
        if (UIParent!=NULL) hres=Link->Resolve(UIParent,SLR_ANY_MATCH | SLR_UPDATE);

        if (SUCCEEDED(hres)){
          // Get the path to the link target.
          Path.SetLength(MAX_PATH+1);
          ZeroMemory(wfd,sizeof(WIN32_FIND_DATA));
          hres=Link->GetPath(Path.Text,MAX_PATH,wfd,(DWORD)0);
          if (SUCCEEDED(hres)==0) Path="";
        }
      }
      if (ReleaseFile) File->Release();
    }
    if (ReleaseLink) Link->Release();
  }
  return Path;
}
//---------------------------------------------------------------------------
HRESULT CreateLink(char *LinkFileName,char *TargetFileName,char *Description,
                    IShellLink *Link,IPersistFile* File,char *IconPath,int IconIdx,
                    bool NoOverwrite)
{
  HRESULT hres=0;
  bool ReleaseLink=true,ReleaseFile=true;

  // Get a pointer to the IShellLink interface.
  if (Link==NULL){
    hres=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&Link);
    if (SUCCEEDED(hres)==0) Link=NULL;
  }else{
    ReleaseLink=0;
  }
  if (Link){
    if (File==NULL){
      // Query IShellLink for the IPersistFile interface for saving the
      // shortcut in persistent storage.
      hres=Link->QueryInterface(IID_IPersistFile,(void**)&File);
      if (SUCCEEDED(hres)==0) File=NULL;
    }else{
      ReleaseFile=0;
    }
    if (File){
      wchar_t WideName[MAX_PATH+1];
      MultiByteToWideChar(CP_ACP,0,LinkFileName,-1,WideName,MAX_PATH);

      hres=0;
      if (NoOverwrite){
        if (access(LinkFileName,0)==0){
          hres=File->Load(WideName,STGM_READ);
          if (SUCCEEDED(hres)){
            WORD HotKey=0;
            char OldDesc[500],Args[500],OldIconPath[MAX_PATH],WorkDir[MAX_PATH];
            int OldIconIdx,ShowCmd;
            Link->GetHotkey(&HotKey);
            Link->GetArguments(Args,500);
            Link->GetDescription(OldDesc,500);
            Link->GetIconLocation(OldIconPath,MAX_PATH,&OldIconIdx);
            Link->GetShowCmd(&ShowCmd);
            Link->GetWorkingDirectory(WorkDir,MAX_PATH);

            Link->SetPath(TargetFileName);
            if (Description){
              Link->SetDescription(Description);
            }else{
              Link->SetDescription(OldDesc);
            }
            if (IconPath){
              Link->SetIconLocation(IconPath,IconIdx);
            }else{
              Link->SetIconLocation(OldIconPath,OldIconIdx);
            }
            Link->SetHotkey(HotKey);
            Link->SetArguments(Args);
            Link->SetShowCmd(ShowCmd);
            Link->SetWorkingDirectory(WorkDir);
            hres=1;
          }
        }
      }
      if (hres==0){
        Link->SetPath(TargetFileName);
        if (Description) Link->SetDescription(Description);
        if (IconPath) Link->SetIconLocation(IconPath,IconIdx);
      }
      File->Save(WideName,true);

      if (ReleaseFile) File->Release();
    }
    if (ReleaseLink) Link->Release();
  }
  return hres;
}

#endif
#endif
//---------------------------------------------------------------------------
void DeleteDirAndContents(char *Dir)
{
  static WIN32_FIND_DATA wfd;
  bool PutSlashBack=0;
  char NewDir[MAX_PATH+1];

  if (Dir[strlen(Dir)-1]=='\\'){
    Dir[strlen(Dir)-1]=0;
    PutSlashBack=true;
  }

  if (GetFileAttributes(Dir)==0xffffffff){
    if (PutSlashBack) Dir[strlen(Dir)]='\\';
    return;
  }

  strcpy(NewDir,Dir);
  strcat(NewDir,"\\*.*");
  HANDLE FHan=FindFirstFile(NewDir,&wfd);
  if (FHan!=INVALID_HANDLE_VALUE){
    do{
      if (strcmp(wfd.cFileName,".") && strcmp(wfd.cFileName,"..")){
        strcpy(NewDir,Dir);
        strcat(NewDir,"\\");
        strcat(NewDir,wfd.cFileName);
        if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
          DeleteDirAndContents(NewDir);
        }else{
          DeleteFile(NewDir);
        }
      }
    }while (FindNextFile(FHan,&wfd));
    FindClose(FHan);
  }

  RemoveDirectory(Dir);

  if (PutSlashBack) Dir[strlen(Dir)]='\\';
}
//---------------------------------------------------------------------------
#ifdef _INC_COMMCTRL
void CentreLVItem(HWND LV,int iItem,int State)
{
  RECT rc,Client;
  LV_ITEM lvi;

  if (State==-1){
    lvi.iItem=iItem;
    lvi.iSubItem=0;
    lvi.stateMask=LVIS_SELECTED | LVIS_FOCUSED;
    State=SendMessage(LV,LVM_GETITEMSTATE,iItem,(LPARAM)&lvi);
  }

  lvi.iItem=iItem;
  lvi.iSubItem=0;
  lvi.stateMask=LVIS_FOCUSED | LVIS_SELECTED;
  lvi.state=0;
  SendMessage(LV,LVM_SETITEMSTATE,iItem,(LPARAM)&lvi);

  ListView_GetItemRect(LV,iItem,&rc,LVIR_BOUNDS);

  rc.bottom=rc.bottom-rc.top-2;
  GetClientRect(LV,&Client);
  Client.right/=2;

  if (rc.bottom<Client.bottom){
    rc.bottom/=2;
    Client.bottom/=2;
  }

  SendMessage(LV,LVM_SETITEMPOSITION,iItem,LONG((WORD(Client.right-16))) | (DWORD(WORD(Client.bottom-rc.bottom)) << 16));

  lvi.iItem=iItem;
  lvi.iSubItem=0;
  lvi.stateMask=LVIS_FOCUSED | LVIS_SELECTED;
  lvi.state=State;
  SendMessage(LV,LVM_SETITEMSTATE,iItem,(LPARAM)&lvi);
}
//---------------------------------------------------------------------------
void GetTabControlPageSize(HWND Tabs,RECT *rc)
{
  POINT pt={0,0};

  GetWindowRect(Tabs,rc);
  ClientToScreen(GetParent(Tabs),&pt);
  OffsetRect(rc,-pt.x,-pt.y);
  SendMessage(Tabs,TCM_ADJUSTRECT,0,(LPARAM)rc);
}
#endif
//---------------------------------------------------------------------------
// Allow for &'s?
WIDTHHEIGHT GetTextSize(HFONT Font,char *Text)
{
  WIDTHHEIGHT wh;
  HDC TempDC=CreateCompatibleDC(NULL);
  HFONT OldFont=(HFONT)SelectObject(TempDC,Font);

  GetTextExtentPoint32(TempDC,Text,strlen(Text),(SIZE*)&wh);

  SelectObject(TempDC,OldFont);
  DeleteDC(TempDC);

  wh.Width++; // Allow for disabled text

  return wh;
}
//---------------------------------------------------------------------------
WIDTHHEIGHT GetCheckBoxSize(HFONT Font,char *Text)
{
  WIDTHHEIGHT wh;
  BITMAP bm;
  HBITMAP BoxBmp=LoadBitmap(NULL,MAKEINTRESOURCE(32759) /*OBM_CHECKBOXES*/ );

  GetObject(BoxBmp,sizeof(BITMAP),&bm);

  DeleteObject(BoxBmp);

  wh.Width=bm.bmWidth/4;
  wh.Height=bm.bmHeight/3;

  if (Text){
    WIDTHHEIGHT TextWH=GetTextSize(Font,Text);
    wh.Width+=5+1+TextWH.Width+1;
    if (TextWH.Height>wh.Height) wh.Height=TextWH.Height;
  }

  return wh;
}
//---------------------------------------------------------------------------
#ifdef _INC_TOOLHELP32
typedef bool (WINAPI *LPTOOLHELPMODULEWALK)(HANDLE,LPMODULEENTRY32);
typedef HANDLE (WINAPI *LPTOOLHELPCREATESNAPSHOT)(DWORD,DWORD);

void GetWindowExePaths(HWND Win,char *Buf,int BufLen)
{
	DWORD ProcID;
	HANDLE Snap;
	MODULEENTRY32 me;
	LPTOOLHELPCREATESNAPSHOT pCreateToolhelp32Snapshot;
	LPTOOLHELPMODULEWALK pModule32First;
	LPTOOLHELPMODULEWALK pModule32Next;
  HINSTANCE hKernel;

	GetWindowThreadProcessId(Win,&ProcID);

  hKernel=GetModuleHandle("KERNEL32.DLL");
  if (hKernel){
    pCreateToolhelp32Snapshot=(LPTOOLHELPCREATESNAPSHOT)GetProcAddress(hKernel,"CreateToolhelp32Snapshot");

		pModule32First=(LPTOOLHELPMODULEWALK)GetProcAddress(hKernel,"Module32First");
    pModule32Next=(LPTOOLHELPMODULEWALK)GetProcAddress(hKernel,"Module32Next");

    if (pModule32First==NULL || pModule32Next==NULL || pCreateToolhelp32Snapshot==NULL) return;
  }else{
		return;
	}

 	Snap=pCreateToolhelp32Snapshot(TH32CS_SNAPMODULE,ProcID);

	Buf[0]=0;
  char *pBuf=Buf,*pBufEnd=Buf+BufLen;
	me.dwSize=sizeof(MODULEENTRY32);
	if (pModule32First(Snap,&me)){
		do{
      if (strcmpi(me.szExePath+(strlen(me.szExePath)-3),"EXE")==0){
        if (pBuf+strlen(me.szExePath)+1>=pBufEnd) break;
        strcpy(pBuf,me.szExePath);
        pBuf+=strlen(me.szExePath)+1;
        pBuf[0]=0;
      }
			me.dwSize=sizeof(MODULEENTRY32);
		}while (pModule32Next(Snap,&me));
	}

	CloseHandle(Snap);
}
#endif
//---------------------------------------------------------------------------
char *RemoveIllegalFromPath(char *Path,bool DriveIncluded,bool RemoveWild,char ReplaceChar,bool STPath)
{
  char *Name,*PathStart=Path,*PathEnd=Path+strlen(Path)-1,*FilNam;
  bool GotSlash;

  if (DriveIncluded) PathStart+=3;

  Name=PathEnd;
  do{
    GotSlash=0;
    while (Name>=PathStart){
      if (*Name=='\\' || (*Name=='/' && STPath==0)){
        GotSlash=true;
        break;
      }
      Name--;
    }
    FilNam=LPSTR(GotSlash ? Name+1:PathStart);
    while (FilNam<=PathEnd){
      char c=*FilNam;
      if (c=='\\' || (c=='/' && STPath==0)){
        break;
      }else{
        switch (c){
          case ':':case '/':
          case '"':case '<':
          case '>':case '|':
            *FilNam=ReplaceChar;
            break;
          case '*':case '?':
            if (RemoveWild) *FilNam=ReplaceChar;
            break;
        }
      }
      FilNam++;
    }
    Name--;
  }while (Name>PathStart);

  return Path;
}
//---------------------------------------------------------------------------
// This takes just a file name, not the folder it is in/drive it is on!
char *RemoveIllegalFromName(char *Name,bool RemoveWild,char ReplaceChar)
{
  int Len=strlen(Name);
  for (int i=0;i<Len;i++){
    switch (Name[i]){
      case ':':case '/':case '"':case '<':case '>':case '|':case '\\':
        Name[i]=ReplaceChar;
        break;
      case '*':case '?':
        if (RemoveWild) Name[i]=ReplaceChar;
        break;
    }
  }
  return Name;
}
//---------------------------------------------------------------------------
LPARAM lParamPointsToParent(HWND Win,LPARAM lPar)
{
  POINT WinPT={0,0},ParentPT={0,0};
  ClientToScreen(Win,&WinPT);
  ClientToScreen(GetParent(Win),&ParentPT);
  return LPARAM((LOWORD(lPar)+(WinPT.x-ParentPT.x)) | ((HIWORD(lPar)+(WinPT.y-ParentPT.y)) << 16));
}
//---------------------------------------------------------------------------
int CBAddString(HWND Combo,char *String)
{
  return SendMessage(Combo,CB_ADDSTRING,0,LPARAM(String));
}
//---------------------------------------------------------------------------
int CBAddString(HWND Combo,char *String,DWORD Data)
{
  int Idx=SendMessage(Combo,CB_ADDSTRING,0,LPARAM(String));
  if (Idx>=0) SendMessage(Combo,CB_SETITEMDATA,Idx,Data);
  return Idx;
}
//---------------------------------------------------------------------------
int CBFindItemWithData(HWND Combo,DWORD Data)
{
  int n,c=SendMessage(Combo,CB_GETCOUNT,0,0);
  for (n=0;n<c;n++){
    if ((DWORD)SendMessage(Combo,CB_GETITEMDATA,n,0)==Data) break;
  }
  if (n>=c) return CB_ERR;

  return n;
}
//---------------------------------------------------------------------------
int CBSelectItemWithData(HWND Combo,DWORD Data)
{
  int Idx=CBFindItemWithData(Combo,Data);
  if (Idx>=0) SendMessage(Combo,CB_SETCURSEL,Idx,0);
  return Idx;
}
//---------------------------------------------------------------------------
int CBGetSelectedItemData(HWND Combo)
{
  int SelIdx=SendMessage(Combo,CB_GETCURSEL,0,0);
  if (SelIdx>-1) return SendMessage(Combo,CB_GETITEMDATA,SelIdx,0);
  return 0;
}
//---------------------------------------------------------------------------
void MoveWindowClient(HWND Win,int x,int y,int w,int h)
{
  RECT rcWin;
  GetWindowRect(Win,&rcWin);

  RECT rcClient;
  GetClientRect(Win,&rcClient);

  POINT ptTL={0,0},ptBR={rcClient.right,rcClient.bottom};
  ClientToScreen(Win,&ptTL);
  ClientToScreen(Win,&ptBR);

  MoveWindow(Win,x-(ptTL.x-rcWin.left),y-(ptTL.x-rcWin.left),
                 w+(rcWin.right-ptBR.x),h+(rcWin.bottom-ptBR.y),true);
}
//---------------------------------------------------------------------------
void GetWindowRectRelativeToParent(HWND Win,RECT *pRc)
{
  GetWindowRect(Win,pRc);

  RECT rcPar;
  GetWindowRect(GetParent(Win),&rcPar);

  pRc->left-=rcPar.left;pRc->right-=rcPar.left;
  pRc->top-=rcPar.top;pRc->bottom-=rcPar.top;
}
//---------------------------------------------------------------------------
#ifdef _INC_COMMCTRL
void ToolsDeleteWithIDs(HWND ToolTip,HWND Parent,DWORD FirstID,...)
{
  DWORD *IDList=&FirstID;
  TOOLINFO ti;
  ti.cbSize=sizeof(TOOLINFO);
  ti.uFlags=TTF_IDISHWND;
  ti.hwnd=Parent;
  while (*IDList){
    ti.uId=(UINT)GetDlgItem(Parent,*IDList);
    if (ti.uId) SendMessage(ToolTip,TTM_DELTOOL,0,(LPARAM)&ti);
    IDList++;
  }
}
//---------------------------------------------------------------------------
void ToolsDeleteAllChildren(HWND ToolTip,HWND Parent)
{
  int c=SendMessage(ToolTip,TTM_GETTOOLCOUNT,0,0);

  TOOLINFO *pDelTI=new TOOLINFO[c];
  int ndel=0;

  // Get tools to be deleted (don't delete immediately as that would change index)
  for (int i=0;i<c;i++){
    TOOLINFO ti;
    ti.cbSize=sizeof(TOOLINFO);
    ti.lpszText=NULL;
    SendMessage(ToolTip,TTM_ENUMTOOLS,i,(LPARAM)&ti);
    if (ti.hwnd==Parent) pDelTI[ndel++]=ti;
  }

  // Delete tools
  for (int i=0;i<ndel;i++){
    SendMessage(ToolTip,TTM_DELTOOL,0,LPARAM(pDelTI+i));
  }
  delete[] pDelTI;
}
//---------------------------------------------------------------------------
void ToolAddWindow(HWND ToolTip,HWND Handle,char *Text)
{
  TOOLINFO ti;
  ti.cbSize=sizeof(TOOLINFO);
  ti.uFlags=TTF_IDISHWND | TTF_SUBCLASS;
  ti.hwnd=GetParent(Handle);
  ti.uId=(UINT)Handle;
  ti.lpszText=Text;
  SendMessage(ToolTip,TTM_ADDTOOL,0,(LPARAM)&ti);
}
//---------------------------------------------------------------------------
HTREEITEM TreeSelectItemWithData(HWND Tree,long n,HTREEITEM Item)
{
  TV_ITEM tvi;

  if (Item==TVI_ROOT) Item=(HTREEITEM)SendMessage(Tree,TVM_GETNEXTITEM,TVGN_CHILD,(LPARAM)Item);
  tvi.mask=TVIF_PARAM;
  while (Item){
    tvi.hItem=Item;
    SendMessage(Tree,TVM_GETITEM,0,LPARAM(&tvi));
    if (tvi.lParam==n){
      SendMessage(Tree,TVM_SELECTITEM,TVGN_CARET,LPARAM(Item));
      SendMessage(Tree,TVM_ENSUREVISIBLE,0,LPARAM(Item));
      return Item;
    }
    HTREEITEM ChildChosen=TreeSelectItemWithData(Tree,n,(HTREEITEM)SendMessage(Tree,TVM_GETNEXTITEM,TVGN_CHILD,(LPARAM)Item));
    if (ChildChosen) return ChildChosen;
    Item=(HTREEITEM)SendMessage(Tree,TVM_GETNEXTITEM,TVGN_NEXT,(LPARAM)Item);
  }
  return NULL;
}
//---------------------------------------------------------------------------
int TreeGetMaxItemWidth(HWND Tree,HTREEITEM Item,int Level)
{
  int MaxWidth=0;
  RECT rc;
  HTREEITEM ChildItem;

  if (Item==TVI_ROOT) Item=(HTREEITEM)SendMessage(Tree,TVM_GETNEXTITEM,TVGN_CHILD,(LPARAM)Item);

  while (Item){
    if (TreeView_GetItemRect(Tree,Item,&rc,true)){
      int Width=rc.right;
//      Width+=SendMessage(Tree,TVM_GETINDENT,0,0);
      MaxWidth=max(Width,MaxWidth);
    }
    ChildItem=(HTREEITEM)SendMessage(Tree,TVM_GETNEXTITEM,TVGN_CHILD,(LPARAM)Item);
    if (ChildItem) MaxWidth=max(TreeGetMaxItemWidth(Tree,ChildItem,Level+1),MaxWidth);

    Item=(HTREEITEM)SendMessage(Tree,TVM_GETNEXTITEM,TVGN_NEXT,(LPARAM)Item);
  }
  return MaxWidth;
}
//---------------------------------------------------------------------------
void ImageList_AddPaddedIcons(HIMAGELIST il,int Align,HICON Icon1,...)
{
  HICON *pIco=&Icon1;
  int il_w,il_h;
  ICONINFO o_ii,n_ii;
  BITMAP bi;

  ImageList_GetIconSize(il,&il_w,&il_h);
  RECT rc={0,0,il_w,il_h};

  HDC ScrDC=GetDC(NULL);
  HDC FromDC=CreateCompatibleDC(ScrDC);
  HDC ToDC=CreateCompatibleDC(ScrDC);
  n_ii.hbmColor=CreateCompatibleBitmap(ScrDC,il_w,il_h);
  n_ii.hbmMask=CreateBitmap(il_w,il_h,1,1,NULL);
  ReleaseDC(NULL,ScrDC);
  SetBkMode(ToDC,OPAQUE);SetROP2(ToDC,R2_COPYPEN);
  SetBkMode(FromDC,OPAQUE);SetROP2(FromDC,R2_COPYPEN);

  while (*pIco){
    GetIconInfo(*(pIco++),&o_ii);

    n_ii.fIcon=o_ii.fIcon;
    n_ii.xHotspot=o_ii.xHotspot;
    n_ii.yHotspot=o_ii.yHotspot;
    GetObject(o_ii.hbmColor,sizeof(BITMAP),&bi);
    int xo=(il_w-bi.bmWidth)/2,yo=(il_h-bi.bmHeight)/2;
    if ((Align & b0011)==b0001) xo=0;
    if ((Align & b0011)==b0010) xo=(il_w-bi.bmWidth);
    if ((Align & b1100)==b0100) yo=0;
    if ((Align & b1100)==b1000) yo=(il_h-bi.bmHeight);

    HBITMAP FromDefBmp=(HBITMAP)SelectObject(FromDC,o_ii.hbmMask);
    HBITMAP ToDefBmp=(HBITMAP)SelectObject(ToDC,n_ii.hbmMask);
    FillRect(ToDC,&rc,(HBRUSH)GetStockObject(WHITE_BRUSH));
    BitBlt(ToDC,xo,yo,bi.bmWidth,bi.bmHeight,FromDC,0,0,SRCCOPY);
    SelectObject(FromDC,o_ii.hbmColor);
    SelectObject(ToDC,n_ii.hbmColor);
    FillRect(ToDC,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));
    BitBlt(ToDC,xo,yo,bi.bmWidth,bi.bmHeight,FromDC,0,0,SRCCOPY);
    SelectObject(FromDC,FromDefBmp);
    SelectObject(ToDC,ToDefBmp);

    DeleteObject(o_ii.hbmMask);
    DeleteObject(o_ii.hbmColor);

    HICON i=CreateIconIndirect(&n_ii);
    ImageList_AddIcon(il,i);
    DestroyIcon(i);
  }

  DeleteDC(FromDC);
  DeleteDC(ToDC);
  DeleteObject(n_ii.hbmColor);
  DeleteObject(n_ii.hbmMask);
}
//---------------------------------------------------------------------------
void ImageList_AddPaddedIcons(HIMAGELIST il,int Align,int nIco,...)
{
  int *p_nIco=&nIco;
  while (*p_nIco){
    ImageList_AddPaddedIcons(il,Align,
            (HICON)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(*(p_nIco++)),IMAGE_ICON,0,0,0),
            NULL);
  }
}
//---------------------------------------------------------------------------
void ImageList_AddPaddedIcons(HIMAGELIST il,int Align,char *sIco,...)
{
  char **p_sIco=&sIco;
  while (*p_sIco){
    ImageList_AddPaddedIcons(il,Align,
                (HICON)LoadImage(GetModuleHandle(NULL),*(p_sIco++),IMAGE_ICON,0,0,0),
                NULL);
  }
}
//---------------------------------------------------------------------------
int LVGetSelItem(HWND LV)
{
  int c=SendMessage(LV,LVM_GETITEMCOUNT,0,0);
  for (int i=0;i<c;i++){
    if (SendMessage(LV,LVM_GETITEMSTATE,i,LVIS_SELECTED)) return i;
  }
  return -1;
}
//---------------------------------------------------------------------------
#ifdef EASYSTR_H
EasyStr LVGetItemText(HWND LV,int i)
{
  EasyStr Ret;
  Ret.SetLength(5000);

  LV_ITEM lvi;
  lvi.mask=LVIF_TEXT;
  lvi.iItem=i;
  lvi.iSubItem=0;
  lvi.pszText=Ret.Text;
  lvi.cchTextMax=5000;
  SendMessage(LV,LVM_GETITEM,0,LPARAM(&lvi));
  return Ret;
}
#endif

#endif
//---------------------------------------------------------------------------
#ifdef EASYSTR_H
EasyStr GetWindowTextStr(HWND Win)
{
  EasyStr Text;
  int Len=GetWindowTextLength(Win)+1;
  Text.SetLength(Len);
  GetWindowText(Win,Text,Len);
  return Text;
}
//---------------------------------------------------------------------------
EasyStr LoadWholeFileIntoStr(char *File)
{
  EasyStr Ret;
  FILE *f=fopen(File,"rb");
  if (f==NULL) return "";

  int len=GetFileLength(f);
  Ret.SetLength(len);
  fread(Ret.Text,1,len,f);

  fclose(f);
  return Ret;
}
//---------------------------------------------------------------------------
bool SaveStrAsFile(EasyStr &s,char *File)
{
  FILE *f=fopen(File,"wb");
  if (f==NULL) return 0;
  fwrite(s.Text,1,s.Length(),f);
  fclose(f);
  return true;
}
#endif
//---------------------------------------------------------------------------
#if !defined(_VC_BUILD) && !defined(_MINGW_BUILD)
void _RTLENTRY __int__(int);
#endif

MODIFIERSTATESTRUCT GetLRModifierStates()
{
  MODIFIERSTATESTRUCT mss;

#if !defined(_VC_BUILD)
  OSVERSIONINFO osvi;
  osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
  GetVersionEx(&osvi);
  if (osvi.dwPlatformId!=VER_PLATFORM_WIN32_NT){
    BYTE ShiftFlags,CtrlAltFlags=0;

    ShiftFlags=BYTE(int(GetKeyState(VK_LSHIFT)<0 ? 2:0) | int(GetKeyState(VK_RSHIFT)<0 ? 1:0));
#if !defined(_MINGW_BUILD)
  /*
      _AH=0x12; __int__(0x16); // Extended Get Keyboard Status  (AT+)
      ShiftFlags=_AL;CtrlAltFlags=_AH;
  */
    if (_no_ints==0){
      _AH=0x2; __int__(0x16);
      ShiftFlags=_AL;
    }
#elif defined(_MINGW_INTS)
    if (_no_ints==0) ShiftFlags=(BYTE)int_16_2();
#endif

    if (GetKeyState(VK_CONTROL)<0) CtrlAltFlags|=(1 << 0) | (1 << 2);
    if (GetKeyState(VK_MENU)<0) CtrlAltFlags|=(1 << 1) | (1 << 3);

    mss.LShift=bool(ShiftFlags & (1 << 1));
    mss.RShift=bool(ShiftFlags & (1 << 0));
    mss.LCtrl=bool(CtrlAltFlags & (1 << 0));
    mss.RCtrl=bool(CtrlAltFlags & (1 << 2));
    mss.LAlt=bool(CtrlAltFlags & (1 << 1));
    mss.RAlt=bool(CtrlAltFlags & (1 << 3));
    return mss;
  }
#endif

  mss.LShift=(GetKeyState(VK_LSHIFT)<0);
  mss.RShift=(GetKeyState(VK_RSHIFT)<0);
  mss.LCtrl=(GetKeyState(VK_LCONTROL)<0);
  mss.RCtrl=(GetKeyState(VK_RCONTROL)<0);
  mss.LAlt=(GetKeyState(VK_LMENU)<0);
  mss.RAlt=(GetKeyState(VK_RMENU)<0);
  return mss;
}
//---------------------------------------------------------------------------
void DrawLine(HDC dc,int x1,int y1,int x2,int y2)
{
  MoveToEx(dc,x1,y1,0);
  LineTo(dc,x2,y2);
}
//---------------------------------------------------------------------------
#ifdef EASYSTR_H
EasyStr ShortenPath(EasyStr Path,HFONT Font,int MaxWidth)
{
  HDC ScrDC=GetDC(NULL);
  HDC DC=CreateCompatibleDC(ScrDC);
  HBITMAP Bmp=CreateCompatibleBitmap(ScrDC,MaxWidth,30);
  ReleaseDC(NULL,ScrDC);

  SelectObject(DC,Bmp);
  SelectObject(DC,Font);

  RECT rc={0,0,MaxWidth,30};
  DrawText(DC,Path,-1,&rc,DT_PATH_ELLIPSIS | DT_LEFT | DT_MODIFYSTRING | DT_NOPREFIX | DT_SINGLELINE);

  DeleteDC(DC);
  DeleteObject(Bmp);

  return Path;
}
#endif
//---------------------------------------------------------------------------
HDC CreateScreenCompatibleDC()
{
  HDC ScrDC=GetDC(NULL);
  HDC NewDC=CreateCompatibleDC(ScrDC);
  ReleaseDC(NULL,ScrDC);
  return NewDC;
}
//---------------------------------------------------------------------------
HBITMAP CreateScreenCompatibleBitmap(int w,int h)
{
  HDC ScrDC=GetDC(NULL);
  HBITMAP NewBMP=CreateCompatibleBitmap(ScrDC,w,h);
  ReleaseDC(NULL,ScrDC);
  return NewBMP;
}
//---------------------------------------------------------------------------
#else

#include "notwin_mymisc.cpp"

#endif

#ifdef UNIX
#include "x/x_mymisc.cpp"
#endif

#ifdef BEOS
#include "beos/be_mymisc.cpp"
#endif

#endif


