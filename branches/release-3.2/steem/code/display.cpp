/*---------------------------------------------------------------------------
FILE: display.cpp
MODULE: Steem
DESCRIPTION: A class to encapsulate the process of outputting to the display.
This contains the DirectDraw code used by Windows Steem for output.
---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
SteemDisplay::SteemDisplay()
{
#ifdef WIN32
  DDObj=NULL;
  DDPrimarySur=NULL;
  DDBackSur=NULL;
  DDClipper=NULL;
  DDBackSurIsAttached=0;
  DDExclusive=0;

  GDIBmp=NULL;
  GDIBmpMem=NULL;

  hFreeImage=NULL;
  ScreenShotFormatOpts=0;
  ScreenShotExt="bmp";

  DrawToVidMem=true;
  BlitHideMouse=true;
  DrawLetterboxWithGDI=0;

#elif defined(UNIX)
  X_Img=NULL;
  AlreadyWarnedOfBadMode=0;
  GoToFullscreenOnRun=0;

#ifndef NO_SHM
  XSHM_Attached=0;
  XSHM_Info.shmaddr=(char*)-1;
  XSHM_Info.shmid=-1;
  SHMCompletion=LASTEvent;
  asynchronous_blit_in_progress=false;
#endif

#ifndef NO_XVIDMODE
  XVM_Modes=NULL;
#endif

#endif

  ScreenShotFormat=0;
  ScreenShotUseFullName=0;ScreenShotAlwaysAddNum=0;
  ScreenShotMinSize=0;
  RunOnChangeToWindow=0;
  DoAsyncBlit=0;

  Method=DISPMETHOD_NONE;
  UseMethods[0]=DISPMETHOD_NONE;
  nUseMethod=0;
}
//---------------------------------------------------------------------------
SteemDisplay::~SteemDisplay() { Release(); }
bool SteemDisplay::BorderPossible() { return (GetScreenWidth()>640); }
//---------------------------------------------------------------------------
void SteemDisplay::SetMethods(int Method1,...)
{
  int *mp=&Method1;
  for (int n=0;n<5;n++){
    UseMethods[n]=mp[n];
    if (UseMethods[n]==0) break;
  }
  nUseMethod=0;
}
//---------------------------------------------------------------------------
HRESULT SteemDisplay::Init()
{
  Release();
  if (FullScreen==0){
    monitor_width=GetScreenWidth();
    monitor_height=GetScreenHeight();
  }

  while (nUseMethod<5){
#ifdef WIN32
    if (UseMethods[nUseMethod]==DISPMETHOD_DD){
      if (InitDD()==DD_OK) return (Method=UseMethods[nUseMethod++]);
    }else if (UseMethods[nUseMethod]==DISPMETHOD_GDI){
      if (InitGDI()) return (Method=UseMethods[nUseMethod++]);
    }
#elif defined(UNIX)
    if (UseMethods[nUseMethod]==DISPMETHOD_X){
      if (InitX()) return (Method=UseMethods[nUseMethod++]);
    }else if (UseMethods[nUseMethod]==DISPMETHOD_XSHM){
      if (InitXSHM()) return (Method=UseMethods[nUseMethod++]);
    }
#endif

    if (UseMethods[nUseMethod]==0){
      break;
    }
    nUseMethod++;
  }
  return 0;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifdef WIN32

#define LOGSECTION LOGSECTION_INIT

HRESULT SteemDisplay::InitDD()
{
  SetNotifyInitText("DirectDraw");

  HRESULT Ret;
  try{
    IDirectDraw *DDObj1=NULL;

    log("STARTUP: Initialising DirectDraw, creating DirectDraw object");
    Ret=CoCreateInstance(CLSID_DirectDraw,NULL,CLSCTX_ALL,IID_IDirectDraw,(void**)&DDObj1);
    if (Ret!=S_OK || DDObj1==NULL){
      EasyStr Err="Unknown error";
      switch (Ret){
        case REGDB_E_CLASSNOTREG:
          Err="The specified class is not registered in the registration database.";
          break;
        case E_OUTOFMEMORY:
          Err="Out of memory.";
          break;
        case E_INVALIDARG:
          Err="One or more arguments are invalid.";
          break;
        case E_UNEXPECTED:
          Err="An unexpected error occurred.";
          break;
        case CLASS_E_NOAGGREGATION:
          Err="This class cannot be created as part of an aggregate.";
          break;
      }
      Err=EasyStr("CoCreateInstance error\n\n")+Err;
      log_write("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      log_write(Err);
      log_write("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
#ifndef ONEGAME
      MessageBox(NULL,Err,T("Steem Engine DirectDraw Error"),
                    MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_TOPMOST);
#endif
      return ~DD_OK;
    }

    log("STARTUP: Initialising DirectDraw object");
    if ((Ret=DDObj1->Initialize(NULL)) != DD_OK){
      DDObj1->Release();
      return DDError("Initialise FAILED",Ret);
    }

    log("STARTUP: Calling QueryInterface");
    if ((Ret=DDObj1->QueryInterface(IID_IDirectDraw2,(LPVOID*)&DDObj)) != DD_OK)
        return DDError("QueryInterface FAILED",Ret);

    log("STARTUP: Calling SetCooperativeLevel");
    if ((Ret=DDObj->SetCooperativeLevel(StemWin,DDSCL_NORMAL)) != DD_OK)
        return DDError("SetCooperativeLevel FAILED",Ret);

    log("STARTUP: Creating the clipper");
    if ((Ret=DDObj->CreateClipper(0,&DDClipper,NULL))!=DD_OK)
        return DDError("CreateClipper FAILED",Ret);

    log("STARTUP: Associating clipper with main window");
    if ((Ret=DDClipper->SetHWnd(0,StemWin)) != DD_OK)
        return DDError("SetHWnd FAILED",Ret);

    log("STARTUP: Creating surfaces");
    if ((Ret=DDCreateSurfaces())!=DD_OK) return Ret;

    log("STARTUP: Performing lock test");
    DDLockFlags=DDLOCK_NOSYSLOCK;
    DDSURFACEDESC ddsd;
    ddsd.dwSize=sizeof(DDSURFACEDESC);
    if (DDBackSur->Lock(NULL,&ddsd,DDLOCK_WAIT | DDLockFlags,NULL)!=DD_OK){
      DDLockFlags=0;
      if ((Ret=DDBackSur->Lock(NULL,&ddsd,DDLOCK_WAIT | DDLockFlags,NULL))!=DD_OK){
        return DDError("Lock test FAILED",Ret);
      }
    }
    DDBackSur->Unlock(NULL);

    log("STARTUP: Enumerating display modes");
    ZeroMemory(DDDisplayModePossible,sizeof(DDDisplayModePossible));
    ZeroMemory(DDClosestHz,sizeof(DDClosestHz));
    DDObj->EnumDisplayModes(DDEDM_REFRESHRATES,NULL,this,DDEnumModesCallback);
    for (int idx=0;idx<3;idx++){
      for (int hicol=0;hicol<2;hicol++){
        for (int n=1;n<NUM_HZ;n++){
          if (DDClosestHz[idx][hicol][n]==0) DDClosestHz[idx][hicol][n]=HzIdxToHz[n]; 
        }
      }
    }

    return DD_OK;
  }catch(...){
    return DDError("DirectDraw caused DISASTER!",DDERR_EXCEPTION);
  }
}
//---------------------------------------------------------------------------
HRESULT WINAPI SteemDisplay::DDEnumModesCallback(LPDDSURFACEDESC ddsd,LPVOID t)
{
  if (ddsd->ddpfPixelFormat.dwRGBBitCount>16) return DDENUMRET_OK;
  
  SteemDisplay *This=(SteemDisplay*)t;
  int hicol=(ddsd->ddpfPixelFormat.dwRGBBitCount>8),idx=-1;

  if (ddsd->dwWidth==640 && ddsd->dwHeight==480) idx=0;
  if (ddsd->dwWidth==800 && ddsd->dwHeight==600) idx=1;
  if (ddsd->dwWidth==640 && ddsd->dwHeight==400) idx=2;

  if (idx>=0){
    This->DDDisplayModePossible[idx][hicol]=true;
    for (int n=1;n<NUM_HZ;n++){
      int diff=abs(HzIdxToHz[n]-int(ddsd->dwRefreshRate));
      int curdiff=abs(HzIdxToHz[n]-int(This->DDClosestHz[idx][hicol][n]));
      if (diff<curdiff && diff<=DISP_MAX_FREQ_LEEWAY){
        This->DDClosestHz[idx][hicol][n]=ddsd->dwRefreshRate;
      }
    }
  }
  return DDENUMRET_OK;
}
//---------------------------------------------------------------------------
HRESULT SteemDisplay::DDCreateSurfaces()
{
  DDSURFACEDESC ddsd;
  HRESULT Ret;

  DDDestroySurfaces();

  int ExtraFlags=0;
  if (DrawToVidMem==0) ExtraFlags=DDSCAPS_SYSTEMMEMORY; // Like malloc
//  ExtraFlags=DDSCAPS_VIDEOMEMORY | DDSCAPS_NONLOCALVIDMEM; // AGP?
//  ExtraFlags=DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM; // Video card?
  for (int n=0;n<2;n++){
    ZeroMemory(&ddsd,sizeof(DDSURFACEDESC));
    ddsd.dwSize=sizeof(DDSURFACEDESC);
    ddsd.dwFlags=DDSD_CAPS;
    ddsd.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE | ExtraFlags;
    if (FullScreen){
      ddsd.ddsCaps.dwCaps|=DDSCAPS_FLIP | DDSCAPS_COMPLEX;
      ddsd.dwFlags|=DDSD_BACKBUFFERCOUNT;
      ddsd.dwBackBufferCount=1;
    }
    if ((Ret=DDObj->CreateSurface(&ddsd,&DDPrimarySur,NULL))!=DD_OK){
      if (n==0){
        ExtraFlags=0;
      }else{
        // Another DirectX app is fullscreen so fail silently
        if (Ret==DDERR_NOEXCLUSIVEMODE) return Ret;
        // Otherwise make a big song and dance!
        return DDError("CreateSurface for PrimarySur FAILED",Ret);
      }
    }else{
      break;
    }
  }
  if (FullScreen) DDBackSurIsAttached=true;

  if ((Ret=DDPrimarySur->SetClipper(DDClipper))!=DD_OK){
    return DDError("SetClipper FAILED",Ret);
  }

  if (FullScreen==0){
    if (DrawToVidMem==0) ExtraFlags=DDSCAPS_SYSTEMMEMORY; // Like malloc
    for (int n=0;n<2;n++){
      ZeroMemory(&DDBackSurDesc,sizeof(DDSURFACEDESC));
      DDBackSurDesc.dwSize=sizeof(DDSURFACEDESC);
      DDBackSurDesc.dwFlags=DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
      DDBackSurDesc.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN | ExtraFlags;
#ifndef NO_CRAZY_MONITOR
      if(extended_monitor){
        DDBackSurDesc.dwWidth=GetScreenWidth();
        DDBackSurDesc.dwHeight=GetScreenHeight();
      }else
#endif
      if (Disp.BorderPossible()){
        DDBackSurDesc.dwWidth=768;
        DDBackSurDesc.dwHeight=400+2*(BORDER_TOP+BORDER_BOTTOM);
      }else{
        DDBackSurDesc.dwWidth=640;
        DDBackSurDesc.dwHeight=480;
      }
      if (draw_blit_source_rect.right>=int(DDBackSurDesc.dwWidth)){
        draw_blit_source_rect.right=int(DDBackSurDesc.dwWidth)-1;
      }
      if (draw_blit_source_rect.bottom>=int(DDBackSurDesc.dwHeight)){
        draw_blit_source_rect.bottom=int(DDBackSurDesc.dwHeight)-1;
      }
      if ((Ret=DDObj->CreateSurface(&DDBackSurDesc,&DDBackSur,NULL))!=DD_OK){
        if (n==0){
          ExtraFlags=0;
        }else{
          return DDError("CreateSurface for BackSur FAILED",Ret);
        }
      }else{
        break;
      }
    }
  }else{
    DDSCAPS caps;

    caps.dwCaps=DDSCAPS_BACKBUFFER;
    if ((Ret=DDPrimarySur->GetAttachedSurface(&caps,&DDBackSur))!=DD_OK){
      return DDError("CreateSurface for BackSur FAILED",Ret);
    }
  }

  DDBackSurDesc.dwSize=sizeof(DDSURFACEDESC);
  if ((Ret=DDBackSur->GetSurfaceDesc(&DDBackSurDesc))!=DD_OK){
    return DDError("GetSurfaceDesc for BackSur FAILED",Ret);
  }

  SurfaceWidth=DDBackSurDesc.dwWidth;
  SurfaceHeight=DDBackSurDesc.dwHeight;
  BytesPerPixel=DDBackSurDesc.ddpfPixelFormat.dwRGBBitCount/8;
  rgb555=(DDBackSurDesc.ddpfPixelFormat.dwGBitMask==0x3E0); //%0000001111100000  //1555
  rgb32_bluestart_bit=int((DDBackSurDesc.ddpfPixelFormat.dwBBitMask==0x0000ff00) ? 8:0);

  draw_init_resdependent();
  palette_prepare(true);

  return DD_OK;
}
//---------------------------------------------------------------------------
void SteemDisplay::DDDestroySurfaces()
{
  if (DDPrimarySur){
    DDPrimarySur->Release(); DDPrimarySur=NULL;
    if (DDBackSurIsAttached) DDBackSur=NULL;
  }
  if (DDBackSur){
    DDBackSur->Release(); DDBackSur=NULL;
  }
  DDBackSurIsAttached=0;
}
//---------------------------------------------------------------------------
HRESULT SteemDisplay::DDError(char *ErrorText,HRESULT DErr)
{
  Release();

  char Text[1000];
  strcpy(Text,ErrorText);
  strcat(Text,"\n\n");
  DDGetErrorDescription(DErr,Text+strlen(Text),499-strlen(Text));
  log_write("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  log_write(Text);
  log_write("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  strcat(Text,EasyStr("\n\n")+T("Would you like to disable the use of DirectDraw?"));
#ifndef ONEGAME
  int Ret=MessageBox(NULL,Text,T("Steem Engine DirectDraw Error"),
                     MB_YESNO | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_TOPMOST);
  if (Ret==IDYES) WriteCSFStr("Options","NoDirectDraw","1",INIFile);
#endif

  return DErr;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool SteemDisplay::InitGDI()
{
  Release();

  int w=640,h=480;
#ifndef NO_CRAZY_MONITOR
  if (extended_monitor){
    w=GetScreenWidth();
    h=GetScreenHeight();
  }else
#endif
  if (GetSystemMetrics(SM_CXSCREEN)>768){
    w=768;
    h=400+2*(BORDER_TOP+BORDER_BOTTOM);
  }

  log(Str("STARTUP: Creating bitmap w=")+w+" h="+h);
  HDC dc=GetDC(NULL);
  GDIBmp=CreateCompatibleBitmap(dc,w,h);
  ReleaseDC(NULL,dc);

  if (GDIBmp==NULL) return 0;

  BITMAP BmpInf;
  GetObject(GDIBmp,sizeof(BITMAP),&BmpInf);
  BytesPerPixel=(BmpInf.bmBitsPixel+7)/8;
  GDIBmpLineLength=BmpInf.bmWidthBytes;

  GDIBmpSize=GDIBmpLineLength*BmpInf.bmHeight;
  log(Str("STARTUP: BytesPerPixel=")+BytesPerPixel+" GDIBmpLineLength="+GDIBmpLineLength+" GDIBmpSize="+GDIBmpSize);

  GDIBmpDC=CreateCompatibleDC(NULL);
  SelectObject(GDIBmpDC,GDIBmp);
  SelectObject(GDIBmpDC,fnt);

  log("STARTUP: Creating bitmap memory");
  try{
    GDIBmpMem=new BYTE[GDIBmpSize+1];
  }catch (...){
    GDIBmpMem=NULL;
    Release();
    return 0;
  }

  if (BytesPerPixel>1){
    SetPixel(GDIBmpDC,0,0,RGB(255,0,0));
    GetBitmapBits(GDIBmp,GDIBmpSize,GDIBmpMem);
    DWORD RedBitMask=0;
    for (int i=BytesPerPixel-1;i>=0;i--){
      RedBitMask<<=8;
      RedBitMask|=GDIBmpMem[i];
    }
    rgb555=(RedBitMask==(hib01111100 | b00000000));
    rgb32_bluestart_bit=int((RedBitMask==0xff000000) ? 8:0);
  }
  SurfaceWidth=w;
  SurfaceHeight=h;
  log(Str("STARTUP: rgb555=")+rgb555+" rgb32_bluestart_bit="+rgb32_bluestart_bit+
        " SurfaceWidth="+SurfaceWidth+" SurfaceHeight="+SurfaceHeight);

  palette_prepare(true);
  draw_init_resdependent();

  return true;
}
#undef LOGSECTION
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HRESULT SteemDisplay::Lock()
{
  switch (Method){
    case DISPMETHOD_DD:
    {
      HRESULT DErr;

      if (DDBackSur==NULL) return DDERR_SURFACELOST;

      if (DDBackSur->IsLost()==DDERR_SURFACELOST) return DDERR_SURFACELOST;

      DDBackSurDesc.dwSize=sizeof(DDSURFACEDESC);
      if ((DErr=DDBackSur->Lock(NULL,&DDBackSurDesc,DDLOCK_WAIT | DDLockFlags,NULL))!=DD_OK){
        if (DErr!=DDERR_SURFACELOST && DErr!=DDERR_SURFACEBUSY){
          DDError(T("DirectDraw Lock Error"),DErr);
          Init();
        }
        return DErr;
      }

      draw_line_length=DDBackSurDesc.lPitch;
      draw_mem=LPBYTE(DDBackSurDesc.lpSurface);
      return DD_OK;
    }
    case DISPMETHOD_GDI:
      draw_line_length=GDIBmpLineLength;
      draw_mem=GDIBmpMem;
      return DD_OK;
  }
  return DDERR_GENERIC;
}
//---------------------------------------------------------------------------
void SteemDisplay::Unlock()
{
  if (Method==DISPMETHOD_DD){
    DDBackSur->Unlock(NULL);
  }else if (Method==DISPMETHOD_GDI){
    SetBitmapBits(GDIBmp,GDIBmpSize,GDIBmpMem);
  }
}
//---------------------------------------------------------------------------
void SteemDisplay::VSync()
{
  if (FullScreen==0) return;

  log_to(LOGSECTION_SPEEDLIMIT,Str("SPEED: VSYNC - Starting wait for VBL at ")+(timeGetTime()-run_start_time));

  BOOL Blanking=FALSE;
  DDObj->GetVerticalBlankStatus(&Blanking);
  if (Blanking==FALSE){
    DWORD botline=480-40;
    if (border & 1){
      botline=600-40;
    }else if (using_res_640_400){
      botline=400-40;
    }
    DWORD line;
    HRESULT hRet;
    do{
      hRet=DDObj->GetScanLine(&line);
      if (line>=botline){
        break;
      }else{
//        Sleep(0);
      }
    }while (hRet==DD_OK);
    if (hRet!=DD_OK && hRet!=DDERR_VERTICALBLANKINPROGRESS){
      DDObj->GetVerticalBlankStatus(&Blanking);
      if (Blanking==FALSE) DDObj->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,NULL);
    }
  }
  log_to(LOGSECTION_SPEEDLIMIT,Str("SPEED: VSYNC - Finished waiting for VBL at ")+(timeGetTime()-run_start_time));
}
//---------------------------------------------------------------------------
bool SteemDisplay::Blit()
{
  if (Method==DISPMETHOD_DD){
    HRESULT hRet;
    if (FullScreen){
      if (runstate==RUNSTATE_RUNNING){
        switch (draw_fs_blit_mode){
          case DFSM_FLIP:
            hRet=DDPrimarySur->Flip(NULL,0); //DDFLIP_WAIT);
            break;
          case DFSM_STRAIGHTBLIT:
          {
            hRet=DDPrimarySur->BltFast(draw_blit_source_rect.left,draw_blit_source_rect.top,
                                        DDBackSur,&draw_blit_source_rect,DDBLTFAST_WAIT);
            break;
          }
          case DFSM_STRETCHBLIT:case DFSM_LAPTOP:
          {
            RECT Dest;
            get_fullscreen_rect(&Dest);
            hRet=DDPrimarySur->Blt(&Dest,DDBackSur,&draw_blit_source_rect,DDBLT_WAIT,NULL);
            break;
          }
        }
        if (hRet==DDERR_SURFACELOST){
          hRet=RestoreSurfaces();
          if (hRet!=DD_OK){
            DDError(T("Drawing memory permanently lost"),hRet);
            Init();
          }
        }
      }else{ //not running right now
        HCURSOR OldCur;
        if (BlitHideMouse) OldCur=SetCursor(NULL);
        RECT Dest;
        get_fullscreen_rect(&Dest);
        for (int i=0;i<2;i++){
          hRet=DDPrimarySur->Blt(&Dest,DDBackSur,&draw_blit_source_rect,DDBLT_WAIT,NULL);
          if (hRet==DDERR_SURFACELOST){
            if (i==0) hRet=RestoreSurfaces();
            if (hRet!=DD_OK){
              DDError(T("Drawing memory permanently lost"),hRet);
              Init();
              break;
            }
          }else{
            break;
          }
        }
        if (BlitHideMouse) SetCursor(OldCur);
      }
    }else{
      HCURSOR OldCur;
      if (stem_mousemode==STEM_MOUSEMODE_DISABLED && BlitHideMouse) OldCur=SetCursor(NULL);

      RECT dest;GetClientRect(StemWin,&dest);
      dest.top+=MENUHEIGHT;dest.right-=4;dest.bottom-=4;
      POINT pt={2,2};
      ClientToScreen(StemWin,&pt);
      OffsetRect(&dest,pt.x,pt.y);

      for (int i=0;i<2;i++){
        hRet=DDPrimarySur->Blt(&dest,DDBackSur,&draw_blit_source_rect,DDBLT_WAIT,NULL);
        if (hRet==DDERR_SURFACELOST){
          if (i==0) hRet=RestoreSurfaces();
          if (hRet!=DD_OK){
            DDError(T("Drawing memory permanently lost"),hRet);
            Init();
            break;
          }
        }else{
          break;
        }
      }
      if (stem_mousemode==STEM_MOUSEMODE_DISABLED && BlitHideMouse) SetCursor(OldCur);
    }
    if (hRet==DD_OK) return true;

  }else if (Method==DISPMETHOD_GDI){
    RECT dest;
    GetClientRect(StemWin,&dest);
    HDC dc=GetDC(StemWin);
    SetStretchBltMode(dc,COLORONCOLOR);
    StretchBlt(dc,2,MENUHEIGHT+2,dest.right-4,dest.bottom-(MENUHEIGHT+4),
               GDIBmpDC,draw_blit_source_rect.left,draw_blit_source_rect.top,
               draw_blit_source_rect.right-draw_blit_source_rect.left,
               draw_blit_source_rect.bottom-draw_blit_source_rect.top,SRCCOPY);
    ReleaseDC(StemWin,dc);
    return true;
  }
  return 0;
}
//---------------------------------------------------------------------------
void SteemDisplay::WaitForAsyncBlitToFinish()
{
}
//---------------------------------------------------------------------------
void SteemDisplay::RunStart(bool Temp)
{
  if (FullScreen==0) return;

  if (Temp==0){
    bool ChangeSize=0;
    int w=640,h=400,hz=0;
#ifndef NO_CRAZY_MONITOR
    if (extended_monitor && (em_width<GetScreenWidth() || em_height<GetScreenHeight())){
      ChangeSize=true;
      w=em_width;
      h=em_height;
    }

    if (extended_monitor==0)
#endif
    if (draw_fs_blit_mode!=DFSM_LAPTOP){
      if (prefer_res_640_400 && border==0 && DDDisplayModePossible[2][int(BytesPerPixel==1 ? 0:1)]){
        ChangeSize=true;
        hz=prefer_pc_hz[BytesPerPixel-1][0];
      }
    }
    if (ChangeSize){
      int hz_ok=0;
      if (SetDisplayMode(w,h,BytesPerPixel*8,hz,&hz_ok)==DD_OK){
        if (hz) tested_pc_hz[BytesPerPixel-1][0]=MAKEWORD(hz,hz_ok);
        using_res_640_400=true;
      }else{
        change_fullscreen_display_mode(0);
      }
    }
  }

  DDPrimarySur->SetClipper(NULL);

  ShowAllDialogs(0);
  SetStemMouseMode(STEM_MOUSEMODE_WINDOW);

#ifdef WIN32
  if (DrawLetterboxWithGDI==0) LockWindowUpdate(StemWin);
  while (ShowCursor(0)>=0);
  SetCursor(NULL);
#endif

  Disp.DrawFullScreenLetterbox();
}
//---------------------------------------------------------------------------
void SteemDisplay::RunEnd(bool Temp)
{
  if (FullScreen==0) return;

  if (using_res_640_400 && Temp==0 && bAppActive){
    // Save background
    RECT rcDest;
    get_fullscreen_rect(&rcDest);
    OffsetRect(&rcDest,-rcDest.left,-rcDest.top);
    int w=rcDest.right,h=rcDest.bottom;

    HRESULT hRet;
    IDirectDrawSurface *SaveSur=NULL;
    DDSURFACEDESC SaveSurDesc;

    ZeroMemory(&SaveSurDesc,sizeof(DDSURFACEDESC));
    SaveSurDesc.dwSize=sizeof(DDSURFACEDESC);
    SaveSurDesc.dwFlags=DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    SaveSurDesc.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    SaveSurDesc.dwWidth=w;
    SaveSurDesc.dwHeight=h;
    hRet=DDObj->CreateSurface(&SaveSurDesc,&SaveSur,NULL);
    if (hRet==DD_OK){
      hRet=SaveSur->Blt(&rcDest,DDBackSur,&draw_blit_source_rect,DDBLT_WAIT,NULL);
      if (hRet!=DD_OK){
        SaveSur->Release(); SaveSur=NULL;
      }
    }else{
      SaveSur->Release(); SaveSur=NULL;
    }

    using_res_640_400=0;
    change_fullscreen_display_mode(true);

    if (SaveSur){
      DDBackSur->Blt(&draw_blit_source_rect,SaveSur,NULL,DDBLT_WAIT,NULL);
      SaveSur->Release();
    }
  }
  DDObj->FlipToGDISurface();
  LockWindowUpdate(NULL);
  DDPrimarySur->SetClipper(DDClipper);

  while (ShowCursor(true)<0);

  ShowAllDialogs(true);

  InvalidateRect(StemWin,NULL,true);
}
//---------------------------------------------------------------------------
void SteemDisplay::DrawFullScreenLetterbox()
{
  if (FullScreen==0 || using_res_640_400) return;
  if (draw_fs_blit_mode==DFSM_LAPTOP) return;
#ifndef NO_CRAZY_MONITOR
  if (extended_monitor) return;
#endif

  if (draw_fs_topgap || (border & 1)){
    if (Method==DISPMETHOD_DD){
      DDBLTFX bfx;
      ZeroMemory(&bfx,sizeof(DDBLTFX));
      bfx.dwSize=sizeof(DDBLTFX);
      bfx.dwFillColor=RGB(0,0,0);

      HDC dc=NULL;
      if (DrawLetterboxWithGDI) dc=GetDC(StemWin);

      RECT Dest={0,0,640,draw_fs_topgap};
      if (border & 1){
        Dest.right=800;
        Dest.bottom=(600-400-2*(BORDER_TOP+BORDER_BOTTOM))/2;
      }
      DDBackSur->Blt(&Dest,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bfx);
      if (dc){
        FillRect(dc,&Dest,(HBRUSH)GetStockObject(BLACK_BRUSH));
      }else{
        DDPrimarySur->Blt(&Dest,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bfx);
      }

      if (border & 1){
        Dest.top=600-Dest.bottom;
        Dest.bottom=600;
      }else{
        Dest.top=440;
        Dest.bottom=480;
      }
      DDBackSur->Blt(&Dest,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bfx);
      if (dc){
        FillRect(dc,&Dest,(HBRUSH)GetStockObject(BLACK_BRUSH));
      }else{
        DDPrimarySur->Blt(&Dest,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bfx);
      }

      if (border & 1){
        int SideGap=(800 - (BORDER_SIDE+320+BORDER_SIDE)*2) / 2;

        Dest.top=0;Dest.bottom=600;
        Dest.left=0;Dest.right=SideGap;
        DDBackSur->Blt(&Dest,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bfx);
        if (dc){
          FillRect(dc,&Dest,(HBRUSH)GetStockObject(BLACK_BRUSH));
        }else{
          DDPrimarySur->Blt(&Dest,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bfx);
        }

        Dest.left=800-SideGap;Dest.right=800;
        DDBackSur->Blt(&Dest,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bfx);
        if (dc){
          FillRect(dc,&Dest,(HBRUSH)GetStockObject(BLACK_BRUSH));
        }else{
          DDPrimarySur->Blt(&Dest,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bfx);
        }
      }
      if (dc) ReleaseDC(StemWin,dc);
    }
  }
}
//---------------------------------------------------------------------------
void SteemDisplay::ScreenChange()
{
  draw_end();
  if (Method==DISPMETHOD_DD){
// DX4         if (DDObj->TestCooperativeLevel()!=DDERR_EXCLUSIVEMODEALREADYSET)
    if (DDCreateSurfaces()!=DD_OK) Init();
  }else if (Method==DISPMETHOD_GDI){
    if (InitGDI()==0){
      Init();
    }else{
      Method=DISPMETHOD_GDI;
    }
  }
}
//---------------------------------------------------------------------------
void SteemDisplay::ChangeToFullScreen()
{
  if (CanGoToFullScreen()==0 || FullScreen || DDExclusive) return;

  draw_end();

  if (runstate==RUNSTATE_RUNNING){
    runstate=RUNSTATE_STOPPING;
    PostMessage(StemWin,WM_SYSCOMMAND,SC_MAXIMIZE,0);
  }else if (runstate!=RUNSTATE_STOPPED){ //Keep trying until succeed!
    PostMessage(StemWin,WM_SYSCOMMAND,SC_MAXIMIZE,0);
  }else if (bAppMinimized){
    ShowWindow(StemWin,SW_RESTORE);
    PostMessage(StemWin,WM_SYSCOMMAND,SC_MAXIMIZE,0);
  }else{
    bool MaximizeDiskMan=0;
    if (DiskMan.IsVisible()){
      if (IsIconic(DiskMan.Handle)) ShowWindow(DiskMan.Handle,SW_RESTORE);
      MaximizeDiskMan=DiskMan.FSMaximized;
      SetWindowLong(DiskMan.Handle,GWL_STYLE,(GetWindowLong(DiskMan.Handle,GWL_STYLE) & ~WS_MAXIMIZE) & ~WS_MINIMIZEBOX);
    }

    FullScreen=true;

    DirectoryTree::PopupParent=StemWin;
    
    GetWindowRect(StemWin,&rcPreFS);

    ShowWindow(GetDlgItem(StemWin,106),SW_SHOWNA);

    SetWindowLong(StemWin,GWL_STYLE,WS_VISIBLE);
    int w=640,h=480;
    if (border & 1){
      w=800;h=600;
    }
#ifndef NO_CRAZY_MONITOR
    if (extended_monitor){
      w=em_width;h=em_height;
    }
#endif
    if (draw_fs_blit_mode==DFSM_LAPTOP){
      w=monitor_width;
      h=monitor_height;
    }

    SetWindowPos(StemWin,HWND_TOPMOST,0,0,w,h,0);

    CheckResetDisplay(true);

    ClipWin=CreateWindow("Steem Fullscreen Clip Window","",WS_CHILD | WS_VISIBLE |
                          WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                          0,MENUHEIGHT,w,h-MENUHEIGHT,StemWin,(HMENU)1111,Inst,NULL);
    DDClipper->SetHWnd(0,ClipWin);

#ifndef ONEGAME
    FSQuitBut=CreateWindow("Steem Fullscreen Quit Button","",WS_CHILD,
                            0,h-MENUHEIGHT-14,16,14,ClipWin,HMENU(100),Inst,NULL);
    ToolAddWindow(ToolTip,FSQuitBut,T("Quit"));
#endif

    bool ShowInfoBox=InfoBox.IsVisible();
    for (int n=0;n<nStemDialogs;n++){
      if (DialogList[n]!=&InfoBox){
        DEBUG_ONLY( if (DialogList[n]!=&HistList) ) DialogList[n]->MakeParent(StemWin);
      }
    }
    InfoBox.Hide();

    SetParent(ToolTip,StemWin);

    HRESULT Ret=DDObj->SetCooperativeLevel(StemWin,DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN |
                                            DDSCL_ALLOWREBOOT);
    if (Ret!=DD_OK){
      DDError(T("Can't SetCooperativeLevel to exclusive"),Ret);
      Init();
      return;
    }
    DDExclusive=true;

    if (change_fullscreen_display_mode(0)==DD_OK){

      if (ShowInfoBox) InfoBox.Show();
      if (MaximizeDiskMan){
        SendMessage(DiskMan.Handle,WM_SETREDRAW,0,0);
        ShowWindow(DiskMan.Handle,SW_MAXIMIZE);
        PostMessage(DiskMan.Handle,WM_SETREDRAW,true,0);
      }

      OptionBox.EnableBorderOptions(true);

      SetForegroundWindow(StemWin);
      SetFocus(StemWin);

      palette_convert_all();

      ONEGAME_ONLY( DestroyNotifyInitWin(); )

      PostRunMessage();
    }else{ //back to windowed mode
      ChangeToWindowedMode(true);
    }
  }
}
//---------------------------------------------------------------------------
void SteemDisplay::ChangeToWindowedMode(bool Emergency)
{
  if (DDExclusive==0 && FullScreen==0) return;

  WIN_ONLY( if (FullScreen) TScreenSaver::killTimer(); )

  bool CanChangeNow=true;
  if (runstate==RUNSTATE_RUNNING){
    runstate=RUNSTATE_STOPPING;
    PostMessage(StemWin,WM_COMMAND,MAKEWPARAM(106,BN_CLICKED),(LPARAM)GetDlgItem(StemWin,106));
    CanChangeNow=0;
  }else if (runstate!=RUNSTATE_STOPPED){ //Keep trying until succeed!
    PostMessage(StemWin,WM_COMMAND,MAKEWPARAM(106,BN_CLICKED),(LPARAM)GetDlgItem(StemWin,106));
    CanChangeNow=0;
  }
  if (CanChangeNow || Emergency){
    if (DDExclusive){
      draw_end();

      DDDestroySurfaces();
      DDObj->RestoreDisplayMode();
      DDObj->SetCooperativeLevel(StemWin,DDSCL_NORMAL);
      DDExclusive=0;
    }

    FullScreen=0;

    if (DDCreateSurfaces()!=DD_OK){
      Init();
    }else{
      DDClipper->SetHWnd(0,StemWin);
    }

    CheckResetDisplay(true); // Hide fullscreen reset display
    ToolsDeleteAllChildren(ToolTip,ClipWin);
    DestroyWindow(ClipWin);
    FSQuitBut=NULL;

    DirectoryTree::PopupParent=NULL;

    LockWindowUpdate(NULL);

    if (border==3) overscan=OVERSCAN_MAX_COUNTDOWN; // Make sure auto border turns off
    
    // Sometimes things won't work if you do them immediately after switching to
    // windowed mode, so post a message and resize all the windows back when we can
    PostMessage(StemWin,WM_USER,12,0);
    ChangeToWinTimeOut=timeGetTime()+2000;
  }
}
//---------------------------------------------------------------------------
bool SteemDisplay::CanGoToFullScreen()
{
  if (Method==DISPMETHOD_DD){
    return true;
  }
  return 0;
}
//---------------------------------------------------------------------------
void SteemDisplay::FlipToDialogsScreen()
{
  if (Method==DISPMETHOD_DD) DDObj->FlipToGDISurface();
}
//---------------------------------------------------------------------------
HRESULT SteemDisplay::SetDisplayMode(int w,int h,int bpp,int hz,int *hz_ok)
{
  if (Method==DISPMETHOD_DD && DDExclusive){
    int idx=-1;
    if (w==640 && h==480) idx=0;
    if (w==800 && h==600) idx=1;
    if (w==640 && h==400) idx=2;
    if (idx>=0){
      for (int n=1;n<NUM_HZ;n++){
        if (hz==HzIdxToHz[n]){
          hz=DDClosestHz[idx][int(bpp>8)][n];
          break;
        }
      }
    }
    log_write(Str("PC DISPLAY: Changing mode to ")+w+"x"+h+"x"+bpp+" "+hz+"hz");
    HRESULT Ret=DDObj->SetDisplayMode(w,h,bpp,hz,0);
    if (Ret!=DD_OK){
      log_write("  It failed");
      if (hz_ok) *hz_ok=0;
      Ret=DDObj->SetDisplayMode(w,h,bpp,0,0);
    }else{
      log_write("  Success");
      if (hz_ok) *hz_ok=1;
    }
    if (Ret!=DD_OK){
//      DDError(T("Can't SetDisplayMode"),Ret);
//      Init();
      return Ret;
    }

    if ((Ret=DDCreateSurfaces())!=DD_OK) Init();

    return Ret;
  }
  return DDERR_GENERIC;
}
//---------------------------------------------------------------------------
HRESULT SteemDisplay::RestoreSurfaces()
{
  if (Method==DISPMETHOD_DD){
    draw_end();
    HRESULT hRet=DDPrimarySur->Restore();
    if (hRet==DD_OK){
      hRet=DDBackSur->Restore();
      if (hRet==DD_OK) return hRet;
    }

    return DDCreateSurfaces();
  }
  return DD_OK;
}
//---------------------------------------------------------------------------
void SteemDisplay::Release()
{
  log_to_section(LOGSECTION_SHUTDOWN,"SHUTDOWN: Display::Release - calling draw_end()");
  draw_end();
  if (GDIBmp!=NULL){
    log_to_section(LOGSECTION_SHUTDOWN,"SHUTDOWN: Freeing GDI stuff");
    DeleteDC(GDIBmpDC);   GDIBmpDC=NULL;
    DeleteObject(GDIBmp); GDIBmp=NULL;
    delete[] GDIBmpMem;
  }
  if (DDObj!=NULL){
    if (DDExclusive || FullScreen){
      log_to_section(LOGSECTION_SHUTDOWN,"SHUTDOWN: Calling ChangeToWindowedMode()");
      ChangeToWindowedMode(true);
    }

    log_to_section(LOGSECTION_SHUTDOWN,"SHUTDOWN: Destroying surfaces");
    DDDestroySurfaces();

    if (DDClipper!=NULL){
      log_to_section(LOGSECTION_SHUTDOWN,"SHUTDOWN: Destroying clipper");
      DDClipper->Release();
      DDClipper=NULL;
    }
    log_to_section(LOGSECTION_SHUTDOWN,"SHUTDOWN: Destroying DD object");
    DDObj->Release();
    DDObj=NULL;
  }
  palette_remove();
  Method=DISPMETHOD_NONE;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifdef SHOW_WAVEFORM
void SteemDisplay::DrawWaveform()
{
  HDC dc;
  if (Method==DISPMETHOD_DD){
    if (DDBackSur->GetDC(&dc)!=DD_OK) return;
  }else if (Method==DISPMETHOD_GDI){
    dc=GDIBmpDC;
  }else{
    return;
  }

  int base=shifter_y-10;
  SelectObject(dc,GetStockObject((STpal[0]<0x777) ? WHITE_PEN:BLACK_PEN));
  MoveToEx(dc,0,base-129,0);
  LineTo(dc,shifter_x,base-129);
  MoveToEx(dc,0,base+1,0);
  LineTo(dc,shifter_x,base+1);
  MoveToEx(dc,0,base - temp_waveform_display[0]/2,0);
  for (int x=0;x<draw_blit_source_rect.right;x++){
    LineTo(dc,x,base - temp_waveform_display[x*SHOW_WAVEFORM]/2);
  }
  MoveToEx(dc,temp_waveform_play_counter/SHOW_WAVEFORM,0,0);
  LineTo(dc,temp_waveform_play_counter/SHOW_WAVEFORM,shifter_y);

  if (Method==DISPMETHOD_DD) DDBackSur->ReleaseDC(dc);
}
#endif
//---------------------------------------------------------------------------
void SteemDisplay::ScreenShotCheckFreeImageLoad()
{
  if ((ScreenShotFormat>FIF_BMP && ScreenShotFormat<IF_TOCLIPBOARD) || ScreenShotFormatOpts){
    if (hFreeImage) return;

    hFreeImage=LoadLibrary(RunDir+"\\FreeImage\\FreeImage.dll");
    if (hFreeImage==NULL) hFreeImage=LoadLibrary(RunDir+"\\FreeImage\\FreeImage\\FreeImage.dll");
    if (hFreeImage==NULL) hFreeImage=LoadLibrary("FreeImage.dll");
    if (hFreeImage==NULL) return;

    FreeImage_Initialise=(FI_INITPROC)GetProcAddress(hFreeImage,"_FreeImage_Initialise@4");
    FreeImage_DeInitialise=(FI_DEINITPROC)GetProcAddress(hFreeImage,"_FreeImage_DeInitialise@0");
    FreeImage_ConvertFromRawBits=
          (FI_CONVFROMRAWPROC)GetProcAddress(hFreeImage,"_FreeImage_ConvertFromRawBits@36");
    FreeImage_FIFSupportsExportBPP=
          (FI_SUPPORTBPPPROC)GetProcAddress(hFreeImage,"_FreeImage_FIFSupportsExportBPP@8");
    FreeImage_Save=(FI_SAVEPROC)GetProcAddress(hFreeImage,"_FreeImage_Save@16");
    FreeImage_Free=(FI_FREEPROC)GetProcAddress(hFreeImage,"_FreeImage_Free@4");


    if (FreeImage_Initialise==NULL || FreeImage_DeInitialise==NULL ||
          FreeImage_ConvertFromRawBits==NULL || FreeImage_Save==NULL ||
          FreeImage_FIFSupportsExportBPP==NULL || FreeImage_Free==NULL){
      FreeLibrary(hFreeImage);hFreeImage=NULL;
      return;
    }
    FreeImage_Initialise(TRUE);
  }else{
    if (hFreeImage==NULL) return;

    FreeImage_DeInitialise();
    FreeLibrary(hFreeImage);hFreeImage=NULL;
  }
}
//---------------------------------------------------------------------------
#endif //WIN32

HRESULT SteemDisplay::SaveScreenShot()
{
  Str ShotFile=ScreenShotNextFile;
  ScreenShotNextFile="";
  bool ToClipboard=(int(ScreenShotFormat)==IF_TOCLIPBOARD);

  if (ShotFile.Empty() && ToClipboard==0){

    DWORD Attrib=GetFileAttributes(ScreenShotFol);
    if (Attrib==0xffffffff || (Attrib & FILE_ATTRIBUTE_DIRECTORY)==0) return DDERR_GENERIC;

    Str Exts="bmp";
    WIN_ONLY( if (hFreeImage) Exts=ScreenShotExt; )

    EasyStr FirstWord="Steem_";
    if (FloppyDrive[0].DiskName.NotEmpty()){
      FirstWord=FloppyDrive[0].DiskName;
      if (ScreenShotUseFullName==0){
        char *spc=strchr(FirstWord,' ');
        if (spc) *spc=0;
      }
    }
    bool AddNumExt=true;
    if (ScreenShotUseFullName){
      ShotFile=ScreenShotFol+SLASH+FirstWord+"."+Exts;
      if (Exists(ShotFile)==0) AddNumExt=ScreenShotAlwaysAddNum;
    }
    if (AddNumExt){
      int Num=0;
      do{
        if (++Num >= 1000) return DDERR_GENERIC;
        ShotFile=ScreenShotFol+SLASH+FirstWord+"_"+(EasyStr("000")+Num).Rights(3)+"."+Exts;
      }while (Exists(ShotFile));
    }
  }

  BYTE *SurMem=NULL;
  long SurLineLen;
  int w,h;

#ifdef WIN32
  IDirectDrawSurface *SaveSur=NULL;
  HBITMAP SaveBmp=NULL;

  // Need to create new surfaces so we can blit in the same way we do to the
  // window, just in case image must be stretched. We can't do this ourselves
  // (even if we wanted to) because some video cards will blur.
  if (Method==DISPMETHOD_DD){
    if (DDBackSur==NULL) return DDERR_GENERIC;

    RECT rcDest={0,0,0,0};
    if (ScreenShotMinSize){
      if (border & 1){
        rcDest.right=WinSizeBorder[screen_res][0].x;
        rcDest.bottom=WinSizeBorder[screen_res][0].y;
      }else{
        rcDest.right=WinSize[screen_res][0].x;
        rcDest.bottom=WinSize[screen_res][0].y;
      }
    }else{
      if (FullScreen){
        get_fullscreen_rect(&rcDest);
        OffsetRect(&rcDest,-rcDest.left,-rcDest.top);
      }else{
        GetClientRect(StemWin,&rcDest);
        rcDest.right-=4;rcDest.bottom-=4+MENUHEIGHT;
      }
    }
    w=rcDest.right;h=rcDest.bottom;

    HRESULT hRet;
    DDSURFACEDESC SaveSurDesc;

    ZeroMemory(&SaveSurDesc,sizeof(DDSURFACEDESC));
    SaveSurDesc.dwSize=sizeof(DDSURFACEDESC);
    SaveSurDesc.dwFlags=DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    SaveSurDesc.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    SaveSurDesc.dwWidth=w;
    SaveSurDesc.dwHeight=h;
    hRet=DDObj->CreateSurface(&SaveSurDesc,&SaveSur,NULL);
    if (hRet!=DD_OK) return hRet;

    hRet=SaveSur->Blt(&rcDest,DDBackSur,&draw_blit_source_rect,DDBLT_WAIT,NULL);
    if (hRet!=DD_OK){
      SaveSur->Release();
      return hRet;
    }

    if (SaveSur->IsLost()==DDERR_SURFACELOST){
      SaveSur->Release();
      return hRet;
    }

    if (ToClipboard==0){
      SaveSurDesc.dwSize=sizeof(DDSURFACEDESC);
      hRet=SaveSur->Lock(NULL,&SaveSurDesc,DDLOCK_WAIT | DDLockFlags,NULL);
      if (hRet!=DD_OK){
        SaveSur->Release();
        return hRet;
      }

      SurMem=(BYTE*)SaveSurDesc.lpSurface;
      SurLineLen=SaveSurDesc.lPitch;
    }
  }else if (Method==DISPMETHOD_GDI){
    if (GDIBmp==NULL) return DDERR_GENERIC;

    BITMAP BmpInf;
    RECT rcDest;

    GetClientRect(StemWin,&rcDest);
    w=rcDest.right-4;h=rcDest.bottom-(4+MENUHEIGHT);

    HDC dc=GetDC(NULL);
    SaveBmp=CreateCompatibleBitmap(dc,w,h);
    ReleaseDC(NULL,dc);

    HDC SaveBmpDC=CreateCompatibleDC(NULL);
    SelectObject(SaveBmpDC,SaveBmp);
    SetStretchBltMode(SaveBmpDC,COLORONCOLOR);
    StretchBlt(SaveBmpDC,0,0,w,h,GDIBmpDC,draw_blit_source_rect.left,draw_blit_source_rect.top,
                 draw_blit_source_rect.right-draw_blit_source_rect.left,
                 draw_blit_source_rect.bottom-draw_blit_source_rect.top,SRCCOPY);
    DeleteDC(SaveBmpDC);

    if (ToClipboard==0){
      GetObject(SaveBmp,sizeof(BITMAP),&BmpInf);
      SurLineLen=BmpInf.bmWidthBytes;

      try{
        DWORD BmpBytes=SurLineLen*BmpInf.bmHeight;
        SurMem=new BYTE[BmpBytes];
        GetBitmapBits(SaveBmp,BmpBytes,SurMem);
      }catch(...){
        DeleteObject(SaveBmp);
        return DDERR_GENERIC;
      }
    }
  }else{
    return DDERR_GENERIC;
  }
#elif defined(UNIX)
  // No need to create a new surface here, X can't stretch
  if (Method==DISPMETHOD_X || Method==DISPMETHOD_XSHM){
    if (X_Img==NULL) return DDERR_GENERIC;

    w=draw_blit_source_rect.right;
    h=draw_blit_source_rect.bottom;
    SurMem=LPBYTE(X_Img->data);
    SurLineLen=X_Img->bytes_per_line;
  }else{
    return DDERR_GENERIC;
  }
#endif

	BYTE *Pixels=SurMem;
  bool ConvertPixels=true;
#ifdef WIN32
  if (hFreeImage && ToClipboard==0){
    if (BytesPerPixel>1){
      if (FreeImage_FIFSupportsExportBPP((FREE_IMAGE_FORMAT)ScreenShotFormat,BytesPerPixel*8)){
        ConvertPixels=0;
      }
    }
  }
#endif

  if (ToClipboard){
    ConvertPixels=0;
#ifdef WIN32
    if (Method==DISPMETHOD_DD){
      HDC DDSaveSurDC=NULL;
      HRESULT hRet=SaveSur->GetDC(&DDSaveSurDC);
      if (hRet!=DD_OK){
        SaveSur->Release();
        return hRet;
      }

      HDC dc=GetDC(NULL);
      SaveBmp=CreateCompatibleBitmap(dc,w,h);
      ReleaseDC(NULL,dc);

      HDC SaveBmpDC=CreateCompatibleDC(NULL);

      SelectObject(SaveBmpDC,SaveBmp);
      BitBlt(SaveBmpDC,0,0,w,h,DDSaveSurDC,0,0,SRCCOPY);

      DeleteDC(SaveBmpDC);
      SaveSur->ReleaseDC(DDSaveSurDC);
    }
    if (OpenClipboard(StemWin)){
      EmptyClipboard();

      SetClipboardData(CF_BITMAP,SaveBmp);

      CloseClipboard();
    }
#endif
  }else if (ConvertPixels){
    Pixels=new BYTE[w*h*3 + 16];
    BYTE *pPix=Pixels;
    switch (BytesPerPixel){
      case 1:
      {
        DWORD Col;
        BYTE *pSur=SurMem+((h-1)*SurLineLen),*pSurLineEnd;
        while (pSur>=SurMem){
          pSurLineEnd=pSur+w;
          for (;pSur<pSurLineEnd;pSur++){
            Col=(DWORD)logpal[(*pSur)-1];
            *LPDWORD(pPix)=((Col & 0xff) << 16) | (Col & 0x00ff00) | ((Col & 0xff0000) >> 16);
            pPix+=3;
          }
          pSur-=SurLineLen+w;
        }
        break;
      }
      case 2:
      {
        WORD Col;
        WORD *pSur=LPWORD(SurMem+((h-1)*SurLineLen)),*pSurLineEnd;
        if (rgb555){
          while (LPBYTE(pSur)>=SurMem){
            pSurLineEnd=pSur+w;
            for (;pSur<pSurLineEnd;pSur++){
              Col=*pSur;
              pPix[0]=BYTE((Col << 3) & b11111000);
              pPix[1]=BYTE((Col >> 2) & b11111000);
              pPix[2]=BYTE((Col >> 7) & b11111000);
              pPix+=3;
            }
            pSur=LPWORD(LPBYTE(pSur)-SurLineLen)-w;
          }
        }else{
          while (LPBYTE(pSur)>=SurMem){
            pSurLineEnd=pSur+w;
            for (;pSur<pSurLineEnd;pSur++){
              Col=*pSur;
              pPix[0]=BYTE((Col << 3) & b11111000);
              pPix[1]=BYTE((Col >> 3) & b11111100);
              pPix[2]=BYTE((Col >> 8) & b11111000);
              pPix+=3;
            }
            pSur=LPWORD(LPBYTE(pSur)-SurLineLen)-w;
          }
        }
        break;
      }
      case 3:
      {
        long WidBytes=(w*3+3) & -4;
        BYTE *pSur=SurMem+((h-1)*SurLineLen);
        while (pSur>=SurMem){
          memcpy(pPix,pSur,WidBytes);
          pSur-=SurLineLen;
          pPix+=WidBytes;
        }
        break;
      }
      case 4:
        DWORD *pSur=LPDWORD(SurMem+((h-1)*SurLineLen)),*pSurLineEnd;
        if (rgb32_bluestart_bit){
          while (LPBYTE(pSur)>=SurMem){
            pSurLineEnd=pSur+w;
            for (;pSur<pSurLineEnd;pSur++){
              *LPDWORD(pPix)=(*pSur) >> rgb32_bluestart_bit;
              pPix+=3;
            }
            pSur=LPDWORD(LPBYTE(pSur)-SurLineLen)-w;
          }
        }else{
          while (LPBYTE(pSur)>=SurMem){
            pSurLineEnd=pSur+w;
            for (;pSur<pSurLineEnd;pSur++){
              *LPDWORD(pPix)=*pSur;
              pPix+=3;
            }
            pSur=LPDWORD(LPBYTE(pSur)-SurLineLen)-w;
          }
        }
        break;
    }
  }

  if (ToClipboard==0){
#ifdef WIN32
    if (hFreeImage){
      FIBITMAP *FIBmp;
      if (ConvertPixels==0){
        DWORD r_mask,g_mask,b_mask;
        switch (BytesPerPixel){
          case 2:
            if (rgb555){
              r_mask=MAKEBINW(b11111000,b00000000);
              g_mask=MAKEBINW(b00000111,b11100000);
              b_mask=MAKEBINW(b00000000,b00011111);
            }else{
              r_mask=MAKEBINW(b01111100,b00000000);
              g_mask=MAKEBINW(b00000011,b11100000);
              b_mask=MAKEBINW(b00000000,b00011111);
            }
            break;
          case 3:case 4:
            r_mask=0xff0000 << rgb32_bluestart_bit;
            g_mask=0x00ff00 << rgb32_bluestart_bit;
            b_mask=0x0000ff << rgb32_bluestart_bit;
            break;
        }
        FIBmp=FreeImage_ConvertFromRawBits(SurMem,w,h,SurLineLen,BytesPerPixel*8,
                                          r_mask,g_mask,b_mask,0);
      }else{
        FIBmp=FreeImage_ConvertFromRawBits(Pixels,w,h,w*3,24,
                                          0xff0000,0x00ff00,0x0000ff,true);
      }
      FreeImage_Save((FREE_IMAGE_FORMAT)ScreenShotFormat,FIBmp,ShotFile,ScreenShotFormatOpts);
      FreeImage_Free(FIBmp);
    }else
#endif
    {
      BITMAPINFOHEADER bih;

      bih.biSize=sizeof(BITMAPINFOHEADER);
      bih.biWidth=w;
      bih.biHeight=h;
      WIN_ONLY(	bih.biPlanes=1; )
      WIN_ONLY(	bih.biBitCount=24; )
      UNIX_ONLY( bih.biPlanes_biBitCount=MAKELONG(1,24); )
      bih.biCompression=0 /*BI_RGB*/;
      bih.biSizeImage=0;
      bih.biXPelsPerMeter=0;
      bih.biYPelsPerMeter=0;
      bih.biClrUsed=0;
      bih.biClrImportant=0;

      FILE *f=fopen(ShotFile,"wb");
      if (f){
        // File header
        WORD bfType=19778; //'BM';
        DWORD bfSize=14 /*sizeof(BITMAPFILEHEADER)*/ + sizeof(BITMAPINFOHEADER)+(w*h*3);
        WORD bfReserved1=0;
        WORD bfReserved2=0;
        DWORD bfOffBits=14 /*sizeof(BITMAPFILEHEADER)*/ + sizeof(BITMAPINFOHEADER);

        fwrite(&bfType,sizeof(bfType),1,f);
        fwrite(&bfSize,sizeof(bfSize),1,f);
        fwrite(&bfReserved1,sizeof(bfReserved1),1,f);
        fwrite(&bfReserved2,sizeof(bfReserved2),1,f);
        fwrite(&bfOffBits,sizeof(bfOffBits),1,f);
        fflush(f);

        fwrite(&bih,sizeof(bih),1,f);
        fflush(f);
        fwrite(Pixels,w*h*3,1,f);
        fflush(f);
        fclose(f);
      }
    }
  }

#ifdef WIN32
  if (Method==DISPMETHOD_DD){
    if (ToClipboard==0) SaveSur->Unlock(NULL);
    SaveSur->Release();
  }else if (Method==DISPMETHOD_GDI){
    delete[] SurMem;
  }
  if (SaveBmp) DeleteObject(SaveBmp);
#endif

  if (ConvertPixels) delete[] Pixels;

  return DD_OK;
}
//---------------------------------------------------------------------------
#ifdef IN_MAIN
#ifdef WIN32
bool SteemDisplay::ScreenShotIsFreeImageAvailable()
{
  if (hFreeImage) return true;

  Str Path;
  Path.SetLength(MAX_PATH);
  char *FilNam;
  if (SearchPath(NULL,"FreeImage.dll",NULL,MAX_PATH,Path.Text,&FilNam)>0) return true;
  if (Exists(RunDir+"\\FreeImage\\FreeImage.dll")) return true;
  if (Exists(RunDir+"\\FreeImage\\FreeImage\\FreeImage.dll")) return true;
  return 0;
}
//---------------------------------------------------------------------------
void SteemDisplay::ScreenShotGetFormats(EasyStringList *pSL)
{
  bool FIAvailable=ScreenShotIsFreeImageAvailable();
  pSL->Sort=eslNoSort;
  pSL->Add(T("To Clipboard"),IF_TOCLIPBOARD);
  pSL->Add("BMP",FIF_BMP);
  if (FIAvailable){
    pSL->Add("JPEG (.jpg)",FIF_JPEG);
    pSL->Add("PNG",FIF_PNG);
    pSL->Add("TARGA (.tga)",FIF_TARGA);
    pSL->Add("TIFF",FIF_TIFF);
    pSL->Add("WBMP",FIF_WBMP);
    pSL->Add("PBM",FIF_PBM);
    pSL->Add("PGM",FIF_PGM);
    pSL->Add("PPM",FIF_PPM);
  }
}
//---------------------------------------------------------------------------
void SteemDisplay::ScreenShotGetFormatOpts(EasyStringList *pSL)
{
  bool FIAvailable=ScreenShotIsFreeImageAvailable();
  pSL->Sort=eslNoSort;
  switch (ScreenShotFormat){
    case FIF_BMP:
      if (FIAvailable){
        pSL->Add(T("Normal"),BMP_DEFAULT);
        pSL->Add("RLE",BMP_SAVE_RLE);
      }
      break;
    case FIF_JPEG:
      pSL->Add(T("Superb Quality"),JPEG_QUALITYSUPERB);
      pSL->Add(T("Good Quality"),JPEG_QUALITYGOOD);
      pSL->Add(T("Normal"),JPEG_QUALITYNORMAL);
      pSL->Add(T("Average Quality"),JPEG_QUALITYAVERAGE);
      pSL->Add(T("Bad Quality"),JPEG_QUALITYBAD);
      break;
    case FIF_PBM:case FIF_PGM:case FIF_PPM:
      pSL->Add(T("Binary"),PNM_SAVE_RAW);
      pSL->Add("ASCII",PNM_SAVE_ASCII);
      break;
  }
}
#endif
#endif
//---------------------------------------------------------------------------
void draw_init_resdependent()
{
  if (draw_grille_black<4) draw_grille_black=4;
  make_palette_table(brightness,contrast);
  palette_convert_all();
  if (BytesPerPixel==1) palette_copy();
  if (osd_plasma_pal){
    delete[] osd_plasma_pal; osd_plasma_pal=NULL;
    delete[] osd_plasma;     osd_plasma=NULL;
  }
}
//---------------------------------------------------------------------------
#ifdef UNIX
#include "x/x_display.cpp"
#endif


