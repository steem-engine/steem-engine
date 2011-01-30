#ifndef CONFIGSTOREFILE_CPP
#define CONFIGSTOREFILE_CPP
//---------------------------------------------------------------------------
#ifndef LINUX
#include <io.h>
#endif

#include <easystr.h>
#include <mymisc.h>
#include <easystringlist.h>
#include <dynamicarray.h>

#include <configstorefile.h>

#ifndef CSF_LOG
#define CSF_LOG(s)
#endif

//---------------------------------------------------------------------------
void ConfigStoreFile::SetInt(char *s,char *k,int v) { SetStr(s,k,EasyStr(v)); }
//---------------------------------------------------------------------------
ConfigStoreFile::ConfigStoreFile(char *NewPath)
{
  Changed=0;
  if (NewPath) Open(NewPath);
}
//---------------------------------------------------------------------------
ConfigStoreFile::~ConfigStoreFile()
{
  Close();
}
//---------------------------------------------------------------------------
void ConfigStoreFile::Open(char *NewPath)
{
  if (Path.NotEmpty()){
    CSF_LOG(EasyStr("INI: File already open, can't open ")+NewPath);
    return;
  }

  CSF_LOG(EasyStr("INI: Opening ")+NewPath);
  Path=NewPath;

  FILE *f=fopen(NewPath,"rb");
  if (f==NULL){
    CSF_LOG(EasyStr("INI: Couldn't open file, it probably doesn't exist"));
    return;
  }

  // Load in all text
  CSF_LOG(EasyStr("INI: Loading contents..."));
  int Len=GetFileLength(f);
  FileBuf.SetLength(Len);
  ZeroMemory(FileBuf.Text,Len);
  fread(FileBuf.Text,Len,1,f);
  fclose(f);
  FileUprBuf.SetLength(Len);

  CSF_LOG(EasyStr("INI: Processing returns..."));
  // Find all returns and change to NULL
  int nSects=0,nKeys=0;
  char *tp=FileBuf.Text;
  for (;;){
    tp=strchr(tp,'\n');
    if (tp==NULL) break;
    if (*(tp+1)=='['){
      nSects++;
    }else{
      nKeys++;
    }
    *tp=0;
    if ((tp-1) >= FileBuf.Text) if (*(tp-1)=='\r') *(tp-1)=0;
    tp++;
  }

  CSF_LOG(EasyStr("INI: Making space for ")+(nSects+2)+" sections and "+nKeys+" keys");
  Sects.Resize(nSects+2);
  Keys.Resize(nKeys);

  CSF_LOG(EasyStr("INI: Getting sections and keys"));
  char *tend=FileBuf.Text+Len,*pUpr=FileUprBuf.Text;
  tp=FileBuf.Text;
  nSects=-1;
  for(;;){
    if (*tp == '['){
      int line_len=strlen(tp);
      char *end_name=strchr(tp,']');
      if (end_name) *end_name=0;

      CSF_SECTS s;
      s.szName=tp+1;
      strcpy(pUpr,tp+1);strupr(pUpr);
      s.szNameUpr=pUpr;
      pUpr+=strlen(tp)+1;

      Sects.Add(s);
      nSects++;

      tp+=line_len; // Ignore rest of line
    }else if (nSects>=0){
      char *eq=strchr(tp,'=');
      if (eq){
        *eq=0;

        CSF_KEYS k;
        k.szName=tp;
        strcpy(pUpr,tp);strupr(pUpr);
        k.szNameUpr=pUpr;
        pUpr+=strlen(tp)+1;
        k.szValue=eq+1;
        k.iSect=nSects;

        Keys.Add(k);
        tp=eq+1;
      }
    }
    do{
      tp+=strlen(tp)+1;
      if (tp>=tend) break;
    }while (tp[0]==0);
    if (tp>=tend) break;
  }
  CSF_LOG(EasyStr("INI: All done loading"));
}
//---------------------------------------------------------------------------
bool ConfigStoreFile::Close()
{
  CSF_LOG(EasyStr("INI: Closing"));
  bool SavedOkay=true;
  if (Path.NotEmpty()){
    if (Changed) SavedOkay=SaveTo(Path);

    CSF_LOG(EasyStr("INI: Deleting ")+szNewMem.NumItems+" new keys");
    for (int n=0;n<szNewMem.NumItems;n++) delete[] szNewMem[n];

    CSF_LOG(EasyStr("INI: Deleting all other memory"));
    Sects.DeleteAll();
    Keys.DeleteAll();
    szNewMem.DeleteAll();
    FileBuf="";
    FileUprBuf="";
  }
  Path="";
  Changed=0;
  CSF_LOG(EasyStr("INI: Closing complete, returning ")+SavedOkay);
  return SavedOkay;
}
//---------------------------------------------------------------------------
bool ConfigStoreFile::SaveTo(char *File)
{
  CSF_LOG(EasyStr("INI: Saving settings to ")+File);

  FILE *f=fopen(File,"wb");
  if (f==NULL){
    CSF_LOG(EasyStr("INI: Can't open, losing all changes!"));
    return 0;
  }

  for (int i=0;i<Sects.NumItems;i++){
    fprintf(f,"[%s]\r\n",Sects[i].szName);
    for (int k=0;k<Keys.NumItems;k++){
      if (Keys[k].iSect==i) fprintf(f,"%s=%s\r\n",Keys[k].szName,Keys[k].szValue);
    }
    fprintf(f,"\r\n");
  }
  fclose(f);
  CSF_LOG(EasyStr("INI: Saved and closed file"));
  return true;
}
//---------------------------------------------------------------------------
bool ConfigStoreFile::FindKey(EasyStr Sect,char *KeyVal,CSF_FIND *pSK)
{
  strupr(Sect);
  for (pSK->iSect=Sects.NumItems-1;pSK->iSect>=0;pSK->iSect--){
    if (strcmp(Sects[pSK->iSect].szNameUpr,Sect)==0) break;
  }
  if (pSK->iSect<0) return 0;

  EasyStr Key=KeyVal;
  strupr(Key);
  for (pSK->iKey=Keys.NumItems-1;pSK->iKey>=0;pSK->iKey--){
    if (Keys[pSK->iKey].iSect==pSK->iSect) if (strcmp(Keys[pSK->iKey].szNameUpr,Key)==0) break;
  }
  return (pSK->iKey>=0);
}
//---------------------------------------------------------------------------
int ConfigStoreFile::GetInt(char *Sect,char *Key,int DefVal)
{
  CSF_LOG(EasyStr("INI: GetInt(")+Sect+","+Key+")");

  CSF_FIND sk;
  if (FindKey(Sect,Key,&sk)) return atoi(Keys[sk.iKey].szValue);
  return DefVal;
}
//---------------------------------------------------------------------------
EasyStr ConfigStoreFile::GetStr(char *Sect,char *Key,char *DefVal)
{
  CSF_LOG(EasyStr("INI: GetStr(")+Sect+","+Key+")");

  CSF_FIND sk;
  if (FindKey(Sect,Key,&sk)) return Keys[sk.iKey].szValue;
  return DefVal;
}
//---------------------------------------------------------------------------
void ConfigStoreFile::SetStr(char *Sect,char *Key,char *Val)
{
  CSF_LOG(EasyStr("INI: SetStr(")+Sect+","+Key+")");
  CSF_FIND sk;
  if (FindKey(Sect,Key,&sk)){
    if (strcmp(Keys[sk.iKey].szValue,Val)){ // If value is different
      char *NewStr=new char[strlen(Val)+1];
      szNewMem.Add(NewStr);
      Keys[sk.iKey].szValue=NewStr;
      strcpy(NewStr,Val);
      Changed=true;
    }
  }else{
    char *Buf,*pBuf;
    int SectLen=0,KeyLen=strlen(Key)+1,ValLen=strlen(Val)+1;
    if (sk.iSect<0) SectLen=strlen(Sect)+1;
    Buf=new char[SectLen*2 + KeyLen*2 + ValLen];pBuf=Buf;
    if (sk.iSect<0){
      sk.iSect=Sects.NumItems;
      CSF_SECTS s;
      s.szName=pBuf;    pBuf+=SectLen;
      s.szNameUpr=pBuf; pBuf+=SectLen;

      strcpy(s.szName,Sect);
      strcpy(s.szNameUpr,Sect);strupr(s.szNameUpr);

      CSF_LOG(EasyStr("INI: Adding new section ")+Sect);
      Sects.Add(s);
    }
    CSF_KEYS k;
    k.szName=pBuf;    pBuf+=KeyLen;
    k.szNameUpr=pBuf; pBuf+=KeyLen;

    strcpy(k.szName,Key);
    strcpy(k.szNameUpr,Key);strupr(k.szNameUpr);
    k.iSect=sk.iSect;
    k.szValue=pBuf;
    strcpy(k.szValue,Val);

    szNewMem.Add(Buf);

    CSF_LOG(EasyStr("INI: Adding new key ")+Key+"="+Val);
    Keys.Add(k);
    Changed=true;
  }
}
//---------------------------------------------------------------------------
void ConfigStoreFile::GetSectionNameList(EasyStringList *pESL)
{
  for (int i=0;i<Sects.NumItems;i++) pESL->Add(Sects[i].szName);
}
//---------------------------------------------------------------------------
// Global functions to replace WriteP... and GetP...
//---------------------------------------------------------------------------
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
int GetCSFInt(char *Sect,char *Key,int DefVal,char *File)
{
  ConfigStoreFile CSF(File);
  return CSF.GetInt(Sect,Key,DefVal);
}
//---------------------------------------------------------------------------
#endif

