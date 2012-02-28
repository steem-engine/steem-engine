/*---------------------------------------------------------------------------
FILE: gui.cpp
MODULE: Steem
DESCRIPTION: This is a core file that has lots and lots of miscellaneous
GUI functions. It creates the main window in MakeGUI, handles translations
and (for some reason) command-line options.
---------------------------------------------------------------------------*/

#include "stemwin.cpp"
#define LOGSECTION LOGSECTION_INIT


#if defined(STEVEN_SEAGAL) && defined(SS_VID_BORDERS)

extern int draw_last_scanline_for_border,res_vertical_scale; // forward

int BorderSize=0; // original=0

int ChangeBorderSize(int size) {
  ASSERT(size==0||size==1||size==2);
  if(FullScreen && size==2)
    size=1; // Very large -> large (max for fullscreen)
  BorderSize=size;
  switch(size)
  {
  case 0:
    SideBorderSize=ORIGINAL_BORDER_SIDE;
    BottomBorderSize=ORIGINAL_BORDER_BOTTOM;
    break;
  case 1:
    SideBorderSize=LARGE_BORDER_SIDE;
    BottomBorderSize=LARGE_BORDER_BOTTOM;
    break;
  case 2:
    SideBorderSize=VERY_LARGE_BORDER_SIDE;
    BottomBorderSize=VERY_LARGE_BORDER_BOTTOM;
  }//sw
  int i,j;
  for(i=0;i<4;i++) 
  {
    for(j=0;j<5;j++) 
    {
      switch(size)
      {
      case 0:
        WinSizeBorder[i][j]=WinSizeBorderOriginal[i][j];
        break;
      case 1:
        WinSizeBorder[i][j]=WinSizeBorderLarge[i][j];
        break;
      case 2:
        WinSizeBorder[i][j]=WinSizeBorderVeryLarge[i][j];
      }//sw
    }
  }
  draw_last_scanline_for_border=shifter_y+res_vertical_scale*(BORDER_BOTTOM);
  StemWinResize();
  Disp.ScreenChange();
  return TRUE;
}

#endif


//---------------------------------------------------------------------------
void GUIRunStart()
{
#ifdef WIN32
  KillTimer(StemWin,SHORTCUTS_TIMER_ID);
  if (AllowTaskSwitch==0) DisableTaskSwitch();
  if (HighPriority) SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
#elif defined(PRIO_PROCESS) && defined(PRIO_MAX)
  hxc::kill_timer(StemWin,SHORTCUTS_TIMER_ID);
	if (HighPriority) setpriority(PRIO_PROCESS,0,PRIO_MAX);
#endif
  CheckResetDisplay(true);
  WIN_ONLY( if (FullScreen) TScreenSaver::killTimer(); )
}
//---------------------------------------------------------------------------
bool GUIPauseWhenInactive()
{
  if ((PauseWhenInactive && bAppActive==0) || timer<CutPauseUntilSysEx_Time){
    bool MouseWasCaptured=(stem_mousemode==STEM_MOUSEMODE_WINDOW);
    if (FullScreen==0) SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
    Sound_Stop(0);

#ifdef WIN32
    SetWindowText(StemWin,Str("Steem - ")+T("Suspended"));

    MSG mess;
    SetTimer(StemWin,MIDISYSEX_TIMER_ID,100,NULL);
    while (GetMessage(&mess,NULL,0,0)){
      if (HandleMessage(&mess)){
        TranslateMessage(&mess);
        DispatchMessage(&mess);
      }
      if (timeGetTime()>CutPauseUntilSysEx_Time && (PauseWhenInactive==0 || bAppActive)) break;
      if (runstate!=RUNSTATE_RUNNING) break;
    }
    if (mess.message==WM_QUIT) QuitSteem();
    KillTimer(StemWin,MIDISYSEX_TIMER_ID);

    SetWindowText(StemWin,stem_window_title);
#elif defined(UNIX)
    XEvent Ev;
    do{
      if (hxc::wait_for_event(XD,&Ev,200)) ProcessEvent(&Ev);
      
      if (timeGetTime()>CutPauseUntilSysEx_Time && (PauseWhenInactive==0 || bAppActive)) break;
    }while (runstate==RUNSTATE_RUNNING && Quitting==0);
#endif

    if (FullScreen==0 && MouseWasCaptured && GetForegroundWindow()==StemWin){
      SetStemMouseMode(STEM_MOUSEMODE_WINDOW);
    }

    Sound_Start();
    return true;
  }
  return 0;
}
//---------------------------------------------------------------------------
void GUIRunEnd()
{
  slow_motion=0;

  if (fast_forward){
    fast_forward=0;
    flashlight(false);

#ifdef WIN32
    SendMessage(GetDlgItem(StemWin,109),BM_SETCHECK,0,1);
#elif defined(UNIX)
    FastBut.set_check(0);
#endif
  }
  WIN_ONLY( if (HighPriority) SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS); )
#ifdef PRIO_PROCESS
	UNIX_ONLY( setpriority(PRIO_PROCESS,0,0); )
#endif
  SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
  WIN_ONLY( EnableTaskSwitch(); )
  WIN_ONLY( SetTimer(StemWin,SHORTCUTS_TIMER_ID,50,NULL); )
  UNIX_ONLY( hxc::set_timer(StemWin,SHORTCUTS_TIMER_ID,50,timerproc,NULL); )
  WIN_ONLY( if (FullScreen) TScreenSaver::prepareTimer(); )
}
//---------------------------------------------------------------------------
void GUIColdResetChangeSettings()
{
  if (OptionBox.NewMemConf0==-1){
    if ((bank_length[0]+bank_length[1])==(KB512+KB128)){  // Old 512Kb
      OptionBox.NewMemConf0=MEMCONF_512;
      OptionBox.NewMemConf1=MEMCONF_512K_BANK1_CONF;
    }else if ((bank_length[0]+bank_length[1])==(MB2+KB512)){  // Old 2Mb
      OptionBox.NewMemConf0=MEMCONF_2MB;
      OptionBox.NewMemConf1=MEMCONF_2MB_BANK1_CONF;
    }
  }
  if (OptionBox.NewMemConf0>=0){
    delete[] Mem;Mem=NULL;
    make_Mem(BYTE(OptionBox.NewMemConf0),BYTE(OptionBox.NewMemConf1));
    OptionBox.NewMemConf0=-1;
  }
  if (OptionBox.NewMonitorSel>=0){
#ifndef NO_CRAZY_MONITOR
    bool old_em=extended_monitor;
    extended_monitor=0;
#endif
    if (OptionBox.NewMonitorSel==1){
      mfp_gpip_no_interrupt &= MFP_GPIP_NOT_COLOUR;
    }else if (OptionBox.NewMonitorSel==0){
      mfp_gpip_no_interrupt |= MFP_GPIP_COLOUR;
    }else{ //crazy monitor
#ifndef NO_CRAZY_MONITOR
      int m=OptionBox.NewMonitorSel-2;
      if (extmon_res[m][2]==1){
        mfp_gpip_no_interrupt &= MFP_GPIP_NOT_COLOUR;
        screen_res=2;
      }else{
        mfp_gpip_no_interrupt |= MFP_GPIP_COLOUR;
        screen_res=0;
      }
      extended_monitor=1;
      em_width=extmon_res[m][0];em_height=extmon_res[m][1];em_planes=extmon_res[m][2];
#endif
    }
#ifndef NO_CRAZY_MONITOR
    if (bool(extended_monitor)!=old_em || extended_monitor){
      if (FullScreen){
        change_fullscreen_display_mode(true);
      }else{
        Disp.ScreenChange(); // For extended monitor
      }
    }
#endif
    OptionBox.NewMonitorSel=-1;
  }
  if (OptionBox.NewROMFile.NotEmpty()){
    if (load_TOS(OptionBox.NewROMFile)){
      Alert(T("The selected TOS file")+" "+OptionBox.NewROMFile+" "+T("is not in the correct format or may be corrupt."),T("Cannot Load TOS"),MB_ICONEXCLAMATION);
    }else{
      ROMFile=OptionBox.NewROMFile;
    }
    OptionBox.NewROMFile="";
  }
}
//---------------------------------------------------------------------------
void GUISaveResetBackup()
{
  DeleteFile(WriteDir+SLASH+"auto_loadsnapshot_backup.sts");
  SaveSnapShot(WriteDir+SLASH+"auto_reset_backup.sts",-1,0); // Don't add to history
}
//---------------------------------------------------------------------------
void GUIDiskErrorEject(int f)
{
  DiskMan.EjectDisk(f);
}
//---------------------------------------------------------------------------
void GUIEmudetectCreateDisk(Str Name,int Sides,int TracksPerSide,int SectorsPerTrack)
{
  Str DiskPath=DiskMan.HomeFol+SLASH+Name+".st";
  DiskMan.CreateDiskImage(Name,TracksPerSide*Sides*SectorsPerTrack,SectorsPerTrack,Sides);
  DiskMan.InsertDisk(0,Name,DiskPath);
}
//---------------------------------------------------------------------------
bool GUICanGetKeys()
{
  return (GetForegroundWindow()==StemWin);
}
//---------------------------------------------------------------------------
#ifdef WIN32
//---------------------------------------------------------------------------
int GetScreenWidth()
{
  return GetSystemMetrics(SM_CXSCREEN);
}
//---------------------------------------------------------------------------
int GetScreenHeight()
{
  return GetSystemMetrics(SM_CYSCREEN);
}
//---------------------------------------------------------------------------
void LoadAllIcons(ConfigStoreFile *NOT_ONEGAME( pCSF ),bool NOT_ONEGAME( FirstCall ))
{
#ifdef ONEGAME
  for (int n=1;n<RC_NUM_ICONS;n++) hGUIIcon[n]=NULL;
#else
  HICON hOld[RC_NUM_ICONS],hOldSmall[RC_NUM_ICONS];
  for (int n=1;n<RC_NUM_ICONS;n++){
    hOld[n]=hGUIIcon[n];
    hOldSmall[n]=hGUIIconSmall[n];
  }

  bool UseDefault=0;
  HDC dc=GetDC(NULL);
  if (GetDeviceCaps(dc,BITSPIXEL)<=8){
    UseDefault=bool(pCSF->GetInt("Icons","UseDefaultIn256",0));
  }
  ReleaseDC(NULL,dc);

  Str File;
  for (int n=1;n<RC_NUM_ICONS;n++){
    int size=RCGetSizeOfIcon(n);
    bool load16too=size & 1;
    size&=~1;

    hGUIIcon[n]=NULL;
    hGUIIconSmall[n]=NULL;
    if (size){
      if (UseDefault==0) File=pCSF->GetStr("Icons",Str("Icon")+n,"");
      if (File.NotEmpty()) hGUIIcon[n]=(HICON)LoadImage(Inst,File,IMAGE_ICON,size,size,LR_LOADFROMFILE);
      if (hGUIIcon[n]==NULL) hGUIIcon[n]=(HICON)LoadImage(Inst,RCNUM(n),IMAGE_ICON,size,size,0);
      if (load16too){
        if (File.NotEmpty()) hGUIIconSmall[n]=(HICON)LoadImage(Inst,File,IMAGE_ICON,16,16,LR_LOADFROMFILE);
        if (hGUIIconSmall[n]==NULL) hGUIIconSmall[n]=(HICON)LoadImage(Inst,RCNUM(n),IMAGE_ICON,16,16,0);
      }
    }
  }
  if (FirstCall==0){
    // Update all window classes, buttons and other icon thingies
    SetClassLong(StemWin,GCL_HICON,long(hGUIIcon[RC_ICO_APP]));
#ifdef _DEBUG_BUILD
    SetClassLong(DWin,GCL_HICON,long(hGUIIcon[RC_ICO_TRASH]));
    SetClassLong(trace_window_handle,GCL_HICON,long(hGUIIcon[RC_ICO_STCLOSE]));
    for (int n=0;n<MAX_MEMORY_BROWSERS;n++){
      if (m_b[n]) m_b[n]->update_icon();
    }
#endif
    for (int n=0;n<nStemDialogs;n++) DialogList[n]->UpdateMainWindowIcon();

    for (int id=100;id<=120;id++){
      if (GetDlgItem(StemWin,id)) PostMessage(GetDlgItem(StemWin,id),BM_RELOADICON,0,0);
    }
    DiskMan.LoadIcons();
    OptionBox.LoadIcons();
    InfoBox.LoadIcons();
    ShortcutBox.UpdateDirectoryTreeIcons(&(ShortcutBox.DTree));
    if (ShortcutBox.pChooseMacroTree) ShortcutBox.UpdateDirectoryTreeIcons(ShortcutBox.pChooseMacroTree);

    for (int n=1;n<RC_NUM_ICONS;n++){
      if (hOld[n]) DestroyIcon(hOld[n]);
      if (hOldSmall[n]) DestroyIcon(hOldSmall[n]);
    }
  }
#endif
}
//---------------------------------------------------------------------------
bool MakeGUI()
{
	fnt=(HFONT)GetStockObject(DEFAULT_GUI_FONT);

  MidGUIRGB=GetMidColour(GetSysColor(COLOR_3DFACE),GetSysColor(COLOR_WINDOW));
  DkMidGUIRGB=GetMidColour(GetSysColor(COLOR_3DFACE),MidGUIRGB);

  PCArrow=LoadCursor(NULL,IDC_ARROW);

  ParentWin=GetDesktopWindow();

  WNDCLASS wc={0,WndProc,0,0,Inst,hGUIIcon[RC_ICO_APP],
                PCArrow,NULL,NULL,"Steem Window"};
  RegisterClass(&wc);

  wc.lpfnWndProc=FSClipWndProc;
  wc.hIcon=NULL;
  wc.hCursor=NULL;
  wc.lpszClassName="Steem Fullscreen Clip Window";
  RegisterClass(&wc);

  wc.lpfnWndProc=FSQuitWndProc;
  wc.lpszClassName="Steem Fullscreen Quit Button";
  RegisterClass(&wc);

  wc.lpfnWndProc=ResetInfoWndProc;
  wc.lpszClassName="Steem Reset Info Window";
  RegisterClass(&wc);

  RegisterSteemControls();
  RegisterButtonPicker();

  StemWin=CreateWindowEx(WS_EX_ACCEPTFILES,"Steem Window",stem_window_title,WS_CAPTION | WS_SYSMENU |
                          WS_MINIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_CLIPSIBLINGS,
                          180,180,324 + GetSystemMetrics(SM_CXFRAME)*2,
                          204+MENUHEIGHT + GetSystemMetrics(SM_CYFRAME)*2 + GetSystemMetrics(SM_CYCAPTION),
                          ParentWin,NULL,Inst,NULL);

  if (StemWin==NULL) return 0;
  if (IsWindow(StemWin)==0){
    StemWin=NULL;
    return 0;
  }

  StemWin_SysMenu=GetSystemMenu(StemWin,0);
  int pos=GetMenuItemCount(StemWin_SysMenu)-2;
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION | MF_STRING,105,T("Smaller Window"));
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION | MF_STRING,104,T("Bigger Window"));
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION | MF_SEPARATOR,0,NULL);
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION | MF_STRING,112,T("Auto Borders"));
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION | MF_STRING,111,T("Always Show Borders"));
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION | MF_STRING,110,T("Never Show Borders"));
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION | MF_SEPARATOR,0,NULL);
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION | MF_STRING,113,T("Disable On Screen Display"));
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION | MF_SEPARATOR,0,NULL);
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION | MF_STRING,102,T("Always On Top"));
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION | MF_STRING,103,T("Restore Aspect Ratio"));
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION | MF_STRING,101,T("Normal Size"));
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION | MF_SEPARATOR,0,NULL);

  ToolTip=CreateWindowEx(WS_EX_TOPMOST,TOOLTIPS_CLASS,NULL,TTS_ALWAYSTIP | TTS_NOPREFIX,
                          0,0,100,100,NULL,NULL,Inst,NULL);
#ifdef _DEBUG_BUILD
  SendMessage(ToolTip,TTM_SETDELAYTIME,TTDT_AUTOPOP,20000); // 20 seconds before disappear
  SendMessage(ToolTip,TTM_SETDELAYTIME,TTDT_INITIAL,100);   // 0.1 of a second before appear
  SendMessage(ToolTip,TTM_SETDELAYTIME,TTDT_RESHOW,10);     // No time when moving from one tool to next
  SendMessage(ToolTip,TTM_SETMAXTIPWIDTH,0,150);
#else
  SendMessage(ToolTip,TTM_SETDELAYTIME,TTDT_AUTOPOP,20000); // 20 seconds before disappear
  SendMessage(ToolTip,TTM_SETDELAYTIME,TTDT_INITIAL,400);   // 0.4 second before appear
  SendMessage(ToolTip,TTM_SETDELAYTIME,TTDT_RESHOW,200);     // 0.2 moving from one tool to next
  SendMessage(ToolTip,TTM_SETMAXTIPWIDTH,0,400);
#endif

#ifndef ONEGAME
  int x=0;
  HWND Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_PLAY),WS_CHILDWINDOW | WS_VISIBLE |
                          PBS_RIGHTCLICK,x,0,20,20,StemWin,(HMENU)101,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Run (Right Click = Slow Motion)"));
  x+=23;

  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_FF),WS_CHILDWINDOW | WS_VISIBLE |
                          PBS_RIGHTCLICK | PBS_DBLCLK,x,0,20,20,StemWin,(HMENU)109,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Fast Forward (Right Click = Searchlight, Double Click = Sticky)"));
  x+=23;

  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_RESET),WS_CHILDWINDOW | WS_VISIBLE |
                          PBS_RIGHTCLICK,x,0,20,20,StemWin,(HMENU)102,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Reset (Left Click = Cold, Right Click = Warm)"));
  x+=23;

  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_SNAPSHOTBUT),WS_CHILDWINDOW | WS_VISIBLE,
                          x,0,20,20,StemWin,(HMENU)108,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Memory Snapshot Menu"));
  x+=23;

  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_TAKESCREENSHOTBUT),WS_CHILDWINDOW | WS_VISIBLE |
                          PBS_RIGHTCLICK,x,0,20,20,StemWin,(HMENU)115,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Take Screenshot")+" ("+T("Right Click = Options")+")");
  x+=23;

  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_PASTE),WS_CHILD | WS_VISIBLE | PBS_RIGHTCLICK,
                          x,0,20,20,StemWin,(HMENU)114,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Paste Text Into ST (Right Click = Options)"));
  x+=23;
#ifdef _RELEASE_BUILD
  // This causes freeze up if tracing in debugger, so only do it in final build
  NextClipboardViewerWin=SetClipboardViewer(StemWin);
#endif
  UpdatePasteButton();

  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_UPDATE),WS_CHILD,
                          x,0,20,20,StemWin,(HMENU)120,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Steem Update Available! Click Here For Details!"));


  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_INFO),WS_CHILD | WS_VISIBLE,
                            100,0,20,20,StemWin,(HMENU)105,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("General Info"));

  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_PATCHES),WS_CHILD | WS_VISIBLE,
                            100,0,20,20,StemWin,(HMENU)113,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Patches"));

  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_SHORTCUT),WS_CHILD | WS_VISIBLE,
                            100,0,20,20,StemWin,(HMENU)112,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Shortcuts"));

  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_OPTIONS),WS_CHILD | WS_VISIBLE,
                          100,0,20,20,StemWin,(HMENU)107,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Options"));

  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_JOY),WS_CHILDWINDOW | WS_VISIBLE,
                          100,0,20,20,StemWin,(HMENU)103,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Joystick Configuration"));

  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_DISKMAN),WS_CHILDWINDOW | WS_VISIBLE,
                          100,0,20,20,StemWin,(HMENU)100,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Disk Manager"));

  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_TOWINDOW),WS_CHILD,
                          120,0,20,20,StemWin,(HMENU)106,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Windowed Mode"));

  SetWindowAndChildrensFont(StemWin,fnt);
#endif

#ifndef ONEGAME
  CentreWindow(StemWin,0);
#else
  MoveWindow(StemWin,GetScreenWidth(),0,100,100,0);
#endif

#ifdef _DEBUG_BUILD
  log("STARTUP: DWin_init Called");
  DWin_init();
#endif

  return true;
}
//---------------------------------------------------------------------------
void CheckResetIcon()
{
  if (StemWin==NULL) return;

  HWND ResetBut=GetDlgItem(StemWin,102);
  if (ResetBut==NULL) return;

  int new_icon=int(OptionBox.NeedReset() ? RC_ICO_RESETGLOW:RC_ICO_RESET);
  Str CurNumText;
  CurNumText.SetLength(20);
  GetWindowText(ResetBut,CurNumText,20);
  if (atoi(CurNumText)!=new_icon) SetWindowText(ResetBut,Str(new_icon));
}
//---------------------------------------------------------------------------
void CheckResetDisplay(bool NOT_ONEGAME(AlwaysHide))
{
#ifndef ONEGAME
  if (pc==rom_addr && StemWin && runstate==RUNSTATE_STOPPED && AlwaysHide==0){
    if (ResetInfoWin==NULL){
      if (FullScreen==0) SetWindowLong(StemWin,GWL_STYLE,GetWindowLong(StemWin,GWL_STYLE) | WS_CLIPCHILDREN);
      ResetInfoWin=CreateWindow("Steem Reset Info Window","",WS_CHILD,
                            0,0,0,0,HWND(FullScreen ? ClipWin:StemWin),(HMENU)9876,Inst,NULL);
      SendMessage(ResetInfoWin,WM_USER,1789,0);
      ShowWindow(ResetInfoWin,SW_SHOWNA);
    }else{
      SendMessage(ResetInfoWin,WM_USER,1789,0);
      InvalidateRect(ResetInfoWin,NULL,0);
    }
  }else if (ResetInfoWin){
    HWND Win=ResetInfoWin;
    ResetInfoWin=NULL;
    DestroyWindow(Win);
    SetWindowLong(StemWin,GWL_STYLE,GetWindowLong(StemWin,GWL_STYLE) & ~WS_CLIPCHILDREN);
  }
#endif
}
//---------------------------------------------------------------------------

#undef LOGSECTION
#define LOGSECTION LOGSECTION_SHUTDOWN

void CleanupGUI()
{
  WNDCLASS wc;
#ifdef _DEBUG_BUILD
  DWin_edit_is_being_temporarily_defocussed=true;
  log("SHUTDOWN: Destroying debug-build menus");
  if (insp_menu) DestroyMenu(insp_menu);

  if (trace_window_handle) DestroyWindow(trace_window_handle);
  trace_window_handle=NULL;

  log("SHUTDOWN: Destroying Boiler Room Mr Statics");
  if (DWin){
    mr_static_delete_children_of(DWin);
    mr_static_delete_children_of(DWin_timings_scroller.GetControlPage());
  }
  
  log("SHUTDOWN: Destroying debug-build Boiler Room window");
  if (DWin) DestroyWindow(DWin);

  log("SHUTDOWN: Destroying debug-build memory browsers");
  for (int n=0;n<MAX_MEMORY_BROWSERS;n++){
    if (m_b[n]!=NULL){
      if (m_b[n]->owner!=NULL){
        if (IsWindow(m_b[n]->owner)){
          DestroyWindow(m_b[n]->owner);
          n--;
        }
      }
    }

  }

  debug_plugin_free();

  if (HiddenParent) DestroyWindow(HiddenParent);

  if (GetClassInfo(Inst,"Steem Debug Window",&wc)){
    UnregisterClass("Steem Debug Window",Inst);
  }
  if (GetClassInfo(Inst,"Steem Trace Window",&wc)){
    UnregisterClass("Steem Mem Browser Window",Inst);
    UnregisterClass("Steem Trace Window",Inst);
  }
  if (mem_browser::icons_bmp){
    DeleteDC(mem_browser::icons_dc);
    DeleteObject(mem_browser::icons_bmp);
  }
#endif

  log("SHUTDOWN: Destroying StemWin");
  if (StemWin){
    CheckResetDisplay(true);
    DestroyWindow(StemWin);
    StemWin=NULL;
  }
  log("SHUTDOWN: Destroying ToolTip");
  if (ToolTip) DestroyWindow(ToolTip);

  if (GetClassInfo(Inst,"Steem Window",&wc)){
    UnregisterSteemControls();
    UnregisterButtonPicker();
    UnregisterClass("Steem Window",Inst);
    UnregisterClass("Steem Fullscreen Clip Window",Inst);
  }

  log("SHUTDOWN: Calling CoUninitialize()");
  CoUninitialize();

  for (int n=1;n<RC_NUM_ICONS;n++) if (hGUIIcon[n]) DestroyIcon(hGUIIcon[n]);
}


#undef LOGSECTION
#define LOGSECTION LOGSECTION_INIT

#endif

bool ComLineArgCompare(char*Arg,char*s,bool truncate=false)
{
  if (*Arg=='/' || *Arg=='-') Arg++;
  if (*Arg=='-') Arg++;
  if (truncate) return(IsSameStr_I(EasyStr(Arg).Lefts(strlen(s)),s));
  return IsSameStr_I(Arg,s);
}

int GetComLineArgType(char *Arg,EasyStr &Path)
{
  if (ComLineArgCompare(Arg,"NODD") || ComLineArgCompare(Arg,"GDI")){
    return ARG_GDI;
  }else if (ComLineArgCompare(Arg,"NODS") || ComLineArgCompare(Arg,"NOSOUND")){
    return ARG_NODS;
  }else if (ComLineArgCompare(Arg,"WINDOW")){
    return ARG_WINDOW;
  }else if (ComLineArgCompare(Arg,"FULLSCREEN")){
    return ARG_FULLSCREEN;
  }else if (ComLineArgCompare(Arg,"NONEW")){
    return ARG_NONEWINSTANCE;
  }else if (ComLineArgCompare(Arg,"OPENNEW")){
    return ARG_ALWAYSNEWINSTANCE;
  }else if (ComLineArgCompare(Arg,"NOLPT")){
    return ARG_NOLPT;
  }else if (ComLineArgCompare(Arg,"NOCOM")){
    return ARG_NOCOM;
  }else if (ComLineArgCompare(Arg,"NOSHM")){
    return ARG_NOSHM;
  }else if (ComLineArgCompare(Arg,"SCLICK")){
    return ARG_SOUNDCLICK;
  }else if (ComLineArgCompare(Arg,"HELP") || ComLineArgCompare(Arg,"H")){
    return ARG_HELP;
  }else if (ComLineArgCompare(Arg,"QUITQUICKLY")){
    return ARG_QUITQUICKLY;
  }else if (ComLineArgCompare(Arg,"DOUBLECHECKSHORTCUTS")){
    return ARG_DOUBLECHECKSHORTCUTS;
  }else if (ComLineArgCompare(Arg,"DONTLIMITSPEED")){
    return ARG_DONTLIMITSPEED;
  }else if (ComLineArgCompare(Arg,"ACCURATEFDC")){
    return ARG_ACCURATEFDC;
  }else if (ComLineArgCompare(Arg,"NOPCJOYSTICKS")){
    return ARG_NOPCJOYSTICKS;
  }else if (ComLineArgCompare(Arg,"OLDPORTIO")){
    return ARG_OLDPORTIO;
  }else if (ComLineArgCompare(Arg,"ALLOWREADOPEN")){
    return ARG_ALLOWREADOPEN;
  }else if (ComLineArgCompare(Arg,"NOINTS")){
    return ARG_NOINTS;
  }else if (ComLineArgCompare(Arg,"STFMBORDER")){
    return ARG_STFMBORDER;
  }else if (ComLineArgCompare(Arg,"SCREENSHOTUSEFULLNAME")){
    return ARG_SCREENSHOTUSEFULLNAME;
  }else if (ComLineArgCompare(Arg,"SCREENSHOTALWAYSADDNUM")){
    return ARG_SCREENSHOTALWAYSADDNUM;
  }else if (ComLineArgCompare(Arg,"ALLOWLPTINPUT")){
    return ARG_ALLOWLPTINPUT;
  }else if (ComLineArgCompare(Arg,"NONOTIFYINIT")){
    return ARG_NONOTIFYINIT;
  }else if (ComLineArgCompare(Arg,"PSGCAPTURE")){
    return ARG_PSGCAPTURE;
  }else if (ComLineArgCompare(Arg,"CROSSMOUSE")){
    return ARG_CROSSMOUSE;
  }else if (ComLineArgCompare(Arg,"RUN")){
    return ARG_RUN;
  }else if (ComLineArgCompare(Arg,"GDIFSBORDER")){
    return ARG_GDIFSBORDER;
  }else if (ComLineArgCompare(Arg,"PASTI")){
    return ARG_PASTI;
  }else if (ComLineArgCompare(Arg,"NOPASTI")){
    return ARG_NOPASTI;
  }else if (ComLineArgCompare(Arg,"NOAUTOSNAPSHOT")){
    return ARG_NOAUTOSNAPSHOT;

  }else if (ComLineArgCompare(Arg,"SOF=",true)){
    Path=strchr(Arg,'=')+1;
    return ARG_SETSOF;
  }else if (ComLineArgCompare(Arg,"FONT=",true)){
    Path=strchr(Arg,'=')+1;
    return ARG_SETFONT;
  }else if (ComLineArgCompare(Arg,"SCREENSHOT=",true)){
    Path=strchr(Arg,'=')+1;
    return ARG_TAKESHOT;
  }else if (ComLineArgCompare(Arg,"SCREENSHOT",true)){
    Path="";
    return ARG_TAKESHOT;
  }else if (ComLineArgCompare(Arg,"DIVUTIME=",true)){
    Path=strchr(Arg,'=')+1;
    return ARG_SETDIVUTIME;
  }else if (ComLineArgCompare(Arg,"DIVSTIME=",true)){
    Path=strchr(Arg,'=')+1;
    return ARG_SETDIVSTIME;
  }else if (ComLineArgCompare(Arg,"PABUFSIZE=",true)){
    Path=strchr(Arg,'=')+1;
    return ARG_SETPABUFSIZE;
  }else if (ComLineArgCompare(Arg,"RTBUFSIZE",true)){
    Path=strchr(Arg,'=')+1;
    return ARG_RTBUFSIZE;
  }else if (ComLineArgCompare(Arg,"RTBUFNUM",true)){
    Path=strchr(Arg,'=')+1;
    return ARG_RTBUFNUM;

  }else{
    int Type=ARG_UNKNOWN;
    char *pArg=Arg;
    if (ComLineArgCompare(Arg,"INI=",true)){
      pArg=strchr(Arg,'=')+1;
      Type=ARG_SETINIFILE;
    }else if (ComLineArgCompare(Arg,"TRANS=",true)){
      pArg=strchr(Arg,'=')+1;
      Type=ARG_SETTRANSFILE;
    }else if (ComLineArgCompare(Arg,"CUTS=",true)){
      pArg=strchr(Arg,'=')+1;
      Type=ARG_SETCUTSFILE;
    }
    Path.SetLength(MAX_PATH);
    GetLongPathName(pArg,Path,MAX_PATH);

    if (Type!=ARG_UNKNOWN) return Type;

    char *dot=strrchr(GetFileNameFromPath(Path),'.');
    if (dot){
      if (ExtensionIsDisk(dot,false)){
        return ARG_DISKIMAGEFILE;
      }else if (ExtensionIsPastiDisk(dot)){
        return ARG_PASTIDISKIMAGEFILE;
      }else if (IsSameStr_I(dot,".STS")){
        return ARG_SNAPSHOTFILE;
      }else if (IsSameStr_I(dot,".STC")){
        return ARG_CARTFILE;
      }else if (IsSameStr_I(dot,".PRG") || IsSameStr_I(dot,".APP") || IsSameStr_I(dot,".TOS")){
        return ARG_STPROGRAMFILE;
      }else if (IsSameStr_I(dot,".GTP") || IsSameStr_I(dot,".TTP")){

        return ARG_STPROGRAMTPFILE;
      }else if (IsSameStr_I(dot,".LNK")){
        return ARG_LINKFILE;
      }else if (IsSameStr_I(dot,".IMG") || IsSameStr_I(dot,".ROM")){
        return ARG_TOSIMAGEFILE;
      }
    }
    return ARG_UNKNOWN;
  }
}

void ParseCommandLine(int NumArgs,char *Arg[],int Level)
{
  log("STARTUP: Command line arguments:");
  for (int n=0;n<NumArgs;n++){
    log(Str("     ")+Arg[n]);
    EasyStr Path;
    int Type=GetComLineArgType(Arg[n],Path);
    switch (Type){
      case ARG_GDI:    WIN_ONLY( TryDD=0; ) break;
      case ARG_NODS:   TrySound=0; break;
      case ARG_NOSHM:  UNIX_ONLY( TrySHM=0; ) break;
      case ARG_NOLPT:  AllowLPT=0; break;
      case ARG_NOCOM:  AllowCOM=0; break;
      case ARG_WINDOW: BootInMode&=~BOOT_MODE_FLAGS_MASK;BootInMode=BOOT_MODE_WINDOW; break;
      case ARG_FULLSCREEN: BootInMode&=~BOOT_MODE_FLAGS_MASK;BootInMode=BOOT_MODE_FULLSCREEN; break;
      case ARG_SETSOF: sound_comline_freq=atoi(Path);sound_chosen_freq=sound_comline_freq;break;
      case ARG_SOUNDCLICK: sound_click_at_start=true; break;
      case ARG_DOUBLECHECKSHORTCUTS: WIN_ONLY( DiskMan.DoExtraShortcutCheck=true; ) break;
      case ARG_DONTLIMITSPEED: disable_speed_limiting=true; break;
      case ARG_ACCURATEFDC: floppy_instant_sector_access=0; break;
      case ARG_NOPCJOYSTICKS: DisablePCJoysticks=true; break;
      case ARG_OLDPORTIO: WIN_ONLY( TPortIO::AlwaysUseNTMethod=0; ) break;
      case ARG_SETDIVUTIME: m68k_divu_cycles=atoi(Path)-4; break;
      case ARG_SETDIVSTIME: m68k_divs_cycles=atoi(Path)-4; break;
      case ARG_TAKESHOT:
        Disp.ScreenShotNextFile=Path;
        if (runstate==RUNSTATE_RUNNING){
          DoSaveScreenShot|=1;
        }else{
          Disp.SaveScreenShot();
        }
        break;
      case ARG_SETPABUFSIZE:  UNIX_ONLY( pa_output_buffer_size=atoi(Path); ) break;
      case ARG_ALLOWREADOPEN: stemdos_comline_read_is_rb=true; break;
      case ARG_NOINTS:        _no_ints=true; break;
      case ARG_STFMBORDER:    stfm_borders=4; break;
      case ARG_SCREENSHOTUSEFULLNAME: Disp.ScreenShotUseFullName=true; break;
      case ARG_SCREENSHOTALWAYSADDNUM: Disp.ScreenShotAlwaysAddNum=true; break;
      case ARG_ALLOWLPTINPUT: comline_allow_LPT_input=true; break;
      case ARG_PSGCAPTURE: psg_always_capture_on_start=true; break;
      case ARG_CROSSMOUSE: no_set_cursor_pos=true; break;
#if defined(UNIX) && !defined(NO_RTAUDIO)
      case ARG_RTBUFSIZE: rt_buffer_size=atoi(Path); break;
      case ARG_RTBUFNUM: rt_buffer_num=atoi(Path); break;
#endif
      case ARG_RUN: BootInMode|=BOOT_MODE_RUN; break;
WIN_ONLY( case ARG_GDIFSBORDER:   Disp.DrawLetterboxWithGDI=true; break; )
      case ARG_PASTI: BootPasti=BOOT_PASTI_ON; break;
      case ARG_NOPASTI: BootPasti=BOOT_PASTI_OFF; break;
      case ARG_NOAUTOSNAPSHOT:
        BootDisk[0]=".";
        BootDisk[1]=".";
        break;

      case ARG_DISKIMAGEFILE:
        if (BootDisk[1].Empty()) BootDisk[int(BootDisk[0].Empty() ? 0:1)]=Path;
        break;
      case ARG_PASTIDISKIMAGEFILE:
        BootPasti=BOOT_PASTI_ON;
        if (BootDisk[1].Empty()) BootDisk[int(BootDisk[0].Empty() ? 0:1)]=Path;
        break;
      case ARG_SNAPSHOTFILE:
        BootDisk[0]=".";
        BootDisk[1]=".";
        BootStateFile=Path;
        break;
      case ARG_CARTFILE:
        if (load_cart(Path)==0){
          CartFile=Path;
          OptionBox.MachineUpdateIfVisible();
        }
        break;
      case ARG_STPROGRAMFILE:
        // Mount folder as Z: (disable all normal hard drives)
        // Copy autorun program into auto folder
        break;
      case ARG_LINKFILE:
#ifdef WIN32
        if (Level<10){
          WIN32_FIND_DATA wfd;
          Path=GetLinkDest(Path,&wfd);
          if (Path.NotEmpty()){
            ParseCommandLine(1,&(Path.Text),Level+1);
          }
        }
#endif
        break;
      case ARG_TOSIMAGEFILE:
        if (BootTOSImage==0){
          if (load_TOS(Path)==0){
            ROMFile=Path;
            BootTOSImage=true;
          }
        }
        break;
    }
  }
}
//---------------------------------------------------------------------------
bool OpenComLineFilesInCurrent(bool AlwaysSendToCurrent)
{
  EasyStringList esl;
  esl.Sort=eslNoSort;
  for (int n=0;n<_argc-1;n++){
    EasyStr Path;
    int Type=GetComLineArgType(_argv[1+n],Path);
#ifdef WIN32
    if (Type==ARG_LINKFILE){
      WIN32_FIND_DATA wfd;
      int Level=0;
      EasyStr DestPath=Path;
      do{

        DestPath=GetLinkDest(DestPath,&wfd);
        if (DestPath.NotEmpty()) break;
        Type=GetComLineArgType(DestPath,Path);
      }while (Type==ARG_LINKFILE && (++Level)<10);
    }
#endif
    switch (Type){
      case ARG_DISKIMAGEFILE:case ARG_PASTIDISKIMAGEFILE:case ARG_SNAPSHOTFILE:
      case ARG_CARTFILE:case ARG_TOSIMAGEFILE:
        esl.Add(Path,0);
      case ARG_TAKESHOT:
        esl.Add(_argv[1+n],0);
        break;
      case ARG_RUN:
        esl.Add(_argv[1+n],1);
        break;
    }
  }
  if (esl.NumStrings){
    bool RunOnly=true;
    for (int i=0;i<esl.NumStrings;i++){
      if (esl[i].Data[0]==0){
        RunOnly=0;
        break;
      }
    }
    // If you only pass the RUN command and haven't specified to open in current
    // then we shouldn't do anything, RUN is handled later.
    if (RunOnly && AlwaysSendToCurrent==0) return 0;
    // Send strings to running Steem
#ifdef WIN32
    HWND CurSteemWin=FindWindow("Steem Window",NULL);
    if (CurSteemWin){
      bool Success=0;
      COPYDATASTRUCT cds;
      cds.dwData=MAKECHARCONST('S','C','O','M');
      for (int n=0;n<esl.NumStrings;n++){
        cds.cbData=strlen(esl[n].String)+1;
        cds.lpData=esl[n].String;
        if (SendMessage(CurSteemWin,WM_COPYDATA,0,LPARAM(&cds))==MAKECHARCONST('Y','A','Y','S')) Success=true;
      }
      if (Success) return true;
    }
#elif defined(UNIX)
#endif
  }
  return 0;
}
#undef LOGSECTION
//---------------------------------------------------------------------------
void InitTranslations()
{
  if (Exists(TranslateFileName)==0){
    TranslateFileName="";
    DirSearch ds;
    if (ds.Find(RunDir+SLASH "Translate_*.txt")) TranslateFileName=RunDir+SLASH+ds.Name;
  }
  if (TranslateFileName.Empty()) return;

  FILE *f=fopen(TranslateFileName,"rb");
  if (f==NULL) return;

  int FileLen=GetFileLength(f);
  TranslateBufLen=FileLen+2000;
  TranslateBuf=new char[TranslateBufLen+8];
  fread(TranslateBuf,FileLen,1,f);
  TranslateBuf[FileLen]=0;
  fclose(f);

  TranslateUpperBuf=new char[strlen(TranslateBuf)+1];
  strcpy(TranslateUpperBuf,TranslateBuf);
  strupr(TranslateUpperBuf);
}
//---------------------------------------------------------------------------
EasyStr Translation(char *s)
{
#ifdef TRANSLATION_TEST
  if (TranslateBuf==NULL) return Str(s).UpperCase();
#else
  if (TranslateBuf==NULL) return s;
#endif
  if (s[0]==0) return s;

  EasyStr UpperS=s;
  strupr(UpperS);

  char *EStart=TranslateUpperBuf-1,*EEnd,*TStart,*TEnd;
  bool Match;
  do{
    EStart=strstr(EStart+1,UpperS);
    if (EStart==NULL) break;
    Match=true;
    if (EStart>TranslateUpperBuf){
      if (*(EStart-1)!='\n') Match=0;
    }
    EEnd=EStart+strlen(s);
    if (EEnd>=TranslateUpperBuf+TranslateBufLen){
      EStart=NULL;break;
    }
    if (*EEnd=='\r'){
      EEnd+=2;
    }else if (*EEnd=='\n'){
      EEnd++;
    }else{
      Match=0;
    }
    if (EEnd>=TranslateUpperBuf+TranslateBufLen){
      EStart=NULL;break;
    }
    if (*EEnd=='\r' || *EEnd=='\n') Match=0;
  }while (Match==0);

  if (EStart==NULL){
    FILE *f=fopen(TranslateFileName,"ab");
    if (f){
      fprintf(f,"\r\n\r\n%s\r\n%s",s,s);
      fclose(f);

      if (strlen(TranslateBuf)+6+strlen(s)*2 >= size_t(TranslateBufLen-8)){
        TranslateBufLen=strlen(TranslateBuf)+6+strlen(s)*2+2000;
        char *Temp=new char[TranslateBufLen+8];
        strcpy(Temp,TranslateBuf);

        delete[] TranslateBuf;
        TranslateBuf=Temp;
      }
      strcat(TranslateBuf,EasyStr("\r\n\r\n")+s+"\r\n"+s);

      delete[] TranslateUpperBuf;
      TranslateUpperBuf=new char[strlen(TranslateBuf)+1];
      strcpy(TranslateUpperBuf,TranslateBuf);
      strupr(TranslateUpperBuf);
    }
#ifdef TRANSLATION_TEST
    return EasyStr(s).UpperCase();
#else
    return s;
#endif
  }
  EEnd-=DWORD(TranslateUpperBuf);
  EEnd+=DWORD(TranslateBuf);

  TStart=EEnd;

  EasyStr Ret;
  
  TEnd=strchr(TStart,'\n');
  if (TEnd){
    if (*(TEnd-1)=='\r') TEnd--;
    char OldEndChar=*TEnd;
    *TEnd=0;
    Ret=TStart;
    *TEnd=OldEndChar;
  }else{
    Ret=TStart;
  }

#ifdef TRANSLATION_TEST
  return Ret.UpperCase();
#else
  return Ret;
#endif
}

//---------------------------------------------------------------------------
char FileTypes[512];

char *FSTypes(int Type,...)
{
  char *tp=FileTypes;
  ZeroMemory(FileTypes,512);

  if (Type==2){
    strcpy(tp,T("Disk Images"));tp+=strlen(tp)+1;
    strcpy(tp,"*.st;*.stt;*.msa;*.dim;*.zip;*.stz");tp+=strlen(tp);
#ifdef RAR_SUPPORT
    strcpy(tp,";*.rar");tp+=strlen(tp);
#endif
#if USE_PASTI
    if (hPasti){
      tp[0]=';';tp++;
      pasti->GetFileExtensions(tp,160,TRUE); // will add "*.st;*.stx"
      tp+=strlen(tp);
    }
#endif
    tp++;
  }else if (Type==3){
    strcpy(tp,T("TOS Images"));tp+=strlen(tp)+1;
    strcpy(tp,"*.img;*.rom");tp+=strlen(tp)+1;
  }else{
    char **pStr=((char**)&Type)+1;
    while (*pStr){
      strcpy(tp,*pStr);
      tp+=strlen(*pStr)+1;
      pStr++;
      strcpy(tp,*pStr);
      tp+=strlen(*pStr)+1;
      pStr++;
    }
  }
  if (Type){
    strcpy(tp,T("All Files"));tp+=strlen(tp)+1;
    strcpy(tp,"*.*");
  }
  return FileTypes;
}
//---------------------------------------------------------------------------
bool CheckForSteemRunning()
{
#ifdef WIN32
  SteemRunningMutex=CreateMutex(NULL,0,"Steem_Running");
  return GetLastError()==ERROR_ALREADY_EXISTS;
#elif defined(UNIX)
  return 0;
#endif
}
//---------------------------------------------------------------------------
bool CleanupTempFiles()
{
  bool SteemHasCrashed=0;

  for (int n=0;n<4;n++){
    char *prefix="MSA";
    switch (n){
      case 1: prefix="ZIP"; break;
      case 2: prefix="FMT"; break;
      case 3: prefix="CRA"; break;
    }
    DirSearch ds;
    if (ds.Find(WriteDir+SLASH+prefix+"*.TMP")){
      EasyStringList FileESL;
      do{
        FileESL.Add(WriteDir+SLASH+ds.Name);
      }while (ds.Next());
      ds.Close();

      for (int i=0;i<FileESL.NumStrings;i++) DeleteFile(FileESL[i].String);

      if (n==3) SteemHasCrashed=true;
    }
  }
  return SteemHasCrashed;
}
//---------------------------------------------------------------------------

#ifdef WIN32
//---------------------------------------------------------------------------
void EnableWindow2(HWND Win,bool Enable,HWND NoDisable)
{
  if (Win!=NoDisable){
    SetWindowLong(Win,GWL_STYLE,(GetWindowLong(Win,GWL_STYLE) & int(Enable ? ~WS_DISABLED:0xffffffff)) | int(Enable==0 ? WS_DISABLED:0));
  }
}
//---------------------------------------------------------------------------
void EnableAllWindows(bool Enable,HWND NoDisable)
{
  if (Enable){
    DisableFocusWin=NULL;
  }else{
    DisableFocusWin=NoDisable;
  }

  DEBUG_ONLY( EnableWindow2(DWin,Enable,NoDisable) );
  DEBUG_ONLY( if (trace_window_handle) EnableWindow2(trace_window_handle,Enable,NoDisable) );
  EnableWindow2(StemWin,Enable,NoDisable);
  if (DiskMan.Handle){
    if (HardDiskMan.Handle){
      EnableWindow2(HardDiskMan.Handle,Enable,NoDisable);
    }else if (DiskMan.VisibleDiag()==NULL){
      EnableWindow2(DiskMan.Handle,Enable,NoDisable);
    }else{
      EnableWindow2(DiskMan.VisibleDiag(),Enable,NoDisable);
    }
  }
  for (int n=0;n<nStemDialogs;n++){
    if (DialogList[n]!=&DiskMan){
      if (DialogList[n]->Handle) EnableWindow2(DialogList[n]->Handle,Enable,NoDisable);
    }
  }
}
//---------------------------------------------------------------------------
void ShowAllDialogs(bool Show)
{
  if (FullScreen==0) return;

  static bool DiskManWasMaximized=0;
  int PosChange=int(Show ? -3000:3000);
  if (DiskMan.Handle){
    if (DiskMan.FSMaximized && Show==0) DiskManWasMaximized=true;
    if (DiskManWasMaximized && Show){
      SetWindowPos(DiskMan.Handle,NULL,-GetSystemMetrics(SM_CXFRAME),MENUHEIGHT,
                    int((border & 1) ? 800:640)+GetSystemMetrics(SM_CXFRAME)*2,
                    int((border & 1) ? 600:480)+GetSystemMetrics(SM_CYFRAME)-MENUHEIGHT,
                    SWP_NOZORDER | SWP_NOACTIVATE);
      DiskManWasMaximized=0;
    }else{
      DiskMan.FSLeft+=PosChange;
      SetWindowPos(DiskMan.Handle,NULL,DiskMan.FSLeft,DiskMan.FSTop,0,0,SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
    }
  }
  for (int n=0;n<nStemDialogs;n++){
    if (DialogList[n]!=&DiskMan){
      if (DialogList[n]->Handle){
        DialogList[n]->FSLeft+=PosChange;
        SetWindowPos(DialogList[n]->Handle,NULL,DialogList[n]->FSLeft,DialogList[n]->FSTop,0,0,SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
      }
    }
  }
  if (FSQuitBut) ShowWindow(FSQuitBut,int(Show ? SW_SHOWNA:SW_HIDE));
}
//---------------------------------------------------------------------------
void HandleKeyPress(UINT VKCode,bool Up,int Extended)
{
  if (disable_input_vbl_count) return;
  if (ikbd_keys_disabled()) return; //in duration mode
  if (macro_play_has_keys) return;

  BYTE STCode=0;

  if ((Extended & 3)==1){
    switch (LOBYTE(VKCode)){
      case VK_RETURN: STCode=STKEY_PAD_ENTER; break;
      case VK_DIVIDE: STCode=STKEY_PAD_DIVIDE; break;
    }
  }

  bool DidShiftSwitching=0;
  int ModifierRestoreArray[3];
  if (EnableShiftSwitching && shift_key_table[0] && (Extended & NO_SHIFT_SWITCH)==0){
    if (STCode==0){
      HandleShiftSwitching(VKCode,Up,STCode,ModifierRestoreArray);
      if (STCode) DidShiftSwitching=true;
    }
  }
  
  if (STCode==0) STCode=key_table[BYTE(VKCode)]; //SS: +- ASCII -> ST scancode
  if (STCode){
    ST_Key_Down[STCode]=!Up;
    if (Up) STCode|=MSB_B;
    keyboard_buffer_write_n_record(STCode);
    
#ifndef DISABLE_STEMDOS
    if (VKCode=='C'){ //control-C
      if (ST_Key_Down[key_table[VK_CONTROL]]){ //control-C
        stemdos_control_c();
      }
    }
#endif
  }//SS: if (STCode): if not, it's a PC key that doesn't translate to ST
  if (DidShiftSwitching) ShiftSwitchRestoreModifiers(ModifierRestoreArray);
}
//---------------------------------------------------------------------------
void SetStemMouseMode(int NewMM)
{
  //TRACE("SetStemMouseMode(%d)\n",NewMM);
  static POINT OldMousePos={-1,0};
//  if (NewMM==stem_mousemode) return;

  if (stem_mousemode!=STEM_MOUSEMODE_WINDOW && NewMM==STEM_MOUSEMODE_WINDOW) GetCursorPos(&OldMousePos);
  stem_mousemode=NewMM;
  if (NewMM==STEM_MOUSEMODE_WINDOW){
    if (no_set_cursor_pos){
      SetCursor(LoadCursor(NULL,RCNUM(IDC_CROSS)));
      POINT pt;
      GetCursorPos(&pt);
      window_mouse_centre_x=pt.x;
      window_mouse_centre_y=pt.y;
    }else{
      SetCursor(NULL);
      RECT rc;
      GetWindowRect(StemWin,&rc);
      window_mouse_centre_x=rc.left+164+GetSystemMetrics(SM_CXFRAME);
      window_mouse_centre_y=rc.top+104+MENUHEIGHT+GetSystemMetrics(SM_CYFRAME)+GetSystemMetrics(SM_CYCAPTION);
      SetCursorPos(window_mouse_centre_x,window_mouse_centre_y);
    }

#ifndef _DEBUG_BUILD
    if (FullScreen){
      ClipCursor(NULL);
    }else{
      RECT rc;
      POINT pt={0,0};
      GetClientRect(StemWin,&rc);
      rc.right-=6;
      rc.bottom-=6+MENUHEIGHT;
      ClientToScreen(StemWin,&pt);
      OffsetRect(&rc,pt.x+3,pt.y+3+MENUHEIGHT);
      ClipCursor(&rc);
    }
#endif
  }else{
    SetCursor(PCArrow);
    if (FullScreen && runstate==RUNSTATE_RUNNING) runstate=RUNSTATE_STOPPING;
#ifndef _DEBUG_BUILD
    ClipCursor(NULL);
#endif
    if (OldMousePos.x>=0 && no_set_cursor_pos==0){
      SetCursorPos(OldMousePos.x,OldMousePos.y);
      OldMousePos.x=-1;
    }
  }
  mouse_move_since_last_interrupt_x=0;
  mouse_move_since_last_interrupt_y=0;
  mouse_change_since_last_interrupt=false;
}
//---------------------------------------------------------------------------
#define WH_KEYBOARD_LL 13
#define LLKHF_ALTDOWN 0x00000020

#ifndef _MINGW_BUILD
typedef struct{
  DWORD vkCode;
  DWORD scanCode;
  DWORD flags;
  DWORD time;
  DWORD dwExtraInfo;
}KBDLLHOOKSTRUCT, *LPKBDLLHOOKSTRUCT;
#endif

LRESULT CALLBACK NTKeyboardProc(INT nCode,WPARAM wParam,LPARAM lParam)
{
  KBDLLHOOKSTRUCT *pkbhs=LPKBDLLHOOKSTRUCT(lParam);

  if (nCode==HC_ACTION){
    bool ControlDown=(GetAsyncKeyState(VK_CONTROL) < 0),AltDown=(pkbhs->flags & LLKHF_ALTDOWN);
    bool ShiftDown=(GetAsyncKeyState(VK_SHIFT) < 0);

    if (pkbhs->vkCode==VK_TAB && AltDown) return 1;

    if (pkbhs->vkCode==VK_ESCAPE && (AltDown || ShiftDown || ControlDown)) return 1;

    if (pkbhs->vkCode==VK_DELETE && AltDown && ControlDown) return 1;

#ifdef ONEGAME
    if (pkbhs->vkCode==VK_LWIN || pkbhs->vkCode==VK_RWIN) return 1;
#endif
  }
  return CallNextHookEx(hNTTaskSwitchHook,nCode,wParam,lParam);
}
//---------------------------------------------------------------------------
void DisableTaskSwitch()
{
  if (TaskSwitchDisabled) return;

  if (WinNT){
    hNTTaskSwitchHook=SetWindowsHookEx(WH_KEYBOARD_LL,HOOKPROC(NTKeyboardProc),NULL,GetCurrentThreadId());
    if (hNTTaskSwitchHook==NULL){
      int Base=1400;
      RegisterHotKey(StemWin,Base++,MOD_ALT,VK_TAB);
      RegisterHotKey(StemWin,Base++,MOD_ALT | MOD_SHIFT,VK_TAB);


      RegisterHotKey(StemWin,Base++,MOD_ALT,VK_ESCAPE);
      RegisterHotKey(StemWin,Base++,MOD_ALT | MOD_SHIFT,VK_ESCAPE);

      RegisterHotKey(StemWin,Base++,MOD_CONTROL,VK_ESCAPE);
      RegisterHotKey(StemWin,Base++,MOD_CONTROL | MOD_ALT,VK_ESCAPE);
      RegisterHotKey(StemWin,Base++,MOD_CONTROL | MOD_SHIFT,VK_ESCAPE);
      RegisterHotKey(StemWin,Base++,MOD_CONTROL | MOD_ALT | MOD_SHIFT,VK_ESCAPE);

      RegisterHotKey(StemWin,Base++,MOD_CONTROL | MOD_ALT,VK_DELETE);
      RegisterHotKey(StemWin,Base++,MOD_CONTROL | MOD_ALT | MOD_SHIFT,VK_DELETE);

      RegisterHotKey(StemWin,Base++,MOD_SHIFT,VK_ESCAPE);

#ifdef ONEGAME
      RegisterHotKey(StemWin,Base++,0,VK_LWIN);
      RegisterHotKey(StemWin,Base++,0,VK_RWIN);
#endif
    }
  }else{
    UINT PrevSS;
    SystemParametersInfo(SPI_SETSCREENSAVERRUNNING,TRUE,&PrevSS,0);
  }
  TaskSwitchDisabled=true;
}
//---------------------------------------------------------------------------
void EnableTaskSwitch()
{
  if (TaskSwitchDisabled==0) return;

  if (WinNT){
    if (hNTTaskSwitchHook==NULL){
      for (int n=1400;n<1411 ONEGAME_ONLY(+2);n++) UnregisterHotKey(StemWin,n);
    }
  }else{
    UINT PrevSS;
    SystemParametersInfo(SPI_SETSCREENSAVERRUNNING,FALSE,&PrevSS,0);
  }

  if (hNTTaskSwitchHook){
    UnhookWindowsHookEx(hNTTaskSwitchHook);
    hNTTaskSwitchHook=NULL;
  }
  TaskSwitchDisabled=0;
}
//---------------------------------------------------------------------------
inline bool HandleMessage(MSG *mess)
{
  if (DiskMan.HasHandledMessage(mess))     return 0;
  if (HardDiskMan.HasHandledMessage(mess)) return 0;
  if (JoyConfig.HasHandledMessage(mess))   return 0;
  if (InfoBox.HasHandledMessage(mess))     return 0;
  if (OptionBox.HasHandledMessage(mess))   return 0;
  if (ShortcutBox.HasHandledMessage(mess)) return 0;
  if (PatchesBox.HasHandledMessage(mess)) return 0;
  return true;
}
//---------------------------------------------------------------------------
int ASMCALL PeekEvent()
{
  static MSG mess;
  if (PeekMessage(&mess,NULL,0,0,PM_REMOVE)==0) return PEEKED_NOTHING;

  if (mess.message==WM_QUIT){
    QuitSteem();
    return PEEKED_QUIT;
  }

  if (HandleMessage(&mess)){
    TranslateMessage(&mess);
    DispatchMessage(&mess);
  }
  return PEEKED_MESSAGE;
}
//---------------------------------------------------------------------------
void UpdatePasteButton()
{
#ifdef _RELEASE_BUILD
  // Only have automatic updating of paste button if final build
  if (PasteText.Empty()) EnableWindow(GetDlgItem(StemWin,114),IsClipboardFormatAvailable(CF_TEXT));
#else
  EnableWindow(GetDlgItem(StemWin,114),true);
#endif
}
//---------------------------------------------------------------------------
#if !defined(_RELEASE_BUILD) && defined(_DEBUG_BUILD)
bool HWNDNotValid(HWND Win,char *File,int Line)
{
  bool Err=0;
  if (Win==NULL){
    Err=true;
  }else if (IsWindow(Win)==0){
    Err=true;
  }
  if (Err){
    Alert(Str("WINDOWS: Arrghh, using ")+long(Win)+" as HWND in file "+File+" at line "+Line,"Window Handle Error",MB_ICONEXCLAMATION);
    return true;
  }
  return 0;
}

LRESULT SendMessage_checkforbugs(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar,char *File,int Line)
{
  if (HWNDNotValid(Win,File,Line)) return 0;
  return SendMessageA(Win,Mess,wPar,lPar);
}

BOOL PostMessage_checkforbugs(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar,char *File,int Line)
{
  if (HWNDNotValid(Win,File,Line)) return 0;
  return PostMessageA(Win,Mess,wPar,lPar);
}

#endif

#endif //WIN32

//---------------------------------------------------------------------------
int Alert(char *Mess,char *Title,UINT Flags)
{
  WIN_ONLY( HWND Win=GetActiveWindow(); )
  Disp.FlipToDialogsScreen();
  log_write(EasyStr(Title)+": "+Mess);
  int Ret=MessageBox(WINDOWTYPE(FullScreen ? StemWin:0),Mess,Title,Flags | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
  WIN_ONLY( SetActiveWindow(Win); )
  return Ret;
}
//---------------------------------------------------------------------------
// Shift and alt switching, use 4 tables of 256 WORDs:
//  table 0 = no shift, no alt
//  table 1 = shift, no alt
//  table 2 = no shift, alt
//  table 3 = shift, alt
// Look up our key (VKCode) in the table, if it isn't 0 then shift/alt switching
// must be performed. The LOBYTE contains the ST key code, the high byte has
// bit 0 set if shift should be down when the key is sent and bit 1 for alt to
// be down. Before the key is pressed we must change the ST shift and alt states
// to what we need by sending releasing/press IKBD messages for those keys.
// After the key has been sent we then send more messages to restore them to
// their former glory. Surprisingly it works great, except if you try to do any
// key repeat.

void ShiftSwitchChangeModifiers(bool ShiftShouldBePressed,bool AltShouldBePressed,int ModifierRestoreArray[3])
{
  // Get current states

  bool STLShiftDown=(ST_Key_Down[key_table[VK_LSHIFT]]);
  bool STRShiftDown=(ST_Key_Down[key_table[VK_RSHIFT]]);
  int STAltDown=(ST_Key_Down[key_table[VK_MENU]] ? BIT_1:0);

  if ((STLShiftDown || STRShiftDown) && ShiftShouldBePressed==0){
    // Send Shift Up Messages
    if (STLShiftDown){
      keyboard_buffer_write_n_record(key_table[VK_LSHIFT] | MSB_B);
      ModifierRestoreArray[0]=1; //Lshift down
    }
    if (STRShiftDown){
      keyboard_buffer_write_n_record(key_table[VK_RSHIFT] | MSB_B);
      ModifierRestoreArray[1]=1; //Rshift down
    }
  }else if ((STLShiftDown || STRShiftDown)==0 && ShiftShouldBePressed){
    // Send Shift Down Message
    keyboard_buffer_write_n_record(key_table[VK_LSHIFT]);
    ModifierRestoreArray[0]=2; //Lshift up
  }
  if (STAltDown && AltShouldBePressed==0){
    // Send Alt Up Messages
    keyboard_buffer_write_n_record(key_table[VK_MENU] | MSB_B);
    ModifierRestoreArray[2]=1; //Alt down
  }else if (STAltDown==0 && AltShouldBePressed){
    // Send Alt Down Message
    keyboard_buffer_write_n_record(key_table[VK_MENU]);
    ModifierRestoreArray[2]=2; //Alt up
  }
}
//---------------------------------------------------------------------------
void ShiftSwitchRestoreModifiers(int ModifierRestoreArray[3])
{
  if (ModifierRestoreArray[0]==1) keyboard_buffer_write_n_record(key_table[VK_LSHIFT]);
  if (ModifierRestoreArray[0]==2) keyboard_buffer_write_n_record(key_table[VK_LSHIFT] | MSB_B);
  if (ModifierRestoreArray[1]==1) keyboard_buffer_write_n_record(key_table[VK_RSHIFT]);
  if (ModifierRestoreArray[2]==1) keyboard_buffer_write_n_record(key_table[VK_MENU]);
  if (ModifierRestoreArray[2]==2) keyboard_buffer_write_n_record(key_table[VK_MENU] | MSB_B);
}
//---------------------------------------------------------------------------
void HandleShiftSwitching(UINT VKCode,bool Up,BYTE &STCode,int ModifierRestoreArray[3])
{
  // These are set to tell the HandleKeyPress routine what to do after it has
  // sent the key.
  ModifierRestoreArray[0]=0;  // LShift
  ModifierRestoreArray[1]=0;  // RShift
  ModifierRestoreArray[2]=0;  // Alt (only one on ST)

  // Don't need to do anything when you release the key
  if (shift_key_table[0]==NULL) return;

  // Get ST code and required modifier states
  int Shift,Alt;
  if (Up==0){ // Pressing key
    // Get current state of modifiers
    Shift=int((ST_Key_Down[key_table[VK_LSHIFT]] || ST_Key_Down[key_table[VK_RSHIFT]]) ? BIT_0:0);
    Alt=int(ST_Key_Down[key_table[VK_MENU]] ? BIT_1:0);

  }else{ // Releasing key
    // Get state of modifiers when key was pressed
    Shift=(KeyDownModifierState[BYTE(VKCode)] & BIT_0);
    Alt=(KeyDownModifierState[BYTE(VKCode)] & BIT_1);
  }
  WORD KeyEntry=shift_key_table[Shift | Alt][BYTE(VKCode)];
  STCode=LOBYTE(KeyEntry);
  KeyDownModifierState[BYTE(VKCode)]=BYTE(Shift | Alt);
  if (STCode && Up==0){
    bool ShiftShouldBePressed=(HIBYTE(KeyEntry) & BIT_0);
    bool AltShouldBePressed=(HIBYTE(KeyEntry) & BIT_1);
    ShiftSwitchChangeModifiers(ShiftShouldBePressed,AltShouldBePressed,ModifierRestoreArray);
  }
}
//---------------------------------------------------------------------------
void PasteIntoSTAction(int Action)
{
  if (Action==STPASTE_STOP || Action==STPASTE_TOGGLE){
    if (PasteText.NotEmpty()){
      PasteText="";
      PasteVBLCount=0;
      WIN_ONLY( SendDlgItemMessage(StemWin,114,BM_SETCHECK,0,0); )
      UNIX_ONLY( PasteBut.set_check(0); )
      return;
    }else{
      if (Action==STPASTE_STOP) return;
    }
  }

#if defined(WIN32)
  if (IsClipboardFormatAvailable(CF_TEXT)==0) return;
  if (OpenClipboard(StemWin)==0) return;

  HGLOBAL hGbl=GetClipboardData(CF_TEXT);
  if (hGbl){
    PasteText=(char*)GlobalLock(hGbl);
    PasteVBLCount=PasteSpeed;
    SendDlgItemMessage(StemWin,114,BM_SETCHECK,1,0);
    GlobalUnlock(hGbl);
  }
  CloseClipboard();
#elif defined(UNIX)
  Window SelectionOwner=XGetSelectionOwner(XD,XA_PRIMARY);
  if (SelectionOwner!=None){
    XEvent SendEv;
    SendEv.type=SelectionRequest;
    SendEv.xselectionrequest.requestor=StemWin;
    SendEv.xselectionrequest.owner=SelectionOwner;
    SendEv.xselectionrequest.selection=XA_PRIMARY;
    SendEv.xselectionrequest.target=XA_STRING;
    SendEv.xselectionrequest.property=XA_CUT_BUFFER0;
    SendEv.xselectionrequest.time=CurrentTime;
    XSendEvent(XD,SelectionOwner,0,0,&SendEv);
    // PasteText,PasteVBLCount and PasteBut are set up
    // in SelectionNotify event handler
	}
#endif
}
//---------------------------------------------------------------------------
void PasteVBL()
{
  if (PasteText.NotEmpty()){
    if ((--PasteVBLCount)<=0){
      // Convert to ST Ascii
      BYTE c=BYTE(PasteText[0]);
      if (c>127){
        c=STCharToPCChar[c-128];
        if (c) PasteText[0]=char(c);
      }

      // Go through every character TOS can produce to find it
      switch (c){
        case '\r': break; // Only need line feeds
        case '\n':
          keyboard_buffer_write_n_record(0x1c);
          keyboard_buffer_write_n_record(BYTE(0x1c | BIT_7));
          break;
        case '\t':
          keyboard_buffer_write_n_record(0x0f);
          keyboard_buffer_write_n_record(BYTE(0x0f | BIT_7));
          break;
        case ' ':
          keyboard_buffer_write_n_record(0x39);
          keyboard_buffer_write_n_record(BYTE(0x39 | BIT_7));
          break;
        default:
          DynamicArray<DWORD> Chars;
          GetAvailablePressChars(&Chars);
          for (int n=0;n<Chars.NumItems;n++){
            if (HIWORD(Chars[n])==c){
              // Now fix shift/alt and press the key
              int ModifierRestoreArray[3]={0,0,0};
              BYTE STCode=LOBYTE(LOWORD(Chars[n]));
              BYTE Modifiers=HIBYTE(LOWORD(Chars[n]));
              ShiftSwitchChangeModifiers(Modifiers & BIT_0,Modifiers & BIT_1,ModifierRestoreArray);
              keyboard_buffer_write_n_record(STCode);
              keyboard_buffer_write_n_record(BYTE(STCode | BIT_7));
              ShiftSwitchRestoreModifiers(ModifierRestoreArray);
              break;
            }
          }
      }
      PasteText.Delete(0,1);
      if (PasteText.NotEmpty()){
        PasteVBLCount=PasteSpeed;
      }else{
        PasteText=""; // Release some memory
        WIN_ONLY( SendDlgItemMessage(StemWin,114,BM_SETCHECK,0,0); )
        UNIX_ONLY( PasteBut.set_check(0); )
      }
    }
  }
}
//---------------------------------------------------------------------------
void UpdateSTKeys()
{
	for (int n=0;n<128;n++){
    if (ST_Key_Down[n]){
      keyboard_buffer_write(BYTE(n | BIT_7));
      ST_Key_Down[n]=0;
    }
  }
}
//---------------------------------------------------------------------------

#ifdef UNIX
#include "x/x_gui.cpp"
#endif

