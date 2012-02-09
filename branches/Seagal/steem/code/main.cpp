/*---------------------------------------------------------------------------
FILE: main.cpp
MODULE: Steem
DESCRIPTION: This file contains the various main routines and general
startup/shutdown code for all the versions of Steem. It also includes the
other files that make up the Steem module.
---------------------------------------------------------------------------*/

#include "../pch.h"
#pragma hdrstop

/*
------------------------------------------------------------------
       S T E E M   E N G I N E

       The STE Emulating Engine

       Last updated [today]
                                                  |||
                                                  |||
                                                 / | \
                                               _/  |  \_
------------------------------------------------------------------
*/
#define IN_MAIN
#include "conditions.h"

const char *stem_version_date_text=__DATE__ " - " __TIME__;

#ifndef ONEGAME
const char *stem_window_title="Steem Engine";
#else
const char *stem_window_title=ONEGAME;
#endif

bool Initialise();
void PerformCleanShutdown();

#include "include.h"

EasyStr CrashFile;

THardDiskManager HardDiskMan;
TDiskManager DiskMan;
TJoystickConfig JoyConfig;
TGeneralInfo InfoBox;
TOptionBox OptionBox;
TShortcutBox ShortcutBox;
TPatchesBox PatchesBox;

#include "controls.cpp"

#include "stemdialogs.cpp"

#include "floppy_drive.cpp"
#include "hdimg.cpp"

#ifndef NO_GETCONTENTS
#include "di_get_contents.cpp"
#endif

#include "diskman.cpp"
#include "harddiskman.cpp"
#include "stjoy.cpp"
#include "infobox.cpp"
#include "options.cpp"
#include "shortcutbox.cpp"
#include "patchesbox.cpp"
#include "dataloadsave.cpp"

#include "display.cpp"
#include "init_sound.cpp"

#include "acc.cpp"
#ifdef _DEBUG_BUILD
  #include "d2.cpp"
#endif
#include "osd.cpp"
#ifdef _DEBUG_BUILD
  #include "iolist.cpp"
  #include "mr_static.cpp"
  #include "mem_browser.cpp"
  #include "dwin_edit.cpp"
  #include "boiler.cpp"
  #include "historylist.cpp"
  #include "trace.cpp"
#endif
#include "loadsave.cpp"
#include "steemintro.cpp"
#include "associate.cpp"
#include "notifyinit.cpp"
#include "gui.cpp"
#include "dir_id.cpp"
#include "palette.cpp"
#include "archive.cpp"
#include "macros.cpp"
#include <wordwrapper.cpp>
#include "screen_saver.cpp"

#ifdef ONEGAME
#define _USE_MEMORY_TO_MEMORY_DECOMPRESSION
#include <urarlib/urarlib.h>
#include <urarlib/urarlib.c>
#include "onegame.cpp"
#endif
//---------------------------------------------------------------------------
#define LOGSECTION LOGSECTION_INIT

int MainRetVal=-50;

#ifdef WIN32
int WINAPI WinMain(HINSTANCE Instance,HINSTANCE,char *,int)
{
  Inst=Instance;
  RunDir=GetEXEDir();

  InitializeCriticalSection(&agenda_cs);

#elif defined(UNIX)

int main(int argc,char *argv[])
{
  _argv=argv;
  _argc=argc;

  for (int n=0;n<_argc-1;n++){
    EasyStr butt;
    int Type=GetComLineArgType(_argv[1+n],butt);
    if (Type==ARG_HELP){
      PrintHelpToStdout();
      return 0;
    }
  }

  if (_argv[0][0]=='/'){ //Full path
    RunDir=_argv[0];
    RemoveFileNameFromPath(RunDir,REMOVE_SLASH);
  }else{
    RunDir.SetLength(MAX_PATH+1);
    getcwd(RunDir,MAX_PATH);
    NO_SLASH(RunDir);
/*
    if (RunDir.RightChar()!='/') RunDir+="/";
    EasyStr Com=_argv[0];
    RemoveFileNameFromPath(Com,REMOVE_SLASH);
    RunDir+=Com;

*/
  }
  printf(EasyStr("\n-- Steem Engine v")+stem_version_text+" --\n\n");
  printf(EasyStr("Steem will save all its settings to ")+RunDir.Text+"\n");

  XD=XOpenDisplay(NULL);
  if (XD==NULL){
    printf("\nFailed to open X display\n");
    return EXIT_FAILURE;
  }

  XSetErrorHandler(HandleXError);

  hxc::modal_notifyproc=steem_hxc_modal_notify;

  InitColoursAndIcons();

#endif

  NO_SLASH(RunDir);

  try{
    if (Initialise()==0){
      CleanUpSteem();
      if (CrashFile.NotEmpty()) DeleteFile(CrashFile);
      return MainRetVal;
    }

#ifdef WIN32
    log("STARTUP: Starting Message Loop");
    MSG MainMess;
    while (GetMessage(&MainMess,NULL,0,0)){
      if (HandleMessage(&MainMess)){
        TranslateMessage(&MainMess);
        TScreenSaver::checkMessage(&MainMess);
        DispatchMessage(&MainMess);
      }
    }
    if (StemWin) ShowWindow(StemWin,SW_HIDE);
#ifdef _DEBUG_BUILD
    if (DWin) ShowWindow(DWin,SW_HIDE);
    if (trace_window_handle) ShowWindow(trace_window_handle,SW_HIDE);
#endif

#elif defined(UNIX)
    XEvent Ev;
    LOOP{
      if (hxc::wait_for_event(XD,&Ev)){
        if (ProcessEvent(&Ev)==PEEKED_RUN){
          Window FocusWin;
          int RevertFlag;
          XGetInputFocus(XD,&FocusWin,&RevertFlag);
          if (FocusWin==StemWin && fast_forward!=RUNSTATE_STOPPED+1 && slow_motion!=RUNSTATE_STOPPED+1){
            SetStemMouseMode(STEM_MOUSEMODE_WINDOW);
          }
          RunBut.set_check(true);
          run();
          RunBut.set_check(0);
        }
      }
      if (Quitting && XPending(XD)==0) break;
    }
    XUnmapWindow(XD,StemWin);
    XFlush(XD);
#endif

    if (SnapShotGetLastBackupPath().NotEmpty()){
      log("SHUTDOWN: Deleting last memory snapshot backup");
      DeleteFile(SnapShotGetLastBackupPath());
    }
    PerformCleanShutdown();
  	return EXIT_SUCCESS;
#if defined(WIN32) && !defined(_DEBUG_BUILD) && !defined(_MINGW_BUILD) && !defined(ONEGAME)
  }__except(EXCEPTION_EXECUTE_HANDLER){
    if (AutoLoadSnapShot){
      MoveFileEx(WriteDir+SLASH+AutoSnapShotName+".sts",
                  WriteDir+SLASH+AutoSnapShotName+"_crash_backup.sts",
                  MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
    }
  }
#else
  }catch(...){}
#endif

  WIN_ONLY( SetErrorMode(0); )
  log_write("Fatal Error - attempting to shut down cleanly");
  PerformCleanShutdown();
  return EXIT_FAILURE;
}
//---------------------------------------------------------------------------
void FindWriteDir()
{
  char TestOutFileName[MAX_PATH+1];
  bool RunDirCanWrite=0;
  if (GetTempFileName(RunDir,"TST",0,TestOutFileName)){
    FILE *f=fopen(TestOutFileName,"wb");
    if (f){
      RunDirCanWrite=true;
      fclose(f);
    }
    DeleteFile(TestOutFileName);
  }
  if (RunDirCanWrite){
    WriteDir=RunDir;
  }else{
#ifdef WIN32
    ITEMIDLIST *idl;
    IMalloc *Mal;

    SHGetMalloc(&Mal);
    if (SHGetSpecialFolderLocation(NULL,CSIDL_APPDATA,&idl)==NOERROR){
      SHGetPathFromIDList(idl,TestOutFileName);
      Mal->Free(idl);
    }else{
      GetTempPath(MAX_PATH,TestOutFileName);
    }
    NO_SLASH(TestOutFileName);
#ifndef ONEGAME
    WriteDir=Str(TestOutFileName)+SLASH+"Steem";
#else
    WriteDir=Str(TestOutFileName)+SLASH+ONEGAME;
    CreateDirectory(WriteDir,NULL);
    WriteDir+=Str(SLASH)+ONEGAME_NAME;
#endif
    CreateDirectory(WriteDir,NULL);
#else
    // Must find a location that is r/w
    WriteDir=RunDir;
#endif
  }
}
//---------------------------------------------------------------------------
bool Initialise()
{
  FindWriteDir();

  runstate=RUNSTATE_STOPPED;
  DEBUG_ONLY( mode=STEM_MODE_INSPECT; )

  TranslateFileName=RunDir+SLASH "Translate.txt";
#ifndef ONEGAME
  INIFile=WriteDir+SLASH "steem.ini";
  bool NoNewInst=0,AlwaysNewInst=0,QuitNow=0,ShowNotify=true;
  for (int n=0;n<_argc-1;n++){
    EasyStr Path;
    int Type=GetComLineArgType(_argv[1+n],Path);
    if (Type==ARG_SETINIFILE){
      INIFile=Path;
    }else if (Type==ARG_SETTRANSFILE){
      if (Exists(Path)) TranslateFileName=Path;
    }else if (Type==ARG_NONEWINSTANCE || Type==ARG_TAKESHOT){
      NoNewInst=true;
    }else if (Type==ARG_ALWAYSNEWINSTANCE){
      AlwaysNewInst=true;
    }else if (Type==ARG_TOSIMAGEFILE){
      ROMFile=Path;
      BootTOSImage=true;
    }else if (Type==ARG_QUITQUICKLY){
      QuitNow=true;
    }else if (Type==ARG_SETFONT){
UNIX_ONLY( hxc::font_sl.Insert(0,0,Path,NULL); )
    }else if (Type==ARG_NONOTIFYINIT){
      ShowNotify=0;
    }
  }
#else
  INIFile=RunDir+SLASH ONEGAME_NAME ".ini";
#endif

  GoodConfigStoreFile CSF(INIFile);

#ifndef ONEGAME
  if ((CSF.GetInt("Options","OpenFilesInNew",true)==0 || NoNewInst) && AlwaysNewInst==0){
    if (OpenComLineFilesInCurrent(NoNewInst)){
      MainRetVal=EXIT_SUCCESS;
      return 0;
    }
  }
  if (BootTOSImage) CSF.SetStr("Machine","ROM_File",ROMFile);

  NoINI=(CSF.GetStr("Machine","ROM_File","")=="");

  CSF.SetInt("Main","DebugBuild",0 DEBUG_ONLY( +1 ) );
  CSF.SetStr("Update","CurrentVersion",Str(stem_version_text));

  if (QuitNow){
    MainRetVal=EXIT_SUCCESS;
    return 0;
  }
#endif

  srand(timeGetTime());

#ifdef ENABLE_LOGFILE
  {
    int n=2;
    LogFileName=WriteDir+SLASH "steem.log";
    while ((logfile=fopen(LogFileName,"wb"))==NULL){
      int a=MessageBox(WINDOWTYPE(0),EasyStr("Can't open ")+LogFileName+"\nSelect Ignore to try a different name","Error",MB_ABORTRETRYIGNORE |
                        MB_ICONEXCLAMATION | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
      if (a==IDABORT){
        QuitSteem();
        break;
      }else if (a==IDIGNORE){
        LogFileName=WriteDir+SLASH "steem ("+(n++)+").log";
      }

    }
  }
  log("STARTUP: Logfile Open");
  load_logsections();
#endif

  InitTranslations();

#if USE_PASTI
  hPasti=LoadLibrary("pasti.dll");
  if (hPasti==NULL) hPasti=LoadLibrary("pasti\\pasti.dll");
  if (hPasti){
    bool Failed=true;
    LPPASTIINITPROC pastiInit=(LPPASTIINITPROC)GetProcAddress(hPasti,"pastiInit");
    if (pastiInit){
      struct pastiCALLBACKS pcb;
      ZeroMemory(&pcb,sizeof(pcb));
      pcb.LogMsg=pasti_log_proc;
      pcb.WarnMsg=pasti_warn_proc;
      pcb.MotorOn=pasti_motor_proc;

      struct pastiINITINFO pii;
      pii.dwSize=sizeof(pii);
      pii.applFlags=0;
      pii.applVersion=2;
      pii.cBacks=&pcb;
      Failed=(pastiInit(&pii)==FALSE);
      pasti=pii.funcs;
    }
    if (Failed){
      FreeLibrary(hPasti);
      hPasti=NULL;
      Alert(T("Pasti initialisation failed"),T("Pasti Error"),MB_ICONEXCLAMATION);
    }else{
      char p_exts[160];
      ZeroMemory(p_exts,160);
      pasti->GetFileExtensions(p_exts,160,TRUE);
      // Convert to NULL terminated list
      for (int i=0;i<160;i++){
        if (p_exts[i]==0) break;
        if (p_exts[i]==';') p_exts[i]=0;
      }

      // Strip *.
      char *p_src=p_exts,*p_dest=pasti_file_exts;
      ZeroMemory(pasti_file_exts,160);
      while (*p_src){
        if (*p_src=='*') p_src++;
        if (*p_src=='.') p_src++;
        strcpy(p_dest,p_src);
        p_dest+=strlen(p_dest)+1;
        p_src+=strlen(p_src)+1;
      }
    }
  }
#endif

  DiskMan.InitGetContents();

#ifndef ONEGAME
  {
    bool TwoSteems=CheckForSteemRunning();

#if defined(UNIX)
    TwoSteems=true;
#endif

    bool CrashedLastTime=CleanupTempFiles();
    if (TwoSteems==0){
      if (CrashedLastTime){
        StepByStepInit=Alert(T("It seems that Steem did not close properly. If it crashed we are terribly sorry, it shouldn't happen. If you can get Steem to crash 2 or more times when doing the same thing then please tell us, it would be a massive help.")+
                "\n\nE-mail: " STEEM_EMAIL "\n\n"+
              T("Please send as much detail as you can and we'll look into it as soon as possible. ")+
              "\n\n"+T("If you are having trouble starting Steem, you might want to step carefully through the initialisation process.  Would you like to do a step-by-step confirmation?"),
              T("Step-By-Step Initialisation"),MB_ICONQUESTION | MB_YESNO)==IDYES;
      }

      CrashFile.SetLength(MAX_PATH);
      GetTempFileName(WriteDir,"CRA",0,CrashFile);
      FILE *f=fopen(CrashFile,"wb");
      if (f){
        fclose(f);
        SetFileAttributes(CrashFile,FILE_ATTRIBUTE_HIDDEN);
      }
    }
  }

  DeleteFile(WriteDir+SLASH "steemcrash.ini");
  if (StepByStepInit && NoINI==0){
    if (Alert(T("It is possible that one of the settings you changed has made Steem crash, do you want to use the default settings? (Note: this won't lose your settings, anything you change this time will be saved to steemcrash.ini)"),
                 T("Use Default Settings?"),MB_ICONQUESTION | MB_YESNO)==IDYES){

      ROMFile=CSF.GetStr("Machine","ROM_File",RunDir+SLASH "tos.img");
      CSF.Close();
      INIFile=WriteDir+SLASH "steemcrash.ini";
      CSF.Open(INIFile);
      CSF.SetStr("Machine","ROM_File",ROMFile);

    }
  }
#endif

  LoadAllIcons(&CSF,true);

  if (ShowNotify) CreateNotifyInitWin();

#ifdef WIN32
  SetNotifyInitText(T("COM and Common Controls"));
  CoInitialize(NULL);
  InitCommonControls();
#endif

  SetNotifyInitText(T("ST Memory"));
  try{
    BYTE ConfigBank1=(BYTE)CSF.GetInt("Machine","Mem_Bank_1",MEMCONF_512);
    BYTE ConfigBank2=(BYTE)CSF.GetInt("Machine","Mem_Bank_2",MEMCONF_512);
    if (ConfigBank1!=MEMCONF_128 && ConfigBank1!=MEMCONF_512 &&
          ConfigBank1!=MEMCONF_2MB && ConfigBank1!=MEMCONF_7MB){
      // Invalid memory somehow
      ConfigBank1=MEMCONF_512;
      ConfigBank2=MEMCONF_512;
    }
    make_Mem(ConfigBank1,ConfigBank2);

    Rom=new BYTE[256*1024];
  }catch (...){
    MessageBox(WINDOWTYPE(0),T("Could not allocate enough memory!"),T("Out Of Memory"),
                MB_ICONEXCLAMATION | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
    MainRetVal=EXIT_FAILURE;
    return 0;
  }

#ifdef _DEBUG_BUILD
  {
    for (int n=0;n<MAX_MEMORY_BROWSERS;n++) m_b[n]=NULL;
    for (int n=0;n<MAX_MR_STATICS;n++) m_s[n]=NULL;
  }
#endif

#ifdef WIN32
  {
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
    WinNT=(osvi.dwPlatformId==VER_PLATFORM_WIN32_NT);
  }

  SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS | SEM_NOALIGNMENTFAULTEXCEPT);
#endif

#ifdef DISABLE_STEMDOS
  for (int n=0;n<26;n++){
    mount_flag[n]=false;
    mount_path[n]="";
  }
#else
  stemdos_init();
#endif

  log("STARTUP: draw_routines_init Called");
  if (draw_routines_init()==0){
    MainRetVal=EXIT_FAILURE;
    return 0;
  }

#ifndef ONEGAME
  {
    int IntroResult=2;
    if (NoINI){
      WIN_ONLY( ShowWindow(NotifyWin,SW_HIDE); )
      IntroResult=SteemIntro();
      if (IntroResult==1){
        MainRetVal=EXIT_FAILURE;
        return 0;
      }
#ifdef WIN32
      if (NotifyWin){
        SetWindowPos(NotifyWin,HWND_TOPMOST,0,0,0,0,SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
        UpdateWindow(NotifyWin);
      }
#endif
    }

    SetNotifyInitText(T("ST Operating System"));
    if (IntroResult==2){
      ROMFile=CSF.GetStr("Machine","ROM_File",RunDir+SLASH "tos.img");
      if (StepByStepInit){
        if (Alert(T("A different TOS version may help to stop Steem crashing, would you like to choose one?"),
                     T("Change TOS?"),MB_ICONQUESTION | MB_YESNO)==IDYES){
          ROMFile="";
        }
      }

      while (load_TOS(ROMFile)){
#ifdef WIN32
        if (NotifyWin) ShowWindow(NotifyWin,SW_HIDE);
#endif
        if (ROMFile.NotEmpty()){
          if (Exists(ROMFile)==0){
            MessageBox((WINDOWTYPE)0,EasyStr(T("Can't find file"))+" "+ROMFile,T("Error Loading OS"),
                        MB_ICONEXCLAMATION | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
          }else{
            MessageBox((WINDOWTYPE)0,ROMFile+" "+T("is not a valid TOS"),T("Error Loading OS"),
                        MB_ICONEXCLAMATION | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
          }
        }
#ifdef WIN32
        ROMFile=FileSelect(NULL,T("Select TOS Image"),RunDir,
                            FSTypes(3,NULL),1,true,"img");
#elif defined(UNIX)
        fileselect.set_corner_icon(&Ico16,ICO16_CHIP);
        ROMFile=fileselect.choose(XD,RunDir,NULL,
            T("Select TOS Image"),FSM_LOAD|FSM_LOADMUSTEXIST,romfile_parse_routine,".img");
#endif
        if (ROMFile.IsEmpty()){
          MainRetVal=EXIT_FAILURE;
          return 0;

        }
      }
#ifdef WIN32
      if (NotifyWin){
        SetWindowPos(NotifyWin,HWND_TOPMOST,0,0,0,0,SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
        UpdateWindow(NotifyWin);
      }
#endif
    }
  }
  CartFile=CSF.GetStr("Machine","Cart_File",CartFile);
  if (CartFile.NotEmpty()){
    if (load_cart(CartFile)) CartFile="";
  }
#else
  if (load_TOS("")){
    MainRetVal=EXIT_FAILURE;
    return 0;
  }
#endif

  SetNotifyInitText(T("Jump Tables"));

  WIN_ONLY( LoadUnzipDLL(); )

  log("STARTUP: cpu_routines_init Called");
  cpu_routines_init();

#ifdef _DEBUG_BUILD
  log("STARTUP: d2_routines_init Called");
  d2_routines_init();

  for (int n=0;n<HISTORY_SIZE;n++) pc_history[n]=0xffffff71;
  pc_history_idx=0;
#endif

  SetNotifyInitText(T("GUI"));
  log("STARTUP: MakeGUI Called");
  if (MakeGUI()==0){
    MainRetVal=EXIT_FAILURE;
    return 0;
  }

  if (Exists(RunDir+SLASH "new_steemupdate.exe")){
    DeleteFile(RunDir+SLASH "steemupdate.exe");
    if (Exists(RunDir+SLASH "steemupdate.exe")==0){
      MoveFile(RunDir+SLASH "new_steemupdate.exe",RunDir+SLASH "steemupdate.exe");
    }
  }

#ifndef ONEGAME
  ParseCommandLine(_argc-1,_argv+1);
#endif

#ifdef WIN32
  Disp.DrawToVidMem=(bool)CSF.GetInt("Options","DrawToVidMem",Disp.DrawToVidMem);
  Disp.BlitHideMouse=(bool)CSF.GetInt("Options","BlitHideMouse",Disp.BlitHideMouse);

  if (CSF.GetInt("Options","NoDirectDraw",0)) TryDD=0;
  if (TryDD && StepByStepInit){
    if (Alert(T("DirectX can cause problems on some set-ups, would you like Steem to stop using DirectDraw for this session? (Note: Not using DirectDraw slows down Steem).")+" "+
              T("To permanently stop using DirectDraw turn on Options->Startup->Never Use DirectDraw."),
                T("No DirectDraw?"),MB_ICONQUESTION | MB_YESNO)==IDYES){
      TryDD=0;
    }
  }
  if (TryDD){
    Disp.SetMethods(DISPMETHOD_DD,DISPMETHOD_GDI,0);
  }else{
    Disp.SetMethods(DISPMETHOD_GDI,0);
  }
#elif defined(UNIX)
  if (CSF.GetInt("Options","NoSHM",0)) TrySHM=0;
  if (TrySHM && StepByStepInit){
    if (Alert(T("It is possible that using the MIT Shared Memory Extension to speed up drawing might cause problems on some systems.  Do you want to disable SHM?"),
                T("No SHM?"),MB_ICONQUESTION | MB_YESNO)==IDYES){
      TrySHM=0;
    }
  }
  if (TrySHM){
    Disp.SetMethods(DISPMETHOD_XSHM,DISPMETHOD_X,0);
  }else{
    Disp.SetMethods(DISPMETHOD_X,0);
  }
#endif

  log("STARTUP: Initialising display");
  Disp.Init();
#ifdef ENABLE_LOGFILE
  {
    EasyStr Mess="STARTUP: Display Init finished. ";
    switch (Disp.Method){
      case DISPMETHOD_NONE:Mess+="It failed, nothing will be drawn.";break;
      case DISPMETHOD_DD:  Mess+="DirectDraw will be used to draw.";break;
      case DISPMETHOD_GDI: Mess+="The GDI will be used to draw.";break;
      case DISPMETHOD_X:   Mess+="Standard Xlib will be used to draw.";break;
      case DISPMETHOD_XSHM:Mess+="Xlib with shared memory extension will be used to draw.";break;
      case DISPMETHOD_BE:  Mess+="Standard Be will be used to draw.";break;
    }
    log(Mess);
  }
#endif

#ifdef WIN32
  if (CSF.GetInt("Options","NoDirectSound",0)) TrySound=0;
#else
  x_sound_lib=CSF.GetInt("Sound","Library",x_sound_lib);
  if (CSF.GetInt("Sound","NoPortAudio",0) && CSF.GetInt("Sound","IgnoreNoPortAudio",0)==0){
    x_sound_lib=0;
    CSF.SetInt("Sound","IgnoreNoPortAudio",1);
  }
  TrySound=x_sound_lib!=0;
#endif
  if (TrySound && StepByStepInit){
    if (Alert(T("Would you like to disable sound for this session?")+" "+
                UNIX_ONLY( T("To permanently disable sound turn on Options->Startup->Never Use PortAudio.") )
                WIN_ONLY(  T("To permanently disable sound turn on Options->Startup->Never Use DirectSound.") ),
                T("No Sound?"),MB_ICONQUESTION | MB_YESNO)==IDYES){
      TrySound=0;
    }
  }
  if (TrySound){
    log("STARTUP: InitSound Called");
    InitSound();
    log(EasyStr("STARTUP: InitSound finished. ")+LPSTR(UseSound ? "DirectSound will be used.":"DirectSound will not be used."));
  }

  SetNotifyInitText(T("Get Ready..."));

  init_DirID_to_text();

#ifdef WIN32
/*
  if (StepByStepInit && WinNT==0){
    if (CSF.GetInt("Main","AllowDTOn98",0)){
      if (Alert(T("The crash may have been caused by Steem's extended features.")+" "+
                T("There are bugs in some versions of Windows that cause Steem to crash when they are enabled.")+" "+
                T("Do you want to disable the extended features?"),
                   T("Disable Extended Features?"),MB_ICONQUESTION | MB_YESNO)==IDYES){
        CSF.SetInt("Main","AllowDTOn98",0);
      }
    }
  }
*/
#endif

  if (StepByStepInit){
    bool PortOpen=0;
    for (int p=0;p<3;p++){
      EasyStr PNam=EasyStr("Port_")+p+"_";
      if (CSF.GetInt("MIDI",PNam+"Type",PORTTYPE_NONE)){
        PortOpen=true;
        break;
      }
    }
    if (PortOpen){
      if (Alert(T("Accessing parallel/serial/MIDI ports can cause Steem to freeze up or crash on some systems.")+" "+
                T("Do you want to stop Steem accessing these ports?"),
                   T("Disable Ports?"),MB_ICONQUESTION | MB_YESNO)==IDYES){
        for (int p=0;p<3;p++){
          EasyStr PNam=EasyStr("Port_")+p+"_";
          CSF.SetInt("MIDI",PNam+"Type",PORTTYPE_NONE);
        }
      }
    }
  }


#ifdef WIN32
#ifndef ONEGAME
  AssociateSteem(".ST","st_disk_image",0,T("ST Disk Image"),DISK_ICON_NUM,0);
  AssociateSteem(".STT","st_disk_image",0,T("ST Disk Image"),DISK_ICON_NUM,0);
  AssociateSteem(".MSA","st_disk_image",0,T("ST Disk Image"),DISK_ICON_NUM,0);
  AssociateSteem(".DIM","st_disk_image",0,T("ST Disk Image"),DISK_ICON_NUM,0);
  AssociateSteem(".STZ","st_disk_image",0,T("ST Disk Image"),DISK_ICON_NUM,0);
#if USE_PASTI
  if (hPasti) AssociateSteem(".STX","st_pasti_disk_image",0,T("ST Disk Image"),DISK_ICON_NUM,0);
#endif
  AssociateSteem(".STS","steem_memory_snapshot",0,T("Steem Memory Snapshot"),SNAP_ICON_NUM,0);
  AssociateSteem(".STC","st_cartridge",0,T("ST ROM Cartridge"),CART_ICON_NUM,0);
  AssociateSteem(".PRG","st_program",0,T("ST Program"),PRG_ICON_NUM,true);
  AssociateSteem(".TOS","st_program",0,T("ST Program"),PRG_ICON_NUM,true);
  AssociateSteem(".APP","st_program",0,T("ST Program"),PRG_ICON_NUM,true);
  AssociateSteem(".GTP","st_program",0,T("ST Program"),PRG_ICON_NUM,true);
  AssociateSteem(".TTP","st_program",0,T("ST Program"),PRG_ICON_NUM,true);
#endif
#endif

#ifndef ONEGAME
  DestroyNotifyInitWin();
#endif

  CSF.SaveTo(INIFile); // Update the INI just in case a dialog does GetCSFInt

  log("STARTUP: LoadState Called");
  LoadState(&CSF);
  log("STARTUP: LoadState finished");

  log("STARTUP: power_on Called");
  power_on();

#ifdef WIN32
#ifndef ONEGAME
  if (CSF.GetInt("Update","AutoUpdateEnabled",true)){
    if (Exists(RunDir+"\\SteemUpdate.exe")){
      EasyStr Online=LPSTR(CSF.GetInt("Update","AlwaysOnline",0) ? " online":"");
      EasyStr NoPatch=LPSTR(CSF.GetInt("Update","PatchDownload",true)==0 ? " nopatchcheck":"");
      EasyStr AskPatch=LPSTR(CSF.GetInt("Update","AskPatchInstall",0) ? " askpatchinstall":"");
      WinExec(EasyStr("\"")+RunDir+"\\SteemUpdate.exe\" silent"+Online+NoPatch+AskPatch,SW_SHOW);
    }
  }
#endif
#endif

#ifdef _DEBUG_BUILD
  log("STARTUP: update_register_display called");
  update_register_display(true);
#endif

  log("STARTUP: draw_init_resdependent called");
  draw_init_resdependent(); //set up palette conversion & stuff
  log("STARTUP: draw called");
  draw(true);
  log("STARTUP: draw finished");

#ifdef WIN32
  SendMessage(ToolTip,TTM_ACTIVATE,ShowTips,0);
  SetTimer(StemWin,SHORTCUTS_TIMER_ID,50,NULL);
#elif defined(UNIX)
  if (ShowTips) hints.start();
  hxc::set_timer(StemWin,SHORTCUTS_TIMER_ID,50,timerproc,NULL);
#endif

#ifndef ONEGAME
  if (BootDisk[0].NotEmpty()){
    if (BootStateFile.NotEmpty()){
      if (LoadSnapShot(BootStateFile)) BootInMode|=BOOT_MODE_RUN;
      LastSnapShot=BootStateFile;
    }else{
#if USE_PASTI
      if (pasti_active){
        // Check you aren't booting with pasti when passing a non-pasti compatible disk
        for (int Drive=0;Drive<2;Drive++){
          if (BootDisk[Drive].NotEmpty() && NotSameStr_I(BootDisk[Drive],".")){
            if (ExtensionIsPastiDisk(strrchr(BootDisk[Drive],'.'))==0) BootPasti=BOOT_PASTI_OFF;
          }
        }
      }
      if (BootPasti!=BOOT_PASTI_DEFAULT){
        bool old_pasti=pasti_active;
        pasti_active=BootPasti==BOOT_PASTI_ON;
        if (DiskMan.IsVisible() && old_pasti!=pasti_active) DiskMan.RefreshDiskView();
      }
#endif
      for (int Drive=0;Drive<2;Drive++){
        if (BootDisk[Drive].NotEmpty() && NotSameStr_I(BootDisk[Drive],".")){
          EasyStr Name=GetFileNameFromPath(BootDisk[Drive]);
          *strrchr(Name,'.')=0;
          if (DiskMan.InsertDisk(Drive,Name,BootDisk[Drive],0,0)){
            if (Drive==0) BootInMode|=BOOT_MODE_RUN;
          }
        }
      }
    }
  }else if (AutoLoadSnapShot && BootTOSImage==0){
    if (Exists(WriteDir+SLASH+AutoSnapShotName+".sts")){
      bool Load=true;
      if (StepByStepInit){
        if (Alert(T("Would you like to restore the state of the ST?"),
                     T("Restore State?"),MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2)==IDNO){
          Load=0;
        }
      }
      if (Load){
        LoadSnapShot(WriteDir+SLASH+AutoSnapShotName+".sts",0,true,0); // Don't add to history, don't change disks
      }
    }
  }
  if (OptionBox.NeedReset()) reset_st(RESET_COLD | RESET_STOP | RESET_CHANGESETTINGS | RESET_NOBACKUP);
  CheckResetDisplay();

  if (Disp.CanGoToFullScreen()){
    bool Full=(BootInMode & BOOT_MODE_FLAGS_MASK)==BOOT_MODE_FULLSCREEN;

    if ((BootInMode & BOOT_MODE_FLAGS_MASK)==BOOT_MODE_DEFAULT){
      Full=CSF.GetInt("Options","StartFullscreen",0);
    }
    if (Full){
#ifdef WIN32
      PostMessage(StemWin,WM_SYSCOMMAND,SC_MAXIMIZE,0);
#elif defined(UNIX)
#endif
    }
  }
#else
  if (OGInit()==0) QuitSteem();
  PostMessage(StemWin,WM_SYSCOMMAND,SC_MAXIMIZE,0);
#endif

  res_change();

#ifdef WIN32
  int ShowState=SW_SHOW;
  if (CSF.GetInt("Main","Maximized",0)) ShowState=SW_MAXIMIZE;
#endif

  CSF.Close();

#ifdef WIN32
  DEBUG_ONLY( ShowWindow(DWin,SW_SHOW); )
  ShowWindow(StemWin,ShowState);
  if (bAOT) SetWindowPos(StemWin,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
#elif defined(UNIX)
  XMapWindow(XD,StemWin);
  XFlush(XD);
#endif

  if (BootInMode & BOOT_MODE_RUN){
    if (GetForegroundWindow()==StemWin) PostRunMessage();
  }

  return true;
}
//---------------------------------------------------------------------------
void make_Mem(BYTE conf0,BYTE conf1)
{
  mmu_memory_configuration=BYTE((conf0 << 2) | conf1);
  bank_length[0]=mmu_bank_length_from_config[conf0];
  bank_length[1]=mmu_bank_length_from_config[conf1];
  mmu_bank_length[0]=bank_length[0];
  mmu_bank_length[1]=bank_length[1];

  mem_len=bank_length[0]+bank_length[1];

  Mem=new BYTE[mem_len+MEM_EXTRA_BYTES];

  for (int y=0;y<MEM_EXTRA_BYTES;y++) Mem[y]=255;
  Mem_End=Mem+mem_len+MEM_EXTRA_BYTES;
  Mem_End_minus_1=Mem_End-1;
  Mem_End_minus_2=Mem_End-2;
  Mem_End_minus_4=Mem_End-4;

  for (int y=0;y<64+PAL_EXTRA_BYTES;y++) palette_exec_mem[y]=0;


  himem=mem_len;
  mmu_confused=false;
}
//---------------------------------------------------------------------------
void GetCurrentMemConf(BYTE MemConf[2])
{
  MemConf[0]=MEMCONF_512;
  MemConf[1]=MEMCONF_512;
  for (int i=0;i<2;i++){
    for (BYTE n=0;n<5;n++){
      if (bank_length[i]==mmu_bank_length_from_config[n]){
        MemConf[i]=n;
        break;
      }
    }
  }
}
#undef LOGSECTION

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void QuitSteem()
{
#ifdef WIN32
  Quitting=true;
  if (runstate!=RUNSTATE_STOPPED){
    runstate=RUNSTATE_STOPPING;
    PostMessage(StemWin,WM_CLOSE,0,0);
  }else if (FullScreen){
    PostMessage(StemWin,WM_COMMAND,MAKEWPARAM(106,BN_CLICKED),(LPARAM)GetDlgItem(StemWin,106));
    PostMessage(StemWin,WM_CLOSE,0,0);
  }else{
    draw_end();
    PostQuitMessage(0);
  }
#elif defined(UNIX)
  Quitting=true;
  if (runstate==RUNSTATE_RUNNING){
    runstate=RUNSTATE_STOPPING;
  }else if (FullScreen){
    Disp.ChangeToWindowedMode();
  }
	XAutoRepeatOn(XD);
#endif
}
//---------------------------------------------------------------------------
#define LOGSECTION LOGSECTION_SHUTDOWN
void CloseAllDialogs()
{
  log("SHUTDOWN: Hiding ShortcutBox");
  ShortcutBox.Hide();
  log("SHUTDOWN: Hiding HardDiskMan");
  HardDiskMan.Hide();
  log("SHUTDOWN: Hiding DiskMan");
  DiskMan.Hide();
  log("SHUTDOWN: Hiding JoyConfig");
  JoyConfig.Hide();
  log("SHUTDOWN: Hiding InfoBox");
  InfoBox.Hide();
  log("SHUTDOWN: Hiding OptionBox");
  OptionBox.Hide();
  log("SHUTDOWN: Hiding PatchesBox");
  PatchesBox.Hide();
}
//---------------------------------------------------------------------------
void PerformCleanShutdown()
{
#ifndef ONEGAME
  log("SHUTDOWN: Opening settings file");
  ConfigStoreFile CSF(INIFile);

  log("SHUTDOWN: Saving visible dialog info");
  HardDiskMan.Hide();
  for (int n=0;n<nStemDialogs;n++) DialogList[n]->SaveVisible(&CSF);

  log("SHUTDOWN: CloseAllDialogs()");
  CloseAllDialogs();

  log("SHUTDOWN: SaveState()");
  SaveState(&CSF);

  log("SHUTDOWN: Closing settings file");
  CSF.Close();
#endif

  log("SHUTDOWN: CleanUpSteem()");
  CleanUpSteem();
  if (CrashFile.NotEmpty()) DeleteFile(CrashFile);
  CrashFile="";
}
//---------------------------------------------------------------------------
void CleanUpSteem()
{
#ifdef WIN32
  KillTimer(StemWin,SHORTCUTS_TIMER_ID);
#elif defined(UNIX)
  hxc::kill_timer(StemWin,HXC_TIMER_ALL_IDS);
#endif

  macro_end(MACRO_ENDRECORD | MACRO_ENDPLAY);

  log("SHUTDOWN: Calling  CloseAllDialogs()");
  CloseAllDialogs();

  log("SHUTDOWN: Closing MIDIPort");
  MIDIPort.Close();
  log("SHUTDOWN: Closing Parallel Port");
  ParallelPort.Close();
  log("SHUTDOWN: Closing Serial Port");
  SerialPort.Close();

#ifndef DISABLE_STEMDOS
  log("SHUTDOWN: Closing all Stemdos files");
  stemdos_close_all_files();
#endif

  log("SHUTDOWN: Releasing Disp (DX shutdown)");
  Disp.Release();
  if (osd_plasma_pal){
    delete[] osd_plasma_pal; osd_plasma_pal=NULL;
    delete[] osd_plasma;     osd_plasma=NULL;
  }

  log("SHUTDOWN: Releasing Sound");
  SoundRelease();
  log("SHUTDOWN: Releasing Joysticks");
  FreeJoysticks();

  log("SHUTDOWN: Calling CleanupGUI()");
  CleanupGUI();

  DestroyKeyTable();

  WIN_ONLY( if (hUnzip) FreeLibrary(hUnzip); enable_zip=false; hUnzip=NULL; )

#ifdef UNIX
  if (XD){
    XCloseDisplay(XD);
    XD=NULL;
  }
#endif

  log("SHUTDOWN: Freeing cart memory");
  if (cart) {delete[] cart;cart=NULL;}
  log("SHUTDOWN: Freeing RAM memory");
  if (Mem) {delete[] Mem;Mem=NULL;}
  log("SHUTDOWN: Freeing ROM memory");
  if (Rom) {delete[] Rom;Rom=NULL;}

  log("SHUTDOWN: DeleteCriticalSection()");
  WIN_ONLY( DeleteCriticalSection(&agenda_cs); )

  log("SHUTDOWN: Deleting TranslateBuf");
  if (TranslateBuf) delete[] TranslateBuf;
  if (TranslateUpperBuf) delete[] TranslateUpperBuf;
  TranslateBuf=NULL;TranslateUpperBuf=NULL;

  log("SHUTDOWN: Closing logfile - bye!!!");

#ifdef ENABLE_LOGFILE
  if (logfile) fclose(logfile);
  logfile=NULL;
#endif

  ONEGAME_ONLY( OGCleanUp(); )

  WIN_ONLY( if (SteemRunningMutex) CloseHandle(SteemRunningMutex); )

#if USE_PASTI
  if (hPasti) FreeLibrary(hPasti);
  hPasti=NULL;
#endif
}
//---------------------------------------------------------------------------

#undef LOGSECTION

