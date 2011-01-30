#ifndef CONFIGSTOREFILE_H
#define CONFIGSTOREFILE_H

#include <dynamicarray.h>
#include <easystringlist.h>

typedef struct{
  int iSect,iKey;
}CSF_FIND;

typedef struct{
  char *szName,*szNameUpr;
}CSF_SECTS;

typedef struct{
  char *szName,*szNameUpr,*szValue;
  int iSect;
}CSF_KEYS;

class ConfigStoreFile
{
private:
  EasyStr Path;
  EasyStr FileBuf,FileUprBuf;
  DynamicArray<CSF_SECTS> Sects;
  DynamicArray<CSF_KEYS> Keys;
  DynamicArray<char*> szNewMem;
public:
  ConfigStoreFile(char* = NULL);
  ~ConfigStoreFile();

  void Open(char* = NULL);
  bool Close(),SaveTo(char*);

  bool FindKey(EasyStr,char *,CSF_FIND*);

  int GetInt(char *,char *,int);
  EasyStr GetStr(char *,char *,char *);

  void SetInt(char *s,char *k,int v);
  void SetStr(char *,char *,char *);

  void GetSectionNameList(EasyStringList*);

  bool Changed;
};

#define WriteCSFInt(s,k,v,f) WriteCSFStr(s,k,EasyStr(v),f)
extern void WriteCSFStr(char *,char *,char *,char *);
extern EasyStr GetCSFStr(char *,char *,char *,char *);
extern int GetCSFInt(char *,char *,int,char *);

#endif

