#ifndef TBROWSEFORFOLDER_H
#define TBROWSEFORFOLDER_H
#include <shlobj.h>
//---------------------------------------------------------------------------
#ifdef STRICT
#define WINDOWPROC WNDPROC
#else
#define WINDOWPROC FARPROC
#endif
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class TBrowseForFolder
{
private:
  static LRESULT __stdcall BrowseWndProc(HWND hwnd,UINT uMsg,WPARAM wPar,LPARAM lPar);
  static LRESULT __stdcall PromptWndProc(HWND hwnd,UINT uMsg,WPARAM wPar,LPARAM lPar);
  static int __stdcall BrowseCallbackProc(HWND,UINT,LPARAM,LPARAM);

  void DoPrompt(HWND);
  
  int ChildId;
  bool QuitFlag;
  static WINDOWPROC OldBrowseWndProc;
  char *NewFolName;
public:
  TBrowseForFolder();
  ~TBrowseForFolder();
  void Execute(HWND);

  char *Title,*Path;
  char Selected[MAX_PATH+1];
};

WINDOWPROC TBrowseForFolder::OldBrowseWndProc;
//---------------------------------------------------------------------------
TBrowseForFolder::TBrowseForFolder()
{
  Title=new char[200];
  strcpy(Title,"Pick a Folder");
  Path=new char[MAX_PATH+1];
  Path[0]=0;
  NewFolName=NULL;
}
//---------------------------------------------------------------------------
TBrowseForFolder::~TBrowseForFolder()
{
  delete[] Title;
  delete[] Path;
}
//---------------------------------------------------------------------------
void TBrowseForFolder::Execute(HWND Parent)
{
  ITEMIDLIST *idl,*src=NULL;
  IMalloc *Mal;SHGetMalloc(&Mal);

  char *fol=new char[MAX_PATH];
  BROWSEINFO bi;
  bi.hwndOwner=Parent;
  bi.pidlRoot=src; //Directory to start from (NULL=desktop)
  bi.pszDisplayName=fol; //Doesn't return full path
  bi.lpszTitle=Title;
  bi.ulFlags=BIF_RETURNONLYFSDIRS; //No false folders (like DUN)
  bi.lpfn=BrowseCallbackProc; // Function to handle various notification (Drag Drop)
  bi.lParam=(LPARAM)this; // What to call that func with
  bi.iImage=0;

  idl=SHBrowseForFolder(&bi);
  if (idl!=NULL){
    SHGetPathFromIDList(idl,Path);
    Mal->Free(idl);
  }else{
    Path[0]=0;
  }
  delete[] fol;
}
//---------------------------------------------------------------------------
int __stdcall TBrowseForFolder::BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lPar,LPARAM lpThis)
{
  switch (uMsg){
    case BFFM_INITIALIZED:
    {
      TBrowseForFolder *This=(TBrowseForFolder*)lpThis;

      RECT box;
      GetWindowRect(hwnd,&box);
      box.right-=box.left;
      box.bottom-=box.top;
      SetWindowPos(hwnd,0,(GetSystemMetrics(SM_CXSCREEN)/2)-(box.right/2),
                    (GetSystemMetrics(SM_CYSCREEN)/2)-(box.bottom/2),
                    0,0,SWP_NOSIZE | SWP_NOZORDER);

      SendMessage(hwnd,BFFM_SETSELECTION,true,(long)This->Path);

      int ChildId;
      for (ChildId=6000;ChildId<7000;ChildId++)
        if (GetDlgItem(hwnd,ChildId)==NULL) break;

      int x=LOWORD(GetDialogBaseUnits()),y=HIWORD(GetDialogBaseUnits());
      HWND but=CreateWindow("BUTTON","New Folder",WS_CHILDWINDOW | /*WS_VISIBLE | */WS_TABSTOP | BS_PUSHBUTTON,
            (6*x)/4,(135*y)/8,(50*x)/4+1,(11*y)/8+1,
            hwnd,(HMENU)ChildId,GetModuleHandle(NULL),NULL);
      SendMessage(but,WM_SETFONT,SendMessage(hwnd,WM_GETFONT,0,0),0);
      SetWindowLong(but,GWL_USERDATA,(long)This);

      This->OldBrowseWndProc=(WINDOWPROC)SetWindowLong(hwnd,GWL_WNDPROC,(long)This->BrowseWndProc);

      POINT pt={box.right/2,box.bottom/2};
      HWND TreeView=ChildWindowFromPoint(hwnd,pt);
      SetWindowLong(TreeView,GWL_STYLE,GetWindowLong(TreeView,GWL_STYLE) | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP);

      break;
    }
    case BFFM_SELCHANGED:
    {
      TBrowseForFolder *This=(TBrowseForFolder*)lpThis;
      SHGetPathFromIDList(LPITEMIDLIST(lPar),This->Selected);
      break;
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
LRESULT __stdcall TBrowseForFolder::BrowseWndProc(HWND hwnd,UINT uMsg,WPARAM wPar,LPARAM lPar)
{
  if (uMsg==WM_COMMAND){
    if (HIWORD(wPar)==BN_CLICKED){
      HWND but=FindWindowEx(hwnd,NULL,"BUTTON","New Folder");
      if ((HWND)lPar==but){
        TBrowseForFolder *This=(TBrowseForFolder*)GetWindowLong(but,GWL_USERDATA);
        char *path=This->Selected;
        if (path[0]!=0){
          if (path[strlen(path)-1]!='\\') strcat(path,"\\");
          char temp[MAX_PATH+1];
          strcpy(temp,path);
          strcat(temp,"__TEST______");
          if (CreateDirectory(temp,NULL)==0){
            MessageBox(hwnd,"Unable to create directory here.","Sorry",0);
          }else{
            RemoveDirectory(temp);

            This->DoPrompt(hwnd);

            SendMessage(but,BM_SETSTYLE,BS_DEFPUSHBUTTON,1);
            SendMessage(FindWindowEx(hwnd,NULL,"BUTTON","OK"),BM_SETSTYLE,BS_PUSHBUTTON,1);
            SetFocus(FindWindowEx(hwnd,NULL,"BUTTON","OK"));
            SetForegroundWindow(hwnd);

            if (This->NewFolName!=NULL){
              strcpy(temp,path);
              strcat(temp,This->NewFolName);
              CreateDirectory(temp,NULL);
              // Delete all children of selected item
              // Add secret item that tells it to get folders
              // Expand selected
              SendMessage(hwnd,BFFM_SETSELECTION,true,(long)temp);
            }
            delete[] This->NewFolName;This->NewFolName=NULL;
          }
        }
      }
    }
  }
  return CallWindowProc(TBrowseForFolder::OldBrowseWndProc,hwnd,uMsg,wPar,lPar);
}
//---------------------------------------------------------------------------
void TBrowseForFolder::DoPrompt(HWND parent)
{
  HINSTANCE Inst=GetModuleHandle(NULL);

  WNDCLASS wnd;
  wnd.style=0;
  wnd.lpfnWndProc=PromptWndProc;
  wnd.cbWndExtra=0;
  wnd.cbClsExtra=0;
  wnd.hInstance=Inst;
  wnd.hIcon=NULL;
  wnd.hCursor=LoadCursor(NULL,IDC_ARROW);
  wnd.hbrBackground=HBRUSH(COLOR_BTNFACE+1);
  wnd.lpszMenuName=NULL;
  wnd.lpszClassName="TBrowseForFolder Prompt";
  RegisterClass(&wnd);

  HWND Win=CreateWindowEx(WS_EX_DLGMODALFRAME | WS_EX_CONTROLPARENT,
      "TBrowseForFolder Prompt","Enter a Name for the New Folder",
      WS_CAPTION | WS_SYSMENU,0,0,300,78+GetSystemMetrics(SM_CYCAPTION),
      parent,NULL,Inst,NULL);
  if (IsWindow(Win)){
    SetWindowLong(Win,GWL_USERDATA,(long)this);

    HWND Edit=CreateWindowEx(512,"EDIT","",WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP,
        10,10,274,21,Win,(HMENU)100,Inst,NULL);

    HWND Okay=CreateWindowEx(0,"BUTTON","OK",
        WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON | WS_DISABLED,
        110,40,84,23,Win,(HMENU)101,Inst,NULL);

    HWND Cancel=CreateWindowEx(0,"BUTTON","Cancel",
        WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        200,40,84,23,Win,(HMENU)IDCANCEL,Inst,NULL);

    SendMessage(Edit,WM_SETFONT,SendMessage(parent,WM_GETFONT,0,0),0);
    SendMessage(Cancel,WM_SETFONT,SendMessage(parent,WM_GETFONT,0,0),0);
    SendMessage(Okay,WM_SETFONT,SendMessage(parent,WM_GETFONT,0,0),0);

    RECT box;
    GetWindowRect(parent,&box);
    box.right-=box.left;
    box.left+=box.right/2;
    box.bottom-=box.top;
    box.top+=box.bottom/2;
    SetWindowPos(Win,0,box.left-150,box.top-40,0,0,SWP_NOSIZE | SWP_NOZORDER);

    EnableWindow(parent,0);
    ShowWindow(Win,SW_SHOW);
    SetFocus(Edit);

    MSG mess;
    QuitFlag=0;
    while (QuitFlag==0){
      GetMessage(&mess,0,0,0);
      if (IsDialogMessage(Win,&mess)==0){
        TranslateMessage(&mess);
        DispatchMessage(&mess);
      }
    }
    EnableWindow(parent,true);
  }
  UnregisterClass(wnd.lpszClassName,Inst);
}
//---------------------------------------------------------------------------
#define GET_THIS This=(TBrowseForFolder*)GetWindowLong(Win,GWL_USERDATA)

LRESULT __stdcall TBrowseForFolder::PromptWndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  TBrowseForFolder *This;

	switch (Mess){
    case WM_COMMAND:
      GET_THIS;
      switch (LOWORD(wPar)){
        case 100:
          if (HIWORD(wPar)==EN_CHANGE){
            delete[] This->NewFolName;
            int len=SendMessage((HWND)lPar,WM_GETTEXTLENGTH,0,0)+1;
            This->NewFolName=new char[len];
            SendMessage((HWND)lPar,WM_GETTEXT,len,(LPARAM)This->NewFolName);
            if (len==1){
              EnableWindow(GetDlgItem(Win,101),0);
            }else{
              EnableWindow(GetDlgItem(Win,101),true);
            }
          }
          break;
        case IDOK:
          if (IsWindowEnabled(GetDlgItem(Win,101))){
            SendMessage(GetDlgItem(Win,101),BM_CLICK,0,0);
          }else{
            MessageBeep(0);
          }
          break;
        case 101:
          if (HIWORD(wPar)==BN_CLICKED){
            DestroyWindow(Win);
          }
          break;
        case IDCANCEL:
          if (HIWORD(wPar)==BN_CLICKED){
            PostMessage(Win,WM_CLOSE,0,0);
          }
          break;
      }
      break;
    case WM_CLOSE:
      GET_THIS;
      delete[] This->NewFolName;This->NewFolName=NULL;
      break;
    case WM_DESTROY:
      GET_THIS;
      This->QuitFlag=true;
      break;
    case DM_GETDEFID:
      return (101 | (WORD(DC_HASDEFID) << 16));
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}
#undef GET_THIS
//---------------------------------------------------------------------------
#endif
