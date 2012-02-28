
extern "C" void ASMCALL Blitter_Start_Now();
extern void Blitter_Draw();

#ifdef IN_EMU
//---------------------------------------------------------------------------
struct _BLITTER_STRUCT{
  WORD HalfToneRAM[16];

  short SrcXInc,SrcYInc;
  MEM_ADDRESS SrcAdr;

  WORD EndMask[3];

  short DestXInc,DestYInc;
  MEM_ADDRESS DestAdr;

  WORD XCount,YCount;
  BYTE Hop,Op;

  char LineNumber;
  bool Smudge,Hog;//,Busy;

  BYTE Skew;
  bool NFSR,FXSR;

  DWORD SrcBuffer;

  bool Busy;

  int XCounter,YCounter; //internal counters
  WORD Mask;   //internal mask register
  bool Last;   //last flag

  bool HasBus;
  int TimeToSwapBus;

  bool NeedDestRead; //from Op


  bool InBlitter_Draw; //are we in the routine?

}Blit;

BYTE Blitter_IO_ReadB(MEM_ADDRESS);
void Blitter_IO_WriteB(MEM_ADDRESS,BYTE);
#endif

