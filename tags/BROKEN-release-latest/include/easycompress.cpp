#ifndef EASYCOMPRESS_H
#define EASYCOMPRESS_H
//---------------------------------------------------------------------------
#define MIN_PACK_LENGTH 4  //If we have 4 words the same in a row then pack them
//---------------------------------------------------------------------------
void EasyCompressFromMem(void *Buf,long Len,FILE *f)
{
  WORD *Mem=(WORD*)Buf,*SearchAdr,*MemEnd=(WORD*)(long(Buf)+Len);
  WORD ChangedLen,SameLen;
  WORD Last,This;

  WORD Version=0;
  fwrite(&Version,1,2,f);

  while (Mem<MemEnd){
    Last=0;
    ChangedLen=0;
    SameLen=0;
    SearchAdr=Mem;
    for(;;){
      if (SearchAdr>=MemEnd){
        if (SameLen){
          ChangedLen+=SameLen; //They weren't the same for long enough
          SameLen=0;
        }
        break;
      }
      This=*SearchAdr;
      if (This==Last){
        SameLen++;
        if (SameLen>=MIN_PACK_LENGTH) break;
      }else{
        if (SameLen){
          ChangedLen+=SameLen;  //They weren't the same for long enough
          SameLen=0;
        }
        ChangedLen++;
        if (ChangedLen>=16000) break;
      }

      Last=This;
      SearchAdr++;
    }
    if (ChangedLen){
      fwrite(&ChangedLen,1,2,f);
      fwrite(Mem,1,ChangedLen*2,f);
      Mem+=ChangedLen;
    }
    if (SameLen){
      SearchAdr=Mem+SameLen;  //SearchAdr is just after the last character searched
      while (*SearchAdr==This && SameLen<16000 && SearchAdr<MemEnd){
        SameLen++;
        SearchAdr++;
      }
      Mem+=SameLen;

      SameLen|=0x8000; //Set high bit
      fwrite(&SameLen,1,2,f);
      fwrite(&This,1,2,f);
    }
  }
  SameLen=0xffff;
  fwrite(&SameLen,1,2,f);
}
//---------------------------------------------------------------------------
#define EASYCOMPRESS_BUFFERTOSMALL 1
#define EASYCOMPRESS_CORRUPTFILE 2
//---------------------------------------------------------------------------
int EasyUncompressToMem(void *Buf,int Len,FILE* &f,bool FIsMem)
{
  WORD *Mem=(WORD*)Buf,*MemEnd=(WORD*)(long(Buf)+Len),Desc,NumWords;
  WORD *p=(WORD*)f;

  WORD Version=0xffff;
  if (FIsMem==0) fread(&Version,1,2,f);
  if (FIsMem) Version=*(p++);
  if (Version!=0) return EASYCOMPRESS_CORRUPTFILE;

  for(;;){
    if (FIsMem==0){
      if (fread(&Desc,1,2,f)<2) return EASYCOMPRESS_CORRUPTFILE;
    }
    if (FIsMem) Desc=*(p++);
    if (Desc==0xffff) break;

    if (Desc & 0x8000){
      NumWords=Desc & (WORD)0x7fff;
      if (Mem+NumWords > MemEnd) return EASYCOMPRESS_BUFFERTOSMALL;

      if (FIsMem==0) fread(&Desc,1,2,f);
      if (FIsMem) Desc=*(p++);
      for (WORD n=0;n<NumWords;n++) *(Mem++)=Desc;
    }else{
      NumWords=Desc;
      if (Mem+NumWords > MemEnd) return EASYCOMPRESS_BUFFERTOSMALL;

      if (FIsMem==0){
        fread(Mem,1,NumWords*2,f);
        Mem+=NumWords;
      }else{
        for (int n=0;n<NumWords;n++) *(Mem++)=*(p++);
      }
    }
  }
  if (FIsMem) f=(FILE*)p;
  return 0;
}
//---------------------------------------------------------------------------
int EasyUncompressToMemFromMem(void *Buf,int Len,BYTE* &pByte)
{
  FILE *f=(FILE*)pByte;
  int Ret=EasyUncompressToMem(Buf,Len,f,true);
  pByte=(BYTE*)f;
  return Ret;
}
//---------------------------------------------------------------------------
#endif

