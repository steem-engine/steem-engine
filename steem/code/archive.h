#ifdef UNIX
#include <unzip.h>
#endif

#ifdef WIN32
#include "unzip_win32.h"
#endif

#ifndef NO_RAR_SUPPORT
#define RAR_SUPPORT
#endif

#ifdef RAR_SUPPORT

#include <unrarlib.h>

#endif

#define ZIPPY_FAIL true
#define ZIPPY_SUCCEED 0

class zipclass{
private:
public:
#ifdef UNIX
  //use zlib
  unzFile uf;
  unz_global_info gi;
  char filename_inzip[256];
  unz_file_info fi;
#endif
#ifdef WIN32
  PackStruct PackInfo;
#endif
#ifdef RAR_SUPPORT
  ArchiveList_struct *rar_list,*rar_current;
#endif

  bool is_open;
  int current_file_n;
  int current_file_offset;
  WORD attrib;
  long crc;
  int err;
  char type[12];
  EasyStr last_error;
  zipclass();
  ~zipclass(){ };

  bool first(char*);
  bool next();
  bool close();
  char* filename_in_zip();
  void list_contents(char*,EasyStringList*,bool=false);
  bool extract_file(char*,int,char*,bool=false,DWORD=0);
}zippy;


