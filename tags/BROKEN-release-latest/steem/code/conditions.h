//---------------------------------------------------------------------------
//                         Conditional Defines for Steem
//---------------------------------------------------------------------------
#if !defined(_NO_DEBUG_BUILD) && !defined(_DEBUG_BUILD) && !defined(ONEGAME) && defined(WIN32)
#define _DEBUG_BUILD
#endif


//#include "mmgr/mmgr.h"


#ifndef WIN32
#include "notwindows.h"
#endif

// These should all be commented out for release!
#ifdef UNIX
//#define ENABLE_LOGFILE
#endif
//#define DISABLE_STEMDOS
//#define DISABLE_PSG
//#define DISABLE_BLITTER
//#define SHOW_DRAW_SPEED
//#define SHOW_WAVEFORM 1
//#define WRITE_ONLY_SINE_WAVE
//#define DRAW_ALL_ICONS_TO_SCREEN
//#define DRAW_TIMER_TO_SCREEN
//#define TRANSLATION_TEST
//#define NO_486_ASM
//#define NO_CRAZY_MONITOR
//#define NO_CSF
//#define USE_PORTAUDIO_ON_WIN32
//#define X_NO_PC_JOYSTICKS
//#define NO_XVIDMODE

#if defined(_DEBUG_BUILD) && defined(_BCB_BUILD)
#define PEEK_RANGE_TEST
#endif

// This should be left in for release (work in progress)
#define NO_GETCONTENTS

#if defined(UNIX) && !defined(NO_GETCONTENTS)
#define NO_GETCONTENTS
#endif

// These should always be left like this

#define SCREENS_PER_SOUND_VBL 1
#ifdef CYGWIN
#define NO_XVIDMODE
#endif

#ifdef _DEBUG_BUILD
#define ENABLE_VARIABLE_SOUND_DAMPING
#define ENABLE_LOGFILE
#endif

#if defined(_BCC_BUILD) || defined(_VC_BUILD) || defined(_MINGW_BUILD)
#define _RELEASE_BUILD
#endif

/*
#ifndef ONEGAME
#define ONEGAME
#define OG_NM1_IDX 1
#define OG_NM2_IDX 2
#define OG_AW1_IDX 3

#define OG_AW2_IDX 4
#define OG_SAT1_IDX 5
#define OG_SAT2_IDX 6
#define ONEGAME_NAME "nm1"
#define ONEGAME_IDX OG_NM1_IDX

#undef _DEBUG_BUILD
#undef ENABLE_LOGFILE
#endif
*/

#ifdef IN_MAIN
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)
#endif

//---------------------------------------------------------------------------
//     Set up some standard functions/defines that some compilers don't
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//                                Visual C++
//---------------------------------------------------------------------------
#ifdef _VC_BUILD

#define HMONITOR_DECLARED
#ifndef OBM_COMBO
#define OBM_COMBO 32738
#endif
#ifndef DIDEVTYPE_JOYSTICK
#define DIDEVTYPE_JOYSTICK 4
#endif

EXT int _argc INIT(0);
#ifdef IN_MAIN
char *_argv[2]={"",""};
#else
EXT char *_argv[2];
#endif

//---------------------------------------------------------------------------
//                                GCC/MinGW
//---------------------------------------------------------------------------
#elif defined(_MINGW_BUILD) || defined(UNIX)

#define ASMCALL /*this should be C calling convention*/

#ifdef UNIX

#define max(a,b) (a>b ? a:b)
#define min(a,b) (a>b ? b:a)
#define strcmpi strcasecmp

EXT Display *XD INIT(NULL);
EXT XContext cWinThis,cWinProc;

EXT char **_argv INIT(NULL);
EXT int _argc INIT(0);

EXT void UnixOutput(char *Str)
#ifdef IN_MAIN
{
  printf("%s\r\n",Str);
}
#else
;
#endif

#elif defined(WIN32)

#define _MINGW_INTS
extern "C" int ASMCALL int_16_2();

#endif

#if defined(IN_MAIN)

#ifdef UNIX
char *itoa(int i,char *s,int radix)
{
  if (radix==10) sprintf(s,"%i",(int)i);
  if (radix==16) sprintf(s,"%x",(int)i);
  return s;
}
#endif

char *ultoa(unsigned long l,char *s,int radix)
{
  if (radix==10) sprintf(s,"%u",(unsigned int)l);
  if (radix==16) sprintf(s,"%x",(unsigned int)l);
  return s;
}
char strupr_convert_buf[256]={0},strlwr_convert_buf[256]={0};

char *strupr(char *s)
{
  if (strupr_convert_buf[0]==0){
    strupr_convert_buf[0]=1;
    for (int i=1;i<256;i++){
      strupr_convert_buf[i]=(char)i;
      if (islower(i)) strupr_convert_buf[i]=toupper((char)i);
    }
  }
  char *p=s;
  while (*p){
    *p=strupr_convert_buf[(unsigned char)(*p)];
    p++;
  }
  return s;
}

char *strlwr(char *s)
{
  if (strlwr_convert_buf[0]==0){
    strlwr_convert_buf[0]=1;
    for (int i=1;i<256;i++){
      strlwr_convert_buf[i]=(char)i;
      if (isupper(i)) strlwr_convert_buf[i]=tolower((char)i);
    }
  }
  char *p=s;
  while (*p){
    *p=strlwr_convert_buf[(unsigned char)(*p)];
    p++;
  }
  return s;
}

#endif

//---------------------------------------------------------------------------
//                            Add more compilers here
//---------------------------------------------------------------------------
#endif

#ifndef random
#define random(n) (rand() % (n))
#endif

//---------------------------------------------------------------------------
//         Some nice macros to make porting easier (less #ifdefs)
//---------------------------------------------------------------------------
#ifdef WIN32

#define WIN_ONLY(a) a
#define UNIX_ONLY(a)
#define SLASH "\\"
#define SLASHCHAR '\\'
typedef HWND WINDOWTYPE;
#define UnixOutput(a)

#elif defined(UNIX)

#define WIN_ONLY(a)
#define UNIX_ONLY(a) a
#define SLASH "/"
#define SLASHCHAR '/'
typedef Window WINDOWTYPE;

#endif

#ifdef _DEBUG_BUILD
#define DEBUG_ONLY(s) s
#else
#define DEBUG_ONLY(s)
#endif

#ifdef ENABLE_LOGFILE
#define LOG_ONLY(s) s
#else
#define LOG_ONLY(s)
#endif

#ifdef ONEGAME

#define ONEGAME_ONLY(s) s
#define NOT_ONEGAME(s)
#define DISABLE_STEMDOS
#define NO_RAR_SUPPORT

#else
#define ONEGAME_ONLY(s)
#define NOT_ONEGAME(s) s
#endif

#ifndef ASMCALL

#ifdef WIN32
#define ASMCALL __cdecl
#else
#define ASMCALL

#endif

#endif

#ifndef _VC_BUILD
#define SET_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
                       const GUID name={l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#else
#define SET_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)
#endif

#ifndef BIG_ENDIAN_PROCESSOR

// Little endian: least significant byte , low mid byte, hi mid byte , most significant byte
#define MEM_DIR -1
#define MEM_GE <=
#define MORE_SIGNIFICANT_BYTE_OFFSET 1
#define LPLOWORD(a) ((WORD*)(&a))
#define LPHIWORD(a) (((WORD*)(&a))+1)
// B_n - n=significance (0 is LSB, 3 is MSB)
#define DWORD_B_0(s) *( ((BYTE*)(s))   )
#define DWORD_B_1(s) *( ((BYTE*)(s)) +1)
#define DWORD_B_2(s) *( ((BYTE*)(s)) +2)
#define DWORD_B_3(s) *( ((BYTE*)(s)) +3)
#define DWORD_B(s,n) *( ((BYTE*)(s)) +(n))
#define DWORD_W_0(s) *( ((WORD*)(s))   )
#define DWORD_W_1(s) *( ((WORD*)(s)) +1)
#define lpDWORD_B_0(s)  ( ((BYTE*)(s)) )
#define lpDWORD_B_1(s)  ( ((BYTE*)(s)) +1)
#define lpDWORD_B_2(s)  ( ((BYTE*)(s)) +2)
#define lpDWORD_B_3(s)  ( ((BYTE*)(s)) +3)

#define WORD_B_0(s) *( ((BYTE*)(s))   )
#define WORD_B_1(s) *( ((BYTE*)(s)) +1)
#define WORD_B(s,n) *( ((BYTE*)(s)) +(n))
#define lpWORD_B_0(s)  ( ((BYTE*)(s))  )
#define lpWORD_B_1(s)  ( ((BYTE*)(s)) +1)

#else

// Big endian: most significant byte , hi mid byte, low mid byte , least significant byte
#define MEM_DIR 1
#define MEM_GE >=
#define MORE_SIGNIFICANT_BYTE_OFFSET -1
#define LPHIWORD(a) ((WORD*)(&a))
#define LPLOWORD(a) (((WORD*)(&a))+1)
// B_n - n=significance (0 is LSB, 3 is MSB)
#define DWORD_B_3(s) *( ((BYTE*)(s))   )
#define DWORD_B_2(s) *( ((BYTE*)(s)) +1)
#define DWORD_B_1(s) *( ((BYTE*)(s)) +2)
#define DWORD_B_0(s) *( ((BYTE*)(s)) +3)
#define DWORD_B(s,n) *( ((BYTE*)(s)) +(3-(n)))
#define DWORD_W_1(s) *( ((WORD*)(s))   )
#define DWORD_W_0(s) *( ((WORD*)(s)) +1)
#define lpDWORD_B_3(s)  ( ((BYTE*)(s))   )
#define lpDWORD_B_2(s)  ( ((BYTE*)(s)) +1)
#define lpDWORD_B_1(s)  ( ((BYTE*)(s)) +2)
#define lpDWORD_B_0(s)  ( ((BYTE*)(s)) +3)

#define WORD_B_1(s) *( ((BYTE*)(s))   )
#define WORD_B_0(s) *( ((BYTE*)(s)) +1)
#define WORD_B(s,n) *( ((BYTE*)(s)) +(1-(n)))
#define lpWORD_B_1(s)  ( ((BYTE*)(s))   )
#define lpWORD_B_0(s)  ( ((BYTE*)(s)) +1)

#endif

#define MAKECHARCONST(a,b,c,d) (BYTE(a) | (BYTE(b) << 8) | (BYTE(c) << 16) | (BYTE(d) << 24))

#undef MAKEWORD
#undef MAKELONG

#define MAKEWORD(a,b) ((WORD)(((WORD)(((BYTE)(a))) | (((WORD)((BYTE)(b))) << 8))))
#define MAKELONG(a,b) ((LONG)(((LONG)(((WORD)(a))) | (((DWORD)((WORD)(b))) << 16))))
#define SWAPBYTES(Var) (Var=MAKEWORD(HIBYTE((Var)),LOBYTE((Var))))
#define SWAPWORDS(Var) (Var=MAKELONG(HIWORD((Var)),LOWORD((Var))))

// These are so you can do MAKEBINW(b00000011,b11100000);
#define MAKEBINW(high,low) ((BYTE(high) << 8) | BYTE(low))
#define MAKEBINL(highest,high,low,lowest) \
         ( (BYTE(highest) << 24) | (BYTE(high) << 16) | \
           (BYTE(low) << 8) | BYTE(lowest) )

#define MEM_ADDRESS unsigned long

#define DEFVAL(s) =s

#ifdef _DEBUG_BUILD

extern bool logging_suspended;
extern bool logsection_enabled[100];
class EasyStr;
extern void log_write(EasyStr);

#define LOGSECTION_INIFILE 19
#define LOGSECTION_GUI 20
#define CSF_LOG(s) if (logsection_enabled[LOGSECTION_INIFILE] && logging_suspended==0) log_write(s)
#define DTREE_LOG(s) if (logsection_enabled[LOGSECTION_GUI] && logging_suspended==0) log_write(s)

#ifndef _RELEASE_BUILD
extern bool HWNDNotValid(HWND,char*,int);
extern LRESULT SendMessage_checkforbugs(HWND,UINT,WPARAM,LPARAM,char*,int);
extern BOOL PostMessage_checkforbugs(HWND,UINT,WPARAM,LPARAM,char*,int);
static BOOL DestroyWindow_checkforbugs(HWND Win,char *File,int Line)
{
  if (HWNDNotValid(Win,File,Line)) return 0;
  return DestroyWindow(Win);
}
static BOOL InvalidateRect_checkforbugs(HWND Win,CONST RECT *pRC,BOOL bErase,char *File,int Line)
{
  if (HWNDNotValid(Win,File,Line)) return 0;
  return InvalidateRect(Win,pRC,bErase);
}
static BOOL SWP_checkforbugs(HWND Win,HWND WinAfter,int x,int y,int w,int h,UINT Flags,char *File,int Line)
{
  if (HWNDNotValid(Win,File,Line)) return 0;
  return SetWindowPos(Win,WinAfter,x,y,w,h,Flags);
}

#undef SendMessage
#define SendMessage(win,m,w,l) SendMessage_checkforbugs(win,m,w,l,__FILE__,__LINE__)
#undef PostMessage
#define PostMessage(win,m,w,l) PostMessage_checkforbugs(win,m,w,l,__FILE__,__LINE__)
#define DestroyWindow(win) DestroyWindow_checkforbugs(win,__FILE__,__LINE__)
#define InvalidateRect(win,rc,b) InvalidateRect_checkforbugs(win,rc,b,__FILE__,__LINE__)
#define SetWindowPos(win,win2,x,y,w,h,f) SWP_checkforbugs(win,win2,x,y,w,h,f,__FILE__,__LINE__)
#endif

#else

#define CSF_LOG(s)
#define DTREE_LOG(s)

#endif

#define MAX_PC_JOYS 8

