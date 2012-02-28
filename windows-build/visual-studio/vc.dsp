# Microsoft Developer Studio Project File - Name="Steem" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Steem - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vc.mak" CFG="Steem - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Steem - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Steem - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Steem - Win32 Boiler" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /w /W0 /Gm /GX /ZI /Od /D "_VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /I /I /I /I /GZ /c
# ADD CPP /nologo /w /W0 /Gm /ZI /Od /I "..\..\include\\" /I "..\..\steem\code\\" /I "..\..\3rdparty\\" /D "_DEBUG" /D "_NO_DEBUG_BUILD" /D "_VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STEVEN_SEAGAL" /FR /I /I /I /I /GZ /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dinput.lib dxguid.lib winmm.lib ComCtl32.Lib /nologo /subsystem:windows /debug /machine:IX86 /out:"Debug\Steem.exe" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 dinput.lib dxguid.lib winmm.lib ComCtl32.Lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:IX86 /out:"Debug\Steem.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /w /W0 /GX /Zi /Ox /Ot /Og /Oi /Ob2 /Gy /D "_VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /I /I /I /I /GA /GF
# ADD CPP /nologo /w /W0 /Zi /Ob2 /Gy /I "..\..\include\\" /I "..\..\steem\code\\" /I "..\..\3rdparty\\" /D "NDEBUG" /D "_NO_DEBUG_BUILD" /D "_VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STEVEN_SEAGAL" /FR /GA /GF /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dinput.lib dxguid.lib winmm.lib ComCtl32.Lib /nologo /subsystem:windows /machine:IX86 /out:"Release\Steem.exe" /pdbtype:sept /opt:ref /opt:icf
# ADD LINK32 dinput.lib dxguid.lib winmm.lib ComCtl32.Lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:IX86 /out:"Release\Steem SSE 3.3.exe" /pdbtype:sept /opt:ref /opt:icf
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy "Release\Steem SSE 3.3.exe" "G:\emu\ST\bin\steem"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Steem___Win32_Boiler"
# PROP BASE Intermediate_Dir "Steem___Win32_Boiler"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Steem___Win32_Boiler"
# PROP Intermediate_Dir "Steem___Win32_Boiler"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /w /W0 /Zi /Ob2 /Gy /I "..\..\include\\" /I "..\..\steem\code\\" /I "..\..\3rdparty\\" /D "NDEBUG" /D "_NO_DEBUG_BUILD" /D "_VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STEVEN_SEAGAL" /FR /GA /GF /c
# ADD CPP /nologo /w /W0 /Zi /Od /Ob2 /Gy /I "..\..\include\\" /I "..\..\steem\code\\" /I "..\..\3rdparty\\" /D "NDEBUG" /D "_NO_DEBUG_BUILD" /D "_VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STEVEN_SEAGAL" /FR /GA /GF /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dinput.lib dxguid.lib winmm.lib ComCtl32.Lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:IX86 /out:"Release\Steem3.3B.exe" /pdbtype:sept /opt:ref /opt:icf
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 dinput.lib dxguid.lib winmm.lib ComCtl32.Lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:IX86 /out:"Release\Steem SSE 3.3B Boiler.exe" /pdbtype:sept /opt:ref /opt:icf
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "Steem - Win32 Debug"
# Name "Steem - Win32 Release"
# Name "Steem - Win32 Boiler"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;def;odl;idl;hpj;bat;asm"
# Begin Source File

SOURCE=..\..\steem\emu.cpp
DEP_CPP_EMU_C=\
	"..\..\3rdparty\hatari\configuration.h"\
	"..\..\3rdparty\hatari\cycInt.h"\
	"..\..\3rdparty\hatari\cycles.h"\
	"..\..\3rdparty\hatari\ikbd.c"\
	"..\..\3rdparty\hatari\ikbd.h"\
	"..\..\3rdparty\hatari\joy.h"\
	"..\..\3rdparty\hatari\log.h"\
	"..\..\3rdparty\hatari\mfp.h"\
	"..\..\3rdparty\hatari\screen.h"\
	"..\..\3rdparty\hatari\SDL_keysym.h"\
	"..\..\3rdparty\hatari\utils.c"\
	"..\..\3rdparty\hatari\utils.h"\
	"..\..\3rdparty\hatari\video.c"\
	"..\..\3rdparty\hatari\video.h"\
	"..\..\3rdparty\pasti\div68kCycleAccurate.c"\
	"..\..\3rdparty\pasti\pasti.h"\
	"..\..\include\binary.h"\
	"..\..\include\circularbuffer.h"\
	"..\..\include\clarity.h"\
	"..\..\include\dirsearch.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easystr.h"\
	"..\..\include\internal_speaker.h"\
	"..\..\include\mymisc.h"\
	"..\..\include\notwin_mymisc.h"\
	"..\..\include\portio.h"\
	"..\..\include\x\x_mymisc.h"\
	"..\..\steem\code\acc.h"\
	"..\..\steem\code\blitter.cpp"\
	"..\..\steem\code\blitter.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\cpu.cpp"\
	"..\..\steem\code\cpu.h"\
	"..\..\steem\code\cpuinit.cpp"\
	"..\..\steem\code\debug_emu.cpp"\
	"..\..\steem\code\debug_emu.h"\
	"..\..\steem\code\display.h"\
	"..\..\steem\code\draw.cpp"\
	"..\..\steem\code\draw.h"\
	"..\..\steem\code\emulator.cpp"\
	"..\..\steem\code\emulator.h"\
	"..\..\steem\code\fdc.cpp"\
	"..\..\steem\code\fdc.h"\
	"..\..\steem\code\floppy_drive.h"\
	"..\..\steem\code\gui.h"\
	"..\..\steem\code\hdimg.h"\
	"..\..\steem\code\ikbd.cpp"\
	"..\..\steem\code\ikbd.h"\
	"..\..\steem\code\init_sound.h"\
	"..\..\steem\code\iolist.h"\
	"..\..\steem\code\ior.cpp"\
	"..\..\steem\code\iorw.h"\
	"..\..\steem\code\iow.cpp"\
	"..\..\steem\code\loadsave.h"\
	"..\..\steem\code\loadsave_emu.cpp"\
	"..\..\steem\code\macros.h"\
	"..\..\steem\code\mfp.cpp"\
	"..\..\steem\code\mfp.h"\
	"..\..\steem\code\midi.cpp"\
	"..\..\steem\code\midi.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\onegame.h"\
	"..\..\steem\code\osd.h"\
	"..\..\steem\code\palette.h"\
	"..\..\steem\code\psg.cpp"\
	"..\..\steem\code\psg.h"\
	"..\..\steem\code\reset.cpp"\
	"..\..\steem\code\reset.h"\
	"..\..\steem\code\rs232.cpp"\
	"..\..\steem\code\rs232.h"\
	"..\..\steem\code\run.cpp"\
	"..\..\steem\code\run.h"\
	"..\..\steem\code\shortcutbox.h"\
	"..\..\Steem\code\SSE.h"\
	"..\..\steem\code\SSECpu.cpp"\
	"..\..\steem\code\SSECpu.h"\
	"..\..\steem\code\SSEVideo.cpp"\
	"..\..\steem\code\SSEVideo.h"\
	"..\..\steem\code\SteemFreeImage.h"\
	"..\..\steem\code\steemh.h"\
	"..\..\steem\code\stemdos.cpp"\
	"..\..\steem\code\stemdos.h"\
	"..\..\steem\code\stjoy.h"\
	"..\..\steem\code\stports.cpp"\
	"..\..\steem\code\stports.h"\
	"..\..\steem\code\translate.h"\
	"..\..\steem\code\x\x_midi.cpp"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_EMU_C=\
	"..\..\3rdparty\hatari\avi_record.h"\
	"..\..\3rdparty\hatari\dmaSnd.h"\
	"..\..\3rdparty\hatari\falcon\hostscreen.h"\
	"..\..\3rdparty\hatari\falcon\videl.h"\
	"..\..\3rdparty\hatari\ioMem.h"\
	"..\..\3rdparty\hatari\keymap.h"\
	"..\..\3rdparty\hatari\m68000.h"\
	"..\..\3rdparty\hatari\main.h"\
	"..\..\3rdparty\hatari\memorySnapShot.h"\
	"..\..\3rdparty\hatari\printer.h"\
	"..\..\3rdparty\hatari\screenSnapShot.h"\
	"..\..\3rdparty\hatari\shortcut.h"\
	"..\..\3rdparty\hatari\sound.h"\
	"..\..\3rdparty\hatari\spec512.h"\
	"..\..\3rdparty\hatari\stMemory.h"\
	"..\..\3rdparty\hatari\vdi.h"\
	"..\..\3rdparty\hatari\ymFormat.h"\
	"..\..\include\beos\be_mymisc.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\steem\helper.cpp
DEP_CPP_HELPE=\
	"..\..\include\binary.h"\
	"..\..\include\choosefolder.cpp"\
	"..\..\include\circularbuffer.cpp"\
	"..\..\include\circularbuffer.h"\
	"..\..\include\clarity.h"\
	"..\..\include\configstorefile.cpp"\
	"..\..\include\configstorefile.h"\
	"..\..\include\configstorefile_bad.cpp"\
	"..\..\include\directory_tree.cpp"\
	"..\..\include\directory_tree.h"\
	"..\..\include\dirsearch.cpp"\
	"..\..\include\dirsearch.h"\
	"..\..\include\dynamicarray.cpp"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easycompress.cpp"\
	"..\..\include\easystr.cpp"\
	"..\..\include\easystr.h"\
	"..\..\include\easystringlist.cpp"\
	"..\..\include\easystringlist.h"\
	"..\..\include\input_prompt.cpp"\
	"..\..\include\input_prompt.h"\
	"..\..\include\mymisc.cpp"\
	"..\..\include\mymisc.h"\
	"..\..\include\notwin_mymisc.cpp"\
	"..\..\include\notwin_mymisc.h"\
	"..\..\include\portio.cpp"\
	"..\..\include\portio.h"\
	"..\..\include\scrollingcontrolswin.cpp"\
	"..\..\include\scrollingcontrolswin.h"\
	"..\..\include\x\hxc.cpp"\
	"..\..\include\x\hxc.h"\
	"..\..\include\x\hxc_alert.cpp"\
	"..\..\include\x\hxc_alert.h"\
	"..\..\include\x\hxc_button.cpp"\
	"..\..\include\x\hxc_dir_lv.cpp"\
	"..\..\include\x\hxc_dir_lv.h"\
	"..\..\include\x\hxc_dropdown.cpp"\
	"..\..\include\x\hxc_edit.cpp"\
	"..\..\include\x\hxc_fileselect.cpp"\
	"..\..\include\x\hxc_fileselect.h"\
	"..\..\include\x\hxc_listview.cpp"\
	"..\..\include\x\hxc_popup.cpp"\
	"..\..\include\x\hxc_popup.h"\
	"..\..\include\x\hxc_popuphints.cpp"\
	"..\..\include\x\hxc_popuphints.h"\
	"..\..\include\x\hxc_prompt.cpp"\
	"..\..\include\x\hxc_prompt.h"\
	"..\..\include\x\hxc_scrollarea.cpp"\
	"..\..\include\x\hxc_scrollbar.cpp"\
	"..\..\include\x\hxc_textdisplay.cpp"\
	"..\..\include\x\icongroup.cpp"\
	"..\..\include\x\icongroup.h"\
	"..\..\include\x\x_mymisc.cpp"\
	"..\..\include\x\x_mymisc.h"\
	"..\..\include\x\x_portio.cpp"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\translate.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_HELPE=\
	"..\..\include\beos\be_mymisc.cpp"\
	"..\..\include\beos\be_mymisc.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\Steem\code\SSE.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\Steem.cpp
DEP_CPP_STEEM=\
	"..\..\3rdparty\pasti\pasti.h"\
	"..\..\3rdparty\unrarlib\unrarlib\unrarlib.c"\
	"..\..\3rdparty\unrarlib\unrarlib\unrarlib.h"\
	"..\..\include\binary.h"\
	"..\..\include\choosefolder.h"\
	"..\..\include\circularbuffer.h"\
	"..\..\include\clarity.h"\
	"..\..\include\configstorefile.h"\
	"..\..\include\dderr_meaning.h"\
	"..\..\include\di_get_contents.cpp"\
	"..\..\include\di_get_contents.h"\
	"..\..\include\directory_tree.h"\
	"..\..\include\dirsearch.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easycompress.h"\
	"..\..\include\easystr.h"\
	"..\..\include\easystringlist.h"\
	"..\..\include\input_prompt.h"\
	"..\..\include\mymisc.h"\
	"..\..\include\notwin_mymisc.h"\
	"..\..\include\portio.h"\
	"..\..\include\scrollingcontrolswin.h"\
	"..\..\include\wordwrapper.cpp"\
	"..\..\include\wordwrapper.h"\
	"..\..\include\x\hxc.h"\
	"..\..\include\x\hxc_alert.h"\
	"..\..\include\x\hxc_dir_lv.h"\
	"..\..\include\x\hxc_fileselect.h"\
	"..\..\include\x\hxc_popup.h"\
	"..\..\include\x\hxc_popuphints.h"\
	"..\..\include\x\hxc_prompt.h"\
	"..\..\include\x\icongroup.h"\
	"..\..\include\x\x_mymisc.h"\
	"..\..\steem\code\acc.cpp"\
	"..\..\steem\code\acc.h"\
	"..\..\steem\code\archive.cpp"\
	"..\..\steem\code\archive.h"\
	"..\..\steem\code\associate.cpp"\
	"..\..\steem\code\blitter.h"\
	"..\..\steem\code\boiler.cpp"\
	"..\..\steem\code\boiler.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\controls.cpp"\
	"..\..\steem\code\cpu.h"\
	"..\..\steem\code\d2.cpp"\
	"..\..\steem\code\d2.h"\
	"..\..\steem\code\dataloadsave.cpp"\
	"..\..\steem\code\dataloadsave.h"\
	"..\..\steem\code\debug_emu.h"\
	"..\..\steem\code\dir_id.cpp"\
	"..\..\steem\code\dir_id.h"\
	"..\..\steem\code\diskman.cpp"\
	"..\..\steem\code\diskman.h"\
	"..\..\steem\code\diskman_diags.cpp"\
	"..\..\steem\code\diskman_drag.cpp"\
	"..\..\steem\code\display.cpp"\
	"..\..\steem\code\display.h"\
	"..\..\steem\code\draw.h"\
	"..\..\steem\code\dwin_edit.cpp"\
	"..\..\steem\code\dwin_edit.h"\
	"..\..\steem\code\emulator.h"\
	"..\..\steem\code\fdc.h"\
	"..\..\steem\code\floppy_drive.cpp"\
	"..\..\steem\code\floppy_drive.h"\
	"..\..\steem\code\gui.cpp"\
	"..\..\steem\code\gui.h"\
	"..\..\steem\code\harddiskman.cpp"\
	"..\..\steem\code\harddiskman.h"\
	"..\..\steem\code\hdimg.cpp"\
	"..\..\steem\code\hdimg.h"\
	"..\..\steem\code\historylist.cpp"\
	"..\..\steem\code\historylist.h"\
	"..\..\steem\code\ikbd.h"\
	"..\..\steem\code\include.h"\
	"..\..\steem\code\infobox.cpp"\
	"..\..\steem\code\infobox.h"\
	"..\..\steem\code\init_sound.cpp"\
	"..\..\steem\code\init_sound.h"\
	"..\..\steem\code\iolist.cpp"\
	"..\..\steem\code\iolist.h"\
	"..\..\steem\code\iorw.h"\
	"..\..\steem\code\key_table.h"\
	"..\..\steem\code\loadsave.cpp"\
	"..\..\steem\code\loadsave.h"\
	"..\..\steem\code\macros.cpp"\
	"..\..\steem\code\macros.h"\
	"..\..\steem\code\main.cpp"\
	"..\..\steem\code\mem_browser.cpp"\
	"..\..\steem\code\mem_browser.h"\
	"..\..\steem\code\mfp.h"\
	"..\..\steem\code\midi.h"\
	"..\..\steem\code\mr_static.cpp"\
	"..\..\steem\code\mr_static.h"\
	"..\..\steem\code\notifyinit.cpp"\
	"..\..\steem\code\notifyinit.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\onegame.cpp"\
	"..\..\steem\code\onegame.h"\
	"..\..\steem\code\options.cpp"\
	"..\..\steem\code\options.h"\
	"..\..\steem\code\options_create.cpp"\
	"..\..\steem\code\osd.cpp"\
	"..\..\steem\code\osd.h"\
	"..\..\steem\code\palette.cpp"\
	"..\..\steem\code\palette.h"\
	"..\..\steem\code\patchesbox.cpp"\
	"..\..\steem\code\patchesbox.h"\
	"..\..\steem\code\psg.h"\
	"..\..\steem\code\reset.h"\
	"..\..\steem\code\resnum.h"\
	"..\..\steem\code\rs232.h"\
	"..\..\steem\code\run.h"\
	"..\..\steem\code\screen_saver.cpp"\
	"..\..\steem\code\screen_saver.h"\
	"..\..\steem\code\shortcutbox.cpp"\
	"..\..\steem\code\shortcutbox.h"\
	"..\..\Steem\code\SSE.cpp"\
	"..\..\Steem\code\SSE.h"\
	"..\..\steem\code\SteemFreeImage.h"\
	"..\..\steem\code\steemh.h"\
	"..\..\steem\code\steemintro.cpp"\
	"..\..\steem\code\stemdialogs.cpp"\
	"..\..\steem\code\stemdialogs.h"\
	"..\..\steem\code\stemdos.h"\
	"..\..\steem\code\stemwin.cpp"\
	"..\..\steem\code\stjoy.cpp"\
	"..\..\steem\code\stjoy.h"\
	"..\..\steem\code\stjoy_directinput.cpp"\
	"..\..\steem\code\stports.h"\
	"..\..\steem\code\trace.cpp"\
	"..\..\steem\code\translate.h"\
	"..\..\steem\code\unzip_win32.h"\
	"..\..\steem\code\x\x_controls.cpp"\
	"..\..\steem\code\x\x_controls.h"\
	"..\..\steem\code\x\x_diskman.cpp"\
	"..\..\steem\code\x\x_display.cpp"\
	"..\..\steem\code\x\x_gui.cpp"\
	"..\..\steem\code\x\x_harddiskman.cpp"\
	"..\..\steem\code\x\x_infobox.cpp"\
	"..\..\steem\code\x\x_joy.cpp"\
	"..\..\steem\code\x\x_notifyinit.cpp"\
	"..\..\steem\code\x\x_options.cpp"\
	"..\..\steem\code\x\x_options_create.cpp"\
	"..\..\steem\code\x\x_patchesbox.cpp"\
	"..\..\steem\code\x\x_screen_saver.cpp"\
	"..\..\steem\code\x\x_shortcutbox.cpp"\
	"..\..\steem\code\x\x_sound.cpp"\
	"..\..\steem\code\x\x_sound_portaudio.cpp"\
	"..\..\steem\code\x\x_sound_rtaudio.cpp"\
	"..\..\steem\code\x\x_stemdialogs.cpp"\
	"..\..\steem\code\x\x_stemwin.cpp"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_STEEM=\
	"..\..\..\..\..\usr\include\linux\kd.h"\
	"..\..\include\beos\be_mymisc.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc"
# Begin Source File

SOURCE=..\..\Steem\code\SSE.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\steem\rc\debug_close.ico
# End Source File
# Begin Source File

SOURCE=..\..\Steem\rc\debug_icons.bmp
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\debug_trash.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\disk_manager.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_accurate_fdc.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_back.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_disk.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_disk_history_menu.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_disk_link.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_disk_link_broken.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_disk_readonly.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_disk_zip_readonly.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_disk_zip_readwrite.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_drive_a.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_drive_b.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_drive_b_disabled.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_folder.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_folder_link.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_folder_link_broken.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_folder_parent.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_forward.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_hard_drives.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_hard_drives_francais.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_home.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_menu.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_set_home.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_tools_menu.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\fast_forward.ico
# End Source File
# Begin Source File

SOURCE=..\..\Steem\rc\flags.bmp
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\fullscreen_quit.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\hard_drive.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\info.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\info_cart_howto.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\info_disk_howto.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\info_drawspeed.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\info_faq.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\info_links.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\info_text.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\joy.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\macro_play.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\memory_snapshot.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\memory_snapshot_file_icon.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_associations.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_brightcon.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_display.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_fullscreen.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_general.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_icons.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_machine.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_macros.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_midi.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_osd.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_ports.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_profiles.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_sound.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_startup.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_tos.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\paste.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\patch.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\patch_new.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\pc_folder.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\point.cur
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\program_file_icon.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\record.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\reset.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\reset_need.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\resource.rc
# ADD BASE RSC /l 0x809 /i "\data\prg\ST\steem\rc"
# ADD RSC /l 0x409 /i "\data\prg\ST\steem\rc"
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\run.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\shortcut.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\shortcut_off.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\shortcut_on.ico
# End Source File
# Begin Source File

SOURCE=..\..\Steem\rc\st_chars_mono.bmp
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\steem.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\steem_256_file_icon.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\take_screenshot.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\update.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\windowed_mode.ico
# End Source File
# End Group
# Begin Group "Third Party"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\3rdparty\unrarlib\unrarlib\unrarlib.c
DEP_CPP_UNRAR=\
	"..\..\3rdparty\unrarlib\unrarlib\unrarlib.h"\
	
# End Source File
# End Group
# Begin Group "Object Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\steem\asm\asm_draw_VC.obj
# End Source File
# Begin Source File

SOURCE=..\..\steem\asm\asm_osd_VC.obj
# End Source File
# Begin Source File

SOURCE=..\..\include\asm\asm_portio_vc.obj
# End Source File
# End Group
# Begin Group "code"

# PROP Default_Filter ""
# End Group
# Begin Source File

SOURCE=..\..\Steem\rc\charset.blk
# End Source File
# End Target
# End Project
