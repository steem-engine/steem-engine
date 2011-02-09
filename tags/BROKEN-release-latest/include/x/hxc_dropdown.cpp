//---------------------------------------------------------------
void hxc_dropdown::init()
{
  can_have_children=true; //the listview
  destroyproc=(LPHXC_DESTROYPROC)destroy;
  deleteproc=(LPHXC_DELETEPROC)delete_hxc_dropdown;

  undropped_h=23;
  dropped_h=200;
  dropped_w=0;

  border=1;

  dropped=false;
  sel=0;
  lv.sel=0;
	grandfather=0;
  lv.owner=this;
  lpig=NULL;
}
//---------------------------------------------------------------------------
#define DD_TEXT_XPAD 2

bool hxc_dropdown::create(Display*d,Window daddy,int x,int y,
                      int w,int h,LPHXC_DROPDOWNNOTIFYPROC pass_notifyproc)
{
  if (XD!=NULL) destroy(this);
  XD=d;
  parent=daddy;
  notifyproc=pass_notifyproc;

  load_res(XD);

  XSetWindowAttributes swa;
  swa.backing_store=NotUseful;
  swa.background_pixel=col_white;
  handle=XCreateWindow(XD,parent,x,y,w,undropped_h+border*2,0,
                           CopyFromParent,InputOutput,CopyFromParent,
                           CWBackingStore|CWBackPixel,&swa);
	hxc::x=x;hxc::y=y;hxc::w=w;hxc::h=undropped_h+border*2;

  SetProp(XD,handle,cWinProc,(DWORD)WinProc);
  SetProp(XD,handle,cWinThis,(DWORD)this);

  XSelectInput(XD,handle,KeyPressMask | KeyReleaseMask |
                         ButtonPressMask | ButtonReleaseMask |
                         ExposureMask | FocusChangeMask |
                         StructureNotifyMask);

  lv.sel=sel;
  dropped_h=h;

  XMapWindow(XD,handle);
  draw();

  return false;
}
//---------------------------------------------------------------------------
int hxc_dropdown::select_item_by_data(int matchme,int datnum)
{
  int ret=lv.select_item_by_data(matchme,datnum);
  if (ret>=0) changesel(lv.sel);
  return ret;
}
//---------------------------------------------------------------------------
void hxc_dropdown::make_empty()
{
  lv.sl.DeleteAll();
  sel=0;
}
//---------------------------------------------------------------------------
void hxc_dropdown::additem(char* t)
{
  lv.additem(t);
}
//---------------------------------------------------------------------------
void hxc_dropdown::additem(char* t,int x)
{
  lv.additem(t,x);
}
//---------------------------------------------------------------------------
void hxc_dropdown::changesel(int ns)
{
  if(ns!=sel){
    if(ns<0 || ns>=lv.sl.NumStrings){
      ns=0;
    }
    sel=ns;
  }
	lv.changesel(sel);
}
//---------------------------------------------------------------------------
int hxc_dropdown::get_min_width()
{
  load_res(XD);

  int tw=0;
	for (int n=0;n<sl.NumStrings;n++){
		tw=max(tw,get_text_width(XD,sl[n].String));		
	}

  free_res(XD);
  return border+DD_TEXT_XPAD+tw+DD_TEXT_XPAD+LV_SB_W+border;
}
//---------------------------------------------------------------------------
void hxc_dropdown::draw(bool draw_contents)
{
  if (XD==NULL) return;

  XWindowAttributes wa;
  XGetWindowAttributes(XD,handle,&wa);

  DWORD col_fore,col_back;
//  if(on){
//    col_fore=col_white;
//    col_back=col_black;
//  }else{
  col_fore=col_black;
  col_back=col_white;
//  }

  if (draw_contents){
    XSetForeground(XD,gc,col_back);
    XFillRectangle(XD, handle, gc, border, border, wa.width-border*2, wa.height-border*2);
    XSetForeground(XD,gc,col_fore);

    int ytp=(wa.height+(font->ascent)-(font->descent))/2;

    if (sel<lv.sl.NumStrings){
    	int x=border+DD_TEXT_XPAD;
    	if (lpig){
    		if (lv.sl[sel].NumData){
		    	if (lv.sl[sel].String[0]==0){
		    		x=((wa.width-border-LV_SB_W) - lpig->IconWidth)/2;
		    	}
    			if (lpig->DrawIcon(lv.sl[sel].Data[0]-101,handle,gc,
    														x,(wa.height - lpig->IconHeight)/2)){
    				x+=lpig->IconWidth+2;
    			}   			
    		}
    	}
      XDrawString(XD,handle,gc,border+DD_TEXT_XPAD,
            ytp,lv.sl[sel].String,strlen(lv.sl[sel].String));
    }

    int xp=wa.width-border-LV_SB_W;
    XSetForeground(XD,gc,col_grey);
    XFillRectangle(XD, handle, gc, xp, border, LV_SB_W, wa.height-border*2);
    XSetForeground(XD,gc,col_black);
//void hxc::draw_triangle(Display*XD,Drawable d,GC gc,int x,int y,int size,bool upwards){
    draw_triangle(XD,handle,gc,xp+LV_SB_W/2,wa.height/2,4,1);
  }
  Window Foc=0;
  int RevertTo;
  XGetInputFocus(XD,&Foc,&RevertTo);
  if(Foc==handle){
    draw_border(XD,handle,gc,0,0,wa.width,wa.height,border,col_black,col_black);
  }else{
    draw_border(XD,handle,gc,0,0,wa.width,wa.height,border,col_border_dark,col_border_light);
  }
}
//---------------------------------------------------------------------------
void hxc_dropdown::destroy(hxc_dropdown*This)
{
  if (This->XD==NULL) return;

  if(This->dropped){
    This->lv.destroy(&(This->lv));
    This->dropped=false;
  }
  RemoveProp(This->XD,This->handle,cWinProc);
  RemoveProp(This->XD,This->handle,cWinThis);
  XDestroyWindow(This->XD,This->handle);

  free_res(This->XD);

  This->handle=0;
  This->XD=NULL;
}
//---------------------------------------------------------------------------
int hxc_dropdown::WinProc(hxc_dropdown *This,Window Win,XEvent *Ev)
{
  if (This->XD==NULL) return 0;
	This->common_winproc(Ev);

//  int mx,my;
//  bool leftbuttondown;
//  bool Press=0;
//  Time event_time;

  switch(Ev->type){
    case FocusIn:
      This->draw(false);
      break;
    case FocusOut:
      This->draw(false);
      break;
/*
    case MotionNotify:
      mx=Ev->xmotion.x-(This->border);
      my=Ev->xmotion.y-(This->border);
      event_time=Ev->xmotion.time;
      leftbuttondown=(Ev->xmotion.state & Button1Mask); */
    case ButtonPress:
      if (Ev->xbutton.button==Button1){
        if (This->dropped){
          This->retract();
          XSetInputFocus(This->XD,This->handle,RevertToParent,CurrentTime);
        }else{
          This->drop();
        }
      }
      break;


    case KeyPress:
      This->lv.sel=This->sel;
      This->lv.handle_keypress(This->XD,&(Ev->xkey));
      break;
    case Expose:
      if (Ev->xexpose.count>0) break;
      This->draw();
      XSync(This->XD,False);
      break;
  }
  return 0;
}
//---------------------------------------------------------------------------
int hxc_dropdown::listview_notify_proc(hxc_listview*lv,int mess,int i)
{
  hxc_dropdown*This=(hxc_dropdown*)(lv->owner);
  if (This->dropped==0){
    // Keyboard messages
    if (mess==LVN_SELCHANGE){
      This->changesel(lv->sel);
      This->draw();
      if (This->notifyproc) (This->notifyproc)(This,DDN_SELCHANGE,This->sel);
    }
    return 0;
  }
  if (mess==LVN_RETURN || mess==LVN_CB_RETRACT){
  	if (mess==LVN_CB_RETRACT && i<0){
	    This->retract();
	    XFlush(This->XD);
  		return 0; // cancelled
  	}
    This->changesel(lv->sel);
    This->retract();

    This->draw();
    if (This->notifyproc) (This->notifyproc)(This,DDN_SELCHANGE,This->sel);
    if (This->XD){
      XSetInputFocus(This->XD,This->handle,RevertToParent,CurrentTime);
      XFlush(This->XD);
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
void hxc_dropdown::drop()
{
  lv.itemheight=(font->ascent)+(font->descent)+2; //use the listview's font!

  lv.in_combo=true;
  lv.sel=sel;
  lv.create(XD,handle,0,h,int((dropped_w==0) ? w:dropped_w),
            min((lv.itemheight)*(lv.sl.NumStrings)+(lv.border)*2,dropped_h),
            listview_notify_proc,this);

  XSetInputFocus(XD,lv.handle,RevertToParent,CurrentTime);
  XFlush(XD);
  dropped=true;
  lv.draw(true);
  lv.scrollto(sel*lv.itemheight);
}
//---------------------------------------------------------------------------
void hxc_dropdown::retract()
{
  if (dropped==0) return;

  bool was_focused;
  Window Foc=0;
  int RevertTo;
  XGetInputFocus(XD,&Foc,&RevertTo);
  was_focused=(Foc==lv.handle);
  lv.destroy(&lv);
  if(was_focused){
    XSetInputFocus(XD,handle,RevertToParent,CurrentTime);
  }
  dropped=false;
}
//---------------------------------------------------------------------------

