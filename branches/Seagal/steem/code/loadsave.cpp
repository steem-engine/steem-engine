/*---------------------------------------------------------------------------
FILE: loadsave.cpp
MODULE: Steem
DESCRIPTION: Lots of functions to deal with loading and saving various data
to and from files. This includes loading TOS images and handling steem.ini
and steem.new.
---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
int LoadSnapShotChangeDisks(Str NewDisk[2],Str NewDiskInZip[2],Str NewDiskName[2])
{
  int save_mediach[2]={floppy_mediach[0],floppy_mediach[1]};
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
        floppy_mediach[disk]=save_mediach[disk];
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
bool LoadSnapShot(char *FilNam,bool AddToHistory=true,bool ShowErrorMess=true,bool ChangeDisks=true)
{
#ifndef ONEGAME
  int Failed=2,Version=0;
  bool FileError=0;

  if (Exists(FilNam)==0) FileError=true;
  if (FileError==0){
    bool LoadingResetBackup=IsSameStr_I(FilNam,WriteDir+SLASH+"auto_reset_backup.sts");
    bool LoadingLoadSnapBackup=IsSameStr_I(FilNam,WriteDir+SLASH+"auto_loadsnapshot_backup.sts");
    if (ChangeDisks && LoadingResetBackup==0 && LoadingLoadSnapBackup==0){ // Don't backup on auto load
      DeleteFile(WriteDir+SLASH+"auto_reset_backup.sts");
      SaveSnapShot(WriteDir+SLASH+"auto_loadsnapshot_backup.sts",-1,0);
    }
    reset_st(RESET_COLD | RESET_STOP | RESET_NOCHANGESETTINGS | RESET_NOBACKUP);

    FILE *f=fopen(FilNam,"rb");
    if (f){
      Failed=LoadSaveAllStuff(f,LS_LOAD,-1,ChangeDisks,&Version);
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
  reset_st(RESET_COLD | RESET_STOP | RESET_NOCHANGESETTINGS | RESET_NOBACKUP);

  BYTE *p=(BYTE*)FilNam;
  int Failed=LoadSaveAllStuff(p,LS_LOAD,-1,ChangeDisks,&Version);
  if (Failed==0) Failed=EasyUncompressToMemFromMem(Mem+MEM_EXTRA_BYTES,mem_len,p);
  if (Failed) Failed=1; 
#endif

  if (Failed==0){
    if (AddToHistory) AddSnapShotToHistory(FilNam);

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
    reset_st(RESET_COLD | RESET_STOP | RESET_CHANGESETTINGS | RESET_NOBACKUP);
  }
  return Failed==0;
}
//---------------------------------------------------------------------------
#ifndef ONEGAME
void SaveSnapShot(char *FilNam,int Version=-1,bool AddToHistory=true)
{
  FILE *f=fopen(FilNam,"wb");
  if (f!=NULL){
    LoadSaveAllStuff(f,LS_SAVE,Version);

    EasyCompressFromMem(Mem+MEM_EXTRA_BYTES,mem_len,f);

    fclose(f);

    if (AddToHistory) AddSnapShotToHistory(FilNam);
  }
}
#else
void SaveSnapShot(char *,int=-1,bool=true) {}
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
  char name[256];
}MEM_BROW_LOAD;
#endif

void LoadState(GoodConfigStoreFile *pCSF)
{
  LoadAllDialogData(true,INIFile,NULL,pCSF);
  log_to(LOGSECTION_INIT,"STARTUP: Finished LoadAllDialogData");

#ifdef _DEBUG_BUILD
  DynamicArray<MEM_BROW_LOAD> browsers;
  int dru_combo_idx=0;
  Str dru_edit;

  debug_ads.DeleteAll();
  for (int n=0;;n++){
    DEBUG_ADDRESS da;
    da.ad=pCSF->GetInt("Debug Addresses",Str("Address")+n,0xffffffff);
    if (da.ad==0xffffffff) break;
    da.mode=pCSF->GetInt("Debug Addresses",Str("Mode")+n,0);
    da.bwr=pCSF->GetInt("Debug Addresses",Str("BWR")+n,0);
    da.mask[0]=(WORD)pCSF->GetInt("Debug Addresses",Str("MaskW")+n,0xffff);
    da.mask[1]=(WORD)pCSF->GetInt("Debug Addresses",Str("MaskR")+n,0xffff);
    strcpy(da.name,pCSF->GetStr("Debug Addresses",Str("Name")+n,""));
    debug_ads.Add(da);
  }

  WINPOSITIONDATA wpd;
  GetWindowPositionData(DWin,&wpd);
  MoveWindow(DWin,pCSF->GetInt("Debug Options","Boiler Left",wpd.Left),pCSF->GetInt("Debug Options","Boiler Top",wpd.Top),
                    pCSF->GetInt("Debug Options","Boiler Width",wpd.Width),pCSF->GetInt("Debug Options","Boiler Height",wpd.Height),0);

  GetWindowPositionData(trace_window_handle,&wpd);
  MoveWindow(trace_window_handle,pCSF->GetInt("Debug Options","Trace Left",wpd.Left),pCSF->GetInt("Debug Options","Trace Top",wpd.Top),wpd.Width,wpd.Height,0);

  for (int n=0;n<MAX_MEMORY_BROWSERS;n++){
    Str Key=Str("Browser")+n+" ";
    MEM_BROW_LOAD b;
    b.x=pCSF->GetInt("Debug Browsers",Key+"Left",-300);
    if (b.x==-300) break;
    b.y=pCSF->GetInt("Debug Browsers",Key+"Top",0);
    b.w=pCSF->GetInt("Debug Browsers",Key+"Width",100);
    b.h=pCSF->GetInt("Debug Browsers",Key+"Height",100);
    b.ad=pCSF->GetInt("Debug Browsers",Key+"Address",0);
    strcpy(b.name,pCSF->GetStr("Debug Browsers",Key+"Name","Memory"));
    b.type=(type_disp_type)pCSF->GetInt("Debug Browsers",Key+"Type",0);
    b.n_cols=0;
    for (int m=0;m<20;m++){
      b.col_w[m]=pCSF->GetInt("Debug Browsers",Key+"Column"+m,-1);
      if (b.col_w[m]<0) break;
      b.n_cols++;
    }
    browsers.Add(b);
  }

  breakpoint_mode=pCSF->GetInt("Debug Options","Breakpoint Mode",breakpoint_mode);
  monitor_mode=pCSF->GetInt("Debug Options","Monitor Mode",monitor_mode);

  mem_browser::ex_style=pCSF->GetInt("Debug Options","Browsers on Taskbar",mem_browser::ex_style);

  logging_suspended=pCSF->GetInt("Debug Options","Suspend Logging",logging_suspended);
  debug_wipe_log_on_reset=pCSF->GetInt("Debug Options","Wipe Log On Reset",debug_wipe_log_on_reset);
  LogViewProg=pCSF->GetStr("Debug Options","Log Viewer",LogViewProg);

  crash_notification=pCSF->GetInt("Debug Options","Crash Notify",crash_notification);
  boiler_show_stack_display(pCSF->GetInt("Debug Options","Stack Display",0));

  debug_gun_pos_col=pCSF->GetInt("Debug Options","Gun Display Colour",debug_gun_pos_col);
  trace_show_window=pCSF->GetInt("Debug Options","Trace Show",trace_show_window);
  dru_combo_idx=pCSF->GetInt("Debug Options","Run Until",dru_combo_idx);
  dru_edit=pCSF->GetStr("Debug Options","Run Until Text",dru_edit);
  debug_monospace_disa=pCSF->GetInt("Debug Options","Monospace Disa",debug_monospace_disa);
  debug_uppercase_disa=pCSF->GetInt("Debug Options","Uppercase Disa",debug_uppercase_disa);

  log_to(LOGSECTION_INIT,"STARTUP: Updating debug GUI");
  debug_update_bkmon();
  CheckMenuRadioItem(boiler_op_menu,1501,1503,1501+crash_notification,MF_BYCOMMAND);
  CheckMenuItem(boiler_op_menu,1514,MF_BYCOMMAND | int(trace_show_window ? MF_CHECKED:MF_UNCHECKED));
  CheckMenuItem(boiler_op_menu,1515,MF_BYCOMMAND | int(debug_monospace_disa ? MF_CHECKED:MF_UNCHECKED));
  CheckMenuItem(boiler_op_menu,1516,MF_BYCOMMAND | int(debug_uppercase_disa ? MF_CHECKED:MF_UNCHECKED));

#if defined(STEVEN_SEAGAL) && defined(SS_DEBUG)
  SSDebug.OutputTraceToFile
    =pCSF->GetInt("Debug Options","OutputTraceToFile",SSDebug.OutputTraceToFile);
  SSDebug.TraceFileLimit
    =pCSF->GetInt("Debug Options","TraceFileLimit",SSDebug.TraceFileLimit);

  CheckMenuItem(boiler_op_menu,1517,MF_BYCOMMAND|
    ((int)(SSDebug.OutputTraceToFile)?MF_CHECKED:MF_UNCHECKED));
  CheckMenuItem(boiler_op_menu,1518,MF_BYCOMMAND|
    ((int)(SSDebug.TraceFileLimit)?MF_CHECKED:MF_UNCHECKED));
#endif
  
  CheckMenuItem(logsection_menu,1013,MF_BYCOMMAND | int(debug_wipe_log_on_reset ? MF_CHECKED:MF_UNCHECKED));
  CheckMenuItem(breakpoint_menu,1103,MF_BYCOMMAND | int((monitor_mode==2) ? MF_CHECKED:MF_UNCHECKED));
  CheckMenuItem(breakpoint_menu,1104,MF_BYCOMMAND | int((monitor_mode==3) ? MF_CHECKED:MF_UNCHECKED));
  CheckMenuItem(breakpoint_menu,1107,MF_BYCOMMAND | int((breakpoint_mode==2) ? MF_CHECKED:MF_UNCHECKED));
  CheckMenuItem(breakpoint_menu,1108,MF_BYCOMMAND | int((breakpoint_mode==3) ? MF_CHECKED:MF_UNCHECKED));
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
    SetWindowText(mb->owner,browsers[b].name);
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
  pCSF->DeleteSection("Debug Addresses");
  for (int n=0;n<debug_ads.NumItems;n++){
    pCSF->SetInt("Debug Addresses",Str("Address")+n,debug_ads[n].ad & 0xffffff);
    pCSF->SetInt("Debug Addresses",Str("Mode")+n,debug_ads[n].mode);
    pCSF->SetInt("Debug Addresses",Str("BWR")+n,debug_ads[n].bwr);
    pCSF->SetInt("Debug Addresses",Str("MaskW")+n,debug_ads[n].mask[0]);
    pCSF->SetInt("Debug Addresses",Str("MaskR")+n,debug_ads[n].mask[1]);
    pCSF->SetStr("Debug Addresses",Str("Name")+n,debug_ads[n].name);
  }

  WINPOSITIONDATA wpd;
  GetWindowPositionData(DWin,&wpd);
  pCSF->SetInt("Debug Options","Boiler Left",wpd.Left);
  pCSF->SetInt("Debug Options","Boiler Top",wpd.Top);
  pCSF->SetInt("Debug Options","Boiler Width",wpd.Width);
  pCSF->SetInt("Debug Options","Boiler Height",wpd.Height);

  GetWindowPositionData(trace_window_handle,&wpd);
  pCSF->SetInt("Debug Options","Trace Left",wpd.Left);
  pCSF->SetInt("Debug Options","Trace Top",wpd.Top);

  pCSF->DeleteSection("Debug Browsers");
  int i=0;
  for (int n=0;n<MAX_MEMORY_BROWSERS;n++){
    if (m_b[n]!=NULL){
      Str Key=Str("Browser")+i+" ";
      GetWindowPositionData(m_b[n]->owner,&wpd);
      pCSF->SetInt("Debug Browsers",Key+"Left",wpd.Left);
      pCSF->SetInt("Debug Browsers",Key+"Top",wpd.Top);
      pCSF->SetInt("Debug Browsers",Key+"Width",wpd.Width);
      pCSF->SetInt("Debug Browsers",Key+"Height",wpd.Height);
      pCSF->SetInt("Debug Browsers",Key+"Address",int(m_b[n]->ad));
      pCSF->SetInt("Debug Browsers",Key+"Type",m_b[n]->disp_type);
      for (int m=0;m<m_b[n]->columns;m++){
        pCSF->SetInt("Debug Browsers",Key+"Column"+m,(int)SendMessage(m_b[n]->handle,LVM_GETCOLUMNWIDTH,m,0));
      }
      pCSF->SetStr("Debug Browsers",Key+"Name",GetWindowTextStr(m_b[n]->owner));
      i++;
    }
  }

  pCSF->SetInt("Debug Options","Breakpoint Mode",breakpoint_mode);
  pCSF->SetInt("Debug Options","Monitor Mode",monitor_mode);

  pCSF->SetInt("Debug Options","Browsers on Taskbar",mem_browser::ex_style);

  pCSF->SetInt("Debug Options","Suspend Logging",logging_suspended);
  pCSF->SetInt("Debug Options","Wipe Log On Reset",debug_wipe_log_on_reset);
  pCSF->SetStr("Debug Options","Log Viewer",LogViewProg);

  pCSF->SetInt("Debug Options","Crash Notify",crash_notification);
  pCSF->SetInt("Debug Options","Stack Display",SendDlgItemMessage(DWin,209,CB_GETCURSEL,0,0));

  pCSF->SetInt("Debug Options","Gun Display Colour",debug_gun_pos_col);
  pCSF->SetInt("Debug Options","Trace Show",trace_show_window);
  pCSF->SetInt("Debug Options","Run Until",SendDlgItemMessage(DWin,1020,CB_GETCURSEL,0,0));
  pCSF->SetStr("Debug Options","Run Until Text",GetWindowTextStr(GetDlgItem(DWin,1021)));
  pCSF->SetInt("Debug Options","Monospace Disa",debug_monospace_disa);
  pCSF->SetInt("Debug Options","Uppercase Disa",debug_uppercase_disa);

#if defined(STEVEN_SEAGAL) && defined(SS_DEBUG)
  pCSF->SetInt("Debug Options","OutputTraceToFile",SSDebug.OutputTraceToFile);
  pCSF->SetInt("Debug Options","TraceFileLimit",SSDebug.TraceFileLimit);
#endif

  FILE *bf=fopen(WriteDir+SLASH "logsection.dat","wb");
  if (bf){
    for (int n=0;n<100;n++){
      if (logsection_enabled[n]==false) fprintf(bf,"%i\r\n",n);
    }
    fprintf(bf,"\r\n");
    fclose(bf);
  }
#endif
  if (AutoLoadSnapShot) SaveSnapShot(WriteDir+SLASH+AutoSnapShotName+".sts",-1,0);
}
//---------------------------------------------------------------------------
void LoadSavePastiActiveChange()
{
  DiskMan.RefreshDiskView();
}
//---------------------------------------------------------------------------

