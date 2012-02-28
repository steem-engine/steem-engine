#include "pch.h"
#pragma hdrstop
//---------------------------------------------------------------------------
/*
Conditional defines:

_BCB_BUILD - Compile in Borland C++ Builder
_BCC_BUILD - Compile with Borland C++ command line
_MINGW_BUILD - Compile with MinGW
_VC_BUILD - Compile with Microsoft Visual C++

WIN32 - Windows
UNIX - Unix
LINUX - Add some Linux extras
CYGWIN - Cygwin

_DEBUG_BUILD/NO_DEBUG_BUILD - Control debug build on/off (Win only)
NO_486_ASM - Disable all use of assembler routines (must add draw_c.cpp)
ENABLE_LOGFILE - Turn logging on (even when debug build is off)
NO_RAR_SUPPORT - Turn off that loverly RAR code
BIG_ENDIAN_PROCESSOR - Hmm Mac, Sparc, what great fun you think. Well think again
                        it don't work yet, maybe some day...
NO_SHM - Don't use the MIT shared memory extension (Unix only)

Required include directories:

[Steem Base]/code/ - Damn BCB wouldn't let me have forward slashes so I can't leave this one off
[Outside Steem]/include/ - These files are intended to be separate from Steem, so you
                           can use them in other projects.
[Outside Steem]/3rdparty/ - All code that isn't ours, for instance the UnRAR stuff goes
                            in 3rdparty/unrarlib/

Required libraries:

Windows BCC - import32.lib  cw32mt.lib  c0w32.obj
Windows MinGW - winmm  uuid  comctl32  ole32
Unix - X11  Xext  pthread
*/


//---------------------------------------------------------------------------
// SS: Except in the old BCBuilder, the macros USE... will do NOTHING at all!
// the includes are in main (last lines of this file)
#ifndef _BCB_BUILD	
#define USEUNIT(ModName)
#define USEOBJ(FileName)
#define USERC(FileName)
#define USEASM(FileName)
#define USEDEF(FileName)
#define USERES(FileName)
#define USETLB(FileName)
#define USELIB(FileName)
#define USEFILE(FileName)
#define USEIDL(FileName)
#else
#pragma message("Build for Borland C++ Builder...")
#include <condefs.h> 
#endif

USELIB("asm\asm_draw.obj");
USELIB("asm\asm_osd_draw.obj");
USELIB("..\..\include\asm\asm_portio.obj");
USELIB("..\..\3rdparty\urarlib\urarlib.obj");


//USEUNIT("..\..\3rdparty\mmgr\mmgr.cpp");



USEFILE("pch.h");

USEFILE("code\gui.cpp");
USEFILE("code\d2.cpp");
USEFILE("code\acc.cpp");
USEFILE("code\mr_static.cpp");
USEFILE("code\mem_browser.cpp");
USEFILE("code\dwin_edit.cpp");
USEFILE("code\boiler.cpp");
USEFILE("code\loadsave.cpp");
USEFILE("code\floppy_drive.cpp");
USEFILE("code\diskman.cpp");
USEFILE("code\harddiskman.cpp");
USEFILE("code\hdimg.cpp");
USEFILE("code\stjoy.cpp");
USEFILE("code\stconfig.cpp");
USEFILE("code\controls.cpp");
USEFILE("code\infobox.cpp");
USEFILE("code\options.cpp");
USEFILE("code\historylist.cpp");
USEFILE("code\blitter.cpp");
USEFILE("code\stemdialogs.cpp");
USEFILE("code\ikbd.cpp");
USEFILE("code\associate.cpp");
USEFILE("code\iolist.cpp");
USEFILE("code\notifyinit.cpp");
USEFILE("code\unzip.h");
USEFILE("code\steemintro.cpp");
USEFILE("code\diskman_diags.cpp");
USEFILE("code\diskman_drag.cpp");
USEFILE("code\midibox.cpp");
USEFILE("code\shortcutbox.cpp");
USEFILE("code\include.h");
USEFILE("code\stemwin.cpp");
USEFILE("code\dataloadsave.cpp");
USEFILE("code\main.cpp");
USEFILE("code\init_sound.cpp");
USEFILE("code\dir_id.cpp");
USEFILE("code\key_table.h");
USEFILE("code\options_create.cpp");
USEFILE("code\patchesbox.cpp");
USEFILE("code\palette.cpp");
USEFILE("code\resnum.h");
USEFILE("code\archive.cpp");
USEFILE("code\macros.cpp");
USEFILE("code\onegame.cpp");
USEFILE("code\conditions.h");
USEFILE("code\display.cpp");
USEFILE("code\osd.cpp");
USEFILE("asm\asm_draw.asm");
USEFILE("asm\asm_draw_scrolled.asm");
USEFILE("asm\asm_osd_draw.asm");
USEFILE("asm\asm_calc_col.asm");
USEFILE("asm\asm_lowres.asm");
USEFILE("asm\asm_medhires.asm");
USEFILE("code\draw_c\draw_c_lowres_scanline.cpp");
USEFILE("code\draw_c\draw_c_hires_scanline.cpp");
USEFILE("code\draw_c\draw_c.cpp");
USEFILE("code\draw_c\draw_c_medres_scanline.cpp");
USEFILE("code\draw_c\draw_c_osd_blueize_line.cpp");
USEFILE("code\draw_c\draw_c_osd_draw_char.cpp");
USEFILE("code\x\x_stemwin.cpp");
USEFILE("code\x\x_diskman.cpp");
USEFILE("code\x\x_display.cpp");
USEFILE("code\x\x_gui.cpp");
USEFILE("code\x\x_harddiskman.cpp");
USEFILE("code\x\x_infobox.cpp");
USEFILE("code\x\x_joy.cpp");
USEFILE("code\x\x_midi.cpp");
USEFILE("code\x\x_notifyinit.cpp");
USEFILE("code\x\x_options.cpp");
USEFILE("code\x\x_options_create.cpp");
USEFILE("code\x\x_patchesbox.cpp");
USEFILE("code\x\x_shortcutbox.cpp");
USEFILE("code\x\x_sound.cpp");
USEFILE("code\x\x_stemdialogs.cpp");
USEFILE("code\x\x_controls.cpp");
USEFILE("..\..\include\x\hxc.cpp");
USEFILE("..\..\include\x\hxc_button.cpp");
USEFILE("..\..\include\x\hxc_textdisplay.cpp");
USEFILE("..\..\include\x\hxc_dropdown.cpp");
USEFILE("..\..\include\x\hxc_edit.cpp");
USEFILE("..\..\include\x\hxc_listview.cpp");
USEFILE("..\..\include\x\hxc_scrollarea.cpp");
USEFILE("..\..\include\x\hxc_scrollbar.cpp");
USEFILE("..\..\include\x\hxc_fileselect.cpp");
USEFILE("..\..\include\x\hxc_alert.cpp");
USEFILE("..\..\include\x\hxc_popup.cpp");
USEFILE("..\..\include\x\hxc_popuphints.cpp");
USEFILE("..\..\include\x\hxc_dir_lv.cpp");
USEFILE("..\..\include\x\hxc_prompt.cpp");
USEFILE("..\..\include\x\icongroup.cpp");
USEFILE("..\..\include\x\x_mymisc.cpp");
USEFILE("..\..\include\x\x_portio.cpp");

USEUNIT("helper.cpp");
USEFILE("..\..\include\mymisc.cpp");
USEFILE("..\..\include\easystr.cpp");
USEFILE("..\..\include\easystringlist.cpp");
USEFILE("..\..\include\circularbuffer.cpp");
USEFILE("..\..\include\dynamicarray.h");
USEFILE("..\..\include\portio.cpp");
USEFILE("..\..\include\di_get_contents.cpp");
USEFILE("..\..\include\directory_tree.cpp");
USEFILE("..\..\include\configstorefile.cpp");
USEFILE("..\..\include\dirsearch.cpp");

USEUNIT("emu.cpp");
USEFILE("code\cpu.cpp");
USEFILE("code\debug_emu.cpp");
USEFILE("code\gui_emu.h");
USEFILE("code\emulator.cpp");
USEFILE("code\reset.cpp");
USEFILE("code\draw.cpp");
USEFILE("code\trace.cpp");
USEFILE("code\stemdos.cpp");
USEFILE("code\fdc.cpp");
USEFILE("code\psg.cpp");
USEFILE("code\steemh.h");
USEFILE("code\mfp.cpp");
USEFILE("code\run.cpp");
USEFILE("code\ior.cpp");
USEFILE("code\iow.cpp");
USEFILE("code\iorw.h");
USEFILE("code\stports.cpp");
USEFILE("code\rs232.cpp");
USEFILE("code\loadsave_emu.cpp");
USEFILE("code\midi.cpp");

USERC("rc\resource.rc");
//---------------------------------------------------------------------------
#define IN_MAIN

#include "conditions.h"



// SS: VC++ 6.0 (and below) scoping bugfix
#ifdef _VC_BUILD
#if defined(_MSC_VER) && _MSC_VER <= 1200
#define for if(0); else for
#endif
#endif

#include "main.cpp"

#ifdef DUMMY_PROCEDURE_FOR_BCB_IDE
int main()
{
}
#endif


