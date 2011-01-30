#include <sys/time.h>
//---------------------------------------------------------------------------
void unix_non_resizable_window(Display*XD,Window Handle)
{
  XSizeHints *pHints=XAllocSizeHints();
  if (pHints){
    pHints->flags=PMinSize | PMaxSize;

    XWindowAttributes wa;
    XGetWindowAttributes(XD,Handle,&wa);

    pHints->min_width=wa.width;
    pHints->min_height=wa.height;
    pHints->max_width=wa.width;
    pHints->max_height=wa.height;
    XSetWMNormalHints(XD,Handle,pHints);
    XFree(pHints);
  }
}

bool IsFocussed(Display*disp,Window win)
{
  Window Foc=0;
  int RevertTo;
  XGetInputFocus(disp,&Foc,&RevertTo);
  return (Foc==win);
}


void SetWindowHints(Display *XD,Window Win,bool Input,int State,
                    Pixmap IconPixmap,Pixmap IconMaskPixmap,XID Group,
                    bool Urgent)
{
  XWMHints *hints=XAllocWMHints();
  if (hints){
    hints->flags=InputHint | StateHint | IconMaskHint |
                    WindowGroupHint | int(Urgent ? XUrgencyHint:0);
    if(IconPixmap){
      hints->flags|=IconPixmapHint;
    }
    hints->input=Input;
    hints->initial_state=State;
    hints->icon_pixmap=IconPixmap;
    hints->icon_mask=IconMaskPixmap;
    hints->window_group=Group;
    XSetWMHints(XD,Win,hints);
    XFree(hints);
  }
}

void SetWindowNormalSize(Display *XD,Window Win,int min_w,int min_h,
													int max_w,int max_h,int w_inc,int h_inc,int grav)
{
  XSizeHints *pHints=XAllocSizeHints();
  if (pHints){
    pHints->flags=PMinSize | PMaxSize | PResizeInc | PWinGravity;
    pHints->min_width=min_w;
    pHints->min_height=min_h;
    pHints->max_width=int(max_w ? max_w:2000);
    pHints->max_height=int(max_h ? max_h:2000);
    pHints->width_inc=w_inc;
    pHints->height_inc=h_inc;
    pHints->win_gravity=grav;

    XSetWMSizeHints(XD,Win,pHints,XA_WM_NORMAL_HINTS);
    XFree(pHints);
  }
}

void SetWindowGravity(Display *XD,Window Win,int Grav)
{
  XSetWindowAttributes swa;
  swa.bit_gravity=StaticGravity;
  swa.win_gravity=Grav;
  XChangeWindowAttributes(XD,Win,CWBitGravity | CWWinGravity,&swa);
}

// This takes just a file name, not the folder it is in/drive it is on!
char *RemoveIllegalFromName(char *Name,bool RemoveWild,char ReplaceChar)
{
  int Len=strlen(Name);
  for (int i=0;i<Len;i++){
    switch (Name[i]){
      case '/':
        Name[i]=ReplaceChar;
        break;
      case '*':case '?':
        if (RemoveWild) Name[i]=ReplaceChar;
        break;
    }
  }
  return Name;
}

char *RemoveIllegalFromPath(char *Path,bool DriveIncluded,bool RemoveWild,char ReplaceChar,bool STPath)
{
  char *Name,*PathStart=Path,*PathEnd=Path+strlen(Path)-1,*FilNam;
  bool GotSlash;

  char Slash=char(STPath ? '\\':'/');

  Name=PathEnd;
  do{
    GotSlash=0;
    while (Name>=PathStart){
      if (*Name==Slash){
        GotSlash=true;
        break;
      }
      Name--;
    }
    FilNam=LPSTR(GotSlash ? Name+1:PathStart);
    while (FilNam<=PathEnd){
      char c=*FilNam;
      if (c==Slash){
        break;
      }else{
        switch (c){
          case '/':
            if (STPath) *FilNam=ReplaceChar;
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
char *GetFileNameFromPath(char *fil)
{
  char *Slash=strrchr(fil,'/');
  if (Slash) return Slash+1;
  return fil;
}
//---------------------------------------------------------------------------
bool MoveFile(char *From,char *To)
{
  return rename(From,To)!=-1;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
DWORD timeGetTime()
{
  struct timeval tv;
  struct timezone tz;       //ignored
  static unsigned long old_tv_sec=0;
  static unsigned long time_base=0;
  gettimeofday(&tv,&tz);
  tv.tv_sec&=((1<<20)-1);
  if((unsigned long)tv.tv_sec<old_tv_sec){ //looped round
    time_base+=old_tv_sec; //1<<20)*1000);
  }
  old_tv_sec=tv.tv_sec;
  return (((tv.tv_usec)/1000)    // fractional part in milliseconds
          +((tv.tv_sec)*1000)    // seconds, mod 2^20
          +time_base );          // to avoid looping at bad values, less
                                 // than 2147483648*2-1
}
//---------------------------------------------------------------------------
void Sleep(DWORD milli)
{
	struct timeval TimeOut;
	TimeOut.tv_sec=milli/1000;
	TimeOut.tv_usec=(milli % 1000)*1000;
	select(0,NULL,NULL,NULL,&TimeOut);
}
//---------------------------------------------------------------------------
#define FILE_ATTRIBUTE_ARCHIVE    0
#define FILE_ATTRIBUTE_HIDDEN     0
#define FILE_ATTRIBUTE_NORMAL     0
#define FILE_ATTRIBUTE_OFFLINE    0
#define FILE_ATTRIBUTE_READONLY   1
#define FILE_ATTRIBUTE_SYSTEM     0
#define FILE_ATTRIBUTE_TEMPORARY  0
#define FILE_ATTRIBUTE_COMPRESSED 0
#define FILE_ATTRIBUTE_DIRECTORY  2

/*
`S_IRUSR' Read permission bit for the owner of the file.
`S_IWUSR' Write permission bit for the owner of the file.
`S_IXUSR' Execute (for ordinary files) or search (for directories)
          permission bit for the owner of the file.
`S_IRWXU' (S_IRUSR | S_IWUSR | S_IXUSR)

`S_IRGRP' Read permission bit for the group owner of the file.
`S_IWGRP' Write permission bit for the group owner of the file.
`S_IXGRP' Execute or search permission bit for the group owner of the file.
`S_IRWXG' (S_IRGRP | S_IWGRP | S_IXGRP)

`S_IROTH' Read permission bit for other users.
`S_IWOTH' Write permission bit for other users.
`S_IXOTH' Execute or search permission bit for other users.
`S_IRWXO' (S_IROTH | S_IWOTH | S_IXOTH)

`S_ISUID' Set-user-ID on execute bit
`S_ISGID' Set-group-ID on execute bit

`S_ISVTX' "sticky" bit
*/


bool SetFileAttributes(char *File,DWORD Attrib)
{
  struct stat s;
  if (stat(File,&s)==-1) return 0;

  if (S_ISDIR(s.st_mode)){
    if ((Attrib & FILE_ATTRIBUTE_DIRECTORY)==0) return 0;
  }else if (S_ISREG(s.st_mode)){
    if (Attrib & FILE_ATTRIBUTE_DIRECTORY) return 0;
  }
  s.st_mode&=S_ISUID | S_ISGID | S_ISVTX | S_IRUSR | S_IWUSR | S_IXUSR |
              S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH;
  if (Attrib & FILE_ATTRIBUTE_READONLY){
    s.st_mode&=~S_IWUSR;
  }else{
    s.st_mode|=S_IWUSR;
  }
  return chmod(File,s.st_mode)==0;
}
//---------------------------------------------------------------------------
DWORD GetFileAttributes(char *File)
{
  DWORD Attrib=0;
  struct stat s;
  if (stat(File,&s)==-1) return 0xffffffff;
  if (S_ISDIR(s.st_mode)) Attrib|=FILE_ATTRIBUTE_DIRECTORY;

  uid_t cur_uid=geteuid();
  gid_t cur_gid=getegid();
  if (cur_uid==s.st_uid){
    if ((s.st_mode & S_IWUSR)==0) Attrib|=FILE_ATTRIBUTE_READONLY;
  }else if (cur_gid==s.st_gid){
    if ((s.st_mode & S_IWGRP)==0) Attrib|=FILE_ATTRIBUTE_READONLY;
  }else{
    if ((s.st_mode & S_IWOTH)==0) Attrib|=FILE_ATTRIBUTE_READONLY;
  }
  return Attrib;
}
//---------------------------------------------------------------------------
bool CreateDirectory(char *Path,void*)
{
  return mkdir(Path,S_IRWXU | S_IRWXG | S_IRWXO)==0;
}
//---------------------------------------------------------------------------
bool RemoveDirectory(char *Path)
{
  return rmdir(Path)==0;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool matches_mask(char*_t,char*_mask)
{
  EasyStr t=_t;strupr(t.Text);
  EasyStr mask=_mask;strupr(mask.Text);
  char*mp=mask.Text;
  char*tp=t.Text;
  for(;;){
    if(*mp=='*'){
      while((*mp)=='*' || (*mp)=='?'){
        if((*mp)=='?'){
          if(!(*tp))return false;
          tp++;
        }
        mp++;
      }
      if(!(*mp))return true;  //Doesn't want us to match anything from here.
      EasyStr to_match;
      while((*mp)!='?' && (*mp)!='*' && (*mp)){
        to_match+=*mp;
        mp++;
      }
      for(;;){
        char*tmt=to_match.Text;
        char*tp2=tp;
        while((*tmt)==(*tp2) && (*tmt)){
          tp2++;
          tmt++;
        }
        if(!(*tmt)){  //we hit the end of the string to match without encoutering a character that didn't match
          tp=tp2;
//          if(!(*mp))return true; //matched everything
          break;
        }else{ //didn't match
          tp++; //look at next character
          if(!(*tp))return false; //ran out of text looking for a match
        }
      }
    }else if((*mp)=='?'){
      if(!(*tp))return false; //no character there!
      mp++;
      tp++;
    }else{
      if((*mp) != (*tp)){
        return false; //doesn't match
      }else{
        mp++;
        tp++;
      }
    }
    if(!(*mp)){
      if(!(*tp))return true; //all characters match
      else return false; //didn't match
    }
  }
}

bool filename_matches_mask(char*_t,char*_mask)
{
  EasyStr mask=_mask,t=_t;
  char *dotstar=strstr(mask,".*");
  char *dot_p=strchr(t,'.');
  if (dotstar){ //ignore all extensions
    *dotstar=0;
    if (dot_p) *dot_p=0;
  }
  if (strchr(mask,'.')){ //there's a . in the mask
    return matches_mask(t,mask);
  }else if (strchr(t,'.')){
    return false;  //file has an extension - that's bad!
  }else{
    return matches_mask(t,mask);  //no extensions anywhere
  }
}



//extern void log_write(EasyStr);

EasyStr find_file_i(EasyStr path,EasyStr f) //search for file f in path
{
  NO_SLASH(path);
  //log_write(EasyStr("find_file_i(\"")+path+"\",\""+f+"\")");
  int sp;
  for(;;){
    for (sp=0;f[sp];sp++) if (f[sp]=='/')break;
    if (f[sp]){  //found slash
      EasyStr f2=f;
      f2[sp]=0;  //knock off slash
      path=find_file_i(path,f2);
      f=f.Text+(sp+1);
    }else{ //no slashes, search for filename and return
      DIR *dp;
      struct dirent *ep;
      dp=opendir(path+"/");
      if (dp){
        for(;;){
          ep=readdir(dp);
          if (ep==NULL) break;
          if (strcmpi(ep->d_name,f)==0){ //filenames match apart for case
            EasyStr ret=path+"/"+(ep->d_name);
            closedir(dp); // ep might be cleared here, so we store above
            //log_write(EasyStr("find_file_i found ")+ret);
            return ret;
          }
        }
        closedir(dp);
      }
      return (path+"/"+f);
    }
  }
}
//---------------------------------------------------------------------------
bool SetProp(Display *XD,Window Win,XContext Prop,DWORD Val)
{
  return XSaveContext(XD,Win,Prop,(XPointer)Val)==0;
}
//---------------------------------------------------------------
DWORD GetProp(Display *XD,Window Win,XContext Prop)
{
  XPointer Dat=0;
  if(XFindContext(XD,Win,Prop,&Dat))Dat=0;
  return (DWORD)Dat;
}
//---------------------------------------------------------------------------
DWORD RemoveProp(Display *XD,Window Win,XContext Prop)
{
  return XDeleteContext(XD,Win,Prop);
}
//---------------------------------------------------------------------------
DWORD GetColourValue(Display *XD,WORD R,WORD G,WORD B,DWORD Default)
{
  if (XD==NULL) return 0;

  XColor xc={Default,R,G,B};
  XAllocColor(XD,XDefaultColormap(XD,XDefaultScreen(XD)),&xc);
  return xc.pixel;
}
//---------------------------------------------------------------------------
void ShowHideWindow(Display *XD,Window Win,bool Show)
{
  if (Show){
    XMapWindow(XD,Win);
  }else{
    XUnmapWindow(XD,Win);
  }
}
//---------------------------------------------------------------------------
bool copy_file_byte_by_byte(char *From,char *To)
{
  bool succeed=0;
  FILE *in=fopen(From,"rb");
  if (in){
    FILE *out=fopen(To,"wb");
    if (out){
      BYTE buf[32000];
      int n;
      do{
        n=fread(buf,1,32000,in);
        if (n>0) fwrite(buf,1,n,out);
      }while (n==32000);
      succeed=true;
      fclose(out);
    }
    fclose(in);
  }
  return succeed;
}
//---------------------------------------------------------------------------

