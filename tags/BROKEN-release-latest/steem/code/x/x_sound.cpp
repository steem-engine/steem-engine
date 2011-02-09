PortAudioStream *pa_out=NULL;
PaTimestamp pa_start_time;
bool pa_init=0;

void PAFreeBuffer(bool);
int PACallback(void*,void*,unsigned long,PaTimestamp,void*);

// Enough for 500ms at 100Khz in 16-bit stereo
#define PA_SOUND_BUF_LEN (50000*4)
BYTE pa_sound_buf[PA_SOUND_BUF_LEN+16];
//---------------------------------------------------------------------------
HRESULT InitSound()
{
  WIN_ONLY( if (Pa_LoadDLL()==paNoError) )
  {
    PaError err=Pa_Initialize();
    if (err==paNoError){
      pa_init=true;
#ifdef UNIX
      sound_device_name=Pa_GetDeviceInfo(Pa_GetDefaultOutputDeviceID())->name;
#endif
      return DS_OK;
    }else{
      log_write(Str("Sound error: Can't initialise PortAudio, error #")+int(err));
    }
  }
  UseSound=0;
  return DSERR_GENERIC;
}
//---------------------------------------------------------------------------
DWORD SoundGetTime()
{
  if (pa_out==NULL) return 0;
  return DWORD(Pa_StreamTime(pa_out));
}
//---------------------------------------------------------------------------
void SoundRelease()
{
  UseSound=0;
  if (pa_init){
    PAFreeBuffer(true);
    Pa_Terminate();
    WIN_ONLY( Pa_FreeDLL(); )
    pa_init=0;
  }
}
//---------------------------------------------------------------------------
HRESULT SoundStartBuffer(int flatlevel1,int flatlevel2)
{
  if (UseSound==0) return DSERR_GENERIC;

  if (pa_out) PAFreeBuffer(true);

  sound_buffer_length=PA_SOUND_BUF_LEN/sound_bytes_per_sample;

  PaSampleFormat pa_format=paUInt8;
  if (sound_num_bits==16) pa_format=paInt16;

  PaDeviceID out_dev=Pa_GetDefaultOutputDeviceID();
#ifdef UNIX
  int c=Pa_CountDevices();
  for (PaDeviceID i=0;i<c;i++){
    const PaDeviceInfo *pdev=Pa_GetDeviceInfo(i);
    if (pdev->maxOutputChannels>0){
      if (IsSameStr_I(pdev->name,sound_device_name)){
        out_dev=i;
        break;
      }
    }
  }
#endif
  const PaDeviceInfo *pdev=Pa_GetDeviceInfo(out_dev);
  if (pdev->numSampleRates==-1){
    // Range, make sure we fit inside
    sound_freq=max(min(sound_chosen_freq,int(pdev->sampleRates[1])),
                            int(pdev->sampleRates[0]));
  }else{
    // Values, pick closest
    sound_freq=-999999;
    for (int i=0;i<pdev->numSampleRates;i++){
      if (abs(int(pdev->sampleRates[i]-sound_chosen_freq))<
            abs(sound_freq-sound_chosen_freq)){
        sound_freq=int(pdev->sampleRates[i]);
      }
    }
    if (sound_freq<=0) sound_freq=sound_chosen_freq;
  }
  Pa_OpenStream(&pa_out,paNoDevice,0,0,NULL,out_dev,sound_num_channels,
                pa_format,NULL,sound_freq,pa_output_buffer_size,0,
                paDitherOff | paClipOff,PACallback,NULL);
  if (pa_out==NULL) return DSERR_GENERIC;

  sound_low_quality=(sound_freq<35000);
  WIN_ONLY( DSOpen=true; )
  UNIX_ONLY( sound_device=0; )

	BYTE *p=pa_sound_buf;
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
  Pa_StartStream(pa_out);
  pa_start_time=Pa_StreamTime(pa_out);
  return DS_OK;
}
//---------------------------------------------------------------------------
int PACallback(void*,void *pOutBuf,unsigned long Samples,PaTimestamp OutTime,void*)
{
  if (pOutBuf==NULL || Samples<=0) return 0;

  DWORD buf_idx=DWORD(OutTime-pa_start_time);
  DWORD buflen_bytes=PA_SOUND_BUF_LEN;
  DWORD bytes=Samples*sound_bytes_per_sample,bytes2=0;

  buf_idx%=buflen_bytes; // make sure there is no overflow when convert to bytes
  buf_idx*=sound_bytes_per_sample;
  buf_idx%=buflen_bytes;
  if (buf_idx+bytes > buflen_bytes){
    bytes2=(buf_idx+bytes)-buflen_bytes;
    bytes-=bytes2;
  }
  memcpy(pOutBuf,pa_sound_buf+buf_idx,bytes);
  if (bytes2) memcpy(LPBYTE(pOutBuf)+bytes,pa_sound_buf,bytes2);

//  log_write(Str("PortAudio: Reading - ")+buf_idx+" to "+(buf_idx+bytes));
//  if (bytes2) log_write(Str("PortAudio: Reading - ")+0+" to "+bytes2);
  return 0;
}
//---------------------------------------------------------------------------
void SoundChangeVolume()
{
}
//---------------------------------------------------------------------------
void PAFreeBuffer(bool Immediate)
{
  if (pa_out==NULL) return;

  if (Immediate){
    Pa_AbortStream(pa_out);
  }else{
    Pa_StopStream(pa_out);
  }
  Pa_CloseStream(pa_out);
  pa_out=NULL;
  WIN_ONLY( DSOpen=0; )
  UNIX_ONLY( sound_device=-1; )
}
//---------------------------------------------------------------------------
HRESULT Sound_Stop(bool Immediate)
{
  sound_record_close_file();
  sound_record=false;
  if (sound_internal_speaker) SoundStopInternalSpeaker();

  PAFreeBuffer(Immediate);
  return DS_OK;
}
//---------------------------------------------------------------------------
// Start and Len are byte values
HRESULT SoundLockBuffer(DWORD Start,DWORD Len,LPVOID *lpDatAdr1,DWORD *lpLockLength1,LPVOID *lpDatAdr2,DWORD *lpLockLength2)
{
  if (pa_out==NULL) return DSERR_GENERIC;

  DWORD buflen_bytes=PA_SOUND_BUF_LEN;
  *lpDatAdr1=pa_sound_buf+Start;
  *lpLockLength1=Len;
  if (Start+Len>buflen_bytes){
    *lpDatAdr2=pa_sound_buf;
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

