//---------------------------------------------------------------
hxc_scrollarea::hxc_scrollarea()
{
  can_have_children=true; //loads!
  destroyproc=(LPHXC_DESTROYPROC)destroy;
  deleteproc=(LPHXC_DELETEPROC)delete_hxc_scrollarea;

	ww=100;hh=100;
	sx=0;sy=0;
	hsb.horizontal=true;
	vsb.horizontal=false;
	hsb.owner=this;
	vsb.owner=this;
  notifyproc=NULL;
}
//---------------------------------------------------------------
bool hxc_scrollarea::create(Display*d,Window daddy,int x,int y,int w,int h,LPHXC_SCROLLAREANOTIFYPROC np)
{
  if (XD!=NULL) destroy(this);
  XD=d;
  parent=daddy;
  notifyproc=np;

  load_res(XD);

  XSetWindowAttributes swa;
  swa.backing_store=NotUseful;
  frame=XCreateWindow(XD,parent,x,y,w,h,0,
                           CopyFromParent,InputOutput,CopyFromParent,
                           CWBackingStore,&swa);
	hxc::x=x;hxc::y=y;hxc::w=w;hxc::h=h;
  XSelectInput(XD,frame,StructureNotifyMask|
                            ButtonPressMask|ButtonReleaseMask);

  ww=w;hh=h;

  swa.backing_store=NotUseful;
  swa.background_pixel=col_bk;
  handle=XCreateWindow(XD,frame,0,0,w,h,0,
                           CopyFromParent,InputOutput,CopyFromParent,
                           CWBackingStore|CWBackPixel,&swa);

  SetProp(XD,handle,cWinProc,(DWORD)surface_WinProc);
  SetProp(XD,handle,cWinThis,(DWORD)this);
  SetProp(XD,frame,cWinProc,(DWORD)frame_WinProc);
  SetProp(XD,frame,cWinThis,(DWORD)this);

  XSelectInput(XD,handle,KeyPressMask|KeyReleaseMask|
                            ButtonPressMask|ButtonReleaseMask|
                            Button1MotionMask|
                            ExposureMask);

  XMapWindow(XD,handle);
  XMapWindow(XD,frame);

	hscroll=false;
	vscroll=false;
  return false;
}

void hxc_scrollarea::rangecheck(){
	int fw=w;if(vscroll)fw-=HXC_SCROLLBAR_WIDTH;
	int fh=h;if(hscroll)fh-=HXC_SCROLLBAR_WIDTH;
	if(sx>ww-fw)sx=ww-fw;
	if(sy>hh-fh)sy=hh-fh;
	if(sx<0)sx=0;
	if(sy<0)sy=0;
}

void hxc_scrollarea::adjust()
{
	bool old_hscroll=hscroll;
	bool old_vscroll=vscroll;
/*	if(hscroll && vscroll){
		if(cornerbox){
		  XDestroyWindow(XD,cornerbox);
		  cornerbox=0;
		}
	} */
//	if(hscroll)hsb.destroy(&hsb);
//	if(vscroll)vsb.destroy(&vsb); //destroy and recreate so that they come to the top
	ww=w-HXC_SCROLLBAR_WIDTH;hh=h-HXC_SCROLLBAR_WIDTH;
	hscroll=false;vscroll=false;
	int hxcw,hxch;
	int x1,y1,w1,h1;
	int fw=w,fh=h;
  hxc*lphxc=first_hxc;
  while(lphxc!=NULL){
    if(lphxc->XD){
      if(lphxc->parent==handle){
      	hxcw=(lphxc->x)+(lphxc->w)+5;
      	hxch=(lphxc->y)+(lphxc->h)+5;
      	if(hxcw>ww){
      		ww=hxcw;
      	}
      	if(hxch>hh){
      		hh=hxch;
      	}
      }
    }
    lphxc=lphxc->next_hxc;
  }
  if(ww>fw){
  	hscroll=true;
  	fh-=HXC_SCROLLBAR_WIDTH;
  }
  if(hh>fh){
  	vscroll=true;
  	fw-=HXC_SCROLLBAR_WIDTH;
  }
  if((ww>fw) && (!hscroll)){
  	hscroll=true;
  	fh-=HXC_SCROLLBAR_WIDTH;
  }
  XWindowAttributes wa;
  XGetWindowAttributes(XD,frame,&wa);

  if(fw!=wa.width || fh!=wa.height){
  	XResizeWindow(XD,frame,fw,fh);
  }
	if (sx>ww-fw) sx=ww-fw;
	if (sy>hh-fh) sy=hh-fh;
  if (sx<0) sx=0;
  if (sy<0) sy=0;

	if(ww<fw)ww=fw;
	if(hh<fh)hh=fh;
	if(hscroll){
		x1=1;y1=h-HXC_SCROLLBAR_WIDTH-2;
		w1=fw-2;h1=HXC_SCROLLBAR_WIDTH;
//		if(vscroll)w1-=HXC_SCROLLBAR_WIDTH;
		hsb.init(ww,fw,sx);
		if(!old_hscroll)hsb.create(XD,parent,x1,y1,w1,h1,
                      scrollbar_notify_proc,this);
    else{
    	XMoveResizeWindow(XD,hsb.handle,x1,y1,w1,h1);
//    	vsb.draw();
    }
	}else if(old_hscroll){
		hsb.destroy(&hsb);
	}
	if(vscroll){
		x1=w-HXC_SCROLLBAR_WIDTH+1;y1=1;
		w1=HXC_SCROLLBAR_WIDTH;h1=fh-2;
//		if(hscroll)h1-=HXC_SCROLLBAR_WIDTH;
		vsb.init(hh,fh,sy);
		if(!old_vscroll)vsb.create(XD,parent,x1,y1,w1,h1,
                      scrollbar_notify_proc,this);
    else{
    	XMoveResizeWindow(XD,vsb.handle,x1,y1,w1,h1);
//    	vsb.draw();
    }
	}else if(old_vscroll){
		vsb.destroy(&vsb);
	}
/*	
	if(hscroll && vscroll){
    XSetWindowAttributes swa;
    swa.backing_store=NotUseful;
    swa.background_pixel=col_bk;
    cornerbox=XCreateWindow(XD,frame,w-HXC_SCROLLBAR_WIDTH,h-HXC_SCROLLBAR_WIDTH,
    													HXC_SCROLLBAR_WIDTH,HXC_SCROLLBAR_WIDTH,0,
                             CopyFromParent,InputOutput,CopyFromParent,
                             CWBackingStore|CWBackPixel,&swa);
  	SetProp(XD,cornerbox,cWinProc,(DWORD)0);
	  SetProp(XD,cornerbox,cWinThis,(DWORD)this);
	} */

	XMoveResizeWindow(XD,handle,-sx,-sy,ww,hh);
}

void hxc_scrollarea::destroy(hxc_scrollarea*This)
{
  if (This->XD==NULL) return;

  RemoveProp(This->XD,This->handle,cWinProc);
  RemoveProp(This->XD,This->handle,cWinThis);
  XDestroyWindow(This->XD,This->handle);
  RemoveProp(This->XD,This->frame,cWinProc);
  RemoveProp(This->XD,This->frame,cWinThis);
  XDestroyWindow(This->XD,This->frame);
/*	if(This->cornerbox){
	  XDestroyWindow(This->XD,This->cornerbox);
	} */

  free_res(This->XD);

  This->handle=0;
  This->frame=0;
  This->XD=NULL;
}


int hxc_scrollarea::scrollbar_notify_proc(hxc_scrollbar *sb,int mess,int i)
{
	hxc_scrollarea*This=(hxc_scrollarea*)(sb->owner);
	if(sb->horizontal){
		if(mess==SBN_SCROLL){
			This->scrollto(i,This->sy);
		}else if(mess==SBN_SCROLLBYONE){
			This->scrollto(This->sx+(i*10),This->sy);
		}
	}else{
		if(mess==SBN_SCROLL){
			This->scrollto(This->sx,i);
		}else if(mess==SBN_SCROLLBYONE){
			This->scrollto(This->sx,This->sy+(i*10));
		}
	}
  return 0;
}

void hxc_scrollarea::scrollto(int new_sx,int new_sy)
{
	bool refresh=false;
	if (new_sx!=sx){
		sx=new_sx;
	 	rangecheck();
		if (hscroll){
		 	hsb.pos=sx;
		 	hsb.draw();
		}
		refresh=true;
	}
	if (new_sy!=sy){
		sy=new_sy;
	 	rangecheck();
		if (vscroll){
		 	vsb.pos=sy;
		 	vsb.draw();
		}
		refresh=true;
	}
 	if (refresh) XMoveWindow(XD,handle,-(sx),-(sy));
}

void hxc_scrollarea::resize(int wid,int hi){
	if(w!=wid || h!=hi){
		w=wid;h=hi;
		adjust();
	}
}

int hxc_scrollarea::surface_WinProc(hxc_scrollarea *This,Window Win,XEvent *Ev)
{
//	This->common_winproc(Ev);
  switch(Ev->type){
  case MotionNotify:
    break;
  case ButtonRelease:
    break;
  case ButtonPress:
    switch (Ev->xbutton.button){
      case Button4:
        This->scrollto(This->sx,This->sy-This->vsb.viewrange);
        return 0;
      case Button5:
        This->scrollto(This->sx,This->sy+This->vsb.viewrange);
        return 0;
    }
    if (This->notifyproc) This->notifyproc(This,SAN_CLICK,Ev->xbutton.button);
    break;
  case Expose:
    break;
  case ConfigureNotify:
  	{
  		XWindowAttributes wa;
  	  XGetWindowAttributes(This->XD,This->handle,&wa);
  	  int old_ww=This->ww;
  	  int old_hh=This->hh;
  	  This->ww=wa.width;
  	  This->hh=wa.height;
  	  if(old_ww!=(This->ww) || (old_hh!=(This->hh))){
			 	This->adjust();
			}
      break;
    }
  }
  return 0;
}

int hxc_scrollarea::frame_WinProc(hxc_scrollarea *This,Window Win,XEvent *Ev)
{
  switch(Ev->type){
  case MotionNotify:
    break;
  case ButtonRelease:
    break;
  case ButtonPress:
    if (This->notifyproc) This->notifyproc(This,SAN_CLICK,Ev->xbutton.button);
    break;
  case Expose:
    break;
  case ConfigureNotify:
  	{
  		XWindowAttributes wa;
  	  XGetWindowAttributes(This->XD,This->frame,&wa);
  	  int old_w=This->w;
  	  int old_h=This->h;
  	  This->x=wa.x;
  	  This->y=wa.y;
  	  This->w=wa.width;
  	  This->h=wa.height;
  	  if(This->hscroll)(This->h)+=HXC_SCROLLBAR_WIDTH;
  	  if(This->vscroll)(This->w)+=HXC_SCROLLBAR_WIDTH;
  	  if(old_w!=(This->w) || (old_h!=(This->h))){
			 	This->adjust();
			}
      break;
    }
  }
  return 0;
}

