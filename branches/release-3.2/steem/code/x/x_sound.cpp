// Enough for 500ms at 100Khz in 16-bit stereo
#define X_SOUND_BUF_LEN_BYTES (50000*4)
BYTE x_sound_buf[X_SOUND_BUF_LEN_BYTES+16];

void XSoundInitBuffer(int,int);

#ifndef NO_PORTAUDIO
#include "x_sound_portaudio.cpp"
#define PA_ONLY(s) s
#else
#define PA_ONLY(s)
#endif

#ifndef NO_RTAUDIO
#include "x_sound_rtaudio.cpp"
#define RT_ONLY(s) s
#else
#define RT_ONLY(s)
#endif
//---------------------------------------------------------------------------
HRESULT InitSound()
{
  SoundRelease();
  PA_ONLY( if (x_sound_lib==XS_PA) PA_Init(); )
  RT_ONLY( if (x_sound_lib==XS_RT) Rt_Init(); )
  return DSERR_GENERIC;
}
//---------------------------------------------------------------------------
DWORD SoundGetTime()
{
  PA_ONLY( if (UseSound==XS_PA) return PA_GetTime(); )
  RT_ONLY( if (UseSound==XS_RT) return Rt_GetTime(); )
  return 0;
}
//---------------------------------------------------------------------------
void SoundRelease()
{
  PA_ONLY( PA_Release(); )
  RT_ONLY( Rt_Release(); )
  UseSound=0;
}
//---------------------------------------------------------------------------
void XSoundInitBuffer(int flatlevel1,int flatlevel2)
{
	BYTE *p=x_sound_buf;
	for (int i=0;i<sound_buffer_length;i++){
    if (sound_num_bits==8){
      *(p++)=BYTE(flatlevel1);
      if (sound_num_channels==2) *(p++)=BYTE(flatlevel2);
    }else{
      *LPWORD(p)=WORD(char(flatlevel1) << 8);p+=2;
      if (sound_num_channels==2){
        *LPWORD(p)=WORD(char(flatlevel2) << 8);p+=2;
      }
    }
	}
}
//---------------------------------------------------------------------------
HRESULT SoundStartBuffer(int flatlevel1,int flatlevel2)
{
  PA_ONLY( if (UseSound==XS_PA) return PA_StartBuffer(flatlevel1,flatlevel2); )
  RT_ONLY( if (UseSound==XS_RT) return Rt_StartBuffer(flatlevel1,flatlevel2); )
  return DSERR_GENERIC;
}
//---------------------------------------------------------------------------
bool SoundActive()
{
  PA_ONLY( if (UseSound==XS_PA) return PA_IsPlaying(); )
  RT_ONLY( if (UseSound==XS_RT) return Rt_IsPlaying(); )
  return 0;
}
//---------------------------------------------------------------------------
void SoundChangeVolume()
{
  PA_ONLY( if (UseSound==XS_PA) PA_ChangeVolume(); )
  RT_ONLY( if (UseSound==XS_RT) Rt_ChangeVolume(); )
}
//---------------------------------------------------------------------------
HRESULT Sound_Stop(bool Immediate)
{
  sound_record_close_file();
  sound_record=false;
  if (sound_internal_speaker) SoundStopInternalSpeaker();

  PA_ONLY( if (UseSound==XS_PA) return PA_Stop(Immediate); )
  RT_ONLY( if (UseSound==XS_RT) return Rt_Stop(Immediate); )
  return DS_OK;
}
//---------------------------------------------------------------------------
// Start and Len are byte values
HRESULT SoundLockBuffer(DWORD Start,DWORD Len,LPVOID *lpDatAdr1,DWORD *lpLockLength1,LPVOID *lpDatAdr2,DWORD *lpLockLength2)
{
  if (SoundActive()==0) return DSERR_GENERIC;

  DWORD buflen_bytes=X_SOUND_BUF_LEN_BYTES;
  *lpDatAdr1=x_sound_buf+Start;
  *lpLockLength1=Len;
  if (Start+Len>buflen_bytes){
    *lpDatAdr2=x_sound_buf;
    *lpLockLength2=(Start+Len)-buflen_bytes;
    *lpLockLength1-=*lpLockLength2;
  }else{
    *lpDatAdr2=NULL;
    *lpLockLength2=0;
  }
//  log_write(Str("PortAudio: Writing - ")+Start+" to "+(Start+*lpLockLength1));
//  if (*lpLockLength2) log_write(Str("PortAudio: Writing - ")+0+" to "+*lpLockLength2);
  return DS_OK;
}
//---------------------------------------------------------------------------
void SoundUnlock(LPVOID,DWORD,LPVOID,DWORD)
{
}
//---------------------------------------------------------------------------
HRESULT SoundError(char *,HRESULT DErr)
{
  SoundRelease();
  return DErr;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void internal_speaker_sound_by_period(int UNIX_ONLY( counter ))
{
#ifdef LINUX
	if (console_device!=-1) ioctl(console_device,KIOCSOUND,counter);
#endif
}
//---------------------------------------------------------------------------

