#define SHORTCUT_ACTION_DD_WID 300
//---------------------------------------------------------------------------
TShortcutBox::TShortcutBox()
{
	CurrentCutSelType=0;
  Section="Shortcuts";
}
//---------------------------------------------------------------------------
void TShortcutBox::Show()
{
  if (StandardShow(600,400,T("Shortcuts"),
      ICO16_CUT,ButtonPressMask,(LPWINDOWPROC)WinProc)) return;

  st_chars_ig.LoadIconsFromMemory(XD,Get_st_charset_bmp(),16,RGB(255,255,255));
	st_chars_ig.IconHeight=16;
	st_chars_ig.NumIcons=256-32;

	int y=10;

  dir_lv.ext_sl.DeleteAll();
  dir_lv.ext_sl.Add(3,T("Parent Directory"),1,1,0);
  dir_lv.ext_sl.Add(3,"",ICO16_FOLDER,ICO16_FOLDERLINK,0);
  dir_lv.ext_sl.Add(3,"stcut",ICO16_CUTON,ICO16_CUTONLINK,0);
  dir_lv.ext_sl.Add(3,"stcut",ICO16_CUTOFF,ICO16_CUTOFFLINK,0);
  dir_lv.lpig=&Ico16;
  dir_lv.base_fol=CutDir;
  dir_lv.fol=CutDir;
  dir_lv.allow_type_change=true;
  dir_lv.show_broken_links=0;
	if (CurrentCutSel.NotEmpty()){
		dir_lv.fol=CurrentCutSel;
		RemoveFileNameFromPath(dir_lv.fol,REMOVE_SLASH);
	}
  dir_lv.create(XD,Handle,10,y,325,120,dir_lv_notify_proc,this);

  new_cut_but.create(XD,Handle,10,y+125,160,25,button_notify_proc,this,
  										BT_TEXT,T("New Shortcuts"),20000,BkCol);

  change_fol_but.create(XD,Handle,10+165,y+125,160,25,button_notify_proc,this,
  										BT_TEXT,T("Change Store Folder"),20001,BkCol);

	help_td.create(XD,Handle,345,y,245,150,BkCol);
	help_td.set_text(T("Note: Shortcuts only work when Steem's main window is activated."));
	y+=160;

  sa_border.create(XD,Handle,9,y,602-20,390-y,
                    NULL,this,BT_GROUPBOX,"",0,BkCol);

	sa.create(XD,sa_border.handle,1,1,sa_border.w-2,sa_border.h-2,NULL,this);

	if (CurrentCutSel.NotEmpty()){
		dir_lv.select_item_by_name(GetFileNameFromPath(CurrentCutSel));
	}
  LoadCutsAndCreateCutControls();

  XMapWindow(XD,Handle);

  if (StemWin) CutBut.set_check(true);
}
//---------------------------------------------------------------------------
void TShortcutBox::AddPickerLine(int p)
{
	if (add_but[0].handle==0){
    add_but[0].create(XD,sa.handle,0,0,(sa.w-10-HXC_SCROLLBAR_WIDTH)/2-5,25,button_notify_proc,this,
                         BT_TEXT,T("Add New"),9998,BkCol);
    add_but[1].create(XD,sa.handle,0,0,(sa.w-10-HXC_SCROLLBAR_WIDTH)/2-5,25,button_notify_proc,this,
                         BT_TEXT,T("Add Copy"),9999,BkCol);
  }
  if (p<0) return;

  int Base=p*100;
  int x=5,y=5+p*30;
  hxc_buttonpicker *p_pick;
  hxc_dropdown *p_dd;

  for (int n=0;n<3;n++){
    p_pick=new hxc_buttonpicker(XD,sa.handle,x,y,80,25,picker_notify_proc,this,Base+n);
    x+=80;
    p_pick->allow_joy=true;
    p_pick->DirID=CurrentCuts[p].Id[n];

    new hxc_button(XD,sa.handle,x,y,10,25,NULL,this,
                          BT_TEXT | BT_STATIC | BT_BORDER_NONE | BT_TEXT_CENTRE,
                          (char*)((n==2) ? "=":"+"),Base+90+n,BkCol);
    x+=10;
  }

  TranslateCutNames();

  p_dd=new hxc_dropdown(XD,sa.handle,x,y,165,300,dd_notify_proc,this);
	p_dd->id=Base+3;
	p_dd->make_empty();
  for (int s=0;s<TranslatedCutNamesSL.NumStrings;s++){
    long i=TranslatedCutNamesSL[s].Data[0];
    switch (i){
  		// Fullscreen not available
    	case 15:case 16:case 17:
    		break;

    	default:
			  p_dd->sl.Add(TranslatedCutNamesSL[s].String,i);
		}
  }
  for (int i=0;i<2;i++){
  	if (p_dd->select_item_by_data(CurrentCuts[p].Action,0)>=0) break;
    p_dd->sl.Add(T("Other"),CurrentCuts[p].Action);
  }
  p_dd->dropped_w=165+5+75+40;
  x+=165+5;

  p_pick=new hxc_buttonpicker(XD,sa.handle,x,y,70,25,
								       picker_notify_proc,this,Base+4);
	p_pick->st_keys_only=true;
	p_pick->DirID=CurrentCuts[p].PressKey;

  p_dd=new hxc_dropdown(XD,sa.handle,x,y,70,400,dd_notify_proc,this);
	p_dd->make_empty();

	DynamicArray<DWORD> Chars;
	GetAvailablePressChars(&Chars);
	for (int i=0;i<Chars.NumItems;i++){
		p_dd->sl.Add("",101+BYTE(HIWORD(Chars[i]))-32,Chars[i]);
	}
 	p_dd->select_item_by_data(CurrentCuts[p].PressChar,1);

	p_dd->lpig=&st_chars_ig;
	p_dd->lv.lpig=&st_chars_ig;
	p_dd->lv.display_mode=1;

	p_dd->id=Base+8;

  hxc_button *p_but=new hxc_button(XD,sa.handle,x,y,70,25,button_notify_proc,
                          this,BT_TEXT,"",Base+9,BkCol);
  SetMacroFileButtonText(p_but,p);
  x+=75;

  new hxc_button(XD,sa.handle,x,y,40,25,button_notify_proc,this,
												BT_TEXT,T("Del"),Base+5,BkCol);

  ShowHidePressSTKeyPicker(p);
}
//---------------------------------------------------------------------------
void TShortcutBox::SetMacroFileButtonText(hxc_button *p_but,int p)
{
  if (CurrentCuts[p].MacroFileIdx>=0){
    Str Text=CurrentCutsStrings[CurrentCuts[p].MacroFileIdx].String;
    Str Name=GetFileNameFromPath(Text);
    char *dot=strrchr(Name,'.');
    if (dot) *dot=0;
    p_but->set_text(Name);
  }else{
    p_but->set_text(T("Choose"));
  }
}
//---------------------------------------------------------------------------
PICKERLINE TShortcutBox::GetLine(int p)
{
	PICKERLINE pl;
	int Base=p*100;
	for (int n=0;n<3;n++){
		pl.p_sign[n]=(hxc_button*)hxc::find(sa.handle,Base+90+n);
		pl.p_id[n]=(hxc_buttonpicker*)hxc::find(sa.handle,Base+n);
	}
	pl.p_action=(hxc_dropdown*)hxc::find(sa.handle,Base+3);
	pl.p_stchar=(hxc_dropdown*)hxc::find(sa.handle,Base+8);
	pl.p_macro=(hxc_button*)hxc::find(sa.handle,Base+9);
	pl.p_del=(hxc_button*)hxc::find(sa.handle,Base+5);
	pl.p_stkey=(hxc_buttonpicker*)hxc::find(sa.handle,Base+4);
	return pl;
}
//---------------------------------------------------------------------------
void TShortcutBox::UpdateAddButsPosition()
{
  add_but[0].x=5,add_but[0].y=5+CurrentCuts.NumItems*30;
  XMoveWindow(XD,add_but[0].handle,add_but[0].x,add_but[0].y);

  add_but[1].x=5+(sa.w-10-HXC_SCROLLBAR_WIDTH)/2+5;
  add_but[1].y=5+CurrentCuts.NumItems*30;
  XMoveWindow(XD,add_but[1].handle,add_but[1].x,add_but[1].y);
}
//---------------------------------------------------------------------------
void TShortcutBox::ShowHidePressSTKeyPicker(int p)
{
  PICKERLINE pl=GetLine(p);
  if (pl.p_stkey==NULL || pl.p_stchar==NULL || pl.p_action==NULL) return;

	XUnmapWindow(XD,pl.p_stkey->handle);
	XUnmapWindow(XD,pl.p_stchar->handle);
	XUnmapWindow(XD,pl.p_macro->handle);

	int w=235;
	if (CurrentCuts[p].Action==CUT_PRESSKEY) w=160;
	if (CurrentCuts[p].Action==CUT_PRESSCHAR) w=160;
	if (CurrentCuts[p].Action==CUT_PLAYMACRO) w=160;
	XResizeWindow(XD,pl.p_action->handle,w,pl.p_action->h);

	if (CurrentCuts[p].Action==CUT_PRESSKEY){
		XMapWindow(XD,pl.p_stkey->handle);
	}else if (CurrentCuts[p].Action==CUT_PRESSCHAR){
		XMapWindow(XD,pl.p_stchar->handle);
	}else if (CurrentCuts[p].Action==CUT_PLAYMACRO){
		XMapWindow(XD,pl.p_macro->handle);
	}
}
//---------------------------------------------------------------------------
void TShortcutBox::Hide()
{
  if (XD==NULL || Handle==0) return;

  StandardHide();

  TranslatedCutNamesSL.DeleteAll();
	st_chars_ig.FreeIcons();

  if (CurrentCutSelType>0 && CurrentCutSel.NotEmpty()){
    SaveShortcutInfo(CurrentCuts,CurrentCutSel);
  }
	LoadAllCuts();

  if (StemWin) CutBut.set_check(0);
}
//---------------------------------------------------------------------------
int TShortcutBox::WinProc(TShortcutBox *This,Window Win,XEvent*Ev)
{
  switch (Ev->type){
    case ClientMessage:
      if (Ev->xclient.message_type==hxc::XA_WM_PROTOCOLS){
        if (Atom(Ev->xclient.data.l[0])==hxc::XA_WM_DELETE_WINDOW){
          This->Hide();
        }
      }
      break;
  }
  return PEEKED_MESSAGE;
}
//---------------------------------------------------------------------------
int TShortcutBox::picker_notify_proc(hxc_buttonpicker *bp,int mess,int i)
{
	TShortcutBox *This=(TShortcutBox*)bp->owner;
  if (mess==BPN_CHANGE){
    int p=bp->id / 100;
    int idn=bp->id % 100;
		if (idn<3){
			CurrentCuts[p].Id[idn]=i;
		}else{
			CurrentCuts[p].PressKey=i;
		}
    CurrentCuts[p].Down=2;
    This->UpdateDisableIfDownLists();
  }else if (mess==BPN_FOCUSCHANGE){
  	if (i==FocusOut){
  		This->help_td.set_text("");
  	}else if (i==FocusIn){
      EasyStr Message;
  		if (bp->st_keys_only==0){
        if (NumJoysticks){
          Message=T("Press any key, the middle mouse button or a joystick button/direction.")+"\n\n";
        }else{
          Message=T("Press any key or the middle mouse button.")+"\n\n";
        }
      }else{
        Message=T("Press a key that was on the ST keyboard or F11, F12, Page Up or Page Down.")+"\n\n";
      }
      Message+=T("Press the pause/break key to clear your selection.");
	 		This->help_td.set_text(Message);
  	}
		This->help_td.draw();
  }
  return 0;
}
//---------------------------------------------------------------------------
void TShortcutBox::DeleteCut(int p)
{
  for (int n=p;n<CurrentCuts.NumItems-1;n++){
  	PICKERLINE pl=GetLine(n);
    for (int i=0;i<3;i++){
      CurrentCuts[n].Id[i]=CurrentCuts[n+1].Id[i];
      pl.p_id[i]->DirID=CurrentCuts[n].Id[i];
    }
    CurrentCuts[n].Action=CurrentCuts[n+1].Action;
    pl.p_action->select_item_by_data(CurrentCuts[n].Action,0);

    CurrentCuts[n].PressKey=CurrentCuts[n+1].PressKey;
    pl.p_stkey->DirID=CurrentCuts[n].PressKey;

    CurrentCuts[n].PressChar=CurrentCuts[n+1].PressChar;
	 	pl.p_stchar->select_item_by_data(CurrentCuts[n].PressChar,1);

    CurrentCuts[n].MacroFileIdx=CurrentCuts[n+1].MacroFileIdx;
    SetMacroFileButtonText(pl.p_macro,n);

    ShowHidePressSTKeyPicker(n);

    pl.p_action->draw();
    for (int i=0;i<3;i++) pl.p_id[i]->draw(true);
    pl.p_stkey->draw(true);

    CurrentCuts[n].Down=CurrentCuts[n+1].Down;
  }
  CurrentCuts.NumItems--;
 	PICKERLINE pl=GetLine(CurrentCuts.NumItems);
	for (int i=0;i<3;i++){
		hxc::destroy(pl.p_id[i]);
		hxc::destroy(pl.p_sign[i]);
	}
  hxc::destroy(pl.p_action);
  hxc::destroy(pl.p_stkey);
	hxc::destroy(pl.p_stchar);
	hxc::destroy(pl.p_macro);
	hxc::destroy(pl.p_del);

  UpdateAddButsPosition();
  int old_sy=sa.sy;
  sa.adjust();
  sa.scrollto(0,old_sy);
  UpdateDisableIfDownLists();
}
//---------------------------------------------------------------------------
int TShortcutBox::button_notify_proc(hxc_button *But,int Mess,int *Inf)
{
	TShortcutBox *This=(TShortcutBox*)But->owner;

  if (Mess==BN_CLICKED){
    if (But->id==9998 || But->id==9999){
  		if (This->CurrentCutSelType<=0) return 0;
  			
  		SHORTCUTINFO si;
      if ((But->id & 1) && CurrentCuts.NumItems){ // Copy
        si=CurrentCuts[CurrentCuts.NumItems-1];
        if (si.MacroFileIdx>=0){
          Str MacroFile=CurrentCutsStrings[si.MacroFileIdx].String;
          si.MacroFileIdx=CurrentCutsStrings.Add(MacroFile);
        }
      }else{
        ClearSHORTCUTINFO(&si);
      }
      si.pESL=&CurrentCutsStrings;
      CurrentCuts.Add(si);

      This->AddPickerLine(CurrentCuts.NumItems-1);
      This->UpdateAddButsPosition();
      This->sa.adjust();
      This->sa.scrollto(0,CurrentCuts.NumItems*30+25-This->sa.h);
      This->UpdateDisableIfDownLists();
    }else if (But->id<20000){
      if ((But->id % 100)==5){
        This->DeleteCut(But->id/100);
      }else if ((But->id % 100)==9){
        But->set_check(true);

        int Num=But->id/100;
        int StrIdx=CurrentCuts[Num].MacroFileIdx;
        Str CurFile;
        if (StrIdx>=0) CurFile=CurrentCutsStrings[StrIdx].String;

        Str NewFile=This->ChooseMacro(CurFile);
        SetForegroundWindow(This->Handle);
        if (NewFile.NotEmpty()){
          if (StrIdx>=0){
            CurrentCutsStrings.SetString(StrIdx,NewFile);
          }else{
            CurrentCuts[Num].MacroFileIdx=CurrentCutsStrings.Add(NewFile);
          }
          This->SetMacroFileButtonText(But,Num);
        }
        But->set_check(0);
      }
    }else if (But->id==20000){
      hxc_prompt prompt;
      EasyStr new_name=prompt.ask(XD,T("New Shortcuts"),T("Enter Name"));
      if (new_name.NotEmpty()){
        EasyStr new_path=GetUniquePath(This->dir_lv.fol,new_name+".stcut");
        FILE *f=fopen(new_path,"wb");
        if (f){
          fclose(f);
          This->dir_lv.refresh_fol();
      		This->dir_lv.select_item_by_name(GetFileNameFromPath(new_path));
          This->ChangeCutFile(new_path,1,true);
        }
      }
    }else if (But->id==20001){
      fileselect.set_corner_icon(&Ico16,ICO16_FOLDER);
      EasyStr new_path=fileselect.choose(XD,This->CutDir,"",T("Pick a Folder"),
        FSM_CHOOSE_FOLDER | FSM_CONFIRMCREATE,folder_parse_routine,"");
      if (new_path.NotEmpty()){
        NO_SLASH(new_path);
        This->CutDir=new_path;
        CutFiles.DeleteAll();
        This->LoadAllCuts();

        This->dir_lv.base_fol=This->CutDir;
        This->dir_lv.fol=This->CutDir;
        This->dir_lv.refresh_fol();
      }
    }
  }
	return 0;
}
//---------------------------------------------------------------------------
int TShortcutBox::dd_notify_proc(hxc_dropdown *DD,int Mess,int Inf)
{
	TShortcutBox *This=(TShortcutBox*)DD->owner;
  if (DD->id<20000){
    int p=DD->id / 100;
    if (Mess==DDN_DROPWHERE){
      dd_drop_position *dp=(dd_drop_position*)Inf;
      dp->parent=This->Handle;
      dp->x=(DD->x + (This->sa_border.x+1)) - This->sa.sx;
      dp->y=(DD->y + (This->sa_border.y+1)) - This->sa.sy;
      if ((DD->id % 100)==3) dp->w=SHORTCUT_ACTION_DD_WID;
      dp->h=300;
      return 1;
    }else if (Mess==DDN_SELCHANGE){
      if ((DD->id % 100)==3){ //Action DD
        CurrentCuts[p].Action=DD->sl[DD->sel].Data[0];
		    This->ShowHidePressSTKeyPicker(p);
      }else if ((DD->id % 100)==8){ //PressChar
        CurrentCuts[p].PressChar=DD->sl[DD->sel].Data[1];
      }
    }
  }
	return 0;
}

void TShortcutBox::LoadCutsAndCreateCutControls()
{
  AddPickerLine(-1); // create add buts
	LoadAllCuts();
  for (int i=0;i<CurrentCuts.NumItems;i++) AddPickerLine(i);
  UpdateAddButsPosition();
	sa.adjust();
	sa.scrollto(0,0);
}

void TShortcutBox::ChangeCutFile(Str new_sel,int new_type,bool save_old)
{
  if (CurrentCutSel.NotEmpty() && CurrentCutSelType>0 && save_old){
    SaveShortcutInfo(CurrentCuts,CurrentCutSel);
  }

  hxc::destroy_children_of(sa.handle);

  CurrentCutSel=new_sel;
  CurrentCutSelType=new_type;

  LoadCutsAndCreateCutControls();
}

int TShortcutBox::dir_lv_notify_proc(hxc_dir_lv *lv,int Mess,int i)
{
	TShortcutBox *This=(TShortcutBox*)(lv->owner);
	switch (Mess){
		case DLVN_SELCHANGE:
		{
      Str new_sel;
      int new_type=0;
      if (i>=0){
	      new_type=lv->sl[i].Data[DLVD_TYPE]-1;
        if (new_type==-1){ // Up folder
        	new_sel=lv->fol+"/..";
        }else{
        	new_sel=lv->get_item_path(i);
        }
	    }
	    if (new_sel==This->CurrentCutSel) break;

      This->ChangeCutFile(new_sel,new_type,true);
			break;
		}
    case DLVN_NAMECHANGED:
    {
      Str new_name=lv->get_item_path(i);
      if (This->CurrentCutSelType==2){
        for (int i=0;i<CutFiles.NumStrings;i++){
          if (IsSameStr_I(CutFiles[i].String,This->CurrentCutSel)) CutFiles.Delete(i--);
        }
        CutFiles.Add(new_name);
      }
      This->CurrentCutSel=new_name;
      break;
    }

    case DLVN_GETTYPE:
      return int((CutFiles.FindString((char*)i)>-1) ? 3:2);
    case DLVN_TYPECHANGE:
    {
     	EasyStr changed=lv->get_item_path(i,true);
      int new_type=lv->sl[i].Data[DLVD_TYPE]-1;
      for (int i=0;i<CutFiles.NumStrings;i++){
        if (IsSameStr_I(CutFiles[i].String,changed)) CutFiles.Delete(i--);
      }
      if (new_type==2) CutFiles.Add(changed);
      if (IsSameStr_I(changed,This->CurrentCutSel)) This->CurrentCutSelType=new_type;
      This->LoadAllCuts(0);
      break;
    }
    case DLVN_FOLDERMOVED:
    case DLVN_ITEMDELETED:
    {
      char *path=(char*)i;
      for (int i=0;i<CutFiles.NumStrings;i++){
        if (strstr(CutFiles[i].String,path)){
          if (Mess==DLVN_FOLDERMOVED){
            Str new_path=CutFiles[i].String;
            new_path.Delete(0,strlen(path));
            new_path.Insert(path+strlen(path)+1,0);
            CutFiles.SetString(i,new_path);
          }else{
            CutFiles.Delete(i--);
          }
        }
      }
      break;
    }
	}
  return 0;
}
//---------------------------------------------------------------------------
Str TShortcutBox::ChooseMacro(Str Current)
{
  int dlv_h=200;
  int w=300,h=10+dlv_h+10;

  Window handle=hxc::create_modal_dialog(XD,w,h,T("Choose a Macro"),true);
  if (handle==0) return "";

  int y=10;

  hxc_dir_lv dlv;
  dlv.ext_sl.Add(3,T("Parent Directory"),1,1,0);
  dlv.ext_sl.Add(3,"",ICO16_FOLDER,ICO16_FOLDERLINK,0);
  dlv.ext_sl.Add(3,"stmac",ICO16_MACROS,ICO16_MACROLINK,0);
  dlv.lpig=&Ico16;
  dlv.base_fol=OptionBox.MacroDir;
  dlv.fol=OptionBox.MacroDir;
  dlv.allow_type_change=0;
  dlv.show_broken_links=0;
  dlv.choose_only=true;
	if (Current.NotEmpty()){
		dlv.fol=Current;
		RemoveFileNameFromPath(dlv.fol,REMOVE_SLASH);
	}
  dlv.create(XD,handle,10,y,w-20,dlv_h,NULL,NULL);
  dlv.select_item_by_name(GetFileNameFromPath(Current));
  if (dlv.lv.sel<0) dlv.lv.changesel(0);
  y+=dlv_h+10;

  EasyStr ret;
  bool show=true;
  for (;;){
    int chosen=hxc::show_modal_dialog(XD,handle,show);
    if (chosen!=1) break;

    ret=dlv.get_item_path(dlv.lv.sel);
    if (ret.NotEmpty()) break;
    show=0;
  }

  hxc::destroy_modal_dialog(XD,handle);
  return ret;
}
//---------------------------------------------------------------------------

