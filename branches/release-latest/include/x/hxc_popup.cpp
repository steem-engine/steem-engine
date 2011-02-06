#ifndef HXC_POPUP_CPP
#define HXC_POPUP_CPP

#include "hxc_popup.h"
//---------------------------------------------------------------------------
hxc_popup::hxc_popup()
{
  menu.Sort=eslNoSort;
  itemheight=0;
  border=1;
  lpig=NULL;
  handle=0;
  XD=NULL;
  notifyproc=NULL;
}
//---------------------------------------------------------------------------
bool hxc_popup::create(Display *d,Window daddy,int _x,int _y,HXC_POPUPNOTIFYPROC np,void *o)
{
  if (handle) close(true);

  XD=d;
	notifyproc=np;
	owner=o;

  sy=0;
  sel=-1;
  clicked_in=0;

  hxc::load_res(XD);

  if (itemheight==0){
    itemheight=hxc::font->ascent+hxc::font->descent+2;
    if (lpig) itemheight=max(itemheight,lpig->IconHeight+2);
  }
  w=0;
  int iw=0;
  if (lpig) iw=lpig->IconWidth+2;
  for (int n=0;n<menu.NumStrings;n++){
    w=max(border+6+iw+(int)hxc::get_text_width(XD,menu[n].String)+6+border,w);
  }
  h=menu.NumStrings*itemheight+border*2;

  int move_h=0;
  if (daddy==0){
    if (_x==POP_CURSORPOS){
      Window in_win,in_child;
      int rootx,rooty;
      x=XDisplayWidth(XD,XDefaultScreen(XD))/2;
      y=XDisplayWidth(XD,XDefaultScreen(XD))/2;
      UINT mask;
      XQueryPointer(XD,XDefaultRootWindow(XD),&in_win,&in_child,
                        &rootx,&rooty,&x,&y,&mask);
    }else{
      x=_x,y=_y;
    }
  }else{
    Window child;
    XTranslateCoordinates(XD,daddy,XDefaultRootWindow(XD),_x,_y,&x,&y,&child);

    XWindowAttributes wa;
    XGetWindowAttributes(XD,daddy,&wa);
    move_h=wa.height;
  }

  int sw=XDisplayWidth(XD,XDefaultScreen(XD));
  int sh=XDisplayHeight(XD,XDefaultScreen(XD));
  if (move_h>=sh) move_h=0;

  if (x+w>sw) x=sw-w;
	if (y+h>sh){
    if (y-h-move_h>=0){
      y-=h+move_h;
    }else{
      y-=(y+h)-sh;
    }
  }

  XSetWindowAttributes swa;
  swa.backing_store=NotUseful;
  swa.override_redirect=True;
  swa.cursor=hxc::arrow_cursor;
  handle=XCreateWindow(XD,XDefaultRootWindow(XD),x,y,w,h,0,
                           CopyFromParent,InputOutput,CopyFromParent,
                           CWBackingStore | CWOverrideRedirect | CWCursor,
                           &swa);

  SetProp(XD,handle,cWinProc,(DWORD)WinProc);
  SetProp(XD,handle,cWinThis,(DWORD)this);
  SetProp(XD,handle,hxc::cModal,(DWORD)0xffffffff);

  XSelectInput(XD,handle,KeyPressMask | KeyReleaseMask |
                            ButtonPressMask | ButtonReleaseMask |
                            ExposureMask | FocusChangeMask |
                            PointerMotionMask | LeaveWindowMask);

  XMapWindow(XD,handle);

  XGrabPointer(XD,handle,False,PointerMotionMask | ButtonPressMask |
          			ButtonReleaseMask,GrabModeAsync,GrabModeAsync,None,
           			None,CurrentTime);
              
  hxc::popup_active=handle;

  return false;
}
//---------------------------------------------------------------------------
void hxc_popup::draw(int i)
{
  int y=border,bh=h-border*2;
  int first=0,last=menu.NumStrings-1;
  if (i>=0){
    y+=itemheight*i;
    bh=itemheight;
    first=i;
    last=i;
  }

  XSetForeground(XD,hxc::gc,hxc::col_bk);
  XFillRectangle(XD,handle,hxc::gc,border,y,w-border*2,bh);
  XSetForeground(XD,hxc::gc,hxc::col_black);
	int text_off=(itemheight - hxc::font->descent + hxc::font->ascent)/2;
  for (int n=first;n<=last;n++){
    bool line=IsSameStr(menu[n].String,"-");
  	if (sel==n && line==0){
      XSetForeground(XD,hxc::gc,hxc::col_sel_back);
      XFillRectangle(XD,handle,hxc::gc,border,y,w-border*2,itemheight);
      XSetForeground(XD,hxc::gc,hxc::col_sel_fore);
  	}
    if (line){
      XDrawLine(XD,handle,hxc::gc,border+2,y+border+itemheight/2,w-1-2-border,y+border+itemheight/2);
    }else{
      int x=border+4;
      if (lpig){
        lpig->DrawIcon(menu[n].Data[0],handle,hxc::gc,x,y+(itemheight-lpig->IconHeight)/2);
        x+=lpig->IconWidth + 5;
      }
      XDrawString(XD,handle,hxc::gc,x,y+text_off,menu[n].String,strlen(menu[n].String));
    }
    y+=itemheight;
    XSetForeground(XD,hxc::gc,hxc::col_black);
  }
  if (i==-1) hxc::draw_border(XD,handle,hxc::gc,0,0,w,h,border,hxc::col_black,hxc::col_black);
}
//---------------------------------------------------------------------------
void hxc_popup::close(bool suppress_notify)
{
  if (XD==NULL) return;

  XUngrabPointer(XD,CurrentTime);
  RemoveProp(XD,handle,hxc::cModal);
  RemoveProp(XD,handle,cWinProc);
  RemoveProp(XD,handle,cWinThis);
  if (hxc::popup_active==handle) hxc::popup_active=None;
  XDestroyWindow(XD,handle);

  hxc::free_res(XD);

  handle=0;
  XD=NULL;
  itemheight=0;
  lpig=NULL;

  if (notifyproc && suppress_notify==0) notifyproc(this,POP_CANCEL,-1);
}
//---------------------------------------------------------------------------
int hxc_popup::WinProc(hxc_popup *This,Window Win,XEvent *Ev)
{
  if (This->XD==NULL) return 0;

  int mx=0,my=0;
  unsigned int buttonmask=0;
  bool Press=0;

  switch (Ev->type){
    case LeaveNotify:
    {
      int old_sel=This->sel;
      This->sel=-1;
      if (old_sel!=-1) This->draw(old_sel);
      break;
    }
    case MotionNotify:
      mx=Ev->xmotion.x-(This->border);
      my=Ev->xmotion.y-(This->border);
      buttonmask=(Ev->xmotion.state & Button123Mask);
    case ButtonPress:
    {
      if (Ev->type==ButtonPress){
        mx=Ev->xbutton.x-(This->border);
        my=Ev->xbutton.y-(This->border);
        if (Ev->xbutton.button==Button1) buttonmask|=Button1Mask;
        if (Ev->xbutton.button==Button2) buttonmask|=Button2Mask;
        if (Ev->xbutton.button==Button3) buttonmask|=Button3Mask;
        Press=true;
      }
      bool mouse_in=(mx>=0 && mx<This->w - This->border*2 &&
                      my>=0 && my<This->h - This->border*2);
      if (Press && buttonmask){
        if (mouse_in){
          This->clicked_in=true;
        }else{
          This->close(0);
        }
      }
      if (This->handle){
        int over=(my + This->sy) / This->itemheight;
        if (mouse_in==0) over=-1;
        if (over!=This->sel && This->XD){
          int old_sel=This->sel;
          This->sel=over;
          if (old_sel!=-1) This->draw(old_sel);
          if (over!=-1) This->draw(over);
        }
      }
      break;
    }
    case ButtonRelease:
    {
      if (Ev->xbutton.button==Button4 || Ev->xbutton.button==Button5) break; // No wheel
      
      mx=Ev->xbutton.x-(This->border);
      my=Ev->xbutton.y-(This->border);
      bool mouse_in=(mx>=0 && mx<This->w - This->border*2 &&
                      my>=0 && my<This->h - This->border*2);
      bool mouse_in_window=(mx>=-This->border && mx<=This->w - This->border &&
                      my>=-This->border && my<=This->h - This->border);
      if (mouse_in && This->clicked_in){
        int i=(my + This->sy) / This->itemheight;
        This->close(true);
        if (This->notifyproc) (This->notifyproc)(This,POP_CHOOSE,i);
      }else if (mouse_in_window==0){
        This->close(0);
      }
      This->clicked_in=0;
      break;
    }
    case Expose:
      This->draw();
      break;
  }
  return 0;
}
//---------------------------------------------------------------------------
#endif

