// Steem Steven Seagal Edition
// SSE.h

/*

This is based on the source code for Steem 3.2 as released by Steem authors,
Ant & Russ Hayward.
Current site for this build: http://ataristeven.host898.net/Steem.htm
SVN code repository is at http://code.google.com/p/steem-engine/



Added some files to the project. In folder 'code':
-SSE.h (this file, with defines & declarations)
-SSE.cpp
-SSECpu.h
-SSECpu.cpp
-SSEVideo.h
-SSEVideo.cpp
-A new folder '3rdparty\hatari' and some files from this emulator copied and 
slightly adapted to be used in Steem (shameless code theft).
-A file div68kCycleAccurate.c in '3rdparty\pasti'.
Other mods are in Steem code, inside blocks defined by STEVEN_SEAGAL
Many other defines are used to segment code. This is heavy but it makes 
debugging a lot easier.
To enjoy the new features, you must define STEVEN_SEAGAL in your
script or your IDE!
If not, you should get the last 3.2 build that compiles in VC6 (only
thing changed is no update attempt).
This file SSE.h must be included even if you don't want the 
SSE build because we need to define (neutralise) the 
debug macros.

My inane comments outside of defined blocks generally are marked by 'SS:'
They don't mean that I did something cool, only that I comment the source.
		  
VC6 is used as IDE, but also Notepad and the free (and discontinued)
Borland C++ 5.5 compiler, like the original Steem.
Compatibility with those compilers is a requirement of this build.
The project should also compile in Unix/Linux (g++).

*/

/*
TODO:
  - boiler copy disassembly to clipboard
  - fix sound in Linux
  - restore log feature someplaces
*/

// Version : 3.3.0, 28/02/2012

#pragma once // VC guard
#ifndef STEVEN_SEAGAL_H // BCC guard
#define STEVEN_SEAGAL_H


//////////////////
// BIG SWITCHES //
//////////////////

#if defined(STEVEN_SEAGAL)
#if defined(_DEBUG) || defined(_DEBUG_BUILD)
#define SS_DEBUG  
#endif
#define SS_CPU        // exceptions, instruction fixes, prefetch
#define SS_HATARI     // using some Hatari code
#define SS_IKBD       // keyboard, mouse, joystick
#define SS_INTERRUPT  // MFP, HBL, VBL
#define SS_OSD        // On Screen Display
#define SS_STF        // the switch Steem authors always refused to provide
#define SS_VARIOUS    // hacks; program ID; Mouse capture; MMU...
#define SS_VIDEO      // shifter tricks; large borders
#endif


///////////
// DEBUG //
///////////

#if defined(SS_DEBUG)

#define TRACE_MAX_WRITES 200000 // to avoid too big file
 
#if defined(_DEBUG) && defined(_VC_BUILD)

#define TRACE _trace // even in VC6, we use our TRACE to be independent of MFC
void _trace(char *fmt, ...);
#define ASSERT(x) {if(!(x)) _asm{int 0x03}}
#define VERIFY(x) {if(!(x)) _asm{int 0x03}}
#define BREAKPOINT _asm { int 3 }
#define BRK(x) {TRACE(#x); TRACE("\n");}

#else // for boiler

#define TRACE SSDebug.TraceToFile
#define BREAKPOINT {if(SSDebug.ReportBreakpoints) \
SSDebug.ReportBreakpoints=(Alert("Breakpoint! Click cancel to stop those boxes","BRK",MB_OKCANCEL)==IDOK);} 
#define ASSERT(x) {if(SSDebug.ReportBreakpoints&&(!(x))) { \
TRACE("Assert failed: %s\n",#x); \
SSDebug.ReportBreakpoints=(Alert(#x,"ASSERT",MB_OKCANCEL)==IDOK);}}
#define VERIFY ASSERT
#define BRK(x){if(SSDebug.ReportBreakpoints) { \
TRACE("Breakpoint: %s\n",#x); \
SSDebug.ReportBreakpoints=(Alert(#x,"Breakpoint",MB_OKCANCEL)==IDOK);}}

#endif

#else // release versions

#define BREAKPOINT 
#define VERIFY(x) x // I never use VERIFY
#define TRACE
#define ASSERT(x)
#define BRK(x) 

#endif // #if defined(SS_DEBUG)


#if defined(STEVEN_SEAGAL)


/////////////////
// PORTABILITY //
/////////////////

#if defined(UNIX)
#define TRUE 1
#define FALSE 0
#define BOOL int
#else
#define Uint8 BYTE
#define Uint16 WORD
#define Uint32 DWORD
#define Uint64 long // not used, just to make it compile
#endif

#if defined(_BCC_BUILD)
void DummyTrace(int, ...) {}
void DummyTrace2(char*, ...) {}
#pragma warn- 8004 
#pragma warn- 8012
#pragma warn- 8019
#pragma warn- 8027
#pragma warn- 8057
#pragma warn- 8071
#endif

#if defined(_VC_BUILD)
#pragma warning (disable : 4800) // 'int' : forcing value to bool 'true' or 'false' (performance warning)
#endif
 

/////////
// CPU //
/////////

#if defined(SS_CPU)  // Nice fixes here, all legit
#define SS_CPU_EXCEPTION    // crash like Windows 98
#define SS_CPU_PREFETCH     // fetch like a dog
#define SS_CPU_CLR          // read before writing
#define SS_CPU_DIV          // divide like Caesar
#define SS_CPU_MOVE_B       // move like a superstar
#define SS_CPU_MOVE_W
#define SS_CPU_MOVE_L
//#define SS_CPU_TAS

// change at leisure
#if defined(SS_CPU_EXCEPTION)
#if defined(_DEBUG)
#define EXCEPTIONS_REPORTED 25
#else // in file
#define EXCEPTIONS_REPORTED 155
#endif
#endif

// CPU inlined macros
extern inline void FetchWord(WORD &dest_word); 
extern inline void  HandleIOAccess();
#if defined(_VC_BUILD)
extern inline void set_pc(MEM_ADDRESS ad);
#else
extern /*inline*/ void set_pc(MEM_ADDRESS ad); // silly problem to investigate
#endif
#define SET_PC(x) set_pc(x)
extern inline void m68k_Process();
extern inline void m68k_perform_rte();
extern inline void FetchTiming();
extern inline void m68kReadBFromAddr();
extern inline void m68kReadWFromAddr();
extern inline void m68kReadLFromAddr();
extern inline void m68kSetDestBToAddr();
extern inline void m68kSetDestWToAddr();
extern inline void m68kSetDestLToAddr();
extern inline void m68kSetDestB(unsigned long addr);
extern inline void m68kSetDestW(unsigned long addr);
extern inline void m68kSetDestL(unsigned long addr);

#if defined(SS_CPU_PREFETCH)
inline void PrefetchIrc();
inline void RefetchIr();
#endif

struct TCpu {
  // functions
  TCpu();
#if defined(SS_CPU_PREFETCH)
  WORD FetchForCall(MEM_ADDRESS ad);
#endif
  // data
#if defined(SS_CPU_PREFETCH)
  BOOL CallPrefetch; 
  WORD *PrefetchAddress; 
  int PrefetchClass;
  WORD PrefetchedOpcode;
#endif
#if defined(SS_DEBUG)
  int nExceptions;
  int nInstr;
  WORD PreviousIr;
  bool NextIrFetched; // fetched next instruction?
#endif
};
extern TCpu Cpu;

#endif


//////////
// IKBD //
//////////

#if defined(SS_IKBD)
#define SS_IKBD_TIGHTER // quicker handling
#if defined(SS_HATARI)
#define SS_IKBD_HATARI // just for Dragonnels, Froggies, TB2 custom programs
#endif

#if defined(SS_IKBD_TIGHTER)
#define  ACIA_DELAY_HBL 12
extern BOOL trigger_ACIA_irq;
#endif

#if defined(SS_VARIOUS) // some IKBD hacks
#define SS_IKBD_SENTINEL
#if defined(SS_IKBD_HATARI)  
#define SS_IKBD_FROGGIES
// restarting Steem on this menu can crash in debug build, even if not defined
#define SS_IKBD_DRAGONNELS 
#endif
#endif

#if defined(SS_DEBUG)
//#define SS_IKBD_TRACE
#endif

#define IKBD_HBLS_FROM_COMMAND_WRITE_TO_PROCESS_ALT 2

#endif


///////////////
// INTERRUPT //
///////////////
// (also see video part)
#if defined(SS_INTERRUPT)

#define SS_MFP_PENDING // extending a present feature (hack or fix?)
#define SS_MFP_RATIO // change the values of CPU & MFP freq in STF mode
#define  MFP_CYCLES 56 // usual interrupt cycles = 56 (Steem, Hatari)
#define  HBL_CYCLES 56
#if defined(SS_VARIOUS) // hands-off hack, "fixes" Auto168, BBC52
#define  VBL_CYCLES (SpecificHacks ? 52 : 56)
#else
#define  VBL_CYCLES 56
#endif

#if defined(SS_MFP_RATIO) 
// Those values used in Steem 3.2 allow playing Lethal Xcess (STE)
// ratio = 3.2637954514114691404060324731328
#define  MFP_CLK_LE 2451 // Steem 3.2
#define  MFP_CLK_LE_EXACT 2451134 // Steem 3.2
#define  CPU_STE_TH 8000000 // Steem 3.2 
// Between 2451168 and 2451226 cycles, as measured by Steem authors:
#define  MFP_CLK_STE_EXACT 2451182 // unused
// Those values also are OK for Lethal Xcess (STF), and the rest
// ratio = 3.2638541666666666666666666666667
#define  CPU_STF_PAL 8021248 // ( 2^8 * 31333 )
#define  MFP_CLK_TH 2457
#define  MFP_CLK_TH_EXACT 2457600 // ( 2^15 * 3 * 5^2 )

extern double CpuMfpRatio;
extern DWORD CpuNormalHz;
#define CPU_CYCLES_PER_MFP_CLK CpuMfpRatio // !!!!!!!!!!!!!!!!!!!!!!!!!
#endif

#endif


/////////
// OSD //
/////////

#if defined(SS_OSD)
#define RED_LED_DELAY 1500 // Red floppy led for writing, in ms
#define HD_TIMER 100 // Yellow hard disk led (imperfect)
#endif


/////////////
// STF/STE //
/////////////

#if defined(SS_STF)
extern enum EST_type {STF,STE} ST_type;
EST_type SwitchSTType(EST_type); // adjust "hardware"
#endif


/////////////
// VARIOUS //
/////////////

#if defined(SS_VARIOUS)

#define SS_VAR_MMU_FIX // emulation fix (? Super Neo Demo Show)

#define WINDOW_TITLE "Steem Engine SSE 3.3.0" // check other places
// since there are no betas, the last number is for bug fixes

#if defined(SS_DEBUG)
enum EReportGeneralInfos {START,STOP} ;
int ReportGeneralInfos(EReportGeneralInfos when);
#endif

#if defined(WIN32)
#define SS_VAR_MOUSE_CAPTURE
#endif
#define SS_VAR_F12 // F12 starts/stops emulation like everywhere
#define SS_VAR_PROG_ID // Program ID

#if defined(SS_VAR_MOUSE_CAPTURE)
extern int CaptureMouse;
#endif

extern int SpecificHacks;
inline void CheckAgenda() ; // inlining a macro

#if defined(SS_VAR_PROG_ID) // used for hacks/better display
extern enum EProgram {
  NONE,
  BPOC,       // Best Part of the Creation
  DOLB,       // Death of the Left Border
  DRAGONNELS, // Dragonnels
  ENCH_LAND,  // Enchanted Land
  FOREST,     // aka SHFORSTV.EXE
  FROGGIES,   // Froggies over the Fence
  LX_STF,     // Lethal Xcess
  MOLZ,       // More or less zero
  MOLZ_SPIRAL,
  NBL,        // No Buddies Land
  NCG,        // No Cooper greetings
  NGC,        // Delirious 4
  NIGHTMARE,  // Delirious 4
  OMEGA,      // Omega demo fullscreen
  SENTINEL,   // The Sentinel
  ST_CNX,     // Punish your Machine/Let's do the twist again
  TEKILA,     // Delirious 4
  TB2_MENU,   // Transbeauce 2
  LAST_PRG
} Program;
extern int nProgramChanges;
EProgram SetProgram(EProgram new_program); // ID and report program...
#if defined(SS_DEBUG)
#define MAX_PROGRAM_CHANGES 20 // this limit only for reporting
extern char program_name[][20]; // see acc.cpp
#endif
#endif

#if defined(SS_DEBUG) // for temporary use
extern int debug0,debug1,debug2,debug3,debug4,debug5,debug6,debug7,debug8,debug9;
#endif

#endif

#if defined(SS_DEBUG)

struct TDebug {
  TDebug();
  ~TDebug();
#if !defined(_DEBUG) && defined(_DEBUG_BUILD)
  void TraceToFile(char *fmt, ...);  
#endif
  BOOL OutputTraceToFile; // can be disabled in Boiler
  BOOL ReportBreakpoints; // disabled when clicking cancel in the box
  BOOL TraceFileLimit; // stop TRACING to file at +-3+MB
  FILE *trace_file_pointer; 
  int nTrace;
};
extern TDebug SSDebug;

#endif


///////////
// VIDEO //
///////////

#if defined(SS_VIDEO)

#define SS_VID_BORDERS // 3 settings up to 416x280!

#if defined(SS_HATARI)
#define SS_VID_HATARI // this disables too much (TODO)
#endif

// interrupt mods depending on Hatari's video.c code
#if  defined(SS_INTERRUPT) && defined(SS_VID_HATARI)
#define SS_JITTER // replaces WOBBLE for HBL & VBL, same effect!
#define SS_MFP_TIMER_B // in run.cpp
#endif

// Some video/shifter hacks
#define SS_VID_BPOC // Best part of the Creation
#if defined(SS_VARIOUS) 
#define SS_VID_DISTORTION // Omega Full Overscan big big hack
#endif
#define SS_VID_DOLB // Death of the Left Border
#define SS_VID_NGC // D4/NGC
#define SS_VID_NIGHTMARE // D4/Nightmare
#define SS_VID_ST_CNX // Let's Do The Twist Again
#define SS_VID_TEKILA // D4/Tekila

#define  FIRST_VISIBLE_SCANLINE (-30)
#define  LAST_LINE_BEFORE_BOTTOM_BORDER 199
#define  BORDERS_NONE 0	// 00
#define  BORDERS_ON 1	// 01
#define  BORDERS_AUTO_OFF 2	// 10
#define  BORDERS_AUTO_ON 3	// 11

#if defined(SS_VID_BORDERS)
extern int BorderSize; // 0=Steem 3.2 1 Large 2 Very large
extern int SideBorderSize,BottomBorderSize;
#define SS_VID_BORDERS_HACKS // necessary especially for border 40
#define ORIGINAL_BORDER_SIDE 32
#define LARGE_BORDER_SIDE 40 // max for 800x600 display (fullscreen)
#define VERY_LARGE_BORDER_SIDE 48 // for the total 416 display
#define BORDER_SIDE SideBorderSize // !!!!!!!!!!!!!!!!!!!!!
#define BORDER_EXTRA (SideBorderSize-ORIGINAL_BORDER_SIDE) // 0 8 16, in pixels
#define ORIGINAL_BORDER_BOTTOM 40 
#define LARGE_BORDER_BOTTOM 45 // the best experience all around?
#define VERY_LARGE_BORDER_BOTTOM 50 // reveals some trash
#define BORDER_BOTTOM BottomBorderSize // !!!!!!!!!!!!!!!!!!!!!!!
int ChangeBorderSize(int size); // gui.cpp
#endif

// Shifter declaration
struct TShifter {
  // Code
  TShifter(); 
  ~TShifter();
  inline void AddExtraToShifterDrawPointerAtEndOfLine(unsigned long &extra);
  inline void AddFreqChange(int f);
  inline void AddResChange(int r);
  inline int CalcFreqIdx(int freq);
  inline int CheckFreqs(int t, int idx);
  void CheckOverscan();
  void CheckSideBorders();
  inline int DE(); // Display Enabled
#if defined(WIN32)
  inline int DrawBlackPixels(int first_pixel,int npixels) ;
  inline void DrawBufferedScanlineToVideo();
#endif
  void DrawScanlineTo(int cycles_since_hbl);
  inline void DrawScanlineToEnd();
  inline void GetPosition( int *pFrameCycles , int *pHBL , int *pLineCycles );
  inline MEM_ADDRESS ReadSDP(int cycles_since_hbl);
  inline int IncScanline();
  inline void SetRes(BYTE NewRes);
  inline void SetSync(BYTE NewSync);
  void Vbl();
  // Data
  int HblStartingFreq;
  int n0ByteLines;
  int Offset;
#if defined(WIN32)
  BYTE *ScanlineBuffer;
#endif
#if defined(SS_DEBUG)
  int BorderMaskTrace,OverscanModeTrace;
#endif
  int SteemBorderMask; 
  int SteemOverscanMode;
};
extern TShifter Shifter;

// Debug
#if defined(SS_DEBUG)
//#define SS_VIDEO_DRAW_DBG  // bypass new detection & drawing
//#define ASSERT_AT_XXX_VIDEO_EVENTS
//#define REPORT_AT_XXX_VIDEO_EVENTS // Only once in the session.
//#define REPORT_AFTER_200VBL
//#define SS_VID_REPORT_FIRST_EVENTS // also using XXX_VIDEO_EVENTS
  // #VBL to reset the shifter tricks info.
#if defined(KEEP_REPORTING_SHIFTER_TRICKS)
#define  VBL_TRACE_REPORT 400
#endif
#if defined(ASSERT_AT_XXX_VIDEO_EVENTS)||defined(REPORT_AT_XXX_VIDEO_EVENTS)\
  || defined(SS_VID_REPORT_FIRST_EVENTS)
#define  XXX_VIDEO_EVENTS 800
#endif

// Used to record & check one video event.
struct SVideoEvent {
  int m_Scanline; 
  int m_Cycle;
  char m_Type;
  int m_Value; 
  int Add(int scanline,int cycle, char type, int value); 
  int Check(int cycle, char type, int value);
  int Init();
};

// Record & analyse video events.
class TVideoEvents {
public:
  enum {MAX_EVENTS=210*32*2};
 int TriggerReport; // set 2 to ask a full report, then it's set FALSE again.
  TVideoEvents();
  int Add(int scanline, int cycle, char type, int value);
  int Init();
  int Report();
  int Vbl(); 
  struct SVideoEvent m_VideoEvent[MAX_EVENTS]; // it's public
private:
  int m_nEvents; // how many video events occurred this vbl?
  int m_nEventsPreviousVbl;
  int m_bChanged; // true if there's at least one change compared with last vbl
  int m_nReports;
};

extern TVideoEvents VideoEvents;

#endif// dbg

#endif// video

#else
//// if STEVEN_SEAGAL is NOT defined:

#endif//#ifdef STEVEN_SEAGAL

#endif// #ifndef STEVEN_SEAGAL_H 


