//---------------------------------------------------------------------------
void SetStemWinSize(int w,int h,int xo,int yo)
{
  if (XD==NULL) return;

  if (bAppMaximized==0 && bAppMinimized==0){
    XResizeWindow(XD,StemWin,2+w+2,MENUHEIGHT+2+h+2);
    XClearArea(XD,StemWin,0,0,2+w+2,MENUHEIGHT+2+h+2,True);
  }else{
    ///// Adjust restore size
  }

  XSizeHints *pHints=XAllocSizeHints();
  if (pHints){
    pHints->flags=PMinSize;
		pHints->min_width=320+4;
		pHints->min_height=200+4+MENUHEIGHT;
    XSetWMSizeHints(XD,StemWin,pHints,XA_WM_NORMAL_HINTS);
    XFree(pHints);
  }
}
//---------------------------------------------------------------------------
void MoveStemWin(int x,int y,int w,int h)
{
  if (XD==NULL) return;

  if (x==MSW_NOCHANGE || y==MSW_NOCHANGE){
    x=MSW_NOCHANGE;
    y=MSW_NOCHANGE;
  }

  XWindowAttributes wa;
  XGetWindowAttributes(XD,StemWin,&wa);
  int new_w=int((w==MSW_NOCHANGE) ? wa.width:w);
  int new_h=int((h==MSW_NOCHANGE) ? wa.height:h);
  if (w==wa.width && h==wa.height){ // Don't resize
    if (x!=MSW_NOCHANGE) XMoveWindow(XD,StemWin,x,y);
  }else{
    if (x==MSW_NOCHANGE){
      XResizeWindow(XD,StemWin,new_w,new_h);
    }else{
      XMoveResizeWindow(XD,StemWin,x,y,new_w,new_h);
    }
  }
}
//---------------------------------------------------------------------------
int StemWinProc(void*,Window Win,XEvent *Ev)
{
//	printf("%i\n",Ev->type);
#ifndef NO_SHM
  if (Ev->type==Disp.SHMCompletion){
    Disp.asynchronous_blit_in_progress=false;
  }else
#endif

  switch (Ev->type){
    case Expose:
    {
      XWindowAttributes wa;
      XGetWindowAttributes(XD,StemWin,&wa);

      hxc::clip_to_expose_rect(XD,&(Ev->xexpose),DispGC);

      if (Ev->xexpose.y+Ev->xexpose.height>MENUHEIGHT){
        draw_end();
        if (draw_blit()==0){
          XSetForeground(XD,DispGC,BlackCol);
          XFillRectangle(XD,StemWin,DispGC,2,MENUHEIGHT+2,
                          wa.width-4,wa.height-(MENUHEIGHT+4));
        }
        Disp.Surround();
      }

      XSetForeground(XD,DispGC,BkCol);
      XFillRectangle(XD,StemWin,DispGC,0,0,wa.width,MENUHEIGHT);
      XSetClipMask(XD,DispGC,None);

      XSync(XD,False);
      break;
    }
    case ButtonPress:
      if (Ev->xbutton.button==Button4 || Ev->xbutton.button==Button5) break;
      if (runstate==RUNSTATE_RUNNING && stem_mousemode==STEM_MOUSEMODE_DISABLED){
        if (Ev->xbutton.y>MENUHEIGHT){
          SetForegroundWindow(Win,Ev->xbutton.time);
          SetStemMouseMode(STEM_MOUSEMODE_WINDOW);
        }
      }else if (runstate==RUNSTATE_STOPPED && StartEmuOnClick){
        PostRunMessage();
      }
    case ButtonRelease:
      break;
    case KeyPress:
    case KeyRelease:
    {
      bool Up=(Ev->type==KeyRelease);
      if (Up==0 && GetKeyState(Ev->xkey.keycode)<0){ //Key repeat
      	return PEEKED_MESSAGE;
      }
      SetKeyState(Ev->xkey.keycode,!Up);

      KeySym ks=XKeycodeToKeysym(XD,Ev->xkey.keycode,0);
      if (ks!=XK_Shift_L && ks!=XK_Shift_R &&
          ks!=XK_Control_L && ks!=XK_Control_R &&
          ks!=XK_Alt_L && ks!=XK_Alt_R){
        if (Ev->xkey.keycode==Key_Pause){
          if (Up==0) return PEEKED_MESSAGE;
          if (runstate==RUNSTATE_RUNNING){
            if (GetKeyStateSym(XK_Shift_R)<0 || GetKeyStateSym(XK_Shift_L)<0 || FullScreen){
              runstate=RUNSTATE_STOPPING;
              SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
              break;
            }else{
              if (stem_mousemode==STEM_MOUSEMODE_DISABLED){
                SetStemMouseMode(STEM_MOUSEMODE_WINDOW);
              }else if (stem_mousemode==STEM_MOUSEMODE_WINDOW){
                SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
              }
            }
          }else if (runstate==RUNSTATE_STOPPED){
            return PEEKED_RUN;
          }
        }else if (joy_is_key_used(BYTE(Ev->xkey.keycode))==0 &&
                    CutDisableKey[BYTE(Ev->xkey.keycode)]==0){
          HandleKeyPress(Ev->xkey.keycode,Up,0);
        }
      }
      break;
    }
    case ClientMessage:
      if (Ev->xclient.message_type==hxc::XA_WM_PROTOCOLS){
        if (Atom(Ev->xclient.data.l[0])==hxc::XA_WM_DELETE_WINDOW){
          QuitSteem();
        }
      }else if (Ev->xclient.message_type==RunSteemAtom){
      	if (RunMessagePosted){
  				RunMessagePosted=0;
          if (runstate==RUNSTATE_STOPPED){
            return PEEKED_RUN;
          }else if (runstate==RUNSTATE_RUNNING){
            runstate=RUNSTATE_STOPPING;
            SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
          }
        }
      }else if (Ev->xclient.message_type==LoadSnapShotAtom){
    		if (runstate==RUNSTATE_STOPPED){
          bool AddToHistory=true;
          Str fn=LastSnapShot;
          if (Ev->xclient.data.l[0]==207) fn=WriteDir+SLASH+"auto_reset_backup.sts", AddToHistory=0;
          if (Ev->xclient.data.l[0]==208) fn=WriteDir+SLASH+"auto_loadsnapshot_backup.sts", AddToHistory=0;
    	    LoadSnapShot(fn,AddToHistory);
          if (Ev->xclient.data.l[0]==207 || Ev->xclient.data.l[0]==208) DeleteFile(fn);
    	  }else{
    	  	runstate=RUNSTATE_STOPPING;

          XEvent SendEv;
          SendEv.type=ClientMessage;
          SendEv.xclient.window=StemWin;
          SendEv.xclient.message_type=LoadSnapShotAtom;
          SendEv.xclient.format=32;
          SendEv.xclient.data.l[0]=Ev->xclient.data.l[0];
          XSendEvent(XD,StemWin,0,0,&SendEv);
    	  }
      }
      break;
    case ConfigureNotify:
    {
      XWindowAttributes wa;
      XGetWindowAttributes(XD,StemWin,&wa);

      if (DiskBut.handle){
        XMoveWindow(XD,InfBut,wa.width-135,0);
        XMoveWindow(XD,PatBut,wa.width-112,0);
        XMoveWindow(XD,CutBut,wa.width-89,0);
        XMoveWindow(XD,OptBut,wa.width-66,0);
        XMoveWindow(XD,JoyBut,wa.width-43,0);
        XMoveWindow(XD,DiskBut,wa.width-20,0);
      }
      bool OldCanUse=CanUse_400;
      if (draw_grille_black<10) draw_grille_black=10;
      if (border & 1){
        CanUse_400=(wa.width>=(2+BORDER_SIDE*2+640+BORDER_SIDE*2+2) &&
                      wa.height>=(MENUHEIGHT + 2+BORDER_TOP*2+400+BORDER_BOTTOM*2+2));
      }else{
        CanUse_400=(wa.width>=(2+640+2) && wa.height>=(MENUHEIGHT+2+400+2));
      }
      if (OldCanUse!=CanUse_400 && FullScreen==0){
        draw_end();
        draw(0);
      }
      x_draw_surround_count=max(x_draw_surround_count,10);
      break;
    }
    case SelectionNotify:
      if (Ev->xselection.property!=None){
        if (Ev->xselection.target==XA_STRING){
          Atom actual_type_return;
          int actual_format_return;
          unsigned long nitems_return;
          unsigned long bytes_after_return;
          char *t;
          XGetWindowProperty(XD,Win,Ev->xselection.property,
                        /*long_offset*/ 0, /*long_length*/ 5000,
                        /*delete*/ True, XA_STRING,
                        &actual_type_return, &actual_format_return,
                        &nitems_return, &bytes_after_return,
                        (BYTE**)(&t));
          if (actual_type_return==XA_STRING){
            PasteText=t;
            PasteVBLCount=PasteSpeed;
            PasteBut.set_check(true);
            XFree(t);
          }
        }
      }
      break;
    case FocusIn:
      bAppActive=true;
      XAutoRepeatOff(XD);
      break;
    case FocusOut:
    {
      Window Foc=0;
      int RevertTo;
      XGetInputFocus(XD,&Foc,&RevertTo);
    	if (Foc!=StemWin){
				XAutoRepeatOn(XD);
        SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
        ZeroMemory(KeyState,sizeof(KeyState));
        UpdateSTKeys();
      	bAppActive=0;
      }
      break;
    }
    case MapNotify:
	  	bAppActive=true;
    	bAppMinimized=0;
    	break;
    case UnmapNotify:
    	bAppMinimized=true;
    	break;
    case DestroyNotify:
      StemWin=0;
      QuitSteem();
      return PEEKED_QUIT;
  }
  return PEEKED_MESSAGE;
}
//---------------------------------------------------------------------------
int snapshot_parse_filename(char*fn,struct stat*s)
{
  if (S_ISDIR(s->st_mode)) return FS_FTYPE_FOLDER;
  if (has_extension(fn,".STS")){
    return FS_FTYPE_FILE_ICON+ICO16_SNAPSHOTS;
  }
  return FS_FTYPE_REJECT;
}
//---------------------------------------------------------------------------
void SnapShotProcess(int i)
{
  bool WaitUntilStopped=0;
  if (i==200 /* Load Snapshot */ || i==201 /* Save Snapshot */){
    fileselect.set_corner_icon(&Ico16,ICO16_SNAPSHOT);
    Str LastSnapShotFol=LastSnapShot;
    RemoveFileNameFromPath(LastSnapShotFol,REMOVE_SLASH);
    EasyStr fn=fileselect.choose(XD,LastSnapShotFol,GetFileNameFromPath(LastSnapShot),
                T("Memory Snapshots"),((i==200) ? FSM_LOAD:FSM_SAVE) | FSM_LOADMUSTEXIST |
                FSM_CONFIRMOVERWRITE,snapshot_parse_filename,".sts");
    if (fileselect.chose_option==FSM_LOAD){
      LastSnapShot=fn;
      WaitUntilStopped=true;
    }else if (fileselect.chose_option==FSM_SAVE){
      LastSnapShot=fn;
      SaveSnapShot(fn,-1);
    }
  }else if (i==205){ // Save over last
    if (SnapShotGetLastBackupPath().NotEmpty()){
      // Make backup, could be on different drive so do it the slow way
      copy_file_byte_by_byte(LastSnapShot,SnapShotGetLastBackupPath());
    }
    SaveSnapShot(LastSnapShot,-1);
  }else if (i==206){ // Undo save over
    // Restore backup, can only get here if backup path is valid
    copy_file_byte_by_byte(SnapShotGetLastBackupPath(),LastSnapShot);
    remove(SnapShotGetLastBackupPath());
  }else if (i>=210 && i<220){ // Load recent
    LastSnapShot=StateHist[i-210];
    WaitUntilStopped=true;
  }else if (i==207 || i==208){ // undo reset/last snap
    WaitUntilStopped=true;
  }
  if (WaitUntilStopped){
    if (runstate==RUNSTATE_RUNNING) runstate=RUNSTATE_STOPPING;

    XEvent SendEv;
    SendEv.type=ClientMessage;
    SendEv.xclient.window=StemWin;
    SendEv.xclient.message_type=LoadSnapShotAtom;
    SendEv.xclient.format=32;
    SendEv.xclient.data.l[0]=i;
    XSendEvent(XD,StemWin,0,0,&SendEv);
  }
}
//---------------------------------------------------------------------------
int stemwin_popup_notify(hxc_popup *pop,int mess,int idx)
{
	if (mess==POP_CHOOSE){
    int i=pop->menu[idx].Data[1];
    if (i>=100 && i<200){
      PasteSpeed=(i-100)+1;
    }else if (i>=200 && i<300){
      SnapShotProcess(i);
    }
  }
	SnapShotBut.set_check(0);
	PasteBut.set_check(PasteText.NotEmpty());
	return 0;
}

int StemWinButtonNotifyProc(hxc_button *But,int Mess,int *Inf)
{
  switch (But->id){
    case 101:
      if (Inf[0]==Button3){
      	slow_motion_change(Mess==BN_DOWN);
      }else if (Mess==BN_CLICKED){
	      PostRunMessage();
	    }
      break;
    case 109:
      if (Mess!=BN_DOWN && Mess!=BN_UP) break;
      
      if (Mess==BN_DOWN){
        if (fast_forward_stuck_down){
          fast_forward_stuck_down=0;
          Mess=BN_UP;
        }else{
          if (DWORD(Inf[1])<ff_doubleclick_time){
            fast_forward_stuck_down=true;
            ff_doubleclick_time=0;
          }else{
            ff_doubleclick_time=DWORD(Inf[1])+FF_DOUBLECLICK_MS;
          }
        }
      }else{
        if (fast_forward_stuck_down) break;
      }
      fast_forward_change(Mess==BN_DOWN,Inf[0]==Button3);
      break;
    case 102:
    {
      bool Warm=(Inf[0]==Button3);
      reset_st(DWORD(Warm ? RESET_WARM:RESET_COLD) | DWORD(Warm ? RESET_NOSTOP:RESET_STOP) |
                  RESET_CHANGESETTINGS | RESET_BACKUP);
      break;
    }
    case 108: // Memory Snapshots
    	if (Mess==BN_CLICKED){
        But->set_check(true);

        EasyStringList sl;
        SnapShotGetOptions(&sl);

    		pop.lpig=NULL;
        pop.menu.DeleteAll();
        for (int i=0;i<sl.NumStrings;i++){
          Str Text=sl[i].String;
          while (Text.InStr("&")>=0) Text.Delete(Text.InStr("&"),1);
          pop.menu.Add(Text,-1,sl[i].Data[0]);
        }
        pop.create(XD,But->handle,0,But->h,stemwin_popup_notify,NULL);
      }
      break;
    case 116:
      if (runstate==RUNSTATE_RUNNING){
        DoSaveScreenShot|=1;
      }else{
        Disp.SaveScreenShot();
      }
      break;
    case 114: // Paste
      if (Inf[0]==Button3){
      	if (Mess==BN_CLICKED){
      		But->set_check(true);
      		pop.lpig=&Ico16;
	      	pop.menu.DeleteAll();
	        for (int n=0;n<11;n++){
	        	long ico=ICO16_UNRADIOMARKED;
	        	if (PasteSpeed==(1+n)) ico=ICO16_RADIOMARK;
	        	pop.menu.Add(T("Delay")+" - "+n,ico,100+n);
	        }
	        pop.create(XD,But->handle,0,But->h,stemwin_popup_notify,NULL);
	      }
      }else{
		    PasteIntoSTAction(STPASTE_TOGGLE);
		  }
      break;
    case 115:
      Disp.GoToFullscreenOnRun=But->checked;
      if (runstate==RUNSTATE_RUNNING){
        runstate=RUNSTATE_STOPPING;
        RunWhenStop=true;
      }
      break;

    case 100: //DiskMan
      DiskMan.ToggleVisible();
      break;
    case 103: //Joy
      JoyConfig.ToggleVisible();
      break;
    case 105:
      InfoBox.ToggleVisible();
      break;
    case 107:
      OptionBox.ToggleVisible();
      break;
    case 112:
      ShortcutBox.ToggleVisible();
      break;
    case 113:
      PatchesBox.ToggleVisible();
      break;
    case 120: // AutoUpdate
      break;
  }
  return 0;
}
//---------------------------------------------------------------------------
int timerproc(void*,Window,int id)
{
  if (id==SHORTCUTS_TIMER_ID){
    JoyGetPoses();
    ShortcutsCheck();
  }
  return HXC_TIMER_REPEAT;
}
//---------------------------------------------------------------------------

