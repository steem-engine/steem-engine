#ifdef UNIX
bool enable_zip=true;
#else
bool enable_zip=false;
#endif

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif
//---------------------------------------------------------------------------
#ifdef WIN32
HINSTANCE Inst;
#endif
#define HInstance Inst
//---------------------------------------------------------------------------
#include <easystr.h>
#include <mymisc.h>
#include <easycompress.h>
#include <easystringlist.h>
#include <portio.h>
#include <dynamicarray.h>
#include <dirsearch.h>

#if USE_PASTI
#include <pasti/pasti.h>
#endif

#include <configstorefile.h>
#define GoodConfigStoreFile ConfigStoreFile

#ifndef NO_PORTAUDIO
#ifdef WIN32
#if defined(USE_PORTAUDIO_ON_WIN32) && defined(IN_MAIN)
#include <portaudio/portaudio_dll.cpp>
#endif
#else
#include <portaudio.h>
#endif
#endif

//#define NO_RTAUDIO

#ifndef NO_RTAUDIO
#ifdef UNIX
#include <RtAudio.h>
using namespace std;
#endif
#endif

typedef EasyStr Str;

#include "translate.h"

#ifdef WIN32

#include <choosefolder.h>
#include <scrollingcontrolswin.h>
#include <directory_tree.h>
#include <input_prompt.h>

#elif defined(UNIX)

#include <x/hxc.h>
#include <x/hxc_alert.h>
#include <x/hxc_prompt.h>
#include <x/hxc_fileselect.h>
#include <x/hxc_popup.h>
#include <x/hxc_popuphints.h>
#include <x/hxc_dir_lv.h>
#include "x/x_controls.h"

//#include <X11/Xcms.h>

#ifdef LINUX
#include </usr/include/linux/kd.h>  // linux internal speaker header
#endif

#endif
//---------------------------------------------------------------------------
#ifdef WIN32
#include "resnum.h"
#endif

#include "steemh.h"
#include "notifyinit.h"
#include "stports.h"
#include "dir_id.h"
#include "fdc.h"
#include "floppy_drive.h"
#include "hdimg.h"
#include "init_sound.h"
#include "psg.h"
#include "loadsave.h"

#ifndef NO_GETCONTENTS
#include "di_get_contents.h"
#endif

#include "stemdialogs.h"
#include "harddiskman.h"
#include "diskman.h"
#include "stjoy.h"
#include "infobox.h"
#include "options.h"
#include "shortcutbox.h"
#include "patchesbox.h"

#include "display.h"
#include "draw.h"
#include "osd.h"
#include "palette.h"
#include "acc.h"
#include "ikbd.h"
#include "key_table.h"
#include "mfp.h"
#include "blitter.h"
#include "run.h"
#include "reset.h"
#include "stemdos.h"
#include "iorw.h"
#include "cpu.h"
#include "midi.h"
#include "rs232.h"
#ifdef _DEBUG_BUILD
  #include "historylist.h"
  #include "d2.h"
  #include "dwin_edit.h"
  #include "iolist.h"
  #include "mem_browser.h"
  #include "mr_static.h"
  #include "debug_emu.h"
  #include "boiler.h"
  #include "trace.h"
#endif
#include "archive.h"
#include "gui.h"
#include "macros.h"
#include "dataloadsave.h"
#include "emulator.h"
#include <wordwrapper.h>
#include "screen_saver.h"

#ifdef WIN32
#include "dderr_meaning.h"
#endif

#ifdef ONEGAME
#include "onegame.h"
#endif

