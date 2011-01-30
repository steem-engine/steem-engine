/*---------------------------------------------------------------------------
FILE: directory_tree.cpp
MODULE: helper
DESCRIPTION: A useful class to implement a tree control containing folders
and optionally files and allow the user to rename/delete etc...
---------------------------------------------------------------------------*/

#ifndef DIRECTORY_TREE_CPP
#define DIRECTORY_TREE_CPP

#include <commctrl.h>
#include <shlobj.h>

#include <easystr.h>
#include <easystringlist.h>
#include <mymisc.h>

#include "directory_tree.h"

#ifndef SLASH
#define SLASH "\\"
#endif

#ifndef DTREE_LOG
#define DTREE_LOG(s)
#endif

#ifndef T
#define T(s) s
#endif

#define DT_DROP_NOWHERE 0
#define DT_DROP_ASCHILD 1

HWND DirectoryTree::PopupParent=NULL;
//---------------------------------------------------------------------------
void DirectoryTree::RefreshDirectory()
{
  DeleteChildrenOfItem(RootItem);
  PutDirectoryInTree(RootItem,RootFol);
}
LRESULT DirectoryTree::SendTreeMessage(UINT m,WPARAM w,LPARAM l)
{
  if (hTree==NULL) return 0;
  return SendMessage(hTree,m,w,l);
}
void DirectoryTree::EnsureItemExpanded(HTREEITEM Item)
{
  TV_ITEM Inf=GetItem(Item,TVIF_STATE);
  if ((Inf.state & TVIS_EXPANDED)==0){
    Inf.state|=TVIS_EXPANDED;
    SendTreeMessage(TVM_SETITEM,0,LPARAM(&Inf));
    PutDirectoryInTree(Item,GetItemPath(Item));
    UpdateWindow(hTree);
  }
}

#ifdef DT_TEST
int TestStage=0;
#define DT_TEST_ONLY(s) s
#define DT_TEST_STAGE(i) if (TestStage>=i)
#else
#define DT_TEST_ONLY(s)
#define DT_TEST_STAGE(i)
#endif
//---------------------------------------------------------------------------
DirectoryTree::DirectoryTree()
{
  hTree=NULL;
  FileMasksESL.Sort=eslNoSort;
  DragItem=NULL, DragEntered=0, DragButton=0, DropTarget=NULL;
  IDBase=64300;
  il=NULL;
  AllowTypeChange=0;
  DoNotifyOnChange=true;
  ExpandTimerActive=0;
  NotifyProc=NULL;
}
//---------------------------------------------------------------------------
DirectoryTree::~DirectoryTree()
{
  Destroy();
}
//---------------------------------------------------------------------------
bool DirectoryTree::Create(HWND Par,int x,int y,int w,int h,int ID,DWORD Flags,
                            PDTNOTIFYPROC NP,void* That,EasyStr RF,EasyStr RN,bool CO)
{
  if (hTree){
    DTREE_LOG("DTree: Create: Destroying old tree");
    Destroy();
  }

  InitCommonControls();

  NotifyProc=NP;
  NotifyThis=That;
  hParent=Par;
  TreeID=ID;
  RootFol=RF;NO_SLASH(RootFol);

  ChooseOnly=CO;
  DWORD COFlags=TVS_EDITLABELS;
  if (ChooseOnly) COFlags=TVS_DISABLEDRAGDROP;

  DTREE_LOG("DTree: Create: Calling CreateWindow");
  hTree=CreateWindowEx(512,WC_TREEVIEW,"",WS_CHILD | Flags | TVS_HASLINES |
                        COFlags | TVS_SHOWSELALWAYS | TVS_HASBUTTONS,
                        x,y,w,h,Par,(HMENU)ID,GetModuleHandle(NULL),NULL);
  SetProp(hTree,"DirectoryTreeThis",this);
  OldTVWndProc=(WNDPROC)GetWindowLong(hTree,GWL_WNDPROC);
  SetWindowLong(hTree,GWL_WNDPROC,(long)TVWndProc);

  ReloadIcons(ILC_COLOR4);

  DTREE_LOG("DTree: Create: Inserting the root item");
  RootItem=InsertItem(RN,TVI_ROOT,TVI_FIRST,0,0,0,TVIS_BOLD | TVIS_EXPANDED | TVIS_EXPANDEDONCE);

  DTREE_LOG(EasyStr("DTree: Create: Inserting contents of ")+RootFol);
  PutDirectoryInTree(RootItem,RootFol);

  return hTree!=NULL;
}
//---------------------------------------------------------------------------
void DirectoryTree::ReloadIcons(UINT ILCDepth)
{
  if (hTree==NULL) return;

  DTREE_LOG("DTree: Create: Making the image list");
  HIMAGELIST new_il=ImageList_Create(16,16,ILC_MASK | ILCDepth,FileMasksESL.NumStrings,FileMasksESL.NumStrings);
  for (int n=0;n<FileMasksESL.NumStrings;n++){
    ImageList_AddIcon(new_il,(HICON)FileMasksESL[n].Data[0]);
  }
  SendTreeMessage(TVM_SETIMAGELIST,TVSIL_NORMAL,(LPARAM)new_il);
  if (il) ImageList_Destroy(il);
  il=new_il;
}
//---------------------------------------------------------------------------
bool DirectoryTree::FindFilesAndFolders(EasyStr Fol,EasyStringList &fol_sl,EasyStringList &fil_sl,bool RetFirst)
{
  bool Found=0;

  DTREE_LOG(EasyStr("DTree: FindFilesAndFolders: Searching for ")+Fol+"\\*.*");
  WIN32_FIND_DATA wfd;
  HANDLE f=FindFirstFile(Fol+"\\*.*",&wfd);
  if (f==INVALID_HANDLE_VALUE) return Found;

  EasyStr Ext,Name;
  do{
    if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
      if (NotSameStr(wfd.cFileName,".") && NotSameStr(wfd.cFileName,"..")){
        Found=true;
        DTREE_LOG(EasyStr("DTree: FindFilesAndFolders: Found directory ")+wfd.cFileName);
        if (RetFirst){
          FindClose(f);
          return Found;
        }
        DTREE_LOG("DTree: FindFilesAndFolders: Adding to fol_sl");
        fol_sl.Add(wfd.cFileName);
      }
    }else{
      Ext="";
      Name=wfd.cFileName;
      char *dot=strrchr(Name,'.');
      if (dot){
        Ext=dot+1;
        *dot=0;
      }
      for (int n=1;n<FileMasksESL.NumStrings;n++){
        if (IsSameStr_I(Ext,FileMasksESL[n].String)){
          Found=true;
          DTREE_LOG(EasyStr("DTree: FindFilesAndFolders: Found file ")+wfd.cFileName);
          if (RetFirst){
            FindClose(f);
            return Found;
          }
          if (NotifyProc){
            EasyStr FullPath=Fol+SLASH+wfd.cFileName;
            DTREE_LOG(EasyStr("DTree: FindFilesAndFolders: Requesting type for ")+FullPath);
            int newn=NotifyProc(this,NotifyThis,DTM_GETTYPE,int(FullPath.Text),n);
            if (newn) n=newn;
          }
          DTREE_LOG("DTree: FindFilesAndFolders: Adding to fil_sl");
          fil_sl.Add(Name,n);
          break;
        }
      }
    }
  }while (FindNextFile(f,&wfd));
  FindClose(f);
  return Found;
}
//---------------------------------------------------------------------------
void DirectoryTree::PutDirectoryInTree(HTREEITEM ParentItem,EasyStr Fol)
{
  if (hTree==NULL) return;

  EasyStringList fol_sl,fil_sl;
  fol_sl.Sort=eslSortByNameI;
  fil_sl.Sort=eslSortByNameI;

  FindFilesAndFolders(Fol,fol_sl,fil_sl,0);
  DTREE_LOG(EasyStr("DTree: PutDirectoryInTree: Folders=")+fol_sl.NumStrings+" Files="+fil_sl.NumStrings);

  DT_TEST_STAGE(2){
    for (int n=0;n<fol_sl.NumStrings;n++){
      bool ShowButton=0;
      DT_TEST_STAGE(3){
        if (FindFilesAndFolders(Fol+SLASH+fol_sl[n].String,fol_sl,fil_sl,true)) ShowButton=true;
      }
      DTREE_LOG(EasyStr("DTree: PutDirectoryInTree: Inserting folder ")+fol_sl[n].String);
      InsertItem(fol_sl[n].String,ParentItem,TVI_LAST,0,ShowButton);
    }
  }
  DT_TEST_STAGE(1){
    for (int n=0;n<fil_sl.NumStrings;n++){
      DTREE_LOG(EasyStr("DTree: PutDirectoryInTree: Inserting file ")+fil_sl[n].String);
      InsertItem(fil_sl[n].String,ParentItem,TVI_LAST,fil_sl[n].Data[0],0);
    }
  }
}
//---------------------------------------------------------------------------
EasyStr DirectoryTree::GetItemPath(HTREEITEM Item)
{
  if (hTree==NULL) return "";
  if (Item==NULL || Item==TVI_ROOT || Item==RootItem) return RootFol;

  EasyStr Fol,Name;
  static TV_ITEM Inf;

  Name.SetLength(500);
  Inf.mask=TVIF_TEXT | TVIF_IMAGE | TVIF_HANDLE;
  Inf.pszText=Name;
  Inf.cchTextMax=500;
  for(;;){
    Inf.hItem=Item;
    SendTreeMessage(TVM_GETITEM,0,(LPARAM)&Inf);
    if (Inf.iImage){ // File
      if (FileMasksESL[Inf.iImage].String[0]) Name+=EasyStr(".")+FileMasksESL[Inf.iImage].String;
    }
    Fol=Name+"\\"+Fol;
    Item=TreeView_GetParent(hTree,Item);
    if (Item==NULL || Item==RootItem) break;
  }

  Fol=RootFol+"\\"+Fol;
  *(Fol.Right())=0; // Cut of trailing slash
  return Fol;
}
//---------------------------------------------------------------------------
TV_ITEM DirectoryTree::GetItem(HTREEITEM Item,int Mask,char *TextBuf,int TextBufLen)
{
  TV_ITEM Inf;
  Inf.mask=Mask | TVIF_HANDLE;
  Inf.stateMask=0xffffffff;
  Inf.pszText=TextBuf;
  Inf.cchTextMax=TextBufLen;
  Inf.hItem=Item;
  SendTreeMessage(TVM_GETITEM,0,(LPARAM)&Inf);
  return Inf;
}
//---------------------------------------------------------------------------
bool DirectoryTree::ItemIsChild(HTREEITEM Parent,HTREEITEM Child)
{
  if (hTree==NULL) return 0;
  if (Child==Parent) return true;
  if (Child==TVI_ROOT || Child==RootItem) return 0;
  do{
    Child=TreeView_GetParent(hTree,Child);
    if (Child==Parent) return true;
  }while (Child!=NULL);
  return 0;
}
//---------------------------------------------------------------------------
HTREEITEM DirectoryTree::SelectItemByPath(EasyStr Path)
{
  if (hTree==NULL) return NULL;

  NO_SLASH(Path);

  DTREE_LOG(EasyStr("DTree: SelectItemByPath: Path=")+Path);
  HTREEITEM Item;
  Item=RootItem;
  if (IsSameStr_I(Path,RootFol) || Path.Empty()){
    DTREE_LOG(EasyStr("DTree: SelectItemByPath: Found root"));
    SendTreeMessage(TVM_SELECTITEM,TVGN_CARET,(LPARAM)Item);
    return Item;
  }
  if (strstr(Path,RootFol)==Path.Text){
    char PathBuf[MAX_PATH+2],ItemName[501];
    ZeroMemory(PathBuf,MAX_PATH+2);
    strcpy(PathBuf,Path.Text+strlen(RootFol)+1);
    int Len=strlen(PathBuf);
    for (int n=0;n<Len;n++){
      if (PathBuf[n]=='\\' || PathBuf[n]=='/') PathBuf[n]=0;
    }
    TV_ITEM Inf;
    Item=TreeView_GetChild(hTree,Item);
    char *pName=PathBuf;
    DTREE_LOG(EasyStr("DTree: SelectItemByPath: Finding item with name ")+pName);
    while (Item){
      Inf=GetItem(Item,TVIF_TEXT | TVIF_IMAGE,ItemName,500);
      if (FileMasksESL[Inf.iImage].String[0]){
        strcat(ItemName,".");
        strcat(ItemName,FileMasksESL[Inf.iImage].String);
      }
      if (IsSameStr_I(ItemName,pName)){
        DTREE_LOG(EasyStr("DTree: SelectItemByPath: Found one"));
        pName+=strlen(pName)+1; // Next string
        if (pName[0]==0){ // Last string
          DTREE_LOG(EasyStr("DTree: SelectItemByPath: That's what we wanted, selecting"));
          SendTreeMessage(TVM_SELECTITEM,TVGN_CARET,(LPARAM)Item);
          DTREE_LOG(EasyStr("DTree: SelectItemByPath: Returning selected item"));
          return Item;
        }
        DTREE_LOG(EasyStr("DTree: SelectItemByPath: Expanding this item to get at its children"));
        EnsureItemExpanded(Item);
        Item=TreeView_GetChild(hTree,Item);
      }else{
        Item=TreeView_GetNextSibling(hTree,Item);
      }
    }
  }else{
    DTREE_LOG(EasyStr("DTree: SelectItemByPath: Couldn't select anything"));
  }
  return NULL;
}
//---------------------------------------------------------------------------
HTREEITEM DirectoryTree::InsertItem(char *t,HTREEITEM Parent,HTREEITEM After,int Image,bool ShowButton,LPARAM Data,UINT State)
{
  if (hTree==NULL) return NULL;

  TV_INSERTSTRUCT tvis;
  tvis.hParent=Parent;
  tvis.hInsertAfter=After;
  tvis.item.mask=TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_CHILDREN;
  tvis.item.pszText=t;
  tvis.item.lParam=Data;
  tvis.item.iImage=Image;
  tvis.item.iSelectedImage=Image;
  tvis.item.stateMask=0xffffffff;
  tvis.item.state=State;
  tvis.item.cChildren=ShowButton;
  return (HTREEITEM)SendTreeMessage(TVM_INSERTITEM,0,(LPARAM)&tvis);
}
//---------------------------------------------------------------------------
HTREEITEM DirectoryTree::NewItem(EasyStr ItemName,HTREEITEM Parent,int Type,bool Edit)
{
  if (hTree==NULL) return NULL;

  EnsureItemExpanded(Parent);

  EasyStr ParentFol=GetItemPath(Parent);
  EasyStr Ext,Name=ItemName;
  if (FileMasksESL[Type].String[0]) Ext=EasyStr(".")+FileMasksESL[Type].String;
  int n=2;
  while (Exists(ParentFol+"\\"+Name+Ext)) Name=ItemName+" ("+(n++)+")";

  if (Type==0){
    if (CreateDirectory(ParentFol+"\\"+Name,NULL)==0) return 0;
  }else{
    FILE *f=fopen(ParentFol+"\\"+Name+Ext,"wb");
    if (f==NULL) return NULL;
    fclose(f);
  }

  HTREEITEM Item=InsertItem(Name,Parent,TVI_LAST,Type);
  if (Item){
    if (Edit) SetFocus(hTree);
    SendTreeMessage(TVM_ENSUREVISIBLE,0,(LPARAM)Item);
    SendTreeMessage(TVM_SELECTITEM,TVGN_CARET,(LPARAM)Item);
    if (Edit) SendTreeMessage(TVM_EDITLABEL,0,(LPARAM)Item);
    return Item;
  }
  return NULL;
}
//---------------------------------------------------------------------------
bool DirectoryTree::DeleteItem(HTREEITEM Item)
{
  if (hTree==NULL) return 0;

  char Path[MAX_PATH+2];
  ZeroMemory(Path,MAX_PATH+2);
  strcpy(Path,GetItemPath(Item));
  EasyStr Mess=T("Deleting...");

  EnableWindow(hParent,0);
  SHFILEOPSTRUCT fos;
  fos.hwnd=HWND(PopupParent ? PopupParent:hParent);
  fos.wFunc=FO_DELETE;
  fos.pFrom=Path;
  fos.pTo="\0\0";
  fos.fFlags=FILEOP_FLAGS(int((GetKeyState(VK_SHIFT)<0) ? 0:FOF_ALLOWUNDO));
  fos.hNameMappings=NULL;
  fos.lpszProgressTitle=Mess.Text;
  if (SHFileOperation(&fos)==0){
    if (fos.fAnyOperationsAborted==0){
      EnableWindow(hParent,true);
      HTREEITEM Par=TreeView_GetParent(hTree,Item);
      SendTreeMessage(TVM_DELETEITEM,0,(LPARAM)Item);

      if (Par && Par!=RootItem){
        TV_ITEM Inf=GetItem(Par,TVIF_CHILDREN);
        if (TreeView_GetChild(hTree,Par)){
          Inf.cChildren=1;
        }else{
          Inf.cChildren=0;
        }
        SendTreeMessage(TVM_SETITEM,0,(LPARAM)&Inf);
        if (Inf.cChildren==0) SendTreeMessage(TVM_EXPAND,TVE_COLLAPSE,(LPARAM)Par);
      }
      return true;
    }
  }
  EnableWindow(hParent,true);
  return 0;
}
//---------------------------------------------------------------------------
bool DirectoryTree::DeleteChildrenOfItem(HTREEITEM Item)
{
  Item=TreeView_GetChild(hTree,Item);
  bool HadItems=(Item!=NULL);
  while (Item){
    HTREEITEM ItemToDel=Item;
    Item=TreeView_GetNextSibling(hTree,Item);
    SendTreeMessage(TVM_DELETEITEM,0,(LPARAM)ItemToDel);
  }
  return HadItems;
}
//---------------------------------------------------------------------------
void DirectoryTree::Destroy()
{
  if (ExpandTimerActive){
    KillTimer(hParent,9876);ExpandTimerActive=0;
  }
  if (hTree){
    HWND Win=hTree;
  	hTree=NULL;
    DestroyWindow(Win);
  }
  if (il) ImageList_Destroy(il);
  il=NULL;
}
//---------------------------------------------------------------------------
bool DirectoryTree::ProcessMessage(UINT Mess,WPARAM wPar,LPARAM lPar)
{
  if (hTree==NULL) return 0;
#ifdef DT_TEST
  if (TestStage<5) return 0;
#endif

  DTREE_LOG(EasyStr("DTree: ProcessMessage: Processing message ")+Mess);
  switch (Mess){
    case WM_CONTEXTMENU:
    {
      DTREE_LOG(EasyStr("DTree: ProcessMessage: WM_CONTEXTMENU"));
      if (HWND(wPar)!=hTree || ChooseOnly) break;

      HTREEITEM Target;  // handle of target item
      TV_HITTESTINFO hti;  // hit test information
      POINT spt,cpt;

      GetCursorPos(&spt);
      cpt=spt;
      ScreenToClient(hTree,&cpt);

      hti.pt=cpt;
      Target=TreeView_HitTest(hTree,&hti);
      if ((hti.flags & TVHT_ONITEM)==0) Target=NULL;

      HMENU Menu=CreatePopupMenu();

      MenuItem=Target;
      bool Root=0;

      if (Target){
        SendTreeMessage(TVM_SELECTITEM,TVGN_CARET,(LPARAM)Target);
        Root=(Target==RootItem);

        TV_ITEM Inf=GetItem(Target,TVIF_IMAGE);
        if (Inf.iImage==0){ // Folder
          AppendMenu(Menu,MF_BYPOSITION | MF_STRING,IDBase+2,T("&Add Sub-Folder"));
          AppendMenu(Menu,MF_BYPOSITION | MF_SEPARATOR,0,NULL);
          AppendMenu(Menu,MF_BYPOSITION | MF_STRING,IDBase+3,T("&Open Folder In Explorer"));
          if (Root==0) AppendMenu(Menu,MF_BYPOSITION | MF_SEPARATOR,0,NULL);
        }
        if (Root==0){
          AppendMenu(Menu,MF_BYPOSITION | MF_STRING,IDBase+4,T("Delete")+" \10DEL");
          AppendMenu(Menu,MF_BYPOSITION | MF_STRING,IDBase+5,T("&Rename")+" \10F2");
          AppendMenu(Menu,MF_BYPOSITION | MF_SEPARATOR,0,NULL);
        }
        if (Inf.iImage){ // File
          if (TreeView_GetParent(hTree,Target)!=RootItem){
            AppendMenu(Menu,MF_BYPOSITION | MF_STRING,IDBase+1,T("&New Folder Here"));
          }
        }
      }
      if (Root==0){
        AppendMenu(Menu,MF_BYPOSITION | MF_STRING,IDBase,T("New Folder In &Root"));
      }

      TrackPopupMenu(Menu,TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                      spt.x,spt.y,0,hParent,NULL);

      DestroyMenu(Menu);
      break;
    }
    case WM_NOTIFY:
    {
      if (wPar!=WPARAM(TreeID)) break;

      NMHDR *pnmh=(NMHDR*)lPar;
      DTREE_LOG(EasyStr("DTree: ProcessMessage: WM_NOTIFY, code=")+pnmh->code);
      switch (pnmh->code){
        case TVN_BEGINLABELEDIT:
        {
          WndProcRet=0;
          if (((TV_DISPINFO*)lPar)->item.hItem==RootItem) WndProcRet=TRUE;

          DTREE_LOG(EasyStr("DTree: ProcessMessage: Finished processing message ")+Mess);
          return true;
        }
        case TVN_ENDLABELEDIT:
        {
          TV_DISPINFO *DispInf=(TV_DISPINFO*)lPar;
          WndProcRet=0;

          EasyStr Name;
          Name.SetLength(500);
          TV_ITEM Inf=GetItem(DispInf->item.hItem,TVIF_TEXT | TVIF_IMAGE,Name.Text,500);

          EasyStr NewName;
          if (DispInf->item.pszText){
            NewName=DispInf->item.pszText;
          }else{
            NewName=Name;
          }

          EasyStr Ext;
          if (Inf.iImage){
            if (FileMasksESL[Inf.iImage].String[0]) Ext=EasyStr(".")+FileMasksESL[Inf.iImage].String;
          }

          HTREEITEM Par=TreeView_GetParent(hTree,DispInf->item.hItem);
          MoveItems(DispInf->item.hItem,Par,0,NewName+Ext);
          DTREE_LOG(EasyStr("DTree: ProcessMessage: Finished processing message ")+Mess);
          return true; // return WndProcRet
        }
        case TVN_SELCHANGED:
        {
          NM_TREEVIEW *Inf=(NM_TREEVIEW*)lPar;

          if (NotifyProc && DoNotifyOnChange){
            DTREE_LOG(EasyStr("DTree: ProcessMessage: Notifying of selection change"));
            HTREEITEM NewItem=Inf->itemNew.hItem;
            DTREE_LOG(EasyStr("                NewItem=")+DWORD(NewItem));
            HTREEITEM OldItem=Inf->itemOld.hItem;
            DTREE_LOG(EasyStr("                OldItem=")+DWORD(OldItem));
            NotifyProc(this,NotifyThis,DTM_SELCHANGED,int(NewItem),int(OldItem));
            DTREE_LOG(EasyStr("DTree: ProcessMessage: NotifyProc returned"));
          }
          break;
        }
        case TVN_BEGINDRAG:
          DragButton=1;
        case TVN_BEGINRDRAG:
        {
          if (DragButton==0) DragButton=2;
          NM_TREEVIEW *Inf=(NM_TREEVIEW*)lPar;

          {
            TV_HITTESTINFO hti;  // hit test information
            POINT mpt;
            GetCursorPos(&mpt);
            ScreenToClient(hTree,&mpt);

            hti.pt=mpt;
            if (TreeView_HitTest(hTree,&hti)!=Inf->itemNew.hItem) break;
            if ((hti.flags & TVHT_ONITEM)==0) break;
          }

          SetFocus(hTree);

          if (Inf->itemNew.hItem==RootItem) break;

          DragItem=Inf->itemNew.hItem;
          SetCapture(hParent);
          SetCursor(LoadCursor(NULL,IDC_ARROW));

          DragIL=(HIMAGELIST)SendTreeMessage(TVM_CREATEDRAGIMAGE,0,(LPARAM)DragItem);
          int w,h;
          ImageList_GetIconSize(DragIL,&w,&h);

          POINT pt={0,0};
          ClientToScreen(hTree,&pt);
          RECT rc;
          GetWindowRect(hParent,&rc);
          pt.x-=rc.left;pt.y-=rc.top;

          ImageList_BeginDrag(DragIL,0,-pt.x + w/2,-pt.y + h/2); // This is relative to hParent

          GetCursorPos(&pt);
          ScreenToClient(hTree,&pt);

          DropTarget=NULL;
          TreeView_SelectDropTarget(hTree,DropTarget);
          UpdateWindow(hTree);

          ImageList_DragEnter(hParent,pt.x,pt.y);
          DragEntered=true;
          break;
        }
        case TVN_ITEMEXPANDING:
        {
          NM_TREEVIEW *Inf=(NM_TREEVIEW*)lPar;
          if (Inf->itemNew.hItem!=RootItem){
            if (Inf->action==TVE_EXPAND){
              DTREE_LOG(EasyStr("DTree: ProcessMessage: Expanded folder ")+GetItemPath(Inf->itemNew.hItem));
              PutDirectoryInTree(Inf->itemNew.hItem,GetItemPath(Inf->itemNew.hItem));
            }
            WndProcRet=0;
          }else{
            WndProcRet=TRUE;
          }
          DTREE_LOG(EasyStr("DTree: ProcessMessage: Finished processing message ")+Mess);
          return true;
        }
        case TVN_ITEMEXPANDED:
        {
          NM_TREEVIEW *Inf=(NM_TREEVIEW*)lPar;
          if (Inf->itemNew.hItem!=RootItem){
            if (Inf->action==TVE_COLLAPSE){
              DTREE_LOG("DTree: PutDirectoryInTree: Checking for existing items and deleting");
              bool HadItems=DeleteChildrenOfItem(Inf->itemNew.hItem);
              TV_ITEM tvi;
              tvi.mask=TVIF_HANDLE | TVIF_CHILDREN;
              tvi.hItem=Inf->itemNew.hItem;
              tvi.cChildren=HadItems;
              SendTreeMessage(TVM_SETITEM,0,LPARAM(&tvi));
            }
          }
          WndProcRet=0;
          return true;
        }
        case TVN_KEYDOWN:
        {
          if (ChooseOnly) break;

          TV_KEYDOWN *KeyInf=(TV_KEYDOWN*)lPar;
          if (KeyInf->wVKey==VK_DELETE){
            MenuItem=TreeView_GetSelection(hTree);
            SendMessage(hParent,WM_COMMAND,IDBase+4,0);
          }else if (KeyInf->wVKey==VK_F2){
            MenuItem=TreeView_GetSelection(hTree);
            SendMessage(hParent,WM_COMMAND,IDBase+5,0);
          }else if (KeyInf->wVKey==VK_ESCAPE){
            DragEnd(true);
          }
          break;
        }
      }
      break;
    }
    case WM_MOUSEMOVE:
      if (DragItem) DragMove();
      break;
    case WM_TIMER:
    {
      DTREE_LOG(EasyStr("DTree: ProcessMessage: WM_TIMER"));
      if (wPar!=9876) break;
      KillTimer(hParent,9876);ExpandTimerActive=0;
      if (DropTarget==NULL) break;

      TV_ITEM Inf=GetItem(DropTarget,TVIF_IMAGE | TVIF_STATE);
      if (Inf.iImage==0 && (Inf.state & TVIS_EXPANDED)==0){
        ImageList_DragLeave(hParent);
        UpdateWindow(hParent);

        EnsureItemExpanded(DropTarget);

        POINT mpt;
        GetCursorPos(&mpt);
        ScreenToClient(hTree,&mpt);
        ImageList_DragEnter(hParent,mpt.x,mpt.y);
        UpdateWindow(hParent);
      }
      break;
    }
    case WM_COMMAND:
      DTREE_LOG(EasyStr("DTree: ProcessMessage: WM_COMMAND"));
      if (LOWORD(wPar)==IDCANCEL) DragEnd(true); // Esc
      if (LOWORD(wPar)>=IDBase && LOWORD(wPar)<IDBase+100){
        int ID=LOWORD(wPar)-IDBase;
        HTREEITEM Parent=NULL;
        switch (ID){
          case 0:
            Parent=RootItem;
          case 1:
            if (Parent==NULL) Parent=TreeView_GetParent(hTree,MenuItem);
          case 2:
          {
            if (Parent==NULL) Parent=MenuItem;

            NewItem(T("New Folder"),Parent,0);
            break;
          }
          case 3:
            ShellExecute(NULL,NULL,GetItemPath(MenuItem),NULL,NULL,SW_SHOW);
            break;
          case 4:
          {
            // When we delete the selected item the selection changes to another one
            // Don't send TVN_SELCHANGED notify as the OldItem is bad
            DoNotifyOnChange=0;
            EasyStr ItemPath=GetItemPath(MenuItem);
            if (DeleteItem(MenuItem)){
              HTREEITEM NewSel=TreeView_GetSelection(hTree);
              if (NewSel==NULL){
                NewSel=RootItem;
                SendTreeMessage(TVM_SELECTITEM,TVGN_CARET,(LPARAM)NewSel);
              }
              if (NotifyProc){
                NotifyProc(this,NotifyThis,DTM_ITEMDELETED,int(ItemPath.Text),0);
                NotifyProc(this,NotifyThis,DTM_SELCHANGED,(int)NewSel,0);
              }
            }
            DoNotifyOnChange=true;
            break;
          }
          case 5:
            SendTreeMessage(TVM_EDITLABEL,0,(LPARAM)MenuItem);
            break;
          case 10:case 11:
            MenuItem=(HTREEITEM)ID;
            break;
        }
      }
      break;
    case WM_LBUTTONUP:
      if (DragButton==1) DragEnd(0);
      break;
    case WM_RBUTTONUP:
      if (DragButton==2) DragEnd(0);
      break;
    case WM_LBUTTONDOWN:
      if (DragButton==2) DragEnd(true);
      break;
    case WM_RBUTTONDOWN:
      if (DragButton==1) DragEnd(true);
      break;
    case WM_CAPTURECHANGED:
      DragEnd(true);
      break;
  }
  DTREE_LOG(EasyStr("DTree: ProcessMessage: Finished processing message ")+Mess);
  return 0;
}
//---------------------------------------------------------------------------
LRESULT __stdcall DirectoryTree::TVWndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  DTREE_LOG(EasyStr("DTree: TVWndProc: Processing message ")+Mess);
  DirectoryTree *This=(DirectoryTree*)GetProp(Win,"DirectoryTreeThis");
  if (This==NULL) return 0;
  switch (Mess){
    case WM_LBUTTONDOWN:case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:case WM_RBUTTONDBLCLK:
    {
      DTREE_LOG(EasyStr("DTree: TVWndProc: WM_LBUTTONDOWN/WM_LBUTTONDBLCLK/WM_RBUTTONDOWN/WM_RBUTTONDBLCLK"));
      TV_HITTESTINFO hti;
      HTREEITEM Item;

      DT_TEST_STAGE(6){
        hti.pt.x=LOWORD(lPar);hti.pt.y=HIWORD(lPar);
        Item=(HTREEITEM)SendMessage(Win,TVM_HITTEST,0,(LPARAM)&hti);
        if ( !(hti.flags==TVHT_ONITEMBUTTON && Mess==WM_LBUTTONDOWN)){
          if (Item==NULL || (hti.flags & TVHT_ONITEM)==0){
            SendMessage(Win,TVM_ENDEDITLABELNOW,1,0);
            SetFocus(Win);
            DTREE_LOG(EasyStr("DTree: TVWndProc: Finished processing message ")+Mess);
            return 0;
          }
        }
      }
      DT_TEST_STAGE(7){
        if (This->AllowTypeChange && (Mess==WM_LBUTTONDOWN || Mess==WM_LBUTTONDBLCLK) &&
              (hti.flags & TVHT_ONITEMICON)){
          int Type=This->GetItem(Item,TVIF_IMAGE).iImage;
          if (Type>0){
            EasyStr Ext=This->GetItemPath(Item);
            char *dot=strrchr(Ext,'.');
            if (dot) Ext=dot+1;
            int t=Type+1;
            for(;;){
              if (t>=This->FileMasksESL.NumStrings) t=1;
              if (t==Type) break;
              if (IsSameStr_I(This->FileMasksESL[t].String,Ext)){
                TV_ITEM tvi;
                tvi.hItem=Item;
                tvi.mask=TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_HANDLE;
                tvi.iImage=t;
                tvi.iSelectedImage=t;
                This->SendTreeMessage(TVM_SETITEM,0,LPARAM(&tvi));
                if (This->NotifyProc) This->NotifyProc(This,This->NotifyThis,DTM_TYPECHANGED,(int)Item,0);
                break;
              }
              t++;
            }
            DTREE_LOG(EasyStr("DTree: TVWndProc: Finished processing message ")+Mess);
            return 0;
          }
        }
      }
      break;
    }
    case WM_VSCROLL:
    {
      DTREE_LOG(EasyStr("DTree: TVWndProc: WM_VSCROLL"));
      if (This->DragEntered==0) break;

      ImageList_DragLeave(This->hParent);

      LRESULT Ret=CallWindowProc(This->OldTVWndProc,Win,Mess,wPar,lPar);
      UpdateWindow(Win);

      POINT mpt;
      GetCursorPos(&mpt);
      ScreenToClient(Win,&mpt);
      ImageList_DragEnter(This->hParent,mpt.x,mpt.y);
      This->DragMove();

      DTREE_LOG(EasyStr("DTree: TVWndProc: Finished processing message ")+Mess);
      return Ret;
    }
    case 0x3d: //WM_GETOBJECT:
    {
      // For some reason passing this to the OldWndProc was causing a crash
      // on Windows 98. MS says if you aren't interested pass it to the
      // DefWindowProc, hope it works properly!
      DTREE_LOG("DTree: TVWndProc: WM_GETOBJECT, calling DefWindowProc");
      LRESULT Ret=DefWindowProc(GetParent(Win),Mess,wPar,lPar);
      DTREE_LOG(EasyStr("DTree: TVWndProc: DefWindowProc returned ")+Ret);
      return Ret;
    }
    case WM_DESTROY:
    {
      DTREE_LOG(EasyStr("DTree: TVWndProc: WM_DESTROY"));
      if (This->hTree){
        This->hTree=NULL;
        This->Destroy();
      }
      RemoveProp(Win,"DirectoryTreeThis");
      break;
    }
  }
  DTREE_LOG(EasyStr("DTree: TVWndProc: Finished processing message ")+Mess+" passing to WndProc");
  LRESULT Ret=CallWindowProc(This->OldTVWndProc,Win,Mess,wPar,lPar);
  DTREE_LOG(EasyStr("DTree: TVWndProc: WndProc returned ")+Ret);
  return Ret;
}
//---------------------------------------------------------------------------
void DirectoryTree::DragMove()
{
  if (DragItem==NULL || hTree==NULL) return;

  DTREE_LOG("DTree: DragMove");

  TV_HITTESTINFO hti;  // hit test information
  POINT mpt;
  HTREEITEM OldDropTarget=DropTarget;
  HCURSOR NewCursor=NULL;
  HTREEITEM DragPar=TreeView_GetParent(hTree,DragItem);

  GetCursorPos(&mpt);
  ScreenToClient(hTree,&mpt);

  hti.pt=mpt;
  DropTarget=TreeView_HitTest(hTree,&hti);
  if (DropTarget){
    // Is DropTarget decended from DragItem? Can't move a folder to its own sub-folder!
    if (ItemIsChild(DragItem,DropTarget)){
      NewCursor=LoadCursor(NULL,IDC_NO);
      DropPos=DT_DROP_NOWHERE;
      DropTarget=DragPar;
    }else{
      TV_ITEM DropInf=GetItem(DropTarget,TVIF_IMAGE,NULL,0);
      if (DropInf.iImage){ // File
        DropTarget=TreeView_GetParent(hTree,DropTarget);
      }
      if ((DropTarget==DragPar && DragButton==1) || DropTarget==DragItem){
        NewCursor=LoadCursor(NULL,IDC_NO);
        DropPos=DT_DROP_NOWHERE;
      }else{
        NewCursor=LoadCursor(NULL,IDC_ARROW);
        DropPos=DT_DROP_ASCHILD;
      }
    }
  }else{
    switch (hti.flags){
      case TVHT_ABOVE:case TVHT_BELOW:
      case TVHT_TOLEFT:case TVHT_TORIGHT:
        if (DragEntered){
          ImageList_DragLeave(hParent);
          DragEntered=0;
        }
        SetCursor(LoadCursor(NULL,IDC_NO));
        DropPos=DT_DROP_NOWHERE;
        if (hti.flags==TVHT_ABOVE){
          SendTreeMessage(TVM_SELECTITEM,TVGN_FIRSTVISIBLE,(LPARAM)
                          TreeView_GetPrevVisible(hTree,
                                  TreeView_GetFirstVisible(hTree)));
        }else if (hti.flags==TVHT_BELOW){
          SendTreeMessage(TVM_SELECTITEM,TVGN_FIRSTVISIBLE,(LPARAM)
                          TreeView_GetNextVisible(hTree,
                                  TreeView_GetFirstVisible(hTree)));
        }else if (hti.flags==TVHT_TOLEFT){
          SendTreeMessage(WM_HSCROLL,(UINT)SB_LINELEFT,0);
        }else if (hti.flags==TVHT_TORIGHT){
          SendTreeMessage(WM_HSCROLL,(UINT)SB_LINERIGHT,0);
        }
        UpdateWindow(hTree);
        break;
      case TVHT_NOWHERE:
      {
        DropTarget=RootItem;
        if ((DropTarget==DragPar && DragButton==1) || DropTarget==DragItem){
          NewCursor=LoadCursor(NULL,IDC_NO);;
          DropPos=DT_DROP_NOWHERE;
        }else{
          NewCursor=LoadCursor(NULL,IDC_ARROW);
          DropPos=DT_DROP_ASCHILD;
        }
      }
    }
  }
  if (OldDropTarget!=DropTarget){
    if (ExpandTimerActive) KillTimer(hParent,9876);
    SetTimer(hParent,9876,1000,NULL);ExpandTimerActive=true;
    ImageList_DragLeave(hParent);

    TreeView_SelectDropTarget(hTree,DropTarget);
    UpdateWindow(hTree);

    if (NewCursor) SetCursor(NewCursor);
    if (DropTarget){
      ImageList_DragEnter(hParent,mpt.x,mpt.y);
      DragEntered=true;
    }else{
      DragEntered=0;
    }
  }else{
    if (NewCursor) SetCursor(NewCursor);
    if (DragEntered) ImageList_DragMove(mpt.x,mpt.y);
  }
  DTREE_LOG("DTree: DragMove Finished");
}
//---------------------------------------------------------------------------
void DirectoryTree::DragEnd(bool Cancel)
{
  DTREE_LOG("DTree: DragEnd");
  static bool InDragEnd=0;

  if (InDragEnd || DragItem==NULL || hTree==NULL) return;
  InDragEnd=true;

  if (DragEntered) ImageList_DragLeave(hParent);
  ImageList_EndDrag();
  ImageList_Destroy(DragIL);
  DragEntered=0;

  HTREEITEM Src=DragItem,Dest=DropTarget;

  DragItem=NULL;
  DropTarget=NULL;
  ReleaseCapture();

  HTREEITEM EditItem=NULL;
  if (DropPos==DT_DROP_ASCHILD && Cancel==0 && Dest){
    bool Copy=0,SameFol=0;
    if (DragButton==2){
      MenuItem=NULL;
      HMENU Menu=CreatePopupMenu();

      EasyStr SrcFol=GetItemPath(Src);
      RemoveFileNameFromPath(SrcFol,REMOVE_SLASH);
      SameFol=IsSameStr_I(SrcFol,GetItemPath(Dest));
      if (SameFol==0){
        // Can't move it to the same place, can only make copy
        AppendMenu(Menu,MF_BYPOSITION | MF_STRING,IDBase+10,T("&Move Here"));
      }
      AppendMenu(Menu,MF_BYPOSITION | MF_STRING,IDBase+11,T("&Copy Here"));
      AppendMenu(Menu,MF_BYPOSITION | MF_SEPARATOR,0,NULL);
      AppendMenu(Menu,MF_BYPOSITION | MF_STRING,IDBase+12,T("Cancel"));

      POINT spt;
      GetCursorPos(&spt);
      TrackPopupMenu(Menu,TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                      spt.x,spt.y,0,hParent,NULL);

      DestroyMenu(Menu);

      MSG mess;
      while (PeekMessage(&mess,hParent,WM_COMMAND,WM_COMMAND,PM_REMOVE)) DispatchMessage(&mess);
      if (MenuItem==NULL){
        Cancel=true;
      }else{
        if ((int)MenuItem == 11) Copy=true;
      }
    }
    if (Cancel==0){
      EditItem=MoveItems(Src,Dest,Copy);
      if (SameFol==0) EditItem=NULL;
    }
  }
  TreeView_SelectDropTarget(hTree,NULL);

  DragButton=0;

  if (EditItem) SendTreeMessage(TVM_EDITLABEL,0,(LPARAM)EditItem);

  InDragEnd=0;
  DTREE_LOG("DTree: DragEnd Finished");
}
//---------------------------------------------------------------------------
HTREEITEM DirectoryTree::MoveItems(HTREEITEM Item,HTREEITEM NewParent,bool Copy,EasyStr NewName)
{
  if (hTree==NULL) return NULL;

  DTREE_LOG("DTree: MoveItems");

  EnsureItemExpanded(NewParent);

  TV_ITEM SrcInf;

  char From[MAX_PATH+2];
  ZeroMemory(From,MAX_PATH+2);
  strcpy(From,GetItemPath(Item));
  EasyStr DestFol=GetItemPath(NewParent);
  if (NewName.Empty()) NewName=strrchr(From,'\\')+1;

  EasyStr SrcFol=From;
  RemoveFileNameFromPath(SrcFol,REMOVE_SLASH);

  EasyStr Dest=DestFol+"\\"+NewName;
  if (IsSameStr_I(SrcFol,DestFol) && Copy){
    Dest=DestFol;
    EasyStr Ext;
    char *dot=strrchr(NewName,'.');
    if (dot){
      Ext=dot;
      *dot=0;
    }
    EasyStr f;
    int i=2;
    do{
      f=NewName+" ("+(i++)+")"+Ext;
    }while (Exists(Dest+"\\"+f));
    NewName=f;
    Dest+=EasyStr("\\")+NewName;
  }

  if (IsSameStr_I(SrcFol,DestFol) && Copy==0){
    // Renaming, quicker to do it like this
    if (NotSameStr(From,Dest)) if (MoveFile(From,Dest)==0) return NULL;
  }else{
    EasyStr Mess=LPSTR(Copy ? T("Copying..."):T("Moving..."));
    SHFILEOPSTRUCT fos;
    fos.hwnd=HWND(PopupParent ? PopupParent:hParent);
    fos.wFunc=int(Copy ? FO_COPY:FO_MOVE);
    fos.pFrom=From;
    fos.pTo=Dest;
    fos.fFlags=FILEOP_FLAGS(FOF_ALLOWUNDO);
    fos.hNameMappings=NULL;
    fos.lpszProgressTitle=Mess.Text;
    EnableWindow(hParent,0);
    int Ret=SHFileOperation(&fos);
    EnableWindow(hParent,true);
    if (Ret || fos.fAnyOperationsAborted) return NULL;
  }

  HTREEITEM NewItem,After;
  SrcInf=GetItem(Item,TVIF_IMAGE);

  SendTreeMessage(WM_SETREDRAW,0,0);

  // Put into alphabetical order
  After=TreeView_GetChild(hTree,NewParent);
  if (After==NULL){
    After=TVI_FIRST;
  }else{
    HTREEITEM InsertPos=TVI_FIRST;
    TV_ITEM Inf;
    if (SrcInf.iImage){ // File, skip past folders
      do{
        Inf=GetItem(After,TVIF_IMAGE);
        if (Inf.iImage) break;
        InsertPos=After;
        After=TreeView_GetNextSibling(hTree,After);
      }while (After);
    }
    if (After){
      char Text[501];
      do{
        Inf=GetItem(After,TVIF_TEXT | TVIF_IMAGE,Text,500);
        if (strcmpi(NewName,Text)<0 || (Inf.iImage && SrcInf.iImage==0)){
          After=InsertPos;
          break;
        }
        InsertPos=After;
        After=TreeView_GetNextSibling(hTree,After);
      }while (After);
    }
    if (After==NULL) After=TVI_LAST;
  }

  DoNotifyOnChange=0;

  HTREEITEM SelItem=NULL;
  HTREEITEM OldParent=TreeView_GetParent(hTree,Item),OldItem=Item;
  if (After!=Item || Copy){
    NewItem=CopyItemAndChildren(Item,NewParent,After,&SelItem);
    if (Copy==0) SendTreeMessage(TVM_DELETEITEM,0,(LPARAM)Item);
    Item=NewItem;
  }

  { // Set the name
    SrcInf.mask=TVIF_TEXT | TVIF_HANDLE;
    SrcInf.hItem=Item;

    EasyStr DispName=NewName;
    if (SrcInf.iImage){
      char *dot=strrchr(DispName,'.');
      if (dot) *dot=0;
    }
    SrcInf.pszText=DispName;
    SendTreeMessage(TVM_SETITEM,0,(LPARAM)&SrcInf);
    if (Copy==0 && NotifyProc){
      NotifyProc(this,NotifyThis,DTM_NAMECHANGED,(int)Item,0);
      // if folder have to send different name changed message for all
      // files inside folder. This is so program can maintain lists of files.
      if (SrcInf.iImage==0) NotifyProc(this,NotifyThis,DTM_FOLDERMOVED,(int)From,(int)(DestFol+SLASH+NewName).Text);
    }
  }

  if (NotifyProc && AllowTypeChange){
    if (SrcInf.iImage){
      // Update for file
      SrcInf.mask=TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_HANDLE;
      SrcInf.hItem=Item;
      EasyStr FullPath=GetItemPath(Item);
      DTREE_LOG(EasyStr("DTree: MoveItem: Requesting type for ")+FullPath);
      int newn=NotifyProc(this,NotifyThis,DTM_GETTYPE,int(FullPath.Text),SrcInf.iImage);
      if (newn) SrcInf.iImage=newn;
      SrcInf.iSelectedImage=SrcInf.iImage;
      SendTreeMessage(TVM_SETITEM,0,(LPARAM)&SrcInf);
    }else{
      // Update all items in this folder!
    }
  }

  if (OldParent && OldParent!=RootItem && Copy==0){
    // Update the +/- button for the folder we moved from
    SrcInf.mask=TVIF_CHILDREN | TVIF_HANDLE;
    SrcInf.hItem=OldParent;
    if (TreeView_GetChild(hTree,OldParent)){
      SrcInf.cChildren=1;
    }else{
      SrcInf.cChildren=0;
    }
    SendTreeMessage(TVM_SETITEM,0,(LPARAM)&SrcInf);
    if (SrcInf.cChildren==0) SendTreeMessage(TVM_EXPAND,TVE_COLLAPSE,(LPARAM)OldParent);
  }
  if (NewParent!=RootItem){
    // Update the +/- button for the folder we moved/copied to
    SrcInf.mask=TVIF_CHILDREN | TVIF_HANDLE;
    SrcInf.hItem=NewParent;
    SrcInf.cChildren=1;
    SendTreeMessage(TVM_SETITEM,0,(LPARAM)&SrcInf);
  }

  SendTreeMessage(WM_SETREDRAW,1,0);
  InvalidateRect(hTree,NULL,true);
  SendTreeMessage(TVM_ENSUREVISIBLE,0,(LPARAM)Item);
  if (SelItem){
    SendTreeMessage(TVM_SELECTITEM,TVGN_CARET,(LPARAM)SelItem);
    // Don't need selchange if you are moving, as it is the same file
    if (Copy && NotifyProc) NotifyProc(this,NotifyThis,DTM_SELCHANGED,(int)SelItem,(int)OldItem);
  }

  DTREE_LOG("DTree: MoveItems Finished");

  DoNotifyOnChange=true;

  return Item;
}
//---------------------------------------------------------------------------
HTREEITEM DirectoryTree::CopyItemAndChildren(HTREEITEM Item,HTREEITEM NewParent,HTREEITEM After,
                                HTREEITEM *pSelItem)
{
  static char buf[501];
  static TV_ITEM Inf;
  HTREEITEM NextItem,NewItem;
  bool SelectThis=0;

  Inf.mask=TVIF_TEXT | TVIF_STATE | TVIF_CHILDREN | TVIF_PARAM | TVIF_IMAGE | TVIF_HANDLE;
  Inf.hItem=Item;
  Inf.pszText=buf;
  Inf.cchTextMax=500;
  Inf.stateMask=0xffffffff;
  SendTreeMessage(TVM_GETITEM,0,(LPARAM)&Inf);
  if (Inf.state & TVIS_SELECTED){
    SelectThis=true;
    Inf.state&=~TVIS_SELECTED; // Don't select now, wait until later
  }
  NewItem=InsertItem(buf,NewParent,After,Inf.iImage,Inf.cChildren,Inf.lParam,Inf.state);
  if (SelectThis) *pSelItem=NewItem;

  NextItem=TreeView_GetChild(hTree,Item);
  while (NextItem){
    CopyItemAndChildren(NextItem,NewItem,TVI_LAST,pSelItem);
    NextItem=TreeView_GetNextSibling(hTree,NextItem);
  }
  return NewItem;
}

#endif

