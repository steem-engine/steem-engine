/*---------------------------------------------------------------------------
FILE: emulator.cpp
MODULE: emu
DESCRIPTION: Miscellaneous core emulator functions. An important function is
init_timings that sets up all Steem's counters and clocks. Also included is
the code for Steem's agenda system that schedules tasks to be performed at
the end of scanlines.
---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
void init_timings()
{
  // don't do anything to agendas here!

  disk_light_off_time=timeGetTime()+DisableDiskLightAfter;

  fdc_str&=BYTE(~FDC_STR_MOTOR_ON);
  shifter_first_draw_line=0;
  shifter_last_draw_line=shifter_y;
  if (COLOUR_MONITOR==0) shifter_freq=MONO_HZ;
  CALC_SHIFTER_FREQ_IDX;
  CALC_CYCLES_FROM_HBL_TO_TIMER_B(shifter_freq);

  screen_res_at_start_of_vbl=screen_res;
  shifter_freq_at_start_of_vbl=shifter_freq;
  scanline_time_in_cpu_cycles_at_start_of_vbl=scanline_time_in_cpu_cycles[shifter_freq_idx];
  hbl_pending=true;

  cpu_time_of_start_of_event_plan=0; //0x7f000000; // test overflow
  if (n_cpu_cycles_per_second>8000000){
    screen_event_pointer=event_plan_boosted[shifter_freq_idx];
  }else{
    screen_event_pointer=event_plan[shifter_freq_idx];
  }
  screen_event_vector=screen_event_pointer->event;
  time_of_next_event=cpu_time_of_start_of_event_plan+(screen_event_pointer->time);
  cpu_cycles=(screen_event_pointer->time);
  cpu_timer=time_of_next_event;

  cpu_time_of_last_vbl=ABSOLUTE_CPU_TIME;
  time_of_next_timer_b=cpu_time_of_last_vbl+160000;
  scan_y=-scanlines_above_screen[shifter_freq_idx];
  time_of_last_hbl_interrupt=ABSOLUTE_CPU_TIME;

  cpu_time_of_first_mfp_tick=ABSOLUTE_CPU_TIME;
  shifter_cycle_base=ABSOLUTE_CPU_TIME;

  // This is a hack to make the first screen work
  if (pc==(MEM_ADDRESS)(LPEEK(0x0070) & 0xffffff)){
    // Stopped at VBL
    INSTRUCTION_TIME_ROUND(56+4); //time for the interrupt and the instruction before the VBL
    vbl_pending=0;
  }else{
    vbl_pending=true;
  }

  for (int i=0;i<16;i++){
    mfp_time_of_start_of_last_interrupt[i]=ABSOLUTE_CPU_TIME-CYCLES_FROM_START_OF_MFP_IRQ_TO_WHEN_PEND_IS_CLEARED-2;
  }
  mfp_init_timers();

  shifter_draw_pointer=xbios2;
  shifter_draw_pointer_at_start_of_line=shifter_draw_pointer;
  shifter_pixel=shifter_hscroll; //start by drawing this pixel
  left_border=BORDER_SIDE;right_border=BORDER_SIDE;
  overscan_add_extra=0;
  scanline_drawn_so_far=0;
  cpu_timer_at_start_of_hbl=cpu_time_of_last_vbl-(scanline_time_in_cpu_cycles_at_start_of_vbl-(screen_event_pointer->time));
  shifter_freq_change_idx=0;
  for (int n=0;n<32;n++){
    shifter_freq_change[n]=shifter_freq;
    shifter_freq_change_time[n]=ABSOLUTE_CPU_TIME;
  }
  
  ikbd_joy_poll_line=0;
  ikbd_key_poll_line=0;
  ikbd_mouse_poll_line=0;

  dma_sound_on_this_screen=0;
  dma_sound_output_countdown=0;
  dma_sound_samples_countdown=0;
  dma_sound_channel_buf_last_write_t=0;

#if USE_PASTI
  pasti_update_time=ABSOLUTE_CPU_TIME+8000000;
#endif

  hbl_count=0;
}
//---------------------------------------------------------------------------
#define LOGSECTION LOGSECTION_TRAP

void intercept_os()
{
  if (pc==os_gemdos_vector){
    intercept_gemdos();
    ioaccess|=IOACCESS_INTERCEPT_OS;
  }else if (pc==os_bios_vector){
    intercept_bios();
    ioaccess|=IOACCESS_INTERCEPT_OS;
  }else if (pc==os_xbios_vector){
    intercept_xbios();
    ioaccess|=IOACCESS_INTERCEPT_OS;
  }
  if (hdimg_active) hdimg_intercept(pc);

#ifndef NO_CRAZY_MONITOR
  if (extended_monitor){
    if ((ir&15)==2){ //VDI or AES
#ifdef ENABLE_LOGFILE
      if (logsection_enabled[LOGSECTION_TRAP]) log_os_call(2);
#endif
      if (r[0]==0x73){ //vdi
        MEM_ADDRESS adddd=m68k_lpeek(r[1]); //r[1] has vdi parameter block.  adddd points to the control array
        if (m68k_dpeek(adddd+0)==1){ //v_opnwk OPCODE
          if (vdi_intout==0){ //only do this once
            on_rte=ON_RTE_EMHACK;
            on_rte_interrupt_depth=interrupt_depth;
            vdi_intout=m68k_lpeek(r[1]+12);
          }
        }
      }
    }
    ioaccess|=IOACCESS_INTERCEPT_OS;
  }
#endif
}
//---------------------------------------------------------------------------
void intercept_gemdos()
{
#ifdef ENABLE_LOGFILE
  if (logsection_enabled[LOGSECTION_TRAP]){
    log_os_call(1);
  }
#endif

#ifndef DISABLE_STEMDOS
  stemdos_intercept_trap_1();
#endif
}
//---------------------------------------------------------------------------
void intercept_bios()
{
  bool Invalid=0;
  MEM_ADDRESS sp=get_sp_before_trap(&Invalid);
  if (Invalid) return;

  WORD func=m68k_dpeek(sp);

#ifdef ENABLE_LOGFILE
  if (logsection_enabled[LOGSECTION_TRAP]){
    if (func!=1) log_os_call(13);
  }
#endif

  if (func==10){ // Drvbits
    if (os_hdimg_init_vector==0) hdimg_init_vectors();
    if (disable_input_vbl_count>30) disable_input_vbl_count=0; // If TOS is making BIOS calls then the IKBD int is ready
#ifdef DISABLE_STEMDOS
  }
#else
    stemdos_update_drvbits();  //in case it's BIOS(10)
  }else if (func==7){
    int d=m68k_dpeek(sp+2);
    if (d>=2){
      if (stemdos_check_mount(d)){
        m68k_dpoke(sp+2,0); // Make it get the BPB of A: instead, this might contain nonsense!
      }
    }
  }else if (func==9){ // Mediach
    int d=m68k_dpeek(sp+2);
    if (d>=2){
      if (stemdos_check_mount(d)){
        r[0]=0; // Hasn't changed - everything ignores this anyway
        M68K_PERFORM_RTE(;);  //don't need to check interrupts because sr won't actually have changed
      }
    }
  }
#endif
}
//---------------------------------------------------------------------------
void intercept_xbios()
{
  bool Invalid=0;
  MEM_ADDRESS sp=get_sp_before_trap(&Invalid);
  if (Invalid) return;

#ifdef ENABLE_LOGFILE
  if (logsection_enabled[LOGSECTION_TRAP]){
    log_os_call(14);
  }
#endif

  if (m68k_dpeek(sp)==37 && r[6]==r[7] && r[7]==0x456d753f){ // Vsync with Emu?, emudtect
    r[6]=0x53544565;
    r[7]=0x6d456e67;
    areg[0]=0xffc100;
    emudetect_called=true;
    emudetect_init();

    M68K_PERFORM_RTE(;);  //don't need to check interrupts because sr won't actually have changed
  }else if (m68k_dpeek(sp)==23 && stemdos_intercept_datetime){ // Get clock time
    time_t timer=time(NULL);
    struct tm *lpTime=localtime(&timer);
    r[0]=TMToDOSDateTime(lpTime);

    M68K_PERFORM_RTE(;);  //don't need to check interrupts because sr won't actually have changed
/*
  }else if (m68k_dpeek(sp)==4 && extended_monitor){
    /// Getrez returns different values in TT modes
    int em_mode=-1;
    if (em_width==640 && em_height==480 && em_planes==4) em_mode=4;
    if (em_width==1280 && em_height==960 && em_planes==1) em_mode=6;
    if (em_mode>=0){
      r[0]=em_mode;
      M68K_PERFORM_RTE(;);  //don't need to check interrupts because sr won't actually have changed
    }
*/
  }
}

#undef LOGSECTION
//---------------------------------------------------------------------------
int ACIAClockToHBLS(int ClockDivide,bool MIDI_In)
{
  int HBLs=1;
  // We assume the default setting of 9 bits per byte sent, I'm not sure
  // if the STs ACIAs worked in any other mode. The ACIA master clock is 500kHz.
  if (shifter_freq==MONO_HZ){
    if (ClockDivide==1){ // Divide by 16 (31250 bits per second)
      HBLs=int(HBLS_PER_SECOND_MONO/(500000.0/16.0/9.0) + 1);
    }else if (ClockDivide==2){ // Divide by 64 (7812.5 bits per second)
      HBLs=int(HBLS_PER_SECOND_MONO/(500000.0/64.0/9.0) + 1);
    }
  }else{
    if (ClockDivide==1){ // Divide by 16 (31250 bits per second)
      HBLs=int(HBLS_PER_SECOND_AVE/(500000.0/16.0/9.0) + 1);
    }else if (ClockDivide==2){ // Divide by 64 (7812.5 bits per second)
      HBLs=int(HBLS_PER_SECOND_AVE/(500000.0/64.0/9.0) + 1);
    }
  }
  if (MIDI_In && MIDI_in_speed!=100){
    HBLs*=100;
    HBLs/=MIDI_in_speed;
  }
  return HBLs;
}
//---------------------------------------------------------------------------
void ACIA_Reset(int nACIA,bool Cold)
{
//  if (Cold){
  acia[nACIA].tx_flag=0;
  if (nACIA==NUM_ACIA_IKBD) agenda_delete(agenda_acia_tx_delay_IKBD);
  if (nACIA==NUM_ACIA_MIDI) agenda_delete(agenda_acia_tx_delay_MIDI);
//  }
  acia[nACIA].rx_not_read=0;
  acia[nACIA].overrun=0;
  acia[nACIA].clock_divide=int((nACIA==NUM_ACIA_MIDI) ? 1:2);
  acia[nACIA].tx_irq_enabled=0;
  acia[nACIA].rx_irq_enabled=true;
  acia[nACIA].data=0;
  acia[nACIA].last_tx_write_time=0;
  LOG_ONLY( if (nACIA==0 && acia[nACIA].irq) log_to_section(LOGSECTION_IKBD,EasyStr("IKBD: ACIA reset - Changing ACIA IRQ bit from ")+ACIA_IKBD.irq+" to 0"); )
  acia[nACIA].irq=false;
  if (Cold==0) mfp_gpip_set_bit(MFP_GPIP_ACIA_BIT,!(ACIA_IKBD.irq || ACIA_MIDI.irq));
}
//---------------------------------------------------------------------------
void ACIA_SetControl(int nACIA,BYTE Val)
{
  acia[nACIA].clock_divide=      (Val & b00000011);
  acia[nACIA].tx_irq_enabled=    (Val & b01100000)==BIT_5;
  acia[nACIA].rx_irq_enabled=bool(Val & b10000000);
  LOG_ONLY( if (nACIA==0) log_to(LOGSECTION_IKBD,EasyStr("IKBD: ACIA control set to ")+itoa(Val,d2_t_buf,2)); )

  if (acia[nACIA].tx_irq_enabled){
    acia[nACIA].irq=true;
  }else{
    LOG_ONLY( if (nACIA==0 && ACIA_IKBD.irq) log_to(LOGSECTION_IKBD,EasyStr("IKBD: ACIA set control - Changing ACIA IRQ bit from ")+ACIA_IKBD.irq+" to 0"); )
    acia[nACIA].irq=false;
  }
  mfp_gpip_set_bit(MFP_GPIP_ACIA_BIT,!(ACIA_IKBD.irq || ACIA_MIDI.irq));
}
//---------------------------------------------------------------------------  Agenda
#define LOGSECTION LOGSECTION_AGENDA
int MILLISECONDS_TO_HBLS(int ms)
{
  if (shifter_freq==MONO_HZ) return int((ms*HBLS_PER_SECOND_MONO)/1000);
  return int((ms*HBLS_PER_SECOND_AVE)/1000);
}

void agenda_add(LPAGENDAPROC action,int pause,int param)
{
  if (agenda_length>=MAX_AGENDA_LENGTH){
    log_write("AARRRGGGHH!: Agenda full, can't add!");
    return;
  }

  WIN_ONLY( EnterCriticalSection(&agenda_cs); )
  unsigned long target_time=hbl_count+pause;
  int n=0;
  while (n<agenda_length && (signed int)(agenda[n].time-target_time)>0) n++;

  //budge the n, n+1, ... along
  for (int nn=agenda_length;nn>n;nn--){
    agenda[nn]=agenda[nn-1];
  }
  agenda[n].perform=action;
  agenda[n].time=target_time;
  agenda[n].param=param;
  agenda_next_time=agenda[agenda_length].time;
  agenda_length++;
  log(EasyStr("TASKS: Agenda length = ")+agenda_length);
  WIN_ONLY( LeaveCriticalSection(&agenda_cs); )
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void agenda_delete(LPAGENDAPROC job)
{
  WIN_ONLY( EnterCriticalSection(&agenda_cs); )
  for (int n=0;n<agenda_length;n++){
    if (agenda[n].perform==job){
      for (int nn=n;nn<agenda_length;nn++){
        agenda[nn]=agenda[nn+1];
      }
      agenda_length--;
      n--;
    }
  }
  if (agenda_length){
    agenda_next_time=agenda[agenda_length-1].time;
  }else{
    agenda_next_time=hbl_count-1; //wait 42 hours
  }
  WIN_ONLY( LeaveCriticalSection(&agenda_cs); )
}
//---------------------------------------------------------------------------
int agenda_get_queue_pos(LPAGENDAPROC job)
{
  WIN_ONLY( EnterCriticalSection(&agenda_cs); )
  int n=agenda_length-1;
  for (;n>=0;n--){
    if (agenda[n].perform==job) break;
  }
  WIN_ONLY( LeaveCriticalSection(&agenda_cs); )
  return n;
}
//---------------------------------------------------------------------------
void agenda_acia_tx_delay_IKBD(int)
{
  ACIA_IKBD.tx_flag=0; //finished transmitting
  if (ACIA_IKBD.tx_irq_enabled) ACIA_IKBD.irq=true;
  mfp_gpip_set_bit(MFP_GPIP_ACIA_BIT,!(ACIA_IKBD.irq || ACIA_MIDI.irq));
}
//---------------------------------------------------------------------------
void agenda_acia_tx_delay_MIDI(int)
{
  ACIA_MIDI.tx_flag=0; //finished transmitting
  if (ACIA_MIDI.tx_irq_enabled) ACIA_MIDI.irq=true;
  mfp_gpip_set_bit(MFP_GPIP_ACIA_BIT,!(ACIA_IKBD.irq || ACIA_MIDI.irq));
}
#undef LOGSECTION
//-------------------------------------------------- MMU Confused
MEM_ADDRESS mmu_confused_address(MEM_ADDRESS ad)
{
  int bank=0;
/*
  if(ad>mmu_bank_length[0]+mmu_bank_length[1]){
    return 0xffffff;   //bus error
  }else if(ad>=mmu_bank_length[0]){
    bank=1;
    ad-=mmu_bank_length[0];
  }
*/
  if (ad>FOUR_MEGS){
    return 0xffffff;   //bus error
  }else if (ad>=mmu_bank_length[0]){
    bank=1;
    ad-=mmu_bank_length[0];
    if (ad>=mmu_bank_length[1]) return 0xfffffe; //gap
  }

//  int sec=ad&0xfffe00;
  if (mmu_bank_length[bank]==MB2){
    if (bank_length[bank]==KB128){
//      ad&=0xffe1ff;
//      ad=((ad&0xffe)>>4)+(ad&0x1ff);
      ad=((ad&0xf00000)>>4)
        +((ad&0x03f800)>>2)
         +(ad&0x0001ff);
//                ^lose $c0000       ^lose $600


/*      if(sec&0xf00)return 0xfffffe;     //section absent
      ad-=(0xe00*(sec/0x200));   //close gaps
*/
    }else if (bank_length[bank]==KB512){
//      ad&=0xfff3ff;
      ad=((ad&0xf00000)>>2)
        +((ad&0x07f800)>>1)
         +(ad&0x0003ff);
//                 [ [ [ [
//            ^ lose $80000        ^lose $400

//      ad&=0xffedff;

/*
      if(sec&0x400)return 0xfffffe;    //section absent
      ad-=(0x400*(sec/0x800));  //close gaps
*/  }else if (bank_length[bank]==0){
      ad=0xfffffe; //gap
    }
  }else if (mmu_bank_length[bank]==KB512){
    if (bank_length[bank]==KB128){
      ad=((ad & 0xfff800) >> 2)+(ad & 0x1ff);
    }else if (bank_length[bank]==KB512){
      //do nothing
    }else if (bank_length[bank]==MB2){
//      ad+=(sec/0x400)*0x400;   //add on gaps
 /////      ad*=2;ad+=ad*0x800; // this is nonsense, adds on far too much!
    }else{
      ad=0xfffffe; //gap
    }
  }else if (mmu_bank_length[bank]==KB128){
    if (bank_length[bank]==KB128){
      //do nothing
    }else if (bank_length[bank]==KB512){
      //?????????????????
//      ad+=(sec/0x200)*0x600;   //add on gaps
    }else if (bank_length[bank]==MB2){
      //?????????????????
//      ad+=(sec/0x200)*0xe00;   //add on gaps
    }else{
      ad=0xfffffe; //gap
    }
  }else if (mmu_bank_length[bank]==0){
    ad=0xfffffe; //gap
  }
  if (bank==1 && ad<FOUR_MEGS) ad+=bank_length[0];
  return ad;
}

BYTE ASMCALL mmu_confused_peek(MEM_ADDRESS ad,bool cause_exception)
{
  MEM_ADDRESS c_ad=mmu_confused_address(ad);
  if (c_ad==0xffffff){   //bus error
    if (cause_exception) exception(BOMBS_BUS_ERROR,EA_READ,ad);
    return 0;
  }else if (c_ad==0xfffffe){  //gap in memory
    return 0xff;
  }else if (c_ad<mem_len){
    return PEEK(c_ad);
  }else{
    return 0xff;
  }
}

WORD ASMCALL mmu_confused_dpeek(MEM_ADDRESS ad,bool cause_exception)
{
  MEM_ADDRESS c_ad=mmu_confused_address(ad);
  if (c_ad==0xffffff){   //bus error
    if (cause_exception) exception(BOMBS_BUS_ERROR,EA_READ,ad);
    return 0;
  }else if (c_ad==0xfffffe){  //gap in memory
    return 0xffff;
  }else if (c_ad<mem_len){
    return DPEEK(c_ad);
  }else{
    return 0xffff;
  }
}

LONG ASMCALL mmu_confused_lpeek(MEM_ADDRESS ad,bool cause_exception)
{
  WORD a=mmu_confused_dpeek(ad,cause_exception);
  WORD b=mmu_confused_dpeek(ad+2,cause_exception);
  return MAKELONG(b,a);
}

void ASMCALL mmu_confused_set_dest_to_addr(int bytes,bool cause_exception)
{
  MEM_ADDRESS c_ad=mmu_confused_address(abus);
  if (c_ad==0xffffff){  //bus error
    if (cause_exception) exception(BOMBS_BUS_ERROR,EA_WRITE,abus);
  }else if (c_ad==0xfffffe){  //gap in memory
    m68k_dest=&iobuffer;  //throw away result
  }else if ((c_ad+bytes) <= mem_len){
    m68k_dest=Mem_End-c_ad-bytes;
  }else{
    m68k_dest=&iobuffer;  //throw away result
  }
}

#ifndef NO_CRAZY_MONITOR

void call_a000()
{
  on_rte_return_address=(PC32);
  //now save regs a0,a1,d0 ?
  INSTRUCTION_TIME_ROUND(0);  // Round first for interrupts
  INSTRUCTION_TIME_ROUND(34);
  on_rte=ON_RTE_LINE_A;
  DPEEK(0)=0xa000;
//    m68k_interrupt(LPEEK(BOMBS_LINE_A*4));
  WORD _sr=sr;
  if (!SUPERFLAG) change_to_supervisor_mode();
  m68k_PUSH_L(0);
  m68k_PUSH_W(_sr);
  SET_PC(LPEEK(BOMBS_LINE_A*4));
//  log(EasyStr("interrupt - increasing interrupt depth from ")+interrupt_depth+" to "+(interrupt_depth+1));
  SR_CLEAR(SR_TRACE);
  interrupt_depth++;
  memcpy(save_r,r,16*4);
  on_rte_interrupt_depth=interrupt_depth;
}

void extended_monitor_hack()
{
  em_width&=-16;
  if (line_a_base==0){
    line_a_base=areg[0];
    LPEEK(0)=ROM_LPEEK(0);
    memcpy(r,save_r,15*4);
  }

  int real_planes=em_planes;
  if (screen_res==1) real_planes=2;
//  log_write(EasyStr("doing the em hack - line-A base at ")+HEXSl(line_a_base,6));

  m68k_dpoke(line_a_base-12,WORD(em_width));            //V_REZ_HZ -12  WORD  Horizontal pixel resolution.
  m68k_dpoke(line_a_base-4,WORD(em_height));            //V_REZ_VT -4  WORD  Vertical pixel resolution.
  m68k_dpoke(line_a_base-2,WORD(em_width*real_planes/8)); //BYTES_LIN -2  WORD  Bytes per screen line.
  m68k_dpoke(line_a_base,WORD(real_planes));              //PLANES 0  WORD  Number of planes in the current resolution
  m68k_dpoke(line_a_base+2,WORD(em_width*real_planes/8)); //WIDTH 2  WORD  Width of the destination form in bytes
  int h=8;if(em_planes==1)h=16; //height of a character
  m68k_dpoke(line_a_base-40,WORD(em_width*real_planes*h/8));//V_CEL_WR -40  WORD  Number of bytes between character cells
  m68k_dpoke(line_a_base-44,WORD(em_width/8-1));        //V_CEL_MX -44  WORD  Number of text columns - 1.
  m68k_dpoke(line_a_base-42,WORD(em_height/h-1));      //V_CEL_MY -42  WORD  Number of text rows - 1.

  if (vdi_intout){
//    log_write("Changing intout[0] etc.");
    m68k_dpoke(line_a_base-692,WORD(em_width-1));
    m68k_dpoke(line_a_base-690,WORD(em_height-1));
//    log_write(EasyStr("Wrote the new dimensions to $")+HEXSl(line_a_base-692,6));
    m68k_dpoke(vdi_intout,WORD(em_width-1));
    m68k_dpoke(vdi_intout+2,WORD(em_height-1));
//    log_write(EasyStr("Wrote the new dimensions to $")+HEXSl(vdi_intout,6));
  }else{
    int bytes_required=em_width*em_height*em_planes/8;
    if (bytes_required>32000){
      on_rte=ON_RTE_DONE_MALLOC_FOR_EM;
      on_rte_interrupt_depth=interrupt_depth;
      memcpy(save_r,r,16*4);
      m68k_PUSH_L((bytes_required+255)&-256);
      m68k_PUSH_W(0x48); //malloc

//      log_write(HEXSl(pc,6)+EasyStr("Calling Malloc(")+HEXSl((bytes_required+255)&-256,6));
      m68k_interrupt(os_gemdos_vector);
//      log_write_stack();
      sr|=SR_IPL_7;

/*
      xbios2=(mem_len-bytes_required)&-256;
      LPEEK(SV_memtop)=xbios2;
      LPEEK(SV_v_bas_ad)=xbios2;
      LPEEK(SVscreenpt)=xbios2;
*/
/*
      xbios2=(SV_membot+255)&-256;
      LPEEK(SV_membot)=xbios2+bytes_required;
      LPEEK(SV_v_bas_ad)=xbios2;
      LPEEK(SVscreenpt)=xbios2; */
    }
  }

/*
  int screen_mem_len=em_width*em_planes*em_height/8;
  xbios2=(himem-screen_mem_len)&-256;
  m68k_lpoke(0x42e,xbios2); //set in system variables - end of TPA
  m68k_lpoke(0x44e,xbios2); //set in system variables - _v_bas_ad
*/

/*

From Sasa Rakic


640x400x1 bitplane
~~~~~~~~~~~~~~~~~~
word -12(a0) = screen witdh  ($0280=640 pixels)
word  -4(a0) = screen height ($0190=400 pixels)
word  -2(a0) = scan line length in bytes ($0050=80 bytes)
word   2(a0) = Width of the destination form in bytes ($0050=80)
word -40(a0) = Number of bytes between character cells ($0500=1280)
word -44(a0) = Number of text columns - 1 ($004f=79)
word -42(a0) = Number of text rows - 1 ($0018=24)

VgaWin Values for:

800x600x1 bitplane
~~~~~~~~~~~~~~~~~~
word -12(a0) = screen witdh  ($0320=800 pixels)
word  -4(a0) = screen height ($0258=600 pixels)
word  -2(a0) = scan line length in bytes ($0064=100 bytes)
word   2(a0) = Width of the destination form in bytes ($0064=100)
word -40(a0) = Number of bytes between character cells ($0640=1600)
word -44(a0) = Number of text columns - 1 ($0063=99)
word -42(a0) = Number of text rows - 1 ($0024=36)

Also must be changed screen video memory address (can be set via XBIOS
function or :

move.l d0,$44e.w
swap d0
move.b d0,$ffff8201.w
rol.l #8,d0
move.b do,$ffff8203.w
rol.l #8,d0
move.b d0,$ffff820d.w

*/

}

void emudetect_init()
{
  emudetect_falcon_stpal.Resize(256);
  emudetect_falcon_pcpal.Resize(256);
}

void emudetect_reset()
{
  emudetect_called=0;
  emudetect_write_logs_to_printer=0;
  emudetect_falcon_stpal.DeleteAll();
  emudetect_falcon_pcpal.DeleteAll();
  emudetect_falcon_mode=0;
  emudetect_falcon_mode_size=1;
}

void emudetect_falcon_palette_convert(int n)
{
  if (BytesPerPixel==1 || emudetect_called==0) return; // 256 mode could cause slow down, can't make it work.

  DWORD val=emudetect_falcon_stpal[n];
  emudetect_falcon_pcpal[n]=colour_convert(DWORD_B(&val,0),DWORD_B(&val,1),DWORD_B(&val,3));
}

void ASMCALL emudetect_falcon_draw_scanline(int border1,int picture,int border2,int hscroll)
{
  if (emudetect_called==0 || draw_lock==0) return;

  int st_line_bytes=emudetect_falcon_mode_size*320 * emudetect_falcon_mode;

  MEM_ADDRESS source=shifter_draw_pointer & 0xffffff;
  if (source+st_line_bytes>mem_len) return;

  int wh_mul=1;
  if (emudetect_falcon_mode_size==1 && draw_blit_source_rect.right>320+BORDER_SIDE+BORDER_SIDE){
    wh_mul=2;
  }

  // border always multiple of 4 so write using longs only
  DWORD bord_col=0xffffffff;
  if (emudetect_falcon_mode==1) bord_col=emudetect_falcon_pcpal[0];
  if (BytesPerPixel==1) border1/=4, border2/=4, bord_col|=(bord_col & 0xffff) << 16;
  if (BytesPerPixel==2) border1/=2, border2/=2;
  int DestInc=4;
  if (BytesPerPixel==3) DestInc=3;

  for (int y=0;y<wh_mul;y++){
    LPDWORD plDest=LPDWORD(draw_dest_ad);
    for (int x=0;x<wh_mul;x++){
      for (int n=border1;n>0;n--){
        *plDest=bord_col;
        plDest=LPDWORD(LPBYTE(plDest)+DestInc);
      }
    }

    LPBYTE pbDest=LPBYTE(plDest);
    LPWORD pwDest=LPWORD(plDest);
    source+=hscroll*emudetect_falcon_mode;
    if (emudetect_falcon_mode==1){
      DWORD col;
      for (int n=picture;n>0;n--){
        col=emudetect_falcon_pcpal[PEEK(source++)];
        for (int x=0;x<wh_mul;x++){
          switch (BytesPerPixel){
            case 1: *(pbDest++)=BYTE(col); break;
            case 2: *(pwDest++)=WORD(col); break;
            case 3: *(plDest)=col; plDest=LPDWORD(LPBYTE(plDest)+3); break;
            case 4: *(plDest++)=col; break;
          }
        }
      }
    }else if (emudetect_falcon_mode==2){
      DWORD src;
      for (int n=picture;n>0;n--){
        src=DPEEK(source);source+=2;
        int r=(src & MAKEBINW(b11111000,b00000000)) >> 11;
        int g=(src & MAKEBINW(b00000111,b11100000)) >> 5;
        int b=(src & MAKEBINW(b00000000,b00011111));
        for (int x=0;x<wh_mul;x++){
          switch (BytesPerPixel){
            case 1: *(pbDest++)=0; break;
            case 2:
              if (rgb555){
                *(pwDest++)=WORD((r << (15-5)) | ((g & ~1) << (10-6)) | b);
              }else{
                *(pwDest++)=WORD((r << (16-5)) | (g << (11-6)) | b);
              }
              break;
            case 3:case 4:
              *(plDest)=( (r << (24-5)) | (g << (16-6)) | (b << (8-5)) ) << rgb32_bluestart_bit;
              plDest=LPDWORD(LPBYTE(plDest)+DestInc);
              break;
          }
        }
      }
    }
    if (BytesPerPixel==1) plDest=LPDWORD(pbDest);
    if (BytesPerPixel==2) plDest=LPDWORD(pwDest);
    for (int x=0;x<wh_mul;x++){
      for (int n=border2;n>0;n--){
        *plDest=bord_col;
        plDest=LPDWORD(LPBYTE(plDest)+DestInc);
      }
    }
    draw_dest_ad+=draw_line_length;
  }
}

#endif

