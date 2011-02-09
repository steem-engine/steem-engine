#define CUT_PRESSKEY 0
#define CUT_PRESSCHAR 39
#define CUT_PLAYMACRO 44

#define CUT_TOGGLESTARTSTOP 3
#define CUT_TOGGLEFULLSCREEN 17
#define CUT_TAKESCREENSHOT 29

#define NUM_SHORTCUTS (53+1+1 DEBUG_ONLY(+5))

const char *ShortcutNames[NUM_SHORTCUTS*2]=
  {
#ifdef _DEBUG_BUILD
  "Trace Into",(char*)200,"Step Over",(char*)203,"Run to/for",(char*)204,
  "Run to RTE",(char*)201,"Toggle Suspend Logging",(char*)202,
#endif
  "Press ST Key ->",(char*)CUT_PRESSKEY,"Type ST Character ->",(char*)CUT_PRESSCHAR,

  "Take Screenshot",(char*)CUT_TAKESCREENSHOT,"Take Multiple Screenshots (Hold)",(char*)43,

  "Fast Forward",(char*)6,"Fast Forward (Toggle)",(char*)51,
  "Searchlight Fast Forward",(char*)28,"Searchlight Fast Forward (Toggle)",(char*)52,
  "Slow Motion",(char*)33,"Slow Motion 10%",(char*)35,"Slow Motion 25%",(char*)36,"Slow Motion 50%",(char*)37,

  "Start Emulation",(char*)1,
  "Stop Emulation",(char*)2,"Toggle Emulation Start/Stop",(char*)CUT_TOGGLESTARTSTOP,

#ifdef UNIX
  "Toggle Port 1 Joystick Active",(char*)50,"Toggle Port 0 Joystick Active",(char*)49,
#endif

  "Swap Disks In Drives",(char*)13,

  "Play Macro ->",(char*)CUT_PLAYMACRO,"Stop Playing Macro",(char*)48,
  "Record New Macro Start/Stop",(char*)45,"Record New Macro (Hold)",(char*)46,
  "Stop Macro Recording",(char*)47,

  "Cold Reset",(char*)4,"Cold Reset and Run",(char*)5,
  "Warm Reset",(char*)27,

  "Capture Mouse",(char*)7,
  "Release Mouse",(char*)8,"Toggle Mouse Capture",(char*)9,

  "Toggle Mute",(char*)12,

  "Show Borders (Auto Border Mode Only)",(char*)34,

  "Load Memory Snapshot",(char*)10,"Save Memory Snapshot",(char*)11,"Save Over Last Memory Snapshot",(char*)53,

  "Hide Scrolling Message",(char*)14,

  "Switch to Fullscreen",(char*)15,
  "Switch to Windowed",(char*)16,"Toggle Fullscreen/Windowed",(char*)CUT_TOGGLEFULLSCREEN,

  "Press ST Left Mouse Button",(char*)18,"Press ST Right Mouse Button",(char*)19,

  "Increase ST CPU Speed",(char*)20,"Decrease ST CPU Speed",(char*)21,
  "Normal ST CPU Speed",(char*)22,

  "Exit Steem",(char*)23,

  "Show OSD",(char*)24,"Hide OSD",(char*)25,"Toggle OSD Hide/Show",(char*)26,

  "Start Paste",(char*)40,"Stop Paste",(char*)41,"Toggle Paste Start/Stop",(char*)42,

  "Start Sound Recording",(char*)30,"Stop Sound Recording",(char*)31,
  "Toggle Sound Record On/Off",(char*)32,

  "Pause Until Next SysEx (MIDI)",(char*)38,
  NULL,NULL};
//---------------------------------------------------------------------------
void TShortcutBox::TranslateCutNames()
{
  if (TranslatedCutNamesSL.NumStrings==0){
    TranslatedCutNamesSL.Sort=eslNoSort;
    for (int n=0;n<NUM_SHORTCUTS*2;n+=2){
      if (ShortcutNames[n]==NULL) break;
      long i=(long)ShortcutNames[n+1];
      if (i<200){
        TranslatedCutNamesSL.Add(T((char*)ShortcutNames[n]),i);
      }else{
        TranslatedCutNamesSL.Add((char*)ShortcutNames[n],i); // Debug only shortcuts
      }
    }
  }
}
//---------------------------------------------------------------------------
void CutDisableID(WORD ID)
{
  if (BLANK_DIRID(ID)) return;

  if (HIBYTE(ID)==0){
    CutDisableKey[ID]=true;
    BYTE STKey=key_table[ID];
    if (STKey){
      if (ST_Key_Down[STKey]) HandleKeyPress(ID,true,IGNORE_EXTEND);
    }
#ifdef WIN32
    if (ID==VK_SHIFT){
      if (ST_Key_Down[VK_LSHIFT]) HandleKeyPress(VK_LSHIFT,true,IGNORE_EXTEND);
      if (ST_Key_Down[VK_RSHIFT]) HandleKeyPress(VK_RSHIFT,true,IGNORE_EXTEND);
    }
#endif
  }else if (HIBYTE(ID)>=10){
    int DirID=int((HIBYTE(ID) & 1)==0 ? BYTE(ID):-BYTE(ID));
    int JoyNum=(HIBYTE(ID)-10)/10;
    if (DirID>=200){
      CutDisablePOV[JoyNum][DirID-200]=true;
    }else if (DirID>=100){
      CutButtonMask[JoyNum] &= ~(1 << (DirID-100));
    }else{
      CutDisableAxis[JoyNum][DirID+10]=true;
    }
  }
}
//---------------------------------------------------------------------------
void ShortcutsCheck()
{
  ZeroMemory(CutDisableKey,sizeof(CutDisableKey));
  ZeroMemory(CutDisableAxis,sizeof(CutDisableAxis));
  ZeroMemory(CutDisablePOV,sizeof(CutDisablePOV));
  for (int n=0;n<MAX_PC_JOYS;n++) CutButtonMask[n]=0xffffffff;
  DoSaveScreenShot&=~2; // Clear animation screenshot
  CutModDown=0;

  for (int cuts=0;cuts<2;cuts++){
    if (ShortcutBox.CurrentCutSelType!=2) cuts++;
    SHORTCUTINFO *pCuts=(SHORTCUTINFO*)((cuts==0) ? &(CurrentCuts[0]):&(Cuts[0]));
    int NumItems=int((cuts==0) ? CurrentCuts.NumItems:Cuts.NumItems);
    for (int n=0;n<NumItems;n++){
      int NotPressed=-1,NumBlank=0;
      for (int b=0;b<3;b++){
        if (NOT_BLANK_DIRID(pCuts[n].Id[b])){
          if (IsDirIDPressed(pCuts[n].Id[b],50,0)==0 || bAppActive==0){
            NotPressed=int((NotPressed==-1) ? b:1000);
          }
        }else{
          NumBlank++;
        }
        if (NotPressed==1000) break;
      }
      BYTE Down=(NotPressed==-1 && NumBlank<3);
      if (NotPressed>=0 && NotPressed<=2){
        CutDisableID(pCuts[n].Id[NotPressed]);
      }else if (Down){
        for (int b=0;b<3;b++) CutDisableID(pCuts[n].Id[b]);
      }
      pCuts[n].OldDown=pCuts[n].Down;
      if (pCuts[n].OldDown==2){
        if (Down==0){
          pCuts[n].OldDown=0;
          pCuts[n].Down=0;
        }
      }else{
        pCuts[n].Down=Down;
      }
    }
    for (int n=0;n<NumItems;n++){
      for (int c=0;c<5;c++){
        if (pCuts[n].DisableIfCutDownList[c]==NULL) break;

        if (pCuts[n].DisableIfCutDownList[c]->Down){
          if (pCuts[n].OldDown==1) DoShortcutUp(pCuts[n]);
          pCuts[n].Down=2;
          break;
        }
      }
    }
    for (int n=0;n<NumItems;n++){
      if (pCuts[n].Down==1){
        DoShortcutDown(pCuts[n]);
      }else if (pCuts[n].Down==0 && pCuts[n].OldDown==1){
        DoShortcutUp(pCuts[n]);
      }
    }
    for (int n=0;n<NumItems;n++){
      if (pCuts[n].Down==2){
        bool Clash=0;
        for (int c=0;c<5;c++){
          if (pCuts[n].DisableIfCutDownList[c]==NULL) break;

          if (pCuts[n].DisableIfCutDownList[c]->Down){
            Clash=true;
            break;
          }
        }
        if (Clash==0){
          pCuts[n].OldDown=0;
          pCuts[n].Down=1;

          DoShortcutDown(pCuts[n]);
        }
      }
    }
  }
  MouseWheelMove=0;
}
//---------------------------------------------------------------------------
void DoShortcutDown(SHORTCUTINFO &Inf)
{
#ifdef WIN32
  if (GetWindowLong(StemWin,GWL_STYLE) & WS_DISABLED) return;
#endif
  if (ShortcutBox.Picking) return;
  if (bAppActive==0) return;

  if (Inf.Action==0){
    if (Inf.PressKey==VK_SHIFT || Inf.PressKey==VK_LSHIFT) CutModDown|=1;
    if (Inf.PressKey==VK_SHIFT || Inf.PressKey==VK_RSHIFT) CutModDown|=2;
    if (Inf.PressKey==VK_CONTROL) CutModDown|=b00001100;
    if (Inf.PressKey==VK_MENU) CutModDown|=b00110000;
  }
  switch (Inf.Action){
    case 20:case 21:case 34:case 35:case 36:case 37:case 43:
      break;
    default:
      if (Inf.OldDown) return;
  }

  switch (Inf.Action){
    case 0:
      if (NOT_BLANK_DIRID(Inf.PressKey) && runstate==RUNSTATE_RUNNING &&
            GetForegroundWindow()==StemWin){
        int Extend=IGNORE_EXTEND;
        if (HIBYTE(Inf.PressKey)==1) Extend=1;
        if (Inf.PressKey==VK_SHIFT){
          HandleKeyPress(VK_LSHIFT,0,Extend);
          HandleKeyPress(VK_RSHIFT,0,Extend);
        }else{
          HandleKeyPress(Inf.PressKey,0,Extend | NO_SHIFT_SWITCH);
        }

#ifdef WIN32
        int n=0;
        while (TaskSwitchVKList[n]){
          if (Inf.PressKey==TaskSwitchVKList[n]){
            CutTaskSwitchVKDown[n]=true;
            break;
          }
          n++;
        }
#endif
      }
      break;
    case 1:
      if (fast_forward==0){
        SetForegroundWindow(StemWin);
        if (runstate!=RUNSTATE_RUNNING) PostRunMessage();
      }
      break;
    case 2:
      if (runstate==RUNSTATE_RUNNING) PostRunMessage();
      break;
    case 3:
      if (runstate!=RUNSTATE_RUNNING) SetForegroundWindow(StemWin);
      PostRunMessage();
      break;
    case 4:
      reset_st();
      break;
    case 5:
      SetForegroundWindow(StemWin);
      reset_st(0);
      if (runstate!=RUNSTATE_RUNNING) PostRunMessage();
      break;
    case 6:
      fast_forward_change(true,0);
      break;
    case 7:
      if (FullScreen==0){
        if (stem_mousemode==STEM_MOUSEMODE_DISABLED && runstate==RUNSTATE_RUNNING){
          SetForegroundWindow(StemWin);
          SetStemMouseMode(STEM_MOUSEMODE_WINDOW);
        }
      }else{
        if (runstate!=RUNSTATE_RUNNING){
          SetForegroundWindow(StemWin);
          PostRunMessage();
        }
      }
      break;
    case 8:
      if (stem_mousemode!=STEM_MOUSEMODE_DISABLED && runstate==RUNSTATE_RUNNING){
        if (FullScreen==0){
          SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
        }else{
          PostRunMessage();
        }
      }
      break;
    case 9:
      if (FullScreen==0){
        if (runstate==RUNSTATE_RUNNING){
          if (stem_mousemode==STEM_MOUSEMODE_DISABLED){
            SetForegroundWindow(StemWin);
            SetStemMouseMode(STEM_MOUSEMODE_WINDOW);
          }else{
            SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
          }
        }
      }else{
        if (runstate!=RUNSTATE_RUNNING) SetForegroundWindow(StemWin);
        PostRunMessage();
      }
      break;

    case 10:case 11:case 53:
    {
      int i=190+Inf.Action; // 200=Load, 201=Save
      if (Inf.Action==53) i=205; // 205=Save Over Last
#ifdef WIN32
      PostMessage(StemWin,WM_COMMAND,i,0);
#elif defined(UNIX)
    	SnapShotBut.set_check(true);
      SnapShotProcess(i);
    	SnapShotBut.set_check(0);
#endif
      break;
    }

    case 12: //Toggle Mute
    {
      if (sound_mode!=SOUND_MODE_MUTE) sound_last_mode=sound_mode;
      OptionBox.SoundModeChange(int(sound_mode ? SOUND_MODE_MUTE:sound_last_mode),0,true);
      break;
    }
    case 13: //Swap Disks
      DiskMan.SwapDisks(-1);
      break;
    case 14:  // Stop current scroller
      if (timeGetTime()<osd_scroller_finish_time) osd_scroller_finish_time=0;

      break;
    case 15:
      if (FullScreen==0 && Disp.CanGoToFullScreen()){
#ifdef WIN32
        PostMessage(StemWin,WM_SYSCOMMAND,SC_MAXIMIZE,0);
#elif defined(UNIX)
#endif
      }
      break;
    case 16:
      if (FullScreen){
        if (runstate==RUNSTATE_RUNNING) Disp.RunOnChangeToWindow=true;
#ifdef WIN32
        PostMessage(StemWin,WM_COMMAND,MAKEWPARAM(106,BN_CLICKED),(LPARAM)GetDlgItem(StemWin,106));
#elif defined(UNIX)
        if (runstate==RUNSTATE_RUNNING) PostRunMessage();
#endif
      }
      break;
    case 17:
      if (FullScreen==0){
        if (Disp.CanGoToFullScreen()){
#ifdef WIN32
          PostMessage(StemWin,WM_SYSCOMMAND,SC_MAXIMIZE,0);
#elif defined(UNIX)
#endif
        }
      }else{
        if (runstate==RUNSTATE_RUNNING) Disp.RunOnChangeToWindow=true;
#ifdef WIN32
        PostMessage(StemWin,WM_COMMAND,MAKEWPARAM(106,BN_CLICKED),(LPARAM)GetDlgItem(StemWin,106));
#elif defined(UNIX)
        if (runstate==RUNSTATE_RUNNING) PostRunMessage();
#endif
      }
      break;
    case 18:
      CutButtonDown[0]=true;
      break;
    case 19:
      CutButtonDown[1]=true;
      break;
    case 20:
      if (n_cpu_cycles_per_second<128000000){
        n_cpu_cycles_per_second+=1000000;
        if (runstate==RUNSTATE_RUNNING) osd_init_run(0);
        prepare_cpu_boosted_event_plans();
      }
      break;
    case 21:
      if (n_cpu_cycles_per_second>8000000){
        n_cpu_cycles_per_second-=1000000;
        if (runstate==RUNSTATE_RUNNING) osd_init_run(0);
        prepare_cpu_boosted_event_plans();
      }
      break;
    case 22:
      n_cpu_cycles_per_second=8000000;
      prepare_cpu_boosted_event_plans();
      break;
    case 23:
      QuitSteem();
      break;
    case 24:
      if (runstate==RUNSTATE_RUNNING) osd_init_run(0);
      break;
    case 25:
      if (runstate==RUNSTATE_RUNNING) osd_hide();
      break;
    case 26:
      if (runstate==RUNSTATE_RUNNING){
        if (osd_is_on(true)==0){
          osd_init_run(0);
        }else{
          osd_hide();
        }
      }
      break;
    case 27:
      reset_st(0,true);
      break;
    case 28:
      fast_forward_change(true,true);
      break;
    case 43:
      DoSaveScreenShot|=2;
      break;
    case 29:
      if (runstate==RUNSTATE_RUNNING){
        DoSaveScreenShot|=1;
      }else{
        Disp.SaveScreenShot();
      }
      break;
    case 30:
      OptionBox.SetRecord(true);
      break;
    case 31:
      OptionBox.SetRecord(0);
      break;
    case 32:
      OptionBox.SetRecord(!sound_record);
      break;
    case 33:
      if (slow_motion==0) slow_motion_change(true);
      break;
    case 34:
      if (runstate==RUNSTATE_RUNNING) overscan=OVERSCAN_MAX_COUNTDOWN*2;
      break;
    case 35:
      cut_slow_motion_speed=100;
      if (slow_motion==0) slow_motion_change(true);
      break;
    case 36:
      cut_slow_motion_speed=250;
      if (slow_motion==0) slow_motion_change(true);
      break;
    case 37:
      cut_slow_motion_speed=500;
      if (slow_motion==0) slow_motion_change(true);
      break;
    case 38:
      CutPauseUntilSysEx_Time=timeGetTime()+5000;
      break;
    case 39:
    {
      if (Inf.PressChar==0 || runstate!=RUNSTATE_RUNNING) break;

      int ModifierRestoreArray[3]={0,0,0};
      BYTE STCode=LOBYTE(LOWORD(Inf.PressChar));
      BYTE Modifiers=HIBYTE(LOWORD(Inf.PressChar));
      ShiftSwitchChangeModifiers(Modifiers & BIT_0,Modifiers & BIT_1,ModifierRestoreArray);
      keyboard_buffer_write_n_record(STCode);
      keyboard_buffer_write_n_record(BYTE(STCode | BIT_7));
      ShiftSwitchRestoreModifiers(ModifierRestoreArray);
      break;
    }
    case 40:case 41:case 42:
    {
      int n=STPASTE_START;
      if (Inf.Action==41) n=STPASTE_STOP;
      if (Inf.Action==42) n=STPASTE_TOGGLE;
      PasteIntoSTAction(n);
      break;
    }
    case 44:
      if (Inf.MacroFileIdx<0) break;
      if (macro_play) macro_end(MACRO_ENDPLAY);

      macro_play_file=Inf.pESL->Get(Inf.MacroFileIdx).String;
      macro_advance(MACRO_STARTPLAY);
      break;
    case 45:case 46:
    {
      if (macro_record){
        macro_end(MACRO_ENDRECORD);
        break;
      }
      Str File=OptionBox.CreateMacroFile(0);
      if (File.Empty()) break;

      macro_record_file=File;
      macro_advance(MACRO_STARTRECORD);
      break;
    }
    case 47:
      if (macro_record) macro_end(MACRO_ENDRECORD);
      break;
    case 48:
      if (macro_play) macro_end(MACRO_ENDPLAY);
      break;

#ifdef UNIX
    case 49:case 50:
    {
      int i=(Inf.Action)-49;
      Joy[i].ToggleKey=!Joy[i].ToggleKey;
      JoyConfig.enable_but[i].set_check(bool(Joy[i].ToggleKey));
      break;
    }
#endif
    case 51:case 52:
      if (fast_forward_stuck_down){
        fast_forward_stuck_down=0;
        fast_forward_change(0,0);
      }else{
        fast_forward_stuck_down=true;
        fast_forward_change(true,(Inf.Action==52));
      }
      break;

#ifdef _DEBUG_BUILD
    case 200: // Trace Into
      if (runstate!=RUNSTATE_RUNNING || GetForegroundWindow()!=StemWin){
        PostMessage(DWin,WM_COMMAND,MAKEWPARAM(1002,BN_CLICKED),0);
      }
      break;
    case 203: // Step Over
      if (runstate!=RUNSTATE_RUNNING || GetForegroundWindow()!=StemWin){
        PostMessage(DWin,WM_COMMAND,MAKEWPARAM(1003,BN_CLICKED),0);
      }
      break;
    case 201: // Run to RTE
      if (runstate!=RUNSTATE_RUNNING || GetForegroundWindow()!=StemWin){
        PostMessage(DWin,WM_COMMAND,MAKEWPARAM(1011,BN_CLICKED),0);
      }
      break;
    case 202: // Suspend logging
      if (runstate!=RUNSTATE_RUNNING || GetForegroundWindow()!=StemWin){
        PostMessage(DWin,WM_COMMAND,1012,0);
      }
      break;
    case 204: // Run to/for
      if (runstate!=RUNSTATE_RUNNING || GetForegroundWindow()!=StemWin){
        PostMessage(DWin,WM_COMMAND,MAKEWPARAM(1022,BN_CLICKED),0);
      }
      break;
#endif
  }
}
//---------------------------------------------------------------------------
void DoShortcutUp(SHORTCUTINFO &Inf)
{
  switch (Inf.Action){
    case 0:
      if (NOT_BLANK_DIRID(Inf.PressKey) && runstate==RUNSTATE_RUNNING &&
            GetForegroundWindow()==StemWin){
        int Extend=IGNORE_EXTEND;
        if (HIBYTE(Inf.PressKey)==1) Extend=1;
        if (Inf.PressKey==VK_SHIFT){
          HandleKeyPress(VK_LSHIFT,true,Extend);
          HandleKeyPress(VK_RSHIFT,true,Extend);
        }else{
          HandleKeyPress(Inf.PressKey,true,Extend | NO_SHIFT_SWITCH);
        }

#ifdef WIN32
        int n=0;
        while (TaskSwitchVKList[n]){
          if (Inf.PressKey==TaskSwitchVKList[n]){
            CutTaskSwitchVKDown[n]=0;
            break;
          }
          n++;
        }
#endif
      }
      break;
    case 6:
      if (flashlight_flag==0) fast_forward_change(0,0);
      break;
    case 18:
      CutButtonDown[0]=0;
      break;
    case 19:
      CutButtonDown[1]=0;
      break;
    case 28:
      if (flashlight_flag) fast_forward_change(0,0);
      break;
    case 33: if (cut_slow_motion_speed==0) slow_motion_change(0); break;
    case 35:
      if (cut_slow_motion_speed==100){
        slow_motion_change(0);
        cut_slow_motion_speed=0;
      }
      break;
    case 36:
      if (cut_slow_motion_speed==250){
        slow_motion_change(0);
        cut_slow_motion_speed=0;
      }
      break;
    case 37:
      if (cut_slow_motion_speed==500){
        slow_motion_change(0);
        cut_slow_motion_speed=0;
      }
      break;
    case 46:
      if (macro_record) macro_end(MACRO_ENDRECORD);
      break;
  }
}
//---------------------------------------------------------------------------
void TShortcutBox::UpdateDisableIfDownLists()
{
  for (int cuts=0;cuts<2;cuts++){
    if (CurrentCutSelType!=2) cuts++;
    SHORTCUTINFO *pCuts=(SHORTCUTINFO*)((cuts==0) ? &(CurrentCuts[0]):&(Cuts[0]));
    int NumItems=int((cuts==0) ? CurrentCuts.NumItems:Cuts.NumItems);
    for (int n=0;n<NumItems;n++){
      for (int i=0;i<5;i++) pCuts[n].DisableIfCutDownList[i]=NULL;

      int Num0s=0;
      for (int i=0;i<3;i++){
        if (BLANK_DIRID(pCuts[n].Id[i])) Num0s++;
      }
      if (Num0s){
        for (int cuts2=0;cuts2<2;cuts2++){
          if (CurrentCutSelType!=2) cuts2++;
          SHORTCUTINFO *pCuts2=(SHORTCUTINFO*)((cuts2==0) ? &(CurrentCuts[0]):&(Cuts[0]));
          int NumItems2=int((cuts2==0) ? CurrentCuts.NumItems:Cuts.NumItems);
          for (int c=0;c<NumItems2;c++){
            if (pCuts2!=pCuts || c!=n){
              int CutNum0s=0;
              for (int i=0;i<3;i++){
                if (BLANK_DIRID(pCuts2[c].Id[i])) CutNum0s++;
              }
              if (Num0s>CutNum0s){
                int NumSame=0;
                for (int tn_i=0;tn_i<3;tn_i++){
                  if (NOT_BLANK_DIRID(pCuts[n].Id[tn_i])){
                    for (int tc_i=0;tc_i<3;tc_i++){
                      if (pCuts[n].Id[tn_i]==pCuts2[c].Id[tc_i]) NumSame++;
                    }
                  }
                }
                if (NumSame+Num0s>=3){
                  for (int i=0;i<5;i++){
                    if (pCuts[n].DisableIfCutDownList[i]==NULL){
                      pCuts[n].DisableIfCutDownList[i]=&pCuts2[c];
                      break;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}
//---------------------------------------------------------------------------
#ifdef WIN32

TShortcutBox::TShortcutBox()
{
  Left=(GetSystemMetrics(SM_CXSCREEN)-586)/2;
  Top=(GetSystemMetrics(SM_CYSCREEN)-(406+GetSystemMetrics(SM_CYCAPTION)))/2;

  FSLeft=(640-586)/2;
  FSTop=(480-(406+GetSystemMetrics(SM_CYCAPTION)))/2;

  ScrollPos=0;

  CutFiles.Sort=eslNoSort;

  PopupOpen=0;

  Section="Shortcuts";
}
//---------------------------------------------------------------------------
void TShortcutBox::ManageWindowClasses(bool Unreg)
{
  char *ClassName="Steem Shortcuts";
  if (Unreg){
    UnregisterClass(ClassName,Inst);
  }else{
    RegisterMainClass(WndProc,ClassName,RC_ICO_SHORTCUT);
  }
}
//---------------------------------------------------------------------------
void TShortcutBox::Show()
{
  if (Handle!=NULL){
    ShowWindow(Handle,SW_SHOWNORMAL);
    SetForegroundWindow(Handle);
    return;
  }
  if (FullScreen) Top=max(Top,MENUHEIGHT);

  ManageWindowClasses(SD_REGISTER);
  Handle=CreateWindowEx(WS_EX_CONTROLPARENT,"Steem Shortcuts",T("Shortcuts"),
                          WS_CAPTION | WS_SYSMENU,Left,Top,586,406+GetSystemMetrics(SM_CYCAPTION),
                          ParentWin,NULL,Inst,NULL);
  if (HandleIsInvalid()){
    ManageWindowClasses(SD_UNREGISTER);
    return;
  }

  SetWindowLong(Handle,GWL_USERDATA,(long)this);

  MakeParent(HWND(FullScreen ? StemWin:NULL));

  int y=10;
  DTree.AllowTypeChange=true;
  DTree.FileMasksESL.DeleteAll();
  DTree.FileMasksESL.Add("",0,RC_ICO_PCFOLDER);
  DTree.FileMasksESL.Add("stcut",0,RC_ICO_SHORTCUT_OFF);
  DTree.FileMasksESL.Add("stcut",0,RC_ICO_SHORTCUT_ON);
  UpdateDirectoryTreeIcons(&DTree);
  DTree.Create(Handle,10,y,300,100,100,WS_VISIBLE | WS_TABSTOP,DTreeNotifyProc,this,CutDir,T("Shortcuts"));

  InfoWin=CreateWindowEx(512,"Static","",WS_CHILD | WS_VISIBLE,
                          320,y,250,130,Handle,(HMENU)50,HInstance,NULL);
  y+=105;

  CreateWindow("Button",T("New Shortcuts"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                10,y,145,23,Handle,(HMENU)70,HInstance,NULL);

  CreateWindow("Button",T("Change Store Folder"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                165,y,145,23,Handle,(HMENU)71,HInstance,NULL);
  y+=30;

  Scroller.CreateEx(WS_EX_DLGMODALFRAME,WS_CHILD | WS_VISIBLE | WS_VSCROLL,
                        10,y,560,390-y,Handle,101,HInstance);
  Scroller.SetVDisableNoScroll(true);


  CreateWindow("Button",T("Add New"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                4,4,275-GetSystemMetrics(SM_CXVSCROLL),23,Scroller,(HMENU)60,HInstance,NULL);

  CreateWindow("Button",T("Add Copy"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                279,4,275-GetSystemMetrics(SM_CXVSCROLL),23,Scroller,(HMENU)61,HInstance,NULL);

  DTree.SelectItemByPath(CurrentCutSel);
  Scroller.SetVPos(ScrollPos);

  SetWindowAndChildrensFont(Handle,Font);

  Focus=DTree.hTree;

  ShowWindow(Handle,SW_SHOW);
  SetFocus(Focus);

  if (StemWin) PostMessage(StemWin,WM_USER,1234,0);
}
//---------------------------------------------------------------------------
#define HIGHEST_CUT_CONTROL_ID 11

void TShortcutBox::AddPickerLine(int p)
{
  HWND Win;
  int Base=1000;
  int x=4,y=4;
  bool PressKey=(CurrentCuts[p].Action==CUT_PRESSKEY),
        PressChar=(CurrentCuts[p].Action==CUT_PRESSCHAR),
        PlayMacro=(CurrentCuts[p].Action==CUT_PLAYMACRO);

  Win=CreateWindowEx(512,"Steem Button Picker","",WS_CHILD | BPS_INSHORTCUT,
                 x,p*30+y,65,23,Scroller,HMENU(Base+p*100),HInstance,NULL);
  SetWindowWord(Win,0,CurrentCuts[p].Id[0]);

  CreateWindow("Static","+",WS_CHILD | SS_CENTER,
                   65+x,3+p*30+y,9,23,Scroller,HMENU(Base+p*100+6),HInstance,NULL);

  Win=CreateWindowEx(512,"Steem Button Picker","",WS_CHILD | BPS_INSHORTCUT,
                   75+x,p*30+y,65,23,Scroller,HMENU(Base+p*100+1),HInstance,NULL);
  SetWindowWord(Win,0,CurrentCuts[p].Id[1]);

  CreateWindow("Static","+",WS_CHILD | SS_CENTER,
                  140+x,3+p*30+y,9,23,Scroller,HMENU(Base+p*100+7),HInstance,NULL);

  Win=CreateWindowEx(512,"Steem Button Picker","",WS_CHILD | BPS_INSHORTCUT,
                 150+x,p*30+y,65,23,Scroller,HMENU(Base+p*100+2),HInstance,NULL);
  SetWindowWord(Win,0,CurrentCuts[p].Id[2]);

  CreateWindow("Static","=",WS_CHILD | SS_CENTER,
                   215+x,3+p*30+y,9,23,Scroller,HMENU(Base+p*100+8),HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
                    225+x,p*30+y,int((PressKey || PressChar || PlayMacro) ? 210:270),300,
                    Scroller.GetControlPage(),HMENU(Base+p*100+3),HInstance,NULL);
  TranslateCutNames();
  for (int n=0;n<TranslatedCutNamesSL.NumStrings;n++){
	  CBAddString(Win,TranslatedCutNamesSL[n].String,TranslatedCutNamesSL[n].Data[0]);
  }
  for (int i=0;i<2;i++){
    if (CBSelectItemWithData(Win,CurrentCuts[p].Action)!=CB_ERR) break;
    CBAddString(Win,T("Other"),CurrentCuts[p].Action);
  }
  SendMessage(Win,CB_SETDROPPEDWIDTH,270,0);

  Win=CreateWindowEx(512,"Steem Button Picker","",WS_CHILD | BPS_NOJOY | BPS_INSHORTCUT,
                      440+x,p*30+y,55,23,Scroller,HMENU(Base+p*100+4),HInstance,NULL);
  SetWindowWord(Win,0,CurrentCuts[p].PressKey);

  Win=CreateWindowEx(512,"Steem ST Character Chooser","",
                WS_CHILD | WS_TABSTOP,440+x,p*30+y,55,25,Scroller,HMENU(Base+p*100+10),HInstance,NULL);
  SendMessage(Win,CB_SETCURSEL,0,CurrentCuts[p].PressChar);

  Win=CreateWindow("Button",T("Choose"),WS_CHILD | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                440+x,p*30+y,55,25,Scroller,HMENU(Base+p*100+11),HInstance,NULL);
  SetMacroFileButtonText(Win,p);

  CreateWindow("Button",T("Del"),WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON,
                500+x,p*30+y,49-GetSystemMetrics(SM_CXVSCROLL),23,Scroller,HMENU(Base+p*100+5),HInstance,NULL);

  for (int i=Base+p*100;i<=Base+p*100+HIGHEST_CUT_CONTROL_ID;i++){
    if ((i % 100)==4){
      ShowWindow(GetDlgItem(Scroller.GetControlPage(),i),int(PressKey ? SW_SHOW:SW_HIDE));
    }else if ((i % 100)==10){
      ShowWindow(GetDlgItem(Scroller.GetControlPage(),i),int(PressChar ? SW_SHOW:SW_HIDE));
    }else if ((i % 100)==11){
      ShowWindow(GetDlgItem(Scroller.GetControlPage(),i),int(PlayMacro ? SW_SHOW:SW_HIDE));
    }else{
      if (GetDlgItem(Scroller.GetControlPage(),i)){
        ShowWindow(GetDlgItem(Scroller.GetControlPage(),i),SW_SHOW);
      }
    }
  }
}
//---------------------------------------------------------------------------
void TShortcutBox::SetMacroFileButtonText(HWND But,int p)
{
  if (CurrentCuts[p].MacroFileIdx>=0){
    Str Text=CurrentCutsStrings[CurrentCuts[p].MacroFileIdx].String;
    Str Name=GetFileNameFromPath(Text);
    char *dot=strrchr(Name,'.');
    if (dot) *dot=0;
    SendMessage(But,WM_SETTEXT,0,LPARAM(Name.Text));
  }else{
    SendMessage(But,WM_SETTEXT,0,LPARAM(T("Choose").Text));
  }
}
//---------------------------------------------------------------------------
void TShortcutBox::Hide()
{
  if (Handle==NULL) return;

  ScrollPos=Scroller.GetVPos();

  ShowWindow(Handle,SW_HIDE);
  if (FullScreen) SetFocus(StemWin);

  DestroyWindow(Handle);Handle=NULL;
  TranslatedCutNamesSL.DeleteAll();

  if (CurrentCutSelType) SaveShortcutInfo(CurrentCuts,CurrentCutSel);
  LoadAllCuts();

  if (StemWin) PostMessage(StemWin,WM_USER,1234,0);
  ManageWindowClasses(SD_UNREGISTER);
}
//---------------------------------------------------------------------------
bool TShortcutBox::HasHandledMessage(MSG *mess)
{
  if (Handle){
    if (mess->message==WM_KEYDOWN){
      if (mess->wParam==VK_TAB){
        if (GetKeyState(VK_CONTROL)>=0) return IsDialogMessage(Handle,mess);
      }
    }
    return 0;
  }else{
    return 0;
  }
}
//---------------------------------------------------------------------------
void TShortcutBox::UpdateAddButsPosition()
{
  if (GetDlgItem(Scroller.GetControlPage(),60)==NULL) return;
  
  SetWindowPos(GetDlgItem(Scroller.GetControlPage(),60),0,
                4,4 + CurrentCuts.NumItems*30,0,0,SWP_NOZORDER | SWP_NOSIZE);
  SetWindowPos(GetDlgItem(Scroller.GetControlPage(),61),0,
                279,4 + CurrentCuts.NumItems*30,0,0,SWP_NOZORDER | SWP_NOSIZE);
}
//---------------------------------------------------------------------------
Str TShortcutBox::ShowChooseMacroBox(Str CurrentMacro)
{
  EnableAllWindows(0,Handle);

  PopupOpen=true;

  WNDCLASS wc={0,ChooseMacroWndProc,0,0,HInstance,NULL,PCArrow,
                HBRUSH(COLOR_BTNFACE+1),NULL,"Steem Shortcuts Choose Macro Dialog"};

  RegisterClass(&wc);

  HWND Win=CreateWindowEx(WS_EX_CONTROLPARENT | int(FullScreen ? WS_EX_TOPMOST:0),
                          "Steem Shortcuts Choose Macro Dialog",T("Choose a Macro"),WS_CAPTION,
                           100,100,326,356+GetSystemMetrics(SM_CYCAPTION),
                           Handle,NULL,HInstance,NULL);

  if (Win==NULL || IsWindow(Win)==0) return "";

  SetWindowLong(Win,GWL_USERDATA,(long)this);

  CreateWindow("Button",T("OK"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                    100,320,100,23,Win,(HMENU)IDOK,HInstance,NULL);

  CreateWindow("Button",T("Cancel"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                    210,320,100,23,Win,(HMENU)IDCANCEL,HInstance,NULL);

  DirectoryTree Tree;
  pChooseMacroTree=&Tree;
  Tree.FileMasksESL.DeleteAll();
  Tree.FileMasksESL.Add("",0,RC_ICO_PCFOLDER);
  Tree.FileMasksESL.Add("stmac",0,RC_ICO_OPS_MACROS);
  UpdateDirectoryTreeIcons(&Tree);
  Tree.Create(Win,10,10,300,300,100,WS_VISIBLE | WS_TABSTOP,ChooseMacroTreeNotifyProc,this,
                OptionBox.MacroDir,T("Macros"),true);
  Tree.SelectItemByPath(CurrentMacro);

  SetWindowAndChildrensFont(Win,Font);

  CentreWindow(Win,0);
  PopupFocus=Tree.hTree;
  ShowWindow(Win,SW_SHOW);
  EnableWindow(Handle,0);

  MSG mess;
  while (GetMessage(&mess,NULL,0,0)){
    if (IsDialogMessage(Win,&mess)==0){
      TranslateMessage(&mess);
      DispatchMessage(&mess);
    }
    if (PopupOpen==0) break;
  }
  if (mess.message==WM_QUIT){
    QuitSteem();
    return "";
  }

  EnableWindow(Handle,true);
  SetForegroundWindow(Handle);
  EnableAllWindows(true,Handle);

  pChooseMacroTree=NULL;
  DestroyWindow(Win);
  UnregisterClass(wc.lpszClassName,Inst);

  Str Temp=ChooseMacroSel;
  ChooseMacroSel="";
  return Temp;
}
//---------------------------------------------------------------------------
#define GET_THIS This=(TShortcutBox*)GetWindowLong(Win,GWL_USERDATA);

LRESULT __stdcall TShortcutBox::WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  LRESULT Ret=DefStemDialogProc(Win,Mess,wPar,lPar);
  if (StemDialog_RetDefVal) return Ret;

  if (DTree.ProcessMessage(Mess,wPar,lPar)) return DTree.WndProcRet;

  TShortcutBox *This;
  switch (Mess){
    case WM_COMMAND:
    {
      GET_THIS;
      switch (LOWORD(wPar)){
        case 60:case 61:
        {
          SHORTCUTINFO si;
          if (LOWORD(wPar)==61 && CurrentCuts.NumItems){ // Copy
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
          SetWindowAndChildrensFont(This->Scroller.GetControlPage(),This->Font);
          This->UpdateAddButsPosition();
          This->Scroller.AutoSize();
          This->Scroller.SetVPos(32000);
          This->UpdateDisableIfDownLists();
          break;
        }
        case 70:
          if (HIWORD(wPar)!=BN_CLICKED) break;
          DTree.NewItem(T("New Shortcuts"),DTree.RootItem,1);
          break;
        case 71:
        {
          if (HIWORD(wPar)!=BN_CLICKED) break;

          SendMessage(HWND(lPar),BM_SETCHECK,1,true);
          EnableAllWindows(0,Win);

          EasyStr NewFol=ChooseFolder(HWND(FullScreen ? StemWin:Win),T("Pick a Folder"),This->CutDir);
          if (NewFol.NotEmpty() && NotSameStr_I(NewFol,This->CutDir)){
            NO_SLASH(NewFol);
            This->CutDir=NewFol;
            CutFiles.DeleteAll();
            This->LoadAllCuts();

            DTree.RootFol=NewFol;
            DTree.RefreshDirectory();
          }

          SetForegroundWindow(Win);
          EnableAllWindows(true,Win);
          SetFocus(HWND(lPar));
          SendMessage(HWND(lPar),BM_SETCHECK,0,true);
          break;
        }
      }
      if (LOWORD(wPar)>=1000 && LOWORD(wPar)<40000){
        int Base=1000;
        int Num=(LOWORD(wPar)-Base)/100;
        int ID=LOWORD(wPar) % 100;
        HWND Par=This->Scroller.GetControlPage();
        switch (ID){
          case 0:case 1:case 2: // Buttons picked
            CurrentCuts[Num].Id[ID]=GetWindowWord(HWND(lPar),0);
            CurrentCuts[Num].Down=2;
            This->UpdateDisableIfDownLists();
            break;
          case 3:
            if (HIWORD(wPar)==CBN_SELENDOK){
              int OldAction=CurrentCuts[Num].Action;
              CurrentCuts[Num].Action=(BYTE)SendMessage(HWND(lPar),CB_GETITEMDATA,
                                              SendMessage(HWND(lPar),CB_GETCURSEL,0,0),0);
              if (CurrentCuts[Num].Action!=OldAction){
                ShowWindow(GetDlgItem(Par,Base + Num*100 + 4),SW_HIDE);
                ShowWindow(GetDlgItem(Par,Base + Num*100 + 10),SW_HIDE);
                ShowWindow(GetDlgItem(Par,Base + Num*100 + 11),SW_HIDE);
                switch (CurrentCuts[Num].Action){
                  case CUT_PRESSKEY:
                    SetWindowPos(GetDlgItem(Par,LOWORD(wPar)),0,0,0,210,300,SWP_NOZORDER | SWP_NOMOVE);
                    ShowWindow(GetDlgItem(Par,Base + Num*100 + 4),SW_SHOW);
                    break;
                  case CUT_PRESSCHAR:
                    SetWindowPos(GetDlgItem(Par,LOWORD(wPar)),0,0,0,210,300,SWP_NOZORDER | SWP_NOMOVE);
                    ShowWindow(GetDlgItem(Par,Base + Num*100 + 10),SW_SHOW);
                    break;
                  case CUT_PLAYMACRO:
                    SetWindowPos(GetDlgItem(Par,LOWORD(wPar)),0,0,0,210,300,SWP_NOZORDER | SWP_NOMOVE);
                    ShowWindow(GetDlgItem(Par,Base + Num*100 + 11),SW_SHOW);
                    break;
                  default:
                    SetWindowPos(GetDlgItem(Par,LOWORD(wPar)),0,0,0,270,300,SWP_NOZORDER | SWP_NOMOVE);
                }
              }
            }
            break;
          case 4:
            CurrentCuts[Num].PressKey=(WORD)GetWindowWord(HWND(lPar),0);
            CurrentCuts[Num].Down=2;
            break;
          case 5:           // Del
            if (HIWORD(wPar)==BN_CLICKED){
              for (int n=Num;n<CurrentCuts.NumItems-1;n++){
                CurrentCuts[n].Id[0]=CurrentCuts[n+1].Id[0];
                SetWindowWord(GetDlgItem(Par,Base+n*100),0,CurrentCuts[n].Id[0]);
                CurrentCuts[n].Id[1]=CurrentCuts[n+1].Id[1];
                SetWindowWord(GetDlgItem(Par,Base+n*100+1),0,CurrentCuts[n].Id[1]);
                CurrentCuts[n].Id[2]=CurrentCuts[n+1].Id[2];
                SetWindowWord(GetDlgItem(Par,Base+n*100+2),0,CurrentCuts[n].Id[2]);

                CurrentCuts[n].Action=CurrentCuts[n+1].Action;
                CBSelectItemWithData(GetDlgItem(Par,Base+n*100+3),CurrentCuts[n].Action);

                bool PressKey=(CurrentCuts[n].Action==CUT_PRESSKEY);
                bool PressChar=(CurrentCuts[n].Action==CUT_PRESSCHAR);
                bool PlayMacro=(CurrentCuts[n].Action==CUT_PLAYMACRO);

                SetWindowPos(GetDlgItem(Par,Base+n*100+3),NULL,0,0,
                              int((PressKey || PressChar || PlayMacro) ? 210:270),300,SWP_NOZORDER | SWP_NOMOVE);

                CurrentCuts[n].PressKey=CurrentCuts[n+1].PressKey;
                SetWindowWord(GetDlgItem(Par,Base+n*100+4),0,(WORD)CurrentCuts[n].PressKey);
                ShowWindow(GetDlgItem(Par,Base+n*100+4),PressKey ? SW_SHOW:SW_HIDE);

                CurrentCuts[n].PressChar=CurrentCuts[n+1].PressChar;
                ShowWindow(GetDlgItem(Par,Base+n*100+10),PressChar ? SW_SHOW:SW_HIDE);
                SendMessage(GetDlgItem(Par,Base+n*100+10),CB_SETCURSEL,0,CurrentCuts[n+1].PressChar);

                CurrentCuts[n].MacroFileIdx=CurrentCuts[n+1].MacroFileIdx;
                ShowWindow(GetDlgItem(Par,Base+n*100+11),PlayMacro ? SW_SHOW:SW_HIDE);
                This->SetMacroFileButtonText(GetDlgItem(Par,Base+n*100+11),n);

                for (int c=Base+n*100;c<=(Base+n*100+HIGHEST_CUT_CONTROL_ID);c++){
                  if (GetDlgItem(Par,c)) InvalidateRect(GetDlgItem(Par,c),NULL,0);
                }

                CurrentCuts[n].Down=CurrentCuts[n+1].Down;
              }
              CurrentCuts.NumItems--;
              for (int c=Base+CurrentCuts.NumItems*100;c<=(Base+CurrentCuts.NumItems*100+HIGHEST_CUT_CONTROL_ID);c++){
                if (GetDlgItem(Par,c)) DestroyWindow(GetDlgItem(Par,c));
              }
              This->UpdateAddButsPosition();
              This->Scroller.AutoSize();
              This->UpdateDisableIfDownLists();
            }
            break;
          case 10:
            if (HIWORD(wPar)==CBN_SELENDOK){
              CurrentCuts[Num].PressChar=SendMessage(HWND(lPar),CB_GETCURSEL,0,0);
              CurrentCuts[Num].Down=2;
            }
            break;
          case 11: // Macro choose button
          {
            SendMessage(HWND(lPar),BM_SETCHECK,1,0);

            int StrIdx=CurrentCuts[Num].MacroFileIdx;
            Str CurFile;
            if (StrIdx>=0) CurFile=CurrentCutsStrings[StrIdx].String;

            EasyStr NewFile=This->ShowChooseMacroBox(CurFile);
            SetFocus(HWND(lPar));
            if (NewFile.NotEmpty()){
              if (StrIdx>=0){
                CurrentCutsStrings.SetString(StrIdx,NewFile);
              }else{
                CurrentCuts[Num].MacroFileIdx=CurrentCutsStrings.Add(NewFile);
              }
              This->SetMacroFileButtonText(HWND(lPar),Num);
            }

            SendMessage(HWND(lPar),BM_SETCHECK,0,0);
            break;
          }
        }
      }
      break;
    }
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
void TShortcutBox::ChangeCutFile(Str NewSel,int Type,bool SaveOld)
{
  if (CurrentCutSelType && SaveOld && CurrentCutSel.NotEmpty()){
    DTREE_LOG(EasyStr("DTree: Saving current shortcuts to ")+CurrentCutSel);
    SaveShortcutInfo(CurrentCuts,CurrentCutSel);
  }

  ShowWindow(Scroller.GetControlPage(),SW_HIDE);

  // Delete current cut controls
  DTREE_LOG(EasyStr("DTree: Deleting current cut controls"));
  DynamicArray<HWND> ChildList;
  HWND FirstChild=GetWindow(Scroller.GetControlPage(),GW_CHILD);
  HWND Child=FirstChild;
  while (Child){
    if (GetDlgCtrlID(Child)>=1000) ChildList.Add(Child);
    Child=GetWindow(Child,GW_HWNDNEXT);
    if (Child==FirstChild) break;
  }
  for (int i=0;i<ChildList.NumItems;i++) DestroyWindow(ChildList[i]);
  DTREE_LOG(EasyStr("DTree: Controls deleted"));

  CurrentCutSel=NewSel;
  CurrentCutSelType=Type;

  DTREE_LOG(EasyStr("DTree: Loading all cuts again"));
  LoadAllCuts();

  DTREE_LOG(EasyStr("DTree: Adding cut controls n stuff"));
  for (int i=0;i<CurrentCuts.NumItems;i++) AddPickerLine(i);
  UpdateAddButsPosition();
  SetWindowAndChildrensFont(Scroller.GetControlPage(),Font);
  Scroller.AutoSize();
  DTREE_LOG(EasyStr("DTree: Done controls n stuff"));

  EnableWindow(GetDlgItem(Scroller.GetControlPage(),60),(Type>0));
  EnableWindow(GetDlgItem(Scroller.GetControlPage(),61),(Type>0));

  ShowWindow(Scroller.GetControlPage(),SW_SHOW);
}
//---------------------------------------------------------------------------
int TShortcutBox::DTreeNotifyProc(DirectoryTree *pTree,void *t,int Mess,int i1,int i2)
{
  DTREE_LOG(EasyStr("DTree: Shortcut DTreeNotifyProc Mess=")+Mess);
  TShortcutBox *This=(TShortcutBox*)t;

  if (Mess==DTM_GETTYPE){
    if (i2==1 || i2==2){
      DTREE_LOG(EasyStr("DTree: DTM_GETTYPE, trying to find ")+((char*)i1)+" in list");
      if (CutFiles.FindString_I((char*)i1)>=0){
        DTREE_LOG(EasyStr("DTree: Found, returning 2"));
        return 2;
      }
      DTREE_LOG(EasyStr("DTree: Not found, returning 1"));
      return 1;
    }
    return 0;
  }else if (Mess==DTM_FOLDERMOVED || Mess==DTM_ITEMDELETED){
    Str From=Str((char*)i1).UpperCase();
    for (int i=0;i<CutFiles.NumStrings;i++){
      if (strstr(Str(CutFiles[i].String).UpperCase(),From)){
        if (i2){
          Str NewPath=CutFiles[i].String;
          NewPath.Delete(0,strlen(From));
          NewPath.Insert((char*)i2,0);
          CutFiles.SetString(i,NewPath);
        }else{
          CutFiles.Delete(i--);
        }
      }
    }
    return 0;
  }

  if (Mess!=DTM_SELCHANGED && Mess!=DTM_NAMECHANGED && Mess!=DTM_TYPECHANGED){
    DTREE_LOG(EasyStr("DTree: Finished Shortcut DTreeNotifyProc Mess=")+Mess);
    DTREE_LOG("");
    return 0;
  }

  DTREE_LOG(EasyStr("DTree: Getting item path and type for ")+DWORD(i1));
  Str NewSel=pTree->GetItemPath((HTREEITEM)i1);
  int Type=pTree->GetItem((HTREEITEM)i1,TVIF_IMAGE).iImage;
  DTREE_LOG(EasyStr("DTree: NewSel=")+NewSel+" Type="+Type);
  if (Mess==DTM_SELCHANGED){
    This->ChangeCutFile(NewSel,Type,i2!=0);
  }else if (Mess==DTM_NAMECHANGED){
    DTREE_LOG(EasyStr("DTree: DTM_NAMECHANGED Changed from ")+This->CurrentCutSel);
    if (This->CurrentCutSelType==2){
      for (int i=0;i<CutFiles.NumStrings;i++){
        if (IsSameStr_I(CutFiles[i].String,This->CurrentCutSel)) CutFiles.Delete(i--);
      }
      CutFiles.Add(NewSel);
    }
    This->CurrentCutSel=NewSel;
  }else if (Mess==DTM_TYPECHANGED){
    DTREE_LOG(EasyStr("DTree: DTM_TYPECHANGED removing file from list"));
    for (int i=0;i<CutFiles.NumStrings;i++){
      if (IsSameStr_I(CutFiles[i].String,NewSel)) CutFiles.Delete(i--);
    }
    if (Type==2){
      DTREE_LOG(EasyStr("DTree: DTM_TYPECHANGED adding file to list"));
      CutFiles.Add(NewSel);
    }
    if (IsSameStr_I(NewSel,This->CurrentCutSel)) This->CurrentCutSelType=Type;
    DTREE_LOG(EasyStr("DTree: Loading cuts"));
    This->LoadAllCuts(0);
  }
  DTREE_LOG(EasyStr("DTree: Finished Shortcut DTreeNotifyProc Mess=")+Mess);
  DTREE_LOG("");
  return 0;
}
//---------------------------------------------------------------------------
LRESULT __stdcall TShortcutBox::ChooseMacroWndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  TShortcutBox *This;
  GET_THIS;
  if (pChooseMacroTree){
    if (pChooseMacroTree->ProcessMessage(Mess,wPar,lPar)) return pChooseMacroTree->WndProcRet;
  }

  switch (Mess){
    case WM_COMMAND:
      switch (LOWORD(wPar)){
        case IDCANCEL:
          This->ChooseMacroSel="";
        case IDOK:
          This->PopupOpen=0;
          return 0;
      }
      break;
    case WM_MOVING:case WM_SIZING:
      if (FullScreen){
        RECT *rc=(RECT*)lPar;
        if (rc->top<MENUHEIGHT){
          if (Mess==WM_MOVING) rc->bottom+=MENUHEIGHT-rc->top;
          rc->top=MENUHEIGHT;
          return true;
        }
        RECT LimRC={0,MENUHEIGHT+GetSystemMetrics(SM_CYFRAME),
                    GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN)};
        ClipCursor(&LimRC);
      }
      break;
    case WM_CAPTURECHANGED:   //Finished
      if (FullScreen) ClipCursor(NULL);
      break;
    case WM_ACTIVATE:
      if (wPar==WA_INACTIVE) This->PopupFocus=GetFocus();
      break;
    case WM_SETFOCUS:
      SetFocus(This->PopupFocus);
      break;
    case DM_GETDEFID:
      return MAKELONG(IDOK,DC_HASDEFID);
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
int TShortcutBox::ChooseMacroTreeNotifyProc(DirectoryTree *pTree,void *t,int Mess,int i1,int)
{
  TShortcutBox *This=(TShortcutBox*)t;
  if (Mess==DTM_SELCHANGED || Mess==DTM_NAMECHANGED){
    Str NewSel=pTree->GetItemPath((HTREEITEM)i1);
    if (pTree->GetItem((HTREEITEM)i1,TVIF_IMAGE).iImage==1){
      This->ChooseMacroSel=NewSel;
      EnableWindow(GetDlgItem(GetParent(pTree->hTree),IDOK),TRUE);
    }else{
      This->ChooseMacroSel="";
      EnableWindow(GetDlgItem(GetParent(pTree->hTree),IDOK),FALSE);
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
#undef GET_THIS
//---------------------------------------------------------------------------
#endif

#ifdef UNIX
#include "x/x_shortcutbox.cpp"
#endif

