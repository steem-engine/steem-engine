#define DMA_ADDRESS_IS_VALID_R (dma_address<himem)
#define DMA_ADDRESS_IS_VALID_W (dma_address<himem && dma_address>=MEM_FIRST_WRITEABLE)

#define DMA_INC_ADDRESS                                    \
  if (dma_sector_count){                                   \
    dma_address++;                                         \
    dma_bytes_written_for_sector_count++;                  \
    if (dma_bytes_written_for_sector_count>=512){        \
      dma_bytes_written_for_sector_count=0;              \
      dma_sector_count--;                                  \
      dma_status|=BIT_1;  /* DMA sector count not 0 */   \
      if (dma_sector_count==0) dma_status&=~BIT_1;     \
    }                                                      \
  }                                                        \

// 5 revolutions per second, 313*50 HBLs per second
#define FDC_HBLS_PER_ROTATION (313*50/5)
// 5% of track is index pulse, too high?
#define FDC_HBLS_OF_INDEX_PULSE (FDC_HBLS_PER_ROTATION/20)

#define LOGSECTION LOGSECTION_FDC
//---------------------------------------------------------------------------
int floppy_current_drive()
{
  if ((psg_reg[PSGR_PORT_A] & BIT_1)==0){ // Drive A
    return 0;
  }else if ((psg_reg[PSGR_PORT_A] & BIT_2)==0){ // Drive B
    return 1;
  }
  return 0; // Neither, guess A
}
//---------------------------------------------------------------------------
BYTE floppy_current_side()
{
  return (psg_reg[PSGR_PORT_A] & BIT_0)==0;
}
//---------------------------------------------------------------------------
BYTE read_from_dma()
{
  if (DMA_ADDRESS_IS_VALID_R) return PEEK(dma_address);
  return 0xff;
}
//---------------------------------------------------------------------------
void write_to_dma(int Val,int Num=1)
{
  int n=Num;
  if (Num<=0) n=1;
  for (int i=0;i<n;i++){
    if (dma_sector_count==0) break;

    if (DMA_ADDRESS_IS_VALID_W) PEEK(dma_address)=BYTE(Val);
    if (Num<=0) break;
    DMA_INC_ADDRESS;
  }
}
//---------------------------------------------------------------------------
bool floppy_handle_file_error(int floppyno,bool Write,int sector,int PosInSector,bool FromFormat)
{
  static DWORD last_reinsert_time[2]={0,0};
  TFloppyImage *floppy=&FloppyDrive[floppyno];

  log_write(EasyStr("File error - re-inserting disk ")+LPSTR(floppyno ? "B":"A"));

  bool WorkingNow=0;
  if (timer>=last_reinsert_time[floppyno]+2000 && floppy->DiskInDrive()){
    // Over 2 seconds since last failure
    FILE *Dest=NULL;
    if (FromFormat){
      if (floppy->ReopenFormatFile()) Dest=floppy->Format_f;
    }else{
      if (floppy->ReinsertDisk()) Dest=floppy->f;
    }
    if (Dest){
      if (floppy->SeekSector(floppy_current_side(),floppy_head_track[floppyno],sector,FromFormat)==0){
        fseek(Dest,PosInSector,SEEK_CUR);
        BYTE temp=read_from_dma();
        if (Write){
          WorkingNow=fwrite(&temp,1,1,Dest);
        }else{
          WorkingNow=fread(&temp,1,1,Dest);
        }
        if (DMA_ADDRESS_IS_VALID_W && dma_sector_count) write_to_dma(temp,0);
      }
    }else{
      GUIDiskErrorEject(floppyno);
    }
  }
  last_reinsert_time[floppyno]=timer;

  return NOT WorkingNow;
}
//---------------------------------------------------------------------------
bool floppy_track_index_pulse_active()
{
  if (floppy_type1_command_active==1){
    return (hbl_count % FDC_HBLS_PER_ROTATION)>=(FDC_HBLS_PER_ROTATION-FDC_HBLS_OF_INDEX_PULSE);
  }
  return 0;
}
//---------------------------------------------------------------------------
void fdc_type1_check_verify()
{
  if (FDC_VERIFY==0) return;

  // This reads an ID field and checks that track number matches floppy_head_track
  // It will fail on an unformatted track or if there is no disk of course
  int floppyno=floppy_current_drive();
  TFloppyImage *floppy=&FloppyDrive[floppyno];
  if (floppy_head_track[floppyno]>FLOPPY_MAX_TRACK_NUM || floppy->Empty()){
    fdc_str|=FDC_STR_SEEK_ERROR;
  }else if (floppy->TrackIsFormatted[floppy_current_side()][floppy_head_track[floppyno]]==0){
    // If track is formatted then it is okay to seek to it, otherwise do this:
    if (floppy_head_track[floppyno]>=floppy->TracksPerSide) fdc_str|=FDC_STR_SEEK_ERROR;
    if (floppy_current_side() >= floppy->Sides) fdc_str|=FDC_STR_SEEK_ERROR;
  }
DEBUG_ONLY( if (fdc_str & FDC_STR_SEEK_ERROR) log("     Verify failed (track not formatted)"); )
}
//---------------------------------------------------------------------------
void floppy_fdc_command(BYTE cm)
{
  log(Str("FDC: ")+HEXSl(old_pc,6)+" - executing command $"+HEXSl(cm,2));
  if (fdc_str & FDC_STR_BUSY){
    if ((cm & (BIT_7+BIT_6+BIT_5+BIT_4))!=0xd0){ // Not force interrupt
      log("     Command ignored, FDC is busy!");
      return;
    }
  }

  mfp_gpip_set_bit(MFP_GPIP_FDC_BIT,true); // Turn off IRQ output
  agenda_delete(agenda_fdc_finished);

  floppy_irq_flag=0;
  if (floppy_current_drive()==1 && num_connected_floppies<2){ // Drive B disconnected?
    return;
  }

  disk_light_off_time=timer+DisableDiskLightAfter;
  fdc_cr=cm;
  agenda_delete(agenda_fdc_motor_flag_off);

  // AFAIK the FDC turns the motor on automatically whenever it receives a command.
  // Normally the FDC delays execution until the motor has reached top speed but
  // there is a bit in type 1, 2 and 3 commands that will make them execute
  // while the motor is in the middle of spinning up (BIT_3).
  bool delay_exec=0;
  if ((fdc_str & FDC_STR_MOTOR_ON)==0){
    if ((cm & (BIT_7+BIT_6+BIT_5+BIT_4))!=0xd0){ // Not force interrupt
      if ((cm & BIT_3)==0){
        delay_exec=true; // Delay command until after spinup
      }
    }
    fdc_str=FDC_STR_BUSY | FDC_STR_MOTOR_ON;
    fdc_spinning_up=int(delay_exec ? 2:1);
    if (floppy_instant_sector_access){
      agenda_add(agenda_fdc_spun_up,MILLISECONDS_TO_HBLS(100),delay_exec);
    }else{
      // 6 revolutions but guaranteed 1 second spinup at 5 RPS, how?
      agenda_add(agenda_fdc_spun_up,MILLISECONDS_TO_HBLS(1100),delay_exec);
    }
  }
  if (delay_exec==0) fdc_execute();
}
//---------------------------------------------------------------------------
void agenda_fdc_spun_up(int do_exec)
{
  fdc_spinning_up=0;
  if (do_exec) fdc_execute();
}
//---------------------------------------------------------------------------
void fdc_execute()
{
  // We need to do something here to make the command take more time
  // if the disk spinning up (fdc_spinning_up).

  int floppyno=floppy_current_drive();
  TFloppyImage *floppy=&FloppyDrive[floppyno];

  floppy_irq_flag=FLOPPY_IRQ_YES;
  double hbls_to_interrupt=64.0;

  // The FDC timings don't change when you switch to mono, but HBLs do.
  // This variable corrects for that.
  double hbl_multiply=1.0;
  if (shifter_freq==MONO_HZ){
    hbl_multiply=double(HBLS_PER_SECOND_MONO)/double(HBLS_PER_SECOND_AVE);
  }

  if ((fdc_cr & BIT_7)==0){
    // Type 1 commands
    hbls_to_interrupt=fdc_step_time_to_hbls[fdc_cr & (BIT_0 | BIT_1)];

    switch (fdc_cr & (BIT_7+BIT_6+BIT_5+BIT_4)){
      case 0x00: //restore to track 0
        if (FDC_VERIFY && floppy->Empty()){ //no disk
          fdc_str=FDC_STR_SEEK_ERROR | FDC_STR_MOTOR_ON | FDC_STR_BUSY;
        }else{
          if (floppy_head_track[floppyno]==0){
            hbls_to_interrupt=2;
          }else{
            if (floppy_instant_sector_access==0) hbls_to_interrupt*=floppy_head_track[floppyno];
            floppy_head_track[floppyno]=0;
          }
          fdc_tr=0;
          fdc_str=FDC_STR_MOTOR_ON | FDC_STR_BUSY;
          floppy_type1_command_active=1;
          log(Str("FDC: Restored drive ")+char('A'+floppyno)+" to track 0");
        }
        break;
      case 0x10: //seek to track number in data register
        agenda_add(agenda_floppy_seek,2,0);
        fdc_str=FDC_STR_MOTOR_ON | FDC_STR_BUSY;
        floppy_irq_flag=0;
        log(Str("FDC: Seeking drive ")+char('A'+floppyno)+" to track "+fdc_dr+" hbl_count="+hbl_count);
        floppy_type1_command_active=1;
        break;
      default: //step, step in, step out
      {
        fdc_str=FDC_STR_MOTOR_ON | FDC_STR_BUSY;
        char d=1; //step direction, default is inwards
        if (floppy->Empty()){
          if (FDC_VERIFY) fdc_str|=FDC_STR_SEEK_ERROR;
        }else{
          switch (fdc_cr & (BIT_7+BIT_6+BIT_5)){
            case 0x20: if (fdc_last_step_inwards_flag==0) d=-1; break;
            case 0x60: d=-1; break;
          }
          fdc_last_step_inwards_flag = (d==1);
          if (fdc_cr & BIT_4){ //U flag, update track register
            fdc_tr+=d;
          }
          if (d==-1 && floppy_head_track[floppyno]==0){   //trying to step out from track 0
            fdc_tr=0; //here we set the track register
          }else{ //can step
            floppy_head_track[floppyno]+=d;
            log(Str("FDC: Stepped drive ")+char('A'+floppyno)+" to track "+floppy_head_track[floppyno]);
            fdc_type1_check_verify();
          }
          floppy_type1_command_active=1;
        }
      }
    }
  }else{
    floppy_type1_command_active=0;
    fdc_str&=BYTE(~FDC_STR_WRITE_PROTECT);
    LOG_ONLY( int n_sectors=dma_sector_count; )
    switch (fdc_cr & (BIT_7+BIT_6+BIT_5+BIT_4)){

      // Type 2
      case 0x80:case 0xa0:LOG_ONLY( n_sectors=1; ) // Read/write single sector
      case 0x90:case 0xb0:                         // Read/write multiple sectors
        if (floppy->Empty() || floppy_head_track[floppyno]>FLOPPY_MAX_TRACK_NUM){
          fdc_str=FDC_STR_MOTOR_ON | FDC_STR_SEEK_ERROR | FDC_STR_BUSY;
          floppy_irq_flag=FLOPPY_IRQ_ONESEC;  //end command after 1 second
          break;
        }
#ifdef ENABLE_LOGFILE
        {
          Str RW="Reading",Secs="one sector";
          if (fdc_cr & 0x20) RW="Writing";
          if (n_sectors>1) Secs="multiple sectors";
          log(Str("FDC: ")+RW+" "+Secs+" from drive "+char('A'+floppyno)+
                 " track "+floppy_head_track[floppyno]+
                 " side "+floppy_current_side()+
                 " sector "+fdc_sr+
                 " into address "+HEXSl(dma_address,6)+
                 " dma_sector_count="+dma_sector_count);
        }
#endif
        if (floppy_instant_sector_access){
          agenda_add(agenda_floppy_readwrite_sector,int(hbls_to_interrupt*hbl_multiply),MAKELONG(0,fdc_cr));
          fdc_str=FDC_STR_MOTOR_ON | FDC_STR_BUSY;
          floppy_irq_flag=0;
        }else{
          FDC_IDField IDList[30];
          int SectorIdx=-1;
          int nSects=floppy->GetIDFields(floppy_current_side(),floppy_head_track[floppyno],IDList);
          for (int n=0;n<nSects;n++){
            if (IDList[n].Track==fdc_tr && IDList[n].Side==floppy_current_side()){
              if (IDList[n].SectorNum==fdc_sr){
                SectorIdx=n;
                break;
              }
            }
          }
          if (SectorIdx>-1){
            // Break up the readable track into nSects sections,
            // agenda_floppy_readwrite_sector occurs at the start of one of these sections
            DWORD HBLsPerTrack=FDC_HBLS_PER_ROTATION-FDC_HBLS_OF_INDEX_PULSE;
            DWORD HBLsPerSector=HBLsPerTrack/nSects;
            DWORD HBLsAtStartOfRotation=(hbl_count/FDC_HBLS_PER_ROTATION)*FDC_HBLS_PER_ROTATION;

            DWORD HBLOfSectorStart=HBLsAtStartOfRotation + SectorIdx*HBLsPerSector;

            if (HBLOfSectorStart<hbl_count) HBLOfSectorStart+=FDC_HBLS_PER_ROTATION;

            agenda_delete(agenda_floppy_readwrite_sector);
            agenda_add(agenda_floppy_readwrite_sector,int(hbl_multiply*(HBLOfSectorStart-hbl_count))+2,MAKELONG(0,fdc_cr));
            floppy_irq_flag=0;
            fdc_str=FDC_STR_MOTOR_ON | FDC_STR_BUSY;
          }else{
            floppy_irq_flag=FLOPPY_IRQ_ONESEC;
            fdc_str=FDC_STR_MOTOR_ON | FDC_STR_SEEK_ERROR | FDC_STR_BUSY;  //sector not found
          }
        }
        break;

      // Type 3
      case 0xc0: //read address
        log(Str("FDC: Type III Command - read address to ")+HEXSl(dma_address,6)+"from drive "+char('A'+floppyno));

        if (floppy->Empty()){
          floppy_irq_flag=0;  //never cause interrupt, timeout
        }else{
          FDC_IDField IDList[30];
          DWORD nSects=floppy->GetIDFields(floppy_current_side(),floppy_head_track[floppyno],IDList);
          if (nSects){
            DWORD DiskPosition=hbl_count % FDC_HBLS_PER_ROTATION;
            // Break up the track into nSects sections, agenda_read_address
            // occurs at the end of one of these sections
            DWORD HBLsPerTrack=FDC_HBLS_PER_ROTATION-FDC_HBLS_OF_INDEX_PULSE;
            DWORD HBLsPerSector=HBLsPerTrack/nSects;

            DWORD HBLsToNextSector=HBLsPerSector - (DiskPosition % HBLsPerSector);
            DWORD NextIDNum=DiskPosition / HBLsPerSector;

            if (NextIDNum>=nSects){ // Track index pulse
              NextIDNum=0;
              // Go to next revolution
              HBLsToNextSector=(FDC_HBLS_PER_ROTATION-DiskPosition) + HBLsPerSector;
            }
            agenda_delete(agenda_floppy_read_address);
            agenda_add(agenda_floppy_read_address,int(hbl_multiply*HBLsToNextSector)+2,NextIDNum);
            floppy_irq_flag=0;
            fdc_str=FDC_STR_MOTOR_ON | FDC_STR_BUSY;
          }else{
            fdc_str=FDC_STR_MOTOR_ON | FDC_STR_SEEK_ERROR | FDC_STR_BUSY;  //sector not found
            floppy_irq_flag=0;  //never cause interrupt, timeout
          }
        }
        break;
      case 0xe0:  //read track
        log(Str("FDC: Type III Command - read track to ")+HEXSl(dma_address,6)+" from drive "+char('A'+floppyno)+
                  " dma_sector_count="+dma_sector_count);

        fdc_str=FDC_STR_MOTOR_ON | FDC_STR_BUSY;
        floppy_irq_flag=0;
        if (floppy->DiskInDrive()){
          agenda_delete(agenda_floppy_read_track);
          DWORD DiskPosition=hbl_count % FDC_HBLS_PER_ROTATION;
          agenda_add(agenda_floppy_read_track,int(hbl_multiply*(FDC_HBLS_PER_ROTATION-DiskPosition)),0);
        }
        break;
      case 0xf0:  //write (format) track
        log(Str("FDC: - Type III Command - write track from address ")+HEXSl(dma_address,6)+" to drive "+char('A'+floppyno));

        floppy_irq_flag=0;
        fdc_str=FDC_STR_MOTOR_ON | FDC_STR_BUSY;
        if (floppy->DiskInDrive() && floppy->ReadOnly==0 && floppy_head_track[floppyno]<=FLOPPY_MAX_TRACK_NUM){
          if (floppy->Format_f==NULL) floppy->OpenFormatFile();
          if (floppy->Format_f){
            agenda_delete(agenda_floppy_write_track);
            DWORD DiskPosition=hbl_count % FDC_HBLS_PER_ROTATION;
            agenda_add(agenda_floppy_write_track,int(hbl_multiply*(FDC_HBLS_PER_ROTATION-DiskPosition)),0);
            floppy_write_track_bytes_done=0;
          }
        }
        break;
      case 0xd0:        //force interrupt
      {
        log(Str("FDC: ")+HEXSl(old_pc,6)+" - Force interrupt: t="+hbl_count);

        bool type23_active=(agenda_get_queue_pos(agenda_floppy_readwrite_sector)>=0 ||
                            agenda_get_queue_pos(agenda_floppy_read_address)>=0 ||
                            agenda_get_queue_pos(agenda_floppy_read_track)>=0 ||
                            agenda_get_queue_pos(agenda_floppy_write_track)>=0);
        agenda_delete(agenda_floppy_seek);
        agenda_delete(agenda_floppy_readwrite_sector);
        agenda_delete(agenda_floppy_read_address);
        agenda_delete(agenda_floppy_read_track);
        agenda_delete(agenda_floppy_write_track);
        agenda_delete(agenda_fdc_finished);

        fdc_str=BYTE(fdc_str & FDC_STR_MOTOR_ON);
        if (fdc_cr & b1100){
          agenda_fdc_finished(0); // Interrupt CPU immediately
        }else{
          floppy_irq_flag=0;
          mfp_gpip_set_bit(MFP_GPIP_FDC_BIT,true); // Turn off IRQ output
        }
        /* From Jorge Cwik
          By the way, the status after a Type IV command depends on the previous
          state of the fdc. If there was a command active, the status is the type of
          the previous command. If there was no command, then the status is of
          Type I. So resetting the FDC twice in a row guarantee a Type I status
          (first time terminates a command, if one is active; second time activates
          a type I status because no command was active).
        */
        if (type23_active){
          floppy_type1_command_active=0;
        }else{
          floppy_type1_command_active=2;
        }
        break;
      }
    }
  }
  if (fdc_str & FDC_STR_MOTOR_ON) agenda_add(agenda_fdc_motor_flag_off,MILLISECONDS_TO_HBLS(1800),0);
  if (floppy_irq_flag){
    if (floppy_irq_flag!=FLOPPY_IRQ_NOW){ // Don't need to add agenda if it has happened
      if (floppy_irq_flag==FLOPPY_IRQ_ONESEC){
        agenda_add(agenda_fdc_finished,MILLISECONDS_TO_HBLS(1000),0);
      }else{
        agenda_add(agenda_fdc_finished,int(hbls_to_interrupt*hbl_multiply),0);
      }
    }
  }
}
//---------------------------------------------------------------------------
void agenda_fdc_motor_flag_off(int)
{
  fdc_str&=BYTE(~FDC_STR_MOTOR_ON);
}
//---------------------------------------------------------------------------
void agenda_fdc_finished(int)
{
  log("FDC: Finished command, GPIP bit low.");
  floppy_irq_flag=FLOPPY_IRQ_NOW;
  mfp_gpip_set_bit(MFP_GPIP_FDC_BIT,0); // Sets bit in GPIP low (and it stays low)

  fdc_str&=BYTE(~FDC_STR_BUSY); // Turn off busy bit
  fdc_str&=BYTE(~FDC_STR_T1_TRACK_0); // This is lost data bit for non-type 1 commands
  if (floppy_type1_command_active){
    if (floppy_head_track[floppy_current_drive()]==0) fdc_str|=FDC_STR_T1_TRACK_0;
    floppy_type1_command_active=2;
  }
}
//---------------------------------------------------------------------------
void agenda_floppy_seek(int)
{
  int floppyno=floppy_current_drive();

  if (floppy_head_track[floppyno]==fdc_dr){
    log(Str("FDC: Finished seeking to track ")+fdc_dr+" hbl_count="+hbl_count);
    fdc_tr=fdc_dr;
    fdc_type1_check_verify();
    agenda_fdc_finished(0);
    return;
  }
  if (floppy_head_track[floppyno]>fdc_dr){
    floppy_head_track[floppyno]--;
  }else if (floppy_head_track[floppyno]<fdc_dr){
    floppy_head_track[floppyno]++;
  }
  int hbls_to_interrupt=fdc_step_time_to_hbls[fdc_cr & (BIT_0 | BIT_1)];
  if (floppy_instant_sector_access) hbls_to_interrupt>>=5;
  double hbl_multiply=1.0;
  if (shifter_freq==MONO_HZ) hbl_multiply=double(HBLS_PER_SECOND_MONO)/double(HBLS_PER_SECOND_AVE);
  agenda_add(agenda_floppy_seek,int(hbl_multiply*double(hbls_to_interrupt)),0);
}
//---------------------------------------------------------------------------
void agenda_floppy_readwrite_sector(int Data)
{
  int floppyno=floppy_current_drive();
  TFloppyImage *floppy=&FloppyDrive[floppyno];
  bool FromFormat=0;
  if (floppy_head_track[floppyno]<=FLOPPY_MAX_TRACK_NUM){
    FromFormat=floppy->TrackIsFormatted[floppy_current_side()][floppy_head_track[floppyno]];
  }

  int Command=HIWORD(Data);

  BYTE WriteProtect=0;
  if ((Command & 0x20) && floppy->ReadOnly){ // Write
    WriteProtect=FDC_STR_WRITE_PROTECT;
  }

  disk_light_off_time=timer+DisableDiskLightAfter;
  fdc_str=BYTE(FDC_STR_BUSY | FDC_STR_MOTOR_ON | WriteProtect);
  floppy_irq_flag=0;
  if (floppy_access_ff) floppy_access_ff_counter=FLOPPY_FF_VBL_COUNT;
  agenda_delete(agenda_fdc_motor_flag_off);
  agenda_add(agenda_fdc_motor_flag_off,MILLISECONDS_TO_HBLS(1800),0);

instant_sector_access_loop:

  int Part=LOWORD(Data);
  int SectorStage=(Part % 71); // 0=seek, 1-64=read/write, 65=end of sector, 66-70=gap

  if (SectorStage==0){
    if (floppy->SeekSector(floppy_current_side(),floppy_head_track[floppyno],fdc_sr,FromFormat)){
      // Error seeking sector, it doesn't exist
      floppy_irq_flag=FLOPPY_IRQ_ONESEC;  //end command after 1 second
    }
#ifdef ONEGAME
     else{
      if (Command & 0x20){ // Write
        OGWriteSector(floppy_current_side(),floppy_head_track[floppyno],fdc_sr,floppy->BytesPerSector);
      }
    }
#endif
  }else if (SectorStage<=64){
    FILE *f=(FILE*)(FromFormat ? floppy->Format_f:floppy->f);
    int BytesPerStage=16;
    int PosInSector=(SectorStage-1)*BytesPerStage;

    BYTE Temp;
    if (Command & 0x20){ // Write
      if (floppy->ReadOnly){
        floppy_irq_flag=FLOPPY_IRQ_NOW; //interrupt with write-protect flag
        FDCCantWriteDisplayTimer=timer+3000;
      }else{
        floppy->WrittenTo=true;
        if (floppy->IsZip()) FDCCantWriteDisplayTimer=timer+5000; // Writing will be lost!
        for (int bb=BytesPerStage;bb>0;bb--){
          Temp=read_from_dma();
          if (fwrite(&Temp,1,1,f)==0){
            if (floppy_handle_file_error(floppyno,true,fdc_sr,PosInSector,FromFormat)){
              floppy_irq_flag=FLOPPY_IRQ_ONESEC;  //end command after 1 second
              break;
            }
          }
          dma_address++;
          PosInSector++;
        }
      }
    }else{
      BYTE *lpDest;
      for (int bb=BytesPerStage;bb>0;bb--){
        if (DMA_ADDRESS_IS_VALID_W && dma_sector_count){
          lpDest=lpPEEK(dma_address);
        }else{
          lpDest=&Temp;
        }
        if (fread(lpDest,1,1,f)==0){
          if (floppy_handle_file_error(floppyno,0,fdc_sr,PosInSector,FromFormat)){
            floppy_irq_flag=FLOPPY_IRQ_ONESEC;  //end command after 1 second
            break;
          }
        }
        DMA_INC_ADDRESS;
        PosInSector++;
      }
    }
    if (PosInSector>=int(FromFormat ? floppy->FormatLargestSector:floppy->BytesPerSector)){
      Part=64; // Done sector, last part
    }
  }else if (SectorStage==65){
    log(Str("FDC: Finished reading/writing sector ")+fdc_sr+" of track "+floppy_head_track[floppyno]+" of side "+floppy_current_side());
    floppy_irq_flag=FLOPPY_IRQ_NOW;
    if (Command & BIT_4){ // Multiple sectors
      fdc_sr++;
      floppy_irq_flag=0;
    }
  }

  Part++;

  switch (floppy_irq_flag){
    case FLOPPY_IRQ_NOW:
      log("FDC: Finished read/write sector");
      fdc_str=BYTE(WriteProtect | FDC_STR_MOTOR_ON);
      agenda_fdc_finished(0);
      return; // Don't loop
    case FLOPPY_IRQ_ONESEC:
      // sector not found
      fdc_str=BYTE(WriteProtect | FDC_STR_MOTOR_ON | /*FDC_STR_SEEK_ERROR | */FDC_STR_BUSY);
      agenda_add(agenda_fdc_finished,FDC_HBLS_PER_ROTATION*int((shifter_freq==MONO_HZ) ? 11:5),0);
      return; // Don't loop
  }

  if (floppy_instant_sector_access){
    Data=MAKELONG(Part,Command);
    goto instant_sector_access_loop;
  }else{
    // 8000 bytes per revolution * 5 revolutions per second
    int bytes_per_second=8000*5;
    int hbls_per_second=HBLS_PER_SECOND_AVE; // 60hz and 50hz are roughly the same
    if (shifter_freq==MONO_HZ) hbls_per_second=int(HBLS_PER_SECOND_MONO);
    int n_hbls=hbls_per_second/(bytes_per_second/16);
    agenda_add(agenda_floppy_readwrite_sector,n_hbls,MAKELONG(Part,Command));
  }
}
//---------------------------------------------------------------------------
void agenda_floppy_read_address(int idx)
{
  int floppyno=floppy_current_drive();
  TFloppyImage *floppy=&FloppyDrive[floppyno];
  FDC_IDField IDList[30];
  int nSects=floppy->GetIDFields(floppy_current_side(),floppy_head_track[floppyno],IDList);
  if (idx<nSects){
    log(Str("FDC: Reading address for sector ")+IDList[idx].SectorNum+" on track "+
                floppy_head_track[floppyno]+", side "+floppy_current_side()+" hbls="+hbl_count);
    fdc_read_address_buffer[fdc_read_address_buffer_len++]=IDList[idx].Track;
    fdc_read_address_buffer[fdc_read_address_buffer_len++]=IDList[idx].Side;
    fdc_read_address_buffer[fdc_read_address_buffer_len++]=IDList[idx].SectorNum;
    fdc_read_address_buffer[fdc_read_address_buffer_len++]=IDList[idx].SectorLen;
    fdc_read_address_buffer[fdc_read_address_buffer_len++]=IDList[idx].CRC1;
    fdc_read_address_buffer[fdc_read_address_buffer_len++]=IDList[idx].CRC2;
    if (fdc_read_address_buffer_len>=16){ // DMA buffering madness
      for (int n=0;n<16;n++) write_to_dma(fdc_read_address_buffer[n]);
      memmove(fdc_read_address_buffer,fdc_read_address_buffer+16,4);
      fdc_read_address_buffer_len-=16;
    }
    fdc_str&=BYTE(~FDC_STR_WRITE_PROTECT);
    fdc_str|=FDC_STR_MOTOR_ON;
    agenda_fdc_finished(0);
    disk_light_off_time=timer+DisableDiskLightAfter;
    agenda_delete(agenda_fdc_motor_flag_off);
    agenda_add(agenda_fdc_motor_flag_off,MILLISECONDS_TO_HBLS(1800),0);
  }
}
//---------------------------------------------------------------------------
void agenda_floppy_read_track(int part)
{
  static int BytesRead;
  static WORD CRC;
  int floppyno=floppy_current_drive();
  TFloppyImage *floppy=&FloppyDrive[floppyno];
  bool Error=0;
  bool FromFormat=0;
  if (floppy_head_track[floppyno]<=FLOPPY_MAX_TRACK_NUM){
    FromFormat=floppy->TrackIsFormatted[floppy_current_side()][floppy_head_track[floppyno]];
  }
  if (part==0) BytesRead=0;
  disk_light_off_time=timer+DisableDiskLightAfter;
  fdc_str|=FDC_STR_BUSY;
  if (floppy_access_ff) floppy_access_ff_counter=FLOPPY_FF_VBL_COUNT;
  agenda_delete(agenda_fdc_motor_flag_off);
  agenda_add(agenda_fdc_motor_flag_off,MILLISECONDS_TO_HBLS(1800),0);

  int TrackBytes=floppy->GetRawTrackData(floppy_current_side(),floppy_head_track[floppyno]);
  if (TrackBytes){
    fseek(floppy->f,BytesRead,SEEK_CUR);
    int ReinsertAttempts=0;
    BYTE Temp,*lpDest;
    for (int n=0;n<16;n++){
      if (BytesRead>=TrackBytes) break;

      if (DMA_ADDRESS_IS_VALID_W && dma_sector_count){
        lpDest=lpPEEK(dma_address);
      }else{
        lpDest=&Temp;
      }
      if (fread(lpDest,1,1,floppy->f)==0){
        if (ReinsertAttempts++ > 2){
          Error=true;
          break;
        }
        if (floppy->ReinsertDisk()){
          TrackBytes=floppy->GetRawTrackData(floppy_current_side(),floppy_head_track[floppyno]);
          fseek(floppy->f,BytesRead,SEEK_CUR);
          n--;
        }
      }else{
        DMA_INC_ADDRESS;
        BytesRead++;
      }
    }
  }else{
    int DDBytes=6272-(floppy_head_track[floppyno]/25)*16; // inner tracks (higher numbers) are a bit shorter

    FDC_IDField IDList[30];
    int nSects=floppy->GetIDFields(floppy_current_side(),floppy_head_track[floppyno],IDList);
    if (nSects==0){
      // Unformatted track, read in random values
      TrackBytes=DDBytes;
      for (int bb=0;bb<16;bb++){
        write_to_dma(rand());
        BytesRead++;
      }
    }else{
      // Find out if it is a high density track
      TrackBytes=0;
      for (int n=0;n<nSects;n++) TrackBytes+=22+12+3+1+6+22+12+3+1 + (128 << IDList[n].SectorLen) + 26;
      if (TrackBytes>DDBytes){
        TrackBytes=DDBytes*2;
      }else{
        TrackBytes=DDBytes;
      }
      if (part/154<nSects){
        int IDListIdx=part/154;
        BYTE SectorNum=IDList[IDListIdx].SectorNum;
        int SectorBytes=(128 << IDList[IDListIdx].SectorLen);

        BYTE pre_sect[200];
        int i=0;
        for (int n=0;n<22;n++) pre_sect[i++]=0x4e;  // Gap 1 & 3 (22 bytes)
        for (int n=0;n<12;n++) pre_sect[i++]=0x00;  // Gap 3 (12)
        for (int n=0;n<3;n++) pre_sect[i++]=0xa1;   // Marker
        pre_sect[i++]=0xfe;                         // Start of address mark
        pre_sect[i++]=IDList[IDListIdx].Track;
        pre_sect[i++]=IDList[IDListIdx].Side;
        pre_sect[i++]=IDList[IDListIdx].SectorNum;
        pre_sect[i++]=IDList[IDListIdx].SectorLen;
        pre_sect[i++]=IDList[IDListIdx].CRC1;
        pre_sect[i++]=IDList[IDListIdx].CRC2;
        for (int n=0;n<22;n++) pre_sect[i++]=0x4e; // Gap 2
        for (int n=0;n<12;n++) pre_sect[i++]=0x00; // Gap 2
        for (int n=0;n<3;n++) pre_sect[i++]=0xa1;  // Marker
        pre_sect[i++]=0xfb;                        // Start of data

        int num_bytes_to_write=16;
        int byte_idx=(part % 154)*16;

        // Write the gaps/address before the sector
        if (byte_idx<i){
          while (num_bytes_to_write>0){
            write_to_dma(pre_sect[byte_idx++]);
            num_bytes_to_write--;
            BytesRead++;
            if (byte_idx>=i) break;
          }
        }
        byte_idx-=i;

        // Write the sector
        if (num_bytes_to_write>0 && byte_idx>=0 && byte_idx<SectorBytes){
          if (byte_idx==0){
            CRC=0xffff;
            fdc_add_to_crc(CRC,0xa1);
            fdc_add_to_crc(CRC,0xa1);
            fdc_add_to_crc(CRC,0xa1);
            fdc_add_to_crc(CRC,0xfb);
          }
          if (floppy->SeekSector(floppy_current_side(),floppy_head_track[floppyno],
                                    SectorNum,FromFormat)){
            // Can't seek to sector!
            while (num_bytes_to_write>0){
              write_to_dma(0x00);
              fdc_add_to_crc(CRC,0x00);
              num_bytes_to_write--;
              BytesRead++;
              byte_idx++;
              if (byte_idx>=SectorBytes) break;
            }
          }else{
            FILE *f=(FILE*)(FromFormat ? floppy->Format_f:floppy->f);
            fseek(f,byte_idx,SEEK_CUR);
            BYTE Temp,*pDest;
            for (;num_bytes_to_write>0;num_bytes_to_write--){
              if (DMA_ADDRESS_IS_VALID_W && dma_sector_count){
                pDest=lpPEEK(dma_address);
              }else{
                pDest=&Temp;
              }
              if (fread(pDest,1,1,f)==0){
                if (floppy_handle_file_error(floppyno,0,SectorNum,byte_idx,FromFormat)){
                  fdc_str=FDC_STR_MOTOR_ON | FDC_STR_SEEK_ERROR | FDC_STR_BUSY;
                  Error=true;
                  num_bytes_to_write=0;
                  break;
                }
              }
              fdc_add_to_crc(CRC,*pDest);
              DMA_INC_ADDRESS;
              BytesRead++;
              byte_idx++;
              if (byte_idx>=SectorBytes) break;
            }
          }
        }
        byte_idx-=SectorBytes;

        // Write CRC
        if (num_bytes_to_write>0 && byte_idx>=0 && byte_idx<2){
          if (byte_idx==0){
            write_to_dma(HIBYTE(CRC));          // End of Data Field (CRC)
            byte_idx++;
            BytesRead++;
            num_bytes_to_write--;
          }
          if (byte_idx==1 && num_bytes_to_write>0){
            write_to_dma(LOBYTE(CRC));          // End of Data Field (CRC)
            byte_idx++;
            BytesRead++;
            num_bytes_to_write--;
          }
        }
        byte_idx-=2;

        // Write Gap 4
        if (num_bytes_to_write>0 && byte_idx>=0 && byte_idx<24){
          while (num_bytes_to_write>0){
            write_to_dma(0x4e);
            byte_idx++;
            num_bytes_to_write--;
            BytesRead++;
            if (byte_idx>=24){
              // Move to next sector (-1 because we ++ below)
              part=(IDListIdx+1)*154-1;
              break;
            }
          }
        }
      }else{
        // End of track, read in 0x4e
        write_to_dma(0x4e,16);
        BytesRead+=16;
      }
    }
  }
  part++;

  if (BytesRead>=TrackBytes){ //finished reading in track
    fdc_str=FDC_STR_MOTOR_ON;  //all fine!
    agenda_fdc_finished(0);
    log(Str("FDC: Read track finished, t=")+hbl_count);
  }else if (Error==0){   //read more of the track
    // 8000 bytes per revolution * 5 revolutions per second
    int bytes_per_second=8000*5;
    int hbls_per_second=HBLS_PER_SECOND_AVE; // 60hz and 50hz are roughly the same
    if (shifter_freq==MONO_HZ) hbls_per_second=int(HBLS_PER_SECOND_MONO);
    int n_hbls=hbls_per_second/(bytes_per_second/16);
    agenda_add(agenda_floppy_read_track,n_hbls,part);
  }
}

void agenda_floppy_write_track(int part)
{
  static int SectorLen,nSector=-1;
  int floppyno=floppy_current_drive();
  TFloppyImage *floppy=&FloppyDrive[floppyno];
  BYTE Data;
  int TrackBytes=6448; // Double density format only

  if (floppy->ReadOnly || floppy->STT_File){
    fdc_str=FDC_STR_MOTOR_ON | FDC_STR_WRITE_PROTECT;
    agenda_fdc_finished(0);
    return;
  }

  floppy->WrittenTo=true;

  bool Error=0;
  fdc_str|=FDC_STR_BUSY;
  if (floppy_access_ff) floppy_access_ff_counter=FLOPPY_FF_VBL_COUNT;
  agenda_delete(agenda_fdc_motor_flag_off);
  agenda_add(agenda_fdc_motor_flag_off,MILLISECONDS_TO_HBLS(1800),0);

  disk_light_off_time=timer+DisableDiskLightAfter;
  if (part==0){ // Find data/address marks
    // Find address marks and read SectorLen and nSector from the address
    // Must have [[0xa1] 0xa1] 0xa1 0xfe or [[0xc2] 0xc2] 0xc2 0xfe

    // Find data marks and increase part
    // Must have [[0xa1] 0xa1] 0xa1 0xfb or [[0xc2] 0xc2] 0xc2 0xfb
    for (int n=0;n<16;n++){
      Data=read_from_dma();
      dma_address++;
      floppy_write_track_bytes_done++;
      if (Data==0xa1 || Data==0xf5 || Data==0xc2 || Data==0xf6){ // Start of gap 3
        int Timeout=10;
        do{
          Data=read_from_dma();
          dma_address++;floppy_write_track_bytes_done++;
        }while ((Data==0xa1 || Data==0xf5 || Data==0xc2 || Data==0xf6) && (--Timeout)>0);
        if (Data==0xfe){ // Found address mark
          if (dma_address+4<himem){
            nSector=PEEK(dma_address+2);
            switch (PEEK(dma_address+3)){
              case 0:  SectorLen=128;break;
              case 1:  SectorLen=256;break;
              case 2:  SectorLen=512;break;
              case 3:  Error=true /*SectorLen=1024*/ ;break;
              default: Error=true;
            }
            if (Error){
              log(Str("FDC: Format data with invalid sector length (")+PEEK(dma_address+3)+"). Skipping this ID field.");
              Error=0;
            }
          }
        }else if (Data==0xfb){
          part++; // Read next SectorLen bytes of data
          break;
        }
      }
    }
  }else{
    bool IgnoreSector=true;
    if (nSector>=0){
      IgnoreSector=floppy->SeekSector(floppy_current_side(),floppy_head_track[floppyno],nSector,true);
    }
    if (IgnoreSector){
      LOG_ONLY( if (nSector<0) log("FDC: Format sector data with no address, it will be lost in the ether"); )
      LOG_ONLY( if (nSector>=0) log("FDC: Format can't write sector, sector number too big for this type of image"); )

      dma_address+=SectorLen;
      floppy_write_track_bytes_done+=SectorLen;
      part=0;
      nSector=-1;
    }else{
      fseek(floppy->Format_f,(part-1)*16,SEEK_CUR);

      floppy->FormatMostSectors=max(nSector,floppy->FormatMostSectors);
      floppy->FormatLargestSector=max(SectorLen,floppy->FormatLargestSector);
      floppy->TrackIsFormatted[floppy_current_side()][floppy_head_track[floppyno]]=true;

      for (int bb=0;bb<16;bb++){
        Data=read_from_dma();dma_address++;
/*
        if (Data==0xf5){
          Data=0xa1;
        }else if (Data==0xf6){
          Data=0xc2;
        }else if (Data==0xf7){
          WriteCRC=true;
          Data=0;
        }
*/
        if (fwrite(&Data,1,1,floppy->Format_f)==0){
          Error=true;
          if (floppy->ReopenFormatFile()){
            floppy->SeekSector(floppy_current_side(),floppy_head_track[floppyno],nSector,true);
            fseek(floppy->Format_f,(part-1)*16 + bb,SEEK_CUR);
            Error=(fwrite(&Data,1,1,floppy->Format_f)==0);
          }
        }
        if (Error) break;

        floppy_write_track_bytes_done++;
      }
      part++;
      if ((part-1)*16>=SectorLen){
        nSector=-1;
        part=0;
      }
    }
  }
  if (floppy_write_track_bytes_done>TrackBytes){
    log(Str("FDC: Format finished, wrote ")+floppy_write_track_bytes_done+" bytes");
    fdc_str=FDC_STR_MOTOR_ON;  //all fine!
    agenda_fdc_finished(0);
    fflush(floppy->Format_f);
  }else if (Error){
    log("FDC: Format aborted, can't write to format file");
    fdc_str=FDC_STR_MOTOR_ON | FDC_STR_SEEK_ERROR | FDC_STR_BUSY;
  }else{ //write more of the track
    // 8000 bytes per revolution * 5 revolutions per second
    int bytes_per_second=8000*5;
    int hbls_per_second=HBLS_PER_SECOND_AVE; // 60hz and 50hz are roughly the same
    if (shifter_freq==MONO_HZ) hbls_per_second=int(HBLS_PER_SECOND_MONO);
    int n_hbls=hbls_per_second/(bytes_per_second/16);
    agenda_add(agenda_floppy_write_track,n_hbls,part);
  }
}
//---------------------------------------------------------------------------
void fdc_add_to_crc(WORD &crc,BYTE data)
{
  for (int i=0;i<8;i++){
    crc = WORD((crc << 1) ^ ((((crc >> 8) ^ (data << i)) & 0x0080) ? 0x1021 : 0));
  }
}
//---------------------------------------------------------------------------
#undef LOGSECTION






