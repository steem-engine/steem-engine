Atom hxc_listview::sb_but_timer_atom;
clock_t hxc_listview::sb_but_down_time;
//---------------------------------------------------------------
hxc_listview::hxc_listview()
{
  can_have_children=0;
  destroyproc=(LPHXC_DESTROYPROC)destroy;
  deleteproc=(LPHXC_DELETEPROC)delete_hxc_listview;

  sl.Sort=eslNoSort;
  sx=0;sy=0;
  itemheight=20;

  has_scrollbar=false;
  sb_up_state=0,sb_down_state=0;
  sb_drag_y=-999;

  border=1;
  sel=-1;
  display_mode=0;  //highlight according to first integer attribute
  lpig=NULL;
  allow_drag=0;
  checkbox_mode=0;
  dragging=-1;
  clicked_in=0;
  in_combo=0;

  text_pix=0;
  text_gc=0;
}
//---------------------------------------------------------------------------
bool hxc_listview::create(Display*d,Window daddy,int x,int y,
                      int w,int h,LPHXC_LISTVIEWNOTIFYPROC pass_notifyproc)
{
  if (XD!=NULL) destroy(this);
  XD=d;
  parent=daddy;
  notifyproc=pass_notifyproc;
	col_bg=col_white;

  load_res(XD);

  if (w==0) w=get_max_width(XD);

  unsigned long flags=CWBackingStore;
  if (in_combo){
    XWindowAttributes wa;
    wa.height=0;
    if (daddy){
      XGetWindowAttributes(XD,daddy,&wa);
      Window child;
      XTranslateCoordinates(XD,daddy,XDefaultRootWindow(XD),x,y,&x,&y,&child);
    }
    parent=XDefaultRootWindow(XD);

    int sw=XDisplayWidth(XD,XDefaultScreen(XD));
    int sh=XDisplayHeight(XD,XDefaultScreen(XD));
    if (wa.height>=sh) wa.height=0;
    if (x+w>sw) x=sw-w;
    if (y+h>sh){
      if (y-h-wa.height>=0){
        y-=h+wa.height;
      }else{
        y-=(y+h)-sh;
      }
    }
    flags|=CWOverrideRedirect | CWCursor;
  }

  hxc::x=x;hxc::y=y;hxc::w=w;hxc::h=h;

  XSetWindowAttributes swa;
  swa.backing_store=NotUseful;
  swa.override_redirect=True;
  swa.cursor=arrow_cursor;
  handle=XCreateWindow(XD,parent,x,y,w,h,0,CopyFromParent,
                  InputOutput,CopyFromParent,flags,&swa);

  SetProp(XD,handle,cWinProc,(DWORD)WinProc);
  SetProp(XD,handle,cWinThis,(DWORD)this);

  XSelectInput(XD,handle,KeyPressMask | KeyReleaseMask |
                            ButtonPressMask | ButtonReleaseMask |
                            ExposureMask | FocusChangeMask |
                            StructureNotifyMask | PointerMotionMask |
                            LeaveWindowMask | Button1MotionMask |
                            Button2MotionMask | Button3MotionMask);

  sb_but_timer_atom=XInternAtom(XD,"HXC_Button_Timer",0);

  itemheight=(font->ascent)+(font->descent)+2;
  if (display_mode==1) if (itemheight<18) itemheight=18;

  searchtext="";

  XMapWindow(XD,handle);

  if (in_combo){
    SetProp(XD,handle,cModal,(DWORD)0xffffffff); // Can't disable if no parent window!
    XGrabPointer(XD,handle,False,PointerMotionMask | ButtonPressMask |
            			ButtonReleaseMask,GrabModeAsync,GrabModeAsync,None,
            			None,CurrentTime);
    clicked_in=2;
 	}

  return false;
}

int hxc_listview::get_max_width(Display *XD)
{
  load_res(XD);

  int w=border*2;
  for (int i=0;i<sl.NumStrings;i++){
    int x=border+5+XTextWidth(font,sl[i].String,strlen(sl[i].String))+10+border;
    if (display_mode==1 && lpig) x+=lpig->IconWidth+5;
    w=max(x,w);
  }
  free_res(XD);
  return w;
}


int hxc_listview::select_item_by_data(int matchme,int datnum)
{
  for (int n=0;n<sl.NumStrings;n++){
    if (datnum<sl[n].NumData){
      if (sl[n].Data[datnum]==matchme){
        changesel(n);
        return n;
      }
    }
  }
  return -1;
}

void hxc_listview::additem(char* t)
{
  sl.Add(t,0);
}

void hxc_listview::additem(char* t,int x)
{
  sl.Add(t,x);
}

void hxc_listview::contents_change()
{
  if (XD==NULL) return;
  sy=0;
  draw(true);
}

void hxc_listview::changesel(int ns)
{
  if (XD==NULL){
    sel=ns;
    return;
  }
  if(ns!=sel){
    if(ns<-1 || ns>=sl.NumStrings){
      ns=-1;
    }
    highlightitem(sel,0);
    highlightitem(ns,2);
    sel=ns;
  }
}

int hxc_listview::getitemsperscreen()
{
  if (XD==NULL) return 20;

  return (h-border*2)/itemheight;
}

void hxc_listview::scrollto(int newsy)
{
  if (XD==NULL) return;

  if (newsy<0) newsy=0;
  int maxsy=max((sl.NumStrings)*itemheight-(h-border*2),0);
  if (newsy>maxsy) newsy=maxsy;
  if (sy!=newsy){
    sy=newsy;
    draw(true);
    XFlush(XD);
  }
}

void hxc_listview::make_item_visible(int s)
{
	int old_sy=sy;
  int topitem=(sy+itemheight-1)/itemheight;
  if (s<topitem){
    sy=s*itemheight;
  }else if (s*itemheight+itemheight>=sy+h){
    sy=max(s*itemheight-(h-border*2)+itemheight,0);
  }
  if (sy!=old_sy) draw(true,true);
}

void hxc_listview::highlightitem(int s,int on)
{
  if (XD==NULL || text_pix==0) return;
  // on = 0: not selected, 1: selected, 2: selected & scroll to
  if (s<0 || s>=sl.NumStrings) return;
  if (itemheight<=0) itemheight=20;

  DWORD col_fore=col_black,col_back=col_bg;
  if (on){
    col_fore=col_sel_fore;
    col_back=col_sel_back;
  }
  int cw=w-border*2,ch=h-border*2;
  int itemcount=ch/itemheight;
  int topitem=(sy+itemheight-1)/itemheight;
  int savesel=sel;
  if (on==2){
    sel=-1;
    make_item_visible(s);
    sel=savesel;
  }else if (s<topitem-1 || s>topitem+itemcount+2){
    return;
  }
  if (has_scrollbar) cw-=LV_SB_W;

  int y=s*itemheight-sy+border;
  draw_item(border,y,s,col_back,col_fore);
  XCopyArea(XD,text_pix,handle,gc,0,y-border,cw,itemheight,border,y);
}

void hxc_listview::draw_item(int win_x,int y,int s,
                            unsigned long bk,unsigned long fore)
{
  if (XD==NULL || handle==0) return;

  int cw=w-border*2,ch=h-border*2;
  if (has_scrollbar) cw-=LV_SB_W;
  if (cw<0 || ch<0) return;

  y-=border;
  XSetForeground(XD,text_gc,bk);
  XFillRectangle(XD,text_pix,text_gc,0,y,cw,itemheight);
  XSetForeground(XD,text_gc,fore);
  int x=5;
	int ty=y+(itemheight - font->descent + font->ascent)/2;
  if (display_mode==1){
    int icon_num=-1;
    if (lpig){
    	if (sl[s].String[0]==0){
    		x=((w-border-LV_SB_W) - lpig->IconWidth)/2;
    	}
    	if (sl[s].Data[0]==50){
		    icon_num=0;
		  }else{
	      icon_num=sl[s].Data[0]-101;
	    }
      lpig->DrawIcon(icon_num,text_pix,text_gc,x,y+(itemheight-lpig->IconHeight)/2);
      x+=lpig->IconWidth + 5;
    }else if (sl[s].Data[0]==50){
      XDrawString(XD,text_pix,text_gc,x,ty,"*",1);
      x+=16;
    }
  }
  if (columns.NumItems){
  	EasyStr coltext;
  	coltext.SetLength(strlen(sl[s].String)+columns.NumItems);
  	memset(coltext.Text,0,strlen(sl[s].String)+columns.NumItems);
		strcpy(coltext,sl[s].String); // make sure there are enough NULLS
  	for (int i=0;i<(int)strlen(sl[s].String);i++) if (coltext[i]=='\01') coltext[i]=0;

  	char *t=coltext.Text;
	  XDrawString(XD,text_pix,text_gc,x,ty,t,strlen(t));
	  t+=strlen(t)+1;
	  for (int n=0;n<columns.NumItems;n++){
      if (columns[n]>=0){
  		  XDrawString(XD,text_pix,text_gc,columns[n],ty,t,strlen(t));
      }
		  t+=strlen(t)+1;
		}
	}else{
	  XDrawString(XD,text_pix,text_gc,x,ty,sl[s].String,strlen(sl[s].String));
	}
}

void hxc_listview::destroy(hxc_listview*This)
{
  if (This->XD==NULL) return;

  if (This->in_combo){
    XUngrabPointer(This->XD,CurrentTime);
    RemoveProp(This->XD,This->handle,cModal);
  }
  RemoveProp(This->XD,This->handle,cWinProc);
  RemoveProp(This->XD,This->handle,cWinThis);
  XDestroyWindow(This->XD,This->handle);

  if (This->text_pix){
    XFreePixmap(This->XD,This->text_pix);
    XFreeGC(This->XD,This->text_gc);
    This->text_pix=0;
    This->text_gc=0;
  }
  free_res(This->XD);

  This->handle=0;
  This->XD=NULL;
  This->sel=-1;
}

void hxc_listview::draw(bool update_scrollbar,bool draw_contents,bool contents_change)
{
  if (XD==NULL || handle==0) return;

  if (draw_contents){
    int ns=sl.NumStrings;
    if (itemheight<=0) itemheight=20;
    int cw=w-border*2,ch=h-border*2;
    int item_h=ns*itemheight;
    int real_w=cw;

		if (ch < item_h){
      if (sy+ch > item_h) sy=item_h - ch;
      has_scrollbar=true;
      cw-=LV_SB_W;
    }else{
      has_scrollbar=0;
      sb_up_state=0;sb_down_state=0;sb_drag_y=-999;
      sy=0;
    }
    draw_scrollbar(real_w,ch);

    int y=border-(sy % itemheight);
    int n=sy/itemheight;

    static int old_cw=0,old_ch=0;
    if (old_cw!=w || old_ch!=ch || text_pix==0){
      if (text_pix) XFreePixmap(XD,text_pix);
      text_pix=XCreatePixmap(XD,handle,cw,ch,
                    XDefaultDepth(XD,XDefaultScreen(XD)));
      if (text_gc==0){
        text_gc=XCreateGC(XD,text_pix,0,NULL);
        XSetFont(XD,text_gc,font->fid);
      }
      old_cw=cw, old_ch=ch;
      contents_change=true;
    }
    if (contents_change){
      while (n<ns && y<h){
        draw_item(border,y,n++,col_bg,col_black);
        y+=itemheight;
      }
      if ((y-border)<h){
        XSetForeground(XD,text_gc,col_bg);
        XFillRectangle(XD,text_pix,text_gc,0,y-border,cw,ch-(y-border));
      }
    }
    if (text_pix) XCopyArea(XD,text_pix,handle,gc,0,0,cw,ch,border,border);
    if (sel!=-1) highlightitem(sel,1);
  }
  if (border){
    Window Foc=0;
    int RevertTo;
    XGetInputFocus(XD,&Foc,&RevertTo);
    if (Foc==handle){
      draw_border(XD,handle,gc,0,0,w,h,border,col_black,col_black);
    }else{
      draw_border(XD,handle,gc,0,0,w,h,border,col_border_dark,col_border_light);
    }
  }
}

int hxc_listview::match_string(int s,char *t,bool check_s)
{
  int start_at=s;
  if (start_at<0) start_at=0;

  int n=start_at;
  if (check_s==0) n++;
  if (n>=sl.NumStrings) n=0;

  int chars=strlen(t);
  EasyStr n_mem;
  n_mem.SetLength(chars);

  do{
    memcpy(n_mem.Text,sl[n].String,min(chars,(int)strlen(sl[n].String)+1));
    if (IsSameStr_I(n_mem,t)) return n;
    n++;
    if (n>=sl.NumStrings) n=0;
  }while (n!=start_at);
  return -1;
}

int hxc_listview::WinProc(hxc_listview *This,Window Win,XEvent *Ev)
{
  if (This->XD==NULL) return 0;
	This->common_winproc(Ev);

  if (This->clicked_in!=1) if (This->scrollbar_process_event(Ev)) return 0;

  int mx=0,my=0;
  unsigned int buttonmask=0;
  bool Press=0;
  Time event_time=0;

  switch (Ev->type){
    case FocusIn:
      This->draw(0,0);
      break;
    case FocusOut:
    {
      This->draw(0,0);

      if (This->in_combo){
        Window Foc=0;
        int RevertTo;
        XGetInputFocus(This->XD,&Foc,&RevertTo);
      	if (Foc!=This->handle){
  	      if (This->notifyproc) (This->notifyproc)(This,LVN_CB_RETRACT,-1);
  	    }
  	  }
      break;
    }
    case MotionNotify:
      mx=Ev->xmotion.x-(This->border);
      my=Ev->xmotion.y-(This->border);
      event_time=Ev->xmotion.time;
      buttonmask=(Ev->xmotion.state & Button123Mask);
    case ButtonPress:
    {
      int wheel_move=0;
      if (Ev->type==ButtonPress){
        mx=Ev->xbutton.x-(This->border);
        my=Ev->xbutton.y-(This->border);
        event_time=Ev->xbutton.time;
        if (Ev->xbutton.button==Button1) buttonmask|=Button1Mask;
        if (Ev->xbutton.button==Button2) buttonmask|=Button2Mask;
        if (Ev->xbutton.button==Button3) buttonmask|=Button3Mask;
        if (Ev->xbutton.button==Button4) wheel_move=-1;
        if (Ev->xbutton.button==Button5) wheel_move=1;
        Press=true;
      }
      if (wheel_move){
        This->scrollto(This->sy+((This->h - This->border*2)-This->itemheight)*wheel_move);
      }
      int real_over=(my + This->sy) / This->itemheight;
      int over=min(max(real_over,0),This->sl.NumStrings-1);
      int old_sel=This->sel;
      bool mouse_in_window=(mx>=-This->border && mx<=This->w-This->border &&
                              my>=-This->border && my<=This->h-This->border);
      int lv_w=This->w-This->border*2;
      if (This->has_scrollbar) lv_w-=LV_SB_W;
      bool mouse_in=(mx>=0 && mx<lv_w && my>=0 && my<This->h - This->border*2);

      if (buttonmask && mouse_in_window){
        XSetInputFocus(This->XD,Win,RevertToParent,CurrentTime);
      }
      if (This->in_combo){
				if (mouse_in) if (This->clicked_in==2) This->clicked_in=3;
				if (mouse_in || (buttonmask & Button1Mask)) This->changesel(over);
				if (Press && buttonmask){
					if (mouse_in){
  					This->clicked_in=1;
          }else{
	          if (This->notifyproc) This->notifyproc(This,LVN_CB_RETRACT,-1);
          }
				}
      }else{
	      bool doubleclick=false;
        if (Press && mouse_in && buttonmask) This->clicked_in=1;
        if (Press && over==real_over && (buttonmask & Button1Mask)){
          if (event_time - This->last_click_time < 200){
            //might be double-click
            if (abs(mx - This->last_click_x)<=3 && abs(my - This->last_click_y)<=3){
              doubleclick=true;
            }
          }
        }
  			if (This->allow_drag){
          if (Press){
            if (over==real_over && This->clicked_in){
              if (This->checkbox_mode && (buttonmask & Button1Mask)){
                if (mx<5+This->lpig->IconWidth){
                  if (This->notifyproc) if (This->notifyproc(This,LVN_ICONCLICK,real_over)!=0){
                    mx=-999;
                  }
                }
              }
              if (mx>=0) This->changesel(over);
            }
  				}else{
            if (This->dragging>-1){
            	This->drag_move(mx,my);
            }else if (abs(mx - This->last_click_x)>3 || abs(my - This->last_click_y)>3){
            	if (real_over==over && buttonmask && This->clicked_in && This->last_click_x!=-999){
                if (buttonmask & Button3Mask) This->drag_button=Button3;
                if (buttonmask & Button2Mask) This->drag_button=Button2;
                if (buttonmask & Button1Mask) This->drag_button=Button1;
              	This->drag_start(This->sel);
              }
            }
  				}
  			}else if (buttonmask & Button1Mask){
 		      if (Press && This->checkbox_mode){
		      	if (mx<5+This->lpig->IconWidth){
   	          if (This->notifyproc) if (This->notifyproc(This,LVN_ICONCLICK,real_over)!=0){
             		mx=-999;
             	}
            }
          }
  				if (This->clicked_in && mx!=-999){
   	        This->changesel(over);
   	      }
        }
        if (Press){
          This->last_click_x=mx;
          This->last_click_y=my;
          This->last_click_time=event_time;
					if (mouse_in==0) This->last_click_x=-999;
					if (This->allow_drag && over!=real_over) This->last_click_x=-999;
        }
        if (doubleclick){
          This->last_click_x=-999;
          if (This->notifyproc) (This->notifyproc)(This,LVN_DOUBLECLICK,This->sel);
        }else if (This->sel > -1 && This->sel!=old_sel && This->dragging==-1){
        	if (This->notifyproc) (This->notifyproc)(This,LVN_SINGLECLICK,This->sel);
        }
      }
      break;
		}
    case ButtonRelease:
    {
      if (Ev->xbutton.button==Button4 || Ev->xbutton.button==Button5) break; // No wheel
      if (This->dragging > -1){
        This->drag_end(Ev->xbutton.x-(This->border),
                        Ev->xbutton.y-(This->border));
      }else if (This->clicked_in && This->notifyproc){
	      mx=Ev->xbutton.x - This->border;
	      my=Ev->xbutton.y - This->border;
	      int over=(my + This->sy) / This->itemheight;
        int sb_w=int(This->has_scrollbar ? LV_SB_W:0);
        if (over<0 || over > This->sl.NumStrings-1) over=-1;
        bool mouse_in=true;
        if (mx<0 - This->border || mx>=This->w - This->border - sb_w) mouse_in=0;
        if (my<0 - This->border || my>=This->h - This->border) mouse_in=0;
        if (This->in_combo){
          if (mouse_in==0) over=-1;
          if (This->clicked_in!=2 && !(This->clicked_in==3 && over<0)){
            This->notifyproc(This,LVN_CB_RETRACT,over);
          }
        }else if (mouse_in && (Ev->xbutton.button==Button3 || Ev->xbutton.button==Button2)){
          This->notifyproc(This,LVN_CONTEXTMENU,over);
        }
      }
      This->clicked_in=0;
      break;
    }
    case KeyPress:
      This->handle_keypress(This->XD,&(Ev->xkey));
      break;
    case Expose:
    {
      hxc::clip_to_expose_rect(This->XD,&(Ev->xexpose));
      XWindowAttributes wa;
      XGetWindowAttributes(This->XD,This->handle,&wa);
      This->w=wa.width;
      This->h=wa.height;
      This->draw(true,true,0);
      XSetClipMask(This->XD,hxc::gc,None);
      break;
    }
  }
  return 0;
}

void hxc_listview::draw_scrollbar(int w,int view_h)
{
  if (has_scrollbar==0) return;

  int x=w-LV_SB_W+border,y=border;
  for (int n=0;n<2;n++){
    int state=sb_up_state;
    if (n==1) state=sb_down_state;
    int indent=int(state ? 1:0);
    XSetForeground(XD,gc,col_bk);
    XFillRectangle(XD,handle,gc,x+indent,y+indent,LV_SB_W-indent*2,LV_SB_BUT_H-indent*2);
    XSetForeground(XD,gc,col_black);
    indent=int((state==2) ? 1:0);
    draw_triangle(XD,handle,gc,x+LV_SB_W/2+indent,y+LV_SB_BUT_H/2+indent,4,n);
    if (state){
      unsigned long tl_col=col_white,br_col=col_border_dark;
      if (state==2){
        tl_col=col_border_dark,br_col=col_white;
      }
      draw_border(XD,handle,gc,x,y,LV_SB_W,LV_SB_BUT_H,1,tl_col,br_col);
    }
    y=border+view_h-LV_SB_BUT_H;
  }
  int bar_h,bar_y;
  sb_calc_bar_pos(view_h,bar_y,bar_h);

  XSetForeground(XD,gc,col_border_light);
  XFillRectangle(XD,handle,gc,x,border+LV_SB_BUT_H,LV_SB_W,view_h-LV_SB_BUT_H*2);
  XSetForeground(XD,gc,col_bk);
  XFillRectangle(XD,handle,gc,x+1,border+bar_y+1,LV_SB_W-2,bar_h-2);
  unsigned long tl_col=col_white,br_col=col_border_dark;
  if (sb_drag_y!=-999 && sb_drag_y!=0xface){
    tl_col=col_border_dark,br_col=col_white;
  }
  draw_border(XD,handle,gc,x,border+bar_y,LV_SB_W,bar_h,1,tl_col,br_col);
}

void hxc_listview::sb_calc_bar_pos(int view_h,int &bar_y,int &bar_h)
{
  int item_h=sl.NumStrings*itemheight;
  int sb_h=view_h-LV_SB_BUT_H*2;    if (sb_h<10) sb_h=10;
  bar_h=(view_h*sb_h)/item_h;   if (bar_h<LV_SB_W-2) bar_h=LV_SB_W-2;

  int hr=sb_h-bar_h;        if (hr<0) hr=0;
  int rr=item_h-view_h;     if (rr<1) rr=1;
  bar_y=(sy*hr)/rr;
  if (bar_y+bar_h>=sb_h){
    if (bar_h>sb_h) bar_h=sb_h;
    bar_y=sb_h-bar_h;
  }
  bar_y+=LV_SB_BUT_H;
}

#define LV_BUT_SCROLL_SPEED 4

bool hxc_listview::scrollbar_process_event(XEvent *Ev)
{
  if (has_scrollbar==0) return 0;

  int cw=w-border*2,ch=h-border*2;
  int mx,my;
  unsigned int buttonmask=0;
  bool press=0;

  switch (Ev->type)
  {
    case ClientMessage:
    	if (Ev->xclient.message_type==sb_but_timer_atom){
    		clock_t t=clock();
      	if (t>sb_but_down_time){
      		int move_amount=(t-sb_but_down_time)/(CLOCKS_PER_SEC/100)*LV_BUT_SCROLL_SPEED;
      		if (move_amount<LV_BUT_SCROLL_SPEED) move_amount=LV_BUT_SCROLL_SPEED;
      		if (sb_up_state==2) scrollto(sy-move_amount);
      		if (sb_down_state==2) scrollto(sy+move_amount);
					sb_but_down_time=t+(CLOCKS_PER_SEC/100);
				}
				if (sb_but_down) send_event(XD,handle,sb_but_timer_atom);
				return true;
      }
			return 0;
    case LeaveNotify:
			if (sb_drag_y==-999 && sb_but_down==0){
				sb_up_state=0;
				sb_down_state=0;
				draw_scrollbar(cw,ch);
			}
			return 0;
    case MotionNotify:
      mx=Ev->xmotion.x-border;
      my=Ev->xmotion.y-border;
      buttonmask=(Ev->xmotion.state & Button123Mask);
      break;
    case ButtonPress:
      press=true;
      if (Ev->xbutton.button==Button1) buttonmask|=Button1Mask;
      if (Ev->xbutton.button==Button2) buttonmask|=Button2Mask;
      if (Ev->xbutton.button==Button3) buttonmask|=Button3Mask;
      if (buttonmask==0) return 0;
    case ButtonRelease:
      if (Ev->xbutton.button==Button4 || Ev->xbutton.button==Button5) return 0; // No wheel
      mx=Ev->xbutton.x-border;
      my=Ev->xbutton.y-border;
      break;
    default:
      return 0;
	}

  int x=cw-LV_SB_W;
  int sb_h=ch-LV_SB_BUT_H*2;    if (sb_h<10) sb_h=10;
  int bar_h,bar_y;
  sb_calc_bar_pos(ch,bar_y,bar_h);

  bool in_up=0,in_down=0,in_bar=0,in_pgup=0,in_pgdown=0,in_scroll=0;
  if (mx>=x && mx<cw && my>=0 && my<ch){
    if (press) in_scroll=true;
    if (my<LV_SB_BUT_H){
      in_up=true;
    }else if (my<bar_y){
      in_pgup=true;
    }else if (my<bar_y+bar_h){
      in_bar=true;
    }else if (my<ch-LV_SB_BUT_H){
      in_pgdown=true;
    }else{
      in_down=true;
    }
  }
  int new_sy=sy,old_sy=sy,old_up_state=sb_up_state,old_down_state=sb_down_state;
  int old_drag_y=sb_drag_y;

  sb_up_state=0;
  sb_down_state=0;
  if (sb_drag_y==-999 && sb_but_down==0){
    if ((buttonmask & Button2Mask) && press){ //MMB
      if (in_pgup || in_pgdown || in_bar) sb_drag_y=0xface;
    }
  }
  if (sb_drag_y==-999){
    if (sb_but_down==0){
      if ((buttonmask & Button1Mask) && press){
        if (in_up) sb_but_down=1;
        if (in_down) sb_but_down=2;
        if (in_pgup){
          sb_but_down=3;
          new_sy-=ch;
        }else if (in_pgdown){
          sb_but_down=3;
          new_sy+=ch;
        }else if (in_bar){
	      	sb_drag_y=my;
	      	sb_drag_bar_y=bar_y-LV_SB_BUT_H;
	      }
	      if (sb_but_down<3){
   				send_event(XD,handle,sb_but_timer_atom);
					sb_but_down_time=clock()+(CLOCKS_PER_SEC/8);
				}
      }
    }else if ((buttonmask & Button1Mask)==0){
      sb_but_down=0;
    }
  }else{
  	int new_bar_y;
  	if (sb_drag_y==0xface){
	  	new_bar_y=my-LV_SB_BUT_H - bar_h/2;
	  }else{
	  	new_bar_y=sb_drag_bar_y+(my-sb_drag_y);
	  }
    if (new_bar_y<0) new_bar_y=0;

    // convert new_bar_y to sy
    int item_h=sl.NumStrings*itemheight;
    new_sy=(new_bar_y*item_h)/sb_h;

    in_scroll=true;
    sb_but_down=0;
  	if (sb_drag_y==0xface){
	    if ((buttonmask & Button2Mask)==0) sb_drag_y=-999;
  	}else{
	    if ((buttonmask & Button1Mask)==0) sb_drag_y=-999;
	  }
  }
  if (sb_but_down){
    if (sb_but_down==1 && in_up){
      new_sy-=LV_BUT_SCROLL_SPEED;
      sb_up_state=2;
    }else if (sb_but_down==2 && in_down){
      new_sy+=LV_BUT_SCROLL_SPEED;
      sb_down_state=2;
    }
    in_scroll=true;
  }

  if (sb_drag_y==-999 && sb_but_down==0){
    if (in_up) sb_up_state=1;
    if (in_down) sb_down_state=1;
  }
  if (new_sy!=old_sy){
    scrollto(new_sy);
    if (sy!=old_sy) return in_scroll; // redrawn
  }
  if (old_up_state!=sb_up_state || old_down_state!=sb_down_state ||
        (old_drag_y!=-999)!=(sb_drag_y!=-999)){
    draw_scrollbar(cw,ch);
  }
  return in_scroll;
}


void hxc_listview::handle_keypress(Display *XD,XKeyEvent *xkey)
{
  if (XD==NULL) return;
  KeySym ks;
  ks=XKeycodeToKeysym(XD,xkey->keycode,0);

  if (notifyproc){
    if (notifyproc(this,LVN_KEYPRESS,ks)!=0) return;
  }

  int newsel=-999;
  switch (ks){
    case XK_Up:
      newsel=(sel)-1;
      break;
    case XK_Down:
      newsel=(sel)+1;
      break;
    case XK_Page_Up:case XK_Page_Down:
    {
      int np=getitemsperscreen();
      newsel=(sel)-np;
      if (ks==XK_Page_Down) newsel=(sel)+np;
      break;
    }
    case XK_Home:
      newsel=0;
      break;
    case XK_End:
      newsel=(sl.NumStrings)-1;
      break;
    case XK_Return:case XK_KP_Enter:
      if (sel!=-1 && notifyproc) (notifyproc)(this,LVN_RETURN,sel);
      break;
  }

  if (newsel==-999){
    char buf[33];
    int nc=XLookupString(xkey,buf,32,&ks,&searchtext_xcompose_status);
    if (nc){
      buf[nc]=0;
      bool check_sel=true;
      if (IsSameStr_I(searchtext,buf)){
        searchtext="";
        check_sel=0;
      }
      if (xkey->time > searchtext_timeout){
        searchtext=buf;
        check_sel=0;
      }else{
        searchtext+=buf;
      }
      searchtext_timeout=xkey->time+500;

      newsel=match_string(sel,searchtext,check_sel);
      if (newsel<0){
        newsel=-999;
        searchtext="";
      }
    }
  }

  if (newsel!=-999){
    if (newsel<0) newsel=0;
    if (newsel>=(sl.NumStrings)) newsel=(sl.NumStrings)-1;
    changesel(newsel);
    if (sel!=-1 && notifyproc) notifyproc(this,LVN_SELCHANGE,sel);
  }
}

void hxc_listview::drag_start(int over)
{
	if (notifyproc) if (notifyproc(this,LVN_CANTDRAG,over)) return;

	dragging=over;
  changesel(over);

  EasyStr string=sl[over].String;
  char *name_end=strchr(string,'\01');
  if (name_end) *name_end=0;
	int tw=hxc::get_text_width(XD,string);
	unsigned int w=8+2+tw+2; w+=31; w&=~31;
  unsigned int h=itemheight+8;
	int ty=8+(itemheight - font->descent + font->ascent)/2;

/*
  unsigned int ret_w,ret_h;
  int countdown=10;
  while (countdown--){
    XQueryBestSize(XD,CursorShape,handle,w,h,&ret_w,&ret_h);
    if (ret_w>=w && ret_h>=h) break;
    if (ret_w<w) w+=16;
    if (ret_h<h) h+=4;
  }
  w=ret_w;
  h=ret_h;
*/

  while ((int)w<8+2+tw+2){
    *(string.Right())=0;
    if (string.Empty()) break;
  	tw=hxc::get_text_width(XD,string);
  }

	unsigned int bytes=((w * h)+7)/8;
  char *dat=new char[bytes];

  memset(dat,0,bytes);
  Pixmap SrcPix=XCreatePixmapFromBitmapData(XD,handle,
                    dat,w,h,0,0,1);

  Pixmap MaskPix=XCreatePixmapFromBitmapData(XD,handle,
                    dat,w,h,0,0,1);

	XGCValues gcv;
	gcv.foreground=1;
	gcv.font=font->fid;
  GC mono_gc=XCreateGC(XD,SrcPix,GCForeground | GCFont,&gcv);

  for (int n=5;n>0;n--) XDrawLine(XD,SrcPix,mono_gc,1,6-n,n,6-n);
  XDrawLine(XD,SrcPix,mono_gc,1,1,8,8);
  XDrawString(XD,SrcPix,mono_gc,8+2,ty,string,strlen(string));
  XDrawRectangle(XD,SrcPix,mono_gc,8,8,2+tw+1,itemheight-1);

  for (int n=7;n>0;n--) XDrawLine(XD,MaskPix,mono_gc,0,7-n,n,7-n);
  XDrawLine(XD,MaskPix,mono_gc,1,1,8,8);
  XDrawLine(XD,MaskPix,mono_gc,1,2,8,9);
  XDrawLine(XD,MaskPix,mono_gc,2,1,9,8);
  XFillRectangle(XD,MaskPix,mono_gc,8,8,2+tw+2,itemheight);

  XFreeGC(XD,mono_gc);


  XColor ccols[2];
  ccols[0].pixel=col_black;
  XQueryColor(XD,XDefaultColormap(XD,XDefaultScreen(XD)),&(ccols[0]));
  ccols[1].pixel=col_bg;
  XQueryColor(XD,XDefaultColormap(XD,XDefaultScreen(XD)),&(ccols[1]));
  drag_cursor=XCreatePixmapCursor(XD,SrcPix,MaskPix,
                    &(ccols[0]),&(ccols[1]),0,0);
  XFreePixmap(XD,SrcPix);
  XFreePixmap(XD,MaskPix);
  delete[] dat;

  XGrabPointer(XD,handle,False,PointerMotionMask | ButtonPressMask |
  					ButtonReleaseMask,GrabModeAsync,GrabModeAsync,None,
  					drag_cursor,CurrentTime);
}

void hxc_listview::drag_move(int mx,int my)
{
  if (my<-10 || my>h-border*2 +10) return;

	bool CantDrop=0;
	int sb_w=int(has_scrollbar ? LV_SB_W:0);
	if (mx<0 || mx>w-border*2-sb_w) CantDrop=true;
	if (my<0 || my>h-border*2) CantDrop=true;
  int over=min(max((my+sy)/itemheight,0),sl.NumStrings-1);
	if (notifyproc && CantDrop==0) CantDrop=(bool)notifyproc(this,LVN_CANTDROP,over);
	if (CantDrop){
		changesel(dragging);
		make_item_visible(over);
	}else{
		changesel(over);	
	}
}

void hxc_listview::drag_end(int mx,int my)
{
	int i=dragging;
	dragging=-1;
  XUngrabPointer(XD,CurrentTime);
  XFreeCursor(XD,drag_cursor);
	
	if (notifyproc==NULL) return;
	
	hxc_listview_drop_struct ds;
	ds.dragged=i;
	
	ds.dx=mx;
	ds.dy=my;
  ds.button=drag_button;

  ds.on=(my+sy)/itemheight;
  if (ds.on<0 || ds.on>=sl.NumStrings){
  	ds.on=-1;
  }else{
  	if (notifyproc(this,LVN_CANTDROP,ds.on)) ds.on=-1;
  }

  ds.in_lv=true;
	int sb_w=int(has_scrollbar ? LV_SB_W:0);
	if (mx<0 || mx>w-border*2-sb_w) ds.in_lv=0;
	if (my<0 || my>h-border*2) ds.in_lv=0;

	notifyproc(this,LVN_DROP,(int)&ds);
}

bool hxc_listview::is_dropped_in(hxc_listview_drop_struct *pds,hxc *phxc)
{
	int par_x=x+pds->dx,par_y=y+pds->dy;
	if (par_x >= phxc->x && par_x <= (phxc->x+phxc->w)){
		if (par_y >= phxc->y && par_y <= (phxc->y+phxc->h)){
			return true;
		}
	}
	return 0;
}

