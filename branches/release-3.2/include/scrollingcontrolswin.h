#ifndef SCROLLINGCONTROLSWIN_H
#define SCROLLINGCONTROLSWIN_H

#define SCWM_SET WM_USER
#define SCWM_GET WM_USER+1

#define SCWS_HVPOS 0xAACE
#define SCWS_ALLOWDRAG 0xAACF

#define SCWS_HEIGHT 0xFACB
#define SCWS_VLINESIZE 0xFACC
#define SCWS_VOVERLAPSIZE 0xFACD
#define SCWS_VPOS 0xFACE
#define SCWS_VDISABLENOSCROLL 0xFACF

#define SCWS_WIDTH 0xEACB
#define SCWS_HLINESIZE 0xEACC
#define SCWS_HOVERLAPSIZE 0xEACD
#define SCWS_HPOS 0xEACE
#define SCWS_HDISABLENOSCROLL 0xEACF

#define SCWM_GETCONTROLPAGE WM_USER+2
#define SCWM_AUTOSIZE WM_USER+3
#define SCWM_SETTINGCHANGE WM_USER+4
#define SCWM_SETBKCOLOR WM_USER+5
#define SCWM_SETBKCOLOUR WM_USER+5

class ScrollControlWin
{
  static LRESULT WINAPI ScrollWndProc(HWND,UINT,WPARAM,LPARAM);
  static LRESULT WINAPI ControlPageWndProc(HWND,UINT,WPARAM,LPARAM);
  HWND Scroller;
public:
  ScrollControlWin();
  ScrollControlWin(DWORD Style,int x,int y,int w,int h,
               HWND Parent,int Id,HINSTANCE Inst,LPVOID CreateStuff=NULL);
  ScrollControlWin(DWORD ExStyle,DWORD Style,int x,int y,int w,int h,
               HWND Parent,int Id,HINSTANCE Inst,LPVOID CreateStuff=NULL);
  ~ScrollControlWin();
  operator HWND();
  HWND Create(DWORD Style,int x,int y,int w,int h,HWND Parent,int Id,
              HINSTANCE Inst,LPVOID CreateStuff=NULL);
  HWND CreateEx(DWORD ExStyle,DWORD Style,int x,int y,int w,int h,
               HWND Parent,int Id,HINSTANCE Inst,LPVOID CreateStuff=NULL);
  BOOL Destroy();

  void SetHeight(int Val);
  void SetWidth(int Val);
  void SetHLineSize(int Val);
  void SetVLineSize(int Val);
  void SetHOverlapSize(int Val);
  void SetVOverlapSize(int Val);
  void SetHPos(int Val);
  void SetVPos(int Val);
  void SetHDisableNoScroll(bool Val);
  void SetVDisableNoScroll(bool Val);

  void SetPos(int H,int V);
  void SetAllowDrag(bool Val);

  void AutoSize(int x=0,int y=0);
  void AutoSizeHidden(int x=0,int y=0);

  void SetBkColour(COLORREF Col);
  void SetBkColor(COLORREF Col);

  short GetHeight();
  short GetWidth();
  short GetHLineSize();
  short GetVLineSize();
  short GetHOverlapSize();
  short GetVOverlapSize();
  short GetHPos();
  short GetVPos();
  bool GetHDisableNoScroll();
  bool GetVDisableNoScroll();

  bool GetAllowDrag();

  HWND GetControlPage();

  WINDOWPROC GetControlPageWndProc();
  WINDOWPROC SetControlPageWndProc(WINDOWPROC Proc);
  static bool ShouldPassOnMessage(UINT);
};

#endif

