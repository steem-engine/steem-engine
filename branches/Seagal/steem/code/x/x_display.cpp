//---------------------------------------------------------------------------
bool SteemDisplay::CheckDisplayMode(DWORD red_mask,DWORD green_mask,DWORD blue_mask)
{
  bool Valid=0;
  if (BytesPerPixel==1){
//    if (AlreadyWarnedOfBadMode==0){
//    	MessageBox(0,T("XSteem does not yet support 8-bit mode.  If you want us to add this feature write to us and let us know."),T("Display Error"),MB_ICONEXCLAMATION);
//    	AlreadyWarnedOfBadMode=true;
//    }
//    return 0;
    Valid=true;
  }else if (BytesPerPixel==2){
    if (blue_mask==   MAKEBINW(b00000000,b00011111)){
      if (green_mask==MAKEBINW(b00000011,b11100000) &&
          red_mask==  MAKEBINW(b01111100,b00000000)){
        rgb555=true;
        Valid=true;
      }else if (green_mask==MAKEBINW(b00000111,b11100000) &&
                red_mask==  MAKEBINW(b11111000,b00000000)){
        rgb555=0;
        Valid=true;
      }
    }
  }else if (BytesPerPixel<=4){
    rgb555=0;
    rgb32_bluestart_bit=0;
    Valid=(blue_mask==0x0000ff && green_mask==0x00ff00 && red_mask==0xff0000);
    if (BytesPerPixel==4 && Valid==0){
      if (blue_mask==0x0000ff00 && green_mask==0x00ff0000 && red_mask==0xff000000){
        Valid=true;
        rgb32_bluestart_bit=8;
      }
    }
  }
  if (Valid==0){
    if (AlreadyWarnedOfBadMode==0){
    	EasyStr Text=T("Sorry, your current screen mode is not supported by Steem.");
      if (BytesPerPixel<=4){
      	Text+="\n\n";
        Text+=T("If you want you can e-mail us with the below text and we'll consider adding support for it:")+"\n\n";

        EasyStr Bin;
        for (int n=0;n<BytesPerPixel*8;n++){
          char c='0';
          if ((red_mask >> n) & 1) c='R';
          if ((green_mask >> n) & 1) c='G';
          if ((blue_mask >> n) & 1) c='B';
          Bin.Insert(c,0);
        }
        Text+=Bin;
      }
      MessageBox(0,Text,T("Display Error"),MB_ICONINFORMATION);
    	AlreadyWarnedOfBadMode=true;
    }
    return 0;
  }
  return true;
}
//---------------------------------------------------------------------------
bool SteemDisplay::InitX()
{
  if (XD==NULL) return 0;

  Release();

  int Scr=XDefaultScreen(XD);
  int w=640,h=480;
  if (Disp.BorderPossible()){
    w=768;h=400+2*(BORDER_TOP+BORDER_BOTTOM);
  }
#ifndef NO_CRAZY_MONITOR
  if(extended_monitor){
    w=GetScreenWidth();
    h=GetScreenHeight();
  }
#endif

  int Depth=XDefaultDepth(XD,Scr);

  BytesPerPixel=(Depth+7)/8;
  if (Depth>=24) BytesPerPixel=4;

  char *ImgMem=(char*)malloc(w*h*BytesPerPixel);
  X_Img=XCreateImage(XD,XDefaultVisual(XD,Scr),
                      Depth,ZPixmap,0,ImgMem,
                      w,h,BytesPerPixel*8,0);
  if (X_Img){
    BytesPerPixel=(X_Img->bits_per_pixel+7)/8;
    if (CheckDisplayMode(X_Img->red_mask,X_Img->green_mask,X_Img->blue_mask)==0){
      Release();
      return 0;
    }
  }else{
    free(ImgMem);
    MessageBox(0,T("Couldn't create XImage."),T("Display Error"),MB_ICONINFORMATION);
    return 0;
  }

  SurfaceWidth=w;
  SurfaceHeight=h;
  draw_init_resdependent();
  palette_prepare(true);

  return true;
}
//---------------------------------------------------------------------------
#ifndef NO_SHM
_XFUNCPROTOBEGIN
int XShmGetEventBase(
#if NeedFunctionPrototypes
Display *
#endif
);
_XFUNCPROTOEND
#endif

bool SteemDisplay::InitXSHM()
{
#ifdef NO_SHM
  return 0;
#else
  if (XD==NULL) return 0;

  Release();

  if (XShmQueryExtension(XD)==0){
    MessageBox(0,T("MIT shared memory extension not available."),T("SHM Error"),MB_ICONINFORMATION | MB_OK);
    return 0;
  }

  int Scr=XDefaultScreen(XD);
  int w=640,h=480;
  if (BorderPossible()){
    w=768;h=400+2*(BORDER_TOP+BORDER_BOTTOM);
  }
#ifndef NO_CRAZY_MONITOR
  if(extended_monitor){
    w=GetScreenWidth();
    h=GetScreenHeight();
  }
#endif

  X_Img=XShmCreateImage(XD,XDefaultVisual(XD,Scr),
                 XDefaultDepth(XD,Scr),ZPixmap,NULL,&XSHM_Info,w,h);
  if (X_Img==NULL){
    MessageBox(0,T("Couldn't create shared memory XImage."),T("SHM Error"),MB_ICONINFORMATION);
    Release();return 0;
  }

  XSHM_Info.shmid=shmget(IPC_PRIVATE,X_Img->bytes_per_line*X_Img->height,IPC_CREAT | 0777);
  if (XSHM_Info.shmid==-1){
    MessageBox(0,T("Couldn't allocate shared memory."),T("SHM Error"),MB_ICONINFORMATION);
    Release();return 0;
  }

  XSHM_Info.shmaddr=(char*)shmat(XSHM_Info.shmid,0,0);
  if (XSHM_Info.shmaddr==(char*)-1){
    MessageBox(0,T("Couldn't attach shared memory."),T("SHM Error"),MB_ICONINFORMATION);
    Release();return 0;
  }
  X_Img->data=XSHM_Info.shmaddr;

  XSHM_Info.readOnly=0;
  if (XShmAttach(XD,&XSHM_Info)==0){
    MessageBox(0,T("The X server couldn't attach the shared memory."),T("SHM Error"),MB_ICONINFORMATION);
    Release();return 0;
  }
  XSHM_Attached=true;

  SHMCompletion=XShmGetEventBase(XD)+ShmCompletion;
//	SHMCompletion=65; //it is for us!

  BytesPerPixel=(X_Img->bits_per_pixel+7)/8;
  if (CheckDisplayMode(X_Img->red_mask,X_Img->green_mask,X_Img->blue_mask)==0){
    Release();return 0;
  }

  printf(EasyStr("Bytes per pixel=")+BytesPerPixel+"  Depth="+XDefaultDepth(XD,Scr)+"\n");
  SurfaceWidth=w;
  SurfaceHeight=h;
  draw_init_resdependent();
  palette_prepare(true);

  return true;
#endif
}
//---------------------------------------------------------------------------
HRESULT SteemDisplay::Lock()
{
  if (XD==NULL) return DDERR_GENERIC;

  WaitForAsyncBlitToFinish();
  if (Method==DISPMETHOD_X){
    draw_mem=LPBYTE(X_Img->data);
    draw_line_length=X_Img->bytes_per_line;
    return DD_OK;
  }else if (Method==DISPMETHOD_XSHM){
    draw_mem=LPBYTE(X_Img->data);
    draw_line_length=X_Img->bytes_per_line;
    return DD_OK;
  }
  return DDERR_GENERIC;
}
//---------------------------------------------------------------------------
void SteemDisplay::Surround()
{
  if (FullScreen) return;

  XWindowAttributes wa;
  XGetWindowAttributes(XD,StemWin,&wa);

  int w=wa.width,h=wa.height-(MENUHEIGHT);

  int sw=draw_blit_source_rect.right;
  int sh=draw_blit_source_rect.bottom;
  int dx=(w-(sw+4))/2;
  int dy=(h-(sh+4))/2;
  int fx1=dx,fy1=dy,fx2=dx+sw+4,fy2=dy+sh+4;
  XSetForeground(XD,DispGC,BkCol);

  int bh=dy;
  if (h & 1) bh++;
  if (dy>0){ //draw grey border top and bottom
    XFillRectangle(XD,StemWin,DispGC,0,MENUHEIGHT,w,dy);
  }else{
    dy=0;
    fy1=0;
    fy2=h;
  }
  if (bh>0) XFillRectangle(XD,StemWin,DispGC,0,dy+sh+(MENUHEIGHT+4),w,bh);

  int rw=dx;
  if (w & 1) rw++;
  if (dx>0){ //draw grey border left and right
    XFillRectangle(XD,StemWin,DispGC,0,dy+(MENUHEIGHT),dx,sh+4);
  }else{
    fx1=0;
    fx2=w;
  }
  if (rw>0) XFillRectangle(XD,StemWin,DispGC,dx+sw+4,dy+(MENUHEIGHT),rw,sh+4);

  fy1+=MENUHEIGHT;fy2+=MENUHEIGHT;
  XSetForeground(XD,DispGC,BlackCol);
  XDrawLine(XD,StemWin,DispGC,fx1+1,fy1+1,fx2-1,fy1+1);
  XDrawLine(XD,StemWin,DispGC,fx1+1,fy1+2,fx1+1,fy2-1);
  XSetForeground(XD,DispGC,BorderDarkCol);
  XDrawLine(XD,StemWin,DispGC,fx1,fy1,fx2-1,fy1);
  XDrawLine(XD,StemWin,DispGC,fx1,fy1,fx1,fy2-1);
  XSetForeground(XD,DispGC,WhiteCol);
  XDrawLine(XD,StemWin,DispGC,fx1+1,fy2-1,fx2-1,fy2-1);
  XDrawLine(XD,StemWin,DispGC,fx2-1,fy1+1,fx2-1,fy2-1);
  XSetForeground(XD,DispGC,BorderLightCol);
  XDrawLine(XD,StemWin,DispGC,fx1+2,fy2-2,fx2-2,fy2-2);
  XDrawLine(XD,StemWin,DispGC,fx2-2,fy1+2,fx2-2,fy2-2);
}

void SteemDisplay::VSync()
{
}

bool SteemDisplay::Blit()
{
  if (XD==NULL) return 0;

  int sx,sy,sw,sh,dx,dy;
  Window ToWin;
  if (FullScreen){
    ToWin=XVM_FullWin;

    sx=draw_blit_source_rect.left;
    sy=draw_blit_source_rect.top;
    sw=draw_blit_source_rect.right;
    sh=draw_blit_source_rect.bottom;

    dx=max((XVM_FullW-sw)/2,0);
    dy=max((XVM_FullH-sh)/2,0);
    if (sh>XVM_FullH) sh=XVM_FullH;
  }else{
    ToWin=StemWin;

    XWindowAttributes wa;
    XGetWindowAttributes(XD,StemWin,&wa);
    int w=wa.width-4,h=wa.height-(MENUHEIGHT+4);
    if (w<=0 || h<=0) return true;

    dx=(w-draw_blit_source_rect.right)/2;
  	dy=(h-draw_blit_source_rect.bottom)/2;
  	sx=draw_blit_source_rect.left;
  	sy=draw_blit_source_rect.top;
  	sw=draw_blit_source_rect.right;
  	sh=draw_blit_source_rect.bottom;
  	if (dx<0){
  		sx-=dx;
  		sw=w;
  		dx=0;
  	}
  	if (dy<0){
  		sy-=dy;
  		sh=h;
  		dy=0;
  	}
  	dy+=MENUHEIGHT+2;
  	dx+=2;
  }

  bool DoneIt=0;
//	printf("XPutImage(... ,%i,%i,%i,%i,%i,%i)\n",draw_blit_source_rect.left,draw_blit_source_rect.top,
//              dx,dy,sw,sh);
  if (Method==DISPMETHOD_X){
    XPutImage(XD,ToWin,DispGC,X_Img,sx,sy,
              dx,dy,sw,sh);
    DoneIt=true;
  }else if (Method==DISPMETHOD_XSHM){
#ifndef NO_SHM
    Disp.WaitForAsyncBlitToFinish();

    XShmPutImage(XD,ToWin,DispGC,X_Img,sx,sy,dx,dy,sw,sh,True);
    asynchronous_blit_in_progress=true;
    if (Disp.DoAsyncBlit==0) Disp.WaitForAsyncBlitToFinish();

    DoneIt=true;
#endif
  }

#ifdef DRAW_ALL_ICONS_TO_SCREEN
  XSetForeground(XD,DispGC,BlackCol);
  for (int n=Ico16.NumIcons-1;n>=0;n--){
    Ico16.DrawIcon(n,ToWin,DispGC,n*16,30);
    XDrawString(XD,ToWin,DispGC,n*16,65,EasyStr(n),strlen(EasyStr(n)));
  }
  for (int n=Ico32.NumIcons-1;n>=0;n--){
    Ico32.DrawIcon(n,ToWin,DispGC,n*32,80);
    XDrawString(XD,ToWin,DispGC,n*32,135,EasyStr(n),strlen(EasyStr(n)));
  }
#endif
#ifdef DRAW_TIMER_TO_SCREEN
  EasyStr tt=timer;
  XDrawString(XD,ToWin,DispGC,10,40,tt,tt.Length());
#endif
  if (runstate!=RUNSTATE_RUNNING && pc==rom_addr){
    // If all initialisation failed might be 0x0
    if (sw>=320 && sh>=200) osd_draw_reset_info(dx,dy,sw,sh);
  }

  return DoneIt;
}
//---------------------------------------------------------------------------
void SteemDisplay::WaitForAsyncBlitToFinish()
{
#ifndef NO_SHM
  if (asynchronous_blit_in_progress==0) return;

  XEvent ev;
  clock_t wait_till=clock()+(CLOCKS_PER_SEC/50);
  for (int wait=50000;wait>=0;wait--){
    if (XCheckTypedEvent(XD,SHMCompletion,&ev)) break;
    if (clock()>wait_till) break;
  }
  asynchronous_blit_in_progress=false;
#endif
}
//---------------------------------------------------------------------------
void SteemDisplay::Unlock()
{
}
//---------------------------------------------------------------------------
void SteemDisplay::RunStart(bool)
{
  if (FullScreenBut.checked) ChangeToFullScreen();
}
//---------------------------------------------------------------------------
void SteemDisplay::RunEnd(bool)
{
  if (FullScreen){
    ChangeToWindowedMode();
    draw(true);
  }
}
//---------------------------------------------------------------------------
void SteemDisplay::ScreenChange()
{
  if (Method==DISPMETHOD_X){
    if (InitX()){
      Method=DISPMETHOD_X;
    }else{
      Init();
    }
  }else if (Method==DISPMETHOD_XSHM){
    if (InitXSHM()){
      Method=DISPMETHOD_XSHM;
    }else{
      Init();
    }
  }
}
//---------------------------------------------------------------------------
void SteemDisplay::ChangeToFullScreen()
{
#ifndef NO_XVIDMODE
  if (FullScreen) return;
  if (CanGoToFullScreen()==0) return;

  int Screen=XDefaultScreen(XD);
  if (XF86VidModeGetAllModeLines(XD,Screen,&XVM_nModes,&XVM_Modes)==0) return;

  int w=640,h=480;
  if (border & 1){
    w=800;h=600;
  }
  if (prefer_res_640_400 && border==0){
    w=640,h=400;
  }
#ifndef NO_CRAZY_MONITOR
  if (extended_monitor){
    w=em_width;h=em_height;
  }
#endif

  XF86VidModeModeInfo *Mode=NULL;
  int diff=0xffff;
  for (int a=0;a<2;a++){
    if (prefer_res_640_400 && border==0) h=400;
    for (int n=0;n<2;n++){
      for (int i=0;i<XVM_nModes;i++){
        if (a==0){
          // get exact
          if (XVM_Modes[i]->hdisplay==w && XVM_Modes[i]->vdisplay==h){
            Mode=XVM_Modes[i];
            break;
          }
        }else{
          // get closest
          if (XVM_Modes[i]->hdisplay>=w && XVM_Modes[i]->vdisplay>=h){
            int new_diff=(XVM_Modes[i]->hdisplay-w)+(XVM_Modes[i]->vdisplay-h);
            if (new_diff<diff){
              Mode=XVM_Modes[i];
              diff=new_diff;
            }
          }
        }
      }
      if (Mode || h!=400) break;

      h=480;
    }
    if (Mode) break;
  }
  if (Mode==NULL){
    Alert(T("Can't change to fullscreen. Your video card doesn't support the required screen mode")+
            " ("+w+"x"+h+")",T("Error"),MB_ICONERROR);
    XFree(XVM_Modes);
    return;
  }
  
  w=Mode->hdisplay, h=Mode->vdisplay;
  int x=(GetScreenWidth()-w)/2, y=(GetScreenHeight()-h)/2;

  XSetWindowAttributes swa;
  swa.backing_store=NotUseful;
  swa.override_redirect=True;
  XVM_FullWin=XCreateWindow(XD,XDefaultRootWindow(XD),x,y,w,h,Screen,
                           CopyFromParent,InputOutput,CopyFromParent,
                           CWBackingStore | CWOverrideRedirect,&swa);

  SetProp(XD,XVM_FullWin,cWinProc,(DWORD)XVM_WinProc);
  SetProp(XD,XVM_FullWin,cWinThis,(DWORD)this);
  SetProp(XD,XVM_FullWin,hxc::cModal,(DWORD)0xffffffff);

  XSelectInput(XD,XVM_FullWin,KeyPressMask | KeyReleaseMask |
                            ButtonPressMask | ButtonReleaseMask |
                            ExposureMask | FocusChangeMask);

  XMapWindow(XD,XVM_FullWin);

  XF86VidModeGetViewPort(XD,Screen,&XVM_ViewX,&XVM_ViewY);
  FullScreen=XF86VidModeSwitchToMode(XD,Screen,Mode);
  if (FullScreen){
    XF86VidModeSetViewPort(XD,Screen,x,y);
    draw_grille_black=max(draw_grille_black,50);

    XGrabPointer(XD,XVM_FullWin,False,ButtonPressMask | ButtonReleaseMask,
                  GrabModeAsync,GrabModeAsync,XVM_FullWin,EmptyCursor,CurrentTime);

    window_mouse_centre_x=w/2;
    window_mouse_centre_y=h/2;
    XWarpPointer(XD,None,XVM_FullWin,0,0,0,0,window_mouse_centre_x,window_mouse_centre_y);
    mouse_move_since_last_interrupt_x=0;
    mouse_move_since_last_interrupt_y=0;
    mouse_change_since_last_interrupt=false;

    XVM_FullW=w;
    XVM_FullH=h;
    if (XVM_FullH<480) using_res_640_400=true;
  }else{
    XDestroyWindow(XD,XVM_FullWin);
    Alert(T("Can't change to fullscreen. There was an error switching to the required screen mode")+
            " ("+w+"x"+h+")",T("Error"),MB_ICONERROR);
    XFree(XVM_Modes);
  }
#endif
}
//---------------------------------------------------------------------------
#ifndef NO_XVIDMODE
int SteemDisplay::XVM_WinProc(void*,Window Win,XEvent *Ev)
{
#ifndef NO_SHM
  if (Ev->type==Disp.SHMCompletion){
    Disp.asynchronous_blit_in_progress=false;
    return PEEKED_MESSAGE;
  }
#endif
  switch (Ev->type){
    case Expose:
      draw_grille_black=max(draw_grille_black,50);
      break;
    case ButtonPress: // For MMB
    case ButtonRelease:
    case KeyPress:
    case KeyRelease:
      return StemWinProc(NULL,StemWin,Ev);
    case FocusOut:
      runstate=RUNSTATE_STOPPING;
      break;
  }
  return PEEKED_MESSAGE;
}
#endif
//---------------------------------------------------------------------------
void SteemDisplay::ChangeToWindowedMode(bool)
{
#ifndef NO_XVIDMODE
  if (FullScreen==0) return;

  int Screen=XDefaultScreen(XD);
  XF86VidModeSwitchToMode(XD,Screen,XVM_Modes[0]);
  XF86VidModeSetViewPort(XD,Screen,XVM_ViewX,XVM_ViewY);

  XFree(XVM_Modes);

  XDestroyWindow(XD,XVM_FullWin);

  FullScreen=0;
  using_res_640_400=0;
#endif
}
//---------------------------------------------------------------------------
void SteemDisplay::DrawFullScreenLetterbox()
{
  if (FullScreen==0) return;
  if (using_res_640_400) return;
#ifndef NO_CRAZY_MONITOR
  if (extended_monitor) return;
#endif
  
  XSetForeground(XD,DispGC,BlackCol);

  int w_gap=XVM_FullW-draw_blit_source_rect.right;
  int h_gap=XVM_FullH-draw_blit_source_rect.bottom;
  if (w_gap){
    XFillRectangle(XD,XVM_FullWin,DispGC,0,0,w_gap/2,XVM_FullH);
    XFillRectangle(XD,XVM_FullWin,DispGC,w_gap/2+draw_blit_source_rect.right,0,w_gap/2+1,600);
  }
  if (h_gap){
    XFillRectangle(XD,XVM_FullWin,DispGC,0,0,XVM_FullW,h_gap/2);
    XFillRectangle(XD,XVM_FullWin,DispGC,0,h_gap/2+draw_blit_source_rect.bottom,XVM_FullW,h_gap/2+1);
  }
}
//---------------------------------------------------------------------------
bool SteemDisplay::CanGoToFullScreen()
{
#ifdef NO_XVIDMODE
  return 0;
#else
  int evbase,errbase;
  return XF86VidModeQueryExtension(XD,&evbase,&errbase);
#endif
}
//---------------------------------------------------------------------------
void SteemDisplay::FlipToDialogsScreen()
{
}
//---------------------------------------------------------------------------
HRESULT SteemDisplay::SetDisplayMode(int,int,int,int,int*)
{
  return DDERR_GENERIC;
}
//---------------------------------------------------------------------------
HRESULT SteemDisplay::RestoreSurfaces()
{
  ScreenChange();
  return DD_OK;
}
//---------------------------------------------------------------------------
void SteemDisplay::Release()
{
  draw_end();

#ifndef NO_SHM
  if (XSHM_Attached && XD){
    XSync(XD,False);
    if (XD) XShmDetach(XD,&XSHM_Info);XSHM_Attached=0;
    if (XD) XSync(XD,False);
  }
#endif

  if (X_Img){
    XDestroyImage(X_Img);X_Img=NULL;
  }

#ifndef NO_SHM
  if (XSHM_Info.shmaddr!=(char*)-1){
    shmdt(XSHM_Info.shmaddr);XSHM_Info.shmaddr=(char*)-1;
  }
  if (XSHM_Info.shmid!=-1){
    shmctl(XSHM_Info.shmid,IPC_RMID,0);XSHM_Info.shmid=-1;
  }
#endif

  palette_remove();
  Method=0;
}
//---------------------------------------------------------------------------
#ifdef SHOW_WAVEFORM
void SteemDisplay::DrawWaveform()
{
}
#endif
//---------------------------------------------------------------------------

