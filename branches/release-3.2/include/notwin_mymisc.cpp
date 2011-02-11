/*
EasyStr CachedPPFile;
EasyStr CachedPPText;
//---------------------------------------------------------------------------
bool IsCachedPrivateProfile() { return CachedPPFile.NotEmpty(); }
//---------------------------------------------------------------------------
void UnCachePrivateProfile()
{
  FILE *f=fopen(CachedPPFile,"wb");
  if (f){
    fwrite(CachedPPText.Text,CachedPPText.Length(),1,f);
    fclose(f);
    CachedPPFile="";
  }
}
//---------------------------------------------------------------------------
bool CachePrivateProfile(char *FileName)
{
  FILE *f=fopen(FileName,"rb");
  if (f==NULL) return 0;

  UnCachePrivateProfile();
  int Len=GetFileLength(f);
  CachedPPText.SetLength(Len);
  fread(CachedPPText.Text,Len,1,f);
  fclose(f);
  CachedPPFile=FileName;
  return true;
}
//---------------------------------------------------------------------------
#define RET "\n"

EasyStr GetPPEasyStr(char *SectionName,char *KeyName,char *Default,char *FileName)
{
  EasyStr Sect=(EasyStr("[")+SectionName+"]").UpperCase();
  EasyStr Key=EasyStr(KeyName).UpperCase()+"=";

  bool FromCache=(strcmpi(CachedPPFile,FileName)==0);
  int Len;
  EasyStr FileText;
  if (FromCache==0){
    FILE *f=fopen(FileName,"rb");
    if (f==NULL) return Default;

    Len=GetFileLength(f);
    FileText.SetLength(Len);
    fread(FileText.Text,Len,1,f);
    fclose(f);
  }else{
    Len=CachedPPText.Length();
  }
  EasyStr &Text=(EasyStr&)(FromCache ? CachedPPText:FileText);

  bool FoundSect=0;
  int Pos=0,RelPos,StartOfLine;
  char Line[1001];
  for(;;){
    Line[0]=0;
    RelPos=0;
    StartOfLine=Pos;
    do{
      if (Pos>=Len) break;
      if (Text[Pos]=='\n' || Text[Pos]=='\r'){
        while (Text[Pos]=='\n' || Text[Pos]=='\r' && Pos<Len) Pos++;
        break;
      }
      Line[RelPos++]=Text[Pos++];
    }while (RelPos<1000);
    if (Pos>=Len && RelPos==0) break;
    Line[RelPos]=0;
    strupr(Line);

    if (FoundSect==0){
      if (strstr(Line,Sect)==Line) FoundSect=true;
    }else{
      if (strstr(Line,Key)==Line){
        int ValStart=StartOfLine+Key.Length(),ValEnd=ValStart;
        while (Text[ValEnd]!='\n' && Text[ValEnd]!='\r' && ValEnd<Len) ValEnd++;
        char SaveChar=Text[ValEnd];
        Text[ValEnd]=0;
        EasyStr Ret=Text.Text+ValStart;
        Text[ValEnd]=SaveChar;
        return Ret;
      }else if (Line[0]=='[' && Line[strlen(Line)-1]==']'){
        break;
      }
    }
  }
  return Default;
}
//---------------------------------------------------------------------------
UINT GetPrivateProfileInt(char *Sect,char *Key,int Default,char *File)
{
  return (UINT)(atoi(GetPPEasyStr(Sect,Key,EasyStr(Default),File)));
}
//---------------------------------------------------------------------------
bool WritePrivateProfileString(char *SectionName,char *KeyName,const char *String,char *FileName)
{
  EasyStr Sect=(EasyStr("[")+SectionName+"]").UpperCase();
  EasyStr Key=EasyStr(KeyName).UpperCase()+"=";

  bool FromCache=(strcmpi(CachedPPFile,FileName)==0);
  int Len;
  EasyStr FileText;
  if (FromCache==0){
    FILE *f=fopen(FileName,"rb");
    if (f){
      Len=GetFileLength(f);
      FileText.SetLength(Len);
      fread(FileText.Text,Len,1,f);
      fclose(f);
    }
  }
  EasyStr &Text=(EasyStr&)(FromCache ? CachedPPText:FileText);
  Len=Text.Length();

  bool FoundSect=0;
  int Pos=0,RelPos,StartOfLine;
  char Line[1001];
  for(;;){
    Line[0]=0;
    RelPos=0;
    StartOfLine=Pos;
    do{
      if (Pos>=Len) break;
      if (Text[Pos]=='\n' || Text[Pos]=='\r'){
        while (Text[Pos]=='\n' || Text[Pos]=='\r' && Pos<Len) Pos++;
        break;
      }
      Line[RelPos++]=Text[Pos++];
    }while (RelPos<1000);
    if (Pos>=Len && RelPos==0) break;
    Line[RelPos]=0;
    strupr(Line);

    if (FoundSect==0){
      if (strstr(Line,Sect)==Line) FoundSect=true;
    }else{
      if (strstr(Line,Key)==Line){
        Text.Delete(StartOfLine,strlen(Line));
        Text.Insert(EasyStr(KeyName)+"="+(char*)String,StartOfLine);
        Pos=-1;
        break;
      }else if (Line[0]=='[' && Line[strlen(Line)-1]==']'){
        StartOfLine--;
        while (Text[StartOfLine]=='\n' || Text[StartOfLine]=='\r') StartOfLine--;
        Text.Insert(EasyStr(RET)+KeyName+"="+(char*)String,StartOfLine+1);
        Pos=-1;
        break;
      }
    }
  }
  if (Pos>=Len){
    StartOfLine--;
    while (StartOfLine>=0){
      if (Text[StartOfLine]!='\n' && Text[StartOfLine]!='\r') break;
      StartOfLine--;
    }
    if (FoundSect==0){
      EasyStr ToWrite=(char*)((StartOfLine<0) ? "":RET RET);
      ToWrite+=EasyStr("[")+SectionName+"]" RET;
      ToWrite+=EasyStr(KeyName)+"="+(char*)String;

      Text.Insert(ToWrite,StartOfLine+1);
    }else{
      Text.Insert(EasyStr(RET)+KeyName+"="+(char*)String,StartOfLine+1);
    }
  }
  if (FromCache==0){
    FILE *f=fopen(FileName,"wb");
    if (f){
      fwrite(Text.Text,Text.Length(),1,f);
      fclose(f);
      return true; //Success!
    }
  }

  return 0;
}
#undef RET
*/
//---------------------------------------------------------------------------
void ZeroMemory(void *Mem,DWORD Len)
{
  memset(Mem,0,Len);
}
//---------------------------------------------------------------------------
bool DeleteFile(char *File)
{
  return unlink(File)==0;
}

//---------------------------------------------------------------------------
UINT GetTempFileName(char *PathName,char *Prefix,UINT Unique,char *TempFileName)
{
  EasyStr Ret;
  WORD Num=WORD(Unique ? WORD(Unique):WORD(rand() & 0xffff));
  for(;;){
    Ret=PathName;
    Ret+=SLASH;
    Ret+=Prefix;
    Ret.SetLength(MAX_PATH+4);
    char *StartOfNum=Ret.Right()+1;
    ultoa(Num,StartOfNum,16);
    strupr(StartOfNum);
    Ret+=".TMP";
    if (Ret.Length()<MAX_PATH){
      if (Unique==0){
        if (access(Ret,0)==0){ //File exists
          Num++;
        }else{
          strcpy(TempFileName,Ret);
          fclose(fopen(TempFileName,"wb"));
          return Num;
        }
      }else{
        strcpy(TempFileName,Ret);
        return Num;
      }
    }else{
      return 0;
    }
  }
}
//---------------------------------------------------------------------------

