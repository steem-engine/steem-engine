/*---------------------------------------------------------------------------
FILE: historylist.cpp
MODULE: Steem
CONDITION: _DEBUG_BUILD
DESCRIPTION: The history list window in the debug build that shows a list of
recently executed commands.
---------------------------------------------------------------------------*/


//---------------------------------------------------------------------------
THistoryList::THistoryList()
{
  Width=300+GetSystemMetrics(SM_CXFRAME)*2;
  Height=300+GetSystemMetrics(SM_CYFRAME)*2+GetSystemMetrics(SM_CYCAPTION);
  Left=(GetSystemMetrics(SM_CXSCREEN)-Width)/2;
  Top=(GetSystemMetrics(SM_CYSCREEN)-Height)/2;
}
//---------------------------------------------------------------------------
void THistoryList::ManageWindowClasses(bool Unreg)
{
  char *ClassName="Steem History List";
  if (Unreg){
    UnregisterClass(ClassName,Inst);
  }else{
    RegisterMainClass(WndProc,ClassName,RC_ICO_STCLOSE);
  }
}
//---------------------------------------------------------------------------
THistoryList::~THistoryList()
{
  if (IsVisible()) Hide();
}
//---------------------------------------------------------------------------
void THistoryList::Show()
{
  if (Handle!=NULL){
    ShowWindow(Handle,SW_SHOWNORMAL);
    SetForegroundWindow(Handle);
    return;
  }

  ManageWindowClasses(SD_REGISTER);
  Handle=CreateWindowEx(WS_EX_CONTROLPARENT | WS_EX_TOOLWINDOW,
                          "Steem History List","History List",
                          WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX,
                          Left,Top,Width,Height,
                          NULL,NULL,HInstance,NULL);
  if (HandleIsInvalid()){
    ManageWindowClasses(SD_UNREGISTER);
    return;
  }

  SetWindowLong(Handle,GWL_USERDATA,(long)this);

  HWND hLB=CreateWindowEx(512,"ListBox","History",WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_USETABSTOPS |
                            WS_TABSTOP | LBS_NOINTEGRALHEIGHT | LBS_HASSTRINGS | LBS_NOTIFY,
                            10,10,280,250,Handle,(HMENU)100,HInstance,NULL);
  INT Tabs[1]={(GetTextSize(Font,"X  ").Width*4)/LOWORD(GetDialogBaseUnits())};
  SendMessage(hLB,LB_SETTABSTOPS,1,LPARAM(Tabs));

  CreateWindow("Button","Toggle Breakpoint",WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                  10,267,135,23,Handle,(HMENU)101,HInstance,NULL);

  CreateWindow("Button","Refresh",WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                  155,267,135,23,Handle,(HMENU)102,HInstance,NULL);


  SetWindowAndChildrensFont(Handle,Font);

  RefreshHistoryBox();

  Focus=GetDlgItem(Handle,100);

  ShowWindow(Handle,SW_SHOW);
  SetFocus(Focus);
}
//---------------------------------------------------------------------------
void THistoryList::Hide()
{
  if (Handle==NULL) return;

  ShowWindow(Handle,SW_HIDE);

  DestroyWindow(Handle);Handle=NULL;
  ManageWindowClasses(SD_UNREGISTER);
}
//---------------------------------------------------------------------------
void THistoryList::RefreshHistoryBox()
{
  HWND Win=GetDlgItem(Handle,100);
  SendMessage(Win,LB_RESETCONTENT,0,0);

  int n=pc_history_idx;
  EasyStr Dissasembly;
  do{
    n--;
    if (n<0) n=HISTORY_SIZE-1;
    if (pc_history[n]==0xffffff71) break;
    Dissasembly=debug_parse_disa_for_display(disa_d2(pc_history[n]));
    SendMessage(Win,LB_INSERTSTRING,0,(long)((HEXSl(pc_history[n],6)+" - "+Dissasembly).Text));
    SendMessage(Win,LB_SETITEMDATA,0,n);
  }while (n!=pc_history_idx);

  RECT rc;
  GetClientRect(Win,&rc);
  int Selected=SendMessage(Win,LB_GETCOUNT,0,0)-1;
  SendMessage(Win,LB_SETCURSEL,Selected,0);
  SendMessage(Win,LB_SETTOPINDEX,max(Selected-(rc.bottom/SendMessage(Win,LB_GETITEMHEIGHT,0,0)),(long)0),0);
  SendMessage(Win,WM_SETFONT,WPARAM(debug_monospace_disa ? GetStockObject(ANSI_FIXED_FONT):Font),TRUE);
}
//---------------------------------------------------------------------------
#define GET_THIS This=(THistoryList*)GetWindowLong(Win,GWL_USERDATA);

LRESULT __stdcall THistoryList::WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  LRESULT Ret=DefStemDialogProc(Win,Mess,wPar,lPar);
  if (StemDialog_RetDefVal) return Ret;

  THistoryList *This;
  switch (Mess){
    case WM_COMMAND:
      if (LOWORD(wPar)==100){
        if (HIWORD(wPar)==LBN_DBLCLK){
          int Selected=SendMessage(GetDlgItem(Win,100),LB_GETCURSEL,0,0);
          PostMessage(DWin,WM_COMMAND,17000+SendMessage(GetDlgItem(Win,100),LB_GETITEMDATA,Selected,0),0);
        }
      }else if (LOWORD(wPar)==101){
        if (HIWORD(wPar)==BN_CLICKED){
          HWND HistBox=GetDlgItem(Win,100);
          int Selected=SendMessage(HistBox,LB_GETCURSEL,0,0);
          if (Selected>-1){
            int SelHistNum=SendMessage(HistBox,LB_GETITEMDATA,Selected,0);
            EasyStr Text;
            Text.SetLength(SendMessage(HistBox,LB_GETTEXTLEN,Selected,0)+1);
            SendMessage(HistBox,LB_GETTEXT,Selected,(long)Text.Text);
            SendMessage(HistBox,LB_DELETESTRING,Selected,0);
            SendMessage(HistBox,LB_INSERTSTRING,Selected,(long)Text.Text);
            SendMessage(HistBox,LB_SETITEMDATA,Selected,SelHistNum);
            SendMessage(HistBox,LB_SETCURSEL,Selected,0);
          }
        }
      }else if (LOWORD(wPar)==102){
        if (HIWORD(wPar)==BN_CLICKED){
          GET_THIS;
          This->RefreshHistoryBox();
        }
      }
      break;
    case WM_SIZE:
      SetWindowPos(GetDlgItem(Win,100),0,0,0,LOWORD(lPar)-20,HIWORD(lPar)-50,SWP_NOMOVE | SWP_NOZORDER);
      SetWindowPos(GetDlgItem(Win,101),0,10,HIWORD(lPar)-30,(LOWORD(lPar)/2)-15,23,SWP_NOZORDER);
      SetWindowPos(GetDlgItem(Win,102),0,LOWORD(lPar)/2+5,HIWORD(lPar)-30,(LOWORD(lPar)/2)-15,23,SWP_NOZORDER);

      GET_THIS;
      if (IsIconic(Win)==0){
        if (IsZoomed(Win)==0){
          This->Maximized=0;

          RECT rc;GetWindowRect(Win,&rc);
          This->Left=rc.left;This->Top=rc.top;
          This->Width=rc.right-rc.left;This->Height=rc.bottom-rc.top;
        }else{
          This->Maximized=true;
        }
      }
      break;
    case WM_GETMINMAXINFO:
      ((MINMAXINFO*)lPar)->ptMinTrackSize.x=200+GetSystemMetrics(SM_CXFRAME)*2+GetSystemMetrics(SM_CXVSCROLL);
      ((MINMAXINFO*)lPar)->ptMinTrackSize.y=100+GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYFRAME)*2;
      break;
    case WM_CLOSE:
      GET_THIS;
      This->Hide();
      return 0;
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}

#undef GET_THIS

