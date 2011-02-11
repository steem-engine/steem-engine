/*---------------------------------------------------------------------------
FILE: rs232.cpp
MODULE: emu
DESCRIPTION: Serial port emulation.
---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
// Send this to modem to test ATDT1471\r
//---------------------------------------------------------------------------
void RS232_VBL(int)
{
  DWORD Flags=SerialPort.GetModemFlags();
  // If flag is on bit=0
  mfp_gpip_set_bit(MFP_GPIP_CTS_BIT,bool((Flags & MS_CTS_ON) ? 0:true));
  mfp_gpip_set_bit(MFP_GPIP_DCD_BIT,bool((Flags & MS_RLSD_ON) ? 0:true));
  mfp_gpip_set_bit(MFP_GPIP_RING_BIT,bool((Flags & MS_RING_ON) ? 0:true));
  if (SerialPort.IsPCPort()){
    agenda_delete(RS232_VBL);
    agenda_add(RS232_VBL,int((shifter_freq==MONO_HZ) ? 14:6),0);
  }
}
//---------------------------------------------------------------------------
BYTE RS232_ReadReg(int Reg)
{
  switch (Reg){
    case MFPR_RSR:
      if (rs232_recv_overrun==0) mfp_reg[MFPR_RSR]&=BYTE(~BIT_6);
      break;
    case MFPR_TSR:
      mfp_reg[MFPR_RSR]&=BYTE(~BIT_6); // Clear underrun
      break;
    case MFPR_UDR:
      //added 22/5/2000
      mfp_reg[MFPR_RSR]&=BYTE(~BIT_7); //clear RX buffer full
      if (rs232_recv_overrun){
        mfp_reg[MFPR_RSR]|=BIT_6;
        rs232_recv_overrun=0;
        if (mfp_interrupt_enabled[MFP_INT_RS232_RECEIVE_ERROR]){
          mfp_interrupt(MFP_INT_RS232_RECEIVE_ERROR,ABSOLUTE_CPU_TIME);
        }else{
          mfp_interrupt(MFP_INT_RS232_RECEIVE_BUFFER_FULL,ABSOLUTE_CPU_TIME);
        }
      }
      return rs232_recv_byte;
  }
  return mfp_reg[Reg];
}
//---------------------------------------------------------------------------
void RS232_CalculateBaud(bool Div16,BYTE cr,bool SetBaudNow)
{
  if (cr){ //Timer D running
    int hbls_per_second=int((shifter_freq==MONO_HZ) ? HBLS_PER_SECOND_MONO:HBLS_PER_SECOND_AVE);
    if (Div16){
      rs232_hbls_per_word=(mfp_timer_prescale[cr]*BYTE_00_TO_256(mfp_reg[MFPR_TDDR]))*16*
                              rs232_bits_per_word*hbls_per_second/MFP_CLK_EXACT;
    }else{
      rs232_hbls_per_word=max((mfp_timer_prescale[cr]*BYTE_00_TO_256(mfp_reg[MFPR_TDDR]))*
                            rs232_bits_per_word*hbls_per_second/MFP_CLK_EXACT,1);
    }
    if (SerialPort.IsPCPort()){
      if (SetBaudNow==0){
        UpdateBaud=true;
        return;
      }
      BYTE UCR=mfp_reg[MFPR_UCR];
      double Baud=double(19200*4)/(mfp_timer_prescale[cr]*BYTE_00_TO_256(mfp_reg[MFPR_TDDR]));
      if (Div16==0) Baud*=16;
      BYTE StopBits=ONESTOPBIT;
      switch (UCR & b00011000){
        case b00010000:StopBits=ONE5STOPBITS;break;
        case b00011000:StopBits=TWOSTOPBITS; break;
      }
      int RTS=int((psg_reg[PSGR_PORT_A] & BIT_3) ? RTS_CONTROL_ENABLE:RTS_CONTROL_DISABLE);
      int DTR=int((psg_reg[PSGR_PORT_A] & BIT_4) ? DTR_CONTROL_ENABLE:DTR_CONTROL_DISABLE);
      SerialPort.SetupCOM(DWORD(Baud),0,RTS,DTR,
                          bool(UCR & BIT_2),BYTE((UCR & BIT_1) ? EVENPARITY:ODDPARITY),
                          StopBits,BYTE(8-((UCR & b01100000) >> 5)));
      UpdateBaud=0;
    }
  }else{
    rs232_hbls_per_word=80000000;
  }
}
//---------------------------------------------------------------------------
void RS232_WriteReg(int Reg,BYTE NewVal)
{
  switch (Reg){
    case MFPR_UCR:
    {
      int old_bpw=rs232_bits_per_word;
      rs232_bits_per_word=1+BYTE(8-((NewVal & b01100000) >> 5))+1;
      switch (NewVal & b00011000){
        case b00010000: // 1.5
        case b00011000: // 2
          rs232_bits_per_word++;
          break;
      }
      NewVal&=b11111110;
      if ((mfp_reg[MFPR_UCR] & BIT_7)!=(NewVal & BIT_7) || old_bpw!=rs232_bits_per_word){
        mfp_reg[MFPR_UCR]=NewVal;
        RS232_CalculateBaud(bool(NewVal & BIT_7),mfp_get_timer_control_register(3),0);
      }
      break;
    }
    case MFPR_RSR:
      if ((NewVal & BIT_0)==0 && (mfp_reg[MFPR_RSR] & BIT_0)){ //disable receiver
        NewVal=0;
      }
      NewVal&=BYTE(~BIT_7);
      NewVal|=BYTE(mfp_reg[MFPR_RSR] & BIT_7);
      break;
    case MFPR_TSR:
      if ((NewVal & BIT_0) && (mfp_reg[MFPR_TSR] & BIT_0)==0){ //enable transmitter
        NewVal&=BYTE(~BIT_4); //Clear END
      }

      NewVal&=BYTE(~BIT_7);
      NewVal|=BYTE(mfp_reg[MFPR_TSR] & BIT_7);

      if ((NewVal & BIT_3)!=(mfp_reg[MFPR_TSR] & BIT_3)){
        if (NewVal & BIT_3){
          SerialPort.StartBreak();
          agenda_delete(agenda_serial_sent_byte);
          agenda_add(agenda_serial_break_boundary,rs232_hbls_per_word,0);
        }else{
          SerialPort.EndBreak();
          agenda_delete(agenda_serial_break_boundary);
          if ((mfp_reg[MFPR_TSR] & BIT_7)==0){ // tx buffer not empty
            agenda_add(agenda_serial_sent_byte,2,0);
          }
        }
      }
      break;
    case MFPR_UDR:
      if ((mfp_reg[MFPR_TSR] & BIT_0) && (mfp_reg[MFPR_TSR] & BIT_3)==0){
        // Transmitter enabled and no break

        if (UpdateBaud) RS232_CalculateBaud(bool(mfp_reg[MFPR_UCR] & BIT_7),mfp_get_timer_control_register(3),true);
        mfp_reg[MFPR_TSR]&=BYTE(~BIT_7);
        agenda_add(agenda_serial_sent_byte,rs232_hbls_per_word,0);
        if ((mfp_reg[MFPR_TSR] & b00000110)==b00000110){ //loopback
          agenda_add(agenda_serial_loopback_byte,rs232_hbls_per_word+1,NewVal);
        }else{
          SerialPort.OutputByte(BYTE(NewVal & (0xff >> ((mfp_reg[MFPR_UCR] & b01100000) >> 5))));
        }
      }
      return;
  }
  mfp_reg[Reg]=NewVal;
}

void agenda_serial_sent_byte(int)
{
  mfp_reg[MFPR_TSR]|=BYTE(BIT_7); //buffer empty
  mfp_interrupt(MFP_INT_RS232_TRANSMIT_BUFFER_EMPTY,ABSOLUTE_CPU_TIME);

  if ((mfp_reg[MFPR_TSR] & BIT_0)==0){ // transmitter disabled
    mfp_reg[MFPR_TSR]|=BYTE(BIT_4); //End
    mfp_interrupt(MFP_INT_RS232_TRANSMIT_ERROR,ABSOLUTE_CPU_TIME);
    if (mfp_reg[MFPR_TSR] & BIT_5) mfp_reg[MFPR_RSR]|=BIT_0; //Auto turnaround! 
  }
}

void agenda_serial_break_boundary(int)
{
  if ((mfp_reg[MFPR_TSR] & BIT_6)==0) mfp_interrupt(MFP_INT_RS232_TRANSMIT_ERROR,ABSOLUTE_CPU_TIME);
  agenda_add(agenda_serial_break_boundary,rs232_hbls_per_word,0);
}

void agenda_serial_loopback_byte(int NewVal)
{
  if (mfp_reg[MFPR_RSR] & BIT_0){
    if ((mfp_reg[MFPR_RSR] & BIT_7 /*Buffer Full*/)==0 ){
      rs232_recv_byte=BYTE(NewVal);
      rs232_recv_overrun=0;
    }else{
      rs232_recv_overrun=true;
    }

    mfp_reg[MFPR_RSR]&=BYTE(~(BIT_2 /*Char in progress*/ | BIT_3 /*Break*/ |
                              BIT_4 /*Frame Error*/ |      BIT_5 /*Parity Error*/));
    mfp_reg[MFPR_RSR]|=BIT_7 /*Buffer Full*/;
    mfp_interrupt(MFP_INT_RS232_RECEIVE_BUFFER_FULL,ABSOLUTE_CPU_TIME);
  }
}

