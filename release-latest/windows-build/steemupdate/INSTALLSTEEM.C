//---------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  if (Win==MainWin){
    if (Mess==WM_CLOSE) PostQuitMessage(0);
  }else if (Win==InstallWin){
    switch (Mess){
      case WM_CREATE:
      {
        SetProp(Win,"NotifyText",NULL);
        break;
      }
      case WM_PAINT:
      {
        HDC DC;
        RECT rc;
        char *Text;
        HBRUSH br;
        SIZE sz;

        GetClientRect(Win,&rc);

        DC=GetDC(Win);

        SelectObject(DC,GetStockObject(DEFAULT_GUI_FONT));

        br=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
        FillRect(DC,&rc,br);
        DeleteObject(br);

        SetBkMode(DC,TRANSPARENT);

        Text=(char*)GetProp(Win,"NotifyText");
        if (Text){
          GetTextExtentPoint32(DC,Text,strlen(Text),&sz);
          TextOut(DC,(rc.right-sz.cx)/2,(rc.bottom-sz.cy)/2,Text,strlen(Text));
        }

        ReleaseDC(Win,DC);
        ValidateRect(Win,NULL);
        return 0;
      }
      case WM_USER:
        if (wPar==12345){
          char *Text=(char*)GetProp(Win,"NotifyText"),*NewText=(char*)lPar;
          if (Text) free(Text);

          Text=(char*)malloc(strlen(NewText)+1);
          strcpy(Text,NewText);
          SetProp(Win,"NotifyText",Text);

          InvalidateRect(Win,NULL,1);
        }
        break;
      case WM_DESTROY:
      {
        char *Text=(char*)GetProp(Win,"NotifyText");
        if (Text) free(Text);
        break;
      }
      case WM_NCLBUTTONDOWN:case WM_NCRBUTTONDOWN:case WM_NCMBUTTONDOWN:
      case WM_NCLBUTTONUP:case WM_NCRBUTTONUP:case WM_NCMBUTTONUP:
        return 0;
    }
  }else{
    switch (Mess){
      case WM_COMMAND:
        if (HIWORD(wPar)==BN_CLICKED){
          if (LOWORD(wPar)>200){
            ButtonSelected=LOWORD(wPar)-201;
          }else if (LOWORD(wPar)==IDOK && Win==ZipProgress){
            ButtonSelected=800;
          }
        }
        break;
      case WM_ACTIVATE:
        if (wPar==WA_INACTIVE) Focus=GetFocus();
        break;
      case WM_SETFOCUS:
        SetFocus(Focus);
        break;
      case DM_GETDEFID:
        return MAKELONG(201,DC_HASDEFID);
    }
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
LRESULT __stdcall EditNoCaretWndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  switch (Mess){
    case WM_LBUTTONDOWN: case WM_MBUTTONDOWN: case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:   case WM_MBUTTONUP:   case WM_RBUTTONUP:
      return 0;
    case WM_SETCURSOR:
      SetCursor(LoadCursor(NULL,IDC_ARROW));
      return 0;
  }
  return CallWindowProc(OldEditWndProc,Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
WSADATA *InitWS(void)
{
  int Error;
  static WSADATA WsaData;
  BOOL ReturnValue=TRUE;

  Error=WSAStartup(MAKEWORD(1,1),&WsaData);
  if (Error){
    MessageBox(NULL,"Could not find the required version of WinSock.",
                 "Error",MB_ICONSTOP | MB_TASKMODAL | MB_SETFOREGROUND | MB_TOPMOST);
    ReturnValue=FALSE;
  }
  if (ReturnValue) return &WsaData;

  return NULL;
}
//---------------------------------------------------------------------------
void PRINT(char *Text)
{
  SendMessage(EditWin,EM_SETSEL,0xffffffff,0xffffffff);
  SendMessage(EditWin,EM_REPLACESEL,0,(LPARAM)Text);
  if (IsWindowVisible(MainWin)) UpdateWindow(EditWin);
}
//---------------------------------------------------------------------------
void AddLastError(char *PrintBuffer)
{
  switch (WSAGetLastError()){
    case WSANOTINITIALISED:	strcat(PrintBuffer,"A successful WSAStartup must occur before using this function.");break;
    case WSAENETDOWN:	strcat(PrintBuffer,"The network subsystem has failed.");break;
    case WSAHOST_NOT_FOUND:	strcat(PrintBuffer,"Authoritative Answer Host not found.");break;
    case WSATRY_AGAIN:	strcat(PrintBuffer,"Non-Authoritative Host not found, or server failure.");break;
    case WSANO_RECOVERY:	strcat(PrintBuffer,"Nonrecoverable error occurred.");break;
    case WSANO_DATA:	strcat(PrintBuffer,"Valid name, no data record of requested type.");break;
    case WSAEINPROGRESS:	strcat(PrintBuffer,"A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.");break;
    case WSAEFAULT:	strcat(PrintBuffer,"The name argument is not a valid part of the user address space.");break;
    case WSAEINTR:	strcat(PrintBuffer,"The (blocking) call was canceled through WSACancelBlockingCall.");break;
    case WSAEADDRINUSE:	strcat(PrintBuffer,"The specified address is already in use.");break;
    case WSAEALREADY:	strcat(PrintBuffer,"A nonblocking connect call is in progress on the specified socket.Note  In order to preserve backward compatibility, this error is reported as WSAEINVAL to Windows Sockets 1.1 applications that link to either WINSOCK.DLL or WSOCK32.DLL.");break;
    case WSAEADDRNOTAVAIL: strcat(PrintBuffer,"The specified address is not available from the local machine.");break;
    case WSAEAFNOSUPPORT: strcat(PrintBuffer,"Addresses in the specified family cannot be used with this socket.");break;
    case WSAECONNREFUSED: strcat(PrintBuffer,"The attempt to connect was forcefully rejected.");break;
    case WSAEINVAL: strcat(PrintBuffer,"The parameter s is a listening socket, or the destination address specified is not consistent with that of the constrained group the socket belongs to.");break;
    case WSAEISCONN: strcat(PrintBuffer,"The socket is already connected (connection-oriented sockets only).");break;
    case WSAENETUNREACH: strcat(PrintBuffer,"The network cannot be reached from this host at this time.");break;
    case WSAENOBUFS: strcat(PrintBuffer,"No buffer space is available. The socket cannot be connected.");break;
    case WSAENOTSOCK: strcat(PrintBuffer,"The descriptor is not a socket.");break;
    case WSAETIMEDOUT: strcat(PrintBuffer,"Attempt to connect timed out without establishing a connection.");break;
    case WSAEWOULDBLOCK: strcat(PrintBuffer,"The socket is marked as nonblocking and the connection cannot be completed immediately. It is possible to select the socket while it is connecting by selecting it for writing.");break;
    case WSAEACCES: strcat(PrintBuffer,"Attempt to connect datagram socket to broadcast address failed because setsockopt SO_BROADCAST is not enabled.");break;
    default: strcat(PrintBuffer,"Unknown Error!");
  }
  strcat(PrintBuffer,"\r\n");
}
//---------------------------------------------------------------------------
void CentreWindow(HWND Win,bool Redraw)
{
  RECT rc;
  int W,H;

  GetWindowRect(Win,&rc);
  W=rc.right-rc.left;
  H=rc.bottom-rc.top;
  MoveWindow(Win,(GetSystemMetrics(SM_CXSCREEN)-W)/2,(GetSystemMetrics(SM_CYSCREEN)-H)/2,W,H,Redraw);
}
//---------------------------------------------------------------------------
typedef bool (WINAPI *LPMODULEWALK)(HANDLE,void */*LPMODULEENTRY32*/);
typedef HANDLE (WINAPI *LPCREATESNAPSHOT)(DWORD,DWORD);

void GetWindowExePath(HWND Win,char *Buf)
{
  DWORD ProcID;
  HANDLE Snap;
  MODULEENTRY32 me;
  LPCREATESNAPSHOT pCreateToolhelp32Snapshot;
  LPMODULEWALK pModule32First;
  LPMODULEWALK pModule32Next;
  HINSTANCE hKernel;
  char *pBuf;

  Buf[0]=0;

  GetWindowThreadProcessId(Win,&ProcID);

  hKernel=GetModuleHandle("KERNEL32.DLL");
  if (hKernel){
    pCreateToolhelp32Snapshot=(LPCREATESNAPSHOT)GetProcAddress(hKernel,"CreateToolhelp32Snapshot");

    pModule32First=(LPMODULEWALK)GetProcAddress(hKernel,"Module32First");
    pModule32Next=(LPMODULEWALK)GetProcAddress(hKernel,"Module32Next");

    if (pModule32First==NULL || pModule32Next==NULL || pCreateToolhelp32Snapshot==NULL) return;
  }else{
    return;
  }

  Snap=pCreateToolhelp32Snapshot(TH32CS_SNAPMODULE,ProcID);

  pBuf=Buf;
  me.dwSize=sizeof(MODULEENTRY32);
  if (pModule32First(Snap,&me)){
    do{
      if (strcmpi(me.szExePath+(strlen(me.szExePath)-3),"EXE")==0){
        strcpy(pBuf,me.szExePath);
        pBuf+=strlen(me.szExePath)+1;
        pBuf[0]=0;
      }
      me.dwSize=sizeof(MODULEENTRY32);
    }while (pModule32Next(Snap,&me));
  }

  CloseHandle(Snap);
}
//---------------------------------------------------------------------------
HWND GetNextSteemWindow(HWND LastWin)
{
  HWND Win;
  char Buf[MAX_PATH*5];

  if (LastWin){
    Win=GetWindow(LastWin,GW_HWNDNEXT);
  }else{
    Win=GetWindow(GetDesktopWindow(),GW_CHILD);
  }
  do{
    if (GetClassName(Win,Buf,300)){
      if (strcmpi(Buf,"Steem Window")==0){
        GetWindowExePath(Win,Buf);
        if (Buf[0]){
          int PathLen;
          char *pBuf=Buf;

          for (;;){
            PathLen=strlen(pBuf);
            if (PathLen==0) break;
            *(strrchr(pBuf,'\\'))=0;
            if (strcmpi(pBuf,RunDir)==0) break;
            pBuf+=PathLen+1;
          }
          if (pBuf[0]) break;
        }
      }
    }
  }while ( (Win=GetWindow(Win,GW_HWNDNEXT))!=NULL );

  return Win;
}
//---------------------------------------------------------------------------
HWND GetFirstSteemWindow()
{
  return GetNextSteemWindow(NULL);
}
//---------------------------------------------------------------------------
int NewSteemDialog(char *Desc)
{
  HWND Win,Edit,But;
  MSG msg;

  {
    WNDCLASS wc;

    memset(&wc,0,sizeof(WNDCLASS));
    wc.style=CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_NOCLOSE;
    wc.lpfnWndProc=(WNDPROC)WndProc;
    wc.hInstance=Inst;
    wc.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
    wc.lpszClassName="Steem Update Choice Window";
    wc.lpszMenuName=NULL;
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.hIcon=LoadIcon(Inst,"APPICON");
    if (RegisterClass(&wc)==0) return -1;
  }

  Win=CreateWindow("Steem Update Choice Window","New Version of Steem",WS_CAPTION | WS_SYSMENU,
                        100,100,406,421+GetSystemMetrics(SM_CYCAPTION),
                        NULL,NULL,Inst,NULL);
  if (Win==NULL) return -1;

  Edit=CreateWindowEx(512,"Edit","",WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE,
                          10,10,380,300,Win,(HMENU)200,Inst,NULL);
  SetWindowText(Edit,Desc);
  SendMessage(Edit,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),0);
  SetWindowLong(Edit,GWL_WNDPROC,(long)EditNoCaretWndProc);

  But=CreateWindow("Button","Download Now!",WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON,
                          10,320,380,25,Win,(HMENU)201,Inst,NULL);
  SendMessage(But,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),0);

  But=CreateWindow("Button","Download Some Other Time",WS_VISIBLE | WS_CHILD | WS_TABSTOP,
                          10,350,380,25,Win,(HMENU)202,Inst,NULL);
  SendMessage(But,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),0);

  But=CreateWindow("Button","Never Download This Version",WS_VISIBLE | WS_CHILD | WS_TABSTOP,
                          10,380,380,25,Win,(HMENU)203,Inst,NULL);
  SendMessage(But,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),0);

  CentreWindow(Win,0);

  Focus=GetDlgItem(Win,201);
  if (Silent){
    HWND SteemWin;

    SteemWin=GetFirstSteemWindow();
    if (SteemWin!=NULL){
      do{
        SendMessage(SteemWin,WM_USER+2,54542,(LPARAM)Win);
      }while ( (SteemWin=GetNextSteemWindow(SteemWin))!=NULL );
    }else{
      ShowWindow(Win,SW_SHOW);
      SetForegroundWindow(Win);
    }
  }else{
    ShowWindow(Win,SW_SHOW);
    SetForegroundWindow(Win);
  }

  ButtonSelected=-1;

  SetTimer(Win,1,1000,NULL);
  while (GetMessage(&msg,NULL,0,0)){
    if (IsDialogMessage(Win,&msg)==0){
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    if (ButtonSelected>=0) break;
    if (msg.message==WM_TIMER){
      if (IsWindowVisible(Win)==0 && GetFirstSteemWindow()==NULL){
        ShowWindow(Win,SW_SHOW);
      }
    }
  }
  KillTimer(Win,1);

  DestroyWindow(Win);

  if (Silent){
    HWND SteemWin;
    // Allow for more than one SteemWin
    SteemWin=GetFirstSteemWindow();
    if (SteemWin!=NULL){
      do{
        SendMessage(SteemWin,WM_USER+2,54542,(LPARAM)NULL);
      }while ( (SteemWin=GetNextSteemWindow(SteemWin))!=NULL );
    }
  }

  return ButtonSelected;
}
//---------------------------------------------------------------------------
void DeleteDirAndContents(char *Dir)
{
  static WIN32_FIND_DATA wfd;
  bool PutSlashBack=0;
  char NewDir[MAX_PATH+1];
  HANDLE FHan;

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
  FHan=FindFirstFile(NewDir,&wfd);
  if (FHan!=INVALID_HANDLE_VALUE){
    do{
      if (strcmp(wfd.cFileName,".") && strcmp(wfd.cFileName,"..")){
        strcpy(NewDir,Dir);
        strcat(NewDir,"\\");
        strcat(NewDir,wfd.cFileName);
        if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
          DeleteDirAndContents(NewDir);
        }else{
          SetFileAttributes(NewDir,FILE_ATTRIBUTE_NORMAL);
          DeleteFile(NewDir);
        }
      }
    }while (FindNextFile(FHan,&wfd));
  }
  FindClose(FHan);

  RemoveDirectory(Dir);

  if (PutSlashBack) Dir[strlen(Dir)]='\\';
}
//---------------------------------------------------------------------------
#define ISTYPE_QUIT 0
#define ISTYPE_STEEMIDLE 1
#define ISTYPE_NOSTEEM 2

int WaitForIdleSteem()
{
  HWND SteemWin;
  MSG msg;

  SteemWin=GetFirstSteemWindow();
  if (SteemWin){
    if (Silent){
      bool Running=0;

      do{
        if (SendMessage(SteemWin,WM_USER+2,2323,0)){
          Running=true;
          break;
        }
      }while ( (SteemWin=GetNextSteemWindow(SteemWin))!=NULL );

      if (Running){
        SetTimer(MainWin,1,1000,NULL);
        while (GetMessage(&msg,NULL,0,0)){
          TranslateMessage(&msg);
          DispatchMessage(&msg);

          if (msg.message==WM_TIMER){
            Running=0;
            do{
              if (SendMessage(SteemWin,WM_USER+2,2323,0)){
                Running=true;
                break;
              }
            }while ( (SteemWin=GetNextSteemWindow(SteemWin))!=NULL );
            if (Running==0) break;
          }
        }
        KillTimer(MainWin,1);

        if (msg.message==WM_QUIT) return ISTYPE_QUIT;
      }
    }
    return ISTYPE_STEEMIDLE;
  }
  return ISTYPE_NOSTEEM;
}
//---------------------------------------------------------------------------
bool LoadZipLib(HINSTANCE *phZip)
{
  HINSTANCE h;

  h=LoadLibrary("unzipd32.dll");
  if (h){
    GetFirstInZip=(int(_stdcall*)(char*,PackStruct*))GetProcAddress(h,"GetFirstInZip");
    GetNextInZip=(int(_stdcall*)(PackStruct*))GetProcAddress(h,"GetNextInZip");
    CloseZipFile=(void(_stdcall*)(PackStruct*))GetProcAddress(h,"CloseZipFile");
    UnzipFile=(int(_stdcall*)(char*,char*,WORD,long,void*,long))GetProcAddress(h,"unzipfile");
    if (! (GetFirstInZip && GetNextInZip && CloseZipFile && UnzipFile) ){
      FreeLibrary(h);
    }else{
      *phZip=h;
      return true;
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
bool InstallNewSteem(char *FileName,char *VersionText)
{
  HINSTANCE hZip=NULL;
  bool UnzippedOkay=0,ZipCorrupt=0;
  int Ret;
  HWND SteemWin;
  char Message[800];
  MSG msg;
  bool KeepOld;

  Ret=WaitForIdleSteem();
  if (Ret==ISTYPE_QUIT){
    PostQuitMessage(0);
    return 0;
  }
  if (Ret==ISTYPE_NOSTEEM){
    sprintf(Message,"Steem version %s is ready to install, "
                    "do you want to install it now?",VersionText);
    Ret=MessageBox(NULL,Message,"Install Steem Now?",MB_YESNO | MB_ICONQUESTION | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
  }else{
    sprintf(Message,"Steem version %s is ready to install, "
                    "do you want to close Steem and install it now?",VersionText);
    Ret=MessageBox(NULL,Message,"Install Steem Now?",MB_YESNO | MB_ICONQUESTION | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
  }
  if (Ret==IDNO){
    sprintf(Message,"When you are ready to install the new version of Steem you can either run update again "
                    "(it won't re-download anything) or manually unzip all the files from %s to your "
                    "Steem directory, overwriting the files currently there.",FileName);

    MessageBox(NULL,Message,"Install Cancelled",MB_ICONINFORMATION | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
    return 0;
  }

  InstallWin=CreateWindowEx(WS_EX_TOPMOST,"Steem Update Window","Please Wait",WS_CAPTION,0,0,200,100,NULL,NULL,Inst,NULL);
  CentreWindow(InstallWin,0);

  SteemWin=GetFirstSteemWindow();
  if (SteemWin){
    SendMessage(InstallWin,WM_USER,12345,(long)"Closing Steem");
    ShowWindow(InstallWin,SW_SHOW);

    do{
      PostMessage(SteemWin,WM_CLOSE,0,0);
    }while ( (SteemWin=GetNextSteemWindow(SteemWin))!=NULL );

    SetTimer(MainWin,1,1000,NULL);
    while (GetMessage(&msg,NULL,0,0)){
      TranslateMessage(&msg);
      DispatchMessage(&msg);

      if (msg.message==WM_TIMER){
        SteemWin=GetFirstSteemWindow();
        if (SteemWin==NULL) break;
        PostMessage(SteemWin,WM_CLOSE,0,0);
      }
    }
    KillTimer(MainWin,1);

    if (msg.message==WM_QUIT){
      PostQuitMessage(0);
      return 0;
    }

    Sleep(5000);  //Make sure it is closed
  }
  sprintf(Message,"All updates are better than the last version but in some rare circumstances "
                  "fixing something can cause a program that used to work to stop. Would you "
                  "like to keep your old version of Steem just in case?",VersionText);
  KeepOld=(MessageBox(NULL,Message,"Keep Old Version?",MB_YESNO | MB_ICONQUESTION | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND)==IDYES);


  SendMessage(InstallWin,WM_USER,12345,(long)"Decompacting New Version");
  ShowWindow(InstallWin,SW_SHOW);
  UpdateWindow(InstallWin);

  if (LoadZipLib(&hZip)){
    PackStruct PackInfo;

    if (GetFirstInZip(FileName,&PackInfo)==UNZIP_Ok){
      int n=0,nFiles=1,i,ErrVal;
      char DestFile[MAX_PATH+1],SrcFile[MAX_PATH+1];
      struct ZippedFileInfo{
        char Path[MAX_PATH+1];
        int HeaderOffset,Attr;
      }*pZfi;

      while (GetNextInZip(&PackInfo)==UNZIP_Ok) nFiles++;
      CloseZipFile(&PackInfo);

      pZfi=(struct ZippedFileInfo*)malloc(sizeof(struct ZippedFileInfo)*nFiles);

      GetFirstInZip(FileName,&PackInfo);
      do{
        if (strcmpi(PackInfo.FileName,"steemupdate.exe")==0){
          strcpy(PackInfo.FileName,"new_steemupdate.exe");
        }
        char *c=PackInfo.FileName+strlen(PackInfo.FileName)-1;
        if (*c=='/') *c='\\';

        strcpy(pZfi[n].Path,PackInfo.FileName);
        pZfi[n].HeaderOffset=PackInfo.HeaderOffset;
        pZfi[n].Attr=PackInfo.Attr;
        n++;
      }while (GetNextInZip(&PackInfo)==UNZIP_Ok);
      CloseZipFile(&PackInfo);

      sprintf(DestFile,"%s\\NewVersion",RunDir);
      CreateDirectory(DestFile,NULL);

      // Unzip files and folders to the NewVersion directory
      UnzippedOkay=true;
      for (i=0;i<nFiles;i++){
        sprintf(DestFile,"%s\\NewVersion\\%s",RunDir,pZfi[i].Path);
    if (DestFile[strlen(DestFile)-1]=='\\'){
          DestFile[strlen(DestFile)-1]=0;
          CreateDirectory(DestFile,NULL);
    }else{
          ErrVal=UnzipFile(FileName,DestFile,(WORD)pZfi[i].Attr,
                          (long)pZfi[i].HeaderOffset,NULL,0);
          if (ErrVal!=UNZIP_Ok){
              UnzippedOkay=0;
              break;
          }
    }
      }
      FreeLibrary(hZip);

      if (UnzippedOkay){
        SendMessage(InstallWin,WM_USER,12345,(long)"Backing Up Old Version");
        UpdateWindow(InstallWin);

        sprintf(DestFile,"%s\\v%s",RunDir,CurrentVersion);
        CreateDirectory(DestFile,NULL);

        // Move files to be overwritten to the OldVersion directory
        for (i=0;i<nFiles;i++){
          sprintf(DestFile,"%s\\v%s\\%s",RunDir,CurrentVersion,pZfi[i].Path);
          if (DestFile[strlen(DestFile)-1]=='\\'){
            DestFile[strlen(DestFile)-1]=0;
            CreateDirectory(DestFile,NULL);
          }else{
            sprintf(SrcFile,"%s\\%s",RunDir,pZfi[i].Path);
            if (access(SrcFile,0)==0){
              if (MoveFile(SrcFile,DestFile)==0) UnzippedOkay=0;
            }
          }
        }
        if (UnzippedOkay){
          if (KeepOld){
            int i;
            // A few extra for if you want to keep your old version
            for (i=0;i<MAX_BACKUP_FILES;i++){
              if (FilesToBackup[i]==NULL) break;

              sprintf(SrcFile,"%s\\%s",RunDir,FilesToBackup[i]);
              sprintf(DestFile,"%s\\v%s\\%s",RunDir,CurrentVersion,FilesToBackup[i]);
              if (access(SrcFile,0)==0) CopyFile(SrcFile,DestFile,0);
            }
          }

          // Remove old empty dirs from root
          for (i=0;i<nFiles;i++){
            sprintf(DestFile,"%s\\%s",RunDir,pZfi[i].Path);
            if (DestFile[strlen(DestFile)-1]=='\\'){
              DestFile[strlen(DestFile)-1]=0;
              RemoveDirectory(DestFile);
            }
          }
          SendMessage(InstallWin,WM_USER,12345,(long)"Installing New Version");
          UpdateWindow(InstallWin);

          // Move files from NewVersion into root
          for (i=0;i<nFiles;i++){
            sprintf(SrcFile,"%s\\NewVersion\\%s",RunDir,pZfi[i].Path);
            sprintf(DestFile,"%s\\%s",RunDir,pZfi[i].Path);
            if (DestFile[strlen(DestFile)-1]=='\\'){
              DestFile[strlen(DestFile)-1]=0;
              CreateDirectory(DestFile,NULL);
            }else{
              MoveFile(SrcFile,DestFile);
            }
          }

          SendMessage(InstallWin,WM_USER,12345,(long)"Cleaning Up");
          UpdateWindow(InstallWin);

          if (KeepOld==0){
            sprintf(DestFile,"%s\\v%s",RunDir,CurrentVersion);
            DeleteDirAndContents(DestFile);
          }else{
            // No SteemUpdate for old versions
            sprintf(DestFile,"%s\\v%s\\new_steemupdate.exe",RunDir,CurrentVersion);
            DeleteFile(DestFile);
          }
          sprintf(DestFile,"%s\\NewVersion",RunDir);
          DeleteDirAndContents(DestFile);

          DeleteFile(FileName);
        }else{
          SendMessage(InstallWin,WM_USER,12345,(long)"Error - Restoring From Backup");
          UpdateWindow(InstallWin);

          // Restore files from OldVersion folder
          for (i=0;i<nFiles;i++){
            sprintf(SrcFile,"%s\\v%s\\%s",RunDir,CurrentVersion,pZfi[i].Path);
            sprintf(DestFile,"%s\\%s",RunDir,pZfi[i].Path);
            if (access(SrcFile,0)==0){
              if (DestFile[strlen(DestFile)-1]!='\\'){
                if (access(DestFile,0)==0){
                  SetFileAttributes(DestFile,FILE_ATTRIBUTE_NORMAL);
                  DeleteFile(DestFile);
                }
                MoveFile(SrcFile,DestFile);
              }
            }
          }

          sprintf(DestFile,"%s\\v%s",RunDir,CurrentVersion);
          DeleteDirAndContents(DestFile);
          sprintf(DestFile,"%s\\NewVersion",RunDir);
          DeleteDirAndContents(DestFile);

          DestroyWindow(InstallWin);

          MessageBox(NULL,"Update was unable to remove some of your old Steem files. It may be they are in use "
                          "by another program. Please try again soon (maybe after your next restart), "
                          "your current version of Steem will still work in the meantime.","Update Error",
                      MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST | MB_TASKMODAL);

        }
        free(pZfi);
        DestroyWindow(InstallWin);

        return UnzippedOkay;
      }else{
        sprintf(DestFile,"%s\\NewVersion",RunDir);
        DeleteDirAndContents(DestFile);

        ZipCorrupt=true;
      }
      free(pZfi);
    }else{
      FreeLibrary(hZip);

      ZipCorrupt=true;
    }
  }else{
    DestroyWindow(InstallWin);

    sprintf(Message,"Update could not find it's unzipping library unzipd32.dll, you will have to "
                    "manually unzip all the files from %s to your Steem directory, overwriting the "
                    "files currently there.",FileName);

    MessageBox(NULL,Message,"Install Failed",MB_ICONEXCLAMATION | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
  }
  DestroyWindow(InstallWin);

  if (ZipCorrupt){
    char NewFileName[MAX_PATH+2];
    strcpy(NewFileName,FileName);
    strcpy(NewFileName+strlen(NewFileName)-4,"_corrupt.zip");
    MoveFileEx(FileName,NewFileName,MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);
    sprintf(Message,"The zip file containing Steem and its required files seems to be corrupt! "
                    "It could be that the zip is okay and this program has a problem, just in "
                    "case the downloaded zip is here so you can manually attempt to extract the files:"
                    "\n\n%s\n\nIt may be that the file wasn't properly download from the "
                    "internet, if you run Update again it will re-download it.\n"
                    "If the same thing happens again please send us an e-mail to the address:\n\n"
                    STEEM_EMAIL "\n\nWe will look into it right away.",NewFileName);
    MessageBox(NULL,Message,"Install Failed",MB_ICONEXCLAMATION | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
  }
  return 0;
}

