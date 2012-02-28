/*---------------------------------------------------------------------------
FILE: helper.cpp
MODULE: helper
DESCRIPTION: The hub for the helper module that includes utility functions
used by Steem. Basically includes all the files that are in the object.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#define IN_HELPER

#include "conditions.h"


// SS: VC++ 6.0 (and below) scoping bugfix
#ifdef _VC_BUILD
#if defined(_MSC_VER) && _MSC_VER <= 1200
#define for if(0); else for
#endif
#endif


#include <easystr.cpp>
#include <mymisc.cpp>
#include <easycompress.cpp>
#include <easystringlist.cpp>
#include <circularbuffer.cpp>
#include <dynamicarray.cpp>

#include "translate.h"

#include <portio.cpp>
#include <dirsearch.cpp>

#ifdef WIN32

#include <choosefolder.cpp>
#include <scrollingcontrolswin.cpp>
#include <directory_tree.cpp>
#include <input_prompt.cpp>

#else

#include <x/hxc.cpp>
#include <x/hxc_alert.cpp>
#include <x/hxc_fileselect.cpp>
#include <x/hxc_popup.cpp>
#include <x/hxc_popuphints.cpp>
#include <x/hxc_dir_lv.cpp>
#include <x/hxc_prompt.cpp>
#include <x/icongroup.cpp>

#endif

#if defined(ONEGAME) || defined(NO_CSF) && !defined(UNIX)
#include <configstorefile_bad.cpp>
#else
#include <configstorefile.cpp>
#endif


