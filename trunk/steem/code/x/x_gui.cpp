//---------------------------------------------------------------------------
int HandleXError(Display *XD,XErrorEvent *pXErr)
{
  XError=*pXErr;
  log_write("X Error, oh dear, things may be going wrong.");
  char ErrText[300]={0};
  XGetErrorText(XD,XError.type,ErrText,299);
  if (ErrText[0]) log_write(ErrText);
  return 0;
}
//---------------------------------------------------------------------------
void InitColoursAndIcons()
{
  int Scr=XDefaultScreen(XD);
	if (XDefaultDepth(XD,Scr)==8){ // Oh no! 8-bit!
    XVisualInfo vith;
    //I want to set the member called "class".  But gcc
    //doesn't like members with reserved words for names!
    *((&(vith.depth))+1)=PseudoColor;
    int how_many=0;
    XVisualInfo *vi=XGetVisualInfo(XD,VisualClassMask,&vith,&how_many);
    if (how_many){
      colormap=XCreateColormap(XD,XDefaultRootWindow(XD),vi->visual,AllocAll);
      XFree(vi);

      for (int n=0;n<257;n++){
        logpal[n]=0;
        new_pal[n].pixel=n;
        if (n<256){
        	XQueryColor(XD,XDefaultColormap(XD,XDefaultScreen(XD)),new_pal+n);
  	      XStoreColor(XD,colormap,new_pal+n);
  	    }
        new_pal[n].flags=DoRed | DoGreen | DoBlue;
      }
  		
      for(int n=0;n<8;n++){
      	logpal[n]=0xffffffff;
      }
  		XColor c;
  		for (int n=0;n<18;n++){
  			int nn=standard_palette[n][0];
  			int col=standard_palette[n][1];
  			logpal[nn]=0xffffffff;
    	  c.flags=DoRed | DoGreen | DoBlue;
  			c.pixel=nn;
  			c.red=(col & 0xff0000) >> 8;
  			c.green=(col & 0xff00);
  			c.blue=(col & 0xff) << 8;
  	    XStoreColor(XD,colormap,&c);
  	    new_pal[nn]=c;
  		}
  		IconGroup::ColList=(long(*)[2])standard_palette;
  		IconGroup::ColListLen=18;
			hxc::alloc_colours_vector=steem_hxc_alloc_colours;
			hxc::free_colours_vector=steem_hxc_free_colours;
  	}
  	
    WhiteCol=255;
    BlackCol=0;
    BkCol=13;
    BorderLightCol=14;
    BorderDarkCol=254;
	}else{
    WhiteCol=WhitePixel(XD,Scr);
    BlackCol=BlackPixel(XD,Scr);
    BkCol=GetColourValue(XD,192 << 8,192 << 8,192 << 8,WhiteCol);
    BorderLightCol=GetColourValue(XD,224 << 8,224 << 8,224 << 8,WhiteCol);;
    BorderDarkCol=GetColourValue(XD,128 << 8,128 << 8,128 << 8,BlackCol);
  }

  cWinProc=XUniqueContext();
  cWinThis=XUniqueContext();

  Ico16.LoadIconsFromMemory(XD,Get_icon16_bmp(),16);
  Ico32.LoadIconsFromMemory(XD,Get_icon32_bmp(),32);
  Ico64.LoadIconsFromMemory(XD,Get_icon64_bmp(),64);
  IcoTOSFlags.LoadIconsFromMemory(XD,Get_tos_flags_bmp(),RC_FLAG_WIDTH);
  hxc_button::pcheck_ig=&Ico16;
  hxc_button::check_on_icon=ICO16_TICKED;
  hxc_button::check_off_icon=ICO16_UNTICKED;

  fileselect.set_alert_box_icons(&Ico32,&Ico16);
  fileselect.lpig=&Ico16;

  hints.XD=XD;
}

void steem_hxc_alloc_colours(Display*)
{
  hxc::col_black=0;
  hxc::col_white=255;
  hxc::col_grey=13;
  hxc::col_border_dark=254;
  hxc::col_border_light=14;
  hxc::col_sel_back=247;
  hxc::col_sel_fore=255;
  hxc::col_bk=13;
  hxc::colormap=colormap;
}

void steem_hxc_free_colours(Display*){
}


void LoadAllIcons(ConfigStoreFile*,bool)
{
  // Overwrite some of the icons
}
//---------------------------------------------------------------------------
void steem_hxc_modal_notify(bool going)
{
  // Warning: This could be called at any time! As we only need to do anything
  // when running it is safe.
  if (runstate!=RUNSTATE_RUNNING) return;

  if (going){
    Sound_Stop(0);
  }else{
    Sound_Start();
  }
}
//---------------------------------------------------------------------------
bool MakeGUI()
{
  if (XD==NULL) return 0;

  UNIX_get_fake_VKs();
  RunSteemAtom=XInternAtom(XD,"SteemRun",0);
  LoadSnapShotAtom=XInternAtom(XD,"SteemLoadSnapShot",0);
#ifdef ALLOW_XALLOCID
  SteemWindowGroup=XAllocID(XD);
#endif

  XSetWindowAttributes swa;
  swa.backing_store=NotUseful;
  swa.colormap=colormap;
  StemWin=XCreateWindow(XD,
  				XDefaultRootWindow(XD),
                200,
                200,
                2+320+2,
                MENUHEIGHT+2+200+2,
                0,
                CopyFromParent,
                InputOutput,
                CopyFromParent, 
                CWBackingStore | int(colormap ? CWColormap:0),
                &swa);
  if (StemWin==0) return 0;

  hxc::load_res(XD);

  Atom Prots[1]={hxc::XA_WM_DELETE_WINDOW};
  XSetWMProtocols(XD,StemWin,Prots,1);

  DispGC=XCreateGC(XD,StemWin,0,NULL);

  XSetStandardProperties(XD,StemWin,"Steem Engine","Steem",None,_argv,_argc,NULL);

  StemWinIconPixmap=Ico16.CreateIconPixmap(ICO16_STEEM,DispGC);
  StemWinIconMaskPixmap=Ico16.CreateMaskBitmap(ICO16_STEEM);
  SetWindowHints(XD,StemWin,True,NormalState,StemWinIconPixmap,StemWinIconMaskPixmap,SteemWindowGroup,0);

  XSelectInput(XD,StemWin,KeyPressMask | KeyReleaseMask |
                    ButtonPressMask | ButtonReleaseMask |
                    ExposureMask | StructureNotifyMask |
                    VisibilityChangeMask | FocusChangeMask);

  SetProp(StemWin,cWinProc,(DWORD)StemWinProc);

  int x=0;
  RunBut.create(XD,StemWin,x,0,20,20,StemWinButtonNotifyProc,NULL,
                  BT_ICON | BT_UPDOWNNOTIFY,"",101,BkCol);
  RunBut.set_icon(&Ico16,ICO16_RUN);
  hints.add(RunBut.handle,T("Run (Right Click = Slow Motion)"),StemWin);
  x+=23;

  FastBut.create(XD,StemWin,x,0,20,20,StemWinButtonNotifyProc,NULL,
                  BT_ICON | BT_UPDOWNNOTIFY,"",109,BkCol);
  FastBut.set_icon(&Ico16,ICO16_FF);
  hints.add(FastBut.handle,T("Fast Forward (Right Click = Searchlight, Double Click = Sticky)"),StemWin);
  x+=23;

  ResetBut.create(XD,StemWin,x,0,20,20,StemWinButtonNotifyProc,NULL,
                    BT_ICON,"",102,BkCol);
  ResetBut.set_icon(&Ico16,ICO16_RESET);
  hints.add(ResetBut.handle,T("Reset (Left Click = Cold, Right Click = Warm)"),StemWin);
  x+=23;

  SnapShotBut.create(XD,StemWin,x,0,20,20,StemWinButtonNotifyProc,NULL,
                      BT_ICON,"",108,BkCol);
  SnapShotBut.set_icon(&Ico16,ICO16_SNAPSHOTS);
  hints.add(SnapShotBut.handle,T("Memory Snapshot Menu"),StemWin);
  x+=23;

  ScreenShotBut.create(XD,StemWin,x,0,20,20,StemWinButtonNotifyProc,NULL,
                      BT_ICON,"",116,BkCol);
  ScreenShotBut.set_icon(&Ico16,ICO16_TAKESCREENSHOTBUT);
  hints.add(ScreenShotBut.handle,T("Take Screenshot"),StemWin);
  x+=23;

  PasteBut.create(XD,StemWin,x,0,20,20,StemWinButtonNotifyProc,NULL,
                      BT_ICON,"",114,BkCol);
  PasteBut.set_icon(&Ico16,ICO16_PASTE);
  hints.add(PasteBut.handle,T("Paste Text Into ST (Right Click = Options)"),StemWin);
  x+=23;

  if (Disp.CanGoToFullScreen()){
    FullScreenBut.create(XD,StemWin,x,0,20,20,StemWinButtonNotifyProc,NULL,
                        BT_ICON | BT_TOGGLE,"",115,BkCol);
    FullScreenBut.set_icon(&Ico16,ICO16_FULLSCREEN);
    hints.add(FullScreenBut.handle,T("Fullscreen"),StemWin);
  }

  InfBut.create(XD,StemWin,0,0,20,20,StemWinButtonNotifyProc,NULL,
                    BT_ICON,"",105,BkCol);
  InfBut.set_icon(&Ico16,ICO16_GENERALINFO);
  hints.add(InfBut.handle,T("General Info"),StemWin);

  PatBut.create(XD,StemWin,0,0,20,20,StemWinButtonNotifyProc,NULL,
                    BT_ICON,"",113,BkCol);
  PatBut.set_icon(&Ico16,ICO16_PATCHES);
  hints.add(PatBut.handle,T("Patches"),StemWin);

  CutBut.create(XD,StemWin,0,0,20,20,StemWinButtonNotifyProc,NULL,
                    BT_ICON,"",112,BkCol);
  CutBut.set_icon(&Ico16,ICO16_CUT);
  hints.add(CutBut.handle,T("Shortcuts"),StemWin);

  OptBut.create(XD,StemWin,0,0,20,20,StemWinButtonNotifyProc,NULL,
                    BT_ICON,"",107,BkCol);
  OptBut.set_icon(&Ico16,ICO16_OPTIONS);
  hints.add(OptBut.handle,T("Options"),StemWin);

  JoyBut.create(XD,StemWin,0,0,20,20,StemWinButtonNotifyProc,NULL,
                    BT_ICON,"",103,BkCol);
  JoyBut.set_icon(&Ico16,ICO16_JOY);
  hints.add(JoyBut.handle,T("Joystick Configuration"),StemWin);

  DiskBut.create(XD,StemWin,0,0,20,20,StemWinButtonNotifyProc,NULL,
                    BT_ICON,"",100,BkCol);
  DiskBut.set_icon(&Ico16,ICO16_DISKMAN);
  hints.add(DiskBut.handle,T("Disk Manager"),StemWin);


#ifndef CYGWIN
  // Create empty 1 bit per pixel bitmap
  char *Dat=new char[16*16/8];
  ZeroMemory(Dat,16*16/8);
  Pixmap EmptyPix=XCreatePixmapFromBitmapData(XD,StemWin,
                    Dat,16,16,0,0,1);
  XColor ccols[2];
  ccols[0].pixel=0;
  ccols[1].pixel=0;
  EmptyCursor=XCreatePixmapCursor(XD,EmptyPix,EmptyPix,
                    &ccols[0],&ccols[1],8,8);
  XFreePixmap(XD,EmptyPix);
  delete[] Dat;
#else
  EmptyCursor=XCreateFontCursor(XD,XC_cross);
#endif

  return true;
}
//---------------------------------------------------------------------------
void CheckResetIcon()
{
  if (ResetBut.handle==0) return;
  int new_icon=int(OptionBox.NeedReset() ? ICO16_RESETGLOW:ICO16_RESET);
  if (ResetBut.icon_index!=new_icon){
    ResetBut.icon_index=new_icon;
    ResetBut.draw();
  }
}
//---------------------------------------------------------------------------
void CheckResetDisplay(bool)
{
  if (pc==rom_addr && StemWin){
		XWindowAttributes wa;
	  XGetWindowAttributes(XD,StemWin,&wa);
  	XClearArea(XD,StemWin,2,MENUHEIGHT,wa.width-4,
            wa.height-4-MENUHEIGHT,True); // Make StemWin redraw
  }
}
//---------------------------------------------------------------------------
void XGUIUpdatePortDisplay()
{
  if (OptionBox.IsVisible()){
    // Update open buttons
    for (int p=0;p<3;p++) OptionBox.UpdatePortDisplay(p);
  }
}
//---------------------------------------------------------------------------
void PostRunMessage()
{
  if (XD==NULL || StemWin==0 || RunMessagePosted) return;

  XEvent SendEv;
  SendEv.type=ClientMessage;
  SendEv.xclient.window=StemWin;
  SendEv.xclient.message_type=RunSteemAtom;
  SendEv.xclient.format=32;
  XSendEvent(XD,StemWin,0,0,&SendEv);
  RunMessagePosted=true;
}
//---------------------------------------------------------------------------
// NOTE: Everything in this function MUST be checked for existance,
//       it can be called *before* MakeGUI!
void CleanupGUI()
{
  if (XD==NULL) return;

  if (StemWin){
		XAutoRepeatOn(XD);
    hxc::destroy_children_of(StemWin);
    XDestroyWindow(XD,StemWin);StemWin=0;
    hxc::free_res(XD);
  }
  if (DispGC){
    XFreeGC(XD,DispGC);DispGC=0;
  }
  if (EmptyCursor){
    XFreeCursor(XD,EmptyCursor);EmptyCursor=0;
  }
  Ico16.FreeIcons();
  Ico32.FreeIcons();
  Ico64.FreeIcons();
  IcoTOSFlags.FreeIcons();
  if (StemWinIconPixmap){
  	XFreePixmap(XD,StemWinIconPixmap);StemWinIconPixmap=0;
  }
  if (StemWinIconMaskPixmap){
	  XFreePixmap(XD,StemWinIconMaskPixmap);StemWinIconMaskPixmap=0;
	}
  if (colormap){
    XFreeColormap(XD,colormap);
    colormap=0;
  }
  hints.stop();
}
//---------------------------------------------------------------------------
void PrintHelpToStdout()
{
  printf(" \nsteem: run XSteem, the Atari STE emulator for X \n");
  printf("Written by Anthony and Russell Hayward.   \n \n");

  printf("Usage:  steem [options] [disk_image_a [disk_image_b]] [cartridge]\n");
  printf("        steem [options] [state_file]\n \n");

  printf("  disk image: name of disk image (extension ST/MSA/DIM/STT/ZIP/RAR) ");
  printf("for Steem to load.  If 2 disks are specified, the first ");
  printf("will be ST drive A: and the second drive B:.\n \n");

  printf("  cartridge:  name of a cartridge image (.STC) to be loaded.\n \n");

  printf("  state file: previously saved state file (.STS) to load.  If none ");
  printf("is specified, Steem will load \"auto.sts\" provided ");
  printf("the relevant option is checked in the Options dialog.\n \n");

  printf("  tos image:  name of TOS image to use (.IMG or .ROM).\n \n");

  printf("  options:    list of options separated by spaces.  Options are case-");
  printf("independent and can be prefixed by -, --, / or nothing.\n");
  printf("              NOSHM: disable use of Shared Memory.\n");
  printf("              NOSOUND: no sound output.\n");
  printf("              SOF=<n>: set sound output frequency to <n> Hz.\n");
  printf("              PABUFSIZE=<n>: set PortAudio buffer size to <n> samples.\n");
  printf("              FONT=<string>: use a different font.\n");
  printf("              HELP: print this message and quit.\n");
  printf("              INI=<file>: use <file> instead of steem.ini to ");
  printf("initialise options.\n");
  printf("              TRANS=<file>: use <file> instead of searching for ");
  printf("Translate.txt or Translate_*.txt to ");
  printf("translate the GUI text.\n \n");
  
  printf("All of these options (except INI= and TRANS=) can be changed ");
  printf("from the GUI once Steem is running.  It is easiest just to run ");
  printf("Steem and play with the GUI.\n\n");
}
//---------------------------------------------------------------------------
int GetScreenWidth()
{
  static int Wid;
  if (XD){
    return (Wid=XDisplayWidth(XD,XDefaultScreen(XD)));
  }else{
    return Wid;
  }
}
//---------------------------------------------------------------------------
int GetScreenHeight()
{
  static int Height;
  if (XD){
    return (Height=XDisplayHeight(XD,XDefaultScreen(XD)));
  }else{
    return Height;
  }
}
//---------------------------------------------------------------------------
bool GetWindowPositionData(Window Win,WINPOSITIONDATA *wpd)
{
  if (Win==0 || XD==NULL) return 1;

  XWindowAttributes wa;
  XGetWindowAttributes(XD,Win,&wa);
  wpd->Left=wa.x;
  wpd->Top=wa.y;
  wpd->Width=wa.width;
  wpd->Height=wa.height;

  wpd->Maximized=0;
  wpd->Minimized=0;

  return 0;
}
//---------------------------------------------------------------------------
void CentreWindow(Window Win,bool)
{
  if (XD==NULL) return;

  XWindowAttributes wa;
  XGetWindowAttributes(XD,Win,&wa);
  XMoveWindow(XD,Win,(GetScreenWidth()-wa.width)/2,
               (GetScreenHeight()-wa.height)/2);
}
//---------------------------------------------------------------------------
bool SetForegroundWindow(Window Win,Time TimeStamp)
{
  if (XD==NULL || Win==0) return 0;

  XRaiseWindow(XD,Win);
  XSync(XD,False);
  XError.display=NULL;
  XSetInputFocus(XD,Win,RevertToNone,TimeStamp);
  XFlush(XD);
  return (XError.display==NULL);
}
//---------------------------------------------------------------------------
Window GetForegroundWindow()
{
  if (XD==NULL) return 0;

	Window Foc;
	int Revert;
	XGetInputFocus(XD,&Foc,&Revert);
  return Foc;
}
//---------------------------------------------------------------------------
/*
NoEventMask            No events wanted
KeyPressMask           Keyboard down events wanted
KeyReleaseMask         Keyboard up events wanted
ButtonPressMask        Pointer button down events wanted
ButtonReleaseMask      Pointer button up events wanted
EnterWindowMask        Pointer window entry events wanted
LeaveWindowMask        Pointer window leave events wanted
PointerMotionMask      Pointer motion events wanted
PointerMotionHint-     Pointer motion hints wanted
Mask
Button1MotionMask      Pointer motion while button 1 down
Button2MotionMask      Pointer motion while button 2 down
Button3MotionMask      Pointer motion while button 3 down
Button4MotionMask      Pointer motion while button 4 down
Button5MotionMask      Pointer motion while button 5 down
ButtonMotionMask       Pointer motion while any button
                       down
KeymapStateMask        Keyboard state wanted at window
                       entry and focus in
ExposureMask           Any exposure wanted
VisibilityChangeMask   Any change in visibility wanted
StructureNotifyMask    Any change in window structure
                       wanted
ResizeRedirectMask     Redirect resize of this window
SubstructureNotify-    Substructure notification wanted
Mask
SubstructureRedi-      Redirect structure requests on
rectMask               children
FocusChangeMask        Any change in input focus wanted
PropertyChangeMask     Any change in property wanted
ColormapChangeMask     Any change in colormap wanted
OwnerGrabButtonMask    Automatic grabs should activate
                       with owner_events set to True
-------------------------------------------------------------
Event Category           Event Type
-------------------------------------------------------------
Keyboard events          KeyPress, KeyRelease
Pointer events           ButtonPress, ButtonRelease, Motion-
                         Notify

Window crossing events   EnterNotify, LeaveNotify
Input focus events       FocusIn, FocusOut
Keymap state notifica-   KeymapNotify
tion event
Exposure events          Expose, GraphicsExpose, NoExpose
Structure control        CirculateRequest, ConfigureRequest,
events                   MapRequest, ResizeRequest
Window state notifica-   CirculateNotify, ConfigureNotify,
tion events              CreateNotify, DestroyNotify,
                         GravityNotify, MapNotify,
                         MappingNotify, ReparentNotify,
                         UnmapNotify, VisibilityNotify
Colormap state notifi-   ColormapNotify
cation event
Client communication     ClientMessage, PropertyNotify,
events                   SelectionClear, SelectionNotify,
                         SelectionRequest
-------------------------------------------------------------
*/
//---------------------------------------------------------------------------
typedef int WINDOWPROC(void*,Window,XEvent*);
typedef WINDOWPROC* LPWINDOWPROC;

int ProcessEvent(XEvent *Ev)
{
  if (XD==NULL) return PEEKED_NOTHING;

  LPWINDOWPROC WinProc=(LPWINDOWPROC)GetProp(Ev->xany.window,cWinProc);
  if (WinProc) return WinProc((void*)GetProp(Ev->xany.window,cWinThis),Ev->xany.window,Ev);

  return PEEKED_MESSAGE;
}
//---------------------------------------------------------------------------
int ASMCALL PeekEvent()
{
  if (XD==NULL) return PEEKED_NOTHING;

  hxc::check_timers();
  if (XPending(XD)==0) return PEEKED_NOTHING;
  XEvent Ev;
  XNextEvent(XD,&Ev);
  return ProcessEvent(&Ev);
}
//---------------------------------------------------------------------------
void SetStemMouseMode(int NewMM)
{
//  static POINT OldMousePos={-1,0};

//  if (stem_mousemode!=STEM_MOUSEMODE_WINDOW && NewMM==STEM_MOUSEMODE_WINDOW) GetCursorPos(&OldMousePos);
  stem_mousemode=NewMM;

  if (XD==NULL) return;

  if (NewMM==STEM_MOUSEMODE_WINDOW){
#ifdef CYGWIN
    POINT pt;
    GetCursorPos(&pt);
    window_mouse_centre_x=pt.x;
    window_mouse_centre_y=pt.y;
    XGrabPointer(XD,StemWin,0,0,GrabModeAsync,GrabModeAsync,
                  None,EmptyCursor,CurrentTime);
#else
    window_mouse_centre_x=164;
    window_mouse_centre_y=MENUHEIGHT+104;
    XGrabPointer(XD,StemWin,0,0,GrabModeAsync,GrabModeAsync,
                  StemWin,EmptyCursor,CurrentTime);
    XWarpPointer(XD,None,StemWin,0,0,0,0,window_mouse_centre_x,window_mouse_centre_y);
#endif
  }else{
    if (FullScreen && runstate==RUNSTATE_RUNNING) runstate=RUNSTATE_STOPPING;
    XUngrabPointer(XD,CurrentTime);
/*
    if (OldMousePos.x>=0){
      SetCursorPos(OldMousePos.x,OldMousePos.y);
      OldMousePos.x=-1;
    }
*/
  }
  mouse_move_since_last_interrupt_x=0;
  mouse_move_since_last_interrupt_y=0;
  mouse_change_since_last_interrupt=false;
}
//---------------------------------------------------------------------------
void ShowAllDialogs(bool Show)
{
}
//---------------------------------------------------------------------------
void GUIUpdateInternalSpeakerBut()
{
  OptionBox.internal_speaker_but.set_check(false);
}
//---------------------------------------------------------------------------
void HandleKeyPress(UINT KeyCode,bool Up,int Extend)
{
  if (disable_input_vbl_count) return;
  if (ikbd_keys_disabled()) return;  //duration mode

  BYTE STCode=0;

  bool DidShiftSwitching=0;
  int ModifierRestoreArray[3];
  if (EnableShiftSwitching && shift_key_table[0] && (Extend & NO_SHIFT_SWITCH)==0){
    HandleShiftSwitching(KeyCode,Up,STCode,ModifierRestoreArray);
    if (STCode) DidShiftSwitching=true;
  }

  if (STCode==0) STCode=key_table[BYTE(KeyCode)];
  if (STCode){
    ST_Key_Down[STCode]=!Up;
    if (Up) STCode|=MSB_B;
    keyboard_buffer_write_n_record(STCode);

#ifndef DISABLE_STEMDOS
    if (KeyCode==XK_c){ //control-C
      if (ST_Key_Down[key_table[VK_CONTROL]]){ //control-C
        stemdos_control_c();
      }
    }
#endif
  }
  if (DidShiftSwitching) ShiftSwitchRestoreModifiers(ModifierRestoreArray);
}
//---------------------------------------------------------------------------
short GetKeyState(int Key)
{
  if (Key==VK_LBUTTON || Key==VK_RBUTTON || Key==VK_MBUTTON){
    Window InWin,InChild;
    int RootX,RootY,WinX,WinY;
    UINT Mask;
    XQueryPointer(XD,StemWin,&InWin,&InChild,
                  &RootX,&RootY,&WinX,&WinY,&Mask);
    if (Key==VK_LBUTTON) return short((Mask & Button1Mask) ? -1:0);
    if (Key==VK_MBUTTON) return short((Mask & Button2Mask) ? -1:0);
    if (Key==VK_RBUTTON) return short((Mask & Button3Mask) ? -1:0);
  }else if (Key==VK_SHIFT){
    if (KeyState[VK_LSHIFT]<0 || KeyState[VK_RSHIFT]<0) return -1;
  }else if (Key==VK_CONTROL){
    if (KeyState[VK_LCONTROL]<0 || KeyState[VK_RCONTROL]<0) return -1;
  }else if (Key==VK_MENU){
    if (KeyState[VK_LMENU]<0 || KeyState[VK_RMENU]<0) return -1;
  }
  return KeyState[BYTE(Key)];
}
//---------------------------------------------------------------------------
void SetKeyState(int Key,bool Down,bool Toggled)
{
  KeyState[BYTE(Key)]=short((Toggled ? 1:0) | (Down ? 0x8000:0));
}
//---------------------------------------------------------------------------
short GetKeyStateSym(KeySym Sym)
{
  return KeyState[XKeysymToKeycode(XD,Sym)];
}
//---------------------------------------------------------------------------
MODIFIERSTATESTRUCT GetLRModifierStates()
{
  MODIFIERSTATESTRUCT mss;
  mss.LShift=(GetKeyState(VK_LSHIFT)<0);
  mss.RShift=(GetKeyState(VK_RSHIFT)<0);
  mss.LCtrl=(GetKeyState(VK_LCONTROL)<0);
  mss.RCtrl=(GetKeyState(VK_RCONTROL)<0);
  mss.LAlt=(GetKeyState(VK_LMENU)<0);
  mss.RAlt=(GetKeyState(VK_RMENU)<0);
  return mss;
}
//---------------------------------------------------------------------------
int MessageBox(WINDOWTYPE,char *Text,char *Caption,UINT Flags)
{
  int icon_index=-1;
  int mb_ico=(Flags&MB_ICONMASK);
  if(mb_ico==MB_ICONEXCLAMATION){
    icon_index=ICO32_EXCLAM;
  }else if(mb_ico==MB_ICONQUESTION){
    icon_index=ICO32_QUESTION;
  }else if(mb_ico==MB_ICONSTOP){
    icon_index=ICO32_STOP;
  }else if(mb_ico==MB_ICONINFORMATION){
    icon_index=ICO32_INFO;
  }
  if(icon_index==-1){
    alert.set_icons(NULL,0);
  }else{
    alert.set_icons(&Ico32,icon_index,&Ico16,icon_index);
  }
  int default_option=-1;
  switch(Flags&MB_DEFMASK){
  case MB_DEFBUTTON1:default_option=1;break;
  case MB_DEFBUTTON2:default_option=2;break;
  case MB_DEFBUTTON3:default_option=3;break;
  case MB_DEFBUTTON4:default_option=4;break;
  }
  int choice;

  switch (Flags & MB_TYPEMASK){
    case MB_OKCANCEL:
      choice=alert.ask(XD,Text,Caption,T("Okay")+"|"+T("Cancel"),default_option,1);
      if(choice==0)return IDOK;
      else return IDCANCEL;
    case MB_ABORTRETRYIGNORE:
      choice=alert.ask(XD,Text,Caption,T("Abort")+"|"+T("Retry")+"|"+T("Ignore"),default_option,0);
      if(choice==0)return IDABORT;
      else if(choice==1)return IDRETRY;
      else return IDRETRY;
    case MB_YESNOCANCEL:
      choice=alert.ask(XD,Text,Caption,T("Yes")+"|"+T("No")+"|"+T("Cancel"),default_option,2);
      if(choice==0)return IDYES;
      else if(choice==1)return IDNO;
      else return IDCANCEL;
    case MB_YESNO:
      choice=alert.ask(XD,Text,Caption,T("Yes")+"|"+T("No"),default_option,1);
      if(choice==0)return IDYES;
      else return IDNO;
    case MB_RETRYCANCEL:
      choice=alert.ask(XD,Text,Caption,T("Retry")+"|"+T("Cancel"),default_option,1);
      if(choice==0)return IDRETRY;
      else return IDCANCEL;
    default:
      alert.ask(XD,Text,Caption,T("Okay"),default_option,0);
      return IDOK;
  }
}
//---------------------------------------------------------------------------
void GetCursorPos(POINT *pPt)
{
  Window InWin,InChild;
  int RootX,RootY;
  UINT Mask;
  if (XQueryPointer(XD,Window(FullScreen ? Disp.XVM_FullWin:StemWin),&InWin,&InChild,
                    &RootX,&RootY,(int*)&(pPt->x),(int*)&(pPt->y),&Mask)==0){
    pPt->x=window_mouse_centre_x;pPt->y=window_mouse_centre_y;
  }
}
//---------------------------------------------------------------------------
void SetCursorPos(int x,int y)
{
  XWarpPointer(XD,None,Window(FullScreen ? Disp.XVM_FullWin:StemWin),0,0,0,0,x,y);
}
//---------------------------------------------------------------------------
int hyperlink_np(hxc_button *b,int mess,int *pi)
{
  if (mess!=BN_CLICKED) return 0;

  EasyStr Text=b->text;
  char *pipe=strchr(Text,'|');
  if (pipe) Text=pipe+1;
  bool web=IsSameStr_I(Text.Lefts(7),"http://");
  bool ftp=IsSameStr_I(Text.Lefts(6),"ftp://");
  bool email=IsSameStr_I(Text.Lefts(7),"mailto:");
  if (web || ftp || email){
    if (email) Text=Text.Text+7; // strip "mailto:"
    // Shell browser
    Str comline=Comlines[COMLINE_HTTP];
    if (ftp) comline=Comlines[COMLINE_FTP];
    if (email) comline=Comlines[COMLINE_MAILTO];
    shell_execute(comline,Str("[URL]\n")+Text+"\n[ADDRESS]\n"+Text);
  }
  return 0;
}
//---------------------------------------------------------------------------

