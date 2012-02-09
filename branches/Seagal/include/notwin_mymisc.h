#if defined(UNIX) || defined(BEOS)
#ifndef SLASH
#define SLASH "/"
#define SLASHCHAR '/'
#endif

#ifndef MAX_PATH
#define MAX_PATH 5000
#endif

#ifndef DWORD
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef unsigned int UINT;
#endif

#else

#ifndef SLASH
#define SLASH "\\"
#define SLASHCHAR '\\'
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#endif

/*
extern EasyStr CachedPPFile;
extern EasyStr CachedPPText;
extern bool IsCachedPrivateProfile();
extern void UnCachePrivateProfile();
extern bool CachePrivateProfile(char *FileName);
extern EasyStr GetPPEasyStr(char *SectionName,char *KeyName,char *Default,char *FileName);
extern UINT GetPrivateProfileInt(char *Sect,char *Key,int Default,char *File);
extern bool WritePrivateProfileString(char *SectionName,char *KeyName,const char *String,char *FileName);
*/
extern void ZeroMemory(void *Mem,DWORD Len);
extern bool DeleteFile(char *File);
extern UINT GetTempFileName(char *PathName,char *Prefix,UINT Unique,char *TempFileName);

