#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

class CircularBuffer
{
private:
  LPBYTE Buf,pStart,pEnd,pCurRead,pCurWrite;
  DWORD BufSize;
  volatile bool Lock;
public:
  CircularBuffer(DWORD Size=0);
  ~CircularBuffer();
  bool Create(DWORD),AddByte(BYTE),AddBytes(BYTE*,DWORD);
  void Reset();
  void Destroy();
  bool AreBytesInBuffer();
  BYTE ReadByte();
  void NextByte();
#ifndef WIN32
  // On Windows Sleep(0) will let another thread take over instantly
  int Sleep(int n);
#endif
  bool IsLocked();
};
#endif
