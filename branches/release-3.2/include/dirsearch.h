#ifndef DIRSEARCH_H
#define DIRSEARCH_H

class DirSearch
{
private:
#ifdef WIN32
  HANDLE hFind;
#else
  DIR *dp;
  struct dirent *ep;
  struct stat s;
  EasyStr fullpath;
  EasyStr mask;
#endif
public:
  DirSearch();
  DirSearch(char *);
  ~DirSearch();
  bool Find(char *);
  bool Next();
#ifdef WIN32
  void SetFoundFile();
#else
  void SetFoundFile(char *);
#endif
  void Close();

#ifdef WIN32
  WIN32_FIND_DATA FindDat;

  DWORD &Attrib;
  FILETIME &CreationTime;
  FILETIME &LastAccessTime;
  FILETIME &LastWriteTime;
  DWORD &SizeHigh;
  DWORD &SizeLow;

  char *Name;
  char *ShortName;
#else
  DWORD Attrib;
  DWORD CreationTime;
  DWORD LastAccessTime;
  DWORD LastWriteTime;
  DWORD SizeHigh;
  DWORD SizeLow;

  EasyStr Name;
  EasyStr ShortName;
#endif

  bool FoundFile;
  bool st_only;
};


#endif

