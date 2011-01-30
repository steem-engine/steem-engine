#define BLITTER_START_WAIT 8
#define BLITTER_END_WAIT 0

#define LOGSECTION LOGSECTION_BLITTER
//---------------------------------------------------------------------------
WORD Blitter_DPeek(MEM_ADDRESS ad)
{
  ad&=0xffffff;
  if (ad>=himem){
    if (ad>=MEM_IO_BASE){
      WORD RetVal=0xffff;
      TRY_M68K_EXCEPTION
        RetVal=io_read_w(ad);
      CATCH_M68K_EXCEPTION
      END_M68K_EXCEPTION
      return RetVal;
    }
    if (ad>=rom_addr){
      if (ad<rom_addr+tos_len) return ROM_DPEEK(ad-rom_addr);
    }
    if (ad>=MEM_EXPANSION_CARTRIDGE){
      if (cart && ad<MEM_EXPANSION_CARTRIDGE + 128*1024) return CART_DPEEK(ad-MEM_EXPANSION_CARTRIDGE);
    }
  }else{
    return DPEEK(ad);
  }
  return 0xffff;
}
//---------------------------------------------------------------------------
void Blitter_DPoke(MEM_ADDRESS abus,WORD x)
{
  abus&=0xffffff;
  if (abus>=MEM_IO_BASE){
    TRY_M68K_EXCEPTION
      io_write_w(abus,x);
    CATCH_M68K_EXCEPTION
    END_M68K_EXCEPTION
  }else if (abus>=MEM_FIRST_WRITEABLE && abus<himem){
    DPEEK(abus)=x;
  }
}
//---------------------------------------------------------------------------
inline void Blitter_ReadSource(MEM_ADDRESS SrcAdr)
{
  if (Blit.SrcXInc>=0){
    Blit.SrcBuffer<<=16;
    Blit.SrcBuffer|=Blitter_DPeek(SrcAdr);
  }else{
    Blit.SrcBuffer>>=16;
    Blit.SrcBuffer|=Blitter_DPeek(SrcAdr) << 16;
  }
}
//---------------------------------------------------------------------------
void Blitter_Start_Line()
{
  if (Blit.YCounter<=0){ // Blit finished?
    Blit.Busy=false;
    log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Blitter_Start_Line changing GPIP bit from "+
            bool(mfp_reg[MFPR_GPIP] & MFP_GPIP_BLITTER_BIT)+" to 0");
    mfp_gpip_set_bit(MFP_GPIP_BLITTER_BIT,0);
    Blit.HasBus=false;
#if BLITTER_END_WAIT!=0
    INSTRUCTION_TIME_ROUND(BLITTER_END_WAIT);
#endif
    log(Str("BLITTER: ")+HEXSl(old_pc,6)+" ------------- BLITTING DONE --------------");

#ifdef _DEBUG_BUILD
    if (stop_on_blitter_flag && runstate==RUNSTATE_RUNNING){
      runstate=RUNSTATE_STOPPING;
      runstate_why_stop="Blitter completed an operation";
    }
#endif
  }else{ //prepare next line

    Blit.Mask=Blit.EndMask[0];
    Blit.Last=0;

    if (Blit.FXSR){
      Blitter_ReadSource(Blit.SrcAdr);
      Blit.SrcAdr+=Blit.SrcXInc;
    }
  }
}
//---------------------------------------------------------------------------
void ASMCALL Blitter_Start_Now()
{
  ioaccess=0;
  Blit.Busy=true;
  log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Blitter_Start_Now changing GPIP bit from "+
          bool(mfp_reg[MFPR_GPIP] & MFP_GPIP_BLITTER_BIT)+" to 1");
  mfp_gpip_set_bit(MFP_GPIP_BLITTER_BIT,true);
  Blit.YCounter=Blit.YCount;
  /*Only want to start the line if not in the middle of one.*/
  if (WORD(Blit.XCounter-Blit.XCount)==0) Blitter_Start_Line();
  Blitter_Draw();
  check_for_interrupts_pending();
}
//---------------------------------------------------------------------------
void Blitter_Blit_Word()
{
  if (Blit.Busy==0) return;

  WORD SrcDat,DestDat=0,NewDat;  //internal data registers
  // The modes 0,3,12,15 are source only
  if (Blit.XCounter==1){
    Blit.Last=true;
    if (Blit.XCount>1) Blit.Mask=Blit.EndMask[2];
  }
  if ((Blit.Op % 5)!=0 && Blit.Hop>1){ //won't read source for 0,5,10,15 or Hop=0,1
    if (Blit.NFSR && Blit.Last){
      if (Blit.SrcXInc>=0){
        Blit.SrcBuffer<<=16;
      }else{
        Blit.SrcBuffer>>=16;
      }
    }else{
      Blitter_ReadSource(Blit.SrcAdr);
      INSTRUCTION_TIME_ROUND(4);
    }
    if (Blit.Last){
      Blit.SrcAdr+=Blit.SrcYInc;
    }else{
      if ((Blit.NFSR && Blit.XCounter==2)==0){
        Blit.SrcAdr+=Blit.SrcXInc;
      }
    }
  }
  switch (Blit.Hop){
    case 0:
      SrcDat=WORD(0xffff);
      break;
    case 1:
      if (Blit.Smudge){
        SrcDat=Blit.HalfToneRAM[WORD(Blit.SrcBuffer >> Blit.Skew) & (BIT_0 | BIT_1 | BIT_2 | BIT_3)];
      }else{
        SrcDat=Blit.HalfToneRAM[int(Blit.LineNumber)];
      }
      break;
    default:
      SrcDat=WORD(Blit.SrcBuffer >> Blit.Skew);
      if (Blit.Hop==3){
        if (Blit.Smudge==0){
          SrcDat&=Blit.HalfToneRAM[int(Blit.LineNumber)];
        }else{
          SrcDat&=Blit.HalfToneRAM[SrcDat & (BIT_0 | BIT_1 | BIT_2 | BIT_3)];
        }
      }
  }
  if (Blit.NeedDestRead || Blit.Mask!=0xffff){
    DestDat=Blitter_DPeek(Blit.DestAdr);
    NewDat=DestDat & WORD(~(Blit.Mask));
    INSTRUCTION_TIME_ROUND(4);
  }else{
    NewDat=0; //Blit.Mask is FFFF and we're in a source-only mode
  }

  switch (Blit.Op){
    case 0:
      NewDat|=WORD(0) & Blit.Mask; break;
    case 1:
      NewDat|=WORD(SrcDat & DestDat) & Blit.Mask; break;
    case 2:
      NewDat|=WORD(SrcDat & ~DestDat) & Blit.Mask; break;
    case 3:
      NewDat|=SrcDat & Blit.Mask; break;
    case 4:
      NewDat|=WORD(~SrcDat & DestDat) & Blit.Mask; break;
    case 5:
      NewDat|=DestDat & Blit.Mask; break;
    case 6:
      NewDat|=WORD(SrcDat ^ DestDat) & Blit.Mask; break;
    case 7:
      NewDat|=WORD(SrcDat | DestDat) & Blit.Mask; break;
    case 8:
      NewDat|=WORD(~SrcDat & ~DestDat) & Blit.Mask; break;
    case 9:
      NewDat|=WORD(~SrcDat ^ DestDat) & Blit.Mask; break;
    case 10:
      NewDat=DestDat^Blit.Mask; break;  // ~DestAdr & Blit.Mask
    case 11:
      NewDat|=WORD(SrcDat | ~DestDat) & Blit.Mask; break;
    case 12:
      NewDat|=WORD(~SrcDat) & Blit.Mask; break;
    case 13:
      NewDat|=WORD(~SrcDat | DestDat) & Blit.Mask; break;
    case 14:
      NewDat|=WORD(~SrcDat | ~DestDat) & Blit.Mask; break;
    case 15:
      NewDat|=WORD(0xffff) & Blit.Mask; break;
  }
  Blitter_DPoke(Blit.DestAdr,NewDat);
  INSTRUCTION_TIME_ROUND(4);
  if (Blit.Last){
    Blit.DestAdr+=Blit.DestYInc;
  }else{
    Blit.DestAdr+=Blit.DestXInc;
  }
  Blit.Mask=Blit.EndMask[1];

  if((--Blit.XCounter) <= 0){
    Blit.LineNumber+=char((Blit.DestYInc>=0) ? 1:-1);
    Blit.LineNumber&=15;
    Blit.YCounter--;
    Blit.YCount=(WORD)Blit.YCounter;
    Blit.XCounter=int(Blit.XCount ? Blit.XCount:65536);  //init blitter for line
    Blitter_Start_Line();
  }
}

void Blitter_Draw()
{
//  MEM_ADDRESS SrcAdr=Blit.SrcAdr,DestAdr=Blit.DestAdr;

//  Blit.YCounter=int(Blit.YCount ? Blit.YCount:65536);
  INSTRUCTION_TIME_ROUND(BLITTER_START_WAIT);

  if (Blit.YCount==0){  //see note in Blitter.txt - trying to restart with a ycount of zero results in no restart
/*
     * If the BUSY flag is
     * reset when the Y_Count is zero, the flag will remain clear
     * indicating BLiTTER completion and the BLiTTER won't be restarted.
*/
    Blit.Busy=false;
    log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Blitter_Draw YCount==0 changing GPIP bit from "+
         bool(mfp_reg[MFPR_GPIP] & MFP_GPIP_BLITTER_BIT)+" to 0");
    mfp_gpip_set_bit(MFP_GPIP_BLITTER_BIT,0);
    return;
  }else{
    Blit.YCounter=Blit.YCount;
  }

//  WORD SrcDat,DestDat,Mask,NewDat;
//  bool Last;
//  long Cycles=0;

  log(Str("BLITTER: ")+HEXSl(old_pc,6)+" ------------- BLITTING NOW --------------");
  log(EasyStr("SrcAdr=$")+HEXSl(Blit.SrcAdr,6)+", SrcXInc="+Blit.SrcXInc+", SrcYInc="+Blit.SrcYInc);
  log(EasyStr("DestAdr=$")+HEXSl(Blit.DestAdr,6)+", DestXInc="+Blit.DestXInc+", DestYInc="+Blit.DestYInc);
  log(EasyStr("XCount=")+int(Blit.XCount ? Blit.XCount:65536)+", YCount="+Blit.YCount);
  log(EasyStr("Skew=")+Blit.Skew+", NFSR="+(int)Blit.NFSR+", FXSR="+(int)Blit.FXSR);
  log(EasyStr("Hog=")+Blit.Hog+", Op="+Blit.Op+", Hop="+Blit.Hop);

  // Turn off the "assigned a value that is never used" warning
//#ifdef WIN32
//  #pragma option -w-aus-
//#endif
  Blit.HasBus=true;
  Blit.TimeToSwapBus=ABSOLUTE_CPU_TIME+64;

//  while(Blit.Busy){
//    Blitter_Blit_Word();
//  }

#ifdef _DEBUG_BUILD
  MEM_ADDRESS monitor_altered;
#endif

  while (runstate==RUNSTATE_RUNNING){
    while (cpu_cycles>0 && runstate==RUNSTATE_RUNNING){
      if (Blit.HasBus){
        Blitter_Blit_Word();
      }else{
        DEBUG_ONLY( pc_history[pc_history_idx++]=pc; )
        DEBUG_ONLY( if (pc_history_idx>=HISTORY_SIZE) pc_history_idx=0; )

#undef LOGSECTION
#define LOGSECTION LOGSECTION_CPU
        m68k_PROCESS
#undef LOGSECTION
#define LOGSECTION LOGSECTION_BLITTER

        CHECK_BREAKPOINT
      }
      if (Blit.Busy){
        if (Blit.Hog==0){ //not in hog mode, keep switching bus
          if (((ABSOLUTE_CPU_TIME-Blit.TimeToSwapBus)>=0)){
            Blit.HasBus=!(Blit.HasBus);
            if (Blit.HasBus){
              log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Swapping bus to blitter at "+ABSOLUTE_CPU_TIME);
            }else{
              log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Swapping bus to CPU at "+ABSOLUTE_CPU_TIME);
            }
            Blit.TimeToSwapBus+=64;
          }
        }
      }else{
        Blit.HasBus=false;
        break;
      }
    }

    if (cpu_cycles>0) break;
    if (Blit.Busy==0) break; //enough!

    DEBUG_ONLY( if (runstate!=RUNSTATE_RUNNING) break; )
    DEBUG_ONLY( mode=STEM_MODE_INSPECT; )

    while (cpu_cycles<=0){
      screen_event_vector();
      prepare_next_event();
      if (cpu_cycles>0) check_for_interrupts_pending();
    }
    CHECK_BREAKPOINT

    DEBUG_ONLY( mode=STEM_MODE_CPU; )
//---------------------------------------------------------------------------
  } //more CPU!



//#ifdef WIN32
//  #pragma option -w-aus
//#endif

//  if ((Blit.Op % 5)!=0 && Blit.Hop>1) Blit.SrcAdr=SrcAdr;
//  Blit.DestAdr=DestAdr;
//  Blit.YCount=0;
//  if (Blit.Hog) INSTRUCTION_TIME_ROUND(Cycles);
}
//---------------------------------------------------------------------------
BYTE Blitter_IO_ReadB(MEM_ADDRESS Adr)
{
#ifdef DISABLE_BLITTER
  exception(BOMBS_BUS_ERROR,EA_READ,Adr);
  return 0;
#else
  MEM_ADDRESS Offset=Adr-0xFF8A00;

  if (Offset<0x20){
    int nWord=(Offset/2);
    if (Offset & 1){  // Low byte
      return LOBYTE(Blit.HalfToneRAM[nWord]);
    }else{
      return HIBYTE(Blit.HalfToneRAM[nWord]);
    }
  }

  switch (Offset){
    case 0x20: return HIBYTE(Blit.SrcXInc);
    case 0x21: return LOBYTE(Blit.SrcXInc);
    case 0x22: return HIBYTE(Blit.SrcYInc);
    case 0x23: return LOBYTE(Blit.SrcYInc);
    case 0x24: return 0;
    case 0x25: return LOBYTE(HIWORD(Blit.SrcAdr));
    case 0x26: return HIBYTE(LOWORD(Blit.SrcAdr));
    case 0x27: return LOBYTE(Blit.SrcAdr);

    case 0x28: return HIBYTE(Blit.EndMask[0]);
    case 0x29: return LOBYTE(Blit.EndMask[0]);
    case 0x2A: return HIBYTE(Blit.EndMask[1]);
    case 0x2B: return LOBYTE(Blit.EndMask[1]);
    case 0x2C: return HIBYTE(Blit.EndMask[2]);
    case 0x2D: return LOBYTE(Blit.EndMask[2]);

    case 0x2E: return HIBYTE(Blit.DestXInc);
    case 0x2F: return LOBYTE(Blit.DestXInc);
    case 0x30: return HIBYTE(Blit.DestYInc);
    case 0x31: return LOBYTE(Blit.DestYInc);
    case 0x32: return 0;
    case 0x33: return LOBYTE(HIWORD(Blit.DestAdr));
    case 0x34: return HIBYTE(LOWORD(Blit.DestAdr));
    case 0x35: return LOBYTE(Blit.DestAdr);

    case 0x36: return HIBYTE(Blit.XCount);
    case 0x37: return LOBYTE(Blit.XCount);
    case 0x38: return HIBYTE(Blit.YCount);
    case 0x39: return LOBYTE(Blit.YCount);

    case 0x3A: return Blit.Hop;
    case 0x3B: return Blit.Op;

    case 0x3C: return BYTE(Blit.LineNumber | (Blit.Smudge << 5) | (Blit.Hog << 6) | (Blit.Busy<<7));
    case 0x3D: return BYTE(Blit.Skew | (Blit.NFSR << 6) | (Blit.FXSR << 7));
    case 0x3E:case 0x3F:return 0;
  }
  exception(BOMBS_BUS_ERROR,EA_READ,Adr);
  return 0;
#endif
}
//---------------------------------------------------------------------------
void Blitter_IO_WriteB(MEM_ADDRESS Adr,BYTE Val)
{
#ifdef DISABLE_BLITTER
  exception(BOMBS_BUS_ERROR,EA_WRITE,Adr);
  return;
#else
  MEM_ADDRESS Offset=Adr-0xFF8A00;
  if (Offset<0x3A && io_word_access==0){
    return;
  }

//  bool old_blit_primed=blit_primed;
//  blit_primed=true;

  if (Offset<0x20){
    int nWord=(Offset/2);
    if (Offset & 1){  // Low byte
      Blit.HalfToneRAM[nWord]=MAKEWORD(Val,HIBYTE(Blit.HalfToneRAM[nWord]));
    }else{
      Blit.HalfToneRAM[nWord]=MAKEWORD(LOBYTE(Blit.HalfToneRAM[nWord]),Val);
    }
    return;
  }
  switch (Offset){
    case 0x20: WORD_B_1(&Blit.SrcXInc)=Val;return;
    case 0x21: WORD_B_0(&Blit.SrcXInc)=BYTE(Val & ~1);
      log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter SrcXInc to "+Blit.SrcXInc);
      return;
    case 0x22: WORD_B_1(&Blit.SrcYInc)=Val;return;
    case 0x23: WORD_B_0(&Blit.SrcYInc)=BYTE(Val & ~1);
      log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter SrcYInc to "+Blit.SrcYInc);
      return;
    case 0x24: return;
    case 0x25: DWORD_B_2(&Blit.SrcAdr)=Val;
      log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter SrcAdr to "+HEXSl(Blit.SrcAdr,6));
      return;
    case 0x26: DWORD_B_1(&Blit.SrcAdr)=Val;
      log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter SrcAdr to "+HEXSl(Blit.SrcAdr,6));
      return;
    case 0x27: DWORD_B_0(&Blit.SrcAdr)=BYTE(Val & ~1);
      log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - set blitter SrcAdr to "+HEXSl(Blit.SrcAdr,6));
      return;

    case 0x28: WORD_B_1(Blit.EndMask)=Val;return;
    case 0x29: WORD_B_0(Blit.EndMask)=Val;return;
    case 0x2a: WORD_B_1(Blit.EndMask+1)=Val;return;
    case 0x2b: WORD_B_0(Blit.EndMask+1)=Val;return;
    case 0x2c: WORD_B_1(Blit.EndMask+2)=Val;return;
    case 0x2d: WORD_B_0(Blit.EndMask+2)=Val;return;

    case 0x2E: WORD_B_1(&Blit.DestXInc)=Val;return;
    case 0x2F: WORD_B_0(&Blit.DestXInc)=BYTE(Val & ~1);return;
    case 0x30: WORD_B_1(&Blit.DestYInc)=Val;return;
    case 0x31: WORD_B_0(&Blit.DestYInc)=BYTE(Val & ~1);return;
    case 0x32: return;
    case 0x33: DWORD_B_2(&Blit.DestAdr)=Val;
      log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter DestAdr to "+HEXSl(Blit.DestAdr,6));
      return;
    case 0x34: DWORD_B_1(&Blit.DestAdr)=Val;
      log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter DestAdr to "+HEXSl(Blit.DestAdr,6));
      return;
    case 0x35: DWORD_B_0(&Blit.DestAdr)=BYTE(Val & ~1);
      log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter DestAdr to "+HEXSl(Blit.DestAdr,6));
      return;

    case 0x36:
      WORD_B_1(&Blit.XCount)=Val;
      Blit.XCounter=Blit.XCount;
      if (Blit.XCounter==0) Blit.XCounter=65536;
      return;
    case 0x37:
      WORD_B_0(&Blit.XCount)=Val;
      Blit.XCounter=Blit.XCount;
      if (Blit.XCounter==0) Blit.XCounter=65536;
      log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter XCount to "+Blit.XCount);
      return;
    case 0x38:
      WORD_B_1(&Blit.YCount)=Val;
      return;
    case 0x39:
      WORD_B_0(&Blit.YCount)=Val;
      log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter YCount to "+Blit.YCount);
      return;

    case 0x3A: Blit.Hop=BYTE(Val & (BIT_0 | BIT_1));
               log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter Hop to "+Blit.Hop);
               return;
    case 0x3B: Blit.Op=BYTE(Val & (BIT_0 | BIT_1 | BIT_2 | BIT_3));
               Blit.NeedDestRead=(Blit.Op && (Blit.Op!=3) && (Blit.Op!=12) && (Blit.Op!=15));
               log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter Op to "+Blit.Op);
               return;

    case 0x3C:
      Blit.LineNumber=BYTE(Val & (BIT_0 | BIT_1 | BIT_2 | BIT_3));
      Blit.Smudge=bool(Val & BIT_5);
      Blit.Hog=bool(Val & BIT_6);

      if (Blit.Busy==0){
        if (Val & BIT_7){ //start new
          if (Blit.YCount) ioaccess|=IOACCESS_FLAG_DO_BLIT;
        }
      }else{ //there's already a blit in progress
        if (Val & BIT_7){ // Restart
          log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Blitter restarted - swapping bus to Blitter at "+ABSOLUTE_CPU_TIME);
          Blit.HasBus=true;
//          Blit.TimeToSwapBus=ABSOLUTE_CPU_TIME+64;
          Blit.TimeToSwapBus+=64;
        }else{ // Stop
          Blit.Busy=false;
          log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Blitter clear busy changing GPIP bit from "+
                  bool(mfp_reg[MFPR_GPIP] & MFP_GPIP_BLITTER_BIT)+" to 0");
          mfp_gpip_set_bit(MFP_GPIP_BLITTER_BIT,0);
        }
      }
      return;
    case 0x3D: Blit.Skew=BYTE(Val & (BIT_0 | BIT_1 | BIT_2 | BIT_3));
               Blit.NFSR=bool(Val & BIT_6);
               Blit.FXSR=bool(Val & BIT_7);
               log(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter Skew to "+Blit.Skew+", NFSR to "+(int)Blit.NFSR+", FXSR to "+(int)Blit.FXSR);
    case 0x3E:case 0x3F:return;
  }
  exception(BOMBS_BUS_ERROR,EA_WRITE,Adr);
#endif
}
//---------------------------------------------------------------------------
#undef LOGSECTION


