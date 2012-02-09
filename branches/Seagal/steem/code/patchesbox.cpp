/*---------------------------------------------------------------------------
FILE: patchesbox.cpp
MODULE: Steem
DESCRIPTION: The code for Steem's patches dialog that allows the user to
apply patches to fix ST programs that don't work or are incompatible with
Steem.
---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
void TPatchesBox::RefreshPatchList()
{
  if (Handle==WINDOWTYPE(0)) return;

  EasyStr ThisVerText=stem_version_text;
  for (int n=0;n<ThisVerText.Length();n++){ // Cut off beta number
    if (ThisVerText[n]<'0' || ThisVerText[n]>'9'){
      if (ThisVerText[n]!='.'){
        ThisVerText[n]=0;
        break;
      }
    }
  }
  double ThisVer=atof(ThisVerText);
#ifdef WIN32
  EasyStringList sl;
  SendDlgItemMessage(Handle,100,LB_RESETCONTENT,0,0);
#elif defined(UNIX)
  EasyStringList &sl=PatchList.sl;
  sl.DeleteAll();
#endif
  sl.Sort=eslSortByNameI;

  DirSearch ds;
  if (ds.Find(PatchDir+SLASH "*.stp")){
    do{
      if (ThisVer<atof(GetCSFStr("Text","Obsolete","9999",PatchDir+SLASH+ds.Name))){
        *strrchr(ds.Name,'.')=0;
        if (ds.Name[0]) sl.Add(ds.Name);
      }
    }while (ds.Next());
  }
  if (sl.NumStrings){
    int iSel=-1;
    for (int s=0;s<sl.NumStrings;s++){
      WIN_ONLY( SendDlgItemMessage(Handle,100,LB_ADDSTRING,0,LPARAM(sl[s].String)); )
      if (IsSameStr_I(sl[s].String,SelPatch)) iSel=s;
    }
    if (iSel==-1){
      iSel=0;
      SelPatch=sl[0].String;
    }
#ifdef WIN32
    EnableWindow(GetDlgItem(Handle,100),TRUE);
    SendDlgItemMessage(Handle,100,LB_SETCURSEL,iSel,0);
#elif defined(UNIX)
		PatchList.changesel(iSel);
		PatchList.draw(true,true);
#endif
  }else{
    WIN_ONLY( EnableWindow(GetDlgItem(Handle,100),0); )
    SelPatch="";
  }
  ShowPatchFile();

#ifdef WIN32
  WriteCSFStr(Section,"LastKnownVersion",GetPatchVersion(),INIFile);
  SetButtonIcon();
#endif
}
//---------------------------------------------------------------------------
void TPatchesBox::PatchPoke(MEM_ADDRESS &ad,int Len,DWORD Data)
{
  ad&=0xffffff;
  if (ad<himem){
    switch (Len){
      case 1: PEEK(ad)=BYTE(Data);   break;
      case 2: DPEEK(ad)=WORD(Data);  break;
      case 4: LPEEK(ad)=DWORD(Data); break;
    }
  }else if (ad>=MEM_IO_BASE){
    TRY_M68K_EXCEPTION
      switch (Len){
        case 1: io_write_b(ad,BYTE(Data));  break;
        case 2: io_write_w(ad,WORD(Data));  break;
        case 4: io_write_l(ad,DWORD(Data)); break;
      }
    CATCH_M68K_EXCEPTION
    END_M68K_EXCEPTION
  }
  ad+=Len;
}
//---------------------------------------------------------------------------
void TPatchesBox::ApplyPatch()
{
	if (SelPatch.Empty()) return;

  EasyStr pf=PatchDir+SLASH+SelPatch+".stp";
  FILE *f=fopen(pf,"rb");
  if (f){
    EasyStr Text;
    int Len=GetFileLength(f);
    Text.SetLength(Len);
    fread(Text.Text,1,Len,f);
    fclose(f);

    strupr(Text);

    int NumBytesChanged=0;
    DynamicArray<BYTE> Bytes;
    EasyStringList Offsets;
    Offsets.Sort=eslNoSort;

    char *Sect=strstr(Text,"\n[PATCH]");

    bool WordOnly;
    char *OffsetSect=strstr(Text,"\n[OFFSETS]");
    if (OffsetSect){
      OffsetSect+=2; // skip \n[
      char *OffsetSectEnd=strstr(OffsetSect,"\n[");
      if (OffsetSectEnd==NULL) OffsetSectEnd=Text.Right()+1; // point to NULL
      char *tp=OffsetSect;
      while (tp<OffsetSectEnd){
        if (*tp==13 || *tp==10) *tp=0;
        tp++;
      }
      tp=OffsetSect+strlen(OffsetSect)+1;
      while (tp<OffsetSectEnd){
        char *next_line=tp+strlen(tp)+1;
        char *eq=strchr(tp,'=');
        if (eq){
          *eq=0;eq++;
          // Offset name = tp
          WordOnly=0;
          acc_parse_search_string(eq,Bytes,WordOnly);
          MEM_ADDRESS offset_ad=acc_find_bytes(Bytes,WordOnly,0,1);
          if (offset_ad<=0xffffff){
            while (tp[0]==' ') tp++;
            while (*(tp+strlen(tp)-1)==' ') *(tp+strlen(tp)-1)=0;
            Offsets.Add(tp,(long)offset_ad);
          }
        }
        tp=next_line;
      }
    }

    bool ReturnLengths;
    if (Sect){
      Sect+=2; // skip \n[
      char *SectEnd=strstr(Sect,"\n[");
      if (SectEnd==NULL) SectEnd=Sect+strlen(Sect); // point to NULL

      char *tp=Sect;
      while (tp<SectEnd){
        if (*tp==13 || *tp==10) *tp=0;
        tp++;
      }
      tp=Sect+strlen(Sect)+1;
      while (tp<SectEnd){
        char *next_line=tp+strlen(tp)+1;
        char *eq=strchr(tp,'=');
        if (eq){
          *eq=0;eq++;

          // tp can = Off+$x= or Off= or $x=.
          MEM_ADDRESS offset_ad=0xffffffff;
          int dir=-1;
          char *sym=strchr(tp,'-');
          if (sym==NULL) sym=strchr(tp,'+'), dir=1;
          if (sym) *sym=0;

          // sym points to + or -, dir is 1 for + and -1 for -. If sym==null tp is either Off= or $x=.
          while (tp[0]==' ') tp++;
          while (*(tp+strlen(tp)-1)==' ') *(tp+strlen(tp)-1)=0;

          for (int i=0;i<Offsets.NumStrings;i++){
            if (IsSameStr(Offsets[i].String,tp)){
              offset_ad=(MEM_ADDRESS)Offsets[i].Data[0];
              if (sym==NULL) dir=0; // no offset
              break;
            }
          }
          // if offset_ad is 0xffffffff then tp hasn't been found. When sym is set
          // this should cause this part of the patch to be skipped. If sym isn't set then
          // we assume tp is an absolute address.
          if (sym){
            tp=sym+1;
          }else if (offset_ad==0xffffffff && dir){
            offset_ad=0; // not found so treat tp as an absolute address
          }
          if (offset_ad<=0xffffff){
            MEM_ADDRESS ad=offset_ad + HexToVal(tp)*dir;  // dir=0 if there is no offset

            ReturnLengths=true;
            acc_parse_search_string(eq,Bytes,ReturnLengths);
            // Bytes is now a list of bytes in big endian format, between each byte is a length byte
            int i=0;
            while (i<Bytes.NumItems){
              NumBytesChanged+=Bytes[i+1];
              if (Bytes[i+1]==1){
                PatchPoke(ad,1,Bytes[i]);
                i+=2;
              }else if (Bytes[i+1]==2){
                PatchPoke(ad,2,MAKEWORD(Bytes[i+2],Bytes[i]));
                i+=4;
              }else if (Bytes[i+1]==4){
                PatchPoke(ad,4,MAKELONG(MAKEWORD(Bytes[i+6],Bytes[i+4]),MAKEWORD(Bytes[i+2],Bytes[i])));
                i+=8;
              }
            }
          }
        }
        tp=next_line;
      }
    }
    if (NumBytesChanged){
      Alert(T("Patch successfully applied, number of bytes changed: ")+NumBytesChanged,T("Successful Patch"),MB_ICONINFORMATION | MB_OK);
    }else{
      Alert(T("This patch file doesn't appear to do anything!"),T("Patch Error"),MB_ICONEXCLAMATION | MB_OK);
    }
  }else{
    Alert(T("Couldn't open the patch file!"),T("Patch Error"),MB_ICONEXCLAMATION | MB_OK);
  }
}
//---------------------------------------------------------------------------
void TPatchesBox::GetPatchText(char *File,Str Text[4])
{
  ConfigStoreFile CSF(File);
  char *Name[3]={"Description","ApplyWhen","Version"};
  Str NewSect=T("Patch Text Section=");
  if (NewSect=="Patch Text Section=") NewSect="";
  char *Sect[2]={NewSect,"Text"};
  for (int s=0;s<2;s++){
    if (Sect[s][0]==0) s++;
    for (int n=0;n<3;n++){
      if (Text[n].Empty()) Text[n]=CSF.GetStr(Sect[s],Name[n],"");
    }
  }
  Text[3]=CSF.GetStr("Text","PatchAuthor","");
  if (NewSect.NotEmpty()){
    Str TransBy=CSF.GetStr(NewSect,"PatchAuthor","");
    if (TransBy.NotEmpty()) Text[3]+=Str(WIN_ONLY("\r") "\n")+TransBy;
  }
  CSF.Close();
}
//---------------------------------------------------------------------------
#ifdef WIN32
//---------------------------------------------------------------------------
TPatchesBox::TPatchesBox()
{
  Left=(GetSystemMetrics(SM_CXSCREEN)-456)/2;
  Top=(GetSystemMetrics(SM_CYSCREEN)-(411+GetSystemMetrics(SM_CYCAPTION)))/2;

  FSLeft=(640-456)/2;
  FSTop=(480-(411+GetSystemMetrics(SM_CYCAPTION)))/2;

  Section="Patches";
}
//---------------------------------------------------------------------------
void TPatchesBox::ManageWindowClasses(bool Unreg)
{
  char *ClassName="Steem Patches";
  if (Unreg){
    UnregisterClass(ClassName,Inst);
  }else{
    RegisterMainClass(WndProc,ClassName,RC_ICO_PATCHES);
  }
}
//---------------------------------------------------------------------------
EasyStr TPatchesBox::GetPatchVersion()
{
  DWORD Attrib=GetFileAttributes(PatchDir);
  if (Attrib<0xffffffff && (Attrib & FILE_ATTRIBUTE_DIRECTORY)){
    FILE *f=fopen(PatchDir+SLASH+"version","rb");
    if (f){
      char Text[100];ZeroMemory(Text,100);
      fread(Text,1,100,f);
      fclose(f);
      return EasyStr(Text);
    }
  }
  return "";
}
//---------------------------------------------------------------------------
void TPatchesBox::SetButtonIcon()
{
  if (StemWin==NULL) return;

  Str LastVerSeen=GetCSFStr("Patches","LastKnownVersion","",INIFile);
  if (LastVerSeen.NotEmpty()){
    if ( NotSameStr_I(GetPatchVersion(),LastVerSeen) ){
      SendDlgItemMessage(StemWin,113,WM_SETTEXT,0,LPARAM(Str(RC_ICO_PATCHESNEW).Text));
      return;
    }
  }
  SendDlgItemMessage(StemWin,113,WM_SETTEXT,0,LPARAM(Str(RC_ICO_PATCHES).Text));
}
//---------------------------------------------------------------------------
void TPatchesBox::Show()
{
  if (Handle!=NULL){
    ShowWindow(Handle,SW_SHOWNORMAL);
    SetForegroundWindow(Handle);
    return;
  }
  if (FullScreen) Top=max(Top,MENUHEIGHT);

  ManageWindowClasses(SD_REGISTER);
  Handle=CreateWindowEx(WS_EX_CONTROLPARENT,"Steem Patches",T("Patches"),
                          WS_CAPTION | WS_SYSMENU,Left,Top,456,411+GetSystemMetrics(SM_CYCAPTION),
                          ParentWin,NULL,Inst,NULL);
  if (HandleIsInvalid()){
    ManageWindowClasses(SD_UNREGISTER);
    return;
  }

  SetWindowLong(Handle,GWL_USERDATA,(long)this);

  MakeParent(HWND(FullScreen ? StemWin:NULL));

  CreateWindow("Static",T("Available Patches"),WS_VISIBLE | WS_CHILD,
                  10,10,200,20,Handle,(HMENU)99,HInstance,NULL);

  CreateWindowEx(512,"Listbox","",WS_VISIBLE | WS_CHILD | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY,
                  10,30,180,323,Handle,(HMENU)100,HInstance,NULL);

  CreateWindow("Static",T("Description"),WS_VISIBLE | WS_CHILD,
                  200,10,240,20,Handle,(HMENU)199,HInstance,NULL);

  HWND Win=CreateWindowEx(512,"Edit","",WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
                  200,30,240,80,Handle,(HMENU)200,HInstance,NULL);
  MakeEditNoCaret(Win);

  CreateWindow("Static",T("Apply When"),WS_VISIBLE | WS_CHILD,
                  200,120,240,20,Handle,(HMENU)209,HInstance,NULL);

  Win=CreateWindowEx(512,"Edit","",WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
                  200,140,240,40,Handle,(HMENU)210,HInstance,NULL);
  MakeEditNoCaret(Win);

  CreateWindow("Static",T("Version"),WS_VISIBLE | WS_CHILD,
                  200,190,240,20,Handle,(HMENU)219,HInstance,NULL);

  Win=CreateWindowEx(512,"Edit","",WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
                  200,210,240,40,Handle,(HMENU)220,HInstance,NULL);
  MakeEditNoCaret(Win);

  CreateWindow("Static",T("Patch Author(s)"),WS_VISIBLE | WS_CHILD,
                  200,260,240,20,Handle,(HMENU)229,HInstance,NULL);

  Win=CreateWindowEx(512,"Edit","",WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
                  200,280,240,40,Handle,(HMENU)230,HInstance,NULL);
  MakeEditNoCaret(Win);

  CreateWindow("Button",T("Apply Now"),WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                  200,330,240,23,Handle,(HMENU)300,HInstance,NULL);


  CreateWindow("Static","",WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
                 1,360,450,2,Handle,(HMENU)399,HInstance,NULL);

  int Wid=GetTextSize(Font,T("Patch folder")).Width;
  CreateWindow("Static",T("Patch folder"),WS_VISIBLE | WS_CHILD,
                  10,375,Wid,23,Handle,(HMENU)400,HInstance,NULL);

  CreateWindowEx(512,"Steem Path Display",PatchDir,WS_CHILD | WS_VISIBLE,
                  15+Wid,370,340-(15+Wid),25,Handle,(HMENU)401,HInstance,NULL);

  CreateWindow("Button",T("Choose"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                          350,371,90,23,Handle,(HMENU)402,HInstance,NULL);


  SetWindowAndChildrensFont(Handle,Font);

  RefreshPatchList();

  Focus=GetDlgItem(Handle,100);
  ShowWindow(Handle,SW_SHOW);
  SetFocus(Focus);

  if (StemWin!=NULL) PostMessage(StemWin,WM_USER,1234,0);
}
//---------------------------------------------------------------------------
void TPatchesBox::ShowPatchFile()
{
	EnableWindow(GetDlgItem(Handle,200),SelPatch.NotEmpty());
	EnableWindow(GetDlgItem(Handle,210),SelPatch.NotEmpty());
	EnableWindow(GetDlgItem(Handle,220),SelPatch.NotEmpty());
	EnableWindow(GetDlgItem(Handle,230),SelPatch.NotEmpty());
	EnableWindow(GetDlgItem(Handle,300),SelPatch.NotEmpty());
	if (SelPatch.NotEmpty()){
    Str Text[4];
    GetPatchText(PatchDir+SLASH+SelPatch+".stp",Text);

	  SendDlgItemMessage(Handle,200,WM_SETTEXT,0,LPARAM(Text[0].Text));
	  SendDlgItemMessage(Handle,210,WM_SETTEXT,0,LPARAM(Text[1].Text));
	  SendDlgItemMessage(Handle,220,WM_SETTEXT,0,LPARAM(Text[2].Text));
	  SendDlgItemMessage(Handle,230,WM_SETTEXT,0,LPARAM(Text[3].Text));
	}
}
//---------------------------------------------------------------------------
void TPatchesBox::Hide()
{
  if (Handle==NULL) return;

  ShowWindow(Handle,SW_HIDE);
  if (FullScreen) SetFocus(StemWin);

  DestroyWindow(Handle);Handle=NULL;

  if (StemWin) PostMessage(StemWin,WM_USER,1234,0);
  ManageWindowClasses(SD_UNREGISTER);
}
//---------------------------------------------------------------------------
#define GET_THIS This=(TPatchesBox*)GetWindowLong(Win,GWL_USERDATA);

LRESULT __stdcall TPatchesBox::WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  LRESULT Ret=DefStemDialogProc(Win,Mess,wPar,lPar);
  if (StemDialog_RetDefVal) return Ret;

  TPatchesBox *This;
  switch (Mess){
    case WM_COMMAND:
      GET_THIS;
      switch (LOWORD(wPar)){
        case 100:
          if (HIWORD(wPar)==LBN_SELCHANGE){
            EasyStr NewSel;
            NewSel.SetLength(MAX_PATH);
            SendMessage(HWND(lPar),LB_GETTEXT,SendMessage(HWND(lPar),LB_GETCURSEL,0,0),LPARAM(NewSel.Text));
            if (NotSameStr_I(NewSel,This->SelPatch)){
              This->SelPatch=NewSel;
              This->ShowPatchFile();
            }
          }
          break;
        case 300:
          if (This->SelPatch.NotEmpty()) This->ApplyPatch();
          break;
        case 402:
          SendMessage(HWND(lPar),BM_SETCHECK,1,true);
          EnableAllWindows(0,Win);

          EasyStr NewFol=ChooseFolder(HWND(FullScreen ? StemWin:Win),T("Pick a Folder"),This->PatchDir);
          if (NewFol.NotEmpty()){
            NO_SLASH(NewFol);
            SendDlgItemMessage(Win,401,WM_SETTEXT,0,(long)(NewFol).Text);
            SendDlgItemMessage(Win,200,WM_SETTEXT,0,LPARAM(""));
            SendDlgItemMessage(Win,210,WM_SETTEXT,0,LPARAM(""));
            SendDlgItemMessage(Win,220,WM_SETTEXT,0,LPARAM(""));
            SendDlgItemMessage(Win,230,WM_SETTEXT,0,LPARAM(""));
            This->PatchDir=NewFol;
            This->RefreshPatchList();
          }

          SetForegroundWindow(Win);
          EnableAllWindows(true,Win);
          SetFocus(HWND(lPar));
          SendMessage(HWND(lPar),BM_SETCHECK,0,true);

          break;
      }
      break;
    case (WM_USER+1011):
    {
      GET_THIS;

      HWND NewParent=(HWND)lPar;
      if (NewParent){
        This->CheckFSPosition(NewParent);
        SetWindowPos(Win,NULL,This->FSLeft,This->FSTop,0,0,SWP_NOZORDER | SWP_NOSIZE);
      }else{
        SetWindowPos(Win,NULL,This->Left,This->Top,0,0,SWP_NOZORDER | SWP_NOSIZE);
      }
      This->ChangeParent(NewParent);

      break;
    }
    case WM_CLOSE:
      GET_THIS;
      This->Hide();
      return 0;
    case DM_GETDEFID:
      return 0;
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}
#undef GET_THIS

#elif defined(UNIX)
#include "x/x_patchesbox.cpp"
#endif

