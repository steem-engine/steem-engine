// Steem Steven Seagal Edition
// SSEVideo.h

#if !defined(SS_VID_HATARI)
#include "..\..\3rdparty\hatari\integrate.h"
// also used in Steem border tests
#define BORDERMASK_NONE			0x00	/* no effect on this line */
#define BORDERMASK_LEFT_OFF		0x01	/* removal of left border with hi/lo res switch -> +26 bytes */
#define BORDERMASK_LEFT_PLUS_2		0x02	/* line starts earlier in 60 Hz -> +2 bytes */
#define BORDERMASK_STOP_MIDDLE		0x04	/* line ends in hires at cycle 160 -> -106 bytes */
#define BORDERMASK_RIGHT_MINUS_2	0x08	/* line ends earlier in 60 Hz -> -2 bytes */
#define BORDERMASK_RIGHT_OFF		0x10	/* removal of right border -> +44 bytes */
#define BORDERMASK_RIGHT_OFF_FULL	0x20	/* full removal of right border and next left border -> +22 bytes */
#define BORDERMASK_OVERSCAN_MED_RES	0x40	/* some borders were removed and the line is in med res instead of low res */
#define BORDERMASK_EMPTY_LINE		0x80	/* 60/50 Hz switch prevents the line to start, video counter is not incremented */
#define BORDERMASK_LEFT_OFF_MED		0x100	/* removal of left border with hi/med res switch -> +26 bytes (for 4 pixels hardware scrolling) */
#define BORDERMASK_LEFT_OFF_2_STE	0x200	/* shorter removal of left border with hi/lo res switch -> +20 bytes (STE only)*/
#define BORDERMASK_BLANK_LINE		0x400	/* 60/50 Hz switch blanks the rest of the line, but video counter is still incremented */
#endif
// we add:
#define BORDERMASK_4BIT_SCROLL 0x2000 // we work with an offset for this
#if defined(SS_VID_DISTORTION) 
#define BORDERMASK_FREQ_DISTORTION 0x4000 // change sync while DE is on (hack)
#endif

inline void TShifter::AddExtraToShifterDrawPointerAtEndOfLine(unsigned long &extra) {
  // What a beautiful name!
  // Replacing macro ADD_EXTRA_TO_SHIFTER_DRAW_POINTER_AT_END_OF_LINE(s)
  extra+=(shifter_fetch_extra_words)*2;     
  if(shifter_skip_raster_for_hscroll)
    extra+= (left_border) ? 8 : 2;
  extra+=overscan_add_extra;
}


inline void TShifter::AddFreqChange(int f) {
  // Replacing macro ADD_SHIFTER_FREQ_CHANGE(shifter_freq)
  shifter_freq_change_idx++;
  shifter_freq_change_idx&=31;
  shifter_freq_change_time[shifter_freq_change_idx]=ABSOLUTE_CPU_TIME;
  shifter_freq_change[shifter_freq_change_idx]=f;                    
}


inline int TShifter::CalcFreqIdx(int freq) {
  // Replacing macro CALC_SHIFTER_FREQ_IDX
  int idx; 
  if(freq==50)
    idx=0;
  else if(freq==60)
    idx=1;
  else
    idx=2;
  return idx;
}


inline int TShifter::CheckFreqs(int t, int idx) {
  int i=idx;
  while(shifter_freq_change_time[i]-t>0) // avoid direct a>b...
  {
    i--;
    i&=31; // forcing 0-31
    if(i==idx)     // looped back!
    { 
      i=-1; // we didn't find a possible event
      break; 
    }
  }//wend
  return i; 
}


inline int TShifter::DE() {
  // DE=Display Enabled (ST video RAM being fetched and drawn)
  int rv=0;
  int cycles_since_hbl=ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl;
  if(!(SteemBorderMask & BORDERMASK_BLANK_LINE) && cycles_since_hbl<372)
  {
    MEM_ADDRESS CurrentSDP=ReadSDP(cycles_since_hbl); // TODO no ReadSDP?
    if(CurrentSDP>shifter_draw_pointer_at_start_of_line) 
      rv=1;
  }
  return rv;
}


#ifdef WIN32

extern int WinSizeForRes[4]; // forward

inline int TShifter::DrawBlackPixels(int first_pixel,int npixels) {
  if(screen_res==2)
    return FALSE;
  int last_pixel=(BORDER_SIDE+320+BORDER_SIDE)*(1+screen_res);
  ASSERT(first_pixel>0 && first_pixel+npixels-1<=last_pixel);
  int repeats=WinSizeForRes[screen_res]+1; // 2 commonest
  if(repeats<2)
    return FALSE;
  BYTE *ptr;
  // If the asm function drew in a buffer, this is where we must draw too
  if(ScanlineBuffer)
    ptr=ScanlineBuffer;
  // If not, we compute the start of the scanline in the PC drawing zone
  else
  {
    ptr=draw_dest_next_scanline; 
    ptr-=draw_dest_increase_y; 
  }
  // Get the starting pixel.
  ptr+=(first_pixel-1)*BytesPerPixel*repeats;
  // Zero memory for npixels.
  memset(ptr,0,npixels*BytesPerPixel*repeats);
  // The same for doubled line. If we're drawing in the buffer,
  // it will be doubled by the routine in DrawScanlineToEnd 
  // copying it to the drawing zone.
  if(draw_dest_increase_y>draw_line_length
    && !TShifter::ScanlineBuffer)
  {
    ASSERT(draw_dest_increase_y>draw_line_length);
    ptr+=draw_line_length;
    memset(ptr,0,npixels*BytesPerPixel*repeats);
  }
  return TRUE;
}


inline void TShifter::DrawBufferedScanlineToVideo() {
  // replacing macro DRAW_BUFFERED_SCANLINE_TO_VIDEO
  if(draw_store_dest_ad)
  { 
    // Bytes that will be copied.
    int amount_drawn=(int)(draw_dest_ad-draw_temp_line_buf); 
    // From draw_temp_line_buf to draw_store_dest_ad
    DWORD *src=(DWORD*)draw_temp_line_buf; 
    DWORD *dest=(DWORD*)draw_store_dest_ad;  
    while(src<(DWORD*)draw_dest_ad) 
      *(dest++)=*(src++); 
    if(draw_med_low_double_height)
    {
      src=(DWORD*)draw_temp_line_buf;                        
      dest=(DWORD*)(draw_store_dest_ad+draw_line_length);      
      while(src<(DWORD*)draw_dest_ad) 
        *(dest++)=*(src++);       
    }                                                              
    draw_dest_ad=draw_store_dest_ad+amount_drawn;                    
    draw_store_dest_ad=NULL;                                           
    draw_scanline=draw_store_draw_scanline; 
  }
  ScanlineBuffer=NULL;
}

#endif //WIN32


inline void TShifter::DrawScanlineToEnd()  {
#ifdef SS_VIDEO_DRAW_DBG
  draw_scanline_to_end();
  return;
#endif
  MEM_ADDRESS nsdp; 
#ifndef NO_CRAZY_MONITOR // SS: we don't care about this yet
  if (emudetect_falcon_mode!=EMUD_FALC_MODE_OFF){
    int pic=320*emudetect_falcon_mode_size,bord=0;
    // We double the size of borders too to keep the aspect ratio of the screen the same
    if (border & 1) bord=BORDER_SIDE*emudetect_falcon_mode_size;
    if (scan_y>=draw_first_possible_line && scan_y<draw_last_possible_line){
      bool in_border=(scan_y>=draw_first_scanline_for_border && scan_y<draw_last_scanline_for_border);
      bool in_pic=(scan_y>=shifter_first_draw_line && scan_y<shifter_last_draw_line);
      if (in_pic || in_border){
        // We only have 200 scanlines, but if emudetect_falcon_mode_size==2
        // then the picture must be 400 pixels high. So to make it work we
        // draw two different lines at the end of each scanline.
        // To make it more confusing in some drawing modes if
        // emudetect_falcon_mode_size==1 we have to draw two identical
        // lines. That is handled by the draw_scanline routine. 
        for (int n=0;n<emudetect_falcon_mode_size;n++){
          if (in_pic){
            nsdp=shifter_draw_pointer + pic*emudetect_falcon_mode;
            draw_scanline(bord,pic,bord,shifter_hscroll);
            AddExtraToShifterDrawPointerAtEndOfLine(nsdp);
            shifter_draw_pointer=nsdp;
          }else{
            draw_scanline(bord+pic+bord,0,0,0);
          }
          draw_dest_ad=draw_dest_next_scanline;
          draw_dest_next_scanline+=draw_dest_increase_y;
        }
      }
    }
    shifter_pixel=shifter_hscroll; //start by drawing this pixel
    scanline_drawn_so_far=BORDER_SIDE+320+BORDER_SIDE;
  }else if (extended_monitor){
    int h=min(em_height,Disp.SurfaceHeight);
    int w=min(em_width,Disp.SurfaceWidth);
    if (extended_monitor==1){	// Borders needed, before hack
      if (em_planes==1){ //mono
        int y=h/2-200, x=(w/2-320)&-16;
        if (scan_y<h){
          if (scan_y<y || scan_y>=y+400){
            draw_scanline(w/16,0,0,0);
          }else{
            draw_scanline(x/16,640/16,w/16-x/16-640/16,0);
            shifter_draw_pointer+=80;
          }
        }
      }else{
        int y=h/2-100, x=(w/2-160)&-16;
        if (scan_y<h){
          if (scan_y<y || scan_y>=y+200){
            draw_scanline(w,0,0,0);
          }else{
            draw_scanline(x,320,w-x-320,0);
            shifter_draw_pointer+=160;
          }
        }
      }
      draw_dest_ad=draw_dest_next_scanline;
      draw_dest_next_scanline+=draw_dest_increase_y;
    }else{
      if (scan_y<h){
        if (em_planes==1) w/=16;
        if (screen_res==1) w/=2; // medium res routine draws two pixels for every one w
        draw_scanline(0,w,0,0);
        draw_dest_ad=draw_dest_next_scanline;
        draw_dest_next_scanline+=draw_dest_increase_y;
      }
      int real_planes=em_planes;
      if (screen_res==1) real_planes=2;
      shifter_draw_pointer+=em_width*real_planes/8;
    }
  }else
#endif// #ifndef NO_CRAZY_MONITOR
  
  // Colour.
  if(screen_res<ST_HIGH_RES)
  {
    // draw to the end
#if defined(SS_DEBUG)    // could avoid some bad crashes
    try
    {
      DrawScanlineTo(CYCLES_FROM_HBL_TO_LEFT_BORDER_OPEN+320+BORDER_SIDE);
    }
    catch(...)
    {
      BRK(Exception in DrawScanlineTo!);
    }
#else
    DrawScanlineTo(CYCLES_FROM_HBL_TO_LEFT_BORDER_OPEN+320
      +BORDER_SIDE);
#endif
#if defined(WIN32)
    DrawBufferedScanlineToVideo();
#endif
    if(scan_y>=draw_first_possible_line 
      && scan_y<draw_last_possible_line)
    {
      // thsee variables are pointers to PC video memory
      draw_dest_ad=draw_dest_next_scanline;
      draw_dest_next_scanline+=draw_dest_increase_y;
    }
    shifter_pixel=shifter_hscroll; //start by drawing this pixel
  }
  // Monochrome.
  else 
  {
    if(scan_y>=shifter_first_draw_line && scan_y<shifter_last_draw_line)
    {
      nsdp=shifter_draw_pointer+80;
      shifter_pixel=shifter_hscroll; //start by drawing this pixel
      if(scan_y>=draw_first_possible_line && scan_y<draw_last_possible_line)
      {
        if(border & 1)
          draw_scanline((BORDER_SIDE*2)/16, 640/16, (BORDER_SIDE*2)/16,0);
        else
          draw_scanline(0,640/16,0,0);
        draw_dest_ad=draw_dest_next_scanline;
        draw_dest_next_scanline+=draw_dest_increase_y;
      }

      shifter_draw_pointer=nsdp;
    }
    else if(scan_y>=draw_first_scanline_for_border && scan_y<draw_last_scanline_for_border)
    {
      if(scan_y>=draw_first_possible_line && scan_y<draw_last_possible_line)
      {
        if(border & 1)
          draw_scanline((BORDER_SIDE*2+640+BORDER_SIDE*2)/16,0,0,0); // rasters!
        else
          draw_scanline(640/16,0,0,0);
        draw_dest_ad=draw_dest_next_scanline;
        draw_dest_next_scanline+=draw_dest_increase_y;
      }
    }
    scanline_drawn_so_far=BORDER_SIDE+320+BORDER_SIDE; //border1+picture+border2;
  }//end Monochrome
}


inline void TShifter::GetPosition ( int *pFrameCycles , int *pHBL , int *pLineCycles ) {
  // in Steem we want to pick the right BorderMask for the line to display
#if defined(SS_VID_HATARI)
  Video_GetPosition_OnWriteAccess ( pFrameCycles , pHBL , pLineCycles ); 
  int RealLineCycles=ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl;
  if( (*pLineCycles) < RealLineCycles )
  {
    (*pHBL)--;
    (*pLineCycles)=RealLineCycles ;
  }
#else
  // TODO
#endif
}


inline int TShifter::IncScanline() {
  scan_y++; 
  HblStartingFreq=shifter_freq;
  left_border=BORDER_SIDE;
  if(shifter_hscroll) 
    left_border+=16;
  if(shifter_hscroll_extra_fetch) 
    left_border-=16;
  right_border=BORDER_SIDE;
  overscan_add_extra=0;
  // In the STE if you make hscroll non-zero in the normal way then the shifter
  // buffers 2 rasters ahead. We don't do this so to make sdp correct at the
  // end of the line we must add a raster.  
  shifter_skip_raster_for_hscroll = shifter_hscroll!=0;
  Shifter.SteemBorderMask=0;
  return scan_y;
}


inline MEM_ADDRESS TShifter::ReadSDP(int cycles_since_hbl) {
  // improved get_shifter_draw_pointer(cycles_since_hbl)
  if (bad_drawing){
    // Fake SDP
    if (scan_y<0){
      return xbios2;
    }else if (scan_y<shifter_y){
      int line_len=(160/res_vertical_scale);
      return xbios2 + scan_y*line_len + min(cycles_since_hbl/2,line_len) & ~1;
    }else{
      return xbios2+32000;
    }
  }
  CheckSideBorders();
  int HatariBorderMask=0,FrameCycles, HblCounterVideo, LineCycles;
  GetPosition( &FrameCycles , &HblCounterVideo , &LineCycles );
#if defined(SS_VID_HATARI)
  HatariBorderMask=ShifterFrame.ShifterLines[HblCounterVideo].BorderMask;
#endif
  if(scan_y+n0ByteLines>=shifter_first_draw_line  
    && scan_y+n0ByteLines<shifter_last_draw_line)
  {
    int bytes_ahead=(shifter_hscroll_extra_fetch) ? 16 : 8;
    int starts_counting=CYCLES_FROM_HBL_TO_LEFT_BORDER_OPEN/2 - bytes_ahead;
    int bytes_to_count=160; //160 bytes later
    if(!left_border)
    { 
      starts_counting-=26; 
      bytes_to_count+=26; //160+26+44=230, max overscan
    }
    if(!right_border)
    {
      bytes_to_count+=44; // fix #1 for Enchanted Land
      if(HatariBorderMask & BORDERMASK_RIGHT_OFF_FULL) // not SteemBorderMask
      {
        bytes_to_count+=24; // fix #2 for Enchanted Land
#if defined(SS_VARIOUS) && defined(SS_VAR_PROG_ID)
        SetProgram(ENCH_LAND);
#endif
      }
    }
    else if(shifter_skip_raster_for_hscroll)
      bytes_to_count+=8;
    if(overscan_add_extra<-60) 
      bytes_to_count-=106; //big right border  
    if(SteemBorderMask & BORDERMASK_LEFT_PLUS_2)
    {
      starts_counting+=2;
      bytes_to_count+=2; // fix #1 for Forest
    }
    else if(left_border==BORDER_SIDE && shifter_freq==60)
      starts_counting+=2; // fixes Swedish New Year Demo/TCB but dubious
    if(SteemBorderMask & BORDERMASK_RIGHT_MINUS_2)
      bytes_to_count-=2;
    if(SteemBorderMask & BORDERMASK_EMPTY_LINE)
      return shifter_draw_pointer_at_start_of_line; // fix #2 for Forest
    int c=cycles_since_hbl/2-starts_counting;
    MEM_ADDRESS sdp=shifter_draw_pointer_at_start_of_line;
    if (c>=bytes_to_count)
      sdp+=bytes_to_count+(shifter_fetch_extra_words*2);
    else if (c>=0){
      c&=-2;
      sdp+=c;
    }
//    TRACE("VBL %d y%d cycle %d freq%d Read SDP %X\n",nVBLs,scan_y,cycles_since_hbl,shifter_freq,sdp);
    return sdp;
  }
  else
    return shifter_draw_pointer;
}


inline void TShifter::SetRes(BYTE NewRes) {
#if defined(SS_VID_HATARI)
  Video_WriteToShifter(NewRes);
#if defined(SS_DEBUG)
  int FrameCycles, HblCounterVideo, LineCycles;
  Video_GetPosition_OnWriteAccess ( &FrameCycles , &HblCounterVideo , &LineCycles );
  VideoEvents.Add( HblCounterVideo , LineCycles, 'R', NewRes); 
  TShifter::BorderMaskTrace|=ShifterFrame.ShifterLines[HblCounterVideo].BorderMask;
#endif
#endif
}


inline void TShifter::SetSync(BYTE NewSync) {
#if defined(SS_VID_HATARI)
  int FrameCycles, HblCounterVideo, LineCycles;
  Video_GetPosition_OnWriteAccess ( &FrameCycles , &HblCounterVideo , &LineCycles );
#if defined(SS_DEBUG)
  VideoEvents.Add( HblCounterVideo , LineCycles, 'S', NewSync); 
  TShifter::BorderMaskTrace|=ShifterFrame.ShifterLines[HblCounterVideo].BorderMask;
#endif
  // The function will read the value in IO, so we must fake it
  int current_shifter_freq=shifter_freq;
  shifter_freq=(NewSync & 2) ? 50 : 60; 
  Video_Sync_WriteByte();
#if defined(SS_VID_DISTORTION) // !!!!!!!!!!!!!!!!!!!!!!!!!!!
  // This is definetely a very dangerous hack, before true emul;
  // the idea is that distortion confuses the shifter's timings
  // but this is just an excuse to have fun, it's certainly false.
  // The real problem could be in ReadSDP but I can't find the bug.
  if(SpecificHacks
    && shifter_freq!=current_shifter_freq 
    && shifter_freq==50 
    && DE()
    && LineCycles>52 && LineCycles<372
#ifndef SS_DEBUG // targetting Omega: what could this break?
    && LineCycles>100 && LineCycles<130 && scan_y==-29 // check SoWatt/Sync
#endif
    )
  {
    SteemBorderMask|=BORDERMASK_FREQ_DISTORTION; // Omega Full Overscan
    INSTRUCTION_TIME(-4); // OMG cheat! 
#if defined(SS_VAR_PROG_ID)
    SetProgram(OMEGA);
#endif
  }
#endif
  shifter_freq=current_shifter_freq;
#endif
}

// just taking some unimportant code out of DST for clarity
#define   AUTO_BORDER_ADJUST  \
          if(!(border & 1)) { \
            if(scanline_drawn_so_far<BORDER_SIDE) { \
              border1-=(BORDER_SIDE-scanline_drawn_so_far); \
              if(border1<0){ \
                picture+=border1; \
                if(screen_res==ST_LOW_RES) {  \
                  hscroll-=border1;  \
                  shifter_draw_pointer+=(hscroll/16)*8; \
                  hscroll&=15; \
                }else if(screen_res==ST_MEDIUM_RES) { \
                  hscroll-=border1*2;  \
                  shifter_draw_pointer+=(hscroll/16)*4; \
                  hscroll&=15; \
                } \
                border1=0; \
                if(picture<0) picture=0; \
              } \
            } \
            int ta=(border1+picture+border2)-320; \
            if(ta>0) { \
              border2-=ta; \
              if(border2<0)  { \
                picture+=border2; \
                border2=0; \
                if (picture<0)  picture=0; \
              } \
            } \
            border1=border2=0; \
          }
