/*---------------------------------------------------------------------------
FILE: di_get_contents.cpp
MODULE: Steem
DESCRIPTION: This is the code for the disk image recognition system that uses
the TOSEC database. It is written as a library to enable it to be used in
other emulators at a later date.
---------------------------------------------------------------------------*/

char GetContents_ListFile[512]={0};

typedef void GETZIPCRCSPROC(char*,DWORD*,int);
typedef BYTE* CONVERTTOSTPROC(char*,int,int*);

GETZIPCRCSPROC *GetContents_GetZipCRCsProc=NULL;
CONVERTTOSTPROC *GetContents_ConvertToSTProc=NULL;
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
DWORD GetContents_GetCRCFromMemory(BYTE *mem,int len)
{
  DWORD *lpCRCTable;
  DWORD CRC;

  lpCRCTable=(DWORD*)malloc(sizeof(DWORD)*256);
  GetContents_InitCRC32Table(lpCRCTable);
  CRC=GetContents_GetCRC(mem,len,lpCRCTable);
  free(lpCRCTable);
  return CRC;
}
//---------------------------------------------------------------------------
DWORD GetContents_GetCRCFromFile(char *Fil)
{
  FILE *f;
  int Len;
  BYTE *Block;
  DWORD CRC;

  f=fopen(Fil,"rb");
  if (f==NULL) return 0;

  fseek(f,0,SEEK_END);
  Len=ftell(f);

  fseek(f,0,SEEK_SET);
  Block=(BYTE*)malloc(Len);
  fread(Block,1,Len,f);
  CRC=GetContents_GetCRCFromMemory(Block,Len);
  free(Block);

  fclose(f);
  return CRC;
}
//---------------------------------------------------------------------------
char *GetContents_GetNameAndContent(char *t,char *Text,int Len,char **pName,char **pContent)
{
  char *line;
  // Find the start of the line
  while (t > Text){
    if (*t=='\r' || *t=='\n' || *t==0){
      t++;
      break;
    }
    t--;
  }
  line=t;
  while (t < Text+Len){
    if (*t=='\r' || *t=='\n' || *t==0){
      *(t++)=0;
      break;
    }
    t++;
  }
  
  *pContent=strchr(line+1,'\"')+2;
  if ((*pContent)[0]=='\"'){
    (*pContent)++;
    *strchr((*pContent),'\"')=0;
  }else{
    (*pContent)[0]=0;
  }

  (*pName)=line+1;
  *strchr((*pName),'\"')=0;
  return t;
}
//---------------------------------------------------------------------------
// Format of string: "TOSEC Name","Content","CRC1","CRC2",...,"CRCn"
//---------------------------------------------------------------------------
// GetContentsFromDiskImage - get the name and a contents list of a disk image
//      Fil - Full path of disk image, can be any format including an archive
//      szRetBuf - Buffer in which to return NULL-terminated list of contents
//      iRetBufLen - Number of bytes in the buffer
//      OnAmbiguity - What to do if there is more than one match
//---------------------------------------------------------------------------
DWORD GetContentsFromDiskImage(char *Fil,char *szRetBuf,int iRetBufLen,int /*OnAmbiguity*/)

{
  char szCRC[16];
  int nFound=0;
  DWORD dwCRCs[30];
  char ext[16];
  char *pRet=szRetBuf;
  char *pRetEnd=szRetBuf+iRetBufLen-1; // -1 for double NULL

  memset(szRetBuf,0,iRetBufLen);

  { // Get CRC
    char *dot;

    memset(dwCRCs,0,sizeof(dwCRCs));
    dot=strrchr(Fil,'.');
    if (dot){
      strcpy(ext,dot+1);
      if (strcmpi(ext,"zip")==0){
        // Extract CRC from zip file (quicker than decompressing)
        if (GetContents_GetZipCRCsProc){
          GetContents_GetZipCRCsProc(Fil,dwCRCs,30);
        }
        if (dwCRCs[0]==0) return GC_CANTFINDCONTENTS;
      }else{
        dwCRCs[0]=GetContents_GetCRCFromFile(Fil);
      }
    }
  }

  {
    FILE *f;
    f=fopen(GetContents_ListFile,"rb");
    if (f){
      char *Text,*t,*name,*content;
      int Len;

      fseek(f,0,SEEK_END);
      Len=ftell(f);
      fseek(f,0,SEEK_SET);

      Text=(char*)malloc(Len+1);
      fread(Text,1,Len,f);
      fclose(f);
      Text[Len]=0;

      for (int test=0;test<2;test++){ // Check list for match
        for (int i=0;i<30;i++){
          if (dwCRCs[i]==0) break;

          sprintf(szCRC,"%8.8X",(unsigned int)(dwCRCs[i]));

          t=Text;
          for(;;){
            t=strstr(t,szCRC);
            if (t==NULL) break;

            t=GetContents_GetNameAndContent(t,Text,Len,&name,&content);
            if (nFound==0){
              if (pRet+strlen(name)>=pRetEnd){ nFound=GC_TOOSMALL;break; }
              strcpy(pRet,name);
              pRet+=strlen(pRet)+1;
              nFound++;
              if (content[0]){
                if (strcmpi(name,content)!=0){ // not the same
                  if (pRet+strlen(content)>=pRetEnd){ nFound=GC_TOOSMALL;break; }
                  strcpy(pRet,content);
                  pRet+=strlen(pRet)+1;
                  nFound++;
                }
              }
            }else if (content[0]){
              if (strcmpi(name,szRetBuf)==0){ // same disk
                if (pRet+strlen(content)>=pRetEnd){ nFound=GC_TOOSMALL;break; }
                strcpy(pRet,content);
                pRet+=strlen(pRet)+1;
                nFound++;
              }else{
                // Ambiguity!
                if (pRet+strlen(content)>=pRetEnd){ nFound=GC_TOOSMALL;break; }
                strcpy(pRet,content);
                pRet+=strlen(pRet)+1;
                nFound++;
              }
            }
          }
        }
        if (nFound || GetContents_ConvertToSTProc==NULL || test>0) break;
        if (strcmpi(ext,"st")==0) break;

        // Not found, convert disk to ST format and get CRC
        {
          BYTE *mem;
          int len,i=0;

          memset(dwCRCs,0,sizeof(dwCRCs));
          for (int n=0;n<30;n++){
            mem=GetContents_ConvertToSTProc(Fil,n,&len);
            if (mem){
              dwCRCs[i++]=GetContents_GetCRCFromMemory(mem,len);
              free(mem);
            }else if (len==-1){
              break;
            }
          }
        }
      }
      free(Text);
    }
  }
  if (nFound==0) return GC_CANTFINDCONTENTS;

  return nFound;
}
//---------------------------------------------------------------------------
void GetContents_SearchDatabase(char *szFind,char *szRetBuf,int iRetBufLen)
{
  char *pRet=szRetBuf;
  char *pRetEnd=szRetBuf+iRetBufLen-1; // -2 for treble NULL!

  memset(szRetBuf,0,iRetBufLen);

  FILE *f;
  f=fopen(GetContents_ListFile,"rb");
  if (f){
    char *Text,*TextUpr,*FindUpr,*pTextUpr,*name,*content,LastName[200]={0};
    int Len;

    fseek(f,0,SEEK_END);
    Len=ftell(f);
    fseek(f,0,SEEK_SET);

    Text=(char*)malloc(Len+1);
    fread(Text,1,Len,f);
    fclose(f);
    Text[Len]=0;

    TextUpr=(char*)malloc(Len+1);
    strcpy(TextUpr,Text);
    strupr(TextUpr);

    FindUpr=(char*)malloc(strlen(szFind)+1);
    strcpy(FindUpr,szFind);
    strupr(FindUpr);

    pTextUpr=TextUpr;
    for(;;){
      pTextUpr=strstr(pTextUpr,FindUpr);
      if (pTextUpr==NULL) break;

      char *pNextLineUpr=(GetContents_GetNameAndContent(Text+(pTextUpr-TextUpr),Text,Len,&name,&content)-Text)+TextUpr;
      if (pTextUpr-TextUpr <= content+strlen(content)-Text){ // Not CRC!  
        if (strcmpi(name,LastName)){ // Different name
          if (LastName[0]) pRet++; // Terminate list for last name
          if (pRet+strlen(name)>=pRetEnd) break;
          strcpy(pRet,name); pRet+=strlen(pRet)+1;
        }
        strcpy(LastName,name);
        if (content[0]){
          if (pRet+strlen(content)>=pRetEnd) break;
          strcpy(pRet,content);
          pRet+=strlen(pRet)+1;
        }
      }
      pTextUpr=pNextLineUpr;
    }
    free(Text);
    free(TextUpr);
    free(FindUpr);
  }
}
//---------------------------------------------------------------------------

