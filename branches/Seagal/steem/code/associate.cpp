/*---------------------------------------------------------------------------
FILE: associate.cpp
MODULE: Steem
DESCRIPTION: The code to perform the sometimes confusing task of associating
Steem with required file types.
---------------------------------------------------------------------------*/

#ifdef WIN32
LONG RegCopyKey(HKEY FromKeyParent,char *FromKeyName,HKEY ToKeyParent,char *ToKeyName)
{
  LONG Ret;
  HKEY FromKey,ToKey;

  if ( (Ret=RegOpenKey(FromKeyParent,FromKeyName,&FromKey))!=ERROR_SUCCESS ) return Ret;
  if ( (Ret=RegCreateKey(ToKeyParent,ToKeyName,&ToKey))!=ERROR_SUCCESS ){
    RegCloseKey(FromKey);
    return Ret;
  }

  int i=0;
  EasyStr Name,Data;
  DWORD NameSize,DataSize,Type;
  Name.SetLength(8192);
  Data.SetLength(8192);
  for (;;){
    NameSize=8192;DataSize=8192;
    if (RegEnumValue(FromKey,i++,Name,&NameSize,NULL,&Type,LPBYTE(Data.Text),&DataSize)!=ERROR_SUCCESS) break;
    RegSetValueEx(ToKey,Name,0,Type,LPBYTE(Data.Text),DataSize);
  }

  i=0;
  EasyStr Class;
  DWORD ClassSize;
  HKEY NewKey;
  FILETIME Time;
  Class.SetLength(8192);
  for (;;){
    NameSize=8192;ClassSize=8192;
    if (RegEnumKeyEx(FromKey,i++,Name,&NameSize,NULL,Class,&ClassSize,&Time)!=ERROR_SUCCESS) break;
    if (RegCreateKeyEx(ToKey,Name,0,Class,0,KEY_ALL_ACCESS,NULL,&NewKey,&Type)==ERROR_SUCCESS){
      RegCloseKey(NewKey);
      RegCopyKey(FromKey,Name,ToKey,Name);
    }
  }

  RegCloseKey(FromKey);
  RegCloseKey(ToKey);

  return Ret;
}
#endif
//---------------------------------------------------------------------------
bool IsSteemAssociated(EasyStr Exts)
{
#ifdef WIN32
  if (Exts[0]!='.') Exts.Insert(".",0);

  HKEY Key;
  EasyStr KeyName;
  long Size;

  if (RegOpenKey(HKEY_CLASSES_ROOT,Exts,&Key)==ERROR_SUCCESS){
    Size=400;
    KeyName.SetLength(Size);
    RegQueryValue(Key,NULL,KeyName.Text,&Size);
    RegCloseKey(Key);
  }

  if (KeyName.Empty()) KeyName=Exts;

  if (RegOpenKey(HKEY_CLASSES_ROOT,KeyName+"\\Shell",&Key)==ERROR_SUCCESS){
    Size=400;
    EasyStr Default;
    Default.SetLength(Size);
    RegQueryValue(Key,NULL,Default.Text,&Size);
    RegCloseKey(Key);
    if (Default=="OpenSteem"){
      if (RegOpenKey(HKEY_CLASSES_ROOT,KeyName+"\\Shell\\OpenSteem\\Command",&Key)==ERROR_SUCCESS){
        Size=400;
        EasyStr RegCommand;
        RegCommand.SetLength(Size);
        RegQueryValue(Key,NULL,RegCommand.Text,&Size);
        RegCloseKey(Key);

        EasyStr ThisExeName=GetEXEFileName();
        EasyStr Command=char('"');
        Command.SetLength(MAX_PATH+5);
        GetLongPathName(ThisExeName,Command.Text+1,MAX_PATH);
        Command+=EasyStr(char('"'))+" \"%1\"";

        if (IsSameStr_I(Command,RegCommand)) return true;
      }
    }
  }
#elif defined(UNIX)
#endif
  return 0;
}
//---------------------------------------------------------------------------
void AssociateSteem(EasyStr Exts,EasyStr FileClass,bool Force,char *TypeDisplayName,int IconNum,bool IconOnly)
{
#ifdef WIN32
  if (Exts[0]!='.') Exts.Insert(".",0);

  HKEY Key;
  EasyStr KeyName,OriginalKeyName;
  long Size=400;
  bool OriginalForce=Force;

  if (RegOpenKey(HKEY_CLASSES_ROOT,Exts,&Key)==ERROR_SUCCESS){
    Size=400;
    KeyName.SetLength(Size);
    RegQueryValue(Key,NULL,KeyName.Text,&Size);
    RegCloseKey(Key);

    OriginalKeyName=KeyName;
  }else{
    Force=true;
  }

  // If pointing at .st and only Steem on run list move to better FileClass key
  bool IgnoreExts=true;
  if (KeyName.Empty()){
    if (RegOpenKey(HKEY_CLASSES_ROOT,Exts+"\\Shell",&Key)==ERROR_SUCCESS){
      KeyName.SetLength(500);

      int n=0;
      while (RegEnumKey(Key,n++,KeyName.Text,500)==ERROR_SUCCESS){
        if (NotSameStr_I(KeyName,"OpenSteem")){
          IgnoreExts=0;
          KeyName=Exts;
          break;
        }
      }
      RegCloseKey(Key);
    }
    if (IgnoreExts){
      KeyName=FileClass;
      Force=true;
    }
  }

  if (NotSameStr_I(KeyName,FileClass) && NotSameStr_I(KeyName,FileClass+"_"+(Exts.Text+1)) &&
        Force && IgnoreExts){
    // If the file class is not the standard Steem class then it could be used for multiple
    // extensions. To avoid Steem associating itself with all those files make copy of
    // current key and associate in new key.
    EasyStr NewKey=FileClass+"_"+(Exts.Text+1);
    RegCopyKey(HKEY_CLASSES_ROOT,KeyName,HKEY_CLASSES_ROOT,NewKey);
    KeyName=NewKey;
  }

  if (RegOpenKey(HKEY_CLASSES_ROOT,KeyName,&Key)!=ERROR_SUCCESS){
    Force=true; // File class key doesn't exist, force creation.
  }else{
    RegCloseKey(Key);
  }

  // Leave shell integration well alone
  if (RegOpenKey(HKEY_CLASSES_ROOT,KeyName+"\\ShellEx",&Key)==ERROR_SUCCESS){
    RegCloseKey(Key);
    return;
  }
  if (RegOpenKey(HKEY_CLASSES_ROOT,KeyName+"\\CLSID",&Key)==ERROR_SUCCESS){
    RegCloseKey(Key);
    return;
  }

  // Set .st key default value to the type name we are about to make Steem entry for
  // (might be same as it was)
  if (RegCreateKey(HKEY_CLASSES_ROOT,Exts,&Key)!=ERROR_SUCCESS) return;
  RegSetValueEx(Key,NULL,0,REG_SZ,(BYTE*)KeyName.Text,strlen(KeyName)+1);
  if (NotSameStr_I(OriginalKeyName,KeyName)){
    if (strstr(OriginalKeyName.UpperCase(),FileClass.UpperCase())==NULL){
      // Backup old name, unless it is a Steem class
      RegSetValueEx(Key,"Steem_Backup",0,REG_SZ,(BYTE*)OriginalKeyName.Text,strlen(KeyName)+1);
    }
  }
  RegCloseKey(Key);

  if (Force){
    // Set display name of file type (default value of type class key)
    if (RegCreateKey(HKEY_CLASSES_ROOT,KeyName,&Key)!=ERROR_SUCCESS) return;
    RegSetValueEx(Key,NULL,0,REG_SZ,(BYTE*)TypeDisplayName,strlen(TypeDisplayName)+1);
    RegCloseKey(Key);
  }

  if (IconOnly==0){
    // Add steem to the run list
    if (RegCreateKey(HKEY_CLASSES_ROOT,KeyName+"\\Shell\\OpenSteem",&Key)!=ERROR_SUCCESS) return;
    RegSetValueEx(Key,NULL,0,REG_SZ,(BYTE*)CStrT("Open with Steem"),16);
    RegCloseKey(Key);

    // Get default program to run
    if (RegOpenKey(HKEY_CLASSES_ROOT,KeyName+"\\Shell",&Key)!=ERROR_SUCCESS) return;
    Size=400;
    EasyStr Default;
    Default.SetLength(Size);
    RegQueryValue(Key,NULL,Default.Text,&Size);
    if (Default.Empty()) Default="open";

    HKEY Key2;
    if (RegOpenKey(HKEY_CLASSES_ROOT,KeyName+"\\Shell\\"+Default,&Key2)!=ERROR_SUCCESS){
      // No default, make Steem default
      Force=true;
    }else{
      RegCloseKey(Key2);
    }

    if (Force) RegSetValueEx(Key,NULL,0,REG_SZ,(BYTE*)"OpenSteem",10);
    RegCloseKey(Key);
  }

  // Find out if we should add the Command key with the path of Steem, only
  // if there is no command key, or we are forcing
  bool SetToThisEXE=bool(IconOnly ? 0:true);
  if (Force==0 && IconOnly==0){
    if (RegOpenKey(HKEY_CLASSES_ROOT,KeyName+"\\Shell\\OpenSteem\\Command",&Key)==ERROR_SUCCESS){
      SetToThisEXE=0;
      RegCloseKey(Key);
    }
  }

  EasyStr ThisExeName=GetEXEFileName();
  if (SetToThisEXE){
    // Set command key
    if (RegCreateKey(HKEY_CLASSES_ROOT,KeyName+"\\Shell\\OpenSteem\\Command",&Key)!=ERROR_SUCCESS) return;

    EasyStr Command=EasyStr("\"")+ThisExeName+"\" \"%1\"";
    RegSetValueEx(Key,NULL,0,REG_SZ,(BYTE*)Command.Text,Command.Length()+1);

    RegCloseKey(Key);
  }

  if (RegCreateKey(HKEY_CLASSES_ROOT,KeyName+"\\DefaultIcon",&Key)!=ERROR_SUCCESS) return;
  EasyStr IconStr=ThisExeName+","+IconNum;
  if (OriginalForce){
    // Only set to Steem icon if the user chooses to associate with Steem
    RegSetValueEx(Key,NULL,0,REG_SZ,(BYTE*)IconStr.Text,IconStr.Length()+1);
  }else{
    // If icon is from this EXE make sure it is using at the right number
    // If no icon then set to Steem's icon (may as well)
    Size=400;
    EasyStr CurIconStr;
    CurIconStr.SetLength(Size);
    RegQueryValue(Key,NULL,CurIconStr.Text,&Size);
    char *comma=strchr(CurIconStr,',');
    if (comma){
      *comma=0;
      if (IsSameStr_I(CurIconStr,ThisExeName)){
        RegSetValueEx(Key,NULL,0,REG_SZ,(BYTE*)IconStr.Text,IconStr.Length()+1);
      }
    }else if (CurIconStr.Empty()){
      RegSetValueEx(Key,NULL,0,REG_SZ,(BYTE*)IconStr.Text,IconStr.Length()+1);
    }
  }
  RegCloseKey(Key);
#elif defined(UNIX)
#endif
}
//---------------------------------------------------------------------------
