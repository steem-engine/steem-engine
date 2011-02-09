#define LOGSECTION LOGSECTION_IO
/*
  Secret addresses:
    poke byte into FFC123 - stops program running
    poke long into FFC1F0 - logs the string at the specified memory address,
                            which must be null-terminated
*/
//---------------------------------------------------------------------------
void ASMCALL io_write_b(MEM_ADDRESS addr,BYTE io_src_b)
{
/*
  Allowed IO writes (OR 0)

  FF8000 - FF800F MMU
  FF8200 - FF820F SHIFTER
  FF8240 - FF827F pallette, res
  FF8600 - FF860F FDC
  FF8800 - FF88FF sound chip
  FF8900 - FF893F DMA sound, microwire
  FF8A00 - FF8A3F blitter

  FF9000
  FF9001
  FF9202  paddles
  FF9203  paddles
  FFFA01, odd numbers up to FFFA3F MFP
  FFFC00 - FFFCFF  ACIA, realtime clock
  FFFD00 - FFFDFF
*/

  log_io_write(addr,io_src_b);

#ifdef _DEBUG_BUILD
  debug_check_io_monitor(addr,0,io_src_b);
#endif

#ifdef ONEGAME
  if (addr>=OG_TEXT_ADDRESS && addr<=OG_TEXT_ADDRESS+OG_TEXT_LEN){
    if (addr==(OG_TEXT_ADDRESS+OG_TEXT_LEN)){
      OGSetRestart();
      return;
    }
    OG_TextMem[addr-OG_TEXT_ADDRESS]=(char)io_src_b;
    return;
  }
#endif
  switch (addr & 0xffff00){   //0xfffe00
    case 0xfffc00:{  //--------------------------------------- ACIAs
      // Only cause bus jam once per word
      DEBUG_ONLY( if (mode==STEM_MODE_CPU) ) if (io_word_access==0 || (addr & 1)==0) BUS_JAM_TIME(8);

      switch (addr){
    /******************** Keyboard ACIA ************************/

      case 0xfffc00:  //control
        if ((io_src_b & 3)==3){
          log_to(LOGSECTION_IKBD,Str("IKBD: ")+HEXSl(old_pc,6)+" - ACIA reset"); 
          ACIA_Reset(NUM_ACIA_IKBD,0);
        }else{
          ACIA_SetControl(NUM_ACIA_IKBD,io_src_b);
        }
        break;
      case 0xfffc02:  //data
      {
        bool TXEmptyAgenda=(agenda_get_queue_pos(agenda_acia_tx_delay_IKBD)>=0);
        if (TXEmptyAgenda==0){
          if (ACIA_IKBD.tx_irq_enabled){
            ACIA_IKBD.irq=false;
            mfp_gpip_set_bit(MFP_GPIP_ACIA_BIT,!(ACIA_IKBD.irq || ACIA_MIDI.irq));
          }
          agenda_add(agenda_acia_tx_delay_IKBD,2 /*ACIAClockToHBLS(ACIA_IKBD.clock_divide)*/,0);
        }
        ACIA_IKBD.tx_flag=true; //flag for transmitting
        // If send new byte before last one has finished being sent
        if (abs(ABSOLUTE_CPU_TIME-ACIA_IKBD.last_tx_write_time)<ACIA_CYCLES_NEEDED_TO_START_TX){
          // replace old byte with new one
          int n=agenda_get_queue_pos(agenda_ikbd_process);
          if (n>=0){
            log_to(LOGSECTION_IKBD,Str("IKBD: ")+HEXSl(old_pc,6)+" - Received new command before old one was sent, replacing "+
                                      HEXSl(agenda[n].param,2)+" with "+HEXSl(io_src_b,2));
            agenda[n].param=io_src_b;
          }
        }else{
          // there is a delay before the data gets to the IKBD
          ACIA_IKBD.last_tx_write_time=ABSOLUTE_CPU_TIME;
          agenda_add(agenda_ikbd_process,IKBD_HBLS_FROM_COMMAND_WRITE_TO_PROCESS,io_src_b);
        }
        break;
      }

    /******************** MIDI ACIA *********************************/

      case 0xfffc04:  //control
        if ((io_src_b & 3)==3){ // Reset
          log_to(LOGSECTION_IKBD,Str("MIDI: ")+HEXSl(old_pc,6)+" - ACIA reset");
          ACIA_Reset(NUM_ACIA_MIDI,0);
        }else{
          ACIA_SetControl(NUM_ACIA_MIDI,io_src_b);
        }
        break;
      case 0xfffc06:  //data
      {
        bool TXEmptyAgenda=(agenda_get_queue_pos(agenda_acia_tx_delay_MIDI)>=0);
        if (TXEmptyAgenda==0){
          if (ACIA_MIDI.tx_irq_enabled){
            ACIA_MIDI.irq=false;
            mfp_gpip_set_bit(MFP_GPIP_ACIA_BIT,!(ACIA_IKBD.irq || ACIA_MIDI.irq));
          }
          agenda_add(agenda_acia_tx_delay_MIDI,2 /*ACIAClockToHBLS(ACIA_MIDI.clock_divide)*/,0);
        }
        ACIA_MIDI.tx_flag=true;  //flag for transmitting
        MIDIPort.OutputByte(io_src_b);
        break;
      }
    //-------------------------- unrecognised -------------------------------------------------
      default:
        break;  //all writes allowed
      }
    }
    break;
    case 0xfffa00:  //--------------------------------------- MFP
    {
      if (addr<0xfffa40){
        // Only cause bus jam once per word
        DEBUG_ONLY( if (mode==STEM_MODE_CPU) ) if (io_word_access==0 || (addr & 1)==1) BUS_JAM_TIME(4);

        if (addr & 1){
          if (addr<0xfffa30){
            int old_ioaccess=ioaccess;
            int n=(addr-0xfffa01) >> 1;
            if (n==MFPR_GPIP || n==MFPR_AER || n==MFPR_DDR){
              // The output from the AER is eored with the GPIP/input buffer state
              // and that input goes into a 1-0 transition detector. So if the result
              // used to be 1 and now it is 0 an interrupt will occur (if the
              // interrupt is enabled of course).
              BYTE old_gpip=BYTE(mfp_reg[MFPR_GPIP] & ~mfp_reg[MFPR_DDR]);
              old_gpip|=BYTE(mfp_gpip_input_buffer & mfp_reg[MFPR_DDR]);
              BYTE old_aer=mfp_reg[MFPR_AER];

              if (n==MFPR_GPIP){  // Write to GPIP (can only change bits set to 1 in DDR)
                io_src_b&=mfp_reg[MFPR_DDR];
                // Don't change the bits that are 0 in the DDR
                io_src_b|=BYTE(mfp_gpip_input_buffer & ~mfp_reg[MFPR_DDR]);
                mfp_gpip_input_buffer=io_src_b;
              }else{
                mfp_reg[n]=io_src_b;
              }
              BYTE new_gpip=BYTE(mfp_reg[MFPR_GPIP] & ~mfp_reg[MFPR_DDR]);
              new_gpip|=BYTE(mfp_gpip_input_buffer & mfp_reg[MFPR_DDR]);
              BYTE new_aer=mfp_reg[MFPR_AER];

              for (int bit=0;bit<8;bit++){
                int irq=mfp_gpip_irq[bit];
                if (mfp_interrupt_enabled[irq]){
                  BYTE mask=BYTE(1 << bit);
                  bool old_1_to_0_detector_input=((old_gpip & mask) ^ (old_aer & mask))==mask;
                  bool new_1_to_0_detector_input=((new_gpip & mask) ^ (new_aer & mask))==mask;
                  if (old_1_to_0_detector_input && new_1_to_0_detector_input==0){
                    // Transition the right way! Set pending (interrupts happen later)
                    // Don't need to call set_pending routine here as this can never
                    // happen soon after an interrupt
                    mfp_reg[MFPR_IPRA+mfp_interrupt_i_ab(irq)]|=mfp_interrupt_i_bit(irq);
                  }
                }
              }
            }else if (n>=MFPR_IERA && n<=MFPR_IERB){ //enable
              // See if timers have timed out before write to enabled. This is needed
              // because MFP_CALC_INTERRUPTS_ENABLED religns the timers (so they would
              // not cause a timeout if they are overdue at this point)
              // Update v2.5: I don't think this will happen, when you write to this
              // register the MFP will turn of the interrupt line straight away.
//              mfp_check_for_timer_timeouts();

              ///// Update v2.7, why does calc interrupts enabled need to realign the timers?
              //  It has been removed, so don't need this for anything

              mfp_reg[n]=io_src_b;
              MFP_CALC_INTERRUPTS_ENABLED;
              for (n=0;n<4;n++){
                bool new_enabled=(mfp_interrupt_enabled[mfp_timer_irq[n]] && (mfp_get_timer_control_register(n) & 7));
                if (new_enabled && mfp_timer_enabled[n]==0){
                  // Timer should have been running but isn't, must put into future
                  int stage=(mfp_timer_timeout[n]-ABSOLUTE_CPU_TIME);
                  if (stage<=0){
                    stage+=((-stage/mfp_timer_period[n])+1)*mfp_timer_period[n];
                  }else{
                    stage%=mfp_timer_period[n];
                  }
                  mfp_timer_timeout[n]=ABSOLUTE_CPU_TIME+stage;
                }

                LOG_ONLY( if (new_enabled!=mfp_timer_enabled[n]) log_to(LOGSECTION_MFP_TIMERS,Str("MFP: ")+HEXSl(old_pc,6)+
                                                  " - Timer "+char('A'+n)+" enabled="+new_enabled); )
                mfp_timer_enabled[n]=new_enabled;
              }

              /*
                Disabling an interrupt channel has no
                effect on the corresponding bit in Interrupt In-Service
                Registers (ISRA, ISRB) ; thus, if the In-service
                Registers are used and an interrupt is in service on
                that channel when the channel is disabled, it will remain
                in service until cleared in the normal manner.

                mfp_reg[MFPR_ISRA]&=mfp_reg[MFPR_IERA]; //no in service on disabled registers
                mfp_reg[MFPR_ISRB]&=mfp_reg[MFPR_IERB]; //no in service on disabled registers
              */

              /*
                and any pending interrupt on that channel will be cleared by disabling
                that channel.
              */
              mfp_reg[MFPR_IPRA]&=mfp_reg[MFPR_IERA]; //no pending on disabled registers
              mfp_reg[MFPR_IPRB]&=mfp_reg[MFPR_IERB]; //no pending on disabled registers
            }else if (n>=MFPR_IPRA && n<=MFPR_ISRB){ //can only clear bits in IPR, ISR
              mfp_reg[n]&=io_src_b;
            }else if (n>=MFPR_TADR && n<=MFPR_TDDR){ //have to set counter as well as data register
              mfp_set_timer_reg(n,mfp_reg[n],io_src_b);
              mfp_reg[n]=io_src_b;
            }else if (n==MFPR_TACR || n==MFPR_TBCR){ //wipe low-bit on set
              io_src_b &= BYTE(0xf);
              mfp_set_timer_reg(n,mfp_reg[n],io_src_b);
              mfp_reg[n]=io_src_b;
            }else if (n==MFPR_TCDCR){
              io_src_b&=BYTE(b01110111);
              mfp_set_timer_reg(n,mfp_reg[n],io_src_b);
              mfp_reg[n]=io_src_b;
            }else if (n==MFPR_VR){
              mfp_reg[MFPR_VR]=io_src_b;
              if (!MFP_S_BIT){
                mfp_reg[MFPR_ISRA]=0;
                mfp_reg[MFPR_ISRB]=0;
              }
            }else if (n>=MFPR_SCR){
              RS232_WriteReg(n,io_src_b);
            }else{
              mfp_reg[n]=io_src_b;
            }
            // The MFP doesn't update for about 8 cycles, so we should execute the next
            // instruction before causing any interrupts
            ioaccess=old_ioaccess;
            if ((ioaccess & (IOACCESS_FLAG_FOR_CHECK_INTRS_MFP_CHANGE | IOACCESS_FLAG_FOR_CHECK_INTRS |
                                IOACCESS_FLAG_DELAY_MFP))==0){
              ioaccess|=IOACCESS_FLAG_FOR_CHECK_INTRS_MFP_CHANGE;
            }
          }
        }else{ // even
          // Byte access causes bus error
          if (io_word_access==0) exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
        }
      }else{ // beyond allowed range
        exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
      }
      break;
    }
    case 0xff9800:        // Falcon 256 colour palette
    case 0xff9900:        // Falcon 256 colour palette
    case 0xff9a00:        // Falcon 256 colour palette
    case 0xff9b00:        // Falcon 256 colour palette
      if (emudetect_called){
        int n=(addr-0xff9800)/4;
        DWORD val=emudetect_falcon_stpal[n];
        DWORD_B(&val,addr & 3)=BYTE(io_src_b & ~1);
        emudetect_falcon_stpal[n]=val;
        emudetect_falcon_palette_convert(n);
        return; // No exception
      }
      exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
      break;
    case 0xff8a00:      //----------------------------------- Blitter
      Blitter_IO_WriteB(addr,io_src_b);
      break;
    case 0xff8900:      //----------------------------------- STE DMA Sound
      if (addr>0xff893f){ //illegal range
        exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
      }else{
        switch (addr){
          case 0xff8900:   //Nowt
            break;
          case 0xff8901:   //DMA control register
            if ((dma_sound_control & BIT_0) && (io_src_b & BIT_0)==0){  //Stopping
              dma_sound_write_to_buffer(ABSOLUTE_CPU_TIME);

              dma_sound_start=next_dma_sound_start;
              dma_sound_end=next_dma_sound_end;
              dma_sound_addr_to_read_next=dma_sound_start;

              DMA_SOUND_CHECK_TIMER_A;
              dma_sound_prepare_for_end(0,0,0); // Stop event
              ioaccess|=IOACCESS_FLAG_FOR_CHECK_INTRS; // Do this to prepare event again
            }else if ((dma_sound_control & BIT_0)==0 && (io_src_b & BIT_0)){ //Start playing
              dma_sound_write_to_buffer(ABSOLUTE_CPU_TIME);

              dma_sound_start_time=ABSOLUTE_CPU_TIME;
              dma_sound_start=next_dma_sound_start;
              dma_sound_end=next_dma_sound_end;
              dma_sound_countdown=0;
              dma_sound_addr_to_read_next=dma_sound_start;
              dma_sound_prepare_for_end(dma_sound_start,ABSOLUTE_CPU_TIME,true);
              ioaccess|=IOACCESS_FLAG_FOR_CHECK_INTRS; // Do this to prepare event again
            }
            dma_sound_control=io_src_b;
            if (tos_version>=0x106) mfp_gpip_set_bit(MFP_GPIP_MONO_BIT,bool(COLOUR_MONITOR)^bool(dma_sound_control & BIT_0));

            log_to(LOGSECTION_SOUND,EasyStr("SOUND: ")+HEXSl(old_pc,6)+" - DMA sound control set to "+(dma_sound_control & 3));
            break;
          case 0xff8903:   //HiByte of frame start address
          case 0xff8905:   //MidByte of frame start address
          case 0xff8907:   //LoByte of frame start address
            switch (addr & 0xf){
              case 0x3:next_dma_sound_start&=0x00ffff;next_dma_sound_start|=io_src_b << 16;break;
              case 0x5:next_dma_sound_start&=0xff00ff;next_dma_sound_start|=io_src_b << 8;break;
              case 0x7:next_dma_sound_start&=0xffff00;next_dma_sound_start|=io_src_b;break;
            }
            if ((dma_sound_control & BIT_0)==0){
              dma_sound_start=next_dma_sound_start;
              dma_sound_addr_to_read_next=dma_sound_start;
            }
            log_to(LOGSECTION_SOUND,EasyStr("SOUND: ")+HEXSl(old_pc,6)+" - DMA sound start address set to "+HEXSl(next_dma_sound_start,6));
            break;
          case 0xff890f:   //HiByte of frame end address
          case 0xff8911:   //MidByte of frame end address
          case 0xff8913:   //LoByte of frame end address
            switch (addr & 0xf){
              case 0xf:next_dma_sound_end&=0x00ffff;next_dma_sound_end|=io_src_b << 16;break;
              case 0x1:next_dma_sound_end&=0xff00ff;next_dma_sound_end|=io_src_b << 8;break;
              case 0x3:next_dma_sound_end&=0xffff00;next_dma_sound_end|=io_src_b;break;
            }
            if ((dma_sound_control & BIT_0)==0) dma_sound_end=next_dma_sound_end;
            log_to(LOGSECTION_SOUND,EasyStr("SOUND: ")+HEXSl(old_pc,6)+" - DMA sound end address set to "+HEXSl(next_dma_sound_end,6));
            break;
          case 0xff8921:   //Sound mode control
            dma_sound_write_to_buffer(ABSOLUTE_CPU_TIME);

            dma_sound_mode=io_src_b;
            dma_sound_freq=dma_sound_mode_to_freq[(dma_sound_mode & BIT_7)!=0][dma_sound_mode & 3];
            log_to(LOGSECTION_SOUND,EasyStr("SOUND: ")+HEXSl(old_pc,6)+" - DMA sound mode set to "+HEXSl(dma_sound_mode,2)+" freq="+dma_sound_freq);
            break;

          case 0xff8922: // Set high byte of MicroWire_Data
            MicroWire_Data=MAKEWORD(LOBYTE(MicroWire_Data),io_src_b);
            break;
          case 0xff8923: // Set low byte of MicroWire_Data
          {
            MicroWire_Data=MAKEWORD(io_src_b,HIBYTE(MicroWire_Data));
            MicroWire_StartTime=ABSOLUTE_CPU_TIME;
            int dat=MicroWire_Data & MicroWire_Mask;
            int b;
            for (b=15;b>=10;b--){
              if (MicroWire_Mask & (1 << b)){
                if ((dat & (1 << b)) && (dat & (1 << (b-1)))==0){  //DMA Sound Address
                  int dat_b=b-2;
                  for (;dat_b>=8;dat_b--){ // Find start of data
                    if (MicroWire_Mask & (1 << dat_b)) break;
                  }
                  dat >>= dat_b-8; // Move 9 highest bits of data to the start
                  int nController=(dat >> 6) & b0111;
                  switch (nController){
                    case b0011: // Master Volume
                    case b0101: // Left Volume
                    case b0100: // Right Volume
                      if (nController==b0011){
                        // 20 is practically silent!
                        dma_sound_volume=(dat & b00111111);
                        if (dma_sound_volume>47) dma_sound_volume=0;
                        if (dma_sound_volume>40) dma_sound_volume=40;
                      }else{
                        int new_val=(dat & b00011111);
                        if (new_val>23) new_val=0;
                        if (new_val>20) new_val=20;
                        if (nController==b0101) dma_sound_l_volume=new_val;
                        if (nController==b0100) dma_sound_r_volume=new_val;
                      }
                      long double lv,rv,mv;
                      lv=dma_sound_l_volume;lv=lv*lv*lv*lv;
                      lv/=(20.0*20.0*20.0*20.0);
                      rv=dma_sound_r_volume;rv=rv*rv*rv*rv;
                      rv/=(20.0*20.0*20.0*20.0);
                      mv=dma_sound_volume;  mv=mv*mv*mv*mv*mv*mv*mv*mv;
                      mv/=(40.0*40.0*40.0*40.0*40.0*40.0*40.0*40.0);
                      // lv rv and mv are numbers between 0 and 1
                      dma_sound_l_top_val=BYTE(128.0*lv*mv);
                      dma_sound_r_top_val=BYTE(128.0*rv*mv);
                      log_to_section(LOGSECTION_SOUND,EasyStr("SOUND: ")+HEXSl(old_pc,6)+" - DMA sound set volume master="+dma_sound_volume+
                                      " l="+dma_sound_l_volume+" r="+dma_sound_r_volume);
                      break;
                    case b0010: // Treble
                    case b0001: // Base
                      break;
                    case b0000: // Mixer
                      dma_sound_mixer=dat & b00000011; // 1=PSG too, anything else only DMA
                      log_to_section(LOGSECTION_SOUND,EasyStr("SOUND: ")+HEXSl(old_pc,6)+" - DMA sound mixer is set to "+dma_sound_mixer);
                      break;
                  }
                }
                break;
              }
            }
            break;
          }
          case 0xff8924:  // Set high byte of MicroWire_Mask
            MicroWire_Mask=MAKEWORD(LOBYTE(MicroWire_Mask),io_src_b);
            break;
          case 0xff8925:  // Set low byte of MicroWire_Mask
            MicroWire_Mask=MAKEWORD(io_src_b,HIBYTE(MicroWire_Mask));
            break;
        }
      }
      break;
    case 0xff8800:{  //--------------------------------------- sound chip
      if ((ioaccess & IOACCESS_FLAG_PSG_BUS_JAM_W)==0){
        DEBUG_ONLY( if (mode==STEM_MODE_CPU) ) BUS_JAM_TIME(4);
        ioaccess|=IOACCESS_FLAG_PSG_BUS_JAM_W;
      }
      if ((addr & 1) && io_word_access) break; //odd addresses ignored on word writes

      if ((addr & 2)==0){  //read data / register select
        psg_reg_select=io_src_b;
        if (psg_reg_select<16){
          psg_reg_data=psg_reg[psg_reg_select];
        }else{
          psg_reg_data=0xff;
        }
      }else{  //write data
        if (psg_reg_select>15) return;
        psg_reg_data=io_src_b;

        BYTE old_val=psg_reg[psg_reg_select];
        psg_set_reg(psg_reg_select,old_val,io_src_b);
        psg_reg[psg_reg_select]=io_src_b;

        if (psg_reg_select==PSGR_PORT_A){
          SerialPort.SetDTR(io_src_b & BIT_4);
          SerialPort.SetRTS(io_src_b & BIT_3);
#ifdef ENABLE_LOGFILE
          if ((old_val & (BIT_1+BIT_2))!=(io_src_b & (BIT_1+BIT_2))){
            if ((psg_reg[PSGR_PORT_A] & BIT_1)==0){ //drive 0
              log_to_section(LOGSECTION_FDC,Str("FDC: ")+HEXSl(old_pc,6)+" - Set current drive to A:");
            }else if ((psg_reg[PSGR_PORT_A] & BIT_2)==0){ //drive 1
              log_to_section(LOGSECTION_FDC,Str("FDC: ")+HEXSl(old_pc,6)+" - Set current drive to B:");
            }else{                             //who knows?
              log_to_section(LOGSECTION_FDC,Str("FDC: ")+HEXSl(old_pc,6)+" - Unset current drive - guess A:");
            }
            disk_light_off_time=timer+DisableDiskLightAfter;
          }
#endif
        }else if (psg_reg_select==PSGR_PORT_B){
          if (ParallelPort.IsOpen()){
            if (ParallelPort.OutputByte(io_src_b)==0){
              log_write("ARRRGGHH: Lost printer character, printer not responding!!!!");
            }
            UpdateCentronicsBusyBit();
          }
        }else if (psg_reg_select==PSGR_MIXER){
          UpdateCentronicsBusyBit();
        }
      }
      break;
    }case 0xff8600:{  //--------------------------------------- DMA / FDC
      if (addr>0xff860f){ //past allowed range
        exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
      }else{
/* -- Keep this here!
#ifdef ENABLE_LOGFILE
        EasyStr bin_src_b;bin_src_b.SetLength(8);
        itoa(io_src_b,bin_src_b,2);
        EasyStr a=EasyStr("FDC: Writing byte ")+bin_src_b.LPad(8,'0')+" to IO address "+HEXSl(addr,6);
#ifdef _DEBUG_BUILD
        iolist_entry *iol=search_iolist(addr);
        if (iol) a+=EasyStr(" (")+(iol->name)+")";
#endif
        log_to_section(LOGSECTION_FDC,a);
#endif
*/
        switch (addr){
          case 0xff8604:  //high byte of FDC access
            if (dma_mode & BIT_4){ //write DMA sector counter, 0x190
              dma_sector_count&=0xff;
              dma_sector_count|=int(io_src_b) << 8;
              if (dma_sector_count){
                dma_status|=BIT_1;
              }else{
                dma_status&=BYTE(~BIT_1); //status register bit for 0 count
              }
              dma_bytes_written_for_sector_count=0;
              log_to(LOGSECTION_FDC,Str("FDC: ")+HEXSl(old_pc,6)+" - Set DMA sector count to "+dma_sector_count);
              break;
            }
            if (dma_mode & BIT_3){ // HD access
              log_to(LOGSECTION_FDC,Str("FDC: ")+HEXSl(old_pc,6)+" - Writing $"+HEXSl(io_src_b,2)+"xx to HDC register #"+((dma_mode & BIT_1) ? 1:0));
              break;
            }
            break;
          case 0xff8605:  //low byte of FDC access
          {
            if (dma_mode & BIT_4){ //write FDC sector counter, 0x190
              dma_sector_count&=0xff00;
              dma_sector_count|=io_src_b;
              if (dma_sector_count){
                dma_status|=BIT_1;
              }else{
                dma_status&=BYTE(~BIT_1); //status register bit for 0 count
              }
              dma_bytes_written_for_sector_count=0;
              log_to(LOGSECTION_FDC,Str("FDC: ")+HEXSl(old_pc,6)+" - Set DMA sector count to "+dma_sector_count);
              break;
            }
            if (dma_mode & BIT_3){ // HD access
              log_to(LOGSECTION_FDC,Str("FDC: ")+HEXSl(old_pc,6)+" - Writing $xx"+HEXSl(io_src_b,2)+" to HDC register #"+((dma_mode & BIT_1) ? 1:0));
              break;
            }
            switch (dma_mode & (BIT_1+BIT_2)){
              case 0:
                floppy_fdc_command(io_src_b);
                break;
              case 2:
                if ((fdc_str & FDC_STR_BUSY)==0){
                  log_to(LOGSECTION_FDC,EasyStr("FDC: ")+HEXSl(old_pc,6)+" - Setting FDC track register to "+io_src_b);
                  fdc_tr=io_src_b;
                }else{
                  log_to(LOGSECTION_FDC,EasyStr("FDC: ")+HEXSl(old_pc,6)+" - Can't set FDC track register to "+io_src_b+", FDC is busy");
                }
                break;
              case 4:
                if ((fdc_str & FDC_STR_BUSY)==0){
                  log_to(LOGSECTION_FDC,EasyStr("FDC: ")+HEXSl(old_pc,6)+" - Setting FDC sector register to "+io_src_b);
                  fdc_sr=io_src_b;
                }else{
                  log_to(LOGSECTION_FDC,EasyStr("FDC: ")+HEXSl(old_pc,6)+" - Can't set FDC sector register to "+io_src_b+", FDC is busy");
                }
                break;
              case 6:
                log_to(LOGSECTION_FDC,EasyStr("FDC: ")+HEXSl(old_pc,6)+" - Setting FDC data register to "+io_src_b);
                fdc_dr=io_src_b;
                break;
            }
            break;
          }

          // Writes to DMA mode clear the DMA internal buffer
          case 0xff8606:  //high byte of DMA mode
            dma_mode&=0x00ff;
            dma_mode|=WORD(WORD(io_src_b) << 8);

            fdc_read_address_buffer_len=0;
            dma_bytes_written_for_sector_count=0;
            break;
          case 0xff8607:  //low byte of DMA mode
            dma_mode&=0xff00;
            dma_mode|=io_src_b;

            fdc_read_address_buffer_len=0;
            dma_bytes_written_for_sector_count=0;
            break;

          case 0xff8609:  //high byte of DMA pointer
            dma_address&=0x00ffff;
            dma_address|=((MEM_ADDRESS)io_src_b) << 16;
            log_to(LOGSECTION_FDC,EasyStr("FDC: ")+HEXSl(old_pc,6)+" - Set DMA address to "+HEXSl(dma_address,6));
            break;
          case 0xff860b:  //mid byte of DMA pointer
            //DMA pointer has to be initialized in order low, mid, high
            dma_address&=0xff00ff;
            dma_address|=((MEM_ADDRESS)io_src_b) << 8;
            break;
          case 0xff860d:  //low byte of DMA pointer
            //DMA pointer has to be initialized in order low, mid, high
            dma_address&=0xffff00;
            dma_address|=io_src_b;
            break;
          case 0xff860e: //high byte of frequency/density control
            break; //ignore
          case 0xff860f: //low byte of frequency/density control
            break;
        }
      }
      break;
    }
#undef LOGSECTION
#define LOGSECTION LOGSECTION_VIDEO

    case 0xff8200:{  //----------------------------------------=--------------- shifter
                     //----------------------------------------=--------------- shifter
                     //----------------------------------------=--------------- shifter
                     //----------------------------------------=--------------- shifter
                     //----------------------------------------=--------------- shifter
                     //----------------------------------------=--------------- shifter
                     //----------------------------------------=--------------- shifter
                     //----------------------------------------=--------------- shifter
                     //----------------------------------------=--------------- shifter
                     //----------------------------------------=--------------- shifter
                     //----------------------------------------=--------------- shifter
                     //----------------------------------------=--------------- shifter
      if ((addr>=0xff8210 && addr<0xff8240) || addr>=0xff8280){
        exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
      }else if (addr>=0xff8240 && addr<0xff8260){  //palette
        int n=(addr-0xff8240) >> 1;
        BYTE *lpPalEntry=lpWORD_B_0(STpal+n);
        if ((addr & 1)==0){
          lpPalEntry+=MORE_SIGNIFICANT_BYTE_OFFSET;
          io_src_b&=0xf;
        }
        if (*lpPalEntry!=io_src_b){
          *lpPalEntry=io_src_b;
          PAL_DPEEK(n*2)=STpal[n];
          log_to(LOGSECTION_VIDEO,EasyStr("VIDEO: ")+HEXSl(old_pc,6)+" - Palette change at scan_y="+scan_y+" cycle "+(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl));
          if (draw_lock) draw_scanline_to((ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl)+1);
          if (flashlight_flag==0 DEBUG_ONLY( && debug_cycle_colours==0) ){
            palette_convert(n);
          }
        }
      }else{
        switch(addr){
        case 0xff8201:  //high byte of screen memory address
          DWORD_B_2(&xbios2)=io_src_b;
          DWORD_B_0(&xbios2)=0;
          log_to(LOGSECTION_VIDEO,EasyStr("VIDEO: ")+HEXSl(old_pc,6)+" - Set screen base to "+HEXSl(xbios2,6));
          break;
        case 0xff8203:  //mid byte of screen memory address
          DWORD_B_1(&xbios2)=io_src_b;
          DWORD_B_0(&xbios2)=0;
          log_to(LOGSECTION_VIDEO,EasyStr("VIDEO: ")+HEXSl(old_pc,6)+" - Set screen base to "+HEXSl(xbios2,6));
          break;
        case 0xff8205:  //high byte of draw pointer
        case 0xff8207:  //mid byte of draw pointer
        case 0xff8209:  //low byte of draw pointer
        {
//          int srp=scanline_raster_position();
          int dst=ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl;
          dst-=CYCLES_FROM_HBL_TO_LEFT_BORDER_OPEN;
          dst+=16;dst&=-16;
          dst+=CYCLES_FROM_HBL_TO_LEFT_BORDER_OPEN;

          draw_scanline_to(dst); // This makes shifter_draw_pointer up to date
          MEM_ADDRESS nsdp=shifter_draw_pointer;
          DWORD_B(&nsdp,(0xff8209-addr)/2)=io_src_b;
          nsdp%=FOUR_MEGS;
//          int off=(get_shifter_draw_pointer(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl)&-8)-shifter_draw_pointer;
//          shifter_draw_pointer=nsdp-off;
          shifter_draw_pointer_at_start_of_line-=shifter_draw_pointer;
          shifter_draw_pointer_at_start_of_line+=nsdp;
          shifter_draw_pointer=nsdp;

          if (dst>=CYCLES_FROM_HBL_TO_LEFT_BORDER_OPEN-32){
//            shifter_skip_raster_for_hscroll=0; // already fetched first raster if hscroll on
          }

          log_to(LOGSECTION_VIDEO,Str("VIDEO: ")+HEXSl(old_pc,6)+" - Set shifter draw pointer to "+
                    HEXSl(shifter_draw_pointer,6)+" at scanline "+scan_y+", cycle "+
                    (ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl)+", aligned to "+dst);
          break;
        }
        case 0xff820d:  //low byte of screen memory address
          DWORD_B_0(&xbios2)=io_src_b;
          log_to(LOGSECTION_VIDEO,EasyStr("VIDEO: ")+HEXSl(old_pc,6)+" - Set screen base to "+HEXSl(xbios2,6));
          break;
        case 0xff820a:  //synchronization mode
        {
          int new_freq;

          if (io_src_b & 2){
            new_freq=50;
          }else{
            new_freq=60;
          }

          if (screen_res>=2) new_freq=MONO_HZ;

          if (shifter_freq!=new_freq){
            log_to(LOGSECTION_VIDEO,EasyStr("VIDEO: ")+HEXSl(old_pc,6)+" - Changed frequency to "+new_freq+
                            " at scanline "+scan_y+" cycle "+(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl));
            shifter_freq=new_freq;
            CALC_SHIFTER_FREQ_IDX;
            if (shifter_freq_change[shifter_freq_change_idx]!=MONO_HZ){
              ADD_SHIFTER_FREQ_CHANGE(shifter_freq);
            }
            freq_change_this_scanline=true;
            if (FullScreen && border==2){
              int off=shifter_first_draw_line-draw_first_possible_line;
              draw_first_possible_line+=off;
              draw_last_possible_line+=off;
            }
          }
          break;
        }
        case 0xff820f:   //int shifter_fetch_extra_words;
          draw_scanline_to(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl); // Update sdp if off right
          shifter_fetch_extra_words=(BYTE)io_src_b;
          log_to(LOGSECTION_VIDEO,EasyStr("VIDEO: ")+HEXSl(old_pc,6)+" - Set shifter_fetch_extra_words to "+
                    (shifter_fetch_extra_words)+" at scanline "+scan_y+", cycle "+
                    (ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl));
          break;
        case 0xff8264:  // Set hscroll and don't change line length
          // This is an odd register, when you change hscroll below to non-zero each
          // scanline becomes 4 words longer to allow for extra screen data. This register
          // sets hscroll but doesn't do that, instead the left border is increased by
          // 16 pixels. If you have got hscroll extra fetch turned on then setting this
          // to 0 confuses the shifter and causes it to shrink the left border by 16 pixels.
        case 0xff8265:  // Hscroll
          draw_scanline_to(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl); // Update sdp if off right
          shifter_pixel-=shifter_hscroll;
          shifter_hscroll=io_src_b & 0xf;
          shifter_pixel+=shifter_hscroll;

          log_to(LOGSECTION_VIDEO,EasyStr("VIDEO: ")+HEXSl(old_pc,6)+" - Set horizontal scroll ("+HEXSl(addr,6)+
                    ") to "+(shifter_hscroll)+" at scanline "+scan_y+", cycle "+(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl));
          if (addr==0xff8265) shifter_hscroll_extra_fetch=(shifter_hscroll!=0);

          if (ABSOLUTE_CPU_TIME<=cpu_timer_at_start_of_hbl+CYCLES_FROM_HBL_TO_LEFT_BORDER_OPEN-32){
            if (left_border>0){ // Don't do this if left border removed!
              shifter_skip_raster_for_hscroll=(shifter_hscroll!=0 && shifter_hscroll_extra_fetch);
              left_border=BORDER_SIDE;
              if (shifter_hscroll) left_border+=16;
              if (shifter_hscroll_extra_fetch) left_border-=16;
            }
          }
          break;
        case 0xff8260: //resolution
          if (screen_res>=2 || emudetect_falcon_mode!=EMUD_FALC_MODE_OFF) return;
#ifndef NO_CRAZY_MONITOR
          if (extended_monitor){
            screen_res=BYTE(io_src_b & 1);
            return;
          }
#endif
          io_src_b&=3;
          int cycles_in=int(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl);
          int dst=cycles_in;
          dst-=CYCLES_FROM_HBL_TO_LEFT_BORDER_OPEN;
          dst+=16;dst&=-16;
          dst+=CYCLES_FROM_HBL_TO_LEFT_BORDER_OPEN;

          draw_scanline_to(dst);
//          cycles_in%=(scanline_time_in_cpu_cycles[shifter_freq_idx]);
          log_to(LOGSECTION_VIDEO,EasyStr("VIDEO: ")+HEXSl(old_pc,6)+" - Changed screen res to "+
                              io_src_b+" at scanline "+scan_y+", cycle "+cycles_in);
          if ( mfp_gpip_no_interrupt & MFP_GPIP_COLOUR ){
            if ( io_src_b==2 ){
              // Trying to change to hi res in colour - that's crazy!
              // But wait!  You can remove the left border like that!  Wow!
              ADD_SHIFTER_FREQ_CHANGE(MONO_HZ);
              freq_change_this_scanline=true;
            }else{
              if (shifter_freq_change[shifter_freq_change_idx]==MONO_HZ){
                ADD_SHIFTER_FREQ_CHANGE(shifter_freq);
                freq_change_this_scanline=true;
/*
                if (cycles_in<80){
                  int raster_start=(cycles_in-8)/2;
                  shifter_draw_pointer+=raster_start; //adjust sdp for hi-res bit
                  overscan_add_extra-=raster_start;
                  if (raster_start){
                    log_to_section(LOGSECTION_VIDEO,Str("SHIFTER: Adjusted SDP for late "
                                "change from mono, increased by ")+raster_start+" bytes");
                  }
                }
*/
              }
              int old_screen_res=screen_res;
              screen_res=BYTE(io_src_b & 1);
              if (screen_res!=old_screen_res){
                if (screen_res>0){
                  shifter_x=640;
                }else{
                  shifter_x=320;
                }
                if (draw_lock){
                  if (screen_res==0) draw_scanline=draw_scanline_lowres;
                  if (screen_res==1) draw_scanline=draw_scanline_medres;
#ifdef WIN32
                  if (draw_store_dest_ad){
                    draw_store_draw_scanline=draw_scanline;
                    draw_scanline=draw_scanline_1_line[screen_res];
                  }
#endif
                }
                if (mixed_output==3 && (ABSOLUTE_CPU_TIME-cpu_timer_at_res_change<30)){
                  mixed_output=0; //cancel!
                }else if (mixed_output==0){
                  mixed_output=3;
                }else if (mixed_output<2){
                  mixed_output=2;
                }
                cpu_timer_at_res_change=ABSOLUTE_CPU_TIME;
//              shifter_fetch_extra_words=0; //unspecified
//              draw_set_jumps_and_source();
              }
            }
          }

          break;
        }
      }
      break;
    }
#undef LOGSECTION
#define LOGSECTION LOGSECTION_IO
    case 0xff8000:  //--------------------------------------- memory
    {
      if (addr==0xff8001){ //Memory Configuration
        if (old_pc>=FOURTEEN_MEGS && mem_len<=FOUR_MEGS){
          // A program in user memory can't change config, with > 4MB this is always ignored        
          mmu_memory_configuration=io_src_b;
          mmu_bank_length[0]=mmu_bank_length_from_config[(mmu_memory_configuration & b1100) >> 2];
          mmu_bank_length[1]=mmu_bank_length_from_config[(mmu_memory_configuration & b0011)];
          mmu_confused=false;
          if (bank_length[0]) if (mmu_bank_length[0]!=bank_length[0]) mmu_confused=true;
          if (bank_length[1]) if (mmu_bank_length[1]!=bank_length[1]) mmu_confused=true;
          himem=(MEM_ADDRESS)(mmu_confused ? 0:mem_len);
        }
      }else if (addr>0xff800f){
        exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
      }
      break;
    }

    case 0xffc100: //secret Steem registers!
    {
#ifdef _DEBUG_BUILD
      if (addr==0xffc123){ //stop
        if (runstate==RUNSTATE_RUNNING){
          runstate=RUNSTATE_STOPPING;
          SET_WHY_STOP("Software break - write to $FFC123");
        }
        break;
      }else if (addr==0xffc1f4){
        logfile_wipe();
      }
#endif
      if (emudetect_called){
        switch (addr){
          // 100.l = create disk image
          case 0xffc104: emudetect_reset(); break;

          case 0xffc105: new_n_cpu_cycles_per_second=min(max(int(io_src_b),8),128)*1000000; break;

          case 0xffc108: // Run speed percent hi
          case 0xffc109: // Run speed percent low
          {
            WORD percent=WORD(100000/run_speed_ticks_per_second);
            if (addr==0xffc108) WORD_B_1(&percent)=io_src_b; // High byte
            if (addr==0xffc109) WORD_B_0(&percent)=io_src_b; // Low byte
            run_speed_ticks_per_second=100000 / max(int(percent),50);
            break;
          }
          case 0xffc107: snapshot_loaded=bool(io_src_b); break;
          case 0xffc11a: emudetect_write_logs_to_printer=bool(io_src_b); break;
          case 0xffc11b:
            if (extended_monitor==0 && screen_res<2) emudetect_falcon_mode=BYTE(io_src_b);
            break;
          case 0xffc11c:
            emudetect_falcon_mode_size=BYTE((io_src_b & 1)+1);
            emudetect_falcon_extra_height=bool(io_src_b & 2);
            // Make sure we don't mess up video memory. It is possible that the height of
            // scanlines is doubled, if we change to 400 with double height lines then arg!
            draw_set_jumps_and_source();
            break;
        }
        if (addr<0xffc120) break; // No exception!
      }
      exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
    }
    case 0xfffd00:{ //?????
      break;
    }case 0xff9000:{ //?????
      if (addr>0xff9001) exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
      break;
    }case 0xff9200:{ //paddles
      if (addr==0xff9202){ // Doesn't work for high byte
        WORD_B_1(&paddles_ReadMask)=0;
      }else if (addr==0xff9203){
        WORD_B_0(&paddles_ReadMask)=io_src_b;
      }else{
        exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
      }
      break;

    }default:{ //unrecognised
      exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
    }
  }
}
//---------------------------------------------------------------------------
void ASMCALL io_write_w(MEM_ADDRESS addr,WORD io_src_w)
{
  if (addr>=0xff8240 && addr<0xff8260){  //palette
#ifdef _DEBUG_BUILD
    debug_check_io_monitor(addr,0,HIBYTE(io_src_w));
    debug_check_io_monitor(addr+1,0,LOBYTE(io_src_w));
#endif
    int n=(addr-0xff8240) >> 1;
    io_src_w&=0xfff;
    if (STpal[n]!=io_src_w){
      STpal[n]=io_src_w;
      PAL_DPEEK(n*2)=STpal[n];
      log_to(LOGSECTION_VIDEO,Str("VIDEO: ")+HEXSl(old_pc,6)+" - Palette change at scan_y="+scan_y+" cycles so far="+(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl));
      if (draw_lock) draw_scanline_to((ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl)+1);
      if (flashlight_flag==0 DEBUG_ONLY( && debug_cycle_colours==0) ){
        palette_convert(n);
      }
    }
  }else{
    io_word_access=true;
    io_write_b(addr,HIBYTE(io_src_w));
    io_write_b(addr+1,LOBYTE(io_src_w));
    io_word_access=0;
  }
}
//---------------------------------------------------------------------------
void ASMCALL io_write_l(MEM_ADDRESS addr,LONG io_src_l)
{
  if (emudetect_called){
    if (addr==0xffc100){
      DWORD addr=io_src_l;
      Str Name=read_string_from_memory(addr,500);
      addr+=Name.Length()+1;

      int Param[10]={0,0,0,0,0,0,0,0,0,0};
      Str Num;
      for (int n=0;n<10;n++){
        Num=read_string_from_memory(addr,16);
        if (Num.Length()==0) break;
        addr+=Num.Length()+1;
        Param[n]=atoi(Num);
      }

      int Sides=2,TracksPerSide=80,SectorsPerTrack=9;
      if (Param[0]==1 || Param[0]==2) Sides=Param[0];
      if (Param[1]>=10 && Param[1]<=FLOPPY_MAX_TRACK_NUM+1) TracksPerSide=Param[1];
      if (Param[2]>=1 && Param[2]<=FLOPPY_MAX_SECTOR_NUM) SectorsPerTrack=Param[2];
      GUIEmudetectCreateDisk(Name,Sides,TracksPerSide,SectorsPerTrack);
      return;
    }
  }
  if (addr==0xffc1f0){
#ifdef _DEBUG_BUILD
    log_write(Str("ST -- ")+read_string_from_memory(io_src_l,500));
    return;
#else
    if (emudetect_write_logs_to_printer){
      // This can't be turned on unless you call emudetect, so 0xffc1f0 will still work normally
      Str Text=read_string_from_memory(io_src_l,500);
      for (int i=0;i<Text.Length();i++) ParallelPort.OutputByte(Text[i]);
      ParallelPort.OutputByte(13);
      ParallelPort.OutputByte(10);
      return;
    }
#endif
  }
  INSTRUCTION_TIME(-4);
  io_write_w(addr,HIWORD(io_src_l));
  INSTRUCTION_TIME(4);
  io_write_w(addr+2,LOWORD(io_src_l));
}

#undef LOGSECTION

