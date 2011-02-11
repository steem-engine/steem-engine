/*---------------------------------------------------------------------------
FILE: portio.cpp
MODULE: helper
DESCRIPTION: Cross-platform direct port input and output class.
---------------------------------------------------------------------------*/

#ifndef TPORTIO_CPP
#define TPORTIO_CPP

#include "portio.h"
#include <circularbuffer.cpp>

#ifdef UNIX
#include <pthread.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#endif

#ifndef TPORTIO_BUF_SIZE
#define TPORTIO_BUF_SIZE 8192
#endif

#ifndef WIN_ONLY
#ifdef WIN32
#define WIN_ONLY(s) s
#else
#define WIN_ONLY(s)
#endif
#endif

#ifndef UNIX_ONLY
#ifdef WIN32
#define UNIX_ONLY(s)
#else
#define UNIX_ONLY(s) s
#endif
#endif

WIN_ONLY( bool TPortIO::AlwaysUseNTMethod=true; )
//---------------------------------------------------------------------------
TPortIO::TPortIO(char *Port,bool AllowIn,bool AllowOut
#ifdef UNIX
                                                      ,int PortType
#endif
																																		)
{
#ifdef WIN32
  hCom=NULL;
  hInThread=NULL;
  hOutThread=NULL;

  if (AlwaysUseNTMethod){
    WinNT=true;
  }else{
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
    WinNT=(osvi.dwPlatformId==VER_PLATFORM_WIN32_NT);
  }
  hKern32=LoadLibrary("kernel32");
  pCancelIOProc=(LPCANCELIOPROC)GetProcAddress(hKern32,"CancelIo");
  if (hKern32==NULL || pCancelIOProc==NULL){
    WinNT=0;
  }

  if (WinNT){
    hInEvent=CreateEvent(NULL,true,0,NULL);
    hOutEvent=CreateEvent(NULL,true,0,NULL);
    ZeroMemory(&InOverlapStruct,sizeof(OVERLAPPED));
    InOverlapStruct.hEvent=hInEvent;
    lpInOverlapStruct=&InOverlapStruct;
    ZeroMemory(&OutOverlapStruct,sizeof(OVERLAPPED));
    OutOverlapStruct.hEvent=hOutEvent;
    lpOutOverlapStruct=&OutOverlapStruct;
  }else{
    hInEvent=NULL;
    hOutEvent=NULL;
    lpInOverlapStruct=NULL;
    lpOutOverlapStruct=NULL;
  }
#elif defined(UNIX)
	iCom=-1;
	iInThread=0;
	iOutThread=0;
#endif

  InThreadClosed=true;
  OutThreadClosed=true;
  Outputting=0;
  Closing=0;
  OutPause=0;
  InPause=0;
  OutCount=0;
  InCount=0;
  lpInFirstByteProc=NULL;
  lpOutFinishedProc=NULL;
  if (Port){
  	WIN_ONLY(  Open(Port,AllowIn,AllowOut); )
	 	UNIX_ONLY( Open(Port,AllowIn,AllowOut,PortType); )
  }
}
//---------------------------------------------------------------------------
TPortIO::~TPortIO()
{
  Close();
#ifdef WIN32
  if (hInEvent) CloseHandle(hInEvent);
  hInEvent=NULL;
  if (hOutEvent) CloseHandle(hOutEvent);
  hOutEvent=NULL;
#endif
}
//---------------------------------------------------------------------------
bool TPortIO::OutputByte(BYTE Byte)
{
  WIN_ONLY( if (hCom==NULL) return 0; )
  UNIX_ONLY( if (iCom==-1) return 0; )

  bool NoOverflow=OutBuf.AddByte(Byte);
  if (Outputting==0){
    // When outputting ends (or at the start) the byte read
    // is the old one, must advance
    OutBuf.NextByte();
    Outputting=true;
    WIN_ONLY( ResumeThread(hOutThread); )
#ifdef UNIX
 		pthread_mutex_lock(&OutWaitMutex);
 		pthread_cond_signal(&OutWaitCond);
 		pthread_mutex_unlock(&OutWaitMutex);
#endif
  }

  return NoOverflow;
}
//---------------------------------------------------------------------------
bool TPortIO::OutputString(char *Str)
{
  WIN_ONLY( if (hCom==NULL) return 0; )
  UNIX_ONLY( if (iCom==-1) return 0; )

  int Len=strlen(Str);
  for (int n=0;n<Len;n++){
    if (OutputByte(Str[n])==0) return 0;
  }
  return true;
}
//---------------------------------------------------------------------------
void TPortIO::Close()
{
  WIN_ONLY( if (hCom==NULL) return; )
  UNIX_ONLY( if (iCom==-1) return; )

  Closing=true;

#ifdef WIN32
  if (WinNT && pCancelIOProc) pCancelIOProc(hCom); // Cancel all current writes or reads

  // Make sure this dies quickly
  if (hInThread) SetThreadPriority(hInThread,THREAD_PRIORITY_HIGHEST);
#endif

  DWORD TimeOut=GetTickCount()+750;
  while (InThreadClosed==0 || OutThreadClosed==0){
		if (OutThreadClosed==0){
			// Wake up output thread, just in case it decided to sleep
#ifdef WIN32
		  if (hOutThread) ResumeThread(hOutThread);
#elif defined(UNIX)
			if (OutThreadClosed==0){
				pthread_mutex_lock(&OutWaitMutex);
				pthread_cond_signal(&OutWaitCond);
				pthread_mutex_unlock(&OutWaitMutex);
			}
#endif
		}

    Sleep(2);
    if (GetTickCount()>TimeOut) break;
  }

#ifdef WIN32
  if (InThreadClosed==0) TerminateThread(hInThread,0);
  CloseHandle(hInThread);hInThread=NULL;InThreadClosed=true;
  if (OutThreadClosed==0) TerminateThread(hOutThread,0);
  CloseHandle(hOutThread);hOutThread=NULL;OutThreadClosed=true;

  if (hCom){
    PurgeComm(hCom,PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
    CloseHandle(hCom);hCom=NULL;
  }
  if (hKern32) FreeLibrary(hKern32);
  hKern32=NULL;
  pCancelIOProc=NULL;
#else
  if (InThreadClosed==0) pthread_cancel(iInThread);
  iInThread=0;InThreadClosed=true;

	if (iOutThread){
	  if (OutThreadClosed==0) pthread_cancel(iOutThread);
		pthread_cond_destroy(&OutWaitCond);
		pthread_mutex_destroy(&OutWaitMutex);
	}
  iOutThread=0;OutThreadClosed=true;

	if (iCom!=-1){
#ifdef TCIFLUSH
		tcflush(iCom,TCIFLUSH);
#endif
#ifdef TCOFLUSH
		tcflush(iCom,TCOFLUSH);
#endif
		close(iCom);
	}
#endif

  Closing=0;

  InpBuf.Destroy();
  InCount=0;

  OutBuf.Destroy();
  Outputting=0;
  OutCount=0;
}
//---------------------------------------------------------------------------
#ifdef WIN32

int TPortIO::Open(char *PortName,bool AllowIn,bool AllowOut)
{
  if (hCom) Close();

  DWORD Flags=0;
  if (WinNT) Flags=FILE_FLAG_OVERLAPPED;
  hCom=CreateFile(PortName,GENERIC_READ | GENERIC_WRITE,0,NULL,
                    OPEN_EXISTING,Flags,NULL);
  if (hCom==INVALID_HANDLE_VALUE){
    hCom=NULL;
    return 1;
  }

  COMMTIMEOUTS ct={0,1,200,1,200}; //200 milliseconds
  SetCommTimeouts(hCom,&ct);

  SetupCOM(115200,0,RTS_CONTROL_DISABLE,DTR_CONTROL_DISABLE,0,NOPARITY,ONESTOPBIT,8);

  if (InpBuf.Create(TPORTIO_BUF_SIZE)==0){
    Close();
    return 1;
  }
  if (OutBuf.Create(TPORTIO_BUF_SIZE)==0){
    Close();
    return 1;
  }

  DWORD Id;
  if (AllowIn){
    InThreadClosed=0;
    hInThread=CreateThread(NULL,0,InThreadEntryPoint,this,0,&Id);
    if (hInThread==NULL){
      Close();
      return 1;
    }
    SetThreadPriority(hInThread,THREAD_PRIORITY_NORMAL);
  }

  if (AllowOut){
    OutThreadClosed=0;
    hOutThread=CreateThread(NULL,0,OutThreadEntryPoint,this,CREATE_SUSPENDED,&Id);
    if (hOutThread==NULL){
      Close();
      return 1;
    }
    SetThreadPriority(hOutThread,THREAD_PRIORITY_HIGHEST);
  }

  return 0;
}
//---------------------------------------------------------------------------
DWORD __stdcall TPortIO::InThreadEntryPoint(void *t)
{
  TPortIO *This=(TPortIO*)t;

  DWORD BytesRead;
  BYTE TempIn;
  while (This->Closing==0){
    if (This->InPause==0){
      BytesRead=0;
      if (This->WinNT) ResetEvent(This->hInEvent);
      ReadFile(This->hCom,&TempIn,1,&BytesRead,This->lpInOverlapStruct);
      if (This->WinNT){
        WaitForSingleObject(This->hInEvent,250);
        GetOverlappedResult(This->hCom,This->lpInOverlapStruct,&BytesRead,0);
      }
      if (BytesRead){
        bool FirstByte=(This->InpBuf.AreBytesInBuffer()==0);
        This->InpBuf.AddByte(TempIn);
        if (FirstByte){
          if (This->lpInFirstByteProc) This->lpInFirstByteProc();
        }
        This->InCount++;
      }else{
        if (This->WinNT && This->pCancelIOProc) This->pCancelIOProc(This->hCom);
      }
    }else{
      Sleep(50);
    }
  }
  This->InThreadClosed=true;
  return 0;
}
//---------------------------------------------------------------------------
DWORD __stdcall TPortIO::OutThreadEntryPoint(void *t)
{
  TPortIO *This=(TPortIO*)t;

  DWORD BytesWritten;
  BYTE TempOut;
  while (This->Closing==0){
    if (This->Outputting){
      if (This->OutPause==0){
        TempOut=This->OutBuf.ReadByte();
        BytesWritten=0;
        if (This->WinNT) ResetEvent(This->hOutEvent);
        WriteFile(This->hCom,&TempOut,1,&BytesWritten,This->lpOutOverlapStruct);
        if (This->WinNT){
          WaitForSingleObject(This->hOutEvent,250);
          GetOverlappedResult(This->hCom,This->lpOutOverlapStruct,&BytesWritten,0);
        }
        if (BytesWritten){
          if (This->OutBuf.AreBytesInBuffer()){
            This->OutBuf.NextByte();
          }else{
            This->Outputting=0;
          }
          This->OutCount++;
        }else{
          if (This->WinNT && This->pCancelIOProc) This->pCancelIOProc(This->hCom);
        }
      }else{
        Sleep(50);
      }
    }else{
      if (This->lpOutFinishedProc) This->lpOutFinishedProc();
      if (This->Outputting==0) SuspendThread(This->hOutThread);
    }
  }
  This->OutThreadClosed=true;
  return 0;
}
//---------------------------------------------------------------------------
bool TPortIO::StartBreak()
{
  if (hCom==NULL) return 0;

  return SetCommBreak(hCom);
}
//---------------------------------------------------------------------------
bool TPortIO::EndBreak()
{
  if (hCom==NULL) return 0;

  return ClearCommBreak(hCom);
}
//---------------------------------------------------------------------------
void TPortIO::SetupCOM(int BaudRate,bool bXOn_XOff,int RTS,int DTR,bool bParity,BYTE ParityType,BYTE StopBits,BYTE WordLength)
{
  if (hCom==NULL) return;

  DCB dcb;
  ZeroMemory(&dcb,sizeof(DCB));
  dcb.DCBlength=sizeof(DCB);
  GetCommState(hCom,&dcb);
  dcb.BaudRate=BaudRate;
  dcb.fBinary=true;
  dcb.fParity=bParity;
  dcb.fOutxCtsFlow=0;
  dcb.fOutxDsrFlow=0;
  dcb.fDtrControl=DTR;
  dcb.fDsrSensitivity=0;
  dcb.fTXContinueOnXoff=true;
  dcb.fOutX=bXOn_XOff;
  dcb.fInX=bXOn_XOff;
  dcb.fErrorChar=0;
  dcb.fNull=0;
  dcb.fRtsControl=RTS;
  dcb.fAbortOnError=0;
  dcb.ByteSize=WordLength;
  dcb.Parity=ParityType;
  dcb.StopBits=StopBits;
  SetCommState(hCom,&dcb);
}
//---------------------------------------------------------------------------
DWORD TPortIO::GetModemFlags()
{
  if (hCom==NULL) return 0;

  DWORD Flags=MS_CTS_ON; // Default to this if not available
  GetCommModemStatus(hCom,&Flags);
  return Flags;
}
//---------------------------------------------------------------------------
bool TPortIO::SetDTR(bool Val)
{
  if (hCom==NULL) return 0;

  return EscapeCommFunction(hCom,DWORD(Val ? SETDTR:CLRDTR));
}
//---------------------------------------------------------------------------
bool TPortIO::SetRTS(bool Val)
{
  if (hCom==NULL) return 0;

  return EscapeCommFunction(hCom,DWORD(Val ? SETRTS:CLRRTS));
}
//---------------------------------------------------------------------------
HANDLE TPortIO::Handle() { return hCom; }
bool TPortIO::IsOpen() { return hCom!=NULL; }
//---------------------------------------------------------------------------
#endif

BYTE TPortIO::ReadByte() { return InpBuf.ReadByte(); }
void TPortIO::NextByte() { InpBuf.NextByte(); }
bool TPortIO::AreBytesToRead() { return InpBuf.AreBytesInBuffer(); }
bool TPortIO::AreBytesToOutput() { return Outputting; }

#ifdef UNIX
bool TPortIO::IsOpen() { return iCom!=-1; }
#include "x/x_portio.cpp"
#endif

#endif

