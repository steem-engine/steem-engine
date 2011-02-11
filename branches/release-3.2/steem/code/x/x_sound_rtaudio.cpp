int rt_sound_buf_pointer=0; // sample count
bool rt_started=0;

void Rt_FreeBuffer(bool);
//---------------------------------------------------------------------------
HRESULT Rt_Init()
{
  try{
    rt_audio=new RtAudio();
  }catch (RtError &error){
    error.printMessage();
    return DSERR_GENERIC;
  }
  UseSound=XS_RT;

  {
    if(sound_device_name.IsEmpty()){
      RtAudioDeviceInfo radi;
      int c=rt_audio->getDeviceCount();
      for (int i=1;i<=c;i++){
        radi=rt_audio->getDeviceInfo(i);
        if (radi.outputChannels>0){
          if (radi.isDefault){
            sound_device_name=radi.name.c_str();
          }
        }
      }
    }
  }
  return DS_OK;
}
//---------------------------------------------------------------------------
DWORD Rt_GetTime()
{
  return rt_sound_buf_pointer;
}
//---------------------------------------------------------------------------
void Rt_Release()
{
  Rt_FreeBuffer(true);
  delete rt_audio;
  rt_audio=NULL;
}
//---------------------------------------------------------------------------
int Rt_Callback(char *buffer,int bufferSize,void*)
{
  int pointer_byte=rt_sound_buf_pointer;
  pointer_byte%=sound_buffer_length; // Get sample count within buffer
  pointer_byte*=sound_bytes_per_sample; // Convert to bytes

  if (sound_num_bits==8){
    for (int i=0;i<bufferSize;i++){
      if(rt_unsigned_8bit){
        for(int a=0;a<sound_bytes_per_sample;a++)*(buffer++)=char(x_sound_buf[pointer_byte++]);
      }else{
        for(int a=0;a<sound_bytes_per_sample;a++)*(buffer++)=char(x_sound_buf[pointer_byte++] ^ 128);
      }
      if (pointer_byte>=X_SOUND_BUF_LEN_BYTES) pointer_byte-=X_SOUND_BUF_LEN_BYTES;
      rt_sound_buf_pointer++;
    }
  }else{
    for (int i=0;i<bufferSize;i++){
      for (int b=0;b<sound_bytes_per_sample;b++){
        *(buffer++)=x_sound_buf[pointer_byte++];
        if (pointer_byte>=X_SOUND_BUF_LEN_BYTES) pointer_byte-=X_SOUND_BUF_LEN_BYTES;
      }
      rt_sound_buf_pointer++;
    }

  }
  return 0;
}
//---------------------------------------------------------------------------
HRESULT Rt_StartBuffer(int flatlevel1,int flatlevel2)
{
  RtAudioDeviceInfo radi;
  if (rt_audio==NULL) return DSERR_GENERIC;

  Rt_FreeBuffer(true);

  int bufferSize=rt_buffer_size;  // 256 sample frames
  int device=0;        // 0 indicates the default or first available device

  RtAudioFormat format=RTAUDIO_SINT8;
  if (sound_num_bits==16) format=RTAUDIO_SINT16;

  {
    int c=rt_audio->getDeviceCount();
    for (int i=1;i<=c;i++){
      radi=rt_audio->getDeviceInfo(i);
      if (IsSameStr_I(radi.name.c_str(),sound_device_name)){
        device=i;
        break;
      }
    }
  }

  radi=rt_audio->getDeviceInfo(device);
  {
    int closest_freq=0,f;
    for (int n=0;n<int(radi.sampleRates.size());n++){
      f=radi.sampleRates[n];
      if (abs(sound_chosen_freq-f)<abs(sound_chosen_freq-closest_freq)){
        closest_freq=f;
      }
    }
    if (closest_freq==0) closest_freq=44100;
    sound_freq=closest_freq;
  }
  
  try{
    rt_audio->openStream(device,sound_num_channels,0,0,format,sound_freq,&bufferSize,rt_buffer_num);
  }catch (RtError &error){
    try{
      rt_audio->closeStream();
    }catch(...){}
    error.printMessage();
    return DSERR_GENERIC;
  }

  sound_buffer_length=X_SOUND_BUF_LEN_BYTES/sound_bytes_per_sample;
  XSoundInitBuffer(flatlevel1,flatlevel2);
  rt_sound_buf_pointer=0;
  try{
    rt_audio->setStreamCallback(Rt_Callback,0);
    rt_audio->startStream();
  }catch (RtError &error){
    try{
      rt_audio->stopStream();
    }catch(...){}
    try{
      rt_audio->closeStream();
    }catch(...){}
    error.printMessage();
    return DSERR_GENERIC;
  }

  rt_started=true;
  SoundBufStartTime=timeGetTime();
  sound_low_quality=(sound_freq<35000);
  WIN_ONLY( DSOpen=true; )

  return DS_OK;
}
//---------------------------------------------------------------------------
bool Rt_IsPlaying(){ return rt_started; }
//---------------------------------------------------------------------------
void Rt_ChangeVolume()
{
}
//---------------------------------------------------------------------------
void Rt_FreeBuffer(bool)
{
  if (rt_started==0) return;

  try{
    rt_audio->stopStream();
  }catch (...){};
  try{
    rt_audio->closeStream();
  }catch (...){};
  rt_started=0;
}
//---------------------------------------------------------------------------
HRESULT Rt_Stop(bool Immediate)
{
  Rt_FreeBuffer(Immediate);
  return DSERR_GENERIC;
}
//---------------------------------------------------------------------------

