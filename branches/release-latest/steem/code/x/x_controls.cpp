//---------------------------------------------------------------
void hxc_buttonpicker::init()
{
  can_have_children=false;
  destroyproc=(LPHXC_DESTROYPROC)destroy;
  deleteproc=(LPHXC_DELETEPROC)delete_hxc_buttonpicker;

  DirID=0xffff;
  st_keys_only=0;
  allow_joy=0;
}
//---------------------------------------------------------------
bool hxc_buttonpicker::create(Display *d,Window daddy,int x,int y,int w,int h,
            LPHXC_BUTTONPICKERNOTIFYPROC pass_notifyproc,void *o,int pass_ID)
{
  if (XD) destroy(this);
  XD=d;
  parent=daddy;
  notifyproc=pass_notifyproc;
  id=pass_ID;
  owner=o;

  load_res(XD);

  XSetWindowAttributes swa;
  swa.backing_store=NotUseful;
  swa.background_pixel=col_bk;
  handle=XCreateWindow(XD,parent,x,y,w,h,0,
                           CopyFromParent,InputOutput,CopyFromParent,
                           CWBackingStore | CWBackPixel,&swa);
	hxc::x=x;hxc::y=y;hxc::w=w;hxc::h=h;

  if (handle==0){
    XD=NULL;
    return 0;
  }

  SetProp(XD,handle,cWinProc,DWORD(WinProc));
  SetProp(XD,handle,cWinThis,DWORD(this));

  XSelectInput(XD,handle,ButtonPressMask | ButtonReleaseMask |
                  ExposureMask | StructureNotifyMask | FocusChangeMask |
                  EnterWindowMask | LeaveWindowMask |
                  KeyPressMask | KeyReleaseMask);

  XMapWindow(XD,handle);

  return true;
}
//---------------------------------------------------------------
void hxc_buttonpicker::destroy(hxc_buttonpicker *This)
{
  if (This->XD==NULL) return;

  RemoveProp(This->XD,This->handle,cWinProc);
  RemoveProp(This->XD,This->handle,cWinThis);
  hxc::kill_timer(This->handle,HXC_TIMER_ALL_IDS);
  XDestroyWindow(This->XD,This->handle);

  free_res(This->XD);

  This->handle=0;
  This->XD=NULL;
}
//---------------------------------------------------------------------------
int hxc_buttonpicker::key_to_modbit(KeySym k)
{
  if (k==VK_LSHIFT) return   b00000001;
  if (k==VK_RSHIFT) return   b00000010;
  if (k==VK_LCONTROL) return b00000100;
  if (k==VK_RCONTROL) return b00001000;
  if (k==VK_LMENU) return    b00010000;
  if (k==VK_RMENU) return    b00100000;
  return 0;
}
//---------------------------------------------------------------
//---------------------------------------------------------------
void hxc_buttonpicker::get_joystick_down(DWORD *axis_down,DWORD *button_down)
{
  for (int j=0;j<MAX_PC_JOYS;j++){
    axis_down[j]=0;
    button_down[j]=0;
    if (JoyExists[j]){
      int DeadZonePercent=10;
      for (int a=0;a<6;a++){
        if (JoyInfo[j].AxisExists[a]){
          int DeadSize=( (JoyInfo[j].AxisLen[a]/2) * DeadZonePercent )/100;
          if (GetAxisPosition(a,&JoyPos[j]) > JoyInfo[j].AxisMid[a]+DeadSize) axis_down[j]|=(1 << a);
          if (GetAxisPosition(a,&JoyPos[j]) < JoyInfo[j].AxisMid[a]-DeadSize) axis_down[j]|=(1 << (16+a));
        }
      }
      button_down[j]=JoyPos[j].dwButtons;
      if (JoyInfo[j].NumButtons<=31){
        button_down[j]&=DWORD(1 << JoyInfo[j].NumButtons)-1;
      }
    }
  }
}
//---------------------------------------------------------------
WORD hxc_buttonpicker::axis_mask_to_dir_id(int j,DWORD m)
{
  for (int a=0;a<6;a++){
    if (m & (1 << a)){
      return MAKEWORD(a+1,10+j*10);
    }else if (m & (1 << (16+a))){
      return MAKEWORD(a+1,11+j*10);
    }
  }
  return 0xffff;
}
//---------------------------------------------------------------
WORD hxc_buttonpicker::button_mask_to_dir_id(int j,DWORD m)
{
  for (int b=0;b<32;b++){
    if (m & (1 << b)) return MAKEWORD(100+b,10+j*10);
  }
  return 0xffff;
}
//---------------------------------------------------------------
int hxc_buttonpicker::WinProc(hxc_buttonpicker *This,Window Win,XEvent *Ev)
{
	This->common_winproc(Ev);
  if (Ev){
    switch (Ev->type){
      case FocusIn:
        if (This->allow_joy){
          hxc::set_timer(Win,0,50,timer_notify_proc,This);
          get_joystick_down(old_joy_axis_down,old_joy_button_down);
        }
      case FocusOut:
        This->mod_down=0;
        This->draw(false);
        if (This->notifyproc) This->notifyproc(This,BPN_FOCUSCHANGE,Ev->type);
        if (Ev->type==FocusOut) hxc::kill_timer(Win,0);
        break;
      case ButtonPress:
        if (Ev->xbutton.button==Button4 || Ev->xbutton.button==Button5) break;
        XSetInputFocus(This->XD,Win,RevertToParent,CurrentTime);
        if (Ev->xbutton.button==Button2){
	        This->DirID=MAKEWORD(0,2);
	        This->draw(true);
	        if (This->notifyproc) This->notifyproc(This,BPN_CHANGE,This->DirID);
        }
        break;
      case ButtonRelease:
        break;
      case KeyPress:
      {
        KeySym k=Ev->xkey.keycode;
        int mb=This->key_to_modbit(k);
        This->mod_down|=mb;
        // If both pressed
        if ((This->mod_down & b00000011)==b00000011) k=VK_SHIFT;
        if ((This->mod_down & b00001100)==b00001100) k=VK_CONTROL;
        if ((This->mod_down & b00110000)==b00110000) k=VK_MENU;
      	if (This->st_keys_only){
          // If either pressed
          if (mb & b00001100) k=VK_CONTROL;
          if (mb & b00110000) k=VK_MENU;
      		if (key_table[k]) This->DirID=k;
      	}else{
          if (k==Key_Pause){
            This->DirID=0xffff;
          }else{
            This->DirID=k;
          }
        }
        This->draw(true);
        if (This->notifyproc) This->notifyproc(This,BPN_CHANGE,This->DirID);
        break;
      }
      case KeyRelease:
        This->mod_down&=~(This->key_to_modbit(Ev->xkey.keycode));
        break;
      case EnterNotify:
      case LeaveNotify:
        break;
      case Expose:
      case MapNotify:
        This->draw(true);
        break;
    }
  }else{
    This->draw(true);
  }
  return 0;
}


void hxc_buttonpicker::draw(bool draw_contents)
{
	if (XD==NULL) return;

  XWindowAttributes wa;
  XGetWindowAttributes(XD,handle,&wa);

  if(draw_contents){
    XSetForeground(XD,gc,col_white);
    XFillRectangle(XD,handle,gc,1,1,wa.width-2,wa.height-2);
  }

  Window Foc=0;
  int RevertTo;
  XGetInputFocus(XD,&Foc,&RevertTo);
  if(Foc!=handle){
    XSetForeground(XD,gc,col_border_dark);
  }else{
    XSetForeground(XD,gc,col_black);
  }
  XDrawLine(XD,handle,gc,0,0,wa.width-1,0);
  XDrawLine(XD,handle,gc,0,0,0,wa.height-1);
  if(Foc!=handle){
    XSetForeground(XD,gc,col_border_light);
  }
  XDrawLine(XD,handle,gc,0,wa.height-1,wa.width-1,wa.height-1);
  XDrawLine(XD,handle,gc,wa.width-1,wa.height-1,wa.width-1,0);

  if (draw_contents && font){
    text=DirID_to_text(DirID,st_keys_only);
    if (text[0]){

      int w=XTextWidth(font,text,text.Length())+1,
                h=font->ascent+font->descent;
      int x,y=(wa.height-h)/2+font->ascent;
      x=(wa.width-w)/2;
      XSetForeground(XD,gc,col_black);
      XDrawString(XD,handle,gc,x,y,text,text.Length());
    }
  }
  XSync(XD,False);
}
//---------------------------------------------------------------------------
int hxc_buttonpicker::timer_notify_proc(void *t,Window win,int id)
{
  hxc_buttonpicker *This=(hxc_buttonpicker*)t;
  DWORD axis_down[MAX_PC_JOYS],but_down[MAX_PC_JOYS];
  get_joystick_down(axis_down,but_down);
  WORD new_dir_id=0xffff;
  for (int j=0;j<MAX_PC_JOYS;j++){
    // Ignore axis/buttons that were already pressed
    DWORD ax=axis_down[j] & ~old_joy_axis_down[j];
    DWORD but=but_down[j] & ~old_joy_button_down[j];
    if (ax) new_dir_id=axis_mask_to_dir_id(j,ax);
    if (but) new_dir_id=button_mask_to_dir_id(j,but);

    old_joy_axis_down[j]=axis_down[j];
    old_joy_button_down[j]=but_down[j];
  } 
  if (NOT_BLANK_DIRID(new_dir_id)){
    This->DirID=new_dir_id;
    This->draw(true);
    if (This->notifyproc) This->notifyproc(This,BPN_CHANGE,This->DirID);
  }
  return HXC_TIMER_REPEAT;
}
