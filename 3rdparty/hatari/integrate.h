// Hacks that help stealing code from Hatari (http://hatari.tuxfamily.org)
// for Steem SSE

#pragma once
#ifndef INTEGRATE_H
#define INTEGRATE_H

#if defined(_VC_BUILD)
#pragma warning (disable : 4002) // Hatari LOG macro
#endif

// video
#include "configuration.h"
#include "cycles.h"
#include "cycInt.h"
#include "screen.h"
#include "log.h"
#include "video.h"

BYTE IoMem_ReadByte(MEM_ADDRESS addr) {
  BYTE rv;
  switch(addr)
  {
  case 0xff820a:
    rv=(shifter_freq==50) ? b11111110:b11111100;
    break;
  case 0xff8201:
    rv=LOBYTE(HIWORD(xbios2));
    break;
  case 0xff8203:
    rv=HIBYTE(LOWORD(xbios2));
    break;
  case 0xff820d:
#if defined(SS_STF)
    rv=(ST_type==STF) ? 0 : LOBYTE(xbios2);
#else
    rv=LOBYTE(xbios2);
#endif
    break;
  case 0xff8260:
    rv=(BYTE)screen_res;
    break;
  case 0xff820f:
#if defined(SS_STF)
    rv=(ST_type==STE) ? (BYTE)shifter_fetch_extra_words : 0;
#else
    rv=(BYTE)shifter_fetch_extra_words;
#endif
    break;
  case 0xff8240:
    rv=LOBYTE(STpal[0]);
    break;
  case 0xfffa03:
    {
      int n=(addr-0xfffa01) >> 1;
      rv=mfp_reg[n];
    }
    break;
  default:
    BRK(IoMem_ReadByte error);
  }
  return rv;
}


void IoMem_WriteByte(MEM_ADDRESS adr,BYTE byte) {
  switch(adr)
  {
  case 0xff820a:
    shifter_freq=(byte&2) ? 50 : 60;
    break;
  case 0xff8260:
    screen_res=byte;
    break;
  default:
    BRK(IoMem_WriteByte error);
  }
}


WORD IoMem_ReadWord(MEM_ADDRESS adr) { return 0; }
void IoMem_WriteWord(MEM_ADDRESS adr,WORD) {}

struct TIoMem {
  BYTE rv;
  BYTE& operator[] (MEM_ADDRESS adr); // the power of C++...
};

BYTE& TIoMem::operator[] (MEM_ADDRESS adr) {
  rv=IoMem_ReadByte(adr); // we ignore writes
  return rv;
}
TIoMem IoMem;

inline int Cycles_GetCounter(int counter_id) {
  return ABSOLUTE_CPU_TIME-cpu_time_of_last_vbl;
}
#define Cycles_GetCounterOnWriteAccess Cycles_GetCounter
#define Cycles_GetCounterOnReadAccess Cycles_GetCounter

Uint8 *STRam,*pSTScreen;
Uint32 IoAccessCurrentAddress,nIoMemAccessSize;

Uint32 M68000_GetPC() {
  return pc;
}
#define  EXCEPTION_HBLANK     0x00000068
#define  EXCEPTION_VBLANK     0x00000070
#define M68000_EXC_SRC_AUTOVEC  2  /* Auto-vector exception (e.g. VBL) */
void M68000_Exception(int,int) {}
void DmaSnd_STE_HBL_Update() {}
void MemorySnapShot_Store(void*,int) {}
int SDL_SwapBE16(unsigned short) {return 0;}

void Keymap_DebounceAllKeys() {}
void Printer_CheckIdleStatus() {}
void ShortCut_ActKey() {}
void YMFormat_UpdateRecording() {}
void Sound_Update_VBL() {}
void Main_WaitOnVbl() {}

#if defined(_BCC_BUILD)
#define LOG_TRACE DummyTrace
#define LOG_TRACE_PRINT DummyTrace2
#else
#define LOG_TRACE(X)
#define LOG_TRACE_PRINT(X)
#endif
#define LOG_TRACE_LEVEL(X) (0)

	
void do_put_mem_word(void*,WORD w) {}

WORD do_get_mem_word(void*) {return 0;}
                     
/* 68000 operand sizes */
#define SIZE_BYTE  1
#define SIZE_WORD  2
#define SIZE_LONG  4

/* The 8 MHz CPU frequency */
#define CPU_FREQ   8012800                 
#define assert ASSERT 


int bUseVDIRes=0,VDIRes,VDIWidth,VDIPlanes,VDIHeight;
CNF_PARAMS ConfigureParams;                 /* List of configuration for the emulator */
unsigned short vfc_counter;
void CycInt_AcknowledgeInterrupt(void) {}
int PendingInterruptCount;
void __cdecl MFP_TimerB_EventCount_Interrupt(void) {}

unsigned char MFP_TBCR;
void __cdecl CycInt_AddRelativeInterrupt(int,int, interrupt_id){}
int nBorderPixelsRight;
int nBorderPixelsLeft;
void __cdecl Spec512_StartVBL(void){}
FRAMEBUFFER DummyFrameBuffer;
FRAMEBUFFER * pFrameBuffer =&DummyFrameBuffer;
void __cdecl HostScreen_update1(bool){}
void __cdecl HostScreen_renderEnd(void){}
void __cdecl VIDEL_ConvertScreenNoZoom(int,int,int,int){}
void __cdecl VIDEL_ConvertScreenZoom(int,int,int,int){}
int nScreenZoomY;
int nScreenZoomX;
bool __cdecl HostScreen_renderBegin(void){return false;}
void __cdecl HostScreen_setWindowSize(int,int,int){}
void __cdecl HostScreen_updatePalette(int){}
void __cdecl HostScreen_setPaletteColor(unsigned char,unsigned char,unsigned char,unsigned char){}
bool __cdecl Avi_RecordVideoStream(void){return false;}
bool bRecordingAvi;
void __cdecl Cycles_SetCounter(int,int){}
bool __cdecl Screen_Draw(void){return false;}
bool __cdecl VIDEL_renderScreen(void){return false;}
void __cdecl Spec512_StoreCyclePalette(unsigned short,unsigned long){}
void __cdecl VIDEL_ST_ShiftModeWriteByte(void){}
int CurrentInstrCycles;


// ikbd
#include "ikbd.h"
#include "joy.h"
#include "mfp.h"

#define ScanCodeState ST_Key_Down
#define Joy_GetStickData(arg) stick[arg]
void Main_EventHandler(){}
#define SPCFLAG_BRK 0x10
void M68000_SetSpecial(int){}
int bQuitProgram;
#define CALL_VAR(func)  { ((void(*)(void))func)(); }
void M68000_WaitState(int t) {
  INSTRUCTION_TIME_ROUND(t); // used?
}
void __cdecl CycInt_RemovePendingInterrupt( interrupt_id){}
bool __cdecl CycInt_InterruptActive( interrupt_id){return false;}
void __cdecl CycInt_AddAbsoluteInterrupt(int,int, interrupt_id){}
int JoystickSpaceBar;
unsigned char MFP_GPIP;
void __cdecl MFP_InputOnChannel(unsigned char,unsigned char,unsigned char *){}
unsigned char MFP_IERB;
unsigned char MFP_IPRB;
void __cdecl Log_Printf( LOGTYPE,char const *,...){}


#ifndef SS_VID_HATARI
int nVBLs=0;
void __cdecl Video_GetPosition(int *,int *,int *) {};
#endif

#endif//guard