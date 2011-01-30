#ifndef EASYSTRINGLIST_CPP
#define EASYSTRINGLIST_CPP

#include <string.h>

#include <easystringlist.h>

#ifndef ESL_MINSIZE
#define ESL_MINSIZE 16
#endif
//---------------------------------------------------------------------------
void EasyStringList::ReallocData(long Idx,long nDat)
{
  delete[] Info[Idx].Data;

  Info[Idx].NumData=nDat;
  Info[Idx].Data=new long[nDat];
}

EasyStringList::EasyStringList(ESLSortEnum sort1,ESLSortEnum sort2)
{
  Size=ESL_MINSIZE;
  Info=new ESL_Data[Size];
  memset(Info,0,sizeof(ESL_Data)*Size);

  Sort=sort1;
  Sort2=sort2;
  NumStrings=0;
}

ESL_Data& EasyStringList::operator[](int idx)
{
  return Info[max(min(idx,NumStrings-1),0)];
}

ESL_Data& EasyStringList::Get(int idx)
{
  return Info[max(min(idx,NumStrings-1),0)];
}

void EasyStringList::DeleteAll() { ResizeBuf(0); }
//---------------------------------------------------------------------------
void EasyStringList::ResizeBuf(int NewSize)
{
  Size=max(NewSize,ESL_MINSIZE);
  if (NumStrings){
    for (int n=NewSize;n<NumStrings;n++){ // If shrinking
      delete[] Info[n].String;
      delete[] Info[n].Data;
    }
    NumStrings=min(NumStrings,NewSize);

    ESL_Data *NewInfo=new ESL_Data[Size];

    if (NumStrings) memcpy(NewInfo,Info,NumStrings*sizeof(ESL_Data));

    delete[] Info;
    Info=NewInfo;
  }else{
    delete[] Info;

    Info=new ESL_Data[Size];
  }
  if (NumStrings==0) memset(Info,0,sizeof(ESL_Data)); // Zero the first entry
}
//---------------------------------------------------------------------------
int EasyStringList::Compare(int n,ESLSortEnum s,char *Str,long *Dat,long nDat)
{
  if (s==eslSortByNameI){
    return strcmpi(Str,Info[n].String);
  }else if (s==eslSortByName){
    return strcmp(Str,Info[n].String);
  }else if ((long)s>=0){
    if ((long)s>=nDat){
      return 1;
    }else if ((long)s>=Info[n].NumData){
      return -1;
    }else{
      if (Dat[(long)s]<Info[n].Data[(long)s]){
        return -1;
      }else if (Dat[(long)s]==Info[n].Data[(long)s]){
        return 0;
      }
    }
  }
  return 1;
}
//--------------------------------------------------------------------------- 
int EasyStringList::InsertAt(int i,long nDat,char *AddStr,...)
{
  return Insert(i,nDat,AddStr,((long*)(&AddStr))+1);
}

int EasyStringList::Add(char *AddStr)
{
  return Add(0,AddStr);
}
int EasyStringList::Add(char *AddStr,long AddData)
{
  return Add(1,AddStr,AddData);
}
int EasyStringList::Add(char *AddStr,long AddData,long AddData2)
{
  return Add(2,AddStr,AddData,AddData2);
}
int EasyStringList::Add(char *AddStr,long AddData,long AddData2,long AddData3)
{
  return Add(3,AddStr,AddData,AddData2,AddData3);
}
int EasyStringList::Add(long nDat,char *AddStr,...)
{
  long *p=((long*)&AddStr)+1;

  bool Less;
  for (int n=0;n<=NumStrings;n++){
    if (n==NumStrings){
      Less=true;
    }else{
      int Val=Compare(n,Sort,AddStr,p,nDat);
      if (Val==0) Val=Compare(n,Sort2,AddStr,p,nDat);
      Less = Val<=0;
    }

    if (Less) return Insert(n,nDat,AddStr,p);
  }
  return 0;
}
int EasyStringList::Insert(int n,long nDat,char *AddStr,long *p)
{
  if (NumStrings>=Size) ResizeBuf(NumStrings+16);
	if (n>NumStrings) n=NumStrings;
	if (n<0) n=0;

  for (int i=NumStrings;i>n;i--) Info[i]=Info[i-1];

  Info[n].String=new char[strlen(AddStr)+1];
  strcpy(Info[n].String,AddStr);

  Info[n].NumData=nDat;
  Info[n].Data=new long[nDat];
  for (int i=0;i<nDat;i++) Info[n].Data[i]=*(p++);

  NumStrings++;
  return n;
}
//---------------------------------------------------------------------------
int EasyStringList::FindString(char *Str)
{
  for (int n=0;n<NumStrings;n++) if (strcmp(Str,Info[n].String)==0) return n;
  return -1;
}
//---------------------------------------------------------------------------
int EasyStringList::FindString_I(char *Str)
{
  for (int n=0;n<NumStrings;n++) if (strcmpi(Str,Info[n].String)==0) return n;
  return -1;
}
//---------------------------------------------------------------------------
void EasyStringList::ResizeData(long Idx,long nDat)
{
  long *OldData=Info[Idx].Data,OldNumData=Info[Idx].NumData;

  Info[Idx].NumData=nDat;
  Info[Idx].Data=new long[nDat];
  for (int i=0;i<min(nDat,OldNumData);i++) Info[Idx].Data[i]=OldData[i];
  for (int i=OldNumData;i<nDat;i++)        Info[Idx].Data[i]=0;

  delete[] OldData;
}
void EasyStringList::ResizeData(long Idx,long nDat,long Data0,...)
{
  ReallocData(Idx,nDat);

  long *p=&Data0;
  for (int i=0;i<nDat;i++) Info[Idx].Data[i]=*(p++);
}
//---------------------------------------------------------------------------
void EasyStringList::SetString(int Idx,char *NewStr)
{
  delete[] Info[Idx].String;

  Info[Idx].String=new char[strlen(NewStr)+1];
  strcpy(Info[Idx].String,NewStr);
}
//---------------------------------------------------------------------------
void EasyStringList::Delete(int Idx)
{
  NumStrings--;
  delete[] Info[Idx].String;
  delete[] Info[Idx].Data;
  for (int i=Idx;i<NumStrings;i++) Info[i]=Info[i+1];
}
//---------------------------------------------------------------------------
EasyStringList::~EasyStringList()
{
  for (int i=0;i<NumStrings;i++){
    delete[] Info[i].String;
    delete[] Info[i].Data;
  }
  delete[] Info;
}
//---------------------------------------------------------------------------
#endif

