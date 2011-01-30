#ifdef DEBUG_FUNCTIONS

#include <malloc.h>

void SaveHeapLog()
{
  FILE *f;
  _HEAPINFO hi;
  char block[129],*ad;
  block[128]=0;
  int code;
  HANDLE heaps[200];
  DWORD n;
  PROCESS_HEAP_ENTRY phe;

  f=fopen(WriteDir+SLASH+"heap.log","wb");

  SetLastError(0);
  n=GetProcessHeaps(200,heaps);
  heaps[n]=GetProcessHeap();n++;
  fprintf(f,"There are %i heaps for Steem\r\n",n);
  for (DWORD i=0;i<n;i++){
    code=HeapValidate(heaps[i],0,NULL);
    if (code){
      fprintf(f,"Heap %u okay\r\n",i);
    }else{
      fprintf(f,"Heap %u has an error in it!\r\n",i);
    }
    HeapLock(heaps[i]);
    phe.lpData=NULL;
    while (HeapWalk(heaps[i],&phe)){
      fprintf(f,"%s%X\r\n","Bad node, address=",(unsigned long)(phe.lpData));
      ad=((char*)phe.lpData)-64;
      for (int n=0;n<128;n++){
        block[n]=*(ad++);
        if (block[n]==0) block[n]='\\';
        if (block[n]==10) block[n]='\\';
        if (block[n]==13) block[n]='\\';
      }
      fprintf(f,"%s\r\n",block);
    }
    HeapUnlock(heaps[i]);
    if (phe.lpData==NULL) DisplayLastError();
  }

  fprintf(f,"\r\n\r\n");
  code=_heapchk();
  if (code==_HEAPOK){
    fprintf(f,"%s\r\n","Heap okay, walking:");
  }else if (code==_HEAPBADNODE){
    fprintf(f,"%s\r\n","Heap has bad node! Walking anyway:");
  }
  hi._pentry=NULL;
  for(;;){
    code=_rtl_heapwalk(&hi);
    if (code==_HEAPEND) break;
    if (code==_HEAPBADNODE){
      fprintf(f,"%s%X\r\n","Bad node, address=",(unsigned long)(hi.__pentry));
      ad=((char*)hi._pentry)-64;
      for (int n=0;n<128;n++){
        block[n]=*(ad++);
        if (block[n]==0) block[n]='\\';
        if (block[n]==10) block[n]='\\';
        if (block[n]==13) block[n]='\\';
      }
      fprintf(f,"%s\r\n",block);
    }else if (code==_HEAPOK){
      fprintf(f,"%s%X\r\n","Good node, address=",(unsigned long)(hi.__pentry));
    }
  }
  fclose(f);
}
#else
#define SaveHeapLog()
#endif
