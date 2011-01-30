#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H

template <class Type> class DynamicArray
{
  Type *Data;
  int Size;
public:
  DynamicArray()      { Size=0;SizeInc=16;Data=NULL;NumItems=0; }
  DynamicArray(int n) { Size=n;SizeInc=16;Data=new Type[n];NumItems=0; }
  ~DynamicArray()     { if (Data) delete[] Data; }

#if !defined(_VC_BUILD) && !defined(UNIX)
  Type& DynamicArray::operator[](int i) { return Data[i]; }
#endif
  operator Type*()        { return Data; }

  int NumItems;

  void Add(Type x)
  {
    if (NumItems>=Size) Resize(Size+SizeInc);
    Data[NumItems++]=x;
  }

  void DeleteAll(bool resize=true)
  {
    NumItems=0;
    if (resize) Resize(0);
  }

  void Resize(int);

  int GetSize(){ return Size; }

  int SizeInc;
};

template <class Type> void DynamicArray<Type>::Resize(int NewSize)
{
  Type *NewData=NULL;
  if (NewSize) NewData=new Type[NewSize];
  if (Size>0 && NewSize>0) memcpy(NewData,Data,sizeof(Type)*min(Size,NewSize));
  if (Data) delete[] Data;
  Data=NewData;
  Size=NewSize;
  NumItems=min(Size,NumItems);
}

#endif

