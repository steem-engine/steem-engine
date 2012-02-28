#ifdef IN_EMU
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)
#endif
//---------------------------------------------------------------------------
extern void draw_init_resdependent();

#define DISPMETHOD_NONE 0
#define DISPMETHOD_DD 1
#define DISPMETHOD_GDI 2
#define DISPMETHOD_X 3
#define DISPMETHOD_XSHM 4
#define DISPMETHOD_BE 5

extern "C"
{
EXT int BytesPerPixel INIT(2),rgb32_bluestart_bit INIT(0);
EXT bool rgb555 INIT(0);
EXT int monitor_width,monitor_height; //true size of monitor, for LAPTOP mode.
}

#define MONO_HZ 72 /*71.47*/

#define NUM_HZ 6
#define DISP_MAX_FREQ_LEEWAY 5

#ifdef IN_EMU
int HzIdxToHz[NUM_HZ]={0,50,60,MONO_HZ,100,120};
#else
EXT int HzIdxToHz[NUM_HZ];
#endif
//---------------------------------------------------------------------------
class SteemDisplay
{
private:
#ifdef WIN32
  // DD Only
  HRESULT InitDD();
  static HRESULT WINAPI DDEnumModesCallback(LPDDSURFACEDESC,LPVOID);
  HRESULT DDCreateSurfaces();
  void DDDestroySurfaces();
  HRESULT DDError(char *,HRESULT);

  // GDI Only
  bool InitGDI();

  IDirectDraw2 *DDObj;
  IDirectDrawSurface *DDPrimarySur,*DDBackSur;
  IDirectDrawClipper *DDClipper;
  DDSURFACEDESC DDBackSurDesc;
  DWORD DDLockFlags;
  bool DDBackSurIsAttached,DDExclusive;
  bool DDDisplayModePossible[3][2];
  int DDClosestHz[3][2][NUM_HZ];

  HBITMAP GDIBmp;
  HDC GDIBmpDC;
  BYTE *GDIBmpMem;
  int GDIBmpLineLength;
  DWORD GDIBmpSize;
#elif defined(UNIX)
	bool CheckDisplayMode(DWORD,DWORD,DWORD);

  bool AlreadyWarnedOfBadMode;

  bool InitX();

  XImage *X_Img;

  bool InitXSHM();

#ifndef NO_SHM
  XShmSegmentInfo XSHM_Info;
  bool XSHM_Attached;
#endif

#ifndef NO_XVIDMODE
  int XVM_nModes,XVM_ViewX,XVM_ViewY;
  XF86VidModeModeInfo **XVM_Modes;
  static int XVM_WinProc(void*,Window,XEvent*);
#endif

#endif

public:
  SteemDisplay();
  ~SteemDisplay();

  void SetMethods(int,...);
  HRESULT Init();
  HRESULT Lock();
  void VSync();
  bool Blit();
  void WaitForAsyncBlitToFinish();
  void Unlock();
#ifdef SHOW_WAVEFORM
  void DrawWaveform();
#endif
  void RunStart(bool=0),RunEnd(bool=0);
  void ScreenChange();
  void ChangeToFullScreen(),ChangeToWindowedMode(bool=0);
  void DrawFullScreenLetterbox(),FlipToDialogsScreen();
  bool CanGoToFullScreen();
  HRESULT SetDisplayMode(int,int,int,int=0,int* = NULL);
  HRESULT RestoreSurfaces();
  void Release();
  HRESULT SaveScreenShot();
  bool BorderPossible();

  int Method,UseMethods[5],nUseMethod;
  bool RunOnChangeToWindow;
  int SurfaceWidth,SurfaceHeight;
  Str ScreenShotNextFile;
  int ScreenShotFormat;
  int ScreenShotMinSize;
  bool ScreenShotUseFullName,ScreenShotAlwaysAddNum;
  bool DoAsyncBlit;

#ifdef WIN32
  void ScreenShotCheckFreeImageLoad();
#ifdef IN_MAIN
  bool ScreenShotIsFreeImageAvailable();
  void ScreenShotGetFormats(EasyStringList*);
  void ScreenShotGetFormatOpts(EasyStringList*);
#endif

  HINSTANCE hFreeImage;
  int ScreenShotFormatOpts;
  Str ScreenShotExt;
  bool DrawToVidMem,BlitHideMouse;
  
  DWORD ChangeToWinTimeOut;
  bool DrawLetterboxWithGDI;
#endif

#if defined(UNIX)
	void Surround();

#ifndef NO_SHM
	int SHMCompletion;
	bool asynchronous_blit_in_progress;
#endif

  Window XVM_FullWin;
  bool GoToFullscreenOnRun;
  int XVM_FullW,XVM_FullH;

#endif

};

#ifdef IN_MAIN
SteemDisplay Disp;
#else
extern SteemDisplay Disp;
#endif
//---------------------------------------------------------------------------
#ifdef IN_MAIN

WIN_ONLY( bool TryDD=true; )
#ifdef NO_SHM
UNIX_ONLY( bool TrySHM=false; )
#else
UNIX_ONLY( bool TrySHM=true; )
#endif

#define IF_TOCLIPBOARD 0xfff0

#ifdef WIN32
#include "SteemFreeImage.h"

SET_GUID(CLSID_DirectDraw,			0xD7B70EE0,0x4340,0x11CF,0xB0,0x63,0x00,0x20,0xAF,0xC2,0xCD,0x35 );
SET_GUID(IID_IDirectDraw,			0x6C14DB80,0xA733,0x11CE,0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60 );
SET_GUID(IID_IDirectDraw2,                  0xB3A6F3E0,0x2B43,0x11CF,0xA2,0xDE,0x00,0xAA,0x00,0xB9,0x33,0x56 );

#endif
#endif

#undef EXT
#undef INIT


