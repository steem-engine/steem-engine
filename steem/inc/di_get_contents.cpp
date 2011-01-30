//---------------------------------------------------------------------------
DWORD GetContents_Reflect(DWORD ref,char ch)
{
  DWORD value=0;
  int i;

  // Swap bit 0 for bit 7 bit 1 for bit 6, etc.
  for (i=1;i<(ch+1);i++){
    if (ref & 1) value |= 1 << (ch - i);
    ref >>= 1;
  }
  return value;
}
//---------------------------------------------------------------------------
void GetContents_InitCRC32Table(DWORD *lpTable)
{
  // This is the official polynomial used by CRC-32
  // in PKZip, WinZip and Ethernet.
  DWORD Polynomial=0x04c11db7;
  int i,j;

  // 256 values representing ASCII character codes.
  for (i=0;i<=0xFF;i++){
    lpTable[i]=GetContents_Reflect(i,8) << 24;
    for (j=0;j<8;j++) lpTable[i]=(lpTable[i] << 1)^((lpTable[i] & (1 << 31)) ? Polynomial:0);
    lpTable[i]=GetContents_Reflect(lpTable[i],32);
  }
}
//---------------------------------------------------------------------------
int GetContents_GetCRC(BYTE *Block,int BlockLen,DWORD *lpTable)
{
  // Once the lookup table has been filled in by the two functions above,
  // this function creates all CRCs using only the lookup table.

  // Be sure to use unsigned variables,
  // because negative values introduce high bits
  // where zero bits are required.

  // Start out with all bits set high.
  DWORD CRC=0xffffffff;

  // Perform the algorithm on each byte in the block using the lookup table values.
  while (BlockLen--) CRC=(CRC >> 8) ^ lpTable[(CRC & 0xFF) ^ *(Block++)];

  // Exclusive OR the result with the beginning value.
  return CRC ^ 0xffffffff;
}
//---------------------------------------------------------------------------
DWORD GetContents_GetCRCFromImage(char *Fil)
{
  FILE *f;
  int Len;
  DWORD CRC=0;

  f=fopen(Fil,"rb");
  if (f==NULL) return 0;

  fseek(f,0,SEEK_END);
  Len=ftell(f);
  // We skip 51200 bytes at each end of the disk image because this is the area
  // most prone to change, either by high score saving or bootsector customising
  if (Len>51200*2){
    BYTE *Block;
    DWORD *lpCRCTable;

    fseek(f,51200,SEEK_SET);
    Len-=51200*2;
    Len=min(Len,10240); // Get the CRC of this 10 Kb
    Block=(BYTE*)malloc(Len);
    fread(Block,1,Len,f);

    lpCRCTable=(DWORD*)malloc(sizeof(DWORD)*256);
    GetContents_InitCRC32Table(lpCRCTable);
    CRC=GetContents_GetCRC(Block,Len,lpCRCTable);
    free(lpCRCTable);

    free(Block);
  }

  fclose(f);
  return CRC;
}
//---------------------------------------------------------------------------
// Format of string: :CRC: FILENAME,Title,Short Title,Content1,Content2 ... ContentN\n
int GetContents_GetListFromString(char *String,char **lpStrList,int nStrs)
{
  char *t1,*t2;
  int i=0;

  if (nStrs<=0) return 0;

  t1=strchr(String,',');
  if (t1==NULL) return 0;
  t1++;
  while (t1[0] && i<nStrs){
    t2=strstr(t1,",");
    if (t2==NULL){
      strcpy(lpStrList[i],t1);i++;
      break;
    }
    *t2=0;
    strcpy(lpStrList[i],t1);i++;
    t1=t2+1;
  }
  return i;
}
//---------------------------------------------------------------------------
// GetContentsFromDiskImage - get the name and a contents list of a disk image
//      Fil - Full path of disk image, can be any format but must not be an archive
//      Name - Filename of disk image without path (NULL to get this from Fil)
//      ContentsListFol - Full path of folder containing contents lists to search
//      lpStrList - Array of strings in which to return the contents
//      nStrs - Number of strings in the list
//      OnAmbiguity - What to do if there is more than one match
//---------------------------------------------------------------------------
DWORD GetContentsFromDiskImage(char *Fil,char *Name1,char *Name2,char *ContentsListFol,
                                char **lpStrList,int nStrs,int /*OnAmbiguity*/)
{
  char szCRC[16];
  char szFileName[2][MAX_PATH];
  char szDiskString[2][10][1024];
  int nDiskStrings[2]={0,0};

  { // Get CRC
    DWORD dwCRC;

    dwCRC=GetContents_GetCRCFromImage(Fil);
    if (dwCRC){
      sprintf(szCRC,":%8.8X:",dwCRC);
    }else{ // Can't get CRC, have to go on filename match
      szCRC[0]=0;
    }
  }

  { // Get Filename (no extension, no path)
    int Len,i;
    char Buf[2][MAX_PATH],*dot;

    if (Name1){
      strcpy(Buf[0],Name1);
    }else{
      strcpy(Buf[0],Fil);
    }
    if (Name2){
      strcpy(Buf[1],Name2);
    }else{
      Buf[1][0]=0;
    }
    for (i=0;i<2;i++){
      for (Len=strlen(Buf[i]);Len>=0;Len--){
        if (Buf[i][Len]=='\\' || Buf[i][Len]=='/'){
          strcpy(szFileName[i],Buf[i]+Len+1);
          break;
        }else if (Len==0){
          strcpy(szFileName[i],Buf[i]);
        }
      }
      strupr(szFileName[i]);
      dot=strrchr(szFileName[i],'.');
      if (dot) *dot=0;
    }
  }

  { // Check lists for match
    char SearchFol[MAX_PATH];
    WIN32_FIND_DATA wfd;
    HANDLE hFind;

    strcpy(SearchFol,ContentsListFol);
    if (SearchFol[0]){
      if (SearchFol[strlen(SearchFol)-1]=='/' || SearchFol[strlen(SearchFol)-1]=='\\'){
        SearchFol[strlen(SearchFol)-1]=0;
      }
    }
    strcat(SearchFol,"\\*.dicl");
    hFind=FindFirstFile(SearchFol,&wfd);
    if (hFind==INVALID_HANDLE_VALUE) return GC_NOLISTS;

    do{
      if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0){
        FILE *f;
        char szFullPath[MAX_PATH];

        strcpy(szFullPath,ContentsListFol);
        if (szFullPath[0]){
          if (szFullPath[strlen(szFullPath)-1]=='/' || szFullPath[strlen(szFullPath)-1]=='\\'){
            szFullPath[strlen(szFullPath)-1]=0;
          }
        }
        strcat(szFullPath,"\\");
        strcat(szFullPath,wfd.cFileName);
        f=fopen(szFullPath,"rb");
        if (f){
          char *Text,*t,*line,*szSearch;
          int i,SearchType;

          Text=(char*)malloc(wfd.nFileSizeLow+1);
          fread(Text,1,wfd.nFileSizeLow,f);
          fclose(f);
          Text[wfd.nFileSizeLow]=0;

          for (i=0;i<3;i++){
            SearchType=(i==2); // 0 for filename, 1 for CRC
            szSearch=(char*)((SearchType==1) ? szCRC:szFileName[i]);
            if (szSearch[0]!=0){
              t=Text;
              for (;;){
                t=strstr(t,szSearch);
                if (t==NULL) break;

                // Find the start of the line
                while (t > Text){
                  if (*t=='\r' || *t=='\n' || *t==0){
                    t++;
                    break;
                  }
                  t--;
                }
                line=t;
                while (t < Text+wfd.nFileSizeLow){
                  if (*t=='\r' || *t=='\n' || *t==0){
                    *(t++)=0;
                    break;
                  }
                  t++;
                }
                if (nDiskStrings[SearchType]<10){
                  strcpy(szDiskString[SearchType][nDiskStrings[SearchType]++],line);
                }
              }
            }
          }
          free(Text);
        }
      }
    }while (FindNextFile(hFind,&wfd));
    FindClose(hFind);
  }

  if (nDiskStrings[1]){ // CRC matches, more reliable than file names
    if (nDiskStrings[1]==1){ // Only one match, great!
      return GetContents_GetListFromString(szDiskString[1][0],lpStrList,nStrs);
    }else{
      // See if only one of them has a filename match too

      // If not then ambiguity time!
      return GetContents_GetListFromString(szDiskString[1][0],lpStrList,nStrs);
    }
  }else if (nDiskStrings[0]){
    if (nDiskStrings[0]==1){ // Only one match, great!
      return GetContents_GetListFromString(szDiskString[0][0],lpStrList,nStrs);
    }else{
      // Ambiguity time!
      return GetContents_GetListFromString(szDiskString[0][0],lpStrList,nStrs);
    }
  }
  return GC_CANTFINDCONTENTS;
}

