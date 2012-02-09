#ifndef EASYSTR_H
#define EASYSTR_H

#include <stddef.h>

#ifndef EASYSTR_BASE
#define EASYSTR_BASE 0
#endif

struct EasyStr_Dummy_Struct{
  int i;
};

#define EASYSTR_OPERATORS_DEC(TYPE) \
  EasyStr& operator =(TYPE p); \
  EasyStr operator +(TYPE p); \
  EasyStr& operator +=(TYPE p); \
  bool operator ==(TYPE p); \
  bool operator !=(TYPE p);

#define EASYSTR_OPERATORS(TYPE,lpchar) \
  EasyStr& EasyStr::operator =(TYPE p){return EqualsString(lpchar);} \
  EasyStr EasyStr::operator +(TYPE p){return PlusString(lpchar);} \
  EasyStr& EasyStr::operator +=(TYPE p){return PlusEqualsString(lpchar);} \
  bool EasyStr::operator ==(TYPE p){return strcmp(Text,lpchar)==0;} \
  bool EasyStr::operator !=(TYPE p){return strcmp(Text,lpchar)!=0;}

class EasyStr
{
private:
  EasyStr& EqualsString(const char *);
  EasyStr PlusString(const char *);
  EasyStr& PlusEqualsString(const char *);
  bool SameAsString(const char *);
  bool NotSameAsString(const char *);

  char *CharToString(char c){numbuf[0]=c;numbuf[1]=0;return numbuf;}

  void DeleteBuf() { if (Text!=Empty_Text) delete[] Text; }
  void ResizeBuf(int);

  size_t bufsize;
  static char numbuf[32];
  static char Empty_Text[4];
public:
  EasyStr();
  EasyStr(char);
  EasyStr(char *);
  EasyStr(const char *);
  EasyStr(char *,char *);
  EasyStr(const EasyStr&);
  EasyStr(int);EasyStr(unsigned int);
  EasyStr(long);EasyStr(unsigned long);
  EasyStr(bool);

  ~EasyStr();

  char *c_str();

  bool IsEmpty();
  bool Empty();
  bool IsNotEmpty();
  bool NotEmpty();

  int Length();
  void SetLength(int size);
  void SetBufSize(int size);
  int GetBufSize();

  EasyStr& Insert(EasyStr,int); //What to add, where to add it
  EasyStr& Delete(int,int);     //First char to delete, number of chars to delete

  char *Right();
  char *Rights(int);
  char RightChar();
  EasyStr Lefts(int);
  EasyStr Mids(int,int);

  EasyStr UpperCase();
  EasyStr LowerCase();

  EasyStr& LPad(int,char);      //New length of string, char to pad with
  EasyStr& RPad(int,char);      //Ditto
  int InStr(char *);

  int CompareNoCase(char *otext);
//---------------------------------------------------------------------------
  char *Text;
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
  operator char*();
#if !defined(_VC_BUILD) && !defined(_MINGW_BUILD) && !defined(UNIX)
  operator struct EasyStr_Dummy_Struct*();
  char& operator[](unsigned int idx);
#endif

  EASYSTR_OPERATORS_DEC(char*)
  EASYSTR_OPERATORS_DEC(const char*)
  EASYSTR_OPERATORS_DEC(EasyStr&)
  EASYSTR_OPERATORS_DEC(EasyStr*)

  EASYSTR_OPERATORS_DEC(signed int)
  EASYSTR_OPERATORS_DEC(signed short)
  EASYSTR_OPERATORS_DEC(signed long)

  EASYSTR_OPERATORS_DEC(unsigned int)
  EASYSTR_OPERATORS_DEC(unsigned short)
  EASYSTR_OPERATORS_DEC(unsigned long)
  EASYSTR_OPERATORS_DEC(unsigned char)

  EASYSTR_OPERATORS_DEC(bool)
  EASYSTR_OPERATORS_DEC(char)
};
#endif

