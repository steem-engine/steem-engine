/*---------------------------------------------------------------------------
FILE: stemdialogs.cpp
MODULE: Steem
DESCRIPTION: The base class for Steem's dialogs that are used to configure
the emulator and perform additional functions.
---------------------------------------------------------------------------*/

#ifdef WIN32
//---------------------------------------------------------------------------
TStemDialog::TStemDialog()
{
  Handle=NULL;
  Focus=NULL;
  Font=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
  Left=100;Top=100;
  FSLeft=50;FSTop=50;

  if (nStemDialogs<MAX_DIALOGS) DialogList[nStemDialogs++]=this;
}
//---------------------------------------------------------------------------
inline bool TStemDialog::HasHandledMessage(MSG *mess)
{
  if (Handle){
    return IsDialogMessage(Handle,mess);
  }else{
    return 0;
  }
}
//---------------------------------------------------------------------------
void TStemDialog::RegisterMainClass(WNDPROC WndProc,char *ClassName,int nIcon)
{
  WNDCLASS wc;
  wc.style=CS_DBLCLKS;
  wc.lpfnWndProc=WndProc;
  wc.cbClsExtra=0;
  wc.cbWndExtra=0;
  wc.hInstance=(HINSTANCE)GetModuleHandle(NULL);
  nMainClassIcon=nIcon;
  wc.hIcon=hGUIIcon[nIcon];
  wc.hCursor=LoadCursor(NULL,IDC_ARROW);
  wc.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
  wc.lpszMenuName=NULL;
  wc.lpszClassName=ClassName;
  RegisterClass(&wc);
}
//---------------------------------------------------------------------------
void TStemDialog::UpdateMainWindowIcon()
{
  if (Handle) SetClassLong(Handle,GCL_HICON,long(hGUIIcon[nMainClassIcon]));
}
//---------------------------------------------------------------------------
inline bool TStemDialog::HandleIsInvalid()
{
  if (Handle) if (IsWindow(Handle)==0) Handle=NULL;
  return Handle==NULL;
}
//---------------------------------------------------------------------------
void TStemDialog::MakeParent(HWND NewParent)
{
  if (Handle){
    UpdateMainWindowIcon();
    SendMessage(Handle,WM_USER+1011,0,(LPARAM)NewParent);
  }
}
//---------------------------------------------------------------------------
void TStemDialog::ChangeParent(HWND NewParent)
{
  if (NewParent!=NULL){
    RECT rc;
    GetWindowRect(Handle,&rc);
    if (rc.top<MENUHEIGHT){
      SetWindowPos(Handle,NULL,rc.left,MENUHEIGHT,0,0,SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    SetParent(Handle,NewParent);
//    SetWindowLong(Handle,GWL_STYLE,GetWindowLong(Handle,GWL_STYLE) & ~WS_MINIMIZEBOX);
    SetWindowPos(Handle,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
  }else{
    SetParent(Handle,NULL);
//    SetWindowLong(Handle,GWL_STYLE,GetWindowLong(Handle,GWL_STYLE) | WS_MINIMIZEBOX);
    SetWindowPos(Handle,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
  }
}
//---------------------------------------------------------------------------
HTREEITEM TStemDialog::AddPageLabel(char *t,int i)
{
  TV_INSERTSTRUCT tvis;
  tvis.hParent=TVI_ROOT;
  tvis.hInsertAfter=TVI_LAST;
  tvis.item.mask=TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
  tvis.item.pszText=t;
  tvis.item.lParam=i;
  tvis.item.iImage=i;
  tvis.item.iSelectedImage=i;
  return (HTREEITEM)SendMessage(PageTree,TVM_INSERTITEM,0,(LPARAM)&tvis);
}
//---------------------------------------------------------------------------
void TStemDialog::GetPageControlList(DynamicArray<HWND> &ChildList)
{
  HWND FirstChild=GetWindow(Handle,GW_CHILD);
  HWND Child=FirstChild;
  while (Child){
    if (GetDlgCtrlID(Child)<60000) ChildList.Add(Child);
    Child=GetWindow(Child,GW_HWNDNEXT);
    if (Child==FirstChild) break;
  }
}
//---------------------------------------------------------------------------
void TStemDialog::DestroyCurrentPage()
{
  DynamicArray<HWND> ChildList;
  GetPageControlList(ChildList);
  for (int n=0;n<ChildList.NumItems;n++) DestroyWindow(ChildList[n]);
}
//---------------------------------------------------------------------------
void TStemDialog::ShowPageControls()
{
  DynamicArray<HWND> ChildList;
  GetPageControlList(ChildList);
  for (int n=0;n<ChildList.NumItems;n++) ShowWindow(ChildList[n],SW_SHOW);
}
//---------------------------------------------------------------------------
void TStemDialog::SetPageControlsFont()
{
  DynamicArray<HWND> ChildList;
  GetPageControlList(ChildList);
  for (int n=0;n<ChildList.NumItems;n++) SendMessage(ChildList[n],WM_SETFONT,WPARAM(Font),0);
}
//---------------------------------------------------------------------------
#define GET_THIS This=(TStemDialog*)GetWindowLong(Win,GWL_USERDATA);

LRESULT TStemDialog::DefStemDialogProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  StemDialog_RetDefVal=0;
  TStemDialog *This;
  switch (Mess){
    case WM_SYSCOMMAND:
      switch (wPar){
        case SC_MONITORPOWER:
          if (runstate == RUNSTATE_RUNNING) return 0;
          break;
        case SC_SCREENSAVE:
          if (runstate == RUNSTATE_RUNNING || FullScreen) return 0;
          break;
      }
      break;
    case WM_MOVING:case WM_SIZING:
      if (FullScreen){
        RECT *rc=(RECT*)lPar;
        if (rc->top<MENUHEIGHT){
          if (Mess==WM_MOVING) rc->bottom+=MENUHEIGHT-rc->top;
          rc->top=MENUHEIGHT;
          StemDialog_RetDefVal=true;
          return true;
        }
        RECT LimRC={0,MENUHEIGHT+GetSystemMetrics(SM_CYFRAME),
                    GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN)};
        ClipCursor(&LimRC);
      }
      break;
    case WM_MOVE:
    {
      GET_THIS;

      RECT rc;
      GetWindowRect(Win,&rc);
      if (FullScreen){
        if (IsIconic(StemWin)==0 && IsZoomed(StemWin)==0){
          This->FSLeft=rc.left;This->FSTop=rc.top;
        }
      }else{
        if (IsIconic(Win)==0 && IsZoomed(Win)==0){
          This->Left=rc.left;This->Top=rc.top;
        }
      }
      break;
    }
    case WM_CAPTURECHANGED:   //Finished
      if (FullScreen) ClipCursor(NULL);
      break;
    case WM_ACTIVATE:
      if (wPar==WA_INACTIVE){
        GET_THIS;
        This->Focus=GetFocus();
      }else{
        if (IsWindowEnabled(Win)==0){
          PostMessage(StemWin,WM_USER,12345,(LPARAM)Win);
        }
      }
      break;
    case WM_SETFOCUS:
      GET_THIS;
      SetFocus(This->Focus);
      break;
  }
  return 0;
}
#undef GET_THIS
//---------------------------------------------------------------------------
#endif

#ifdef UNIX
#include "x/x_stemdialogs.cpp"
#endif
//---------------------------------------------------------------------------
void TStemDialog::LoadPosition(GoodConfigStoreFile *pCSF)
{
  int PCScreenW=GetScreenWidth(),PCScreenH=GetScreenHeight();

  Left=max(min((int)pCSF->GetInt(Section,"Left",Left),PCScreenW-100),-100);
  Top=max(min((int)pCSF->GetInt(Section,"Top",Top),PCScreenH-70),-70);
  FSLeft=pCSF->GetInt(Section,"FSLeft",FSLeft);
  FSLeft=max(min(FSLeft,640-100),-100);
  FSTop=pCSF->GetInt(Section,"FSTop",FSTop);
  FSTop=max(min(FSTop,480-70),-70);
}
//---------------------------------------------------------------------------
void TStemDialog::SavePosition(bool FinalSave,ConfigStoreFile *pCSF)
{
  pCSF->SetInt(Section,"Left",Left);
  pCSF->SetInt(Section,"Top",Top);
  pCSF->SetInt(Section,"FSLeft",FSLeft);
  pCSF->SetInt(Section,"FSTop",FSTop);
  if (FinalSave==0) SaveVisible(pCSF);
}
//---------------------------------------------------------------------------
void TStemDialog::SaveVisible(ConfigStoreFile *pCSF)
{
  if (Section.NotEmpty()){
    pCSF->SetInt(Section,"Visible",IsVisible());
  }
}
//---------------------------------------------------------------------------
#ifdef WIN32
void TStemDialog::CheckFSPosition(HWND Par)
{
  RECT rc;
  GetClientRect(Par,&rc);
  FSLeft=max(min(FSLeft,(int)rc.right-100),-100);
  FSTop=max(min(FSTop,(int)rc.bottom-70),-70);
}
//---------------------------------------------------------------------------
void TStemDialog::UpdateDirectoryTreeIcons(DirectoryTree *pTree)
{
  for (int n=0;n<pTree->FileMasksESL.NumStrings;n++){
    pTree->FileMasksESL[n].Data[0]=long(hGUIIcon[pTree->FileMasksESL[n].Data[1]]);
  }
  pTree->ReloadIcons(BPPToILC[BytesPerPixel]);
}
#endif
//---------------------------------------------------------------------------

