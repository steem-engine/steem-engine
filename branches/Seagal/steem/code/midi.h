#ifdef IN_EMU
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)

#endif

int MidiGetStatusNumParams(BYTE);

#define MAX_SYSEX_BUFS 10

#define MIDI_NO_RUNNING_STATUS 1
#define MIDI_ALLOW_RUNNING_STATUS 0
EXT int MIDI_out_running_status_flag INIT(MIDI_NO_RUNNING_STATUS);
EXT int MIDI_in_running_status_flag INIT(MIDI_NO_RUNNING_STATUS);
EXT int MIDI_in_n_sysex INIT(2),MIDI_out_n_sysex INIT(2),MIDI_in_speed INIT(100);
EXT WORD MIDI_out_volume INIT(0xffff);
EXT DWORD MIDI_in_sysex_max INIT(64*1024),MIDI_out_sysex_max INIT(64*1024);

#ifdef WIN32
typedef struct{
  BYTE *pData;
  DWORD Len;
  MIDIHDR *pHdr;
}OutSysEx;
#endif

class TMIDIOut
{
private:
#ifdef WIN32
  HMIDIOUT Handle;

  BYTE MessBuf[8];
  int MessBufLen,ParamCount,nStatusParams;

  OutSysEx SysEx[MAX_SYSEX_BUFS+1];
  OutSysEx *pCurSysEx;
  int nSysExBufs;
  DWORD MaxSysExLen;
  MIDIHDR SysExHeader[MAX_SYSEX_BUFS];

  DWORD OldVolume;

#elif defined(UNIX)
#elif defined(BEOS)
#endif
public:
  TMIDIOut(int,int);
  bool AllocSysEx();
  void ReInitSysEx();
  ~TMIDIOut();

  void SendByte(BYTE);

  WIN_ONLY( int GetDeviceID(); )
  WIN_ONLY( bool IsOpen() { return Handle!=NULL; } )
  WIN_ONLY( bool FreeHeader(MIDIHDR *); )

  bool SetVolume(int);
  bool Mute();

  void Reset();

  Str ErrorText;
};
//---------------------------------------------------------------------------
typedef void MIDIINNOTEMPTYPROC();
typedef MIDIINNOTEMPTYPROC* LPMIDIINNOTEMPTYPROC;

class TMIDIIn
{
private:

DEBUG_ONLY(public:)
  CircularBuffer Buf;

#ifdef WIN32
  HMIDIIN Handle;

  MIDIHDR SysExHeader[MAX_SYSEX_BUFS];
  BYTE *SysExBuf[MAX_SYSEX_BUFS];  //[MIDI_SYSEX_BUFFER_SIZE+2]; // +2=Space for SOX and EOX
  volatile bool Killing;
  bool Started;
  DWORD MaxSysExLen;
  int RunningStatus,nSysExBufs;

  static void CALLBACK InProc(HMIDIIN,UINT,DWORD,DWORD,DWORD);
#elif defined(UNIX)
#endif
public:
  TMIDIIn(int,bool,LPMIDIINNOTEMPTYPROC=NULL);
  void AddSysExBufs(),RemoveSysExBufs();
  void ReInitSysEx();
  ~TMIDIIn();

  WIN_ONLY( int GetDeviceID(); )
  WIN_ONLY( bool IsOpen() { return Handle!=NULL; } )

  void Reset();

  bool Start();
  void Stop();

  bool AreBytesToCome() { return Buf.AreBytesInBuffer(); }
  BYTE ReadByte() { return Buf.ReadByte(); }
  void NextByte() { Buf.NextByte(); }

  LPMIDIINNOTEMPTYPROC NotEmptyProc;

  Str ErrorText;
};

#undef EXT
#undef INIT

