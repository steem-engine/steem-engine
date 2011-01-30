IconGroup *hxc_button::pcheck_ig=NULL;
int hxc_button::check_on_icon=-1,hxc_button::check_off_icon=-1;
//---------------------------------------------------------------
void hxc_button::init()
{
  can_have_children=true; //if static
  destroyproc=(LPHXC_DESTROYPROC)destroy;
  deleteproc=(LPHXC_DELETEPROC)delete_hxc_button;

  MouseIn=0;
  MouseDown=0;
  checked=0;
  type=0;
  picons=NULL;
  icon_index=0;
  want_drag_notify=0;
  drop_shadow_bitmap=0;
}
//---------------------------------------------------------------
bool hxc_button::create(Display *d,Window daddy,int x,int y,int w,int h,
            LPHXC_BUTTONNOTIFYPROC pass_notifyproc,void *o,int type_pass,char *Text,int pass_ID,
            DWORD BkCol,XFontStruct *fi)
{
  if (XD) destroy(this);

  XD=d;
  parent=daddy;
  notifyproc=pass_notifyproc;
  id=pass_ID;
  type=type_pass;
  text=Text;
  owner=o;

  load_res(XD);

  col_text=col_black;
  if (type & BT_LINK) col_text=col_sel_back;

  if (fi){
    but_font=fi;
  }else{
    but_font=font;
  }

  if (w==0){ //auto-width
    if (type & BT_TEXT){
      EasyStr DispText=text;
      if (type & BT_LINK){
        char *pipe=strchr(DispText,'|');
        if (pipe) *pipe=0;
      }
  		w=XTextWidth(but_font,DispText,strlen(DispText));
  		if ((type & (BT_STATIC | BT_LINK))==0 || (type & BT_BORDER_MASK)){
  			w+=10;
  		}
      if (type & BT_CHECK){
        if (pcheck_ig) w+=pcheck_ig->IconWidth+4;
      }
  	}else if(type&BT_ICON){
  		w=16;
  		if ((type & (BT_STATIC | BT_LINK))==0 || (type & BT_BORDER_MASK)){
  			w+=2;
  		}
  	}
  }
  if(w==0)w=32;

  if(h==0){ //auto-h
  	if(type&BT_TEXT){
  		h=(but_font->ascent)+(but_font->descent);
  		if ((type & (BT_STATIC | BT_LINK))==0 || (type & BT_BORDER_MASK)){
  			h+=4;
  		}
  	}else if(type&BT_ICON){
  		h=16;
  		if ((type & (BT_STATIC | BT_LINK))==0 || (type & BT_BORDER_MASK)){
  			h+=2;
  		}
  	}else{
  		h=25;
  	}
  }
  if(h==0)w=25;

  col_background=BkCol;

  XSetWindowAttributes swa;
  swa.backing_store=NotUseful;
  swa.background_pixel=BkCol;
  unsigned long flags=CWBackingStore;
  if ((type & BT_NOBACKGROUND)==0) flags|=CWBackPixel;
  handle=XCreateWindow(XD,parent,x,y,w,h,0,
                           CopyFromParent,InputOutput,CopyFromParent,
                           flags,&swa);
	hxc::x=x;hxc::y=y;hxc::w=w;hxc::h=h;

  SetProp(XD,handle,cWinProc,DWORD(WinProc));
  SetProp(XD,handle,cWinThis,DWORD(this));
  button_timer_atom=XInternAtom(XD,"HXC_Button_Timer",0);
  next_move_time=clock();

  if (picons) drop_shadow_bitmap=picons->CreateMaskBitmap(icon_index);

  XSelectInput(XD,handle,ButtonPressMask | ButtonReleaseMask | Button1MotionMask |
                  ExposureMask | StructureNotifyMask | FocusChangeMask |
                  EnterWindowMask | LeaveWindowMask);

  XMapWindow(XD,handle);

  return true;
}
//---------------------------------------------------------------
int hxc_button::get_border_state()
{
	int Ret=0;
  if ((type & (BT_STATIC | BT_LINK))==0){
    if (checked || MouseDown){
      Ret|=BT_BORDER_INDENT;
    }else if (MouseIn){
      Ret|=BT_BORDER_OUTDENT;
    }else{
      Ret|=BT_BORDER_NONE;
    }
  }
  return Ret;
}
//---------------------------------------------------------------
void hxc_button::set_check(bool Check)
{
  if (Check!=checked){
    checked=Check;
    if (XD) WinProc(this,handle,NULL);
  }
}
//---------------------------------------------------------------
void hxc_button::set_text(char* t)
{
  if (text!=t){
    text=t;
    if (XD) WinProc(this,handle,NULL);
  }
}
//---------------------------------------------------------------
void hxc_button::set_icon(IconGroup *pNewIcons,int nIco)
{
  picons=pNewIcons;
  icon_index=nIco;
  if (XD){
    if (picons){
      if (drop_shadow_bitmap) XFreePixmap(XD,drop_shadow_bitmap);
      drop_shadow_bitmap=picons->CreateMaskBitmap(icon_index);
    }
    WinProc(this,handle,NULL);
  }
}
//---------------------------------------------------------------
void hxc_button::destroy(hxc_button*This)
{
  if (This->XD==NULL) return;

  RemoveProp(This->XD,This->handle,cWinProc);
  RemoveProp(This->XD,This->handle,cWinThis);
  hxc::kill_timer(This->handle,HXC_TIMER_ALL_IDS);
  XDestroyWindow(This->XD,This->handle);

  if (This->drop_shadow_bitmap) XFreePixmap(This->XD,This->drop_shadow_bitmap);
  This->drop_shadow_bitmap=0;

  This->MouseIn=0;
  This->MouseDown=0;

  free_res(This->XD);

  This->handle=0;
  This->XD=NULL;
}
//---------------------------------------------------------------
int hxc_button::WinProc(hxc_button *This,Window Win,XEvent *Ev)
{
  if (This==NULL) return 0;
	This->common_winproc(Ev);
  bool Draw=0;
  if (Ev){
   	int Down=This->get_border_state();
    switch (Ev->type){
      case ButtonPress:
        if (Ev->xbutton.button==Button4 || Ev->xbutton.button==Button5) break; // No wheel
        This->MouseDown=Ev->xbutton.button;
        if (This->type & BT_UPDOWNNOTIFY){
          int Inf[2]={Ev->xbutton.button,int(Ev->xbutton.time)};
          if (This->notifyproc) This->notifyproc(This,BN_DOWN,Inf);
        }
        if (This->type & BT_HOLDREPEAT){
          int Inf[1]={Ev->xbutton.button};
          if (This->notifyproc) This->notifyproc(This,BN_CLICKED,Inf);
  				send_event(This->XD,Win,This->button_timer_atom);
					This->next_move_time=clock()+(CLOCKS_PER_SEC/8);
        }
        if (This->get_border_state()!=Down) Draw=true;
        break;
      case ButtonRelease:
      {
        if (Ev->xbutton.button==Button4 || Ev->xbutton.button==Button5) break; // No wheel
        if (This->MouseDown==(Ev->xbutton.button)){
	        if ((This->type & BT_HOLDREPEAT)==0){
            Window InWin,InChild;
            int RootX,RootY,WinX,WinY;
            UINT Mask;
            if (XQueryPointer(This->XD,This->parent,&InWin,&InChild,
                              &RootX,&RootY,&WinX,&WinY,&Mask)){
              if (InChild==Win){
                if (This->type & BT_TOGGLE){
                  This->checked=!This->checked;
                  if (This->type & BT_CHECK) Draw=true;
                }

                int Inf[1]={Ev->xbutton.button};
                if (This->notifyproc) This->notifyproc(This,BN_CLICKED,Inf);
              }
            }
          }
        }
        This->MouseDown=0;
        if (This->type & BT_UPDOWNNOTIFY){
          int Inf[2]={Ev->xbutton.button,int(Ev->xbutton.time)};
          if (This->notifyproc) This->notifyproc(This,BN_UP,Inf);
        }
        if (This->get_border_state()!=Down) Draw=true;
        break;
      }
      case EnterNotify:
        This->MouseIn=true;
        if (This->get_border_state()!=Down) Draw=true;
        break;
      case MotionNotify:
        if (This->want_drag_notify && This->MouseDown){
          if (This->notifyproc) This->notifyproc(This,BN_MOTION,(int*)Ev);
        }
        break;
      case LeaveNotify:
        if (Ev->xcrossing.mode==NotifyGrab){
          if (This->MouseDown){
            if (This->type & BT_UPDOWNNOTIFY){
              int Inf[1]={This->MouseDown};
              if (This->notifyproc) This->notifyproc(This,BN_UP,Inf);
            }
          }
          This->MouseDown=0;
        }
        This->MouseIn=0;
        if (This->get_border_state()!=Down) Draw=true;
        break;
      case Expose:
      {
        if (Ev->xexpose.count>0) break;
        This->draw();
        return 0;
      }
      case ClientMessage:
      	if (Ev->xclient.message_type==This->button_timer_atom){
          if ((This->type & BT_HOLDREPEAT) && This->MouseDown){
          	if (clock()>This->next_move_time){
              int Inf[1]={Ev->xbutton.button};
              if (This->notifyproc) This->notifyproc(This,BN_CLICKED,Inf);
							This->next_move_time=clock()+(CLOCKS_PER_SEC/100);
						}
	  				send_event(This->XD,Win,This->button_timer_atom);
          }
        }
      	break;
    }
  }else{
    Draw=true;
  }
  if (Draw && This->XD){
    XWindowAttributes wa;
    XGetWindowAttributes(This->XD,Win,&wa);

    EasyStr Text=This->text;
   	if (This->type & BT_LINK){
	    char *pipe=strchr(Text,'|');
	    if (pipe) *pipe=0;
    }

    if ((This->type & BT_NOBACKGROUND)==0) XClearWindow(This->XD,Win);

	  if ((This->type & (BT_STATIC | BT_LINK))==0){
	    This->type&=~BT_BORDER_MASK;
			This->type|=This->get_border_state();
		}

    int In=0;
    DWORD TLCol=0,BRCol=0;
    if ((This->type & BT_BORDER_MASK)==BT_BORDER_INDENT){
      TLCol=This->col_border_dark;
      BRCol=This->col_white;
      In=2;
    }else if ((This->type & BT_BORDER_MASK)==BT_BORDER_OUTDENT){
      TLCol=This->col_white;
      BRCol=This->col_border_dark;
      In=1;
    }

    int w=0,h=0;
    if (This->type & BT_TEXT){
      if (This->but_font && Text[0]){
        w=XTextWidth(This->but_font,Text,strlen(Text));
        h=This->but_font->ascent+This->but_font->descent;
      }
    }else if ((This->type & BT_ICON) && This->picons){
      w=This->picons->IconWidth;
      h=This->picons->IconHeight;
    }

    int x=2,y=2;
    if (((This->type & BT_BORDER_MASK)==BT_BORDER_NONE) &&
    			(This->type & (BT_STATIC | BT_LINK))){
    	x=0,y=0;
    }
    if ((This->type & BT_OBJECTPOS_HMASK)==BT_TEXT_CENTRE){
      x=(wa.width-w)/2;
    }else if ((This->type & BT_OBJECTPOS_HMASK)==BT_TEXT_RIGHT){
      x=wa.width-w-2;
    }else if ((This->type & BT_OBJECTPOS_HMASK)==BT_TEXT_PATH){
      x=min(wa.width-w-2,x);
    }
    if ((This->type & BT_OBJECTPOS_VMASK)==BT_TEXT_VCENTRE){
      y=(wa.height-h)/2;
    }else if ((This->type & BT_OBJECTPOS_VMASK)==BT_TEXT_VBOTTOM){
      y=wa.height-h-2;
    }
    bool depressed=0;
    if (In==2 && (This->type & (BT_STATIC | BT_LINK))==0){
      x++,y++;
      depressed=true;
    }
    if (This->type & BT_CHECK){
      if (pcheck_ig){
        pcheck_ig->DrawIcon(int(This->checked ? check_on_icon:check_off_icon),Win,This->gc,x,(wa.height-pcheck_ig->IconHeight)/2+1);
        x+=pcheck_ig->IconWidth+4;
      }
    }
    if (This->type & BT_TEXT){
      if (This->but_font && Text[0]){
        XSetForeground(This->XD,This->gc,This->col_text);
        XSetFont(This->XD,This->gc,This->but_font->fid);
        XDrawString(This->XD,Win,This->gc,x,y+This->but_font->ascent,
                     Text,strlen(Text));
        XSetFont(This->XD,This->gc,This->font->fid);
      	if (This->type & BT_LINK){
          XDrawLine(This->XD,Win,This->gc,x,y+This->but_font->ascent+1,
         														x+w-1,y+This->but_font->ascent+1);
        }
      }
    }else if (This->type & BT_ICON){
      if (This->picons){ //use loaded icons
        if ((This->type & (BT_STATIC | BT_LINK | BT_NOBACKGROUND))==0 && depressed==0 && This->drop_shadow_bitmap){
          XSetForeground(This->XD,This->gc,This->col_border_dark);
          XSetBackground(This->XD,This->gc,This->col_background);
          XCopyPlane(This->XD,This->drop_shadow_bitmap,Win,This->gc,0,0,
                        This->picons->IconWidth,This->picons->IconHeight,x+1,y+1,1);
        }
        This->picons->DrawIcon(This->icon_index,Win,This->gc,x,y);
      }else{ //use standard icons
        if (This->icon_index<4){
	        XSetForeground(This->XD,This->gc,This->col_text);
          draw_triangle(This->XD,Win,This->gc,x,y,4,This->icon_index);
        }
      }
    }

    if (In){
      XSetForeground(This->XD,This->gc,TLCol);
      XDrawLine(This->XD,Win,This->gc,0,0,wa.width-1,0);
      XDrawLine(This->XD,Win,This->gc,0,0,0,wa.height-1);
      XSetForeground(This->XD,This->gc,BRCol);
      XDrawLine(This->XD,Win,This->gc,0,wa.height-1,wa.width-1,wa.height-1);
      XDrawLine(This->XD,Win,This->gc,wa.width-1,wa.height-1,wa.width-1,0);
    }
    XSync(This->XD,False);
  }
  return 0;
}

