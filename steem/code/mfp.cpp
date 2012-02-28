/*---------------------------------------------------------------------------
FILE: mfp.cpp
MODULE: emu
DESCRIPTION: The core of Steem's Multi Function Processor emulation
(MFP 68901). This chip handles most of the interrupt and timing functions in
the ST.
---------------------------------------------------------------------------*/
// SS: like in IO, more comments & changes, MFP=major F. pain
//---------------------------------------------------------------------------
void mfp_gpip_set_bit(int bit,bool set)
{
  BYTE mask=BYTE(1 << bit);
  BYTE set_mask=BYTE(set ? mask:0);
  BYTE cur_val=(mfp_reg[MFPR_GPIP] & mask);
  if (cur_val==set_mask) return; //no change
  bool old_1_to_0_detector_input=(cur_val ^ (mfp_reg[MFPR_AER] & mask))==mask;
  mfp_reg[MFPR_GPIP]&=BYTE(~mask);
  mfp_reg[MFPR_GPIP]|=set_mask;
  // If the DDR bit is low then the bit from the io line is used,
  // if it is high interrupts then it comes from the input buffer.
  // In that case interrupts are handled in the write to the GPIP.
  if (old_1_to_0_detector_input && (mfp_reg[MFPR_DDR] & mask)==0){
    // Transition the right way! Make the interrupt pend (don't cause an intr
    // straight away in case another more important one has just happened).
    mfp_interrupt_pend(mfp_gpip_irq[bit],ABSOLUTE_CPU_TIME);
    ioaccess|=IOACCESS_FLAG_FOR_CHECK_INTRS;
  }
}
//---------------------------------------------------------------------------
void calc_time_of_next_timer_b()
{
  int cycles_in=int(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl);
  if (cycles_in<cpu_cycles_from_hbl_to_timer_b){
    if (scan_y>=shifter_first_draw_line && scan_y<shifter_last_draw_line){
      time_of_next_timer_b=cpu_timer_at_start_of_hbl+cpu_cycles_from_hbl_to_timer_b+TB_TIME_WOBBLE;
    }else{
      time_of_next_timer_b=cpu_timer_at_start_of_hbl+160000;  //put into future
    }
  }else{
    time_of_next_timer_b=cpu_timer_at_start_of_hbl+160000;  //put into future
  }
}
//---------------------------------------------------------------------------
inline BYTE mfp_get_timer_control_register(int n)
{
  if (n==0){
    return mfp_reg[MFPR_TACR];
  }else if (n==1){
    return mfp_reg[MFPR_TBCR];
  }else if (n==2){
    return BYTE((mfp_reg[MFPR_TCDCR] & b01110000) >> 4);
  }else{
    return BYTE(mfp_reg[MFPR_TCDCR] & b00000111);
  }
}

// Use this for single-bit transitions in the GPIP
void mfp_gpip_transition(int bitnum,bool is_0_1_transition)
{
  // Zero for falling edge, 1 for rising edge
  bool edge=mfp_reg[MFPR_AER] & (1<<bitnum); 
  if(!(edge^is_0_1_transition)){
    int irq=mfp_gpip_irq[bitnum]; // Store for faster macro
    mfp_interrupt(irq,ABSOLUTE_CPU_TIME);
  }
}

inline bool mfp_set_pending(int irq,int when_set)
{
//  ASSERT( abs_quick(when_set-mfp_time_of_start_of_last_interrupt[irq])>=CYCLES_FROM_START_OF_MFP_IRQ_TO_WHEN_PEND_IS_CLEARED);
  if(abs_quick(when_set-mfp_time_of_start_of_last_interrupt[irq])
#if defined(STEVEN_SEAGAL) && defined(SS_MFP_PENDING) // fixes Final Conflict
    >=CYCLES_FROM_START_OF_MFP_IRQ_TO_WHEN_PEND_IS_CLEARED + MFP_CYCLES-8)
#else
    >=CYCLES_FROM_START_OF_MFP_IRQ_TO_WHEN_PEND_IS_CLEARED)
#endif
  {
    // Set pending
    mfp_reg[MFPR_IPRA+mfp_interrupt_i_ab(irq)]|=mfp_interrupt_i_bit(irq); 
    return true;
  }
  return (mfp_reg[MFPR_IPRA+mfp_interrupt_i_ab(irq)] 
    & mfp_interrupt_i_bit(irq))!=0;
}

#define LOGSECTION LOGSECTION_MFP_TIMERS
void mfp_set_timer_reg(int reg,BYTE old_val,BYTE new_val)
{
  int timer=0;
  BYTE new_control;
  if (reg>=MFPR_TACR && reg<=MFPR_TCDCR){ //control reg change
    new_control=BYTE(new_val & 15);
    switch (reg){
      case MFPR_TACR: timer=0; break;
      case MFPR_TBCR: timer=1; break;
      case MFPR_TCDCR: //TCDCR
        timer=2; //we'll do D too
        new_control=BYTE((new_val >> 4) & 7);
        break;
    }
    INSTRUCTION_TIME(12); // The MFP doesn't do anything until 12 cycles after the write
    do{
      if (mfp_get_timer_control_register(timer)!=new_control){
        new_control&=7;
        log( EasyStr("MFP: ")+HEXSl(old_pc,6)+" - Changing timer "+char('A'+timer)+" control; current time="+
              ABSOLUTE_CPU_TIME+"; old timeout="+mfp_timer_timeout[timer]+";"
              "\r\n           ("+(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl)+
              " cycles into scanline #"+scan_y+")" );

        // This ensures that mfp_timer_counter is set to the correct value just
        // in case the data register is read while timer is stopped or the timer
        // is restarted before a write to the data register.
        // prescale_count is the number of MFP_CLKs there has been since the
        // counter last decreased.
        int prescale_count=mfp_calc_timer_counter(timer);

        if (new_control){ // Timer running in delay mode

          mfp_timer_timeout[timer]=ABSOLUTE_CPU_TIME;
          mfp_timer_timeout[timer]+=int(double(mfp_timer_prescale[new_control]*mfp_timer_counter[timer]/64)*CPU_CYCLES_PER_MFP_CLK);
//          mfp_timer_timeout[timer]=ABSOLUTE_CPU_TIME+
//              (mfp_timer_prescale[new_control]*(mfp_timer_counter[timer])*125)/MFP_CLK;
              //*8000/MFP_CLK for MFP cycles, /64 for counter resolution
          mfp_timer_enabled[timer]=mfp_interrupt_enabled[mfp_timer_irq[timer]];

          // To make this more accurate for short timers, we should store the fractional
          // part as well.  Then every time it times out, increase the fractional part and
          // see if it goes over one.  If it does, make the next time-out a bit later.
          mfp_timer_period[timer]=int( double(mfp_timer_prescale[new_control]*int(BYTE_00_TO_256(mfp_reg[MFPR_TADR+timer]))) * CPU_CYCLES_PER_MFP_CLK);

          // Here mfp_timer_timeout assumes that the next MFP_CLK tick happens
          // at exactly 3.24 cycles from when the timer is started, but that isn't
          // what really happens. Below we adjust for the fixed boundary of the clock.
          // We also handle prescale (changing between different divides when running)

          // This makes sure that we don't go back in time more than one count
          // It may be that the MFP checks prescale_count==mfp_timer_prescale
          // when it decides when to count, in that case we need a rethink
          // (the timer would fire much later).
          prescale_count=min(prescale_count,mfp_timer_prescale[new_control]);

          // Make manageable time (cpu_time_of_first_mfp_tick is updated every VBL)
          mfp_timer_timeout[timer]-=cpu_time_of_first_mfp_tick;
          
          // Convert to MFP cycles
          mfp_timer_timeout[timer]*=MFP_CLK;
          mfp_timer_timeout[timer]/=8000;

          // Take off number of cycles already counted
          mfp_timer_timeout[timer]-=prescale_count;

          // Convert back to CPU time
          mfp_timer_timeout[timer]*=8000;
          mfp_timer_timeout[timer]/=MFP_CLK;

          // Make absolute time again
          mfp_timer_timeout[timer]+=cpu_time_of_first_mfp_tick;

          log(EasyStr("    Set control to ")+new_control+
                " (reg=$"+HEXSl(new_val,2)+"); data="+mfp_reg[MFPR_TADR+timer]+
                "; counter="+mfp_timer_counter[timer]/64+
                "; period="+mfp_timer_period[timer]+
                "; new timeout="+mfp_timer_timeout[timer]);
        }else{  //timer stopped, or in event count mode
          // This checks all timers to see if they have timed out, if they have then
          // it will set the pend bit. This is dangerous, messes up LXS!
//          mfp_check_for_timer_timeouts();

          mfp_timer_enabled[timer]=false;
          mfp_timer_period_change[timer]=0;
          log(EasyStr("  Set control to ")+new_control+" (reg=$"+HEXSl(new_val,2)+")"+
                "; counter="+mfp_timer_counter[timer]/64+" ;"+
                LPSTR((timer<2 && (new_val & BIT_3)) ? "event count mode.":"stopped.") );
        }
        if (timer==3) RS232_CalculateBaud(bool(mfp_reg[MFPR_UCR] & BIT_7),new_control,0);
      }
      timer++;
      new_control=BYTE(new_val & 7); // Timer D control
    }while (timer==3);
    INSTRUCTION_TIME(-12);

    if (reg==MFPR_TBCR && new_val==8) calc_time_of_next_timer_b();// SS: general use

#ifdef ENABLE_LOGGING
    if (reg<=MFPR_TBCR && new_val>8){
      log("MFP: --------------- PULSE EXTENSION MODE!! -----------------");
      TRACE("MFP: --------------- PULSE EXTENSION MODE!! -----------------");
    }
#endif

    prepare_event_again();
  }else if (reg>=MFPR_TADR && reg<=MFPR_TDDR){ //data reg change
    timer=reg-MFPR_TADR;
    log(Str("MFP: ")+HEXSl(old_pc,6)+" - Changing timer "+char(('A')+timer)+" data reg to "+new_val+" ($"+HEXSl(new_val,2)+") "+
          " at time="+ABSOLUTE_CPU_TIME+" ("+(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl)+
          " cycles into scanline #"+scan_y+"); timeout is "+mfp_timer_timeout[timer]);
    new_control=BYTE(mfp_get_timer_control_register(timer));

    if (new_control==0){  // timer stopped
      mfp_timer_counter[timer]=((int)BYTE_00_TO_256(new_val))*64;
      mfp_timer_period[timer]=int(double(mfp_timer_prescale[new_control]*int(BYTE_00_TO_256(new_val)))*CPU_CYCLES_PER_MFP_CLK);
    }else if (new_control & 7){
      // Need to calculate the period next time the timer times out
      mfp_timer_period_change[timer]=true;
      if (mfp_timer_enabled[timer]==0){
        // If it is disabled it could be in the past, causing instant
        // event_timer_?_timeout, so realign it
        int stage=mfp_timer_timeout[timer]-ABSOLUTE_CPU_TIME;
        if (stage<0) stage+=((-stage/mfp_timer_period[timer])+1)*mfp_timer_period[timer];
        stage%=mfp_timer_period[timer];
        mfp_timer_timeout[timer]=ABSOLUTE_CPU_TIME+stage; //realign
      }
    }
    log(EasyStr("     Period is ")+mfp_timer_period[timer]+" cpu cycles");
    if (reg==MFPR_TDDR && new_val!=old_val){
      RS232_CalculateBaud(bool(mfp_reg[MFPR_UCR] & BIT_7),new_control,0);
    }
  }
}

void mfp_init_timers() // For load state and CPU speed change
{
  MFP_CALC_INTERRUPTS_ENABLED;
  MFP_CALC_TIMERS_ENABLED;
  for (int timer=0;timer<4;timer++){
    BYTE cr=mfp_get_timer_control_register(timer);
    if (cr & 7){ // Not stopped or in event count mode
      // This must allow for counter not being a multiple of 64
      mfp_timer_timeout[timer]=ABSOLUTE_CPU_TIME+int((double(mfp_timer_prescale[cr])*
                      double(mfp_timer_counter[timer])/64.0)*CPU_CYCLES_PER_MFP_CLK);
      mfp_timer_period_change[timer]=true;
    }
  }
  RS232_CalculateBaud(bool(mfp_reg[MFPR_UCR] & BIT_7),mfp_get_timer_control_register(3),true);
}

int mfp_calc_timer_counter(int timer)
{
  BYTE cr=mfp_get_timer_control_register(timer);
  if (cr & 7){
    int stage=mfp_timer_timeout[timer]-ABSOLUTE_CPU_TIME;
    if (stage<0){
      MFP_CALC_TIMER_PERIOD(timer);
      stage+=((-stage/mfp_timer_period[timer])+1)*mfp_timer_period[timer];
    }
    stage%=mfp_timer_period[timer];
    // so stage is a number from 0 to mfp_timer_period-1

    int ticks_per_count=mfp_timer_prescale[cr & 7];
    // Convert to number of MFP cycles until timeout
    stage=int(double(stage)/CPU_CYCLES_PER_MFP_CLK);
    mfp_timer_counter[timer]=(stage/ticks_per_count)*64 + 64;
    // return the number of prescale counts done so far
    return ticks_per_count-((stage % ticks_per_count)+1);
  }
  return 0;
}

#undef LOGSECTION

// called by run when cpu_cycles < 0
void ASMCALL check_for_interrupts_pending()
{

  if (STOP_INTS_BECAUSE_INTERCEPT_OS==0){
    if ((ioaccess & IOACCESS_FLAG_DELAY_MFP)==0){
      for (int irq=15;irq>=0;irq--)
      {
        BYTE i_bit=BYTE(1 << (irq & 7));
        int i_ab=1-((irq & 8) >> 3);
        if (mfp_reg[MFPR_ISRA+i_ab] & i_bit){ //interrupt in service
          break;  //time to stop looking for pending interrupts
        }
        if (mfp_reg[MFPR_IPRA+i_ab] & i_bit){ //is this interrupt pending?
          if (mfp_reg[MFPR_IMRA+i_ab] & i_bit){ //is it not masked out?
            mfp_interrupt(irq,ABSOLUTE_CPU_TIME); //then cause interrupt
            break;        //lower priority interrupts not allowed now.
          }
        }
      }//nxt irq
    }
    if (vbl_pending){
      if ((sr & SR_IPL)<SR_IPL_4){
        VBL_INTERRUPT
#if defined(SS_VID_HATARI)
//        if(LPEEK(0x70)!=0xFC06DE) TRACE("VBL %d vector %X\n",nVBLs,LPEEK(0x70));
#endif
      }
    }
    if (hbl_pending){
      if ((sr & SR_IPL)<SR_IPL_2){
        // Make sure this HBL can't occur when another HBL has already happened
        // but the event hasn't fired yet.
        if (int(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl)<scanline_time_in_cpu_cycles_at_start_of_vbl){
          HBL_INTERRUPT;
        }
#if defined(SS_DEBUG)
        else if(LPEEK(0x0068)<0xFC0000) TRACE("no hbl %X\n",LPEEK(0x0068));
#endif
      }
    }
  }
  prepare_event_again();
}
//---------------------------------------------------------------------------
void mfp_interrupt(int irq,int when_fired)
{
  log_to_section(LOGSECTION_INTERRUPTS,EasyStr("INTERRUPT: MFP IRQ #")+irq+" ("+(char*)name_of_mfp_interrupt[irq]+
                                        ") at PC="+HEXSl(pc,6)+" at time "+ABSOLUTE_CPU_TIME);
  if (mfp_interrupt_enabled[irq]){
    log_to_section(LOGSECTION_INTERRUPTS,EasyStr("  enabled"));
    if (mfp_set_pending(irq,when_fired)==0){
      log_to_section(LOGSECTION_INTERRUPTS,EasyStr("  but ignored due to MFP clearing pending after it was set"));
      TRACE("MFP irq ignored\n");
    }else{
      if ((mfp_reg[MFPR_IMRA+mfp_interrupt_i_ab(irq)] & mfp_interrupt_i_bit(irq))==0){
        log_to_section(LOGSECTION_INTERRUPTS,EasyStr("  but masked"));
      }else if ((mfp_reg[MFPR_ISRA+mfp_interrupt_i_ab(irq)] & (-mfp_interrupt_i_bit(irq))) || (mfp_interrupt_i_ab(irq) && mfp_reg[MFPR_ISRA])){
        log_to_section(LOGSECTION_INTERRUPTS,EasyStr("  but outprioritized - ISR = ")+HEXSl(mfp_reg[MFPR_ISRA],2)+HEXSl(mfp_reg[MFPR_ISRB],2));
      }else{
        if ((sr & SR_IPL) < SR_IPL_6){
          if ((ioaccess & (IOACCESS_FLAG_DELAY_MFP | IOACCESS_INTERCEPT_OS | IOACCESS_INTERCEPT_OS2))==0){
            M68K_UNSTOP;
            mfp_reg[MFPR_IPRA+mfp_interrupt_i_ab(irq)]&=BYTE(~mfp_interrupt_i_bit(irq));
            if (MFP_S_BIT){
              mfp_reg[MFPR_ISRA+mfp_interrupt_i_ab(irq)]|=mfp_interrupt_i_bit(irq);
            }else{
              mfp_reg[MFPR_ISRA+mfp_interrupt_i_ab(irq)]&=BYTE(~mfp_interrupt_i_bit(irq));
            }
            MEM_ADDRESS vector;
            vector=    (mfp_reg[MFPR_VR] & 0xf0)  +(irq);
            vector*=4;
            mfp_time_of_start_of_last_interrupt[irq]=ABSOLUTE_CPU_TIME;
#if defined(STEVEN_SEAGAL) && defined(SS_INTERRUPT) 
            INSTRUCTION_TIME_ROUND(MFP_CYCLES); // just removing a magic value
#else
            INSTRUCTION_TIME_ROUND(56);
#endif
            m68k_interrupt(LPEEK(vector));
            sr=WORD((sr & (~SR_IPL)) | SR_IPL_6);
            log_to_section(LOGSECTION_INTERRUPTS,EasyStr("  IRQ fired - vector=")+HEXSl(LPEEK(vector),6));
            debug_check_break_on_irq(irq);
          }else{
            log_to_section(LOGSECTION_INTERRUPTS,EasyStr("  MFP is too busy to request interrupt"));
          }
        }else{
          log_to_section(LOGSECTION_INTERRUPTS,EasyStr("  masked by CPU (IPL too high)"));
        }
      }
    }
  }
  LOG_ONLY( else{log_to_section(LOGSECTION_INTERRUPTS,"  disabled");}  )
}
//---------------------------------------------------------------------------
#undef LOGSECTION

