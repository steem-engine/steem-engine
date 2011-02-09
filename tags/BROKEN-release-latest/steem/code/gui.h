#ifdef IN_MAIN
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)
#endif

#ifndef ONEGAME
#define MENUHEIGHT 20
#else
#define MENUHEIGHT 0
#endif

#define PEEKED_MESSAGE 0
#define PEEKED_QUIT 1
#define PEEKED_NOTHING 2
#define PEEKED_RUN 3

extern "C" ASMCALL int PeekEvent();

UNIX_ONLY( EXT void PostRunMessage(); )

EXT void GUIRunStart(),GUIRunEnd();
EXT int Alert(char *,char *,UINT);
EXT void QuitSteem();
EXT void fast_forward_change(bool,bool);
EXT bool GUIPauseWhenInactive();
EXT void PasteVBL(),StemWinResize(int DEFVAL(0),int DEFVAL(0));
EXT void GUIDiskErrorEject(int);
EXT bool GUICanGetKeys();
EXT void GUIColdResetChangeSettings();
EXT void CheckResetIcon(),CheckResetDisplay(bool=0);
EXT void UpdateSTKeys();
EXT void GUIEmudetectCreateDisk(Str,int,int,int);

#define STPASTE_TOGGLE 0
#define STPASTE_START 1
#define STPASTE_STOP 2
EXT void PasteIntoSTAction(int);

EXT int DoSaveScreenShot INIT(0);
EXT bool ResChangeResize INIT(true),CanUse_400 INIT(0);
EXT bool bAppActive INIT(true),bAppMinimized INIT(0);
EXT DWORD DisableDiskLightAfter INIT(3000);

#define IGNORE_EXTEND 2
#define NO_SHIFT_SWITCH 8
EXT void HandleKeyPress(UINT,bool,int DEFVAL(IGNORE_EXTEND));

EXT LANGID KeyboardLangID INIT(0);

#define STEM_MOUSEMODE_DISABLED 0
#define STEM_MOUSEMODE_WINDOW 1
#define STEM_MOUSEMODE_BREAKPOINT 3141

EXT int stem_mousemode INIT(STEM_MOUSEMODE_DISABLED);
EXT int window_mouse_centre_x,window_mouse_centre_y;
EXT bool TaskSwitchDisabled INIT(0);
EXT Str ROMFile,CartFile;

EXT bool FSQuitAskFirst INIT(true),Quitting INIT(0);
EXT bool FSDoVsync INIT(0);

#define LVI_SI_CHECKED (1 << 13)
#define LVI_SI_UNCHECKED (1 << 12)
#define PROFILESECT_ON LVI_SI_CHECKED
#define PROFILESECT_OFF LVI_SI_UNCHECKED

EXT int ExternalModDown INIT(0);
EXT bool comline_allow_LPT_input INIT(0);

#define RC_FLAG_WIDTH 16
#define RC_FLAG_HEIGHT 12

#if defined(UNIX)

EXT void XGUIUpdatePortDisplay();
EXT short GetKeyState(int);
#define GetAsyncKeyState GetKeyState
EXT void GUIUpdateInternalSpeakerBut();
EXT void GetCursorPos(POINT *);
EXT void SetCursorPos(int,int);

typedef struct{
  bool LShift,RShift;
  bool LCtrl,RCtrl;
  bool LAlt,RAlt;
}MODIFIERSTATESTRUCT;

EXT MODIFIERSTATESTRUCT GetLRModifierStates();

#define MB_OK 0x00000000L
#define MB_OKCANCEL 0x00000001L
#define MB_ABORTRETRYIGNORE 0x00000002L
#define MB_YESNOCANCEL 0x00000003L
#define MB_YESNO 0x00000004L
#define MB_RETRYCANCEL 0x00000005L

#define MB_ICONHAND 0x00000010L
#define MB_ICONQUESTION 0x00000020L
#define MB_ICONEXCLAMATION 0x00000030L
#define MB_ICONASTERISK 0x00000040L
#define MB_USERICON 0x00000080L
#define MB_ICONWARNING MB_ICONEXCLAMATION
#define MB_ICONERROR MB_ICONHAND
#define MB_ICONINFORMATION MB_ICONASTERISK
#define MB_ICONSTOP MB_ICONHAND

#define MB_DEFBUTTON1 0x00000000L
#define MB_DEFBUTTON2 0x00000100L
#define MB_DEFBUTTON3 0x00000200L
#define MB_DEFBUTTON4 0x00000300L

#define MB_APPLMODAL 0x00000000L
#define MB_SYSTEMMODAL 0x00001000L
#define MB_TASKMODAL 0x00002000L

#define MB_HELP 0x00004000L
#define MB_NOFOCUS 0x00008000L
#define MB_SETFOREGROUND 0x00010000L
#define MB_DEFAULT_DESKTOP_ONLY 0x00020000L
#define MB_TOPMOST 0x00040000L
#define MB_RIGHT 0x00080000L

#define MB_TYPEMASK 0x0000000FL
#define MB_ICONMASK 0x000000F0L
#define MB_DEFMASK 0x00000F00L
#define MB_MODEMASK 0x00003000L
#define MB_MISCMASK 0xFFFFC000L

#define IDOK                1
#define IDCANCEL            2
#define IDABORT             3
#define IDRETRY             4
#define IDIGNORE            5
#define IDYES               6
#define IDNO                7

#endif
//---------------------------------------------------------------------------
#ifdef IN_MAIN

#define SHORTCUTS_TIMER_ID 2000
#define DISPLAYCHANGE_TIMER_ID 2100
#define MIDISYSEX_TIMER_ID 2200

void LoadAllIcons(ConfigStoreFile *,bool=0);

int GetComLineArgType(char *,EasyStr &);

// Flags
#define ARG_UNKNOWN 0
#define ARG_GDI 1
#define ARG_NODS 2
#define ARG_WINDOW 3
#define ARG_NOLPT 4
#define ARG_NOCOM 5
#define ARG_NOSHM 6
#define ARG_QUITQUICKLY 7
#define ARG_SOUNDCLICK 8
#define ARG_HELP 9
#define ARG_FULLSCREEN 10
#define ARG_DOUBLECHECKSHORTCUTS 11
#define ARG_DONTLIMITSPEED 12
#define ARG_EXACTSPEEDLIMITNONE 13
#define ARG_EXACTSPEEDLIMITTICK 14
#define ARG_EXACTSPEEDLIMITHP 15
#define ARG_EXACTSPEEDLIMITSLEEPTIME 16
#define ARG_EXACTSPEEDLIMITSLEEPHP 17
#define ARG_ACCURATEFDC 18
#define ARG_NOPCJOYSTICKS 19
#define ARG_OLDPORTIO 20
#define ARG_ALLOWREADOPEN 21
#define ARG_NOINTS 22
#define ARG_STFMBORDER 23
#define ARG_SCREENSHOTUSEFULLNAME 24
#define ARG_ALLOWLPTINPUT 25

// Settings
#define ARG_SETSOF 100
#define ARG_SETINIFILE 101
#define ARG_SETTRANSFILE 102
#define ARG_SETFONT 103
#define ARG_SETCUTSFILE 104
#define ARG_SETDIVUTIME 105
#define ARG_SETDIVSTIME 106
#define ARG_TAKESHOT 107
#define ARG_SETPABUFSIZE 108

// Files
#define ARG_DISKIMAGEFILE 201
#define ARG_SNAPSHOTFILE 202
#define ARG_CARTFILE 203
#define ARG_STPROGRAMFILE 204
#define ARG_STPROGRAMTPFILE 205
#define ARG_LINKFILE 206
#define ARG_TOSIMAGEFILE 207

#define ARG_NONEWINSTANCE 250
#define ARG_ALWAYSNEWINSTANCE 251

void ParseCommandLine(int,char*[],int=0);

bool MakeGUI();

void SetStemWinSize(int,int,int=0,int=0);
void SetStemMouseMode(int);

#define MSW_NOCHANGE int(0x7fff)
void MoveStemWin(int,int,int,int);

int GetScreenWidth(),GetScreenHeight();

void ShowAllDialogs(bool);
void slow_motion_change(bool);

bool RunMessagePosted=0;

BYTE KeyDownModifierState[256];

void ShiftSwitchChangeModifiers(bool,bool,int[]);
void ShiftSwitchRestoreModifiers(int[]);
void HandleShiftSwitching(UINT,bool,BYTE&,int[]);

Str SnapShotGetLastBackupPath();
void SnapShotGetOptions(EasyStringList*);

int PasteVBLCount=0,PasteSpeed=2;
Str PasteText;
bool StartEmuOnClick=0;
//---------------------------------------------------------------------------
#ifdef WIN32
LRESULT __stdcall WndProc(HWND,UINT,WPARAM,LPARAM);
LRESULT __stdcall FSClipWndProc(HWND,UINT,WPARAM,LPARAM);
LRESULT __stdcall FSQuitWndProc(HWND,UINT,WPARAM,LPARAM);

HWND FSQuitBut=NULL;

HICON hGUIIcon[RC_NUM_ICONS],hGUIIconSmall[RC_NUM_ICONS];

inline bool HandleMessage(MSG*);
void EnableAllWindows(bool,HWND);

void fast_forward_change(bool,bool);
void HandleButtonMessage(UINT,HWND);
void DisableTaskSwitch();
void EnableTaskSwitch();

bool IsSteemAssociated(EasyStr);
void AssociateSteem(EasyStr,EasyStr,bool,char *,int,bool);
void UpdatePasteButton();

HWND StemWin=NULL,ParentWin=NULL,ToolTip=NULL,DisableFocusWin=NULL,UpdateWin=NULL;
HMENU StemWin_SysMenu=NULL;
HFONT fnt;
HCURSOR PCArrow;
COLORREF MidGUIRGB,DkMidGUIRGB;
HANDLE SteemRunningMutex=NULL;

bool WinNT=0;
bool AllowTaskSwitch = NOT_ONEGAME(true) ONEGAME_ONLY(0);
HHOOK hNTTaskSwitchHook=NULL;
HWND NextClipboardViewerWin=NULL;

#define PostRunMessage()  if (RunMessagePosted==0){ \
                            SendDlgItemMessage(StemWin,101,BM_SETCLICKBUTTON,1,0); \
                            PostMessage(StemWin,WM_COMMAND,MAKEWPARAM(101,BN_CLICKED),(LPARAM)GetDlgItem(StemWin,101)); \
                            RunMessagePosted=true; \
                          }
//---------------------------------------------------------------------------
#elif defined(UNIX)
//---------------------------------------------------------------------------
XErrorEvent XError;
int HandleXError(Display*,XErrorEvent*);

int StemWinProc(void*,Window,XEvent*);
int timerproc(void*,Window,int);

void steem_hxc_modal_notify(bool);

hxc_popup pop;
hxc_popuphints hints;

int ProcessEvent(XEvent *);
void InitColoursAndIcons();

void steem_hxc_alloc_colours(Display*);
void steem_hxc_free_colours(Display*);

Window StemWin=0;
GC DispGC=0;
Cursor EmptyCursor=0;
Atom RunSteemAtom,LoadSnapShotAtom;
XID SteemWindowGroup;
DWORD BlackCol=0,WhiteCol=0,BkCol=0,BorderLightCol,BorderDarkCol;
hxc_alert alert;
//XFontStruct *GUIFont=NULL,*SmallFont=NULL;

void PrintHelpToStdout();
#define GetLongPathName(from,to,len) if (to!=from){strncpy(to,from,len);to[len-1]=0;}
bool SetForegroundWindow(Window,Time=CurrentTime);
Window GetForegroundWindow();
void CentreWindow(Window,bool);
bool GetWindowPositionData(Window,WINPOSITIONDATA *);

short KeyState[256]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
void SetKeyState(int,bool,bool=false);
short GetKeyStateSym(KeySym);

int MessageBox(WINDOWTYPE,char *,char *,UINT);

typedef int (BUTNOTIFYPROC)(Window,int,int,int*);
typedef BUTNOTIFYPROC* LPBUTNOTIFYPROC;
int StemWinButtonNotifyProc(Window,int,int,int *);

void SnapShotProcess(int);

#define ICO16_FOLDER 0
#define ICO16_PARENTDIRECTORY 1
#define ICO16_EXCLAMATION 2
#define ICO16_QUESTION 3
#define ICO16_STOP 4
#define ICO16_INFO 5
#define ICO16_FF 6
#define ICO16_FORWARD 7
#define ICO16_HARDDRIVE 8
#define ICO16_HOMEFOLDER 9
#define ICO16_GENERALINFO 10
#define ICO16_JOY 11
#define ICO16_PORTS 12
#define ICO16_OSD 13
#define ICO16_STEEM 14
#define ICO16_INSERTDISK 15
#define ICO16_RESET 16
#define ICO16_RUN 17
#define ICO16_SETHOMEFOLDER 18
#define ICO16_CUT 19
#define ICO16_SNAPSHOTS 20
#define ICO16_SNAPSHOT 20
#define ICO16_EJECTDISK 21
#define ICO16_OPTIONS 22
#define ICO16_BACK 23
#define ICO16_STCONFIG 24
#define ICO16_CHIP 24
#define ICO16_CART 24
#define ICO16_SOUND 25
#define ICO16_DISKMAN 26
#define ICO16_DISK 26
#define ICO16_JOYDIR 27
#define ICO16_ZIP_RO 28
#define ICO16_PATCHES 29
#define ICO16_TOOLS ICO16_OPTIONS
#define ICO16_DISPLAY 32
#define ICO16_BRIGHTCON 31
#define ICO16_LINKS 33
#define ICO16_README 34
#define ICO16_FAQ 35
#define ICO16_DRAWSPEED 36
#define ICO16_PASTE 37
#define ICO16_FUJI16 38
#define ICO16_ACCURATEFDC 39
#define ICO16_MIDI 40
#define ICO16_CUTON 41
#define ICO16_CUTOFF 42
#define ICO16_PATCHESNEW 43
#define ICO16_RESETGLOW 44
#define ICO16_PROFILE 45
#define ICO16_ST 46
#define ICO16_MACROS 47
#define ICO16_TICKED 48
#define ICO16_RADIOMARK 49
#define ICO16_DISKMANMENU 50
#define ICO16_FOLDERLINK 51
#define ICO16_FOLDERLINKBROKEN 52
#define ICO16_DISKLINK 53
#define ICO16_DISKLINKBROKEN 54
#define ICO16_CUTONLINK 55
#define ICO16_CUTOFFLINK 56
#define ICO16_ZIP_RW 57
#define ICO16_DISK_RO 58
#define ICO16_PROFILELINK 59
#define ICO16_MACROLINK 60
#define ICO16_UNTICKED 61
#define ICO16_FULLSCREEN 62
#define ICO16_TAKESCREENSHOTBUT 63

#define ICO32_JOYDIR 0
#define ICO32_RECORD 1
#define ICO32_DRIVE_A 2
#define ICO32_DRIVE_B 3
#define ICO32_LINKCUR 4
#define ICO32_PLAY 5
#define ICO32_DRIVE_B_OFF 6

#define ICO64_STEEM 0
#define ICO64_HARDDRIVES 1
#define ICO64_HARDDRIVES_FR 6

extern "C" LPBYTE Get_icon16_bmp(),Get_icon32_bmp(),Get_icon64_bmp(),Get_tos_flags_bmp();
IconGroup Ico16,Ico32,Ico64,IcoTOSFlags;
Pixmap StemWinIconPixmap=0,StemWinIconMaskPixmap=0;

hxc_button RunBut,FastBut,ResetBut,SnapShotBut,ScreenShotBut,PasteBut,FullScreenBut;
hxc_button InfBut,PatBut,CutBut,OptBut,JoyBut,DiskBut;

#define FF_DOUBLECLICK_MS 200

DWORD ff_doubleclick_time=0;

int StemWinButtonNotifyProc(hxc_button*,int,int*);

inline bool SetProp(Window Win,XContext Prop,DWORD Val)
{
  return SetProp(XD,Win,Prop,Val);
}

inline DWORD GetProp(Window Win,XContext Prop)
{
  return GetProp(XD,Win,Prop);
}

inline DWORD RemoveProp(Window Win,XContext Prop)
{
  return RemoveProp(XD,Win,Prop);
}

int romfile_parse_routine(char*fn,struct stat*s)
{
  if (S_ISDIR(s->st_mode)) return FS_FTYPE_FOLDER;
  if (has_extension_list(fn,".IMG",".ROM",NULL)){
    return FS_FTYPE_FILE_ICON+ICO16_STCONFIG;
  }
  return FS_FTYPE_REJECT;
}

int diskfile_parse_routine(char *fn,struct stat *s)
{
  if (S_ISDIR(s->st_mode)) return FS_FTYPE_FOLDER;
  if (FileIsDisk(fn)){
	  return FS_FTYPE_FILE_ICON+ICO16_DISKMAN;	
  }
  return FS_FTYPE_REJECT;
}

int wavfile_parse_routine(char *fn,struct stat *s)
{
  if (S_ISDIR(s->st_mode)) return FS_FTYPE_FOLDER;
  if (has_extension(fn,".WAV")){
	  return FS_FTYPE_FILE_ICON+ICO16_SOUND;	
  }
  return FS_FTYPE_REJECT;
}

int folder_parse_routine(char *fn,struct stat *s)
{
  if (S_ISDIR(s->st_mode)) return FS_FTYPE_FOLDER;
  return FS_FTYPE_REJECT;
}

int cartfile_parse_routine(char *fn,struct stat*s)
{
  if (S_ISDIR(s->st_mode)) return FS_FTYPE_FOLDER;
  if (has_extension(fn,".STC")){
    if ((s->st_size)==128*1024+4){
      return FS_FTYPE_FILE_ICON+ICO16_CART;
    }
  }
  return FS_FTYPE_REJECT;
}

hxc_fileselect fileselect;

#endif

bool load_cart(char*);
void CleanUpSteem();
bool StepByStepInit=0;
EasyStr RunDir,WriteDir,INIFile,ScreenShotFol;
EasyStr LastSnapShot,BootStateFile,StateHist[10],AutoSnapShotName="auto";
bool BootDisk[2]={0,0},PauseWhenInactive=0,BootTOSImage=0;
bool bAOT=0,bAppMaximized=0;
#ifndef ONEGAME
bool AutoLoadSnapShot=true,ShowTips=true;
#else
bool AutoLoadSnapShot=0,ShowTips=0;
#endif
bool AllowLPT=true,AllowCOM=true;
bool HighPriority=0;

#define BOOT_MODE_DEFAULT 0
#define BOOT_MODE_FULLSCREEN 1
#define BOOT_MODE_WINDOW 2
int BootInMode=BOOT_MODE_DEFAULT;

char *FSTypes(int,...);

bool NoINI;

const POINT WinSize[4][5]={ {{320,200},{640,400},{960, 600},{1280,800},{-1,-1}},
                            {{640,200},{640,400},{1280,400},{1280,800},{-1,-1}},
                            {{640,400},{1280,800},{-1,-1}},
                            {{800,600},{-1,-1}}};

const POINT WinSizeBorder[4][5]={ {{320+BORDER_SIDE*2,200+(BORDER_TOP+BORDER_BOTTOM)},
                                   {640+(BORDER_SIDE*2)*2,400+2*(BORDER_TOP+BORDER_BOTTOM)},
                                   {960+(BORDER_SIDE*3)*2, 600+3*(BORDER_TOP+BORDER_BOTTOM)},
                                   {1280+(BORDER_SIDE*4)*2,800+4*(BORDER_TOP+BORDER_BOTTOM)},
                                   {-1,-1}},
                                  {{640+(BORDER_SIDE*2)*2,200+(BORDER_TOP+BORDER_BOTTOM)},
                                   {640+(BORDER_SIDE*2)*2,400+2*(BORDER_TOP+BORDER_BOTTOM)},
                                   {1280+(BORDER_SIDE*4)*2,400+2*(BORDER_TOP+BORDER_BOTTOM)},
                                   {1280+(BORDER_SIDE*4)*2,800+4*(BORDER_TOP+BORDER_BOTTOM)},
                                   {-1,-1}},
                                  {{640+(BORDER_SIDE*2)*2,400+2*(BORDER_TOP+BORDER_BOTTOM)},
                                   {1280+(BORDER_SIDE*4)*2,800+4*(BORDER_TOP+BORDER_BOTTOM)},
                                   {-1,-1}},
                                   {{800,600},
                                   {-1,-1}}
                                   };

int WinSizeForRes[4]={0,0,0,0};

RECT rcPreFS;

void flashlight(bool);

BYTE PCCharToSTChar[128]={  0,  0,  0,159,  0,  0,187,  0,  0,  0,  0,  0,181,  0,  0,  0,
                            0,  0,154,  0,  0,  0,  0,255,  0,191,  0,  0,182,  0,  0,  0,
                            0,173,  0,156,  0,157,  0,221,185,189,  0,174,170,  0,190,  0,
                          248,241,253,254,  0,230,188,  0,  0,199,  0,175,172,171,  0,168,
                          182,  0,  0,183,142,143,147,128,  0,144,  0,  0,  0,  0,  0,  0,
                            0,165,  0,  0,  0,184,153,194,178,  0,  0,  0,154,  0,  0,158,
                          133,160,131,176,132,134,145,135,138,130,136,137,141,161,140,139,
                            0,164,149,162,147,177,148,246,179,151,163,150,154,  0,  0,152};

BYTE STCharToPCChar[128]={199,  0,233,226,228,224,229,231,234,235,232,239,238,236,196,197,
                          201,230,  0,244,246,242,251,249,255,214,252,  0,163,165,223,131,
                          225,237,243,250,241,209,  0,  0,191,  0,172,189,188,161,171,187,
                          227,245,216,248,  0,140,156,195,213,168,  0,134,182,169,174,153,
                            0,  0,215,  0,  0,  0,  0,185,  0,  0,  0,  0,  0,  0,  0,  0,
                            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,167,  0,  0,
                            0,  0,  0,  0,  0,  0,181,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                            0,177,  0,  0,  0,  0,247,  0,176,  0,  0,  0,  0,178,179,151};


#endif

#undef EXT
#undef INIT

