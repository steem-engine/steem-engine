//---------------------------------------------------------------------------
class TPatchesBox : public TStemDialog
{
private:
  void GetPatchText(char*,Str[4]);
#if defined(WIN32)
  static LRESULT __stdcall WndProc(HWND,UINT,WPARAM,LPARAM);
  void ManageWindowClasses(bool);
#elif defined(UNIX)
  static int WinProc(TPatchesBox*,Window,XEvent*);
	static int ListviewNotifyHandler(hxc_listview*,int,int);
	static int ButtonNotifyHandler(hxc_button*,int,int[]);

  hxc_button PatchLabel,DescLabel,ApplyWhenLabel,VersionLabel,AuthorLabel;
  hxc_listview PatchList;
  hxc_button ApplyBut;
	hxc_textdisplay DescText,ApplyWhenText,VersionText,AuthorText;
  hxc_button PatchDirLabel,PatchDirText,PatchDirBut;
#endif
public:
  TPatchesBox();
  ~TPatchesBox() { Hide(); }
  void Show(),Hide();
  bool ToggleVisible(){ IsVisible() ? Hide():Show();return IsVisible(); }
  bool LoadData(bool,GoodConfigStoreFile*,bool* = NULL),SaveData(bool,ConfigStoreFile*);

  EasyStr SelPatch,PatchDir;

  void PatchPoke(MEM_ADDRESS &,int,DWORD);
  void RefreshPatchList();
  void ShowPatchFile();
  void ApplyPatch();

#ifdef WIN32
  EasyStr GetPatchVersion();
  void SetButtonIcon();
#elif defined(UNIX)
#endif
};

