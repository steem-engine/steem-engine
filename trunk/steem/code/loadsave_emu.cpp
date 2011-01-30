//---------------------------------------------------------------------------
void ReadWriteVar(void *lpVar,DWORD szVar,NOT_ONEGAME( FILE *f ) ONEGAME_ONLY( BYTE* &pMem ),
                        int LoadOrSave,int Type,int Version)
{
  bool SaveSize;
  if (Type==0){ // Variable
    SaveSize=(Version==17);
  }else if (Type==1){ // Array
    SaveSize=(Version>=3);
  }else{  // Struct
    SaveSize=(Version>=5);
  }

#ifndef ONEGAME
  if (SaveSize==0){
    if (LoadOrSave==LS_SAVE){
      fwrite(lpVar,1,szVar,f);
    }else{
      fread(lpVar,1,szVar,f);
    }
//    log_write(Str(szVar));
  }else{
    if (LoadOrSave==LS_SAVE){
      fwrite(&szVar,1,sizeof(szVar),f);
      fwrite(lpVar,1,szVar,f);
//      log_write(Str("Block, l=")+szVar);
    }else{
      DWORD l=0;
      fread(&l,1,sizeof(l),f);
      if (szVar<l){
        fread(lpVar,1,szVar,f);
        fseek(f,l-szVar,SEEK_CUR);
      }else{
        fread(lpVar,1,l,f);
      }
//      log_write(Str("Block, l=")+l);
    }
  }
#else
  if (LoadOrSave==LS_LOAD){
    BYTE *pVar=(BYTE*)lpVar;
    if (SaveSize==0){
      for (DWORD n=0;n<szVar;n++) *(pVar++)=*(pMem++);
    }else{
      DWORD l=*LPDWORD(pMem);pMem+=4;
      for (DWORD n=0;n<l;n++){
        BYTE b=*(pMem++);
        if (n<szVar) *(pVar++)=b;
      }
    }
  }
#endif
}
//---------------------------------------------------------------------------
int ReadWriteEasyStr(EasyStr &s,NOT_ONEGAME( FILE *f ) ONEGAME_ONLY( BYTE* &pMem ),int LoadOrSave,int)
{
#ifndef ONEGAME
  int l;
  if (LoadOrSave==LS_SAVE){
    l=s.Length();
    fwrite(&l,1,sizeof(l),f);
    fwrite(s.Text,1,l,f);
//    log_write(Str("String, l=")+l);
  }else{
    l=-1;
    fread(&l,1,sizeof(l),f);
//    log_write(Str("String, l=")+l);
    if (l<0 || l>260) return 2; // Corrupt snapshot
    s.SetLength(l);
    if (l) fread(s.Text,1,l,f);
  }
#else
  if (LoadOrSave==LS_LOAD){
    int l=*(int*)(pMem);pMem+=4;
    if (l<0 || l>260) return 2; // Corrupt snapshot
    s.SetLength(l);
    char *pT=s.Text;
    for (int n=0;n<l;n++) *(pT++)=(char)*(pMem++);
  }
#endif

  return 0;
}

#define ReadWrite(var) ReadWriteVar(&(var),sizeof(var),f,LoadOrSave,0,Version)
#define ReadWriteArray(var) ReadWriteVar(var,sizeof(var),f,LoadOrSave,1,Version)
#define ReadWriteStruct(var) ReadWriteVar(&(var),sizeof(var),f,LoadOrSave,2,Version)
#define ReadWriteStr(s) {int i=ReadWriteEasyStr(s,f,LoadOrSave,Version);if (i) return i; }
//---------------------------------------------------------------------------
int LoadSaveAllStuff(NOT_ONEGAME( FILE *f ) ONEGAME_ONLY( BYTE* &f ),
                      bool LoadOrSave,int Version,bool NOT_ONEGAME( ChangeDisksAndCart ),int *pVerRet)
{
  ONEGAME_ONLY( BYTE *pStartByte=f; )

  if (Version==-1) Version=SNAPSHOT_VERSION;
  ReadWrite(Version);        //4
  if (pVerRet) *pVerRet=Version;

  ReadWrite(pc);             //4
  ReadWrite(pc_high_byte);   //4

  ReadWriteArray(r);
  ReadWrite(sr);             //2
  ReadWrite(other_sp);       //4

  ReadWrite(xbios2);         //4

  ReadWriteArray(STpal);

  ReadWrite(interrupt_depth); //4
  ReadWrite(on_rte);          //4
  ReadWrite(on_rte_interrupt_depth); //4

  ReadWrite(shifter_draw_pointer); //4
  ReadWrite(shifter_freq);         //4
  if (shifter_freq>65) shifter_freq=MONO_HZ;
  ReadWrite(shifter_x);            //4
  ReadWrite(shifter_y);            //4
  ReadWrite(shifter_scanline_width_in_bytes); //4
  ReadWrite(shifter_fetch_extra_words);       //4
  ReadWrite(shifter_hscroll);                 //4
  if (Version==17 || Version==18){ // The unreleased freak versions!
    ReadWrite(screen_res);
  }else{
    char temp=char(screen_res);
    ReadWrite(temp);                //1
    screen_res=temp;
  }

  ReadWrite(mmu_memory_configuration);  //1

  ReadWriteArray(mfp_reg);

  int dummy[4]; //was mfp_timer_precounter[4];
  ReadWriteArray(dummy);

  {
    // Make sure saving can't affect current emulation
    int save_mfp_timer_counter[4],save_mfp_timer_period[4];
    if (LoadOrSave==LS_SAVE){
      memcpy(save_mfp_timer_counter,mfp_timer_counter,sizeof(mfp_timer_counter));
      memcpy(save_mfp_timer_period,mfp_timer_period,sizeof(mfp_timer_period));
      for (int n=0;n<4;n++) mfp_calc_timer_counter(n);
    }
    ReadWriteArray(mfp_timer_counter);
    if (LoadOrSave==LS_SAVE){
      memcpy(mfp_timer_counter,save_mfp_timer_counter,sizeof(mfp_timer_counter));
      memcpy(mfp_timer_period,save_mfp_timer_period,sizeof(mfp_timer_period));
    }
  }

  ReadWrite(mfp_gpip_no_interrupt); //1

  ReadWrite(psg_reg_select);        //4
  ReadWriteArray(psg_reg);

  ReadWrite(dma_sound_control);     //1
  ReadWrite(dma_sound_start);       //4
  ReadWrite(dma_sound_end);         //4
  ReadWrite(dma_sound_mode);        //1

  ReadWriteStruct(ikbd);

  ReadWriteArray(keyboard_buffer);
  ReadWrite(keyboard_buffer_length); //4
  if (Version<8){
    keyboard_buffer[0]=0;
    keyboard_buffer_length=0;
  }

  ReadWrite(dma_mode);          //2
  ReadWrite(dma_status);        //1
  ReadWrite(dma_address);       //4
  ReadWrite(dma_sector_count);  //4
  ReadWrite(fdc_cr);            //1
  ReadWrite(fdc_tr);            //1
  ReadWrite(fdc_sr);            //1
  ReadWrite(fdc_str);           //1
  ReadWrite(fdc_dr);                     //1
  ReadWrite(fdc_last_step_inwards_flag); //1
  ReadWriteArray(floppy_head_track);
  ReadWriteArray(floppy_mediach);

#ifdef DISABLE_STEMDOS
  int stemdos_Pexec_list_ptr=0;
  MEM_ADDRESS stemdos_Pexec_list[76];
  ZeroMemory(stemdos_Pexec_list,sizeof(stemdos_Pexec_list));
  int stemdos_current_drive=0;
#endif
  ReadWrite(stemdos_Pexec_list_ptr);  //4
  ReadWriteArray(stemdos_Pexec_list);
  ReadWrite(stemdos_current_drive);   //4

  EasyStr NewROM=ROMFile;
  ReadWriteStr(NewROM);

  WORD NewROMVer=tos_version;
  if (Version>=7){
    ReadWrite(NewROMVer);
  }else{
    NewROMVer=0x701;
  }

  ReadWrite(bank_length[0]);
  ReadWrite(bank_length[1]);
  if (LoadOrSave==LS_LOAD){
    BYTE MemConf[2]={MEMCONF_512,MEMCONF_512};
    GetCurrentMemConf(MemConf);
    delete[] Mem;Mem=NULL;
    make_Mem(MemConf[0],MemConf[1]);
  }

  EasyStr NewDiskName[2],NewDisk[2];
  if (Version>=1){
    for (int disk=0;disk<2;disk++){
      NewDiskName[disk]=FloppyDrive[disk].DiskName;
      ReadWriteStr(NewDiskName[disk]);
      NewDisk[disk]=FloppyDrive[disk].GetDisk();
      ReadWriteStr(NewDisk[disk]);
    }
  }
  if (Version>=2){
#ifdef DISABLE_STEMDOS
    Str mount_gemdos_path[26];
#endif
    for (int n=0;n<26;n++) ReadWriteStr(mount_gemdos_path[n]);
#ifndef DISABLE_STEMDOS
    if (LoadOrSave==LS_LOAD) stemdos_check_paths();
#endif
  }
  if (Version>=4) ReadWriteStruct(Blit);

  if (Version>=5) ReadWriteArray(ST_Key_Down);

  if (Version>=8) ReadWriteStruct(ACIA_IKBD);

  if (Version>=9){
#ifdef DISABLE_STEMDOS
    MEM_ADDRESS stemdos_dta;
#endif
    ReadWrite(stemdos_dta); //4
  }

  if (Version>=10){
    if ((dma_sound_control & BIT_0) && LoadOrSave==LS_SAVE){ // DMA sound playing
      // Fix dma_sound_addr_to_read_next, can't do any harm
      dma_sound_write_to_buffer(ABSOLUTE_CPU_TIME);
    }
    ReadWrite(dma_sound_addr_to_read_next);    //4
    ReadWrite(next_dma_sound_end);   //4
    ReadWrite(next_dma_sound_start); //4
  }else if (LoadOrSave==LS_LOAD){
    next_dma_sound_end=dma_sound_end;
    next_dma_sound_start=dma_sound_start;
    dma_sound_addr_to_read_next=dma_sound_start;
  }
//  dma_sound_subdivide_minus_one=dma_mode_to_subdivide[sound_low_quality][dma_sound_mode & 3];
  dma_sound_freq=dma_sound_mode_to_freq[(dma_sound_mode & BIT_7)!=0][dma_sound_mode & 3];
  dma_sound_countdown=dma_sound_freq;

  DWORD StartOfData=0;
  NOT_ONEGAME( DWORD StartOfDataPos=ftell(f); )
  if (Version>=11) ReadWrite(StartOfData);

  if (Version>=12){
    ReadWrite(os_gemdos_vector);
    ReadWrite(os_bios_vector);
    ReadWrite(os_xbios_vector);
  }

  if (Version>=13) ReadWrite(paddles_ReadMask);

  EasyStr NewCart=CartFile;
  if (Version>=14) ReadWriteStr(NewCart);

  if (Version>=15){
    ReadWrite(rs232_recv_byte);
    ReadWrite(rs232_recv_overrun);
    ReadWrite(rs232_bits_per_word);
    ReadWrite(rs232_hbls_per_word);
  }

  EasyStr NewDiskInZip[2];
  if (Version>=20){
    for (int disk=0;disk<2;disk++){
      NewDiskInZip[disk]=FloppyDrive[disk].DiskInZip;
      ReadWriteStr(NewDiskInZip[disk]);
    }
  }

#ifndef ONEGAME
  bool ChangeTOS=true,ChangeCart=ChangeDisksAndCart,ChangeDisks=ChangeDisksAndCart;
#endif
  DWORD ExtraFlags=0;

  if (Version>=21) ReadWrite(ExtraFlags);

#ifndef ONEGAME
  if (ExtraFlags & BIT_0) ChangeDisks=0;
  // Flag here for saving disks in this file? (huge!)
  if (ExtraFlags & BIT_1) ChangeTOS=0;
  // Flag here for only asking user to locate version and country code TOS?
  // Flag here for saving TOS in this file?
  if (ExtraFlags & BIT_2) ChangeCart=0;
  // Flag here for saving the cart in this file?
#endif

  if (Version>=22){
#ifdef DISABLE_STEMDOS
    #define MAX_STEMDOS_FSNEXT_STRUCTS 100
    struct _STEMDOS_FSNEXT_STRUCT{
      MEM_ADDRESS dta;
      EasyStr path;
      EasyStr NextFile;
      int attr;
      DWORD start_hbl;
    }stemdos_fsnext_struct[MAX_STEMDOS_FSNEXT_STRUCTS];
#endif
    int max_fsnexts=MAX_STEMDOS_FSNEXT_STRUCTS;
    ReadWrite(max_fsnexts);
    for (int n=0;n<max_fsnexts;n++){
      ReadWrite(stemdos_fsnext_struct[n].dta);
      // If this is invalid then it will just return "no more files"
      ReadWriteStr(stemdos_fsnext_struct[n].path);
      ReadWriteStr(stemdos_fsnext_struct[n].NextFile);
      ReadWrite(stemdos_fsnext_struct[n].attr);
      ReadWrite(stemdos_fsnext_struct[n].start_hbl);
    }
  }

  if (Version>=23) ReadWrite(shifter_hscroll_extra_fetch);

#ifdef NO_CRAZY_MONITOR
  int em_width=480,em_height=480,em_planes=4,extended_monitor=0,aes_calls_since_reset=0;
  long save_r[16];
  MEM_ADDRESS line_a_base=0,vdi_intout=0;
#endif
  bool old_em=bool(extended_monitor);
  if (Version>=24){
    ReadWrite(em_width);
    ReadWrite(em_height);
    ReadWrite(em_planes);
    ReadWrite(extended_monitor);
    ReadWrite(aes_calls_since_reset);
    ReadWriteArray(save_r);
    ReadWrite(line_a_base);
    ReadWrite(vdi_intout);
  }else if (LoadOrSave==LS_LOAD){
    extended_monitor=0;
  }

  if (Version>=25){
    int save_mfp_timer_counter[4],save_mfp_timer_period[4];
    if (LoadOrSave==LS_SAVE){
      memcpy(save_mfp_timer_counter,mfp_timer_counter,sizeof(mfp_timer_counter));
      memcpy(save_mfp_timer_period,mfp_timer_period,sizeof(mfp_timer_period));
    }
    for (int n=0;n<4;n++){
      BYTE prescale_ticks=0;
      if (LoadOrSave==LS_SAVE) prescale_ticks=(BYTE)mfp_calc_timer_counter(n);
      ReadWrite(prescale_ticks);
      if (LoadOrSave==LS_LOAD){
        int ticks_per_count=mfp_timer_prescale[mfp_get_timer_control_register(n) & 7];
        mfp_timer_counter[n]-=int(double(64.0/ticks_per_count)*double(prescale_ticks));
      }
    }
    if (LoadOrSave==LS_SAVE){
      memcpy(mfp_timer_counter,save_mfp_timer_counter,sizeof(mfp_timer_counter));
      memcpy(mfp_timer_period,save_mfp_timer_period,sizeof(mfp_timer_period));
    }
  }
  if (Version>=26){
    ReadWriteArray(mfp_timer_period);
  }else if (LoadOrSave==LS_LOAD){
    for (int timer=0;timer<4;timer++) MFP_CALC_TIMER_PERIOD(timer);
  }
  if (Version>=27){
    ReadWriteArray(mfp_timer_period_change);
  }else if (LoadOrSave==LS_LOAD){
    for (int timer=0;timer<4;timer++) mfp_timer_period_change[timer]=0;
  }
  if (Version>=28){
    int rel_time=ABSOLUTE_CPU_TIME-dma_sound_start_time;
    ReadWrite(rel_time);
    if (LoadOrSave==LS_LOAD) dma_sound_start_time=rel_time;
  }else if (LoadOrSave==LS_LOAD){
    dma_sound_start_time=0;
  }

  if (Version>=29){
    ReadWrite(emudetect_called);
    if (LoadOrSave==LS_LOAD) emudetect_init();
  }

  if (Version>=30){
    ReadWrite(MicroWire_Mask);
    ReadWrite(MicroWire_Data);
    ReadWrite(dma_sound_volume);
    ReadWrite(dma_sound_l_volume);
    ReadWrite(dma_sound_r_volume);
    ReadWrite(dma_sound_l_top_val);
    ReadWrite(dma_sound_r_top_val);
    ReadWrite(dma_sound_mixer);
  }

  int NumFloppyDrives=num_connected_floppies;
  if (Version>=31){
    ReadWrite(NumFloppyDrives);
  }else{
    NumFloppyDrives=2;
  }

  bool spin_up=bool(fdc_spinning_up);
  if (Version>=32) ReadWrite(spin_up);
  if (Version>=33){
    ReadWrite(fdc_spinning_up);
  }else if (LoadOrSave==LS_LOAD){
    fdc_spinning_up=spin_up;
  }

  if (Version>=34) ReadWrite(emudetect_write_logs_to_printer);

  if (Version>=35){
    ReadWrite(psg_reg_data);
    ReadWrite(floppy_type1_command_active);
    ReadWrite(fdc_read_address_buffer_len);
    ReadWriteArray(fdc_read_address_buffer);
    ReadWrite(dma_bytes_written_for_sector_count);
  }

  if (Version>=36){
    struct _AGENDA_STRUCT temp_agenda[MAX_AGENDA_LENGTH];
    int temp_agenda_length=agenda_length;
    for (int i=0;i<agenda_length;i++) temp_agenda[i]=agenda[i];

    if (LoadOrSave==LS_SAVE){
      // Convert vectors to indexes and hbl_counts to relative
      for (int i=0;i<temp_agenda_length;i++){
        int l=0;
        while (DWORD(agenda_list[l])!=1){
          if (temp_agenda[i].perform==agenda_list[l]){
            temp_agenda[i].perform=(LPAGENDAPROC)l;
            break;
          }
          l++;
        }
        if (DWORD(agenda_list[l])==1) temp_agenda[i].perform=(LPAGENDAPROC)-1;
        temp_agenda[i].time-=hbl_count;
      }
    }
    ReadWrite(temp_agenda_length);
    for (int i=0;i<temp_agenda_length;i++){
      ReadWriteStruct(temp_agenda[i]);
    }
    if (LoadOrSave==LS_LOAD){
      int list_len=0;
      while (DWORD(agenda_list[++list_len])!=1);

      for (int i=0;i<temp_agenda_length;i++){
        int idx=int(temp_agenda[i].perform);
        if (idx>=list_len || idx<0){
          temp_agenda[i].perform=NULL;
        }else{
          temp_agenda[i].perform=agenda_list[idx];
        }
      }
      agenda_length=temp_agenda_length;
      for (int i=0;i<agenda_length;i++) agenda[i]=temp_agenda[i];
      agenda_next_time=0xffffffff;
      if (agenda_length) agenda_next_time=agenda[agenda_length-1].time;
    }
  }

  if (Version>=37){
    ReadWrite(stemdos_intercept_datetime);
  }

  if (Version>=38){
    ReadWrite(emudetect_falcon_mode);
    ReadWrite(emudetect_falcon_mode_size);

    DWORD l=256;
    if (emudetect_called==0) l=0;
    ReadWrite(l);
    for (DWORD n=0;n<l;n++){
      ReadWrite(emudetect_falcon_stpal[n]);
    }
  }

  // End of data, seek to compressed memory
  if (Version>=11){
#ifndef ONEGAME
    if (LoadOrSave==LS_SAVE){
      StartOfData=ftell(f);
      fseek(f,StartOfDataPos,SEEK_SET);
      ReadWrite(StartOfData);
    }
    // Seek to start of compressed data (this was loaded earlier if LS_LOAD)
    fseek(f,StartOfData,SEEK_SET);
#else
    f=pStartByte+StartOfData;
#endif
  }

  if (LoadOrSave==LS_SAVE) return 0;

  init_screen();

#ifndef NO_CRAZY_MONITOR
  if (bool(extended_monitor)!=old_em || extended_monitor){
    if (FullScreen){
      change_fullscreen_display_mode(true);
    }else{
      Disp.ScreenChange(); // For extended monitor
    }
  }
#endif

#ifndef ONEGAME
  if (ChangeTOS){
    int ret=LoadSnapShotChangeTOS(NewROM,NewROMVer);
    if (ret>0) return ret;
  }

  if (ChangeDisks){
    int ret=LoadSnapShotChangeDisks(NewDisk,NewDiskInZip,NewDiskName);
    if (ret>0) return ret;
  }

  if (ChangeCart){
    int ret=LoadSnapShotChangeCart(NewCart);
    if (ret>0) return ret;
  }

  LoadSaveChangeNumFloppies(NumFloppyDrives);
#endif

  return 0;
}
#undef ReadWrite
#undef ReadWritePtr
#undef ReadWriteStruct
//---------------------------------------------------------------------------
void LoadSnapShotUpdateVars(int Version)
{
  SET_PC(PC32);
  ioaccess=0;

  init_timings();

  UpdateSTKeys();

  if (Version<36){
    // No agendas saved
    if (ikbd.resetting) ikbd_reset(0);
    if (ikbd.mouse_mode==IKBD_MOUSE_MODE_OFF) ikbd.port_0_joy=true;

    if (keyboard_buffer_length) agenda_add(agenda_keyboard_replace,ACIAClockToHBLS(ACIA_IKBD.clock_divide)+1,0);
    if (MIDIPort.AreBytesToCome()) agenda_add(agenda_midi_replace,ACIAClockToHBLS(ACIA_MIDI.clock_divide,true)+1,0);
    if (floppy_irq_flag==FLOPPY_IRQ_YES || floppy_irq_flag==FLOPPY_IRQ_ONESEC){
      agenda_add(agenda_fdc_finished,MILLISECONDS_TO_HBLS(2),0);
    }
    if (fdc_spinning_up) agenda_add(agenda_fdc_spun_up,MILLISECONDS_TO_HBLS(40),fdc_spinning_up==2);
    if (ACIA_MIDI.tx_flag) agenda_add(agenda_acia_tx_delay_MIDI,2,0);
    if (ACIA_IKBD.tx_flag) agenda_add(agenda_acia_tx_delay_IKBD,2,0);
  }

  // Change dma_sound_start_time from relative to absolute
  dma_sound_start_time=ABSOLUTE_CPU_TIME-dma_sound_start_time;
  dma_sound_prepare_for_end(dma_sound_start,dma_sound_start_time,dma_sound_control & BIT_0);
  prepare_event_again();

  snapshot_loaded=true;

  res_change();

  palette_convert_all();
  draw(false);

  for (int n=0;n<16;n++) PAL_DPEEK(n*2)=STpal[n];
}
//---------------------------------------------------------------------------


