// Steem Steven Seagal Edition
// SSEVideo.cpp

#if defined(SS_VID_HATARI)
// must be included before the header file because of the inline functions
#include "..\..\3rdparty\hatari\video.c" 
#endif

#include "SSEVideo.h"
TShifter Shifter; // singleton

#if defined(SS_VID_BORDERS)
int SideBorderSize=ORIGINAL_BORDER_SIDE; // 32
int BottomBorderSize=ORIGINAL_BORDER_BOTTOM; // 40
#endif

#if defined(SS_DEBUG)
TVideoEvents VideoEvents;  // singleton
#endif


/////////////////////////////
// Video - object TShifter //
/////////////////////////////

TShifter::TShifter() {
  ScanlineBuffer=NULL;
}


TShifter::~TShifter() {
}


void TShifter::CheckOverscan() {
  // Checks at end of scanline, called by event_scanline()
  int HatariBorderMask=0, FrameCycles, HblCounterVideo, LineCycles;
  GetPosition( &FrameCycles , &HblCounterVideo , &LineCycles );
#if defined(SS_VID_HATARI)
  Video_InterruptHandler_HBL();
  if(nScanlinesPerFrame<nEndHBL) // correction/integration
  {
    nScanlinesPerFrame=nEndHBL+1;
    ShifterFrame.ShifterLines[nHBL].StartCycle = cpu_timer_at_start_of_hbl-cpu_time_of_last_vbl;
  }
  HatariBorderMask=ShifterFrame.ShifterLines[nHBL-1].BorderMask; // notice -1
#endif
#if defined(SS_DEBUG)
  BorderMaskTrace|=HatariBorderMask|SteemBorderMask;
#if defined(SS_VID_HATARI)  
  OverscanModeTrace|=OverscanMode;
#endif
  OverscanModeTrace|=SteemOverscanMode;
#endif
#if defined(SS_VID_HATARI)  // finish side overscan (-2, +2)
  if(scan_y>=shifter_first_draw_line && scan_y<shifter_last_draw_line)
  {
    // Line -2
    if(HatariBorderMask & BORDERMASK_RIGHT_MINUS_2)
      shifter_draw_pointer-=2;  
    // Line +2 late detection
    if( (HatariBorderMask & BORDERMASK_LEFT_PLUS_2)
      && !(SteemBorderMask & BORDERMASK_LEFT_PLUS_2))
    {
      shifter_draw_pointer+=2;
    }
    // Cancel line +2
    if(!(HatariBorderMask & BORDERMASK_LEFT_PLUS_2)
      && (SteemBorderMask & BORDERMASK_LEFT_PLUS_2))
    {
      shifter_draw_pointer-=2; 
    }
  }
#endif
  // vertical overscan - experience says trust Steem for now
  int adjusted_scan_y=scan_y-n0ByteLines;
  BOOL on_overscan_limit=(adjusted_scan_y==FIRST_VISIBLE_SCANLINE
    || adjusted_scan_y==LAST_LINE_BEFORE_BOTTOM_BORDER);
  int SteemTest=0;
  int t,i=shifter_freq_change_idx;
  int freq_at_trigger=shifter_freq;
  // Steem test
  if(screen_res==ST_HIGH_RES) 
    freq_at_trigger=MONO_HZ;
  if(emudetect_overscans_fixed) // else? // didn't check that part
    freq_at_trigger=(on_overscan_limit) ? 60:0;
  else if(on_overscan_limit 
    && shifter_freq_at_start_of_vbl==50
    && freq_change_this_scanline)
  {
    t=cpu_timer_at_start_of_hbl+CYCLES_FROM_HBL_TO_RIGHT_BORDER_CLOSE+98; // 502
    i=CheckFreqs(t,shifter_freq_change_idx); // will not hang!
    freq_at_trigger=(i>-1) ? shifter_freq_change[i] : 0;
  }
  if(on_overscan_limit && freq_at_trigger==60
    && shifter_freq_at_start_of_vbl==50)
  {
    if(adjusted_scan_y==FIRST_VISIBLE_SCANLINE)
    {
      if(!(SteemOverscanMode & OVERSCANMODE_TOP))
        SteemTest|=OVERSCANMODE_TOP;
    }
    else
    {
      if(!(SteemOverscanMode & OVERSCANMODE_BOTTOM))
        SteemTest|=OVERSCANMODE_BOTTOM;
    }
  }
  // Top border
  if( !(SteemOverscanMode & OVERSCANMODE_TOP) // already removed!
    && ( 
#if defined(SS_VID_HATARI)
    (OverscanMode & OVERSCANMODE_TOP) ||
#endif
    (SteemTest & OVERSCANMODE_TOP)  ))
  {
    SteemOverscanMode|=OVERSCANMODE_TOP;
    shifter_first_draw_line=-29;
    overscan=OVERSCAN_MAX_COUNTDOWN;
    time_of_next_timer_b=time_of_next_event+cpu_cycles_from_hbl_to_timer_b
      +TB_TIME_WOBBLE; // we'll probably soon replace those
    if(FullScreen&&border==2)
    {    //hack overscans
      int off=shifter_first_draw_line-draw_first_possible_line;
      draw_first_possible_line+=off;
      draw_last_possible_line+=off;
    }
#if defined(SS_VID_HATARI)
    // if only detected by Steem, need to update Hatari variables
    if(!(OverscanMode & OVERSCANMODE_TOP))
    {
      OverscanMode |= OVERSCANMODE_TOP;
      nStartHBL = VIDEO_START_HBL_60HZ;
    }
#endif
  }
  // Bottom border 
  else if( !(SteemOverscanMode & OVERSCANMODE_BOTTOM) // already removed!
    && (
#if defined(SS_VID_HATARI)
    (OverscanMode & OVERSCANMODE_BOTTOM) ||
#endif
    (SteemTest & OVERSCANMODE_BOTTOM)  ))
  {
    SteemOverscanMode|=OVERSCANMODE_BOTTOM;
    overscan=OVERSCAN_MAX_COUNTDOWN;
    // Must be time of the next scanline or we don't get a Timer B on scanline 200!
    time_of_next_timer_b=time_of_next_event+cpu_cycles_from_hbl_to_timer_b+ TB_TIME_WOBBLE;
#if defined(SS_VID_HATARI)
    if(OverscanMode & OVERSCANMODE_BOTTOM)
    {
      // 2 kinds, 50hz & 60hz
      if(nEndHBL==VIDEO_END_HBL_50HZ+VIDEO_HEIGHT_BOTTOM_50HZ)
      {
        // Timer B will fire for the last time when scan_y is 246
        shifter_last_draw_line=247; // 50hz
      }
      else
        shifter_last_draw_line=229; // 60hz
    }
    else
    {
      shifter_last_draw_line=247;
      OverscanMode |= OVERSCANMODE_BOTTOM;
      nEndHBL = VIDEO_END_HBL_50HZ+VIDEO_HEIGHT_BOTTOM_50HZ;	
    }
#else
    shifter_last_draw_line=247;
#endif
  }
}


void TShifter::CheckSideBorders() {
// We may come here several times per scanline, at shifter events (not freq)
#if defined(SS_DEBUG) && defined(SS_VIDEO_DRAW_DBG)
  draw_check_border_removal(); // base function
  return;
#endif
 //draw_check_border_removal(); return; //dbg
  int HatariBorderMask=0, FrameCycles, HblCounterVideo, LineCycles;
  GetPosition( &FrameCycles , &HblCounterVideo , &LineCycles );
#if defined(SS_VID_HATARI)
  HatariBorderMask=ShifterFrame.ShifterLines[HblCounterVideo].BorderMask;
#endif
  int act=ABSOLUTE_CPU_TIME,t,i;
  if(screen_res>=ST_HIGH_RES
    || (scan_y<shifter_first_draw_line || scan_y>=shifter_last_draw_line)
    || !act) // ST is resetting
  {
    return;
  }

  ///////////////////////////////////
  //  LEFT BORDER OFF (line +26)   //
  ///////////////////////////////////
  
  if(left_border>0)
  {
#if !defined(SS_VID_HATARI) 
    // Steem test
    t=cpu_timer_at_start_of_hbl+2+stfm_borders; //trigger point
    if(act-t>0)
    {
      i=CheckFreqs(t,shifter_freq_change_idx);
      if(i>=0 && shifter_freq_change[i]==MONO_HZ)
        SteemBorderMask|=BORDERMASK_LEFT_OFF;
    }
#endif
    if((SteemBorderMask & BORDERMASK_LEFT_OFF)
      ||(HatariBorderMask & BORDERMASK_LEFT_OFF)
      ||(HatariBorderMask & BORDERMASK_LEFT_OFF_MED))
    {
#if defined(SS_VID_HATARI) 
      SteemBorderMask|=BORDERMASK_LEFT_OFF;
#endif
      left_border=0;
#if defined(SS_VARIOUS) && defined(SS_VAR_PROG_ID) && defined(SS_VID_BORDERS_HACKS)
      // the -4 shift is rarely necessary and is ugly - list to complete
      if(SpecificHacks && SideBorderSize==VERY_LARGE_BORDER_SIDE
        && ShifterFrame.ShifterLines[ HblCounterVideo ].DisplayPixelShift == -4
        && (Program==OMEGA||Program==TB2_MENU||Program==FOREST||Program==DRAGONNELS)) 
      {
#if defined(SS_VID_HATARI)
        ShifterFrame.ShifterLines[ HblCounterVideo ].DisplayPixelShift = 0;
#endif
      }
      else
#endif
        shifter_pixel+=4; // real emulation requires shift. Overscan#6
      overscan_add_extra+=OVERSCAN_ADD_EXTRA_FOR_LEFT_BORDER_REMOVAL; //+2
      overscan=OVERSCAN_MAX_COUNTDOWN; // for auto border
      if(shifter_hscroll>=12) // STE shifter bug
        shifter_draw_pointer+=8; // but don't compensate in overscan_add_extra
#if defined(SS_VID_BORDERS)
      if(SideBorderSize==LARGE_BORDER_SIDE) // 40
      {
         shifter_pixel+=8; // 4+20+2=26, and it's the only way
#if defined(SS_VARIOUS) && defined(SS_VID_BPOC)
         if(SpecificHacks && Program==BPOC)
           shifter_pixel+=4; // just a hack
#endif
      }
      else if(SideBorderSize==ORIGINAL_BORDER_SIDE) // 32
#endif
         shifter_draw_pointer+=8; // 8+16+2=26

#if defined(SS_VARIOUS) && defined(SS_STF) && defined(SS_VID_DOLB) && defined(SS_VID_HATARI)
      if(SpecificHacks && ST_type==STF 
        && (SteemBorderMask & BORDERMASK_LEFT_OFF)
        && FrameCycles - ShifterFrame.ResPosHi.FrameCycles==16
        && ShifterFrame.ResPosHi.LineCycles==512)
      {
#if defined(SS_VAR_PROG_ID)
        if(Program!=FOREST)
          SetProgram(DOLB);
#endif
        ShifterFrame.ShifterLines[ HblCounterVideo ].DisplayPixelShift = 0;
      }
#endif
    }
#if defined(SS_VID_HATARI)
    else
      SteemBorderMask&=~BORDERMASK_LEFT_OFF;
#endif
  }

  ///////////////////////////////////////////
  //  STE ONLY LEFT BORDER OFF (line +20)  //
  ///////////////////////////////////////////
  // TODO with fewer hacks!
#if defined(SS_VID_HATARI)
  if(HatariBorderMask & BORDERMASK_LEFT_OFF_2_STE 
    && !(SteemBorderMask & BORDERMASK_LEFT_OFF_2_STE))
  {
    SteemBorderMask|=BORDERMASK_LEFT_OFF_2_STE;
    overscan_add_extra=4;
#if defined(SS_VID_BORDERS_HACKS)
    if(SideBorderSize==ORIGINAL_BORDER_SIDE)
#endif
      left_border=0;
#if defined(SS_VID_BORDERS_HACKS)
    else
      left_border=4;
#endif
    overscan=OVERSCAN_MAX_COUNTDOWN;
    if(shifter_fetch_extra_words) // + STE scrolling (MOLZ/Spiral)
    {
#if defined(SS_VID_BORDERS_HACKS)
      if(SideBorderSize==ORIGINAL_BORDER_SIDE)
      {
#endif
        if(shifter_fetch_extra_words!=16)
          overscan_add_extra+=6;
#if defined(SS_VID_BORDERS_HACKS)        
      }
      else if(shifter_fetch_extra_words==16 && shifter_hscroll>6
        || shifter_fetch_extra_words==12 && shifter_hscroll>3)
        overscan_add_extra+=-8;
#endif
    }
  }
#endif

  //////////////////////////////////////////
  // BLANK LINE & 0 BYTE LINE (line -160) //
  //////////////////////////////////////////

  // Steem test, only for blank line
  if(shifter_freq_at_start_of_vbl==50 && !draw_line_off)
  {
    t=cpu_timer_at_start_of_hbl+28; //trigger point
    if(act-t>0)
    {
      i=CheckFreqs(t,shifter_freq_change_idx);
      if(i>=0 && shifter_freq_change[i]==60 && shifter_freq_change_time[i]==t)
        SteemBorderMask |= BORDERMASK_BLANK_LINE;
    }
  }
  if(!draw_line_off)
  {
    if((SteemBorderMask & BORDERMASK_BLANK_LINE) // we keep this one (Forest)
      || (HatariBorderMask & BORDERMASK_BLANK_LINE)
      || (HatariBorderMask & BORDERMASK_EMPTY_LINE)
      )
    {
#if defined(SS_VID_HATARI) 
      SteemBorderMask |= BORDERMASK_BLANK_LINE;
#endif
      draw_line_off=TRUE;
      memset(PCpal,0,sizeof(long)*16); // all colours black
#if defined(SS_VID_HATARI) 
      // 0-byte line (line -160)
      // (No Buddies Land, Nostalgic-O/Lemmings, D4/NGC)
      if((HatariBorderMask & BORDERMASK_EMPTY_LINE))
      {
        SteemBorderMask |= BORDERMASK_EMPTY_LINE;
        shifter_draw_pointer-=160;
        n0ByteLines++;
        if(!(OverscanMode & OVERSCANMODE_BOTTOM))
        {
          shifter_last_draw_line++; // don't miss overscan (D4/NGC)
          nScanlinesPerFrame++;
        }
#if defined(SS_VAR_PROG_ID)
        if(ShifterFrame.ResPosLo.LineCycles==508)
        {
          if(ShifterFrame.ResPosHi.LineCycles==500)
            SetProgram(NBL);
        }
        if( (ShifterFrame.FreqPos60.LineCycles == LINE_START_CYCLE_50
          ||ShifterFrame.FreqPos60.LineCycles == LINE_START_CYCLE_50+4)
          && ShifterFrame.FreqPos50.LineCycles==68)
          SetProgram(FOREST);
#endif
      }
#endif
    }
#if defined(SS_VID_HATARI) 
    else
      SteemBorderMask &= ~BORDERMASK_BLANK_LINE;
#endif
  }

  //////////////////////
  // MED RES OVERSCAN //
  //////////////////////
  // 20 for NCG   512R2 12R0 20R1
  // 28 for PYM/BPOC  512R2 12R0 28R1
  // 36 for NCG off lines 512R2 12R0 36R1 (strange!)
  // 16 for Drag/Reset
  // 12 & 16 for PYM/STCNX left off

#if defined(SS_VID_HATARI)
  if((HatariBorderMask & BORDERMASK_OVERSCAN_MED_RES)
    &&!(SteemBorderMask & BORDERMASK_OVERSCAN_MED_RES))
  {
    Offset=ShifterFrame.ResPosHi.LineCycles; // we use our offset
    if(Offset-ShifterFrame.ResPosMed.LineCycles>0) // was on previous line
      Offset=512-Offset+ShifterFrame.ResPosMed.LineCycles;
    else
      Offset=ShifterFrame.ResPosMed.LineCycles-Offset;
    SteemBorderMask|=BORDERMASK_OVERSCAN_MED_RES;
#if defined(SS_VAR_PROG_ID)
    if(Offset==28)
      SetProgram(BPOC);
    else if( (Offset==20 || Offset==36) && Program!=NGC) // D4/NGC comes here
      SetProgram(NCG);
#endif
#if defined(SS_VID_BORDERS_HACKS)
    if(SideBorderSize==LARGE_BORDER_SIDE // necessary
      && ShifterFrame.ResPosMed.LineCycles>ORIGINAL_BORDER_SIDE)
      shifter_draw_pointer-=(ShifterFrame.ResPosMed.LineCycles-ORIGINAL_BORDER_SIDE);
#endif
  }
#endif
  
  /////////////////////
  // 4BIT HARDSCROLL //
  /////////////////////
  // PYM/ST-CNX (lines 70-282; offset 8, 12, 16, 20)
  // D4/NGC (lines 76-231)
  // D4/Nightmare (lines 143-306, cycles 28 32 36 40)
  
#if defined(SS_VID_HATARI) 
#if defined(SS_STF)
  if(ST_type==STF
    && (HatariBorderMask & BORDERMASK_LEFT_OFF_MED)
    && !ShifterFrame.Res
    && ShifterFrame.ResPosLo.LineCycles<=32 
    && !(SteemBorderMask & BORDERMASK_4BIT_SCROLL) )
  {
#if defined(SS_VAR_PROG_ID)
    SetProgram(ST_CNX);
#endif
    SteemBorderMask |= BORDERMASK_4BIT_SCROLL;
    Offset=ShifterFrame.ResPosLo.LineCycles-ShifterFrame.ResPosMed.LineCycles;
  }
#endif
  if( (HatariBorderMask & BORDERMASK_LEFT_OFF)
    && (SteemBorderMask & BORDERMASK_OVERSCAN_MED_RES)
    && !ShifterFrame.Res
    && ShifterFrame.ResPosLo.LineCycles<=40 
    && ShifterFrame.ResPosLo.LineCycles>ShifterFrame.ResPosMed.LineCycles )
  {
    SteemBorderMask |= BORDERMASK_4BIT_SCROLL;
    Offset=ShifterFrame.ResPosLo.LineCycles-ShifterFrame.ResPosMed.LineCycles;
#if defined(SS_VID_BORDERS_HACKS)
    if(Offset>=16 && SideBorderSize==LARGE_BORDER_SIDE)
      shifter_draw_pointer+=4; // necessary with current system
#endif
#if defined(SS_VAR_PROG_ID)
    if(SpecificHacks)
    {
      int offset=ShifterFrame.ResPosHi.LineCycles;
      if(offset>ShifterFrame.ResPosLo.LineCycles) // was on previous line
        offset=512-offset+ShifterFrame.ResPosMed.LineCycles;
      else
        offset=ShifterFrame.ResPosMed.LineCycles-offset;
      ASSERT(offset==20||offset==24);
      SetProgram( (offset==20) ? NGC : NIGHTMARE );
    }
#endif
  }
#endif

  ///////////////////////////////////
  // SMALLER LEFT BORDER (line +2) //
  ///////////////////////////////////

#if !defined(SS_VID_HATARI) 
  // Steem test
  if(shifter_freq_at_start_of_vbl==50 && left_border==BORDER_SIDE)
  {
    t=cpu_timer_at_start_of_hbl+58; 
    if(act-t>0)
    {
      i=CheckFreqs(t,shifter_freq_change_idx);
      if(i>=0 && shifter_freq_change[i]==60)
          SteemBorderMask|=BORDERMASK_LEFT_PLUS_2;
    }
  }
#endif
  if(left_border!=BORDER_SIDE)
    ; // pass
  else if( (SteemBorderMask & BORDERMASK_LEFT_PLUS_2) 
    || (HatariBorderMask & BORDERMASK_LEFT_PLUS_2))
  {
#if defined(SS_VID_HATARI) 
    SteemBorderMask |= BORDERMASK_LEFT_PLUS_2;
#endif
    left_border-=4; // 2 bytes -> 4 cycles
    overscan_add_extra=OVERSCAN_ADD_EXTRA_FOR_SMALL_LEFT_BORDER_REMOVAL; // 2
    overscan=OVERSCAN_MAX_COUNTDOWN; // 25
  }
#if defined(SS_VID_HATARI) 
  else
    SteemBorderMask &= ~BORDERMASK_LEFT_PLUS_2;
#endif

  ///////////////////////////////////////
  //   BIG  RIGHT BORDER (line -106)   //
  ///////////////////////////////////////

  if(right_border_changed) 
    return; 

#if !defined(SS_VID_HATARI) 
  // Steem test
  t=cpu_timer_at_start_of_hbl+172; //trigger point for big right border
  if(act-t>=0)
  {
     i=CheckFreqs(t,shifter_freq_change_idx);
     if(i>=0 && shifter_freq_change[i]==MONO_HZ)
       SteemBorderMask|=BORDERMASK_STOP_MIDDLE;
  }
#endif
  if((SteemBorderMask & BORDERMASK_STOP_MIDDLE)
    || (HatariBorderMask & BORDERMASK_STOP_MIDDLE))
  {
#if defined(SS_VID_HATARI) 
    SteemBorderMask |= BORDERMASK_STOP_MIDDLE;
#endif
    overscan_add_extra+=OVERSCAN_ADD_EXTRA_FOR_GREAT_BIG_RIGHT_BORDER; // -106
    right_border_changed=true;
#if defined(SS_VID_HATARI) 
    if(HatariBorderMask & BORDERMASK_LEFT_PLUS_2 
      && SteemBorderMask & BORDERMASK_LEFT_PLUS_2)
    {     // It happens in Forest but doesn't seem to change display
      ShifterFrame.ShifterLines[HblCounterVideo].BorderMask &= ~BORDERMASK_LEFT_PLUS_2;
      SteemBorderMask&= ~BORDERMASK_LEFT_PLUS_2;
      left_border+=4;
      overscan_add_extra-=OVERSCAN_ADD_EXTRA_FOR_SMALL_LEFT_BORDER_REMOVAL; // 2
    }
#endif
  }
#if defined(SS_VID_HATARI) 
  else
    SteemBorderMask &= ~BORDERMASK_STOP_MIDDLE;
#endif
   
  ///////////////////////////////////
  // BIGGER RIGHT BORDER (line -2) //
  ///////////////////////////////////

  if( shifter_freq_at_start_of_vbl!=50
    && !(HatariBorderMask&BORDERMASK_RIGHT_MINUS_2)
    && !(HatariBorderMask&BORDERMASK_RIGHT_OFF ) )
  {
    return; 
  }
#if !defined(SS_VID_HATARI) 
  // Steem test
  t=cpu_timer_at_start_of_hbl+372; //trigger point for early right border
  if(act-t>=0)
  {
    i=CheckFreqs(t,shifter_freq_change_idx);
    if(i==-1
      && !(HatariBorderMask&BORDERMASK_RIGHT_MINUS_2)
      && !(HatariBorderMask&BORDERMASK_RIGHT_OFF) )
    {
      return; 
    }
    if(shifter_freq_change[i]==60)
    {
      // With an added logical condition
      if(shifter_freq_change_time[i]>cpu_timer_at_start_of_hbl)
        SteemBorderMask|=BORDERMASK_RIGHT_MINUS_2;
    }
  }
#endif  
  if((SteemBorderMask & BORDERMASK_RIGHT_MINUS_2)
    ||(HatariBorderMask & BORDERMASK_RIGHT_MINUS_2))
  {
#if defined(SS_VID_HATARI) 
    SteemBorderMask |= BORDERMASK_RIGHT_MINUS_2;
#else // action here, in CheckOverscan for Hatari analysis
    overscan_add_extra+=OVERSCAN_ADD_EXTRA_FOR_EARLY_RIGHT_BORDER;
    right_border_changed=true;
#endif
  }
#if defined(SS_VID_HATARI) 
  else
    SteemBorderMask &= ~BORDERMASK_RIGHT_MINUS_2;
#endif  

  /////////////////////////////////
  // RIGHT BORDER OFF (line +44) // 
  /////////////////////////////////

#if !defined(SS_VID_HATARI) 
  // Steem test; bizarre but generally working, like the rest
  t=cpu_timer_at_start_of_hbl+378; //trigger point for right border cut (SS:378!)
  if(!(SteemBorderMask & BORDERMASK_RIGHT_MINUS_2) // beware!
    //&&!(HatariBorderMask & BORDERMASK_RIGHT_MINUS_2)
    && act-t>=0)
  {
    i=CheckFreqs(t,shifter_freq_change_idx);
    if(i!=-1)
    {// it looks for the NEXT change too!
      if(i!=shifter_freq_change_idx)
      {
        int i1=(i+1) & 31;
        if(shifter_freq_change_time[i1]-t<=0) 
          i=i1;
      }
      if((shifter_freq_change[i]!=50)
        && (shifter_freq_change_time[i]-cpu_timer_at_start_of_hbl>0)) // added this...
        SteemBorderMask|=BORDERMASK_RIGHT_OFF;
    }
  }
#endif
#if defined(SS_VID_DISTORTION)
  // hack for Omega Full Overscan, part 2 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  if( SpecificHacks && (HatariBorderMask & BORDERMASK_RIGHT_OFF)
    && (HatariBorderMask & BORDERMASK_LEFT_PLUS_2)
    && Shifter.HblStartingFreq==60
    && (SteemBorderMask & BORDERMASK_FREQ_DISTORTION))
  { // hit at 372 on the real thing, then 376 other lines... maybe
    HatariBorderMask &= ~BORDERMASK_RIGHT_OFF; // cancel this
    SteemBorderMask&=~BORDERMASK_RIGHT_OFF;
    overscan_add_extra+=-4;
  }
#endif
  if((SteemBorderMask & BORDERMASK_RIGHT_OFF)
    ||(HatariBorderMask & BORDERMASK_RIGHT_OFF))
  {
#if defined(SS_VID_HATARI) 
    SteemBorderMask|=BORDERMASK_RIGHT_OFF;
#endif
    right_border=0;
    overscan_add_extra+=OVERSCAN_ADD_EXTRA_FOR_RIGHT_BORDER_REMOVAL;  // 28 (+16=44)
#if defined(SS_VID_BORDERS)
    if(SideBorderSize==VERY_LARGE_BORDER_SIDE)
      overscan_add_extra-=BORDER_EXTRA/2; // 20 + 24=44
#endif
    overscan=OVERSCAN_MAX_COUNTDOWN; // 25
    right_border_changed=true;
  }
#if defined(SS_VID_HATARI) 
  else
     SteemBorderMask&=~BORDERMASK_RIGHT_OFF;
#endif
}


void TShifter::DrawScanlineTo(int cycles_since_hbl) { // aka DST
#ifdef SS_VIDEO_DRAW_DBG
  draw_scanline_to(cycles_since_hbl); // base function
  return;
#endif
  if(screen_res>=ST_HIGH_RES) 
    return; // TODO: draw black pixels instead?
#ifndef NO_CRAZY_MONITOR
  if(extended_monitor || emudetect_falcon_mode!=EMUD_FALC_MODE_OFF) 
    return;
#endif
  int HatariBorderMask=0, FrameCycles, HblCounterVideo, LineCycles;
  GetPosition( &FrameCycles , &HblCounterVideo , &LineCycles );
#if defined(SS_VID_HATARI) 
  HatariBorderMask=ShifterFrame.ShifterLines[HblCounterVideo].BorderMask;
#endif
  int black_border_left=0;
  int black_border_right=0;
  if(HatariBorderMask||freq_change_this_scanline)
    CheckSideBorders(); 
  int pixels_in=cycles_since_hbl-52+BORDER_EXTRA; // 52: in case we're at 60hz
  if(pixels_in > BORDER_SIDE+320+BORDER_SIDE) 
    pixels_in=BORDER_SIDE+320+BORDER_SIDE;  // max 384, 400, 416
  if(pixels_in>=0)
  {
#ifdef WIN32 // prepare buffer & ASM routine
    if(draw_buffer_complex_scanlines && draw_lock)
    {
      if(scan_y>=draw_first_scanline_for_border 
        && scan_y<draw_last_scanline_for_border)
      {
        if(draw_store_dest_ad==NULL 
          && pixels_in<BORDER_SIDE+320+BORDER_SIDE)
        {
          draw_store_dest_ad=draw_dest_ad;
          ScanlineBuffer=draw_dest_ad=draw_temp_line_buf;
          draw_store_draw_scanline=draw_scanline;
        }
        if(draw_store_dest_ad) 
          draw_scanline=draw_scanline_1_line[screen_res];
      }
    }
#endif
    // lines with shifter fetching: draw from scanline_drawn_so_far to pixels_in
    if(scan_y>=shifter_first_draw_line && scan_y<shifter_last_draw_line)
    {
      int border1=0,border2=0,picture=0,hscroll=0; // parameters for ASM routine
      int picture_left_edge=left_border;
      if(left_border<0)
      {
        TRACE("Debug line %d left_border was %d pixels_in %d HatariBorderMask %X\n",
          HblCounterVideo,left_border,pixels_in,HatariBorderMask); 
        return; // may avoid crash
      }
      //last pixel from extreme left to draw of picture
      int picture_right_edge=BORDER_SIDE+320+BORDER_SIDE-right_border;
      if(pixels_in>picture_left_edge)
      { //might be some picture to draw = fetching RAM
        if(scanline_drawn_so_far>picture_left_edge)
        {
          picture=pixels_in-scanline_drawn_so_far;
          if(picture>picture_right_edge-scanline_drawn_so_far)
            picture=picture_right_edge-scanline_drawn_so_far;
        }
        else
        {
          picture=pixels_in-picture_left_edge;
          if(picture>picture_right_edge-picture_left_edge)
            picture=picture_right_edge-picture_left_edge;
        }
        if(picture<0)
          picture=0;
      }
      if(scanline_drawn_so_far<left_border)
      {
        if(pixels_in>left_border)
          border1=left_border-scanline_drawn_so_far;
        else
          border1=pixels_in-scanline_drawn_so_far; // we're not yet at end of border
        if(border1<0) 
          border1=0;
      }
      border2=pixels_in-scanline_drawn_so_far-border1-picture;
      if(border2<0) 
        border2=0;
      int old_shifter_pixel=shifter_pixel;
      shifter_pixel+=picture;
      MEM_ADDRESS nsdp=shifter_draw_pointer;
      if(screen_res==ST_LOW_RES) 
      {
        hscroll=old_shifter_pixel & 15;
        nsdp-=(old_shifter_pixel/16)*8;
        nsdp+=(shifter_pixel/16)*8;
#if defined(SS_VID_HATARI)
#if defined(SS_VARIOUS) && defined(SS_VAR_PROG_ID) && defined(SS_VID_DOLB) && defined(SS_VID_HATARI) && defined(SS_STF)
        // Death of the Left Border
        if(SpecificHacks
          && (SteemBorderMask & BORDERMASK_LEFT_OFF)
          && ST_type==STF
          && ShifterFrame.ShifterLines[HblCounterVideo].DisplayPixelShift==0
          && Program==DOLB)
        {
          shifter_draw_pointer+=4; // correct shift
          hscroll=0; // looks better
        }
#endif
        // 4bit scrolling
        if((SteemBorderMask & BORDERMASK_4BIT_SCROLL))
        {
          shifter_draw_pointer+=8-Offset/2; // shift (LOL) the shifter's pointer
#if defined(SS_VARIOUS)
          // Further adjustments of SDP for better experience (unimportant hacks)
          if(SpecificHacks && (HatariBorderMask & BORDERMASK_LEFT_OFF_MED)) 
          { // PYM/ST-CNX
#if defined(SS_VID_BORDERS_HACKS)
            if(SideBorderSize>=LARGE_BORDER_SIDE)
              shifter_draw_pointer+=8; 
#endif
          }
#if defined(SS_VAR_PROG_ID)
          else if(SpecificHacks && Program==NGC)
            shifter_draw_pointer+=8;
#endif
#endif
          int STF_PixelScroll=13+8-Offset-8; // shift scanline 
          hscroll-=STF_PixelScroll; // we use hscroll->quick
          if(hscroll<0)
          {
            if(picture>-hscroll)
            {
              picture+=hscroll;
              border1-=hscroll;
              hscroll=0;
            }
            else if(!picture)
              hscroll+=STF_PixelScroll;
            else
              BRK(rendering error MED RES hardscroll); // shouldn't happen
          }
        }
        // STE-only left off
        if(SteemBorderMask&BORDERMASK_LEFT_OFF_2_STE)
        {
          shifter_draw_pointer+=8; // centering
#if defined(SS_VID_BORDERS_HACKS)
          if(SideBorderSize==VERY_LARGE_BORDER_SIDE)
          {
            if(picture>4) // creating little colour 0 borders
              picture-=4,border1+=4;
            if(picture>4)
              picture-=6,border2+=6;
          }
#endif
        }
#endif
      }
      else if(screen_res==ST_MEDIUM_RES)
      {
        hscroll=(old_shifter_pixel*2) & 15;
        nsdp-=(old_shifter_pixel/8)*4;
        nsdp+=(shifter_pixel/8)*4;
#if defined(SS_VID_HATARI)
        // Med Res Overscan
        if((HatariBorderMask & BORDERMASK_OVERSCAN_MED_RES))
        {
          if(!((Offset-4)%16)) // 20, 36
            shifter_draw_pointer-=2; // shift for NCG, strange!
#if defined(SS_VID_BORDERS_HACKS) && defined(SS_VID_BPOC) && defined(SS_VARIOUS)
          if(SideBorderSize==LARGE_BORDER_SIDE 
            && (Program==BPOC||SpecificHacks) ) // just a better fit
            shifter_draw_pointer+=8;
#endif
        }
#endif
      }
      if(draw_lock) // draw_lock is set true in draw_begin(), false in draw_end()
      {
        // real lines
        if(scan_y>=draw_first_possible_line 
          && scan_y<draw_last_possible_line)
        {
          // actually draw it
          if(picture_left_edge<0) 
            picture+=picture_left_edge;
          AUTO_BORDER_ADJUST; // hack borders if necessary
          DEBUG_ONLY( shifter_draw_pointer+=debug_screen_shift; );
          if(hscroll>=16) // convert excess hscroll in SDP shift
          {
            shifter_draw_pointer+=8*(hscroll/16);
            hscroll-=16*(hscroll/16);
          }
          // call to appropriate ASSEMBLER routine!
          try 
          {
            draw_scanline(border1,picture,border2,hscroll); 
          }
          catch(...) 
          {
            TRACE("Exception while drawing\n");
          }

#ifdef WIN32 // black borders hack
#if defined(SS_VARIOUS) && defined(SS_VID_BORDERS_HACKS) && defined(SS_VAR_PROG_ID)
          // Hide trash in the left border caused by MED RES shifter tricks
          if(SpecificHacks && SideBorderSize>=LARGE_BORDER_SIDE
            && ( (SteemBorderMask & BORDERMASK_OVERSCAN_MED_RES)
            || (SteemBorderMask & BORDERMASK_4BIT_SCROLL) )
            && (Program==NCG || Program==NGC /*|| Program==ST_CNX */
            || Program==NIGHTMARE))
          { // not very beautiful but better than the mess it hides
            black_border_left=BORDER_EXTRA;
          }
#endif
#if defined(SS_VID_BORDERS) && defined(SS_VID_HATARI)     
          // left border removal shift, we must compensate on the right
          if(SideBorderSize==VERY_LARGE_BORDER_SIDE && !black_border_right 
            && (SteemBorderMask & BORDERMASK_LEFT_OFF)
            && ShifterFrame.ShifterLines[ HblCounterVideo ].DisplayPixelShift == -4
            &&shifter_freq_at_start_of_vbl==50)
          {
            black_border_right=4; // 2 bytes lost
          }
#endif
          if(black_border_left>0)
            DrawBlackPixels(1,black_border_left);
          if(black_border_right>0)
          {
            int right_edge=(1+screen_res)*(320+2*BORDER_SIDE);
            DrawBlackPixels(right_edge-black_border_right+1,black_border_right);
          }
#endif
        }
      }
      shifter_draw_pointer=nsdp;
      if(pixels_in>=picture_right_edge 
        && scanline_drawn_so_far<picture_right_edge)
      {
        // adjust SDP according to shifter tricks
        AddExtraToShifterDrawPointerAtEndOfLine(shifter_draw_pointer);
      }
    }
    // overscan lines = a big "left border"
    else if(scan_y>=draw_first_scanline_for_border 
      && scan_y<draw_last_scanline_for_border)
    {
      int border1; // the only var. sent to draw_scanline
      int left_visible_edge,right_visible_edge;
      // Borders on
      if(border & 1)
      {
        left_visible_edge=0;
        right_visible_edge=BORDER_SIDE + 320 + BORDER_SIDE;
      }
      // No, only the part between the borders
      else 
      {
        left_visible_edge=BORDER_SIDE;
        right_visible_edge=320+BORDER_SIDE;
      }
      if(scanline_drawn_so_far<=left_visible_edge)
        border1=pixels_in-left_visible_edge;
      else
        border1=pixels_in-scanline_drawn_so_far;
      if(border1<0)
        border1=0;
      else if(border1> (right_visible_edge - left_visible_edge))
        border1=(right_visible_edge - left_visible_edge);
      if(scan_y>=draw_first_possible_line && scan_y<draw_last_possible_line)
        draw_scanline(border1,0,0,0);
    }
    scanline_drawn_so_far=pixels_in;
  }//if(pixels_in>=0)
}


void TShifter::Vbl() {
  SteemOverscanMode=n0ByteLines=0;
#if defined(SS_DEBUG)
  VideoEvents.Vbl(); 
#endif
#if defined(SS_VID_HATARI) 
  Video_InterruptHandler_VBL();
#endif
}


#if defined(STEVEN_SEAGAL) && defined(SS_DEBUG) && defined(SS_VIDEO)

// Each shifter event of the frame is recorded. A pretty useful report can be 
// issued in a debug build (assign a key), or you can break & trace.

////////////////////////////////
// Video - object SVideoEvent //
////////////////////////////////

int SVideoEvent::Add(int scanline,int cycle, char type, int value) {
  // return true if needed to change something
  // false if this event was exactly as in the parameters
  int rv=0;
  if(m_Scanline!=scanline)
    m_Scanline=scanline, rv++;
  if(m_Cycle!=cycle)
    m_Cycle=cycle, rv++;
  if(m_Type!=type)
    m_Type=type, rv++;
  if(m_Value!=value)
    m_Value=value, rv++;
  return rv;
}


int SVideoEvent::Check(int cycle, char type, int value) {
  int rv=(m_Cycle==cycle && m_Type==type && m_Value==value);
  return rv;
}


int SVideoEvent::Init() {
  m_Scanline=-1; // 0 is a real value (contrary to my habits)
  m_Cycle=-1; // 0 is a real value
  m_Type='N';//NONE;
  m_Value=-1; // 0 is a real value
  return TRUE;
}


/////////////////////////////////
// Video - object TVideoEvents //
/////////////////////////////////

TVideoEvents::TVideoEvents() {
  Init();
} 


int TVideoEvents::Add(int scanline, int cycle, char type, int value) {
  int rv;
  //if(type=='P') return FALSE;
  m_nEvents++; // starting from 0 each VBL
  ASSERT(m_nEvents<=MAX_EVENTS);
  rv=m_VideoEvent[m_nEvents].Add(scanline, cycle, type, value);
  if(rv)
      m_bChanged++;
#if defined(SS_DEBUG) && (defined(ASSERT_AT_XXX_VIDEO_EVENTS)\
  ||defined(REPORT_AT_XXX_VIDEO_EVENTS)|| defined(SS_VID_REPORT_FIRST_EVENTS))
  // Place here your breakpoints 
  if(m_nEvents==XXX_VIDEO_EVENTS)
  {
#ifdef REPORT_AT_XXX_VIDEO_EVENTS
    Report();
#endif
#ifdef ASSERT_AT_XXX_VIDEO_EVENTS
    BREAKPOINT;
#endif
  }
#endif
  return rv;
}


int TVideoEvents::Init() {
  m_nEventsPreviousVbl=m_nEvents=m_bChanged=m_nReports=TriggerReport=0;
  int i;
  for(i=0;i<MAX_EVENTS;i++)
    m_VideoEvent[i].Init();
  return TRUE;
}


int TVideoEvents::Report() {
  // A really helpful feature for debugging - assign to shortcut
  TRACE("Saving video timings...\n");
  FILE* fp;
  fp=fopen("shifter_tricks.txt","w"); // only one file name...
  ASSERT(fp);
  if(fp)
  {
    fprintf(fp,"Steem shifter tricks report\n");
#if defined(SS_STF)
    fprintf(fp,"Machine is ST%c ; ",ST_type==STF? 'F' : 'E');
#endif
    //EasyStr tmp=FloppyDrive[0].DiskName.c_str();
    fprintf(fp,"Floppy disk in A is %s",FloppyDrive[0].DiskName.c_str()); 
#if defined(SS_VARIOUS)  && defined(SS_VAR_PROG_ID)
    fprintf(fp," Current identified program: %s\n",program_name[Program]);
#endif
#if defined(SS_VARIOUS)
    fprintf(fp," Hacks: ");
    if(SpecificHacks)
      fprintf(fp,"ON");
    else
      fprintf(fp,"OFF");
#endif
#if defined(SS_VIDEO) && defined(SS_VID_BORDERS)
    fprintf(fp," Borders: %d",BorderSize);
#endif
    fprintf(fp,"\n");
    fprintf(fp,"Video events %d to %d:",1,m_nEvents);
    int i,j;
    for(i=1,j=-1;i<=m_nEvents;i++)
    {
      if(m_VideoEvent[i].m_Scanline!=j)
      {
        j=m_VideoEvent[i].m_Scanline;
#if defined(SS_VID_HATARI) 
        fprintf(fp,"\nLine %03d - Mask: %X - Events: ",j,ShifterFrame.ShifterLines[j].BorderMask);
#else
        fprintf(fp,"\nLine %03d - Events: ",j);
#endif
      }
      if(m_VideoEvent[i].m_Type=='P')
        fprintf(fp," %03d:%c%03X",m_VideoEvent[i].m_Cycle,m_VideoEvent[i].m_Type,m_VideoEvent[i].m_Value);
      else
        fprintf(fp," %03d:%c%03d",m_VideoEvent[i].m_Cycle,m_VideoEvent[i].m_Type,m_VideoEvent[i].m_Value);
    }//nxt
    fclose(fp);
  }
  m_nReports++;


  return m_nReports;
}


int TVideoEvents::Vbl() {
  int rv= TriggerReport;
  if(TriggerReport==2 && m_nEvents)
    TriggerReport--;
  else if(TriggerReport==1 && m_nEvents)
  {
    Report();
    TriggerReport=FALSE;
  }
#if defined(SS_VID_REPORT_FIRST_EVENTS) && defined(SS_DEBUG)
  if(m_nEvents>=XXX_VIDEO_EVENTS && !m_nReports)
  {
    Report();
    m_nReports++;
  }
#endif
#if defined(REPORT_AFTER_200VBL) && defined(SS_DEBUG) // old way
  if(rv)
  {
    Report();
    m_nReports=-1;
  }
  else if(m_nReports>-1);
    m_nReports++;
#endif
  if(m_nEvents>0)
    m_nEventsPreviousVbl=m_nEvents;
  m_bChanged=0;
  m_nEvents=0;
  return rv;
}

#endif//debug video

