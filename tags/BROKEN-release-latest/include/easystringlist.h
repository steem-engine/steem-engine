#ifndef EASYSTRINGLIST_H
#define EASYSTRINGLIST_H
//---------------------------------------------------------------------------
typedef enum {eslNoSort=-1,eslSortByName=-2,eslSortByNameI=-3,eslSortByData0=0, 
              eslSortByData1=1,eslSortByData2=2,eslSortByData3=3,eslSortByData4=4, 
              eslSortByData5=5,eslSortByData6=6,eslSortByData7=7,eslSortByData8=8} ESLSortEnum; 
//---------------------------------------------------------------------------
typedef struct{
  char *String;
  long *Data;
  long NumData;
}ESL_Data;
//---------------------------------------------------------------------------
class EasyStringList
{
private:
  void ReallocData(long Idx,long nDat);
  ESL_Data *Info;
  int Size;
public:
  int NumStrings;
  ESLSortEnum Sort,Sort2;

  EasyStringList(ESLSortEnum sort1=eslSortByNameI,ESLSortEnum sort2=eslSortByNameI);
  ~EasyStringList();

  ESL_Data& operator[](int);
  ESL_Data& Get(int);

  int Compare(int,ESLSortEnum,char *,long *,long);
  int Add(long,char*,...);
  int Add(char*);
  int Add(char*,long);
  int Add(char*,long,long);
  int Add(char*,long,long,long);

  int InsertAt(int,long,char *,...);
	int Insert(int,long,char*,long*);

  void SetString(int,char *);
  int FindString(char*),FindString_I(char*);

  void ResizeData(long,long);
  void ResizeData(long,long,long,...);

  void Delete(int);
  void DeleteAll();

  void ResizeBuf(int);
};
#endif
