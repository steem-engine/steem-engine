//---------------------------------------------------------------------------
void trace()
{
  SendMessage(trace_window_handle,WM_SETTEXT,0,(long)"Trace");
  trace_init();
  d2_trace=true;
  disa_d2(pc);
  int time_text_entry=trace_entries,cpu_cycles_this_instruction;
  trace_add_text(EasyStr("Instruction time (not rounded): "));
  d2_trace=false;

  runstate=RUNSTATE_STOPPED;
  runstate_why_stop="";
  debug_in_trace=true;

  debug_trace_event_plan_init();

  int old_cpu_time=ABSOLUTE_CPU_TIME;

  //execute
//  try{
  TRY_M68K_EXCEPTION
    pc_history[pc_history_idx++]=pc;
    if (pc_history_idx>=HISTORY_SIZE) pc_history_idx=0;

    mode=STEM_MODE_CPU;

    draw_begin();
    debug_update_drawing_position();

    m68k_process();
    cpu_cycles_this_instruction=ABSOLUTE_CPU_TIME-old_cpu_time;

    debug_check_for_events();

    draw_end();

    mode=STEM_MODE_INSPECT;

    update_display_after_trace();

    debug_check_monitors();
//  }catch (m68k_exception &e){
  CATCH_M68K_EXCEPTION
    m68k_exception &e=ExceptionObject;
    mode=STEM_MODE_INSPECT;
    if (e.bombs>7){
      e.crash();
      cpu_cycles_this_instruction=ABSOLUTE_CPU_TIME-old_cpu_time;
      trace_exception_display(&e);
    }else{
      switch (Alert("Exception - do you want to crash(=ABORT)\nor re-execute?(=RETRY)\nor skip?(=IGNORE)",
                          (EasyStr("Exception ")+itoa(e.bombs,d2_t_buf,10)).c_str(),
                          MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION)){
      case IDABORT:
        e.crash();
        trace_exception_display(&e);
        break;
      case IDRETRY:
        SET_PC(old_pc);
        break;
      case IDIGNORE:
        SET_PC(dpc);
        break;
      }
      cpu_cycles_this_instruction=ABSOLUTE_CPU_TIME-old_cpu_time;
    }
  END_M68K_EXCEPTION

  debug_in_trace=0;

  strcpy(t_d_e[time_text_entry].name,EasyStr("Instruction time (not rounded): ")+(cpu_cycles_this_instruction)+" cycles");
  update_register_display(true);
  trace_display();
  
  if (runstate_why_stop.NotEmpty()){
    Alert(runstate_why_stop,"Interrupt",0);
    runstate_why_stop="";
  }
}
//---------------------------------------------------------------------------
void trace_again()
{
  //reset all values
  SET_PC(trace_pc);
  sr=trace_sr_before;
  for (int n=0;n<trace_entries;n++){
    if (t_d_e[n].regflag){
      switch (t_d_e[n].bytes){
        case 1:*(BYTE*)(t_d_e[n].ptr)=LOBYTE(t_d_e[n].val[0]);break;
        case 2:*(WORD*)(t_d_e[n].ptr)=LOWORD(t_d_e[n].val[0]);break;
        case 3:case 4:*(LONG*)(t_d_e[n].ptr)=t_d_e[n].val[0];break;
      }
    }else{
      switch (t_d_e[n].bytes){
        case 1:d2_poke(t_d_e[n].ad,LOBYTE(t_d_e[n].val[0]));break;
        case 2:d2_dpoke(t_d_e[n].ad,LOWORD(t_d_e[n].val[0]));break;
        case 3:case 4:d2_lpoke(t_d_e[n].ad,t_d_e[n].val[0]);break;
      }
    }
  }
  trace();
}
//---------------------------------------------------------------------------
void trace_display_clear()
{
  SendMessage(trace_scroller.GetControlPage(),WM_SETREDRAW,0,0);
  for (int n=0;n<MAX_TRACE_DISPLAY_ENTRIES;n++){
    if (trace_hLABEL[n]){
      if (IsWindow(trace_hLABEL[n])){
        DestroyWindow(trace_hLABEL[n]);
        trace_hLABEL[n]=NULL;
      }
    }
  }
  mr_static_delete_children_of(trace_scroller.GetControlPage());
  SendMessage(trace_scroller.GetControlPage(),WM_SETREDRAW,1,0);
  InvalidateRect(trace_scroller,NULL,0);
}
//---------------------------------------------------------------------------
void trace_init()
{
  trace_display_clear();
  trace_entries=0;
  trace_sr_before=sr;
  trace_pc=pc;
}
//---------------------------------------------------------------------------
void trace_add_movem_block(char*name,int aregn,short when,int bytes,MEM_ADDRESS ad,int count)
{
  char tb[30];
  if(d2_trace && count){
    for(int n=0;n<count;n++){
      if(aregn!=-1){strcpy(tb,STRS(ad+n*bytes-areg[aregn]));strcat(tb,"(");
        strcat(tb,reg_name(aregn+8));strcat(tb,")");
      }else tb[0]=0;
      trace_add_entry(name,tb,when,false,bytes,ad+n*bytes);
    }
  }
}
//---------------------------------------------------------------------------
void trace_exception_display(m68k_exception*exc){
  d2_trace=true;
  trace_add_text((char*)bombs_name[exc->bombs]);
  if(exc->action==EA_INST){
    trace_add_text((char*)(char*)exception_action_name[exc->action]);
  }else{
    trace_add_entry("during ",(char*)exception_action_name[exc->action],0,false,4,exc->address);
  }
  trace_add_entry("exception vector ","",TDE_BEFORE,false,4,(exc->bombs)*4);
  d2_trace=false;
}

void trace_add_text(char*tt)
{
  if(d2_trace){
    int n=trace_entries;
    strcpy(t_d_e[n].name,tt);
    t_d_e[n].when=TDE_TEXT_ONLY;
    if(trace_entries<MAX_TRACE_DISPLAY_ENTRIES)
      trace_entries++;
  }
}


void trace_add_entry(char*name1,char*name2,short when,bool regflag,
    int bytes,MEM_ADDRESS ad)
{

  if(d2_trace){
    int n=trace_entries;
    if(strlen(name1)+strlen(name2)>100)strcpy(t_d_e[n].name,"name too long");
    else {strcpy(t_d_e[n].name,name1);strcat(t_d_e[n].name,name2);}
    t_d_e[n].when=when;
    t_d_e[n].bytes=bytes;
    t_d_e[n].regflag=regflag;
    if(t_d_e[n].regflag){
      t_d_e[n].ptr=(unsigned long*)ad;
      if((t_d_e[n].when)&TDE_BEFORE){
        t_d_e[n].val[0]=*(t_d_e[n].ptr);
      }
    }else{
      t_d_e[n].ad=ad&0xffffff;
      if((t_d_e[n].when)&TDE_BEFORE){
        switch(t_d_e[n].bytes){
          case 1:t_d_e[n].val[0]=d2_peek(t_d_e[n].ad);break;
          case 2:t_d_e[n].val[0]=d2_dpeek(t_d_e[n].ad);break;
          case 4:t_d_e[n].val[0]=d2_lpeek(t_d_e[n].ad);break;
        }
      }
    }
    if(trace_entries<MAX_TRACE_DISPLAY_ENTRIES)
      trace_entries++;
  }
}
//---------------------------------------------------------------------------
void trace_get_after()
{
  for (int n=0;n<trace_entries;n++){
    if ((t_d_e[n].when)&TDE_AFTER){
      if (t_d_e[n].regflag){
        t_d_e[n].val[1]=*(t_d_e[n].ptr);
      }else{
        switch(t_d_e[n].bytes){
          case 1:t_d_e[n].val[1]=d2_peek(t_d_e[n].ad);break;
          case 2:t_d_e[n].val[1]=d2_dpeek(t_d_e[n].ad);break;
          case 4:t_d_e[n].val[1]=d2_lpeek(t_d_e[n].ad);break;
        }
      }
    }
  }
}
//---------------------------------------------------------------------------
void trace_display()
{
//  HWND hLABEL;
  m_b_trace.ad=trace_pc;
  m_b_trace.update();

  trace_display_clear();

  HWND Parent=trace_scroller.GetControlPage();
  int y=2,x,max_x=430;
  for (int n=0;n<trace_entries;n++){
    x=2;
    int name_width=get_text_width(t_d_e[n].name);
    trace_hLABEL[n]=CreateWindowEx(0,"Static",t_d_e[n].name,WS_VISIBLE | WS_CHILD,
        x,y+3,name_width,22,Parent,(HMENU)101,Inst,NULL);
    SendMessage(trace_hLABEL[n],WM_SETFONT,(UINT)fnt,0);
    if(t_d_e[n].when!=TDE_TEXT_ONLY){
      x+=name_width+20;
      if (t_d_e[n].regflag){
        if (t_d_e[n].when & TDE_BEFORE){
          new mr_static("before",t_d_e[n].name,x,y,Parent,
            (HMENU)1,(MEM_ADDRESS)&(t_d_e[n].val[0]),t_d_e[n].bytes,MST_REGISTER,true,NULL);
          x+=100;
        }
        if (t_d_e[n].when&TDE_AFTER){
          new mr_static("after",t_d_e[n].name,x,y,Parent,
            (HMENU)1,(MEM_ADDRESS)(t_d_e[n].ptr),t_d_e[n].bytes,MST_REGISTER,true,NULL);
          x+=100;
        }
      }else{
        //memory address
        EasyStr ad_desc="";

        iolist_entry*il[4]={NULL,NULL,NULL,NULL};
        int iwid[4]={0,0,0,0};
        int ic=0,ox;
        bool iols=false;
        for(int b=0;b<t_d_e[n].bytes;b++){
          iolist_entry*i=search_iolist(t_d_e[n].ad+b);
          if(i){
            if(ad_desc[0])ad_desc+=", ";
            ad_desc+=i->name;
            if(i->bitmask[0]){
              iwid[ic]=iolist_box_width(i);
              il[ic++]=i;
              iols=true;
              if(i->bytes==2)b++;
            }
          }
        }
        if(ad_desc[0]){  //special addresses
          ad_desc+=" - ";
        }
        ad_desc+="address";


        new mr_static(ad_desc,"address",x,y,Parent,
          (HMENU)0,(MEM_ADDRESS)&(t_d_e[n].ad),3,MST_ADDRESS,false,NULL);

        x+=get_text_width(ad_desc)+60;
        if (t_d_e[n].when & TDE_BEFORE){
          ox=x;
          new mr_static("before",t_d_e[n].name,x,y,Parent,
            (HMENU)1,t_d_e[n].ad,t_d_e[n].bytes,MST_HISTORIC_MEMORY,true,(mem_browser*)&(t_d_e[n].val[0]));
          if(iols){
            x+=100;
            for(int m=0;m<ic;m++){
              new mr_static("",il[m]->name,x,y,Parent,
                (HMENU)1,(MEM_ADDRESS)(il[m]),il[m]->bytes,MST_HISTORIC_IOLIST,true,(mem_browser*)&(t_d_e[n].val[0]));
              x+=iwid[m]+10;
            }
            max_x=max(x,max_x);
            x=ox;
            y+=24;
          }else{
            x+=100;
          }
        }
        if (t_d_e[n].when & TDE_AFTER){
          new mr_static("after",t_d_e[n].name,x,y,Parent,
            (HMENU)1,t_d_e[n].ad,t_d_e[n].bytes,MST_MEMORY,true,NULL);
          x+=100;
          if (iols){
            for (int m=0;m<ic;m++){
              new mr_static("",il[m]->name,x,y,Parent,
                (HMENU)1,(MEM_ADDRESS)(il[m]),il[m]->bytes,MST_IOLIST,true,NULL);
              x+=iwid[m]+10;
            }
          }
        }
      }
    }
    max_x=max(x,max_x);

    y+=24;
  }
  y+=5;
  trace_scroller.SetHeight(1);trace_scroller.SetWidth(1);

  if (trace_show_window){
    SetWindowPos(trace_window_handle,HWND_TOP,0,0,
                  max_x+30,y+153+GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYHSCROLL),
                  SWP_NOMOVE | SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOACTIVATE);
  }else{
    ShowWindow(trace_window_handle,SW_HIDE);
  }

  trace_scroller.AutoSize();
}
//---------------------------------------------------------------------------
LRESULT __stdcall trace_window_WndProc(HWND Win,UINT Mess,UINT wPar,long lPar)
{
	switch (Mess){
    case WM_SIZE:
      MoveWindow(m_b_trace.handle,10,5,LOWORD(lPar)-20,50,true);
      MoveWindow(trace_repeat_trace_button,10,HIWORD(lPar)-35,LOWORD(lPar) - 20,30,true);
      SetWindowPos(trace_scroller,0,0,0,LOWORD(lPar)-20,HIWORD(lPar)-145,SWP_NOZORDER | SWP_NOMOVE);
      break;
    case WM_CLOSE:
      ShowWindow(Win,SW_HIDE);
  //    m_b_trace.active=false;
      return 0;
    case WM_DESTROY:
  //    m_b_trace.active=false;

  //    m_b_stack.active=false;
      trace_display_clear(); // Free mr_static memory
      trace_entries=0;
      break;
    case WM_CONTEXTMENU:
    case WM_COMMAND:
      if(HIWORD(wPar)==BN_CLICKED){
        switch(LOWORD(wPar)){
          case 1003:  //trace;
            trace_again();
            break;
        }
      }

      break;
    case WM_ACTIVATE:
      break;
  }

	return DefWindowProc(Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
void trace_window_init()
{
  trace_window_handle=CreateWindowEx(/*WS_EX_TOOLWINDOW*/0,"Steem Trace Window","Trace",
      WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX,
      110,310,370,400,NULL,NULL,Inst,0);

  m_b_trace.owner=trace_window_handle;
  m_b_trace.handle=CreateWindowEx(512,WC_LISTVIEW,"Freak!",
      LVS_REPORT | LVS_SHAREIMAGELISTS | LVS_NOSORTHEADER | WS_VISIBLE | WS_CHILDWINDOW | WS_CLIPSIBLINGS,
      10,1,400,55,m_b_trace.owner,(HMENU)1,Inst,NULL);

  SetWindowLong(m_b_trace.handle,GWL_WNDPROC,(long)mem_browser_WndProc);
  SetWindowLong(m_b_trace.handle,GWL_USERDATA,(LONG)&m_b_trace);

//  m_b_trace.active=false;
  m_b_trace.disp_type=DT_INSTRUCTION;
  m_b_trace.mode=MB_MODE_FIXED;
  m_b_trace.editflag=true;
  m_b_trace.editbox=NULL;
  m_b_trace.lb_height=1;

  m_b_trace.init();

  CreateWindowEx(0,"Static","sr before",WS_VISIBLE | WS_CHILDWINDOW | SS_LEFTNOWORDWRAP,
      10,63,50,17,trace_window_handle,(HMENU)0,Inst,NULL);

  trace_sr_before_display=CreateWindowEx(512,"Static","trace sr display",WS_BORDER | WS_VISIBLE | WS_CHILDWINDOW | SS_NOTIFY,
      60,60,200,20,trace_window_handle,(HMENU)0,Inst,NULL);
  SetWindowLong(trace_sr_before_display,GWL_USERDATA,(LONG)&trace_sr_before);
  SetWindowLong(trace_sr_before_display,GWL_WNDPROC,(long)sr_display_WndProc);

  CreateWindowEx(0,"Static","sr after",WS_VISIBLE | WS_CHILDWINDOW | SS_LEFTNOWORDWRAP,
      10,83,50,17,trace_window_handle,(HMENU)0,Inst,NULL);

  trace_sr_after_display=CreateWindowEx(512,"Static","trace sr display",WS_BORDER | WS_VISIBLE | WS_CHILDWINDOW | SS_NOTIFY,
      60,80,200,20,trace_window_handle,(HMENU)0,Inst,NULL);
  SetWindowLong(trace_sr_after_display,GWL_USERDATA,(LONG)&sr);
  SetWindowLong(trace_sr_after_display,GWL_WNDPROC,(long)sr_display_WndProc);

  trace_scroller.CreateEx(512,WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL,10,105,260,130,trace_window_handle,100,Inst);

  trace_repeat_trace_button=CreateWindowEx(512,"Button","Repeat Trace",WS_BORDER | WS_VISIBLE | WS_CHILDWINDOW | BS_PUSHBUTTON,
      150,76,130,35,trace_window_handle,(HMENU)1003,Inst,NULL);

  SetWindowAndChildrensFont(trace_window_handle,fnt);
}
