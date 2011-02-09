#ifdef WIN32

#ifndef ONEGAME
#define NOTIFYINIT_WIDTH 250
#define NOTIFYINIT_HEIGHT 120
#else
#define NOTIFYINIT_WIDTH (GetSystemMetrics(SM_CXSCREEN))
#define NOTIFYINIT_HEIGHT (GetSystemMetrics(SM_CYSCREEN))
#endif
//---------------------------------------------------------------------------
void CreateNotifyInitWin()
{
  WNDCLASS wc;
  wc.style=CS_NOCLOSE;
  wc.lpfnWndProc=NotifyInitWndProc;
  wc.cbWndExtra=0;
  wc.cbClsExtra=0;
  wc.hInstance=Inst;
  wc.hIcon=hGUIIcon[RC_ICO_APP];
  wc.hCursor=LoadCursor(NULL,IDC_WAIT);
#ifndef ONEGAME
  wc.hbrBackground=ClassBkColour(COLOR_BTNFACE);
#else
  wc.hbrBackground=NULL;
#endif
  wc.lpszMenuName=NULL;
  wc.lpszClassName="Steem Init Window";
  RegisterClass(&wc);

#ifndef ONEGAME
  NotifyWin=CreateWindow("Steem Init Window",T("Steem is Initialising"),WS_SYSMENU,
                          0,0,NOTIFYINIT_WIDTH,NOTIFYINIT_HEIGHT,NULL,NULL,Inst,NULL);
  CentreWindow(NotifyWin,0);
  SetWindowPos(NotifyWin,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
  UpdateWindow(NotifyWin);
#else
  NotifyWinParent=CreateWindow("Steem Init Window","",0,0,0,2,2,NULL,NULL,Inst,NULL);
  NotifyWin=CreateWindowEx(WS_EX_TOPMOST,"Steem Init Window","",WS_POPUP,0,0,NOTIFYINIT_WIDTH,NOTIFYINIT_HEIGHT,
                            NotifyWinParent,NULL,Inst,NULL);
  SetWindowLong(NotifyWin,GWL_STYLE,WS_POPUP);
  MoveWindow(NotifyWin,0,0,NOTIFYINIT_WIDTH,NOTIFYINIT_HEIGHT,0);
  ShowWindow(NotifyWin,SW_SHOW);
  SetWindowPos(NotifyWin,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
  UpdateWindow(NotifyWin);
  SetCursor(NULL);
#endif
}
//---------------------------------------------------------------------------
#ifndef ONEGAME
void SetNotifyInitText(char *NewText)
{
  SendMessage(NotifyWin,WM_USER,12345,(LPARAM)NewText);
  UpdateWindow(NotifyWin);
}
#else
void SetNotifyInitText(char *){}
#endif
//---------------------------------------------------------------------------
LRESULT __stdcall NotifyInitWndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  switch (Mess){
#ifndef ONEGAME
    case WM_CREATE:
    {
      char *Text=new char[200];
      strcpy(Text,T("Please wait..."));
      SetProp(Win,"NotifyText",Text);
      break;
    }
    case WM_PAINT:
    {
      HDC DC;
      RECT rc;
      char *Text;
      SIZE sz;

      GetClientRect(Win,&rc);

      DC=GetDC(Win);

      SelectObject(DC,GetStockObject(DEFAULT_GUI_FONT));

      HBRUSH br=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
      FillRect(DC,&rc,br);
      DeleteObject(br);

      SetBkMode(DC,TRANSPARENT);

      Text=(char*)GetProp(Win,"NotifyText");

      GetTextExtentPoint32(DC,Text,strlen(Text),&sz);
      TextOut(DC,(rc.right-sz.cx)/2,(rc.bottom-sz.cy)/2,Text,strlen(Text));

      ReleaseDC(Win,DC);
      ValidateRect(Win,NULL);
      return 0;
    }
    case WM_USER:
      if (wPar==12345){
        char *Text=(char*)GetProp(Win,"NotifyText"),*NewText=(char*)lPar;
        delete[] Text;

        Text=new char[strlen(NewText)+1];
        strcpy(Text,NewText);
        SetProp(Win,"NotifyText",Text);

        InvalidateRect(Win,NULL,1);
      }
      break;
    case WM_DESTROY:
      delete[] (char*)GetProp(Win,"NotifyText");
      RemoveProp(Win,"NotifyText");
      break;
#else
    case WM_PAINT:
    {
      HDC DC=GetDC(Win);
      RECT rc;
      GetClientRect(Win,&rc);
      FillRect(DC,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));
      ReleaseDC(Win,DC);
      ValidateRect(Win,NULL);
      return 0;
    }
    case WM_SETCURSOR:
      SetCursor(NULL);
      return TRUE;
/*
    case WM_CREATE:
    {
      HBITMAP bmp=LoadBitmap(Inst,RCNUM(2));
      HDC SrcDC=GetDC(NULL);
      HDC dc=CreateCompatibleDC(SrcDC);
      ReleaseDC(NULL,SrcDC);
      SelectObject(dc,bmp);
      SetProp(Win,"BitmapDC",dc);
      SetProp(Win,"BitmapBMP",bmp);
      break;
    }
    case WM_PAINT:
    {
      HDC dc=GetDC(Win);
      BitBlt(dc,0,0,320,200,(HDC)GetProp(Win,"BitmapDC"),0,0,SRCCOPY);
      ReleaseDC(Win,dc);
      ValidateRect(Win,NULL);
      return 0;
    }
    case WM_NCLBUTTONDOWN:case WM_NCRBUTTONDOWN:case WM_NCMBUTTONDOWN:
    case WM_NCLBUTTONUP:case WM_NCRBUTTONUP:case WM_NCMBUTTONUP:
      return 0;
    case WM_DESTROY:
      DeleteDC((HDC)GetProp(Win,"BitmapDC"));
      DeleteObject(GetProp(Win,"BitmapBMP"));
      RemoveProp(Win,"BitmapDC");
      RemoveProp(Win,"BitmapBMP");
      break;
*/
#endif
  }
	return DefWindowProc(Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
void DestroyNotifyInitWin()
{
  if (NotifyWin==NULL) return;

  ShowWindow(NotifyWin,SW_HIDE);
  UpdateWindow(NotifyWin);
  DestroyWindow(NotifyWin);
  ONEGAME_ONLY( DestroyWindow(NotifyWinParent); )
  NotifyWin=NULL;

  UnregisterClass("Steem Init Window",Inst);
}
//---------------------------------------------------------------------------
#endif

#ifdef UNIX
#include "x/x_notifyinit.cpp"
#endif

