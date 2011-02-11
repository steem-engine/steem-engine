#ifndef HXC_POPUPHINTS_CPP
#define HXC_POPUPHINTS_CPP

#include "hxc_popuphints.h"
//---------------------------------------------------------------------------
hxc_popuphints::hxc_popuphints()
{
  win_list.Sort=eslNoSort;
  border=1;
  handle=0;
  XD=NULL;
  notifyproc=NULL;
  delay_before_show=500;
  time_to_show=10000;
  mouse_in_win=None;
  mouse_in_count=0;
  max_width=0;
}
//---------------------------------------------------------------------------
void hxc_popuphints::get_current_lines(EasyStringList *psl)
{
  psl->Sort=eslNoSort;
  int l=current_text.Length();
  char *buf=new char[l+1];
  char *line=buf,*buf_end=buf+l;
  strcpy(buf,current_text);

  while (strrchr(buf,'\n')) *strrchr(buf,'\n')=0;

  char *changed=NULL;
  do{
    bool next_line=true;
    if (line[0]){
      if (hxc::get_text_width(XD,line)>max_width){
        char *new_end=strrchr(line,' ');
        if (new_end){
          if (changed) *changed=' '; // restore next line
          changed=new_end;
          *changed=0;
          next_line=0;
        }
      }
    }
    if (next_line){
      line+=strlen(line)+1;
      changed=NULL;
    }
  }while (line<buf_end);

  line=buf;
  do{
    psl->Add(line);
    line+=strlen(line)+1;
  }while (line<buf_end);
  delete[] buf;
}
//---------------------------------------------------------------------------
void hxc_popuphints::start()
{
  hxc::set_timer(None,0,100,(LPHXC_TIMERPROC)timerproc,this);
}
//---------------------------------------------------------------------------
void hxc_popuphints::stop()
{
  hxc::kill_timer(None,0);
}
//---------------------------------------------------------------------------
void hxc_popuphints::add(Window win,EasyStr s,Window p)
{
  win_list.Add(s,win,p);
}
//---------------------------------------------------------------------------
void hxc_popuphints::remove(Window win)
{
  for (int i=0;i<win_list.NumStrings;i++){
    if ((Window)(win_list[i].Data[0])==win){
      win_list.Delete(i--);
    }
  }
}
//---------------------------------------------------------------------------
void hxc_popuphints::remove_all_children(Window p)
{
  for (int i=0;i<win_list.NumStrings;i++){
    if ((Window)(win_list[i].Data[1])==p){
      win_list.Delete(i--);
    }
  }
}
//---------------------------------------------------------------------------
void hxc_popuphints::change(Window win,EasyStr new_s)
{
  for (int i=0;i<win_list.NumStrings;i++){
    if ((Window)(win_list[i].Data[0])==win){
      win_list.SetString(i,new_s);
    }
  }
}
//---------------------------------------------------------------------------
bool hxc_popuphints::create()
{
  if (XD==NULL) return 0;
  if (handle) close();

  hxc::load_res(XD);

  if (max_width==0) max_width=XDisplayWidth(XD,XDefaultScreen(XD))/2;

  int th=hxc::font->ascent+hxc::font->descent+2;
  EasyStringList lines;
  get_current_lines(&lines);

  w=border+2+2+border;
  h=border+2+border;
  for (int n=0;n<lines.NumStrings;n++){
    w=max(border+2+(int)hxc::get_text_width(XD,lines[n].String)+2+border,w);
    h+=th;
  }

  Window in_win,in_child;
  int childx,childy,mx,my;
  x=XDisplayWidth(XD,XDefaultScreen(XD))/2;
  y=XDisplayWidth(XD,XDefaultScreen(XD))/2;
  UINT mask;
  XQueryPointer(XD,XDefaultRootWindow(XD),&in_win,&in_child,
                    &mx,&my,&childx,&childy,&mask);
  x=mx;
  y=my+20;

  int sw=XDisplayWidth(XD,XDefaultScreen(XD));
  int sh=XDisplayHeight(XD,XDefaultScreen(XD));
  if (x+w>sw) x=sw-w;
	if (y+h>sh) y=my-5-h;

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

  XSelectInput(XD,handle,ExposureMask);

  XMapWindow(XD,handle);
  return false;
}
//---------------------------------------------------------------------------
void hxc_popuphints::draw()
{
  if (XD==NULL || handle==0) return;
  
  XSetForeground(XD,hxc::gc,hxc::col_white);
  XFillRectangle(XD,handle,hxc::gc,border,border,w-border*2,h-border*2);
  XSetForeground(XD,hxc::gc,hxc::col_black);
  hxc::draw_border(XD,handle,hxc::gc,0,0,w,h,border,hxc::col_black,hxc::col_black);

	int th=hxc::font->descent + hxc::font->ascent + 2;
  int ty=border+2+hxc::font->ascent;
  EasyStringList lines;
  get_current_lines(&lines);
  for (int n=0;n<lines.NumStrings;n++){
    XDrawString(XD,handle,hxc::gc,border+2,ty,lines[n].String,strlen(lines[n].String));
    ty+=th;
  }
}
//---------------------------------------------------------------------------
void hxc_popuphints::close()
{
  if (XD==NULL || handle==0) return;

  RemoveProp(XD,handle,hxc::cModal);
  RemoveProp(XD,handle,cWinProc);
  RemoveProp(XD,handle,cWinThis);
  XDestroyWindow(XD,handle);

  hxc::free_res(XD);

  handle=0;
}
//---------------------------------------------------------------------------
int hxc_popuphints::WinProc(hxc_popuphints *This,Window Win,XEvent *Ev)
{
  if (This->XD==NULL) return 0;

  switch (Ev->type){
    case Expose:
      This->draw();
      break;
  }
  return 0;
}
//---------------------------------------------------------------------------
int hxc_popuphints::timerproc(hxc_popuphints *This,Window,int)
{
  // Get the topmost window that contains the mouse
  Window win=XDefaultRootWindow(This->XD);
  Window in_win,in_child;
  int rootx,rooty,winx,winy;
  UINT mask;
  for(;;){
    in_child=None;
    XQueryPointer(This->XD,win,&in_win,&in_child,
                      &rootx,&rooty,&winx,&winy,&mask);
    if (in_child==None || in_child==win || (mask & Button123Mask) || hxc::popup_active!=None) break;
    win=in_child;
  }

  // See if that is on our list
  bool showing=0,found_window=0;
  for (int n=0;n<This->win_list.NumStrings;n++){
    if ((Window)(This->win_list[n].Data[0])==win){
      found_window=true;
      if (This->mouse_in_win==win){
        This->mouse_in_count++;
        if (This->mouse_in_count>=This->delay_before_show/100 &&
              This->mouse_in_count<This->delay_before_show/100+This->time_to_show/100){
          This->current_text=This->win_list[n].String;
          showing=true;
        }
      }else{
        This->mouse_in_count=0;
      }
    }
  }
  if (found_window==0) This->mouse_in_count=0;
  This->mouse_in_win=win;
  
  if (showing && This->handle==0){
    This->create();
  }else if (showing==0 && This->handle){
    This->close();
  }
  return HXC_TIMER_REPEAT;
}
//---------------------------------------------------------------------------
#endif

