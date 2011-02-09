#define LOGSECTION LOGSECTION_MIDI
//---------------------------------------------------------------------------
int MidiGetStatusNumParams(BYTE StatByte)
{
  switch (StatByte >> 4){
    case b1000:case b1001:  // Voice/channel message
    case b1010:case b1011:
    case b1110:
      return 2;
    case b1100:case b1101:
      return 1;
    default:  // System message
      if (StatByte & BIT_3) return 0; // Real time message

      switch (StatByte & b00000111){
        case b0000:
        case b0110:
        case b0111:
          return 0;
        case b0011:
        case b0001: // MIDI Time Code
        case b0101: // Cable Select
          return 1;
        case b0010:
          return 2;
        default:    // Undefined
          return 2;
      }
  }
}

#ifdef WIN32
//---------------------------------------------------------------------------//
//                                TMIDIOut                                   //
//---------------------------------------------------------------------------//
TMIDIOut::TMIDIOut(int Device,int Volume)
{
  Handle=NULL;
  
  bool AllocFailed=0;
  if (AllocSysEx()==0){
    ErrorText=T("Unable to allocate enough memory for this MIDI device.");
    AllocFailed=true;
  }
  if (AllocFailed==0){
    Reset();
    Sleep(100);
    if ( midiOutOpen(&Handle,Device,0,0,CALLBACK_NULL)==MMSYSERR_NOERROR ){
      midiOutGetVolume(Handle,&OldVolume);
      SetVolume(Volume);
    }else{
      ErrorText=T("Failed to open the MIDI device, it may already be in use.");
      Handle=NULL;
      AllocFailed=true;
    }
  }
  if (AllocFailed){
    for (int n=0;n<nSysExBufs;n++){
      if (SysEx[n].pData){ delete[] SysEx[n].pData;SysEx[n].pData=NULL; }
    }
  }
}
//---------------------------------------------------------------------------
bool TMIDIOut::FreeHeader(MIDIHDR *pHdr)
{
  if (pHdr==NULL) return true;
  if (pHdr->lpData==NULL) return true; // Don't need to free it

  if (midiOutUnprepareHeader(Handle,pHdr,sizeof(MIDIHDR))!=MMSYSERR_NOERROR) return 0;

  pHdr->dwFlags=MHDR_DONE;
  pHdr->lpData=NULL;
  // Remove from list
  for (int n=0;n<nSysExBufs;n++) if (SysEx[n].pHdr==pHdr) SysEx[n].pHdr=NULL;
  return true;
}
//---------------------------------------------------------------------------
void TMIDIOut::SendByte(BYTE Val)
{
  if (Handle==NULL) return;

  bool SendBuffer=0,AddToBuffer=true;

  if ((Val & BIT_7)==0){ // Not status byte
    if (pCurSysEx==NULL){
      if (nStatusParams==-1){
        AddToBuffer=0; //Ignore all bytes until a status is sent after reset
      }else{
        if (ParamCount<=0){ // running status
          ParamCount=nStatusParams-1;
        }else{
          ParamCount--;
        }
        if (ParamCount<=0) SendBuffer=true;
      }
    }
  }else if (Val & BIT_7){ // Status byte
    if ( (Val & b11111000)==b11111000 ){  // Real time message
      midiOutShortMsg(Handle,Val);
      AddToBuffer=0;
    }else{
      if (pCurSysEx){
        if (pCurSysEx->pData[pCurSysEx->Len - 1]!=b11110111){
          pCurSysEx->pData[pCurSysEx->Len++]=b11110111; // Put an EOX on the end
        }

        MIDIHDR *pHdr=NULL;
        for (int n=0;n<MAX_SYSEX_BUFS;n++){
          if (SysExHeader[n].dwFlags & MHDR_DONE){
            if (FreeHeader(&(SysExHeader[n]))){
              pHdr=&(SysExHeader[n]);
              break;
            }
          }
        }

        if (pHdr){
          ZeroMemory(pHdr,sizeof(MIDIHDR));
          pHdr->lpData=(char*)(pCurSysEx->pData);
          pHdr->dwBufferLength=pCurSysEx->Len;
          pHdr->dwBytesRecorded=pCurSysEx->Len; // Shouldn't need to set this
          midiOutPrepareHeader(Handle,pHdr,sizeof(MIDIHDR));
          midiOutLongMsg(Handle,pHdr,sizeof(MIDIHDR));

          pCurSysEx->pHdr=pHdr;
        }else{
          log("MIDI: No sysex headers available, ignoring message!");
        }

        if (Val==b11110111) AddToBuffer=0;
        pCurSysEx=NULL;
      }
      if (Val==b11110000){
        // Find next free sysex buffer
        for (int n=0;n<nSysExBufs;n++){
          if (SysEx[n].pHdr){
            // FreeHeader will set SysEx[n].pHdr to NULL if it is freed successfully
            if (SysEx[n].pHdr->dwFlags & MHDR_DONE) FreeHeader(SysEx[n].pHdr);
          }
          if (SysEx[n].pHdr==NULL){
            pCurSysEx=&(SysEx[n]);
            pCurSysEx->Len=0;
            break;
          }
        }
        LOG_ONLY( if (pCurSysEx==NULL) log("MIDI: No sysex buffers available, ignoring message!"); )
      }else{
        if (AddToBuffer){
          int NumParams=MidiGetStatusNumParams(Val);
          if (NumParams>0){
            // Lose anything that has come before
            MessBufLen=0;
            nStatusParams=NumParams;
            ParamCount=NumParams;
          }else{
            midiOutShortMsg(Handle,Val);
            AddToBuffer=0;
          }
        }
      }
    }
  }
  if (AddToBuffer){
    if (pCurSysEx==NULL){
      if (MessBufLen<8){
        MessBuf[MessBufLen++]=Val;
      }else{
        log("MIDI: Out message buffer overflow!");
      }
    }else{
      if (pCurSysEx->Len < MaxSysExLen){
        pCurSysEx->pData[pCurSysEx->Len++]=Val;
      }else{
        log("MIDI: Out sysex buffer overflow!");
      }
    }
  }
  if (SendBuffer){
    switch (MessBufLen){
      case 1:
        midiOutShortMsg(Handle,MessBuf[0]);
        break;
      case 2:
        midiOutShortMsg(Handle,MessBuf[0] | (MessBuf[1] << 8));
        break;
      default:
        midiOutShortMsg(Handle,MessBuf[0] | (MessBuf[1] << 8) | (MessBuf[2] << 16));
        break;
    }
    MessBufLen=MIDI_out_running_status_flag;
  }
}
//---------------------------------------------------------------------------
int TMIDIOut::GetDeviceID()
{
  int DeviceID=-999;
  if (Handle) midiOutGetID(Handle,(UINT*)&DeviceID);
  return DeviceID;
}
//---------------------------------------------------------------------------
bool TMIDIOut::SetVolume(int Volume)
{
  return bool((Handle==NULL) ? 0:(midiOutSetVolume(Handle,MAKELONG(Volume,Volume))==MMSYSERR_NOERROR));
}
//---------------------------------------------------------------------------
bool TMIDIOut::Mute()
{
  return bool((Handle==NULL) ? 0:(midiOutSetVolume(Handle,0)==MMSYSERR_NOERROR));
}
//---------------------------------------------------------------------------
void TMIDIOut::Reset()
{
  MessBuf[0]=0;
  MessBufLen=0;
  ParamCount=0;
  nStatusParams=-1;
  pCurSysEx=NULL;
}
//---------------------------------------------------------------------------
bool TMIDIOut::AllocSysEx()
{
  MaxSysExLen=MIDI_out_sysex_max-64;
  nSysExBufs=MIDI_out_n_sysex+1;
  try{
    for (int n=0;n<MAX_SYSEX_BUFS;n++){
      SysExHeader[n].lpData=NULL;
      SysExHeader[n].dwFlags=MHDR_DONE;
    }
    for (int n=0;n<nSysExBufs;n++){
      SysEx[n].pData=NULL;
      SysEx[n].pHdr=NULL;
    }
    for (int n=0;n<nSysExBufs;n++) SysEx[n].pData=new BYTE[MaxSysExLen+1];
    return true;
  }catch(...){
    return 0;
  }
}
//---------------------------------------------------------------------------
void TMIDIOut::ReInitSysEx()
{
  if (Handle==NULL) return;

  midiOutReset(Handle);
  midiOutShortMsg(Handle,b11110111); // Send an EOX, just in case (shouldn't do any harm)
  for (int n=0;n<MAX_SYSEX_BUFS;n++) FreeHeader(&(SysExHeader[n]));
  for (int n=0;n<nSysExBufs;n++) if (SysEx[n].pData) delete[] SysEx[n].pData;

  if (pCurSysEx) nStatusParams=-1; // If currently sending sysex ignore rest
  pCurSysEx=NULL;
  AllocSysEx();
}
//---------------------------------------------------------------------------
TMIDIOut::~TMIDIOut()
{
  if (Handle==NULL) return;

  midiOutReset(Handle);
  midiOutShortMsg(Handle,b11110111); // Send an EOX, just in case (shouldn't do any harm)
  for (int n=0;n<MAX_SYSEX_BUFS;n++) FreeHeader(&(SysExHeader[n]));
  SetVolume(WORD(OldVolume));
  midiOutClose(Handle);

  for (int n=0;n<nSysExBufs;n++) if (SysEx[n].pData) delete[] SysEx[n].pData;
  Handle=NULL;
  Sleep(100);
}
//---------------------------------------------------------------------------//
//                                TMIDIIn                                    //
//---------------------------------------------------------------------------//
TMIDIIn::TMIDIIn(int Device,bool StartNow,LPMIDIINNOTEMPTYPROC NEP)
{
  Handle=NULL;
  NotEmptyProc=NEP;
  Killing=0;
  Started=0;

  MaxSysExLen=MIDI_in_sysex_max-64;
  nSysExBufs=MIDI_in_n_sysex;
  ZeroMemory(SysExBuf,sizeof(SysExBuf));

  bool AllocFailed=(Buf.Create(MaxSysExLen+10000)==0);
  try{
    for (int n=0;n<nSysExBufs;n++) SysExBuf[n]=new BYTE[MaxSysExLen+2];
  }catch(...){
    AllocFailed=true;
  }
  if (AllocFailed){
    ErrorText=T("Unable to allocate enough memory for this MIDI device.");
  }else{
    Reset();
    Sleep(100);
    if ( midiInOpen(&Handle,Device,(DWORD)InProc,(DWORD)this,CALLBACK_FUNCTION)==MMSYSERR_NOERROR ){
      if (StartNow) Start();
    }else{
      ErrorText=T("Failed to open the MIDI device, it may already be in use.");
      Handle=NULL;
      AllocFailed=true;
    }
  }
  if (AllocFailed){
    Buf.Destroy();
    for (int n=0;n<nSysExBufs;n++){
      if (SysExBuf[n]){ delete[] SysExBuf[n]; SysExBuf[n]=NULL; }
    }
    Reset();
  }
}
//---------------------------------------------------------------------------
void CALLBACK TMIDIIn::InProc(HMIDIIN Handle,UINT Msg,DWORD dwThis,DWORD MidiMess,DWORD)
{
  TMIDIIn *This=(TMIDIIn*)dwThis;

  if (This->Killing) return;

  BYTE *pData;
  DWORD DataLen=0;
  MIDIHDR *pSysExHdr=NULL;
  LOG_ONLY( bool Err=0; )
  switch (Msg){
    case MIM_ERROR:
      log(EasyStr("MIDI In: Invalid Short Message received - ")+HEXSl(MidiMess,8));
    case MIM_DATA:
    {
      pData=LPBYTE(&MidiMess);
      int nParams=MidiGetStatusNumParams(pData[0]);
      DataLen=1+nParams;
      if (MIDI_in_running_status_flag==MIDI_ALLOW_RUNNING_STATUS){
        if (This->RunningStatus==pData[0]){
          pData++;
          DataLen--;
        }else{
          if (nParams){
            This->RunningStatus=pData[0];
          }else{
            This->RunningStatus=0;
          }
        }
      }
      break;
    }

    case MIM_LONGERROR:
      LOG_ONLY( Err=true; )
    case MIM_LONGDATA:
      pSysExHdr=LPMIDIHDR(MidiMess);
      pData=LPBYTE(pSysExHdr->lpData);
      DataLen=pSysExHdr->dwBytesRecorded;
      This->RunningStatus=0;
      LOG_ONLY( if (Err) log(EasyStr("MIDI In: Invalid Long Message received - length=")+DataLen) );
      LOG_ONLY( if (Err==0) log(EasyStr("MIDI In: Long message received - length=")+DataLen) );
      CutPauseUntilSysEx_Time=0;

      if (DataLen==0 || pData[DataLen-1]!=b11110111) pData[DataLen++]=b11110111; // If no EOX add it

      if (pData[0]!=b11110000){
        pData--;
        pData[0]=b11110000;
        DataLen++;
      }
      LOG_ONLY( if (DataLen>This->MaxSysExLen-8) log("MIDI In: Large sysex message received, possible overflow.") );
      break;
  }
  if (DataLen){
    while (This->Buf.IsLocked()) Sleep(0);

    if (This->NotEmptyProc){
      if (This->Buf.AreBytesInBuffer()==0) This->NotEmptyProc();
    }
    This->Buf.AddBytes(pData,DataLen);

    if (pSysExHdr){
      midiInUnprepareHeader(Handle,pSysExHdr,sizeof(MIDIHDR));
      ZeroMemory(pSysExHdr,sizeof(MIDIHDR));
      pSysExHdr->lpData=(char*)pData;
      pSysExHdr->dwBufferLength=This->MaxSysExLen;
      pSysExHdr->dwFlags=0;
      midiInPrepareHeader(Handle,pSysExHdr,sizeof(MIDIHDR));
      midiInAddBuffer(Handle,pSysExHdr,sizeof(MIDIHDR));
    }
  }
}
//---------------------------------------------------------------------------
int TMIDIIn::GetDeviceID()
{
  if (Handle==NULL) return -999;

  int DeviceID=-999;
  midiInGetID(Handle,(UINT*)&DeviceID);
  return DeviceID;
}
//---------------------------------------------------------------------------
void TMIDIIn::Reset()
{
  if (Handle==NULL) return;

  bool WasStarted=Started;
  Stop();
  while (Buf.IsLocked()) Sleep(0);
  Buf.Reset();
  RunningStatus=0;
  if (WasStarted) Start();
}
//---------------------------------------------------------------------------
bool TMIDIIn::Start()
{
  if (Handle==NULL || Started) return Started;

  AddSysExBufs();
  Started = (midiInStart(Handle)==MMSYSERR_NOERROR);
  if (Started==0) Stop();
  return Started;
}
//---------------------------------------------------------------------------
void TMIDIIn::Stop()
{
  if (Handle==NULL || Started==0) return;

  Started=0;
  Killing=true;
  midiInStop(Handle);
  midiInReset(Handle);
  RemoveSysExBufs();
  Killing=0;
}
//---------------------------------------------------------------------------
void TMIDIIn::AddSysExBufs()
{
  if (Handle==NULL) return;

  for (int n=0;n<nSysExBufs;n++){
    if (SysExBuf[n]){
      ZeroMemory(&(SysExHeader[n]),sizeof(MIDIHDR));
      SysExHeader[n].lpData=(char*)(SysExBuf[n]+1);
      SysExHeader[n].dwBufferLength=MaxSysExLen;
      SysExHeader[n].dwFlags=0;
      midiInPrepareHeader(Handle,&(SysExHeader[n]),sizeof(MIDIHDR));
      midiInAddBuffer(Handle,&(SysExHeader[n]),sizeof(MIDIHDR));
    }
  }
}
//---------------------------------------------------------------------------
void TMIDIIn::RemoveSysExBufs()
{
  if (Handle==NULL) return;

  for (int n=0;n<nSysExBufs;n++){
    if (SysExBuf[n]) midiInUnprepareHeader(Handle,&(SysExHeader[n]),sizeof(MIDIHDR));
  }
}
//---------------------------------------------------------------------------
void TMIDIIn::ReInitSysEx()
{
  if (Handle==NULL) return;

  bool WasStarted=Started;
  Stop();
  for (int n=0;n<nSysExBufs;n++){
    if (SysExBuf[n]){
      delete[] SysExBuf[n];
      SysExBuf[n]=NULL;
    }
  }

  MaxSysExLen=MIDI_in_sysex_max-64;
  nSysExBufs=MIDI_in_n_sysex;
  try{
    for (int n=0;n<nSysExBufs;n++) SysExBuf[n]=new BYTE[MaxSysExLen+2];
  }catch(...){}

  if (WasStarted) Start();
}
//---------------------------------------------------------------------------
TMIDIIn::~TMIDIIn()
{
  if (Handle==NULL) return;

  Stop();
  midiInClose(Handle);

  for (int n=0;n<nSysExBufs;n++) if (SysExBuf[n]) delete[] SysExBuf[n];
}
//---------------------------------------------------------------------------
#endif

#ifdef UNIX
#include "x/x_midi.cpp"
#endif

#undef LOGSECTION

