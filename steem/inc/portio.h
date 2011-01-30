#ifndef TPORTIO_H
#define TPORTIO_H

#ifdef UNIX
//---------------------------------------------------------------------------
#define TPORTIO_NUM_TYPES 5
// Don't change these numbers!
#define TPORTIO_TYPE_SERIAL 0
#define TPORTIO_TYPE_PARALLEL 1
#define TPORTIO_TYPE_MIDI 2
#define TPORTIO_TYPE_UNKNOWN 3
#define TPORTIO_TYPE_PIPE 4
//---------------------------------------------------------------------------
#define RTS_CONTROL_DISABLE    0x00
#define RTS_CONTROL_ENABLE     0x01
#define RTS_CONTROL_HANDSHAKE  0x02
#define RTS_CONTROL_TOGGLE     0x03

#define DTR_CONTROL_DISABLE    0x00
#define DTR_CONTROL_ENABLE     0x01

#define ONESTOPBIT 0
#define ONE5STOPBITS 1
#define TWOSTOPBITS 2

#define EVENPARITY 2
#define ODDPARITY 1

#define MS_CTS_ON           ((DWORD)0x0010)
#define MS_DSR_ON           ((DWORD)0x0020)
#define MS_RING_ON          ((DWORD)0x0040)
#define MS_RLSD_ON          ((DWORD)0x0080)

#endif

#include "circularbuffer.h"

typedef void PORTIOINFIRSTBYTEPROC();
typedef void PORTIOOUTFINISHEDPROC();
typedef PORTIOINFIRSTBYTEPROC* LPPORTIOINFIRSTBYTEPROC;
typedef PORTIOOUTFINISHEDPROC* LPPORTIOOUTFINISHEDPROC;

class TPortIO
{
private:

#ifdef WIN32
  static DWORD __stdcall InThreadEntryPoint(void*);
  static DWORD __stdcall OutThreadEntryPoint(void*);

  HANDLE hCom,hInThread,hOutThread;
#else
  static void* InThreadFunc(void*);
  static void* OutThreadFunc(void*);

	int iCom;
	pthread_t iInThread,iOutThread;
	pthread_cond_t OutWaitCond;
	pthread_mutex_t OutWaitMutex;
	int Type;
#endif

  volatile bool InThreadClosed,OutThreadClosed;
  CircularBuffer InpBuf,OutBuf;
  volatile bool Outputting,Closing;
public:
#ifdef WIN32
  TPortIO(char* = NULL,bool=true,bool=true);
  int Open(char*,bool=true,bool=true);
#else
  TPortIO(char* = NULL,bool=true,bool=true,int=3);
  int Open(char*,bool=true,bool=true,int=3);
#endif
  ~TPortIO();
  void SetupCOM(int,bool,int,int,bool,BYTE,BYTE,BYTE);
  bool OutputByte(BYTE),OutputString(char*);
  DWORD GetModemFlags();
  BYTE ReadByte();
  void NextByte();
  bool AreBytesToRead();
  bool AreBytesToOutput();
  void Close();
  void SetPause(bool,bool);
  bool StartBreak(),EndBreak();
  bool SetDTR(bool),SetRTS(bool);

#ifdef WIN32
  HANDLE Handle();
  bool IsOpen();

  bool WinNT;
  static bool AlwaysUseNTMethod;
  HANDLE hOutEvent,hInEvent;
  OVERLAPPED OutOverlapStruct,InOverlapStruct;
  LPOVERLAPPED lpOutOverlapStruct,lpInOverlapStruct;
#elif defined(UNIX) || defined(BEOS)
  bool IsOpen();
#endif

  LPPORTIOINFIRSTBYTEPROC lpInFirstByteProc;
  LPPORTIOOUTFINISHEDPROC lpOutFinishedProc;
  volatile bool OutPause,InPause;
  volatile int OutCount,InCount;
};

#endif

