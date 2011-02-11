//---------------------------------------------------------------
hxc_scrollbar::hxc_scrollbar()
{
  can_have_children=true; //the buttons!
  destroyproc=(LPHXC_DESTROYPROC)destroy;
  deleteproc=(LPHXC_DELETEPROC)delete_hxc_scrollbar;

  horizontal=false;
  range=1000;pos=0;viewrange=10;
  dragstage=0;
  ty=0;th=20;
  arrowheight=HXC_SCROLLBAR_WIDTH;
}
//---------------------------------------------------------------------------
bool hxc_scrollbar::create(Display*d,Window daddy,int x,int y,
                      int w,int h,LPHXC_SCROLLBARNOTIFYPROC pass_notifyproc){
  if (XD!=NULL) destroy(this);
  XD=d;
  parent=daddy;
  notifyproc=pass_notifyproc;

  load_res(XD);

  XSetWindowAttributes swa;
  swa.backing_store=NotUseful;
  swa.background_pixel=col_border_light;
  handle=XCreateWindow(XD,parent,x,y,w,h,0,
                           CopyFromParent,InputOutput,CopyFromParent,
                           CWBackingStore|CWBackPixel,&swa);
	hxc::x=x;hxc::y=y;hxc::w=w;hxc::h=h;

  SetProp(XD,handle,cWinProc,(DWORD)WinProc);
  SetProp(XD,handle,cWinThis,(DWORD)this);

  XSelectInput(XD,handle,KeyPressMask|KeyReleaseMask|
                            ButtonPressMask|ButtonReleaseMask|
                            Button1MotionMask|Button2MotionMask|
                            ExposureMask|StructureNotifyMask);
	if(horizontal){
    UpBut.create(XD,handle,0,0,arrowheight,h,button_notify_proc,this,BT_ICON | BT_HOLDREPEAT,"",-1,col_grey);
    DownBut.create(XD,handle,w-arrowheight,0,arrowheight,h,button_notify_proc,this,BT_ICON | BT_HOLDREPEAT,"",1,col_grey);
    UpBut.set_icon(NULL,2);
    DownBut.set_icon(NULL,3);
	}else{
    UpBut.create(XD,handle,0,0,w,arrowheight,button_notify_proc,this,BT_ICON | BT_HOLDREPEAT,"",-1,col_grey);
    DownBut.create(XD,handle,0,h-arrowheight,w,arrowheight,button_notify_proc,this,BT_ICON | BT_HOLDREPEAT,"",1,col_grey);
    UpBut.set_icon(NULL,0);
    DownBut.set_icon(NULL,1);
  }

  XSetForeground(XD,gc,col_black);
  XMapWindow(XD,handle);
  return false;
}

void hxc_scrollbar::rangecheck(){
  if(range<=0)range=1;
  if(viewrange<=0)viewrange=1;
  if(viewrange>range)viewrange=range;
  if(pos<0)pos=0;
  if(pos>range-viewrange)pos=range-viewrange;
}

void hxc_scrollbar::init(int _range,int _viewrange,int _pos)
{
  range=_range;viewrange=_viewrange;pos=_pos;
  rangecheck();
}

void hxc_scrollbar::draw()
{
  if (XD==NULL) return;

  rangecheck();
  XWindowAttributes wa;
  XGetWindowAttributes(XD,handle,&wa);
  int h,w,hr,rr;
	if(horizontal){
  	h=wa.width-(2*arrowheight),w=wa.height;
	}else{
  	h=wa.height-(2*arrowheight);w=wa.width;
	}
  if(h<10)h=10;
  th=(viewrange*h)/range;
  if (th<w-2){
    th=w-2;
  }
  hr=h-th;if(hr<0)hr=0;
  rr=range-viewrange;if(rr<1)rr=1;
  ty=(pos*hr)/rr;
  if (ty+th>=h) ty=h-th;
  XClearWindow(XD,handle);
  XSetForeground(XD,gc,col_grey);
  DWORD col_top=col_border_light,col_bottom=col_border_dark;
  if (dragstage){
    col_top=col_border_dark,col_bottom=col_border_light;
  }
  int x1=0,y1=ty+arrowheight,w1=w,h1=th;
  if(horizontal){
  	int buf=x1;x1=y1;y1=buf;
  	buf=w1;w1=h1;h1=buf;
  }
  XFillRectangle(XD, handle, gc, x1+1,y1+1,w1-2,h1-2);
  draw_border(XD,handle,gc,x1,y1,w1,h1,0,col_top,col_bottom);
}

void hxc_scrollbar::destroy(hxc_scrollbar *This)
{
  if (This->XD==NULL) return;

  This->UpBut.destroy(&(This->UpBut));
  This->DownBut.destroy(&(This->DownBut));
  RemoveProp(This->XD,This->handle,cWinProc);
  RemoveProp(This->XD,This->handle,cWinThis);
  XDestroyWindow(This->XD,This->handle);

  free_res(This->XD);

  This->handle=0;
  This->XD=NULL;
}

void hxc_scrollbar::notify_reposition(int newpos)
{
  if (newpos<0) newpos=0;
  if (newpos>range-viewrange) newpos=max(range-viewrange,0);
  if (newpos!=pos){
    if (notifyproc) notifyproc(this,SBN_SCROLL,newpos);
  }
}

int hxc_scrollbar::button_notify_proc(hxc_button *But,int Mess,int *Inf)
{
  if (Mess==BN_CLICKED){
    hxc_scrollbar *This=(hxc_scrollbar*)GetProp(But->XD,But->parent,cWinThis);
    if (This->notifyproc) (This->notifyproc)(This,SBN_SCROLLBYONE,But->id);
  }
  return 0;
}


void hxc_scrollbar::do_drag(int y,bool by_top){ //pass the y-position of the top of the scrolling box, or the middle
  XWindowAttributes wa;
  XGetWindowAttributes(XD,handle,&wa);
  int h,w;
	if(horizontal){
		w=wa.height-2;
		h=wa.width-(2*(arrowheight));
	}else{
		w=wa.width-2;
		h=wa.height-(2*(arrowheight));
	}

  int th=((viewrange)*h)/(range);
  if (th<w){
    th=w;
  }
  h-=th;

  if(h<10)h=10;
//  int y=(my-(This->arrowheight))-(This->drag_y);
  if(!by_top)y-=th/2;
  int rr=(range)-(viewrange);if(rr<1)rr=1;

  pos=(y*rr)/h;
  rangecheck();
  if (notifyproc) notifyproc(this,SBN_SCROLL,pos);
}

int hxc_scrollbar::WinProc(hxc_scrollbar *This,Window Win,XEvent *Ev)
{
	int mpos;
	This->common_winproc(Ev);
  switch(Ev->type){
  case MotionNotify:
  	if(This->horizontal){
	  	mpos=Ev->xmotion.x;
	  }else{
	  	mpos=Ev->xmotion.y;
	  }
  	
    if((Ev->xmotion.state) & Button1Mask){
      if(This->dragstage==1){
        (This->do_drag)(mpos-(This->arrowheight)-(This->drag_y),true);
      }
    }else if((Ev->xmotion.state) & Button2Mask){
      (This->do_drag)(mpos-(This->arrowheight),false); //set middle where clicked, if possible
    }
    break;
  case ButtonRelease:
    if (Ev->xbutton.button==Button4 || Ev->xbutton.button==Button5) break; // No wheel
    if(This->dragstage){
      This->dragstage=0;
      This->draw();
    }
    break;
  case ButtonPress:
    if (Ev->xbutton.button==Button1){
      XWindowAttributes wa;
      XGetWindowAttributes(This->XD,This->handle,&wa);
      int h;
      int mx,my;
      if (This->horizontal){
        mx=Ev->xbutton.y;
        my=Ev->xbutton.x;
        h=wa.width;
      }else{
        mx=Ev->xbutton.x;
        my=Ev->xbutton.y;
        h=wa.height;
      }

      if (my<This->arrowheight){  //scrolled one up
        if (This->notifyproc) (This->notifyproc)(This,SBN_SCROLLBYONE,-1);
      }else if (my>h-(This->arrowheight)){  //scroll one down
        if (This->notifyproc) (This->notifyproc)(This,SBN_SCROLLBYONE,1);
      }else if (my<(This->arrowheight)+(This->ty)){  //page up
        if (This->notifyproc) (This->notify_reposition)((This->pos)-(This->viewrange));
      }else if (my>(This->arrowheight)+(This->ty)+(This->th)){ //page down
        if (This->notifyproc) (This->notify_reposition)((This->pos)+(This->viewrange));
      }else{   //start drag
        This->dragstage=1;
        This->drag_y=my-((This->arrowheight)+(This->ty));
        This->draw();
      }
      break;
    }else if (Ev->xbutton.button==Button2){ //MMB
    	if(This->horizontal){
  	  	mpos=Ev->xmotion.x;
  	  }else{
  	  	mpos=Ev->xmotion.y;
  	  }
      (This->do_drag)(mpos-(This->arrowheight),false); //set middle where clicked, if possible
    }else if (Ev->xbutton.button==Button4){  //wheel up
      if (This->notifyproc) (This->notify_reposition)((This->pos)-(This->viewrange));
    }else if (Ev->xbutton.button==Button5){ //wheel down
      if (This->notifyproc) (This->notify_reposition)((This->pos)+(This->viewrange));
    }

    break;
  case Expose:
    if (Ev->xexpose.count>0) break;
    This->draw();
    XSync(This->XD,0);
    break;
  case ConfigureNotify:
    XWindowAttributes wa,but_wa;
    XGetWindowAttributes(This->XD,Win,&wa);
    XGetWindowAttributes(This->XD,This->DownBut.handle,&but_wa);
    if(This->horizontal){
    	XMoveWindow(This->XD,This->DownBut.handle,wa.width-but_wa.height,0);
    }else{
    	XMoveWindow(This->XD,This->DownBut.handle,0,wa.height-but_wa.height);
    }
    XSync(This->XD,0);
    break;
  }
  return 0;
}

