/*---------------------------------------------------------------------------
FILE: debug_emu.cpp
MODULE: emu
DESCRIPTION: General low-level debugging functions.
---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
// This is for if the emu is half way though the screen, it should be called
// immediately after draw_begin to fix draw_dest_ad
void debug_update_drawing_position(int *pHorz)
{
  int y=-scanlines_above_screen[0];
  if (shifter_freq_at_start_of_vbl==60) y=-scanlines_above_screen[1];
  for (;y<scan_y;y++){
    if (y>=draw_first_scanline_for_border || y>=shifter_first_draw_line){
      if (y<shifter_last_draw_line || y<draw_last_scanline_for_border){
        if (y>=draw_first_possible_line && y<draw_last_possible_line){
          draw_dest_ad=draw_dest_next_scanline;
          draw_dest_next_scanline+=draw_dest_increase_y;
        }
      }
    }
  }
  if (screen_res_at_start_of_vbl<2){
    if ((scan_y>=draw_first_scanline_for_border || scan_y>=shifter_first_draw_line) &&
          (scan_y<shifter_last_draw_line || scan_y<draw_last_scanline_for_border)){
      int horz_scale=1;
      if (screen_res_at_start_of_vbl==1 || mixed_output || draw_med_low_double_height) horz_scale=2;

      int x=scanline_drawn_so_far;
      if ((border & 1)==0) 
        x=max(x-BORDER_SIDE,0);
      x*=horz_scale;

      draw_dest_ad+=x*BytesPerPixel;
      if (pHorz) *pHorz=horz_scale;
    }
  }
}
//---------------------------------------------------------------------------
void update_display_after_trace()
{
  if (extended_monitor) return;

  // Draw can call this routine, and it calls draw, arrrggghh!
  static bool in=0;
  if (in) return;
  in=true;

  osd_no_draw=true;
  if (screen_res_at_start_of_vbl<2 && redraw_after_trace==0){
    RECT old_src=draw_blit_source_rect;
    draw_begin();
    if (draw_blit_source_rect.left!=old_src.left || draw_blit_source_rect.top!=old_src.top ||
          draw_blit_source_rect.right!=old_src.right || draw_blit_source_rect.bottom!=old_src.bottom){
      draw_end();
      draw(false);
      draw_begin();
    }
    draw_buffer_complex_scanlines=0;

    int horz_scale=0;
    debug_update_drawing_position(&horz_scale);

#if defined(STEVEN_SEAGAL) && defined(SS_VIDEO)
    Shifter.DrawScanlineTo(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl);
#else
    draw_scanline_to(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl);
#endif
    if ((scanline_drawn_so_far < BORDER_SIDE+320+BORDER_SIDE) && horz_scale){

      int line_add=0;
      if (draw_med_low_double_height) line_add=draw_line_length;
      for (int i=0;i<horz_scale;i++){
        DWORD col=colour_convert(GetRValue(debug_gun_pos_col),GetGValue(debug_gun_pos_col),GetBValue(debug_gun_pos_col));
        switch (BytesPerPixel){
          case 1:
            *draw_dest_ad=BYTE(col);
            *(draw_dest_ad+line_add)=BYTE(col);
            break;
          case 2:
            *LPWORD(draw_dest_ad)=WORD(col);
            *LPWORD(draw_dest_ad+line_add)=WORD(col);
            break;
          case 3:
          case 4:
            *LPDWORD(draw_dest_ad)=DWORD(col);
            *LPDWORD(draw_dest_ad+line_add)=DWORD(col);
            break;
        }
        draw_dest_ad+=BytesPerPixel;
      }
    }
    draw_end();
    draw_blit();
  }else{
    draw(false);
  }
  osd_no_draw=0;
  in=0;
}
//---------------------------------------------------------------------------
void breakpoint_log()
{
  if (logging_suspended) return;

  Str logline=Str("\r\n!!!! PASSED BREAKPOINT at address $")+HEXSl(pc,6)+", sr="+HEXSl(sr,4);
  logline+="\r\n";
  for (int n=0;n<8;n++) logline+=Str("d")+n+"="+HEXSl(r[n],6)+"  ";
  logline+="\r\n";
  for (int n=0;n<8;n++) logline+=Str("a")+n+"="+HEXSl(areg[n],6)+"  ";
  logline+="\r\n";
  logline+=Str("time=")+ABSOLUTE_CPU_TIME+" scanline="+scan_y+" cycle="+(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl);
  logline+="\r\n";
  log_write(logline);
}

//---------------------------------------------------------------------------
void breakpoint_check()
{
  if (runstate!=RUNSTATE_RUNNING) return;

  for (int n=0;n<debug_num_bk;n++){
    if (debug_bk_ad[n]==pc){
      if (debug_get_ad_mode(pc)==3){
        breakpoint_log();
      }else{
        runstate=RUNSTATE_STOPPING;
        SET_WHY_STOP(Str("Hit breakpoint at address $")+HEXSl(pc,6));
      }
      return;
    }
  }
}
//---------------------------------------------------------------------------
void debug_update_cycle_counts()
{
  debug_cycles_since_VBL=ABSOLUTE_CPU_TIME-cpu_time_of_last_vbl;
  debug_cycles_since_HBL=ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl;
#if defined(STEVEN_SEAGAL) && defined(SS_VIDEO)
  debug_VAP=Shifter.ReadSDP(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl);
#else
  debug_VAP=get_shifter_draw_pointer(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl);
#endif

  for (int t=0;t<4;t++){
    if (mfp_timer_enabled[t]){
      debug_time_to_timer_timeout[t]=mfp_timer_timeout[t]-ABSOLUTE_CPU_TIME;
    }else{
      debug_time_to_timer_timeout[t]=0;
    }
  }
}
//---------------------------------------------------------------------------
extern BYTE d2_peek(MEM_ADDRESS);
extern WORD d2_dpeek(MEM_ADDRESS);

void debug_hit_mon(MEM_ADDRESS ad,int read)
{
  if (mode!=STEM_MODE_CPU) return;
#if defined(STEVEN_SEAGAL)
  WORD mask=debug_get_ad_mask(ad,TRUE);
#else
  WORD mask=debug_get_ad_mask(ad,read);
#endif
  int bytes=2;
  if (mask==0xff00) bytes=1;
  if (mask==0x00ff) bytes=1, ad++;
  int val=int((bytes==1) ? int(d2_peek(ad)):int(d2_dpeek(ad)));
  Str mess;
  if (read){
    mess=HEXSl(old_pc,6)+": Read "+val+" ($"+HEXSl(val,bytes*2)+") from address $"+HEXSl(ad,6);
  }else{
    mess=HEXSl(old_pc,6)+": Write to address $"+HEXSl(ad,6);
  }
  int mode=debug_get_ad_mode(ad & ~1);
  if (mode==2){
    if (runstate==RUNSTATE_RUNNING){
      runstate=RUNSTATE_STOPPING;
      SET_WHY_STOP(mess);
    }else if (runstate==RUNSTATE_STOPPED){
      Alert(mess,"Monitor Activated",0);
    }
  }else{
    debug_mem_write_log_address=ad;
    debug_mem_write_log_bytes=bytes;
    ioaccess|=IOACCESS_DEBUG_MEM_WRITE_LOG;
  }
}
//---------------------------------------------------------------------------
void debug_hit_io_mon_write(MEM_ADDRESS ad,int val)
{
  if (mode!=STEM_MODE_CPU) return;
#if defined(STEVEN_SEAGAL)
  WORD mask=debug_get_ad_mask(ad,TRUE);
#else
  WORD mask=debug_get_ad_mask(ad,read);
#endif
  int bytes=2;
  if (mask==0xff00) bytes=1;
  if (mask==0x00ff) bytes=1, ad++;
  Str mess=HEXSl(old_pc,6)+": Wrote to address $"+HEXSl(ad,6)+", new value is "+val+" ($"+HEXSl(val,bytes*2)+")";
  int mode=debug_get_ad_mode(ad & ~1);
  if (mode==2){
    if (runstate==RUNSTATE_RUNNING){
      runstate=RUNSTATE_STOPPING;
      SET_WHY_STOP(mess);
    }else if (runstate==RUNSTATE_STOPPED){
      Alert(mess,"Monitor Activated",0);
    }
  }else{
    log_write(mess);
  }
}
//---------------------------------------------------------------------------
void debug_check_for_events()
{
  while (cpu_cycles<=0){
    screen_event_vector();
    prepare_next_event();
    if (cpu_cycles>0) check_for_interrupts_pending();
  }
}
//---------------------------------------------------------------------------
void debug_trace_event_plan_init()
{
  if (screen_event_pointer==NULL) screen_event_pointer=event_plan[shifter_freq_idx];
}
//---------------------------------------------------------------------------
void iolist_debug_add_pseudo_addresses()
{
  iolist_add_entry(IOLIST_PSEUDO_AD_PSG+0x000,"PSG0 Ch.A Freq L",1,NULL,psg_reg);
  iolist_add_entry(IOLIST_PSEUDO_AD_PSG+0x002,"PSG1 Ch.A Freq H",1,NULL,psg_reg+1);
  iolist_add_entry(IOLIST_PSEUDO_AD_PSG+0x004,"PSG2 Ch.B Freq L",1,NULL,psg_reg+2);
  iolist_add_entry(IOLIST_PSEUDO_AD_PSG+0x006,"PSG3 Ch.B Freq H",1,NULL,psg_reg+3);
  iolist_add_entry(IOLIST_PSEUDO_AD_PSG+0x008,"PSG4 Ch.C Freq L",1,NULL,psg_reg+4);
  iolist_add_entry(IOLIST_PSEUDO_AD_PSG+0x00a,"PSG5 Ch.C Freq H",1,NULL,psg_reg+5);
  iolist_add_entry(IOLIST_PSEUDO_AD_PSG+0x00c,"PSG6 Noise Freq",1,NULL,psg_reg+6);
  iolist_add_entry(IOLIST_PSEUDO_AD_PSG+0x00e,"PSG7 Mixer",1,"PortB Out|PortA Out|Ch.C Noise off|B Noise Off|A Noise Off|C Tone Off|B Tone Off|A Tone Off",psg_reg+7);
  iolist_add_entry(IOLIST_PSEUDO_AD_PSG+0x010,"PSG8 Ch.A Ampl",1,".|.|.|env|A3|A2|A1|A0",psg_reg+8);
  iolist_add_entry(IOLIST_PSEUDO_AD_PSG+0x012,"PSG9 Ch.B Ampl",1,".|.|.|env|A3|A2|A1|A0",psg_reg+9);
  iolist_add_entry(IOLIST_PSEUDO_AD_PSG+0x014,"PSG10 Ch.C Ampl",1,".|.|.|env|A3|A2|A1|A0",psg_reg+10);
  iolist_add_entry(IOLIST_PSEUDO_AD_PSG+0x016,"PSG11 Env Period H",1,NULL,psg_reg+11);
  iolist_add_entry(IOLIST_PSEUDO_AD_PSG+0x018,"PSG12 Env Period L",1,NULL,psg_reg+12);
  iolist_add_entry(IOLIST_PSEUDO_AD_PSG+0x01a,"PSG13 Env shape",1,".|.|.|.|Continue|Attack|Alternate|Hold",psg_reg+13);
  iolist_add_entry(IOLIST_PSEUDO_AD_PSG+0x01c,"PSG14 Port A",1,"IDE Drv on|SCC A|Mon Jack GPO|Int. Spkr|Cent strobe|RS232 DTR|RS232 RTS|Drv 1|Drv 0|Drv side",psg_reg+14);
  iolist_add_entry(IOLIST_PSEUDO_AD_PSG+0x01e,"PSG15 Port B (Parallel port)",1,NULL,psg_reg+15);

  iolist_add_entry(IOLIST_PSEUDO_AD_FDC+0x000,"FDC Command Register",1,NULL,&fdc_cr);
  iolist_add_entry(IOLIST_PSEUDO_AD_FDC+0x002,"FDC Status Register",1,"Motor|Write Protect|Spin/Rec|Seek Fail|CRC Err|Track 0|Index|Busy",&fdc_str);
  iolist_add_entry(IOLIST_PSEUDO_AD_FDC+0x004,"FDC Sector Register",1,NULL,&fdc_sr);
  iolist_add_entry(IOLIST_PSEUDO_AD_FDC+0x006,"FDC Track Register",1,NULL,&fdc_tr);
  iolist_add_entry(IOLIST_PSEUDO_AD_FDC+0x008,"FDC Data Register",1,NULL,&fdc_dr);
  iolist_add_entry(IOLIST_PSEUDO_AD_FDC+0x00a,"FDC Drive/Side (PSG 14)",1,"B|A|Side 0",&(psg_reg[14]));
  iolist_add_entry(IOLIST_PSEUDO_AD_FDC+0x00c,"FDC Current Track Drive A",1,NULL,&(floppy_head_track[0]));
  iolist_add_entry(IOLIST_PSEUDO_AD_FDC+0x00e,"FDC Current Track Drive B",1,NULL,&(floppy_head_track[1]));
  iolist_add_entry(IOLIST_PSEUDO_AD_FDC+0x010,"FDC Spinning Up",1,NULL,lpDWORD_B_0(&fdc_spinning_up));
  iolist_add_entry(IOLIST_PSEUDO_AD_FDC+0x012,"FDC Type 1 Command Active",1,NULL,lpDWORD_B_0(&floppy_type1_command_active));
  iolist_add_entry(IOLIST_PSEUDO_AD_FDC+0x014,"FDC Stepping In Flag",1,NULL,lpDWORD_B_0(&fdc_last_step_inwards_flag));
  iolist_add_entry(IOLIST_PSEUDO_AD_FDC+0x018,"DMA Address High",1,NULL,lpDWORD_B_2(&dma_address));
  iolist_add_entry(IOLIST_PSEUDO_AD_FDC+0x01a,"DMA Address Mid",1,NULL,lpDWORD_B_1(&dma_address));
  iolist_add_entry(IOLIST_PSEUDO_AD_FDC+0x01c,"DMA Address Low",1,NULL,lpDWORD_B_0(&dma_address));
  iolist_add_entry(IOLIST_PSEUDO_AD_FDC+0x01e,"DMA Mode",1,"FDC Transfer|Disable DMA|.|Sec Count Select|HDC|A1|A0|.",lpWORD_B_0(&dma_mode));
  iolist_add_entry(IOLIST_PSEUDO_AD_FDC+0x020,"DMA Write From RAM (Bit 8 of Mode)",1,NULL,lpWORD_B_1(&dma_mode));
  iolist_add_entry(IOLIST_PSEUDO_AD_FDC+0x022,"DMA Status",1,"DRQ|Sec Count Not 0|No Error",&dma_status);
  iolist_add_entry(IOLIST_PSEUDO_AD_FDC+0x024,"DMA Sector Count",1,NULL,lpDWORD_B_0(&dma_sector_count));

  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x000,"IKBD Mouse Mode",1,NULL,lpDWORD_B_0(&ikbd.mouse_mode));
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x002,"IKBD Joy Mode",1,NULL,lpDWORD_B_0(&ikbd.joy_mode));
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x004,"IKBD Send Nothing Flag",1,NULL,(BYTE*)&ikbd.send_nothing);
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x006,"IKBD Resetting Flag",1,NULL,(BYTE*)&ikbd.resetting);
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x008,"IKBD Mouse Button Action",1,"Keys|Release ABS|Press ABS",&ikbd.mouse_button_press_what_message);
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x00a,"IKBD Port 0 Joystick Flag",1,NULL,(BYTE*)&ikbd.port_0_joy);
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x00c,"IKBD Mouse Y Reverse Flag",1,NULL,(BYTE*)&ikbd.mouse_upside_down);
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x00e,"IKBD Relative Mouse Threshold X",1,NULL,lpDWORD_B_0(&ikbd.relative_mouse_threshold_x));
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x010,"IKBD Relative Mouse Threshold Y",1,NULL,lpDWORD_B_0(&ikbd.relative_mouse_threshold_y));
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x012,"IKBD Abs Mouse Pos X High",1,NULL,lpDWORD_B_1(&ikbd.abs_mouse_x));
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x014,"IKBD Abs Mouse Pos X Low",1,NULL,lpDWORD_B_0(&ikbd.abs_mouse_x));
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x016,"IKBD Abs Mouse Pos Y High",1,NULL,lpDWORD_B_1(&ikbd.abs_mouse_y));
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x018,"IKBD Abs Mouse Pos Y Low",1,NULL,lpDWORD_B_0(&ikbd.abs_mouse_y));
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x01a,"IKBD Abs Mouse Max X High",1,NULL,lpDWORD_B_1(&ikbd.abs_mouse_max_x));
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x01c,"IKBD Abs Mouse Max X Low",1,NULL,lpDWORD_B_0(&ikbd.abs_mouse_max_x));
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x01e,"IKBD Abs Mouse Max Y High",1,NULL,lpDWORD_B_1(&ikbd.abs_mouse_max_y));
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x020,"IKBD Abs Mouse Max Y Low",1,NULL,lpDWORD_B_0(&ikbd.abs_mouse_max_y));
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x022,"IKBD Abs Mouse Scale X",1,NULL,lpDWORD_B_0(&ikbd.abs_mouse_scale_x));
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x024,"IKBD Abs Mouse Scale Y",1,NULL,lpDWORD_B_0(&ikbd.abs_mouse_scale_y));
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x026,"IKBD Absolute Mouse Buttons",1,
            "RMB Down|RMB Was Down|LMB Down|LMB Was Down",lpDWORD_B_0(&ikbd.abs_mousek_flags));
  iolist_add_entry(IOLIST_PSEUDO_AD_IKBD+0x028,"IKBD Joy Button Duration",1,NULL,lpDWORD_B_0(&ikbd.duration));
}
//---------------------------------------------------------------------------
int __stdcall debug_plugin_read_mem(DWORD ad,BYTE *buf,int len)
{
  if (ad>=himem) return 0;
  if (ad+len>=himem) len=himem-ad;

  int n_bytes=len;
  BYTE *p=lpPEEK(ad);
  while (len--){
    *(buf++)=*p;
    p+=MEM_DIR;
  }
  return n_bytes;
}
//---------------------------------------------------------------------------
int __stdcall debug_plugin_write_mem(DWORD ad,BYTE *buf,int len)
{
  if (ad>=himem) return 0;
  if (ad+len>=himem) len=himem-ad;

  int n_bytes=len;
  BYTE *p=lpPEEK(ad);
  while (len--){
    *p=*(buf++);
    p+=MEM_DIR;
  }
  return n_bytes;
}
//---------------------------------------------------------------------------


