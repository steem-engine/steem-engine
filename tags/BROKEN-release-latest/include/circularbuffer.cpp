/*---------------------------------------------------------------------------
FILE: circularbuffer.cpp
MODULE: helper
DESCRIPTION: Class to implement a circular FIFO buffer.
---------------------------------------------------------------------------*/

#ifndef CIRCULARBUFFER_CPP
#define CIRCULARBUFFER_CPP

#include "circularbuffer.h"
//---------------------------------------------------------------------------
CircularBuffer::CircularBuffer(DWORD Size)
{
  Buf=NULL;
  Lock=0;

  if (Size>=2) Create(Size);
}
//---------------------------------------------------------------------------
CircularBuffer::~CircularBuffer() { Destroy(); }
//---------------------------------------------------------------------------
bool CircularBuffer::AreBytesInBuffer()
{
  return bool(Buf ? (pCurRead!=pCurWrite-1 && !(pCurRead==pEnd-1 && pCurWrite==pStart)):0);
}
//---------------------------------------------------------------------------
BYTE CircularBuffer::ReadByte() { return BYTE(Buf ? *pCurRead:BYTE(0)); }
//---------------------------------------------------------------------------
#ifndef WIN32
// On Windows Sleep(0) will let another thread take over instantly
int CircularBuffer::Sleep(int n) { return n; }
#endif
//---------------------------------------------------------------------------
bool CircularBuffer::IsLocked(){ return Lock; }
//---------------------------------------------------------------------------
bool CircularBuffer::Create(DWORD Size)
{
  if (Buf || Size<2) return 0;

  try{
    Buf=new BYTE[Size];
  }catch(...){
    return 0;
  }
  BufSize=Size;

  pStart=Buf;
  pEnd=Buf+BufSize;
  Reset();

  return true;
}
//---------------------------------------------------------------------------
bool CircularBuffer::AddByte(BYTE Data)
{
  if (Buf==NULL) return 0;

  while (Lock) Sleep(0);
  Lock=true;

  bool Overflow=(pCurRead==pCurWrite);
  *(pCurWrite++)=Data;
  if (pCurWrite>=pEnd) pCurWrite=pStart;
  if (Overflow) pCurRead=pCurWrite;

  Lock=0;
  return Overflow==0;
}
//---------------------------------------------------------------------------
bool CircularBuffer::AddBytes(BYTE *pData,DWORD DataLen)
{
  if (Buf==NULL || DataLen>=BufSize) return 0;

  while (Lock) Sleep(0);
  Lock=true;

  bool Overflow=0;
  BYTE *pOldWrite=pCurWrite;
  if (pCurWrite+DataLen<pEnd){
    pCurWrite+=DataLen;
    if (pCurRead>=pOldWrite && pCurRead<pCurWrite){
      pCurRead=pCurWrite;
      Overflow=true;
    }
    Lock=0;

    memcpy(pOldWrite,pData,DataLen);
  }else{
    bool Overlap=(pCurRead>=pCurWrite);
    int ToEnd = pEnd-pCurWrite;
    pCurWrite=pStart+(DataLen-ToEnd);
    if (pCurRead<pCurWrite || Overlap){
      pCurRead=pCurWrite;
      Overflow=true;
    }
    Lock=0;

    memcpy(pOldWrite,pData,ToEnd);
    memcpy(pStart,pData+ToEnd,DataLen-ToEnd);
  }
  return Overflow==0;
}
//---------------------------------------------------------------------------
void CircularBuffer::NextByte()
{
  while (Lock) Sleep(0);
  if (AreBytesInBuffer()){
    if ((++pCurRead)>=pEnd) pCurRead=pStart;
  }
}
//---------------------------------------------------------------------------
void CircularBuffer::Reset()
{
  if (Buf==NULL) return;

  while (Lock) Sleep(0);
  Buf[0]=0;
  pCurRead=pStart;
  pCurWrite=pStart+1;
}
//---------------------------------------------------------------------------
void CircularBuffer::Destroy()
{
  if (Buf==NULL) return;

  while (Lock) Sleep(0);
  delete[] Buf;Buf=NULL;
}

//---------------------------------------------------------------------------
#endif
