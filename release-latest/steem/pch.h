/*---------------------------------------------------------------------------
FILE: pch.h
MODULES: ALL
DESCRIPTION: System specific includes, declarations and macros. PCH stands
for pre-compiled headers (to reduce build time on slower development PCs).
---------------------------------------------------------------------------*/

#ifndef __PCH_H
#define __PCH_H

#if defined(LINUX) || defined(CYGWIN)

#ifndef UNIX
#define UNIX
#endif

#elif !defined(WIN32)

#define WIN32

#endif

#ifdef WIN32

#define WINVER 0x0400
#define _WIN32_WINNT 0x0300
#define DIRECTDRAW_VERSION 0x0200
#define DIRECTINPUT_VERSION 0x0500
#define DIRECTSOUND_VERSION 0x0200
#define OEMRESOURCE 1
#define STRICT 1
#pragma anon_structs on
//---------------------------------------------------------------------------
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <io.h>
#include <math.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <binary.h>
#include <time.h>
#include <setjmp.h>
#include <clarity.h>

#include <ddraw.h>
#include <dsound.h>
#include <dinput.h>

#if defined(_BCB_BUILD) || defined(_BCC_BUILD)

#include <except.h>
#include <dos.h>

#ifndef SPI_SETSCREENSAVERRUNNING
#define SPI_SETSCREENSAVERRUNNING 97
#endif

#elif defined(_MINGW_BUILD)

#include <winnls.h>
#include <ctype.h>

#define _INC_TIME
#define _INC_COMMCTRL
#define _SHLOBJ_H_
#define EnumDateFormats EnumDateFormatsA
#define SUBLANG_SWEDISH 0x01    // Swedish
#define LVS_EX_CHECKBOXES 0x00000004
#define LVM_SETICONSPACING (LVM_FIRST + 53)
#define ListView_SetIconSpacing(hwndLV,cx,cy) (DWORD)SendMessage((hwndLV), LVM_SETICONSPACING, 0, MAKELONG(cx,cy))
#define LVM_SETEXTENDEDLISTVIEWSTYLE (LVM_FIRST + 54)   // optional wParam == mask
#define ListView_SetExtendedListViewStyle(hwndLV,dw) (DWORD)SendMessage((hwndLV), LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dw)
#define TTM_SETMAXTIPWIDTH (WM_USER + 24)
#define SPI_SETSCREENSAVERRUNNING 97
#define LANG_BASQUE 0x2d
#define LANG_CATALAN 0x03

extern char *ultoa(unsigned long,char*,int);
extern char *strupr(char*);
extern char *strlwr(char*);
extern int random(int);

#elif defined(_VC_BUILD)

extern int random(int);

#endif

#ifndef CSIDL_PROGRAM_FILES
#define CSIDL_PROGRAM_FILES                             0x0026
#endif
//---------------------------------------------------------------------------
#else
//---------------------------------------------------------------------------
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <binary.h>
#include <limits.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#define _INC_TIME
#include <dirent.h>
#include <setjmp.h>
#include <sys/resource.h>

#ifndef CYGWIN
// XImage shared memory extension
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/xf86vmode.h>
#else
#define NO_SHM
#endif

#include <sys/ioctl.h>
#include <fcntl.h>

#include <clarity.h>
#include <pthread.h>

#ifdef LINUX
#include <linux/joystick.h>
#include <sys/vfs.h>   // for statfs
#endif

#include <unistd.h>

#include <string.h>
#include <ctype.h>

extern char *ultoa(unsigned long,char*,int);
extern char *strupr(char*);
extern char *strlwr(char*);
extern int random(int);
extern char *itoa(int,char*,int);

#endif

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#endif

