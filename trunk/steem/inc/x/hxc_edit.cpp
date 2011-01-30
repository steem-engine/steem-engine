//---------------------------------------------------------------
void hxc_edit::init()
{
  can_have_children=false;
  destroyproc=(LPHXC_DESTROYPROC)destroy;
  deleteproc=(LPHXC_DELETEPROC)delete_hxc_edit;
  sx=0;
  border=1;
  pad_x=5;
  cursor=0;
  selflag=false;
  dragging=false;
}
//---------------------------------------------------------------------------
#define HXC_TEXT_SCROLL_JUMP (w/2)

bool hxc_edit::create(Display*d,Window daddy,int x,int y,
                      int w,int h,LPHXC_EDITNOTIFYPROC pass_notifyproc)
{
  if (XD!=NULL) destroy(this);
  XD=d;
  parent=daddy;
  notifyproc=pass_notifyproc;

  load_res(XD);


  XSetWindowAttributes swa;
  swa.backing_store=NotUseful;
  swa.background_pixel=col_white;
	
	hxc::x=x;hxc::y=y;hxc::w=w;hxc::h=h;
  handle=XCreateWindow(XD,parent,x,y,w,h,0,
                           CopyFromParent,InputOutput,CopyFromParent,
                           CWBackingStore|CWBackPixel,&swa);

  SetProp(XD,handle,cWinProc,(DWORD)WinProc);
  SetProp(XD,handle,cWinThis,(DWORD)this);

  XSelectInput(XD,handle,KeyPressMask|KeyReleaseMask|
                            ButtonPressMask|ButtonReleaseMask|
                            ExposureMask|FocusChangeMask|
                            StructureNotifyMask|Button1MotionMask
                            );

  XSetForeground(XD,gc,col_black);

  draw(true);

  XMapWindow(XD,handle);

  return false;
}

void hxc_edit::scrollto(int newsx){
  XWindowAttributes wa;
  XGetWindowAttributes(XD,handle,&wa);
  int w=wa.width-border*2;
//  int h=wa.height-border*2;
  int tw=XTextWidth(font,text,strlen(text));
  int maxsx=tw-w+pad_x*2;
  if(newsx>maxsx){
    newsx=maxsx;
  }
  if(newsx<0)newsx=0;
  sx=newsx;
}

void hxc_edit::set_text(char*t,bool selectall)
{
	if(t){
	  text=t;
	}else{
		text="";
	}
	if(selectall){
		select_all();
	}else{
		selflag=false;
		cursor=(int)strlen(text);
	}
	draw();
}

void hxc_edit::select_all()
{
	selflag=true;
	sel1=0;
	sel2=(int)strlen(text);
	cursor=sel2;
	draw();
}

void hxc_edit::destroy(hxc_edit*This)
{
  if (This->XD==NULL) return;

  RemoveProp(This->XD,This->handle,cWinProc);
  RemoveProp(This->XD,This->handle,cWinThis);
  XDestroyWindow(This->XD,This->handle);

  free_res(This->XD);

  This->handle=0;
  This->XD=NULL;
}

void hxc_edit::draw(bool draw_contents)
{
  if (XD==NULL) return;

  XWindowAttributes wa;
  XGetWindowAttributes(XD,handle,&wa);
  int w=wa.width-border*2;
//  int h=wa.height-border*2;
//  Window Foc=0;
//  int RevertTo;
//  XGetInputFocus(XD,&Foc,&RevertTo);
  bool has_focus=IsFocussed(XD,handle);
  if (draw_contents){
//    int maxsy=n_lines*textheight-h;
//    if(sy>maxsy)scrollto(maxsy);
    int cursor_x,scx;
    if(cursor==0){
      cursor_x=0;
    }else{
//      char buf=text[cursor];
//      text[cursor]=0;
      cursor_x=XTextWidth(font,text,cursor);
//      text[cursor]=buf;
    }
    scx=cursor_x+pad_x-sx;
    int newsx;
    if(scx<pad_x){ //gone off the  left
      newsx=cursor_x-pad_x;
      if(sx-newsx<HXC_TEXT_SCROLL_JUMP && !(dragging)){
        newsx=sx-HXC_TEXT_SCROLL_JUMP;
      }
      scrollto(newsx);
      scx=cursor_x+pad_x-sx;
    }
    if(scx>w-pad_x){ //gone off the right
      newsx=cursor_x-w+(pad_x*2);
      if(newsx-sx<HXC_TEXT_SCROLL_JUMP && !(dragging)){
        newsx=sx+HXC_TEXT_SCROLL_JUMP;
      }
      scrollto(newsx);
      scx=cursor_x+pad_x-sx;
    }

    XSetForeground(XD,gc,col_white);
    XFillRectangle(XD,handle,gc,border,border,wa.width-2*border,wa.height-2*border);
    XSetForeground(XD,gc,col_black);

    if (sel1==sel2) selflag=false;
    int selstart,selend;
    int xp;
    int ytp=(wa.height+(font->ascent)-(font->descent))/2;
    int y1=ytp-(font->ascent),y2=ytp+(font->descent);
    if(selflag && has_focus){
      if(sel1<sel2){selstart=sel1;selend=sel2;}
      else{selstart=sel2;selend=sel1;}
      xp=border+pad_x-sx;
      XSetForeground(XD,gc,col_sel_back);
      int x=XTextWidth(font,text,selstart);
      int w=XTextWidth(font,text.Text+selstart,selend-selstart);
      XFillRectangle(XD,handle,gc,xp+x,y1,w,y2-y1+1);

      XSetForeground(XD,gc,col_black);
      XDrawString(XD,handle,gc,xp,ytp,text,selstart);
      XSetForeground(XD,gc,col_sel_fore);
      XDrawString(XD,handle,gc,xp+x,ytp,text.Text+selstart,selend-selstart);
      XSetForeground(XD,gc,col_black);
      x=XTextWidth(font,text,selend);
      XDrawString(XD,handle,gc,xp+x,ytp,text.Text+selend,strlen(text.Text+selend));
    }else{
      XDrawString(XD,handle,gc,border+pad_x-sx,ytp,text,strlen(text));
      if(has_focus){
        XDrawLine(XD,handle,gc,border+scx,y1,border+scx,y2);
      }
    }
  }
  if(border){
    if(has_focus){
      draw_border(XD,handle,gc,0,0,wa.width,wa.height,border,col_black,col_black);
    }else{
      draw_border(XD,handle,gc,0,0,wa.width,wa.height,border,col_border_dark,col_border_light);
    }
  }
}

int hxc_edit::character_index_from_x_coordinate(int x){
  if(font==NULL){
    return 0;
  }
  int l=(int)strlen(text),w=0,ow;
  char buf;
  for(int n=1;n<=l;n++){
    buf=text[n];
    text[n]=0;
    ow=w;
    w=XTextWidth(font,text,n);
    text[n]=buf;
    if(w>x){
      if(w-x>=x-ow){
        return n-1; //go left of click
      }else{
        return n; //go right of click
      }
    }
  }
  return l;
}


bool hxc_edit::is_alphanumeric_character(char c){
  return (c>='A' && c<='Z') || (c>='a' && c<='z') || (c>='0' && c<='9');
}

void hxc_edit::replace_selection(char*t)
{
  if (selflag){
	  int selstart=sel1,selend;
    if (sel1!=sel2){
      if (sel1<sel2){
      	selstart=sel1;selend=sel2;
      }else{
      	selstart=sel2;selend=sel1;
      }
      text.Delete(selstart,selend-selstart);
    }
    selflag=0;
    cursor=selstart;
  }
  if (*t){
    text.Insert(t,cursor);
    cursor+=strlen(t);
  }
}


void hxc_edit::copy_selection_to_clipboard()
{
  int selstart,selend;
  if (sel1==sel2) selflag=false;
  if (selflag){
    if (sel1<sel2){
      selstart=sel1;selend=sel2;
    }else{
      selstart=sel2;selend=sel1;
    }
    clipboard=text.Mids(selstart,selend-selstart);
  }
  XSetSelectionOwner(XD,XA_PRIMARY,handle,CurrentTime);
}

bool hxc_edit::get_clipboard_text()
{
  Window selection_owner=XGetSelectionOwner(XD,XA_PRIMARY);
  if (selection_owner==None){
    clipboard="";
    return false; //succeed
  }else if (selection_owner==handle){
    return false; //succeed
  }else{
    XEvent SendEv;
    SendEv.type=SelectionRequest;
    SendEv.xselectionrequest.requestor=handle;
    SendEv.xselectionrequest.owner=selection_owner;
    SendEv.xselectionrequest.selection=XA_PRIMARY;
    SendEv.xselectionrequest.target=XA_STRING;
    SendEv.xselectionrequest.property=XA_CUT_BUFFER0;
    SendEv.xselectionrequest.time=CurrentTime;
    XSendEvent(XD,selection_owner,0,0,&SendEv);
    clipboard="";
    return true; //fail - for now! (?)
  }
}


void hxc_edit::copy()
{
  if (selflag) copy_selection_to_clipboard();
}

void hxc_edit::cut()
{
  if (selflag){
    copy_selection_to_clipboard();
    replace_selection("");
  }
}

void hxc_edit::paste(){
  if(!get_clipboard_text()){
    replace_selection(clipboard);
  }
}

void hxc_edit::handle_keypress(Display*XD,XKeyEvent*xk){
  if(XD==NULL)return;
  KeySym ks;
  ks=XKeycodeToKeysym(XD, xk->keycode, 0);

  int blockkey=0;
  int oldcursor=cursor;
	bool modified=false;

  if(IsModifierKey(ks)){
    return;
  }
  switch(ks){
    case XK_Left:
      if(cursor>0){
        cursor--;
        if (xk->state & ControlMask){
          int cn=cursor;bool a9=is_alphanumeric_character(text[cn]),olda9;
          for(;;){
            cn--;
            if(cn<=0){
              cursor=0;break;
            }else{
              olda9=a9;
              a9=is_alphanumeric_character(text[cn]);
              if(olda9 && !a9){
                cursor=cn+1;
                break;
              }
            }
          }
        }
      }
      if (xk->state & ShiftMask){
        blockkey=-1;
        if (!selflag) sel1=oldcursor;
        sel2=cursor;
      }
      break;
    case XK_Right:
      if(cursor<(int)strlen(text)){
        cursor++;
        if (xk->state & ControlMask){
          int cn=cursor;bool a9=is_alphanumeric_character(text[cn]),olda9;
          for(;;){
            cn++;
            if(cn>=(int)strlen(text)){
              cursor=(int)strlen(text);break;
            }else{
              olda9=a9;
              a9=is_alphanumeric_character(text[cn]);
              if(!olda9 && a9){
                cursor=cn;
                break;
              }
            }
          }
        }
      }
      if (xk->state & ShiftMask){
        blockkey=-1;
        if (!selflag) sel1=oldcursor;
        sel2=cursor;
      }
      break;
    case XK_Up:
      break;
    case XK_Down:
      break;
    case XK_Home:case XK_Page_Up:
      cursor=0;
      if (xk->state & ShiftMask){
        blockkey=-1;
        if (!selflag) sel1=oldcursor;
        sel2=cursor;
      }
      break;
    case XK_End:case XK_Page_Down:
      cursor=(int)strlen(text);
      if (xk->state & ShiftMask){
        blockkey=-1;
        if (!selflag) sel1=oldcursor;
        sel2=cursor;
      }
      break;
    case XK_Delete:
      if (selflag){
        replace_selection("");
        modified=true;
      }else if(cursor<(int)strlen(text)){
        text.Delete(cursor,1);
        modified=true;
      }
      break;
    case XK_BackSpace:
      if (selflag){
        replace_selection("");
        modified=true;
      }else if (cursor>0){
        text.Delete(--cursor,1);
        modified=true;
      }
      break;
    case XK_Return:case XK_KP_Enter:
			if(notifyproc)notifyproc(this,EDN_RETURN,0);
			return;
    default:{
      if((xk->state) & ControlMask){
        if(ks==XK_c){ //copy
          copy();blockkey=2;
        }else if(ks==XK_x){ //cut
          cut();
	        modified=true;
        }else if(ks==XK_v){ //paste
          paste();blockkey=2;
	        modified=true;
        }else if(ks==XK_a){ //select all
        	select_all();
        }
//      }else if(ks==XK_Copy){  Copy, Cut, Paste don't exist
//        copy();blockkey=true;
      }else if( (((xk->state) & ShiftMask) && ks==XK_Delete) /*||
                  ks==XK_Cut */){
        cut();
        modified=true;
      }else if( (((xk->state) & ShiftMask) && ks==XK_Insert) /*||
                  ks==XK_Paste */){
        paste();blockkey=2;
        modified=true;
      }else{
        char buf[33];
        int nc=XLookupString(xk,buf,32,&ks,&xcompose_status);
        if (nc){
          buf[nc]=0;
          if (selflag){
            replace_selection(buf);
          }else{
            cursor=max(0,min((int)strlen(text),cursor));
            text.Insert(buf,cursor);
            cursor+=nc;
          }
	        modified=true;
        }
      }
    }
  }
  if (blockkey!=2){
    if (blockkey==-1 && sel1!=sel2) selflag=true;
    else selflag=false;
  }
  if(XD){
  	draw(true);
		if(modified){
			if(notifyproc)notifyproc(this,EDN_CHANGE,0);
		}
	  XFlush(XD);
	}
}

void hxc_edit::select_word(int cn){
  bool a9=is_alphanumeric_character(text[cn]); //,olda9;
  int original_cn=cn;
  for(;;){
    cn--;
    if(cn<=0){
      sel1=0;break;
    }else{
//      olda9=a9;
      if(a9!=is_alphanumeric_character(text[cn])){
        sel1=cn+1;
        break;
      }
    }
  }
  cn=original_cn;
  int l=strlen(text);
  for(;;){
    cn++;
    if(cn>=l){
      cursor=l;break;
    }else{
//      olda9=a9;
      if(a9!=is_alphanumeric_character(text[cn])){
        cursor=cn;
        break;
      }
    }
  }
  sel2=cursor;
  selflag=true;
}

int hxc_edit::WinProc(hxc_edit *This,Window Win,XEvent *Ev)
{
  if (This->XD==NULL) return 0;
  This->dragging=false;
	This->common_winproc(Ev);

  int mx,my;
//  bool leftbuttondown;

  switch(Ev->type){
    case SelectionRequest:{
/*
typedef struct {
     int type;                 SelectionRequest
     unsigned long serial;     # of last request processed by server
     Bool send_event;          true if this came from a SendEvent request
     Display *display;         Display the event was read from
     Window owner;
     Window requestor;
     Atom selection;
     Atom target;
     Atom property;
     Time time;
} XSelectionRequestEvent;
*/
      XEvent SendEv;
//typedef struct {
//     int type;                /* SelectionRequest */
//     unsigned long serial;    /* # of last request processed by server */
//     Bool send_event;         /* true if this came from a SendEvent request */
//     Display *display;        /* Display the event was read from */
/*     Window owner;
     Window requestor;
     Atom selection;
     Atom target;
     Atom property;
     Time time;
} XSelectionRequestEvent; */
      Window dest_win=Ev->xselectionrequest.requestor;
      Atom prop=0;

      SendEv.type=SelectionNotify;
      SendEv.xselection.requestor=dest_win;
      bool failed=false;
      if(Ev->xselectionrequest.target!=XA_STRING){
        failed=true;
      }else{
        prop=Ev->xselectionrequest.property;
        if(prop==None){
          prop=XA_CUT_BUFFER0;
        }
        XChangeProperty(This->XD,dest_win,prop,XA_STRING,8,
                  PropModeReplace, (BYTE*)(This->clipboard.Text),
                  (int)(This->clipboard.Length()) );
        failed=false; //if it gets here, it hasn't failed!
      }

      if(failed){
        SendEv.xselection.selection=None;
        SendEv.xselection.target=None;
        SendEv.xselection.property=None;
        SendEv.xselection.time=CurrentTime;
      }else{
        SendEv.xselection.selection=Ev->xselectionrequest.selection;
        SendEv.xselection.target=XA_STRING;
        SendEv.xselection.property=prop;
        SendEv.xselection.time=CurrentTime;
      }
      XSendEvent(This->XD,dest_win,0,0,&SendEv);

      break;
    }case SelectionNotify:
/*
typedef struct {
     int type;                 SelectionNotify
     unsigned long serial;     # of last request processed by server
     Bool send_event;          true if this came from a SendEvent request
     Display *display;         Display the event was read from
     Window requestor;
     Atom selection;
     Atom target;
     Atom property;            atom or None
     Time time;
} XSelectionEvent;
*/
      if(Ev->xselection.property!=None){
        if(Ev->xselection.target==XA_STRING){
/*
int XGetWindowProperty(display, w, property, long_offset, long_length, delete, req_type,
                        actual_type_return, actual_format_return, nitems_return, bytes_after_return,
                        prop_return)
*/
          Atom actual_type_return;
          int actual_format_return;
          unsigned long nitems_return;
          unsigned long bytes_after_return;
          char*t;
          XGetWindowProperty(This->XD,This->handle,Ev->xselection.property,
                        /*long_offset*/ 0, /*long_length*/ 5000,
                        /*delete*/ True, XA_STRING,
                        &actual_type_return, &actual_format_return,
                        &nitems_return, &bytes_after_return,
                        (BYTE**)(&t));
          if (actual_type_return==XA_STRING){
            This->replace_selection(t);
            This->clipboard="";
	          XFree(t);
            This->draw(true);
          }
        }
      }
      break;
    case FocusIn:
      This->draw(true);
      break;
    case FocusOut:
      This->draw(true);
 			if (This->notifyproc) This->notifyproc(This,EDN_LOSTFOCUS,0);
      break;
    case MotionNotify:{
      mx=Ev->xmotion.x;
      int x=mx-(This->border)-(This->pad_x)+(This->sx);
      if(!(This->selflag)){
        This->selflag=true;
        This->sel1=This->cursor;
      }
      This->cursor=(This->character_index_from_x_coordinate(x));
      This->sel2=This->cursor;
      This->dragging=true;
      This->draw(true);
      break;
    }case ButtonPress:{
      if (Ev->xbutton.button==Button4 || Ev->xbutton.button==Button5) break;
    	bool just_activate=true;
    	if(!(This->selflag))just_activate=false;
    	if(IsFocussed(This->XD,Win)){
    		just_activate=false;
    	}else if(!just_activate){
      	XSetInputFocus(This->XD,Win,RevertToParent,CurrentTime);
    	}
			if(just_activate){    	
      	XSetInputFocus(This->XD,Win,RevertToParent,CurrentTime);
      }else{
        if (Ev->xbutton.button==Button1){
          bool doubleclick=false;
          int cn;
          mx=Ev->xbutton.x;my=Ev->xbutton.y;
          if (Ev->xbutton.time - This->last_click_time < 200){
            //might be double-click
            if (abs(mx - This->last_click_x)<3 && abs(my - This->last_click_y)<3){
              doubleclick=true;
            }
          }
//          XSetInputFocus(This->XD,Win,RevertToParent,CurrentTime);
          int x=mx-(This->border)-(This->pad_x)+(This->sx);
          cn=(This->character_index_from_x_coordinate(x));
          if(doubleclick){
            This->select_word(cn);
          }else{
            This->cursor=cn;
            This->last_click_x=mx;
            This->last_click_y=my;
            This->last_click_time=Ev->xbutton.time;
            This->selflag=false;
          }
          This->draw(true);
        }else if (Ev->xbutton.button==Button2){ //MMB
          mx=Ev->xbutton.x;
//          XSetInputFocus(This->XD,Win,RevertToParent,CurrentTime);
          int x=mx-(This->border)-(This->pad_x)+(This->sx);
          This->cursor=(This->character_index_from_x_coordinate(x));
          This->selflag=false; //remove current selection
          This->paste();
          This->draw(true);
        }
      }
      break;
    }case KeyPress:
      This->handle_keypress(This->XD,&(Ev->xkey));
      break;
    case Expose:
      if (Ev->xexpose.count>0) break;
      This->draw(true);
      XSync(This->XD,0);
      break;
    case ConfigureNotify:{
      This->scrollto(This->sx); //in case it's showing whitespace, oh no!
/*
      XWindowAttributes wa;
      XGetWindowAttributes(This->XD,Win,&wa);
      if(This->has_scrollbar){
        (This->sb.init)((This->n_lines)*(This->textheight),
                wa.height-(This->border)*2,This->sy);
        XMoveResizeWindow(This->XD,This->sb.handle,
                        wa.width-LV_SCROLLBAR_WIDTH-(This->border),(This->border),
                        LV_SCROLLBAR_WIDTH,max(wa.height-(This->border)*2,10));
      }
      This->fix_top_left_char=true;
      This->wordwrapped=false;
      XSync(This->XD,0); */
    }break;
  }
  return 0;
}

