//---------------------------------------------------------------------------
bool IsToggled(int j)
{
  if (j==3 || j==5){
    // These joysticks are disabled when a JagPad is in their port
    if (Joy[j-1].Type==JOY_TYPE_JAGPAD) return 0;
  }
  if (Joy[j].ToggleKey<=1) return Joy[j].ToggleKey;

  bool Toggled=bool(GetKeyState(Joy[j].ToggleKey) & 1);
  if (Joy[j].ToggleKey==VK_NUMLOCK) return Toggled==0;

  return Toggled;
}
//---------------------------------------------------------------------------
bool IsJoyActive(int j)
{
  return IsToggled(j);
}
//---------------------------------------------------------------------------
void joy_read_buttons()
{
  if (bAppActive==0){
    // No fire buttons held down
    for (int Port=0;Port<8;Port++) stick[Port] &= BYTE(~BIT_7);
    return;
  }

  static int AutoFireCount[8]={0,0,0,0,0,0,0,0};

  for (int Port=0;Port<8;Port++){
    // Change high bit of stick[Port] according to joystick button state
    bool JoyDown=0;
    if (IsToggled(Port)){
      JoyDown=IsDirIDPressed(Joy[Port].DirID[4],Joy[Port].DeadZone,true);
      if (Joy[Port].AnyFireOnJoy && JoyDown==0){
        int JoyNum=Joy[Port].AnyFireOnJoy-1;
        if ((JoyPos[JoyNum].dwButtons & CutButtonMask[JoyNum] & JoyAnyButtonMask[JoyNum])!=0) JoyDown=true;
      }
      if (Joy[Port].AutoFireSpeed){
        if (IsDirIDPressed(Joy[Port].DirID[5],Joy[Port].DeadZone,true)){
          if (AutoFireCount[Port]<Joy[Port].AutoFireSpeed){
            JoyDown=true;
            if (AutoFireCount[Port]<=0) AutoFireCount[Port]=Joy[Port].AutoFireSpeed*2;
          }else{
            JoyDown=0;
          }
          AutoFireCount[Port]--;
        }else{
          AutoFireCount[Port]=0;
        }
      }
    }
    if (Port<2){
      if (stem_mousemode==STEM_MOUSEMODE_WINDOW){
        if (GetKeyState(int(Port ? VK_RBUTTON:VK_LBUTTON))<0) JoyDown=true;
      }
      if (CutButtonDown[Port]) JoyDown=true;
    }
    if (JoyDown){
      stick[Port] |= BIT_7;
    }else{
      stick[Port] &= BYTE(~BIT_7);
    }
  }
}
//---------------------------------------------------------------------------
// return high bit of stick[Port] or'd with low 4 bits joystick position
BYTE joy_get_pos(int Port)
{
  if (bAppActive==0) return 0;

  int Ret=stick[Port] & BIT_7;

  if (IsToggled(Port)==0) return BYTE(Ret);

  for (int n=0;n<4;n++){
    if (IsDirIDPressed(Joy[Port].DirID[n],Joy[Port].DeadZone,true)) Ret|=(1 << n);
  }
  // Don't allow both up and down or left and right to be pressed at the same time
  if ((Ret & (1 | 2))==(1 | 2)) Ret&=~(1 | 2);
  if ((Ret & (4 | 8))==(4 | 8)) Ret&=~(4 | 8);
  return BYTE(Ret);
}
//---------------------------------------------------------------------------
DWORD GetAxisPosition(int AxNum,JOYINFOEX *ji)
{
  switch (AxNum){
    case 0: return ji->dwXpos;
    case 1: return ji->dwYpos;
    case 2: return ji->dwZpos;
    case 3: return ji->dwRpos;
    case 4: return ji->dwUpos;
    case 5: return ji->dwVpos;
  }
  return 0;
}
//---------------------------------------------------------------------------
void JoyPosReset(int n)
{
  JoyPos[n].dwXpos=JoyInfo[n].AxisMid[0];
  JoyPos[n].dwYpos=JoyInfo[n].AxisMid[1];
  JoyPos[n].dwZpos=JoyInfo[n].AxisMid[2];
  JoyPos[n].dwRpos=JoyInfo[n].AxisMid[3];
  JoyPos[n].dwUpos=JoyInfo[n].AxisMid[4];
  JoyPos[n].dwVpos=JoyInfo[n].AxisMid[5];
  JoyPos[n].dwButtons=0;
  JoyPos[n].dwButtonNumber=0;
  JoyPos[n].dwPOV=0xffffffff;
}
//---------------------------------------------------------------------------
bool joy_is_key_used(BYTE Key)
{
  for (int i=0;i<8;i++){
    if (IsToggled(i)){
      int Last=int(Joy[i].AutoFireSpeed ? 6:5);
      for (int n=0;n<Last;n++){
        if (Joy[i].DirID[n]==Key) return true;
      }
      if (Joy[i].Type==JOY_TYPE_JAGPAD){
        for (int n=0;n<17;n++){
          if (Joy[i].JagDirID[n]==Key) return true;
        }
      }
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
void TJoystickConfig::CreateJoyAnyButtonMasks()
{
  // Create a bitmask so that buttons used for directions/autofire don't count
  // as fire when Any Button On.. is used
  int n;
  for (int j=0;j<MAX_PC_JOYS;j++){
    JoyAnyButtonMask[j]=DWORD(pow(2,JoyInfo[j].NumButtons)-1);
    for (int Port=0;Port<8;Port++){
      for (n=0;n<6;n++){
        if (n==4) n++; //Fire
        if (n==5 && Joy[Port].AutoFireSpeed==0) break;
        int DirID=Joy[Port].DirID[n];
        if (HIBYTE(DirID)==10+(j*10)){
          if (LOBYTE(DirID)>=100 && LOBYTE(DirID)<200){
            JoyAnyButtonMask[j] &= ~(1 << (LOBYTE(DirID)-100));
          }
        }
      }
    }
  }
}
//---------------------------------------------------------------------------
void SetJoyToDefaults(int j,int Defs)
{
  switch (Defs){
    case 0:
      JoySetup[0][j].DirID[0]=MAKEWORD(2,11);  JoySetup[0][j].DirID[1]=MAKEWORD(2,10);
      JoySetup[0][j].DirID[2]=MAKEWORD(1,11);  JoySetup[0][j].DirID[3]=MAKEWORD(1,10);
      JoySetup[0][j].DirID[4]=MAKEWORD(100,10);JoySetup[0][j].DirID[5]=MAKEWORD(101,10);
      break;
    case 1:
      JoySetup[0][j].DirID[0]=MAKEWORD(2,21);  JoySetup[0][j].DirID[1]=MAKEWORD(2,20);
      JoySetup[0][j].DirID[2]=MAKEWORD(1,21);  JoySetup[0][j].DirID[3]=MAKEWORD(1,20);
      JoySetup[0][j].DirID[4]=MAKEWORD(100,20);JoySetup[0][j].DirID[5]=MAKEWORD(101,20);
      break;
    case 2:
      JoySetup[0][j].DirID[0]=VK_UP;           JoySetup[0][j].DirID[1]=VK_DOWN;
      JoySetup[0][j].DirID[2]=VK_LEFT;         JoySetup[0][j].DirID[3]=VK_RIGHT;
#ifdef WIN32
      JoySetup[0][j].DirID[4]=VK_CONTROL; JoySetup[0][j].DirID[5]=VK_RETURN;
#elif defined(UNIX)
      JoySetup[0][j].DirID[4]=XKeysymToKeycode(XD,XK_Control_R);
      JoySetup[0][j].DirID[5]=XKeysymToKeycode(XD,XK_Return);
#endif
      break;
    case 3:
#ifdef WIN32
      JoySetup[0][j].DirID[0]='W';             JoySetup[0][j].DirID[1]='Z';
      JoySetup[0][j].DirID[2]='A';             JoySetup[0][j].DirID[3]='S';
      JoySetup[0][j].DirID[4]=VK_SHIFT;        JoySetup[0][j].DirID[5]=VK_TAB;
#elif defined(UNIX)
      JoySetup[0][j].DirID[0]=XKeysymToKeycode(XD,XK_W);
      JoySetup[0][j].DirID[1]=XKeysymToKeycode(XD,XK_Z);
      JoySetup[0][j].DirID[2]=XKeysymToKeycode(XD,XK_A);
      JoySetup[0][j].DirID[3]=XKeysymToKeycode(XD,XK_S);
      JoySetup[0][j].DirID[4]=VK_LSHIFT;       JoySetup[0][j].DirID[5]=VK_TAB;
#endif
      break;
  }
}
//---------------------------------------------------------------------------
DWORD GetJagPadDown(int n,DWORD Mask)
{
  if ((macro_play_has_joys || macro_record) && Mask<0xffffffff){
    return macro_jagpad[int(n==N_JOY_STE_B_0 ? 1:0)] & Mask;
  }

  if (IsToggled(n)==0) return 0;

  DWORD Ret=0;

  // JagPad only buttons
  for (int b=0;b<17;b++){
    if (Mask & (1 << b)) if (IsDirIDPressed(Joy[n].JagDirID[b],Joy[n].DeadZone,true)) Ret|=(1 << b);
  }

  // Directions
  for (int b=0;b<4;b++){
    if (Mask & (1 << (17+b))) if (IsDirIDPressed(Joy[n].DirID[b],Joy[n].DeadZone,true)) Ret|=(1 << (17+b));
  }

  // Don't allow both up and down or left and right to be pressed at the same time
  #define UD_MASK ((1 << 17) | (1 << 18))
  #define LR_MASK ((1 << 19) | (1 << 20))

  if ((Ret & UD_MASK)==UD_MASK) Ret&=~UD_MASK;
  if ((Ret & LR_MASK)==LR_MASK) Ret&=~LR_MASK;

  #undef UD_MASK
  #undef LR_MASK

  return Ret;
}
//---------------------------------------------------------------------------
DWORD ReadJagPad(int n)
{
  int Offset4=0,Offset2=0;
  DWORD Ret=0;
  if (n==N_JOY_STE_B_0){ Offset4=4;Offset2=2; }

  DWORD DownMask;

  switch (LOBYTE(paddles_ReadMask)){
    case 0xfd:case 0xdf:
      DownMask=GetJagPadDown(n,JAGPAD_BUT_KEY_AST_BIT | JAGPAD_BUT_KEY_7_BIT |
                              JAGPAD_BUT_KEY_4_BIT | JAGPAD_BUT_KEY_1_BIT |
                              JAGPAD_BUT_FIRE_B_BIT);
      if (DownMask & JAGPAD_BUT_KEY_AST_BIT) Ret|=BIT_8 << Offset4;
      if (DownMask & JAGPAD_BUT_KEY_7_BIT) Ret|=BIT_9 << Offset4;
      if (DownMask & JAGPAD_BUT_KEY_4_BIT) Ret|=BIT_10 << Offset4;
      if (DownMask & JAGPAD_BUT_KEY_1_BIT) Ret|=BIT_11 << Offset4;

      if (DownMask & JAGPAD_BUT_FIRE_B_BIT) Ret|=BIT_17 << Offset2;
      break;
    case 0xfb:case 0xbf:
      DownMask=GetJagPadDown(n,JAGPAD_BUT_KEY_0_BIT | JAGPAD_BUT_KEY_8_BIT |
                              JAGPAD_BUT_KEY_5_BIT | JAGPAD_BUT_KEY_2_BIT |
                              JAGPAD_BUT_FIRE_C_BIT);
      if (DownMask & JAGPAD_BUT_KEY_0_BIT) Ret|=BIT_8 << Offset4;
      if (DownMask & JAGPAD_BUT_KEY_8_BIT) Ret|=BIT_9 << Offset4;
      if (DownMask & JAGPAD_BUT_KEY_5_BIT) Ret|=BIT_10 << Offset4;
      if (DownMask & JAGPAD_BUT_KEY_2_BIT) Ret|=BIT_11 << Offset4;

      if (DownMask & JAGPAD_BUT_FIRE_C_BIT) Ret|=BIT_17 << Offset2;
      break;
    case 0xf7:case 0x7f:
      DownMask=GetJagPadDown(n,JAGPAD_BUT_KEY_HASH_BIT | JAGPAD_BUT_KEY_9_BIT |
                              JAGPAD_BUT_KEY_6_BIT | JAGPAD_BUT_KEY_3_BIT |
                              JAGPAD_BUT_KEY_OPTION_BIT);
      if (DownMask & JAGPAD_BUT_KEY_HASH_BIT) Ret|=BIT_8 << Offset4;
      if (DownMask & JAGPAD_BUT_KEY_9_BIT) Ret|=BIT_9 << Offset4;
      if (DownMask & JAGPAD_BUT_KEY_6_BIT) Ret|=BIT_10 << Offset4;
      if (DownMask & JAGPAD_BUT_KEY_3_BIT) Ret|=BIT_11 << Offset4;

      if (DownMask & JAGPAD_BUT_KEY_OPTION_BIT) Ret|=BIT_17 << Offset2;
      break;
    default:
      DownMask=GetJagPadDown(n,JAGPAD_DIR_U_BIT | JAGPAD_DIR_D_BIT |
                              JAGPAD_DIR_L_BIT | JAGPAD_DIR_R_BIT |
                              JAGPAD_BUT_KEY_PAUSE_BIT | JAGPAD_BUT_FIRE_A_BIT);
      if (DownMask & JAGPAD_DIR_U_BIT) Ret|=BIT_8 << Offset4;
      if (DownMask & JAGPAD_DIR_D_BIT) Ret|=BIT_9 << Offset4;
      if (DownMask & JAGPAD_DIR_L_BIT) Ret|=BIT_10 << Offset4;
      if (DownMask & JAGPAD_DIR_R_BIT) Ret|=BIT_11 << Offset4;

      if (DownMask & JAGPAD_BUT_KEY_PAUSE_BIT) Ret|=BIT_16 << Offset2;
      if (DownMask & JAGPAD_BUT_FIRE_A_BIT) Ret|=BIT_17 << Offset2;
      break;
  }
  return Ret;
}
//---------------------------------------------------------------------------
BYTE JoyReadSTEAddress(MEM_ADDRESS addr,bool *pIllegal)
{
  switch (addr){
    case 0xff9200: return 0xff;
    case 0xff9201:
    {
      int Ret=0;

      if (Joy[N_JOY_STE_A_0].Type==JOY_TYPE_JAGPAD){
        Ret|=HIWORD(ReadJagPad(N_JOY_STE_A_0));
      }else{
        Ret|=bool(stick[N_JOY_STE_A_0] & BIT_7);
        Ret|=bool(stick[N_JOY_STE_A_1] & BIT_7)*BIT_1;
      }

      if (Joy[N_JOY_STE_B_0].Type==JOY_TYPE_JAGPAD){
        Ret|=HIWORD(ReadJagPad(N_JOY_STE_B_0));
      }else{
        Ret|=bool(stick[N_JOY_STE_B_0] & BIT_7)*BIT_2;
        Ret|=bool(stick[N_JOY_STE_B_1] & BIT_7)*BIT_3;
      }

      return (BYTE)~Ret;
    }
    case 0xff9202:case 0xff9203:
    {
      int Ret=0;

      if (Joy[N_JOY_STE_A_0].Type==JOY_TYPE_JAGPAD){
        Ret|=ReadJagPad(N_JOY_STE_A_0);
      }else{
        Ret|=((stick[N_JOY_STE_A_0] & b1000) >> 3);
        Ret|=((stick[N_JOY_STE_A_0] & b0100) >> 1);
        Ret|=((stick[N_JOY_STE_A_0] & b0010) << 1);
        Ret|=((stick[N_JOY_STE_A_0] & b0001) << 3);

        Ret|=((stick[N_JOY_STE_A_1] & b1000) << 5);
        Ret|=((stick[N_JOY_STE_A_1] & b0100) << 7);
        Ret|=((stick[N_JOY_STE_A_1] & b0010) << 9);
        Ret|=((stick[N_JOY_STE_A_1] & b0001) << 11);
      }

      if (Joy[N_JOY_STE_B_0].Type==JOY_TYPE_JAGPAD){
        Ret|=ReadJagPad(N_JOY_STE_B_0);
      }else{
        Ret|=((stick[N_JOY_STE_B_0] & b1000) << 1);
        Ret|=((stick[N_JOY_STE_B_0] & b0100) << 3);
        Ret|=((stick[N_JOY_STE_B_0] & b0010) << 5);
        Ret|=((stick[N_JOY_STE_B_0] & b0001) << 7);

        Ret|=((stick[N_JOY_STE_B_1] & b1000) << 9);
        Ret|=((stick[N_JOY_STE_B_1] & b0100) << 11);
        Ret|=((stick[N_JOY_STE_B_1] & b0010) << 13);
        Ret|=((stick[N_JOY_STE_B_1] & b0001) << 15);
      }

//          Ret &= ~paddles_ReadMask;
      Ret=~Ret;

      return BYTE((addr==0xff9202) ? HIBYTE(Ret):LOBYTE(Ret));
    }
    case 0xff9210: return 0xff;
    case 0xff9211: return 6;   // Nonsense
    case 0xff9212: return 0xff;
    case 0xff9213: return 1;   // Nonsense
    case 0xff9214: return 0xff;
    case 0xff9215: return 9;   // Nonsense
    case 0xff9216: return 0xff;
    case 0xff9217: return 15;   // Nonsense
    case 0xff9220: return b11111011; // Don't ask why
    case 0xff9221: return b11111110;
    case 0xff9222: return b11111011; // Don't ask why
    case 0xff9223: return b11111110;
    default: *pIllegal=true;
  }
  return 0xff;
}
//---------------------------------------------------------------------------
#ifdef WIN32

#ifndef NO_DIRECTINPUT
#include "stjoy_directinput.cpp"
#endif

void FreeJoysticks()
{
#ifndef NO_DIRECTINPUT
  DIFreeJoysticks();
#endif

  NumJoysticks=0;
  for (int n=0;n<MAX_PC_JOYS;n++) JoyExists[n]=0;
}

void InitJoysticks(int Method)
{
  FreeJoysticks();

  JoyReadMethod=Method;

  if (Method==PCJOY_READ_DONT) return;

  if (Method==PCJOY_READ_DI){
#ifndef NO_DIRECTINPUT
    DIInitJoysticks();
#endif
  }else if (Method==PCJOY_READ_WINMM){
    JOYCAPS jc;
    for (UINT i=0;i<2;i++){ // 2 is max for MMSystem
      if (joyGetDevCaps(i,&jc,sizeof(JOYCAPS))==JOYERR_NOERROR){
        JoyExists[i]=true;

        JoyInfo[i].NumButtons=jc.wNumButtons;

        JoyInfo[i].AxisExists[AXIS_X]=true;
        JoyInfo[i].AxisExists[AXIS_Y]=true;
        JoyInfo[i].AxisExists[AXIS_Z]=(bool)(jc.wCaps & JOYCAPS_HASZ);
        JoyInfo[i].AxisExists[AXIS_R]=(bool)(jc.wCaps & JOYCAPS_HASR);
        JoyInfo[i].AxisExists[AXIS_U]=(bool)(jc.wCaps & JOYCAPS_HASU);
        JoyInfo[i].AxisExists[AXIS_V]=(bool)(jc.wCaps & JOYCAPS_HASV);
        JoyInfo[i].AxisExists[AXIS_POV]=(bool)(jc.wCaps & JOYCAPS_HASPOV);

        JoyInfo[i].NeedsEx=JoyInfo[i].AxisExists[AXIS_R] || JoyInfo[i].AxisExists[AXIS_U] ||
                            JoyInfo[i].AxisExists[AXIS_V] || JoyInfo[i].AxisExists[AXIS_POV] ||
                             (JoyInfo[i].NumButtons>4);

        JoyInfo[i].ExFlags=JOY_RETURNBUTTONS | JOY_RETURNCENTERED | JOY_RETURNX | JOY_RETURNY;
        if (JoyInfo[i].AxisExists[AXIS_Z]) JoyInfo[i].ExFlags|=JOY_RETURNZ;
        if (JoyInfo[i].AxisExists[AXIS_R]) JoyInfo[i].ExFlags|=JOY_RETURNR;
        if (JoyInfo[i].AxisExists[AXIS_U]) JoyInfo[i].ExFlags|=JOY_RETURNU;
        if (JoyInfo[i].AxisExists[AXIS_V]) JoyInfo[i].ExFlags|=JOY_RETURNV;
        if (JoyInfo[i].AxisExists[AXIS_POV]) JoyInfo[i].ExFlags|=JOY_RETURNPOVCTS;

        JoyInfo[i].AxisMin[AXIS_X]=jc.wXmin;JoyInfo[i].AxisMax[AXIS_X]=jc.wXmax;
        JoyInfo[i].AxisMin[AXIS_Y]=jc.wYmin;JoyInfo[i].AxisMax[AXIS_Y]=jc.wYmax;
        if (JoyInfo[i].AxisExists[AXIS_Z]){
          JoyInfo[i].AxisMin[AXIS_Z]=jc.wZmin;JoyInfo[i].AxisMax[AXIS_Z]=jc.wZmax;
        }
        if (JoyInfo[i].AxisExists[AXIS_R]){
          JoyInfo[i].AxisMin[AXIS_R]=jc.wRmin;JoyInfo[i].AxisMax[AXIS_R]=jc.wRmax;
        }
        if (JoyInfo[i].AxisExists[AXIS_U]){
          JoyInfo[i].AxisMin[AXIS_U]=jc.wUmin;JoyInfo[i].AxisMax[AXIS_U]=jc.wUmax;
        }
        if (JoyInfo[i].AxisExists[AXIS_V]){
          JoyInfo[i].AxisMin[AXIS_V]=jc.wVmin;JoyInfo[i].AxisMax[AXIS_V]=jc.wVmax;
        }
        for (int n=0;n<6;n++){
          if (JoyInfo[i].AxisExists[n]){
            if (JoyInfo[i].AxisMin[n]>JoyInfo[i].AxisMax[n]){
              UINT Temp=JoyInfo[i].AxisMin[n];
              JoyInfo[i].AxisMin[n]=JoyInfo[i].AxisMax[n];
              JoyInfo[i].AxisMax[n]=Temp;
            }
            JoyInfo[i].AxisMid[n]=(JoyInfo[i].AxisMax[n]+JoyInfo[i].AxisMin[n])/2;
            JoyInfo[i].AxisLen[n]=JoyInfo[i].AxisMax[n]-JoyInfo[i].AxisMin[n];
          }
        }
        JoyInfo[i].WaitRead=0;
        JoyInfo[i].WaitReadTime=50; // 50 VBLs

        NumJoysticks++;
      }
    }
  }
  JoyGetPoses();
}
//---------------------------------------------------------------------------
void JoyGetPoses()
{
  static JOYINFO ji;
  bool Fail;
  for (int n=0;n<MAX_PC_JOYS;n++){
    if (JoyExists[n]){
      if (JoyReadMethod==PCJOY_READ_WINMM){
        if (JoyInfo[n].WaitRead){
          JoyInfo[n].WaitRead--;
        }else{
          if (JoyInfo[n].NeedsEx==0){
            Fail=(joyGetPos(n,&ji)!=JOYERR_NOERROR);
            if (Fail==0){
              JoyPos[n].dwXpos=ji.wXpos;
              JoyPos[n].dwYpos=ji.wYpos;
              if (JoyInfo[n].AxisExists[AXIS_Z]) JoyPos[n].dwZpos=ji.wZpos;
              JoyPos[n].dwButtons=ji.wButtons;
            }
          }else{
            JoyPos[n].dwSize=sizeof(JOYINFOEX);
            JoyPos[n].dwFlags=JoyInfo[n].ExFlags;
            Fail=(joyGetPosEx(n,&JoyPos[n])!=JOYERR_NOERROR);
          }
          if (Fail){
            JoyPosReset(n); // Nothing stuck on
            JoyInfo[n].WaitRead=JoyInfo[n].WaitReadTime; // Wait this many VBLs
            // Max 3 minutes between reads
            JoyInfo[n].WaitReadTime=min(JoyInfo[n].WaitReadTime*2,50*60*3);
          }else{
            JoyInfo[n].WaitReadTime=20; // 20 VBLs
          }
        }
      }else if (JoyReadMethod==PCJOY_READ_DI){
        DIJoyGetPos(n);
      }
    }
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------------------------------- CONFIG ----------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define JOYCONFIG_HEIGHT(hot) (10+30+30+(hot)+285+10+28+10+6+GetSystemMetrics(SM_CYCAPTION))

TJoystickConfig::TJoystickConfig()
{
  Left=(GetSystemMetrics(SM_CXSCREEN)-546)/2;
  Top=(GetSystemMetrics(SM_CYSCREEN)-JOYCONFIG_HEIGHT(20))/2;

  FSLeft=(640-546)/2;
  FSTop=(480-JOYCONFIG_HEIGHT(20))/2;

  Section="Joysticks";
}
//---------------------------------------------------------------------------
void TJoystickConfig::ManageWindowClasses(bool Unreg)
{
  WNDCLASS wc;
  char *ClassName[2]={"Steem Joystick Config","Steem Joystick DeadZone"};
  if (Unreg){
    for (int n=0;n<2;n++) UnregisterClass(ClassName[n],Inst);
  }else{
    RegisterMainClass(WndProc,ClassName[0],RC_ICO_JOY);

    wc.style=CS_DBLCLKS;
    wc.hInstance=(HINSTANCE)GetModuleHandle(NULL);
    wc.hIcon=NULL;
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName=NULL;
    wc.lpfnWndProc=TJoystickConfig::DeadZoneWndProc;
    wc.cbClsExtra=0;
    wc.cbWndExtra=4;
    wc.lpszClassName=ClassName[1];
    RegisterClass(&wc);
  }
}
//---------------------------------------------------------------------------
bool TJoystickConfig::HasHandledMessage(MSG *mess)
{
  if (Handle!=NULL){
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
void TJoystickConfig::Show()
{
  if (Handle!=NULL){
    ShowWindow(Handle,SW_SHOWNORMAL);
    SetForegroundWindow(Handle);
    return;
  }
  if (FullScreen) Top=max(Top,MENUHEIGHT);

  ManageWindowClasses(SD_REGISTER);
  Handle=CreateWindowEx(WS_EX_CONTROLPARENT | WS_EX_APPWINDOW,"Steem Joystick Config",T("Joysticks"),
                          WS_CAPTION | WS_SYSMENU,
                          Left,Top,0,0,ParentWin,NULL,HInstance,NULL);
  if (HandleIsInvalid()){
    ManageWindowClasses(SD_UNREGISTER);
    return;
  }

  SetWindowLong(Handle,GWL_USERDATA,(long)this);

  MakeParent(HWND(FullScreen ? StemWin:NULL));

  HWND Win;
  int w;

  w=get_text_width(T("Read PC joystick(s) using"));
  CreateWindow("Static",T("Read PC joystick(s) using"),WS_CHILD | WS_VISIBLE,
                          10,14,w,23,Handle,(HMENU)90,HInstance,NULL);
  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | WS_VISIBLE |
                      CBS_HASSTRINGS | CBS_DROPDOWNLIST,
                      15+w,10,530-(15+w),200,Handle,(HMENU)91,HInstance,NULL);
  CBAddString(Win,T("Nothing (PC Joysticks Off)"),PCJOY_READ_DONT);
  CBAddString(Win,T("Windows Multimedia"),PCJOY_READ_WINMM);
#ifndef NO_DIRECTINPUT
  CBAddString(Win,T("DirectInput"),PCJOY_READ_DI);
#endif
  CBSelectItemWithData(Win,JoyReadMethod);
  
  w=get_text_width(T("Current configuration"));
  CreateWindow("Static",T("Current configuration"),WS_CHILD | WS_VISIBLE,
                          10,44,w,23,Handle,(HMENU)1100,HInstance,NULL);
  Win=CreateWindow("Combobox","",WS_CHILD | WS_TABSTOP | WS_VISIBLE |
                      CBS_HASSTRINGS | CBS_DROPDOWNLIST,
                      15+w,40,530-(15+w),200,Handle,(HMENU)1101,HInstance,NULL);
  for (int n=0;n<3;n++) SendMessage(Win,CB_ADDSTRING,0,LPARAM((T("Joystick Setup")+" #"+(n+1)).Text));
  SendMessage(Win,CB_SETCURSEL,nJoySetup,0);

  HWND Tabs=CreateWindow(WC_TABCONTROL,"",WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_FOCUSONBUTTONDOWN,
                          10,70,520,310,Handle,(HMENU)99,HInstance,NULL);

  TC_ITEM tci;
  tci.mask=TCIF_TEXT;

  tci.pszText=StaticT("Standard Ports");
  SendMessage(Tabs,TCM_INSERTITEM,0,(LPARAM)&tci);

  tci.pszText=StaticT("STE Port A");
  SendMessage(Tabs,TCM_INSERTITEM,1,(LPARAM)&tci);

  tci.pszText=StaticT("STE Port B");
  SendMessage(Tabs,TCM_INSERTITEM,2,(LPARAM)&tci);

  tci.pszText=StaticT("Parallel Ports");
  SendMessage(Tabs,TCM_INSERTITEM,3,(LPARAM)&tci);

  SendMessage(Tabs,TCM_SETCURSEL,BasePort/2,0);

  RECT rc;
  GetTabControlPageSize(GetDlgItem(Handle,99),&rc);

  {
    // rc.top=70+height of tab buttons
    int HeightOfTabButs=(rc.top-70);
    SetWindowPos(Tabs,0,0,0,520,HeightOfTabButs+285,SWP_NOZORDER | SWP_NOMOVE);
    SetWindowPos(Handle,0,0,0,546,JOYCONFIG_HEIGHT(HeightOfTabButs),SWP_NOZORDER | SWP_NOMOVE);
  }

  int x=20,y,FireY;
  for (int p=0;p<2;p++){
    y=rc.top;
    Group[p]=CreateWindow("Button","",WS_CHILD | WS_VISIBLE | BS_GROUPBOX | WS_CLIPCHILDREN,
                  x,y,245,275,Handle,(HMENU)(100+p*100),HInstance,NULL);

    if (p==0){
      SetWindowLong(Group[0],GWL_USERDATA,(DWORD)this);
      OldGroupBoxWndProc=(WINDOWPROC)SetWindowLong(Group[0],GWL_WNDPROC,(DWORD)GroupBoxWndProc);

      w=GetCheckBoxSize(Font,"JagPad").Width;
      JagBut=CreateWindow("Button","JagPad",WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                       235-w,0,w,18,Group[0],(HMENU)98,HInstance,NULL);
      SendMessage(JagBut,WM_SETFONT,(WPARAM)Font,0);
    }

    y+=20;

    w=get_text_width(T("Active"));
    CreateWindow("Static",T("Active"),WS_CHILD | WS_VISIBLE,
                          x+10,y+4,w,23,Handle,(HMENU)(101+p*100),HInstance,NULL);
    Win=CreateWindow("Combobox","",WS_CHILD | WS_VISIBLE | WS_TABSTOP |
                      CBS_HASSTRINGS | CBS_DROPDOWNLIST,
                      x+15+w,y,225-(5+w),200,
                      Handle,(HMENU)(102+p*100),HInstance,NULL);
    SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Never"));
    SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Always"));
    SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("When Scroll Lock On"));
    SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("When Num Lock Off"));
    switch (Joy[BasePort+p].ToggleKey){
      case VK_SCROLL:
        SendMessage(Win,CB_SETCURSEL,2,0);
        break;
      case VK_NUMLOCK:
        SendMessage(Win,CB_SETCURSEL,3,0);
        break;
      default:
        SendMessage(Win,CB_SETCURSEL,Joy[BasePort+p].ToggleKey,0);
    }
    y+=30;

    // Left Right Up Down
    Win=CreateWindowEx(512,"Steem Button Picker","",
                      WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | WS_DISABLED,
                      x+90,y,65,23,Handle,(HMENU)(110+p*100),HInstance,NULL);
    SetWindowWord(Win,0,(WORD)Joy[BasePort+p].DirID[0]);
    y+=28;

    // DeadZone
    Win=CreateWindowEx(512,"Steem Joystick DeadZone","",
                      WS_CHILDWINDOW | WS_VISIBLE | WS_DISABLED,
                      x+88,y,70,70,Handle,(HMENU)(120+p*100),HInstance,NULL);
    SetWindowLong(Win,GWL_USERDATA,(long)this);
    y+=24;

    Win=CreateWindowEx(512,"Steem Button Picker","",
                      WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_DISABLED,
                      x+10,y,65,23,Handle,(HMENU)(112+p*100),HInstance,NULL);
    SetWindowWord(Win,0,(WORD)Joy[BasePort+p].DirID[2]);

    Win=CreateWindowEx(512,"Steem Button Picker","",
                      WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | WS_DISABLED,
                      x+170,y,65,23,Handle,(HMENU)(113+p*100),HInstance,NULL);
    SetWindowWord(Win,0,(WORD)Joy[BasePort+p].DirID[3]);
    y+=23+23+5;

    Win=CreateWindowEx(512,"Steem Button Picker","",
                      WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | WS_DISABLED,
                      x+90,y,65,23,Handle,(HMENU)(111+p*100),HInstance,NULL);
    SetWindowWord(Win,0,(WORD)Joy[BasePort+p].DirID[1]);
    y+=30;

    FireY=y;
    w=get_text_width(T("Fire button"));
    CreateWindow("Static",T("Fire button"),WS_CHILD | WS_VISIBLE,
                            x+10,y+4,w,23,Handle,(HMENU)(150+p*100),HInstance,NULL);

    Win=CreateWindowEx(512,"Steem Button Picker","",
                      WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | WS_DISABLED,
                      x+w+15,y,65,23,Handle,(HMENU)(114+p*100),HInstance,NULL);
    SetWindowWord(Win,0,(WORD)Joy[p].DirID[4]);
    y+=30;

    EasyStr Text=T("Or any button on");
    CreateWindow("Static",Text,WS_CHILD | WS_VISIBLE | int(NumJoysticks==0 ? WS_DISABLED:0),
                  x+10,y+4,GetTextSize(Font,Text).Width,23,Handle,(HMENU)(116+p*100),HInstance,NULL);

    w=GetTextSize(Font,Text).Width;
    HWND Win=CreateWindow("Combobox","",WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | int(NumJoysticks==0 ? WS_DISABLED:0),
                            x+w+15,y,235-(w+15),200,Handle,(HMENU)(118+p*100),HInstance,NULL);
    SendMessage(Win,CB_ADDSTRING,0,(long)"-");
    for (int n=0;n<MAX_PC_JOYS;n++) if (JoyExists[n]) SendMessage(Win,CB_ADDSTRING,0,(long)((T("Joystick")+" "+(n+1)).Text));
    y+=30;

    w=get_text_width(T("Autofire"));
    CreateWindow("Static",T("Autofire"),WS_CHILD | WS_VISIBLE,
                            x+10,y+4,w,23,Handle,(HMENU)(151+p*100),HInstance,NULL);

    Win=CreateWindow("Combobox","",WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
                            x+15+w,y,80,200,Handle,(HMENU)(117+p*100),HInstance,NULL);
    SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Off"));
    SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("V.Fast"));
    SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Fast"));
    SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Medium"));
    SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Slow"));
    SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("V.Slow"));

    Win=CreateWindowEx(512,"Steem Button Picker","",
                      WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | WS_DISABLED,
                      x+w+100,y,65,23,Handle,(HMENU)(115+p*100),HInstance,NULL);
    SetWindowWord(Win,0,(WORD)Joy[p].DirID[5]);
    x+=255;
  }

  // Jaguar buttons/keys
  x=30;
  w=get_text_width(T("Fire buttons"));
  CreateWindow("Static",T("Fire buttons"),WS_CHILD | SS_CENTER,
                          x,FireY,225,23,Handle,(HMENU)180,HInstance,NULL);
  for (int n=0;n<3;n++){
    EasyStr But=char('A'+n);
    w=get_text_width(But);
    CreateWindow("Static",But,WS_CHILD,x,FireY+29,w,23,Handle,(HMENU)(181+n),HInstance,NULL);
    x+=w+2;
    CreateWindowEx(512,"Steem Button Picker","",WS_CHILD | WS_TABSTOP,
                        x,FireY+25,62,23,Handle,(HMENU)(160+n),HInstance,NULL);
    x+=65;
  }
  y=rc.top+25;

  EasyStr Option=T("Option"),Pause=T("Pause");
  char *JagButName[14]={Option,Pause,"0","1","2","3","4","5","6","7","8","9","#","*"};
  w=0;
  for (int n=0;n<14;n++) w=max(w,get_text_width(JagButName[n]));
  for (int n=0;n<14;n++){
    x=285+(115*(n & 1));
    CreateWindow("Static",JagButName[n],WS_CHILD | SS_CENTER,
                            x,y+4,w,23,Handle,(HMENU)(184+n),HInstance,NULL);
    x+=w+2;
    CreateWindowEx(512,"Steem Button Picker","",WS_CHILD | WS_TABSTOP,
                        x,y,65,23,Handle,(HMENU)(163+n),HInstance,NULL);
    if (n & 1) y+=30;
  }

  y=rc.top+285+10;
  int Wid=GetTextSize(Font,T("Mouse speed")+": "+T("Min")).Width;
  CreateWindow("Static",T("Mouse speed")+": "+T("Min"),WS_VISIBLE | WS_CHILD,
                  10,y+4,Wid,23,Handle,(HMENU)995,HInstance,NULL);

  int Wid2=GetTextSize(Font,T("Max")).Width;
  CreateWindow("Static",T("Max"),WS_VISIBLE | WS_CHILD,
                  530-Wid2,y+4,Wid2,23,Handle,(HMENU)998,HInstance,NULL);

  Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBS_HORZ,
                    15+Wid,y,530-Wid2-5-(15+Wid),27,Handle,(HMENU)1000,HInstance,NULL);
  SendMessage(Win,TBM_SETRANGE,0,MAKELPARAM(1,19));
  SendMessage(Win,TBM_SETPOS,1,mouse_speed);
  SendMessage(Win,TBM_SETLINESIZE,0,1);
  SendMessage(Win,TBM_SETPAGESIZE,0,1);
  SendMessage(Win,TBM_SETTIC,0,10);

  for (int p=0;p<2;p++) JoyModeChange(BasePort+p,p*100+100);

  SetWindowAndChildrensFont(Handle,Font);

  Focus=GetDlgItem(Handle,99);

  ShowWindow(Handle,SW_SHOW);
  SetFocus(Focus);

  if (StemWin!=NULL) PostMessage(StemWin,WM_USER,1234,0);
}
//---------------------------------------------------------------------------
void TJoystickConfig::Hide()
{
  if (Handle==NULL) return;

  ShowWindow(Handle,SW_HIDE);
  if (FullScreen) SetFocus(StemWin);

  DestroyWindow(Handle);Handle=NULL;

  if (StemWin) PostMessage(StemWin,WM_USER,1234,0);

  ManageWindowClasses(SD_UNREGISTER);
}
//---------------------------------------------------------------------------
void TJoystickConfig::FillJoyTypeCombo()
{
  if (BasePort==2 || BasePort==4){
    SendMessage(JagBut,BM_SETCHECK,(Joy[BasePort].Type==JOY_TYPE_JAGPAD) ? BST_CHECKED:BST_UNCHECKED,0);
    EnableWindow(JagBut,true);
  }else{
    SendMessage(JagBut,BM_SETCHECK,BST_UNCHECKED,0);
    EnableWindow(JagBut,0);
  }
}
//---------------------------------------------------------------------------
void TJoystickConfig::CheckJoyType()
{
  bool Port0Hidden=bool(GetWindowLong(GetDlgItem(Handle,95),GWL_STYLE) & WS_VISIBLE);
  int Port0ShowType=SW_SHOW;
  bool PortOAlter=Port0Hidden;

  bool Port1Jagpad=bool(GetWindowLong(GetDlgItem(Handle,170),GWL_STYLE) & WS_VISIBLE);
  int JagShowType=SW_HIDE,NormShowType=SW_SHOW;
  bool Port1Alter=Port1Jagpad;

  if (BasePort==2 || BasePort==4){
    if (Joy[BasePort].Type==JOY_TYPE_JAGPAD){
      if (Port1Jagpad){
        Port1Alter=0;
      }else{
        Port1Alter=true;
        JagShowType=SW_SHOW;
        NormShowType=SW_HIDE;
      }
    }
  }
  if (Port1Alter){
    ShowWindow(GetDlgItem(Handle,95),SW_HIDE);

    int DoJag=(JagShowType==SW_HIDE);
    for (int n=0;n<2;n++){
      if (DoJag){
        for (int n=180;n<200;n++) if (GetDlgItem(Handle,n)) ShowWindow(GetDlgItem(Handle,n),JagShowType);
        for (int n=160;n<180;n++) if (GetDlgItem(Handle,n)) ShowWindow(GetDlgItem(Handle,n),JagShowType);
      }else{
        for (int n=114;n<=118;n++) ShowWindow(GetDlgItem(Handle,n),NormShowType);

        for (int n=150;n<=151;n++) ShowWindow(GetDlgItem(Handle,n),NormShowType);

        for (int n=201;n<=202;n++) ShowWindow(GetDlgItem(Handle,n),NormShowType);

        for (int n=210;n<=218;n++) ShowWindow(GetDlgItem(Handle,n),NormShowType);
        ShowWindow(GetDlgItem(Handle,220),NormShowType);

        for (int n=250;n<=251;n++) ShowWindow(GetDlgItem(Handle,n),NormShowType);
      }

      DoJag=!DoJag;
    }
  }
  if (PortOAlter){
    if (Port0ShowType==SW_SHOW) ShowWindow(GetDlgItem(Handle,95),SW_HIDE);

    for (int n=101;n<=102;n++) ShowWindow(GetDlgItem(Handle,n),Port0ShowType);

    for (int n=110;n<=113;n++) ShowWindow(GetDlgItem(Handle,n),Port0ShowType);
    if (Port1Alter==0 || Port0ShowType==SW_HIDE){
      for (int n=114;n<=118;n++) ShowWindow(GetDlgItem(Handle,n),Port0ShowType);
      for (int n=150;n<=151;n++) ShowWindow(GetDlgItem(Handle,n),Port0ShowType);
    }
    ShowWindow(GetDlgItem(Handle,120),Port0ShowType);


    if (Port0ShowType==SW_HIDE) ShowWindow(GetDlgItem(Handle,95),SW_SHOW);
  }

  EasyStr PortName[2];
  if (BasePort==0){
    PortName[0]=T("Port 0 (mouse)");
    PortName[1]=T("Port 1");
  }else if (BasePort==2 || BasePort==4){
    PortName[0]=LPSTR(Joy[BasePort].Type ? T("Pad"):T("Stick 0"));
    PortName[1]=LPSTR(Joy[BasePort].Type ? T("Pad Keyboard"):T("Stick 1"));
  }else{
    PortName[0]=T("Parallel 0");
    PortName[1]=T("Parallel 1");
  }
  SendMessage(Group[0],WM_SETTEXT,0,LPARAM(PortName[0].Text));
  SendMessage(Group[1],WM_SETTEXT,0,LPARAM(PortName[1].Text));
}
//---------------------------------------------------------------------------
void TJoystickConfig::JoyModeChange(int Port,int base)
{
  int NewMode=Joy[Port].ToggleKey>=1;

  switch (Joy[Port].ToggleKey){
    case 0:case 1:
      SendDlgItemMessage(Handle,base+2,CB_SETCURSEL,Joy[Port].ToggleKey,0);
      break;
    case VK_SCROLL:
      SendDlgItemMessage(Handle,base+2,CB_SETCURSEL,2,0);
      break;
    case VK_NUMLOCK:
      SendDlgItemMessage(Handle,base+2,CB_SETCURSEL,3,0);
      break;
  }

  for (int n=base+10;n<=base+15;n++){
    EnableWindow(GetDlgItem(Handle,n),NewMode);
    InvalidateRect(GetDlgItem(Handle,n),NULL,0);
    SetWindowWord(GetDlgItem(Handle,n),0,(WORD)(NewMode ? Joy[Port].DirID[n-(base+10)]:0));
  }

  EnableWindow(GetDlgItem(Handle,base+17),NewMode);
  SendMessage(GetDlgItem(Handle,base+17),CB_SETCURSEL,WPARAM(NewMode ? Joy[Port].AutoFireSpeed:0),0);
  if (NumJoysticks) EnableWindow(GetDlgItem(Handle,base+18),NewMode);
  SendMessage(GetDlgItem(Handle,base+18),CB_SETCURSEL,WPARAM(NewMode ? Joy[Port].AnyFireOnJoy:0),0);

  if (Port==2 || Port==4){
    for (int n=160;n<180;n++){
      if (GetDlgItem(Handle,n)){
        SetWindowWord(GetDlgItem(Handle,n),0,(WORD)(NewMode ? Joy[Port].JagDirID[n-160]:0));
        EnableWindow(GetDlgItem(Handle,n),NewMode);
        InvalidateRect(GetDlgItem(Handle,n),NULL,0);
      }
    }
  }

  FillJoyTypeCombo();
  CheckJoyType();

  EnableWindow(GetDlgItem(Handle,base+20),NewMode);
  InvalidateRect(GetDlgItem(Handle,base+20),NULL,0);

  CreateJoyAnyButtonMasks();

  if (Port==N_JOY_PARALLEL_0 || Port==N_JOY_PARALLEL_1){
    OptionBox.UpdateParallel();
  }
}
//---------------------------------------------------------------------------
#define GET_THIS This=(TJoystickConfig*)GetWindowLong(Win,GWL_USERDATA);

LRESULT __stdcall TJoystickConfig::WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  LRESULT Ret=DefStemDialogProc(Win,Mess,wPar,lPar);
  if (StemDialog_RetDefVal) return Ret;

  TJoystickConfig *This;
  switch (Mess){
    case WM_COMMAND:
      GET_THIS;
      switch LOWORD(wPar){
        case 91:
          if (HIWORD(wPar)==CBN_SELENDOK){
            InitJoysticks(CBGetSelectedItemData((HWND)lPar));
          }
          break;
        case 1101:
          if (HIWORD(wPar)==CBN_SELENDOK){
            for (int n=0;n<8;n++) JoySetup[nJoySetup][n]=Joy[n];
            nJoySetup=SendMessage(HWND(lPar),CB_GETCURSEL,0,0);
            for (int n=0;n<8;n++) Joy[n]=JoySetup[nJoySetup][n];
            This->JoyModeChange(BasePort,100);
            This->JoyModeChange(BasePort+1,200);
          }
          break;
        case 102:case 202:
          if (HIWORD(wPar)==CBN_SELENDOK){
            int Idx=SendMessage((HWND)lPar,CB_GETCURSEL,0,0);
            int Port=BasePort+(LOWORD(wPar)/100 - 1);
            switch (Idx){
              case 0:
                Joy[Port].ToggleKey=0;
                break;
              case 1:
                Joy[Port].ToggleKey=1;
                break;
              case 2:
                Joy[Port].ToggleKey=VK_SCROLL;
                break;
              case 3:
                Joy[Port].ToggleKey=VK_NUMLOCK;
                break;
            }
            This->JoyModeChange(Port,LOWORD(wPar)-2);
          }
          break;
        case 118:case 218:
        {
          int Port=BasePort+(LOWORD(wPar)/100 - 1);
          Joy[Port].AnyFireOnJoy=SendMessage((HWND)lPar,CB_GETCURSEL,0,0);
          break;
        }
        case 117:case 217:
        {
          int Port=BasePort+(LOWORD(wPar)/100 - 1);
          Joy[Port].AutoFireSpeed=SendMessage((HWND)lPar,CB_GETCURSEL,0,0);
          CreateJoyAnyButtonMasks();
          break;
        }
        case 110:case 111:case 112:case 113:case 114:case 115:
        case 210:case 211:case 212:case 213:case 214:case 215:
        {
          int Port=BasePort+(LOWORD(wPar)/100 - 1);
          Joy[Port].DirID[(LOWORD(wPar) % 100)-10]=GetWindowWord(HWND(lPar),0);
          CreateJoyAnyButtonMasks();
          break;
        }
      }
      if (LOWORD(wPar)>=160 && LOWORD(wPar)<177){ //Jagpad
        int Port=BasePort+(LOWORD(wPar)/100 - 1);
        Joy[Port].JagDirID[(LOWORD(wPar) % 100)-60]=GetWindowWord(HWND(lPar),0);
        CreateJoyAnyButtonMasks();
      }
      break;
    case WM_NOTIFY:
    {
      NMHDR *hdr=(NMHDR*)lPar;
      if (hdr->idFrom==99){
        GET_THIS;
        if (hdr->code==TCN_SELCHANGE){
          int NewBase=SendMessage(GetDlgItem(This->Handle,99),TCM_GETCURSEL,0,0)*2;
          if (BasePort!=NewBase){
            BasePort=NewBase;
            This->JoyModeChange(BasePort,100);
            This->JoyModeChange(BasePort+1,200);
          }
        }
      }
      break;
    }
    case WM_HSCROLL:
    {
      if (HWND(lPar)==GetDlgItem(Win,1000)){
        mouse_speed=SendMessage(HWND(lPar),TBM_GETPOS,0,0);
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
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
LRESULT __stdcall TJoystickConfig::DeadZoneWndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  bool DoPaint=0;
  switch (Mess){
    case WM_CREATE:
    {
      SetTimer(Win,1,60,NULL);

      int *OldDirPos=new int[4];
      ZeroMemory(OldDirPos,sizeof(OldDirPos));
      SetWindowWord(Win,0,LOWORD(OldDirPos));
      SetWindowWord(Win,2,HIWORD(OldDirPos));

      break;
    }
    case WM_PAINT:case WM_ENABLE:
      DoPaint=true;
      break;
    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:
      if (wPar & MK_LBUTTON){
        RECT rc;
        GetClientRect(Win,&rc);
        int Port=BasePort+(GetDlgCtrlID(Win)/100 - 1);
        double NewDZ=max(abs(LOWORD(lPar) - rc.right/2),abs(HIWORD(lPar) - rc.bottom/2));
        Joy[Port].DeadZone=int((NewDZ/double(rc.right/2))*100);

        DoPaint=true;
      }
      break;
    case WM_TIMER:
      DoPaint=true;
      break;
    case WM_DESTROY:
      KillTimer(Win,1);
      delete[] (int*)MAKELONG(GetWindowWord(Win,0),GetWindowWord(Win,2));
      break;
  }
  if (DoPaint){
    HDC DC;
    RECT rc;
    HBRUSH Bk;
    int Port=BasePort+(GetDlgCtrlID(Win)/100 - 1);

    GetClientRect(Win,&rc);
    int hx=rc.right/2,hy=rc.bottom/2;

    int DirPos[4],*OldDirPos=(int*)MAKELONG(GetWindowWord(Win,0),GetWindowWord(Win,2));
    for (int n=0;n<4;n++){
      DirPos[n]=0;

      int DirID=Joy[Port].DirID[n];
      if (HIBYTE(DirID)>=10 && DirID<0xffff){
        int ID=LOBYTE(DirID);
        int JoyNum=(HIBYTE(DirID)-10)/10;
        if (ID && JoyExists[JoyNum]){
          if (ID<100){  // Axis
            int AxNum=ID-1;
            if (JoyInfo[JoyNum].AxisExists[AxNum]){
              double pos;
              if (HIBYTE(DirID) & 1){
                pos=JoyInfo[JoyNum].AxisLen[AxNum]/2 - GetAxisPosition(AxNum,&JoyPos[JoyNum])-JoyInfo[JoyNum].AxisMin[AxNum];
              }else{
                pos=GetAxisPosition(AxNum,&JoyPos[JoyNum]) - JoyInfo[JoyNum].AxisMin[AxNum] - JoyInfo[JoyNum].AxisLen[AxNum]/2;
              }
              if (pos<=JoyInfo[JoyNum].AxisLen[AxNum]){
                if (n<2){
                  DirPos[n]=int((pos/double(JoyInfo[JoyNum].AxisLen[AxNum]/2)) * double(hy));
                }else{
                  DirPos[n]=int((pos/double(JoyInfo[JoyNum].AxisLen[AxNum]/2)) * double(hx));
                }
              }
            }
          }else if (ID>=200 && JoyInfo[JoyNum].AxisExists[AXIS_POV]){  // POV
            if (POV_CONV(JoyPos[JoyNum].dwPOV)==DWORD(ID-200)) DirPos[n]=int((n<2) ? hy:hx);
          }else{
            if (JoyPos[JoyNum].dwButtons & (1 << (ID-100))) DirPos[n]=int((n<2) ? hy:hx);
          }
        }
      }else{
        if (IsDirIDPressed(DirID,Joy[Port].DeadZone,0)) DirPos[n]=int((n<2) ? hy:hx);
      }
    }
    int n;
    for (n=0;n<4;n++){
      if ((DirPos[n]*100)>=(hx * Joy[Port].DeadZone)) break;
    }
    if (n==4){
      DirPos[0]=0;DirPos[1]=0;DirPos[2]=0;DirPos[3]=0;
    }

    if (Mess==WM_TIMER){
      for (n=0;n<4;n++){
        if (DirPos[n]!=OldDirPos[n]) break;
      }
      if (n==4) return 0;

      memcpy(OldDirPos,DirPos,sizeof(DirPos));
    }

    DC=GetDC(Win);

    Bk=CreateSolidBrush(GetSysColor(int(IsWindowEnabled(Win) ? COLOR_WINDOW:COLOR_BTNFACE)));
    FillRect(DC,&rc,Bk);
    DeleteObject(Bk);

    if (Joy[Port].ToggleKey){
      int dx=int(hx * Joy[Port].DeadZone)/100,dy=int(hy * Joy[Port].DeadZone)/100;

      HANDLE Old=SelectObject(DC,CreateSolidBrush(MidGUIRGB));
      Rectangle(DC,int(hx-dx),int(hy-dy),int(hx+dx),int(hy+dy));
      DeleteObject(SelectObject(DC,Old));

      int x3=rc.right/3,y3=rc.bottom/3;

      HANDLE OldPen=SelectObject(DC,CreatePen(PS_SOLID,1,GetSysColor(COLOR_WINDOWTEXT)));

      MoveToEx(DC,hx,hy,0);LineTo(DC,x3,0);
      MoveToEx(DC,hx,hy,0);LineTo(DC,x3*2,0);

      MoveToEx(DC,hx,hy,0);LineTo(DC,x3,rc.bottom-1);
      MoveToEx(DC,hx,hy,0);LineTo(DC,x3*2,rc.bottom-1);

      MoveToEx(DC,hx,hy,0);LineTo(DC,0,y3);
      MoveToEx(DC,hx,hy,0);LineTo(DC,0,y3*2);

      MoveToEx(DC,hx,hy,0);LineTo(DC,rc.right-1,y3);
      MoveToEx(DC,hx,hy,0);LineTo(DC,rc.right-1,y3*2);

      int xpos=hx,ypos=hy;

      ypos-=DirPos[0];
      ypos+=DirPos[1];
      xpos-=DirPos[2];
      xpos+=DirPos[3];

      Old=SelectObject(DC,CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT)));
      Ellipse(DC,xpos-4,ypos-4,xpos+4,ypos+4);
      DeleteObject(SelectObject(DC,Old));

      DeleteObject(SelectObject(DC,OldPen));
    }

    ReleaseDC(Win,DC);
    ValidateRect(Win,NULL);

    if (Mess==WM_PAINT) return 0;
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
LRESULT __stdcall TJoystickConfig::GroupBoxWndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  TJoystickConfig *This;
  GET_THIS;

  if (Mess==WM_COMMAND){
    if (LOWORD(wPar)==98){
      if (HIWORD(wPar)==BN_CLICKED){
        bool NewType=SendMessage((HWND)lPar,BM_GETCHECK,0,0);
        if (NewType!=Joy[BasePort].Type){
          Joy[BasePort].Type=NewType;
          This->JoyModeChange(BasePort,100);
          This->JoyModeChange(BasePort+1,200);
        }
      }
    }
  }
  return CallWindowProc(This->OldGroupBoxWndProc,Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
#undef GET_THIS
#endif

#ifdef UNIX
#include "x/x_joy.cpp"
#endif

