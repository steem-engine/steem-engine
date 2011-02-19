#ifndef _HXC_CONTROLS_CPP
#define _HXC_CONTROLS_CPP

#include "hxc.h"
#include "icongroup.cpp"
#include <dynamicarray.cpp>
#include <easystr.cpp>
#include <easystringlist.cpp>
#include <mymisc.cpp>

#ifndef T
#define T(s) s
#endif

int hxc::res_users=0;
XContext hxc::cModal=0;
XFontStruct* hxc::font=NULL;
DWORD hxc::col_grey,hxc::col_black,hxc::col_white,hxc::col_border_dark,hxc::col_border_light,
        hxc::col_bk,hxc::col_sel_back,hxc::col_sel_fore;
GC hxc::gc;
EasyStringList hxc::font_sl;
void (*hxc::alloc_colours_vector)(Display*)=hxc::alloc_colours;
void (*hxc::free_colours_vector)(Display*)=hxc::free_colours;
void (*hxc::modal_notifyproc)(bool)=NULL;
Colormap hxc::colormap=0;
Cursor hxc::arrow_cursor=0;
Atom hxc::XA_WM_PROTOCOLS,hxc::XA_WM_DELETE_WINDOW;
int hxc::modal_result;
Window hxc::modal_focus_win=0;
Window hxc::popup_active=None;

hxc *hxc::first_hxc=NULL,*hxc::last_hxc=NULL;
hxc_timer *hxc::first_timer=NULL,*hxc::last_timer=NULL;

void hxc::set_timer(Window win,int id,int interval,LPHXC_TIMERPROC np,void *data)
{
  kill_timer(win,id);
  
  hxc_timer *tp=new hxc_timer;
  tp->time=timeGetTime()+interval;
  tp->win=win;
  tp->id=id;
  tp->interval=interval;
  tp->notifyproc=np;
  tp->data=data;
  
  if (first_timer==NULL) first_timer=tp;
  tp->prev=last_timer;
  if (tp->prev) tp->prev->next=tp;
  last_timer=tp;
  tp->next=NULL;
}

void hxc::kill_hxc_timer(hxc_timer *tp)
{
  if (tp->prev) tp->prev->next=tp->next;
  if (tp->next) tp->next->prev=tp->prev;
  if (last_timer==tp) last_timer=tp->prev;
  if (first_timer==tp) first_timer=tp->next;
  delete tp;
}

void hxc::kill_timer(Window win,int id)
{
  hxc_timer *tp=first_timer,*next_tp;
  while (tp){
    next_tp=tp->next;
    if (tp->win==win || win==HXC_TIMER_ALL_WINS){
      if (tp->id==id || id==HXC_TIMER_ALL_IDS) kill_hxc_timer(tp);
    }
    tp=next_tp;
  }
}

void hxc::kill_all_timers()
{
  hxc::kill_timer(HXC_TIMER_ALL_WINS,HXC_TIMER_ALL_IDS);
}

void hxc::check_timers()
{
  hxc_timer *tp=first_timer,*next_tp;
  DWORD t=timeGetTime();
  while (tp){
    next_tp=tp->next;
    if (t>=tp->time){
      tp->time+=tp->interval;
      if (tp->notifyproc(tp->data,tp->win,tp->id)==HXC_TIMER_STOP){
        kill_hxc_timer(tp);
      }
      t=timeGetTime();
      next_tp=first_timer; // Just in case lots of timers were deleted
    }
    tp=next_tp;
  }
}

bool hxc::wait_for_event(Display *XD,XEvent *pev,DWORD return_time)
{
  if (return_time) return_time+=timeGetTime();
  LOOP{
    check_timers();
    
    if (XPending(XD)){
      XNextEvent(XD,pev);
      return true;
    }
    if (return_time){
      if (timeGetTime()>=return_time) return 0;
    }

    fd_set in_set;
    struct timeval wait_for;
    wait_for.tv_sec=0;
    wait_for.tv_usec=20*1000; // 1000=1ms
    FD_ZERO(&in_set);
    FD_SET(ConnectionNumber(XD),&in_set);
    select(ConnectionNumber(XD)+1,&in_set,NULL,NULL,&wait_for);
  }
}
  
int hxc::get_text_width(Display *XD,char *t)
{
  load_res(XD);
  int w=XTextWidth(hxc::font,t,strlen(t));
  free_res(XD);
  return w;
}

void hxc::alloc_colours(Display *XD)
{
  col_black=BlackPixel(XD,DefaultScreen(XD));
  col_white=WhitePixel(XD,DefaultScreen(XD));
  col_grey=GetColourValue(XD,192<<8,192<<8,192<<8,col_white);
  col_border_dark=GetColourValue(XD,128<<8,128<<8,128<<8,col_black);
  col_border_light=GetColourValue(XD,224<<8,224<<8,224<<8,col_white);
  col_sel_back=GetColourValue(XD,0<<8,0<<8,192<<8,col_black);
  col_sel_fore=GetColourValue(XD,255<<8,255<<8,255<<8,col_white);
  col_bk=GetColourValue(XD,192<<8,192<<8,192<<8,col_grey);
}

void hxc::load_res(Display*XD)
{
  if (res_users==0){
    alloc_colours_vector(XD);

    gc=XCreateGC(XD,XDefaultRootWindow(XD),0,NULL);

    font=NULL;
	for (int f=0;f<font_sl.NumStrings;f++){
	  font=XLoadQueryFont(XD,font_sl[f].String);
	  if (font) break;
	}
    if (font==NULL) font=XLoadQueryFont(XD,"8x13");
    if (font) XSetFont(XD,gc,font->fid);
    cModal=XUniqueContext();
  	arrow_cursor=XCreateFontCursor(XD,XC_arrow);
    XA_WM_PROTOCOLS=XInternAtom(XD,"WM_PROTOCOLS",0);
    XA_WM_DELETE_WINDOW=XInternAtom(XD,"WM_DELETE_WINDOW",0);
  }
  res_users++;
}

void hxc::free_colours(Display*XD){
  DWORD ColList[6]={    col_grey,    col_border_dark,    col_border_light,
                      col_sel_back,    col_sel_fore,    col_bk};
  XFreeColors(XD,XDefaultColormap(XD,XDefaultScreen(XD)),
                ColList,6,0);
}

void hxc::free_res(Display* XD){
  if(res_users==1){
    if(XD){
      if (font){
        XFreeFont(XD,font);font=NULL;
      }
      XFreeGC(XD,gc);
      XFreeCursor(XD,arrow_cursor);
      free_colours_vector(XD);
      XSync(XD,False);
    }
    res_users=0;
  }else if(res_users>1){
    res_users--;
  }
}

hxc* hxc::find(Window par,int id)
{
  hxc *lphxc=hxc::first_hxc;
  while (lphxc){
    if (lphxc->XD){
      if (lphxc->parent==par && lphxc->id==id){
        return lphxc;
      }
    }
    lphxc=lphxc->next_hxc;
  }
  return NULL;
}

void hxc::destroy_children_of(Window h)
{
  hxc *lphxc=hxc::first_hxc,*next;
  while (lphxc){
    next=lphxc->next_hxc;
    if (lphxc->XD){
      if (lphxc->parent==h){
        if (lphxc->can_have_children){
          hxc::destroy_children_of(lphxc->handle);
        }
        hxc::destroy(lphxc);
        // lphxc might have been deleted now!
      }
    }
    lphxc=next;
  }
}

void hxc::destroy(hxc *lphxc)
{
  lphxc->destroyproc(lphxc);
  if (lphxc->dynamically_allocated){
    // Have to cast the pointer to the correct type
    // before deleting (so destructors are called properly),
    // but we don't know what type it may be here. The
    // solution is a pointer to a static procedure that
    // performs the cast, there is one of these for every type.
    // Could probably do this by overloading new/delete
    // but I wasn't sure about how well that would work.
    LPHXC_DELETEPROC delete_procedure=lphxc->deleteproc;
    delete_procedure(lphxc);
  }
}

void hxc::modal_children(Display*XD,Window hnd,Window modal_win)
{
  hxc *lphxc=first_hxc;
  while (lphxc!=NULL){
    if (lphxc->XD){
      if (lphxc->parent==hnd){
        if (lphxc->can_have_children){
          hxc::modal_children(XD,lphxc->handle,modal_win);
        }
        if (modal_win){
        	SetProp(XD,lphxc->handle,cModal,modal_win);
        }else{
					RemoveProp(XD,lphxc->handle,cModal);
        }
      }
    }
    lphxc=lphxc->next_hxc;
  }
}

bool hxc::suppress_mess_for_modal(Display*XD,XEvent *ev)
{
//return true if the event should be passed to disabled windows
	switch(ev->type){
	case KeyPress: //case KeyRelease:
	case ButtonPress:case ButtonRelease:case MotionNotify:case EnterNotify:
	case FocusIn:
		return true;
	case ClientMessage:
    if (ev->xclient.message_type==XA_WM_PROTOCOLS){
      if ((Atom)(ev->xclient.data.l[0])==XA_WM_DELETE_WINDOW){
				return true;
      }
    }
    break;
	}
	return false;
	
/*
Keyboard events          KeyPress, KeyRelease
Pointer events           ButtonPress, ButtonRelease, Motion-
                         Notify
Window crossing events   EnterNotify, LeaveNotify
Input focus events       FocusIn, FocusOut
Keymap state notifica-   KeymapNotify
tion event
Exposure events          Expose, GraphicsExpose, NoExpose
Structure control        CirculateRequest, ConfigureRequest,
events                   MapRequest, ResizeRequest
Window state notifica-   CirculateNotify, ConfigureNotify,
tion events              CreateNotify, DestroyNotify,
                         GravityNotify, MapNotify,
                         MappingNotify, ReparentNotify,
                         UnmapNotify, VisibilityNotify
Colormap state notifi-   ColormapNotify
cation event
Client communication     ClientMessage, PropertyNotify,
events                   SelectionClear, SelectionNotify,
                         SelectionRequest
-------------------------------------------------------------
*/	
}

bool hxc::SetProp(Display *XD,Window Win,XContext Prop,DWORD Val)
{
  if (XD==NULL) return 0;

  return XSaveContext(XD,Win,Prop,(XPointer)Val)==0;
}

DWORD hxc::GetProp(Display *XD,Window Win,XContext Prop)
{
  if (XD==NULL) return 0;

  XPointer Dat=NULL;
  XFindContext(XD,Win,Prop,&Dat);
  return (DWORD)Dat;
}

void hxc::RemoveProp(Display *XD,Window Win,XContext Prop)
{
  if (XD) XDeleteContext(XD,Win,Prop);
}

DWORD hxc::GetColourValue(Display *XD,WORD R,WORD G,WORD B,DWORD Default)
{
  if (XD==NULL) return 0;

  XColor xc={Default,R,G,B};
  XAllocColor(XD,XDefaultColormap(XD,XDefaultScreen(XD)),&xc);
  return xc.pixel;
}

void hxc::draw_triangle(Display*XD,Drawable d,GC gc,int x,int y,int size,int dir){
//dir = 0 up, 1 down, 2 left, 3 right
  if(dir>=2){
    int xv=-1;if(dir&1)xv=1;
    x+=(xv*size)/2;
    for(int n=0;n<size;n++){
      XDrawLine(XD,d,gc,x,y-n,x,y+n);
      x-=xv;
    }
  }else{
    int yv=-1;if(dir)yv=1;
    y+=(yv*size)/2;
    for(int n=0;n<size;n++){
      XDrawLine(XD,d,gc,x-n,y,x+n,y);
      y-=yv;
    }
  }
}


void hxc::draw_border(Display* XD,Drawable handle,GC gc,
                      int x,int y,int w,int h,int border,DWORD col_top,DWORD col_bottom)
{
  for (int n=0;n<max(border,1);n++){
    int l=x+n,r=x+w-1-n;
    int t=y+n,b=y+h-1-n;
    XSetForeground(XD,gc,col_top);
    XDrawLine(XD,handle,gc,l,t,r,t);
    XDrawLine(XD,handle,gc,l,t,l,b);
    XSetForeground(XD,gc,col_bottom);
    XDrawLine(XD,handle,gc,l,b,r,b);
    XDrawLine(XD,handle,gc,r,b,r,t);
  }
}

void hxc::send_event(Display *XD,Window Win,Atom Mess,long d0,long d1,long d2,long d3,long d4)
{
  XEvent Ev;
  Ev.type=ClientMessage;
  Ev.xclient.window=Win;
  Ev.xclient.message_type=Mess;
  Ev.xclient.format=32;
	Ev.xclient.data.l[0]=d0;
	Ev.xclient.data.l[1]=d1;
	Ev.xclient.data.l[2]=d2;
	Ev.xclient.data.l[3]=d3;
	Ev.xclient.data.l[4]=d4;
  XSendEvent(XD,Win,0,0,&Ev);
}

hxc::hxc()
{
  parent=0;handle=0;owner=NULL;
  XD=NULL;
  id=0;
  can_have_children=false;
  dynamically_allocated=0;

  destroyproc=(LPHXC_DESTROYPROC)default_destroy_proc;
  deleteproc=(LPHXC_DELETEPROC)default_delete_proc;
  previous_hxc=last_hxc;
  if(previous_hxc)previous_hxc->next_hxc=this;
  if(first_hxc==NULL)first_hxc=this;
  next_hxc=NULL;
  last_hxc=this;

  if (font_sl.NumStrings==0){
    font_sl.Sort=eslNoSort;
    font_sl.Add("-adobe-helvetica-bold-r-normal-*-14-100-*-*-*-*-iso8859-1");
    font_sl.Add("-b&h-lucida-medium-r-normal-*-*-120-*-*-p-*-iso8859-1");
    font_sl.Add("-urw-palatino-medium-r-normal-*-*-140-*-*-p-*-iso8859-1");
    font_sl.Add("-mdk-helvetica-medium-r-normal-*-*-130-*-*-p-*-tcvn-5712"); // not iso8859-1, accents mangled!
  }
}

hxc::~hxc(){
  if(previous_hxc!=NULL){  //there's another class pointing to this one
    previous_hxc->next_hxc=next_hxc; //close up hole in list
  }
  if(next_hxc!=NULL){        //there's another class pointed to by this one
    next_hxc->previous_hxc=previous_hxc;  //close up hole in list
  }
  if(last_hxc==this){
    last_hxc=previous_hxc;
  }
  if(first_hxc==this){
    first_hxc=next_hxc;
  }
}

void hxc::common_winproc(XEvent*ev)
{
	if (XD && handle && ev){
  	if (ev->type==ConfigureNotify){
  		XWindowAttributes wa;
  	  XGetWindowAttributes(XD,handle,&wa);
  	  x=wa.x;
  	  y=wa.y;
  	  w=wa.width;
  	  h=wa.height;
    }
  }
}
//---------------------------------------------------------------------------
void hxc::clip_to_expose_rect(Display *XD,XExposeEvent *pex,GC gc)
{
  if (gc==0) gc=hxc::gc;

  XRectangle rect;
  rect.x=0;rect.y=0;
  rect.width=pex->width;
  rect.height=pex->height;
  XSetClipRectangles(XD,gc,pex->x,pex->y,
                       &rect,1,Unsorted);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
Window hxc::create_modal_dialog(Display *XD,int w,int h,char *title,bool create_okcancel)
{
  load_res(XD);

  if (create_okcancel) h+=25+10;

  XSetWindowAttributes swa;
  swa.backing_store=NotUseful;
  swa.background_pixel=hxc::col_bk;
	int scr=XDefaultScreen(XD);
  Window handle=XCreateWindow(XD,XDefaultRootWindow(XD),
  												 (XDisplayWidth(XD,scr)-w)/2,(XDisplayHeight(XD,scr)-h)/2,
                           w,h,0,CopyFromParent,InputOutput,CopyFromParent,
                           CWBackingStore | CWBackPixel,&swa);
  if (handle==0) return 0;

  if (hxc::colormap!=0){
    XSetWindowColormap(XD,handle,hxc::colormap);
  }

  SetProp(XD,handle,cWinProc,(DWORD)modal_dialog_winproc);
  SetProp(XD,handle,cWinThis,(DWORD)XD);

  Atom Prots[1]={XA_WM_DELETE_WINDOW};
  XSetWMProtocols(XD,handle,Prots,1);

  XSelectInput(XD,handle,ExposureMask | StructureNotifyMask);

  XStoreName(XD,handle,title);

  unix_non_resizable_window(XD,handle);

  XWMHints *hints=XAllocWMHints();
  if (hints){
    hints->flags=InputHint | StateHint | XUrgencyHint;
    hints->input=True;
    hints->initial_state=NormalState;
    XSetWMHints(XD,handle,hints);
    XFree(hints);
  }

  if (create_okcancel){
    new hxc_button(XD,handle,10,h-10-25,(w-20)/2-5,25,modal_but_np,
                NULL,BT_TEXT,T("OK"),1,hxc::col_bk);

    new hxc_button(XD,handle,10+(w-20)/2+5,h-10-25,(w-20)/2-5,25,modal_but_np,
                NULL,BT_TEXT,T("Cancel"),2,hxc::col_bk);
  }

  return handle;
}
//---------------------------------------------------------------------------
int hxc::modal_dialog_winproc(Display *XD,Window Win,XEvent *Ev)
{
  if (Ev->type==ClientMessage){
    if (Ev->xclient.message_type==XA_WM_PROTOCOLS){
      if ((Atom)(Ev->xclient.data.l[0])==XA_WM_DELETE_WINDOW){
        if (modal_result==0) modal_result=2;
      }
    }
  }else if (Ev->type==MapNotify){
    if (modal_focus_win) XSetInputFocus(XD,modal_focus_win,RevertToParent,CurrentTime);
  }else if (Ev->type==Expose){
    if (Ev->xexpose.count>0) return 0;

    XWindowAttributes wa;
    XGetWindowAttributes(XD,Win,&wa);
    hxc::draw_border(XD,Win,hxc::gc,0,0,wa.width,wa.height,2,
                      hxc::col_border_light,hxc::col_border_dark);
  }
  return 0;
}
//---------------------------------------------------------------------------
int hxc::modal_but_np(hxc_button *b,int mess,int*)
{
  if (mess==BN_CLICKED) modal_result=b->id;
  return 0;
}
//---------------------------------------------------------------------------
int hxc::show_modal_dialog(Display *XD,Window handle,bool show_win,Window focus_win)
{
  if (hxc::modal_notifyproc) hxc::modal_notifyproc(true);

  XEvent Ev;
  LPWINDOWPROC WinProc;

  // Flush any messages pending
  do{
    wait_for_event(XD,&Ev,50);
    WinProc=(LPWINDOWPROC)hxc::GetProp(XD,Ev.xany.window,cWinProc);
    if (WinProc) WinProc((void*)hxc::GetProp(XD,Ev.xany.window,cWinThis),Ev.xany.window,&Ev);
  }while (XPending(XD));

  if (show_win){
    hxc::modal_children(XD,handle,handle);
    hxc::SetProp(XD,handle,hxc::cModal,handle);
    XSetTransientForHint(XD,handle,handle);

    XMapWindow(XD,handle);
    modal_focus_win=focus_win;
  }

  modal_result=0;
  LOOP{
    if (hxc::wait_for_event(XD,&Ev)){
      WinProc=(LPWINDOWPROC)  hxc::GetProp(XD,Ev.xany.window,cWinProc);
      if (WinProc){
        DWORD modal=hxc::GetProp(XD,Ev.xany.window,hxc::cModal);
      	if (modal!=handle && modal!=0xffffffff){ //not modal
      		if (hxc::suppress_mess_for_modal(XD,&Ev)==0){
  		    	WinProc((void*)  hxc::GetProp(XD,Ev.xany.window,cWinThis),Ev.xany.window,&Ev);
      		}else{
      			if (Ev.type==FocusIn) XRaiseWindow(XD,handle);
      		}
      	}else{
  	    	WinProc((void*)  hxc::GetProp(XD,Ev.xany.window,cWinThis),Ev.xany.window,&Ev);
  			}
      }
    }
    if (modal_result) if (XPending(XD)==0) break;
  }
  return modal_result;
}
//---------------------------------------------------------------------------
void hxc::destroy_modal_dialog(Display *XD,Window handle)
{
	hxc::modal_children(XD,handle,0);
  hxc::destroy_children_of(handle);

  hxc::RemoveProp(XD,handle,cWinProc);
  hxc::RemoveProp(XD,handle,cWinThis);
 	hxc::RemoveProp(XD,handle,hxc::cModal);
  hxc::kill_timer(handle,HXC_TIMER_ALL_IDS);
  XDestroyWindow(XD,handle);

  hxc::free_res(XD);
  if (hxc::modal_notifyproc) hxc::modal_notifyproc(0);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include "hxc_scrollbar.cpp"
#include "hxc_listview.cpp"
#include "hxc_button.cpp"
#include "hxc_dropdown.cpp"
#include "hxc_textdisplay.cpp"
#include "hxc_edit.cpp"
#include "hxc_scrollarea.cpp"

#endif

