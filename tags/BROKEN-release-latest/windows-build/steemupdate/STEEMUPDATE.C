#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <winsock2.h>
#include <wininet.h>
#include <tlhelp32.h>
//---------------------------------------------------------------------------
typedef struct{
  char InternalUse[12];    // Used internally by the dll
  int Time;                // File time
  int Size;                // File size
  int CompressSize;        // Size in zipfile
  int HeaderOffset;        // File offset in zip
  long Crc;                // CRC, sort of checksum
  char FileName[260];      // File name
  WORD PackMethod;         /* 0=Stored, 1=Shrunk, 2=Reduced 1, 3=Reduced 2, 4=Reduced 3, 5=Reduced 4,
  6=Imploded,7=Tokenized (format does not exist), 8=Deflated,
  More than 8=Unknown method.
  For this DLL this number can only be 0, 8, or more than 8
  */
  WORD Attr;               // File attributes
  WORD Flags;              // Only used by ARJ unpacker (LOBYTE: arj_flags, HIBYTE: file_type)
}
PackStruct;

int (_stdcall *GetFirstInZip)(char*,PackStruct*); //find out what files are in the ZIP file (first file)
int (_stdcall *GetNextInZip)(PackStruct*);        //get next file in ZIP
void (_stdcall *CloseZipFile)(PackStruct*);       //free buffers and close ZIP after GetNextInZip()
int (_stdcall *UnzipFile)(char*,char*,WORD,long,void*,long);        //unzipping

#define UNZIP_Ok           0              //Unpacked ok
#define UNZIP_CRCErr       1              //CRC error
#define UNZIP_WriteErr     2              //Error writing out file: maybe disk full
#define UNZIP_ReadErr      3              //Error reading zip file
#define UNZIP_ZipFileErr   4              //Error in zip structure
#define UNZIP_UserAbort    5              //Aborted by user
#define UNZIP_NotSupported 6              //ZIP Method not supported!
#define UNZIP_Encrypted    7              //Zipfile encrypted
#define UNZIP_InUse        -1              //DLL in use by other program!
#define UNZIP_DLLNotFound  -2              //DLL not loaded!

#ifndef __cplusplus
typedef int bool;
#define true 1
#define false 0
#endif
//---------------------------------------------------------------------------
HINSTANCE Inst;
WNDPROC OldEditWndProc;
HWND MainWin,EditWin,InstallWin=NULL;
WSADATA *WsaData;
int ButtonSelected=-1,DebugBuild=0,StartSteem=0,Silent=0,Downloading=0,AlwaysOnline=0;
int CheckForPatches=true,AskPatchInstall=0;
char INIFile[MAX_PATH+1],RunDir[MAX_PATH+1],ZipFileName[MAX_PATH+1],ProxyURL[250]={
  0};
char CurrentVersion[20]={
  0};
HWND Focus,ZipProgress=NULL,ProgBar;
DWORD LastRecv;
FILE *f;
SOCKET Socket;
DWORD IP_Addr=0;
int Port=-1;
int nRuns,nOffline,nWSError;

#define MAX_BACKUP_FILES 20
char *FilesToBackup[MAX_BACKUP_FILES]={
  "steem.ini","shortcuts.dat","breaks.dat","logsection.dat",
  "unzipd32.dll","auto.sts","steem.new","links.txt","readme.txt",
  "debug.txt","Steem.exe",NULL};

char *SteemNewServer[4]={
  "steem.atari.st",
  "www.steem.quicksurfer.co.uk",
  "www.blimey.strayduck.com",
  NULL};

char *SteemNewURL[4]={
  "http://steem.atari.st/steem.new",
  "http://www.steem.quicksurfer.co.uk/steem.new",
  "http://www.blimey.strayduck.com/steem.new",
  NULL};

#define STEEM_EMAIL "steem@gmx.net"

char SteemZipServer[500];    //="steem.atari.st";
char SteemZipFolderURL[500]; //="http://steem.atari.st/steem.new";

FILE *LogFile=NULL;
//---------------------------------------------------------------------------
#include "installsteem.c"
//---------------------------------------------------------------------------
SOCKET HTTPConnect(DWORD IP)
{
  int nRet;
  LPSERVENT lpServEnt;
  SOCKADDR_IN saServer;
  SOCKET Socket;
  char Buffer[500];

  PRINT("Creating socket\r\n");
  // Create a TCP/IP stream socket
  Socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  if (Socket==INVALID_SOCKET){
    strcpy(Buffer,"Socket failed: ");
    AddLastError(Buffer);
    PRINT(Buffer);
    return INVALID_SOCKET;
  }

  if (Port==-1){
    PRINT("Finding HTTP port\r\n");
    // Find the port number for the HTTP service on TCP
    lpServEnt=getservbyname("HTTP","TCP");
    if (lpServEnt==NULL){
      saServer.sin_port=htons(80);
      PRINT("Can't find, guessing 80\r\n");
    }
    else{
      saServer.sin_port=lpServEnt->s_port;
      sprintf(Buffer,"Port: %u\r\n",lpServEnt->s_port);
      PRINT(Buffer);
    }
  }
  else{
    saServer.sin_port=htons((u_short)Port);
    sprintf(Buffer,"Port: %u\r\n",Port);
    PRINT(Buffer);
  }

  PRINT("Connecting to server\r\n");
  // Fill in the rest of the server address structure
  saServer.sin_family=AF_INET;
  saServer.sin_addr.s_addr=IP;
  ZeroMemory(saServer.sin_zero,sizeof(saServer.sin_zero));
  // Connect the socket
  nRet=connect(Socket,(LPSOCKADDR)&saServer,sizeof(SOCKADDR_IN));
  if (nRet==SOCKET_ERROR){
    strcpy(Buffer,"Connect failed: ");
    AddLastError(Buffer);
    PRINT(Buffer);
    closesocket(Socket);
    return INVALID_SOCKET;
  }
  return Socket;
}
//---------------------------------------------------------------------------
DWORD WINAPI DownloadSteemDotNew(LPVOID pass_serv)
{
  int nRet=SOCKET_ERROR;
  char Buffer[1000];
  DWORD serv=(DWORD)pass_serv;

  Socket=HTTPConnect(IP_Addr);
  if (Socket!=INVALID_SOCKET){
    // Now connected to HTTP server

    PRINT("Sending GET message\r\n");
    // Format the HTTP request
    sprintf(Buffer,"GET %s HTTP/1.1\r\n"
          "Host: %s\r\n"
          "\r\n",SteemNewURL[serv],SteemNewServer[serv]);
    nRet=send(Socket,Buffer,strlen(Buffer),0);
    if (nRet!=SOCKET_ERROR){
      bool First=true;
      FILE *f;

      sprintf(Buffer,"%s\\new_steem.new",RunDir);
      f=fopen(Buffer,"w+b");
      if (f){
        PRINT("Waiting to receive data\r\n\r\n");
        for (;;){
          // Wait to receive, nRet = number of bytes received
          nRet=recv(Socket,Buffer,950,0);
          LastRecv=GetTickCount();
          if (nRet==SOCKET_ERROR){
            strcpy(Buffer,"Recv failed: ");
            AddLastError(Buffer);
            strcat(Buffer,"\r\n");
            PRINT(Buffer);
            break;
          }
          else if (nRet==0){
            PRINT("Server has closed connection, file done\r\n");
            break;
          }

          Buffer[nRet]=0;
          if (First){
            char *EndOfHeader;

            EndOfHeader=strstr(Buffer,"\r\n\r\n");
            if (EndOfHeader){
              fprintf(f,"%s",EndOfHeader+4);
            }
            First=0;
          }
          else{
            fprintf(f,"%s",Buffer);
          }
        }
        fseek(f,-100,SEEK_END);
        fread(Buffer,1,100,f);
        fclose(f);

        Buffer[100]=0;
        if (strstr(Buffer,"[END]")){  //Got end of file

          char SteemNewFN[MAX_PATH+1];

          sprintf(SteemNewFN,"%s\\steem.new",RunDir);
          SetFileAttributes(SteemNewFN,FILE_ATTRIBUTE_NORMAL);
          DeleteFile(SteemNewFN);
          sprintf(Buffer,"%s\\new_steem.new",RunDir);
          MoveFile(Buffer,SteemNewFN);
        }
        else{
          sprintf(Buffer,"%s\\new_steem.new",RunDir);
          DeleteFile(Buffer);
          PRINT("Steem.new didn't have the correct ending, download must have failed\r\n");
        }
      }
      else{
        PRINT("Couldn't open temporary steem.new\r\n");
      }
    }
    else{
      strcpy(Buffer,"Send failed: ");
      AddLastError(Buffer);
      PRINT(Buffer);
    }
    closesocket(Socket);
  }
  Downloading=0;
  return nRet;
}
//---------------------------------------------------------------------------
DWORD WINAPI DownloadSteemDotZip(LPVOID pass_FileSize)
{
  int nRet=SOCKET_ERROR;
  char Buffer[1000];
  DWORD FileSize=(DWORD)pass_FileSize;

  PRINT("Sending...\r\n");
  // Format the HTTP request
  sprintf(Buffer,"GET %s" "%s HTTP/1.1\r\n" "Host: %s\r\n"
        "Accept: application/zip\r\n" "Content-Type: application/zip\r\n" "\r\n",
    SteemZipFolderURL,strrchr(ZipFileName,'\\')+1,SteemZipServer);
  PRINT(Buffer);
  nRet=send(Socket,Buffer,strlen(Buffer),0);
  if (nRet!=SOCKET_ERROR){
    f=fopen(ZipFileName,"wb");
    if (f){
      DWORD Len;
      int GotSoFar=0,OnePercent=FileSize/100;
      ShowWindow(ZipProgress,SW_SHOW);

      PRINT("Waiting to receive data\r\n\r\n");
      for (;;){
        // Wait to receive, nRet = number of bytes received
        nRet=recv(Socket,Buffer,500,0);
        LastRecv=GetTickCount();
        if (nRet==SOCKET_ERROR){
          strcpy(Buffer,"nRecv failed: ");
          AddLastError(Buffer);
          strcat(Buffer,"\r\n");
          PRINT(Buffer);
          break;
        }
        else if (nRet==0){
          PRINT("Server has closed connection, file done!\r\n");
          break;
        }
        else if (ButtonSelected==800){
          nRet=SOCKET_ERROR;
          break;
        }

        if (GotSoFar==0){
          char *PastHeader;
          // Skip header
          Buffer[nRet]=0;
          PastHeader=strstr(Buffer,"\r\n\r\n");
          if (PastHeader){
            PastHeader+=4;
            GotSoFar=nRet-(long)(PastHeader-Buffer);
            fwrite(PastHeader,1,GotSoFar,f);
          }
        }
        else{
          fwrite(Buffer,1,nRet,f);
          fflush(f);
          if ((GotSoFar+nRet)/OnePercent > GotSoFar/OnePercent){
            SendMessage(ProgBar,PBM_SETPOS,(GotSoFar+nRet)/OnePercent,0);
          }
          GotSoFar+=nRet;
        }
      }

      Len=ftell(f);
      fclose(f);
      f=NULL;

      if (Len!=FileSize){
        DeleteFile(ZipFileName);
        PRINT("Download failed, file size wrong.\r\n");
        nRet=SOCKET_ERROR;
      }
      else{
        nRet=0;
      }
    }
    else{
      sprintf(Buffer,"%s couldn't be open.",ZipFileName);
      PRINT(Buffer);
    }
  }
  else{
    strcpy(Buffer,"Send failed: ");
    AddLastError(Buffer);
    PRINT(Buffer);
  }
  Downloading=0;
  return nRet;
}
//---------------------------------------------------------------------------
DWORD WINAPI DownloadPatchesDotZip(LPVOID pass_FileSize)
{
  int nRet=SOCKET_ERROR;
  char Buffer[1000];
  DWORD FileSize=(DWORD)pass_FileSize;

  PRINT("Sending...\r\n");
  // Format the HTTP request
  sprintf(Buffer,"GET %s" "%s HTTP/1.1\r\n" "Host: %s\r\n"
        "Accept: application/zip\r\n" "Content-Type: application/zip\r\n" "\r\n",
    SteemZipFolderURL,strrchr(ZipFileName,'\\')+1,SteemZipServer);
  PRINT(Buffer);
  nRet=send(Socket,Buffer,strlen(Buffer),0);
  if (nRet!=SOCKET_ERROR){
    f=fopen(ZipFileName,"wb");
    if (f){
      DWORD Len;
      int GotSoFar=0;

      PRINT("Waiting to receive data\r\n\r\n");
      for (;;){
        // Wait to receive, nRet = number of bytes received
        nRet=recv(Socket,Buffer,500,0);
        LastRecv=GetTickCount();
        if (nRet==SOCKET_ERROR){
          strcpy(Buffer,"nRecv failed: ");
          AddLastError(Buffer);
          strcat(Buffer,"\r\n");
          PRINT(Buffer);
          break;
        }
        else if (nRet==0){
          PRINT("Server has closed connection, file done!\r\n");
          break;
        }

        if (GotSoFar==0){
          char *PastHeader;
          // Skip header
          Buffer[nRet]=0;
          PastHeader=strstr(Buffer,"\r\n\r\n");
          if (PastHeader){
            PastHeader+=4;
            GotSoFar=nRet-(long)(PastHeader-Buffer);
            fwrite(PastHeader,1,GotSoFar,f);
          }
        }
        else{
          fwrite(Buffer,1,nRet,f);
          fflush(f);
        }
      }

      Len=ftell(f);
      fclose(f);
      f=NULL;

      if (Len!=FileSize){
        DeleteFile(ZipFileName);
        PRINT("Download failed, file size wrong.\r\n");
        nRet=SOCKET_ERROR;
      }
      else{
        nRet=0;
      }
    }
    else{
      sprintf(Buffer,"%s couldn't be open.",ZipFileName);
      PRINT(Buffer);
    }
  }
  else{
    strcpy(Buffer,"Send failed: ");
    AddLastError(Buffer);
    PRINT(Buffer);
  }
  Downloading=0;
  return nRet;
}
//---------------------------------------------------------------------------
DWORD GetIP(char *ServerName)
{
  char Buffer[500];
  LPHOSTENT lpHostEntry;

  sprintf(Buffer,"Getting IP address of %s\r\n",ServerName);
  PRINT(Buffer);
  lpHostEntry=gethostbyname(ServerName);
  if (lpHostEntry==NULL){
    strcpy(Buffer,"DNS lookup failed: ");
    AddLastError(Buffer);
    PRINT(Buffer);
    return 0;
  }

  return ((LPIN_ADDR)*lpHostEntry->h_addr_list)->S_un.S_addr;
}
//---------------------------------------------------------------------------
int nNewServ=0;

void GetFullURL(char *URL,char *Server,char *FolderURL,char *FileName)
{
  if (strstr(URL,"http://")==URL){ // Full URL

    strcpy(Server,URL+strlen("http://"));
    *strchr(Server,'/')=0;

    strcpy(FolderURL,URL);
    sprintf(FileName,"%s\\%s",RunDir,strrchr(FolderURL,'/')+1);
    *(strrchr(FolderURL,'/')+1)=0;
  }
  else{
    strcpy(Server,SteemNewServer[nNewServ]);
    strcpy(FolderURL,SteemNewURL[nNewServ]);
    *(strrchr(FolderURL,'/')+1)=0;
    sprintf(FileName,"%s\\%s",RunDir,URL);
  }
}
//---------------------------------------------------------------------------
bool CheckForUpdate()
{
  int nRet=0;
  bool Connected;
  DWORD ConnectedState=0;
  FILE *f;
  bool WSError=0;
  char SteemNewFN[MAX_PATH+1];
  MSG msg={
    NULL,WM_NULL,0,0	};

#ifdef __cplusplus
  Connected=0;
#else
  Connected=AlwaysOnline ? true:InternetGetConnectedState(&ConnectedState,0);
#endif

  if (Connected){
    HANDLE hThread;
    DWORD ThreadId;
    int errs=0;

    for (;true;nNewServ++){
      if (SteemNewServer[nNewServ]==NULL) break;

      if (ProxyURL[0]) IP_Addr=GetIP(ProxyURL);
      if (IP_Addr==0) IP_Addr=GetIP(SteemNewServer[nNewServ]);

      if (IP_Addr){
        LastRecv=GetTickCount();
        Downloading=true;
        hThread=CreateThread(NULL,0,DownloadSteemDotNew,(LPVOID)nNewServ,0,&ThreadId);

        SetTimer(MainWin,1,100,NULL);
        while (GetMessage(&msg,NULL,0,0)){
          TranslateMessage(&msg);
          DispatchMessage(&msg);

          if (msg.message==WM_TIMER){
            if (Downloading==0) break;

            if (GetTickCount()-LastRecv > (2 * 60000)) break;
          }
        }
        KillTimer(MainWin,1);

        if (Downloading){
          TerminateThread(hThread,0);
          CloseHandle(hThread);
          closesocket(Socket);
        }
        else{
          DWORD Code=SOCKET_ERROR;
          GetExitCodeThread(hThread,&Code);
          CloseHandle(hThread);
          if (Code==(DWORD)SOCKET_ERROR){
            errs++;
          }
          else{
            break;
          }
        }
      }
      else{
        errs++;
      }
    }
    if (errs>=nNewServ){
      WSError=true;
      nNewServ=0;
    }
    if (msg.message==WM_QUIT) return 0;
  }
  else{
    nOffline++;
  }

  sprintf(SteemNewFN,"%s\\steem.new",RunDir);
  f=fopen(SteemNewFN,"rb");
  if (f){
    char VersionText[200],*MessageStart,*VerEnd=NULL,*ZipFileNameEnd=NULL,*FileSizeEnd=NULL,*Description=NULL;
    DWORD FileSize=0;
    char *UpdateBuf=NULL,UpdateType[50];
    int UpdateLen;
    int DownloadedOkay=0;
    HANDLE hThread;
    DWORD ThreadId;

    fseek(f,0,SEEK_END);
    UpdateLen=ftell(f);
    fseek(f,0,SEEK_SET);

    UpdateBuf=(char*)malloc(UpdateLen+1);
    fread(UpdateBuf,1,UpdateLen,f);
    UpdateBuf[UpdateLen]=0;

    fclose(f);

    if (DebugBuild){
      strcpy(UpdateType,"[UPDATEDEBUG]\r\n");
    }
    else{
      strcpy(UpdateType,"[UPDATEUSER]\r\n");
    }
    MessageStart=strstr(UpdateBuf,UpdateType);
    if (MessageStart){
      memcpy(VersionText,MessageStart+strlen(UpdateType),200);
      VersionText[199]=0;
      VerEnd=strchr(VersionText,' ');
      if (VerEnd){
        *VerEnd=0;

        ZipFileNameEnd=strchr(VerEnd+1,' ');
        if (ZipFileNameEnd){
          *ZipFileNameEnd=0;

          FileSizeEnd=strchr(ZipFileNameEnd+1,'\r');
          if (FileSizeEnd){
            char *DescEnd;

            *FileSizeEnd=0;
            FileSize=atoi(ZipFileNameEnd+1);
            Description=MessageStart+strlen(UpdateType)+(((long)FileSizeEnd+2)-((long)VersionText));
            DescEnd=strstr(Description,"\r\n\r\n[");
            if (DescEnd) *DescEnd=0;
          }
        }
      }
    }
    if (VerEnd && ZipFileNameEnd && FileSizeEnd && Description && FileSize>=100000){
      int Choice;
      char ForbiddenVersion[20]={
        0			};

      WritePrivateProfileString("Update","LatestVersion",VersionText,INIFile);

      GetPrivateProfileString("Update","CurrentVersion","1",CurrentVersion,20,INIFile);
      GetPrivateProfileString("Update","DoNotDownloadVersion","0",ForbiddenVersion,20,INIFile);
      if (strcmpi(VersionText,CurrentVersion)!=0 && strcmpi(VersionText,ForbiddenVersion)!=0){
        // Version is different
        char ZipURL[500];

        strcpy(ZipURL,VerEnd+1);
        GetFullURL(ZipURL,SteemZipServer,SteemZipFolderURL,ZipFileName);

        if (access(ZipFileName,0)==0){
          WIN32_FIND_DATA wfd;
          HANDLE Find;

          Find=FindFirstFile(ZipFileName,&wfd);
          if (Find!=INVALID_HANDLE_VALUE){
            FindClose(Find);

            DownloadedOkay=(wfd.nFileSizeLow==FileSize);
          }
        }

        if (DownloadedOkay==0){
          Choice=NewSteemDialog(Description);
          if (Choice==0){
            int WinH;
            RECT rc;

            if (ProxyURL[0]) IP_Addr=GetIP(ProxyURL);
            if (IP_Addr==0) IP_Addr=GetIP(SteemZipServer);

            if (IP_Addr){
              Socket=HTTPConnect(IP_Addr);
              if (Socket!=INVALID_SOCKET){
                // Now connected to HTTP server
                InitCommonControls();

                SystemParametersInfo(SPI_GETWORKAREA,0,&rc,0);
                WinH=51+GetSystemMetrics(SM_CYSMCAPTION);
                ZipProgress=CreateWindowEx(0,"Steem Update Window","Downloading Steem...",WS_CAPTION,
                  rc.right-246,rc.bottom-WinH,246,WinH,NULL,NULL,Inst,NULL);

                ProgBar=CreateWindow(PROGRESS_CLASS,"",WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
                  10,10,150,25,ZipProgress,(HMENU)1000,Inst,NULL);

                Focus=CreateWindow("Button","Cancel",WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                  170,10,60,25,ZipProgress,(HMENU)1001,Inst,NULL);
                SendMessage(Focus,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),0);

                LastRecv=GetTickCount();
                Downloading=true;
                hThread=CreateThread(NULL,0,DownloadSteemDotZip,(void*)FileSize,0,&ThreadId);

                SetTimer(MainWin,1,100,NULL);
                while (GetMessage(&msg,NULL,0,0)){
                  TranslateMessage(&msg);
                  DispatchMessage(&msg);

                  if (msg.message==WM_TIMER){
                    if (Downloading==0) break;

                    if (GetTickCount()-LastRecv > (2 * 60000)) break;
                  }
                  else if (msg.message==WM_KEYDOWN){
                    if (msg.wParam==VK_RETURN) ButtonSelected=800;
                  }
                }
                KillTimer(MainWin,1);

                if (Downloading){
                  TerminateThread(hThread,SOCKET_ERROR);
                  if (f) fclose(f);
                  DeleteFile(ZipFileName);
                }
                else{
                  DWORD Code=SOCKET_ERROR;
                  GetExitCodeThread(hThread,&Code);
                  DownloadedOkay=(Code!=(DWORD)SOCKET_ERROR);
                  WSError=(Code==(DWORD)SOCKET_ERROR);
                }
                DestroyWindow(ZipProgress);
                ZipProgress=NULL;

                CloseHandle(hThread);

                closesocket(Socket);
              }
              else{
                WSError=true;
              }
            }
            else{
              WSError=true;
            }
            if (DownloadedOkay==0 && Silent){
              MessageBox(NULL,"There was an error downloading Steem, please try again later or manually "
                    "download and install Steem from http://steem.atari.org/","Download Failed",
                MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST | MB_TASKMODAL);
            }
          }
          else if (Choice==1){
            // Download some other time
          }
          else if (Choice==2){
            char Message[500];

            sprintf(Message,"Are you sure? If you select yes then you will have to download and "
                  "install version %s manually if you change your mind.",VersionText);

            if (MessageBox(NULL,Message,"Never Download or Install?",MB_ICONQUESTION | MB_YESNO |
                  MB_SETFOREGROUND | MB_TOPMOST | MB_TASKMODAL)==IDYES){
              WritePrivateProfileString("Update","DoNotDownloadVersion",VersionText,INIFile);
            }
          }
        }
        if (DownloadedOkay){
          if (InstallNewSteem(ZipFileName,VersionText)){
            char Message[500];

            WritePrivateProfileString("Update","CurrentVersion",VersionText,INIFile);
            WritePrivateProfileString("Update","DoNotDownloadVersion",VersionText,INIFile);

            sprintf(Message,"Steem version %s has been sucessfully installed. Would you like to run it now?",VersionText);
            StartSteem=(MessageBox(NULL,Message,"Install Complete",
              MB_YESNO | MB_ICONQUESTION | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND)==IDYES);
          }
        }
      }
      else{
        PRINT("There is no new version available\r\n");
      }
    }
    else{
      PRINT("Steem.new is not a valid update file\r\n");
    }
    if (DownloadedOkay==0 && Connected & CheckForPatches){
      PRINT("\r\nChecking patches\r\n");
      MessageStart=strstr(UpdateBuf,"[UPDATEPATCHES]\r\n");
      if (MessageStart){
        char CurVer[100]={
          0,0,0,0,0				}
        ,NewVer[100]={
          0,0,0,0,0				}
        ,*tp,
        PatchURL[300],PatchDir[MAX_PATH+1],TempBuf[MAX_PATH+1];
        int PatchZipSize;

        MessageStart+=strlen("[UPDATEPATCHES]\r\n");

        strcpy(PatchDir,RunDir);
        strcat(PatchDir,"\\patches");
				// Always install to patches in Steem folder.
//        GetPrivateProfileString("Patches","PatchDir",PatchDir,PatchDir,MAX_PATH,INIFile);

        sprintf(TempBuf,"%s\\version",PatchDir);
        f=fopen(TempBuf,"rb");
        if (f){
          fread(CurVer,1,100,f);
          fclose(f);
        }

        tp=strchr(MessageStart,' ');
        if (tp){
          *tp=0;
          strcpy(NewVer,MessageStart);

          MessageStart=tp+1;
          tp=strchr(MessageStart,' ');
        }
        if (tp){
          *tp=0;
          strcpy(PatchURL,MessageStart);

          MessageStart=tp+1;
          tp=strchr(MessageStart,'\r');
        }
        if (tp){
          *tp=0;
          PatchZipSize=atoi(MessageStart);
        }
        if (tp){
          if (strcmpi(NewVer,CurVer)==0){
            tp=NULL;
            PRINT("No new patches available\r\n");
          }
          else{
            GetFullURL(PatchURL,SteemZipServer,SteemZipFolderURL,ZipFileName);

            if (ProxyURL[0]) IP_Addr=GetIP(ProxyURL);
            if (IP_Addr==0) IP_Addr=GetIP(SteemZipServer);

            if (IP_Addr){
              Socket=HTTPConnect(IP_Addr);
              if (Socket!=INVALID_SOCKET){
                LastRecv=GetTickCount();
                Downloading=true;
                DownloadedOkay=0;
                hThread=CreateThread(NULL,0,DownloadPatchesDotZip,(void*)PatchZipSize,0,&ThreadId);

                SetTimer(MainWin,1,100,NULL);
                while (GetMessage(&msg,NULL,0,0)){
                  TranslateMessage(&msg);
                  DispatchMessage(&msg);

                  if (msg.message==WM_TIMER){
                    if (Downloading==0) break;

                    if (GetTickCount()-LastRecv > (2 * 60000)) break;
                  }
                }
                KillTimer(MainWin,1);

                if (Downloading){
                  TerminateThread(hThread,SOCKET_ERROR);
                  if (f) fclose(f);
                  DeleteFile(ZipFileName);
                }
                else{
                  DWORD Code=SOCKET_ERROR;
                  GetExitCodeThread(hThread,&Code);
                  DownloadedOkay=(Code!=(DWORD)SOCKET_ERROR);
                }
                CloseHandle(hThread);

                closesocket(Socket);

                if (DownloadedOkay){
                  HINSTANCE hZip;
                  PackStruct PackInfo;

                  if (LoadZipLib(&hZip)){
                    if (GetFirstInZip(ZipFileName,&PackInfo)==UNZIP_Ok){
                      int Ret=IDYES;
                      HWND SteemWin;

                      if (AskPatchInstall){
                        if (WaitForIdleSteem()!=ISTYPE_QUIT){
                          Ret=MessageBox(NULL,"There are new patches, would you like to install them now?",
                            "Patch Update",MB_ICONQUESTION | MB_YESNO | MB_SETFOREGROUND | MB_TOPMOST | MB_TASKMODAL);
                        }
                        else{
                          Ret=IDNO;
                          PostQuitMessage(0);
                        }
                      }
                      if (Ret==IDYES){
                        int n=0,nFiles=1,ErrVal;
                        char DestFile[MAX_PATH+1],SrcFile[MAX_PATH+1];
                        struct ZippedFileInfo{
                          char Path[MAX_PATH+1];
                          int HeaderOffset,Attr;
                        }
                        *pZfi;

                        while (GetNextInZip(&PackInfo)==UNZIP_Ok) nFiles++;
                        CloseZipFile(&PackInfo);

                        pZfi=(struct ZippedFileInfo*)malloc(sizeof(struct ZippedFileInfo)*nFiles);

                        GetFirstInZip(ZipFileName,&PackInfo);
                        do{
                          strcpy(pZfi[n].Path,PackInfo.FileName);
                          pZfi[n].HeaderOffset=PackInfo.HeaderOffset;
                          pZfi[n].Attr=PackInfo.Attr;
                          n++;
                        }
                        while (GetNextInZip(&PackInfo)==UNZIP_Ok);
                        CloseZipFile(&PackInfo);

                        CreateDirectory(PatchDir,NULL);
                        for (n=0;n<nFiles;n++){
                          sprintf(SrcFile,"%s\\%s",RunDir,pZfi[n].Path);
                          sprintf(DestFile,"%s\\%s",PatchDir,pZfi[n].Path);
                          if (DestFile[strlen(DestFile)-1]=='\\' || DestFile[strlen(DestFile)-1]=='/'){
                            DestFile[strlen(DestFile)-1]=0;
                            CreateDirectory(DestFile,NULL);
                          }
                          else{
                            UnzipFile(ZipFileName,DestFile,(WORD)pZfi[n].Attr,
                              (long)pZfi[n].HeaderOffset,NULL,0);
                          }
                        }
                        free(pZfi);
                        PRINT("Patches installed\r\n");

                        SteemWin=GetFirstSteemWindow();
                        while (SteemWin){
                          SendMessage(SteemWin,WM_USER+2,12345,0);
                          SteemWin=GetNextSteemWindow(SteemWin);
                        }
                      }
                    }
                    else{
                      WSError=true;
                    }
                    FreeLibrary(hZip);
                  }
                  DeleteFile(ZipFileName);
                }
                else{
                  WSError=true;
                }
              }
              else{
                WSError=true;
              }
            }
            else{
              WSError=true;
            }
          }
        }
        else{
          PRINT("Error reading patch update information.\r\n");
        }
      }
      else{
        PRINT("No patch update section.\r\n");
      }
    }
    free(UpdateBuf);
  }
  else{
    PRINT("Couldn't open Steem.new\r\n");
  }
  return WSError;
}
//---------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
  MSG msg;
  bool WSError=0;

  Inst=hInstance;

  CreateMutex(NULL,0,"SteemUpdate_Running");
  if (GetLastError()==ERROR_ALREADY_EXISTS) return 0;

  {
    char ComLine[500],*Proxy,*PortText;

    strcpy(ComLine,lpCmdLine);
    strlwr(ComLine);

    Silent=(strstr(ComLine,"silent")!=NULL);
    AlwaysOnline=(strstr(ComLine,"online")!=NULL);
    CheckForPatches=(strstr(ComLine,"nopatchcheck")==NULL);
    AskPatchInstall=(strstr(ComLine,"askpatchinstall")!=NULL);

    Proxy=strstr(ComLine,"proxy=");
    if (Proxy){
      char *ProxyEnd;

      Proxy+=6;
      ProxyEnd=strchr(Proxy,' ');
      if (ProxyEnd==NULL){
        strcpy(ProxyURL,Proxy);
      }
      else{
        int Len=ProxyEnd-Proxy;
        memcpy(ProxyURL,Proxy,Len);
        ProxyURL[Len]=0;
      }
    }
    PortText=strstr(ComLine,"port=");
    if (PortText){
      char PortNumText[200],*PortEnd;

      PortText+=5;
      PortEnd=strchr(PortText,' ');
      if (PortEnd==NULL){
        strcpy(PortNumText,PortText);
      }
      else{
        int Len=PortEnd-PortText;
        memcpy(PortNumText,PortText,Len);
        PortNumText[Len]=0;
      }
      Port=atoi(PortNumText);
    }
  }

  GetModuleFileName(NULL,RunDir,MAX_PATH);
  if (strrchr(RunDir,'\\')) *(strrchr(RunDir,'\\'))=0;

  strcpy(INIFile,RunDir);
  strcat(INIFile,"\\Steem.ini");
  if (access(INIFile,0)){  // Doesn't exist

    if (Silent==0){
      MessageBox(NULL,"Please put SteemUpdate.exe in the same directory as Steem.exe and try again.","Error",
        MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST);
    }
    return EXIT_FAILURE;
  }
  {
    char CurrentVersion[20]={
      0		};
    GetPrivateProfileString("Update","CurrentVersion","",CurrentVersion,20,INIFile);

    if (CurrentVersion[0]==0){
      if (Silent==0){
        MessageBox(NULL,"Please run Steem.exe before you run SteemUpdate.exe.","Error",
          MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST);
      }
      return EXIT_FAILURE;
    }
  }

  if (Silent){
    SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_LOWEST);
    SetTimer(NULL,3478,30000,NULL);
    while (GetMessage(&msg,NULL,0,0)){
      if (msg.message==WM_TIMER){
        bool DoStuff=true;
        HWND SteemWin=NULL;

        while ((SteemWin=GetNextSteemWindow(SteemWin))!=NULL){
          if (SendMessage(SteemWin,WM_USER+2,2323,0)){ //Steem is emulating

            DoStuff=0;
            break;
          }
        }
        if (DoStuff) break;
      }

      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    KillTimer(NULL,3478);
  }

  DebugBuild=GetPrivateProfileInt("Main","DebugBuild",0,INIFile);

  nRuns=GetPrivateProfileInt("Update","Runs",0,INIFile);
  nOffline=GetPrivateProfileInt("Update","Offline",0,INIFile);
  nWSError=GetPrivateProfileInt("Update","WSError",0,INIFile);

  {
    WNDCLASS wc;

    memset(&wc,0,sizeof(WNDCLASS));
    wc.style=CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc=(WNDPROC)WndProc;
    wc.hInstance=Inst;
    wc.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
    wc.lpszClassName="Steem Update Window";
    wc.lpszMenuName=NULL;
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.hIcon=LoadIcon(Inst,"APPICON");
    if (RegisterClass(&wc)==0) return 0;
  }

  MainWin=CreateWindow("Steem Update Window","Checking for Update",
    WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU,
    100,100,506,406+GetSystemMetrics(SM_CYCAPTION),
    NULL,NULL,Inst,NULL);
  if (MainWin==NULL) return 0;

  EditWin=CreateWindowEx(512,"Edit","",WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE,
    10,10,480,380,MainWin,(HMENU)100,Inst,NULL);
  OldEditWndProc=(WNDPROC)SetWindowLong(EditWin,GWL_WNDPROC,(long)EditNoCaretWndProc);
  SendMessage(EditWin,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),0);

  WsaData=InitWS();
  if (WsaData!=NULL){
    if (Silent==0){
      CentreWindow(MainWin,0);
      ShowWindow(MainWin,SW_SHOW);
    }

    WSError=CheckForUpdate();
    if (Silent==0){ // Winsock Error and not on silent running

      ShowWindow(MainWin,SW_SHOW);
      while (GetMessage(&msg,NULL,0,0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
    else{
      DestroyWindow(MainWin);
    }

    WSACleanup();
  }
  else{
    WSError=true;
  }
  if (WSError) nWSError++;

  {
    char Buf[100];
    WritePrivateProfileString("Update","Runs",itoa(nRuns+1,Buf,10),INIFile);
    WritePrivateProfileString("Update","Offline",itoa(nOffline,Buf,10),INIFile);
    WritePrivateProfileString("Update","WSError",itoa(nWSError,Buf,10),INIFile);
  }

  if (StartSteem){
    strcat(RunDir,"\\Steem.exe");
    WinExec(RunDir,SW_SHOW);
  }

  return msg.wParam;
}

