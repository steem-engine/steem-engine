#ifndef BAD_CONFIG_STORE_FILE_H
#define BAD_CONFIG_STORE_FILE_H
//---------------------------------------------------------------------------
#include <easystr.h>
#include <mymisc.h>

#include <configstorefile.h>
//---------------------------------------------------------------------------
char CSF_text_buf[5001];
//---------------------------------------------------------------------------
ConfigStoreFile::ConfigStoreFile(char *NewPath)
{
  if (NewPath) Open(NewPath);
}
ConfigStoreFile::~ConfigStoreFile()
{
  Close();
}
//---------------------------------------------------------------------------
bool ConfigStoreFile::Close() { Path="";return true; }
//---------------------------------------------------------------------------
void ConfigStoreFile::Open(char *NewPath)
{
  if (Path.NotEmpty()) return; // A file is already open

  Path=NewPath;
}
//---------------------------------------------------------------------------
int ConfigStoreFile::GetInt(char *Sect,char *Key,int DefVal)
{
  return (int)GetPrivateProfileInt(Sect,Key,(UINT)DefVal,Path);
}
//---------------------------------------------------------------------------
EasyStr ConfigStoreFile::GetStr(char *Sect,char *Key,char *DefVal)
{
  GetPrivateProfileString(Sect,Key,DefVal,CSF_text_buf,5000,Path);
  return CSF_text_buf;
}
//---------------------------------------------------------------------------
void ConfigStoreFile::SetInt(char *Sect,char *Key,int Val)
{
  WritePrivateProfileString(Sect,Key,EasyStr(Val),Path);
}
//---------------------------------------------------------------------------
void ConfigStoreFile::SetStr(char *Sect,char *Key,char *Val)
{
  WritePrivateProfileString(Sect,Key,Val,Path);
}
//---------------------------------------------------------------------------
void ConfigStoreFile::GetSectionNameList(EasyStringList *pESL)
{
  char Names[5001],*tp=Names;
  GetPrivateProfileSectionNames(Names,5000,Path);
  do{
    pESL->Add(tp);
    tp+=strlen(tp)+1;
  }while (tp[0]);
}
//---------------------------------------------------------------------------
bool ConfigStoreFile::SaveTo(char*)
{
  return true;
}
//---------------------------------------------------------------------------
// Global functions to replace WriteP... and GetP...
//---------------------------------------------------------------------------
void WriteCSFStr(char *Sect,char *Key,char *Val,char *File)
{
  WritePrivateProfileString(Sect,Key,Val,File);
}
//---------------------------------------------------------------------------
EasyStr GetCSFStr(char *Sect,char *Key,char *DefVal,char *File)
{
  GetPrivateProfileString(Sect,Key,DefVal,CSF_text_buf,5000,File);
  return CSF_text_buf;
}
//---------------------------------------------------------------------------
int GetCSFInt(char *Sect,char *Key,int DefVal,char *File)
{
  return (int)GetPrivateProfileInt(Sect,Key,DefVal,File);
}
//---------------------------------------------------------------------------
#endif

