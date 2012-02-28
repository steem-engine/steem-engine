#ifdef IN_MAIN
#define EXT
#else
#define EXT extern
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

EXT int Alert(char *,char *,UINT);
EXT void ShowAllDialogs(bool);
EXT void fast_forward_change(bool,bool);
EXT void slow_motion_change(bool);

EXT int PasteVBLCount=0,PasteSpeed=2;
EXT Str PasteText;

EXT void fast_forward_change(bool,bool);

#ifdef WIN32

EXT void DisableTaskSwitch(),EnableTaskSwitch();
EXT void UpdatePasteButton();

#elif defined(UNIX)

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

EXT void QuitSteem();
EXT bool HighPriority=0;
EXT int DoSaveScreenShot=0;

EXT bool ResChangeResize=true,CanUse_400=0;

#define STEM_MOUSEMODE_DISABLED 0
#define STEM_MOUSEMODE_WINDOW 1
#define STEM_MOUSEMODE_BREAKPOINT 3141

EXT int stem_mousemode=STEM_MOUSEMODE_DISABLED;
EXT int window_mouse_centre_x,window_mouse_centre_y;

#undef EXT

