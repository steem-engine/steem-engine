PortAudioStream *pa_out=NULL;
PaTimestamp pa_start_time;
bool pa_init=0;

void PA_FreeBuffer(bool);
int PA_Callback(void*,void*,unsigned long,PaTimestamp,void*);
//---------------------------------------------------------------------------
HRESULT PA_Init()
{
  WIN_ONLY( if (Pa_LoadDLL()==paNoError) )
  {
    PaError err=Pa_Initialize();
    if (err==paNoError){
      pa_init=true;
#ifdef UNIX
      if(sound_device_name.IsEmpty()){
        sound_device_name=Pa_GetDeviceInfo(Pa_GetDefaultOutputDeviceID())->name;
      }
#endif
      UseSound=XS_PA;
      return DS_OK;
    }else{
      printf("Pa_Initialize Error: %s\n",Pa_GetErrorText(err));
    }
  }
  return DSERR_GENERIC;
}
//---------------------------------------------------------------------------
DWORD PA_GetTime()
{
  if (pa_out==NULL) return 0;
  if (sound_time_method<2){
    return DWORD(Pa_StreamTime(pa_out)-pa_start_time);
  }else{
    DWORD mSecs=(timeGetTime()-SoundBufStartTime);
//    printf("SOUND: returning %i\n",(mSecs*sound_freq)/1000);
    return (mSecs*sound_freq)/1000;
  }
}
//---------------------------------------------------------------------------
void PA_Release()
{
  if (pa_init){
    PA_FreeBuffer(true);
    Pa_Terminate();
    WIN_ONLY( Pa_FreeDLL(); )
    pa_init=0;
  }
}
//---------------------------------------------------------------------------
HRESULT PA_StartBuffer(int flatlevel1,int flatlevel2)
{
  if (pa_init==0) return DSERR_GENERIC;

  if (pa_out) PA_FreeBuffer(true);

  sound_buffer_length=X_SOUND_BUF_LEN_BYTES/sound_bytes_per_sample;

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
    sound_freq=sound_chosen_freq;
    if (sound_freq>int(pdev->sampleRates[1])) sound_freq=int(pdev->sampleRates[1]);
    if (sound_freq<int(pdev->sampleRates[0])) sound_freq=int(pdev->sampleRates[0]);
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
  PaError err=Pa_OpenStream(&pa_out,paNoDevice,0,0,NULL,out_dev,sound_num_channels,
                pa_format,NULL,sound_freq,pa_output_buffer_size,0,
                paDitherOff | paClipOff,PA_Callback,NULL);
  if (pa_out==NULL){
    printf("Pa_OpenStream Error: %s\n",Pa_GetErrorText(err));
    return DSERR_GENERIC;
  }

  XSoundInitBuffer(flatlevel1,flatlevel2);

  err=Pa_StartStream(pa_out);
  if (err){
    PA_FreeBuffer(true);
    printf("Pa_StartStream Error: %s\n",Pa_GetErrorText(err));
    return DSERR_GENERIC;
  }


  pa_start_time=Pa_StreamTime(pa_out);
  SoundBufStartTime=timeGetTime();
  sound_low_quality=(sound_freq<35000);
  WIN_ONLY( DSOpen=true; )

  return DS_OK;
}
//---------------------------------------------------------------------------
bool PA_IsPlaying(){ return pa_out!=NULL; }
//---------------------------------------------------------------------------
int PA_Callback(void*,void *pOutBuf,unsigned long Samples,PaTimestamp OutTime,void*)
{
  if (pOutBuf==NULL || Samples==0) return 0;

  DWORD buf_idx=DWORD(OutTime-pa_start_time);
  DWORD buflen_bytes=X_SOUND_BUF_LEN_BYTES;
  DWORD bytes=Samples*sound_bytes_per_sample,bytes2=0;

  buf_idx%=buflen_bytes; // make sure there is no overflow when convert to bytes
  buf_idx*=sound_bytes_per_sample;
  buf_idx%=buflen_bytes;
  if (buf_idx+bytes > buflen_bytes){
    bytes2=(buf_idx+bytes)-buflen_bytes;
    bytes-=bytes2;
  }
  memcpy(pOutBuf,x_sound_buf+buf_idx,bytes);
  if (bytes2) memcpy(LPBYTE(pOutBuf)+bytes,x_sound_buf,bytes2);

//  log_write(Str("PortAudio: Reading - ")+buf_idx+" to "+(buf_idx+bytes)+" time="+SoundGetTime());
//  if (bytes2) log_write(Str("PortAudio: Reading - ")+0+" to "+bytes2+" time="+SoundGetTime());
  return 0;
}
//---------------------------------------------------------------------------
void PA_ChangeVolume()
{
}
//---------------------------------------------------------------------------
void PA_FreeBuffer(bool Immediate)
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
}
//---------------------------------------------------------------------------
HRESULT PA_Stop(bool Immediate)
{
  PA_FreeBuffer(Immediate);
  return DS_OK;
}
//---------------------------------------------------------------------------

