/*---------------------------------------------------------------------------
FILE: options_create.cpp
MODULE: Steem
DESCRIPTION: Functions to create the pages of the options dialog box.
---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
void TOptionBox::CreatePage(int n)
{
  switch (n){
    case 9:CreateMachinePage();break;
    case 10:CreateTOSPage();break;
    case 13:CreateMacrosPage();break;
    case 12:CreatePortsPage();break;
    case 4:CreateMIDIPage();break;

    case 0:CreateGeneralPage();break;
    case 5:CreateSoundPage();break;
    case 1:CreateDisplayPage();break;
    case 15:CreateOSDPage();break;
    case 3:CreateFullscreenPage();break;
    case 2:CreateBrightnessPage();break;
    case 11:CreateProfilesPage();break;
    case 6:CreateStartupPage();break;
    case 14:CreateIconsPage();break;
    case 8:CreateAssocPage();break;
    case 7:CreateUpdatePage();break;
  }
}
//---------------------------------------------------------------------------
void TOptionBox::CreateMachinePage()
{
  HWND Win;
  long Wid;

  int y=10;

  Wid=get_text_width(T("ST CPU speed"));
  CreateWindow("Static",T("ST CPU speed"),WS_CHILD,page_l,y+4,Wid,23,Handle,(HMENU)403,HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD  | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
                          page_l+5+Wid,y,page_w-(5+Wid),400,Handle,(HMENU)404,HInstance,NULL);
  EasyStr Mhz=T("Megahertz");
  CBAddString(Win,EasyStr("8 ")+Mhz+" ("+T("ST standard")+")",8000000);
  CBAddString(Win,EasyStr("9 ")+Mhz,9000000);
  CBAddString(Win,EasyStr("10 ")+Mhz,10000000);
  CBAddString(Win,EasyStr("11 ")+Mhz,11000000);
  CBAddString(Win,EasyStr("12 ")+Mhz,12000000);
  CBAddString(Win,EasyStr("14 ")+Mhz,14000000);
  CBAddString(Win,EasyStr("16 ")+Mhz,16000000);
  CBAddString(Win,EasyStr("20 ")+Mhz,20000000);
  CBAddString(Win,EasyStr("24 ")+Mhz,24000000);
  CBAddString(Win,EasyStr("28 ")+Mhz,28000000);
  CBAddString(Win,EasyStr("32 ")+Mhz,32000000);
  CBAddString(Win,EasyStr("36 ")+Mhz,36000000);
  CBAddString(Win,EasyStr("40 ")+Mhz,40000000);
  CBAddString(Win,EasyStr("44 ")+Mhz,44000000);
  CBAddString(Win,EasyStr("48 ")+Mhz,48000000);
  CBAddString(Win,EasyStr("56 ")+Mhz,56000000);
  CBAddString(Win,EasyStr("64 ")+Mhz,64000000);
  CBAddString(Win,EasyStr("80 ")+Mhz,80000000);
  CBAddString(Win,EasyStr("96 ")+Mhz,96000000);
  CBAddString(Win,EasyStr("128 ")+Mhz,128000000);
  if (CBSelectItemWithData(Win,n_cpu_cycles_per_second)<0){
    EasyStr Cycles=n_cpu_cycles_per_second;
    Cycles=Cycles.Lefts(Cycles.Length()-6);

    SendMessage(Win,CB_SETCURSEL,CBAddString(Win,Cycles+" "+T("Megahertz"),n_cpu_cycles_per_second),0);
  }
  y+=30;

  Wid=GetTextSize(Font,T("Memory size")).Width;
  CreateWindow("Static",T("Memory size"),WS_CHILD,page_l,y+4,Wid,20,Handle,HMENU(8090),HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST,
                          page_l+5+Wid,y,page_w-(5+Wid),200,Handle,(HMENU)8100,HInstance,NULL);
  CBAddString(Win,"512Kb",MAKELONG(MEMCONF_512,MEMCONF_0));
  CBAddString(Win,"1 MB",MAKELONG(MEMCONF_512,MEMCONF_512));
  CBAddString(Win,"2 MB",MAKELONG(MEMCONF_2MB,MEMCONF_0));
  CBAddString(Win,"4 MB",MAKELONG(MEMCONF_2MB,MEMCONF_2MB));
  CBAddString(Win,"14 MB",MAKELONG(MEMCONF_7MB,MEMCONF_7MB));
  y+=30;

  Wid=GetTextSize(Font,T("Monitor")).Width;
  CreateWindow("Static",T("Monitor"),WS_CHILD,page_l,y+4,Wid,20,Handle,HMENU(8091),HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST,
                          page_l+5+Wid,y,page_w-(5+Wid),200,Handle,(HMENU)8200,HInstance,NULL);
  CBAddString(Win,T("Colour")+" ("+T("Low/Med Resolution")+")");
  CBAddString(Win,T("Monochrome")+" ("+T("High Resolution")+")");
#ifndef NO_CRAZY_MONITOR
  for (int n=0;n<EXTMON_RESOLUTIONS;n++){
    CBAddString(Win,T("Extended Monitor At")+" "+extmon_res[n][0]+"x"+extmon_res[n][1]+"x"+extmon_res[n][2]);
  }
#endif
  y+=30;

  CreateWindow("Button",T("Keyboard"),WS_CHILD | BS_GROUPBOX,
                  page_l,y,page_w,80,Handle,(HMENU)8093,HInstance,NULL);
  y+=20;

  Wid=GetTextSize(Font,T("Language")).Width;
  CreateWindow("Static",T("Language"),WS_CHILD,
                  page_l+10,y+4,Wid,25,Handle,(HMENU)8400,HInstance,NULL);

  HWND Combo=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST,
                          page_l+15+Wid,y,(page_w-20)-(5+Wid),200,Handle,(HMENU)8401,HInstance,NULL);
  CBAddString(Combo,T("United States"),MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US));
  CBAddString(Combo,T("United Kingdom"),MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_UK));
  CBAddString(Combo,T("Australia (UK TOS)"),MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_AUS));
  CBAddString(Combo,T("German"),MAKELANGID(LANG_GERMAN,SUBLANG_GERMAN));
  CBAddString(Combo,T("French"),MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH));
  CBAddString(Combo,T("Spanish"),MAKELANGID(LANG_SPANISH,SUBLANG_SPANISH));
  CBAddString(Combo,T("Italian"),MAKELANGID(LANG_ITALIAN,SUBLANG_ITALIAN));
  CBAddString(Combo,T("Swedish"),MAKELANGID(LANG_SWEDISH,SUBLANG_SWEDISH));
  CBAddString(Combo,T("Norwegian"),MAKELANGID(LANG_NORWEGIAN,SUBLANG_NEUTRAL));
  CBAddString(Combo,T("Belgian"),MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH_BELGIAN));
  if (CBSelectItemWithData(Combo,KeyboardLangID)<0){
    SendMessage(Combo,CB_SETCURSEL,0,0);
  }
  y+=30;

  Wid=GetCheckBoxSize(Font,T("Shift and alternate correction")).Width;
  Win=CreateWindow("Button",T("Shift and alternate correction"),WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX,
                          page_l+10,y,Wid,25,Handle,(HMENU)8402,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,EnableShiftSwitching,0);
  EnableWindow(Win,ShiftSwitchingAvailable);
  ToolAddWindow(ToolTip,Win,T("When checked this allows Steem to emulate all keys correctly, it does this by changing the shift and alternate state of the ST when you press them.")+" "+
                              T("This could interfere with games and other programs, only use it if you are doing lots of typing.")+" "+
                              T("Please note that instead of pressing Alt-Gr or Control to access characters on the right-hand side of a key, you have to press Alt or Alt+Shift (this is how it was done on an ST)."));
  y+=40;


  CreateWindow("Button",T("Cartridge"),WS_CHILD | BS_GROUPBOX,
                  page_l,y,page_w,80,Handle,(HMENU)8093,HInstance,NULL);
  y+=20;

  CreateWindowEx(512,"Steem Path Display","",WS_CHILD,
                  page_l+10,y,page_w-20,22,Handle,(HMENU)8500,HInstance,NULL);
  y+=30;

  CreateWindow("Button",T("Choose"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                  page_l+10,y,(page_w-20)/2-5,23,Handle,(HMENU)8501,HInstance,NULL);

  CreateWindow("Button",T("Remove"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                  page_l+10+(page_w-20)/2+5,y,(page_w-20)/2-5,23,Handle,(HMENU)8502,HInstance,NULL);
  y+=40;

  WIDTHHEIGHT wh=GetTextSize(Font,T("Memory and monitor changes don't take effect until the next cold reset of the ST"));
  if (wh.Width>=page_w) wh.Height=(wh.Height+1)*2;
  CreateWindow("Static",T("Memory and monitor changes don't take effect until the next cold reset of the ST"),
        WS_CHILD,page_l,y,page_w,wh.Height,Handle,HMENU(8600),HInstance,NULL);
  y+=wh.Height+5;

  CreateWindow("Button",T("Perform cold reset now"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                  page_l,y,page_w,23,Handle,(HMENU)8601,HInstance,NULL);
//  y+=30;

  MachineUpdateIfVisible();
  if (Focus==NULL) Focus=GetDlgItem(Handle,404);
  SetPageControlsFont();
  ShowPageControls();
}
//---------------------------------------------------------------------------
void TOptionBox::MachineUpdateIfVisible()
{
  TOSRefreshBox("");

  if (Handle==NULL) return;
  if (GetDlgItem(Handle,8100)==NULL) return;

  int memconf=4;
  if (NewMemConf0<0){
    if (mem_len<1024*1024){
      memconf=0;
    }else if (mem_len<2*1024*1024){
      memconf=1;
    }else if (mem_len<4*1024*1024){
      memconf=2;
    }else if (mem_len<14*1024*1024){
      memconf=3;
    }
  }else{
    if (NewMemConf0==MEMCONF_512) memconf=int((NewMemConf1==MEMCONF_512) ? 1:0); // 1Mb:512Kb
    if (NewMemConf0==MEMCONF_2MB) memconf=int((NewMemConf1==MEMCONF_2MB) ? 3:2); // 4Mb:2Mb
  }
  SendMessage(GetDlgItem(Handle,8100),CB_SETCURSEL,memconf,0);

  int monitor_sel=NewMonitorSel;
  if (monitor_sel<0) monitor_sel=GetCurrentMonitorSel();
  SendMessage(GetDlgItem(Handle,8200),CB_SETCURSEL,monitor_sel,0);

  SetWindowText(GetDlgItem(Handle,8500),CartFile);
  EnableWindow(GetDlgItem(Handle,8502),CartFile.NotEmpty());
}
//---------------------------------------------------------------------------
void TOptionBox::CreateTOSPage()
{
  int y=10,Wid;
  HWND Win;

  Wid=GetTextSize(Font,T("Sort by")).Width;
  CreateWindow("Static",T("Sort by"),WS_CHILD,page_l,y+4,Wid,25,
                  Handle,HMENU(8310),HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP,page_l+Wid+5,y,page_w-(Wid+5),200,
                  Handle,HMENU(8311),HInstance,NULL);
  CBAddString(Win,T("Version (Ascending)"),MAKELONG((WORD)eslSortByData0,0));
  CBAddString(Win,T("Version (Descending)"),MAKELONG((WORD)eslSortByData0,1));
  CBAddString(Win,T("Language"),MAKELONG((WORD)eslSortByData1,0));
  CBAddString(Win,T("Date (Ascending)"),MAKELONG((WORD)eslSortByData2,0));
  CBAddString(Win,T("Date (Descending)"),MAKELONG((WORD)eslSortByData2,1));
  CBAddString(Win,T("Name (Ascending)"),MAKELONG((WORD)(signed short)eslSortByNameI,0));
  CBAddString(Win,T("Name (Descending)"),MAKELONG((WORD)(signed short)eslSortByNameI,1));
  if (CBSelectItemWithData(Win,MAKELONG(eslTOS_Sort,eslTOS_Descend))<0){
    SendMessage(Win,CB_SETCURSEL,0,0);
    eslTOS_Sort=eslSortByData0;
    eslTOS_Descend=0;
  }
  y+=30;

  WIDTHHEIGHT wh=GetTextSize(Font,T("TOS changes don't take effect until the next cold reset of the ST"));
  if (wh.Width>=page_w) wh.Height=(wh.Height+1)*2;

  int TOSBoxHeight=(OPTIONS_HEIGHT-20)-(10+30+30+wh.Height+5+23+10);

  Win=CreateWindowEx(512,"ListBox","",WS_CHILD | WS_VSCROLL |
                  WS_TABSTOP | LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_SORT,
                  page_l,y,page_w,TOSBoxHeight,Handle,(HMENU)8300,HInstance,NULL);
  SendMessage(Win,LB_SETITEMHEIGHT,0,max((int)GetTextSize(Font,"HyITljq").Height+4,RC_FLAG_HEIGHT+4));
  y+=TOSBoxHeight+10;

  CreateWindow("Button",T("Add"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                  page_l,y,page_w/2-5,23,Handle,(HMENU)8301,HInstance,NULL);

  CreateWindow("Button",T("Remove"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                  page_l+page_w/2+5,y,page_w/2-5,23,Handle,(HMENU)8302,HInstance,NULL);
  y+=30;

  CreateWindow("Static",T("TOS changes don't take effect until the next cold reset of the ST"),
        WS_CHILD,page_l,y,page_w,40,Handle,HMENU(8600),HInstance,NULL);
  y+=wh.Height+5;

  CreateWindow("Button",T("Perform cold reset now"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                  page_l,y,page_w,23,Handle,(HMENU)8601,HInstance,NULL);

  MachineUpdateIfVisible();
  if (Focus==NULL) Focus=GetDlgItem(Handle,8300);
  SetPageControlsFont();
  ShowPageControls();
}
//---------------------------------------------------------------------------
BOOL CALLBACK TOptionBox::EnumDateFormatsProc(char *DateFormat)
{
  USDateFormat=(strchr(DateFormat,'m')<strchr(DateFormat,'d'));
  return 0;
}
//---------------------------------------------------------------------------
void TOptionBox::CreateGeneralPage()
{
  HWND Win;
  long Wid;

  int y=10;

  CreateWindow("Static","",WS_CHILD | SS_CENTER,page_l,y,page_w,20,Handle,(HMENU)1040,HInstance,NULL);
  y+=20;

  Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD | WS_TABSTOP | TBS_HORZ,
                    page_l,y,page_w,28,Handle,(HMENU)1041,HInstance,NULL);
  SendMessage(Win,TBM_SETRANGE,0,MAKELPARAM(0,190)); // Each tick worth 5
  SendMessage(Win,TBM_SETPOS,1,((100000/run_speed_ticks_per_second)-50)/5);
  SendMessage(Win,TBM_SETLINESIZE,0,1);
  for (int n=0;n<190;n+=10) SendMessage(Win,TBM_SETTIC,0,n);
  SendMessage(Win,TBM_SETPAGESIZE,0,5);

  SendMessage(Handle,WM_HSCROLL,0,LPARAM(Win));
  y+=35;

  CreateWindow("Static",T("Slow motion speed")+": "+(slow_motion_speed/10)+"%",WS_CHILD | SS_CENTER,
                          page_l,y,page_w,20,Handle,(HMENU)1000,HInstance,NULL);
  y+=20;

  Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD | WS_TABSTOP | TBS_HORZ,
                    page_l,y,page_w,28,Handle,(HMENU)1001,HInstance,NULL);
  SendMessage(Win,TBM_SETRANGE,0,MAKELPARAM(0,79));
  SendMessage(Win,TBM_SETPOS,1,(slow_motion_speed-10)/10);
  SendMessage(Win,TBM_SETLINESIZE,0,1);
  for (int n=4;n<79;n+=5) SendMessage(Win,TBM_SETTIC,0,n);
  SendMessage(Win,TBM_SETPAGESIZE,0,10);
  y+=35;

  CreateWindow("Static","",WS_CHILD | SS_CENTER,page_l,y,page_w,20,Handle,(HMENU)1010,HInstance,NULL);
  y+=20;

  Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD | WS_TABSTOP | TBS_HORZ | TBS_AUTOTICKS,
                    page_l,y,page_w,28,Handle,(HMENU)1011,HInstance,NULL);
  SendMessage(Win,TBM_SETRANGE,0,MAKELPARAM(0,18));
  SendMessage(Win,TBM_SETPOS,1,(1000/max(fast_forward_max_speed,50))-2);
  SendMessage(Win,TBM_SETLINESIZE,0,1);
  SendMessage(Win,TBM_SETTICFREQ,1,0);
  SendMessage(Win,TBM_SETPAGESIZE,0,3);

  SendMessage(Handle,WM_HSCROLL,0,LPARAM(Win));
  y+=35;

  Wid=GetCheckBoxSize(Font,T("Show pop-up hints")).Width;
  Win=CreateWindow("Button",T("Show pop-up hints"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
                          page_l,y,Wid,25,Handle,(HMENU)400,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,ShowTips,0);
  y+=30;

  Wid=GetCheckBoxSize(Font,T("Make Steem high priority")).Width;
  Win=CreateWindow("Button",T("Make Steem high priority"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
                          page_l,y,Wid,25,Handle,(HMENU)1030,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,HighPriority,0);
  ToolAddWindow(ToolTip,Win,T("When this option is ticked Steem will get first use of the CPU ahead of other applications, this means Steem will still run smoothly even if you start doing something else at the same time, but everything else will run slower."));
  y+=30;

  Wid=GetCheckBoxSize(Font,T("Pause emulation when inactive")).Width;
  Win=CreateWindow("Button",T("Pause emulation when inactive"),
                          WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
                          page_l,y,Wid,25,Handle,(HMENU)800,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,PauseWhenInactive,0);
  y+=30;

  Wid=GetCheckBoxSize(Font,T("Disable system keys when running")).Width;
  Win=CreateWindow("Button",T("Disable system keys when running"),
                          WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
                          page_l,y,Wid,25,Handle,(HMENU)700,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,(AllowTaskSwitch==0),0);
  ToolAddWindow(ToolTip,Win,T("When this option is ticked Steem will disable the Alt-Tab, Alt-Esc and Ctrl-Esc key combinations when it is running, this allows the ST to receive those keys. This option doesn't work in fullscreen mode."));
  y+=30;

  Wid=GetCheckBoxSize(Font,T("Automatic fast forward on disk access")).Width;
  Win=CreateWindow("Button",T("Automatic fast forward on disk access"),
                          WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
                          page_l,y,Wid,25,Handle,(HMENU)900,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,floppy_access_ff,0);
  y+=30;

  Wid=GetCheckBoxSize(Font,T("Start emulation on mouse click")).Width;
  Win=CreateWindow("Button",T("Start emulation on mouse click"),
                          WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
                          page_l,y,Wid,25,Handle,(HMENU)901,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,StartEmuOnClick,0);
  ToolAddWindow(ToolTip,Win,T("When this option is ticked clicking a mouse button on Steem's main window will start emulation."));

  SetPageControlsFont();
  if (Focus==NULL) Focus=GetDlgItem(Handle,1041);
  ShowPageControls();
}
//---------------------------------------------------------------------------
void TOptionBox::CreatePortsPage()
{
  HWND Win;
  int y=10,Wid;
  int GroupHeight=(OPTIONS_HEIGHT-10)/3-10;
  int GroupMiddle=20+30+(GroupHeight-20-30)/2;

  for (int p=0;p<3;p++){
    HWND CtrlParent;
    EasyStr PortName;
    int base=9000+p*100;
    switch (p){
      case 0:PortName=T("MIDI Ports");break;
      case 1:PortName=T("Parallel Port");break;
      case 2:PortName=T("Serial Port");break;
    }
    CtrlParent=CreateWindow("Button",PortName,WS_CHILD | BS_GROUPBOX,
                                page_l,y,page_w,GroupHeight,Handle,HMENU(base),HInstance,NULL);
    SetWindowLong(CtrlParent,GWL_USERDATA,(long)this);
    Old_GroupBox_WndProc=(WNDPROC)SetWindowLong(CtrlParent,GWL_WNDPROC,(long)GroupBox_WndProc);
    y+=GroupHeight;
    y+=10;

    Wid=get_text_width(T("Connect to"));
    CreateWindow("Static",T("Connect to"),WS_CHILD | WS_VISIBLE,
                          10,24,Wid,23,CtrlParent,HMENU(base+1),HInstance,NULL);

    Win=CreateWindow("Combobox","",WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
                            15+Wid,20,page_w-10-(15+Wid),200,CtrlParent,HMENU(base+2),HInstance,NULL);
    CBAddString(Win,T("None"),PORTTYPE_NONE);
    CBAddString(Win,T("MIDI Device"),PORTTYPE_MIDI);
    if (AllowLPT) CBAddString(Win,T("Parallel Port (LPT)"),PORTTYPE_PARALLEL);
    if (AllowCOM) CBAddString(Win,T("COM Port"),PORTTYPE_COM);
    CBAddString(Win,T("File"),PORTTYPE_FILE);
    CBAddString(Win,T("Loopback (Output->Input)"),PORTTYPE_LOOP);
    if (CBSelectItemWithData(Win,STPort[p].Type)<0){
      SendMessage(Win,CB_SETCURSEL,0,0);
    }

    // MIDI
    Wid=get_text_width(T("Output device"));
    CreateWindow("Static",T("Output device"),WS_CHILD,
                  10,54,Wid,23,CtrlParent,HMENU(base+10),HInstance,NULL);

    Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
                      15+Wid,50,page_w-10-(15+Wid),200,CtrlParent,HMENU(base+11),HInstance,NULL);
    SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("None"));
    int c=midiOutGetNumDevs();
    MIDIOUTCAPS moc;
    for (int n=-1;n<c;n++){
      midiOutGetDevCaps(n,&moc,sizeof(moc));
      SendMessage(Win,CB_ADDSTRING,0,(long)moc.szPname);
    }
    SendMessage(Win,CB_SETCURSEL,STPort[p].MIDIOutDevice+2,0);

    Wid=get_text_width(T("Input device"));
    CreateWindow("Static",T("Input device"),WS_CHILD,
                            10,80+4,Wid,23,CtrlParent,HMENU(base+12),HInstance,NULL);

    Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
                            15+Wid,80,page_w-10-(15+Wid),200,CtrlParent,HMENU(base+13),HInstance,NULL);
    SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("None"));
    c=midiInGetNumDevs();
    MIDIINCAPS mic;
    for (int n=0;n<c;n++){
      midiInGetDevCaps(n,&mic,sizeof(mic));
      SendMessage(Win,CB_ADDSTRING,0,(long)mic.szPname);
    }
    SendMessage(Win,CB_SETCURSEL,STPort[p].MIDIInDevice+1,0);

    //Parallel
    Wid=get_text_width(T("Select port"));
    CreateWindow("Static",T("Select port"),WS_CHILD,
                            page_w/2-(Wid+105)/2,GroupMiddle-15+4,Wid,23,CtrlParent,HMENU(base+20),HInstance,NULL);

    Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST,
                            page_w/2-(Wid+105)/2+Wid+5,GroupMiddle-15,100,200,CtrlParent,HMENU(base+21),HInstance,NULL);
    for (int n=1;n<10;n++) SendMessage(Win,CB_ADDSTRING,0,long((EasyStr("LPT")+n).Text));
    SendMessage(Win,CB_SETCURSEL,STPort[p].LPTNum,0);

    //COM
    Wid=get_text_width(T("Select port"));
    CreateWindow("Static",T("Select port"),WS_CHILD,
                            page_w/2-(Wid+105)/2,GroupMiddle-15+4,Wid,23,CtrlParent,HMENU(base+30),HInstance,NULL);

    Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST,
                            page_w/2-(Wid+105)/2+Wid+5,GroupMiddle-15,100,200,CtrlParent,HMENU(base+31),HInstance,NULL);
    for (int n=1;n<10;n++) SendMessage(Win,CB_ADDSTRING,0,long((EasyStr("COM")+n).Text));
    SendMessage(Win,CB_SETCURSEL,STPort[p].COMNum,0);

    //File
    CreateWindowEx(512,"Steem Path Display",STPort[p].File,WS_CHILD,
                    10,GroupMiddle-30,page_w-20,22,CtrlParent,HMENU(base+40),HInstance,NULL);

    CreateWindow("Button",T("Change File"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                  10,GroupMiddle,page_w/2-15,23,CtrlParent,HMENU(base+41),HInstance,NULL);

    CreateWindow("Button",T("Reset Current File"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                  page_w/2+5,GroupMiddle,page_w/2-15,23,CtrlParent,HMENU(base+42),HInstance,NULL);

    // Disabled (parallel only)
    if (p==1){
      CreateWindow("Steem Path Display",T("Disabled due to parallel joystick"),
                    WS_CHILD | PDS_VCENTRESTATIC,
                    10,20,page_w-20,GroupHeight-30,CtrlParent,HMENU(99),HInstance,NULL);
    }

    SetWindowAndChildrensFont(CtrlParent,Font);
  }

  SetPageControlsFont();
  if (Focus==NULL) Focus=GetDlgItem(GetDlgItem(Handle,9000),9002);
  ShowPageControls();
  for (int p=0;p<3;p++) PortsMakeTypeVisible(p);
}
//---------------------------------------------------------------------------
void TOptionBox::PortsMakeTypeVisible(int p)
{
  int base=9000+p*100;
  HWND CtrlParent=GetDlgItem(Handle,base);
  if (CtrlParent==NULL) return;

  bool Disabled=(p==1 && (Joy[N_JOY_PARALLEL_0].ToggleKey || Joy[N_JOY_PARALLEL_1].ToggleKey));

  for (int n=base+10;n<base+100;n++){
    if (GetDlgItem(CtrlParent,n)) ShowWindow(GetDlgItem(CtrlParent,n),SW_HIDE);
  }
  if (Disabled==0){
    for (int n=base+STPort[p].Type*10;n<base+STPort[p].Type*10+10;n++){
      if (GetDlgItem(CtrlParent,n)) ShowWindow(GetDlgItem(CtrlParent,n),SW_SHOW);
    }
  }
  if (p==1){
    if (Disabled){
      ShowWindow(GetDlgItem(CtrlParent,base+1),SW_HIDE);
      ShowWindow(GetDlgItem(CtrlParent,base+2),SW_HIDE);
      ShowWindow(GetDlgItem(CtrlParent,99),SW_SHOW);
    }else{
      ShowWindow(GetDlgItem(CtrlParent,99),SW_HIDE);
      ShowWindow(GetDlgItem(CtrlParent,base+1),SW_SHOW);
      ShowWindow(GetDlgItem(CtrlParent,base+2),SW_SHOW);
    }
  }

  // Redraw the groupbox
  RECT rc;
  GetWindowRect(CtrlParent,&rc);
  rc.left+=8;rc.right-=8;rc.top+=20+25;rc.bottom-=5;
  POINT pt={0,0};
  ClientToScreen(Handle,&pt);
  OffsetRect(&rc,-pt.x,-pt.y);
  InvalidateRect(Handle,&rc,true);
}
//---------------------------------------------------------------------------
void TOptionBox::UpdateParallel()
{
  if (Handle) PortsMakeTypeVisible(1);
}
//---------------------------------------------------------------------------
#define GET_THIS This=(TOptionBox*)GetWindowLong(Win,GWL_USERDATA);
LRESULT __stdcall TOptionBox::GroupBox_WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  TOptionBox *This;

  GET_THIS;

  switch (Mess){
    case WM_COMMAND:
    case WM_HSCROLL:
      return SendMessage(This->Handle,Mess,wPar,lPar);
  }
  return CallWindowProc(This->Old_GroupBox_WndProc,Win,Mess,wPar,lPar);
}
#undef GET_THIS
//---------------------------------------------------------------------------
void TOptionBox::CreateBrightnessBitmap()
{
  if (Handle==NULL) return;
  if (GetDlgItem(Handle,2010)==NULL) return;

  if (hBrightBmp) DeleteObject(hBrightBmp);
  HDC ScrDC=GetDC(NULL);
  hBrightBmp=CreateCompatibleBitmap(ScrDC,136+136,160);
  ReleaseDC(NULL,ScrDC);

  make_palette_table(brightness,contrast);
  DrawBrightnessBitmap(hBrightBmp);
  SendMessage(GetDlgItem(Handle,2010),STM_SETIMAGE,IMAGE_BITMAP,LPARAM(hBrightBmp));
}
//---------------------------------------------------------------------------
void TOptionBox::DrawBrightnessBitmap(HBITMAP hBmp)
{
  if (hBmp==NULL) return;

  BITMAP bi;
  GetObject(hBmp,sizeof(BITMAP),&bi);
  int w=bi.bmWidth,h=bi.bmHeight,bpp=bi.bmBitsPixel;

  int text_h=h/8;
  int band_w=w/16;
  int col_h=(h-text_h)/4;
  int BytesPP=(bpp+7)/8;
  BYTE *PicMem=new BYTE[w*h*BytesPP + 16];
  ZeroMemory(PicMem,w*h*BytesPP);
  BYTE *pMem=PicMem;

  int pc_pal_start_idx=10+118+(118-65); // End of the second half of the palette
  PALETTEENTRY *pbuf=(PALETTEENTRY*)&logpal[pc_pal_start_idx];
  for (int y=0;y<h-text_h;y++){
    for (int i=0;i<w;i++){
      int r=((i/band_w) >> 1)+(((i/band_w) & 1) << 3),g=r,b=r;
      int pal_offset=0;
      if (y>col_h*3){
        g=0,b=0;
        pal_offset=48;
      }else if (y>col_h*2){
        r=0,b=0;
        pal_offset=32;
      }else if (y>col_h){
        r=0,g=0;
        pal_offset=16;
      }
      long Col=palette_table[r | (g << 4) | (b << 8)];
      switch (BytesPP){
        case 1:
        {
          int ncol=pal_offset+(i/band_w);
          pbuf[ncol].peFlags=PC_RESERVED;
          pbuf[ncol].peRed=  BYTE((Col & 0xff0000) >> 16);
          pbuf[ncol].peGreen=BYTE((Col & 0x00ff00) >> 8);
          pbuf[ncol].peBlue= BYTE((Col & 0x0000ff));
          *pMem=BYTE(1+pc_pal_start_idx+ncol);
          break;
        }
        case 2:
          *LPWORD(pMem)=WORD(Col);
          break;
        case 3:case 4:
          *LPDWORD(pMem)=DWORD(Col);
          break;
      }
      pMem+=BytesPP;
    }
  }
  SetBitmapBits(hBmp,w*h*BytesPP,PicMem);
  delete[] PicMem;
  if (BytesPP==1) AnimatePalette(winpal,pc_pal_start_idx,64,pbuf);

  int gap_w=band_w/4,gap_h=text_h/8;
  HFONT f=MakeFont("Arial",-(text_h-gap_h),band_w/2 - gap_w);
  HDC ScrDC=GetDC(NULL);
  HDC BmpDC=CreateCompatibleDC(ScrDC);
  ReleaseDC(NULL,ScrDC);

  SelectObject(BmpDC,hBmp);
  SelectObject(BmpDC,f);
  SetTextColor(BmpDC,RGB(224,224,224));
  SetBkMode(BmpDC,TRANSPARENT);
  for (int i=0;i<16;i++){
    TextOut(BmpDC,i*band_w + (band_w-GetTextSize(f,EasyStr(i+1)).Width)/2,h-text_h-1+gap_h/2,EasyStr(i+1),EasyStr(i+1).Length());
  }
  DeleteDC(BmpDC);
  DeleteObject(f);
}
//---------------------------------------------------------------------------
LRESULT __stdcall TOptionBox::Fullscreen_WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  if (Mess==WM_PAINT || Mess==WM_NCPAINT){
    HDC WinDC=GetWindowDC(Win);
    HDC BmpDC=CreateCompatibleDC(WinDC);
    SelectObject(BmpDC,GetProp(Win,"Bitmap"));
    BitBlt(WinDC,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),BmpDC,0,0,SRCCOPY);
    DeleteDC(BmpDC);
    ReleaseDC(Win,WinDC);

    ValidateRect(Win,NULL);
    return 0;
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
void TOptionBox::FullscreenBrightnessBitmap()
{
  int w=GetSystemMetrics(SM_CXSCREEN),h=GetSystemMetrics(SM_CYSCREEN);

  WNDCLASS wc;
  wc.style=CS_DBLCLKS;
  wc.lpfnWndProc=Fullscreen_WndProc;
  wc.cbClsExtra=0;
  wc.cbWndExtra=0;
  wc.hInstance=HInstance;
  wc.hIcon=NULL;
  wc.hCursor=LoadCursor(NULL,IDC_ARROW);
  wc.hbrBackground=NULL;
  wc.lpszMenuName=NULL;
  wc.lpszClassName="Steem Temp Fullscreen Window";
  RegisterClass(&wc);

  HWND Win=CreateWindow("Steem Temp Fullscreen Window","",0,
                            0,0,w,h,Handle,NULL,HInstance,NULL);
  SetWindowLong(Win,GWL_STYLE,0);

  HDC ScrDC=GetDC(NULL);
  HBITMAP hBmp=CreateCompatibleBitmap(ScrDC,w,h);
  ReleaseDC(NULL,ScrDC);

  DrawBrightnessBitmap(hBmp);
  SetProp(Win,"Bitmap",hBmp);

  ShowWindow(Win,SW_SHOW);
  SetWindowPos(Win,HWND_TOPMOST,0,0,w,h,0);
  UpdateWindow(Win);

  bool DoneMouseUp=0;
  MSG mess;
  for (;;){
    PeekMessage(&mess,Win,0,0,PM_REMOVE);
    DispatchMessage(&mess);

    short MouseBut=(GetKeyState(VK_LBUTTON) | GetKeyState(VK_RBUTTON) | GetKeyState(VK_MBUTTON));
    if (MouseBut>=0) DoneMouseUp=true;
    if (MouseBut<0 && DoneMouseUp) break;
  }

  RemoveProp(Win,"Bitmap");
  DestroyWindow(Win);
  DeleteObject(hBmp);

  UnregisterClass("Steem Temp Fullscreen Window",HInstance);
}
//---------------------------------------------------------------------------
void TOptionBox::CreateBrightnessPage()
{
  int mid=page_l + page_w/2;
  RECT rc={mid-136,12,mid+136,12+160};
  AdjustWindowRectEx(&rc,WS_CHILD | SS_BITMAP,0,512);
  HWND Win=CreateWindowEx(512,"Static","",WS_CHILD | SS_BITMAP | SS_NOTIFY,
                            rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,
                            Handle,(HMENU)2010,HInstance,NULL);
  ToolAddWindow(ToolTip,Win,T("Click to view fullscreen"));

  CreateBrightnessBitmap();

  GetWindowRect(Win,&rc);
  POINT pt={0,0};
  ClientToScreen(Handle,&pt);
  int y=(rc.bottom-pt.y)+5;

  CreateWindow("Static",T("There should be 16 vertical strips (one black)"),WS_CHILD | SS_CENTER,
                          page_l,y,page_w,40,Handle,(HMENU)2011,HInstance,NULL);
  y+=40;

  CreateWindow("Static",T("Brightness")+": "+brightness,WS_CHILD | SS_CENTER,
                          page_l,y,page_w,25,Handle,(HMENU)2000,HInstance,NULL);
  y+=25;

  Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD | WS_TABSTOP | TBS_HORZ,
                    page_l,y,page_w,28,Handle,(HMENU)2001,HInstance,NULL);
  SendMessage(Win,TBM_SETRANGE,0,MAKELPARAM(0,256));
  SendMessage(Win,TBM_SETPOS,1,brightness+128);
  SendMessage(Win,TBM_SETLINESIZE,0,1);
  SendMessage(Win,TBM_SETPAGESIZE,0,10);
  SendMessage(Win,TBM_SETTIC,0,128);
  y+=40;

  CreateWindow("Static",T("Contrast")+": "+contrast,WS_CHILD | SS_CENTER,
                          page_l,y,page_w,25,Handle,(HMENU)2002,HInstance,NULL);
  y+=25;

  Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD | WS_TABSTOP | TBS_HORZ,
                    page_l,y,page_w,28,Handle,(HMENU)2003,HInstance,NULL);
  SendMessage(Win,TBM_SETRANGE,0,MAKELPARAM(0,256));
  SendMessage(Win,TBM_SETPOS,1,contrast+128);
  SendMessage(Win,TBM_SETLINESIZE,0,1);
  SendMessage(Win,TBM_SETPAGESIZE,0,10);
  SendMessage(Win,TBM_SETTIC,0,128);

  if (Focus==NULL) Focus=GetDlgItem(Handle,2001);
  SetPageControlsFont();
  ShowPageControls();
}
//---------------------------------------------------------------------------
void TOptionBox::CreateDisplayPage()
{
  HWND Win;
  long Wid;
  int y=10;

  int x=page_l;
  int w=get_text_width(T("Frameskip"));
  CreateWindow("Static",T("Frameskip"),WS_CHILD,
                    x,y+4,w,20,Handle,(HMENU)200,HInstance,NULL);
  x+=w+5;
  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST,
                    x,y,page_w-(w+5),200,Handle,(HMENU)201,HInstance,NULL);
  SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Draw Every Frame"));
  SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Draw Every Second Frame"));
  SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Draw Every Third Frame"));
  SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Draw Every Fourth Frame"));
  SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Auto Frameskip"));
  SendMessage(Win,CB_SETCURSEL,min(frameskip-1,4),0);
  y+=30;

  Wid=get_text_width(T("Borders"));
  CreateWindow("Static",T("Borders"),WS_CHILD,
                          page_l,y+4,Wid,21,Handle,(HMENU)209,HInstance,NULL);

  BorderOption=CreateWindow("Combobox","",WS_CHILD  | WS_TABSTOP | CBS_DROPDOWNLIST,
                          page_l+5+Wid,y,page_w-(5+Wid),200,Handle,(HMENU)207,HInstance,NULL);
  SendMessage(BorderOption,CB_ADDSTRING,0,(long)CStrT("Never Show Borders"));
  SendMessage(BorderOption,CB_ADDSTRING,1,(long)CStrT("Always Show Borders"));
  SendMessage(BorderOption,CB_ADDSTRING,2,(long)CStrT("Auto Borders"));
  if (Disp.BorderPossible()==0 && FullScreen==0) EnableBorderOptions(0);
  y+=30;

  CreateWindow("Button",T("Window Size"),WS_CHILD | BS_GROUPBOX,
                  page_l,y,page_w,20+30+30+30+30+2,Handle,(HMENU)99,HInstance,NULL);
  y+=20;

  Wid=GetCheckBoxSize(Font,T("Automatic resize on resolution change")).Width;
  Win=CreateWindow("Button",T("Automatic resize on resolution change"),WS_CHILD  | WS_TABSTOP | BS_CHECKBOX,
                          page_l+10,y,Wid,23,Handle,(HMENU)300,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,ResChangeResize,0);
  y+=30;

  w=get_text_width(T("Low resolution"));
  CreateWindow("Static",T("Low resolution"),WS_CHILD ,
                          page_l+10,y+4,w,23,Handle,(HMENU)301,HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD  | WS_TABSTOP | CBS_DROPDOWNLIST,
                          page_l+15+w,y,page_w-10-(15+w),200,Handle,(HMENU)302,HInstance,NULL);
  CBAddString(Win,T("Normal Size"),0);
  CBAddString(Win,T("Double Size")+" - "+T("Stretch"),1);
  CBAddString(Win,T("Double Size")+" - "+T("No Stretch"),MAKELONG(1,DWM_NOSTRETCH));
  CBAddString(Win,T("Double Size")+" - "+T("Grille"),MAKELONG(1,DWM_GRILLE));
  CBAddString(Win,T("Treble Size"),2);
  CBAddString(Win,T("Quadruple Size"),3);
  y+=30;

  w=get_text_width(T("Medium resolution"));
  CreateWindow("Static",T("Medium resolution"),WS_CHILD ,
                          page_l+10,y+4,w,23,Handle,(HMENU)303,HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD  | WS_TABSTOP | CBS_DROPDOWNLIST,
                          page_l+15+w,y,page_w-10-(15+w),200,Handle,(HMENU)304,HInstance,NULL);
  CBAddString(Win,T("Normal Size"),0);
  CBAddString(Win,T("Double Height")+" - "+T("Stretch"),1);
  CBAddString(Win,T("Double Height")+" - "+T("No Stretch"),MAKELONG(1,DWM_NOSTRETCH));
  CBAddString(Win,T("Double Height")+" - "+T("Grille"),MAKELONG(1,DWM_GRILLE));
  CBAddString(Win,T("Double Size"),2);
  CBAddString(Win,T("Quadruple Height"),3);
  y+=30;

  w=get_text_width(T("High resolution"));
  CreateWindow("Static",T("High resolution"),WS_CHILD ,
                          page_l+10,y+4,w,23,Handle,(HMENU)305,HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD  | WS_TABSTOP | CBS_DROPDOWNLIST,
                          page_l+15+w,y,page_w-10-(15+w),200,Handle,(HMENU)306,HInstance,NULL);
  CBAddString(Win,T("Normal Size"),0);
  CBAddString(Win,T("Double Size"),1);
  y+=30;

  y+=10;

  EasyStringList format_sl;
  Disp.ScreenShotGetFormats(&format_sl);
  bool FIAvailable=format_sl.NumStrings>2;

  int h=20+30+30+30+25+3;
  CreateWindow("Button",T("Screenshots"),WS_CHILD | BS_GROUPBOX,
                  page_l,y,page_w,h,Handle,(HMENU)99,HInstance,NULL);
  y+=20;

  Wid=get_text_width(T("Folder"));
  CreateWindow("Static",T("Folder"),WS_CHILD,page_l+10,y+4,Wid,23,Handle,(HMENU)1020,HInstance,NULL);

  CreateWindowEx(512,"Steem Path Display",ScreenShotFol,WS_CHILD,
                  page_l+15+Wid,y,page_w-10-(15+Wid),25,Handle,(HMENU)1021,HInstance,NULL);
  y+=30;

  CreateWindow("Button",T("Choose"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                          page_l+10,y,(page_w-20)/2-5,23,Handle,(HMENU)1022,HInstance,NULL);

  CreateWindow("Button",T("Open"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                          page_l+10+(page_w-20)/2+5,y,(page_w-20)/2-5,23,Handle,(HMENU)1023,HInstance,NULL);
  y+=30;

  w=get_text_width(T("Format"));
  CreateWindow("Static",T("Format"),WS_CHILD,page_l+10,y+4,w,23,Handle,(HMENU)1050,HInstance,NULL);

  int l=page_l+10+w+5;
  if (FIAvailable){
    w=(page_w-10-(10+w+5))/2-5;
  }else{
    w=page_w-10-(10+w+5);
  }

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
                          l,y,w,300,Handle,(HMENU)1051,HInstance,NULL);
  for (int i=0;i<format_sl.NumStrings;i++){
    CBAddString(Win,format_sl[i].String,format_sl[i].Data[0]);
  }

  int n,c=SendMessage(Win,CB_GETCOUNT,0,0);
  for (n=0;n<c;n++){
    if (SendMessage(Win,CB_GETITEMDATA,n,0)==Disp.ScreenShotFormat) break;
  }
  if (n>=c){
    Disp.ScreenShotFormat=FIF_BMP;
    Disp.ScreenShotFormatOpts=0;
    n=1;    
  }
  SendMessage(Win,CB_SETCURSEL,n,0);

  if (FIAvailable){
    CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST,
                            l+w+5,y,w,200,Handle,(HMENU)1052,HInstance,NULL);
    FillScreenShotFormatOptsCombo();
  }
  y+=30;

  Wid=GetCheckBoxSize(Font,T("Minimum size screenshots")).Width;
  Win=CreateWindow("Button",T("Minimum size screenshots"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
                          page_l+10,y,Wid,23,Handle,(HMENU)1024,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,Disp.ScreenShotMinSize,0);
  ToolAddWindow(ToolTip,Win,T("This option, when checked, ensures all screenshots will be taken at the smallest size possible for the resolution.")+" "+
                            T("WARNING: Some video cards may cause the screenshots to look terrible in certain drawing modes."));

  UpdateWindowSizeAndBorder();

  if (Focus==NULL) Focus=GetDlgItem(Handle,201);
  SetPageControlsFont();
  ShowPageControls();
}
//---------------------------------------------------------------------------
void TOptionBox::FillScreenShotFormatOptsCombo()
{
  HWND Win=GetDlgItem(Handle,1052);
  if (Win==NULL) return;

  EasyStringList sl;
  sl.Sort=eslNoSort;
  Disp.ScreenShotGetFormatOpts(&sl);

  SendMessage(Win,CB_RESETCONTENT,0,0);
  if (sl.NumStrings){
    EnableWindow(Win,true);
    for (int i=0;i<sl.NumStrings;i++) CBAddString(Win,sl[i].String,sl[i].Data[0]);
  }else{
    EnableWindow(Win,0);
    CBAddString(Win,T("Normal"),0);
  }
  if (CBSelectItemWithData(Win,Disp.ScreenShotFormatOpts)<0){
    SendMessage(Win,CB_SETCURSEL,0,0);
  }
}
//---------------------------------------------------------------------------
void TOptionBox::UpdateWindowSizeAndBorder()
{
  if (BorderOption==NULL) return;

  SendMessage(BorderOption,CB_SETCURSEL,min(border,2),0);
  for (int r=0;r<3;r++){
    DWORD dat=WinSizeForRes[r];
    if (r<2) dat=MAKELONG(dat,draw_win_mode[r]);
    CBSelectItemWithData(GetDlgItem(Handle,302+r*2),dat);
  }
}
//---------------------------------------------------------------------------
void TOptionBox::CreateOSDPage()
{
  HWND Win;
  long Wid;
  int y=10;

  Wid=GetCheckBoxSize(Font,T("Floppy disk access light")).Width;
  Win=CreateWindow("Button",T("Floppy disk access light"),WS_CHILD  | WS_TABSTOP | BS_CHECKBOX,
                          page_l,y,Wid,23,Handle,(HMENU)12000,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,osd_show_disk_light,0);
  y+=30;

  int *p_element[4]={&osd_show_plasma,&osd_show_speed,&osd_show_icons,&osd_show_cpu};
  Str osd_name[4];
  osd_name[0]=T("Logo");
  osd_name[1]=T("Speed bar");
  osd_name[2]=T("State icons");
  osd_name[3]=T("CPU speed indicator");
  for (int i=0;i<4;i++){
    Wid=GetTextSize(Font,osd_name[i]).Width;
    CreateWindow("Static",osd_name[i],WS_CHILD | WS_TABSTOP,
                          page_l,y+4,Wid,23,Handle,(HMENU)0,HInstance,NULL);

    Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST,
                            page_l+Wid+5,y,page_w-(Wid+5),200,Handle,HMENU(12010+i),HInstance,NULL);
    CBAddString(Win,T("Off"),0);
    CBAddString(Win,Str("2 ")+T("Seconds"),2);
    CBAddString(Win,Str("3 ")+T("Seconds"),3);
    CBAddString(Win,Str("4 ")+T("Seconds"),4);
    CBAddString(Win,Str("5 ")+T("Seconds"),5);
    CBAddString(Win,Str("6 ")+T("Seconds"),6);
    CBAddString(Win,Str("8 ")+T("Seconds"),8);
    CBAddString(Win,Str("10 ")+T("Seconds"),10);
    CBAddString(Win,Str("12 ")+T("Seconds"),12);
    CBAddString(Win,Str("15 ")+T("Seconds"),15);
    CBAddString(Win,Str("20 ")+T("Seconds"),20);
    CBAddString(Win,Str("30 ")+T("Seconds"),30);
    CBAddString(Win,T("Always Shown"),OSD_SHOW_ALWAYS);
    if (CBSelectItemWithData(Win,*(p_element[i]))<0){
      SendMessage(Win,CB_SETCURSEL,0,0);
    }
    y+=30;
  }

/*
  Wid=GetCheckBoxSize(Font,T("Old positions")).Width;
  Win=CreateWindow("Button",T("Old positions"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
                          page_l,y,Wid,23,Handle,(HMENU)12050,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,osd_old_pos,0);
  y+=30;
*/

  Wid=GetCheckBoxSize(Font,T("Scrolling messages")).Width;
  Win=CreateWindow("Button",T("Scrolling messages"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
                          page_l,y,Wid,23,Handle,(HMENU)12020,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,osd_show_scrollers,0);
  y+=30;

  Wid=GetCheckBoxSize(Font,T("Disable on screen display")).Width;
  Win=CreateWindow("Button",T("Disable on screen display"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
                          page_l,y,Wid,23,Handle,(HMENU)12030,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,osd_disable,0);

  if (Focus==NULL) Focus=GetDlgItem(Handle,201);
  SetPageControlsFont();
  ShowPageControls();
}
//---------------------------------------------------------------------------
void TOptionBox::CreateFullscreenPage()
{
  HWND Win;
  long w;
  int y=10;

  w=get_text_width(T("Drawing mode"));
  CreateWindow("Static",T("Drawing mode"),WS_CHILD ,
                          page_l,y+4,w,23,Handle,(HMENU)205,HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST,
                          page_l+5+w,y,page_w-(5+w),200,Handle,(HMENU)204,HInstance,NULL);
  SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Screen Flip"));
  SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Straight Blit"));
  SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Stretch Blit"));
  SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Laptop"));
  SendMessage(Win,CB_SETCURSEL,draw_fs_blit_mode,0);
  y+=30;

  int disabledflag=0;
  if (draw_fs_blit_mode==DFSM_STRETCHBLIT || draw_fs_blit_mode==DFSM_LAPTOP) disabledflag=WS_DISABLED;
  w=GetCheckBoxSize(Font,T("Scanline Grille")).Width;
  Win=CreateWindow("Button",T("Scanline Grille"),WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX | disabledflag,
                          page_l,y,w,23,Handle,(HMENU)280,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,(draw_fs_fx==DFSFX_GRILLE ? BST_CHECKED:BST_UNCHECKED),0);
  y+=30;

  w=GetCheckBoxSize(Font,T("Use 256 colour mode")).Width;
  Win=CreateWindow("Button",T("Use 256 colour mode"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
                          page_l,y,w,23,Handle,(HMENU)208,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,display_option_8_bit_fs,0);
  ToolAddWindow(ToolTip,Win,T("When this option is ticked Steem will use 256 colour mode in fullscreen, this is much faster but some screen effects involving many colours will not work"));
  y+=30;

  w=GetCheckBoxSize(Font,T("Use 640x400 (never show borders only)")).Width;
  Win=CreateWindow("Button",T("Use 640x400 (never show borders only)"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
                    page_l,y,w,23,Handle,(HMENU)210,HInstance,NULL);
  ToolAddWindow(ToolTip,Win,T("When this option is ticked Steem will use the 600x400 PC screen resolution in fullscreen if it can"));
  if (draw_fs_blit_mode==DFSM_LAPTOP){
    EnableWindow(Win,0);
  }else{
    EnableWindow(Win,border==0);
  }
  SendMessage(Win,BM_SETCHECK,prefer_res_640_400,0);
  y+=30;

  CreateWindow("Button",T("Synchronisation"),WS_CHILD | BS_GROUPBOX,
                  page_l,y,page_w,170,Handle,(HMENU)99,HInstance,NULL);
  y+=20;

  w=GetCheckBoxSize(Font,T("Vsync to PC display")).Width;
  Win=CreateWindow("Button",T("Vsync to PC display"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
                          page_l+10,y,w,23,Handle,(HMENU)206,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,FSDoVsync,0);
  ToolAddWindow(ToolTip,Win,T("When this option is ticked Steem will synchronise the PC monitor with the ST in fullscreen mode, this makes some things look a lot smoother but can be very slow.")+
                              " "+T("The ST used 50Hz (PAL), 60Hz (NTSC) and 70Hz (Mono), for good synchronisation you should set the PC refresh rate to the same or double the ST refresh rate."));
  y+=30;

  CreateWindow("Static",T("Preferred PC refresh rates:"),WS_CHILD,
                          page_l+10,y,page_w-20,25,Handle,(HMENU)99,HInstance,NULL);
  y+=25;

  w=get_text_width("640x400");
  CreateWindow("Static","640x400",WS_CHILD,
                          page_l+10,y+4,w,25,Handle,(HMENU)99,HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST,
                          page_l+15+w,y,page_w-10-(105+w),200,Handle,(HMENU)220,HInstance,NULL);
  SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Default"));
  for (int n=1;n<NUM_HZ;n++){
    SendMessage(Win,CB_ADDSTRING,0,LPARAM((Str(HzIdxToHz[n])+"Hz").Text));
  }

  CreateWindowEx(512,"Steem Path Display","",WS_CHILD | PDS_VCENTRESTATIC,
                          page_l+page_w-90,y,80,23,Handle,(HMENU)221,HInstance,NULL);
  y+=30;

  w=get_text_width("640x480");
  CreateWindow("Static","640x480",WS_CHILD,
                          page_l+10,y+4,w,25,Handle,(HMENU)99,HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST,
                          page_l+15+w,y,page_w-10-(105+w),200,Handle,(HMENU)222,HInstance,NULL);
  SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Default"));
  for (int n=1;n<NUM_HZ;n++){
    SendMessage(Win,CB_ADDSTRING,0,LPARAM((Str(HzIdxToHz[n])+"Hz").Text));
  }

  CreateWindowEx(512,"Steem Path Display","",WS_CHILD | PDS_VCENTRESTATIC,
                          page_l+page_w-90,y,80,23,Handle,(HMENU)223,HInstance,NULL);
  y+=30;


  w=get_text_width("800x600");
  CreateWindow("Static","800x600",WS_CHILD,page_l+10,y+4,w,25,Handle,(HMENU)99,HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST,
                          page_l+15+w,y,page_w-10-(105+w),200,Handle,(HMENU)224,HInstance,NULL);

  SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Default"));
  for (int n=1;n<NUM_HZ;n++){
    SendMessage(Win,CB_ADDSTRING,0,LPARAM((Str(HzIdxToHz[n])+"Hz").Text));
  }

  CreateWindowEx(512,"Steem Path Display","",WS_CHILD | PDS_VCENTRESTATIC,
                          page_l+page_w-90,y,80,23,Handle,(HMENU)225,HInstance,NULL);
  y+=40;

  w=GetCheckBoxSize(Font,T("Confirm before quit")).Width;
  Win=CreateWindow("Button",T("Confirm before quit"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
                          page_l,y,w,23,Handle,(HMENU)226,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,FSQuitAskFirst,0);

  UpdateHzDisplay();

  if (Focus==NULL) Focus=GetDlgItem(Handle,204);
  SetPageControlsFont();
  ShowPageControls();
}

void TOptionBox::UpdateHzDisplay()
{
  if (Handle==NULL) return;
  if (GetDlgItem(Handle,220)==NULL) return;

  int c256=int(display_option_8_bit_fs ? 0:1);
  for (int i=0;i<3;i++){
    for (int n=0;n<NUM_HZ;n++){
      if (HzIdxToHz[n]==prefer_pc_hz[c256][i]){
        SendDlgItemMessage(Handle,220+i*2,CB_SETCURSEL,n,0);
        break;
      }
    }

    EasyStr Text=T("UNTESTED");
    if (prefer_pc_hz[c256][i]){
      if (LOBYTE(tested_pc_hz[c256][i])==prefer_pc_hz[c256][i]){
        if (HIBYTE(tested_pc_hz[c256][i])){
          Text=T("OK");
        }else{
          Text=T("FAILED");
        }
      }
    }else{
      Text=T("OK");
    }
    SendDlgItemMessage(Handle,221+i*2,WM_SETTEXT,0,LPARAM(Text.Text));
  }
}
//---------------------------------------------------------------------------
void TOptionBox::CreateMIDIPage()
{
  HWND Win;
  int w,y=10,Wid,Wid2,x;

  Wid=GetTextSize(Font,T("Volume")+": "+T("Min")).Width;
  CreateWindow("Static",T("Volume")+": "+T("Min"),WS_CHILD,
                page_l,y+4,Wid,23,Handle,HMENU(6000),HInstance,NULL);

  Wid2=GetTextSize(Font,T("Max")).Width;

  Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD | WS_TABSTOP | TBS_HORZ,
                    page_l+Wid+5,y,page_w-(Wid2+5)-(Wid+5),27,Handle,HMENU(6001),HInstance,NULL);
  SendMessage(Win,TBM_SETRANGEMAX,0,0xffff);
  SendMessage(Win,TBM_SETPOS,true,MIDI_out_volume);
  SendMessage(Win,TBM_SETLINESIZE,0,0xff);
  SendMessage(Win,TBM_SETPAGESIZE,0,0xfff);

  CreateWindow("Static",T("Max"),WS_CHILD,
                page_l+page_w-Wid2,y+4,Wid2,23,Handle,HMENU(6002),HInstance,NULL);
  y+=35;

  w=GetCheckBoxSize(Font,T("Allow running status for output")).Width;
  Win=CreateWindow("Button",T("Allow running status for output"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
                          page_l,y,w,23,Handle,(HMENU)6010,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,int((MIDI_out_running_status_flag==MIDI_ALLOW_RUNNING_STATUS) ? BST_CHECKED:BST_UNCHECKED),0);
  y+=30;

  w=GetCheckBoxSize(Font,T("Allow running status for input")).Width;
  Win=CreateWindow("Button",T("Allow running status for input"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
                          page_l,y,w,23,Handle,(HMENU)6011,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,int((MIDI_in_running_status_flag==MIDI_ALLOW_RUNNING_STATUS) ? BST_CHECKED:BST_UNCHECKED),0);
  y+=30;

  CreateWindow("Button",T("System Exclusive Buffers"),WS_CHILD | BS_GROUPBOX,
                  page_l,y,page_w,85,Handle,(HMENU)99,HInstance,NULL);
  y+=20;

  x=page_l+10;
  Wid=GetTextSize(Font,T("Available for output")).Width;
  CreateWindow("Static",T("Available for output"),WS_CHILD,
                x,y+4,Wid,20,Handle,HMENU(6020),HInstance,NULL);
  x+=Wid+5;

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
                          x,y,40,200,Handle,HMENU(6021),HInstance,NULL);
  for (int n=2;n<MAX_SYSEX_BUFS;n++){
    SendMessage(Win,CB_ADDSTRING,0,(long)Str(n).Text);
  }
  SendMessage(Win,CB_SETCURSEL,MIDI_out_n_sysex-2,0);
  x+=45;

  Wid=GetTextSize(Font,T("size")).Width;
  CreateWindow("Static",T("size"),WS_CHILD,
                x,y+4,Wid,20,Handle,HMENU(6022),HInstance,NULL);
  x+=Wid+5;

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
                          x,y,page_w-10-(x-page_l),200,Handle,HMENU(6023),HInstance,NULL);
  SendMessage(Win,CB_ADDSTRING,0,(long)"16Kb");
  SendMessage(Win,CB_ADDSTRING,0,(long)"32Kb");
  SendMessage(Win,CB_ADDSTRING,0,(long)"64Kb");
  SendMessage(Win,CB_ADDSTRING,0,(long)"128Kb");
  SendMessage(Win,CB_ADDSTRING,0,(long)"256Kb");
  SendMessage(Win,CB_ADDSTRING,0,(long)"512Kb");
  SendMessage(Win,CB_ADDSTRING,0,(long)"1Mb");
  SendMessage(Win,CB_ADDSTRING,0,(long)"2Mb");
  SendMessage(Win,CB_SETCURSEL,log_to_base_2(MIDI_out_sysex_max/1024)-4,0);
  y+=30;

  x=page_l+10;
  Wid=GetTextSize(Font,T("Available for input")).Width;
  CreateWindow("Static",T("Available for input"),WS_CHILD,
                x,y+4,Wid,20,Handle,HMENU(6030),HInstance,NULL);

  x+=Wid+5;

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
                          x,y,40,200,Handle,HMENU(6031),HInstance,NULL);
  for (int n=2;n<MAX_SYSEX_BUFS;n++){
    SendMessage(Win,CB_ADDSTRING,0,(long)Str(n).Text);
  }
  SendMessage(Win,CB_SETCURSEL,MIDI_in_n_sysex-2,0);
  x+=45;

  Wid=GetTextSize(Font,T("size")).Width;
  CreateWindow("Static",T("size"),WS_CHILD,
                x,y+4,Wid,20,Handle,HMENU(6032),HInstance,NULL);
  x+=Wid+5;


  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
                          x,y,page_w-10-(x-page_l),200,Handle,HMENU(6033),HInstance,NULL);
  SendMessage(Win,CB_ADDSTRING,0,(long)"16Kb");
  SendMessage(Win,CB_ADDSTRING,0,(long)"32Kb");
  SendMessage(Win,CB_ADDSTRING,0,(long)"64Kb");
  SendMessage(Win,CB_ADDSTRING,0,(long)"128Kb");
  SendMessage(Win,CB_ADDSTRING,0,(long)"256Kb");
  SendMessage(Win,CB_ADDSTRING,0,(long)"512Kb");
  SendMessage(Win,CB_ADDSTRING,0,(long)"1Mb");
  SendMessage(Win,CB_ADDSTRING,0,(long)"2Mb");
  SendMessage(Win,CB_SETCURSEL,log_to_base_2(MIDI_in_sysex_max/1024) - 4,0);
  y+=30;

  y+=13;

  CreateWindow("Static",T("Input speed")+": "+Str(MIDI_in_speed)+"%",WS_CHILD | SS_CENTER,
                page_l,y,page_w,20,Handle,HMENU(6040),HInstance,NULL);
  y+=20;

  Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD | WS_TABSTOP | TBS_HORZ,
                    page_l,y,page_w,27,Handle,HMENU(6041),HInstance,NULL);
  SendMessage(Win,TBM_SETRANGEMAX,0,99);
  SendMessage(Win,TBM_SETPOS,true,MIDI_in_speed-1);
  SendMessage(Win,TBM_SETLINESIZE,0,1);
  SendMessage(Win,TBM_SETPAGESIZE,0,5);
  for (int n=4;n<99;n+=5) SendMessage(Win,TBM_SETTIC,0,n);
  y+=40;

  Win=CreateWindowEx(512,"Edit",T("The Steem MIDI interface is only suitable for programs that communicate using MIDI messages.")+"\r\n\r\n"+
                        T("Any program that attempts to send raw data over the MIDI ports (for example a MIDI network game) will not work."),
                        WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
                        page_l,y,page_w,OPTIONS_HEIGHT-y-10,Handle,(HMENU)0,HInstance,NULL);
  MakeEditNoCaret(Win);


  if (Focus==NULL) Focus=GetDlgItem(Handle,6001);
  SetPageControlsFont();
  ShowPageControls();
}
//---------------------------------------------------------------------------
void TOptionBox::CreateSoundPage()
{
  HWND Win;
  long Wid;
  int y=10;
  DWORD DisableIfMute=DWORD(((sound_mode==SOUND_MODE_MUTE) || UseSound==0) ? WS_DISABLED:0);
  DWORD DisableIfNoSound=DWORD((UseSound==0) ? WS_DISABLED:0);
  DWORD DisableIfNT=DWORD(WinNT ? WS_DISABLED:0);

  Wid=GetTextSize(Font,T("Output type")).Width;
  CreateWindow("Static",T("Output type"),WS_CHILD | DisableIfNoSound,
                  page_l,y+4,Wid,23,Handle,(HMENU)7049,HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP |
                  DisableIfNoSound | CBS_DROPDOWNLIST,
                  page_l+5+Wid,y,page_w-(5+Wid),200,Handle,(HMENU)7099,HInstance,NULL),
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("None (Mute)"));
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Simulated ST Speaker"));
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Direct"));
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Sharp STFM Samples"));
  SendMessage(Win,CB_SETCURSEL,sound_mode,0);
  y+=30;

  CreateWindow("Button",T("Configuration"),WS_CHILD | BS_GROUPBOX | DisableIfMute,
                  page_l,y,page_w,230,Handle,(HMENU)7105,HInstance,NULL);
  y+=20;

  Str DrivStr=T("Current driver")+": ";
  EasyStr DSDriverModName=GetCSFStr("Options","DSDriverName","",INIFile);
  if (DSDriverModName.Empty()) DSDriverModName=T("Default");
  DrivStr+=DSDriverModName;

  CreateWindow("Static",DrivStr,WS_CHILD,
                  page_l+10,y,page_w-20,23,Handle,(HMENU)7010,HInstance,NULL);
  y+=25;

  Wid=GetTextSize(Font,T("Volume")+": "+T("Min")).Width;
  CreateWindow("Static",T("Volume")+": "+T("Min"),WS_CHILD | DisableIfMute,
                  page_l+10,y+4,Wid,23,Handle,(HMENU)7050,HInstance,NULL);

  int Wid2=GetTextSize(Font,T("Max")).Width;
  CreateWindow("Static",T("Max"),WS_CHILD | DisableIfMute,
                  page_l+page_w-10-Wid2,y+4,Wid2,23,Handle,(HMENU)7051,HInstance,NULL);

  Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD | WS_TABSTOP |
                    DisableIfMute | TBS_HORZ,
                    page_l+15+Wid,y,(page_w-10-(Wid2+5))-(Wid+15),27,Handle,(HMENU)7100,HInstance,NULL);
  SendMessage(Win,TBM_SETRANGE,0,MAKELPARAM(0,9000));
  SendMessage(Win,TBM_SETPOS,1,MaxVolume+9000);
  SendMessage(Win,TBM_SETLINESIZE,0,100);
  SendMessage(Win,TBM_SETPAGESIZE,0,1000);
  y+=35;

  Wid=GetTextSize(Font,T("Frequency")).Width;
  CreateWindow("Static",T("Frequency"),WS_CHILD | DisableIfMute,
                  page_l+10,y+4,Wid,23,Handle,(HMENU)7052,HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP |
                    DisableIfMute | CBS_DROPDOWNLIST,
                    page_l+15+Wid,y,page_w-10-(15+Wid),200,Handle,(HMENU)7101,HInstance,NULL);
  if (sound_comline_freq){
    CBAddString(Win,Str(sound_comline_freq)+"Hz",sound_comline_freq);
  }
  CBAddString(Win,"50066Hz",50066);
  CBAddString(Win,"44100Hz",44100);
  CBAddString(Win,"25033Hz",25033);
  CBAddString(Win,"22050Hz",22050);
  if (CBSelectItemWithData(Win,sound_chosen_freq)==-1){
    SendMessage(Win,CB_SETCURSEL,CBAddString(Win,Str(sound_chosen_freq)+"Hz",sound_chosen_freq),0);
  }
  y+=30;

  Wid=GetTextSize(Font,T("Format")).Width;
  CreateWindow("Static",T("Format"),WS_CHILD | DisableIfMute,
                  page_l+10,y+4,Wid,23,Handle,(HMENU)7060,HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP |
                    DisableIfMute | CBS_DROPDOWNLIST,
                    page_l+15+Wid,y,page_w-10-(15+Wid),200,Handle,(HMENU)7061,HInstance,NULL);
  CBAddString(Win,T("8-Bit Mono"),MAKEWORD(8,1));
  CBAddString(Win,T("8-Bit Stereo"),MAKEWORD(8,2));
  CBAddString(Win,T("16-Bit Mono"),MAKEWORD(16,1));
  CBAddString(Win,T("16-Bit Stereo"),MAKEWORD(16,2));
  SendMessage(Win,CB_SETCURSEL,(sound_num_bits-8)/4 + (sound_num_channels-1),0);
  y+=30;

  Wid=GetCheckBoxSize(Font,T("Write to primary buffer")).Width;
  Win=CreateWindow("Button",T("Write to primary buffer"),WS_CHILD | WS_TABSTOP |
                          BS_CHECKBOX | DisableIfMute,
                          page_l+10,y,Wid,23,Handle,(HMENU)7102,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,sound_write_primary,0);
  ToolAddWindow(ToolTip,Win,T("Steem tries to output sound in a way that is friendly to other programs.")+" "+
                            T("However some sound cards do not like that, if you are having problems check this option to make Steem take full control."));
  y+=30;

  Wid=GetTextSize(Font,T("Timing method")).Width;
  CreateWindow("Static",T("Timing method"),WS_CHILD | DisableIfMute,
                  page_l+10,y+4,Wid,23,Handle,(HMENU)7053,HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP |
                    DisableIfMute | CBS_DROPDOWNLIST,
                    page_l+15+Wid,y,page_w-10-(15+Wid),200,Handle,(HMENU)7103,HInstance,NULL),
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Play Cursor"));
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Write Cursor"));
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Milliseconds"));
  SendMessage(Win,CB_SETCURSEL,sound_time_method,0);
  y+=30;

  Wid=GetTextSize(Font,T("Delay")).Width;
  CreateWindow("Static",T("Delay"),WS_CHILD | DisableIfMute,
                  page_l+10,y+4,Wid,23,Handle,(HMENU)7054,HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | DisableIfMute | WS_VSCROLL | CBS_DROPDOWNLIST,
                    page_l+15+Wid,y,page_w-10-(15+Wid),300,Handle,(HMENU)7104,HInstance,NULL);
  EasyStr Ms=T("Milliseconds");
  for (int i=0;i<=300;i+=20) CBAddString(Win,Str(i)+" "+Ms);
  SendMessage(Win,CB_SETCURSEL,psg_write_n_screens_ahead,0);
  y+=30;
  y+=5;

  CreateWindow("Button",T("Record"),WS_CHILD | BS_GROUPBOX | DisableIfMute,
                  page_l,y,page_w,80,Handle,(HMENU)7200,HInstance,NULL);
  y+=20;

  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_RECORD),WS_CHILD | DisableIfMute,
                page_l+10,y,25,25,Handle,(HMENU)7201,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,sound_record,0);

  if (WAVOutputFile.Empty()) WAVOutputFile=WriteDir+"\\ST.wav";
  CreateWindowEx(512,"Steem Path Display",WAVOutputFile,
                  WS_CHILD | DisableIfMute,
                  page_l+40,y,page_w-10-75-40,25,Handle,(HMENU)7202,HInstance,NULL);

  CreateWindow("Button",T("Choose"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX |
                          BS_PUSHLIKE | DisableIfMute,
                          page_l+page_w-10-70,y,70,23,Handle,(HMENU)7203,HInstance,NULL);
  y+=30;

  Wid=GetCheckBoxSize(Font,T("Warn before overwrite")).Width;
  Win=CreateWindow("Button",T("Warn before overwrite"),WS_CHILD | WS_TABSTOP |
                          BS_CHECKBOX | DisableIfMute,
                          page_l+10,y,Wid,25,Handle,(HMENU)7204,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,RecordWarnOverwrite,0);
  y+=30;
  y+=5;

  Wid=GetCheckBoxSize(Font,T("Internal speaker sound")).Width;
  Win=CreateWindow("Button",T("Internal speaker sound"),WS_CHILD | WS_TABSTOP |
                          BS_CHECKBOX | DisableIfNT,
                          page_l,y,Wid,25,Handle,(HMENU)7300,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,sound_internal_speaker,0);

  if (Focus==NULL) Focus=GetDlgItem(Handle,7099);
  SetPageControlsFont();
  ShowPageControls();
}
//---------------------------------------------------------------------------
void TOptionBox::CreateStartupPage()
{
  ConfigStoreFile CSF(INIFile);
  bool NoDD=(bool)CSF.GetInt("Options","NoDirectDraw",0);
  int y=10,Wid;
  HWND Win;

  Wid=GetCheckBoxSize(Font,T("Restore previous state")).Width;
  Win=CreateWindow("Button",T("Restore previous state"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
                          page_l,y,Wid,23,Handle,(HMENU)3303,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,AutoLoadSnapShot,0);
  y+=30;

  Wid=get_text_width(T("Filename"));
  CreateWindow("Static",T("Filename"),WS_CHILD,
                            page_l,y+4,Wid,25,Handle,(HMENU)3310,HInstance,NULL);

  Win=CreateWindowEx(512,"Edit",AutoSnapShotName,WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL,
                      page_l+Wid+5,y,page_w-(Wid+5),23,Handle,(HMENU)3311,HInstance,NULL);
  SendMessage(Win,WM_SETFONT,(UINT)Font,0);
  SendMessage(Win,EM_LIMITTEXT,100,0);
  int Len=SendMessage(Win,WM_GETTEXTLENGTH,0,0);
  SendMessage(Win,EM_SETSEL,Len,Len);
  SendMessage(Win,EM_SCROLLCARET,0,0);
  y+=30;


  Wid=GetCheckBoxSize(Font,T("Start in fullscreen mode")).Width;
  Win=CreateWindow("Button",T("Start in fullscreen mode"),WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX | int(NoDD ? WS_DISABLED:0),
                          page_l,y,Wid,23,Handle,(HMENU)3302,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,GetCSFInt("Options","StartFullscreen",0,INIFile),0);
  y+=30;

  Wid=GetCheckBoxSize(Font,T("Draw direct to video memory")).Width;
  Win=CreateWindow("Button",T("Draw direct to video memory"),WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX | int(NoDD ? WS_DISABLED:0),
                          page_l,y,Wid,23,Handle,(HMENU)3304,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,GetCSFInt("Options","DrawToVidMem",Disp.DrawToVidMem,INIFile),0);
  ToolAddWindow(ToolTip,Win,T("Drawing direct to video memory is generally very fast but in some situations on some PCs it might cause Steem to slow down a lot.")+" "+
                    T("If you having problems with speed try turning this option off and restarting Steem."));
  y+=30;

  Wid=GetCheckBoxSize(Font,T("Hide mouse pointer when blit")).Width;
  Win=CreateWindow("Button",T("Hide mouse pointer when blit"),WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX | int(NoDD ? WS_DISABLED:0),
                          page_l,y,Wid,23,Handle,(HMENU)3305,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,GetCSFInt("Options","BlitHideMouse",Disp.BlitHideMouse,INIFile),0);
  ToolAddWindow(ToolTip,Win,T("On some video cards, it makes a mess if the mouse pointer is over the area where the card is trying to draw.")+" "+
                    T("This option, when checked, makes Steem hide the mouse before it draws to the screen.")+" "+
                    T("Unfortunately this can make the mouse pointer flicker when Steem is running."));
  y+=30;

  Wid=GetCheckBoxSize(Font,T("Never use DirectDraw")).Width;
  Win=CreateWindow("Button",T("Never use DirectDraw"),WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX,
                          page_l,y,Wid,23,Handle,(HMENU)3300,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,NoDD,0);
  y+=30;

  Win=CreateWindow("Button",T("Never use DirectSound"),WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX,
                          page_l,y,GetCheckBoxSize(Font,T("Never use DirectSound")).Width,20,
                          Handle,(HMENU)3301,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,CSF.GetInt("Options","NoDirectSound",0),0);
  y+=30;

  Wid=get_text_width(T("Sound driver"));
  CreateWindow("Static",T("Sound driver"),WS_CHILD,
                          page_l,y+4,Wid,20,Handle,(HMENU)3000,HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST,
                    page_l+5+Wid,y,page_w-(5+Wid),200,Handle,(HMENU)3001,HInstance,NULL);
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Default"));
  for (int i=0;i<DSDriverModuleList.NumStrings;i++){
    SendMessage(Win,CB_ADDSTRING,0,(LPARAM)DSDriverModuleList[i].String);
  }
  SendMessage(Win,CB_SETCURSEL,0,0);
  EasyStr DSDriverModName=CSF.GetStr("Options","DSDriverName","");
  if (DSDriverModName.NotEmpty()){
    for (int i=0;i<DSDriverModuleList.NumStrings;i++){
      if (IsSameStr_I(DSDriverModuleList[i].String,DSDriverModName)){
        SendMessage(Win,CB_SETCURSEL,1+i,0);
        break;
      }
    }
  }

  CSF.Close();

  if (Focus==NULL) Focus=GetDlgItem(Handle,3303);
  SetPageControlsFont();
  ShowPageControls();
}
//---------------------------------------------------------------------------
void TOptionBox::CreateUpdatePage()
{
  int Wid;

  ConfigStoreFile CSF(INIFile);
  DWORD Disable=DWORD(Exists(RunDir+"\\SteemUpdate.exe") ? 0:WS_DISABLED);
  int Runs=CSF.GetInt("Update","Runs",0),
      Offline=CSF.GetInt("Update","Offline",0),
      WSError=CSF.GetInt("Update","WSError",0),
      y=10;

  EasyStr Info=EasyStr(" ");
  Info+=T("Update has checked for a new Steem")+" "+Runs+" "+time_or_times(Runs)+"\n ";
  Info+=T("It thought you were off-line")+" "+Offline+" "+time_or_times(Offline)+"\n ";
  Info+=T("It encountered an error")+" "+WSError+" "+time_or_times(WSError)+"\n ";
  CreateWindowEx(512,"Static",Info,WS_CHILD | Disable,
                  page_l,y,page_w,80,Handle,(HMENU)4100,HInstance,NULL);
  y+=90;


  Wid=GetCheckBoxSize(Font,T("Disable automatic update checking")).Width;
  HWND ChildWin=CreateWindow("Button",T("Disable automatic update checking"),WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX | Disable,
                          page_l,y,Wid,20,Handle,(HMENU)4200,HInstance,NULL);
  SendMessage(ChildWin,BM_SETCHECK,!CSF.GetInt("Update","AutoUpdateEnabled",true),0);
  y+=30;

  Wid=GetCheckBoxSize(Font,T("This computer is never off-line")).Width;
  ChildWin=CreateWindow("Button",T("This computer is never off-line"),WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX | Disable,
                          page_l,y,Wid,20,Handle,(HMENU)4201,HInstance,NULL);
  SendMessage(ChildWin,BM_SETCHECK,CSF.GetInt("Update","AlwaysOnline",0),0);
  y+=30;

  Wid=GetCheckBoxSize(Font,T("Download new patches")).Width;
  ChildWin=CreateWindow("Button",T("Download new patches"),WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX | Disable,
                          page_l,y,Wid,20,Handle,(HMENU)4202,HInstance,NULL);
  SendMessage(ChildWin,BM_SETCHECK,CSF.GetInt("Update","PatchDownload",1),0);
  y+=30;

  Wid=GetCheckBoxSize(Font,T("Ask before installing new patches")).Width;
  ChildWin=CreateWindow("Button",T("Ask before installing new patches"),WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX | Disable,
                          page_l,y,Wid,20,Handle,(HMENU)4203,HInstance,NULL);
  SendMessage(ChildWin,BM_SETCHECK,CSF.GetInt("Update","AskPatchInstall",0),0);
  y+=30;

  HANDLE UpdateMutex=OpenMutex(MUTEX_ALL_ACCESS,0,"SteemUpdate_Running");
  if (UpdateMutex){
    CloseHandle(UpdateMutex);
    Disable=WS_DISABLED;
  }else if (UpdateWin || FullScreen){
    Disable=WS_DISABLED;
  }
  CreateWindow("Button",T("Check For Update Now"),WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON | int(UpdateWin || FullScreen ? WS_DISABLED:0) | Disable,
                    page_l,y,page_w,23,Handle,(HMENU)4400,HInstance,NULL);

  CSF.Close();

  if (Focus==NULL) Focus=GetDlgItem(Handle,4200);
  SetPageControlsFont();
  ShowPageControls();
}
//---------------------------------------------------------------------------
void TOptionBox::AssAddToExtensionsLV(char *Ext,char *Desc,int Num)
{
  EasyStr Text=Str(Ext)+" ("+Desc+")";
  int y=5 + 30*Num;
  int ButWid=max(GetTextSize(Font,T("Associated")).Width,GetTextSize(Font,T("Associate")).Width)+16;
  int hoff=12-GetTextSize(Font,Text).Height/2;

  HWND But=CreateWindow("Button","",WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                        5,y,ButWid,23,Scroller.GetControlPage(),HMENU(5100+Num),HInstance,NULL);

  HWND Stat=CreateWindow("Steem HyperLink",Text,WS_CHILD | HL_STATIC | HL_WINDOWBK,ButWid+10,y+hoff,300,25,
                       Scroller.GetControlPage(),(HMENU)5000,HInstance,NULL);

  SendMessage(Stat,WM_SETFONT,WPARAM(Font),0);
  SendMessage(But,WM_SETFONT,WPARAM(Font),0);
  if (IsSteemAssociated(Ext)){
    SendMessage(But,WM_SETTEXT,0,LPARAM(T("Associated").Text));
    EnableWindow(But,0);
  }else{
    SendMessage(But,WM_SETTEXT,0,LPARAM(T("Associate").Text));
  }
  ShowWindow(Stat,SW_SHOW);
  ShowWindow(But,SW_SHOW);
}
//---------------------------------------------------------------------------
void TOptionBox::CreateAssocPage()
{
  HWND Win;

  Scroller.CreateEx(512,WS_CHILD | WS_VSCROLL | WS_HSCROLL,page_l,10,page_w,OPTIONS_HEIGHT-10-10-25-10,
                          Handle,5500,HInstance);
  Scroller.SetBkColour(GetSysColor(COLOR_WINDOW));

  AssAddToExtensionsLV(".ST",T("Disk Image"),0);
  AssAddToExtensionsLV(".STT",T("Disk Image"),1);
  AssAddToExtensionsLV(".MSA",T("Disk Image"),2);
#if USE_PASTI
  if (hPasti) AssAddToExtensionsLV(".STX",T("Pasti Disk Image"),3);
#endif
  AssAddToExtensionsLV(".DIM",T("Disk Image"),4);
  AssAddToExtensionsLV(".STZ",T("Zipped Disk Image"),5);
  AssAddToExtensionsLV(".STS",T("Memory Snapshot"),6);
  AssAddToExtensionsLV(".STC",T("Cartridge Image"),7);
  Scroller.AutoSize(5,5);

  int Wid=GetCheckBoxSize(Font,T("Always open files in new window")).Width;
  Win=CreateWindow("Button",T("Always open files in new window"),
                          WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
                          page_l,OPTIONS_HEIGHT-35,Wid,25,Handle,(HMENU)5502,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,GetCSFInt("Options","OpenFilesInNew",true,INIFile),0);

  if (Focus==NULL) Focus=GetDlgItem(Handle,5502);
  SetPageControlsFont();
  ShowPageControls();
}
//---------------------------------------------------------------------------
void TOptionBox::CreateMacrosPage()
{
  int y=10,x,Wid;
  int ctrl_h=10+20+30+30+10;
  HWND Win;

  DTree.FileMasksESL.DeleteAll();
  DTree.FileMasksESL.Add("",0,RC_ICO_PCFOLDER);
  DTree.FileMasksESL.Add("stmac",0,RC_ICO_OPS_MACROS);
  UpdateDirectoryTreeIcons(&DTree);
  DTree.Create(Handle,page_l,y,page_w,OPTIONS_HEIGHT-ctrl_h-10-30,10000,WS_TABSTOP,
                DTreeNotifyProc,this,MacroDir,T("Macros"));
  y+=OPTIONS_HEIGHT-ctrl_h-30;

  CreateWindow("Button",T("New Macro"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                  page_l,y,page_w/2-5,23,Handle,(HMENU)10001,HInstance,NULL);

  CreateWindow("Button",T("Change Store Folder"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                  page_l+page_w/2+5,y,page_w/2-5,23,Handle,(HMENU)10002,HInstance,NULL);


  y=10+OPTIONS_HEIGHT-ctrl_h;
  CreateWindow("Button",T("Controls"),WS_CHILD | BS_GROUPBOX,
                  page_l,y,page_w,ctrl_h-10-10,Handle,(HMENU)10010,HInstance,NULL);
  y+=20;

  x=page_l+10;
  CreateWindow("Steem Flat PicButton",Str(RC_ICO_RECORD),WS_CHILD | WS_TABSTOP,
                x,y,25,25,Handle,(HMENU)10011,HInstance,NULL);
  x+=30;

  CreateWindow("Steem Flat PicButton",Str(RC_ICO_PLAY_BIG),WS_CHILD | WS_TABSTOP,
                x,y,25,25,Handle,(HMENU)10012,HInstance,NULL);
  x+=30;

  Wid=get_text_width(T("Mouse speed"));
  CreateWindow("Static",T("Mouse speed"),WS_CHILD,x,y+4,Wid,23,Handle,(HMENU)10013,HInstance,NULL);
  x+=Wid+5;

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
                          x,y,page_l+page_w-10-x,400,Handle,(HMENU)10014,HInstance,NULL);
  CBAddString(Win,T("Safe"),15);
  CBAddString(Win,T("Slow"),32);
  CBAddString(Win,T("Medium"),64);
  CBAddString(Win,T("Fast"),96);
  CBAddString(Win,T("V.Fast"),127);
  CBSelectItemWithData(Win,127);
  y+=30;

  x=page_l+10;
  Wid=get_text_width(T("Playback event delay"));
  CreateWindow("Static",T("Playback event delay"),WS_CHILD,x,y+4,Wid,23,Handle,(HMENU)10015,HInstance,NULL);
  x+=Wid+5;

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
                          x,y,page_l+page_w-10-x,400,Handle,(HMENU)10016,HInstance,NULL);
  // Number of VBLs that input is allowed to be the same
  CBAddString(Win,T("As Recorded"),0);
  EasyStr Ms=Str(" ")+T("Milliseconds");
  for (int n=1;n<=25;n++) CBAddString(Win,Str(n*20)+Ms,n);
  CBSelectItemWithData(Win,1);

  DTree.SelectItemByPath(MacroSel);

  if (Focus==NULL) Focus=GetDlgItem(Handle,10000);
  SetPageControlsFont();
  ShowPageControls();
}
//---------------------------------------------------------------------------
void TOptionBox::CreateProfilesPage()
{
  int y=10;
  HWND Win;
  int ctrl_h=OPTIONS_HEIGHT/2-30;

  DTree.FileMasksESL.DeleteAll();
  DTree.FileMasksESL.Add("",0,RC_ICO_PCFOLDER);
  DTree.FileMasksESL.Add("ini",0,RC_ICO_OPS_PROFILES);
  UpdateDirectoryTreeIcons(&DTree);
  DTree.Create(Handle,page_l,y,page_w,OPTIONS_HEIGHT-ctrl_h-40,11000,WS_TABSTOP,
                DTreeNotifyProc,this,ProfileDir,T("Profiles"));
  y+=OPTIONS_HEIGHT-ctrl_h-30;

  CreateWindow("Button",T("Save New Profile"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                  page_l,y,page_w/2-5,23,Handle,(HMENU)11001,HInstance,NULL);

  CreateWindow("Button",T("Change Store Folder"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                  page_l+page_w/2+5,y,page_w/2-5,23,Handle,(HMENU)11002,HInstance,NULL);


  y=10+OPTIONS_HEIGHT-ctrl_h;
  CreateWindow("Button",T("Controls"),WS_CHILD | BS_GROUPBOX,
                  page_l,y,page_w,ctrl_h-10-10,Handle,(HMENU)11010,HInstance,NULL);
  y+=20;

  CreateWindow("Button",T("Load Profile"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                  page_l+10,y,(page_w-20)/2-5,23,Handle,(HMENU)11011,HInstance,NULL);

  CreateWindow("Button",T("Save Over Profile"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                  page_l+10+(page_w-20)/2+5,y,(page_w-20)/2-5,23,Handle,(HMENU)11012,HInstance,NULL);
  y+=30;

  Win=CreateWindowEx(512,WC_LISTVIEW,"",WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_DISABLED |
                      LVS_SINGLESEL | LVS_REPORT | LVS_NOCOLUMNHEADER,
                      page_l+10,y,page_w-20,OPTIONS_HEIGHT-y-15,Handle,(HMENU)11013,HInstance,NULL);
  ListView_SetExtendedListViewStyle(Win,LVS_EX_CHECKBOXES);

  RECT rc;
  GetClientRect(Win,&rc);

  LV_COLUMN lvc;
  lvc.mask=LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
  lvc.fmt=LVCFMT_LEFT;
  lvc.cx=rc.right-GetSystemMetrics(SM_CXVSCROLL);
  lvc.pszText="";
  lvc.iSubItem=0;
  SendMessage(Win,LVM_INSERTCOLUMN,0,LPARAM(&lvc));

  LV_ITEM lvi;
  lvi.mask=LVIF_TEXT | LVIF_PARAM;
  int i=0;
  for(;;){
    if (ProfileSection[i].Name==NULL) break;
    lvi.iSubItem=0;
    lvi.pszText=StaticT(ProfileSection[i].Name);
    lvi.lParam=DWORD(ProfileSection[i].ID);
    lvi.iItem=i++;
    SendMessage(Win,LVM_INSERTITEM,0,(LPARAM)&lvi);
  }

  DTree.SelectItemByPath(ProfileSel);

  if (Focus==NULL) Focus=GetDlgItem(Handle,11000);
  SetPageControlsFont();
  ShowPageControls();
}
//---------------------------------------------------------------------------
void TOptionBox::IconsAddToScroller()
{
  for (int n=14100;n<14100+RC_NUM_ICONS;n++){
    if (GetDlgItem(Scroller.GetControlPage(),n)) DestroyWindow(GetDlgItem(Scroller.GetControlPage(),n));
  }

  int x=3,y=3;
  for (int want_size=16;want_size;want_size<<=1){
    for (int n=1;n<RC_NUM_ICONS;n++){
      int size=RCGetSizeOfIcon(n) & ~1;
      switch (n){
        case RC_ICO_HARDDRIVES:
        case RC_ICO_HARDDRIVES_FR:
          int want_ico=RC_ICO_HARDDRIVES;
          if (IsSameStr_I(T("File"),"Fichier")) want_ico=RC_ICO_HARDDRIVES_FR;
          if (n!=want_ico) size=0;
          break;
      }
      if (size==want_size){
        CreateWindow("Steem Flat PicButton",Str(n),WS_CHILD | PBS_RIGHTCLICK,
             x,y,size+4,size+4,Scroller.GetControlPage(),HMENU(14100+n),HInstance,NULL);
        x+=size+4+3;
      }
      if (x+want_size+4+3 >= page_w-GetSystemMetrics(SM_CXVSCROLL) || n==RC_NUM_ICONS-1){
        x=3;
        y+=want_size+4+3;
      }
    }
  }
  for (int n=14100;n<14100+RC_NUM_ICONS;n++){
    if (GetDlgItem(Scroller.GetControlPage(),n)) ShowWindow(GetDlgItem(Scroller.GetControlPage(),n),SW_SHOWNA);
  }
  Scroller.AutoSize(0,5);
}
//---------------------------------------------------------------------------
void TOptionBox::CreateIconsPage()
{
  int th=GetTextSize(Font,T("Left click to change")).Height;
  int y=10,scroller_h=OPTIONS_HEIGHT-10-th-2-10-25-10;

  CreateWindow("Static",T("Left click to change, right to reset"),WS_CHILD,page_l,y,page_w,th,Handle,(HMENU)14002,HInstance,NULL);
  y+=th+2;

  Scroller.CreateEx(512,WS_CHILD | WS_VSCROLL | WS_HSCROLL,page_l,y,
                      page_w,scroller_h,Handle,14010,HInstance);
  Scroller.SetBkColour(GetSysColor(COLOR_BTNFACE));
  IconsAddToScroller();
  y+=scroller_h+10;

  CreateWindow("Button",T("Load Icon Scheme"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                  page_l,y,page_w/2-5,23,Handle,(HMENU)14020,HInstance,NULL);

  CreateWindow("Button",T("All Icons To Default"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                  page_l+page_w/2+5,y,page_w/2-5,23,Handle,(HMENU)14021,HInstance,NULL);


  if (Focus==NULL) Focus=GetDlgItem(Handle,11000);
  SetPageControlsFont();
  ShowPageControls();
}
//---------------------------------------------------------------------------

