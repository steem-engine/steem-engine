#ifndef DIRECTORY_TREE_H
#define DIRECTORY_TREE_H

#define DTM_SELCHANGED 0
#define DTM_NAMECHANGED 1
#define DTM_TYPECHANGED 2
#define DTM_GETTYPE 3
#define DTM_FOLDERMOVED 4
#define DTM_ITEMDELETED 5

class DirectoryTree;

typedef int(*PDTNOTIFYPROC)(DirectoryTree*,void*,int,int,int);

class DirectoryTree
{
private:
  static LPARAM __stdcall TVWndProc(HWND,UINT,WPARAM,LPARAM);
  bool FindFilesAndFolders(EasyStr,EasyStringList&,EasyStringList&,bool);
  void PutDirectoryInTree(HTREEITEM,EasyStr);
  bool ItemIsChild(HTREEITEM,HTREEITEM);
  HTREEITEM InsertItem(char *,HTREEITEM,HTREEITEM,int,bool=0,LPARAM=0,UINT=0);
  bool DeleteItem(HTREEITEM);
  bool DeleteChildrenOfItem(HTREEITEM);
  HTREEITEM MoveItems(HTREEITEM,HTREEITEM,bool,EasyStr="");
  HTREEITEM CopyItemAndChildren(HTREEITEM,HTREEITEM,HTREEITEM,HTREEITEM *);
  void DragEnd(bool),DragMove();

  HTREEITEM MenuItem,DragItem,DropTarget;
  HIMAGELIST DragIL,il;
  bool DragEntered;
  int DragButton;
  bool DoNotifyOnChange,ExpandTimerActive;
  int DropPos;
  WNDPROC OldTVWndProc;
public:
  DirectoryTree();
  ~DirectoryTree();

  bool Create(HWND,int,int,int,int,int,DWORD,PDTNOTIFYPROC,void*,EasyStr,EasyStr,bool=0);
  void Destroy();
  bool ProcessMessage(UINT,WPARAM,LPARAM);
  void RefreshDirectory();
  TV_ITEM GetItem(HTREEITEM,int,char* = NULL,int=0);
  EasyStr GetItemPath(HTREEITEM);
  HTREEITEM NewItem(EasyStr,HTREEITEM,int,bool=true);
  HTREEITEM SelectItemByPath(EasyStr);
  LRESULT SendTreeMessage(UINT m,WPARAM w,LPARAM l);
  void EnsureItemExpanded(HTREEITEM Item);
  void ReloadIcons(UINT);

  HWND hTree,hParent;
  LRESULT WndProcRet;
  int TreeID;
  PDTNOTIFYPROC NotifyProc;
  void *NotifyThis;
  int IDBase;
  bool ChooseOnly,AllowTypeChange;
  HTREEITEM RootItem;

  EasyStr RootFol;
  EasyStringList FileMasksESL;

  static HWND PopupParent;
};
#endif

