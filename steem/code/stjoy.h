#ifdef IN_MAIN
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)
#endif

EXT void JoyGetPoses(),JoyPosReset(int);
EXT BYTE JoyReadSTEAddress(MEM_ADDRESS,bool*);
EXT void joy_read_buttons();
EXT BYTE joy_get_pos(int);
EXT bool IsToggled(int);
EXT bool IsJoyActive(int);
EXT DWORD GetJagPadDown(int,DWORD);
EXT bool joy_is_key_used(BYTE);

#define N_JOY_PORT_0 0
#define N_JOY_PORT_1 1
#define N_JOY_STE_A_0 2
#define N_JOY_STE_A_1 3
#define N_JOY_STE_B_0 4
#define N_JOY_STE_B_1 5
#define N_JOY_PARALLEL_0 6
#define N_JOY_PARALLEL_1 7

EXT WORD paddles_ReadMask INIT(0);
EXT BYTE stick[8];
EXT void InitJoysticks(int=0),FreeJoysticks();

#ifdef IN_MAIN

#ifndef WIN32
typedef struct{
  DWORD dwSize; //Not read
  DWORD dwFlags; //Not read
  DWORD dwXpos;
  DWORD dwYpos;
  DWORD dwZpos;
  DWORD dwRpos;
  DWORD dwUpos;
  DWORD dwVpos;
  DWORD dwButtons;
  DWORD dwButtonNumber; //Not read
  DWORD dwPOV;
}JOYINFOEX;
#endif

static char AxisToName[7]={'X','Y','Z','R','U','V','P'};
//----------------------- Implementation ------------------------
DWORD GetAxisPosition(int,JOYINFOEX *);

JOYINFOEX JoyPos[MAX_PC_JOYS];

#define POV_CONV(POV) (((POV)<0xffff) ? (((POV)+2250)/4500):0xffff)
//----------------------- Info about joysticks ---------------------------------
bool DisablePCJoysticks=0;

#ifdef UNIX
void XOpenJoystick(int),XCloseJoystick(int);
void JoyInitAxisRange(int),JoyInitCalibration(int);
#endif

#define PCJOY_READ_DONT 2
#define PCJOY_READ_WINMM 0
#define PCJOY_READ_DI 1
#define PCJOY_READ_KERNELDRIVER 0

int JoyReadMethod=PCJOY_READ_WINMM;

#if defined(WIN32) && defined(NO_DIRECTINPUT)==0
IDirectInput *DIObj=NULL;
IDirectInputDevice *DIJoy[MAX_PC_JOYS]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
IDirectInputDevice2 *DIJoy2[MAX_PC_JOYS]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
DIJOYSTATE DIJoyPos[MAX_PC_JOYS];
int DIPOVNum=-1,DIAxisNeg[MAX_PC_JOYS][6],DI_RnMap[MAX_PC_JOYS][3],DIDisconnected[MAX_PC_JOYS];
#endif

#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2
#define AXIS_R 3
#define AXIS_U 4
#define AXIS_V 5
#define AXIS_POV 6

int NumJoysticks=0;
bool JoyExists[MAX_PC_JOYS]={0,0,0,0,0,0,0,0};

typedef struct{
  UINT AxisMin[6],AxisMax[6]; // On X user sets these
  UINT AxisMid[6],AxisLen[6];
  bool AxisExists[7];
  int NumButtons;
#ifdef UNIX
  bool On,NoEvent;
  Str DeviceFile;
  int Dev;
  UINT AxisDZ[6],Range;
#elif defined(WIN32)
  UINT ExFlags;
  bool NeedsEx;
  int WaitRead,WaitReadTime;
#endif
}JoystickInfo;
JoystickInfo JoyInfo[MAX_PC_JOYS];

typedef struct{
  bool Valid;
  UINT AxisPos[6];
  UINT Buttons;
  DWORD POV;
}OldJoystickPosition;
OldJoystickPosition OldJoyPos;
//---------------------------------------------------------------------------
class TJoystickConfig : public TStemDialog
{
private:
#ifdef WIN32
  static LRESULT __stdcall WndProc(HWND,UINT,WPARAM,LPARAM);
  static LRESULT __stdcall DeadZoneWndProc(HWND,UINT,WPARAM,LPARAM);
  static LRESULT __stdcall GroupBoxWndProc(HWND,UINT,WPARAM,LPARAM);
  void ManageWindowClasses(bool);

  HWND JagBut,Group[2];
  WINDOWPROC OldGroupBoxWndProc;
#elif defined(UNIX)
  static int WinProc(TJoystickConfig*,Window,XEvent*);
  static int button_notify_proc(hxc_button*,int,int*);
  static int picker_notify_proc(hxc_buttonpicker*,int,int);
  static int dd_notify_proc(hxc_dropdown*,int,int);
  static int scrollbar_notify_proc(hxc_scrollbar*,int,int);
  static int edit_notify_proc(hxc_edit*,int,int);
  static int timerproc(void*,Window,int);

  void ShowAndUpdatePage();
  bool AttemptOpenJoy(int);

  bool ConfigST;
  int PCJoyEdit;

	hxc_button st_group,pc_group,config_group;
  hxc_dropdown config_dd,setup_dd;
	hxc_button dir_par[2],fire_par[2],jagpad_par[2];

  hxc_edit device_ed;

	hxc_button joy_group[2],fire_but_label[2],autofire_but_label[2][2];
	hxc_dropdown autofire_dd[2];
	hxc_scrollbar MouseSpeedSB;
	hxc_button MouseSpeedLabel[4];
#endif
public:
  TJoystickConfig();
  ~TJoystickConfig() { Hide(); };
  void Show(),Hide();
  bool ToggleVisible(){ IsVisible() ? Hide():Show();return IsVisible(); }
  bool LoadData(bool,GoodConfigStoreFile*,bool* = NULL),SaveData(bool,ConfigStoreFile*);
  static void CreateJoyAnyButtonMasks();
  static int BasePort;

#ifdef WIN32
  bool HasHandledMessage(MSG *);
  void FillJoyTypeCombo(),CheckJoyType();
  void JoyModeChange(int,int);
#elif defined(UNIX)
  void UpdateJoyPos();

  hxc_buttonpicker picker[2][6];
  hxc_button enable_but[2];
  hxc_button centre_icon[2];
#endif
};
int TJoystickConfig::BasePort=0;
//----------------- Joystick Interface---------------------
#define JOY_MODE_OFF 0
#define JOY_MODE_KEYBOARD 1
#define JOY_MODE_JOY1 2
#define JOY_MODE_JOY2 3

#define JOY_TYPE_JOY 0
#define JOY_TYPE_JAGPAD 1

#define JAGPAD_BUT_FIRE_A 0
#define JAGPAD_BUT_FIRE_B 1
#define JAGPAD_BUT_FIRE_C 2
#define JAGPAD_BUT_KEY_OPTION 3
#define JAGPAD_BUT_KEY_PAUSE 4
#define JAGPAD_BUT_KEY_0 5
#define JAGPAD_BUT_KEY_1 6
#define JAGPAD_BUT_KEY_2 7
#define JAGPAD_BUT_KEY_3 8
#define JAGPAD_BUT_KEY_4 9
#define JAGPAD_BUT_KEY_5 10
#define JAGPAD_BUT_KEY_6 11
#define JAGPAD_BUT_KEY_7 12
#define JAGPAD_BUT_KEY_8 13
#define JAGPAD_BUT_KEY_9 14
#define JAGPAD_BUT_KEY_HASH 15
#define JAGPAD_BUT_KEY_AST 16

#define JAGPAD_BUT_FIRE_A_BIT (1 << JAGPAD_BUT_FIRE_A)
#define JAGPAD_BUT_FIRE_B_BIT (1 << JAGPAD_BUT_FIRE_B)
#define JAGPAD_BUT_FIRE_C_BIT (1 << JAGPAD_BUT_FIRE_C)
#define JAGPAD_BUT_KEY_OPTION_BIT (1 << JAGPAD_BUT_KEY_OPTION)
#define JAGPAD_BUT_KEY_PAUSE_BIT (1 << JAGPAD_BUT_KEY_PAUSE)
#define JAGPAD_BUT_KEY_0_BIT (1 << JAGPAD_BUT_KEY_0)
#define JAGPAD_BUT_KEY_1_BIT (1 << JAGPAD_BUT_KEY_1)
#define JAGPAD_BUT_KEY_2_BIT (1 << JAGPAD_BUT_KEY_2)
#define JAGPAD_BUT_KEY_3_BIT (1 << JAGPAD_BUT_KEY_3)
#define JAGPAD_BUT_KEY_4_BIT (1 << JAGPAD_BUT_KEY_4)
#define JAGPAD_BUT_KEY_5_BIT (1 << JAGPAD_BUT_KEY_5)
#define JAGPAD_BUT_KEY_6_BIT (1 << JAGPAD_BUT_KEY_6)
#define JAGPAD_BUT_KEY_7_BIT (1 << JAGPAD_BUT_KEY_7)
#define JAGPAD_BUT_KEY_8_BIT (1 << JAGPAD_BUT_KEY_8)
#define JAGPAD_BUT_KEY_9_BIT (1 << JAGPAD_BUT_KEY_9)
#define JAGPAD_BUT_KEY_HASH_BIT (1 << JAGPAD_BUT_KEY_HASH)
#define JAGPAD_BUT_KEY_AST_BIT (1 << JAGPAD_BUT_KEY_AST)

#define JAGPAD_DIR_U_BIT BIT_17
#define JAGPAD_DIR_D_BIT BIT_18
#define JAGPAD_DIR_L_BIT BIT_19
#define JAGPAD_DIR_R_BIT BIT_20

class TJoystick
{
public:
  TJoystick();
  ~TJoystick(){};

  int ToggleKey; //,KeyAutoFire;

  int DirID[6];  // Up Down Left Right Fire AutoFire
  int AnyFireOnJoy,AutoFireSpeed;
  int DeadZone;
  int JagDirID[17];

  int Type;
};

TJoystick::TJoystick()
{
  ToggleKey=0;

  for (int n=0;n<6;n++) DirID[n]=0xffff;
  DeadZone=50;
  AutoFireSpeed=0;
  AnyFireOnJoy=0;

  for (int n=0;n<17;n++) JagDirID[n]=0xffff;

  Type=JOY_TYPE_JOY;

  TJoystickConfig::CreateJoyAnyButtonMasks();
}


TJoystick Joy[8];
TJoystick JoySetup[3][8];
int nJoySetup=0;

// Bitmasks in which all set bits represent a button that can be pressed to
// cause fire in Any Button On... mode
DWORD JoyAnyButtonMask[MAX_PC_JOYS];
#endif

#undef EXT

