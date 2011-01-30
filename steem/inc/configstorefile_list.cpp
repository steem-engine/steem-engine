#ifndef CONFIG_STORE_FILE_H
#define CONFIG_STORE_FILE_H
//---------------------------------------------------------------------------
#include <io.h>
#include <easystr.h>
#include <mymisc.h>
#include <easystringlist.h>
//---------------------------------------------------------------------------
typedef struct{
  EasyStringList *pKeys,*pKeysUpr,*pValues;
}CSF_SECT_LISTS;
//---------------------------------------------------------------------------
class ConfigStoreFile
{
private:
public:
  ConfigStoreFile(char* = NULL);
  ~ConfigStoreFile();

  void Open(char* = NULL);
  bool Close(),SaveTo(char*);

  void AddSection(char*,CSF_SECT_LISTS*);
  void AddKey(CSF_SECT_LISTS*,char*,char*);
  bool GetSection(char*,CSF_SECT_LISTS*);

  unsigned int GetInt(EasyStr,EasyStr,unsigned int);
  EasyStr GetStr(EasyStr,EasyStr,char *);

  void SetInt(EasyStr,EasyStr,unsigned int);
  void SetStr(EasyStr,EasyStr,char *);

  void GetSectionNameList(EasyStringList*);

  EasyStr Path;
  EasyStringList Sections,SectionsUpr;
  bool Changed;
};
//---------------------------------------------------------------------------
ConfigStoreFile::ConfigStoreFile(char *NewPath)
{
  Sections.Sort=eslNoSort;
  SectionsUpr.Sort=eslNoSort;
  Changed=0;
  if (NewPath) Open(NewPath);
}
//---------------------------------------------------------------------------
ConfigStoreFile::~ConfigStoreFile()
{
  Close();
}
//---------------------------------------------------------------------------
void ConfigStoreFile::AddSection(char *Name,CSF_SECT_LISTS *pLists)
{
  pLists->pKeys=new EasyStringList();
  pLists->pKeysUpr=new EasyStringList();
  pLists->pValues=new EasyStringList();
  pLists->pKeys->Sort=eslNoSort;
  pLists->pKeysUpr->Sort=eslNoSort;
  pLists->pValues->Sort=eslNoSort;

  Sections.Add(Name);
  strupr(Name);
  SectionsUpr.Add(3,Name,(long)(pLists->pKeys),(long)(pLists->pKeysUpr),(long)(pLists->pValues));
}
//---------------------------------------------------------------------------
bool ConfigStoreFile::GetSection(char *NameUpr,CSF_SECT_LISTS *pLists)
{
  for (int s=0;s<SectionsUpr.NumStrings;s++){
    if (strcmp(SectionsUpr[s].String,NameUpr)==0){
      pLists->pKeys=(EasyStringList*)(SectionsUpr[s].Data[0]);
      pLists->pKeysUpr=(EasyStringList*)(SectionsUpr[s].Data[1]);
      pLists->pValues=(EasyStringList*)(SectionsUpr[s].Data[2]);
      return true;
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
void ConfigStoreFile::AddKey(CSF_SECT_LISTS *pLists,char *Name,char *Val)
{
  pLists->pKeys->Add(Name);
  strupr(Name);
  pLists->pKeysUpr->Add(Name);
  pLists->pValues->Add(Val);
}
//---------------------------------------------------------------------------
void ConfigStoreFile::Open(char *NewPath)
{
  if (Path.NotEmpty()) return; // A file is already open

  Path=NewPath;

  FILE *f=fopen(NewPath,"rb");
  if (f==NULL) return;

  // Load in all text
  int Len=GetFileLength(f);
  EasyStr File;File.SetLength(Len);
  ZeroMemory(File.Text,Len);
  fread(File.Text,Len,1,f);
  fclose(f);

  // Find all returns and change to NULL
  char *tp=File.Text;
  for(;;){
    tp=strchr(tp,'\n');
    if (tp==NULL) break;
    *tp=0;
    if ((tp-1) >= File.Text) if (*(tp-1)=='\r') *(tp-1)=0;
    tp++;
  }

  char *tend=File.Text+Len;
  tp=File.Text;
  CSF_SECT_LISTS Lists={NULL,NULL,NULL};
  for(;;){
    if (*tp == '['){
      *(tp+strlen(tp)-1)=0; // Cut off ]
      AddSection(tp+1,&Lists);
    }else if (Lists.pKeys){
      char *eq=strchr(tp,'=');
      if (eq){
        *eq=0;
        char *Val=tp+strlen(tp)+1;
        AddKey(&Lists,tp,Val);
        tp=Val;
      }
    }
    do{
      tp+=strlen(tp)+1;
      if (tp>=tend) break;
    }while (tp[0]==0);
    if (tp>=tend) break;
  }
}
//---------------------------------------------------------------------------
bool ConfigStoreFile::Close()
{
  bool SavedOkay=true;
  if (Path.NotEmpty()){
    if (Changed) SavedOkay=SaveTo(Path);
    for (int s=0;s<SectionsUpr.NumStrings;s++){
      for (int n=0;n<3;n++) delete (EasyStringList*)(SectionsUpr[s].Data[n]);
    }
    Sections.DeleteAll();
    SectionsUpr.DeleteAll();
  }
  Path="";
  Changed=0;
  return SavedOkay;
}
//---------------------------------------------------------------------------
bool ConfigStoreFile::SaveTo(char *File)
{
  FILE *f=fopen(File,"wb");
  if (f==NULL) return 0;

  EasyStringList *pKeys,*pValues;
  for (int i=0;i<SectionsUpr.NumStrings;i++){
    fprintf(f,"[%s]\r\n",Sections[i].String);
    pKeys=(EasyStringList*)(SectionsUpr[i].Data[0]);
    pValues=(EasyStringList*)(SectionsUpr[i].Data[2]);
    for (int k=0;k<pKeys->NumStrings;k++){
      fprintf(f,"%s=%s\r\n",pKeys->Get(k).String,pValues->Get(k).String);
    }
    fprintf(f,"\r\n");
  }
  fclose(f);
  return true;
}
//---------------------------------------------------------------------------
unsigned int ConfigStoreFile::GetInt(EasyStr Sect,EasyStr Key,unsigned int DefVal)
{
  CSF_SECT_LISTS Lists;
  strupr(Sect);
  if (GetSection(Sect,&Lists)==0) return DefVal;

  strupr(Key);
  int k=Lists.pKeysUpr->FindString(Key);
  if (k<0) return DefVal;

  return atoi(Lists.pValues->Get(k).String);
}
//---------------------------------------------------------------------------
EasyStr ConfigStoreFile::GetStr(EasyStr Sect,EasyStr Key,char *DefVal)
{
  CSF_SECT_LISTS Lists;
  strupr(Sect);
  if (GetSection(Sect,&Lists)==0) return DefVal;

  strupr(Key);
  int k=Lists.pKeysUpr->FindString(Key);
  if (k<0) return DefVal;

  return Lists.pValues->Get(k).String;
}
//---------------------------------------------------------------------------
void ConfigStoreFile::SetInt(EasyStr Sect,EasyStr Key,unsigned int Val)
{
  Changed=true;
  CSF_SECT_LISTS Lists;
  if (GetSection(Sect.UpperCase(),&Lists)){
    int k=Lists.pKeysUpr->FindString(Key.UpperCase());
    if (k>=0){
      Lists.pValues->SetString(k,EasyStr(Val));
      return;
    }
  }else{
    AddSection(Sect,&Lists);
  }
  AddKey(&Lists,Key,EasyStr(Val));
}
//---------------------------------------------------------------------------
void ConfigStoreFile::SetStr(EasyStr Sect,EasyStr Key,char *Val)
{
  Changed=true;
  CSF_SECT_LISTS Lists;
  if (GetSection(Sect.UpperCase(),&Lists)){
    int k=Lists.pKeysUpr->FindString(Key.UpperCase());
    if (k>=0){
      Lists.pValues->SetString(k,Val);
      return;
    }
  }else{
    AddSection(Sect,&Lists);
  }
  AddKey(&Lists,Key,Val);
}
//---------------------------------------------------------------------------
void ConfigStoreFile::GetSectionNameList(EasyStringList *pESL)
{
  for (int i=0;i<Sections.NumStrings;i++){
    pESL->Add(Sections[i].String);
  }
}
//---------------------------------------------------------------------------
// Global functions to replace WriteP... and GetP...
//---------------------------------------------------------------------------
#if !defined(WriteCSFInt) && !defined(CSF_NO_GLOBALS)
#define WriteCSFInt(s,k,v,f) WriteCSFStr(s,k,EasyStr(v),f)

void WriteCSFStr(char *Sect,char *Key,char *Val,char *File)
{
  ConfigStoreFile CSF(File);
  CSF.SetStr(Sect,Key,Val);
  CSF.Close();
}
//---------------------------------------------------------------------------
EasyStr GetCSFStr(char *Sect,char *Key,char *DefVal,char *File)
{
  ConfigStoreFile CSF(File);
  return CSF.GetStr(Sect,Key,DefVal);
}
//---------------------------------------------------------------------------
unsigned int GetCSFInt(char *Sect,char *Key,unsigned int DefVal,char *File)
{
  ConfigStoreFile CSF(File);
  return CSF.GetInt(Sect,Key,DefVal);
}
//---------------------------------------------------------------------------
#endif

#endif

