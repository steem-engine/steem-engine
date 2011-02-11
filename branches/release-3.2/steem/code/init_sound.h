#ifdef IN_MAIN
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)
#endif

EXT HRESULT Sound_Start(),Sound_Stop(bool=0);
EXT HRESULT SoundStartBuffer(int,int);
EXT void SoundGetPosition(DWORD*,DWORD*);
EXT HRESULT SoundLockBuffer(DWORD,DWORD,LPVOID*,DWORD*,LPVOID*,DWORD*);
EXT void SoundUnlock(LPVOID,DWORD,LPVOID,DWORD);
WIN_ONLY( EXT Str SoundLogError(Str,HRESULT); )
EXT HRESULT SoundError(char*,HRESULT);
EXT void sound_record_open_file();
EXT void sound_record_close_file();
EXT DWORD SoundGetTime();
WIN_ONLY( EXT void SoundChangeVolume(); )
EXT bool SoundActive();

EXT bool DSOpen INIT(0);
EXT int UseSound INIT(0);

#ifdef UNIX
EXT void internal_speaker_sound_by_period(int);

EXT EasyStr sound_device_name; // INIT("/dev/dsp");
EXT int console_device INIT(-1);

EXT int rt_buffer_size INIT(256),rt_buffer_num INIT(4);

#endif

#if defined(UNIX) || (defined(USE_PORTAUDIO_ON_WIN32) && defined(WIN32))
EXT int pa_output_buffer_size INIT(128);
#endif

#ifdef IN_MAIN
HRESULT InitSound();
void SoundRelease();

#ifdef WIN32

SET_GUID(CLSID_DirectSound, 0x47d4d946, 0x62e8, 0x11cf, 0x93, 0xbc, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);
SET_GUID(IID_IDirectSound, 0x279AFA83, 0x4981, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60);

#ifndef USE_PORTAUDIO_ON_WIN32
HRESULT DSGetPrimaryBuffer();
HRESULT DSCreateSoundBuf();
IDirectSound *DSObj=NULL;
IDirectSoundBuffer *PrimaryBuf=NULL,*SoundBuf=NULL;
DWORD DS_SetFormat_freq;
bool DS_GetFormat_Wrong=0;
DSCAPS SoundCaps;
WAVEFORMATEX PrimaryFormat;

void CALLBACK DSStopBufferTimerProc(HWND,UINT,UINT,DWORD);
UINT DSStopBufferTimerID=0;
#endif

#endif


bool TrySound=true;

#ifdef UNIX

#define XS_PA 1
#define XS_RT 2

int x_sound_lib=XS_PA;

#ifndef NO_RTAUDIO
RtAudio *rt_audio=NULL;
int rt_unsigned_8bit=0;
#endif
#endif

#endif


#undef EXT
#undef INIT

