/*---------------------------------------------------------------------------
FILE: controls.cpp
MODULE: Steem
DESCRIPTION: Custom controls used by Steem's GUI.
---------------------------------------------------------------------------*/

#ifdef WIN32

#define PBS_RIGHTCLICK 1
#define PBS_DBLCLK 2
#define BM_GETCLICKBUTTON 0x00F8
#define BM_SETCLICKBUTTON 0x00FA
#define BM_RELOADICON 0x00F9
//---------------------------------------------------------------------------
LRESULT __stdcall PicButton_WndProc(HWND,UINT,WPARAM,LPARAM);
LRESULT __stdcall PathDisplay_WndProc(HWND,UINT,WPARAM,LPARAM);
LRESULT __stdcall STCharChoose_WndProc(HWND,UINT,WPARAM,LPARAM);
LRESULT __stdcall HyperLinkWndProc(HWND,UINT,WPARAM,LPARAM);
LRESULT __stdcall TextDisplayGroupBox_WndProc(HWND,UINT,WPARAM,LPARAM);

typedef struct{
  HICON Ico,ShadowIco;
  int Width,Height;
}PicButtonInfo;

HWND PicButton_Over=NULL;
UINT PicButton_TimerID;
//---------------------------------------------------------------------------
void CALLBACK PicButton_TimerProc(HWND,UINT,UINT,DWORD)
{
  if (PicButton_Over) SendMessage(PicButton_Over,WM_TIMER,0,0);
}
//---------------------------------------------------------------------------
void RegisterSteemControls()
{
  WNDCLASS wnd;
  wnd.style=CS_DBLCLKS;
  wnd.lpfnWndProc=PicButton_WndProc;
  wnd.cbWndExtra=0;
  wnd.cbClsExtra=0;
  wnd.hInstance=Inst;
  wnd.hIcon=NULL;
  wnd.hCursor=LoadCursor(NULL,IDC_ARROW);
  wnd.hbrBackground=NULL;
  wnd.lpszMenuName=NULL;
  wnd.lpszClassName="Steem Flat PicButton";
  RegisterClass(&wnd);

  PicButton_TimerID=SetTimer(NULL,0,100,PicButton_TimerProc);

  wnd.lpfnWndProc=PathDisplay_WndProc;
  wnd.lpszClassName="Steem Path Display";
  RegisterClass(&wnd);

  wnd.lpfnWndProc=STCharChoose_WndProc;
  wnd.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
  wnd.lpszClassName="Steem ST Character Chooser";
  RegisterClass(&wnd);

  wnd.lpfnWndProc=TextDisplayGroupBox_WndProc;
  wnd.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
  wnd.lpszClassName="Steem Text Display";
  RegisterClass(&wnd);

  wnd.hbrBackground=NULL;

  wnd.lpfnWndProc=HyperLinkWndProc;
  wnd.hInstance=Inst;
  wnd.hCursor=NULL;
  wnd.lpszClassName="Steem HyperLink";
  RegisterClass(&wnd);
}
//---------------------------------------------------------------------------
void UnregisterSteemControls()
{
  UnregisterClass("Steem Flat PicButton",Inst);
  KillTimer(NULL,PicButton_TimerID);
  UnregisterClass("Steem Path Display",Inst);
  UnregisterClass("Steem ST Character Chooser",Inst);
  UnregisterClass("Steem HyperLink",Inst);
  UnregisterClass("Steem Text Display",Inst);
}
//---------------------------------------------------------------------------
#define GET_THIS Inf=(PicButtonInfo*)GetProp(Win,"PicInf");

LRESULT __stdcall PicButton_WndProc(HWND Win,UINT Mess,UINT wPar,long lPar)
{
  bool DoPaint=0,CheckState=0;
  PicButtonInfo *Inf;
  switch (Mess){
    case WM_SETTEXT:
      DefWindowProc(Win,Mess,wPar,lPar);
    case BM_RELOADICON:
      GET_THIS;
      if (Inf){
        DestroyIcon(Inf->ShadowIco);
        delete Inf;
      }
    case WM_CREATE:
    {
      HICON Ico;

      int Len=DefWindowProc(Win,WM_GETTEXTLENGTH,0,0)+1;
      char *Text=new char[Len+1];
      DefWindowProc(Win,WM_GETTEXT,Len,(long)Text);
      Ico=hGUIIcon[min(atoi(Text),RC_NUM_ICONS-1)];

      Inf=NULL;
      if (Ico){
        ICONINFO ii;
        GetIconInfo(Ico,&ii);
        BITMAP bi;
        GetObject(ii.hbmColor,sizeof(BITMAP),&bi);

        Inf=new PicButtonInfo;
        Inf->Ico=Ico;
        Inf->Width=bi.bmWidth;
        Inf->Height=bi.bmHeight;

        HDC ScrDC=GetDC(NULL);

        HDC ColDC=CreateCompatibleDC(ScrDC);
        HBITMAP ColBmp=CreateCompatibleBitmap(ScrDC,Inf->Width,Inf->Height);
        SelectObject(ColDC,ColBmp);
        RECT rc={0,0,Inf->Width,Inf->Height};
        FillRect(ColDC,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));

        HDC MaskDC=CreateCompatibleDC(ScrDC);
        SelectObject(MaskDC,ii.hbmMask);

        DWORD ShadowCol=GetSysColor(COLOR_BTNSHADOW);
        for (int x=0;x<Inf->Width;x++){
          for (int y=0;y<Inf->Height;y++){
            if (GetPixel(MaskDC,x,y)==0) SetPixel(ColDC,x,y,ShadowCol);
          }
        }

        DeleteDC(MaskDC);
        DeleteDC(ColDC);
        ReleaseDC(NULL,ScrDC);

        ICONINFO new_ii;
        new_ii.fIcon=true;
        new_ii.hbmColor=ColBmp;
        new_ii.hbmMask=ii.hbmMask;
        Inf->ShadowIco=CreateIconIndirect(&new_ii);

        DeleteObject(ColBmp);

        DeleteObject(ii.hbmColor);
        DeleteObject(ii.hbmMask);
      }

      SetProp(Win,"PicInf",Inf);

      delete[] Text;

      if (Mess==WM_CREATE){
        SetPropI(Win,"State",0);
        SetPropI(Win,"ClickedIn",0);
        SetPropI(Win,"Checked",0);
        SetPropI(Win,"ClickButton",0);
      }else{
        InvalidateRect(Win,NULL,true);
        return 0;
      }
      break;
    }
    case WM_KEYDOWN:
      if (wPar!=VK_SPACE) break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
      if ((Mess==WM_RBUTTONDOWN || Mess==WM_RBUTTONDBLCLK) &&
            (GetWindowLong(Win,GWL_STYLE) & PBS_RIGHTCLICK)==0) break;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
      SetPropI(Win,"ClickedIn",1);
      SetCapture(Win);
      if (GetPropI(Win,"State")!=1){
        SetPropI(Win,"State",1);
        DoPaint=true;
      }
      switch (Mess){
        case WM_LBUTTONDOWN:case WM_LBUTTONDBLCLK: SetPropI(Win,"ClickButton",1); break;
        case WM_RBUTTONDOWN:case WM_RBUTTONDBLCLK: SetPropI(Win,"ClickButton",2); break;
        case WM_MBUTTONDOWN: SetPropI(Win,"ClickButton",3); break;
        default: SetPropI(Win,"ClickButton",0);
      }
      if (Mess==WM_LBUTTONDBLCLK || Mess==WM_RBUTTONDBLCLK && (GetWindowLong(Win,GWL_STYLE) & PBS_DBLCLK)){
        PostMessage(GetParent(Win),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(Win),BN_DBLCLK),(LPARAM)Win);
      }else{
        PostMessage(GetParent(Win),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(Win),BN_PUSHED),(LPARAM)Win);
      }
      break;

    case WM_KEYUP:
      if (wPar!=VK_SPACE) break;
    case WM_RBUTTONUP:
    case WM_LBUTTONUP:
      if (GetPropI(Win,"ClickedIn")){
        ReleaseCapture();
        SetPropI(Win,"ClickedIn",0);
        RECT rc;
        GetClientRect(Win,&rc);
        PostMessage(GetParent(Win),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(Win),BN_UNPUSHED),(LPARAM)Win);
        if (LOWORD(lPar)<rc.right && HIWORD(lPar)<rc.bottom || Mess==WM_KEYUP){
          SendMessage(GetParent(Win),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(Win),BN_CLICKED),(LPARAM)Win);
        }
      }
      CheckState=true;
      break;

    case WM_ENABLE:
      DoPaint=true;
    case WM_TIMER:case WM_MOUSEMOVE:
    case WM_SETFOCUS:case WM_KILLFOCUS:
      CheckState=true;
      break;

    case WM_PAINT:
      DoPaint=true;
      break;
    case BM_SETCHECK:
      SetPropI(Win,"Checked",wPar);
      CheckState=true;
      if (lPar) DoPaint=true;
      break;
    case BM_GETCHECK:
      return GetPropI(Win,"Checked");
    case BM_GETCLICKBUTTON:
      return (LRESULT)GetProp(Win,"ClickButton");
    case BM_SETCLICKBUTTON:
      return (LRESULT)SetPropI(Win,"ClickButton",wPar);
    case WM_DESTROY:
      if (PicButton_Over==Win) PicButton_Over=NULL;
      GET_THIS;
      if (Inf){
        DestroyIcon(Inf->ShadowIco);
        delete Inf;
      }
      RemoveProps(Win,"ClickButton","PicInf","Checked","ClickedIn","State",NULL);
      break;
  }
  if (CheckState){
    DWORD OldState=GetPropI(Win,"State");
    DWORD NewState=DWORD((GetFocus()==Win && IsWindowEnabled(Win)) ? 2:0);
    bool Over=0;
    if (GetPropI(Win,"Checked")==0){
      if (IsWindowEnabled(Win)){
        POINT pt;
        RECT rc;

        GetCursorPos(&pt);
        ScreenToClient(Win,&pt);
        GetClientRect(Win,&rc);
        if (pt.x>=0 && pt.x<rc.right && pt.y>=0 && pt.y<rc.bottom){
          bool Right=bool(GetWindowLong(Win,GWL_STYLE) & 1);

          Over=true;
          NewState=2; // Up
          if (GetCapture()==Win){
            if (GetKeyState(VK_LBUTTON)<0) NewState=1; // Down
            if (GetKeyState(VK_RBUTTON)<0 && Right) NewState=1;
          }else{
            if (GetKeyState(VK_LBUTTON)<0 || GetKeyState(VK_RBUTTON)<0) NewState=0;
          }
          if (PicButton_Over!=Win){
            if (PicButton_Over!=NULL) SendMessage(PicButton_Over,WM_TIMER,0,0);
            PicButton_Over=Win;
          }
        }
      }
    }else{
      NewState=1; // Down
    }
    if (OldState!=NewState){
      SetPropI(Win,"State",NewState);
      DoPaint=true;
    }
    if (PicButton_Over==Win && Over==0) PicButton_Over=NULL;
  }
  if (DoPaint){
    HDC DC;
    RECT box;
    HBRUSH br;
    HPEN TopLeft=NULL,BottomRight=NULL,OldPen;
    DWORD State=GetPropI(Win,"State");
    GET_THIS;

    DC=GetDC(Win);

    if (State==1){ // Down
      TopLeft=CreatePen(PS_SOLID,1,GetSysColor(COLOR_3DSHADOW));
      BottomRight=CreatePen(PS_SOLID,1,GetSysColor(COLOR_3DHIGHLIGHT));

      HDC ScrDC=GetDC(NULL);
      if (GetDeviceCaps(ScrDC,BITSPIXEL)<=8){
        br=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
      }else{
        br=CreateSolidBrush(DimColour(GetSysColor(COLOR_BTNFACE),12));
      }
      ReleaseDC(NULL,ScrDC);
    }else if (State==2){  // Over
      TopLeft=CreatePen(PS_SOLID,1,GetSysColor(COLOR_3DHIGHLIGHT));
      BottomRight=CreatePen(PS_SOLID,1,GetSysColor(COLOR_3DSHADOW));
      br=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    }else{
      br=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    }
    GetClientRect(Win,&box);
    FillRect(DC,&box,br);
    if (Inf){
      if (IsWindowEnabled(Win)){
        int x=(box.right-Inf->Width)/2,y=(box.bottom-Inf->Height)/2;
        if (State!=1){
          DrawIconEx(DC,x+1,y+1,Inf->ShadowIco,Inf->Width,Inf->Height,0,NULL,DI_NORMAL);
        }
        DrawIconEx(DC,x + (State==1),y + (State==1),
                    Inf->Ico,Inf->Width,Inf->Height,0,NULL,DI_NORMAL);
      }else{
        DrawState(DC,br,NULL,LPARAM(Inf->Ico),0,
                  (box.right-Inf->Width)/2 + (State==1),(box.bottom-Inf->Height)/2 + (State==1),
                  Inf->Width,Inf->Height,DST_ICON | DSS_DISABLED);
      }
    }
    DeleteObject(br);

    if (TopLeft!=NULL){
      box.bottom-=1;box.right-=1;

      OldPen=(HPEN)SelectObject(DC,TopLeft);
      MoveToEx(DC,0,box.bottom,0);
      LineTo(DC,0,0);
      LineTo(DC,box.right,0);
      SelectObject(DC,BottomRight);
      LineTo(DC,box.right,box.bottom);
      LineTo(DC,0,box.bottom);
      SelectObject(DC,OldPen);

      DeleteObject(TopLeft);
      DeleteObject(BottomRight);
    }

    ReleaseDC(Win,DC);

    if (Mess==WM_PAINT){
      ValidateRect(Win,NULL);
      return 0;
    }
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}
#undef GET_THIS

#define PDS_VCENTRESTATIC 1

LRESULT __stdcall PathDisplay_WndProc(HWND Win,UINT Mess,UINT wPar,long lPar)
{
  switch (Mess){
    case WM_CREATE:
      SetProp(Win,"DisplayPathFont",(HANDLE)GetStockObject(DEFAULT_GUI_FONT));
      break;
    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      RECT box;
      char *Text;
      int Len;
      bool Static=(GetWindowLong(Win,GWL_STYLE) & PDS_VCENTRESTATIC);

      BeginPaint(Win,&ps);
      SelectObject(ps.hdc,GetProp(Win,"DisplayPathFont"));

      GetClientRect(Win,&box);

      HBRUSH br;
      if (IsWindowEnabled(Win) && Static==0){
        br=CreateSolidBrush(GetSysColor(COLOR_WINDOW));
      }else{
        br=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
      }
      FillRect(ps.hdc,&box,(HBRUSH)br);
      SetBkMode(ps.hdc,TRANSPARENT);

      Len=DefWindowProc(Win,WM_GETTEXTLENGTH,0,0)+1;
      Text=new char[Len+1];
      DefWindowProc(Win,WM_GETTEXT,Len,(long)Text);

      if (Static){
        WIDTHHEIGHT wh=GetTextSize((HFONT)GetProp(Win,"DisplayPathFont"),Text);
        int x=(box.right-wh.Width)/2,y=(box.bottom-wh.Height)/2;
        if (IsWindowEnabled(Win)){
          TextOut(ps.hdc,x,y,Text,strlen(Text));
        }else{
          GrayString(ps.hdc,NULL,NULL,LPARAM(Text),0,x,y,0,0);
        }
      }else{
        box.left++;
        if (IsWindowEnabled(Win)){
          SetTextColor(ps.hdc,GetSysColor(COLOR_WINDOWTEXT));
        }else{
          SetTextColor(ps.hdc,GetSysColor(COLOR_GRAYTEXT));
        }
        DrawText(ps.hdc,Text,-1,&box,DT_PATH_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
      }

      DeleteObject(br);

      delete[] Text;

      EndPaint(Win,&ps);

      return 0;
    }
    case WM_ENABLE:
    case WM_SIZE:
      InvalidateRect(Win,NULL,true);
      break;
    case WM_SETTEXT:
    {
      LRESULT Ret=DefWindowProc(Win,Mess,wPar,lPar);
      InvalidateRect(Win,NULL,true);
      return Ret;
    }
    case WM_SETFONT:
      SetProp(Win,"DisplayPathFont",(HANDLE)wPar);
      break;
    case WM_DESTROY:
      RemoveProp(Win,"DisplayPathFont");
      break;
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
WINDOWPROC OldEditWndProc;

LRESULT __stdcall EditNoCaretWndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  switch (Mess){
    case WM_LBUTTONDOWN:   case WM_MBUTTONDOWN:   case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:     case WM_MBUTTONUP:     case WM_RBUTTONUP:
    case WM_LBUTTONDBLCLK: case WM_MBUTTONDBLCLK: case WM_RBUTTONDBLCLK:
      return 0;
    case WM_SETCURSOR:
      SetCursor(PCArrow);
      return 0;
  }
  return CallWindowProc(OldEditWndProc,Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
void MakeEditNoCaret(HWND Edit)
{
  WINDOWPROC old=(WINDOWPROC)GetWindowLong(Edit,GWL_WNDPROC);
  if (old){
    OldEditWndProc=old;
    SetWindowLong(Edit,GWL_WNDPROC,(long)EditNoCaretWndProc);
  }
}
//---------------------------------------------------------------------------
HBITMAP STCharChoose_CreateSTCharBitmap(BYTE STAscii,HDC ScrDC,HDC TempDC,HDC AllCharsDC,
                                          HDC FgDC,HBRUSH FgBr,HBRUSH BkBr)
{
  if (STAscii<32) STAscii=32;
	STAscii=BYTE(STAscii-32);

	HBITMAP RetBmp;
	RetBmp=CreateCompatibleBitmap(ScrDC,18,18);
	SelectObject(TempDC,RetBmp);

  int CharX=(STAscii % 40)*16,CharY=(STAscii/40)*16;

	RECT rc={0,0,18,18};
	FillRect(TempDC,&rc,BkBr);

	BitBlt(TempDC,1,1,16,16,AllCharsDC,CharX,CharY,SRCAND); // TempDC &= AllCharsDC

	FillRect(FgDC,&rc,FgBr);

  HDC MaskDC=CreateCompatibleDC(ScrDC);
	HBITMAP MaskBmp=CreateCompatibleBitmap(ScrDC,16,16);
  SelectObject(MaskDC,MaskBmp);

	BitBlt(MaskDC,0,0,16,16,AllCharsDC,CharX,CharY,NOTSRCCOPY); // MaskDC = ~AllCharsDC
	BitBlt(FgDC,0,0,16,16,MaskDC,0,0,SRCAND); // FgDC &= MaskDC

  DeleteDC(MaskDC);
  DeleteObject(MaskBmp);

	BitBlt(TempDC,1,1,16,16,FgDC,0,0,SRCPAINT); // TempDC |= FgDC

  return RetBmp;
}
//---------------------------------------------------------------------------
HBITMAP STCharChoose_CreateSTCharBitmap(BYTE STAscii,HBRUSH FgBr,HBRUSH BkBr)
{
  HDC ScrDC=GetDC(NULL);

  HDC STCharsDC=CreateCompatibleDC(ScrDC);
  HBITMAP STCharsBmp=LoadBitmap(Inst,"ST_CHARS");
  SelectObject(STCharsDC,STCharsBmp);

  HDC FgDC=CreateCompatibleDC(ScrDC);
	HBITMAP FgBmp=CreateCompatibleBitmap(ScrDC,16,16);
  SelectObject(FgDC,FgBmp);

  HDC TempDC=CreateCompatibleDC(ScrDC);

  HBITMAP Ret=STCharChoose_CreateSTCharBitmap(STAscii,ScrDC,TempDC,STCharsDC,FgDC,FgBr,BkBr);

  ReleaseDC(NULL,ScrDC);
  DeleteDC(TempDC);
  DeleteDC(FgDC);DeleteObject(FgBmp);
  DeleteDC(STCharsDC);DeleteObject(STCharsBmp);

  return Ret;
}

//---------------------------------------------------------------------------
#define CCPOP_CHOSEN WM_USER+1

LRESULT __stdcall STCharChoose_WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  if (GetWindowLong(Win,GWL_STYLE) & 1){
    // Popup menu to choose char
    switch (Mess){
      case WM_CREATE:
      {     	
        RECT rc;
        GetClientRect(Win,&rc);
        int WinWid=(rc.right/20)*20; // Each char bitmap is 18 pixels+2 for a border

        HDC ScrDC=GetDC(NULL);

        // Get our bitmap of the ST character set (characters 32-255)
        HDC STCharsDC=CreateCompatibleDC(ScrDC);
        HBITMAP STCharsBmp=LoadBitmap(Inst,"ST_CHARS");
        SelectObject(STCharsDC,STCharsBmp);

        HDC TempDC=CreateCompatibleDC(ScrDC);
        HBRUSH BkBr=CreateSolidBrush(GetSysColor(COLOR_WINDOW));
        // Get the standard bitmap for TempDC
        HANDLE OldTempDCBmp=GetCurrentObject(TempDC,OBJ_BITMAP);

        HDC FgDC=CreateCompatibleDC(ScrDC);
        HBITMAP FgBmp=CreateCompatibleBitmap(ScrDC,16,16);
        SelectObject(FgDC,FgBmp);
        HBRUSH FgBr=CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT));

        // Draw characters to this first, then copy to smaller bitmap
        HDC TempCharsDC=CreateCompatibleDC(ScrDC);
        HBITMAP TempCharsBmp=CreateCompatibleBitmap(ScrDC,WinWid,GetScreenHeight());
        SelectObject(TempCharsDC,TempCharsBmp);
        rc.left=0;rc.right=WinWid;
        rc.top=0;rc.bottom=GetScreenHeight();

        HBRUSH GridColBr=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
        FillRect(TempCharsDC,&rc,GridColBr);
        DeleteObject(GridColBr);

      	DynamicArray<DWORD> *lpChars=new DynamicArray<DWORD>();
      	GetAvailablePressChars(lpChars);
      	int x=0,y=0;
        for (int i=0;i<lpChars->NumItems;i++){
          if (x+20>WinWid){
            x=0;
            y+=20;
          }
          // Create a bitmap of the character and select it into TempDC
          STCharChoose_CreateSTCharBitmap(BYTE(HIWORD((*lpChars)[i])),ScrDC,TempDC,STCharsDC,FgDC,FgBr,BkBr);
          BitBlt(TempCharsDC,x+1,y+1,18,18,TempDC,0,0,SRCCOPY);
          // Delete the character bitmap, we have to select a different bitmap
          // into the DC or deleting fails and disasterous memory leak occurs
          DeleteObject(SelectObject(TempDC,OldTempDCBmp));
          x+=20;
        }

        // Create the bitmap we are actually going to draw
        HBITMAP CharsBmp=CreateCompatibleBitmap(ScrDC,WinWid,y+20);
        SelectObject(TempDC,CharsBmp);
        BitBlt(TempDC,0,0,WinWid,y+20,TempCharsDC,0,0,SRCCOPY);

        // Delete the massive bitmap
        DeleteDC(TempCharsDC);
        DeleteObject(TempCharsBmp);

        // Clean up all the schtuff
        DeleteObject(BkBr);
        DeleteObject(FgBr);

        ReleaseDC(NULL,ScrDC);
        DeleteDC(TempDC);
        DeleteDC(FgDC);DeleteObject(FgBmp);
        DeleteDC(STCharsDC);DeleteObject(STCharsBmp);

        // Store the values we need to be able to draw and choose
        SetProp(Win,"CharsBmp",CharsBmp);
        SetProp(Win,"CharVals",lpChars);
        SetPropI(Win,"OverX",0);
        SetPropI(Win,"OverY",0);
        SetPropI(Win,"OverSTAscii",0);

        // Resize the window to fit the bitmap and move it so it is on the screen
        GetWindowRect(Win,&rc);
        SetWindowPos(Win,HWND_TOPMOST,rc.left,min((int)rc.top,GetScreenHeight()-(y+22)),
                                      WinWid+2,y+22,0);
        return 0;
      }
      case WM_PAINT:
      {
        RECT Rc;
        GetClientRect(Win,&Rc);
        BYTE OverSTAscii=(BYTE)GetPropI(Win,"OverSTAscii");

        HBITMAP OverCharBmp=NULL;
        if (OverSTAscii){
          HBRUSH BkBr=CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
          HBRUSH FgBr=CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHTTEXT));
          OverCharBmp=STCharChoose_CreateSTCharBitmap(OverSTAscii,FgBr,BkBr);
          DeleteObject(FgBr);
          DeleteObject(BkBr);
        }

        HDC DC=GetDC(Win);
        HDC TempDC=CreateCompatibleDC(DC);
        SelectObject(TempDC,GetProp(Win,"CharsBmp"));
        BitBlt(DC,0,0,Rc.right,Rc.bottom,TempDC,0,0,SRCCOPY);

        if (OverCharBmp){
          SelectObject(TempDC,OverCharBmp);
          BitBlt(DC,GetPropI(Win,"OverX"),GetPropI(Win,"OverY"),18,18,TempDC,0,0,SRCCOPY);
        }

        DeleteDC(TempDC);
        ReleaseDC(Win,DC);

        if (OverCharBmp) DeleteObject(OverCharBmp);

        ValidateRect(Win,NULL);
        return 0;
      }
      case WM_LBUTTONDOWN:case WM_MBUTTONDOWN:case WM_RBUTTONDOWN:
      {
        SetCapture(Win);
        return 0;
      }
      case WM_MOUSEMOVE:
      case WM_LBUTTONUP:case WM_MBUTTONUP:case WM_RBUTTONUP:
      {
        RECT Rc;
        GetClientRect(Win,&Rc);

        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(Win,&pt);
        int CharsPerLine=Rc.right/20;

        // Get value array
        DynamicArray<DWORD> *lpChars=(DynamicArray<DWORD>*)GetProp(Win,"CharVals");
        DWORD Val=0; // Cancel
        if (pt.x>=0 && pt.x<Rc.right && pt.y>=0 && pt.y<Rc.bottom){
        	int Idx=(pt.y/20)*CharsPerLine + pt.x/20;
          if (Idx < lpChars->NumItems) Val=(*lpChars)[Idx];
        }

        if (Mess==WM_MOUSEMOVE){
          if (BYTE(HIWORD(Val))!=(BYTE)GetPropI(Win,"OverSTAscii")){
            // If you change selection with the button held down then set capture
            // This allows it to work like a pull down menu
            if (GetPropI(Win,"OverSTAscii") && wPar) SetCapture(Win);
            SetPropI(Win,"OverSTAscii",(int)LOBYTE(HIWORD(Val)));
            SetPropI(Win,"OverX",(pt.x/20) * 20+1);
            SetPropI(Win,"OverY",(pt.y/20) * 20+1);
            InvalidateRect(Win,NULL,0);
          }
        }else{
          if (GetCapture()==Win){
            ReleaseCapture();
            SendMessage((HWND)GetProp(Win,"ParentWin"),CCPOP_CHOSEN,0,Val);
            DestroyWindow(Win);
          }
        }
        return 0;
      }
      case WM_KILLFOCUS:
        // Send cancel message
        SendMessage((HWND)GetProp(Win,"ParentWin"),CCPOP_CHOSEN,0,0);
        DestroyWindow(Win);
        return 0;
      case WM_DESTROY:
      {
        DeleteObject(GetProp(Win,"CharsBmp"));
        delete (DynamicArray<DWORD>*)GetProp(Win,"CharVals");
        RemoveProps(Win,"CharsBmp","CharVals","ParentWin","OverSTAscii","OverX","OverY",NULL);
        break;
      }
    }
  }else{
    // Static displaying selection
    switch (Mess){
      case WM_CREATE:
        SetProp(Win,"Selection",NULL);
        SetProp(Win,"PopWindow",NULL);
        break;
      case WM_SETFOCUS:case WM_KILLFOCUS:
        InvalidateRect(Win,NULL,0);
        break;
      case WM_PAINT:
      {
        // Draw fake combo
        HDC DC=GetDC(Win);
        RECT Rc,ArrowRc;
        GetClientRect(Win,&Rc);
        ArrowRc=Rc;

        // Take off width of scrollbar
        Rc.right-=GetSystemMetrics(SM_CXVSCROLL);
        ArrowRc.left=Rc.right;

        bool Focussed=(GetFocus()==Win);
        HBRUSH BkBr=CreateSolidBrush(GetSysColor(Focussed ? COLOR_HIGHLIGHT:COLOR_WINDOW));
        HBRUSH FgBr=CreateSolidBrush(GetSysColor(Focussed ? COLOR_HIGHLIGHTTEXT:COLOR_WINDOWTEXT));

        HBITMAP CharBmp=NULL;
        // Create an individual char bitmap from the ST characters bitmap
        BYTE STAscii=LOBYTE(HIWORD(GetProp(Win,"Selection")));
        if (STAscii) CharBmp=STCharChoose_CreateSTCharBitmap(STAscii,FgBr,BkBr);

        // Draw background
        HANDLE OldPen=SelectObject(DC,CreatePen(PS_SOLID,1,GetSysColor(COLOR_WINDOW)));
        HANDLE OldBr=SelectObject(DC,BkBr);
        Rectangle(DC,0,0,Rc.right,Rc.bottom);
        DeleteObject(SelectObject(DC,OldPen));
        SelectObject(DC,OldBr);

        DeleteObject(BkBr);
        DeleteObject(FgBr);

        // Draw character centrally
        HDC TempDC=CreateCompatibleDC(DC);
        if (CharBmp){
          SelectObject(TempDC,CharBmp);
          BitBlt(DC,Rc.right/2-9,Rc.bottom/2-8,18,18,TempDC,0,0,SRCCOPY);
        }

        if (Focussed){
          Rc.left++;Rc.top++;Rc.right--;Rc.bottom--;
          SetBkMode(DC,OPAQUE);
          DrawFocusRect(DC,&Rc);
        }

        // Draw dropdown button
        DrawEdge(DC,&ArrowRc,EDGE_RAISED,BF_RECT);
        HBITMAP ArrowBmp=LoadBitmap(NULL,MAKEINTRESOURCE(OBM_COMBO));
        BITMAP bm;
        GetObject(ArrowBmp,sizeof(BITMAP),&bm);
        SelectObject(TempDC,ArrowBmp);
        int ArrowW=ArrowRc.right-ArrowRc.left,ArrowH=ArrowRc.bottom-ArrowRc.top;
        BitBlt(DC,ArrowRc.left+ArrowW/2-bm.bmWidth/2,ArrowRc.top+ArrowH/2-bm.bmHeight/2,
                  ArrowW,ArrowH,TempDC,0,0,SRCCOPY);

        DeleteDC(TempDC);
        DeleteObject(ArrowBmp);
        if (CharBmp) DeleteObject(CharBmp);

        // Done (finally)
        ReleaseDC(Win,DC);
        ValidateRect(Win,NULL);
        return 0;
      }
      case CCPOP_CHOSEN:
        // Chosen value in lPar or 0 for cancel
        if (lPar) SendMessage(Win,CB_SETCURSEL,0,lPar);
        SendMessage(GetParent(Win),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(Win),
                                    WORD(LOWORD(lPar) ? CBN_SELENDOK:CBN_SELENDCANCEL)),
                                    LPARAM(Win));
        // Allow Pop to destroy itself before we update "PopWindow"
        PostMessage(Win,WM_USER,0,0);
        return 0;
      case WM_USER:
        SetProp(Win,"PopWindow",NULL);
        return 0;
      case CB_SETCURSEL:
      {
        SetProp(Win,"Selection",(HANDLE)lPar);
        InvalidateRect(Win,NULL,0);
        return 0;
      }
      case CB_GETCURSEL:
        return LRESULT(GetProp(Win,"Selection"));
      case WM_KEYDOWN:
      case WM_LBUTTONDOWN:case WM_MBUTTONDOWN:case WM_RBUTTONDOWN:
      {
        SetFocus(Win);

        // If the popup was visible when you clicked then don't open it again
        if (GetProp(Win,"PopWindow")) return 0;

        RECT rc;
        GetWindowRect(Win,&rc);
        HWND Pop=CreateWindowEx(0,"Steem ST Character Chooser","",WS_POPUP | WS_VISIBLE | WS_BORDER | WS_CHILD | 1,
                      min((int)rc.left,GetScreenWidth()-258),rc.bottom,258,100,Win,NULL,Inst,NULL);
        SetProp(Pop,"ParentWin",Win);
        SetFocus(Pop);

        SetProp(Win,"PopWindow",Pop);
        InvalidateRect(Win,NULL,0);
        return 0;
      }
      case WM_DESTROY:
        RemoveProp(Win,"Selection");
        RemoveProp(Win,"PopWindow");
        break;
    }
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
LRESULT __stdcall HyperLinkWndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  bool DoPaint=0,CheckState=0,GetSize=0;
  switch (Mess){
    case WM_CREATE:
    {
      bool Static=bool(GetWindowLong(Win,GWL_STYLE) & HL_STATIC);
      SetProp(Win,"Font",MakeFont("MS Sans Serif",-10,0,FW_NORMAL,0,Static==0,0));
      SetProp(Win,"ClickedIn",NULL);
      SetProp(Win,"State",NULL);
      GetSize=true;
      break;
    }
    case WM_LBUTTONDOWN:
      if ((GetWindowLong(Win,GWL_STYLE) & HL_STATIC)==0){
        SetPropI(Win,"ClickedIn",1);
        SetPropI(Win,"State",1);
        SetCapture(Win);
        DoPaint=true;
      }else{
        return SendMessage(GetParent(Win),Mess,wPar,lParamPointsToParent(Win,lPar));
      }
      break;
    case WM_LBUTTONUP:
      if ((GetWindowLong(Win,GWL_STYLE) & HL_STATIC)==0){
        if (GetPropI(Win,"ClickedIn")){
          ReleaseCapture();
          SetPropI(Win,"ClickedIn",0);

          RECT rc;
          GetClientRect(Win,&rc);
          if (LOWORD(lPar)<rc.right && HIWORD(lPar)<rc.bottom){
            int Len=DefWindowProc(Win,WM_GETTEXTLENGTH,0,0)+1;
            char *Text=new char[Len+1];
            DefWindowProc(Win,WM_GETTEXT,Len,(long)Text);

            char *tp=Text;
            if (strchr(Text,'|')) tp=strchr(Text,'|')+1;
            ShellExecute(NULL,NULL,tp,"","",SW_SHOW);

            delete[] Text;
          }
        }
        CheckState=true;
      }else{
        return SendMessage(GetParent(Win),Mess,wPar,lParamPointsToParent(Win,lPar));
      }
      break;
    case WM_RBUTTONDOWN:case WM_RBUTTONUP:case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:case WM_MBUTTONUP:case WM_MBUTTONDBLCLK:
      return SendMessage(GetParent(Win),Mess,wPar,lParamPointsToParent(Win,lPar));
    case WM_ENABLE:
      DoPaint=true;
    case WM_TIMER:case WM_MOUSEMOVE:
      CheckState=true;
      break;
    case WM_SETTEXT:
      GetSize=true;
    case WM_PAINT:
      DoPaint=true;
      break;
    case WM_SETFONT:
    {
      DeleteObject(GetProp(Win,"Font"));

      LOGFONT lf;
      GetObject((HFONT)wPar,sizeof(LOGFONT),&lf);
      if ((GetWindowLong(Win,GWL_STYLE) & HL_STATIC)==0 || (GetWindowLong(Win,GWL_STYLE) & HL_UNDERLINE)){
        lf.lfUnderline=true;
      }
      SetProp(Win,"Font",CreateFontIndirect(&lf));

      GetSize=true;
      break;
    }
    case WM_SETCURSOR:
      if (GetWindowLong(Win,GWL_STYLE) & HL_STATIC){
        SetCursor(PCArrow);
      }else{
        SetCursor(LoadCursor(HInstance,"HYPER"));
      }
      break;
    case WM_DESTROY:
      if (PicButton_Over==Win) PicButton_Over=NULL;
      DeleteObject(GetProp(Win,"Font"));

      RemoveProp(Win,"Font");
      RemoveProp(Win,"ClickedIn");
      RemoveProp(Win,"State");
      break;
  }
  if (GetSize){
    HDC DC=GetDC(Win);
    SelectObject(DC,GetProp(Win,"Font"));

    int Len=DefWindowProc(Win,WM_GETTEXTLENGTH,0,0)+1;
    char *Text=new char[Len+1];
    DefWindowProc(Win,WM_GETTEXT,Len,(long)Text);
    if ((GetWindowLong(Win,GWL_STYLE) & HL_STATIC)==0){
      if (strchr(Text,'|')) *strchr(Text,'|')=0;
    }

    SIZE sz;
    GetTextExtentPoint32(DC,Text,strlen(Text),&sz);
    SetWindowPos(Win,NULL,0,0,sz.cx+1,sz.cy,SWP_NOZORDER | SWP_NOMOVE);

    delete[] Text;

    ReleaseDC(Win,DC);
  }
  if (CheckState){
    if ((GetWindowLong(Win,GWL_STYLE) & HL_STATIC)==0){
      DWORD OldState=GetPropI(Win,"State"),NewState=0;
      bool Over=0;
      if (IsWindowEnabled(Win)){
        POINT pt;

        GetCursorPos(&pt);
        ScreenToClient(GetParent(Win),&pt);
        if (ChildWindowFromPoint(GetParent(Win),pt)==Win){
          Over=true;
          if (GetCapture()==Win){
            NewState=1;
          }else{
            NewState=WORD(1-(GetKeyState(VK_LBUTTON)<0 || GetKeyState(VK_RBUTTON)<0));
          }
          if (PicButton_Over!=Win){
            if (PicButton_Over!=NULL) SendMessage(PicButton_Over,WM_TIMER,0,0);
            PicButton_Over=Win;
          }
        }
      }
      if (OldState!=NewState){
        SetPropI(Win,"State",NewState);
        DoPaint=true;
      }
      if (PicButton_Over==Win && Over==0) PicButton_Over=NULL;
    }
  }
  if (DoPaint){
    HDC DC;
    RECT rc;
    HBRUSH br;
    DWORD State=GetPropI(Win,"State");
    long Style=GetWindowLong(Win,GWL_STYLE);

    DC=GetDC(Win);

    GetClientRect(Win,&rc);

    COLORREF BkCol=GetSysColor(COLOR_BTNFACE);
    if (Style & HL_WINDOWBK) BkCol=GetSysColor(COLOR_WINDOW);
    br=CreateSolidBrush(BkCol);
    FillRect(DC,&rc,br);
    DeleteObject(br);

    int Len=DefWindowProc(Win,WM_GETTEXTLENGTH,0,0)+1;
    char *Text=new char[Len+1];
    DefWindowProc(Win,WM_GETTEXT,Len,(long)Text);
    if (strchr(Text,'|')) *strchr(Text,'|')=0;

    SelectObject(DC,GetProp(Win,"Font"));
    SetBkMode(DC,TRANSPARENT);

    if (IsWindowEnabled(Win)){
      if ((Style & HL_STATIC)==0){
        if (State==0){
          SetTextColor(DC,RGB(0,0,255));
        }else{
          SetTextColor(DC,RGB(255,0,0));
        }
      }else{
        if (Style & HL_WINDOWBK){
          SetTextColor(DC,GetSysColor(COLOR_WINDOWTEXT));
        }else{
          SetTextColor(DC,GetSysColor(COLOR_BTNTEXT));
        }
      }
      TextOut(DC,0,0,Text,strlen(Text));
    }else{
      GrayString(DC,NULL,NULL,(long)Text,strlen(Text),0,0,rc.right,rc.bottom);
    }
    delete[] Text;

    ReleaseDC(Win,DC);

    if (Mess==WM_PAINT){
      ValidateRect(Win,NULL);
      return 0;
    }
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
LRESULT __stdcall TextDisplayGroupBox_WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
//  WNDPROC old_gb_WndProc=(WNDPROC)GetWindowLong(Win,GWL_USERDATA);
  HWND TextBox=GetDlgItem(Win,0);
  switch (Mess){
    case WM_SETTEXT:case WM_GETTEXTLENGTH:case EM_GETFIRSTVISIBLELINE:
    case EM_LINEFROMCHAR:case EM_LINESCROLL:case WM_SETFONT:case EM_GETSEL:
    case WM_GETTEXT:case EM_SETSEL:case WM_MOUSEWHEEL:
      return SendMessage(TextBox,Mess,wPar,lPar);
    case WM_KEYDOWN:
      switch (wPar){
        case VK_DOWN:case VK_RIGHT: SendMessage(TextBox,EM_SCROLL,SB_LINEDOWN,0); break;
        case VK_UP:case VK_LEFT:    SendMessage(TextBox,EM_SCROLL,SB_LINEUP,0);   break;
        case VK_NEXT:               SendMessage(TextBox,EM_SCROLL,SB_PAGEDOWN,0); break;
        case VK_PRIOR:              SendMessage(TextBox,EM_SCROLL,SB_PAGEUP,0);   break;
        case VK_END:                SendMessage(TextBox,EM_LINESCROLL,0,30000);   break;
        case VK_HOME:               SendMessage(TextBox,EM_LINESCROLL,0,-30000);  break;
      }
      return 0;
    case WM_USER:
      SetFocus(Win);
      return 0;
    case WM_GETDLGCODE:
      return (LRESULT)DLGC_WANTALLKEYS;
    }
//  return CallWindowProc(old_gb_WndProc,Win,Mess,wPar,lPar);
//  return 0;
  return DefWindowProc(Win,Mess,wPar,lPar);
}

LRESULT __stdcall TextDisplayTextBox_WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar){
//  HWND gb=(HWND)GetWindowLong(Win,GWL_USERDATA);
  WNDPROC old_edit_WndProc=(WNDPROC)GetWindowLong(Win,GWL_USERDATA);
  switch (Mess){
    case WM_SETFOCUS:    case WM_LBUTTONDOWN:

      PostMessage(GetParent(Win),WM_USER,1,1);
      return 0;
//      return SendMessage(GetParent(Win),Mess,wPar,lPar);
    case WM_MBUTTONDOWN:   case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:     case WM_MBUTTONUP:     case WM_RBUTTONUP:
    case WM_LBUTTONDBLCLK: case WM_MBUTTONDBLCLK: case WM_RBUTTONDBLCLK:
      return 0;
    case WM_SETCURSOR:
      SetCursor(PCArrow);
      return 0;
  }
  return CallWindowProc(old_edit_WndProc,Win,Mess,wPar,lPar);
}


HWND CreateTextDisplay(HWND daddy,int x,int y,int w,int h,int id){
//  HWND gb=CreateWindow("Edit","Heff",WS_CHILD | ES_READONLY,
//                            x,y,w,h,daddy,(HMENU)id,HInstance,NULL);
  HWND gb=CreateWindow("Steem Text Display","",WS_CHILD,
                            x,y,w,h,daddy,(HMENU)id,HInstance,NULL);
  if(gb==NULL)return gb; //fail
  WNDPROC old_gb_WndProc=(WNDPROC)SetWindowLong(gb,GWL_WNDPROC,(long)TextDisplayGroupBox_WndProc);
  SetWindowLong(gb,GWL_USERDATA,(long)old_gb_WndProc);
  HWND tb=CreateWindowEx(512,"Edit","",WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_AUTOVSCROLL | WS_VISIBLE,
                            0,0,w,h,gb,(HMENU)0,HInstance,NULL);
  WNDPROC old_edit_WndProc=(WNDPROC)SetWindowLong(tb,GWL_WNDPROC,(long)TextDisplayTextBox_WndProc);
  SetWindowLong(tb,GWL_USERDATA,(long)old_edit_WndProc);
  return tb;
}

/*
  bool hxc_textdisplay::create(Display*d,Window daddy,int x,int y,
                      int w,int h,DWORD pass_col_bk,bool pass_force_scrollbar)

  HWND Win=CreateWindowEx(512,"Edit","",WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_AUTOVSCROLL,
                            page_l,40,page_w,INFOBOX_HEIGHT-50,Handle,(HMENU)500,HInstance,NULL);

*/


#endif//WIN32

#ifdef UNIX
#include "x/x_controls.cpp"
#endif

