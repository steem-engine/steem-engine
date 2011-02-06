//---------------------------------------------------------------------------
int LoadSnapShotChangeDisks(Str NewDisk[2],Str NewDiskInZip[2],Str NewDiskName[2])
{
  for (int disk=0;disk<2;disk++){
    if (NewDisk[disk].IsEmpty()){
      DiskMan.EjectDisk(disk);
    }else{
      bool InsertedDisk=(FloppyDrive[disk].SetDisk(NewDisk[disk],NewDiskInZip[disk])==0);
      if (InsertedDisk==0){
        NewDisk[disk]=EasyStr(GetFileNameFromPath(NewDisk[disk]));
        if (FloppyDrive[disk].SetDisk(DiskMan.HomeFol+SLASH+NewDisk[disk],NewDiskInZip[disk])){
          if (FloppyDrive[disk].SetDisk(RunDir+SLASH+NewDisk[disk],NewDiskInZip[disk])){
            int Ret=Alert(T("When this snapshot was taken there was a disk called")+" "+
                            NewDisk[disk]+" "+T("in ST drive")+" "+char('A'+disk)+". "+
                            T("Steem cannot find this disk. Having different disks in the drives after loading the snapshot could cause errors.")+
                            "\n\n"+T("Do you want to find this disk or its equivalent?"),
                            T("Cannot Find Disk"),MB_YESNOCANCEL | MB_ICONQUESTION);
            if (Ret==IDYES){
              EasyStr Fol=DiskMan.HomeFol,NewerDisk;

              LOOP{
#ifdef WIN32
                NewerDisk=FileSelect(StemWin,T("Locate")+" "+NewDisk[disk],Fol,
                                      FSTypes(2,NULL),1,true,"st");
#elif defined(UNIX)
                fileselect.set_corner_icon(&Ico16,ICO16_DISK);
                NewerDisk=fileselect.choose(XD,Fol,GetFileNameFromPath(NewDisk[disk]),
                                        T("Locate")+" "+NewDisk[disk],FSM_LOAD | FSM_LOADMUSTEXIST,
                                        diskfile_parse_routine,".st");
#endif
                if (NewerDisk.IsEmpty()){
                  if (Alert(T("Do you want to continue trying to load this snapshot?"),T("Carry On Regardless?"),
                                MB_YESNO | MB_ICONQUESTION)==IDNO){
                    return 1;
                  }
                  break;
                }else{
                  if (FloppyDrive[disk].SetDisk(NewerDisk)){
                    int Ret=Alert(T("The disk image you selected is not valid. Do you want to try again? Click on cancel to give up trying to load this snapshot."),
                                        T("Invalid Disk Image"),MB_YESNOCANCEL | MB_ICONEXCLAMATION);
                    if (Ret==IDCANCEL){
                      return 1;
                    }else if (Ret==IDYES){
                      Fol=NewerDisk;
                      RemoveFileNameFromPath(Fol,REMOVE_SLASH);
                    }else{
                      break;
                    }
                  }else{
                    InsertedDisk=true;
                    break;
                  }
                }
              }
            }else if (Ret==IDCANCEL){
              return 1;
            }
          }else{
            InsertedDisk=true;
          }
        }else{
          InsertedDisk=true;
        }
      }
      if (InsertedDisk){
        DiskMan.InsertHistoryAdd(disk,NewDiskName[disk],FloppyDrive[disk].GetDisk(),NewDiskInZip[disk]);
        FloppyDrive[disk].DiskName=NewDiskName[disk];
        if (DiskMan.IsVisible()){
          DiskMan.InsertDisk(disk,FloppyDrive[disk].DiskName,FloppyDrive[disk].GetDisk(),true,0,NewDiskInZip[disk]);
        }
      }
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
int LoadSnapShotChangeCart(Str NewCart)
{
  if (NewCart.Empty()){
    // Remove cart? Yes
    if (cart) delete[] cart;
    cart=NULL;
    CartFile="";
    return 0;
  }

  if (load_cart(NewCart)==0){
    CartFile=NewCart;
    return 0;
  }

  Str NewCartName=GetFileNameFromPath(NewCart);
  char *dot=strrchr(NewCartName,'.');
  if (dot) *dot=0;

  Str Fol=NewCart;
  RemoveFileNameFromPath(Fol,REMOVE_SLASH);
  if (GetFileAttributes(Fol)==0xffffffff){
    Fol=OptionBox.LastCartFile;
    RemoveFileNameFromPath(Fol,REMOVE_SLASH);
  }

  int Ret=Alert(T("When this snapshot was taken there was a cartridge inserted called")+" "+
                  NewCartName+". "+T("Steem cannot find this cartridge, the snapshot may not work properly without it.")+
                  "\n\n"+T("Do you want to find this cartridge?"),
                  T("Cannot Find Cartridge"),MB_YESNOCANCEL | MB_ICONQUESTION);
  if (Ret==IDCANCEL){
    return 1;
  }else if (Ret==IDYES){
    Str NewerCart;
    for(;;){
#ifdef WIN32
      NewerCart=FileSelect(StemWin,T("Locate")+" "+NewCartName,Fol,
                            FSTypes(0,T("ST Cartridge Images").Text,"*.stc",NULL),1,true,"stc");
#elif defined(UNIX)
      fileselect.set_corner_icon(&Ico16,ICO16_CHIP);
      NewerCart=fileselect.choose(XD,Fol,NewCartName,T("Locate")+" "+NewCartName,FSM_LOAD | FSM_LOADMUSTEXIST,
                              cartfile_parse_routine,".stc");
#endif
      if (NewerCart.IsEmpty()){
        if (Alert(T("Do you want to continue trying to load this snapshot?"),T("Carry On Regardless?"),
                      MB_YESNO | MB_ICONQUESTION)==IDNO){
          return 1;
        }
        break;
      }else{
        if (load_cart(NewerCart)){
          int Ret=Alert(T("The cartridge you selected is not valid. Do you want to try again? Click on cancel to give up trying to load this snapshot."),
                              T("Invalid Cartridge Image"),MB_YESNOCANCEL | MB_ICONEXCLAMATION);
          if (Ret==IDCANCEL){
            return 1;
          }else if (Ret==IDYES){
            Fol=NewerCart;
            RemoveFileNameFromPath(Fol,REMOVE_SLASH);
          }else{
            break;
          }
        }else{
          CartFile=NewerCart;
          break;
        }
      }
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
int LoadSnapShotChangeTOS(Str NewROM,int NewROMVer)
{
  bool Fail=0;
  if (load_TOS(NewROM)){
    EasyStr NewROMVersionInfo;
    if (NewROMVer<=0x700) NewROMVersionInfo=Str(" (")+T("version number")+" "+HEXSl(NewROMVer,4)+")";
    int Ret=Alert(T("When this snapshot was taken the TOS image being used was ")+
                      NewROM+NewROMVersionInfo+". "+T("This file cannot now be used, it is either missing or corrupt. Do you want to find an equivalent TOS image, without doing so you cannot load this snapshot."),
                      T("Cannot Use TOS Image"),MB_YESNO | MB_ICONEXCLAMATION);
    if (Ret==IDNO) return 1;

    EasyStr ROMName=GetFileNameFromPath(NewROM);
    LOOP{
      EasyStr Title=T("Locate")+" "+ROMName+NewROMVersionInfo;
#ifdef WIN32
      NewROM=FileSelect(StemWin,Title,RunDir,FSTypes(3,NULL),1,true,"img");
#elif defined(UNIX)
      fileselect.set_corner_icon(&Ico16,ICO16_CHIP);
      NewROM=fileselect.choose(XD,RunDir,ROMName,Title,FSM_LOAD | FSM_LOADMUSTEXIST,
                              romfile_parse_routine,".img");
#endif

      if (NewROM.IsEmpty()){
        Fail=true;
        break;
      }

      if (load_TOS(NewROM)){
        int Ret=Alert(T("This TOS image is corrupt! Do you want to try again?"),
                          T("Cannot Use TOS Image"),MB_YESNO | MB_ICONEXCLAMATION);
        if (Ret==IDNO){
          Fail=true;
          break;
        }
      }else{
        // Check version number
        if (NewROMVer>0x700) break;         // No version number saved
        if (NewROMVer==tos_version) break;

        int Ret=Alert(T("This TOS image's version number doesn't match. Do you want to choose a different one?"),
                        T("TOS Image Version Different"),MB_YESNOCANCEL | MB_ICONQUESTION);
        if (Ret==IDCANCEL || Ret==IDNO){
          if (Ret==IDCANCEL) Fail=true;
          break;
        }
      }
    }
  }
  if (Fail){
    return 1;
  }else{
    ROMFile=NewROM;
  }
  return 0;
}
//---------------------------------------------------------------------------
void LoadSaveChangeNumFloppies(int NumFloppyDrives)
{
  DiskMan.SetNumFloppies(NumFloppyDrives);
}
//---------------------------------------------------------------------------
void AddSnapShotToHistory(char *FilNam)
{
  for (int n=0;n<10;n++){
    if (IsSameStr_I(FilNam,StateHist[n])) StateHist[n]="";
  }
  for (int n=0;n<10;n++){
    bool NoMore=true;
    for (int i=n;i<10;i++){
      if (StateHist[i].NotEmpty()){
        NoMore=0;
        break;
      }
    }
    if (NoMore) break;
    if (StateHist[n].Empty()){
      for (int i=n;i<9;i++){
        StateHist[i]=StateHist[i+1];
      }
      n--;
    }
  }
  for (int n=9;n>0;n--){
    StateHist[n]=StateHist[n-1];
  }
  StateHist[0]=FilNam;
}
//---------------------------------------------------------------------------
bool LoadSnapShot(char *FilNam,bool Auto=0,bool ShowErrorMess=true)
{
#ifndef ONEGAME
  int Failed=2,Version=0;
  bool FileError=0;

  if (Exists(FilNam)==0) FileError=true;
  if (FileError==0){
		reset_st(true,0,0); // Don't change settings to what the user has chosen

    FILE *f=fopen(FilNam,"rb");
    if (f){
      Failed=LoadSaveAllStuff(f,LS_LOAD,-1,(Auto==0),&Version);
      if (Failed==0){
        Failed=int((EasyUncompressToMem(Mem+MEM_EXTRA_BYTES,mem_len,f)!=0) ? 2:0);
      }
      fclose(f);
    }else{
      FileError=true;
    }
  }
  if (FileError){
    Alert(T("Cannot open the snapshot file:")+"\n\n"+FilNam,T("Load Memory Snapshot Failed"),MB_ICONEXCLAMATION);
		return 0;
  }
#else
  reset_st(true,0,0); // Don't change settings to what the user has chosen

  BYTE *p=(BYTE*)FilNam;
  int Failed=LoadSaveAllStuff(p,LS_LOAD,-1,(Auto==0),&Version);
  if (Failed==0) Failed=EasyUncompressToMemFromMem(Mem+MEM_EXTRA_BYTES,mem_len,p);
  if (Failed) Failed=1; 
#endif

  if (Failed==0){
    if (Auto==0) AddSnapShotToHistory(FilNam);

    LoadSnapShotUpdateVars(Version);

    OptionBox.NewMemConf0=-1;
    OptionBox.NewMonitorSel=-1;
    OptionBox.NewROMFile="";
    OptionBox.MachineUpdateIfVisible();
    CheckResetIcon();
    CheckResetDisplay();
    DEBUG_ONLY( update_register_display(true); )
  }else{
    if (Failed>1 && ShowErrorMess){
      Alert(T("Cannot load the snapshot, it is corrupt."),T("Load Memory Snapshot Failed"),MB_ICONEXCLAMATION);
    }
    reset_st(); // Change settings if required
  }
  return Failed==0;
}
//---------------------------------------------------------------------------
#ifndef ONEGAME
void SaveSnapShot(char *FilNam,int Version=-1,bool Auto=0)
{
  FILE *f=fopen(FilNam,"wb");
  if (f!=NULL){
    LoadSaveAllStuff(f,LS_SAVE,Version);

    EasyCompressFromMem(Mem+MEM_EXTRA_BYTES,mem_len,f);

    fclose(f);

    if (Auto==0) AddSnapShotToHistory(FilNam);
  }
}
#else
void SaveSnapShot(char *,int=-1,bool=0) {}
#endif
//---------------------------------------------------------------------------
#ifdef ENABLE_LOGFILE
void load_logsections()
{
  for (int n=0;n<100;n++) logsection_enabled[n]=true;

  FILE *f=fopen(WriteDir+SLASH "logsection.dat","rb");
  if (f!=NULL){
    char tb[50];
    for(;;){
      if (fgets(tb,49,f)==0) break;
      if (tb[0]==0) break;
      int n=atoi(tb);
      if (n>0 && n<100) logsection_enabled[n]=false;
    }
    fclose(f);
  }
  if (logsection_enabled[LOGSECTION_CPU]) log_cpu_count=CPU_INSTRUCTIONS_TO_LOG;
}
#endif
//---------------------------------------------------------------------------
#ifndef ONEGAME
MEM_ADDRESS get_TOS_address(char *File)
{
  if (File[0]==0) return 0;

  FILE *f=fopen(File,"rb");
  if (f==NULL) return 0;

  BYTE HiHi=0,LoHi=0,HiLo=0,LoLo=0;
  fread(&HiLo,1,1,f);
  fread(&LoLo,1,1,f);
  if (HiLo==0x60 && LoLo==0x08){ // Pre-tos machines, need boot disk, no header
    fclose(f);
    return 0xfc0000;
  }else{
    fseek(f,8,SEEK_SET);
    fread(&HiHi,1,1,f);
    fread(&LoHi,1,1,f);
    fread(&HiLo,1,1,f);
    fread(&LoLo,1,1,f);
    fclose(f);

    MEM_ADDRESS new_rom_addr=MAKELONG(MAKEWORD(LoLo,HiLo),MAKEWORD(LoHi,HiHi)) & 0xffffff;
    if (new_rom_addr==0xfc0000) return 0xfc0000;
    if (new_rom_addr==0xe00000) return 0xe00000;
  }
  return 0;
}

bool load_TOS(char *File)
{
  if (File[0]==0) return true;

  MEM_ADDRESS new_rom_addr=get_TOS_address(File);

  FILE *f=fopen(File,"rb");
  if (f==NULL) return true;

  if (new_rom_addr==0xfc0000){
    tos_high=true;
    tos_len=192*1024;
  }else if (new_rom_addr==0xe00000){
    tos_high=0;
    tos_len=256*1024;
  }else{
    fclose(f);
    return true;
  }

  rom_addr=new_rom_addr;
  Rom_End=Rom+tos_len;
  Rom_End_minus_1=Rom_End-1;
  Rom_End_minus_2=Rom_End-2;
  Rom_End_minus_4=Rom_End-4;

  memset(Rom,0xff,256*1024);

  DWORD Len=GetFileLength(f);
  if (Len>tos_len) Len=tos_len;
  for (DWORD m=0;m<Len;m++){
    ROM_PEEK(m)=(BYTE)fgetc(f);
  }
  fclose(f);

  tos_version=ROM_DPEEK(2);
  return 0;
}

#else

bool load_TOS(char *)
{
  tos_len=192*1024;
  tos_high=true;
  rom_addr=0xFC0000;
  Rom_End=Rom+tos_len;
  Rom_End_minus_1=Rom_End-1;
  Rom_End_minus_2=Rom_End-2;
  Rom_End_minus_4=Rom_End-4;
  tos_version=0x0102;
  return 0;
}
#endif
//---------------------------------------------------------------------------
bool load_cart(char *filename) // return true on failure
{
  FILE *f=fopen(filename,"rb");
  if (f==NULL) return true;

  bool Loaded=0;
  DWORD Type=0xffffffff;
  fread(&Type,4,1,f);
  if (Type==0){
    long Len=GetFileLength(f)-4;
    if (Len!=128*1024){
      fclose(f);
      return true;  //not valid length
    }
    if (cart) delete[] cart;
    cart=new BYTE[128*1024];

    for (int bn=Len-1;bn>=0;bn--) fread(cart+bn,1,1,f);

    Cart_End_minus_1=cart+(128*1024-1);
    Cart_End_minus_2=Cart_End_minus_1-1;
    Cart_End_minus_4=Cart_End_minus_1-3;

    if (pc>=MEM_EXPANSION_CARTRIDGE && pc<0xfc0000){
    	SET_PC(PC32);
    }
    Loaded=true;
  }
  fclose(f);
  return Loaded==0;
}
//---------------------------------------------------------------------------
#ifdef _DEBUG_BUILD
typedef struct{
  MEM_ADDRESS ad;
  type_disp_type type;
  int x,y,w,h;
  int n_cols,col_w[20];
}MEM_BROW_LOAD;
#endif

void LoadState(GoodConfigStoreFile *pCSF)
{
  LoadAllDialogData(true,INIFile,NULL,pCSF);
  log_to(LOGSECTION_INIT,"STARTUP: Finished LoadAllDialogData");

#ifdef _DEBUG_BUILD
  char wt[50];
  DynamicArray<MEM_BROW_LOAD> browsers;
  int dru_combo_idx=0;
  Str dru_edit;

  log_to(LOGSECTION_INIT,Str("STARTUP: Loading ")+WriteDir+"\\breaks.dat");
  FILE *bf=fopen(WriteDir+"\\breaks.dat","rb");
  if (bf!=NULL){
    fgets(wt,49,bf);
    num_breakpoints=atoi(wt);
    for (int n=0;n<num_breakpoints;n++){
      fgets(wt,49,bf);
      breakpoint[n]=HexToVal(wt);
    }

    breakpoint_menu_setup();
    fgets(wt,49,bf);
    crash_notification=atoi(wt) & 3;
    LOOP{
      wt[0]='*';
      if (fgets(wt,49,bf)==NULL) break; //get window title
      if (wt[0]=='*'){
        break;
      }else if (strstr(wt,"Monitors")){
        num_monitors=file_read_num(bf);
        for (int n=0;n<num_monitors;n++){
          fgets(wt,49,bf);
          monitor_ad[n]=HexToVal(wt);
          monitor_contents[n]=d2_dpeek(monitor_ad[n]) & monitor_mask[n];
        }
      }else if (strstr(wt,"Monitor_masks")){
        num_monitors=file_read_num(bf);
        for (int n=0;n<num_monitors;n++){
          fgets(wt,49,bf);
          monitor_mask[n]=(WORD)HexToVal(wt);
          monitor_contents[n]=d2_dpeek(monitor_ad[n]) & monitor_mask[n];
        }
      }else if (strstr(wt,"IO_mons")){
        num_io_monitors=file_read_num(bf);
        for (int n=0;n<num_io_monitors;n++){
          fgets(wt,49,bf);
          monitor_io_ad[n]=HexToVal(wt);
          fgets(wt,49,bf);
          monitor_io_mask[n]=(WORD)HexToVal(wt);
          monitor_io_readflag[n]=bool(file_read_num(bf));
        }
      }else if (strstr(wt,"Monitor_breakpoints")){
        monitor_mode=file_read_num(bf);
      }else if (strstr(wt,"Breakpoint_mode")){
        breakpoint_mode=file_read_num(bf);
      }else if (strstr(wt,"Suspend_logging")){
        logging_suspended=file_read_num(bf);
      }else if (strstr(wt,"Stack_display")){
        boiler_show_stack_display(file_read_num(bf));
      }else if (strstr(wt,"Wipe_log_on_reset")){
        debug_wipe_log_on_reset=(bool)file_read_num(bf);
      }else if (strstr(wt,"Brow_on_taskbar")){
        mem_browser::ex_style=(DWORD)file_read_num(bf);
      }else if (strstr(wt,"Log_view_prog")){
        LogViewProg.SetLength(MAX_PATH);
        fgets(LogViewProg.Text,MAX_PATH,bf);
        while (LogViewProg.RightChar()=='\n' || LogViewProg.RightChar()=='\r'){
          *LogViewProg.Right()=0;
        }
      }else if (strstr(wt,"Gun_display_pos_col")){
        debug_gun_pos_col=(DWORD)file_read_num(bf);
      }else if (strstr(wt,"trace_show_window")){
        trace_show_window=(bool)file_read_num(bf);
      }else if (strstr(wt,"debug_run_until_combo")){
        dru_combo_idx=file_read_num(bf);
      }else if (strstr(wt,"debug_run_until_edit")){
        dru_edit.SetLength(49);
        fgets(dru_edit.Text,49,bf);
        while (dru_edit.RightChar()==10 || dru_edit.RightChar()==13) *dru_edit.Right()=0;
      }else{
        int old_pos=ftell(bf);
        int x1,y1,x2,y2;
        x1=file_read_num(bf);y1=file_read_num(bf);
        x2=file_read_num(bf);y2=file_read_num(bf);
        if (strstr(wt,"DWin")){
          MoveWindow(DWin,x1,y1,x2-x1,y2-y1,true);
        }else if (strstr(wt,"trace")){
          SetWindowPos(trace_window_handle,0,x1,y1,0,0,SWP_NOSIZE | SWP_NOZORDER);
        }else if (strstr(wt,"browser") || strstr(wt,"25_brow")){
          MEM_BROW_LOAD b;
          b.ad=(MEM_ADDRESS)file_read_num(bf);
          b.type=(type_disp_type)file_read_num(bf);
          b.x=x1, b.y=y1, b.w=x2-x1, b.h=y2-y1;
          if (strstr(wt,"25_brow")){
            int n_cols=file_read_num(bf);
            b.n_cols=min(n_cols,20);
            for (int n=0;n<n_cols;n++){
              int c=file_read_num(bf);
              if (n<20) b.col_w[n]=c;
            }
          }
          browsers.Add(b);
        }else{
          fseek(bf,old_pos,SEEK_SET);
        }
      }
    }
    log_to(LOGSECTION_INIT,Str("STARTUP: Finished loading ")+WriteDir+"\\breaks.dat");
    fclose(bf);
  }else{
    num_breakpoints=0;
    num_monitors=0;
    num_io_monitors=0;
  }
  log_to(LOGSECTION_INIT,"STARTUP: Updating debug GUI");
  UPDATE_DO_MON_CHECK;
  UPDATE_DO_BREAK_CHECK;
  CheckMenuRadioItem(boiler_op_menu,1501,1503,1501+crash_notification,MF_BYCOMMAND);
  CheckMenuItem(boiler_op_menu,1514,MF_BYCOMMAND | int(trace_show_window ? MF_CHECKED:MF_UNCHECKED));
  CheckMenuItem(logsection_menu,1013,MF_BYCOMMAND | int(debug_wipe_log_on_reset ? MF_CHECKED:MF_UNCHECKED));
  CheckMenuItem(breakpoint_menu,1103,MF_BYCOMMAND | int((monitor_mode==1) ? MF_CHECKED:MF_UNCHECKED));
  CheckMenuItem(breakpoint_menu,1104,MF_BYCOMMAND | int((monitor_mode==2) ? MF_CHECKED:MF_UNCHECKED));
  CheckMenuItem(breakpoint_menu,1107,MF_BYCOMMAND | int((breakpoint_mode==1) ? MF_CHECKED:MF_UNCHECKED));
  CheckMenuItem(breakpoint_menu,1108,MF_BYCOMMAND | int((breakpoint_mode==2) ? MF_CHECKED:MF_UNCHECKED));
  CheckMenuItem(logsection_menu,1012,MF_BYCOMMAND | int(logging_suspended ? MF_CHECKED:MF_UNCHECKED));
  CheckMenuItem(mem_browser_menu,907,MF_BYCOMMAND | int(mem_browser::ex_style ? 0:MF_CHECKED));
  SendDlgItemMessage(DWin,1020,CB_SETCURSEL,dru_combo_idx,0);
  SetWindowText(GetDlgItem(DWin,1021),dru_edit);

  for (int b=0;b<browsers.NumItems;b++){
    mem_browser *mb=new mem_browser(browsers[b].ad,browsers[b].type);
    if (browsers[b].n_cols==mb->columns){
      SendMessage(mb->handle,WM_SETREDRAW,0,0);
      for (int n=0;n<browsers[b].n_cols;n++){
        if (browsers[b].col_w[n]>=0 && browsers[b].col_w[n]<2000){
          SendMessage(mb->handle,LVM_SETCOLUMNWIDTH,n,MAKELPARAM(browsers[b].col_w[n],0));
        }
      }
      SendMessage(mb->handle,WM_SETREDRAW,1,0);
    }
    MoveWindow(mb->owner,browsers[b].x,browsers[b].y,browsers[b].w,browsers[b].h,true);
  }

#endif

  log_to(LOGSECTION_INIT,Str("STARTUP: Loading ")+RunDir+SLASH "steem.new");
  FILE *f=fopen(RunDir+SLASH "steem.new","rt");
  if (f){
    int blanks=0;
    osd_scroller_array.Sort=eslNoSort;
    for(;;){
      char tb[200];
      if (fgets(tb,198,f)==NULL) break;
      strupr(tb);
      if (tb[strlen(tb)-1]=='\n') tb[strlen(tb)-1]=0;
      if (tb[strlen(tb)-1]=='\r') tb[strlen(tb)-1]=0;
      if (tb[0]){
        bool ScrollerSection=0;
        if (IsSameStr_I(tb,"[SCROLLERS]")) ScrollerSection=true;
        if (IsSameStr_I(tb,"[XSCROLLERS]")) ScrollerSection=true;
        WIN_ONLY( if (IsSameStr_I(tb,"[WINSCROLLERS]")) ScrollerSection=true; )
        UNIX_ONLY( if (IsSameStr_I(tb,"[UNIXSCROLLERS]")) ScrollerSection=true; )
        if (ScrollerSection){
          while (tb[0]){
            if (fgets(tb,198,f)==NULL) break;
            if (tb[strlen(tb)-1]=='\n') tb[strlen(tb)-1]=0;
            if (tb[strlen(tb)-1]=='\r') tb[strlen(tb)-1]=0;
            if (tb[0]==0) break;
            osd_scroller_array.Add(tb);
          }
        }
      }else{
        if ((++blanks)>=2) break;
      }
    }
    log_to(LOGSECTION_INIT,Str("STARTUP: Finished loading ")+RunDir+SLASH "steem.new");
    fclose(f);
  }
}
//---------------------------------------------------------------------------
void SaveState(ConfigStoreFile *pCSF)
{
  SaveAllDialogData(true,INIFile,pCSF);

#ifdef _DEBUG_BUILD
  FILE *bf=fopen(WriteDir+"\\breaks.dat","wb");
  if (bf){
    fprintf(bf,"%i\n",num_breakpoints);
    for(int n=0;n<num_breakpoints;n++){
      fprintf(bf,"%s\n",HEXSl(breakpoint[n],6).Text);
    }
    fprintf(bf,"%i\n",crash_notification);

    WINPOSITIONDATA wpd;
    GetWindowPositionData(DWin,&wpd);
    fprintf(bf,"%s\n%i\n%i\n%i\n%i\n","DWin",wpd.Left,wpd.Top,wpd.Left+wpd.Width,wpd.Top+wpd.Height);
    GetWindowPositionData(trace_window_handle,&wpd);
    fprintf(bf,"%s\n%i\n%i\n%i\n%i\n","trace",wpd.Left,wpd.Top,wpd.Left+wpd.Width,wpd.Top+wpd.Height);

    for (int n=0;n<MAX_MEMORY_BROWSERS;n++){
      if (m_b[n]!=NULL){
        GetWindowPositionData(m_b[n]->owner,&wpd);
        fprintf(bf,"%s\n%i\n%i\n%i\n%i\n","25_brow",wpd.Left,wpd.Top,wpd.Left+wpd.Width,wpd.Top+wpd.Height);
        fprintf(bf,"%i\n%i\n%i\n",int(m_b[n]->ad),m_b[n]->disp_type,m_b[n]->columns);
        for (int m=0;m<m_b[n]->columns;m++){
          fprintf(bf,"%i\n",(int)SendMessage(m_b[n]->handle,LVM_GETCOLUMNWIDTH,m,0));
        }
      }
    }
    fprintf(bf,"Monitor_breakpoints\n%i\n",monitor_mode);
    fprintf(bf,"Breakpoint_mode\n%i\n",breakpoint_mode);
    fprintf(bf,"Suspend_logging\n%i\n",logging_suspended);

    fprintf(bf,"Monitors\n%i\n",num_monitors);
    for (int n=0;n<num_monitors;n++) fprintf(bf,"%s\n",HEXSl(monitor_ad[n],6).Text);
    fprintf(bf,"Monitor_masks\n%i\n",num_monitors);
    for (int n=0;n<num_monitors;n++) fprintf(bf,"%s\n",HEXSl(monitor_mask[n],4).Text);

    fprintf(bf,"IO_mons\n%i\n",num_io_monitors);
    for (int n=0;n<num_io_monitors;n++){
      fprintf(bf,"%s\n",HEXSl(monitor_io_ad[n],6).Text);
      fprintf(bf,"%s\n",HEXSl(monitor_io_mask[n],4).Text);
      fprintf(bf,"%i\n",int(monitor_io_readflag[n]));
    }

    int s=SendDlgItemMessage(DWin,209,CB_GETCURSEL,0,0);
    fprintf(bf,"Stack_display\n%i\n",s);
    fprintf(bf,"Wipe_log_on_reset\n%i\n",debug_wipe_log_on_reset);
    fprintf(bf,"Brow_on_taskbar\n%i\n",mem_browser::ex_style);
    fprintf(bf,"Log_view_prog\n%s\n",LogViewProg.Text);
    fprintf(bf,"Gun_display_pos_col\n%i\n",debug_gun_pos_col);
    fprintf(bf,"trace_show_window\n%i\n",trace_show_window);
    fprintf(bf,"debug_run_until_combo\n%i\n",SendDlgItemMessage(DWin,1020,CB_GETCURSEL,0,0));
    fprintf(bf,"debug_run_until_edit\n%s\n",GetWindowTextStr(GetDlgItem(DWin,1021)).Text);
    fprintf(bf,"*\n");
    fclose(bf);
  }

  bf=fopen(WriteDir+SLASH "logsection.dat","wb");
  if (bf){
    for (int n=0;n<100;n++){
      if (logsection_enabled[n]==false) fprintf(bf,"%i\r\n",n);
    }
    fprintf(bf,"\r\n");
    fclose(bf);
  }
#endif
  if (AutoLoadSnapShot) SaveSnapShot(WriteDir+SLASH+AutoSnapShotName+".sts",-1,true);
}

