#define OPTIONS_HEIGHT 400
//---------------------------------------------------------------------------
TOptionBox::TOptionBox()
{
  Section="Options";

	Page=9; // Default to machine
  NewMemConf0=-1,NewMemConf1=-1,NewMonitorSel=-1;
  RecordWarnOverwrite=true;

  eslTOS_Sort=eslSortByData0;
  eslTOS_Descend=0;
}
//---------------------------------------------------------------------------
void TOptionBox::UpdateMacroRecordAndPlay(Str Sel,int Type)
{
  if (Handle==0) return;

  hxc_button *p_grp=(hxc_button*)hxc::find(page_p,2009);
  if (p_grp==NULL) return;
  hxc_button *p_rec=(hxc_button*)hxc::find(p_grp->handle,2010);
  hxc_button *p_play=(hxc_button*)hxc::find(p_grp->handle,2011);
  hxc_dropdown *p_ms=(hxc_dropdown*)hxc::find(p_grp->handle,2012);
  hxc_dropdown *p_ped=(hxc_dropdown*)hxc::find(p_grp->handle,2013);

  if (Sel.Empty()){
    Type=-1;
    if (dir_lv.lv.sel>=0){
      Sel=dir_lv.get_item_path(dir_lv.lv.sel);
      Type=dir_lv.sl[dir_lv.lv.sel].Data[DLVD_TYPE];
    }
  }
  ShowHideWindow(XD,p_grp->handle,Type==2);

  bool CheckRec=0,CheckPlay=0;
  if (Type==2){
    if (macro_record && IsSameStr_I(macro_record_file,Sel)) CheckRec=true;
    if (macro_play && IsSameStr_I(macro_play_file,Sel)) CheckPlay=true;
  }
  p_play->set_check(CheckPlay);
  p_rec->set_check(CheckRec);

  MACROFILEOPTIONS MFO;
  macro_file_options(MACRO_FILE_GET,Sel,&MFO);
  p_ms->select_item_by_data(MFO.max_mouse_speed);
  p_ped->select_item_by_data(MFO.allow_same_vbls);
  p_ms->draw();
  p_ped->draw();
}
//---------------------------------------------------------------------------
void TOptionBox::UpdateProfileDisplay(Str Sel,int Type)
{
  if (Handle==0) return;

  hxc_button *p_grp=(hxc_button*)hxc::find(page_p,2109);
  if (p_grp==NULL) return;

  if (Sel.Empty()){
    Type=-1;
    if (dir_lv.lv.sel>=0){
      Sel=dir_lv.get_item_path(dir_lv.lv.sel);
      Type=dir_lv.sl[dir_lv.lv.sel].Data[DLVD_TYPE];
    }
  }
  ShowHideWindow(XD,p_grp->handle,Type==2);

  if (Type==2){
    ConfigStoreFile CSF;
    CSF.Open(Sel);
    int i=0;
    while (ProfileSection[i].Name){
      int icon=101+ICO16_TICKED;
      if (CSF.GetInt("ProfileSections",ProfileSection[i].Name,PROFILESECT_ON)==PROFILESECT_OFF){
        icon=101+ICO16_UNTICKED;
      }
      profile_sect_lv.sl[i++].Data[0]=icon;
    }
    CSF.Close();
    profile_sect_lv.draw(true);
  }
}
//---------------------------------------------------------------------------
void TOptionBox::UpdatePortDisplay(int p)
{
	if (PortGroup[0].handle==0) return;

	int PortIOType=GetPortIOType(STPort[p].Type);
	if (PortIOType>=0){
    if (STPort[p].Type==PORTTYPE_LAN){
      int Base=1200+p*20;
    	hxc_edit *p_ed_out=(hxc_edit*)hxc::find(LANGroup[p].handle,Base+10);
    	hxc_edit *p_ed_in=(hxc_edit*)hxc::find(LANGroup[p].handle,Base+12);
    	hxc_button *p_but_open=(hxc_button*)hxc::find(LANGroup[p].handle,Base+14);
      if (p_ed_out->text!=STPort[p].PortDev[PortIOType]){
        p_ed_out->set_text(STPort[p].PortDev[PortIOType]);
      }
      if (p_ed_in->text!=STPort[p].LANPipeIn){
        p_ed_in->set_text(STPort[p].LANPipeIn);
      }
      p_but_open->set_check(STPort[p].IsPCPort());
  	  XUnmapWindow(XD,FileGroup[p].handle);
  	  XUnmapWindow(XD,IOGroup[p].handle);
      XMapWindow(XD,LANGroup[p].handle);
    }else{
      if (IODevEd[p].text!=STPort[p].PortDev[PortIOType]){
        IODevEd[p].set_text(STPort[p].PortDev[PortIOType]);
      }
      IOAllowIOBut[p][0].set_check(STPort[p].AllowIO[PortIOType][0]);
      IOAllowIOBut[p][1].set_check(STPort[p].AllowIO[PortIOType][1]);
      IOOpenBut[p].set_check(STPort[p].IsPCPort());
      XUnmapWindow(XD,LANGroup[p].handle);
      XUnmapWindow(XD,FileGroup[p].handle);
      XMapWindow(XD,IOGroup[p].handle);
    }
	}else if (STPort[p].Type==PORTTYPE_FILE){
    XUnmapWindow(XD,LANGroup[p].handle);
	  XUnmapWindow(XD,IOGroup[p].handle);
	  XMapWindow(XD,FileGroup[p].handle);
	}else{
    XUnmapWindow(XD,LANGroup[p].handle);
	  XUnmapWindow(XD,FileGroup[p].handle);
	  XUnmapWindow(XD,IOGroup[p].handle);
	}
}
//---------------------------------------------------------------------------
#include "x_options_create.cpp"

void TOptionBox::Show()
{
  if (Handle) return;

  page_lv.sl.DeleteAll();
  page_lv.sl.Sort=eslNoSort;

  page_lv.sl.Add(T("Machine"),101+ICO16_ST,9);
  page_lv.sl.Add("TOS",101+ICO16_CHIP,10);
  page_lv.sl.Add(T("Macros"),101+ICO16_MACROS,13);
  page_lv.sl.Add(T("Ports"),101+ICO16_PORTS,12);

  page_lv.sl.Add(T("General"),101+ICO16_TOOLS,0);
  if (UseSound) page_lv.sl.Add(T("Sound"),101+ICO16_SOUND,5);
  page_lv.sl.Add(T("Display"),101+ICO16_DISPLAY,1);
  page_lv.sl.Add(T("On Screen Display"),101+ICO16_OSD,14);
  page_lv.sl.Add(T("Brightness")+"/"+T("Contrast"),101+ICO16_BRIGHTCON,2);
  page_lv.sl.Add(T("Profiles"),101+ICO16_PROFILE,11);
  page_lv.sl.Add(T("Startup"),101+ICO16_FUJI16,6);

  page_lv.lpig=&Ico16;
  page_lv.display_mode=1;

  int lv_w=page_lv.get_max_width(XD);
  page_w=380;
  int w=lv_w+10+page_w+10;

  if (StandardShow(w,OPTIONS_HEIGHT,T("Options"),
      ICO16_OPTIONS,ButtonPressMask,(LPWINDOWPROC)WinProc)) return;

  control_parent.create(XD,Handle,lv_w,0,page_w+20,OPTIONS_HEIGHT,
                    NULL,this,BT_STATIC,"",0,hxc::col_bk);
  page_p=control_parent.handle;
  page_l=10;

  page_lv.select_item_by_data(Page,1);
  page_lv.id=60000;
  page_lv.create(XD,Handle,0,0,lv_w,OPTIONS_HEIGHT,listview_notify_proc,this);

  CreatePage(Page);

  if (StemWin) OptBut.set_check(true);

  XMapWindow(XD,Handle);
  XFlush(XD);
}
//---------------------------------------------------------------------------
void TOptionBox::Hide()
{
  if (XD==NULL || Handle==0) return;

  hints.remove_all_children(page_p);
  StandardHide();

  if (StemWin) OptBut.set_check(0);
}
//---------------------------------------------------------------------------
int TOptionBox::WinProc(TOptionBox *This,Window Win,XEvent *Ev)
{
  switch (Ev->type){
    case ClientMessage:
      if (Ev->xclient.message_type==hxc::XA_WM_PROTOCOLS){
        if (Atom(Ev->xclient.data.l[0])==hxc::XA_WM_DELETE_WINDOW){
          This->Hide();
        }
      }
  }
  return PEEKED_MESSAGE;
}
//---------------------------------------------------------------------------
void TOptionBox::EnableBorderOptions(bool)
{
}
//---------------------------------------------------------------------------
int TOptionBox::listview_notify_proc(hxc_listview* LV,int Mess,int i)
{
  TOptionBox *This=(TOptionBox*)(LV->owner);
  if (LV->id==60000){
    if (Mess==LVN_SELCHANGE || Mess==LVN_SINGLECLICK){
      int NewPage=LV->sl[LV->sel].Data[1];
      if (This->Page!=NewPage){
        XUnmapWindow(XD,This->page_p);
        hxc::destroy_children_of(This->page_p);
        hints.remove_all_children(This->page_p);
			  This->brightness_ig.FreeIcons();
        This->Page=NewPage;
        This->CreatePage(This->Page);
        XMapWindow(XD,This->page_p);
      }
    }
  }else if (LV->id==1000){
    if (Mess==LVN_SELCHANGE || Mess==LVN_SINGLECLICK){
      This->NewROMFile=strrchr(LV->sl[LV->sel].String,'\01')+1;
      if (This->NewROMFile[0]!='/') This->NewROMFile.Insert(RunDir+"/",0);
      if (IsSameStr_I(ROMFile,This->NewROMFile)) This->NewROMFile="";
      CheckResetIcon();
    }
  }else if (LV->id==2112){
    if (Mess==LVN_ICONCLICK){
      int icon=LV->sl[i].Data[0]-101;
      if (icon==ICO16_TICKED){
        icon=ICO16_UNTICKED;
      }else{
        icon=ICO16_TICKED;
      }
      WriteCSFInt("ProfileSections",ProfileSection[i].Name,
                  int((icon==ICO16_TICKED) ? PROFILESECT_ON:PROFILESECT_OFF),
                  This->ProfileSel);
      LV->sl[i].Data[0]=101+icon;
      LV->draw(0);
      return 1;
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
int TOptionBox::button_notify_proc(hxc_button*b,int mess,int* ip)
{
  TOptionBox *This=(TOptionBox*)(b->owner);
  if (mess==BN_CLICKED){
    if (b->id==100){ //auto load snapshot
      AutoLoadSnapShot=b->checked;
    }else if (b->id==101){ //never use MIT Shared Memory Extension
      WriteCSFStr("Options","NoSHM",EasyStr(b->checked),INIFile);
    }else if (b->id==102){ //never use PortAudio
      WriteCSFStr("Options","NoPortAudio",EasyStr(b->checked),INIFile);
    }else if (b->id==110){
      PauseWhenInactive=b->checked;
    }else if (b->id==120){
      HighPriority=b->checked;
    }else if (b->id==121){
      ShowTips=b->checked;
      if (ShowTips){
        hints.start();
      }else{
        hints.stop();
      }
    }else if (b->id==130){
      floppy_access_ff=b->checked;
    }else if (b->id==140){
      StartEmuOnClick=b->checked;
    }else if (b->id==210){
      draw_fs_fx=(b->checked ? DFSFX_GRILLE:DFSFX_NONE);
      if (draw_grille_black<4) draw_grille_black=4;
      if (runstate!=RUNSTATE_RUNNING) draw(false);
    }else if (b->id==220){
      Disp.DoAsyncBlit=b->checked;
    }else if (b->id==230){
      ResChangeResize=b->checked;
    }else if (b->id==122){
      This->FullscreenBrightnessBitmap();
    }else if (b->id==250 || b->id==251){
      int res=(b->id)-250;
      if (b->checked){
        WinSizeForRes[res]=1;
      }else{
        WinSizeForRes[res]=0;
      }
      if (ResChangeResize && res==screen_res){
        StemWinResize();
      }
    }else if (b->id==252){
      fileselect.set_corner_icon(&Ico16,ICO16_FOLDER);
      EasyStr Path=fileselect.choose(XD,ScreenShotFol,"",T("Pick a Folder"),
          FSM_CHOOSE_FOLDER | FSM_CONFIRMCREATE,folder_parse_routine,"");
      if (Path.NotEmpty()){
        NO_SLASH(Path);
        ScreenShotFol=Path;
        CreateDirectory(ScreenShotFol,NULL);
        This->screenshots_fol_display.set_text(ScreenShotFol);
      }
    }else if (b->id==253){
      prefer_res_640_400=b->checked;
    }else if (b->id==5100){
      This->SetRecord(!sound_record);
    }else if (b->id==5101){
      fileselect.set_corner_icon(&Ico16,ICO16_SOUND);
      EasyStr NewFile=fileselect.choose(XD,This->WAVOutputDir,
                GetFileNameFromPath(WAVOutputFile),T("Choose WAV Output File"),FSM_OK,
                wavfile_parse_routine,".wav");
      if (NewFile.NotEmpty()){
        WAVOutputFile=NewFile;
        This->WAVOutputDir=NewFile;
        RemoveFileNameFromPath(This->WAVOutputDir,REMOVE_SLASH);
        This->wav_output_label.set_text(NewFile);
      }
    }else if (b->id==5102){
      This->RecordWarnOverwrite=b->checked;
    }else if (b->id==5300){
      if (sound_internal_speaker) internal_speaker_sound_by_period(0);

      sound_internal_speaker=!sound_internal_speaker;
      b->set_check(sound_internal_speaker);
    }else if (b->id==737){ // Choose cart
    	b->set_check(true);
      Str LastCartFol=This->LastCartFile;
      RemoveFileNameFromPath(LastCartFol,REMOVE_SLASH);

      fileselect.set_corner_icon(&Ico16,ICO16_CART);
      EasyStr fn=fileselect.choose(XD,LastCartFol,GetFileNameFromPath(This->LastCartFile),
        T("Find a Cartridge"),FSM_LOAD | FSM_LOADMUSTEXIST,cartfile_parse_routine,".stc");
      if (fn[0]){
        This->LastCartFile=fn;
        if (load_cart(fn)){
          Alert(T("There was an error loading the cartridge."),T("Cannot Load Cartridge"),MB_ICONEXCLAMATION);
        }else{
          CartFile=fn;
          This->cart_display.set_text(fn);
        }
        CheckResetDisplay();
      }
    	b->set_check(0);
    }else if (b->id==747){ // Remove cart
      This->cart_display.set_text("");

      delete[] cart;
      cart=NULL;
      CartFile="";
      if (pc>=MEM_EXPANSION_CARTRIDGE && pc<0xfc0000){
      	SET_PC(PC32);
      }

      CheckResetDisplay();
    }else if (b->id==960){
    	EnableShiftSwitching=b->checked;
      InitKeyTable();
    }else if (b->id==1000){
      reset_st();
    }else if (b->id==1010){
    	b->set_check(true);
      fileselect.set_corner_icon(&Ico16,ICO16_CHIP);
      EasyStr fn=fileselect.choose(XD,This->TOSBrowseDir,"",
        		T("Find a TOS"),FSM_LOAD | FSM_LOADMUSTEXIST,
		        romfile_parse_routine,".img");
      if (fn[0]){
        This->TOSBrowseDir=fn;
        RemoveFileNameFromPath(This->TOSBrowseDir,true);

        bool Found=0;
        for (int i=0;i<This->tos_lv.sl.NumStrings;i++){
        	if (IsSameStr_I(strrchr(This->tos_lv.sl[i].String,'\01')+1,fn)){
        		Found=true;
        		This->tos_lv.changesel(i);
        	}
        }
        if (Found==0){
          if (get_TOS_address(fn)){
            Str Name=GetFileNameFromPath(fn),Ext;
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
            symlink(fn,LinkName);
            This->TOSRefreshBox(fn);
            This->NewROMFile=fn;
          }else{
            Alert(fn+" "+T("is not a valid TOS image."),T("Cannot use TOS"),MB_ICONEXCLAMATION);
          }
        }
	      CheckResetIcon();
      }
    	b->set_check(0);
    }else if (b->id==1011){
      b->set_check(true);
      This->TOSRefreshBox();
      b->set_check(0);
    }else if (b->id>=1200 && b->id<1300){
      int p=(b->id-1200)/20;
      int i=b->id % 20;
      int IOType=GetPortIOType(STPort[p].Type);
      bool ClosePort=0,UpdateDisplay=0;
      switch (i){
        case 1:case 11:case 13:	// Choose device
        {
          Str CurDev=STPort[p].PortDev[IOType];
          if (i==13) CurDev=STPort[p].LANPipeIn;
          b->set_check(true);
          fileselect.set_corner_icon(&Ico16,ICO16_PORTS);
          Str CurFol=CurDev;
          RemoveFileNameFromPath(CurFol,REMOVE_SLASH);
          EasyStr fn=fileselect.choose(XD,CurFol,GetFileNameFromPath(CurDev),
                        T("Choose Device"),FSM_OK | FSM_LOADMUSTEXIST,NULL,"");
          if (fileselect.chose_option==FSM_OK){
            if (i!=13){
              STPort[p].PortDev[IOType]=fn;
            }else{
              STPort[p].LANPipeIn=fn;
            }
            UpdateDisplay=true;
            ClosePort=true;
          }
          b->set_check(0);
          break;
        }
        case 3: STPort[p].AllowIO[IOType][0]=b->checked;ClosePort=true; break;
        case 4: STPort[p].AllowIO[IOType][1]=b->checked;ClosePort=true; break;
        case 5:case 14: // Open device
        {
          if (STPort[p].IsPCPort()){
            ClosePort=true;
          }else{
            Str ErrText,ErrTitle;
            b->set_check(STPort[p].Create(ErrText,ErrTitle,true));
          }
          break;
        }
        case 6: // Choose file
        {
          b->set_check(true);
          fileselect.set_corner_icon(&Ico16,ICO16_PORTS);
          Str CurFol=STPort[p].File;
          RemoveFileNameFromPath(CurFol,REMOVE_SLASH);
          EasyStr fn=fileselect.choose(XD,CurFol,GetFileNameFromPath(STPort[p].File),
                        T("Select Output File"),FSM_OK | FSM_LOADMUSTEXIST,NULL,".dmp");
          if (fileselect.chose_option==FSM_OK){
            STPort[p].File=fn;
            This->FileDisplay[p].set_text(fn);
          }
          b->set_check(0);
          break;
        }
        case 7: // Empty file
        {
          b->set_check(true);
          int Ret=Alert(T("Are you sure? This will permanently delete the contents of the file."),
                          T("Delete Contents?"),MB_ICONQUESTION | MB_YESNO);
          if (Ret==IDYES){
            STPort[p].Close();
            DeleteFile(STPort[p].File);
            Str ErrText,ErrTitle;
            STPort[p].Create(ErrText,ErrTitle,true);
          }
          b->set_check(0);
          break;
        }
      }
      if (ClosePort){
        if (STPort[p].IsPCPort()){
          STPort[p].Close();
          UpdateDisplay=true;
        }
      }
      if (UpdateDisplay) This->UpdatePortDisplay(p);
    }else if (b->id==2001){
      b->set_check(true);
      Str Path=This->CreateMacroFile(true);
      dir_lv.select_item_by_name(GetFileNameFromPath(Path));
      This->MacroSel=Path;
      This->UpdateMacroRecordAndPlay();
      b->set_check(0);
    }else if (b->id==2002 || b->id==2102){ // Change store folder
      bool Macro=(b->id==2002);
      char *Current=This->ProfileDir;
      if (Macro) Current=This->MacroDir;
      fileselect.set_corner_icon(&Ico16,ICO16_FOLDER);
      EasyStr new_path=fileselect.choose(XD,Current,"",T("Pick a Folder"),
          FSM_CHOOSE_FOLDER | FSM_CONFIRMCREATE,folder_parse_routine,"");
      if (new_path.NotEmpty()){
        NO_SLASH(new_path);
        if (Macro){
          This->MacroDir=new_path;
          This->MacroSel="";
        }else{
          This->ProfileDir=new_path;
          This->ProfileSel="";
        }

        This->dir_lv.base_fol=new_path;
        This->dir_lv.fol=new_path;
        This->dir_lv.lv.sel=-1;
        This->dir_lv.refresh_fol();

        This->UpdateMacroRecordAndPlay();
        This->UpdateProfileDisplay();
      }
    }else if (b->id==2010){
      if (macro_record==0){
        macro_record_file=This->MacroSel;
        macro_advance(MACRO_STARTRECORD);
      }else{
        macro_end(MACRO_ENDRECORD);
      }
    }else if (b->id==2011){
      if (macro_play==0){
        macro_play_file=This->MacroSel;
        macro_advance(MACRO_STARTPLAY);
      }else{
        macro_end(MACRO_ENDPLAY);
      }
    }else if (b->id==2101 || b->id==2111){
      b->set_check(true);
      Str Path;
      if (b->id==2111){ // Save over
        if (IDYES==Alert(T("Are you sure?"),T("Overwrite File"),MB_ICONQUESTION | MB_YESNO)){
          Path=This->ProfileSel;
        }
      }else{ // Save new
        hxc_prompt prompt;
        Path=prompt.ask(XD,T("New Profile"),T("Enter Name"));
        if (Path.NotEmpty()){
          Path=GetUniquePath(dir_lv.fol,Path+".ini");
        }
      }
      if (Path.NotEmpty()){
        SaveAllDialogData(0,Path);
        dir_lv.refresh_fol();
        dir_lv.select_item_by_name(GetFileNameFromPath(Path));
        This->ProfileSel=Path;
        This->UpdateProfileDisplay();
      }
      b->set_check(0);
    }else if (b->id==2110){
      b->set_check(true);
      This->LoadProfile(This->ProfileSel);
      b->set_check(0);

    }else if (b->id==12000){
      osd_show_disk_light=b->checked;
    }else if (b->id==12020){
      osd_show_scrollers=b->checked;
    }else if (b->id==12030){
      This->ChangeOSDDisable(b->checked);
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
int TOptionBox::dd_notify_proc(hxc_dropdown*dd,int mess,int i)
{
  TOptionBox*This=(TOptionBox*)(dd->owner);
	if (mess!=DDN_SELCHANGE) return 0;

  if (dd->id==8){
    n_cpu_cycles_per_second=max(min(dd->sl[dd->sel].Data[0],128000000l),8000000l);
    if (runstate==RUNSTATE_RUNNING) osd_init_run(0);
    prepare_cpu_boosted_event_plans();
  }else if (dd->id==5001){ //sound mode
    This->SoundModeChange(i,true,0);
  }else if (dd->id==5000){ //sound device
    sound_device_name=dd->sl[dd->sel].String;
    This->SoundModeChange(sound_mode,0,0);
  }else if (dd->id==5004){ //sound delay
    psg_write_n_screens_ahead=dd->sl[dd->sel].Data[0];
  }else if (dd->id==5002){ //sound freq
    sound_chosen_freq=dd->sl[i].Data[0];
    This->UpdateSoundFreq();
  }else if (dd->id==5003){ //sound format
    This->ChangeSoundFormat(LOBYTE(dd->sl[dd->sel].Data[0]),HIBYTE(dd->sl[dd->sel].Data[0]));
  }else if (dd->id>=12010 && dd->id<12020){
    int *p_element[4]={&osd_show_plasma,&osd_show_speed,&osd_show_icons,&osd_show_cpu};
    *(p_element[dd->id - 12010])=dd->sl[dd->sel].Data[0];
  }
  if (dd==&(This->frameskip_dd)){
    if (i==4){
      frameskip=AUTO_FRAMESKIP;
    }else{
      frameskip=i+1;
    }
  }else if (dd==&(This->border_dd)){
    int newborder=dd->sel,oldborder=border;
    if (This->ChangeBorderModeRequest(newborder)){
      border=newborder;
      if (FullScreen) change_fullscreen_display_mode(true);
      change_window_size_for_border_change(oldborder,newborder);
      draw(false);
    }else{
      dd->changesel(min(oldborder,2));dd->draw();
      dd->lv.changesel(min(oldborder,2));dd->lv.draw(true,true);
      border=oldborder;
    }
  }else if (dd->id==910){
    This->NewMemConf0=dd->lv.sl[dd->sel].Data[0];
    This->NewMemConf1=dd->lv.sl[dd->sel].Data[1];
    if (bank_length[0]==mmu_bank_length_from_config[This->NewMemConf0] &&
        bank_length[1]==mmu_bank_length_from_config[This->NewMemConf1]){
      This->NewMemConf0=-1;
    }
    CheckResetIcon();
  }else if (dd->id==920){
    This->NewMonitorSel=dd->sel;
    if (This->NewMonitorSel==This->GetCurrentMonitorSel()) This->NewMonitorSel=-1;
    CheckResetIcon();
  }else if (dd->id==940){
    KeyboardLangID=(LANGID)(This->keyboard_language_dd.lv.sl[This->keyboard_language_dd.sel].Data[0]);
    InitKeyTable();
  }else if (dd->id==1020){
    This->eslTOS_Sort=(ESLSortEnum)dd->sl[dd->sel].Data[0];
    This->eslTOS_Descend=(bool)dd->sl[dd->sel].Data[1];
    This->TOSRefreshBox();
  }else if (dd->id>=1200 && dd->id<1300){
    int p=(dd->id-1200)/20;
    int NewType=dd->sl[dd->sel].Data[0];
    if (STPort[p].Type!=NewType){
      STPort[p].Close();
      STPort[p].Type=NewType;
      This->UpdatePortDisplay(p);

      // Don't open devices straight away, everything else is okay
      if (GetPortIOType(NewType)==-1){
        Str ErrText,ErrTitle;
        STPort[p].Create(ErrText,ErrTitle,true);
      }
    }
  }else if (dd->id==2012 || dd->id==2013){
    MACROFILEOPTIONS MFO;
    macro_file_options(MACRO_FILE_GET,This->MacroSel,&MFO);
    if (dd->id==2012) MFO.max_mouse_speed=dd->sl[dd->sel].Data[0];
    if (dd->id==2013) MFO.allow_same_vbls=dd->sl[dd->sel].Data[0];
    macro_file_options(MACRO_FILE_SET,This->MacroSel,&MFO);
  }
  return 0;
}
//---------------------------------------------------------------------------
int TOptionBox::edit_notify_proc(hxc_edit *ed,int Mess,int Inf)
{
  switch (ed->id){
    case 5000:
      if (Mess==EDN_CHANGE) sound_device_name=ed->text;
      if (Mess==EDN_RETURN || Mess==EDN_LOSTFOCUS){
        Sound_Stop();
        Sound_Start();
      }
      break;
    case 100:
      if (Mess==EDN_CHANGE) AutoSnapShotName=ed->text;
      break;
  }
  if (ed->id>=1200 && ed->id<1300){
    if (Mess!=EDN_CHANGE) return 0;

    TOptionBox *This=(TOptionBox*)(ed->owner);
    int p=(ed->id-1200)/20;
    int i=ed->id % 20;
    switch (i){
      case 2:
      {
        int IOType=GetPortIOType(STPort[p].Type);
        if (IOType>=0){
          STPort[p].PortDev[IOType]=ed->text;
          if (STPort[p].IsPCPort()){
            STPort[p].Close();
            This->IOOpenBut[p].set_check(0);
          }
        }
        break;
      }
      case 10:
        STPort[p].PortDev[TPORTIO_TYPE_PIPE]=ed->text;
      case 12:
        if (i==12) STPort[p].LANPipeIn=ed->text;
        if (STPort[p].IsPCPort()){
          STPort[p].Close();
          This->UpdatePortDisplay(p);
        }
        break;
    }
  }
	return 0;
}
//---------------------------------------------------------------------------
int TOptionBox::scrollbar_notify_proc(hxc_scrollbar *SB,int Mess,int I)
{
	TOptionBox *This=(TOptionBox*)(SB->owner);
	if (Mess==SBN_SCROLLBYONE){
		SB->pos+=I;
		SB->rangecheck();
	}else if (Mess==SBN_SCROLL){
		SB->pos=I;
	}else{
		return 0;
	}
	bool UpdatePalette=0;
	switch (SB->id){
		case 0: slow_motion_speed=SB->pos*10 + 10; break;
    case 1: fast_forward_max_speed=1000/(SB->pos+2); break;
    case 2: run_speed_ticks_per_second=100000/(50 + SB->pos*5); break;
    case 10: brightness=SB->pos - 128;UpdatePalette=true; break;
    case 11: contrast=SB->pos - 128;UpdatePalette=true; break;
	}
	SB->draw();
	if (UpdatePalette){
		make_palette_table(brightness,contrast);
		if (flashlight_flag==0) palette_convert_all();
		This->DrawBrightnessBitmap(This->brightness_image);
		This->brightness_picture.draw();
	}

	This->RunSpeedLabel.set_text(T("Run speed")+": "+(100000/run_speed_ticks_per_second)+"%");
	This->SMSpeedLabel.set_text(T("Slow motion speed")+": "+(slow_motion_speed/10)+"%");

  Str Text=T("Maximum fast forward speed")+": ";
  if (fast_forward_max_speed>50){
    Text+=Str((1000/fast_forward_max_speed)*100)+"%";
  }else{
    Text+=T("Unlimited");
    fast_forward_max_speed=0;
  }
	This->FFMaxSpeedLabel.set_text(Text);

	This->brightness_label.set_text(T("Brightness")+": "+brightness);
	This->contrast_label.set_text(T("Contrast")+": "+contrast);

	return 0;
}
//---------------------------------------------------------------------------
int TOptionBox::dir_lv_notify_proc(hxc_dir_lv *lv,int Mess,int i)
{
	TOptionBox *This=(TOptionBox*)(lv->owner);
  switch (Mess){
    case DLVN_SELCHANGE:
    {
      Str new_sel;
      if (i>=0){
        if (lv->sl[i].Data[DLVD_TYPE]==0){ // Up folder
          new_sel=lv->fol+"/..";
        }else{
          new_sel=lv->get_item_path(i);
        }
      }
      if (lv->id==2000){ // Macro
        if (new_sel==This->MacroSel) break;
        This->MacroSel=new_sel;
        This->UpdateMacroRecordAndPlay();
      }else{
        if (new_sel==This->ProfileSel) break;
        This->ProfileSel=new_sel;
        This->UpdateProfileDisplay();
      }
      break;
    }
  }
  return 0;
}
//---------------------------------------------------------------------------

