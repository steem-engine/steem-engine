#ifndef SCROLLINGCONTROLSWIN_CPP
#define SCROLLINGCONTROLSWIN_CPP

#ifndef WINDOWPROC
#ifdef STRICT
#define WINDOWPROC WNDPROC
#else
#define WINDOWPROC FARPROC
#endif
#endif

#include <scrollingcontrolswin.h>

ScrollControlWin::ScrollControlWin(){ Scroller=NULL; }
ScrollControlWin::ScrollControlWin(DWORD Style,int x,int y,int w,int h,
               HWND Parent,int Id,HINSTANCE Inst,LPVOID CreateStuff)
{
  Scroller=NULL;CreateEx(0,Style,x,y,w,h,Parent,Id,Inst,CreateStuff);
}
ScrollControlWin::ScrollControlWin(DWORD ExStyle,DWORD Style,int x,int y,int w,int h,
             HWND Parent,int Id,HINSTANCE Inst,LPVOID CreateStuff)
{
  Scroller=NULL;CreateEx(ExStyle,Style,x,y,w,h,Parent,Id,Inst,CreateStuff);
}
ScrollControlWin::~ScrollControlWin()
{
  if (Scroller){
    DestroyWindow(Scroller);Scroller=NULL;
  }
}

ScrollControlWin::operator HWND(){ return Scroller; }

HWND ScrollControlWin::Create(DWORD Style,int x,int y,int w,int h,HWND Parent,int Id,
            HINSTANCE Inst,LPVOID CreateStuff)
{
  return CreateEx(0,Style,x,y,w,h,Parent,Id,Inst,CreateStuff);
}
HWND ScrollControlWin::CreateEx(DWORD ExStyle,DWORD Style,int x,int y,int w,int h,
             HWND Parent,int Id,HINSTANCE Inst,LPVOID CreateStuff)
{
  if (Scroller) return Scroller;

  WNDCLASS wc;

  wc.style=0;
  wc.lpfnWndProc=ScrollWndProc;
  wc.cbWndExtra=0;
  wc.cbClsExtra=0;
  wc.hInstance=Inst;
  wc.hIcon=NULL;
  wc.hCursor=LoadCursor(NULL,IDC_ARROW);
  wc.hbrBackground=NULL;
  wc.lpszMenuName=NULL;
  wc.lpszClassName="Scrolling Control Window";
  RegisterClass(&wc);

  wc.style=CS_OWNDC;
  wc.lpfnWndProc=ControlPageWndProc;
  wc.lpszClassName="Control Page Window";
  RegisterClass(&wc);

  if ((Style & WS_VSCROLL)==0 && (Style & WS_HSCROLL)==0) Style|=WS_VSCROLL | WS_HSCROLL;
  Scroller=CreateWindowEx(ExStyle,"Scrolling Control Window","",
                           Style,x,y,w,h,Parent,(HMENU)Id,Inst,CreateStuff);
  if (Scroller==NULL) return NULL;

  SetProp(Scroller,"This",HANDLE(this));
  return Scroller;
}

BOOL ScrollControlWin::Destroy()
{
  BOOL Ret=0;
  if (Scroller){
    Ret=DestroyWindow(Scroller);Scroller=NULL;
  }
  return Ret;
}

void ScrollControlWin::SetHeight(int Val){ if (Scroller) ScrollWndProc(Scroller,SCWM_SET,SCWS_HEIGHT,Val);}
void ScrollControlWin::SetWidth(int Val){ if (Scroller) ScrollWndProc(Scroller,SCWM_SET,SCWS_WIDTH,Val);}
void ScrollControlWin::SetHLineSize(int Val){ if (Scroller) ScrollWndProc(Scroller,SCWM_SET,SCWS_HLINESIZE,Val);}
void ScrollControlWin::SetVLineSize(int Val){ if (Scroller) ScrollWndProc(Scroller,SCWM_SET,SCWS_VLINESIZE,Val);}
void ScrollControlWin::SetHOverlapSize(int Val){ if (Scroller) ScrollWndProc(Scroller,SCWM_SET,SCWS_HOVERLAPSIZE,Val);}
void ScrollControlWin::SetVOverlapSize(int Val){ if (Scroller) ScrollWndProc(Scroller,SCWM_SET,SCWS_VOVERLAPSIZE,Val);}
void ScrollControlWin::SetHPos(int Val){ if (Scroller) ScrollWndProc(Scroller,SCWM_SET,SCWS_HPOS,Val);}
void ScrollControlWin::SetVPos(int Val){ if (Scroller) ScrollWndProc(Scroller,SCWM_SET,SCWS_VPOS,Val);}
void ScrollControlWin::SetHDisableNoScroll(bool Val){ if (Scroller) ScrollWndProc(Scroller,SCWM_SET,SCWS_HDISABLENOSCROLL,Val);}
void ScrollControlWin::SetVDisableNoScroll(bool Val){ if (Scroller) ScrollWndProc(Scroller,SCWM_SET,SCWS_VDISABLENOSCROLL,Val);}

void ScrollControlWin::SetPos(int H,int V){ if (Scroller) ScrollWndProc(Scroller,SCWM_SET,SCWS_HVPOS,WORD(H) | (WORD(V) << 16));}
void ScrollControlWin::SetAllowDrag(bool Val){ if (Scroller) ScrollWndProc(Scroller,SCWM_SET,SCWS_ALLOWDRAG,Val);}

void ScrollControlWin::AutoSize(int x,int y){ if (Scroller) ScrollWndProc(Scroller,SCWM_AUTOSIZE,0,LPARAM((WORD(y) << 16) | WORD(x)));}
void ScrollControlWin::AutoSizeHidden(int x,int y){ if (Scroller) ScrollWndProc(Scroller,SCWM_AUTOSIZE,1,LPARAM((WORD(y) << 16) | WORD(x)));}

void ScrollControlWin::SetBkColour(COLORREF Col){ if (Scroller) ScrollWndProc(Scroller,SCWM_SETBKCOLOUR,0,Col);}
void ScrollControlWin::SetBkColor(COLORREF Col){ if (Scroller) ScrollWndProc(Scroller,SCWM_SETBKCOLOR,0,Col);}

short ScrollControlWin::GetHeight(){ if (Scroller) return (short)ScrollWndProc(Scroller,SCWM_GET,SCWS_HEIGHT,0); else return 0;}
short ScrollControlWin::GetWidth(){ if (Scroller) return (short)ScrollWndProc(Scroller,SCWM_GET,SCWS_WIDTH,0); else return 0;}
short ScrollControlWin::GetHLineSize(){ if (Scroller) return (short)ScrollWndProc(Scroller,SCWM_GET,SCWS_HLINESIZE,0); else return 0;}
short ScrollControlWin::GetVLineSize(){ if (Scroller) return (short)ScrollWndProc(Scroller,SCWM_GET,SCWS_VLINESIZE,0); else return 0;}
short ScrollControlWin::GetHOverlapSize(){ if (Scroller) return (short)ScrollWndProc(Scroller,SCWM_GET,SCWS_HOVERLAPSIZE,0); else return 0;}
short ScrollControlWin::GetVOverlapSize(){ if (Scroller) return (short)ScrollWndProc(Scroller,SCWM_GET,SCWS_VOVERLAPSIZE,0); else return 0;}
short ScrollControlWin::GetHPos(){ if (Scroller) return (short)ScrollWndProc(Scroller,SCWM_GET,SCWS_HPOS,0); else return 0;}
short ScrollControlWin::GetVPos(){ if (Scroller) return (short)ScrollWndProc(Scroller,SCWM_GET,SCWS_VPOS,0); else return 0;}
bool ScrollControlWin::GetHDisableNoScroll(){ if (Scroller) return (bool)ScrollWndProc(Scroller,SCWM_GET,SCWS_HDISABLENOSCROLL,0); else return 0;}
bool ScrollControlWin::GetVDisableNoScroll(){ if (Scroller) return (bool)ScrollWndProc(Scroller,SCWM_GET,SCWS_VDISABLENOSCROLL,0); else return 0;}

bool ScrollControlWin::GetAllowDrag(){ if (Scroller) return (bool)ScrollWndProc(Scroller,SCWM_GET,SCWS_ALLOWDRAG,0);else return 0;}

HWND ScrollControlWin::GetControlPage(){ if (Scroller) return (HWND)ScrollWndProc(Scroller,SCWM_GETCONTROLPAGE,0,0); else return 0;}

WINDOWPROC ScrollControlWin::GetControlPageWndProc(){if (Scroller) return (WINDOWPROC)GetWindowLong((HWND)ScrollWndProc(Scroller,SCWM_GETCONTROLPAGE,0,0),GWL_WNDPROC); else return NULL;}
WINDOWPROC ScrollControlWin::SetControlPageWndProc(WINDOWPROC Proc){if (Scroller) return (WINDOWPROC)SetWindowLong((HWND)ScrollWndProc(Scroller,SCWM_GETCONTROLPAGE,0,0),GWL_WNDPROC,(long)Proc); else return NULL;}
//---------------------------------------------------------------------------
bool ScrollControlWin::ShouldPassOnMessage(UINT Mess)
{
  switch (Mess){
    case WM_COMMAND:
    case DM_GETDEFID:
    case WM_NOTIFY:case WM_NOTIFYFORMAT:
    case WM_DELETEITEM:
    case WM_DRAWITEM:case WM_MEASUREITEM:
    case WM_HSCROLL:case WM_VSCROLL:
    case WM_VKEYTOITEM:case WM_CHARTOITEM:
    case WM_CTLCOLORBTN:case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
      return true;
  }
  return 0;
}
//---------------------------------------------------------------------------
LRESULT WINAPI ScrollControlWin::ScrollWndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  switch (Mess){
    case WM_CREATE:
    {
      HWND ControlPage=CreateWindowEx(WS_EX_NOPARENTNOTIFY,"Control Page Window","",
                                       WS_CHILD | WS_VISIBLE,0,0,200,200,
                                       Win,(HMENU)100,(HINSTANCE)GetWindowLong(Win,GWL_HINSTANCE),NULL);
      if (ControlPage==NULL) return 1;

      SetProp(Win,"ControlPage",ControlPage);

      SetProp(Win,"Height",HANDLE(600));
      SetProp(Win,"MaxPosY",0);
      SetProp(Win,"LineSizeY",HANDLE(1));
      SetProp(Win,"OverlapSizeY",HANDLE(10));
      SetProp(Win,"DisableNoScrollY",0);

      SetProp(Win,"Width",HANDLE(300));
      SetProp(Win,"MaxPosX",0);
      SetProp(Win,"LineSizeX",HANDLE(1));
      SetProp(Win,"OverlapSizeX",HANDLE(10));
      SetProp(Win,"DisableNoScrollX",0);

      SetProp(Win,"AllowDrag",HANDLE(1));
      SetProp(Win,"This",HANDLE(0));

      SetScrollPos(Win,SB_HORZ,0,0);
      SetScrollPos(Win,SB_VERT,0,0);

      ScrollWndProc(Win,SCWM_SETTINGCHANGE,1234,1234);
      break;
    }
    case WM_PARENTNOTIFY:
      if (LOWORD(wPar)==WM_CREATE){
        SetParent(HWND(lPar),(HWND)GetProp(Win,"ControlPage"));
      }
      break;
    case WM_SIZE:
      ScrollWndProc(Win,SCWM_SETTINGCHANGE,1234,1234);
      break;
    case SCWM_SETTINGCHANGE:
      if (wPar==1234 && lPar==1234){
        RECT rc;
        SCROLLINFO si;
        si.cbSize=sizeof(SCROLLINFO);
        for (int v=0;v<2;v++){
          GetClientRect(Win,&rc);

          si.nPage=max((v ? rc.bottom:rc.right)-int(GetProp(Win,LPCTSTR(v ? "OverlapSizeY":"OverlapSizeX"))),5l);

          si.nMin=0;

          int MaxPos=int(GetProp(Win,LPCTSTR(v ? "Height":"Width"))) - (v ? rc.bottom:rc.right);
          SetProp(Win,LPCTSTR(v ? "MaxPosY":"MaxPosX"),HANDLE(max(MaxPos,0)));
          si.nMax=MaxPos+si.nPage-1;

          si.fMask=SIF_RANGE | SIF_PAGE;
          if (GetProp(Win,LPCTSTR(v ? "DisableNoScrollY":"DisableNoScrollX"))) si.fMask|=SIF_DISABLENOSCROLL;
          SetScrollInfo(Win,v ? SB_VERT:SB_HORZ,&si,true);
        }
        GetClientRect(Win,&rc);
        short OffsetX=(short)-GetScrollPos(Win,SB_HORZ),OffsetY=(short)-GetScrollPos(Win,SB_VERT);
        MoveWindow((HWND)GetProp(Win,"ControlPage"),OffsetX,OffsetY,rc.right-OffsetX,rc.bottom-OffsetY,true);
      }
      break;
    case WM_VSCROLL:case WM_HSCROLL:
      if (LOWORD(wPar)!=SB_ENDSCROLL){
        bool v=(Mess==WM_VSCROLL);

        SCROLLINFO si;
        si.cbSize=sizeof(SCROLLINFO);
        si.fMask=SIF_PAGE | SIF_POS;
        GetScrollInfo(Win,v ? SB_VERT:SB_HORZ,&si);

        switch (LOWORD(wPar)){
          case SB_TOP:
            si.nPos=0;
            break;
          case SB_BOTTOM:
            si.nPos=short((int)GetProp(Win,LPCTSTR(v ? "MaxPosY":"MaxPosX")));
            break;
          case SB_LINEUP:
            si.nPos-=short((int)GetProp(Win,LPCTSTR(v ? "LineSizeY":"LineSizeX")));
            break;
          case SB_LINEDOWN:
            si.nPos+=short((int)GetProp(Win,LPCTSTR(v ? "LineSizeY":"LineSizeX")));
            break;
          case SB_PAGEUP:
            si.nPos-=si.nPage;
            break;
          case SB_PAGEDOWN:
            si.nPos+=si.nPage;
            break;
          case SB_THUMBPOSITION:
          case SB_THUMBTRACK:
            si.nPos=HIWORD(wPar);
            break;
        }
        ScrollWndProc(Win,SCWM_SET,v ? SCWS_VPOS:SCWS_HPOS,si.nPos);
      }
      return 0;
    case SCWM_SET:
    {
      bool v=(HIBYTE(wPar)==0xFA);
      switch (wPar){
        case SCWS_VPOS:case SCWS_HPOS:case SCWS_HVPOS:
        {
          if (wPar!=SCWS_HVPOS){
            lPar=bound(lPar,0,short((int)GetProp(Win,LPCTSTR(v ? "MaxPosY":"MaxPosX"))));
            SetScrollPos(Win,v ? SB_VERT:SB_HORZ,LOWORD(lPar),true);
          }else{
            WORD NewX=min(LOWORD(lPar),WORD((int)GetProp(Win,"MaxPosX"))),
                 NewY=min(HIWORD(lPar),WORD((int)GetProp(Win,"MaxPosY")));
            SetScrollPos(Win,SB_HORZ,NewX,true);
            SetScrollPos(Win,SB_VERT,NewY,true);
          }

          RECT rc;
          GetClientRect(Win,&rc);
          HWND ControlPage=(HWND)GetProp(Win,"ControlPage");
          short OffsetX=(short)-GetScrollPos(Win,SB_HORZ),OffsetY=(short)-GetScrollPos(Win,SB_VERT);
          MoveWindow(ControlPage,OffsetX,OffsetY,rc.right-OffsetX,rc.bottom-OffsetY,true);
          UpdateWindow(Win);
          UpdateWindow(ControlPage);

          return 0;
        }
        case SCWS_HEIGHT:case SCWS_WIDTH:
          SetProp(Win,LPCTSTR(v ? "Height":"Width"),(HANDLE)bound(lPar,1,32000));
          ScrollWndProc(Win,SCWM_SETTINGCHANGE,1234,1234);
          return 0;
        case SCWS_VOVERLAPSIZE:case SCWS_HOVERLAPSIZE:
          SetProp(Win,LPCTSTR(v ? "OverlapSizeY":"OverlapSizeX"),(HANDLE)bound(lPar,0,32000));
          ScrollWndProc(Win,SCWM_SETTINGCHANGE,1234,1234);
          return 0;
        case SCWS_VLINESIZE:case SCWS_HLINESIZE:
          SetProp(Win,LPCTSTR(v ? "LineSizeY":"LineSizeX"),(HANDLE)bound(lPar,1,32000));
          return 0;
        case SCWS_VDISABLENOSCROLL:case SCWS_HDISABLENOSCROLL:
          SetProp(Win,LPCTSTR(v ? "DisableNoScrollY":"DisableNoScrollX"),HANDLE(lPar));
          ShowScrollBar(Win,v ? SB_VERT:SB_HORZ,lPar);
          return 0;
        case SCWS_ALLOWDRAG:
          SetProp(Win,"AllowDrag",HANDLE(lPar));
          return 0;
      }
      break;
    }
    case SCWM_GET:
    {
      bool v=(HIBYTE(wPar)==0xFA);
      switch (wPar){
        case SCWS_VPOS:case SCWS_HPOS:
          return GetScrollPos(Win,v ? SB_VERT:SB_HORZ);
        case SCWS_HEIGHT:case SCWS_WIDTH:
          return (LRESULT)GetProp(Win,LPCTSTR(v ? "Height":"Width"));
        case SCWS_VLINESIZE:case SCWS_HLINESIZE:
          return (LRESULT)GetProp(Win,LPCTSTR(v ? "LineSizeY":"LineSizeX"));
        case SCWS_VOVERLAPSIZE:case SCWS_HOVERLAPSIZE:
          return (LRESULT)GetProp(Win,LPCTSTR(v ? "OverlapSizeY":"OverlapSizeX"));
        case SCWS_VDISABLENOSCROLL:case SCWS_HDISABLENOSCROLL:
          return (bool)GetProp(Win,LPCTSTR(v ? "DisableNoScrollY":"DisableNoScrollX"));
        case SCWS_ALLOWDRAG:
          return (bool)GetProp(Win,"AllowDrag");
      }
      break;
    }
    case SCWM_GETCONTROLPAGE:
      return (LRESULT)GetProp(Win,"ControlPage");
    case SCWM_AUTOSIZE:
    {
      HWND ControlPage=(HWND)GetProp(Win,"ControlPage");
      HWND Child=GetWindow(ControlPage,GW_CHILD);
      int MaxR=-99999,MaxB=-99999;
      POINT WinOrigin={0,0};
      ClientToScreen(ControlPage,&WinOrigin);
      RECT rc;
      while (Child!=NULL){
        if ((GetWindowLong(Child,GWL_STYLE) & WS_VISIBLE) || wPar==1){
          GetWindowRect(Child,&rc);
          if (rc.right>MaxR)  MaxR=rc.right;
          if (rc.bottom>MaxB) MaxB=rc.bottom;
        }
        Child=GetWindow(Child,GW_HWNDNEXT);
      }
      if (MaxR>0){
        MaxR-=WinOrigin.x;
        MaxB-=WinOrigin.y;

        ScrollWndProc(Win,SCWM_SET,SCWS_WIDTH,MaxR+LOWORD(lPar));
        ScrollWndProc(Win,SCWM_SET,SCWS_HEIGHT,MaxB+HIWORD(lPar));
      }else{
        ScrollWndProc(Win,SCWM_SET,SCWS_HEIGHT,1);
        ScrollWndProc(Win,SCWM_SET,SCWS_WIDTH,1);
      }
      return 0;
    }
    case SCWM_SETBKCOLOUR:
    {
      HWND ControlPage=(HWND)GetProp(Win,"ControlPage");
      DeleteObject(GetProp(ControlPage,"BackgroundBrush"));
      SetProp(ControlPage,"BackgroundBrush",CreateSolidBrush(lPar));
      InvalidateRect(ControlPage,NULL,true);
      return 0;
    }
    case WM_DESTROY:
    {
      RemoveProp(Win,"ControlPage");

      RemoveProp(Win,"MaxPosY");
      RemoveProp(Win,"LineSizeY");
      RemoveProp(Win,"OverlapSizeY");
      RemoveProp(Win,"DisableNoScrollY");
      RemoveProp(Win,"Height");

      RemoveProp(Win,"MaxPosX");
      RemoveProp(Win,"LineSizeX");
      RemoveProp(Win,"OverlapSizeX");
      RemoveProp(Win,"DisableNoScrollX");
      RemoveProp(Win,"Width");

      RemoveProp(Win,"AllowDrag");

      ScrollControlWin *This=(ScrollControlWin*)GetProp(Win,"This");
      if (This) This->Scroller=NULL;

      RemoveProp(Win,"This");
      break;
    }
    default:
      if (ShouldPassOnMessage(Mess)) return SendMessage(GetParent(Win),Mess,wPar,lPar);
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
LRESULT WINAPI ScrollControlWin::ControlPageWndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  switch (Mess){
    case WM_CREATE:
      SetProp(Win,"BackgroundBrush",CreateSolidBrush(GetSysColor(COLOR_3DFACE)));
      SetProp(Win,"DragX",0);
      SetProp(Win,"DragY",0);
      break;
    case WM_ERASEBKGND:
    {
      RECT rc;
      GetClientRect(Win,&rc);
      FillRect((HDC)wPar,&rc,(HBRUSH)GetProp(Win,"BackgroundBrush"));
      return 1;
    }
    case WM_LBUTTONDOWN:case WM_RBUTTONDOWN:case WM_MBUTTONDOWN:
      if ( GetProp(GetParent(Win),"AllowDrag") &&
          (GetProp(GetParent(Win),"MaxPosX") || GetProp(GetParent(Win),"MaxPosY")) ){
        SetCapture(Win);
        SetProp(Win,"DragX",HANDLE((int)LOWORD(lPar)));
        SetProp(Win,"DragY",HANDLE((int)HIWORD(lPar)));
        SetCursor(LoadCursor(NULL,IDC_CROSS));
      }
      break;
    case WM_LBUTTONUP:case WM_RBUTTONUP:case WM_MBUTTONUP:
      if (GetCapture()==Win) ReleaseCapture();
      break;
    case WM_MOUSEMOVE:
      if (GetCapture()==Win){
        HWND Scroller=GetParent(Win);

        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(Scroller,&pt);

        RECT rc;
        GetClientRect(Scroller,&rc);

        WORD NewX=(WORD)-min(pt.x-int(GetProp(Win,"DragX")),0l),NewY=(WORD)-min(pt.y-int(GetProp(Win,"DragY")),0l);
        ScrollWndProc(Scroller,SCWM_SET,SCWS_HVPOS,NewX | (NewY << 16));
      }
      break;
    case WM_SETCURSOR:
      if (GetCapture()==Win){
        if (DefWindowProc(Win,Mess,wPar,lPar)==0){
          SetCursor(LoadCursor(NULL,IDC_CROSS));
        }
        return 0;
      }
      break;
    case WM_DESTROY:
      DeleteObject(GetProp(Win,"BackgroundBrush"));
      RemoveProp(Win,"BackgroundBrush");
      RemoveProp(Win,"DragX");
      RemoveProp(Win,"DragY");
      break;
    default:
      if (ShouldPassOnMessage(Mess)) return SendMessage(GetParent(GetParent(Win)),Mess,wPar,lPar);
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}

#endif
