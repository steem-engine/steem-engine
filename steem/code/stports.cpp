/*---------------------------------------------------------------------------
FILE: stports.cpp
MODULE: emu
DESCRIPTION: Code to handle Steem's flexible port redirection system.
---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
// MIDI Port
//---------------------------------------------------------------------------
void agenda_midi_replace(int)
{
  if (MIDIPort.AreBytesToCome()){
    MIDIPort.NextByte();

    if (ACIA_MIDI.rx_not_read){
      // discard data and set overrun
      log_to_section(LOGSECTION_MIDI,"MIDI: Overrun on ACIA! Byte lost!");
      if (ACIA_MIDI.overrun!=ACIA_OVERRUN_YES) ACIA_MIDI.overrun=ACIA_OVERRUN_COMING;
    }else{
      ACIA_MIDI.data=MIDIPort.ReadByte();
      ACIA_MIDI.rx_not_read=true;
    }
    log_to_section(LOGSECTION_MIDI,EasyStr("MIDI: Fire ACIA interrupt"));
    if (ACIA_MIDI.rx_irq_enabled) ACIA_MIDI.irq=true;
    mfp_gpip_set_bit(MFP_GPIP_ACIA_BIT,!(ACIA_IKBD.irq || ACIA_MIDI.irq));

    if (MIDIPort.AreBytesToCome()) agenda_add(agenda_midi_replace,ACIAClockToHBLS(ACIA_MIDI.clock_divide,true),0);
  }
}
//---------------------------------------------------------------------------
void MidiInBufNotEmpty()
{
  agenda_add(agenda_midi_replace,ACIAClockToHBLS(ACIA_MIDI.clock_divide,true)+1,0); //+1 for middle of scanline
}
//---------------------------------------------------------------------------
// Parallel Port
//---------------------------------------------------------------------------
void ParallelInputNotify()
{
  agenda_add(agenda_check_centronics_interrupt,1,0);
}
//---------------------------------------------------------------------------
void ParallelOutputNotify()
{
  agenda_add(agenda_check_centronics_interrupt,1,0);
}
//---------------------------------------------------------------------------
void UpdateCentronicsBusyBit()
{
  if ((stick[N_JOY_PARALLEL_1] & BIT_4)==0){
    if (ParallelPort.IsOpen()){
      if (psg_reg[PSGR_MIXER] & BIT_7){  // Output
        // If there are bytes being sent then input is high
        mfp_gpip_set_bit(MFP_GPIP_CENTRONICS_BIT,ParallelPort.AreBytesToOutput());
      }else{                             // Input
        // If there are bytes being received then input is high
        mfp_gpip_set_bit(MFP_GPIP_CENTRONICS_BIT,ParallelPort.AreBytesToRead());
      }
    }else{
      mfp_gpip_set_bit(MFP_GPIP_CENTRONICS_BIT,true); // Always busy if port closed
    }
  }
}
//---------------------------------------------------------------------------
void agenda_check_centronics_interrupt(int)
{
  UpdateCentronicsBusyBit();
  if (ParallelPort.AreBytesToRead() || ParallelPort.AreBytesToOutput()){
    agenda_add(agenda_check_centronics_interrupt,2,0);
  }
}
//---------------------------------------------------------------------------
// Serial Port
//---------------------------------------------------------------------------
void agenda_serial_replace(int)
{
  if (UpdateBaud) RS232_CalculateBaud(bool(mfp_reg[MFPR_UCR] & BIT_7),mfp_get_timer_control_register(3),true);
  if (SerialPort.AreBytesToCome()){
    SerialPort.NextByte();

    if ((mfp_reg[MFPR_RSR] & BIT_0) /*Recv enable*/ &&
        (mfp_reg[MFPR_TSR] & b00000110)!=b00000110 /*Loopback*/ &&
        (mfp_reg[MFPR_RSR] & BIT_6)==0 /*No overrun*/){
      if ((mfp_reg[MFPR_RSR] & BIT_7 /*Buffer Full*/)==0){
        rs232_recv_byte=SerialPort.ReadByte();
        rs232_recv_overrun=0;
      }else{
        rs232_recv_overrun=true;
      }

      mfp_reg[MFPR_RSR]&=BYTE(~(BIT_2 /*Char in progress*/ | BIT_3 /*Break*/ |
                                BIT_4 /*Frame Error*/ |      BIT_5 /*Parity Error*/));
      mfp_reg[MFPR_RSR]|=BIT_7 /*Buffer Full*/;
      mfp_interrupt(MFP_INT_RS232_RECEIVE_BUFFER_FULL,ABSOLUTE_CPU_TIME);
    }

    if (SerialPort.AreBytesToCome()){
      mfp_reg[MFPR_RSR]|=BYTE(BIT_2); // Character in progress
      agenda_add(agenda_serial_replace,rs232_hbls_per_word,0);
    }else{
      mfp_reg[MFPR_RSR]&=BYTE(~BIT_2); // Character in progress
    }
  }
}
//---------------------------------------------------------------------------
void SerialInBufNotEmpty()
{
  mfp_reg[MFPR_RSR]|=BYTE(BIT_2); // Character in progress
  agenda_add(agenda_serial_replace,rs232_hbls_per_word+1,0); //+1 for middle of scanline
}
//---------------------------------------------------------------------------
TSTPort::TSTPort()
{
  MIDI_Out=NULL;
  MIDI_In=NULL;
  PCPort=NULL;
  PCPortIn=NULL;
  f=NULL;
  LoopBuf=NULL;

  Type=PORTTYPE_NONE;
#ifdef WIN32
  MIDIOutDevice=-2;
  MIDIInDevice=-1;
  COMNum=0;LPTNum=0;
#elif defined(UNIX)
  PortDev[TPORTIO_TYPE_SERIAL]="/dev/ttyS0";
  PortDev[TPORTIO_TYPE_PARALLEL]="/dev/lp0";
  PortDev[TPORTIO_TYPE_MIDI]="/dev/midi";
  PortDev[TPORTIO_TYPE_UNKNOWN]="/dev/null";
  for (int n=0;n<TPORTIO_NUM_TYPES;n++){
  	AllowIO[n][0]=true;
  	AllowIO[n][1]=0;
  	if (n==TPORTIO_TYPE_SERIAL || n==TPORTIO_TYPE_MIDI){ // Input only works on these
      AllowIO[n][1]=true;
    }
  }
#endif
}
//---------------------------------------------------------------------------
bool TSTPort::Create(Str &ErrorText,Str &ErrorTitle,bool DoAlert)
{
  Close();
  bool Running=(runstate==RUNSTATE_RUNNING);
  LPPORTIOINFIRSTBYTEPROC FirstByteInProc=NULL;
  LPPORTIOOUTFINISHEDPROC LastByteOutProc=NULL;
  if (&MIDIPort==this){
    FirstByteInProc=MidiInBufNotEmpty;
  }else if (&ParallelPort==this){
    FirstByteInProc=ParallelInputNotify;
    LastByteOutProc=ParallelOutputNotify;
  }else if (&SerialPort==this){
    FirstByteInProc=SerialInBufNotEmpty;
  }
  bool Error=0;

  switch (Type){
    case PORTTYPE_FILE:
      f=fopen(File,"ab");
      return true;
    case PORTTYPE_LOOP:
      LoopBuf=new CircularBuffer(PORT_LOOPBUFSIZE);
      return true;
  }
#ifdef WIN32
  EasyStr PortName=EasyStr("COM")+(COMNum+1);
  int AllowIn=true,AllowOut=true;
  switch (Type){
    case PORTTYPE_MIDI:
    {
      bool MIDIOutErr=0,MIDIInErr=0;
      if (MIDIOutDevice>-2){
        MIDI_Out=new TMIDIOut(MIDIOutDevice,WORD(Running ? MIDI_out_volume:0));
        if (MIDI_Out->IsOpen()==0) MIDIOutErr=true;
      }
      if (MIDIInDevice>-1){
        MIDI_In=new TMIDIIn(MIDIInDevice,Running,FirstByteInProc);
        if (MIDI_In->IsOpen()==0) MIDIInErr=true;
      }
      if (MIDIInErr && MIDIOutErr){
        ErrorTitle=T("MIDI Errors");
        ErrorText=T("MIDI Output Error")+"\n"+MIDI_Out->ErrorText+"\n\n";
        ErrorText+=T("MIDI Input Error")+"\n"+MIDI_In->ErrorText;
      }else if (MIDIInErr){
        ErrorTitle=T("MIDI Input Error");
        ErrorText=MIDI_In->ErrorText;
      }else if (MIDIOutErr){
        ErrorTitle=T("MIDI Output Error");
        ErrorText=MIDI_Out->ErrorText;
      }
      if (MIDIOutErr){ delete MIDI_Out; MIDI_Out=NULL; }
      if (MIDIInErr){ delete MIDI_In; MIDI_In=NULL; }
      Error=(MIDIOutErr || MIDIInErr);
      break;
    }
    case PORTTYPE_PARALLEL:
      PortName=EasyStr("LPT")+(LPTNum+1);
      AllowIn=comline_allow_LPT_input;
    case PORTTYPE_COM:
      PCPort=new TPortIO(PortName,AllowIn,AllowOut);
      break;
  }
#elif defined(UNIX)
  int PortIOType=GetPortIOType(Type);
  if (PortIOType>=0){
    if (Type==PORTTYPE_LAN){
      PCPort=new TPortIO(PortDev[PortIOType],true,0,PortIOType);
      PCPortIn=new TPortIO(LANPipeIn,0,true,PortIOType);
    }else{
      PCPort=new TPortIO(PortDev[PortIOType],AllowIO[PortIOType][0],AllowIO[PortIOType][1],PortIOType);
    }
  }
#endif
	if (PCPort){
    if (PCPort->IsOpen()==0) Error=true;
    if (PCPortIn) if (PCPortIn->IsOpen()==0) Error=true;
    if (Error){
      ErrorTitle=T("Port Error");
#ifdef WIN32
      ErrorText=T("Could not open port ")+PortName+". "+
                T("It may not exist or it could be in use by another program.");
#else
      Str BadDev;
      ErrorText="";
      if (PCPortIn){
        if (PCPort->IsOpen()==0){
          BadDev=PortDev[PortIOType];
        }else{
          BadDev=LANPipeIn;
        }
      }else if (AllowIO[PortIOType][0] || AllowIO[PortIOType][1]){
        BadDev=PortDev[PortIOType];
      }
      if (BadDev.NotEmpty()){
        ErrorText=T("Could not open device")+" "+BadDev+"\n\n"+
                  T("It may not exist, it could be in use by another program or you may not have permission to access it.");
      }
#endif
      delete PCPort;PCPort=NULL;
      if (PCPortIn) delete PCPortIn;
      PCPortIn=NULL;
    }else{
      PCPort->lpInFirstByteProc=FirstByteInProc;
      PCPort->lpOutFinishedProc=LastByteOutProc;
      PCPort->OutPause=(Running==0);
      PCPort->InPause=(Running==0);
      if (PCPortIn){
        PCPortIn->lpInFirstByteProc=FirstByteInProc;
        PCPortIn->InPause=(Running==0);
      }
    }
  }
  if (Running){
    if (this==&ParallelPort){
      UpdateCentronicsBusyBit();
    }else if (this==&SerialPort){
      SetDTR(psg_reg[PSGR_PORT_A] & BIT_4);
      SetRTS(psg_reg[PSGR_PORT_A] & BIT_3);
    }
  }
  if (DoAlert && Error && ErrorText.NotEmpty()){
    Alert(ErrorText,ErrorTitle,MB_ICONEXCLAMATION | MB_OK);
  }
  CheckResetDisplay();
  return Error==0;
}
//---------------------------------------------------------------------------
bool TSTPort::OutputByte(BYTE Byte)
{
  if (MIDI_Out) MIDI_Out->SendByte(Byte);
  if (f) { fputc(Byte,f);fflush(f); }
  if (PCPort) return PCPort->OutputByte(Byte);
  if (LoopBuf){
    LPPORTIOINFIRSTBYTEPROC FirstByteProc=NULL;
    if (&MIDIPort==this){
      FirstByteProc=MidiInBufNotEmpty;
    }else if (&SerialPort==this){
      FirstByteProc=SerialInBufNotEmpty;
    }
    bool FirstByte=(LoopBuf->AreBytesInBuffer()==0);
    bool RetVal=LoopBuf->AddByte(Byte);
    if (FirstByte && FirstByteProc) FirstByteProc();
    return RetVal;
  }
  return true;
}
//---------------------------------------------------------------------------
bool TSTPort::AreBytesToOutput()
{
  if (PCPort) return PCPort->AreBytesToOutput();

  return 0; // Instant output
}
//---------------------------------------------------------------------------
void TSTPort::StartOutput()
{
  if (MIDI_Out) MIDI_Out->SetVolume(MIDI_out_volume);
  if (PCPort) PCPort->OutPause=0;
}
//---------------------------------------------------------------------------
void TSTPort::StopOutput()
{
  if (MIDI_Out) MIDI_Out->SetVolume(0);
  if (PCPort) PCPort->OutPause=true;
}
//---------------------------------------------------------------------------
void TSTPort::Reset()
{
  if (MIDI_Out) MIDI_Out->Reset();
  if (MIDI_In) MIDI_In->Reset();
  if (LoopBuf) LoopBuf->Reset();
}
//---------------------------------------------------------------------------
void TSTPort::StartInput()
{
  if (MIDI_In) MIDI_In->Start();
  if (PCPortIn){
    PCPortIn->InPause=0;
  }else if (PCPort){
    PCPort->InPause=0;
  }
}
//---------------------------------------------------------------------------
void TSTPort::StopInput()
{
  if (MIDI_In) MIDI_In->Stop();
  if (PCPortIn){
    PCPortIn->InPause=true;
  }else if (PCPort){
    PCPort->InPause=true;
  }
}
//---------------------------------------------------------------------------
bool TSTPort::AreBytesToRead()
{
  if (MIDI_In) return MIDI_In->AreBytesToCome();
  if (PCPortIn) return PCPortIn->AreBytesToRead();
  if (PCPort) return PCPort->AreBytesToRead();
  if (LoopBuf) return LoopBuf->AreBytesInBuffer();
  return 0;
}
//---------------------------------------------------------------------------
void TSTPort::NextByte()
{
  if (MIDI_In) MIDI_In->NextByte();
  if (PCPortIn) PCPortIn->NextByte();
  if (PCPort) PCPort->NextByte();
  if (LoopBuf) LoopBuf->NextByte();
}
//---------------------------------------------------------------------------
BYTE TSTPort::ReadByte()
{
  if (MIDI_In) return MIDI_In->ReadByte();
  if (PCPortIn) return PCPortIn->ReadByte();
  if (PCPort) return PCPort->ReadByte();
  if (LoopBuf) return LoopBuf->ReadByte();
  return 0;
}
//---------------------------------------------------------------------------
void TSTPort::SetupCOM(int BaudRate,bool bXOn_XOff,int RTS,int DTR,bool bParity,BYTE ParityType,BYTE StopBits,BYTE WordLength)
{
  if (Type==PORTTYPE_COM && PCPort){
    PCPort->SetupCOM(BaudRate,bXOn_XOff,RTS,DTR,bParity,ParityType,StopBits,WordLength);
  }
}
//---------------------------------------------------------------------------
DWORD TSTPort::GetModemFlags()
{
  if (PCPort) return PCPort->GetModemFlags();
  if (IsOpen()) return MS_CTS_ON; // Clear to send
  return 0;
}
//---------------------------------------------------------------------------
bool TSTPort::SetDTR(bool Val)
{
  if (PCPort) return PCPort->SetDTR(Val);
  return 0;
}
//---------------------------------------------------------------------------
bool TSTPort::SetRTS(bool Val)
{
  if (PCPort) return PCPort->SetRTS(Val);
  return 0;
}
//---------------------------------------------------------------------------
bool TSTPort::StartBreak()
{
  if (PCPort) return PCPort->StartBreak();
  return 0;
}
//---------------------------------------------------------------------------
bool TSTPort::EndBreak()
{
  if (PCPort) return PCPort->EndBreak();
  return 0;
}
//---------------------------------------------------------------------------
#ifdef WIN32
int TSTPort::GetMIDIOutDeviceID()
{
  if (MIDI_Out) return MIDI_Out->GetDeviceID();
  return -999;
}
//---------------------------------------------------------------------------
int TSTPort::GetMIDIInDeviceID()
{
  if (MIDI_In) return MIDI_In->GetDeviceID();
  return -999;
}
#endif
//---------------------------------------------------------------------------
void TSTPort::Close()
{
  if (MIDI_Out){
    delete MIDI_Out; MIDI_Out=NULL;
  }
  if (MIDI_In){
    delete MIDI_In; MIDI_In=NULL;
  }
  if (PCPort){
    delete PCPort; PCPort=NULL;
  }
  if (PCPortIn){
    delete PCPortIn; PCPortIn=NULL;
  }
  if (f){
    fclose(f); f=NULL;
  }
  if (LoopBuf){
    delete LoopBuf; LoopBuf=NULL;
  }
  if (runstate==RUNSTATE_RUNNING){
    if (this==&ParallelPort) UpdateCentronicsBusyBit();
  }
  CheckResetDisplay();
}
//---------------------------------------------------------------------------
void PortsRunStart()
{
  MIDIPort.StartInput();     MIDIPort.StartOutput();
  ParallelPort.StartInput(); ParallelPort.StartOutput();
  SerialPort.StartInput();   SerialPort.StartOutput();

  RS232_CalculateBaud(bool(mfp_reg[MFPR_UCR] & BIT_7),mfp_get_timer_control_register(3),true);

  // Update external devices (Could have changed while stopped or reset may have happened)
	SerialPort.SetDTR(psg_reg[PSGR_PORT_A] & BIT_4);
	SerialPort.SetRTS(psg_reg[PSGR_PORT_A] & BIT_3);

  RS232_VBL();

  UpdateCentronicsBusyBit();
}
//---------------------------------------------------------------------------
void PortsRunEnd()
{
  MIDIPort.StopInput();     MIDIPort.StopOutput();
  ParallelPort.StopInput(); ParallelPort.StopOutput();
  SerialPort.StopInput();   SerialPort.StopOutput();
}
//---------------------------------------------------------------------------
void PortsOpenAll()
{
#ifndef ONEGAME
  Str ErrorText,ErrorTitle;
  log_to(LOGSECTION_INIT,"STARTUP: Opening MIDIPort");
  MIDIPort.Create(ErrorText,ErrorTitle,true);
  log_to(LOGSECTION_INIT,"STARTUP: Opening ParallelPort");
  ParallelPort.Create(ErrorText,ErrorTitle,true);
  log_to(LOGSECTION_INIT,"STARTUP: Opening SerialPort");
  SerialPort.Create(ErrorText,ErrorTitle,true);
  log_to(LOGSECTION_INIT,"STARTUP: SerialPort opened");
#ifdef UNIX
	UNIX_ONLY( XGUIUpdatePortDisplay(); )
#endif
#endif
}
//---------------------------------------------------------------------------

