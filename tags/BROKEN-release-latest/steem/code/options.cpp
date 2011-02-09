//---------------------------------------------------------------------------
bool TOptionBox::ChangeBorderModeRequest(int newborder)
{
  int newval=newborder;
  if (Disp.BorderPossible()==0 && (FullScreen==0)) newval=0;
  bool proceed=true;
  if (min(border,2)==min(newval,2)){
    proceed=false;
  }else if ((border^newval) & 1){
    if (FullScreen && draw_fs_blit_mode!=DFSM_LAPTOP){
      if (IDCANCEL==Alert(T("This will cause the monitor to change resolution"),
                T("Change Border Mode"),MB_OKCANCEL | MB_DEFBUTTON1 | MB_ICONEXCLAMATION)){
        proceed=false;
      }
    }
  }
  if (proceed) border_last_chosen=newborder;
  return proceed;
}
//---------------------------------------------------------------------------
void TOptionBox::ChangeOSDDisable(bool disable)
{
  osd_disable=disable;
  osd_init_run(0);
#ifdef WIN32
  if (Handle) if (GetDlgItem(Handle,12030)) SendMessage(GetDlgItem(Handle,12030),BM_SETCHECK,osd_disable,0);
  CheckMenuItem(StemWin_SysMenu,113,MF_BYCOMMAND | int(osd_disable ? MF_CHECKED:MF_UNCHECKED));
#endif
  UNIX_ONLY( osd_disable_but.set_check(osd_disable); )
  draw(true);
  CheckResetDisplay();
}
//---------------------------------------------------------------------------
void TOptionBox::SetRecord(bool On)
{
  if (On==0 && sound_record){
    sound_record_close_file();
    sound_record=false;
  }else if (On && sound_record==0){
    WIN_ONLY( if (Handle) if (GetDlgItem(Handle,7201)) SendDlgItemMessage(Handle,7201,BM_SETCHECK,true,0); )
    UNIX_ONLY( record_but.set_check(true); )
    int Ret=IDYES;
    if (RecordWarnOverwrite){
      if (Exists(WAVOutputFile)){
        Ret=Alert(WAVOutputFile+"\n\n"+T("This file already exists, would you like to overwrite it?"),
                        T("Record Over?"),MB_ICONQUESTION | MB_YESNO);
      }
    }
    if (Ret==IDYES){
      timer=timeGetTime();
      sound_record_start_time=timer+100; //start recorfing in 100ms' time
      sound_record=true;
      sound_record_open_file();
    }
  }
  WIN_ONLY( if (Handle) if (GetDlgItem(Handle,7201)) SendDlgItemMessage(Handle,7201,BM_SETCHECK,sound_record,0); )
  UNIX_ONLY( record_but.set_check(sound_record); )
}
//---------------------------------------------------------------------------
void TOptionBox::SoundModeChange(int new_mode,bool ChangeLast,bool UpdateCB)
{
  Sound_Stop(true);

  sound_mode=new_mode;
  if (ChangeLast && sound_mode!=SOUND_MODE_MUTE) sound_last_mode=sound_mode;
  if (Handle){
#ifdef WIN32
    if (UpdateCB && GetDlgItem(Handle,7099)) SendDlgItemMessage(Handle,7099,CB_SETCURSEL,sound_mode,0);
    for (int n=7100;n<7110;n++) if (GetDlgItem(Handle,n)) EnableWindow(GetDlgItem(Handle,n),sound_mode);
    for (int n=7050;n<7062;n++) if (GetDlgItem(Handle,n)) EnableWindow(GetDlgItem(Handle,n),sound_mode);
    for (int n=7200;n<7210;n++) if (GetDlgItem(Handle,n)) EnableWindow(GetDlgItem(Handle,n),sound_mode);
#else
		sound_mode_dd.changesel(sound_mode);
		sound_mode_dd.draw();
#endif
  }

  Sound_Start();
}
//---------------------------------------------------------------------------
void TOptionBox::UpdateRecordBut()
{
  WIN_ONLY( if (Handle) if (GetDlgItem(Handle,7201)) SendDlgItemMessage(Handle,7201,BM_SETCHECK,sound_record,0); )
	UNIX_ONLY( record_but.set_check(sound_record); )
}
//---------------------------------------------------------------------------
void TOptionBox::UpdateSoundFreq()
{
  Sound_Stop(true);

  WIN_ONLY( if (Handle) if (GetDlgItem(Handle,7101)) CBSelectItemWithData(GetDlgItem(Handle,7101),sound_chosen_freq); )
  Sound_Start();

#ifdef ENABLE_VARIABLE_SOUND_DAMPING
  if (sound_low_quality){
    sound_variable_a=0x60;
    sound_variable_d=0xC0;
  }else{
    sound_variable_a=0x20;
    sound_variable_d=0xd0;
  }
  mr_static_update_all();
#endif
}
//---------------------------------------------------------------------------
void TOptionBox::ChangeSoundFormat(BYTE bits,BYTE channels)
{
  Sound_Stop(true);
  sound_num_bits=bits;
  sound_num_channels=channels;
  sound_bytes_per_sample=(sound_num_bits/8)*sound_num_channels;
  Sound_Start();
}
//---------------------------------------------------------------------------
Str TOptionBox::CreateMacroFile(bool Edit)
{
  Str Path="";
#ifdef WIN32
  if (Handle){
    if (GetDlgItem(Handle,10000)){
      HTREEITEM Item=DTree.NewItem(T("New Macro"),DTree.RootItem,1,Edit);
      if (Item) return DTree.GetItemPath(Item);
      return "";
    }
  }
  Path=GetUniquePath(MacroDir,T("New Macro")+".stmac");
#elif defined(UNIX)
  EasyStr name=T("New Macro");
  if (Edit){
    hxc_prompt prompt;
    name=prompt.ask(XD,name,T("Enter Name"));
    if (name.Empty()) return "";
  }
  
  // Put in current folder
  EasyStr fol=MacroSel;
  RemoveFileNameFromPath(fol,REMOVE_SLASH);
  if (fol.Empty()) fol=MacroDir;
  Path=GetUniquePath(fol,name+".stmac");
#endif

  FILE *f=fopen(Path,"wb");
  if (f==NULL) return "";
  fclose(f);

#ifdef UNIX
  if (dir_lv.lv.handle) dir_lv.refresh_fol();
#endif

  return Path;
}
//---------------------------------------------------------------------------
int TOptionBox::GetCurrentMonitorSel()
{
  int monitor_sel=bool(MONO);
#ifndef NO_CRAZY_MONITOR
  if (extended_monitor){
    monitor_sel=2;
    for (int n=0;n<EXTMON_RESOLUTIONS;n++){
      if (em_width==extmon_res[n][0] && em_height==extmon_res[n][1] && em_planes==extmon_res[n][2]){
        monitor_sel=n+2;
      }
    }
  }
#endif
  return monitor_sel;
}
//---------------------------------------------------------------------------
int TOptionBox::TOSLangToFlagIdx(int Lang)
{
  switch (Lang){
    case 7: return 0;  //UK
    case 5: return 2;  //French
    case 0: return 1;  //US
    case 9: return 3;  //Spanish
    case 3: return 4;  //German
    case 11: return 5; //Italian
    case 13: return 6; //Swedish
    case 17: return 7; //Swiss German
  }
  return -1;
}
//---------------------------------------------------------------------------
void TOptionBox::TOSRefreshBox(EasyStr Sel)
{
#ifdef WIN32
  HWND Win=GetDlgItem(Handle,8300);
  if (Win==NULL) return;

  EnumDateFormats(EnumDateFormatsProc,LOCALE_USER_DEFAULT,DATE_SHORTDATE);

  SendMessage(Win,LB_RESETCONTENT,0,0);
  UpdateWindow(Win);
  SendMessage(Win,WM_SETREDRAW,0,0);
#elif defined(UNIX)
  if (tos_lv.handle==0) return;

  tos_lv.sl.DeleteAll(); //clear out the box
	tos_lv.display_mode=1;
	tos_lv.sl.Sort=eslNoSort;	
	tos_lv.lpig=&IcoTOSFlags;

	tos_lv.columns.DeleteAll();	
	tos_lv.columns.Add(5+8+5+hxc::get_text_width(XD,"8.88")+15);	
	tos_lv.columns.Add(page_w-hxc::get_text_width(XD,"12/12/2000")-15);

	EasyStringList eslTOS;
	eslTOS.Sort2=eslSortByData0;

	char LinkPath[MAX_PATH+1];
#endif

  EasyStr Fol=RunDir;
  EasyStr VersionPath; // The first TOS found which matches the current TOS version

  eslTOS.DeleteAll();
  eslTOS.Sort=eslTOS_Sort;

  if (Sel.Empty()){
    if (NewROMFile.Empty()){
      Sel=ROMFile;
    }else{
      Sel=NewROMFile;
    }
  }

 	DirSearch ds;
  if (ds.Find(Fol+SLASH+"*.*")){
    EasyStr Path;
    do{
      if ((ds.Attrib & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN))==0){
        Path=Fol+SLASH+ds.Name;
#ifdef WIN32
        if (has_extension(Path,"LNK")){
          WIN32_FIND_DATA wfd;
          EasyStr DestPath=GetLinkDest(Path,&wfd);
          if (has_extension_list(DestPath,"IMG","ROM",NULL)){
            if (Exists(DestPath)) Path=DestPath;
          }
        }
#elif defined(UNIX)
        memset(LinkPath,0,MAX_PATH+1);
        if (readlink(Path,LinkPath,MAX_PATH)>0){
          if (has_extension_list(LinkPath,"IMG","ROM",NULL)){
            if (Exists(LinkPath)){
              Path=LinkPath;
            }else{
              Path="";
            }
          }
        }
#endif
        if (has_extension_list(Path,"IMG","ROM",NULL)){
          WORD Ver,Date;
          BYTE Country;
          FILE *f=fopen(Path,"rb");
          if (f){
            fseek(f,2,SEEK_SET);
            BYTE b_high,b_low;
            fread(&b_high,1,1,f);fread(&b_low,1,1,f);
            Ver=MAKEWORD(b_low,b_high);

            fseek(f,0x1d,SEEK_SET);
            fread(&Country,1,1,f);

            fseek(f,0x1e,SEEK_SET);
            fread(&b_high,1,1,f);fread(&b_low,1,1,f);
            Date=MAKEWORD(b_low,b_high);

            fclose(f);

            eslTOS.Add(3,Str(GetFileNameFromPath(Path))+"\01"+Path,
                            Ver,Country,Date);
            if (Ver==tos_version && VersionPath.Empty()) VersionPath=Path;
          }
        }
      }
    }while (ds.Next());
    ds.Close();
  }

  int Selected=-1,VersionSel=-1,ROMFileSel=-1;

	int i=0,dir=1;
	if (eslTOS_Descend){
		i=eslTOS.NumStrings-1;
		dir=-1;
	}
  for (int idx=0;idx<eslTOS.NumStrings;idx++){
		char *FullPath=strrchr(eslTOS[i].String,'\01')+1;
    WIN_ONLY( SendMessage(Win,LB_INSERTSTRING,idx,LPARAM("")); )
#ifdef UNIX
		Str t;
    if (eslTOS[i].Data[0]) t=HEXSl(eslTOS[i].Data[0],3).Insert(".",1);
    t+="\01";
		t+=Str(GetFileNameFromPath(eslTOS[i].String))+"\01";
    if (eslTOS[i].Data[0]){
  		t+=Str(eslTOS[i].Data[2] & 0x1f)+"/";
  		t+=Str((eslTOS[i].Data[2] >> 5) & 0xf)+"/";
  		t+=Str((eslTOS[i].Data[2] >> 9)+1980);
    }
    t+="\01";
		t+=FullPath;
		tos_lv.sl.Add(t,101+TOSLangToFlagIdx(eslTOS[i].Data[1]));
#endif
		
    if (IsSameStr_I(FullPath,Sel)) Selected=idx;
    if (IsSameStr_I(FullPath,ROMFile)) ROMFileSel=idx;
    if (IsSameStr_I(FullPath,VersionPath)) VersionSel=idx;
    i+=dir;
  }

  static bool Recursing=0;
  if (Selected<0 && ROMFileSel<0 && Exists(ROMFile)){
    if (Recursing==0){
#ifdef WIN32
      EasyStr LinkName=WriteDir+"\\"+GetFileNameFromPath(ROMFile)+".lnk";
      int n=2;
      while (Exists(LinkName)){
        LinkName=WriteDir+"\\"+GetFileNameFromPath(ROMFile)+" ("+(n++)+")"+".lnk";
      }
      CreateLink(LinkName,ROMFile,T("TOS Image"));
#elif defined(UNIX)
			Str Name=GetFileNameFromPath(ROMFile),Ext;
			char *dot=strrchr(Name,'.');
			if (dot){
				Ext=dot;
				*dot=0;
			}
      EasyStr LinkName=WriteDir+SLASH+Name+Ext;
      int n=2;
      while (Exists(LinkName)){
        LinkName=WriteDir+SLASH+Name+"("+(n++)+")"+Ext;
      }
      symlink(ROMFile,LinkName);
#endif

      Recursing=true;
      TOSRefreshBox(ROMFile);
      Recursing=0;
    }
  }else{
    int iSel=Selected;
    if (iSel<0) iSel=VersionSel;
    if (iSel<0) iSel=max(ROMFileSel,0);
#ifdef WIN32
    SendMessage(Win,LB_SETCURSEL,iSel,0);
    SendMessage(Win,LB_SETCARETINDEX,iSel,0);
#elif defined(UNIX)
	  tos_lv.contents_change();
	  tos_lv.changesel(iSel);
#endif
  }
  WIN_ONLY( SendMessage(Win,WM_SETREDRAW,1,0); )
}
//---------------------------------------------------------------------------
void TOptionBox::LoadProfile(char *File)
{
  GoodConfigStoreFile CSF(File);

  int nSects=0;
  while (ProfileSection[nSects].Name) nSects++;

  bool *DisableSections=new bool[nSects];
  for (int i=0;i<nSects;i++){
    DisableSections[ProfileSection[i].ID]=(CSF.GetInt("ProfileSections",
                    ProfileSection[i].Name,LVI_SI_CHECKED)==LVI_SI_UNCHECKED);
  }

  LoadAllDialogData(0,File,DisableSections,&CSF);
  delete[] DisableSections;

  // Get current settings
  BYTE CurMemConf[2];
  GetCurrentMemConf(CurMemConf);
  int CurMonSel=GetCurrentMonitorSel();

  Str ProfileROM=CSF.GetStr("Machine","ROM_File",ROMFile);
  BYTE ProfileMemConf[2]={(BYTE)CSF.GetInt("Machine","Mem_Bank_1",CurMemConf[0]),
                          (BYTE)CSF.GetInt("Machine","Mem_Bank_2",CurMemConf[1])};
  int ProfileMonSel=!bool(CSF.GetInt("Machine","Colour_Monitor",mfp_gpip_no_interrupt & MFP_GPIP_COLOUR));
#ifndef NO_CRAZY_MONITOR
  if (CSF.GetInt("Machine","ExMon",extended_monitor)){
    int pro_em_width=CSF.GetInt("Machine","ExMonWidth",em_width);
    int pro_em_height=CSF.GetInt("Machine","ExMonHeight",em_height);
    int pro_em_planes=CSF.GetInt("Machine","ExMonPlanes",em_planes);
    ProfileMonSel=2;
    for (int n=0;n<EXTMON_RESOLUTIONS;n++){
      if (pro_em_width==extmon_res[n][0] && pro_em_height==extmon_res[n][1] &&
            pro_em_planes==extmon_res[n][2]){
        ProfileMonSel=n+2;
      }
    }
  }
#endif

  if (NewROMFile.Empty()) if (NotSameStr_I(ROMFile,ProfileROM)) NewROMFile=ProfileROM;
  if (NewMemConf0==-1){
    if (ProfileMemConf[0]!=CurMemConf[0] || ProfileMemConf[1]!=CurMemConf[1]){
      NewMemConf0=ProfileMemConf[0];
      NewMemConf1=ProfileMemConf[1];
    }
  }
  if (NewMonitorSel==-1) if (ProfileMonSel!=CurMonSel) NewMonitorSel=ProfileMonSel;

  // If profile was saved with settings pending, check they still need to pend
  if (IsSameStr_I(NewROMFile,ROMFile)) NewROMFile="";
  if (NewMemConf0==CurMemConf[0] && NewMemConf1==CurMemConf[1]) NewMemConf0=-1;
  if (NewMonitorSel==CurMonSel) NewMonitorSel=-1;

  CSF.Close();

  if (Handle) SetForegroundWindow(Handle);
  CheckResetIcon();
  CheckResetDisplay();
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifdef WIN32
//---------------------------------------------------------------------------
void TOptionBox::UpdateMacroRecordAndPlay(Str SelPath,int Type)
{
  if (Handle==NULL) return;
  if (GetDlgItem(Handle,10000)==NULL) return;

  if (SelPath.Empty()){
    HTREEITEM SelItem=(HTREEITEM)SendMessage(DTree.hTree,TVM_GETNEXTITEM,TVGN_CARET,0);
    SelPath=DTree.GetItemPath(SelItem);
    Type=DTree.GetItem(SelItem,TVIF_IMAGE).iImage;
  }
  bool CheckRec=0,CheckPlay=0;
  if (Type==1){
    if (macro_record && IsSameStr_I(macro_record_file,SelPath)) CheckRec=true;
    if (macro_play && IsSameStr_I(macro_play_file,SelPath)) CheckPlay=true;
  }
  SendDlgItemMessage(Handle,10011,BM_SETCHECK,CheckRec,0);
  SendDlgItemMessage(Handle,10012,BM_SETCHECK,CheckPlay,0);

  MACROFILEOPTIONS MFO;
  macro_file_options(MACRO_FILE_GET,SelPath,&MFO);
  CBSelectItemWithData(GetDlgItem(Handle,10016),MFO.allow_same_vbls);
//  SendDlgItemMessage(Handle,10017,BM_SETCHECK,MFO.add_mouse_together,0);
  CBSelectItemWithData(GetDlgItem(Handle,10014),MFO.max_mouse_speed);
}
//---------------------------------------------------------------------------

#define OPTIONS_HEIGHT 395
//---------------------------------------------------------------------------
void TOptionBox::ChangeScreenShotFormat(int NewFormat,Str Ext)
{
  Disp.ScreenShotFormat=NewFormat;

  char *dot=strrchr(Ext,'.');
  if (dot){
    Ext=dot+1;
    dot=strrchr(Ext,')');
    if (dot) *dot=0;
  }
  Disp.ScreenShotExt=Ext.LowerCase();

  Disp.ScreenShotFormatOpts=0;
  Disp.ScreenShotCheckFreeImageLoad();
  FillScreenShotFormatOptsCombo();

  if (Handle){
    if (GetDlgItem(Handle,1051)) CBSelectItemWithData(GetDlgItem(Handle,1051),NewFormat);
  }
}
//---------------------------------------------------------------------------
void TOptionBox::ChangeScreenShotFormatOpts(int NewOpt)
{
  Disp.ScreenShotFormatOpts=NewOpt;
  Disp.ScreenShotCheckFreeImageLoad();

  if (Handle){
    if (GetDlgItem(Handle,1052)) CBSelectItemWithData(GetDlgItem(Handle,1052),NewOpt);
  }
}
//---------------------------------------------------------------------------
void TOptionBox::ChooseScreenShotFolder(HWND Win)
{
  EnableAllWindows(0,Win);

  EasyStr NewFol=ChooseFolder(HWND(FullScreen ? StemWin:Win),T("Pick a Folder"),ScreenShotFol);
  if (NewFol.NotEmpty()){
    NO_SLASH(NewFol);
    if (Handle) if (GetDlgItem(Handle,1021)) SendDlgItemMessage(Handle,1021,WM_SETTEXT,0,(long)(NewFol.Text));
    ScreenShotFol=NewFol;
  }

  SetForegroundWindow(Win);
  EnableAllWindows(true,Win);
}
//---------------------------------------------------------------------------
void TOptionBox::UpdateForDSError()
{
  if (Handle==NULL) return;

  for (int n=7099;n<7110;n++) if (GetDlgItem(Handle,n)) EnableWindow(GetDlgItem(Handle,n),0);
  for (int n=7049;n<7062;n++) if (GetDlgItem(Handle,n)) EnableWindow(GetDlgItem(Handle,n),0);
  for (int n=7200;n<7210;n++) if (GetDlgItem(Handle,n)) EnableWindow(GetDlgItem(Handle,n),0);
  if (GetDlgItem(Handle,7010)) SendDlgItemMessage(Handle,7010,WM_SETTEXT,0,LPARAM((T("Current driver")+": None").Text));
}
//---------------------------------------------------------------------------
#include "options_create.cpp"
//---------------------------------------------------------------------------
TOptionBox::TOptionBox()
{
  page_l=150;page_w=320;

  Left=(GetSystemMetrics(SM_CXSCREEN)-(3+page_l+page_w+10+3))/2;
  Top=(GetSystemMetrics(SM_CYSCREEN)-(OPTIONS_HEIGHT+6+GetSystemMetrics(SM_CYCAPTION)))/2;

  FSLeft=(640-(3+page_l+page_w+10+3))/2;
  FSTop=(480-(OPTIONS_HEIGHT+6+GetSystemMetrics(SM_CYCAPTION)))/2;

  Section="Options";

  Page=9; // Machine
  hBrightBmp=NULL;
  BorderOption=NULL;
  RecordWarnOverwrite=true;
  il=NULL;
  NewMemConf0=-1,NewMemConf1=-1,NewMonitorSel=-1;

  eslTOS.Sort=eslSortByData0;
  eslTOS.Sort2=eslSortByData0;
  eslTOS_Sort=eslSortByData0;
  eslTOS_Descend=0;
}
//---------------------------------------------------------------------------
void TOptionBox::ManageWindowClasses(bool Unreg)
{
  char *ClassName="Steem Options";
  if (Unreg){
    UnregisterClass(ClassName,Inst);
  }else{
    RegisterMainClass(WndProc,ClassName,RC_ICO_OPTIONS);
  }
}
//---------------------------------------------------------------------------
void TOptionBox::LoadIcons()
{
  if (Handle==NULL) return;

  HIMAGELIST old_il=il;

  il=ImageList_Create(18,20,BPPToILC[BytesPerPixel] | ILC_MASK,10,10);
  if (il){
    ImageList_AddPaddedIcons(il,PAD_ALIGN_RIGHT,hGUIIcon[RC_ICO_OPS_GENERAL],
                          hGUIIcon[RC_ICO_OPS_DISPLAY],hGUIIcon[RC_ICO_OPS_BRIGHTCON],
                          hGUIIcon[RC_ICO_OPS_FULLSCREEN],hGUIIcon[RC_ICO_OPS_MIDI],
                          hGUIIcon[RC_ICO_OPS_SOUND],hGUIIcon[RC_ICO_OPS_STARTUP],hGUIIcon[RC_ICO_OPS_UPDATE],
                          hGUIIcon[RC_ICO_OPS_ASSOC],hGUIIcon[RC_ICO_OPS_MACHINE],hGUIIcon[RC_ICO_CHIP],
                          hGUIIcon[RC_ICO_OPS_PROFILES],hGUIIcon[RC_ICO_EXTERNAL],hGUIIcon[RC_ICO_OPS_MACROS],
                          hGUIIcon[RC_ICO_OPS_ICONS],hGUIIcon[RC_ICO_OPS_OSD],0);
  }
  if (GetDlgItem(Handle,60000)) SendMessage(PageTree,TVM_SETIMAGELIST,TVSIL_NORMAL,(LPARAM)il);
  if (old_il) ImageList_Destroy(old_il);

  if (GetDlgItem(Handle,7201)) SendDlgItemMessage(Handle,7201,BM_RELOADICON,0,0); // Record
  if (GetDlgItem(Handle,10011)) SendDlgItemMessage(Handle,10011,BM_RELOADICON,0,0); // Record macro
  if (GetDlgItem(Handle,10012)) SendDlgItemMessage(Handle,10012,BM_RELOADICON,0,0); // Play macro
  if (Scroller.GetControlPage()){
    for (int i=14100;i<14100+RC_NUM_ICONS;i++){
      if (GetDlgItem(Scroller.GetControlPage(),i)){
        SendDlgItemMessage(Scroller.GetControlPage(),i,BM_RELOADICON,0,0);
      }
    }
  }
  UpdateDirectoryTreeIcons(&DTree);

  CreateBrightnessBitmap();
}
//---------------------------------------------------------------------------
void TOptionBox::Show()
{
  if (Handle!=NULL){
    ShowWindow(Handle,SW_SHOWNORMAL);
    SetForegroundWindow(Handle);
    return;
  }
  if (FullScreen) Top=max(Top,MENUHEIGHT);

  ManageWindowClasses(SD_REGISTER);
  Handle=CreateWindowEx(WS_EX_CONTROLPARENT,"Steem Options",T("Options"),
                          WS_CAPTION | WS_SYSMENU,Left,Top,400,320,
                          ParentWin,NULL,Inst,NULL);
  if (HandleIsInvalid()){
    ManageWindowClasses(SD_UNREGISTER);
    return;
  }

  SetWindowLong(Handle,GWL_USERDATA,(long)this);

  MakeParent(HWND(FullScreen ? StemWin:NULL));

  LoadIcons();

  PageTree=CreateWindowEx(512,WC_TREEVIEW,"",WS_CHILD | WS_VISIBLE | WS_TABSTOP |
                        TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP,
                        0,0,100,OPTIONS_HEIGHT,Handle,(HMENU)60000,Inst,NULL);
  SendMessage(PageTree,TVM_SETIMAGELIST,TVSIL_NORMAL,(LPARAM)il);

  // Emulation options
  AddPageLabel(T("Machine"),9);
  AddPageLabel("TOS",10);
  AddPageLabel(T("Ports"),12);
  AddPageLabel(T("MIDI"),4);
  AddPageLabel(T("Macros"),13);

  // Configuration options
  AddPageLabel(T("General"),0);
  AddPageLabel(T("Sound"),5);
  AddPageLabel(T("Display"),1);
  AddPageLabel(T("On Screen Display"),15);
  AddPageLabel(T("Fullscreen Mode"),3);
  AddPageLabel(T("Brightness")+"/"+T("Contrast"),2);
  AddPageLabel(T("Profiles"),11);
  AddPageLabel(T("Startup"),6);
  AddPageLabel(T("Icons"),14);
  AddPageLabel(T("Auto Update"),7);
  AddPageLabel(T("File Associations"),8);

  page_l=min(2+TreeGetMaxItemWidth(PageTree)+5+2+10,630-(page_w+10));
  SetWindowPos(Handle,NULL,0,0,3+page_l+page_w+10+3,OPTIONS_HEIGHT+6+GetSystemMetrics(SM_CYCAPTION),SWP_NOZORDER | SWP_NOMOVE);
  SetWindowPos(PageTree,NULL,0,0,page_l-10,OPTIONS_HEIGHT,SWP_NOZORDER | SWP_NOMOVE);

  Focus=NULL;
  TreeSelectItemWithData(PageTree,Page);

  ShowWindow(Handle,SW_SHOW);
  SetFocus(Focus);

  if (StemWin!=NULL) PostMessage(StemWin,WM_USER,1234,0);
}
//---------------------------------------------------------------------------
void TOptionBox::EnableBorderOptions(bool enable)
{
  if (enable==0){
    border=0;
  }else{
    border=border_last_chosen | (border & 1);
  }
  CheckMenuRadioItem(StemWin_SysMenu,110,112,110+min(border,2),MF_BYCOMMAND);
  if (Handle==NULL || BorderOption==NULL) return;

  EnableWindow(BorderOption,enable);
  SendMessage(BorderOption,CB_SETCURSEL,min(border,2),0);
}
//---------------------------------------------------------------------------
void TOptionBox::DestroyCurrentPage()
{
  ToolsDeleteAllChildren(ToolTip,Handle);

  // Stop profiles saving out all check states when close
  if (GetDlgItem(Handle,11013)) EnableWindow(GetDlgItem(Handle,11013),0);

  TStemDialog::DestroyCurrentPage();

  BorderOption=NULL;
  if (hBrightBmp) DeleteObject(hBrightBmp);
  hBrightBmp=NULL;
}
//---------------------------------------------------------------------------
void TOptionBox::Hide()
{
  if (Handle==NULL) return;

  ShowWindow(Handle,SW_HIDE);
  if (FullScreen) SetFocus(StemWin);

  DestroyCurrentPage();
  DestroyWindow(Handle);Handle=NULL;

  ImageList_Destroy(il);il=NULL;

  if (StemWin) PostMessage(StemWin,WM_USER,1234,0);
  ManageWindowClasses(SD_UNREGISTER);
}
//---------------------------------------------------------------------------
bool TOptionBox::HasHandledMessage(MSG *mess)
{
  if (Handle!=NULL){
    if (mess->message==WM_KEYDOWN){
      if (mess->wParam==VK_TAB){
        return IsDialogMessage(Handle,mess);
      }
    }
    return 0;
  }else{
    return 0;
  }
}
//---------------------------------------------------------------------------
void TOptionBox::SetBorder(int newborder)
{
  int oldborder=border;
  if (ChangeBorderModeRequest(newborder)){
    border=newborder;
    if (FullScreen) change_fullscreen_display_mode(true);
    change_window_size_for_border_change(oldborder,newborder);
    draw(false);
    InvalidateRect(StemWin,NULL,0);
    if (Handle) if (GetDlgItem(Handle,210)) EnableWindow(GetDlgItem(Handle,210),border==0 && draw_fs_blit_mode!=DFSM_LAPTOP);
  }else{
    if (Handle) if (GetDlgItem(Handle,207)) SendDlgItemMessage(Handle,207,CB_SETCURSEL,oldborder,0);
    border=oldborder;
  }
  CheckMenuRadioItem(StemWin_SysMenu,110,112,110+min(border,2),MF_BYCOMMAND);
}
//---------------------------------------------------------------------------
#define GET_THIS This=(TOptionBox*)GetWindowLong(Win,GWL_USERDATA);

LRESULT __stdcall TOptionBox::WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  if (DTree.ProcessMessage(Mess,wPar,lPar)) return DTree.WndProcRet;
  
  LRESULT Ret=DefStemDialogProc(Win,Mess,wPar,lPar);
  if (StemDialog_RetDefVal) return Ret;

  TOptionBox *This;
  switch (Mess){
    case WM_COMMAND:
      GET_THIS;
      switch (LOWORD(wPar)){
        case 404:
          if (HIWORD(wPar)==CBN_SELENDOK){
            n_cpu_cycles_per_second=max(min(SendMessage(HWND(lPar),CB_GETITEMDATA,SendMessage(HWND(lPar),CB_GETCURSEL,0,0),0),128000000l),8000000l);
            if (runstate==RUNSTATE_RUNNING) osd_init_run(0);
            prepare_cpu_boosted_event_plans();
          }
          break;
        case 700:
          if (HIWORD(wPar)==BN_CLICKED){
            AllowTaskSwitch=!AllowTaskSwitch;
            SendMessage(HWND(lPar),BM_SETCHECK,(AllowTaskSwitch==0),0);
          }
          break;
        case 800:
          if (HIWORD(wPar)==BN_CLICKED){
            PauseWhenInactive=!PauseWhenInactive;
            SendMessage(HWND(lPar),BM_SETCHECK,PauseWhenInactive,0);
          }
          break;
        case 900:
          if (HIWORD(wPar)==BN_CLICKED){
            floppy_access_ff=!floppy_access_ff;
            SendMessage(HWND(lPar),BM_SETCHECK,floppy_access_ff,0);
          }
          break;
        case 901:
          if (HIWORD(wPar)==BN_CLICKED){
            StartEmuOnClick=!StartEmuOnClick;
            SendMessage(HWND(lPar),BM_SETCHECK,StartEmuOnClick,0);
          }
          break;
        case 201:
          if (HIWORD(wPar)==CBN_SELENDOK){
            frameskip=SendMessage(HWND(lPar),CB_GETCURSEL,0,0)+1;
            if(frameskip==5)frameskip=AUTO_FRAMESKIP;
          }
          break;
        case 203:
          if (HIWORD(wPar)==BN_CLICKED){
//            This->ChangeOSD(!osd_on);
          }
          break;
        case 204:
          if (HIWORD(wPar)==CBN_SELENDOK){
            int proceed=1,new_mode=SendMessage(HWND(lPar),CB_GETCURSEL,0,0);  //carry on, don't change res
            if (new_mode!=draw_fs_blit_mode){
              if (FullScreen){
                if (new_mode==DFSM_LAPTOP){
                  if (GetScreenWidth()!=monitor_width || GetScreenHeight()!=monitor_height) proceed=2;
                }else if (draw_fs_blit_mode==DFSM_LAPTOP){
                  if (border & 1){
                    if (monitor_width!=800 || monitor_height!=600) proceed=2;
                  }else{
                    if (monitor_width!=640 || monitor_height!=480) proceed=2;
                  }
                }
                if (proceed==2){
                  if (IDCANCEL==Alert(T("This will cause the monitor to change resolution"),
                          T("Change Fullscreen Mode"),MB_OKCANCEL | MB_DEFBUTTON1 | MB_ICONEXCLAMATION)){
                    proceed=0;
                    SendMessage(HWND(lPar),CB_SETCURSEL,draw_fs_blit_mode,0);
                  }
                }
              }
            }
            if (proceed){
              draw_fs_blit_mode=new_mode;
              if (proceed==2) change_fullscreen_display_mode(true);
              EnableWindow(GetDlgItem(Win,280),(draw_fs_blit_mode<DFSM_STRETCHBLIT));
              EnableWindow(GetDlgItem(Win,281),(draw_fs_blit_mode<DFSM_STRETCHBLIT));
              if (draw_fs_blit_mode==DFSM_LAPTOP){
                EnableWindow(GetDlgItem(Win,210),0);
              }else{
                EnableWindow(GetDlgItem(Win,210),border==0);
              }
              if (draw_grille_black<4) draw_grille_black=4;
            }
          }
          break;
        case 206:
          if (HIWORD(wPar)==BN_CLICKED){
            FSDoVsync=!FSDoVsync;
            SendMessage(HWND(lPar),BM_SETCHECK,FSDoVsync,0);
          }
          break;
        case 207:
          if (HIWORD(wPar)==CBN_SELENDOK){
            This->SetBorder(SendMessage(HWND(lPar),CB_GETCURSEL,0,0));
            if (draw_grille_black<4) draw_grille_black=4;
          }
          break;
        case 208:
          if (HIWORD(wPar)==BN_CLICKED){
            bool proceed=true;
            if (FullScreen){
              if (IDCANCEL==Alert(T("This will cause the monitor to change resolution"),
                        T("Change Colour Depth"),MB_OKCANCEL | MB_DEFBUTTON1 | MB_ICONEXCLAMATION)){
                proceed=false;
              }
            }
            if (proceed){
              display_option_8_bit_fs=!display_option_8_bit_fs;
              SendMessage(HWND(lPar),BM_SETCHECK,display_option_8_bit_fs,0);
              if (FullScreen){
                change_fullscreen_display_mode(false);
                palette_convert_all();
                draw(false);
                InvalidateRect(StemWin,NULL,0);
              }
              This->UpdateHzDisplay();
            }
          }
          break;
        case 210:
          if (HIWORD(wPar)==BN_CLICKED){
            prefer_res_640_400=!prefer_res_640_400;
            SendMessage(HWND(lPar),BM_SETCHECK,prefer_res_640_400,0);
          }
          break;

        case 220:case 222:case 224:
          if (HIWORD(wPar)==CBN_SELENDOK){
            int i=(LOWORD(wPar)-220)/2;
            int c256=int(display_option_8_bit_fs ? 0:1);
            int new_hz=HzIdxToHz[SendMessage(HWND(lPar),CB_GETCURSEL,0,0)];

            if (prefer_pc_hz[c256][i]!=new_hz){
              prefer_pc_hz[c256][i]=new_hz;

              int current_i=int((border & 1) ? 2:1);
              if (FullScreen && current_i==i){
                if (IDYES==Alert(T("Do you want to test this video frequency now?"),
                                  T("Change Monitor Frequency"),MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION)){
                  change_fullscreen_display_mode(false);
                  palette_convert_all();
                  draw(false);
                  InvalidateRect(StemWin,NULL,0);
                }
              }
              This->UpdateHzDisplay();
            }
          }
          break;

        case 226:
          if (HIWORD(wPar)==BN_CLICKED){
            FSQuitAskFirst=!FSQuitAskFirst;
            SendMessage(HWND(lPar),BM_SETCHECK,FSQuitAskFirst,0);
          }
          break;

        case 280:
          if (HIWORD(wPar)==BN_CLICKED){
            draw_fs_fx=(SendMessage(HWND(lPar),BM_GETCHECK,0,0)==BST_CHECKED ? DFSFX_GRILLE:DFSFX_NONE);
            if (draw_grille_black<4) draw_grille_black=4;
            if (runstate!=RUNSTATE_RUNNING && FullScreen) draw(false);
          }
          break;
        case 300:
          if (HIWORD(wPar)==BN_CLICKED){
            ResChangeResize=!ResChangeResize;
            SendMessage(HWND(lPar),BM_SETCHECK,ResChangeResize,0);
          }
          break;
        case 302:case 304:case 306:
          if (HIWORD(wPar)==CBN_SELENDOK){
            int Res=(LOWORD(wPar)-302)/2;
            bool redraw=0;

            DWORD dat=CBGetSelectedItemData(HWND(lPar));
            WinSizeForRes[Res]=LOWORD(dat);
            if (Res<2){
              if (draw_win_mode[Res]!=HIWORD(dat)){
                draw_win_mode[Res]=HIWORD(dat);
                redraw=true;
              }
            }
            if (Res==int(mixed_output ? 1:screen_res)){
              if (redraw && FullScreen==0){
                if (draw_grille_black<4) draw_grille_black=4;
                draw(false);
              }
              if (ResChangeResize) StemWinResize();
            }
          }
          break;
        case 400:
          if (HIWORD(wPar)==BN_CLICKED){
            ShowTips=!ShowTips;
            SendMessage(HWND(lPar),BM_SETCHECK,ShowTips,0);
            SendMessage(ToolTip,TTM_ACTIVATE,ShowTips,0);
          }
          break;
        case 1022:
          if (HIWORD(wPar)==BN_CLICKED){
            SendMessage(HWND(lPar),BM_SETCHECK,1,true);
            This->ChooseScreenShotFolder(Win);
            SetFocus(HWND(lPar));
            SendMessage(HWND(lPar),BM_SETCHECK,0,true);
          }
          break;
        case 1023:
          if (HIWORD(wPar)==BN_CLICKED) ShellExecute(NULL,NULL,ScreenShotFol,"","",SW_SHOWNORMAL);
          break;
        case 1024:
          if (HIWORD(wPar)==BN_CLICKED){
            Disp.ScreenShotMinSize=!Disp.ScreenShotMinSize;
            SendMessage(HWND(lPar),BM_SETCHECK,Disp.ScreenShotMinSize,0);
          }
          break;
        case 1051:
          if (HIWORD(wPar)==CBN_SELENDOK){
            Str Ext;
            Ext.SetLength(200);
            SendMessage(HWND(lPar),CB_GETLBTEXT,SendMessage(HWND(lPar),CB_GETCURSEL,0,0),LPARAM(Ext.Text));
            This->ChangeScreenShotFormat(CBGetSelectedItemData(HWND(lPar)),Ext);
          }
          break;
        case 1052:
          if (HIWORD(wPar)==CBN_SELENDOK){
            This->ChangeScreenShotFormatOpts(CBGetSelectedItemData(HWND(lPar)));
          }
          break;

        case 1030:
          if (HIWORD(wPar)==BN_CLICKED){
            HighPriority=!HighPriority;
            SendMessage(HWND(lPar),BM_SETCHECK,HighPriority,0);
            if (runstate==RUNSTATE_RUNNING){
              SetPriorityClass(GetCurrentProcess(),(HighPriority ? HIGH_PRIORITY_CLASS:NORMAL_PRIORITY_CLASS));
            }
          }
          break;

        case 3001:
          if (HIWORD(wPar)==CBN_SELENDOK){
            int CurSel=SendDlgItemMessage(Win,3001,CB_GETCURSEL,0,0);
            if (CurSel==0){
              WriteCSFStr("Options","DSDriverName","",INIFile);
            }else if (CurSel>0){
              EasyStr DrivName;
              DrivName.SetLength(SendDlgItemMessage(Win,3001,CB_GETLBTEXTLEN,CurSel,0));
              SendDlgItemMessage(Win,3001,CB_GETLBTEXT,CurSel,LPARAM(DrivName.Text));
              WriteCSFStr("Options","DSDriverName",DrivName,INIFile);
            }
          }
          break;
        case 3303:
          if (HIWORD(wPar)==BN_CLICKED){
            AutoLoadSnapShot=!AutoLoadSnapShot;
            SendMessage(HWND(lPar),BM_SETCHECK,AutoLoadSnapShot,0);
          }
          break;
        case 3311:
          if (HIWORD(wPar)==EN_UPDATE){
            EasyStr NewName;
            int Len=SendMessage(HWND(lPar),WM_GETTEXTLENGTH,0,0)+1;
            AutoSnapShotName.SetLength(Len);
            SendMessage(HWND(lPar),WM_GETTEXT,Len,LPARAM(AutoSnapShotName.Text));

            bool SetText=0;
            for (int i=0;i<AutoSnapShotName.Length();i++){
              switch (AutoSnapShotName[i]){
                case ':':case '/':
                case '"':case '<':
                case '>':case '|':
                case '*':case '?':
                case '\\':
                  AutoSnapShotName[i]='-';
                  SetText=true;
                  break;
              }
            }
            if (SetText){
              DWORD Start,End;
              SendMessage(HWND(lPar),EM_GETSEL,WPARAM(&Start),LPARAM(&End));
              SendMessage(HWND(lPar),WM_SETTEXT,0,LPARAM(AutoSnapShotName.Text));
              SendMessage(HWND(lPar),EM_SETSEL,Start,End);
            }
          }
          break;
        case 3300:case 3301:case 3302:case 3304:case 3305:
          if (HIWORD(wPar)==BN_CLICKED){
            char *key="NoDirectDraw";
            if (LOWORD(wPar)==3301) key="NoDirectSound";
            if (LOWORD(wPar)==3302) key="StartFullscreen";
            if (LOWORD(wPar)==3304) key="DrawToVidMem";
            if (LOWORD(wPar)==3305) key="BlitHideMouse";
            WriteCSFStr("Options",key,EasyStr(SendMessage(HWND(lPar),BM_GETCHECK,0,0)),INIFile);
            if (LOWORD(wPar)==3300){
              EnableWindow(GetDlgItem(Win,3302),!SendMessage(HWND(lPar),BM_GETCHECK,0,0));
              EnableWindow(GetDlgItem(Win,3304),!SendMessage(HWND(lPar),BM_GETCHECK,0,0));
              EnableWindow(GetDlgItem(Win,3305),!SendMessage(HWND(lPar),BM_GETCHECK,0,0));
            }
          }
          break;

        case 4200:case 4201:case 4202:case 4203:
          if (HIWORD(wPar)==BN_CLICKED){
            ConfigStoreFile CSF(INIFile);
            CSF.SetStr("Update","AutoUpdateEnabled",
                     LPSTR(SendMessage(GetDlgItem(Win,4200),BM_GETCHECK,0,0)==0 ? "1":"0"));
            CSF.SetStr("Update","AlwaysOnline",
                     LPSTR(SendMessage(GetDlgItem(Win,4201),BM_GETCHECK,0,0) ? "1":"0"));
            CSF.SetStr("Update","PatchDownload",
                     LPSTR(SendMessage(GetDlgItem(Win,4202),BM_GETCHECK,0,0) ? "1":"0"));
            CSF.SetStr("Update","AskPatchInstall",
                     LPSTR(SendMessage(GetDlgItem(Win,4203),BM_GETCHECK,0,0) ? "1":"0"));
            CSF.Close();
          }
          break;
        case 4400:
          if (HIWORD(wPar)==BN_CLICKED){
            EasyStr Online=LPSTR(SendMessage(GetDlgItem(Win,4201),BM_GETCHECK,0,0) ? " online":"");
            EasyStr NoPatch=LPSTR(SendMessage(GetDlgItem(Win,4202),BM_GETCHECK,0,0)==0 ? " nopatchcheck":"");
            EasyStr AskPatch=LPSTR(SendMessage(GetDlgItem(Win,4203),BM_GETCHECK,0,0) ? " askpatchinstall":"");
            WinExec(EasyStr("\"")+RunDir+"\\SteemUpdate.exe\""+Online+NoPatch+AskPatch,SW_SHOW);
          }
          break;


        case 5100:case 5101:case 5102:case 5103:case 5104:case 5105:case 5106:
          if (HIWORD(wPar)==BN_CLICKED){
            EasyStr Ext;
            switch (LOWORD(wPar)){
              case 5100: Ext=".ST";AssociateSteem(Ext,"st_disk_image",true,T("ST Disk Image"),DISK_ICON_NUM,0); break;
              case 5101: Ext=".STT";AssociateSteem(Ext,"st_disk_image",true,T("ST Disk Image"),DISK_ICON_NUM,0); break;
              case 5102: Ext=".MSA";AssociateSteem(Ext,"st_disk_image",true,T("ST Disk Image"),DISK_ICON_NUM,0); break;
              case 5103: Ext=".DIM";AssociateSteem(Ext,"st_disk_image",true,T("ST Disk Image"),DISK_ICON_NUM,0); break;
              case 5104: Ext=".STZ";AssociateSteem(Ext,"st_disk_image",true,T("ST Disk Image"),DISK_ICON_NUM,0); break;
              case 5105: Ext=".STS";AssociateSteem(Ext,"steem_memory_snapshot",true,T("Steem Memory Snapshot"),SNAP_ICON_NUM,0); break;
              case 5106: Ext=".STC";AssociateSteem(Ext,"st_cartridge",true,T("ST ROM Cartridge"),CART_ICON_NUM,0); break;
            }
            HWND But=HWND(lPar);
            if (IsSteemAssociated(Ext)){
              SendMessage(But,WM_SETTEXT,0,LPARAM(T("Associated").Text));
              EnableWindow(But,0);
            }else{
              SendMessage(But,WM_SETTEXT,0,LPARAM(T("Associate").Text));
              EnableWindow(But,true);
            }
          }
          break;
        case 5502:
        {
          if (HIWORD(wPar)!=BN_CLICKED) break;

          ConfigStoreFile CSF(INIFile);
          bool OpenFilesInNew=!CSF.GetInt("Options","OpenFilesInNew",true);
          CSF.SetInt("Options","OpenFilesInNew",OpenFilesInNew);
          CSF.Close();

          SendMessage(HWND(lPar),BM_SETCHECK,OpenFilesInNew,0);
          break;
        }

        case 6010:
          if (HIWORD(wPar)==BN_CLICKED){
            MIDI_out_running_status_flag=!MIDI_out_running_status_flag;
            SendMessage(HWND(lPar),BM_SETCHECK,int((MIDI_out_running_status_flag==MIDI_ALLOW_RUNNING_STATUS) ? BST_CHECKED:BST_UNCHECKED),0);
          }
          break;
        case 6011:
          if (HIWORD(wPar)==BN_CLICKED){
            MIDI_in_running_status_flag=!MIDI_in_running_status_flag;
            SendMessage(HWND(lPar),BM_SETCHECK,int((MIDI_in_running_status_flag==MIDI_ALLOW_RUNNING_STATUS) ? BST_CHECKED:BST_UNCHECKED),0);
          }
          break;
        case 6021:case 6023:
          if (HIWORD(wPar)==CBN_SELENDOK){
            MIDI_out_n_sysex=SendDlgItemMessage(Win,6021,CB_GETCURSEL,0,0)+2;
            MIDI_out_sysex_max=(16 << SendDlgItemMessage(Win,6023,CB_GETCURSEL,0,0))*1024;
            for (int i=0;i<3;i++){
              if (STPort[i].MIDI_Out) STPort[i].MIDI_Out->ReInitSysEx();
            }
          }
          break;
        case 6031:case 6033:
          if (HIWORD(wPar)==CBN_SELENDOK){
            MIDI_in_n_sysex=SendDlgItemMessage(Win,6031,CB_GETCURSEL,0,0)+2;
            MIDI_in_sysex_max=(16 << SendDlgItemMessage(Win,6033,CB_GETCURSEL,0,0))*1024;
            for (int i=0;i<3;i++){
              if (STPort[i].MIDI_In) STPort[i].MIDI_In->ReInitSysEx();
            }
          }
          break;

        case 2010:
          if (HIWORD(wPar)==BN_CLICKED) This->FullscreenBrightnessBitmap();
          break;

        case 7101:
          if (HIWORD(wPar)==CBN_SELENDOK){
            int freq=CBGetSelectedItemData(HWND(lPar));
            if (freq) sound_chosen_freq=freq;
            This->UpdateSoundFreq();
          }
          break;
        case 7061:
          if (HIWORD(wPar)==CBN_SELENDOK){
            WORD dat=(WORD)CBGetSelectedItemData(HWND(lPar));
          	This->ChangeSoundFormat(LOBYTE(dat),HIBYTE(dat));         	
          }
          break;
        case 7102:
          if (HIWORD(wPar)==BN_CLICKED){
            Sound_Stop(true);
            sound_write_primary=!sound_write_primary;
            SendMessage(HWND(lPar),BM_SETCHECK,sound_write_primary,true);
            if (runstate==RUNSTATE_RUNNING){
              Sleep(50);
              Sound_Start();
            }
          }
          break;
        case 7103:
        {
          if (HIWORD(wPar)==CBN_SELENDOK){
            int nstm=SendMessage(HWND(lPar),CB_GETCURSEL,0,0);
            if (nstm!=sound_time_method){
              Sound_Stop(true);
              sound_time_method=nstm;
              if (runstate==RUNSTATE_RUNNING){
                Sleep(50);
                Sound_Start();
              }
            }
          }
          break;
        }
        case 7104:
          if (HIWORD(wPar)==CBN_SELENDOK){
            psg_write_n_screens_ahead=SendMessage(HWND(lPar),CB_GETCURSEL,0,0);
          }
          break;
        case 7099:
          if (HIWORD(wPar)==CBN_SELENDOK){
            This->SoundModeChange(SendMessage(HWND(lPar),CB_GETCURSEL,0,0),true,0);
          }
          break;
        case 7201:
          if (HIWORD(wPar)==BN_CLICKED){
            This->SetRecord(!sound_record);
          }
          break;
        case 7203:
          if (HIWORD(wPar)==BN_CLICKED){
            SendMessage(HWND(lPar),BM_SETCHECK,1,true);
            EnableAllWindows(0,Win);
            EasyStr NewWAV=FileSelect(HWND(FullScreen ? StemWin:Win),T("Choose WAV Output File"),
                                        This->WAVOutputDir,FSTypes(1,T("WAV File").Text,"*.wav",NULL),1,0,"wav");
            if (NewWAV.NotEmpty()){
              WAVOutputFile=NewWAV;
              This->WAVOutputDir=NewWAV;
              RemoveFileNameFromPath(This->WAVOutputDir,REMOVE_SLASH);
              SendMessage(GetDlgItem(Win,7202),WM_SETTEXT,0,LPARAM(WAVOutputFile.Text));
            }
            SetForegroundWindow(Win);
            EnableAllWindows(true,Win);
            SendMessage(HWND(lPar),BM_SETCHECK,0,true);
            SetFocus(HWND(lPar));
          }
          break;
        case 7204:
          if (HIWORD(wPar)==BN_CLICKED){
            This->RecordWarnOverwrite=!This->RecordWarnOverwrite;
            SendMessage(HWND(lPar),BM_SETCHECK,This->RecordWarnOverwrite,0);
          }
          break;
        case 7300:
          if (HIWORD(wPar)==BN_CLICKED){
            if (sound_internal_speaker) SoundStopInternalSpeaker();

            sound_internal_speaker=!sound_internal_speaker;
            SendMessage(HWND(lPar),BM_SETCHECK,sound_internal_speaker,0);
          }
          break;

        case 8100: // Memory size
          if (HIWORD(wPar)==CBN_SELENDOK){
            DWORD Conf=CBGetSelectedItemData(HWND(lPar));
            This->NewMemConf0=LOWORD(Conf);This->NewMemConf1=HIWORD(Conf);
            if (bank_length[0]==mmu_bank_length_from_config[This->NewMemConf0] &&
                bank_length[1]==mmu_bank_length_from_config[This->NewMemConf1]){
              This->NewMemConf0=-1;
            }
            CheckResetIcon();
          }
          break;
        case 8200: // Monitor
          if (HIWORD(wPar)==CBN_SELENDOK){
            This->NewMonitorSel=SendMessage(HWND(lPar),CB_GETCURSEL,0,0);
            if (This->NewMonitorSel==This->GetCurrentMonitorSel()) This->NewMonitorSel=-1;
            CheckResetIcon();
          }
          break;
        case 8501: // Choose cart
          if (HIWORD(wPar)==BN_CLICKED){
            SendMessage(HWND(lPar),BM_SETCHECK,1,true);
            EnableAllWindows(0,Win);

            Str LastCartFol=This->LastCartFile;
            RemoveFileNameFromPath(LastCartFol,REMOVE_SLASH);
            EasyStr NewCart=FileSelect(HWND(FullScreen ? StemWin:Win),T("Find a Cartridge"),
                                        LastCartFol,FSTypes(0,T("ST Cartridge Images").Text,"*.stc",NULL),1,true,"stc",
                                        GetFileNameFromPath(This->LastCartFile));
            if (NewCart.NotEmpty()){
              SetWindowText(GetDlgItem(Win,8500),NewCart);
              EnableWindow(GetDlgItem(Win,8502),true);
              This->LastCartFile=NewCart;
              if (load_cart(NewCart)){
                Alert(T("There was an error loading the cartridge."),T("Cannot Load Cartridge"),MB_ICONEXCLAMATION);
                break;
              }else{
                CartFile=NewCart;
              }
              CheckResetDisplay();
            }

            SetForegroundWindow(Win);
            EnableAllWindows(true,Win);
            SetFocus(HWND(lPar));
            SendMessage(HWND(lPar),BM_SETCHECK,0,true);
          }
          break;
        case 8502: // Remove cart
          if (HIWORD(wPar)==BN_CLICKED){
            SetWindowText(GetDlgItem(Win,8500),"");
            SetFocus(GetDlgItem(Win,8501));
            EnableWindow(GetDlgItem(Win,8502),0);

            delete[] cart;
            cart=NULL;
            CartFile="";
            if (pc>=MEM_EXPANSION_CARTRIDGE && pc<0xfc0000){
            	SET_PC(PC32);
            }

            CheckResetDisplay();
          }
          break;
        case 8601: // Cold reset
          if (HIWORD(wPar)==BN_CLICKED) reset_st();
          break;
        case 8401: // Keyboard language
          if (HIWORD(wPar)==CBN_SELENDOK){
            KeyboardLangID=(LANGID)SendMessage(HWND(lPar),CB_GETITEMDATA,
                                            SendMessage(HWND(lPar),CB_GETCURSEL,0,0),0);
            InitKeyTable();
            EnableWindow(GetDlgItem(Win,8402),ShiftSwitchingAvailable);
          }
          break;
        case 8402: // Keyboard alt and shift correction
          if (HIWORD(wPar)==BN_CLICKED){
            EnableShiftSwitching=SendMessage(HWND(lPar),BM_GETCHECK,0,0);
            InitKeyTable();
          }
          break;

        case 8300: // TOS
          if (HIWORD(wPar)==LBN_SELCHANGE){
            int i=SendMessage(HWND(lPar),LB_GETCURSEL,0,0);
            if (i!=LB_ERR){
              int idx=i;
              if (This->eslTOS_Descend) idx=This->eslTOS.NumStrings-1 - i;
              This->NewROMFile=strchr(This->eslTOS[idx].String,'\01')+1;
              if (IsSameStr_I(ROMFile,This->NewROMFile)) This->NewROMFile="";
            }
            CheckResetIcon();
          }
          break;
        case 8301:
          if (HIWORD(wPar)==BN_CLICKED){
            SendMessage(HWND(lPar),BM_SETCHECK,1,true);
            EnableAllWindows(0,Win);

            OPENFILENAME ofn;
            ZeroMemory(&ofn,sizeof(OPENFILENAME));
            char *files=new char[MAX_PATH*80+1];
            ZeroMemory(files,MAX_PATH*80+1);
            ofn.lStructSize=sizeof(OPENFILENAME);
            ofn.hwndOwner=Win;
            ofn.hInstance=(HINSTANCE)GetModuleHandle(NULL);
            ofn.lpstrFilter=FSTypes(3,NULL);
            ofn.lpstrCustomFilter=NULL;
            ofn.nMaxCustFilter=0;
            ofn.nFilterIndex=1;
            ofn.lpstrFile=files;
            ofn.nMaxFile=MAX_PATH*80;
            ofn.lpstrFileTitle=NULL;
            ofn.nMaxFileTitle=0;
            ofn.lpstrInitialDir=This->TOSBrowseDir;
            ofn.lpstrTitle=StaticT("Select One or More TOS Images");
            ofn.Flags=OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |
                        OFN_ALLOWMULTISELECT | OFN_EXPLORER;
            ofn.lpstrDefExt="IMG";
            ofn.lpfnHook=0;
            ofn.lpTemplateName=0;

            if (GetOpenFileName(&ofn)){
              int nfiles=0;
              char *f=files;
              while (f[0]){
                f+=strlen(f)+1;
                nfiles++;
              }

              if (nfiles==1){
                This->TOSBrowseDir=files;
                RemoveFileNameFromPath(This->TOSBrowseDir,REMOVE_SLASH);
                files+=strlen(This->TOSBrowseDir)+1; // only want name
              }else{
                This->TOSBrowseDir=files;
                files+=strlen(files)+1; // skip to files
              }

              HWND TOSBox=GetDlgItem(Win,8300);
              Str file;

              char *cur_file=files;
              while (cur_file[0]){
                file=This->TOSBrowseDir+SLASH+cur_file;
                int c=SendMessage(TOSBox,LB_GETCOUNT,0,0);
                for (int i=0;i<c;i++){
                  if (IsSameStr_I(file,strrchr(This->eslTOS[i].String,'\01')+1)){
                    if (nfiles==1){
                      SendMessage(TOSBox,LB_SETCARETINDEX,i,0);
                      SendMessage(TOSBox,LB_SETCURSEL,i,0);
                    }
                    file=""; // skip this file
                    break;
                  }
                }
                if (file[0]){
                  WIN32_FIND_DATA wfd;
                  HANDLE hFind=FindFirstFile(file,&wfd);
                  if (hFind!=INVALID_HANDLE_VALUE){
                    FindClose(hFind);

                    if (get_TOS_address(file)){
                      int n=2;
                      EasyStr LinkFileName=WriteDir+"\\"+GetFileNameFromPath(file)+".lnk";
                      while (Exists(LinkFileName)){
                        LinkFileName=WriteDir+"\\"+GetFileNameFromPath(file)+" ("+(n++)+").lnk";
                      }
                      CreateLink(LinkFileName,file,T("TOS Image"));
                      if (nfiles==1) This->NewROMFile=file;
                    }else if (nfiles==1){
                      Alert(Str(file)+" "+T("is not a valid TOS image."),T("Cannot use TOS"),MB_ICONEXCLAMATION);
                    }
                  }
                }
                cur_file+=strlen(cur_file)+1;
              }
              This->TOSRefreshBox();
            }
            SetForegroundWindow(Win);
            EnableAllWindows(true,Win);
            SendMessage(HWND(lPar),BM_SETCHECK,0,true);
            SetFocus(HWND(lPar));
            CheckResetIcon();
          }
          break;
        case 8302:
          if (HIWORD(wPar)==BN_CLICKED){
            SendMessage(HWND(lPar),BM_SETCHECK,1,true);

            HWND TOSBox=GetDlgItem(Win,8300);
            int idx=SendMessage(TOSBox,LB_GETCURSEL,0,0);
            if (This->eslTOS_Descend) idx=This->eslTOS.NumStrings-1 - idx;
         		char *RemovePath=strrchr(This->eslTOS[idx].String,'\01')+1;

            int idx1=idx+int(This->eslTOS_Descend ? 1:-1);
            if (idx1<0) idx1=1;
            if (idx1>=This->eslTOS.NumStrings) idx1=This->eslTOS.NumStrings-2;
         		Str NewSel=strrchr(This->eslTOS[idx1].String,'\01')+1;

            DirSearch ds;
            EasyStringList dellist;
            if (ds.Find(RunDir+SLASH+"*.*")){
              Str Path,RealPath;
              do{
                if ((ds.Attrib & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN))==0){
                  Path=RunDir+SLASH+ds.Name;
                  if (has_extension(Path,"LNK")){
                    WIN32_FIND_DATA wfd;
                    RealPath=GetLinkDest(Path,&wfd);
                  }else{
                    RealPath=Path;
                  }
                  if (IsSameStr_I(RealPath,RemovePath)) dellist.Add(Path,IsSameStr_I(RealPath,Path));
                }
              }while (ds.Next());
              ds.Close();
            }
            for (int i=0;i<dellist.NumStrings;i++){
              if (dellist[i].Data[0]){ // Not link
                SetFileAttributes(dellist[i].String,GetFileAttributes(dellist[i].String) | FILE_ATTRIBUTE_HIDDEN);
              }else{
                DeleteFile(dellist[i].String);
              }
            }
            This->NewROMFile=NewSel;
            This->TOSRefreshBox();
            
            SendMessage(HWND(lPar),BM_SETCHECK,0,true);
          }
          break;
        case 8311:
          if (HIWORD(wPar)==CBN_SELENDOK){
            This->eslTOS_Sort=(ESLSortEnum)(signed short)LOWORD(CBGetSelectedItemData(HWND(lPar)));
            This->eslTOS_Descend=HIWORD(CBGetSelectedItemData(HWND(lPar)));
            This->TOSRefreshBox("");
          }
          break;
        case 10001:
          if (HIWORD(wPar)!=BN_CLICKED) break;
          DTree.NewItem(T("New Macro"),DTree.RootItem,1);
          break;
        case 10002:case 11002:
          if (HIWORD(wPar)==BN_CLICKED){
            SendMessage(HWND(lPar),BM_SETCHECK,1,true);
            EnableAllWindows(0,Win);

            Str *Dir=(Str*)((LOWORD(wPar)==10002) ? &(This->MacroDir):&(This->ProfileDir));

            EasyStr NewFol=ChooseFolder(HWND(FullScreen ? StemWin:Win),T("Pick a Folder"),Dir->Text);
            if (NewFol.NotEmpty()){
              NO_SLASH(NewFol);
              *Dir=NewFol;
              DTree.RootFol=NewFol;
              DTree.RefreshDirectory();
            }

            SetForegroundWindow(Win);
            EnableAllWindows(true,Win);
            SetFocus(HWND(lPar));
            SendMessage(HWND(lPar),BM_SETCHECK,0,true);
          }
          break;
        case 10011:
          if (HIWORD(wPar)!=BN_CLICKED) break;
          if (macro_record==0){
            macro_record_file=This->MacroSel;
            macro_advance(MACRO_STARTRECORD);
          }else{
            macro_end(MACRO_ENDRECORD);
          }
          break;
        case 10012:
          if (HIWORD(wPar)!=BN_CLICKED) break;
          if (macro_play==0){
            macro_play_file=This->MacroSel;
            macro_advance(MACRO_STARTPLAY);
          }else{
            macro_end(MACRO_ENDPLAY);
          }
          break;
        case 10016:case 10017:case 10014:
        {
          MACROFILEOPTIONS MFO;
          macro_file_options(MACRO_FILE_GET,This->MacroSel,&MFO);
          switch (LOWORD(wPar)){
            case 10016:
              if (HIWORD(wPar)==CBN_SELENDOK) MFO.allow_same_vbls=CBGetSelectedItemData(HWND(lPar));
              break;
/*
            case 10017:
              if (HIWORD(wPar)==BN_CLICKED) MFO.add_mouse_together=SendMessage(HWND(lPar),BM_GETCHECK,0,0);
              break;
*/
            case 10014:
              if (HIWORD(wPar)==CBN_SELENDOK) MFO.max_mouse_speed=CBGetSelectedItemData(HWND(lPar));
              break;
          }
          macro_file_options(MACRO_FILE_SET,This->MacroSel,&MFO);
          break;
        }
        case 11001:
        case 11012:
          if (HIWORD(wPar)==BN_CLICKED){
            Str Path;
            if (LOWORD(wPar)==11012){
              if (IDNO==Alert(T("Are you sure?"),T("Overwrite File"),MB_ICONQUESTION | MB_YESNO)) break;
              Path=This->ProfileSel;
            }else{
              HTREEITEM Item=DTree.NewItem(T("New Profile"),DTree.RootItem,1);
              if (Item==NULL) break;
              Path=DTree.GetItemPath(Item);
            }
            SaveAllDialogData(0,Path);
          }
          break;
        case 11011:
          if (HIWORD(wPar)==BN_CLICKED) This->LoadProfile(This->ProfileSel);
          break;
        case 14020:
          if (HIWORD(wPar)==BN_CLICKED){
            SendMessage(HWND(lPar),BM_SETCHECK,1,true);

            EnableAllWindows(0,Win);

            Str LastIconSchemeFol=This->LastIconSchemePath;
            RemoveFileNameFromPath(LastIconSchemeFol,REMOVE_SLASH);

            Str NewFile=FileSelect(HWND(FullScreen ? StemWin:Win),T("Load Icon"),LastIconSchemeFol,
                          FSTypes(1,T("Steem Icon Scheme").Text,"*.stico",NULL),1,true,"stico",
                          GetFileNameFromPath(This->LastIconSchemePath));

            if (NewFile.NotEmpty()){
              This->LastIconSchemePath=NewFile;

              ConfigStoreFile SchemeCSF(NewFile);
              ConfigStoreFile CSF(INIFile);

              Str TransSect=T("Patch Text Section=");
              if (TransSect=="Patch Text Section=") TransSect="Text";

              Str Desc;
              for (int i=0;i<2;i++){
                Desc=SchemeCSF.GetStr(TransSect,"Description","");
                if (Desc.NotEmpty()) break;
                TransSect="Text";
              }
              if (Desc.NotEmpty()) Desc+="\r\n\r\n";
              Desc+=T("Are you sure you want to load this icon scheme?");
              if (Alert(Desc,T("Load Icon Scheme?"),MB_ICONQUESTION | MB_YESNO)==IDYES){
                Str File;
                EasyStringList Fols(eslNoSort);

                Str Fol=This->LastIconSchemePath;
                RemoveFileNameFromPath(Fol,REMOVE_SLASH);

                CSF.SetInt("Icons","UseDefaultIn256",SchemeCSF.GetInt("Options","UseDefaultIn256",0));

                int i=1;
                for(;;){
                  File=SchemeCSF.GetStr("Options",Str("SearchFolder")+(i++),"");
                  if (File.Empty()) break;
                  Fols.Add(Fol+SLASH+File);
                }
                Fols.Add(Fol);
                for (int n=1;n<RC_NUM_ICONS;n++){
                  File=SchemeCSF.GetStr("Icons",Str("Icon")+n,".");
                  if (File!="."){
                    Fol="";
                    for (int i=0;i<Fols.NumStrings;i++){
                      if (Exists(Str(Fols[i].String)+SLASH+File)){
                        Fol=Fols[i].String;
                        break;
                      }
                    }
                    if (Fol.NotEmpty()) CSF.SetStr("Icons",Str("Icon")+n,Fol+SLASH+File);
                  }
                }
                LoadAllIcons(&CSF,0);
              }

              SchemeCSF.Close();
              CSF.Close();
            }

            SetForegroundWindow(Win);
            EnableAllWindows(true,Win);
            SendMessage(HWND(lPar),BM_SETCHECK,0,true);
          }
          break;
        case 14021:
          if (HIWORD(wPar)==BN_CLICKED){
            ConfigStoreFile CSF(INIFile);
            CSF.SetInt("Icons","UseDefaultIn256",0);
            for (int n=1;n<RC_NUM_ICONS;n++) CSF.SetStr("Icons",Str("Icon")+n,"");
            LoadAllIcons(&CSF,0);
            CSF.Close();
          }
          break;
      }
      if (LOWORD(wPar)>=14100 && LOWORD(wPar)<14100+RC_NUM_ICONS){ // Icons
        if (HIWORD(wPar)==BN_CLICKED){
          bool AllowDefault=true;
          Str NewFile;
          SendMessage(HWND(lPar),BM_SETCHECK,1,true);
          if (SendMessage(HWND(lPar),BM_GETCLICKBUTTON,0,0)==1){
            AllowDefault=0;
            EnableAllWindows(0,Win);

            Str LastIconFol=This->LastIconPath;
            RemoveFileNameFromPath(LastIconFol,REMOVE_SLASH);

            NewFile=FileSelect(HWND(FullScreen ? StemWin:Win),T("Load Icon"),LastIconFol,
                            FSTypes(1,T("Icon File").Text,"*.ico",NULL),1,true,"ico",
                            GetFileNameFromPath(This->LastIconPath));
            if (NewFile.NotEmpty()) This->LastIconPath=NewFile;
            SetForegroundWindow(Win);
            EnableAllWindows(true,Win);
          }

          if (NewFile.NotEmpty() || AllowDefault){
            ConfigStoreFile CSF(INIFile);
            CSF.SetStr("Icons",Str("Icon")+(LOWORD(wPar)-14100),NewFile);
            LoadAllIcons(&CSF,0);
            CSF.Close();
          }

          SendMessage(HWND(lPar),BM_SETCHECK,0,true);
        }
      }else if (LOWORD(wPar)>=9000 && LOWORD(wPar)<9300){ // Ports
        Str ErrorText,ErrorTitle;
        int Port=(LOWORD(wPar)-9000)/100;
        int Control=(LOWORD(wPar) % 100);
        switch (Control){
          case 2:
            if (HIWORD(wPar)==CBN_SELENDOK){
              STPort[Port].Type=SendMessage(HWND(lPar),CB_GETITEMDATA,
                                  SendMessage(HWND(lPar),CB_GETCURSEL,0,0),0);
              This->PortsMakeTypeVisible(Port);
              STPort[Port].Create(ErrorText,ErrorTitle,true);
            }
            break;
          case 11:
            if (HIWORD(wPar)==CBN_SELENDOK){
              int NewDevice=SendMessage(HWND(lPar),CB_GETCURSEL,0,0)-2;
              if (NewDevice!=STPort[Port].GetMIDIOutDeviceID()){
                STPort[Port].MIDIOutDevice=NewDevice;
                STPort[Port].Create(ErrorText,ErrorTitle,true);
              }
            }
            break;
          case 13:
            if (HIWORD(wPar)==CBN_SELENDOK){
              int NewDevice=SendMessage(HWND(lPar),CB_GETCURSEL,0,0)-1;
              if (NewDevice!=STPort[Port].GetMIDIInDeviceID()){
                STPort[Port].MIDIInDevice=NewDevice;
                STPort[Port].Create(ErrorText,ErrorTitle,true);
              }
            }
            break;
          case 21:
            if (HIWORD(wPar)==CBN_SELENDOK){
              STPort[Port].LPTNum=SendMessage(HWND(lPar),CB_GETCURSEL,0,0);
              STPort[Port].Create(ErrorText,ErrorTitle,true);
            }
            break;
          case 31:
            if (HIWORD(wPar)==CBN_SELENDOK){
              STPort[Port].COMNum=SendMessage(HWND(lPar),CB_GETCURSEL,0,0);
              STPort[Port].Create(ErrorText,ErrorTitle,true);
            }
            break;
          case 41:
            if (HIWORD(wPar)==BN_CLICKED){
              SendMessage(HWND(lPar),BM_SETCHECK,BST_CHECKED,0);

              EasyStr CurFol=STPort[Port].File;

              EnableAllWindows(0,Win);

              char *CurName=GetFileNameFromPath(CurFol);
              if (CurName>CurFol.Text) *(CurName-1)=0;

              EasyStr FileName=FileSelect(HWND(FullScreen ? StemWin:Win),T("Select Output File"),CurFol,
                                      FSTypes(1,NULL),1,2,"dmp",CurName);
              if (FileName.NotEmpty()){
                STPort[Port].File=FileName;
                SendDlgItemMessage(GetDlgItem(Win,9000+Port*100),9000+Port*100+40,
                              WM_SETTEXT,0,LPARAM(FileName.Text));
                STPort[Port].Create(ErrorText,ErrorTitle,true);
              }
              SetForegroundWindow(Win);

              EnableAllWindows(true,Win);
              SetFocus(HWND(lPar));

              SendMessage(HWND(lPar),BM_SETCHECK,0,0);
            }
            break;
          case 42:
            if (HIWORD(wPar)==BN_CLICKED){
              SendMessage(HWND(lPar),BM_SETCHECK,BST_CHECKED,0);

              int Ret=Alert(T("Are you sure? This will permanently delete the contents of the file."),
                              T("Delete Contents?"),MB_ICONQUESTION | MB_YESNO);
              if (Ret==IDYES){
                STPort[Port].Close();
                DeleteFile(STPort[Port].File);
                STPort[Port].Create(ErrorText,ErrorTitle,true);
              }

              SendMessage(HWND(lPar),BM_SETCHECK,0,0);
            }
            break;
        }
      }else if (LOWORD(wPar)>=12000 && LOWORD(wPar)<12100){
        int i=LOWORD(wPar)-12000;
        if (i==0){
          if (HIWORD(wPar)==BN_CLICKED){
            osd_show_disk_light=!osd_show_disk_light;
            SendMessage(HWND(lPar),BM_SETCHECK,osd_show_disk_light,0);
          }
        }else if (i>=10 && i<20){
          if (HIWORD(wPar)==CBN_SELENDOK){
            int *p_element[4]={&osd_show_plasma,&osd_show_speed,&osd_show_icons,&osd_show_cpu};
            *(p_element[i-10])=CBGetSelectedItemData(HWND(lPar));
            osd_init_run(0);
          }
        }else if (i==20){
          if (HIWORD(wPar)==BN_CLICKED){
            osd_show_scrollers=!osd_show_scrollers;
            SendMessage(HWND(lPar),BM_SETCHECK,osd_show_scrollers,0);
          }
        }else if (i==30){
          if (HIWORD(wPar)==BN_CLICKED) This->ChangeOSDDisable(!osd_disable);
        }else if (i==50){
          if (HIWORD(wPar)==BN_CLICKED){
            osd_old_pos=!osd_old_pos;
            SendMessage(HWND(lPar),BM_SETCHECK,osd_old_pos,0);
            osd_init_run(0);
          }
        }
      }
      break;
    case WM_NOTIFY:
    {
      NMHDR *pnmh=(NMHDR*)lPar;
      if (wPar==60000){
        GET_THIS;
        switch (pnmh->code){
          case TVN_SELCHANGING:
          {
            NM_TREEVIEW *Inf=(NM_TREEVIEW*)lPar;

            if (Inf->action==4096){ //DODGY!!!!!! Undocumented!!!!!
              return true;
            }
            return 0;
          }
          case TVN_SELCHANGED:
          {
            NM_TREEVIEW *Inf=(NM_TREEVIEW*)lPar;

            if (Inf->itemNew.hItem){
              TV_ITEM tvi;

              tvi.mask=TVIF_PARAM;
              tvi.hItem=(HTREEITEM)Inf->itemNew.hItem;
              SendMessage(This->PageTree,TVM_GETITEM,0,(LPARAM)&tvi);
              This->DestroyCurrentPage();
              This->Page=tvi.lParam;
              This->CreatePage(This->Page);
            }
            break;
          }
        }
      }else if (wPar==11013){
        if (IsWindowEnabled(pnmh->hwndFrom)==0) break;
        GET_THIS;
        switch (pnmh->code){
          case LVN_ITEMCHANGED:
          {
            NM_LISTVIEW *pLV=(NM_LISTVIEW*)lPar;
            if (pLV->uChanged & LVIF_STATE){
              LV_ITEM lvi;
              lvi.iItem=pLV->iItem;
              lvi.iSubItem=0;
              lvi.mask=LVIF_PARAM | LVIF_STATE;
              lvi.stateMask=LVIS_STATEIMAGEMASK;
              SendMessage(pnmh->hwndFrom,LVM_GETITEM,0,(LPARAM)&lvi);
              WriteCSFInt("ProfileSections",ProfileSection[pLV->iItem].Name,lvi.state,This->ProfileSel);
            }
            break;
          }
        }
      }
      break;
    }
    case WM_HSCROLL:
    {
      int ID=GetDlgCtrlID(HWND(lPar));
      switch (ID){
        case 2001:
        case 2003:
          brightness=SendDlgItemMessage(Win,2001,TBM_GETPOS,0,0)-128;
          SendDlgItemMessage(Win,2000,WM_SETTEXT,0,LPARAM((T("Brightness")+": "+brightness).Text));
          contrast=SendDlgItemMessage(Win,2003,TBM_GETPOS,0,0)-128;
          SendDlgItemMessage(Win,2002,WM_SETTEXT,0,LPARAM((T("Contrast")+": "+contrast).Text));
          make_palette_table(brightness,contrast);
          if (flashlight_flag==0) palette_convert_all();

          GET_THIS;
          This->DrawBrightnessBitmap(This->hBrightBmp);
          InvalidateRect(GetDlgItem(Win,2010),NULL,true);
          UpdateWindow(GetDlgItem(Win,2010));
          break;
        case 1001:
          slow_motion_speed=(SendDlgItemMessage(Win,1001,TBM_GETPOS,0,0)+1)*10;
          SendDlgItemMessage(Win,1000,WM_SETTEXT,0,LPARAM((T("Slow motion speed")+": "+(slow_motion_speed/10)+"%").Text));
          break;
        case 1011:
        {
          fast_forward_max_speed=1000/(SendDlgItemMessage(Win,1011,TBM_GETPOS,0,0)+2);
          Str Text=T("Maximum fast forward speed")+": ";
          if (fast_forward_max_speed>50){
            Text+=Str((1000/fast_forward_max_speed)*100)+"%";
          }else{
            Text+=T("Unlimited");
            fast_forward_max_speed=0;
          }
          SendDlgItemMessage(Win,1010,WM_SETTEXT,0,LPARAM(Text.Text));
          break;
        }
        case 1041:
        {
          run_speed_ticks_per_second=100000 / (50 + SendDlgItemMessage(Win,1041,TBM_GETPOS,0,0)*5);
          SendDlgItemMessage(Win,1040,WM_SETTEXT,0,
            LPARAM((T("Run speed")+": "+(100000/run_speed_ticks_per_second)+"%").Text));
          break;
        }
        case 7100:
          MaxVolume=SendMessage(HWND(lPar),TBM_GETPOS,0,0)-9000;
          SoundChangeVolume();
          break;
        case 6001:
          MIDI_out_volume=(WORD)SendMessage(HWND(lPar),TBM_GETPOS,0,0);
          if (runstate==RUNSTATE_RUNNING){
            for (int i=0;i<3;i++){
              if (STPort[i].MIDI_Out) STPort[i].MIDI_Out->SetVolume(MIDI_out_volume);
            }
          }
          break;
        case 6041:
          MIDI_in_speed=SendMessage(HWND(lPar),TBM_GETPOS,0,0)+1;
          SendDlgItemMessage(Win,6040,WM_SETTEXT,0,LPARAM((T("Input speed")+": "+Str(MIDI_in_speed)+"%").Text));
          break;
      }
      break;
    }
    case WM_DRAWITEM:
      if (wPar==8300){
        GET_THIS;
        DRAWITEMSTRUCT *di=(DRAWITEMSTRUCT*)lPar;
        HBRUSH br;
        int oldtextcol=GetTextColor(di->hDC),oldmode=GetBkMode(di->hDC);
        if (di->itemState & ODS_SELECTED){
          br=CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
          SetTextColor(di->hDC,GetSysColor(COLOR_HIGHLIGHTTEXT));
        }else{
          br=CreateSolidBrush(GetSysColor(COLOR_WINDOW));
          SetTextColor(di->hDC,GetSysColor(COLOR_WINDOWTEXT));
        }
        SetBkMode(di->hDC,TRANSPARENT);
        FillRect(di->hDC,&(di->rcItem),br);
        DeleteObject(br);
        if (di->itemID < 0xffffffff){
          int idx=di->itemID;
          if (This->eslTOS_Descend) idx=This->eslTOS.NumStrings-1 - di->itemID;
          WORD Ver=(WORD)This->eslTOS[idx].Data[0];
          WORD Lang=(WORD)This->eslTOS[idx].Data[1];
          WORD Date=(WORD)This->eslTOS[idx].Data[2];
          Str Text=This->eslTOS[idx].String;
          char *NameEnd=strchr(Text,'\01');
          if (NameEnd) *NameEnd=0;

          RECT shiftrect=di->rcItem;shiftrect.left+=2;

          int Right=shiftrect.right;
          shiftrect.right=shiftrect.left+60;
          Str szVer=HEXSl(Ver,3).Insert(".",1);
          DrawText(di->hDC,szVer,strlen(szVer),&shiftrect,DT_LEFT | DT_SINGLELINE | DT_VCENTER);

          shiftrect.right=shiftrect.left+GetTextSize((HFONT)GetCurrentObject(di->hDC,OBJ_FONT),szVer).Width+2;
          HDC TempDC=CreateCompatibleDC(di->hDC);
          HANDLE OldBmp=SelectObject(TempDC,LoadBitmap(Inst,"TOSFLAGS"));
          int FlagIdx=This->TOSLangToFlagIdx(Lang);
          if (FlagIdx>=0){
            BitBlt(di->hDC,shiftrect.right,shiftrect.top+((shiftrect.bottom-shiftrect.top)/2)-RC_FLAG_HEIGHT/2,
                    RC_FLAG_WIDTH,RC_FLAG_HEIGHT,TempDC,FlagIdx*RC_FLAG_WIDTH,0,SRCCOPY);
          }
          DeleteObject(SelectObject(TempDC,OldBmp));
          DeleteDC(TempDC);

          shiftrect.left+=60;shiftrect.right=Right-105;
          DrawText(di->hDC,Text,strlen(Text),&shiftrect,DT_LEFT | DT_SINGLELINE | DT_VCENTER);

          FILETIME ft;
          if (DosDateTimeToFileTime(Date,WORD(0),&ft)){
            SYSTEMTIME st;
            FileTimeToSystemTime(&ft,&st);
            Str szDate;
            if (USDateFormat){
              szDate=Str(st.wDay)+"/"+st.wMonth+"/"+st.wYear;
            }else{
              szDate=Str(st.wMonth)+"/"+st.wDay+"/"+st.wYear;
            }
            shiftrect.left=Right-100;shiftrect.right=Right;
            DrawText(di->hDC,szDate,strlen(szDate),&shiftrect,DT_LEFT | DT_SINGLELINE | DT_VCENTER);
          }
        }
        SetTextColor(di->hDC,oldtextcol);
        SetBkMode(di->hDC,oldmode);
        if (di->itemState & ODS_FOCUS) DrawFocusRect(di->hDC,&(di->rcItem));
        return 0;
      }
      break;
    case WM_DELETEITEM:
//      if (wPar==8300) delete (Str*)(((DELETEITEMSTRUCT*)lPar)->itemData);
      break;
    case WM_ACTIVATE:
      if (wPar!=WA_INACTIVE){
        GET_THIS;
        This->DrawBrightnessBitmap(This->hBrightBmp);
      }
      break;
    case (WM_USER+1011):
    {
      GET_THIS;

      HWND NewParent=(HWND)lPar;
      if (NewParent){
        SetWindowPos(Win,NULL,This->FSLeft,This->FSTop,0,0,SWP_NOZORDER | SWP_NOSIZE);
      }else{
        SetWindowPos(Win,NULL,This->Left,This->Top,0,0,SWP_NOZORDER | SWP_NOSIZE);
      }
      This->ChangeParent(NewParent);
      break;
    }
    case WM_CLOSE:
      GET_THIS;
      This->Hide();
      return 0;
    case DM_GETDEFID:
      return 0;
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
int TOptionBox::DTreeNotifyProc(DirectoryTree*,void *t,int Mess,int i1,int)
{
  DTREE_LOG(EasyStr("DTree: Options DTreeNotifyProc Mess=")+Mess);
  TOptionBox *This=(TOptionBox*)t;
  if (Mess==DTM_SELCHANGED || Mess==DTM_NAMECHANGED){
    DTREE_LOG(EasyStr("DTree: Getting item path and type for ")+DWORD(i1));
    Str NewSel=DTree.GetItemPath((HTREEITEM)i1);
    int Type=DTree.GetItem((HTREEITEM)i1,TVIF_IMAGE).iImage;
    DTREE_LOG(EasyStr("DTree: NewSel=")+NewSel+" Type="+Type);
    int DisableLo=0,DisableHi=0;
    if (GetDlgCtrlID(DTree.hTree)==10000){
      This->MacroSel=NewSel;
      DTREE_LOG(EasyStr("DTree: Calling UpdateMacroRecordAndPlay"));
      This->UpdateMacroRecordAndPlay(NewSel,Type);
      DisableLo=10010;DisableHi=10030;
    }else if (GetDlgCtrlID(DTree.hTree)==11000){
      This->ProfileSel=NewSel;
      DisableLo=11010;DisableHi=11030;

      DTREE_LOG(EasyStr("DTree: Updating profile sections"));
      int i=0;
      HWND LV=GetDlgItem(This->Handle,11013);
      EnableWindow(LV,0);
      ConfigStoreFile CSF;
      if (Type==1) CSF.Open(This->ProfileSel);
      while (ProfileSection[i].Name){
        int Check=LVI_SI_CHECKED;
        if (Type==1) Check=CSF.GetInt("ProfileSections",ProfileSection[i].Name,LVI_SI_CHECKED);
        ListView_SetItemState(LV,i++,Check,LVIS_STATEIMAGEMASK);
      }
      if (Type==1) CSF.Close();
    }
    DTREE_LOG(EasyStr("DTree: Enabling/Disabling sections"));
    for (int n=DisableLo;n<DisableHi;n++){
      if (GetDlgItem(This->Handle,n)) EnableWindow(GetDlgItem(This->Handle,n),Type);
    }
  }
  DTREE_LOG(EasyStr("DTree: Finished processing Mess=")+Mess);
  DTREE_LOG("");
  return 0;
}
//---------------------------------------------------------------------------
#undef GET_THIS
//---------------------------------------------------------------------------
#endif

#ifdef UNIX
#include "x/x_options.cpp"
#endif


