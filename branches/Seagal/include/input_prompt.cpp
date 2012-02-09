/*---------------------------------------------------------------------------
FILE: input_prompt.cpp
MODULE: helper
DESCRIPTION: A generic input prompt.
---------------------------------------------------------------------------*/

#ifndef INPUTPROMPT_CPP
#define INPUTPROMPT_CPP
//---------------------------------------------------------------------------
#include "mymisc.h"
//---------------------------------------------------------------------------
#include "input_prompt.h"
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
LRESULT __stdcall InputPrompt_WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  switch (Mess){
    case WM_COMMAND:
    {
      switch (LOWORD(wPar)){
        case IDCANCEL:
          *(bool*)GetProp(Win,"pSuccess")=0;
          SetForegroundWindow((HWND)GetProp(Win,"Parent"));
          DestroyWindow(Win);
          return 0;
        case IDOK:
        {
          EasyStr *pRet=(EasyStr*)GetProp(Win,"pReturnStr");
          int Len=SendMessage(GetDlgItem(Win,100),WM_GETTEXTLENGTH,0,0)+1;
          pRet->SetLength(Len);
          SendMessage(GetDlgItem(Win,100),WM_GETTEXT,Len,LPARAM(pRet->Text));
          DestroyWindow(Win);
          return 0;
        }
      }
      break;
    }
    case WM_SETFOCUS:
      SetFocus(GetDlgItem(Win,100));
      break;
    case DM_GETDEFID:
      return MAKELONG(IDOK,DC_HASDEFID);
    case WM_CLOSE:
      PostMessage(Win,WM_COMMAND,IDCANCEL,0);
      return 0;
    case WM_DESTROY:
    {
      RemoveProp(Win,"pReturnStr");
      RemoveProp(Win,"pSuccess");
      *(HWND*)GetProp(Win,"pWin")=NULL;
      RemoveProp(Win,"pWin");
      RemoveProp(Win,"Parent");
      break;
    }
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
bool InputPrompt_Choose(HWND Parent,char *Title,EasyStr &Ret)
{
  WNDCLASS wc={0,InputPrompt_WndProc,0,0,GetModuleHandle(NULL),NULL,LoadCursor(NULL,IDC_ARROW),
                HBRUSH(COLOR_BTNFACE+1),NULL,"Generic Input Prompt"};

  RegisterClass(&wc);

  HWND Win=CreateWindowEx(0,"Generic Input Prompt",Title,DS_MODALFRAME,
                           100,10,326,10+25+5+25+5+6+GetSystemMetrics(SM_CYCAPTION),
                           Parent,NULL,GetModuleHandle(NULL),NULL);

  if (Win==NULL || IsWindow(Win)==0) return 0;

  bool Success=true;
  SetProp(Win,"pReturnStr",&Ret);
  SetProp(Win,"pSuccess",&Success);
  SetProp(Win,"pWin",&Win);
  SetProp(Win,"Parent",Parent);

  HWND hEd=CreateWindowEx(512,"Edit",Ret,WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                    10,10,300,25,Win,(HMENU)100,GetModuleHandle(NULL),NULL);
  SendMessage(hEd,EM_SETSEL,0,0xffffffff);

  CreateWindow("Button",T("OK"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                    100,40,100,23,Win,(HMENU)IDOK,GetModuleHandle(NULL),NULL);

  CreateWindow("Button",T("Cancel"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                    210,40,100,23,Win,(HMENU)IDCANCEL,GetModuleHandle(NULL),NULL);

  SetWindowAndChildrensFont(Win,(HFONT)GetStockObject(DEFAULT_GUI_FONT));

  CentreWindow(Win,0);
  ShowWindow(Win,SW_SHOW);

  MSG mess;
  while (GetMessage(&mess,NULL,0,0)){
    if (IsDialogMessage(Win,&mess)==0){
      TranslateMessage(&mess);
      DispatchMessage(&mess);
    }
    if (Win==NULL) break;
  }
  UnregisterClass("Generic Input Prompt",GetModuleHandle(NULL));

  return Success;
}

#endif

