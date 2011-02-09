//---------------------------------------------------------------------------
TDiskManager::TDiskManager()
{
  DisksFol="///";
  HomeFol="///";

  Width=500;Height=400;
	HistBackLength=0;
	HistForwardLength=0;

  HideBroken=0;
  BytesPerSectorIdx=2;SecsPerTrackIdx=1;TracksIdx=5;SidesIdx=1;
  DoubleClickAction=2;
  EjectDisksWhenQuit=0;

  ContentConflictAction=2;

  Section="Disks";
}
//---------------------------------------------------------------------------
int TDiskManager::dir_lv_notify_handler(hxc_dir_lv *dlv,int mess,int i)
{
  TDiskManager *This=(TDiskManager*)dlv->owner;
  if (mess==DLVN_FOLDERCHANGE){
    This->set_path((char*)i,true,0);
    return 0;
  }else if (mess==DLVN_DOUBLECLICK || mess==DLVN_RETURN){
    if (This->DoubleClickAction==0 || i<0) return 0;

    EasyStr file=dlv->get_item_path(i);
    if ((GetFileAttributes(file) & FILE_ATTRIBUTE_DIRECTORY)==0){
      int action=2;
      if (This->DoubleClickAction==1) action=0;
      This->PerformInsertAction(action,dlv->get_item_name(i),file,"");
      return 1; // Don't focus the listview
    }
  }else if (mess==DLVN_DROP){
  	hxc_listview_drop_struct *ds=(hxc_listview_drop_struct*)i;
    EasyStr file=dlv->get_item_path(ds->dragged);
    int type=dlv->sl[ds->dragged].Data[DLVD_TYPE];

    if (dlv->lv.is_dropped_in(ds,&(This->HomeBut))){
    }else if (dlv->lv.is_dropped_in(ds,&(This->disk_name[0])) ||
              dlv->lv.is_dropped_in(ds,&(This->drive_icon[0]))){
      if (type>=2){
        This->PerformInsertAction(0,dlv->get_item_name(ds->dragged),file,"");
      }
    }else if (dlv->lv.is_dropped_in(ds,&(This->disk_name[1])) ||
              dlv->lv.is_dropped_in(ds,&(This->drive_icon[1]))){
      if (type>=2){
        This->PerformInsertAction(1,dlv->get_item_name(ds->dragged),file,"");
      }
    }
  }else if (mess==DLVN_CONTEXTMENU){
    dlv->pop.lpig=&Ico16;
    if (i>=0){
      EasyStr file=dlv->get_item_path(i);
      int is_link=dlv->sl[i].Data[DLVD_FLAGS] & DLVF_LINKMASK;
      int type=dlv->sl[i].Data[DLVD_TYPE];
      bool is_zip=type >= This->ArchiveTypeIdx;
      bool read_only=(dlv->sl[i].Data[DLVD_FLAGS] & DLVF_READONLY)!=0;
      int pos=0;
      if (is_link<2){ // 2 = dead link
        if (type>=2){ // File
          dlv->pop.menu.InsertAt(pos++,2,StripAndT("Insert Into Drive &A"),ICO16_INSERTDISK,1010);
          dlv->pop.menu.InsertAt(pos++,2,StripAndT("Insert Into Drive &B"),ICO16_INSERTDISK,1011);
          dlv->pop.menu.InsertAt(pos++,2,StripAndT("Insert, Reset and &Run"),ICO16_INSERTDISK,1012);
          dlv->pop.menu.InsertAt(pos++,2,"-",-1,0);
          dlv->pop.menu.InsertAt(pos++,2,StripAndT("Get &Contents"),-1,1015);
          dlv->pop.menu.InsertAt(pos++,2,"-",-1,0);
          if (is_link){
            dlv->pop.menu.InsertAt(pos++,2,StripAndT("&Go To Disk"),ICO16_FORWARD,1090);
            dlv->pop.menu.InsertAt(pos++,2,StripAndT("Open Disk's Folder in File Manager"),-1,1092);
            dlv->pop.menu.InsertAt(pos++,2,"-",-1,0);
          }
          if (is_zip==0){
            dlv->pop.menu.InsertAt(pos++,2,StripAndT("Read &Only"),
                                    (read_only ? ICO16_TICKED:ICO16_UNTICKED),1040);
            dlv->pop.menu.InsertAt(pos++,2,"-",-1,0);
          }else{
            dlv->pop.menu.InsertAt(pos++,2,StripAndT("E&xtract Disk Here"),ICO16_ZIP_RW,1080);
            dlv->pop.menu.InsertAt(pos++,2,"-",-1,0);
          }
        }else{ // Folder
          int pos=0;
          dlv->pop.menu.InsertAt(pos++,2,StripAndT("Open in File Manager"),-1,1060);
          dlv->pop.menu.InsertAt(pos++,2,StripAndT("&Find..."),-1,1061);
          dlv->pop.menu.InsertAt(pos++,2,"-",-1,0);
        }
      }
    }
    int pos=dlv->pop.menu.NumStrings-1; // New folder
    dlv->pop.menu[pos].Data[0]=ICO16_FOLDER;
    dlv->pop.menu.InsertAt(pos++,2,StripAndT("Open Current Folder In File Manager"),-1,2003);
    dlv->pop.menu.InsertAt(pos++,2,StripAndT("Find In Current Folder"),-1,2010);
    dlv->pop.menu.InsertAt(pos++,2,"-",-1,0);  
    dlv->pop.menu.Add(StripAndT("New Standard &Disk Image"),ICO16_DISK,1001);
    dlv->pop.menu.Add(StripAndT("New Custom Disk &Image"),ICO16_DISK,1002);
  }else if (mess==DLVN_POPCHOOSE){
    if (dlv->pop.menu[i].NumData<2) return 0;

    int action=dlv->pop.menu[i].Data[1];
    switch (action){
      case 1010:
      case 1011:
      case 1012:
        This->PerformInsertAction(action-1010,
                    dlv->get_item_name(dlv->lv.sel),
                    dlv->get_item_path(dlv->lv.sel),"");
        return 0;
      case 1015:
        This->GetContentsSL(dlv->get_item_path(dlv->lv.sel));
        if (This->contents_sl.NumStrings){
          This->ContentsLinksPath=This->DisksFol;
          This->ShowContentDiag();
        }
        return 0;
      case 1040:
        This->ToggleReadOnly(dlv->lv.sel);
        return 0;
      case 1060:case 1092:
      {
        Str fol=dlv->get_item_path(dlv->lv.sel);
        if (action==1092) RemoveFileNameFromPath(fol,KEEP_SLASH);
        shell_execute(Comlines[COMLINE_FM],Str("[PATH]\n")+fol);
        return 0;
      }
      case 1061:
        shell_execute(Comlines[COMLINE_FIND],Str("[PATH]\n")+dlv->get_item_path(dlv->lv.sel));
        return 0;
      case 1080:
        This->ExtractDisks(dlv->get_item_path(dlv->lv.sel));
        return 0;
      case 1090:
      {
        Str File=dlv->get_item_path(dlv->lv.sel,true);
        Str DiskFol=File;
        RemoveFileNameFromPath(DiskFol,REMOVE_SLASH);
        This->set_path(DiskFol);
        dlv->select_item_by_name(GetFileNameFromPath(File));
        return 0;
      }
      case 1001:
      case 1002:
      {
        EasyStr STName;
        int sectors=1440,secs_per_track=9,sides=2;
        if (action==1002){
          STName=This->GetCustomDiskImage(&sectors,&secs_per_track,&sides);
        }else{
          hxc_prompt prompt;
          STName=prompt.ask(XD,T("Blank Disk"),T("Enter Name"));
        }
        if (STName.NotEmpty()){
          STName=GetUniquePath(This->DisksFol,STName+".st");
          if (This->CreateDiskImage(STName,sectors,secs_per_track,sides)){
            This->RefreshDiskView(STName);
          }else{
            Alert(Str(T("Could not create the disk image "))+STName,
                      T("Error"),MB_ICONEXCLAMATION);
          }
        }
        return 0;
      }
      case 2003:
        shell_execute(Comlines[COMLINE_FM],Str("[PATH]\n")+This->DisksFol);
        return 0;
      case 2010:
        shell_execute(Comlines[COMLINE_FIND],Str("[PATH]\n")+This->DisksFol);
        return 0;
    }
  }else if (mess==DLVN_CONTENTSCHANGE){
    dlv_ccn_struct *p_ccn=(dlv_ccn_struct*)i;
    if (p_ccn->time==DLVCCN_BEFORE){
      This->TempEject_InDrive[0]=0;
      This->TempEject_InDrive[1]=0;
      for (int d=0;d<2;d++){
        // Should check whether in folder being deleted too.
        if (IsSameStr_I(FloppyDrive[d].GetDisk(),p_ccn->path)){
          This->TempEject_InDrive[d]=true;
          This->TempEject_Name=FloppyDrive[d].DiskName;
          This->TempEject_DiskInZip[d]=FloppyDrive[d].DiskInZip;
          FloppyDrive[d].RemoveDisk();
        }
      }
    }else if (p_ccn->time==DLVCCN_AFTER){
      Str new_path=p_ccn->path;
      Str new_name=This->TempEject_Name;
      if (p_ccn->success){
        if (p_ccn->action==DLVCCN_DELETE){
          This->UpdateDiskNames(0);
          This->UpdateDiskNames(1);
          return 0;
        }else if (p_ccn->action==DLVCCN_MOVE || p_ccn->action==DLVCCN_RENAME){
          new_path=p_ccn->new_path;
          if (p_ccn->action==DLVCCN_RENAME){
            new_name=GetFileNameFromPath(new_path);
            if (p_ccn->flags & DLVF_EXTREMOVED){
              char *dot=strrchr(new_name,'.');
              if (dot) *dot=0;
            }
          }
        }
      }
      for (int d=0;d<2;d++){
        if (This->TempEject_InDrive[d]){
          This->InsertDisk(d,new_name,new_path,0,0,This->TempEject_DiskInZip[d]);
        }
      }
    }
    return 0;
  }
  return 0;
}
//---------------------------------------------------------------------------
void TDiskManager::UpdateDiskNames(int d)
{
  if (XD==NULL || Handle==0) return;

  if (FloppyDrive[d].DiskInDrive()){
    Str RO;
    if (FloppyDrive[d].ReadOnly) RO=" <RO>";
    disk_name[d].set_text(FloppyDrive[d].DiskName+RO);
  }else{
    disk_name[d].set_text("");
  }
}
//---------------------------------------------------------------------------
void TDiskManager::set_home(Str fol)
{
  if (Alert(fol+"/"+"\n\n"+
        T("Are you sure you want to make this folder your new home folder?"),
        T("Change Home Folder?"),MB_YESNO | MB_ICONQUESTION)==IDYES){
    HomeFol=fol;
  }
}
//---------------------------------------------------------------------------
int TDiskManager::button_notify_handler(hxc_button *But,int mess,int *Inf)
{
  TDiskManager *This=(TDiskManager*)GetProp(But->XD,But->parent,cWinThis);
  if (mess==BN_CLICKED){
    switch (But->id){
      case 2: //back
      	if (This->HistBackLength > 0){
          for (int n=9;n>0;n--){
            This->HistForward[n]=This->HistForward[n-1];
          }
          This->HistForward[0]=This->DisksFol;
          if((This->HistForwardLength)<10)This->HistForwardLength++;

          This->set_path(This->HistBack[0],0);
          for (int n=0;n<9;n++){
            This->HistBack[n]=This->HistBack[n+1];
          }
          This->HistBack[9]="";
          This->HistBackLength--;
				}
      	break;
      case 3: //forward
      	if (This->HistForwardLength > 0){
          for (int n=9;n>0;n--){
            This->HistBack[n]=This->HistBack[n-1];
          }
          if((This->HistBackLength)<10)This->HistBackLength++;
          This->HistBack[0]=This->DisksFol;
          This->set_path(This->HistForward[0],0);
          for (int n=0;n<9;n++){
            This->HistForward[n]=This->HistForward[n+1];
          }
          This->HistForward[9]="";
          This->HistForwardLength--;
				}
      	break;
      case 4: //go home
      {
        bool at_home=IsSameStr(This->HomeFol.Text,This->DisksFol.Text);
        if (Inf[0]!=Button1 || at_home){
          But->set_check(true);
          pop.lpig=&Ico16;
          pop.menu.DeleteAll();
          if (at_home==0){
            pop.menu.Add(This->HomeFol,ICO16_HOMEFOLDER,4000);
            pop.menu.Add("-",-1);
          }
          for (int i=0;i<10;i++){
            pop.menu.Add(Str(i+1)+": "+This->QuickFol[i],ICO16_FOLDER,4010+i);
          }
          pop.create(XD,But->handle,0,But->h,This->menu_popup_notifyproc,This);
        }else{
          This->set_path(This->HomeFol,true);
        }
        break;
      }
      case 5: //set home
      {
        bool at_home=IsSameStr(This->HomeFol.Text,This->DisksFol.Text);
        if (Inf[0]!=Button1 || at_home){
          But->set_check(true);
          pop.lpig=&Ico16;
          pop.menu.DeleteAll();
          if (at_home==0){
            pop.menu.Add(Str("(")+This->HomeFol+")",ICO16_HOMEFOLDER,4100);
            pop.menu.Add("-",-1);
          }
          for (int i=0;i<10;i++){
            pop.menu.Add(Str(i+1)+": ("+This->QuickFol[i]+")",ICO16_FOLDER,4110+i);
          }
          pop.create(XD,But->handle,0,But->h,This->menu_popup_notifyproc,This);
        }else{
          This->set_home(This->DisksFol);
        }
      	break;
			}
			case 6:
      {
    		But->set_check(true);
    		pop.lpig=&Ico16;
      	pop.menu.DeleteAll();
        pop.menu.Add(StripAndT("Disconnect Drive B"),
        	int((num_connected_floppies==1) ? ICO16_TICKED:ICO16_UNTICKED),2012);
        pop.menu.Add(StripAndT("Accurate Disk Access Times (Slow)"),
        	int((floppy_instant_sector_access==0) ? ICO16_TICKED:ICO16_UNTICKED),2013);
        pop.menu.Add(StripAndT("Read/Write Archives (Changes Lost On Eject)"),
        	int(FloppyArchiveIsReadWrite ? ICO16_TICKED:ICO16_UNTICKED),2014);

        pop.menu.Add("-",-1,0);
        pop.menu.Add(StripAndT("Search Disk Image Database"),-1,2025);
        pop.menu.Add(StripAndT("Open Current Folder In File Manager"),-1,2003);
        pop.menu.Add(StripAndT("Find In Current Folder"),-1,2010);
        pop.menu.Add("-",-1,0);
        pop.menu.Add(StripAndT("Automatically Insert &Second Disk"),int(This->AutoInsert2 ? ICO16_TICKED:ICO16_UNTICKED),2016);
//        pop.menu.Add(StripAndT("Hide Dangling Links"),int(This->HideBroken),2002);
        pop.menu.Add(StripAndT("E&ject Disks When Quit"),
        	int(This->EjectDisksWhenQuit ? ICO16_TICKED:ICO16_UNTICKED),2011);
        pop.menu.Add("-",-1,0);
        int idx=pop.menu.NumStrings;
        pop.menu.Add(StripAndT("Double Click On Disk Does &Nothing"),ICO16_UNRADIOMARKED,2007);
        pop.menu.Add(StripAndT("Double Click On Disk Inserts In &Drive A"),ICO16_UNRADIOMARKED,2008);
        pop.menu.Add(StripAndT("Double Click On Disk Inserts, &Resets and Runs"),ICO16_UNRADIOMARKED,2009);
        pop.menu[idx+This->DoubleClickAction].Data[0]=ICO16_RADIOMARK;
        pop.menu.Add("-",-1,0);
        pop.menu.Add(StripAndT("&Close Disk Manager After Insert, Reset and Run"),
                  int(This->CloseAfterIRR ? ICO16_TICKED:ICO16_UNTICKED),2015);
        pop.create(XD,But->handle,0,But->h,This->menu_popup_notifyproc,This);
				break;
      }
			case 10:
				HardDiskMan.Show();
				break;
			case 101:
				This->SetNumFloppies(3-num_connected_floppies);
				break;
      case 200:case 201:
      {
        if (Inf[0]!=Button3 && Inf[0]!=Button2) break;

        pop.lpig=&Ico16;
        pop.menu.DeleteAll();

        int d=But->id-200;
        bool added_line=true;
        if (FloppyDrive[d].NotEmpty()){
          if (FloppyDrive[d].IsZip()==0){
            int ico=ICO16_UNTICKED;
            if (FloppyDrive[d].ReadOnly) ico=ICO16_TICKED;
            pop.menu.Add(StripAndT("Read &Only"),ico,1040,(-d)-1);
          }
        }
        if (FloppyDrive[0].NotEmpty() || FloppyDrive[1].NotEmpty()){
          pop.menu.Add(StripAndT("&Swap"),-1,1100);
          added_line=0;
        }
        if (FloppyDrive[d].NotEmpty()){
          pop.menu.Add(StripAndT("&Remove Disk From Drive"),ICO16_EJECTDISK,1101,d);
          pop.menu.Add(StripAndT("&Go To Disk"),-1,1090,d);
        }
        EasyStr CurrentDiskName=This->CreateDiskName(FloppyDrive[d].DiskName,FloppyDrive[d].DiskInZip);
        for (int n=0;n<10;n++){
          if (This->InsertHist[d][n].Path.NotEmpty()){
            EasyStr MenuItemText=This->CreateDiskName(This->InsertHist[d][n].Name,This->InsertHist[d][n].DiskInZip);
            if (NotSameStr_I(CurrentDiskName,MenuItemText)){
              if (added_line==0){
                pop.menu.Add("-",-1,0,0);
                added_line=true;
              }
              pop.menu.Add(MenuItemText,ICO16_INSERTDISK,3000+n,d);
            }
          }
        }
        if (pop.menu.NumStrings) pop.create(XD,0,POP_CURSORPOS,0,This->menu_popup_notifyproc,This);
        break;
      }
      case 302:case 303:
        This->EjectDisk(But->id & 1);
        break;
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
int TDiskManager::menu_popup_notifyproc(hxc_popup *pPop,int mess,int i)
{
  TDiskManager *This=(TDiskManager*)(pPop->owner);
	int id=pop.menu[i].Data[1];
	if (mess==POP_CHOOSE){
		switch (id){
      case 2002:
        This->HideBroken=!This->HideBroken;
        This->dir_lv.show_broken_links=(This->HideBroken==0);
        This->RefreshDiskView();
        break;
      case 2007:case 2008:case 2009:
        This->DoubleClickAction=id-2007;
        break;
      case 2011:
        This->EjectDisksWhenQuit=!This->EjectDisksWhenQuit;
        break;
      case 2012:
				This->SetNumFloppies(3-num_connected_floppies);
        break;
      case 2013:
        floppy_instant_sector_access=!floppy_instant_sector_access;
        CheckResetDisplay();
        break;
      case 2014:
      {
        FloppyArchiveIsReadWrite=!FloppyArchiveIsReadWrite;
        if (FloppyDrive[0].IsZip()) FloppyDrive[0].ReinsertDisk();
        if (FloppyDrive[1].IsZip()) FloppyDrive[1].ReinsertDisk();
        int zipicon=ICO16_ZIP_RO;
        if (FloppyArchiveIsReadWrite) zipicon=ICO16_ZIP_RW;
        for (int i=This->ArchiveTypeIdx;i<This->dir_lv.ext_sl.NumStrings;i++){
          This->dir_lv.ext_sl[i].Data[0]=zipicon;
        }
        This->UpdateDiskNames(0);
        This->UpdateDiskNames(1);
        This->RefreshDiskView();
        break;
      }
      case 2015:
        This->CloseAfterIRR=!This->CloseAfterIRR;
        break;
      case 2016:
        This->AutoInsert2=!This->AutoInsert2;
        break;
      case 2025:
        This->ShowDatabaseDiag();
        break;
      case 1040:  // Toggle Read-Only
        This->ToggleReadOnly(pop.menu[i].Data[2]);
        break;
      case 1100:
        This->SwapDisks(0);
        break;
      case 1101:
        This->EjectDisk(pop.menu[i].Data[2]);
        break;
      case 1090:
      {
        int d=pop.menu[i].Data[2]; // Get index
        EasyStr DiskFol=FloppyDrive[d].GetDisk();
        RemoveFileNameFromPath(DiskFol,REMOVE_SLASH);
        This->set_path(DiskFol);
        This->dir_lv.select_item_by_name(GetFileNameFromPath(FloppyDrive[d].GetDisk()));
        break;
      }
      case 2003:
        shell_execute(Comlines[COMLINE_FM],Str("[PATH]\n")+This->DisksFol);
        break;
      case 2010:
        shell_execute(Comlines[COMLINE_FIND],Str("[PATH]\n")+This->DisksFol);
        break;
		}
    if (id>=3000 && id<3030){
      int disk=pop.menu[i].Data[2];
      int n=id-3000;
      This->InsertDisk(disk,This->InsertHist[disk][n].Name,This->InsertHist[disk][n].Path,
                        0,true,This->InsertHist[disk][n].DiskInZip,0,true);
    }else if (id>=4000 && id<4100){
      if (id==4000){
        This->set_path(This->HomeFol,true);
      }else{
        id-=4010;
        if (This->QuickFol[id].NotEmpty()) This->set_path(This->QuickFol[id],true);
      }
    }else if (id>=4100 && id<4200){
      if (id==4100){
        This->set_home(This->DisksFol);
      }else{
        id-=4110;
        This->QuickFol[id]=This->DisksFol;
      }
    }
	}
  This->MenuBut.set_check(0);
  This->HomeBut.set_check(0);
  This->SetHomeBut.set_check(0);
	return 0;
}
//---------------------------------------------------------------------------
void TDiskManager::ToggleReadOnly(int i)
{
  EasyStr DiskPath;
  if (i<0){
    DiskPath=FloppyDrive[-(i+1)].GetDisk();
  }else{
    DiskPath=dir_lv.get_item_path(i,true);
  }

  bool InDrive[2]={0,0};
  EasyStr OldName[2],DiskInZip[2];
  for (int d=0;d<2;d++){
    if (IsSameStr_I(FloppyDrive[d].GetDisk(),DiskPath)){
      InDrive[d]=true;
      OldName[d]=FloppyDrive[d].DiskName;
      DiskInZip[d]=FloppyDrive[d].DiskInZip;
      FloppyDrive[d].RemoveDisk();
    }
  }
  DWORD Attrib=GetFileAttributes(DiskPath);
  if (Attrib & FILE_ATTRIBUTE_READONLY){
    SetFileAttributes(DiskPath,Attrib & ~FILE_ATTRIBUTE_READONLY);
  }else{
    SetFileAttributes(DiskPath,Attrib | FILE_ATTRIBUTE_READONLY);
  }

  if (InDrive[0]) InsertDisk(0,OldName[0],DiskPath,0,0,DiskInZip[0]);
  if (InDrive[1]) InsertDisk(1,OldName[1],DiskPath,0,0,DiskInZip[1]);

  RemoveFileNameFromPath(DiskPath,REMOVE_SLASH);
  if (IsSameStr(DiskPath,DisksFol)) RefreshDiskView();
}
//---------------------------------------------------------------------------
void TDiskManager::RefreshDiskView(Str sel)
{
	set_path(DisksFol,0);
  if (sel.NotEmpty()){
    dir_lv.select_item_by_name(GetFileNameFromPath(sel));
  }
}
//---------------------------------------------------------------------------
void TDiskManager::set_path(EasyStr new_path,bool add_to_history,bool change_dir_lv)
{
  if (add_to_history){
    HistForward[0]="";
    HistForwardLength=0;
    for (int n=9;n>0;n--) HistBack[n]=HistBack[n-1];
    HistBack[0]=DisksFol;
    if (HistBackLength<10) HistBackLength++;
  }

	NO_SLASH(new_path);
  if (change_dir_lv){
    dir_lv.fol=new_path;
    dir_lv.refresh_fol();
  }
  DisksFol=new_path;
  DirOutput.set_text(DisksFol);
}
//---------------------------------------------------------------------------
void TDiskManager::Show()
{
  if (Handle) return;

  if (StandardShow(Width,Height,T("Disk Manager"),
        ICO16_DISKMAN,ExposureMask | StructureNotifyMask,
        (LPWINDOWPROC)WinProc,true)) return;

  SetWindowNormalSize(XD,Handle,10+32+10+10+10+60+60+10,110+50+10);

  int y=10;
  for(int d=0;d<2;d++){
    drive_icon[d].create(XD,Handle,10,y-2,32,32,button_notify_handler,this,
                          BT_ICON | BT_STATIC | BT_BORDER_NONE | BT_TEXT_CENTRE,
                          EasyStr(char('A'+d)),100+d,BkCol);
    drive_icon[d].set_icon(&Ico32,ICO32_DRIVE_A+d);

    disk_name[d].create(XD,Handle,43,y,320,25,button_notify_handler,this,
                          BT_TEXT | BT_STATIC | BT_TEXT_CENTRE | BT_BORDER_INDENT,
                          "",200+d,WhiteCol);

    eject_but[d].create(XD,Handle,Width-103,y+1,25,25,
              button_notify_handler,this,BT_ICON,"Eject",302+d,BkCol);
	  eject_but[d].set_icon(&Ico16,ICO16_EJECTDISK);
	
    SetWindowGravity(XD,eject_but[d].handle,NorthEastGravity);
    y+=34;
  }

  HardBut.create(XD,Handle,Width-70,10,60,60,
              button_notify_handler,this,BT_ICON,"",10,BkCol);
  if (IsSameStr_I(T("File"),"Fichier")){
    HardBut.set_icon(&Ico64,ICO64_HARDDRIVES_FR);
  }else{
    HardBut.set_icon(&Ico64,ICO64_HARDDRIVES);
  }
  SetWindowGravity(XD,HardBut.handle,NorthEastGravity);
  hints.add(HardBut.handle,T("Hard Drive Manager"),Handle);


  BackBut.create(XD,Handle,10,82,21,21,
              button_notify_handler,this,BT_ICON,"",2,BkCol);
  BackBut.set_icon(&Ico16,ICO16_BACK);
  hints.add(BackBut.handle,T("Back"),Handle);

  ForwardBut.create(XD,Handle,35,82,21,21,
              button_notify_handler,this,BT_ICON,"",3,BkCol);
  ForwardBut.set_icon(&Ico16,ICO16_FORWARD);
  hints.add(ForwardBut.handle,T("Forward"),Handle);

  HomeBut.create(XD,Handle,60,82,21,21,
              button_notify_handler,this,BT_ICON,"",4,BkCol);
  HomeBut.set_icon(&Ico16,ICO16_HOMEFOLDER);
  hints.add(HomeBut.handle,T("To home folder"),Handle);

  SetHomeBut.create(XD,Handle,85,82,21,21,
              button_notify_handler,this,BT_ICON,"",5,BkCol);
  SetHomeBut.set_icon(&Ico16,ICO16_SETHOMEFOLDER);
  hints.add(SetHomeBut.handle,T("Make this folder your home folder"),Handle);

  MenuBut.create(XD,Handle,110,82,21,21,
              button_notify_handler,this,BT_ICON,"",6,BkCol);
  MenuBut.set_icon(&Ico16,ICO16_DISKMANMENU);
  hints.add(MenuBut.handle,T("Disk Manager options"),Handle);

  DirOutput.create(XD,Handle,135,80,445,25,NULL,this,
                    BT_TEXT | BT_STATIC | BT_TEXT_PATH | BT_BORDER_INDENT,
                    DisksFol,0,WhiteCol);

  if (StemWin) DiskBut.set_check(true);

  dir_lv.ext_sl.DeleteAll();
  dir_lv.ext_sl.Add(3,T("Parent Directory"),1,1,1);
  dir_lv.ext_sl.Add(3,"",ICO16_FOLDER,ICO16_FOLDERLINK,ICO16_FOLDERLINKBROKEN);

  dir_lv.ext_sl.Add(4,"st",ICO16_DISK,ICO16_DISKLINK,ICO16_DISKLINKBROKEN,ICO16_DISK_RO);
  dir_lv.ext_sl.Add(4,"stt",ICO16_DISK,ICO16_DISKLINK,ICO16_DISKLINKBROKEN,ICO16_DISK_RO);
  dir_lv.ext_sl.Add(4,"dim",ICO16_DISK,ICO16_DISKLINK,ICO16_DISKLINKBROKEN,ICO16_DISK_RO);
  dir_lv.ext_sl.Add(4,"msa",ICO16_DISK,ICO16_DISKLINK,ICO16_DISKLINKBROKEN,ICO16_DISK_RO);

  ArchiveTypeIdx=dir_lv.ext_sl.NumStrings;
  int zipicon=ICO16_ZIP_RO;
  if (FloppyArchiveIsReadWrite) zipicon=ICO16_ZIP_RW;
  dir_lv.ext_sl.Add(3,"zip",zipicon,ICO16_DISKLINK,ICO16_DISKLINKBROKEN);
  dir_lv.ext_sl.Add(3,"stz",zipicon,ICO16_DISKLINK,ICO16_DISKLINKBROKEN);
#ifdef RAR_SUPPORT
  dir_lv.ext_sl.Add(3,"rar",zipicon,ICO16_DISKLINK,ICO16_DISKLINKBROKEN);
#endif

  dir_lv.lpig=&Ico16;
  dir_lv.base_fol="";
  dir_lv.fol=DisksFol;
  dir_lv.allow_type_change=0;
  dir_lv.show_broken_links=0; //(HideBroken==0);
  dir_lv.create(XD,Handle,10,110,285,120,dir_lv_notify_handler,this);

  SetNumFloppies(num_connected_floppies);

  UpdateDiskNames(0);
  UpdateDiskNames(1);

  XMapWindow(XD,Handle);
}
//---------------------------------------------------------------------------
void TDiskManager::Hide()
{
  if (XD==NULL) return;

	if (HardDiskMan.IsVisible()) return;

  if (Handle==0) return;

  hints.remove_all_children(Handle);
  StandardHide();

  if (StemWin) DiskBut.set_check(0);
}
//---------------------------------------------------------------------------
int TDiskManager::WinProc(TDiskManager *This,Window Win,XEvent *Ev)
{
  switch (Ev->type){
    case ClientMessage:
      if (Ev->xclient.message_type==hxc::XA_WM_PROTOCOLS){
        if (Atom(Ev->xclient.data.l[0])==hxc::XA_WM_DELETE_WINDOW){
          This->Hide();
        }
      }
      break;
    case ConfigureNotify:
    {
      XWindowAttributes wa;
      XGetWindowAttributes(XD,Win,&wa);

      This->Width=wa.width;This->Height=wa.height;
      int w=This->Width;int h=This->Height;

      for (int d=0;d<2;d++){
  	    XResizeWindow(XD,This->disk_name[d].handle,max(w-(10+32 + 25+10+60+10),10),25);
     	}
      XResizeWindow(XD,This->DirOutput.handle,max(w-145,30),25);
      XResizeWindow(XD,This->dir_lv.lv.handle,max(w-20,10),max(h-120,10));

      XSync(XD,0);
      break;
    }
  }
  return PEEKED_MESSAGE;
}
//---------------------------------------------------------------------------
Str TDiskManager::GetCustomDiskImage(int *pSectors,int *pSecsPerTrack,int *pSides)
{
  int w=300,h=10+35+35+35+25+10;
  Window handle=hxc::create_modal_dialog(XD,w,h,T("Create Custom Disk Image"),true);
  if (handle==0) return "";

  hxc_edit *p_ed;
  hxc_dropdown *p_sides_dd,*p_tracks_dd,*p_secs_dd;

  int y=10,x=10,hw=(w-20)/2,tw;
  hxc_button *p_but=new hxc_button(XD,handle,x,y,0,25,NULL,this,BT_LABEL,T("Name"),0,hxc::col_bk);
  x+=p_but->w+5;

  p_ed=new hxc_edit(XD,handle,x,y,w-10-x,25,NULL,this);
  p_ed->set_text(T("Blank Disk"),true);
  y+=35;

  x=10;
  new hxc_button(XD,handle,x,y,0,25,NULL,this,BT_LABEL,T("Sides"),0,hxc::col_bk);
  x+=hw;

  p_sides_dd=new hxc_dropdown(XD,handle,x,y,hw,200,NULL,this);
  p_sides_dd->additem("1",1);
  p_sides_dd->additem("2",2);
  p_sides_dd->sel=1;
  p_sides_dd->changesel(SidesIdx);
  y+=35;

  x=10;
  new hxc_button(XD,handle,x,y,0,25,NULL,this,BT_LABEL,T("Tracks"),0,hxc::col_bk);
  x+=hw;

  tw=hxc::get_text_width(XD,T("0 to "));
  new hxc_button(XD,handle,x-tw,y,0,25,NULL,this,BT_LABEL,T("0 to "),0,hxc::col_bk);

  p_tracks_dd=new hxc_dropdown(XD,handle,x,y,hw,300,NULL,this);
  for (int n=75;n<=FLOPPY_MAX_TRACK_NUM;n++) p_tracks_dd->additem(Str(n),n);
  p_tracks_dd->sel=80-75;
  p_tracks_dd->changesel(TracksIdx);
  y+=35;

  x=10;
  new hxc_button(XD,handle,x,y,0,25,NULL,this,BT_LABEL,T("Sectors"),0,hxc::col_bk);
  x+=hw;

  tw=hxc::get_text_width(XD,T("1 to "));
  new hxc_button(XD,handle,x-tw,y,0,25,NULL,this,BT_LABEL,T("1 to "),0,hxc::col_bk);

  p_secs_dd=new hxc_dropdown(XD,handle,x,y,hw,300,NULL,this);
  for (int n=8;n<=FLOPPY_MAX_SECTOR_NUM;n++) p_secs_dd->additem(Str(n),n);
  p_secs_dd->sel=9-8;
  p_secs_dd->changesel(SecsPerTrackIdx);

  Str ret;
  if (hxc::show_modal_dialog(XD,handle,true,p_ed->handle)==1){
    ret=p_ed->text;

    // 0 to tracks_per_side inclusive! Choosing 80 gives you 81 tracks.
    int tracks_per_side=p_tracks_dd->sl[p_tracks_dd->sel].Data[0]+1;
    *pSecsPerTrack=p_secs_dd->sl[p_secs_dd->sel].Data[0];
    *pSides=p_sides_dd->sl[p_sides_dd->sel].Data[0];

    *pSectors=*pSecsPerTrack * tracks_per_side * *pSides;

    SidesIdx=p_sides_dd->sel;
    TracksIdx=p_tracks_dd->sel;
    SecsPerTrackIdx=p_secs_dd->sel;
  }

  hxc::destroy_modal_dialog(XD,handle);

  return ret;
}
//---------------------------------------------------------------------------
void TDiskManager::ShowContentDiag()
{
  if (contents_sl.NumStrings<2) return;

  int w=500,h=10+30+20+185+30+30+30+30+10,y=10;
  if (contents_sl.NumStrings==2) h=10+30+10;
  Window handle=hxc::create_modal_dialog(XD,w,h,T("Disk Image Contents"),0);
  if (handle==0) return;

  hxc_button *p_but,*p_but2,*p_group,*p_append_but=NULL;
  hxc_edit *p_path_ed=NULL,*p_append_ed=NULL;
  hxc_dropdown *p_conflict_dd=NULL;
  hxc_listview cont_lv;
  Window focus=0;

  new hxc_button(XD,handle,10,y,0,25,NULL,this,BT_LABEL,T("Short TOSEC name")+" - "+contents_sl[1].String,0,hxc::col_bk);
  y+=30;

  if (contents_sl.NumStrings>2){
    p_but=new hxc_button(XD,handle,10,y,0,0,NULL,this,BT_LABEL,T("Contents"),0,hxc::col_bk);
    y+=p_but->h+2;

    cont_lv.lpig=&Ico16;
    cont_lv.display_mode=1;
    cont_lv.checkbox_mode=true;
    cont_lv.sl.DeleteAll();
    cont_lv.sl.Sort=eslNoSort;
    for (int i=2;i<contents_sl.NumStrings;i++){
      cont_lv.additem(contents_sl[i].String,101+ICO16_TICKED);
    }
    cont_lv.create(XD,handle,10,y,w-20,180,diag_lv_np,this);
    y+=185;

    p_group=new hxc_button(XD,handle,10,y,w-20,120,NULL,this,BT_GROUPBOX,T("Create Links To Selected Contents"),0,hxc::col_bk);

    w=p_group->w;
    y=25;

    p_but=new hxc_button(XD,p_group->handle,10,y,0,25,NULL,this,BT_LABEL,T("In folder"),0,hxc::col_bk);

    p_but2=new hxc_button(XD,p_group->handle,w-10,y,0,25,diag_but_np,this,BT_TEXT,T("Browse"),200,hxc::col_bk);
    p_but2->x-=p_but2->w;
    XMoveWindow(XD,p_but2->handle,p_but2->x,p_but2->y);

    p_path_ed=new hxc_edit(XD,p_group->handle,10+p_but->w+5,y,w-10-10-p_but->w-5-p_but2->w-5,25,NULL,this);
    p_path_ed->set_text(ContentsLinksPath);
    p_path_ed->id=201;
    y+=30;
    focus=p_path_ed->handle;

    p_append_but=new hxc_button(XD,p_group->handle,10,y,0,25,NULL,this,BT_CHECKBOX,T("Append disk name"),101,hxc::col_bk);
    p_append_but->set_check(true);

    p_append_ed=new hxc_edit(XD,p_group->handle,10+p_append_but->w+5,y,w-10-10-p_append_but->w-5,25,NULL,this);
    p_append_ed->set_text(GetContentsGetAppendName(contents_sl[1].String));
    y+=30;

    p_but=new hxc_button(XD,p_group->handle,10,y,0,25,NULL,this,BT_LABEL,T("On name conflict"),0,hxc::col_bk);

    p_but2=new hxc_button(XD,p_group->handle,w-10,y,0,25,hxc::modal_but_np,this,BT_TEXT,T("Create"),1,hxc::col_bk);
    p_but2->x-=p_but2->w;
    XMoveWindow(XD,p_but2->handle,p_but2->x,p_but2->y);

    p_conflict_dd=new hxc_dropdown(XD,p_group->handle,10+p_but->w+5,y,w-10-10-p_but->w-5-p_but2->w-10,200,NULL,this);
    p_conflict_dd->additem(T("Skip"));
    p_conflict_dd->additem(T("Overwrite"));
    p_conflict_dd->additem(T("Rename new"));
    p_conflict_dd->changesel(ContentConflictAction);
  }

  Str DestFol="///",DiskName,NewLink;
  for(;;){
    int id=hxc::show_modal_dialog(XD,handle,true,focus);

    if (contents_sl.NumStrings<=2) break;

    ContentConflictAction=p_conflict_dd->sel;
    if (id==1){ // Create links
      DestFol=p_path_ed->text;
      NO_SLASH(DestFol);
      if (p_append_but->checked) DiskName=Str(" (")+p_append_ed->text+")";
      CreateDirectory(DestFol,NULL);
      if (GetFileAttributes(DestFol)==0xffffffff){
        Alert(T("Invalid directory"),T("Error"),MB_ICONEXCLAMATION);
      }else{
        char *ext;
        ext=strrchr(contents_sl[0].String,'.');
        if (ext==NULL) ext="";
        for (int i=2;i<contents_sl.NumStrings;i++){
          if (cont_lv.sl[i].Data[0]==101+ICO16_TICKED){
            NewLink=DestFol+SLASH+Str(contents_sl[i].String)+DiskName+ext;
            if (Exists(NewLink)){
              if (ContentConflictAction==0){
                NewLink="";
              }else if (ContentConflictAction==2){
                NewLink=GetUniquePath(DestFol,Str(contents_sl[i].String)+DiskName+ext);
              }
            }
            if (NewLink.NotEmpty()) symlink(contents_sl[0].String,NewLink);
          }
        }
        break;
      }
    }else{
      break;
    }
  }
  hxc::destroy_modal_dialog(XD,handle);

  if (IsSameStr_I(DestFol,DisksFol)){
    set_path(DisksFol);
    dir_lv.select_item_by_name(GetFileNameFromPath(NewLink));
  }
}
//---------------------------------------------------------------------------
int TDiskManager::diag_lv_np(hxc_listview *lv,int mess,int i)
{
  if (mess==LVN_ICONCLICK){
    int icon=lv->sl[i].Data[0]-101;
    if (icon==ICO16_TICKED){
      icon=ICO16_UNTICKED;
    }else{
      icon=ICO16_TICKED;
    }
    lv->sl[i].Data[0]=101+icon;
    lv->draw(0);
  }
  return 0;
}
//---------------------------------------------------------------------------
void TDiskManager::ShowDatabaseDiag()
{
  if (GetContentsCheckExist()==0) return;

  int w=600,h=10+30+300+10+hxc::font->ascent+hxc::font->descent+10,y=10;
  Window handle=hxc::create_modal_dialog(XD,w,h,T("Search Disk Image Database"),0);
  if (handle==0) return;

  hxc_button *p_but,*p_but2;
  hxc_edit *p_ed;
  hxc_textdisplay result_td;

  p_but=new hxc_button(XD,handle,10,y,0,25,NULL,this,BT_LABEL,T("Search for"),0,hxc::col_bk);

  p_but2=new hxc_button(XD,handle,w-10,y,0,25,diag_but_np,this,BT_TEXT,T("Go"),100,hxc::col_bk);
  p_but2->x-=p_but2->w;
  XMoveWindow(XD,p_but2->handle,p_but2->x,p_but2->y);

  p_ed=new hxc_edit(XD,handle,10+p_but->w+5,y,w-10-10-p_but->w-5-p_but2->w-5,25,diag_ed_np,this);
  p_ed->set_text("");
  p_ed->id=101;
  y+=30;

  result_td.id=200;
  result_td.border=1;
  result_td.pad_x=5;
  result_td.sy=0;
  result_td.textheight=(hxc::font->ascent)+(hxc::font->descent)+2;
  result_td.create(XD,handle,10,y,w-20,300,hxc::col_white,true);
  y+=310;

  p_but=new hxc_button(XD,handle,10,y,0,0,NULL,this,BT_LABEL,
          T("To download disks see Steem's "),0,hxc::col_bk);

  new hxc_button(XD,handle,10+p_but->w,y,0,0,hyperlink_np,this,BT_LINK|BT_TEXT,
          T("links page")+"|"+STEEM_WEB+"links.htm",0,hxc::col_bk);

  hxc::show_modal_dialog(XD,handle,true,p_ed->handle);
  hxc::destroy_modal_dialog(XD,handle);
}
//---------------------------------------------------------------------------
int TDiskManager::diag_but_np(hxc_button *b,int mess,int *p_i)
{
  if (mess!=BN_CLICKED) return 0;
  if (b->id==100){
//    TDiskManager *This=(TDiskManager*)(b->owner);
    hxc_textdisplay *p_td=(hxc_textdisplay*)hxc::find(b->parent,200);
    hxc_edit *p_ed=(hxc_edit*)hxc::find(b->parent,101);

    Str Find=p_ed->text;
    if (Find.Empty()) return 0;

    char buf[65536],*p=buf;
    GetContents_SearchDatabase(Find,buf,sizeof(buf));

    Str Name,Contents;
    Str outtext;
    while (p[0]){
      Name=p;
      p+=strlen(p)+1;

      Contents="";
      while (p[0]){
        if (Contents[0]) Contents+=", ";
        Contents+=p;
        p+=strlen(p)+1;
      }
      p++; // skip content list null

      outtext+=Name+" - "+Contents+"\n\n";
    }
    p_td->sy=0;
    p_td->set_text(outtext);
    p_td->draw(true);
  }else if (b->id==200){
    b->set_check(true);
    hxc_edit *p_ed=(hxc_edit*)hxc::find(b->parent,201);
    fileselect.set_corner_icon(&Ico16,ICO16_FOLDER);
    EasyStr new_path=fileselect.choose(XD,p_ed->text,"",T("Pick a Folder"),
      FSM_CHOOSE_FOLDER | FSM_CONFIRMCREATE,folder_parse_routine,"");
    if (new_path[0]){
      NO_SLASH(new_path);
      p_ed->set_text(new_path+"/");
    }
    b->set_check(0);
  }
  return 0;
}
//---------------------------------------------------------------------------
int TDiskManager::diag_ed_np(hxc_edit *ed,int mess,int i)
{
  if (mess==EDN_RETURN){
    hxc_button *p_but=(hxc_button*)hxc::find(ed->parent,100);
    int i[1]={Button1};
    diag_but_np(p_but,BN_CLICKED,i);
  }
  return 0;
}
//---------------------------------------------------------------------------

