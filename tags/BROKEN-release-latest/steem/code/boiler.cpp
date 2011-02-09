//---------------------------------------------------------------------------
void debug_trace_crash(m68k_exception &e)
{
  SendMessage(trace_window_handle,WM_SETTEXT,0,(long)"Exception");
  trace_init();
/*          d2_trace=true;
  disa_d2(e.crash_address);
  d2_trace=false;
*/
  trace_pc=e.crash_address;
  trace_sr_before=e._sr;

  update_register_display(true);
  trace_exception_display(&e);
  trace_display();
}
//---------------------------------------------------------------------------
void debug_run_start()
{
  ShowWindow(trace_window_handle,SW_HIDE);
  SetWindowText(DWin_run_button,"Stop");
  for (int n=0;n<num_monitors;n++){
    if (monitor_ad[n]<mem_len) monitor_contents[n]=DPEEK(monitor_ad[n]) & monitor_mask[n];
  }
}
//---------------------------------------------------------------------------
void debug_run_end()
{
  if (runstate_why_stop.NotEmpty()){
    Alert(runstate_why_stop,"Break",0);
    runstate_why_stop="";
  }
  trace_over_breakpoint=0xffffffff;
  SetWindowText(DWin_run_button,"Run");
  update_register_display(true);
  osd_hide();
}
//---------------------------------------------------------------------------
void debug_reset()
{
  for (int n=0;n<num_monitors;n++){
    if (monitor_ad[n]<mem_len) monitor_contents[n]=DPEEK(monitor_ad[n]) & monitor_mask[n];
  }
  update_register_display(true);
}
//---------------------------------------------------------------------------
int get_breakpoint_or_monitor(bool mon,MEM_ADDRESS ad)
{
  if (mon){
    for (int n=0;n<num_monitors;n++) if (monitor_ad[n]==ad) return n;
    for (int n=0;n<num_io_monitors;n++) if (monitor_io_ad[n]==ad) return n;
  }else{
    for (int n=0;n<num_breakpoints;n++) if (breakpoint[n]==ad) return n;
  }
  return -1;
}
//---------------------------------------------------------------------------
void remove_breakpoint_or_monitor(bool is_mon,MEM_ADDRESS ad)
{
  int *num=&num_breakpoints;
  MEM_ADDRESS *ads=breakpoint,*mon_cont=NULL;
  WORD *mon_mask=NULL;
  bool *mon_readflag=NULL;
  if (is_mon){
    num=(int*)((ad>=MEM_IO_BASE) ? &num_io_monitors:&num_monitors);
    ads=(MEM_ADDRESS*)((ad>=MEM_IO_BASE) ? monitor_io_ad:monitor_ad);
    mon_cont=(MEM_ADDRESS*)((ad>=MEM_IO_BASE) ? NULL:monitor_contents);
    mon_mask=(WORD*)((ad>=MEM_IO_BASE) ? monitor_io_mask:monitor_mask);
    mon_readflag=(bool*)((ad>=MEM_IO_BASE) ? monitor_io_readflag:NULL);
  }
  int n=0;
  while (n<*num){
    if (ads[n]==ad){
      for (int m=n;m<(*num)-1;m++){
        ads[m]=ads[m+1];
        if (mon_cont) mon_cont[m]=mon_cont[m+1];
        if (mon_mask) mon_mask[m]=mon_mask[m+1];
        if (mon_readflag) mon_readflag[m]=mon_readflag[m+1];
      }
      (*num)--;
    }else{
      n++;
    }
  }
  UPDATE_DO_MON_CHECK;
  UPDATE_DO_BREAK_CHECK;
  breakpoint_menu_setup();
  mem_browser_update_all();
}
//---------------------------------------------------------------------------
void set_breakpoint_or_monitor(bool is_mon,MEM_ADDRESS ad,WORD mask,bool readflag)
{
  int *num=&num_breakpoints;
  MEM_ADDRESS *ads=breakpoint,*mon_cont=NULL;
  WORD *mon_mask=NULL;
  bool *mon_readflag=NULL;
  if (is_mon){
    num=(int*)((ad>=MEM_IO_BASE) ? &num_io_monitors:&num_monitors);
    ads=(MEM_ADDRESS*)((ad>=MEM_IO_BASE) ? monitor_io_ad:monitor_ad);
    mon_cont=(MEM_ADDRESS*)((ad>=MEM_IO_BASE) ? NULL:monitor_contents);
    mon_mask=(WORD*)((ad>=MEM_IO_BASE) ? monitor_io_mask:monitor_mask);
    mon_readflag=(bool*)((ad>=MEM_IO_BASE) ? monitor_io_readflag:NULL);
  }
  int n;
  for (n=0;ads[n]<ad && n<*num;n++);
  if (n>=*num || ads[n]>ad){
    if (*num>=MAX_BREAKPOINTS){
      MessageBox(NULL,"Too many breakpoints/monitors","That's Enough!",
                  MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_TOPMOST);

    }else{
      for (int m=*num;m>n;m--){
        ads[m]=ads[m-1];
        if (mon_cont) mon_cont[m]=mon_cont[m-1];
        if (mon_mask) mon_mask[m]=mon_mask[m-1];
        if (mon_readflag) mon_readflag[m]=mon_readflag[m-1];
      }
      ads[n]=ad;
      if (mon_mask) mon_mask[n]=mask;
      if (mon_cont) mon_cont[n]=d2_dpeek(ad) & mask;
      if (mon_readflag) mon_readflag[n]=readflag;
      (*num)++;
    }
  }
  UPDATE_DO_MON_CHECK;
  UPDATE_DO_BREAK_CHECK;
  breakpoint_menu_setup();
  mem_browser_update_all();
}
//---------------------------------------------------------------------------
void debug_check_break_on_irq(int irq)
{
  if (break_on_irq[irq]){
    if (runstate==RUNSTATE_RUNNING){
      runstate=RUNSTATE_STOPPING;
      runstate_why_stop=HEXSl(old_pc,6)+": "+name_of_mfp_interrupt[irq]+" Interrupt";
    }
  }
  if (debug_in_trace && irq!=BREAK_IRQ_LINEA_IDX && irq!=BREAK_IRQ_LINEF_IDX && irq!=BREAK_IRQ_TRAP_IDX){
    runstate_why_stop=HEXSl(old_pc,6)+": "+name_of_mfp_interrupt[irq]+" Interrupt";
  }
}
//---------------------------------------------------------------------------
void breakpoint_menu_setup()
{
  char tb[100];
  MEM_ADDRESS save_dpc=dpc;

  RemoveAllMenuItems(breakpoint_menu);
  RemoveAllMenuItems(breakpoint_irq_menu);
  RemoveAllMenuItems(monitor_menu);

  AppendMenu(breakpoint_irq_menu,MF_STRING | int(break_on_irq[BREAK_IRQ_HBL_IDX] ? MF_CHECKED:MF_UNCHECKED),
                9000+BREAK_IRQ_HBL_IDX,"HBL");
  AppendMenu(breakpoint_irq_menu,MF_STRING | int(break_on_irq[BREAK_IRQ_VBL_IDX] ? MF_CHECKED:MF_UNCHECKED),
                9000+BREAK_IRQ_VBL_IDX,"VBL");
  for (int n=0;n<16;n++){
    AppendMenu(breakpoint_irq_menu,MF_STRING | int(break_on_irq[n] ? MF_CHECKED:MF_UNCHECKED),
                  9000+n,name_of_mfp_interrupt[n]);
  }
  AppendMenu(breakpoint_irq_menu,MF_STRING | int(break_on_irq[BREAK_IRQ_LINEA_IDX] ? MF_CHECKED:MF_UNCHECKED),
                9000+BREAK_IRQ_LINEA_IDX,"Line-A");
  AppendMenu(breakpoint_irq_menu,MF_STRING | int(break_on_irq[BREAK_IRQ_LINEF_IDX] ? MF_CHECKED:MF_UNCHECKED),
                9000+BREAK_IRQ_LINEF_IDX,"Line-F");
  AppendMenu(breakpoint_irq_menu,MF_STRING | int(break_on_irq[BREAK_IRQ_TRAP_IDX] ? MF_CHECKED:MF_UNCHECKED),
                9000+BREAK_IRQ_TRAP_IDX,"Trap");
  AppendMenu(breakpoint_irq_menu,MF_SEPARATOR,0,NULL);
  AppendMenu(breakpoint_irq_menu,MF_STRING,9030,"Check All");
  AppendMenu(breakpoint_irq_menu,MF_STRING,9031,"Uncheck All");

  AppendMenu(breakpoint_menu,MF_STRING|int((breakpoint_mode==1) ? MF_CHECKED:0),1107,"Stop On Breakpoints");
  AppendMenu(breakpoint_menu,MF_STRING|int((breakpoint_mode==2) ? MF_CHECKED:0),1108,"Log On Breakpoints");
  AppendMenu(breakpoint_menu,MF_STRING,1100,"Clear All Breakpoints");
  AppendMenu(breakpoint_menu,MF_STRING,1101,"Set Breakpoint At PC");
  AppendMenu(breakpoint_menu,MF_SEPARATOR,0,NULL);
  AppendMenu(breakpoint_menu,MF_POPUP,(int)breakpoint_irq_menu,"Break On Interrupt");



  AppendMenu(monitor_menu,MF_STRING|int((monitor_mode==1) ? MF_CHECKED:0),1103,"Stop On Changes");
  AppendMenu(monitor_menu,MF_STRING|int((monitor_mode==2) ? MF_CHECKED:0),1104,"Log On Changes");
  AppendMenu(monitor_menu,MF_STRING,1106,"Clear All Monitored Addresses");
  AppendMenu(monitor_menu,MF_STRING,1105,"Set Monitor On Screen");


  AppendMenu(breakpoint_menu,MF_SEPARATOR,0,NULL);
  if (num_breakpoints){
    for (int n=0;n<num_breakpoints;n++){
      strcpy(tb,HEXSl(breakpoint[n],6));strcat(tb,"  ");
      strcat(tb,disa_d2(breakpoint[n]));
      AppendMenu(breakpoint_menu,MF_STRING,1110+n,tb);
    }
  }
  AppendMenu(monitor_menu,MF_SEPARATOR,0,NULL);
  if (num_monitors){
    for (int n=0;n<num_monitors;n++){
      MEM_ADDRESS ad=monitor_ad[n];
      if (monitor_mask[n]==0x00ff) ad++;
      char *suff=".b";
      if (monitor_mask[n]==0xffff) suff=".w";

      strcpy(tb,HEXSl(ad,6));strcat(tb,suff);strcat(tb,"  ");
      strcat(tb,disa_d2(monitor_ad[n]));
      AppendMenu(monitor_menu,MF_STRING,1110+MAX_BREAKPOINTS+n,tb);
    }
  }
  if (num_io_monitors){
    for (int n=0;n<num_io_monitors;n++){
      MEM_ADDRESS ad=monitor_io_ad[n];
      if (monitor_io_mask[n]==0x00ff) ad++;
      char *suff=".b";
      if (monitor_io_mask[n]==0xffff) suff=".w";

      strcpy(tb,HEXSl(ad,6));strcat(tb,suff);strcat(tb,"  ");
      iolist_entry *io=search_iolist(ad);
      if (io) strcat(tb,io->name);
      if (monitor_io_mask[n]==0xffff){
        iolist_entry *io2=search_iolist(ad+1);
        if (io2){
          if (io) strcat(tb," | ");
          strcat(tb,io2->name);
        }
      }
      AppendMenu(monitor_menu,MF_STRING,1110+MAX_BREAKPOINTS+MAX_BREAKPOINTS+n,tb);
    }
  }
  dpc=save_dpc;
}
//---------------------------------------------------------------------------
void insp_menu_setup()
{
  char ttt[150];
  for(int n=0;n<3;n++){
    if (insp_menu_long_bytes[n]){
      if(insp_menu_long_bytes[n]>2){
        strcpy(ttt,"New instruction browser at ");strcat(ttt,insp_menu_long_name[n]);
        AppendMenu(insp_menu,MF_ENABLED | MF_STRING,3010+n,ttt);
        strcpy(ttt,"New memory browser at ");strcat(ttt,insp_menu_long_name[n]);
        AppendMenu(insp_menu,MF_ENABLED | MF_STRING,3012+n,ttt);
      }
      strcpy(ttt,"Set register to ");strcat(ttt,insp_menu_long_name[n]);
      AppendMenu(insp_menu,MF_ENABLED | MF_STRING | MF_POPUP,
        (UINT)insp_menu_reg_submenu[n],ttt);

    }
  }
}
//---------------------------------------------------------------------------
void update_register_display(bool reset_pc_display)
{
//  char ttt[30];
  if (reset_pc_display){
    m_b_mem_disa.ad=pc;
    m_b_stack.ad=r[15];
  }

  debug_update_cycle_counts();

  InvalidateRect(sr_display,NULL,0);
  InvalidateRect(trace_sr_before_display,NULL,0);
  InvalidateRect(trace_sr_after_display,NULL,0);

  mr_static_update_all();
  mem_browser_update_all();

/*  for(int n=0;n<MAX_MEMORY_BROWSERS;n++){
    if(m_b[n].active){
      if(m_b[n].disp_type==DT_REGISTERS){
        mem_browser_update(&(m_b[n]));
      }
    }
  }*/
}
//---------------------------------------------------------------------------
long __stdcall sr_display_WndProc(HWND Win,UINT Mess,UINT wPar,long lPar)
{
//  static wait_button_up;
  unsigned short *lpsr;
  int wid;
	switch (Mess){
    case WM_PAINT:
    {
      lpsr=(unsigned short*)GetWindowLong(Win,GWL_USERDATA);
      if (lpsr==NULL) lpsr=&sr;
      PAINTSTRUCT ps;
      BeginPaint(Win,&ps);
      RECT box,rc;GetClientRect(Win,&box);
      HBRUSH bg_br=CreateSolidBrush(GetSysColor(COLOR_WINDOW));
      HBRUSH hi_br=CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
      COLORREF hi_text_col=GetSysColor(COLOR_HIGHLIGHTTEXT);
      COLORREF std_text_col=GetSysColor(COLOR_WINDOWTEXT);

      wid=box.right;
      int x;
      HPEN pen=CreatePen(PS_SOLID,1,GetSysColor(COLOR_WINDOWTEXT));
      HFONT old_fnt=(HFONT)SelectObject(ps.hdc,fnt);
      char *lab="T.S..210...XNZVC";
      WORD mask=0x8000;
      SetBkMode(ps.hdc,TRANSPARENT);
      for(int n=0;n<16;n++){
        x=(n*wid)/16;
        rc.left=x;rc.top=0;rc.right=((n+1)*wid)/16;rc.bottom=box.bottom;
        FillRect(ps.hdc,&rc,HBRUSH(((*lpsr)&mask) ? hi_br:bg_br));
        SetTextColor(ps.hdc,COLORREF(((*lpsr)&mask) ? hi_text_col:std_text_col));
        if (n){
          MoveToEx(ps.hdc,x,0,NULL);
          LineTo(ps.hdc,x,box.bottom);
        }

        CentreTextOut(ps.hdc,x,0,(wid/16),box.bottom,lab+n,1);
        mask>>=1;
      }
      DeleteObject(hi_br);
      DeleteObject(bg_br);
      DeleteObject(pen);
      SelectObject(ps.hdc,old_fnt);

      EndPaint(Win,&ps);

      return 0;
    }
    case WM_LBUTTONDOWN:
    {
      RECT box;GetClientRect(Win,&box);
      int x=(short)LOWORD(lPar);
      if (x>=0){
        int n=(16*x)/box.right;
        n=max(0,min(15,n));
        lpsr=(WORD*)GetWindowLong(Win,GWL_USERDATA);
        if(lpsr==NULL)lpsr=&sr;
        *lpsr^=(unsigned short)(0x8000>>n);
        InvalidateRect(Win,NULL,0);
        InvalidateRect(sr_display,NULL,0);
        InvalidateRect(trace_sr_after_display,NULL,0);
      }
    }
    break;
  }
	return CallWindowProc(Old_sr_display_WndProc,Win,Mess,wPar,lPar);

}
//---------------------------------------------------------------------------
LRESULT __stdcall DWndProc(HWND Win,UINT Mess,UINT wPar,long lPar)
{
	if (Win==HiddenParent) return DefWindowProc(Win,Mess,wPar,lPar);

	switch (Mess){
    case WM_SIZE:
      MoveWindow(DWin_trace_button,340,1,(LOWORD(lPar)-350)/4 - 5,27,true);
      MoveWindow(DWin_trace_over_button,340+(LOWORD(lPar)-350)/4,1,(LOWORD(lPar)-350)/4 - 5,27,true);
      MoveWindow(DWin_run_button,345+(LOWORD(lPar)-350)/2,1,(LOWORD(lPar)-350)/2 - 5,27,true);
      MoveWindow(GetDlgItem(Win,1020),340,30,LOWORD(lPar)-10-340-100-50,200,true);
      MoveWindow(GetDlgItem(Win,1021),LOWORD(lPar)-10-100-50,30,95,27,true);
      MoveWindow(GetDlgItem(Win,1022),LOWORD(lPar)-10-50,30,50,27,true);

      MoveWindow(m_b_mem_disa.handle,10,118,LOWORD(lPar)/2 - 13,HIWORD(lPar)-130,true);
      MoveWindow(DWin_timings_scroller,LOWORD(lPar)/2 + 3,148,LOWORD(lPar)/2 - 13,HIWORD(lPar)-160,true);
      MoveWindow(m_b_stack.handle,LOWORD(lPar)/2 + 3,148,LOWORD(lPar)/2 - 13,HIWORD(lPar)-160,true);
      SetWindowPos(DWin_right_display_combo,NULL,LOWORD(lPar)/2 + 3,118,LOWORD(lPar)/2 - 13,200,SWP_NOZORDER);
      m_b_mem_disa.update();
      m_b_stack.update();
      break;
    case WM_CLOSE:
      QuitSteem();
      return 0;
    case WM_DESTROY:
  //    m_b_mem_disa.active=false;
  //    m_b_stack.active=false;
      DWin=NULL;
      break;
    case WM_CONTEXTMENU:
      if ((HWND)wPar==DWin){
        POINT pt={0,0};ClientToScreen(DWin,&pt);
        if (HIWORD(lPar)-pt.y<0) break;
      }
      insp_menu_subject_type=78; //78=vague click
      insp_menu_subject=(void*)NULL;

      RemoveAllMenuItems(insp_menu);

      AppendMenu(insp_menu,MF_ENABLED | MF_STRING,3001,
        "New instruction browser at pc");
      AppendMenu(insp_menu,MF_ENABLED | MF_STRING,3002,
        "New memory browser at pc");
      AppendMenu(insp_menu,MF_ENABLED | MF_STRING,3003,
        "Register browser");
      TrackPopupMenu(insp_menu,TPM_LEFTALIGN | TPM_LEFTBUTTON,
        LOWORD(lPar),HIWORD(lPar),0,DWin,NULL);

      break;
    case WM_COMMAND:
    {
      if (simultrace!=0 && simultrace!=SIMULTRACE_CHOOSE){
        SendMessage(simultrace,Mess,wPar,lPar);
      }
      int id=LOWORD(wPar);
      if (HIWORD(wPar)==STN_CLICKED || HIWORD(wPar)==BN_CLICKED){
        if (id>=4000 && id<=4150){
          id-=4000;
          int rn=id & 31;
          int n=id/32;
          if((insp_menu_long_bytes[n])==2){
            *(WORD*)(reg_browser_entry_pointer[rn])=LOWORD(insp_menu_long[n]);
          }else{
            *(reg_browser_entry_pointer[rn])=insp_menu_long[n];
          }
          update_register_display(true);
        }else if (id>=1110 && id<1110+MAX_BREAKPOINTS){
          new mem_browser(breakpoint[id-1110],DT_INSTRUCTION);
        }else if (id>=1110+MAX_BREAKPOINTS && id<1110+MAX_BREAKPOINTS+MAX_BREAKPOINTS){
          new mem_browser(monitor_ad[id-(1110+MAX_BREAKPOINTS)],DT_MEMORY);
        }else if (id>=1110+MAX_BREAKPOINTS+MAX_BREAKPOINTS && id<1110+MAX_BREAKPOINTS+MAX_BREAKPOINTS+MAX_BREAKPOINTS){
          new mem_browser(monitor_io_ad[id-(1110+MAX_BREAKPOINTS+MAX_BREAKPOINTS)],DT_MEMORY);
        }else if (id>=301 && id<400){
          id-=300;
          logsection_enabled[id]=!logsection_enabled[id];
          CheckMenuItem(logsection_menu,LOWORD(wPar),MF_BYCOMMAND| int(logsection_enabled[id] ? MF_CHECKED:MF_UNCHECKED));
          if (id==LOGSECTION_CPU){
            if (logsection_enabled[id]) log_cpu_count=CPU_INSTRUCTIONS_TO_LOG;
          }
        }else if (id>=910 && id<1000){
          SetForegroundWindow(m_b[id-910]->owner);
        }else if (id>=17000 && id<20000){
          new mem_browser(pc_history[id-17000],DT_INSTRUCTION);
        }else if (id>=9000 && id<9040){
          id-=9000;
          bool set=0;
          switch (id){
            case 30: /*Check All*/
              set=true;
            case 31: /*Uncheck All*/
              for (int n=0;n<NUM_BREAK_IRQS;n++){
                break_on_irq[n]=set;
                CheckMenuItem(breakpoint_irq_menu,9000+n,int(set ? MF_CHECKED:MF_UNCHECKED));
              }
              break;
            default:
              break_on_irq[id]=!break_on_irq[id];
              CheckMenuItem(breakpoint_irq_menu,9000+id,int(break_on_irq[id] ? MF_CHECKED:MF_UNCHECKED));
          }
        }else{
          switch (id){
            case 3001:
              new mem_browser(pc,DT_INSTRUCTION);
              break;
            case 3002:
              new mem_browser(pc,DT_MEMORY);
              break;
            case 3003:
              new mem_browser(0,DT_REGISTERS);
              break;
            case 3010:case 3011:{
              new mem_browser((MEM_ADDRESS)(insp_menu_long[wPar & 1]&0xfffffe),
                  DT_INSTRUCTION);
              break;
            }
            case 3012:case 3013:{
              new mem_browser((MEM_ADDRESS)(insp_menu_long[wPar & 1]&0xfffffe),
                  DT_MEMORY);
              break;
            }
            case 3015:{
              mr_static*ms=(mr_static*)insp_menu_subject;
              if(ms->editflag){
                set_DWin_edit(0,ms,0,0);
              }break;
            }
            case 3025:
            {
              mem_browser*mb=(mem_browser*)insp_menu_subject;
              if(mb->editflag){
                set_DWin_edit(1,(void*)mb,insp_menu_row,insp_menu_col);
              }
              break;
            }
            case 104:
            {
              //disassemble file
              EasyStr sfn=FileSelect(DWin,"Select a program to disassemble",DiskMan.HomeFol,
                                      "ST Program Files\0*.PRG;*.APP;*.TOS;*.TTP;*.GTP\0All Files\0*.*\0\0",1,
                                      true,"PRG");
              if (sfn.NotEmpty()){
                EasyStr dfn=FileSelect(DWin,"Save disassembly as",WriteDir,
                                        "text\0*.TXT\0All Files\0*.*\0\0",1,false,"TXT");
                if (dfn.NotEmpty()){
                  FILE *sf=fopen(sfn.Text,"rb");
                  if (sf){
                    int prg_len=filelength(fileno(sf))-28;
                    fseek(sf,28,SEEK_SET);
        //            fread(Mem+0x400,prg_len,1,sf);
                    for (int m=0;m<prg_len;m++){
                      PEEK(0x400+m)=(BYTE)fgetc(sf);
                    }
                    fclose(sf);
                    char *tp=dfn.Right();
                    bool add_extn=true;
                    for (int m=0;m<dfn.Length();m++){
                      if (*tp=='.'){
                        add_extn=false;
                        break;
                      }else if (*tp=='\\' || *tp=='/' || *tp==':'){
                        break;
                      }
                      tp--;
                    }
                    if (add_extn) dfn+=".txt";
                    FILE *f=fopen(dfn.Text,"wb");
                    if (f){
                      if(strcmpi(dfn.Rights(2),".S")){
                        disa_to_file(f,0x400,prg_len,false);
                      }else{
                        disa_to_file(f,0x400,prg_len,true);
                      }
                      fclose(f);
                    }else{
                      MessageBox(NULL,EasyStr("Can't open file ")+dfn,"Oi!",
                                  MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_TOPMOST);
                    }
                  }else{
                    MessageBox(NULL,EasyStr("Can't load file ")+sfn,"Oi!",
                                MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_TOPMOST);
                  }
                }

              }
              break;
            }
            case 1001:
            {
              //draw a file
              EasyStr sfn=FileSelect(DWin,"Select a picture",WriteDir,
                                      "Raw Image Files\0*.IMG\0All Files\0*.*\0\0",1,true,"IMG");
              if (sfn.NotEmpty()){
                FILE *sf=fopen(sfn.Text,"rb");
                if (sf){
                  int pic_len=filelength(fileno(sf));
                  if (pic_len==32000){
                    if (xbios2+32000>mem_len){
                      xbios2=0x4000;
                    }
                    for (int m=0;m<32000;m++){
                      PEEK(xbios2+m)=(BYTE)fgetc(sf);
                    }
                  }
                  fclose(sf);
                  draw(false);
                }else{
                  MessageBox(NULL,EasyStr("Can't load file ")+sfn,"Oi!",
                                      MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_TOPMOST);
                }
              }
              break;
            }
            case 1003:  //Step over
            {
              bool step_over=0;
              Str Inst=disa_d2(pc);
              log_to(LOGSECTION_GUI,Str("GUI: Received step over click, instruction=\"")+Inst+"\"");
              if (IsSameStr_I(Inst.Lefts(2),"db")){ //dbCC
                step_over=true;
              }else if (MatchesAnyString_I(Inst.Lefts(3),"bsr","jsr",NULL)){
                step_over=true;
              }else if (MatchesAnyString_I(Inst.Lefts(4),"trap","stop","line",NULL)){
                step_over=true;
              }
              if (step_over){
                trace_over_breakpoint=oi(pc,1);
                log_to(LOGSECTION_GUI,Str("GUI: Running until hit $")+HEXSl(trace_over_breakpoint,6));
                PostRunMessage();
                break;
              }
              // Trace any other instruction
              log_to(LOGSECTION_GUI,Str("GUI: Tracing"));
            }
            case 1002:  //Trace into
              trace();
              break;
            case 1004:  //reset
              reset_st();
              break;
            case 1005:   //view logfile
              if (logfile) ShellExecute(NULL,NULL,LogViewProg,LogFileName,"",SW_SHOW);
              break;
            case 1014:
            {
              EnableAllWindows(0,DWin);

              EasyStr CurLogPath=LogViewProg;
              RemoveFileNameFromPath(CurLogPath,REMOVE_SLASH);
              EasyStr FilNam=FileSelect(DWin,"Choose Program",CurLogPath,"Executables\0*.exe\0\0",
                                          1,true,"exe",GetFileNameFromPath(LogViewProg));
              if (FilNam.NotEmpty()){
                LogViewProg=FilNam;
              }
              SetForegroundWindow(DWin);
              EnableAllWindows(true,DWin);
              break;
            }
            case 1006:   //add mark in logfile
              if (logfile){
                fprintf(logfile,"\r\n\r\n***************************************************************************\r\n");
                fprintf(logfile,"***************************************************************************\r\n");
                fprintf(logfile,"***************************************************************************\r\n\r\n\r\n");
                fflush(logfile);
              }
              break;
            case 1007: //wipe logfile
              logfile_wipe();
              break;
            case 1013:
              debug_wipe_log_on_reset=!debug_wipe_log_on_reset;
              CheckMenuItem(logsection_menu,1013,MF_BYCOMMAND | int(debug_wipe_log_on_reset ? MF_CHECKED:MF_UNCHECKED));
              break;
            case 1008: //simultrace
              if(simultrace==SIMULTRACE_CHOOSE){
                simultrace=NULL;
                CheckMenuItem(menu1,1008,MF_BYCOMMAND|MF_UNCHECKED);
              }else if(simultrace){
                SetWindowText(simultrace,"The Boiler Room");
                SetWindowText(Win,"The Boiler Room");
                simultrace=NULL;
                CheckMenuItem(menu1,1008,MF_BYCOMMAND | MF_UNCHECKED);
              }else{
                MessageBox(NULL,"Move the mouse over the window that you want to control and press S","Simultrace",
                            MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
                simultrace=SIMULTRACE_CHOOSE;
              }
              break;
            case 1009: //Uncheck all logsections
              for (int n=1;logsections[n].Name[0]!='*';n++){
                int i=logsections[n].Index;
                if (i>=0){
                  logsection_enabled[i]=0;
                  CheckMenuItem(logsection_menu,300+i,MF_BYCOMMAND | MF_UNCHECKED);
                }
              }
              break;
            case 1012: //suspend logging
              logging_suspended=!logging_suspended;
              CheckMenuItem(logsection_menu,1012,MF_BYCOMMAND | int(logging_suspended ? MF_CHECKED:MF_UNCHECKED));
              break;
            case 1025: //redraw on stop
              redraw_on_stop=!redraw_on_stop;
              CheckMenuItem(menu1,1025,MF_BYCOMMAND | int(redraw_on_stop ? MF_CHECKED:MF_UNCHECKED));
              break;
            case 1026: //redraw after trace
              redraw_after_trace=!redraw_after_trace;
              CheckMenuItem(menu1,1026,MF_BYCOMMAND | int(redraw_after_trace ? MF_CHECKED:MF_UNCHECKED));
              break;
            case 1027: //Gun position colour
            {
              CHOOSECOLOR cc;
              cc.lStructSize=sizeof(cc);
              cc.hwndOwner=DWin;
              cc.rgbResult=debug_gun_pos_col;
              COLORREF CustCols[16];
              for (int n=0;n<16;n++) CustCols[n]=0;
              cc.lpCustColors=CustCols;
              cc.Flags=CC_FULLOPEN | CC_RGBINIT;
              if (ChooseColor(&cc)){
                debug_gun_pos_col=cc.rgbResult;
                update_display_after_trace();
              }
              break;
            }
            case 1501:case 1502:case 1503:  //crash notification
              crash_notification=LOWORD(wPar)-1501;
              CheckMenuRadioItem(boiler_op_menu,1501,1503,1501+crash_notification,MF_BYCOMMAND);
              break;
            case 1510:
              stop_on_blitter_flag=!stop_on_blitter_flag;
              CheckMenuItem(boiler_op_menu,1510,int(stop_on_blitter_flag ? (MF_BYCOMMAND|MF_CHECKED):(MF_BYCOMMAND|MF_UNCHECKED)));
              break;
            case 1512:
              stop_on_user_change=!stop_on_user_change;
              CheckMenuItem(boiler_op_menu,1512,MF_BYCOMMAND | int(stop_on_user_change!=0 ? MF_CHECKED:MF_UNCHECKED));
              break;
            case 1513:
              stop_on_next_program_run=int(stop_on_next_program_run ? 0:1);
              CheckMenuItem(boiler_op_menu,1513,MF_BYCOMMAND | int(stop_on_next_program_run ? MF_CHECKED:MF_UNCHECKED));
              break;
            case 1514:
              trace_show_window=!trace_show_window;
              CheckMenuItem(boiler_op_menu,1514,MF_BYCOMMAND | int(trace_show_window ? MF_CHECKED:MF_UNCHECKED));
              break;

            case 1780: //turn screen red
            {
              MEM_ADDRESS ad=xbios2;
              for (int o=32000/8;o>0;o--){
                m68k_lpoke(ad,0xffff0000);ad+=4;
                m68k_lpoke(ad,0);ad+=4;
              }
              draw(false);
              break;
            }
            case 1781: //Send MIDI messages
              if (MIDIPort.MIDI_In==NULL) return 0;
              if (MIDIPort.MIDI_In->Handle==NULL) return 0;

              for (int n=1;n<60;n++) MIDIPort.MIDI_In->SysExHeader[0].lpData[n]=b00111100;
              MIDIPort.MIDI_In->SysExHeader[0].lpData[0]=b11110000;
              MIDIPort.MIDI_In->SysExHeader[0].lpData[60]=b11110111;
              MIDIPort.MIDI_In->SysExHeader[0].dwBytesRecorded=61;
              MIDIPort.MIDI_In->InProc(MIDIPort.MIDI_In->Handle,MIM_LONGDATA,DWORD(MIDIPort.MIDI_In),(DWORD)&MIDIPort.MIDI_In->SysExHeader[0],0);

              MIDIPort.MIDI_In->InProc(MIDIPort.MIDI_In->Handle,MIM_DATA,DWORD(MIDIPort.MIDI_In),MAKEWORD(b10110001,0),0);
              MIDIPort.MIDI_In->InProc(MIDIPort.MIDI_In->Handle,MIM_DATA,DWORD(MIDIPort.MIDI_In),MAKEWORD(b10101010,3),0);

              MIDIPort.MIDI_In->SysExHeader[0].lpData[0]=b11110000;
              for (int n=1;n<32;n++) MIDIPort.MIDI_In->SysExHeader[0].lpData[n]=char(n);
              MIDIPort.MIDI_In->SysExHeader[0].lpData[32]=b11110111;
              MIDIPort.MIDI_In->SysExHeader[0].dwBytesRecorded=33;
              MIDIPort.MIDI_In->InProc(MIDIPort.MIDI_In->Handle,MIM_LONGDATA,DWORD(MIDIPort.MIDI_In),(DWORD)&MIDIPort.MIDI_In->SysExHeader[0],0);
              break;
            case 1782: // Send All Keys
              debug_send_alt_keys=0x2;
              debug_send_alt_keys_vbl_countdown=1;
              break;
            case 1789:
              if (debug_cycle_colours){
                debug_cycle_colours=0;
                palette_convert_all();
              }else{
                debug_cycle_colours=1;
              }
              CheckMenuItem(menu1,1789,MF_BYCOMMAND | int(debug_cycle_colours ? MF_CHECKED:MF_UNCHECKED));
              draw(false);
              break;
            case 1600:case 1601:case 1602:case 1603:
              debug_screen_shift=(LOWORD(wPar)-1600)*2;
              CheckMenuRadioItem(shift_screen_menu,1600,1603,1600+(debug_screen_shift/2),MF_BYCOMMAND);
              break;
            case 1010:  //run
            {
              PostRunMessage();
              break;
            }
            case 1011:  //run to rte
            {
              if (runstate==RUNSTATE_STOPPED){
                on_rte=ON_RTE_STOP;
                on_rte_interrupt_depth=interrupt_depth;
                PostRunMessage();
              }
              break;
            }
            case 1100:  //clear all breakpoints
              num_breakpoints=0;
              UPDATE_DO_BREAK_CHECK;
              breakpoint_menu_setup();
              mem_browser_update_all();
              break;
            case 1107:   //toggle breakpoint checking
            case 1108:   //toggle breakpoint checking to logfile
              if (id==1107) breakpoint_mode=int((breakpoint_mode==1) ? 0:1);
              if (id==1108) breakpoint_mode=int((breakpoint_mode==2) ? 0:2);
              UPDATE_DO_BREAK_CHECK;
              CheckMenuItem(breakpoint_menu,1107,MF_BYCOMMAND | int((breakpoint_mode==1) ? MF_CHECKED:MF_UNCHECKED));
              CheckMenuItem(breakpoint_menu,1108,MF_BYCOMMAND | int((breakpoint_mode==2) ? MF_CHECKED:MF_UNCHECKED));
              break;
            case 1101:   //set breakpoint at pc
              set_breakpoint_or_monitor(0,pc);
              break;

            case 1106:  //clear all monitors
              num_monitors=0;
              num_io_monitors=0;
              UPDATE_DO_MON_CHECK;
              breakpoint_menu_setup();
              mem_browser_update_all();
              break;
            case 1103:   //toggle monitoring
            case 1104:   //toggle monitoring to logfile
            case 1105:   //set monitor on screen
              if (id==1103) monitor_mode=int((monitor_mode==1) ? 0:1);
              if (id==1104) monitor_mode=int((monitor_mode==2) ? 0:2);
              if (id==1105){
                monitor_mode=1;
                stem_mousemode=STEM_MOUSEMODE_BREAKPOINT;
              }
              UPDATE_DO_MON_CHECK;
              CheckMenuItem(monitor_menu,1103,MF_BYCOMMAND | int((monitor_mode==1) ? MF_CHECKED:MF_UNCHECKED));
              CheckMenuItem(monitor_menu,1104,MF_BYCOMMAND | int((monitor_mode==2) ? MF_CHECKED:MF_UNCHECKED));
              break;

            case 1999:  //quit
              QuitSteem();
              break;
            case 2200:
              HistList.Show();
              break;
            //////////////////// Browsers Menu
            case 900:case 901:
            {
              mem_browser *mb=new mem_browser(pc,type_disp_type(LOWORD(wPar)==900 ? DT_MEMORY:DT_INSTRUCTION));
              SetFocus(GetDlgItem(mb->owner,3));
              SendMessage(GetDlgItem(mb->owner,3),WM_LBUTTONDOWN,0,0);
              break;
            }
            case 902:
              new mem_browser(0,DT_REGISTERS);
              break;
            case 903:
              new mem_browser(IOLIST_PSEUDO_AD_PSG,DT_MEMORY);
              break;
            case 904:
              new mem_browser(0xfffa00,DT_MEMORY);
              break;
            case 906:
            {
              mem_browser *mb=new mem_browser;
              mb->init_text=true;
              mb->new_window(MEM_START_OF_USER_AREA,DT_MEMORY);
              SetFocus(GetDlgItem(mb->owner,3));
              SendMessage(GetDlgItem(mb->owner,3),WM_LBUTTONDOWN,0,0);
              break;
            }
            case 905:
              for(int n=0;n<MAX_MEMORY_BROWSERS;n++){
                if(m_b[n]!=NULL)PostMessage(m_b[n]->owner,WM_CLOSE,0,0);
              }
              break;
            case 907:
              if (mem_browser::ex_style){
                mem_browser::ex_style=0;
              }else{
                mem_browser::ex_style=WS_EX_TOOLWINDOW;
              }
              CheckMenuItem(mem_browser_menu,907,MF_BYCOMMAND | int(mem_browser::ex_style ? 0:MF_CHECKED));
              break;
            case 908:
              new mem_browser(IOLIST_PSEUDO_AD_FDC,DT_MEMORY);
              break;
            case 909:
              new mem_browser(IOLIST_PSEUDO_AD_IKBD,DT_MEMORY);
              break;
            case 1022:
            {
              DWORD dat=CBGetSelectedItemData(GetDlgItem(Win,1020));
              debug_run_until=LOWORD(dat);

              int len=SendDlgItemMessage(Win,1021,WM_GETTEXTLENGTH,0,0)+1;
              EasyStr valstr;
              valstr.SetLength(len);
              SendDlgItemMessage(Win,1021,WM_GETTEXT,len,LPARAM(valstr.Text));
              debug_run_until_val=atoi(valstr);

              if (debug_run_until==DRU_CYCLE) debug_run_until_val+=ABSOLUTE_CPU_TIME;

              if (runstate==RUNSTATE_STOPPED) PostRunMessage();
              break;
            }
          }  //end switch
        }
      }
      if (id==209 && HIWORD(wPar)==CBN_SELENDOK) boiler_show_stack_display(-1);
      break;
    }
    case WM_CHAR:
    {
      if(wPar=='S' || wPar=='s'){
        if(simultrace==SIMULTRACE_CHOOSE){
          POINT pt;
          GetCursorPos(&pt);
          HWND sw=WindowFromPoint(pt);
          if(sw){
            simultrace=sw;
            SetWindowText(Win,"Master Boiler Room");
            SetWindowText(simultrace,"Slave Boiler Room");
            CheckMenuItem(menu1,1008,MF_BYCOMMAND|MF_CHECKED);
          }
        }
      }
      break;
    }
    case WM_INITMENUPOPUP:
      if ((HMENU)wPar==mem_browser_menu){
        int n=GetMenuItemCount(mem_browser_menu);
        for (int i=0;i<n-12;i++){
          DeleteMenu(mem_browser_menu,12,MF_BYPOSITION);
        }
        bool NoBar=true;
        for (int i=0;i<MAX_MEMORY_BROWSERS;i++){
          if (m_b[i]!=NULL){
            if (NoBar && m_b[i]->disp_type!=DT_REGISTERS){
              AppendMenu(mem_browser_menu,MF_STRING | MF_SEPARATOR,0,NULL);
              NoBar=0;
            }
            switch (m_b[i]->disp_type){
              case DT_MEMORY:
                if (IS_IOLIST_PSEUDO_ADDRESS(m_b[i]->ad)){
                  switch (m_b[i]->ad & 0xfffff000){
                    case IOLIST_PSEUDO_AD_PSG:
                      AppendMenu(mem_browser_menu,MF_STRING,910+i,"PSG Browser");
                      break;
                    case IOLIST_PSEUDO_AD_FDC:
                      AppendMenu(mem_browser_menu,MF_STRING,910+i,"FDC Browser");
                      break;
                    case IOLIST_PSEUDO_AD_IKBD:
                      AppendMenu(mem_browser_menu,MF_STRING,910+i,"IKBD Browser");
                      break;
                  }
                }else{
                  AppendMenu(mem_browser_menu,MF_STRING,910+i,EasyStr(HEXSl(m_b[i]->ad,6))+" - Memory");
                }
                break;
              case DT_INSTRUCTION:
                AppendMenu(mem_browser_menu,MF_STRING,910+i,EasyStr(HEXSl(m_b[i]->ad,6))+" - Instructions");
                break;
              case DT_REGISTERS:
                AppendMenu(mem_browser_menu,MF_STRING,910+i,"Register Browser");
                break;
            }
          }
        }
        return 0;
      }else if ((HMENU)wPar==history_menu){
        RemoveAllMenuItems(history_menu);
        int n=pc_history_idx,c=0;
        EasyStr Dissasembly;
        do{
          n--;
          if (n<0) n=HISTORY_SIZE-1;
          if (pc_history[n]==0xffffff71) break;
          Dissasembly=disa_d2(pc_history[n]);
          InsertMenu(history_menu,0,MF_BYPOSITION | MF_STRING,17000+n,EasyStr(HEXSl(pc_history[n],6))+" - "+Dissasembly);
        }while (n!=pc_history_idx && (c++)<HIST_MENU_SIZE);
        InsertMenu(history_menu,0,MF_BYPOSITION | MF_STRING | MF_SEPARATOR,99,"-");
        InsertMenu(history_menu,0,MF_BYPOSITION | MF_STRING,2200,"History List");
        return 0;
      }else if ((HMENU)wPar==breakpoint_menu || (HMENU)wPar==monitor_menu){
        breakpoint_menu_setup();
        return 0;
      }
      break;
  }
	return DefWindowProc(Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void disa_to_file(FILE*f,MEM_ADDRESS dstart,int dlen,bool as_source)
{
  MEM_ADDRESS dend=dstart+dlen,odpc;
  EasyStr dt,ot;
  int tp;
  char t[20];
  dpc=dstart;
  while (dpc<dend){
    odpc=dpc;
    if(as_source){
      dt=disa_d2(dpc);
      ot=EasyStr("\t")+dt;
      while (ot.Length()<50) ot+=" ";
      ot+="; ";
      while (odpc<dpc){
        ot+=HEXSl(d2_dpeek(odpc),4);
        ot+=" ";
        odpc+=2;
      }
    }else{
      ot="000000 : 0000 0000 0000 0000 0000 : ";
      itoa(dpc-dstart,t,16);
      memcpy((ot.Text+6)-strlen(t),t,strlen(t));
      dt=disa_d2(dpc);
      tp=13;
      while (odpc<dpc){
        itoa(d2_dpeek(odpc),t,16);
        memcpy((ot.Text+tp)-strlen(t),t,strlen(t));
        tp+=5;
        odpc+=2;
      }
      while (tp<34){
        memcpy(ot.Text+(tp-4),"    ",4);
        tp+=5;
      }
      ot+=dt;
    }
    fprintf(f,"%s\r\n",ot.Text);
  }
}
//---------------------------------------------------------------------------
#define LOGSECTION LOGSECTION_INIT
void boiler_show_stack_display(int sel)
{
  if (sel==-1){
    sel=SendDlgItemMessage(DWin,209,CB_GETCURSEL,0,0);
  }else{
    SendDlgItemMessage(DWin,209,CB_SETCURSEL,sel,0);
  }

  LockWindowUpdate(DWin);
  ShowWindow(DWin_timings_scroller,int((sel==1) ? SW_SHOW:SW_HIDE));
  ShowWindow(m_b_stack.handle,int((sel==0) ? SW_SHOW:SW_HIDE));
  LockWindowUpdate(NULL);
}

void DWin_init()
{
  char ttt[200];
  int x,y;
  log("STARTUP: Setting up Reg Browser");
  {//set up reg browser
    int n=1;
    strcpy(reg_browser_entry_name[0],"pc");reg_browser_entry_pointer[0]=&pc;
    for(int m=0;m<16;m++){
      ttt[0]="da"[int((m & 8) ? 1:0)];
      ttt[1]=(char)('0'+(m & 7));
      ttt[2]=0;
      strcpy(reg_browser_entry_name[n],ttt);reg_browser_entry_pointer[n]=(unsigned long*)&(r[m]);
      n++;
    }
    strcpy(reg_browser_entry_name[n],"oth/sp");reg_browser_entry_pointer[n]=(unsigned long*)&(r[15]);
    n++;
    reg_browser_entry_name[n][0]=0;
  }
  log("STARTUP: Reg Browser Done");

  shift_screen_menu=CreatePopupMenu();
  AppendMenu(shift_screen_menu,MF_STRING,1600,"0 bytes");
  AppendMenu(shift_screen_menu,MF_STRING,1601,"2 bytes");
  AppendMenu(shift_screen_menu,MF_STRING,1602,"4 bytes");
  AppendMenu(shift_screen_menu,MF_STRING,1603,"6 bytes");
  CheckMenuRadioItem(shift_screen_menu,1600,1603,1600+(debug_screen_shift/2),MF_BYCOMMAND);

  log("STARTUP: Creating Menu");
  menu=CreateMenu();
  menu1=CreatePopupMenu();
  AppendMenu(menu1,MF_STRING,104,"&Disassemble a File");
  AppendMenu(menu1,MF_STRING,1001,"Load a &Picture");
  AppendMenu(menu1,MF_STRING,1002,"&Trace");
  AppendMenu(menu1,MF_STRING,1010,"&Run");
  AppendMenu(menu1,MF_STRING,1011,"Run to RT&E");

  AppendMenu(menu1,MF_STRING|MF_SEPARATOR,0,"-");
  AppendMenu(menu1,MF_STRING,1004,"&Cold Reset");

  AppendMenu(menu1,MF_STRING|MF_SEPARATOR,0,"-");
  AppendMenu(menu1,MF_STRING,1008,"&Simul-trace");
  AppendMenu(menu1,MF_STRING,1780,"Turn Screen Red");
  AppendMenu(menu1,MF_STRING,1781,"Send MIDI message");
  AppendMenu(menu1,MF_STRING,1782,"Send Key Codes With Alt");

  AppendMenu(menu1,MF_STRING|MF_SEPARATOR,0,"-");
  AppendMenu(menu1,MF_STRING,1789,"Psy&chedelic Mode");
  AppendMenu(menu1,MF_STRING|MF_POPUP,(int)shift_screen_menu,"Shift Display");
  AppendMenu(menu1,MF_STRING,1025,"Redraw On Stop");
  AppendMenu(menu1,MF_STRING,1026,"Redraw After Trace");
  AppendMenu(menu1,MF_STRING,1027,"Choose Gun Position Display Colour");

  AppendMenu(menu1,MF_STRING|MF_SEPARATOR,0,"-");
  AppendMenu(menu1,MF_STRING,1999,"&Quit");
  AppendMenu(menu,MF_STRING|MF_POPUP,(UINT)menu1,"&Test");
  log("STARTUP: Menu Done");

  log("STARTUP: Creating Breakpoint Menu");
  breakpoint_menu=CreatePopupMenu();
  breakpoint_irq_menu=CreatePopupMenu();
  monitor_menu=CreatePopupMenu();

  AppendMenu(menu,MF_STRING|MF_POPUP,(UINT)breakpoint_menu,"B&reakpoints");
  AppendMenu(menu,MF_STRING|MF_POPUP,(UINT)monitor_menu,"&Monitors");

  breakpoint_menu_setup();
  log("STARTUP: Breakpoint Menu Done");

  mem_browser_menu=CreatePopupMenu();
  AppendMenu(menu,MF_STRING | MF_POPUP,(UINT)mem_browser_menu,"&Browsers");
  AppendMenu(mem_browser_menu,MF_STRING,900,"New &Memory Browser");
  AppendMenu(mem_browser_menu,MF_STRING,901,"New &Instruction Browser");
  AppendMenu(mem_browser_menu,MF_STRING,902,"New &Register Browser");
  AppendMenu(mem_browser_menu,MF_STRING,903,"New &PSG Browser");
  AppendMenu(mem_browser_menu,MF_STRING,904,"New M&FP Browser");
  AppendMenu(mem_browser_menu,MF_STRING,906,"New &Text Browser");
  AppendMenu(mem_browser_menu,MF_STRING,908,"New &FDC Browser");
  AppendMenu(mem_browser_menu,MF_STRING,909,"New I&KBD Browser");
  AppendMenu(mem_browser_menu,MF_STRING|MF_SEPARATOR,0,NULL);
  AppendMenu(mem_browser_menu,MF_STRING | int(mem_browser::ex_style ? 0:MF_CHECKED),
                            907,"Put Them On Taskbar &Sucker");
  AppendMenu(mem_browser_menu,MF_STRING|MF_SEPARATOR,0,NULL);
  AppendMenu(mem_browser_menu,MF_STRING,905,"&Close All");
  log("STARTUP: mem_browser menu done");

  history_menu=CreatePopupMenu();
  AppendMenu(menu,MF_STRING | MF_POPUP,(UINT)history_menu,"&History");

  logsection_menu=CreatePopupMenu();
  AppendMenu(menu,MF_STRING | MF_POPUP,(UINT)logsection_menu,"&Log");
  AppendMenu(logsection_menu,MF_STRING,1005,"&View Logfile");
  AppendMenu(logsection_menu,MF_STRING,1014,"&Set Log Viewing Program");
  AppendMenu(logsection_menu,MF_SEPARATOR,0,NULL);
  AppendMenu(logsection_menu,MF_STRING,1006,"Add &Mark");
  AppendMenu(logsection_menu,MF_STRING,1007,"Wipe Logfile");
  AppendMenu(logsection_menu,MF_STRING|
        int(debug_wipe_log_on_reset ? MF_CHECKED:MF_UNCHECKED),1013,"Wipe On Reset");
  AppendMenu(logsection_menu,MF_SEPARATOR,0,NULL);
  AppendMenu(logsection_menu,MF_SEPARATOR,0,NULL);
  for (int n=1;logsections[n].Name[0]!='*';n++){
    int i=logsections[n].Index;
    if (logsections[n].Name[0]=='-'){
      AppendMenu(logsection_menu,MF_SEPARATOR,0,NULL);
    }else{
      AppendMenu(logsection_menu,MF_STRING| int(logsection_enabled[i] ? MF_CHECKED:0),300+i,logsections[n].Name);
    }
  }
  AppendMenu(logsection_menu,MF_SEPARATOR,0,NULL);
  AppendMenu(logsection_menu,MF_SEPARATOR,0,NULL);
  AppendMenu(logsection_menu,MF_STRING,1009,"&Uncheck All");
  AppendMenu(logsection_menu,MF_STRING,1012,"Suspend Logging");

  boiler_op_menu=CreatePopupMenu();
  AppendMenu(boiler_op_menu,MF_STRING,1501,"Notify on &all m68k exceptions 2-8");
  AppendMenu(boiler_op_menu,MF_STRING,1502,"Notify only on crash with &bombs");
  AppendMenu(boiler_op_menu,MF_STRING,1503,"&Don't notify on exceptions");
  AppendMenu(boiler_op_menu,MF_STRING|MF_SEPARATOR,0,"-");
  AppendMenu(boiler_op_menu,MF_STRING,1510,"Stop on blitter");
  AppendMenu(boiler_op_menu,MF_STRING,1512,"Stop on switch to user mode");
  AppendMenu(boiler_op_menu,MF_STRING,1513,"Stop on next program run");
  AppendMenu(boiler_op_menu,MF_STRING|MF_SEPARATOR,0,"-");
  AppendMenu(boiler_op_menu,MF_STRING | MF_CHECKED,1514,"Show trace window");

//  AppendMenu(boiler_op_menu,MF_STRING|MF_SEPARATOR,0,"-");
  AppendMenu(menu,MF_STRING|MF_POPUP,(UINT)boiler_op_menu,"&Options");

  log("STARTUP: calling iolist_init");

  iolist_init();

  log("STARTUP: iolist_init done");

  log("STARTUP: Creating Boiler Room Window");
  WNDCLASS wnd;
  wnd.style=CS_DBLCLKS;
  wnd.lpfnWndProc=DWndProc;
  wnd.cbWndExtra=0;
  wnd.cbClsExtra=0;
  wnd.hInstance=Inst;
  wnd.hIcon=hGUIIcon[RC_ICO_TRASH];
  wnd.hCursor=PCArrow;
  wnd.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
  wnd.lpszMenuName=NULL;
  wnd.lpszClassName="Steem Debug Window";
  RegisterClass(&wnd);

//  HiddenParent=CreateWindow("Steem Debug Window","Steem Hidden Window",0,0,0,0,0,NULL,NULL,Inst,NULL);

  DWin=CreateWindowEx(WS_EX_APPWINDOW,"Steem Debug Window",EasyStr("The Boiler Room: Steem v")+stem_version_text
      ,WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
       | WS_SIZEBOX
      ,60,60,640,400,ParentWin,menu,Inst,0);
  log("STARTUP: Boiler Room Window Done");

  {
    SIZE sz;
    HDC dc=GetDC(DWin);
    HFONT old_font=(HFONT)SelectObject(dc,fnt);
    GetTextExtentPoint32(dc,"CCCC ",5,&sz);
    SelectObject(dc,old_font);
    ReleaseDC(DWin,dc);
    how_big_is_0000=sz.cx;
  }

  log("STARTUP: Registering Mem Browser and Trace Window Classes");
  wnd.style=CS_DBLCLKS;
  wnd.lpfnWndProc=mem_browser_window_WndProc;
  wnd.cbWndExtra=0;
  wnd.cbClsExtra=0;
  wnd.hInstance=Inst;
  wnd.hIcon=hGUIIcon[RC_ICO_STCLOSE];
  wnd.hCursor=PCArrow;
  wnd.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
  wnd.lpszMenuName=NULL;
  wnd.lpszClassName="Steem Mem Browser Window";
  RegisterClass(&wnd);


  wnd.style=CS_DBLCLKS;
  wnd.lpfnWndProc=trace_window_WndProc;
  wnd.cbWndExtra=0;
  wnd.cbClsExtra=0;
  wnd.hInstance=Inst;
  wnd.hIcon=hGUIIcon[RC_ICO_STCLOSE];
  wnd.hCursor=PCArrow;
  wnd.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
  wnd.lpszMenuName=NULL;
  wnd.lpszClassName="Steem Trace Window";
  RegisterClass(&wnd);

  wnd.style=CS_CLASSDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
  wnd.lpfnWndProc=mr_static_WndProc;
  wnd.cbWndExtra=0;
  wnd.cbClsExtra=0;
  wnd.hInstance=Inst;
  wnd.hIcon=NULL;
  wnd.hCursor=PCArrow;
  wnd.hbrBackground=NULL;
  wnd.lpszMenuName=NULL;
  wnd.lpszClassName="Steem Mr Static Control";
  RegisterClass(&wnd);
  log("STARTUP: Mem Browser and Trace Window Classes Registered");

  log("STARTUP: Creating Child Windows");

  new mr_static(/*label*/"pc=",/*name*/"pc",/*x*/10,/*y*/1,
      /*owner*/DWin,/*id*/(HMENU)201,/*pointer*/(MEM_ADDRESS)&pc,
      /*bytes*/ 3,/*regflag*/ MST_REGISTER, /*editflag*/true,
      /*mem_browser to update*/&m_b_mem_disa);

  new mr_static("screen=","screen address",240,1,DWin,(HMENU)294,(MEM_ADDRESS)&xbios2,3,MST_REGISTER,true,NULL);

  lpms_other_sp=new mr_static("other sp=","other sp",110,1,
      DWin,(HMENU)203,(MEM_ADDRESS)&other_sp,3,MST_REGISTER,
      true,NULL);

  DWin_trace_button=CreateWindow("Button","Trace Into",WS_VISIBLE |
      WS_CHILD | BS_CHECKBOX | BS_PUSHLIKE,330,1,140,25,DWin,(HMENU)1002,Inst,NULL);

  DWin_trace_over_button=CreateWindow("Button","Step Over",WS_VISIBLE |
      WS_CHILD | BS_CHECKBOX | BS_PUSHLIKE,330,1,140,25,DWin,(HMENU)1003,Inst,NULL);

  DWin_run_button=CreateWindow("Button","Run",WS_VISIBLE | WS_CHILD | BS_CHECKBOX | BS_PUSHLIKE | WS_CLIPSIBLINGS,
      490,1,140,50,DWin,(HMENU)1010,Inst,NULL);

  HWND Win=CreateWindowEx(512,"Combobox","",WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
      330,26,100,200,DWin,(HMENU)1020,Inst,NULL);
  CBAddString(Win,"Run to next VBL",MAKELONG(DRU_VBL,0));
  CBAddString(Win,"Run to scanline n",MAKELONG(DRU_SCANLINE,0));
  CBAddString(Win,"Run for n cycles",MAKELONG(DRU_CYCLE,0));

  CreateWindowEx(512,"Edit","",WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS,
      490,26,140,25,DWin,(HMENU)1021,Inst,NULL);

  CreateWindow("Button","Go",WS_VISIBLE | WS_CHILD | BS_CHECKBOX | BS_PUSHLIKE | WS_CLIPSIBLINGS,
      490,26,140,25,DWin,(HMENU)1022,Inst,NULL);


  CreateWindowEx(0,"Static","sr = ",WS_VISIBLE | WS_CHILDWINDOW | SS_LEFT,
      10,34,30,20,DWin,(HMENU)274,Inst,NULL);


  sr_display=CreateWindowEx(512,"Static","sr display",WS_BORDER | WS_VISIBLE | WS_CHILDWINDOW | SS_NOTIFY,
      30,30,200,20,DWin,(HMENU)230,Inst,NULL);
  SetWindowLong(sr_display,GWL_USERDATA,(LONG)&sr);
  Old_sr_display_WndProc=(WNDPROC)SetWindowLong(sr_display,GWL_WNDPROC,(long)sr_display_WndProc);
  log("STARTUP: Subclassed sr display");

  for(int n=0;n<16;n++){
    strcpy(ttt,reg_name(n));strcat(ttt,"=");
    ttt[2]='=';ttt[3]=0;
    x=10+(n&7)*80;
    y=60+(n&8)*3;

    new mr_static(/*label*/ttt,/*name*/ttt,/*x*/x,/*y*/y,
        /*owner*/DWin,/*id*/(HMENU)(276+n),/*pointer*/(MEM_ADDRESS)&(r[n]),
        /*bytes*/ 4,/*regflag*/ MST_REGISTER, /*editflag*/true,
        /*mem_browser to update*/NULL);
  }

  m_b_mem_disa.handle=CreateWindowEx(512,WC_LISTVIEW,"Freak!",
      LVS_REPORT | LVS_SHAREIMAGELISTS | LVS_NOSORTHEADER | WS_VISIBLE | WS_CHILDWINDOW | WS_CLIPSIBLINGS,
      10,120,320,190,DWin,(HMENU)200,Inst,NULL);
  SetWindowLong(m_b_mem_disa.handle,GWL_USERDATA,(LONG)&m_b_mem_disa);
  Old_mem_browser_WndProc=(WNDPROC)SetWindowLong(m_b_mem_disa.handle,GWL_WNDPROC,
                                                (long)mem_browser_WndProc);
  log("STARTUP: SubClassed Mem_Browser Listview");

  //  m_b_mem_disa.active=true;
  m_b_mem_disa.owner=DWin;
  m_b_mem_disa.disp_type=DT_INSTRUCTION;
  m_b_mem_disa.ad=pc;
  m_b_mem_disa.mode=MB_MODE_PC;
  m_b_mem_disa.editbox=NULL;
  m_b_mem_disa.editflag=true;

  log("STARTUP: Initing Mem_Browser Listview");
  m_b_mem_disa.init();
  log("STARTUP: Mem_Browser Initialised");

  // User controlled area, stack or timings
  DWin_right_display_combo=CreateWindowEx(512,"Combobox","",WS_CHILD | WS_VISIBLE | CBS_HASSTRINGS | CBS_DROPDOWNLIST,
      345,120,320,200,DWin,(HMENU)209,Inst,NULL);
  SendMessage(DWin_right_display_combo,CB_ADDSTRING,0,LPARAM("Stack Display"));
  SendMessage(DWin_right_display_combo,CB_ADDSTRING,0,LPARAM("Timings Display"));

  // Stack
  m_b_stack.handle=CreateWindowEx(512,WC_LISTVIEW,"",
      LVS_REPORT | LVS_SHAREIMAGELISTS | LVS_NOSORTHEADER | WS_CHILD | WS_CLIPSIBLINGS,
      345,150,320,190,DWin,(HMENU)210,Inst,NULL);
  SetWindowLong(m_b_stack.handle,GWL_USERDATA,(LONG)&m_b_stack);
  SetWindowLong(m_b_stack.handle,GWL_WNDPROC,(long)mem_browser_WndProc);
  log("STARTUP: SubClassed M_B_Stack Listview");

  m_b_stack.owner=DWin;
  m_b_stack.disp_type=DT_MEMORY;
  m_b_stack.ad=pc;
  m_b_stack.mode=MB_MODE_STACK;
  m_b_stack.wpl=1;
  m_b_stack.editbox=NULL;
  m_b_stack.editflag=true;

  log("STARTUP: Initing M_B_Stack Listview");
  m_b_stack.init();


  { // Timings
    DWin_timings_scroller.CreateEx(512,WS_CHILD,0,0,1,1,DWin,220,Inst);

    HWND Par=DWin_timings_scroller.GetControlPage();
    RECT rc;
    int y=5;
    mr_static *ms;
    ms=new mr_static("Cycles Since VBL ","",5,y,Par,
        NULL,(MEM_ADDRESS)&debug_cycles_since_VBL,3,MST_DECIMAL,0,NULL);
    GetWindowRectRelativeToParent(ms->handle,&rc);

    new mr_static("Since HBL ","",rc.right+5,y,Par,
        NULL,(MEM_ADDRESS)&debug_cycles_since_HBL,2,MST_DECIMAL,0,NULL);
    y+=30;

    ms=new mr_static("Current Video Address ","",5,y,Par,
        NULL,(MEM_ADDRESS)&debug_VAP,3,MST_REGISTER,0,NULL);
    GetWindowRectRelativeToParent(ms->handle,&rc);
    
    new mr_static("Current Scanline ","",rc.right+5,y,Par,
        NULL,(MEM_ADDRESS)&scan_y,2,MST_DECIMAL,0,NULL);
    y+=30;

    int x=5;
    for (int t=0;t<4;t++){
      ms=new mr_static(Str("Timer ")+char('A'+t)+" ","",x,y,Par,
            NULL,(MEM_ADDRESS)&debug_time_to_timer_timeout[t],4,MST_DECIMAL,0,NULL);
      y+=30;
      if (t==1){
        y-=60;
        GetWindowRectRelativeToParent(ms->handle,&rc);
        x=rc.right+5;
      }
    }
    DWin_timings_scroller.AutoSize();
  }
  boiler_show_stack_display(0);

  trace_window_init();

  log("STARTUP: Creating DWin_edit");
  DWin_edit=CreateWindowEx(512,"Edit","hi",
      WS_BORDER | WS_CHILDWINDOW | WS_CLIPSIBLINGS | ES_AUTOHSCROLL,
      220,10,60,25,DWin,(HMENU)255,Inst,NULL);
  DWin_edit_subject=NULL;
  DWin_edit_subject_type=-1;
  Old_edit_WndProc=(WNDPROC)SetWindowLong(DWin_edit,GWL_WNDPROC,(long)DWin_edit_WndProc);
  BringWindowToTop(DWin_edit);
  log("STARTUP: DWin_edit Created");
  log("STARTUP: Child Windows Created");

  SetWindowAndChildrensFont(DWin,fnt);

  log("STARTUP: Creating insp_menu");
  insp_menu=CreatePopupMenu();
  for(int m=0;m<3;m++){
    insp_menu_reg_submenu[m]=CreatePopupMenu();
    for(int n=0;n<NUM_REGISTERS_IN_REGISTER_BROWSER;n++){
      AppendMenu(insp_menu_reg_submenu[m],MF_ENABLED | MF_STRING,4000+m*32+n,
                  reg_browser_entry_name[n]);
    }
  }
  log("STARTUP: insp_menu Created");

  #ifdef ENABLE_VARIABLE_SOUND_DAMPING

  int xx=240,yy=30;
  new mr_static(/*label*/"a ",/*name*/"Sound A /256",/*x*/xx,/*y*/yy,
      /*owner*/DWin,/*id*/(HMENU)50066,/*pointer*/(MEM_ADDRESS)&sound_variable_a,
      /*bytes*/ 1,/*regflag*/ MST_REGISTER, /*editflag*/true,
      /*mem_browser to update*/NULL);
  new mr_static(/*label*/"d ",/*name*/"Sound D /256",/*x*/xx+50,/*y*/yy,
      /*owner*/DWin,/*id*/(HMENU)50067,/*pointer*/(MEM_ADDRESS)&sound_variable_d,
      /*bytes*/ 1,/*regflag*/ MST_REGISTER, /*editflag*/true,
      /*mem_browser to update*/NULL);

  #endif
}

void logfile_wipe()
{
  if (logfile){
    fclose(logfile);
    logfile=fopen(LogFileName,"wb");
  }
}

void stop_new_program_exec()
{
  stop_on_user_change=0;
  stop_on_next_program_run=0;
  CheckMenuItem(boiler_op_menu,1513,MF_BYCOMMAND | MF_UNCHECKED);
  SET_WHY_STOP( HEXSl(pc,6)+": New program executed" )
}

void debug_vbl()
{
  if (debug_send_alt_keys){
    debug_send_alt_keys_vbl_countdown--;
    if (debug_send_alt_keys_vbl_countdown==0){
      LOOP{
        bool SpecialKey=0;
        if (debug_send_alt_keys>=0x3b && debug_send_alt_keys<=0x44) SpecialKey=true;
        if (debug_send_alt_keys>=0x61 && debug_send_alt_keys<=0x72) SpecialKey=true;
        switch (debug_send_alt_keys){
          case 0xe:case 0xf:case 0x1d:case 0x2a:case 0x36:case 0x3a:
          case 0x39:case 0x38:case 0x1c:case 0x53:case 0x52:case 0x48:
          case 0x47:case 0x4b:case 0x50:case 0x4d:case 0x4a:case 0x4e:
            SpecialKey=true;
            break;
        }
        if (SpecialKey==0){
          Str HexI=HEXSl(debug_send_alt_keys,2).LowerCase();
          for (int n=0;n<2;n++){
            BYTE VKCode;
            if (HexI[n]>='0' && HexI[n]<='9') VKCode=BYTE(VK_NUMPAD0+(HexI[n]-'0'));
            if (HexI[n]>='a' && HexI[n]<='f') VKCode=BYTE('A'+(HexI[n]-'a'));
            keyboard_buffer_write(key_table[VKCode]);
            keyboard_buffer_write(BYTE(key_table[VKCode] | BIT_7));
          }
          keyboard_buffer_write(key_table[VK_SPACE]);
          keyboard_buffer_write(BYTE(key_table[VK_SPACE] | BIT_7));

          keyboard_buffer_write(key_table[VK_MENU]);
          keyboard_buffer_write(debug_send_alt_keys);
          keyboard_buffer_write(BYTE(debug_send_alt_keys | BIT_7));
          keyboard_buffer_write(BYTE(key_table[VK_MENU] | BIT_7));
          keyboard_buffer_write(key_table[VK_RETURN]);
          keyboard_buffer_write(BYTE(key_table[VK_RETURN] | BIT_7));
        }
        if ((++debug_send_alt_keys)>0x75){
          debug_send_alt_keys=0;
          break;
        }
        debug_send_alt_keys_vbl_countdown=5;
        if (SpecialKey==0) break;
      }
    }
  }
}

#undef LOGSECTION


