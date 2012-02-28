/*---------------------------------------------------------------------------
FILE: dir_id.cpp
MODULE: Steem
DESCRIPTION: This file contains the code for the GUI and the implementation
of Steem's DirID system. A DirID is an integer representing a PC input that
can be mapped to perform a function.
---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
bool IsDirIDPressed(int ID,int DeadZonePercent,bool CheckDisable,bool DiagonalPOV)
{
  if (BLANK_DIRID(ID)) return 0;

  if (HIBYTE(ID)==0){        // Key
    BYTE Key=LOBYTE(ID);
#ifdef WIN32
    switch (Key){
      case VK_LSHIFT:case VK_RSHIFT:
      case VK_LCONTROL:case VK_RCONTROL:
      case VK_LMENU:case VK_RMENU:
      {
        MODIFIERSTATESTRUCT mss=GetLRModifierStates();
        if (Key==VK_LSHIFT && mss.LShift) return true;
        if (Key==VK_RSHIFT && mss.RShift) return true;
        if (Key==VK_LCONTROL && mss.LCtrl) return true;
        if (Key==VK_RCONTROL && mss.RCtrl) return true;
        if (Key==VK_LMENU && mss.LAlt) return true;
        if (Key==VK_RMENU && mss.RAlt) return true;
        break;
      }
    }
#endif
    return bool(GetAsyncKeyState(Key) & MSB_W);
  }else if (HIBYTE(ID)>=10){ // Joystick
    int DirID=LOBYTE(ID);
    if (HIBYTE(ID) & 1) DirID=-DirID;
    if (DirID){
      int JoyNum=(HIBYTE(ID)-10)/10;
      if (JoyExists[JoyNum]==0) return 0;

      if (DirID>=200){    //POV
        DirID-=200;
        if (DirID>=8) return 0;
        if (CheckDisable && CutDisablePOV[JoyNum][DirID]) return 0;
        if (JoyInfo[JoyNum].AxisExists[AXIS_POV]==0) return 0;

        int pos=POV_CONV(JoyPos[JoyNum].dwPOV);
        if (pos==0xffff) return 0;
        if (pos<0) pos=7;
        pos%=8;
        if (pos==DirID) return true;
        if (DiagonalPOV==0) return 0;

        int prev=DirID-1,next=(DirID+1) % 8;
        if (prev<0) prev=7;
        prev%=8;
        return (pos==next || pos==prev);
      }else if (DirID>=100){ // Button
        DirID-=100;
        if (DirID>=JoyInfo[JoyNum].NumButtons) return 0;
        if (CheckDisable){
          return ((JoyPos[JoyNum].dwButtons & CutButtonMask[JoyNum]) >> DirID) & 1;
        }else{
          return (JoyPos[JoyNum].dwButtons >> DirID) & 1;
        }
      }else{           // Axis
        if (CheckDisable && CutDisableAxis[JoyNum][DirID+10]) return 0;

        int AxNum=abs(DirID)-1;
        if (JoyInfo[JoyNum].AxisExists[AxNum]==0) return 0;

        UNIX_ONLY( DeadZonePercent=10; )
        int DeadSize=( (JoyInfo[JoyNum].AxisLen[AxNum]/2) * DeadZonePercent )/100;
        if (DirID<0){
          return GetAxisPosition(AxNum,&JoyPos[JoyNum]) < JoyInfo[JoyNum].AxisMid[AxNum]-DeadSize;
        }else{
          return GetAxisPosition(AxNum,&JoyPos[JoyNum]) > JoyInfo[JoyNum].AxisMid[AxNum]+DeadSize;
        }
      }
    }
  }else if (HIBYTE(ID)==2){                     // Mouse
    if (LOBYTE(ID)==0){
      return GetKeyState(VK_MBUTTON)<0;
    }else{
      return (MouseWheelMove>0 && LOBYTE(ID)==1) || (MouseWheelMove<0 && LOBYTE(ID)==2);
    }
  }else if (HIBYTE(ID)==1){                     // Key with extend
  }
  return 0;
}
//---------------------------------------------------------------------------
#ifdef UNIX
void set_KeyboardButtonName(int bum,...){
  int *p=&bum;
  while (*p!=-1){
    KeyboardButtonName[XKeysymToKeycode(XD,(KeySym)(*p))]=(char*)(*(p+1));
    p+=2;
  }
}
#endif

void init_DirID_to_text()
{
  for (int n=0;n<256;n++) KeyboardButtonName[n]="";

  KeyboardButtonName[VK_SHIFT]="Shift";
  KeyboardButtonName[VK_CONTROL]="Ctrl";
  KeyboardButtonName[VK_MENU]="Alt";
#ifdef WIN32
  KeyboardButtonName[VK_F1]="F1";KeyboardButtonName[VK_F2]="F2";
  KeyboardButtonName[VK_F3]="F3";KeyboardButtonName[VK_F4]="F4";
  KeyboardButtonName[VK_F5]="F5";KeyboardButtonName[VK_F6]="F6";
  KeyboardButtonName[VK_F7]="F7";KeyboardButtonName[VK_F8]="F8";
  KeyboardButtonName[VK_F9]="F9";KeyboardButtonName[VK_F10]="F10";
  KeyboardButtonName[VK_F11]="F11";KeyboardButtonName[VK_F12]="F12";
  KeyboardButtonName[VK_F13]="F13";KeyboardButtonName[VK_F14]="F14";
  KeyboardButtonName[VK_F15]="F15";KeyboardButtonName[VK_F16]="F16";
  KeyboardButtonName[VK_F17]="F17";KeyboardButtonName[VK_F18]="F18";
  KeyboardButtonName[VK_F19]="F19";KeyboardButtonName[VK_F20]="F20";
  KeyboardButtonName[VK_F21]="F21";KeyboardButtonName[VK_F22]="F22";
  KeyboardButtonName[VK_F23]="F23";KeyboardButtonName[VK_F24]="F24";

  KeyboardButtonName[VK_LSHIFT]="L-Shift";KeyboardButtonName[VK_RSHIFT]="R-Shift";
  KeyboardButtonName[VK_LCONTROL]="L-Ctrl";KeyboardButtonName[VK_RCONTROL]="R-Ctrl";
  KeyboardButtonName[VK_LMENU]="L-Alt";KeyboardButtonName[VK_RMENU]="R-Alt";
  KeyboardButtonName[VK_BACK]="Bksp";KeyboardButtonName[VK_CLEAR]="Clear";
  KeyboardButtonName[VK_ESCAPE]="Esc";KeyboardButtonName[VK_SPACE]="Space";
  KeyboardButtonName[VK_RETURN]="Ret";KeyboardButtonName[VK_TAB]="Tab";
  KeyboardButtonName[VK_CAPITAL]="Caps";

  KeyboardButtonName[VK_PRIOR]="PgUp";KeyboardButtonName[VK_NEXT]="PgDn";
  KeyboardButtonName[VK_END]="End";KeyboardButtonName[VK_HOME]="Home";
  KeyboardButtonName[VK_LEFT]="Left";KeyboardButtonName[VK_UP]="Up";
  KeyboardButtonName[VK_RIGHT]="Right";KeyboardButtonName[VK_DOWN]="Down";
  KeyboardButtonName[VK_INSERT]="Ins";KeyboardButtonName[VK_DELETE]="Del";

  KeyboardButtonName[VK_NUMPAD0]="Pad 0";KeyboardButtonName[VK_NUMPAD1]="Pad 1";
  KeyboardButtonName[VK_NUMPAD2]="Pad 2";KeyboardButtonName[VK_NUMPAD3]="Pad 3";
  KeyboardButtonName[VK_NUMPAD4]="Pad 4";KeyboardButtonName[VK_NUMPAD5]="Pad 5";
  KeyboardButtonName[VK_NUMPAD6]="Pad 6";KeyboardButtonName[VK_NUMPAD7]="Pad 7";
  KeyboardButtonName[VK_NUMPAD8]="Pad 8";KeyboardButtonName[VK_NUMPAD9]="Pad 9";
  KeyboardButtonName[VK_MULTIPLY]="Pad *";KeyboardButtonName[VK_ADD]="Pad +";
  KeyboardButtonName[VK_SUBTRACT]="Pad -";KeyboardButtonName[VK_DECIMAL]="Pad .";
  KeyboardButtonName[VK_DIVIDE]="Pad /";

#elif defined(UNIX)
  set_KeyboardButtonName(XK_F1,"F1",XK_F2,"F2",XK_F3,"F3",
    XK_F4,"F4",XK_F5,"F5",XK_F6,"F6",XK_F7,"F7",XK_F8,"F8",
    XK_F9,"F9",XK_F10,"F10",XK_F11,"F11",XK_F12,"F12",XK_F13,"F13",
    XK_F14,"F14",XK_F15,"F15",XK_F16,"F16",XK_F17,"F17",XK_F18,"F18",
    XK_F19,"F19",XK_F20,"F20",XK_F21,"F21",XK_F22,"F22",XK_F23,"F23",
    XK_F24,"F24",XK_Home,"Home",XK_Left,"Left",
    XK_Right,"Right",XK_Up,"Up",XK_Down,"Down",
    XK_Page_Up,"PgUp",XK_Page_Down,"PgDn",
    XK_End,"End",XK_Begin,"Begin",XK_Print,"Print",
    XK_Insert,"Ins",XK_Undo,"Undo",
    XK_Redo,"Redo",XK_Menu,"Menu",XK_Find,"Find",
    XK_Cancel,"Cancel",XK_Help,"Help",XK_KP_Space,"Pad Spc",
    XK_KP_Tab,"Pad Tab",XK_KP_Enter,"Enter",XK_KP_F1,"Pad F1",
    XK_KP_F2,"Pad F2",XK_KP_F3,"Pad F3",XK_KP_F4,"Pad F4",
    XK_KP_Home,"Pad Home",XK_KP_Left,"Pad Left",XK_KP_Up,"Pad Up",
    XK_KP_Right,"Pad Right",XK_KP_Down,"Pad Down",XK_KP_Page_Up,"Pad PgUp",
    XK_KP_Page_Down,"Pad PgDn",XK_KP_End,"Pad End",XK_KP_Begin,"Pad Bgn",
    XK_KP_Insert,"Pad Ins",XK_KP_Delete,"Pad Del",XK_KP_Equal,"Pad =",
    XK_KP_Multiply,"Pad *",XK_KP_Add,"Pad +",XK_KP_Separator,"Pad ,",
    XK_KP_Subtract,"Pad -",XK_KP_Decimal,"Pad .",XK_KP_Divide,"Pad /",
    XK_KP_0,"Pad 0",XK_KP_1,"Pad 1",XK_KP_2,"Pad 2",XK_KP_3,"Pad 3",
    XK_KP_4,"Pad 4",XK_KP_5,"Pad 5",XK_KP_6,"Pad 6",XK_KP_7,"Pad 7",
    XK_KP_8,"Pad 8",XK_KP_9,"Pad 9",XK_BackSpace,"Bksp",
    XK_Tab,"Tab",XK_Linefeed,"LF",XK_Clear,"Clear",
    XK_Return,"Return",XK_Sys_Req,"SysReq",XK_Escape,"Esc",
    XK_Delete,"Del",XK_Shift_L,"L-Shift",XK_Shift_R,"R-Shift",
    XK_Control_L,"L-Ctrl",XK_Control_R,"R-Ctrl",
    XK_Caps_Lock,"Caps",XK_Alt_L,"L-Alt",XK_Alt_R,"R-Alt",
    XK_space,"Space",XK_Scroll_Lock,"Scroll",XK_Num_Lock,"Num Lock",
    -1);
#endif
}
//---------------------------------------------------------------------------
EasyStr DirID_to_text(int ID,bool st_key)
{
  if (KeyboardButtonName[0]==NULL) init_DirID_to_text();
  if (BLANK_DIRID(ID)) return "";

  EasyStr t;

  if (HIBYTE(ID)==0){
    BYTE c=LOBYTE(ID);
    char *KeyName=KeyboardButtonName[c];
    if (KeyName[0]==0){
#ifdef WIN32
      WORD Key;
      BYTE Keys[256];
      ZeroMemory(Keys,256);

      if (ToAscii(c,0,Keys,&Key,0)==1){
        t=".";t[0]=(char)toupper(Key);
      }else{
        t=EasyStr("#")+(c);
      }
#elif defined(UNIX)
      WORD key=XKeycodeToKeysym(XD,c,0);
      if ((key & 0xff00)==0){ //ASCII
        t=".";t[0]=(char)toupper(key);
      }else{
        t=EasyStr("#")+(c);
      }

//    KeyboardButtonName[XKeysymToKeycode(XD,(KeySym)(*p))]=
//      (char*)(*(p+1));
#endif
//      KeyName=t.Text;
    }else{
      t=T(KeyName);
    }
    if (st_key){  // ST Keys Only
#ifdef UNIX
			int VK_PAGEUP=XKeysymToKeycode(XD,XK_Page_Up),
					VK_PAGEDOWN=XKeysymToKeycode(XD,XK_Page_Down),
					VK_F11=XKeysymToKeycode(XD,XK_F11),
					VK_F12=XKeysymToKeycode(XD,XK_F12);
#endif
      if (c==VK_PAGEUP)   t=T("Help");
      if (c==VK_PAGEDOWN) t=T("Undo");
      if (c==VK_F11)      t=T("Pad (");
      if (c==VK_F12)      t=T("Pad )");
    }
  }else if (HIBYTE(ID)>=10){
    int DirID=int((HIBYTE(ID) & 1)==0 ? BYTE(ID):-BYTE(ID));
    int JoyNum=(HIBYTE(ID)-10)/10;
    if (DirID){
      if (DirID>=200){
        t=EasyStr("J  ")+T("Hat")+" "+((DirID-200)*45);
      }else if (DirID>=100){
        t=EasyStr("J  ")+T("But")+" "+(DirID-99);
      }else if (DirID<0){
        t="J    -" WIN_ONLY("-");
      }else if (DirID<7){
        t="J    +";
      }else{
        t="J     ";
      }
      t[1]=char('1'+JoyNum);
      if (abs(DirID)<7) t[3]=AxisToName[abs(DirID)-1];
    }
  }else if (HIBYTE(ID)==2){
    if (LOBYTE(ID)==0){
      t="MMB";
    }else{
      t=EasyStr("Wheel ")+LPSTR((LOBYTE(ID)==1) ? "Up":"Down");
    }
  }else if (HIBYTE(ID)==1){
    WIN_ONLY( if (LOBYTE(ID)==VK_RETURN) t=T("Pad Ret"); )
  }
  return t;
}
//---------------------------------------------------------------------------
#ifdef WIN32

void RegisterButtonPicker()
{
  WNDCLASS wc;
  wc.style=CS_DBLCLKS | CS_CLASSDC;
  wc.lpfnWndProc=ButtonPickerWndProc;
  wc.cbClsExtra=0;
  wc.cbWndExtra=2;
  wc.hInstance=Inst;
  wc.hIcon=NULL;
  wc.hCursor=LoadCursor(NULL,IDC_ARROW);
  wc.hbrBackground=NULL;
  wc.lpszMenuName=NULL;
  wc.lpszClassName="Steem Button Picker";
  RegisterClass(&wc);
}
//---------------------------------------------------------------------------
void UnregisterButtonPicker()
{
  UnregisterClass("Steem Button Picker",Inst);
}
//---------------------------------------------------------------------------
LRESULT __stdcall ButtonPickerWndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  static int WaitingForStill;
  switch (Mess){
    case WM_SETFOCUS:
    {
      SetTimer(Win,1,100,NULL);

      if (GetWindowLong(Win,GWL_STYLE) & BPS_INSHORTCUT){
        EasyStr Message;
        if ((GetWindowLong(Win,GWL_STYLE) & 1)==0){
          if (NumJoysticks){
            Message=T("Press any key, the middle mouse button or a joystick button/direction.")+"\r\n\r\n";
          }else{
            Message=T("Press any key or the middle mouse button.")+"\r\n\r\n";
          }
        }else{
          Message=T("Press a key that was on the ST keyboard or F11, F12, Page Up or Page Down.")+"\r\n\r\n";
        }
        Message+=T("Press the pause/break key to clear your selection.");

        SendMessage(TShortcutBox::InfoWin,WM_SETTEXT,0,(LPARAM)Message.Text);
        TShortcutBox::Picking=true;
      }
    }
    case WM_KILLFOCUS:
      for (int n=0;n<MAX_PC_JOYS;n++) JoyOldPos[n].Valid=0;
      SendMessage(Win,WM_TIMER,0,0);
      WaitingForStill=1;
    case WM_ENABLE:
      InvalidateRect(Win,NULL,0);
      if (Mess==WM_KILLFOCUS){
        KillTimer(Win,1);
        if (GetWindowLong(Win,GWL_STYLE) & BPS_INSHORTCUT){
          SendMessage(TShortcutBox::InfoWin,WM_SETTEXT,0,LPARAM(""));
          TShortcutBox::Picking=0;
        }
        SendMessage(GetParent(Win),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(Win),0),LPARAM(Win));
      }
      break;
    case WM_PAINT:
    {
      RECT rc;
      HBRUSH Bk;

      GetClientRect(Win,&rc);

      PAINTSTRUCT ps;
      BeginPaint(Win,&ps);

      if (IsWindowEnabled(Win)==0){
        Bk=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
      }else if (GetFocus()==Win){
        Bk=CreateSolidBrush(GetSysColor(COLOR_WINDOW));
      }else{
        Bk=CreateSolidBrush(MidGUIRGB);
      }

      FillRect(ps.hdc,&rc,Bk);
      DeleteObject(Bk);

      SelectObject(ps.hdc,fnt);
      SetBkMode(ps.hdc,TRANSPARENT);
      SetTextColor(ps.hdc,GetSysColor(COLOR_BTNTEXT));
      CentreTextOut(ps.hdc,0,0,rc.right-1,rc.bottom-1,
                    DirID_to_text(GetWindowWord(Win,0),GetWindowLong(Win,GWL_STYLE) & 1),-1);

      EndPaint(Win,&ps);

      return 0;
    }
    case WM_KEYDOWN:case WM_SYSKEYDOWN:
      if (wPar!=VK_NUMLOCK && wPar!=VK_SCROLL && wPar!=VK_LWIN && wPar!=VK_RWIN){
        if (wPar==VK_PAUSE){
          SetWindowWord(Win,0,0xffff);
        }else if (GetWindowLong(Win,GWL_STYLE) & 1){  // ST Keys Only
          if (wPar==VK_SHIFT){
            MODIFIERSTATESTRUCT mss=GetLRModifierStates();
            if (mss.LShift==0 && mss.RShift) wPar=VK_RSHIFT;
            if (mss.LShift && mss.RShift==0) wPar=VK_LSHIFT;
          }
          wPar&=0xff;
          if (wPar==VK_RETURN){
            if (lPar & 0x1000000) wPar=MAKEWORD(VK_RETURN,1);  // Enter
          }
          if (key_table[BYTE(wPar)]!=0) SetWindowWord(Win,0,(WORD)wPar);
        }else{
          if (wPar==VK_SHIFT || wPar==VK_CONTROL || wPar==VK_MENU){
            MODIFIERSTATESTRUCT mss=GetLRModifierStates();
            switch (wPar){
              case VK_SHIFT:
                if (mss.LShift==0 && mss.RShift) wPar=VK_RSHIFT;
                if (mss.LShift && mss.RShift==0) wPar=VK_LSHIFT;
                break;
              case VK_CONTROL:
                if (mss.LCtrl==0 && mss.RCtrl) wPar=VK_RCONTROL;
                if (mss.LCtrl && mss.RCtrl==0) wPar=VK_LCONTROL;
                break;
              case VK_MENU:
                if (mss.LAlt==0 && mss.RAlt) wPar=VK_RMENU;
                if (mss.LAlt && mss.RAlt==0) wPar=VK_LMENU;
                break;
            }
          }
          wPar&=0xff;
          SetWindowWord(Win,0,WORD(wPar));
        }
        InvalidateRect(Win,NULL,0);
        SendMessage(GetParent(Win),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(Win),0),LPARAM(Win));
        return 0;
      }
      break;
    case WM_TIMER:
      if ((GetWindowLong(Win,GWL_STYLE) & 1)==0){
        for (int j=0;j<MAX_PC_JOYS;j++){
          if (JoyExists[j]){
            JOYINFOEX &ji=JoyPos[j];
            if (JoyOldPos[j].Valid){
              int DirID=0;
              for (int n=0;n<6;n++){
                if (JoyInfo[j].AxisExists[n]){
                  DWORD Pos=GetAxisPosition(n,&ji);
                  if (Pos!=JoyOldPos[j].AxisPos[n]){
                    if (Pos<JoyInfo[j].AxisMid[n]-(JoyInfo[j].AxisLen[n]/4)){
                      DirID=-(n+1);
                    }else if (Pos>JoyInfo[j].AxisMid[n]+(JoyInfo[j].AxisLen[n]/4)){
                      DirID=n+1;
                    }
                  }
                }
              }
              if (ji.dwButtons!=JoyOldPos[j].Buttons){
                for (int n=0;n<JoyInfo[j].NumButtons;n++){
                  if ((((ji.dwButtons ^ JoyOldPos[j].Buttons) >> n) & 1) && (ji.dwButtons >> n)==1){
                    DirID=100+n;
                    break;
                  }
                }
              }
              if (JoyInfo[j].AxisExists[AXIS_POV] && ji.dwPOV<36000){
                if (POV_CONV(ji.dwPOV)!=POV_CONV(OldJoyPos.POV)) DirID=200+min(POV_CONV(ji.dwPOV),8ul);
              }
              if (DirID){  // Changed
                if (WaitingForStill==0 || (DirID>=100 && DirID<200)){
                  SetWindowWord(Win,0,MAKEWORD((BYTE)abs(DirID),BYTE((DirID>0) ? 10:11) + j*10));
                  InvalidateRect(Win,NULL,0);
                  SendMessage(GetParent(Win),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(Win),0),LPARAM(Win));

                  if (DirID<100 || DirID>=200) WaitingForStill=2;
                }
              }else if (WaitingForStill){
                WaitingForStill--;
              }
            }
            JoyOldPos[j].AxisPos[AXIS_X]=ji.dwXpos;
            JoyOldPos[j].AxisPos[AXIS_Y]=ji.dwYpos;
            if (JoyInfo[j].AxisExists[AXIS_Z]) JoyOldPos[j].AxisPos[AXIS_Z]=ji.dwZpos;
            if (JoyInfo[j].AxisExists[AXIS_R]) JoyOldPos[j].AxisPos[AXIS_R]=ji.dwRpos;
            if (JoyInfo[j].AxisExists[AXIS_U]) JoyOldPos[j].AxisPos[AXIS_U]=ji.dwUpos;
            if (JoyInfo[j].AxisExists[AXIS_V]) JoyOldPos[j].AxisPos[AXIS_V]=ji.dwVpos;
            JoyOldPos[j].Buttons=ji.dwButtons;
            if (JoyInfo[j].AxisExists[AXIS_POV]) OldJoyPos.POV=ji.dwPOV;
            JoyOldPos[j].Valid=true;
          }
        }
        return 0;
      }
      break;
    case WM_LBUTTONDOWN:
      SetFocus(Win);
      break;
    case WM_MBUTTONDOWN:case WM_MBUTTONDBLCLK:
      if (GetFocus()==Win && (GetWindowLong(Win,GWL_STYLE) & 1)==0){
        SetWindowWord(Win,0,MAKEWORD(0,2));
        InvalidateRect(Win,NULL,0);
        SendMessage(GetParent(Win),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(Win),0),LPARAM(Win));
      }
      break;
    case WM_MOUSEWHEEL:
      if (GetFocus()==Win && (GetWindowLong(Win,GWL_STYLE) & 1)==0){
        signed short Dir=(signed short)HIWORD(wPar);
        SetWindowWord(Win,0,MAKEWORD(BYTE((Dir>0) ? 1:2),2));
        InvalidateRect(Win,NULL,0);
        SendMessage(GetParent(Win),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(Win),0),LPARAM(Win));
      }
      return 0;
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}

#endif
//---------------------------------------------------------------------------
int ConvertDirID(int OldDirID,int Type)
{
  switch (Type){
    case DIRID_JOY_KEY:
      return MAKEWORD(OldDirID,0);
    case DIRID_JOY_1:case DIRID_JOY_2:
    {
      int JoyNum=Type-DIRID_JOY_1;
      bool Neg=(OldDirID<0);
      return MAKEWORD(abs(OldDirID),10+(JoyNum*10)+Neg);
    }
  }
  return OldDirID;
}
//---------------------------------------------------------------------------

