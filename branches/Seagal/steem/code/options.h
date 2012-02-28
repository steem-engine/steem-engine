//---------------------------------------------------------------------------
class TOptionBox : public TStemDialog
{
private:
  int page_l,page_w;

#ifdef WIN32
  static LRESULT __stdcall WndProc(HWND,UINT,WPARAM,LPARAM);
  static LRESULT __stdcall Fullscreen_WndProc(HWND,UINT,WPARAM,LPARAM);
  static LRESULT __stdcall GroupBox_WndProc(HWND,UINT,WPARAM,LPARAM);
  static int DTreeNotifyProc(DirectoryTree*,void*,int,int,int);

  void DestroyCurrentPage();
  void ManageWindowClasses(bool);

  void AssAddToExtensionsLV(char *,char *,int);
  void DrawBrightnessBitmap(HBITMAP);
  void CreateBrightnessBitmap();
  void PortsMakeTypeVisible(int);

  HIMAGELIST il;
  HBITMAP hBrightBmp;
  ScrollControlWin Scroller;
  WNDPROC Old_GroupBox_WndProc;
  static DirectoryTree DTree;
  //WIN32
#elif defined(UNIX)
  static int WinProc(TOptionBox*,Window,XEvent*);
	static int listview_notify_proc(hxc_listview*,int,int);
  static int dd_notify_proc(hxc_dropdown*,int,int);
  static int button_notify_proc(hxc_button*,int,int*);
	static int edit_notify_proc(hxc_edit *,int,int);
	static int scrollbar_notify_proc(hxc_scrollbar*,int,int);
  static int dir_lv_notify_proc(hxc_dir_lv*,int,int);

  void DrawBrightnessBitmap(XImage*),UpdateProfileDisplay(Str="",int=-1);
  void FillSoundDevicesDD();

  int page_p;

  hxc_listview page_lv;
  hxc_button control_parent;

  hxc_button cpu_boost_label,pause_inactive_but;
  hxc_dropdown cpu_boost_dd;

	hxc_button memory_label,monitor_label,tos_group;
  hxc_dropdown memory_dd,monitor_dd;
	hxc_button cart_group,cart_display,cart_change_but,cart_remove_but;
	hxc_button keyboard_language_label,keyboard_sc_but;
  hxc_dropdown keyboard_language_dd;
	hxc_button coldreset_but;
  hxc_textdisplay mustreset_td;

  hxc_dropdown tos_sort_dd;
  hxc_listview tos_lv;
	hxc_button tosadd_but,tosrefresh_but;

  hxc_button PortGroup[3],ConnectLabel[3];
  hxc_dropdown ConnectDD[3];
  hxc_button IOGroup[3],IOChooseBut[3],IOAllowIOBut[3][2],IOOpenBut[3];
  hxc_edit IODevEd[3];
  hxc_button LANGroup[3];
  hxc_button FileGroup[3],FileDisplay[3],FileChooseBut[3],FileEmptyBut[3];
  
  hxc_button high_priority_but,start_click_but;
#if defined(STEVEN_SEAGAL) && defined(SS_VARIOUS)
  hxc_button specific_hacks_but;
#endif
  hxc_button FFMaxSpeedLabel,SMSpeedLabel,RunSpeedLabel;
  hxc_scrollbar FFMaxSpeedSB,SMSpeedSB,RunSpeedSB;
  hxc_button ff_on_fdc_but;

  hxc_button fs_label;hxc_dropdown frameskip_dd;
  hxc_button bo_label;hxc_dropdown border_dd;
#if defined(STEVEN_SEAGAL) && defined(SS_STF)
  hxc_button st_type_label;hxc_dropdown st_type_dd;
#endif
#if defined(STEVEN_SEAGAL) && defined(SS_VID_BORDERS)
  hxc_button border_size_label; hxc_dropdown border_size_dd;
#endif
  hxc_button size_group,reschangeresize_but;
  hxc_button lowres_doublesize_but,medres_doublesize_but;
  hxc_button screenshots_group,screenshots_fol_display;
  hxc_button screenshots_fol_label,screenshots_fol_but;

  hxc_button sound_group,sound_mode_label,sound_freq_label,sound_format_label;
  hxc_dropdown sound_mode_dd,sound_freq_dd,sound_format_dd;
	hxc_button device_label,record_group,record_but;
	hxc_button wav_output_label,wav_choose_but,overwrite_ask_but;
  hxc_edit device_ed;

  hxc_listview profile_sect_lv;

  IconGroup brightness_ig;
  XImage *brightness_image;
  hxc_button brightness_picture,brightness_picture_label;
  hxc_button brightness_label;
  hxc_scrollbar brightness_sb;
  hxc_button contrast_label;
  hxc_scrollbar contrast_sb;

  hxc_button auto_sts_but;
  hxc_button auto_sts_filename_label;
  hxc_edit auto_sts_filename_edit;
  hxc_button no_shm_but;

  hxc_button osd_disable_but;

  hxc_listview drop_lv;

  static hxc_dir_lv dir_lv;
#endif//UNIX
  void FullscreenBrightnessBitmap();

  EasyStr WAVOutputDir;
public:
  TOptionBox();
  ~TOptionBox() { Hide(); }
  void Show(),Hide();
  bool ToggleVisible(){ IsVisible() ? Hide():Show();return IsVisible(); }
  void EnableBorderOptions(bool);
  bool ChangeBorderModeRequest(int);
  void ChangeOSDDisable(bool);
  bool LoadData(bool,GoodConfigStoreFile*,bool* = NULL),SaveData(bool,ConfigStoreFile*);

  void CreatePage(int);
  void CreateMachinePage(),CreateTOSPage(),CreateGeneralPage(),CreatePortsPage();
  void CreateSoundPage(),CreateDisplayPage(),CreateBrightnessPage();
  void CreateMacrosPage(),CreateProfilesPage(),CreateStartupPage(),CreateOSDPage();
#ifdef WIN32
	void CreateFullscreenPage(),CreateMIDIPage();
  void CreateUpdatePage(),CreateAssocPage();
  void IconsAddToScroller(),CreateIconsPage();
#else
  void CreatePathsPage();
#endif

  void UpdateSoundFreq();
	void ChangeSoundFormat(BYTE,BYTE);
  void UpdateRecordBut();
  void SetRecord(bool);
  void SoundModeChange(int,bool,bool);
  void UpdateMacroRecordAndPlay(Str="",int=0);
  Str CreateMacroFile(bool);
  void LoadProfile(char*);

  int Page;
  bool RecordWarnOverwrite;
  static bool USDateFormat;
  Str TOSBrowseDir,LastCartFile;

#if defined(WIN32)
  bool HasHandledMessage(MSG *);

  void LoadIcons();
  void ChangeScreenShotFormat(int,Str);
  void ChangeScreenShotFormatOpts(int);
  void ChooseScreenShotFolder(HWND);

  static BOOL CALLBACK EnumDateFormatsProc(char *);

  void UpdateHzDisplay();
  void UpdateWindowSizeAndBorder();
  void SetBorder(int);
  void UpdateForDSError();
  void FillScreenShotFormatOptsCombo();
  void UpdateParallel();

  HWND BorderOption;
#if defined(STEVEN_SEAGAL) && defined(SS_STF)
  HWND STTypeOption;
#endif
#if defined(STEVEN_SEAGAL) && defined(SS_VID_BORDERS)
  HWND BorderSizeOption;
#endif
#elif defined(UNIX)
  void UpdatePortDisplay(int);

  hxc_button internal_speaker_but; // changed in Sound_Start
#if defined(STEVEN_SEAGAL) && defined(SS_VARIOUS)
  hxc_button keyboard_click_but; 
#endif
#endif

  void MachineUpdateIfVisible();
  void TOSRefreshBox(EasyStr="");
  bool NeedReset() { return NewMemConf0>=0 || NewMonitorSel>=0 || NewROMFile.NotEmpty(); }
  int GetCurrentMonitorSel();
	int TOSLangToFlagIdx(int);

  int NewMemConf0,NewMemConf1,NewMonitorSel;
  Str NewROMFile;

  WIN_ONLY( EasyStringList eslTOS; )
  ESLSortEnum eslTOS_Sort;
  bool eslTOS_Descend;

  Str MacroDir,MacroSel;
  Str ProfileDir,ProfileSel;

  Str LastIconPath,LastIconSchemePath;
};
bool TOptionBox::USDateFormat=0;
WIN_ONLY( DirectoryTree TOptionBox::DTree; )
UNIX_ONLY( hxc_dir_lv TOptionBox::dir_lv; )

EasyStr WAVOutputFile;
EasyStringList DSDriverModuleList;

#define EXTMON_RESOLUTIONS 7

const int extmon_res[EXTMON_RESOLUTIONS][3]={
{800,600,1},
{1024,768,1},
{1280,960,1},
{640,400,4},
{800,600,4},
{1024,768,4},
{1280,960,4}};

