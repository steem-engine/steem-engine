//---------------------------------------------------------------------------
void THardDiskManager::update_mount()
{
  for (int n=2;n<26;n++){
    if (IsMountedDrive(char(n+'A'))){
      mount_flag[n]=true;
      mount_path[n]=GetMountedDrivePath(char(n+'A'));
    }else{
      mount_flag[n]=false;
      mount_path[n]="";
    }
  }
  CheckResetDisplay();
}
//---------------------------------------------------------------------------
bool THardDiskManager::IsMountedDrive(char d)
{
  if (d>='C'){
    for (int n=0;n<nDrives;n++){
      if (d==Drive[n].Letter){
        return true;
      }
    }
  }
  return false;
}
//---------------------------------------------------------------------------
EasyStr THardDiskManager::GetMountedDrivePath(char d)
{
  if (d>='C'){
    for (int n=0;n<nDrives;n++){
      if (d==Drive[n].Letter){
        return Drive[n].Path;
      }
    }
  }
  return RunDir;
}
//---------------------------------------------------------------------------
bool THardDiskManager::NewDrive(char *Path)
{
  if (nDrives>=MAX_HARDDRIVES) return 0;

  int Idx=nDrives;
  bool Found=0;

  Drive[Idx].Path=Path;
  NO_SLASH(Drive[Idx].Path);

  for (int Let='C';Let<='Z' && Found==0;Let++){
    for (int i=0;i<nDrives;i++){
      if (Drive[i].Letter==Let){
        break;
      }else if (i==nDrives-1){
        Drive[Idx].Letter=(char)Let;
        Found=true;
      }
    }
  }
  nDrives++;
  return true;
}

#ifdef WIN32
//---------------------------------------------------------------------------
THardDiskManager::THardDiskManager()
{
  Left=GetSystemMetrics(SM_CXSCREEN)/2-258;
  Top=GetSystemMetrics(SM_CYSCREEN)/2-90+GetSystemMetrics(SM_CYCAPTION);

  FSLeft=320 - 258;
  FSTop=240 - 90+GetSystemMetrics(SM_CYCAPTION);

  Section="HardDrives";

  for (int i=0;i<MAX_HARDDRIVES;i++){
    Drive[i].Path="";
    Drive[i].Letter=(char)('C'+i);
  }
  nDrives=0;
  update_mount();

  ApplyChanges=0;

  Font=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
}
//---------------------------------------------------------------------------
void THardDiskManager::ManageWindowClasses(bool Unreg)
{
  char *ClassName="Steem Hard Disk Manager";
  if (Unreg){
    UnregisterClass(ClassName,Inst);
  }else{
    RegisterMainClass(WndProc,ClassName,RC_ICO_HARDDRIVE16);
  }
}
//---------------------------------------------------------------------------
void THardDiskManager::Show()
{
  if (Handle!=NULL){
    SetForegroundWindow(Handle);return;
  }else if (DiskMan.Handle==NULL){
    return;
  }
  HWND Win;

  EnableWindow(DiskMan.Handle,0);

  ManageWindowClasses(SD_REGISTER);
  Handle=CreateWindowEx(WS_EX_CONTROLPARENT,"Steem Hard Disk Manager",T("Hard Drives"),WS_CAPTION | WS_SYSMENU,
                        Left,Top,516,90+GetSystemMetrics(SM_CYCAPTION),
                        DiskMan.Handle,0,HInstance,NULL);
  if (HandleIsInvalid()){
    ManageWindowClasses(SD_UNREGISTER);
    return;
  }

  SetWindowLong(Handle,GWL_USERDATA,(long)this);

  if (FullScreen) MakeParent(StemWin);

  SendMessage( CreateWindow("Button",T("All &Off"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                             10,10,150,23,Handle,(HMENU)90,HInstance,NULL)
                    ,WM_SETFONT,(UINT)Font,0);

  SendMessage( CreateWindow("Button",T("&New Hard Drive"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                            300,10,200,23,Handle,(HMENU)10,HInstance,NULL)
                    ,WM_SETFONT,(UINT)Font,0);

  int Wid=get_text_width(T("When drive A is empty boot from"));
  SendMessage( CreateWindow("Static",T("When drive A is empty boot from"),WS_CHILD | WS_VISIBLE,
                            10,44,Wid,20,Handle,(HMENU)91,HInstance,NULL)
                    ,WM_SETFONT,(UINT)Font,0);

  Win=CreateWindow("Combobox","",WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
                    CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                    15+Wid,40,40,300,Handle,(HMENU)92,HInstance,NULL);
  SendMessage(Win,WM_SETFONT,(UINT)Font,0);
  char DriveName[8];
  DriveName[1]=':';DriveName[2]=0;
  SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Off"));
  for (int i=0;i<24;i++){
    DriveName[0]=(char)('C'+i);
    SendMessage(Win,CB_ADDSTRING,0,(long)DriveName);
  }
#ifndef DISABLE_STEMDOS
  SendMessage(Win,CB_SETCURSEL,stemdos_boot_drive-1,0);
#else
  SendMessage(Win,CB_SETCURSEL,0,0);
  EnableWindow(Win,0);
#endif


  Win=CreateWindow("Button",T("OK"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                    290,40,100,23,Handle,(HMENU)IDOK,HInstance,NULL);
  SendMessage(Win,WM_SETFONT,(UINT)Font,0);

  Win=CreateWindow("Button",T("Cancel"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                    400,40,100,23,Handle,(HMENU)IDCANCEL,HInstance,NULL);
  SendMessage(Win,WM_SETFONT,(UINT)Font,0);

  for (int i=0;i<nDrives;i++) CreateDriveControls(i);

  SetWindowHeight();

  nOldDrives=nDrives;
  if (nDrives){
    OldDrive=new Hard_Disk_Info[nOldDrives];
  }else{
    OldDrive=NULL;
  }
  for (int i=0;i<nDrives;i++) OldDrive[i]=Drive[i];

  ShowWindow(Handle,SW_SHOW);
  SetFocus(GetDlgItem(Handle,int(nDrives ? 100:IDOK)));
}
//---------------------------------------------------------------------------
void THardDiskManager::CreateDriveControls(int Idx)
{
  HWND Win;
  int y=10+(Idx*30);

  if (GetDlgItem(Handle,300+Idx)!=NULL) return;

  Win=CreateWindow("Combobox","",WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
                CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                10,y,40,300,Handle,(HMENU)(300+Idx),HInstance,NULL);
  SendMessage(Win,WM_SETFONT,(UINT)Font,0);
  char DriveName[8];
  DriveName[1]=':';DriveName[2]=0;
  SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Off"));
  for (int i=0;i<24;i++){
    DriveName[0]=(char)('C'+i);
    SendMessage(Win,CB_ADDSTRING,0,(long)DriveName);
  }
  SendMessage(Win,CB_SETCURSEL,(Drive[Idx].Letter-'C')+1,0);

  Win=CreateWindowEx(512,"Edit",Drive[Idx].Path,WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                      55,y,205,23,Handle,(HMENU)(100+Idx),HInstance,NULL);
  SendMessage(Win,WM_SETFONT,(UINT)Font,0);
  SendMessage(Win,EM_LIMITTEXT,MAX_PATH,0);
  int Len=SendMessage(Win,WM_GETTEXTLENGTH,0,0);
  SendMessage(Win,EM_SETSEL,Len,Len);
  SendMessage(Win,EM_SCROLLCARET,0,0);

  Win=CreateWindow("Button",T("Browse"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                265,y,75,23,Handle,(HMENU)(150+Idx),HInstance,NULL);
  SendMessage(Win,WM_SETFONT,(UINT)Font,0);

  Win=CreateWindow("Button",T("Open"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                345,y,75,23,Handle,(HMENU)(250+Idx),HInstance,NULL);
  SendMessage(Win,WM_SETFONT,(UINT)Font,0);

  Win=CreateWindow("Button",T("Remove"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                425,y,75,23,Handle,(HMENU)(200+Idx),HInstance,NULL);
  SendMessage(Win,WM_SETFONT,(UINT)Font,0);

  SetWindowHeight();
}
//---------------------------------------------------------------------------
void THardDiskManager::SetWindowHeight()
{
  RECT rc;
  SetWindowPos(Handle,0,0,0,516,80+GetSystemMetrics(SM_CYCAPTION)+(nDrives*30),SWP_NOZORDER | SWP_NOMOVE | SWP_NOCOPYBITS);
  SetWindowPos(GetDlgItem(Handle,90),0,10,12+(nDrives*30),0,0,SWP_NOZORDER | SWP_NOSIZE | SWP_NOCOPYBITS);
  SetWindowPos(GetDlgItem(Handle,10),0,300,12+(nDrives*30),0,0,SWP_NOZORDER | SWP_NOSIZE | SWP_NOCOPYBITS);
  GetClientRect(GetDlgItem(Handle,91),&rc);
  SetWindowPos(GetDlgItem(Handle,91),0,10,46+(nDrives*30),0,0,SWP_NOZORDER | SWP_NOSIZE | SWP_NOCOPYBITS);
  SetWindowPos(GetDlgItem(Handle,92),0,15+rc.right,42+(nDrives*30),0,0,SWP_NOZORDER | SWP_NOSIZE | SWP_NOCOPYBITS);
  SetWindowPos(GetDlgItem(Handle,IDOK),0,290,42+(nDrives*30),0,0,SWP_NOZORDER | SWP_NOSIZE | SWP_NOCOPYBITS);
  SetWindowPos(GetDlgItem(Handle,IDCANCEL),0,400,42+(nDrives*30),0,0,SWP_NOZORDER | SWP_NOSIZE | SWP_NOCOPYBITS);
}
//---------------------------------------------------------------------------
void THardDiskManager::GetDriveInfo()
{
  for (int i=0;i<nDrives;i++){
    Drive[i].Path.SetLength(MAX_PATH+1);
    SendMessage(GetDlgItem(Handle,100+i),WM_GETTEXT,MAX_PATH,(long)Drive[i].Path.Text);

    NO_SLASH(Drive[i].Path);

    if (Drive[i].Path.Length()==1) Drive[i].Path+=":"; 

    Drive[i].Letter=char(SendMessage(GetDlgItem(Handle,300+i),CB_GETCURSEL,0,0)+'C'-1);
  }
}
//---------------------------------------------------------------------------
void THardDiskManager::Hide()
{
  if (Handle==NULL) return;

  if (ApplyChanges){
    ApplyChanges=0;

    GetDriveInfo();

    for (int i=0;i<nDrives;i++){
      if (Drive[i].Path.IsEmpty()){
        Alert(T("One of the mounted paths is empty!"),T("Empty Path"),MB_ICONEXCLAMATION);
        return;
      }else{
        UINT Type=GetDriveType(EasyStr(Drive[i].Path[0])+":\\");
        if (Type==1){
          Alert(EasyStr(Drive[i].Path[0])+" "+T("is not a valid drive letter."),T("Invalid Drive"),MB_ICONEXCLAMATION);
          return;
        }else if (Type!=DRIVE_REMOVABLE && Type!=DRIVE_CDROM){
          DWORD Attrib=GetFileAttributes(Drive[i].Path);
          if (Attrib==0xffffffff){
            if (Alert(Drive[i].Path+" "+T("does not exist. Do you want to create it?"),T("New Folder?"),
                        MB_ICONQUESTION | MB_YESNO)==IDYES){
              if (CreateDirectory(Drive[i].Path,NULL)==0){
                Alert(T("Could not create the folder")+" "+Drive[i].Path,T("Invalid Path"),MB_ICONEXCLAMATION);
                return;
              }
            }else{
              return;
            }
          }else if ((Attrib & FILE_ATTRIBUTE_DIRECTORY)==0){
            Alert(Drive[i].Path+" "+T("is not a folder."),T("Invalid Path"),MB_ICONEXCLAMATION);
            return;
          }
        }
      }
    }

    //Remove old stemdos drives from ST memory
    long DrvMask=LPEEK(SV_drvbits);
    for (int i=0;i<nOldDrives;i++){
      if (OldDrive[i].Letter>='C'){ // Don't remove it if off!
        DrvMask &= ~(1 << (2+OldDrive[i].Letter-'C'));
      }
    }
    LPEEK(SV_drvbits)=DrvMask;

    update_mount();

#ifndef DISABLE_STEMDOS
    stemdos_boot_drive=SendDlgItemMessage(Handle,92,CB_GETCURSEL,0,0)+1;

    stemdos_update_drvbits();

    stemdos_check_paths();
#endif
  }else{
    nDrives=nOldDrives;
    for (int i=0;i<nOldDrives;i++){
      Drive[i]=OldDrive[i];
    }
  }

  if (OldDrive) delete[] OldDrive;

  EnableWindow(DiskMan.Handle,true);

  if (FullScreen){
    SetFocus(DiskMan.Handle);
  }else{
    SetForegroundWindow(DiskMan.Handle);
  }

  ShowWindow(Handle,SW_HIDE);

  DestroyWindow(Handle);Handle=NULL;

  PostMessage(DiskMan.Handle,WM_USER,0,0);

  ManageWindowClasses(SD_UNREGISTER);
}
//---------------------------------------------------------------------------
#define GET_THIS This=(THardDiskManager*)GetWindowLong(Win,GWL_USERDATA);

LRESULT __stdcall THardDiskManager::WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  LRESULT Ret=DefStemDialogProc(Win,Mess,wPar,lPar);
  if (StemDialog_RetDefVal) return Ret;

  THardDiskManager *This;
  switch (Mess){
    case WM_COMMAND:
    {
      int ID=LOWORD(wPar);
      GET_THIS;

      if (ID==10){
        // New Hard Drive
        if (HIWORD(wPar)==BN_CLICKED){
          if (This->nDrives<MAX_HARDDRIVES){
            This->GetDriveInfo();
            This->NewDrive(WriteDir);
            This->CreateDriveControls(This->nDrives-1);
            SetFocus(GetDlgItem(Win,100+This->nDrives-1));
            SendMessage(GetDlgItem(Win,10),BM_SETSTYLE,0,true);
            SendMessage(GetDlgItem(Win,IDOK),BM_SETSTYLE,1,true);
          }
        }
      }else if (ID==90){
        for (int i=0;i<This->nDrives;i++){
          SendMessage(GetDlgItem(This->Handle,300+i),CB_SETCURSEL,0,0);
        }
      }else if (ID==IDOK || ID==IDCANCEL){
        if (HIWORD(wPar)==BN_CLICKED){
          if (ID==IDOK){
            This->ApplyChanges=true;
          }
          PostMessage(Win,WM_CLOSE,0,0);
        }
      }else if (ID>=150 && ID<300){
        if (HIWORD(wPar)==BN_CLICKED){
          if (ID<200){
            ID-=150;
            // Browse
            SendMessage(HWND(lPar),BM_SETCHECK,1,true);
            EnableAllWindows(0,Win);

            This->GetDriveInfo();
            EasyStr NewFol=ChooseFolder(HWND(FullScreen ? StemWin:Win),T("Pick a Folder"),This->Drive[ID].Path);
            if (NewFol.NotEmpty()){
              SendMessage(GetDlgItem(This->Handle,100+ID),WM_SETTEXT,0,(long)NewFol.Text);
            }

            SetForegroundWindow(Win);
            EnableAllWindows(true,Win);
            SetFocus(HWND(lPar));
            SendMessage(HWND(lPar),BM_SETCHECK,0,true);
          }else if (ID<250){
            // Remove
            ID-=200;
            char Text[MAX_PATH+1];
            This->nDrives--;
            for (int i=ID;i<This->nDrives;i++){
              SendMessage(GetDlgItem(This->Handle,100+i+1),WM_GETTEXT,MAX_PATH,(long)Text);
              SendMessage(GetDlgItem(This->Handle,100+i),WM_SETTEXT,0,(long)Text);
              SendMessage(GetDlgItem(This->Handle,300+i),CB_SETCURSEL,
                           SendMessage(GetDlgItem(This->Handle,300+i+1),CB_GETCURSEL,0,0),0);
            }
            DestroyWindow(GetDlgItem(This->Handle,100+This->nDrives));
            DestroyWindow(GetDlgItem(This->Handle,150+This->nDrives));
            DestroyWindow(GetDlgItem(This->Handle,200+This->nDrives));
            DestroyWindow(GetDlgItem(This->Handle,250+This->nDrives));
            DestroyWindow(GetDlgItem(This->Handle,300+This->nDrives));
            This->GetDriveInfo();
            This->SetWindowHeight();
            if (This->nDrives){
              SetFocus(GetDlgItem(This->Handle,200+min(ID,This->nDrives-1)));
            }else{
              SetFocus(GetDlgItem(Win,IDOK));
            }
            SendMessage(GetFocus(),BM_SETSTYLE,1,true);

          }else{
            ID-=250;
            This->GetDriveInfo();
            ShellExecute(NULL,NULL,This->Drive[ID].Path,"","",SW_SHOWNORMAL);
          }
        }
      }
      break;
    }
    case (WM_USER+1011):
    {
      GET_THIS;

      HWND NewParent=(HWND)lPar;
      if (NewParent){
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
      return MAKELONG(IDOK,DC_HASDEFID);
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}

#undef GET_THIS
//---------------------------------------------------------------------------
#endif

#ifdef UNIX
#include "x/x_harddiskman.cpp"
#endif

