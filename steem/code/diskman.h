#define DISK_UNCOMPRESSED 1
#define DISK_COMPRESSED 2

#define FileIsDisk(s) ExtensionIsDisk(strrchr(s,'.'))
inline int ExtensionIsDisk(char*);

typedef struct{
  EasyStr Name,Path,LinkPath;
  bool UpFolder,Folder,ReadOnly,BrokenLink,Zip;
  int Image;

// Could implement the next few, in a million years!
//
//  EasyStr IconPath;int IconIdx;
//  EasyStr Description
}DiskManFileInfo;

#define DISKVIEWSCROLL_TIMER_ID 1
#define MSACONV_TIMER_ID 2

class TDiskManager : public TStemDialog
{
private:
  EasyStr HistBack[10],HistForward[10];
  void PerformInsertAction(int,EasyStr,EasyStr,EasyStr);
  void ExtractArchiveToSTHardDrive(Str);
  static void GCGetCRC(char*,DWORD*,int);

#ifdef WIN32
  static LRESULT __stdcall WndProc(HWND,UINT,WPARAM,LPARAM);
  static LRESULT __stdcall Drive_Icon_WndProc(HWND,UINT,WPARAM,LPARAM);
  static LRESULT __stdcall DiskView_WndProc(HWND,UINT,WPARAM,LPARAM);
  static LRESULT __stdcall DriveView_WndProc(HWND,UINT,WPARAM,LPARAM);
  static LRESULT __stdcall Dialog_WndProc(HWND,UINT,WPARAM,LPARAM);
  static int CALLBACK CompareFunc(LPARAM,LPARAM,LPARAM);
  void BeginDrag(int,HWND),MoveDrag(),EndDrag(int,int,bool);
  HRESULT CreateLinkCheckForOverwrite(char *,char *,IShellLink *,IPersistFile *);
  bool ImportDiskExists(char *,EasyStr &);
  bool DoCreateMultiLinks(),DoImport();
  void AddFoldersToMenu(HMENU,int,EasyStr,bool);
  bool MoveOrCopyFile(bool,char*,char*,char*,bool);
  void PropShowFileInfo(int);
  void AddFileOrFolderContextMenu(HMENU,DiskManFileInfo *);
  void UpdateBPBFiles(Str,Str,bool);
  void ManageWindowClasses(bool);
  Str GetMSAConverterPath();
  void GoToDisk(Str,bool);

  WINDOWPROC Old_ListView_WndProc;
  HIMAGELIST il[2];

  int Dragging,DragWidth,DragHeight,DropTarget;
  HWND DragLV;
  bool DragEntered,EndingDrag;
  HIMAGELIST DragIL;
  int LastOverID;
  int MenuTarget;
  EasyStringList MenuESL;

  Str MSAConvPath;
  HANDLE MSAConvProcess;
  Str MSAConvSel;

  bool Importing;
#elif defined(UNIX)
	int HistBackLength,HistForwardLength;

  static int WinProc(TDiskManager*,Window,XEvent*);

  void set_path(EasyStr,bool=true,bool=true);
  void UpdateDiskNames(int);
  void ToggleReadOnly(int);
  Str GetCustomDiskImage(int*,int*,int*);

  static int dir_lv_notify_handler(hxc_dir_lv*,int,int);
  static int button_notify_handler(hxc_button*,int,int*);
	static int menu_popup_notifyproc(hxc_popup*,int,int);

  int ArchiveTypeIdx;
  bool TempEject_InDrive[2];
  Str TempEject_Name,TempEject_DiskInZip[2];

  hxc_dir_lv dir_lv;
  hxc_button UpBut,BackBut,ForwardBut,eject_but[2];
  hxc_button DirOutput,disk_name[2],drive_icon[2];
  hxc_button HomeBut,SetHomeBut,MenuBut;
#endif
public:
  TDiskManager();
  ~TDiskManager() { Hide(); }
  void Show(),Hide();
  bool ToggleVisible();
  bool LoadData(bool,GoodConfigStoreFile*,bool* = NULL),SaveData(bool,ConfigStoreFile*);
  void SwapDisks(int);
  bool InsertDisk(int,EasyStr,EasyStr,bool=0,bool=true,EasyStr="",bool=0,bool=0);
  void EjectDisk(int);
  bool AreNewDisksInHistory(int);
  void InsertHistoryAdd(int,char *,char *,char* = "");
  void InsertHistoryDelete(int,char *,char *,char* = "");
  bool CreateDiskImage(char *,int,int,int);
  EasyStr CreateDiskName(char *,char *);
  void SetNumFloppies(int);
  void ExtractDisks(Str);
  
#ifdef WIN32
  bool HasHandledMessage(MSG*);
  void SetDir(EasyStr,bool,EasyStr="",bool=0,EasyStr="",int=0);
  bool SelectItemWithPath(char *,bool=0,char* = NULL);
  bool SelectItemWithLinkPath(char *LinkPath,bool EditLabel=0)
  {
    return SelectItemWithPath(NULL,EditLabel,LinkPath);
  }
  void RefreshDiskView(EasyStr SelPath="",bool EditLabel=0,EasyStr SelLinkPath="",int iItem=0);
  int GetSelectedItem();
  DiskManFileInfo *GetItemInf(int iItem,HWND LV=NULL)
  {
    LV_ITEM lvi;
    lvi.iItem=iItem;
    lvi.iSubItem=0;
    lvi.mask=LVIF_PARAM;
    SendMessage(HWND(LV ? LV:DiskView),LVM_GETITEM,0,(LPARAM)&lvi);
    return (DiskManFileInfo*)lvi.lParam;
  }
  void ShowContentDiag(),ShowDiskDiag(),ShowLinksDiag(),ShowImportDiag(),ShowPropDiag();
  int GetDiskSelectionSize();
  void SetDiskViewMode(int);
  void LoadIcons();

  void SetDriveViewEnable(int,bool);

  HWND DiskView;
  HICON DriveIcon[2],AccurateFDCIcon,DisableDiskIcon;
  HWND ContentDiag,DiskDiag,LinksDiag,ImportDiag,PropDiag,DiagFocus;

  HWND VisibleDiag() { return HWND(long(DiskDiag) | long(LinksDiag) | long(ImportDiag) | long(PropDiag) | long(ContentDiag)); }

  bool AtHome;
  bool ExplorerFolders;


  EasyStr WinSTonPath,WinSTonDiskPath,ImportPath;
  bool ImportOnlyIfExist;
  int ImportConflictAction,ContentConflictAction;

  EasyStr MultipleLinksPath,LinksTargetPath,ContentsLinksPath;

  bool DoExtraShortcutCheck;
#elif defined(UNIX)
	hxc_button HardBut;
	void RefreshDiskView(Str="");
#endif
	bool HideBroken,CloseAfterIRR;
  int SaveScroll;
  EasyStr SaveSelPath;

  int Width,Height,FSWidth,FSHeight;
  bool Maximized,FSMaximized;

  EasyStr DisksFol,HomeFol,ContentListsFol;
  EasyStr QuickFol[10];
  struct _DM_INSERT_STRUCT{
    EasyStr Name,Path,DiskInZip;
  }InsertHist[2][10];

  DiskManFileInfo PropInf;
  BPBINFO bpbi,file_bpbi,final_bpbi;

  bool SmallIcons,AutoInsert2;
  int IconSpacing;
  bool EjectDisksWhenQuit;
  WORD BytesPerSectorIdx,SecsPerTrackIdx,TracksIdx,SidesIdx;

  EasyStringList contents_sl;

  int DoubleClickAction;
};

#ifdef WIN32
void TDiskManager::RefreshDiskView(EasyStr SelPath,bool EditLabel,EasyStr SelLinkPath,int iItem)
{
  SetDir(DisksFol,0,SelPath,EditLabel,SelLinkPath,iItem);
}
#endif

