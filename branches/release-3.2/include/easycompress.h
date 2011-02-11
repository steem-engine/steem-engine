extern void EasyCompressFromMem(void *,long,FILE *);
//---------------------------------------------------------------------------
#define EASYCOMPRESS_BUFFERTOSMALL 1
#define EASYCOMPRESS_CORRUPTFILE 2
//---------------------------------------------------------------------------
extern int EasyUncompressToMem(void *,int,FILE *&,bool=0);
extern int EasyUncompressToMemFromMem(void *,int,BYTE *&);

