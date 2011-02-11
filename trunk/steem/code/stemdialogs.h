#define MAX_DIALOGS 20

#define SD_REGISTER 0
#define SD_UNREGISTER 1
//---------------------------------------------------------------------------
WIN_ONLY( bool StemDialog_RetDefVal; )

class TStemDialog
{
private:
public:
  TStemDialog();
  ~TStemDialog(){};
  void LoadPosition(GoodConfigStoreFile*),SavePosition(bool,ConfigStoreFile*),SaveVisible(ConfigStoreFile*);

#ifdef WIN32
  void CheckFSPosition(HWND);
  void RegisterMainClass(WNDPROC,char*,int);
  static LRESULT DefStemDialogProc(HWND,UINT,WPARAM,LPARAM);
  void MakeParent(HWND),UpdateMainWindowIcon();
  void ChangeParent(HWND);
  bool IsVisible(){return Handle!=NULL;}
  inline bool HasHandledMessage(MSG *);
  inline bool HandleIsInvalid();

  HTREEITEM AddPageLabel(char *,int);
  void DestroyCurrentPage();
  void GetPageControlList(DynamicArray<HWND> &);
  void ShowPageControls(),SetPageControlsFont();
  void UpdateDirectoryTreeIcons(DirectoryTree*);

  HWND Handle,Focus,PageTree;
  HFONT Font;
  int nMainClassIcon;
#elif defined(UNIX)
  bool IsVisible(){ return Handle!=0;}
  Pixmap IconPixmap;
  Pixmap IconMaskPixmap;

  void StandardHide();

  bool StandardShow(int w,int h,char* name,
      int icon_index,long input_mask,LPWINDOWPROC WinProc,bool=false);


  Window Handle;
#endif

  int Left,Top;
  int FSLeft,FSTop;
  EasyStr Section;
};
//---------------------------------------------------------------------------
TStemDialog *DialogList[MAX_DIALOGS]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
int nStemDialogs=0;

#ifdef WIN32
// For some reason in 24-bit and 32-bit screen modes on XP ILC_COLOR24 and
// ILC_COLOR32 icons don't highlight properly, have to be 16-bit.
const UINT BPPToILC[5]={0,ILC_COLOR4,ILC_COLOR16,ILC_COLOR16,ILC_COLOR16};
#endif

