/*---------------------------------------------------------------------------
FILE: init_sound.cpp
MODULE: Steem
DESCRIPTION: The guts of Steem's sound output code, uses a DirectSound buffer
for output.
---------------------------------------------------------------------------*/

#define LOGSECTION LOGSECTION_SOUND
//---------------------------------------------------------------------------
void sound_record_open_file()
{
  if (wav_file) return;
  wav_file=fopen(WAVOutputFile.Text,"wb");
  if (wav_file==NULL){
    Alert(T("Could not open WAV file for writing"),T("WAV Recording Error"),MB_ICONEXCLAMATION);
    sound_record=false;
    return;
  }
  fprintf(wav_file,"RIFF    WAVEfmt ");
  fputc(16,wav_file);  fputc(0,wav_file);  fputc(0,wav_file);  fputc(0,wav_file); //size of header=16
  fputc(1,wav_file);  fputc(0,wav_file); //always
  for (int i=0;i<14;i++) fputc(0,wav_file); // Skip header (written when close)
  fprintf(wav_file,"data    ");

  // Need to put size of file - 44 at position 40 in file (as int in binary little endian)
  // Need to put size of file - 8 at position 4 in file (as int in binary little endian)
  // Need to put header of file at position 0x16 in file
}


void sound_record_close_file()
{
  if (!wav_file)return;
  fflush(wav_file);
  int length=ftell(wav_file);
  fseek(wav_file,4,SEEK_SET);
  SaveInt(length-8,wav_file);
  fseek(wav_file,40,SEEK_SET);
  SaveInt(length-44,wav_file);

  // Write out header
  fseek(wav_file,0x16,SEEK_SET);
  fputc(sound_num_channels,wav_file);  fputc(0,wav_file);
  SaveInt(sound_freq,wav_file);
  SaveInt(sound_freq*sound_bytes_per_sample,wav_file); //bytes per second
  fputc(sound_bytes_per_sample,wav_file);  fputc(0,wav_file);
  fputc(sound_num_bits,wav_file);  fputc(0,wav_file);

  fclose(wav_file);
  wav_file=NULL;
  sound_record=false;
  OptionBox.UpdateRecordBut();
}
//----------------------------------------------------------------
#ifdef WIN32
//---------------------------------------------------------------------------
#ifdef USE_PORTAUDIO_ON_WIN32

#include "x/x_sound.cpp"

#else
//---------------------------------------------------------------------------
BOOL CALLBACK DSEnumProc(LPGUID Guid,LPCSTR Desc,LPCSTR /* Mod */,LPVOID)
{
  log(Str("SOUND: Found device ")+Desc);
  DSDriverModuleList.Add((char*)Desc,(long)Guid);
  return TRUE;
}
//---------------------------------------------------------------------------
HRESULT InitSound()
{
  SetNotifyInitText("DirectSound");

  SoundRelease();

  HRESULT Ret;
  // Hey, this allows Steem to run even if there is no DSound.dll
  log("SOUND: Attempting to load dsound.dll");
  HINSTANCE hDSDll=LoadLibrary("dsound");
  if (hDSDll){
    typedef HRESULT WINAPI DSENUMPROC(LPDSENUMCALLBACK,LPVOID);
    typedef DSENUMPROC* LPDSENUMPROC;
    LPDSENUMPROC DSEnum=(LPDSENUMPROC)GetProcAddress(hDSDll,"DirectSoundEnumerateA");
    DSDriverModuleList.Sort=eslNoSort;
    log("SOUND: Attempting to enumerate devices");
    if (DSEnum!=NULL) DSEnum(DSEnumProc,NULL);
    log("SOUND: Freeing library");
    FreeLibrary(hDSDll);
  }

  log("SOUND: Initialising, creating DirectSound object");
  Ret=CoCreateInstance(CLSID_DirectSound,NULL,CLSCTX_ALL,IID_IDirectSound,(void**)&DSObj);
  if (Ret!=S_OK || DSObj==NULL){
    DSObj=NULL;

    EasyStr Err="Unknown error";
    switch (Ret){
      case REGDB_E_CLASSNOTREG:
        Err="The specified class is not registered in the registration database.";
        break;
      case E_OUTOFMEMORY:
        Err="Out of memory.";
        break;
      case E_INVALIDARG:
        Err="One or more arguments are invalid.";
        break;
      case E_UNEXPECTED:
        Err="An unexpected error occurred.";
        break;
      case CLASS_E_NOAGGREGATION:
        Err="This class cannot be created as part of an aggregate.";
        break;
    }
    Err=EasyStr("SOUND: CoCreateInstance error\n\n")+Err;
    log_write("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    log_write(Err);
    log_write("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
#ifndef ONEGAME
    MessageBox(NULL,Err,T("Steem Engine DirectSound Error"),
                    MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_TOPMOST);
#endif
    return ~DS_OK;
  }

  LPGUID Driver=NULL;
  EasyStr DSDriverModName=GetCSFStr("Options","DSDriverName","",INIFile);
  if (DSDriverModName.NotEmpty()){
    for (int i=0;i<DSDriverModuleList.NumStrings;i++){
      if (IsSameStr_I(DSDriverModuleList[i].String,DSDriverModName)){
        Driver=LPGUID(DSDriverModuleList[i].Data[0]);
        break;
      }
    }
  }
  log("SOUND: Initialising DirectSound object");
  Ret=DSObj->Initialize(Driver);
  if (Ret != DS_OK) return SoundError("DSObj Initialise Failed",Ret);

  log("SOUND: Calling SetCooperativeLevel");
  DSObj->SetCooperativeLevel(StemWin,DSSCL_PRIORITY);

  log("SOUND: Calling GetCaps");
  SoundCaps.dwSize=sizeof(SoundCaps);
  Ret=DSObj->GetCaps(&SoundCaps);
  if (Ret != DS_OK) return SoundError("GetCaps Failed",Ret);

#ifdef ENABLE_LOGFILE
  log("------ Sound capabilities: ------");
  log(EasyStr("dwSize=")+SoundCaps.dwSize);

  log(EasyStr("dwFlags=")+itoa(SoundCaps.dwFlags,d2_t_buf,2));
  if (SoundCaps.dwFlags & DSCAPS_PRIMARYMONO    ) log("    DSCAPS_PRIMARYMONO  ");
  if (SoundCaps.dwFlags & DSCAPS_PRIMARYSTEREO  ) log("    DSCAPS_PRIMARYSTEREO");
  if (SoundCaps.dwFlags & DSCAPS_PRIMARY8BIT    ) log("    DSCAPS_PRIMARY8BIT      ");
  if (SoundCaps.dwFlags & DSCAPS_PRIMARY16BIT   ) log("    DSCAPS_PRIMARY16BIT     ");
  if (SoundCaps.dwFlags & DSCAPS_CONTINUOUSRATE ) log("    DSCAPS_CONTINUOUSRATE   ");
  if (SoundCaps.dwFlags & DSCAPS_EMULDRIVER     ) log("    DSCAPS_EMULDRIVER       ");
  if (SoundCaps.dwFlags & DSCAPS_CERTIFIED      ) log("    DSCAPS_CERTIFIED        ");
  if (SoundCaps.dwFlags & DSCAPS_SECONDARYMONO  ) log("    DSCAPS_SECONDARYMONO    ");
  if (SoundCaps.dwFlags & DSCAPS_SECONDARYSTEREO) log("    DSCAPS_SECONDARYSTEREO  ");
  if (SoundCaps.dwFlags & DSCAPS_SECONDARY8BIT  ) log("    DSCAPS_SECONDARY8BIT    ");
  if (SoundCaps.dwFlags & DSCAPS_SECONDARY16BIT ) log("    DSCAPS_SECONDARY16BIT   ");

  log(EasyStr("dwMinSecondarySampleRate=")+SoundCaps.dwMinSecondarySampleRate);
  log(EasyStr("dwMaxSecondarySampleRate=")+SoundCaps.dwMaxSecondarySampleRate);
  log(EasyStr("dwPrimaryBuffers=")+SoundCaps.dwPrimaryBuffers);
  log(EasyStr("dwMaxHwMixingAllBuffers=")+SoundCaps.dwMaxHwMixingAllBuffers);
  log(EasyStr("dwMaxHwMixingStaticBuffers=")+SoundCaps.dwMaxHwMixingStaticBuffers);
  log(EasyStr("dwMaxHwMixingStreamingBuffers=")+SoundCaps.dwMaxHwMixingStreamingBuffers);
  log(EasyStr("dwFreeHwMixingAllBuffers=")+SoundCaps.dwFreeHwMixingAllBuffers);
  log(EasyStr("dwFreeHwMixingStaticBuffers=")+SoundCaps.dwFreeHwMixingStaticBuffers);
  log(EasyStr("dwFreeHwMixingStreamingBuffers=")+SoundCaps.dwFreeHwMixingStreamingBuffers);
  log(EasyStr("dwMaxHw3DAllBuffers=")+SoundCaps.dwMaxHw3DAllBuffers);
  log(EasyStr("dwMaxHw3DStaticBuffers=")+SoundCaps.dwMaxHw3DStaticBuffers);
  log(EasyStr("dwMaxHw3DStreamingBuffers=")+SoundCaps.dwMaxHw3DStreamingBuffers);
  log(EasyStr("dwFreeHw3DAllBuffers=")+SoundCaps.dwFreeHw3DAllBuffers);
  log(EasyStr("dwFreeHw3DStaticBuffers=")+SoundCaps.dwFreeHw3DStaticBuffers);
  log(EasyStr("dwFreeHw3DStreamingBuffers=")+SoundCaps.dwFreeHw3DStreamingBuffers);
  log(EasyStr("dwTotalHwMemBytes=")+SoundCaps.dwTotalHwMemBytes);
  log(EasyStr("dwFreeHwMemBytes=")+SoundCaps.dwFreeHwMemBytes);
  log(EasyStr("dwMaxContigFreeHwMemBytes=")+SoundCaps.dwMaxContigFreeHwMemBytes);
  log(EasyStr("dwUnlockTransferRateHwBuffers=")+SoundCaps.dwUnlockTransferRateHwBuffers);
  log(EasyStr("dwPlayCpuOverheadSwBuffers=")+SoundCaps.dwPlayCpuOverheadSwBuffers);
  log(EasyStr("dwReserved1=")+SoundCaps.dwReserved1);
  log(EasyStr("dwReserved2=")+SoundCaps.dwReserved2);
  log("---------------------------------");
#endif

  if (SoundCaps.dwMaxSecondarySampleRate<SOUND_DESIRED_LQ_FREQ-(SOUND_DESIRED_LQ_FREQ/5)){
    // Apparently cannot even achieve lowest possible frequency
    SoundCaps.dwMaxSecondarySampleRate=100000; //Ignore!
  }
  UseSound=1;
  return DS_OK;
}
//---------------------------------------------------------------------------
DWORD SoundGetTime()
{
  if (DSOpen==0) return 0;

#if defined(ENABLE_LOGFILE)==0 && defined(SHOW_WAVEFORM)==0
  DWORD play_cursor,write_cursor;
#endif
  DWORD s_time;

  if (sound_time_method<2){
    play_cursor=0,write_cursor=0;
    SoundBuf->GetCurrentPosition(&play_cursor,&write_cursor);
    play_cursor/=sound_bytes_per_sample;
    write_cursor/=sound_bytes_per_sample;
    DWORD cursor=(sound_time_method==0) ? play_cursor:write_cursor;
    if (cursor<psg_last_play_cursor) psg_time_of_start_of_buffer+=sound_buffer_length;
    s_time=psg_time_of_start_of_buffer+cursor;
    psg_last_play_cursor=cursor;
  }else{
    DWORD mSecs=timeGetTime()-SoundBufStartTime;
    s_time=(mSecs*sound_freq)/1000;
  }

#ifdef ENABLE_LOGFILE
  min_write_time=((write_cursor-play_cursor) MOD_PSG_BUF_LENGTH)+s_time;
#endif

  return s_time;
}
//---------------------------------------------------------------------------
HRESULT DSReleaseAllBuffers(HRESULT Ret=DS_OK)
{
  if (SoundBuf && sound_write_primary==0){
    SoundBuf->Stop();SoundBuf->Release();
  }
  if (PrimaryBuf){
    if (sound_write_primary) PrimaryBuf->Stop();
    PrimaryBuf->Release();
  }
  SoundBuf=NULL;
  PrimaryBuf=NULL;
  DSOpen=0;
  if (sound_write_primary && DSObj) DSObj->SetCooperativeLevel(StemWin,DSSCL_PRIORITY);

  return Ret;
}
//---------------------------------------------------------------------------
HRESULT DSGetPrimaryBuffer()
{
  HRESULT Ret;
  DSBUFFERDESC dsbd;

  DSReleaseAllBuffers();
  DSOpen=true; // This will be cleared if any error occurs

  ZeroMemory(&dsbd,sizeof(DSBUFFERDESC));
  dsbd.dwSize=sizeof(DSBUFFERDESC);
  dsbd.dwFlags=DSBCAPS_PRIMARYBUFFER;
  if (sound_write_primary) dsbd.dwFlags|=DSBCAPS_GETCURRENTPOSITION2;
  Ret=DSObj->CreateSoundBuffer(&dsbd,&PrimaryBuf,NULL);
  if (Ret!=DS_OK){
    LOG_ONLY( SoundLogError("SOUND: CreateSoundBuffer for Primary Failed\r\n\r\n",Ret); )
    return DSReleaseAllBuffers(Ret);
  }

  PrimaryFormat.wFormatTag=WAVE_FORMAT_PCM;
  PrimaryFormat.nChannels=sound_num_channels;
  PrimaryFormat.nSamplesPerSec=12000;
  PrimaryFormat.wBitsPerSample=sound_num_bits;
  PrimaryFormat.nBlockAlign=WORD(sound_bytes_per_sample);
  PrimaryFormat.nAvgBytesPerSec=PrimaryFormat.nSamplesPerSec*PrimaryFormat.nBlockAlign;
  PrimaryFormat.cbSize=0;
  PrimaryBuf->SetFormat(&PrimaryFormat);

  DS_GetFormat_Wrong=0;
  int desired_freq=sound_chosen_freq;
  LOOP{
    while (desired_freq>=20000){
      if (desired_freq==sound_comline_freq){ // force if comline freq
        DS_SetFormat_freq=sound_comline_freq;
        PrimaryFormat.nSamplesPerSec=DS_SetFormat_freq;
        PrimaryFormat.nAvgBytesPerSec=PrimaryFormat.nSamplesPerSec*PrimaryFormat.nBlockAlign;
        Ret=PrimaryBuf->SetFormat(&PrimaryFormat);
        log(EasyStr("SOUND: SetFormat to ")+DS_SetFormat_freq+"Hz, it "+LPSTR(Ret==DS_OK ? "succeeded.":"failed."));
        if (Ret==DS_OK){
          sound_freq=sound_comline_freq;
          DS_GetFormat_Wrong=true;
          break;
        }
      }

      DS_SetFormat_freq=min(DWORD(desired_freq),SoundCaps.dwMaxSecondarySampleRate);
      PrimaryFormat.nSamplesPerSec=DS_SetFormat_freq;
      PrimaryFormat.nAvgBytesPerSec=PrimaryFormat.nSamplesPerSec*PrimaryFormat.nBlockAlign;
      Ret=PrimaryBuf->SetFormat(&PrimaryFormat);
      log(EasyStr("SOUND: SetFormat to ")+DS_SetFormat_freq+"Hz, it "+LPSTR(Ret==DS_OK ? "succeeded.":"failed."));
      if (Ret==DS_OK) break;

      DS_SetFormat_freq=min(DWORD((desired_freq/1000)*1000),SoundCaps.dwMaxSecondarySampleRate);
      LOOP{
        PrimaryFormat.nSamplesPerSec=DS_SetFormat_freq;
        PrimaryFormat.nAvgBytesPerSec=PrimaryFormat.nSamplesPerSec*PrimaryFormat.nBlockAlign;
        if ((Ret=PrimaryBuf->SetFormat(&PrimaryFormat))==DS_OK) break;
        log(EasyStr("SOUND: Couldn't SetFormat to ")+DS_SetFormat_freq+"Hz");

        DS_SetFormat_freq-=500;
        // Fail if less than 4/5th of the desired frequency
        if (DS_SetFormat_freq < DWORD(desired_freq-(desired_freq/5 + 500))) break;
      }
      if (Ret==DS_OK) break;

      desired_freq*=4;
      desired_freq/=5;
    }

    //GetFormat
    Ret=PrimaryBuf->GetFormat(&PrimaryFormat,sizeof(PrimaryFormat),NULL);
    if (Ret!=DS_OK){
      sound_freq=DS_SetFormat_freq;
      DS_GetFormat_Wrong=true;
      log(EasyStr("SOUND: GetFormat for primary sound buffer failed, assuming ")+sound_freq+"Hz");
    }else{
      sound_freq=PrimaryFormat.nSamplesPerSec;
      log(EasyStr("SOUND: GetFormat for primary sound buffer returned ")+sound_freq+"Hz");
    }

    if (DS_GetFormat_Wrong) break;

    if (DWORD(sound_freq)>=(DS_SetFormat_freq-2500) && DWORD(sound_freq)<=(DS_SetFormat_freq+2500)){
      break;
    }else if (desired_freq<20000){
      log("   Sound card is a dirty liar! Ignoring what it says and restarting.");
      DS_GetFormat_Wrong=true;
      desired_freq=sound_chosen_freq;
    }else{
      log("   SetFormat failed or sound card is a dirty liar! Trying again.");
      desired_freq*=4;
      desired_freq/=5;
    }
  }

  if (sound_write_primary){
    DSBCAPS caps={sizeof(DSBCAPS)};
    Ret=PrimaryBuf->GetCaps(&caps);
    if (Ret!=DS_OK){
      LOG_ONLY( SoundLogError("SOUND: GetCaps for Primary Failed\r\n\r\n",Ret); )
      return DSReleaseAllBuffers(Ret);
    }
    // sound_buffer_length is in number of samples
    sound_buffer_length=caps.dwBufferBytes/sound_bytes_per_sample;
    if (DS_GetFormat_Wrong) sound_freq=DS_SetFormat_freq;
  }

  return DS_OK;
}
//---------------------------------------------------------------------------
HRESULT DSCreateSoundBuf()
{
  if (sound_write_primary){
    SoundBuf=PrimaryBuf;
    return DS_OK;
  }

  if (SoundBuf){
    SoundBuf->Stop();SoundBuf->Release();SoundBuf=NULL;
    DSOpen=0;
  }

  sound_buffer_length=DEFAULT_SOUND_BUFFER_LENGTH;

  HRESULT Ret;
  DSBUFFERDESC dsbd;
  WAVEFORMATEX wfx;

  wfx.wFormatTag=WAVE_FORMAT_PCM;
  wfx.nChannels=sound_num_channels;
  wfx.nSamplesPerSec=DWORD(DS_GetFormat_Wrong ? DS_SetFormat_freq:sound_freq);
  wfx.wBitsPerSample=sound_num_bits;
  wfx.nBlockAlign=WORD(sound_bytes_per_sample);
  wfx.nAvgBytesPerSec=wfx.nSamplesPerSec*wfx.nBlockAlign;
  wfx.cbSize=0;

  ZeroMemory(&dsbd,sizeof(DSBUFFERDESC));
  dsbd.dwSize=sizeof(DSBUFFERDESC);
  dsbd.dwFlags=DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_STICKYFOCUS;
  dsbd.dwBufferBytes=sound_buffer_length * sound_bytes_per_sample;
  dsbd.lpwfxFormat=&wfx;
  Ret=DSObj->CreateSoundBuffer(&dsbd,&SoundBuf,NULL);
  if (Ret!=DS_OK){
    if (DS_GetFormat_Wrong){
      wfx.nSamplesPerSec=sound_freq;
      wfx.nAvgBytesPerSec=wfx.nSamplesPerSec;

      ZeroMemory(&dsbd,sizeof(DSBUFFERDESC));
      dsbd.dwSize=sizeof(DSBUFFERDESC);
      dsbd.dwFlags=DSBCAPS_CTRLVOLUME |
                    DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_STICKYFOCUS;
      dsbd.dwBufferBytes=sound_buffer_length * sound_bytes_per_sample;
      dsbd.lpwfxFormat=&wfx;
      Ret=DSObj->CreateSoundBuffer(&dsbd,&SoundBuf,NULL);
    }
    if (Ret!=DS_OK){
      LOG_ONLY( SoundLogError("SOUND: CreateSoundBuffer for Secondry Buffer Failed\r\n\r\n",Ret); )
      return DSReleaseAllBuffers(Ret);
    }
  }else{
    // Successfully created a buffer at DS_SetFormat_freq so make output match it
    if (DS_GetFormat_Wrong) sound_freq=DS_SetFormat_freq;
  }

  DSBCAPS caps={sizeof(DSBCAPS)};
  if (SoundBuf->GetCaps(&caps)==DS_OK) sound_buffer_length=caps.dwBufferBytes/sound_bytes_per_sample;

  log(EasyStr("SOUND: Created secondry sound buffer at ")+wfx.nSamplesPerSec+"Hz");

  return DS_OK;
}
//---------------------------------------------------------------------------
void SoundRelease()
{
  UseSound=0;
  if (DSObj!=NULL){
    DSReleaseAllBuffers();
    DSObj->Release();DSObj=NULL;
  }
}
//---------------------------------------------------------------------------
HRESULT SoundStartBuffer(int flatlevel1,int flatlevel2)
{
  if (UseSound==0) return DSERR_GENERIC;

  void *DatAdr,*DatAdr2;
  DWORD LockLength,LockLength2;
  HRESULT Ret;

  if (DSStopBufferTimerID) KillTimer(NULL,DSStopBufferTimerID);
  DSStopBufferTimerID=0;

  DSReleaseAllBuffers();

  Ret=DSGetPrimaryBuffer();
  if (Ret!=DS_OK) return Ret;

  Ret=DSCreateSoundBuf();
  if (Ret!=DS_OK) return Ret;

  sound_low_quality=(sound_freq<35000);

  if (sound_write_primary){
    Ret=DSObj->SetCooperativeLevel(StemWin,DSSCL_WRITEPRIMARY);
    if (Ret!=DS_OK){
      LOG_ONLY( SoundLogError("SOUND: SetCooperativeLevel for DSSCL_WRITEPRIMARY failed\r\n\r\n",Ret); )
      return DSReleaseAllBuffers(Ret);
    }
  }

  Ret=SoundBuf->Lock(0,0,&DatAdr,&LockLength,&DatAdr2,&LockLength2,DSBLOCK_ENTIREBUFFER);
  if (Ret==DSERR_BUFFERLOST){
    Ret=SoundBuf->Restore();
    if (Ret==DS_OK) Ret=SoundBuf->Lock(0,0,&DatAdr,&LockLength,&DatAdr2,&LockLength2,DSBLOCK_ENTIREBUFFER);
  }
  if (Ret!=DS_OK){
    LOG_ONLY( SoundLogError("SOUND: Lock for Sound Buffer Failed\r\n\r\n",Ret); )
    return DSReleaseAllBuffers(Ret);
  }

  // Want to set the value to the highest byte
  BYTE *p=LPBYTE(DatAdr);
  BYTE *p_end=LPBYTE(DatAdr)+LockLength;
  int current_level=(sound_num_bits==8) ? 128:0;
  if (sound_click_at_start) current_level=flatlevel1;
  double v[2]={current_level,current_level};
  double inc[2]={double(flatlevel1-v[0])/600,double(flatlevel2-v[1])/600};
  while (p<p_end && p){
    if (sound_num_bits==8){
      *(p++)=BYTE(v[0]);
      if (sound_num_channels==2) *(p++)=BYTE(v[1]);
    }else{
      *LPWORD(p)=WORD(char(v[0]) << 8);p+=2;
      if (sound_num_channels==2){
        *LPWORD(p)=WORD(char(v[1]) << 8);p+=2;
      }
    }
    if (int(v[0])!=flatlevel1) v[0]+=inc[0];
    if (int(v[1])!=flatlevel2) v[1]+=inc[1];
  }

  SoundBuf->Unlock(DatAdr,LockLength,DatAdr2,LockLength2);

  SoundBuf->SetVolume(MaxVolume);

  SoundBuf->Play(0,0,DSBPLAY_LOOPING);

  SoundBufStartTime=timeGetTime();

  return DS_OK;
}
//---------------------------------------------------------------------------
bool SoundActive()
{
  return UseSound && DSOpen;
}
//---------------------------------------------------------------------------
HRESULT Sound_Stop(bool Immediate)
{
  sound_record_close_file();
  sound_record=false;
  if (sound_internal_speaker) SoundStopInternalSpeaker();

  if (Immediate || sound_click_at_start || sound_write_primary){
    DSReleaseAllBuffers();
  }else if (SoundBuf && DSOpen){
    // No rush, so stop click
    DWORD StartWrite=psg_time_of_next_vbl_for_writing % sound_buffer_length;
    StartWrite*=sound_bytes_per_sample;

    int play_cursor=0;
    SoundBuf->GetCurrentPosition((DWORD*)&play_cursor,NULL);
    play_cursor-=8;
    if (play_cursor<int(StartWrite)) play_cursor+=sound_buffer_length*sound_bytes_per_sample;

    LPVOID lpDat[2]={0,0};
    DWORD DatLen[2]={0,0};
    if (SoundLockBuffer(StartWrite,(play_cursor-StartWrite)-1,
                        &(lpDat[0]),&(DatLen[0]),&(lpDat[1]),&(DatLen[1]))==DS_OK){
      int target_level=BYTE((sound_num_bits==8) ? 128:0);
      WORD dma_l,dma_r;
      dma_sound_get_last_sample(&dma_l,&dma_r);
      int current[2]={(psg_voltage >> 8) + HIBYTE(dma_l),(psg_voltage >> 8) + HIBYTE(dma_r)};
      if (sound_num_bits==16){
        current[0]^=128;current[1]^=128;
      }
      double v[2]={(signed char)(current[0]),(signed char)(current[1])};
      double inc[2]={(target_level-v[0])/600,(target_level-v[1])/600};

      BYTE *p,*p_end;
      for (int n=0;n<2;n++){
        p=LPBYTE(lpDat[n]);
        p_end=p+DatLen[n];
        while (p<p_end && p){
          if (sound_num_bits==8){
            *(p++)=BYTE(v[0]);
            if (sound_num_channels==2) *(p++)=BYTE(v[1]);
          }else{
            *LPWORD(p)=WORD(char(v[0]) << 8);p+=2;
            if (sound_num_channels==2){
              *LPWORD(p)=WORD(char(v[1]) << 8);p+=2;
            }
          }
          if (int(v[0])!=target_level) v[0]+=inc[0];
          if (int(v[1])!=target_level) v[1]+=inc[1];
        }
      }
      SoundUnlock(lpDat[0],DatLen[0],lpDat[1],DatLen[1]);

      DSOpen=0;
      StartWrite+=600*sound_bytes_per_sample;
      // Find out how long it is until we reach the end of the ramp
      // and set up a timer routine to fire then.
      play_cursor=0;
      SoundBuf->GetCurrentPosition((DWORD*)&play_cursor,NULL);
      if (play_cursor>=int(StartWrite)) StartWrite+=sound_buffer_length*sound_bytes_per_sample;
      //                Milliseconds per byte * num bytes
      double ms_per_byte=double(1000.0/sound_freq)/sound_bytes_per_sample;
      UINT ms=(UINT)((StartWrite-play_cursor)*ms_per_byte);
      DSStopBufferTimerID=SetTimer(NULL,0,ms,DSStopBufferTimerProc);
    }else{
      DSReleaseAllBuffers();
    }
  }
  return DS_OK;
}
//---------------------------------------------------------------------------
void CALLBACK DSStopBufferTimerProc(HWND,UINT,UINT,DWORD)
{
  if (DSStopBufferTimerID){
    if (DSOpen==0 && SoundBuf) DSReleaseAllBuffers();
    KillTimer(NULL,DSStopBufferTimerID);
    DSStopBufferTimerID=0;
  }
}
//---------------------------------------------------------------------------
void SoundChangeVolume()
{
  if (SoundBuf){
    SoundBuf->Stop();
    SoundBuf->SetVolume(MaxVolume);
    SoundBuf->Play(0,0,DSBPLAY_LOOPING);
  }
}
//---------------------------------------------------------------------------
HRESULT SoundLockBuffer(DWORD Start,DWORD Len,LPVOID *lpDatAdr1,DWORD *lpLockLength1,LPVOID *lpDatAdr2,DWORD *lpLockLength2)
{
  if (DSOpen==0) return DSERR_GENERIC;

  HRESULT Ret=SoundBuf->Lock(Start,Len,lpDatAdr1,lpLockLength1,lpDatAdr2,lpLockLength2,0);
  if (Ret==DSERR_BUFFERLOST){
    log_write("SOUND: Restoring sound buffer");
    Ret=SoundBuf->Restore();
    if (Ret==DS_OK){
      Ret=SoundBuf->Play(0,0,DSBPLAY_LOOPING);
      if (Ret==DS_OK){
        Ret=SoundBuf->Lock(Start,Len,lpDatAdr1,lpLockLength1,lpDatAdr2,lpLockLength2,0);
      }
    }
  }
  return Ret;
}
//---------------------------------------------------------------------------
void SoundUnlock(LPVOID DatAdr1,DWORD LockLength1,LPVOID DatAdr2,DWORD LockLength2)
{
  if (SoundBuf) SoundBuf->Unlock(DatAdr1,LockLength1,DatAdr2,LockLength2);
}
//---------------------------------------------------------------------------
Str SoundLogError(Str Text,HRESULT DErr)
{
  switch (DErr){
    case DSERR_ALLOCATED:
      Text+="The request failed because resources, such as a priority level,\nwere already in use by another caller.";
      break;
    case DSERR_ALREADYINITIALIZED:
      Text+="The object is already initialized.";
      break;
    case DSERR_BADFORMAT:
      Text+="The specified wave format is not supported.";
      break;
    case DSERR_BUFFERLOST:
      Text+="The buffer memory has been lost and must be restored.";
      break;
    case DSERR_CONTROLUNAVAIL:
      Text+="The buffer control (volume, pan, and so on) requested\nby the caller is not available.";
      break;
    case DSERR_GENERIC:
      Text+="An undetermined error occurred inside the DirectSound subsystem.";
      break;
    case DSERR_INVALIDCALL:
      Text+="This function is not valid for the current state of this object.";
      break;
    case DSERR_INVALIDPARAM:
      Text+="An invalid parameter was passed to the returning function.";
      break;
    case DSERR_NOAGGREGATION:
      Text+="The object does not support aggregation.";
      break;
    case DSERR_NODRIVER:
      Text+="No sound driver is available for use.";
      break;
    case DSERR_NOINTERFACE:
      Text+="The requested COM interface is not available.";
      break;
    case DSERR_OTHERAPPHASPRIO:
      Text+="Another application has a higher priority level,\npreventing this call from succeeding";
      break;
    case DSERR_OUTOFMEMORY:
      Text+="The DirectSound subsystem could not allocate sufficient\nmemory to complete the caller's request.";
      break;
    case DSERR_PRIOLEVELNEEDED:
      Text+="The caller does not have the priority level required\nfor the function to succeed.";
      break;
    case DSERR_UNSUPPORTED:
      Text+="The function called is not supported at this time.";
      break;
  }

  log_write("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  log_write(Text);
  log_write("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  return Text;
}
//---------------------------------------------------------------------------
HRESULT SoundError(char *ErrorText,HRESULT DErr)
{
  SoundRelease();

  Str Err=SoundLogError(Str(ErrorText)+"\n\n",DErr);
  Err+=Str("\n\n")+T("Steem will not be able to output any sound until you restart the program. Would you like to permanently stop Steem trying to use DirectSound at startup?");
#ifndef ONEGAME
  int Ret=MessageBox(NULL,Err,T("Steem Engine DirectSound Error"),
                     MB_YESNO | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_TOPMOST);
  if (Ret==IDYES) WriteCSFStr("Options","NoDirectSound","1",INIFile);
  OptionBox.UpdateForDSError();
#endif
  return DErr;
}
//---------------------------------------------------------------------------
#endif
#endif

#ifdef UNIX
#include "x/x_sound.cpp"
#endif

#undef LOGSECTION


