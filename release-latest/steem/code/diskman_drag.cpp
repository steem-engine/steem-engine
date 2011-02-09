/*---------------------------------------------------------------------------
FILE: diskman_drag.cpp
MODULE: Steem
DESCRIPTION: Routines to handle dragging in the Disk Manager.
---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
void TDiskManager::BeginDrag(int Item,HWND From)
{
  LV_ITEM lvi;
  lvi.mask=LVIF_PARAM;
  lvi.iItem=Item;
  lvi.iSubItem=0;
  SendMessage(From,LVM_GETITEM,0,(LPARAM)&lvi);
  if (((DiskManFileInfo*)lvi.lParam)->UpFolder){
    SetFocus(DiskView);
    return;
  }

  SendMessage(From,LVM_ENSUREVISIBLE,Item,0);
  UpdateWindow(From);

  POINT pt={0,0};

  Dragging=Item;
  DragLV=From;
  DragIL=(HIMAGELIST)SendMessage(DragLV,LVM_CREATEDRAGIMAGE,Dragging,(long)&pt);

  EndingDrag=0;

  SetCapture(Handle);

  ImageList_GetIconSize(DragIL,&DragWidth,&DragHeight);
  if (From==DiskView && SmallIcons){
    DragWidth=(18+GetTextSize(Font,((DiskManFileInfo*)lvi.lParam)->Name).Width)/2;
    DragHeight=-(DragHeight-2);
  }else{
    DragWidth/=2;
    DragWidth-=5;
    DragHeight=0;
  }
  ImageList_BeginDrag(DragIL,0,0,0);

  GetCursorPos(&pt);
  ScreenToClient(Handle,&pt);

  ImageList_DragEnter(Handle,pt.x-DragWidth,pt.y-DragHeight);
  DragEntered=true;


  SetTimer(Handle,DISKVIEWSCROLL_TIMER_ID,30,NULL);
}
//---------------------------------------------------------------------------
#define DRAG_CHECK_FOR_DESELECT \
      if (DeselectDropTarget){ \
        LV_ITEM lvi; \
        lvi.iSubItem=0; \
        lvi.stateMask=LVIS_DROPHILITED; \
        lvi.state=0; \
        SendMessage(DiskView,LVM_SETITEMSTATE,DropTarget,(long)&lvi); \
        UpdateWindow(DiskView); \
        DropTarget=-1; \
      }

void TDiskManager::MoveDrag()
{
  POINT pt,spt;
  bool Okay=0,DeselectDropTarget=(DropTarget>-1);

  LV_ITEM lvi;
  lvi.mask=LVIF_PARAM;
  lvi.iItem=Dragging;
  lvi.iSubItem=0;
  SendMessage(DragLV,LVM_GETITEM,0,(LPARAM)&lvi);
  DiskManFileInfo *DragInf=(DiskManFileInfo*)lvi.lParam;

  GetCursorPos(&spt);pt=spt;
  ScreenToClient(Handle,&pt);
  int OverID=GetDlgCtrlID(ChildWindowFromPoint(Handle,pt));
  if (((OverID==100 || OverID==101) && DragInf->Folder==0) || OverID==102 ||
        (OverID==80 && AtHome==0 && DragLV==DiskView)){
    Okay=true;
    if (OverID==102 && GetDlgCtrlID(DragLV)==102){ // Dragging from DiskView to DiskView
      LV_HITTESTINFO hti;
      LV_ITEM lvi;
      lvi.iSubItem=0;

      hti.pt=spt;
      ScreenToClient(DiskView,&hti.pt);
      int Item=SendMessage(DiskView,LVM_HITTEST,0,(long)&hti);
      if (Item!=DropTarget){
        if (Item>-1){
          lvi.mask=LVIF_PARAM;
          lvi.iItem=Item;
          lvi.iSubItem=0;
          SendMessage(DiskView,LVM_GETITEM,0,(long)&lvi);
          if ( ((DiskManFileInfo*)lvi.lParam)->Folder==0 || Item==Dragging) Item=-1;
        }
        if (Item!=DropTarget){
          if (DragEntered){
            ImageList_DragLeave(Handle);
            DragEntered=0;
          }

          lvi.stateMask=LVIS_DROPHILITED;
          if (DropTarget>-1){
            lvi.state=0;
            SendMessage(DiskView,LVM_SETITEMSTATE,DropTarget,(long)&lvi);
          }
          if (Item>-1){
            lvi.state=LVIS_DROPHILITED;
            SendMessage(DiskView,LVM_SETITEMSTATE,Item,(long)&lvi);
          }
          DropTarget=Item;

          UpdateWindow(DiskView);
        }
      }
      DeselectDropTarget=0;
    }
  }
  if (LastOverID==80 && OverID!=80){
    if (DragEntered){
      ImageList_DragLeave(Handle);
      DragEntered=0;
    }
    SendMessage(GetDlgItem(Handle,80),BM_SETCHECK,0,0);
  }
  if (Okay){
    if (OverID==80 && LastOverID!=80){
      if (DragEntered){
        ImageList_DragLeave(Handle);
        DragEntered=0;
      }
      SendMessage(GetDlgItem(Handle,80),BM_SETCHECK,1,0);
    }
    SetCursor(PCArrow);
    DRAG_CHECK_FOR_DESELECT;
    if (DragEntered==0){
      ImageList_DragEnter(Handle,pt.x-DragWidth,pt.y-DragHeight);
      DragEntered=true;
    }else{
      ImageList_DragMove(pt.x-DragWidth,pt.y-DragHeight);
    }
  }else{
    SetCursor(LoadCursor(NULL,IDC_NO));
    if (DragEntered){
      ImageList_DragLeave(Handle);
      DragEntered=0;
    }
    DRAG_CHECK_FOR_DESELECT;
    if (DragLV==DiskView){
      RECT rc;
      GetWindowRect(DiskView,&rc);
      if (spt.x>=rc.left && spt.y<=rc.right){
        if (spt.y<=rc.top+2 && spt.y>=rc.top-20){
          SendMessage(DiskView,LVM_SCROLL,0,-8);
        }else if (spt.y>=rc.bottom-2 && spt.y<=rc.bottom+10){
          SendMessage(DiskView,LVM_SCROLL,0,8);
        }
        UpdateWindow(DiskView);
      }
    }
  }
  LastOverID=OverID;
}
//---------------------------------------------------------------------------
void TDiskManager::EndDrag(int x,int y,bool RightDrag)
{
  DiskManFileInfo *DragInf=GetItemInf(Dragging,DragLV);

  EndingDrag=true;

  if (DragEntered) ImageList_DragLeave(Handle);
  ImageList_EndDrag();
  ImageList_Destroy(DragIL);

  ReleaseCapture();

  KillTimer(Handle,1);

  POINT pt={x,y};
  HWND LV;
  for (int i=80;i<=102;i++){
    LV=GetDlgItem(Handle,i);
    if (ChildWindowFromPoint(Handle,pt)==LV){
      if (DragLV!=LV && i>80 && DragInf->Folder==0){
        if (i<102){  // Dragged from DiskView or other Floppy to Floppy ListView
          bool InsertIt=true;
          EasyStr DiskInZip;
          if (DragInf->Zip && hUnzip){
            if (DragLV!=DiskView){  // Dragged from one floppy ListView to another
              DiskInZip=FloppyDrive[GetDlgCtrlID(DragLV)-100].DiskInZip;
            }else{
              EasyStringList esl;
              esl.Sort=eslSortByNameI;
              // Get all disks in archive
              zippy.list_contents(DragInf->Path,&esl,true);
              if (esl.NumStrings>1){
                HMENU Pop=CreatePopupMenu();
                for (int n=0;n<esl.NumStrings;n++) AppendMenu(Pop,MF_STRING,4000+n,esl[n].String);
                AppendMenu(Pop,MF_SEPARATOR,4099,NULL);
                AppendMenu(Pop,MF_STRING,4099,T("Cancel"));

                GetCursorPos(&pt);
                TrackPopupMenu(Pop,TPM_LEFTALIGN | TPM_LEFTBUTTON,pt.x,pt.y,0,Handle,NULL);
                DestroyMenu(Pop);

                MenuTarget=0;
                MSG mess;
                while (PeekMessage(&mess,Handle,WM_COMMAND,WM_COMMAND,PM_REMOVE)) DispatchMessage(&mess);
                if (MenuTarget>=4000 && MenuTarget-4000<esl.NumStrings){
                  DiskInZip=esl[MenuTarget-4000].String;
                }else{
                  InsertIt=0;
                }
              }
            }
          }
          if (InsertIt){
            if (InsertDisk(i-100,DragInf->Name,DragInf->Path,0,true,DiskInZip,0,DragLV==DiskView)){
              if (DragLV!=DiskView){  // Dragged from one floppy ListView to another
                EjectDisk(GetDlgCtrlID(DragLV)-100);
              }
            }
          }
        }else{  // Dragged from Floppy to DiskView
          EjectDisk(GetDlgCtrlID(DragLV)-100);
        }
        break;
      }else if ((LV==DiskView && (DropTarget>-1 || RightDrag)) ||
                  (i==80 && AtHome==0 && DragLV==DiskView)){
        bool DraggedToFolder=0;
        DiskManFileInfo *DestInf=NULL;
        EasyStr DestFol;
        if (DropTarget>-1){
          DestInf=GetItemInf(DropTarget);

          if (DestInf->UpFolder){
            DestFol=DisksFol;
            RemoveFileNameFromPath(DestFol,REMOVE_SLASH);
          }else{
            DestFol=DestInf->Path;
          }
          DraggedToFolder=true;
        }else if (i==80){
          DestFol=HomeFol;
        }else{
          DestFol=DisksFol;
        }
        if (RightDrag){
          MenuTarget=0;

          SetFocus(DiskView);

          GetCursorPos(&pt);

          HMENU OpMenu=CreatePopupMenu();
          if (DragInf->LinkPath.IsEmpty()){
            if (DestFol!=DisksFol) AppendMenu(OpMenu,MF_STRING,4000,T("&Move Here"));
            AppendMenu(OpMenu,MF_STRING,4001,T("&Copy Here"));
            AppendMenu(OpMenu,MF_STRING,4002,T("Create &Shortcut Here"));
            if (DragInf->Folder==0) AppendMenu(OpMenu,MF_STRING,4010,T("Create M&ultiple Shortcuts Here"));
            if (DestFol!=DisksFol && DragInf->Folder==0){
              AppendMenu(OpMenu,MF_STRING,4012,T("Move Disk Here and &Get Contents"));
              AppendMenu(OpMenu,MF_STRING,4011,T("Move Disk Here and Create Multiple Shortcuts to it"));
            }
          }else{
            if (DestFol!=DisksFol) AppendMenu(OpMenu,MF_STRING,4003,T("&Move Shortcut Here"));
            AppendMenu(OpMenu,MF_STRING,4004,T("&Copy Shortcut Here"));
            AppendMenu(OpMenu,MF_SEPARATOR,4097,NULL);

            EasyStr FolDisk=DragInf->Path;
            RemoveFileNameFromPath(FolDisk,REMOVE_SLASH);
            if (NotSameStr_I(FolDisk,DestFol)){
              AppendMenu(OpMenu,MF_STRING,4000,LPSTR(DragInf->Folder ? T("Move &Folder Here"):T("Move &Disk Here")));
            }
            AppendMenu(OpMenu,MF_STRING,4001,LPSTR(DragInf->Folder ? T("Copy F&older Here"):T("Copy D&isk Here")));
            if (DragInf->Folder==0) AppendMenu(OpMenu,MF_STRING,4010,T("Create Mu&ltiple Shortcuts To The Disk Here"));
          }
          AppendMenu(OpMenu,MF_SEPARATOR,4099,NULL);

          AppendMenu(OpMenu,MF_STRING,4098,T("Cancel"));

          TrackPopupMenu(OpMenu,TPM_LEFTALIGN | TPM_LEFTBUTTON,pt.x,pt.y,0,Handle,NULL);
          DestroyMenu(OpMenu);

          MSG mess;
          while (PeekMessage(&mess,Handle,WM_COMMAND,WM_COMMAND,PM_REMOVE)) DispatchMessage(&mess);
        }else{
          MenuTarget=int(DragInf->LinkPath.IsEmpty() ? 4000:4003);
        }
        EasyStr SrcFol=LPSTR((MenuTarget==4000 || MenuTarget==4001 || MenuTarget==4011 || MenuTarget==4012) ? DragInf->Path:DragInf->LinkPath);
        RemoveFileNameFromPath(SrcFol,REMOVE_SLASH);
        if (MenuTarget==4002){  //Create Shortcut
          EasyStr LinkName=DestFol+"\\"+DragInf->Name+".lnk";
          int n=2;
          while (Exists(LinkName)){
            LinkName=DestFol+"\\"+DragInf->Name+" ("+(n++)+").lnk";
          }

          CreateLink(LinkName,DragInf->Path);

          if (IsSameStr_I(DestFol,DisksFol)){
            RefreshDiskView("",true,LinkName);
            DropTarget=-1;
          }
        }else if (((MenuTarget==4001 && DragInf->Folder==0) || MenuTarget==4004) && SrcFol==DestFol){
          EasyStr Name=GetFileNameFromPath(LPSTR((MenuTarget==4001) ? DragInf->Path:DragInf->LinkPath));
          EasyStr Ext;
          char *dot=strrchr(Name,'.');
          if (dot){
            Ext=dot;
            *dot=0;
          }

          EasyStr Path;
          int n=2;
          do{
            Path=DestFol+"\\"+Name+" ("+ (n++) +")"+Ext;
          }while (Exists(Path));

          CopyFile(LPSTR((MenuTarget==4001) ? DragInf->Path:DragInf->LinkPath),Path,true);
          if (MenuTarget==4001) UpdateBPBFiles(DragInf->Path,Path,0);

          if (MenuTarget==4001){
            RefreshDiskView(Path,true);
          }else{
            RefreshDiskView("",true,Path);
          }
          DropTarget=-1;
        }else if (MenuTarget==4000 || MenuTarget==4001 || MenuTarget==4011 || MenuTarget==4012 || //Move/Copy Path
                    MenuTarget==4003 || MenuTarget==4004){  //Move/Copy LinkPath
          bool Moving=(MenuTarget==4000 || MenuTarget==4011 || MenuTarget==4003);
          bool DiskIsTarget=(MenuTarget<4003 || MenuTarget==4011);
          bool DoIt=true;
          if (MenuTarget==4012){ // Get contents
            Moving=true;
            DiskIsTarget=true;
            GetContentsSL(DragInf->Path);
            if (contents_sl.NumStrings==0) DoIt=0;
          }
          EasyStr To;
          char From[MAX_PATH+2];
          ZeroMemory(From,MAX_PATH+2);
          if (DiskIsTarget){
            strcpy(From,DragInf->Path);
            To=DestFol;
            if (DragInf->Folder==0) To+=EasyStr("\\")+GetFileNameFromPath(DragInf->Path);
          }else{
            strcpy(From,DragInf->LinkPath);
            To=DestFol+"\\"+GetFileNameFromPath(DragInf->LinkPath);
          }
          char *DiskPath="";
          if (DiskIsTarget) DiskPath=DragInf->Path;
          if (DoIt){
            if (MoveOrCopyFile(Moving,From,To,DiskPath,IsSameStr_I(SrcFol,DestFol))){
              if (Moving && DiskIsTarget && DragInf->LinkPath.NotEmpty()){
                //Update shortcut for the new location of disk
                CreateLink(DragInf->LinkPath,To);
              }
              LinksTargetPath=To;
              if (IsSameStr_I(SrcFol,DisksFol) || IsSameStr_I(DestFol,DisksFol)){
                //Refresh the DiskView
                if (DraggedToFolder){
                  if (DestInf->LinkPath.NotEmpty()){
                    RefreshDiskView("",0,DestInf->LinkPath);
                  }else{
                    RefreshDiskView(DestInf->Path);
                  }
                }else{
                  if (DiskIsTarget){ // Path operation
                    if (DragInf->LinkPath.NotEmpty()){
                      if (IsSameStr_I(DestFol,DisksFol)){
                        RefreshDiskView(To);
                      }else{
                        RefreshDiskView("",0,DragInf->LinkPath);
                      }
                    }else{
                      RefreshDiskView("",0,"",GetSelectedItem());
                    }
                  }else{                // LinkPath operation
                    if (MenuTarget==4003){
                      RefreshDiskView("",0,"",GetSelectedItem());
                    }else{
                      RefreshDiskView("",0,DragInf->LinkPath);
                    }
                  }
                }
                DropTarget=-1;
              }
              if (MenuTarget==4011){
                MultipleLinksPath=SrcFol;
                ShowLinksDiag();
                RefreshDiskView("",0,"",GetSelectedItem());
              }else if (MenuTarget==4012){
                contents_sl.SetString(0,To);
                ContentsLinksPath=DisksFol;
                EnableWindow(Handle,0);
                ShowContentDiag();
              }
            }
          }
        }else if (MenuTarget==4010){
          MultipleLinksPath=DestFol;
          LinksTargetPath=DragInf->Path;
          ShowLinksDiag();
        }
        if (DropTarget>-1){
          LV_ITEM lvi;
          lvi.stateMask=LVIS_DROPHILITED;
          lvi.state=0;
          SendMessage(DiskView,LVM_SETITEMSTATE,DropTarget,(long)&lvi);
          DropTarget=-1;
        }else if (i==80){
          SendMessage(GetDlgItem(Handle,80),BM_SETCHECK,0,0);
        }
        break;
      }
    }
    if (i==80) i=99;
  }
  Dragging=-1;
}
//---------------------------------------------------------------------------
void TDiskManager::UpdateBPBFiles(Str CurDisk,Str NewDisk,bool Moving)
{
  EasyStringList cur_sl,new_sl;
  cur_sl.Add(CurDisk+".steembpb");
  if (NewDisk.NotEmpty()) new_sl.Add(NewDisk+".steembpb");
  if (FileIsDisk(CurDisk)==DISK_COMPRESSED){
    EasyStringList zsl;
    zippy.list_contents(CurDisk,&zsl,true);
    for (int i=0;i<zsl.NumStrings;i++){
      cur_sl.Add(CurDisk + zsl[i].String + ".steembpb");
      if (NewDisk.NotEmpty()) new_sl.Add(NewDisk + zsl[i].String + ".steembpb");
    }
  }
  for (int i=0;i<cur_sl.NumStrings;i++){
    if (NewDisk.NotEmpty()) CopyFile(cur_sl[i].String,new_sl[i].String,0);
    if (Moving || NewDisk.Empty()) DeleteFile(cur_sl[i].String);
  }
}
//---------------------------------------------------------------------------
bool TDiskManager::MoveOrCopyFile(bool Moving,char *From,char *To,char *DiskPath,bool SameFol)
{
  bool InDrive[2]={0,0};
  EasyStr OldName[2],DiskInZip[2];
  if (Moving && DiskPath[0]){ // Moving disk
    for (int disk=0;disk<2;disk++){
      if (IsSameStr_I(FloppyDrive[disk].GetDisk(),DiskPath)){
        InDrive[disk]=true;
        OldName[disk]=FloppyDrive[disk].DiskName;
        DiskInZip[disk]=FloppyDrive[disk].DiskInZip;
        FloppyDrive[disk].RemoveDisk();
      }
    }
  }
  EasyStr Dest=To;
  if (SameFol){
    RemoveFileNameFromPath(Dest,KEEP_SLASH);
    Str FromName=GetFileNameFromPath(From);
    EasyStr Ext;
    char *dot=strrchr(FromName,'.');
    if (dot){
      Ext=dot;
      *dot=0;
    }
    
    Str f;
    int i=2;
    do{
      f=FromName+" ("+(i++)+")"+Ext;
    }while (Exists(Dest+f));
    Dest+=f;
  }

  SHFILEOPSTRUCT fos;
  fos.hwnd=HWND(FullScreen ? StemWin:Handle);
  fos.wFunc=int(Moving ? FO_MOVE:FO_COPY);
  fos.pFrom=From;
  fos.pTo=Dest;
  fos.fFlags=FILEOP_FLAGS(FOF_ALLOWUNDO | int(FullScreen ? FOF_SILENT:0));
  fos.hNameMappings=NULL;
  fos.lpszProgressTitle=LPSTR(Moving ? StaticT("Moving..."):StaticT("Copying..."));
  EnableWindow(Handle,0);
  int Ret=SHFileOperation(&fos);
  EnableWindow(Handle,true);
  if (Ret || fos.fAnyOperationsAborted){
    for (int disk=0;disk<2;disk++){
      if (InDrive[disk]){
        FloppyDrive[disk].SetDisk(DiskPath,DiskInZip[disk]);
        FloppyDrive[disk].DiskName=OldName[disk];
      }
    }
    return 0;
  }else{
    if (DiskPath[0]){ // Doing something with disk image, do the same to
                      // associated steembpb files
      UpdateBPBFiles(DiskPath,Dest,Moving);
    }
    for (int disk=0;disk<2;disk++){
      if (InDrive[disk]){
        InsertHistoryDelete(disk,OldName[disk],DiskPath,DiskInZip[disk]);
        InsertHistoryAdd(disk,OldName[disk],Dest,DiskInZip[disk]);
        FloppyDrive[disk].SetDisk(Dest,DiskInZip[disk]);
        FloppyDrive[disk].DiskName=OldName[disk];
        DiskManFileInfo *DriveInf=GetItemInf(0,GetDlgItem(Handle,100+disk));
        if (DriveInf) DriveInf->Path=Dest;
      }
    }
    return true;
  }
}
//---------------------------------------------------------------------------

