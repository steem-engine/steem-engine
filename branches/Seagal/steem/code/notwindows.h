#ifdef IN_MAIN
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)
#endif

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;
typedef unsigned int UINT;
typedef char* LPSTR;
typedef const char* LPCSTR;

typedef long long LONGLONG;
typedef unsigned long long DWORDLONG;

typedef unsigned char* LPBYTE;
typedef unsigned short* LPWORD;
typedef unsigned long* LPDWORD;
typedef long* LPLONG;
typedef void* LPVOID;

typedef unsigned long HRESULT;

#define DD_OK 0
#define DDERR_GENERIC 1
#define DDERR_SURFACELOST 2

#define DS_OK 0
#define DSERR_GENERIC 1
#define DSERR_BUFFERLOST 2

typedef struct{
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
}RECT;

typedef struct{
  LONG x;
  LONG y;
}POINT;

#define LOBYTE(a) BYTE(a)
#define HIBYTE(a) BYTE(((WORD)(a)) >> 8)

#define LOWORD(a) WORD(a)
#define HIWORD(a) WORD(((DWORD)(a)) >> 16)

typedef unsigned long LANGID;
#define LANG_ENGLISH 0
#define SUBLANG_ENGLISH_UK 0
#define SUBLANG_ENGLISH_US 1
#define SUBLANG_ENGLISH_AUS 2

#define LANG_FRENCH 1
#define SUBLANG_FRENCH 1
#define SUBLANG_FRENCH_BELGIAN 2

#define LANG_GERMAN 2
#define SUBLANG_GERMAN 2

#define LANG_SWEDISH 3
#define SUBLANG_SWEDISH 3

#define LANG_SPANISH 4
#define SUBLANG_SPANISH 5

#define LANG_CATALAN 5
#define SUBLANG_CATALAN 5

#define LANG_BASQUE 6
#define SUBLANG_BASQUE 6

#define LANG_ITALIAN 7
#define SUBLANG_ITALIAN 7

#define LANG_NORWEGIAN 8
#define SUBLANG_NEUTRAL 2

#define LANG_DANISH 9
#define SUBLANG_DANISH 9

#define MAKELANGID MAKELONG

#define MAX_PATH ((PATH_MAX>2000) ? 2000:PATH_MAX)

#define _NO_DEBUG_BUILD

#define FA_NORMAL   0x00
#define FA_RDONLY   0x01
#define FA_HIDDEN   0x02
#define FA_SYSTEM   0x04
#define FA_LABEL    0x08
#define FA_DIREC    0x10
#define FA_ARCH     0x20


#define kbhit() (feof(stdin)==0)

EXT KeyCode VK_LBUTTON,VK_RBUTTON,VK_MBUTTON;
EXT KeyCode VK_F11,VK_F12,VK_END;
EXT KeyCode VK_LEFT,VK_RIGHT,VK_UP,VK_DOWN,VK_TAB;
EXT KeyCode VK_SHIFT,VK_LSHIFT,VK_RSHIFT;
EXT KeyCode VK_MENU,VK_LMENU,VK_RMENU;
EXT KeyCode VK_CONTROL,VK_LCONTROL,VK_RCONTROL;
EXT KeyCode VK_NUMLOCK,VK_SCROLL;

#include <sys/timeb.h>

#undef EXT
#undef INIT

