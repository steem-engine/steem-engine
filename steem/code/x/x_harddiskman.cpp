//---------------------------------------------------------------------------
THardDiskManager::THardDiskManager()
{
  Section="HardDrives";

  OldDrive=NULL;nOldDrives=0;

  for (int i=0;i<MAX_HARDDRIVES;i++){
    Drive[i].Path="";
    Drive[i].Letter=(char)('C'+i);
  }
  DisableHardDrives=0;
  nDrives=0;
  update_mount();

  ApplyChanges=0;
}
//---------------------------------------------------------------------------
void THardDiskManager::Show()
{
	if (Handle) return;

  if (StandardShow(590,10+60+(nDrives*30)+5,T("Hard Drives"),
      ICO16_HARDDRIVE,0,(LPWINDOWPROC)WinProc,true)) return;

  XSizeHints *pHints=XAllocSizeHints();
  if (pHints){
    pHints->flags=PMinSize | PMaxSize;
		pHints->min_width=590;
		pHints->min_height=10+60+5;
    pHints->max_width=590;
    pHints->max_height=10+60+(MAX_HARDDRIVES*30)+5;
    XSetWMSizeHints(XD,Handle,pHints,XA_WM_NORMAL_HINTS);
    XSetWMSizeHints(XD,Handle,pHints,XA_WM_ZOOM_HINTS);
    XFree(pHints);
  }

	int y=10;
	for (int n=0;n<nDrives;n++){
		CreateDriveControls(n); 				  				
  	y+=30;
	}
	all_off_but.create(XD,Handle,10,y,0,25,button_notify_proc,this,BT_CHECKBOX,StripAndT("&Disable All Hard Drives"),400,BkCol);
	SetWindowGravity(XD,all_off_but.handle,SouthEastGravity);
	
	new_but.create(XD,Handle,345,y,235,25,button_notify_proc,this,BT_TEXT,StripAndT("&New Hard Drive"),401,BkCol);
	SetWindowGravity(XD,new_but.handle,SouthEastGravity);
	
	y+=30;
	
	boot_label.create(XD,Handle,10,y,0,25,NULL,this,BT_LABEL,T("When drive A is empty boot from"),402,BkCol);
	SetWindowGravity(XD,boot_label.handle,SouthEastGravity);
	
  boot_dd.make_empty();
  boot_dd.additem("Off");
  for (int i=0;i<24;i++) boot_dd.additem(Str(char('C'+i))+":");
#ifndef DISABLE_STEMDOS
  boot_dd.changesel(stemdos_boot_drive-1);
#endif
	boot_dd.create(XD,Handle,10+boot_label.w+5,y,50,200,NULL);
	SetWindowGravity(XD,boot_dd.handle,SouthEastGravity);

	ok_but.create(XD,Handle,410,y,80,25,
												button_notify_proc,this,BT_TEXT,T("Ok"),403,BkCol);
	SetWindowGravity(XD,ok_but.handle,SouthEastGravity);

	cancel_but.create(XD,Handle,500,y,80,25,
												button_notify_proc,this,BT_TEXT,T("Cancel"),404,BkCol);
	SetWindowGravity(XD,cancel_but.handle,SouthEastGravity);

  XMapWindow(XD,Handle);

  nOldDrives=nDrives;
  if (nDrives){
    OldDrive=new Hard_Disk_Info[nOldDrives];
  }else{
    OldDrive=NULL;
  }
  for (int i=0;i<nDrives;i++){
    OldDrive[i]=Drive[i];
  }

  if (StemWin) DiskMan.HardBut.set_check(true);
}
//---------------------------------------------------------------------------
void THardDiskManager::CreateDriveControls(int n)
{
	int y=10+(n*30);
  drive_dd[n].make_empty();
  drive_dd[n].additem("Off");
  for (int i=0;i<24;i++) drive_dd[n].additem(Str(char('C'+i))+":");
  drive_dd[n].changesel((Drive[n].Letter-'C')+1);
  drive_dd[n].create(XD,Handle,10,y,50,200,NULL);

	drive_ed[n].create(XD,Handle,70,y,240,25,NULL);
	drive_ed[n].set_text(Drive[n].Path+"/");

	drive_browse_but[n].create(XD,Handle,320,y,80,25,
											button_notify_proc,this,BT_TEXT,T("Browse"),n*10,BkCol);

	drive_open_but[n].create(XD,Handle,410,y,80,25,
											button_notify_proc,this,BT_TEXT,T("Open"),n*10+2,BkCol);

	drive_remove_but[n].create(XD,Handle,500,y,80,25,
											button_notify_proc,this,BT_TEXT,T("Remove"),n*10+1,BkCol);
}
//---------------------------------------------------------------------------
void THardDiskManager::SetWindowHeight()
{
	XResizeWindow(XD,Handle,590,10+(nDrives*30)+60+5);
//	unix_non_resizable_window(XD,Handle);
}
//---------------------------------------------------------------------------
void THardDiskManager::GetDriveInfo()
{
	for(int n=0;n<nDrives;n++){
		Drive[n].Letter='B'+drive_dd[n].sel;
		Drive[n].Path=drive_ed[n].text;
		NO_SLASH(Drive[n].Path.Text);
	}
}
//---------------------------------------------------------------------------
void THardDiskManager::Hide()
{
  if (XD==NULL || Handle==0) return;

	if (ApplyChanges){
		GetDriveInfo();
		
 		for(int n=0;n<nDrives;n++){
 			if(Drive[n].Path.Text[0]){
    		if(!Exists(Drive[n].Path)){
          if (Alert(Drive[n].Path+" "+T("does not exist. Do you want to create it?"),T("New Folder?"),
                      MB_ICONQUESTION | MB_YESNO)==IDYES){
            if (CreateDirectory(Drive[n].Path,NULL)==0){
              Alert(T("Could not create the folder")+" "+Drive[n].Path,T("Invalid Path"),MB_ICONEXCLAMATION);
              return;
            }
          }else{
          	return;
          }
        }
      }
    }
		
    // Remove old stemdos drives from ST memory
    long DrvMask=LPEEK(SV_drvbits);
    for (int i=0;i<nOldDrives;i++){
      DrvMask &= ~(1 << (OldDrive[i].Letter-'A'));
    }
    LPEEK(SV_drvbits)=DrvMask;

    update_mount();

#ifndef DISABLE_STEMDOS
    stemdos_boot_drive=boot_dd.sel+1;

    stemdos_update_drvbits();

    stemdos_check_paths();
#endif
  }else{
    nDrives=nOldDrives;
    for (int i=0;i<nOldDrives;i++) Drive[i]=OldDrive[i];
    DisableHardDrives=OldDisableHardDrives;
    update_mount();
  }
  ApplyChanges=0;

  if (OldDrive) delete[] OldDrive;

  StandardHide();

  if (StemWin) DiskMan.HardBut.set_check(0);
}
//---------------------------------------------------------------------------
void THardDiskManager::RemoveLine(int dn)
{
 	for (int n=dn;n<nDrives-1;n++){
   	// copy info from line n+1 to line n
 		drive_dd[n].changesel(drive_dd[n+1].sel);
 		drive_dd[n].draw();
 		drive_ed[n].set_text(drive_ed[n+1].text);
 	}
 	nDrives--;
 	drive_dd[nDrives].destroy(&(drive_dd[nDrives]));
 	drive_ed[nDrives].destroy(&(drive_ed[nDrives]));
 	drive_browse_but[nDrives].destroy(&(drive_browse_but[nDrives]));
 	drive_open_but[nDrives].destroy(&(drive_open_but[nDrives]));
 	drive_remove_but[nDrives].destroy(&(drive_remove_but[nDrives]));
 	SetWindowHeight();
}  					

int THardDiskManager::button_notify_proc(hxc_button *But,int Mess,int *Inf)
{
	THardDiskManager *This=(THardDiskManager*)But->owner;
	if (Mess==BN_CLICKED){
		switch (But->id){
			case 400:
        This->DisableHardDrives=But->checked;
				break;
			case 401:
        if (This->nDrives<MAX_HARDDRIVES){
          This->GetDriveInfo();
          This->NewDrive(WriteDir);
          This->CreateDriveControls(This->nDrives-1);
          This->SetWindowHeight();
        }	
				break;
			case 403:
				This->ApplyChanges=true;
			case 404:
				This->Hide();
				break;			
			default:{
				if (But->id<MAX_HARDDRIVES*10){
  				int r=((But->id)%10);
  				int dn=((But->id)/10);
  				if (r==0){ //browse
  					char*path=This->drive_ed[dn].text.Text;
						fileselect.set_corner_icon(&Ico16,ICO16_FOLDER);
					  EasyStr new_path=fileselect.choose(XD,path,"",T("Pick a Folder"),
		   				FSM_CHOOSE_FOLDER | FSM_CONFIRMCREATE,folder_parse_routine,"");
	   				if(new_path[0]){
	   					NO_SLASH(new_path);
	   					This->drive_ed[dn].set_text(new_path+"/");
	   				}
  				}else if (r==1){  //remove
  					This->RemoveLine(dn);
  				}else if (r==2){  // open
            shell_execute(Comlines[COMLINE_FM],Str("[PATH]\n")+This->drive_ed[dn].text);
          }
  			}
			}
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
int THardDiskManager::WinProc(THardDiskManager *This,Window Win,XEvent*Ev)
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

