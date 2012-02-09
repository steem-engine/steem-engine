/*---------------------------------------------------------------------------
FILE: reset.cpp
MODULE: emu
DESCRIPTION: Functions to reset the emulator to a startup state.
---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
void power_on()
{
  ZeroMemory(Mem+MEM_EXTRA_BYTES,mem_len);
  on_rte=ON_RTE_RTE;
  SET_PC(rom_addr);
  for (int n=0;n<16;n++) r[n]=0;
  other_sp=0;
  ioaccess=0;

  xbios2=mem_len-0x8000;
  if (extended_monitor) xbios2=max(int(mem_len-(em_width*em_height*em_planes)/8),0);

  ZeroMemory(STpal,sizeof(STpal));
  if (tos_version>0x104){
    STpal[0]=0xfff;
  }else if (tos_version>0){
    STpal[0]=0x777;
  }
  for (int n=0;n<16;n++) PAL_DPEEK(n*2)=STpal[n];
  palette_convert_all();

  DEBUG_ONLY( if (debug_wipe_log_on_reset) logfile_wipe(); )

  log_write("************************* Power On ************************");

  osd_init_run(0);

  mixed_output=0;

  floppy_mediach[0]=0;
  floppy_mediach[1]=0;

  os_gemdos_vector=0;
  os_bios_vector=0;
  os_xbios_vector=0;
  
#ifndef DISABLE_STEMDOS
  stemdos_reset();
#endif

  interrupt_depth=0;
  hbl_count=0;
  agenda_length=0;

  LPEEK(0)=ROM_LPEEK(0);
  LPEEK(4)=ROM_LPEEK(4);

  sr=0x2700;

  emudetect_reset();
  snapshot_loaded=0;

  fdc_str=BIT_2;
  for (int floppyno=0;floppyno<2;floppyno++){
    floppy_head_track[floppyno]=0;
  }
  fdc_tr=0;fdc_sr=0;fdc_dr=0;

  psg_reg_select=0;
  psg_reg_data=0;

  floppy_irq_flag=0;
  fdc_spinning_up=0;
  floppy_type1_command_active=2;

  hdimg_reset();

  reset_peripherals(true);

  init_screen();
  init_timings();
  hbl_pending=false;

  disable_input_vbl_count=50*3; // 3 seconds
}
//---------------------------------------------------------------------------
#define LOGSECTION LOGSECTION_ALWAYS
void reset_peripherals(bool Cold)
{
  log("***** reset peripherals ****");

#ifndef NO_CRAZY_MONITOR
  if (extended_monitor){
    if (em_planes==1){
      screen_res=2;
    }else{
      screen_res=0;
    }
    shifter_freq=50;
    shifter_freq_idx=0;
  }else
#endif

  if (COLOUR_MONITOR){
    screen_res=0;
    shifter_freq=60;
    shifter_freq_idx=1;
  }else{
    screen_res=2;
    shifter_freq=MONO_HZ;
    shifter_freq_idx=2;
  }
  shifter_hscroll=0;
  shifter_hscroll_extra_fetch=0;
  shifter_fetch_extra_words=0; //unspecified
  shifter_first_draw_line=0;
  shifter_last_draw_line=shifter_y;
  CALC_CYCLES_FROM_HBL_TO_TIMER_B(shifter_freq);
  vbl_pending=false;

  dma_status=1;  //no error, apparently
  dma_mode=0;
  dma_sector_count=0xffff;
  fdc_read_address_buffer_len=0;
  dma_bytes_written_for_sector_count=0;

#if USE_PASTI
  if (hPasti){
//    log_to(LOGSECTION_PASTI,"PASTI: Reset, calling HwReset()");
    pasti->HwReset(Cold);
  }
#endif

  ZeroMemory(mfp_reg,sizeof(mfp_reg));
  mfp_reg[MFPR_GPIP]=mfp_gpip_no_interrupt;
  mfp_reg[MFPR_AER]=0x4;   // CTS goes the other way
  mfp_reg[MFPR_TSR]=BIT_7 | BIT_4;  //buffer empty | END
  for (int timer=0;timer<4;timer++) mfp_timer_counter[timer]=256*64;
  MFP_CALC_INTERRUPTS_ENABLED;
  MFP_CALC_TIMERS_ENABLED;

  dma_sound_control=0;
  dma_sound_start=0,next_dma_sound_start=0;
  dma_sound_end=0,next_dma_sound_end=0;
  dma_sound_fetch_address=dma_sound_start;
  dma_sound_mode=BIT_7;
  dma_sound_freq=dma_sound_mode_to_freq[0];
  dma_sound_output_countdown=0;
  dma_sound_samples_countdown=0;
  dma_sound_last_word=MAKEWORD(128,128);

  MicroWire_Mask=0x07ff,MicroWire_Data=0;
  dma_sound_volume=40;
  dma_sound_l_volume=20;
  dma_sound_r_volume=20;
  dma_sound_l_top_val=128;
  dma_sound_r_top_val=128;
  dma_sound_mixer=1;

  ACIA_Reset(NUM_ACIA_IKBD,true);

  ikbd_reset(true); // Always cold reset, soft reset is different

  ACIA_Reset(NUM_ACIA_MIDI,true);

  MIDIPort.Reset();
  ParallelPort.Reset();
  SerialPort.Reset();

  for (int n=0;n<16;n++) psg_reg[n]=0;
  psg_reg[PSGR_PORT_A]=0xff; // Set DTR RTS, no drive, side 0, strobe and GPO

  RS232_CalculateBaud(bool(mfp_reg[MFPR_UCR] & BIT_7),mfp_get_timer_control_register(3),true);
  RS232_VBL();

  ZeroMemory(&Blit,sizeof(Blit));

  cpu_stopped=false;

  if (runstate==RUNSTATE_RUNNING) prepare_event_again();
}
#undef LOGSECTION
//---------------------------------------------------------------------------
void reset_st(DWORD flags)
{
  bool Stop=bool(flags & RESET_NOSTOP)==0;
  bool Warm=bool(flags & RESET_WARM);
  bool ChangeSettings=bool(flags & RESET_NOCHANGESETTINGS)==0;
  bool Backup=bool(flags & RESET_NOBACKUP)==0;
  
  if (runstate==RUNSTATE_RUNNING && Stop) runstate=RUNSTATE_STOPPING;
  if (Backup) GUISaveResetBackup();

  if (Warm){
    reset_peripherals(0);
#ifndef DISABLE_STEMDOS
    stemdos_set_drive_reset();
#endif
    SET_PC(LPEEK(4));

    if (runstate==RUNSTATE_STOPPED){
      // Hack alert! Show user reset has happened
      MEM_ADDRESS old_x2=xbios2;
      xbios2=0;
      draw_end();
      draw(0);
      xbios2=old_x2;
    }
  }else{
    if (ChangeSettings) GUIColdResetChangeSettings();
    power_on();
    palette_convert_all();

    if (ResChangeResize) StemWinResize();

    draw_end();
    draw(true);
  }
  DEBUG_ONLY( debug_reset(); )
  shifter_freq_at_start_of_vbl=shifter_freq;
  PasteIntoSTAction(STPASTE_STOP);
  CheckResetIcon();
  CheckResetDisplay();
  
#ifndef NO_CRAZY_MONITOR
  /////
  line_a_base=0;
  vdi_intout=0;
  aes_calls_since_reset=0;
  if (extended_monitor) extended_monitor=1; //first stage of extmon init
#endif
}



