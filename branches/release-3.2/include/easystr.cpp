/*---------------------------------------------------------------------------
FILE: easystr.cpp
MODULE: helper
DESCRIPTION: A (supposedly) easy to use string class.
---------------------------------------------------------------------------*/

#ifndef EASYSTR_CPP
#define EASYSTR_CPP

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <easystr.h>

char EasyStr::numbuf[32];
char EasyStr::Empty_Text[4]={0,0,0,0}; // It is done like this so it can be written
                                        // to (in the case of EasyStr[0]=0
//---------------------------------------------------------------------------
//------------------------------- Constructors ------------------------------
//---------------------------------------------------------------------------
EasyStr::EasyStr()
{
  bufsize=0;
  Text=Empty_Text;
}
EasyStr::EasyStr(char *nt)
{
  bufsize=strlen(nt);
  Text=new char[bufsize+1];
  strcpy(Text,nt);
}
EasyStr::EasyStr(const char *nt)
{
  bufsize=strlen(nt);
  Text=new char[bufsize+1];
  strcpy(Text,nt);
}
EasyStr::EasyStr(char c)
{
  bufsize=1;
  Text=new char[bufsize+1];
  Text[0]=c;Text[1]=0;
}
EasyStr::EasyStr(char *nt1,char *nt2)
{
  bufsize=strlen(nt1)+strlen(nt2);
  Text=new char[bufsize+1];
  strcpy(Text,nt1);
  strcat(Text,nt2);
}
EasyStr::EasyStr(const EasyStr& nt)
{
  bufsize=strlen(nt.Text);
  Text=new char[bufsize+1];
  strcpy(Text,nt.Text);
}

EasyStr::EasyStr(int num)
{
  itoa(num,numbuf,10);
  bufsize=strlen(numbuf);
  Text=new char[bufsize+1];
  strcpy(Text,numbuf);
}
EasyStr::EasyStr(unsigned int num)
{
  ultoa(num,numbuf,10);
  bufsize=strlen(numbuf);
  Text=new char[bufsize+1];
  strcpy(Text,numbuf);
}
EasyStr::EasyStr(long num)
{
  itoa(num,numbuf,10);
  bufsize=strlen(numbuf);
  Text=new char[bufsize+1];
  strcpy(Text,numbuf);
}
EasyStr::EasyStr(unsigned long num)
{
  ultoa(num,numbuf,10);
  bufsize=strlen(numbuf);
  Text=new char[bufsize+1];
  strcpy(Text,numbuf);
}
EasyStr::EasyStr(bool num)
{
  bufsize=1;
  Text=new char[bufsize+1];
  Text[0]=char(num ? '1':'0');Text[1]=0;
}
//---------------------------------------------------------------------------
//---------------------------- Member Functions -----------------------------
//---------------------------------------------------------------------------
EasyStr::~EasyStr(){ DeleteBuf(); }
char *EasyStr::c_str(){ return Text; }
bool EasyStr::IsEmpty(){ return !(Text[0]); }
bool EasyStr::Empty(){ return !(Text[0]); }
bool EasyStr::IsNotEmpty(){ return (bool)(Text[0]); }
bool EasyStr::NotEmpty(){ return (bool)(Text[0]); }
int EasyStr::Length(){ return strlen(Text); }
void EasyStr::SetLength(int size){ ResizeBuf(max(size,0)); }
void EasyStr::SetBufSize(int size){ ResizeBuf(max(size,0)); }
int EasyStr::GetBufSize(){ return bufsize; }
int EasyStr::CompareNoCase(char *otext){ return strcmpi(Text,otext); }
//---------------------------------------------------------------------------
void EasyStr::ResizeBuf(int size)
{
  char *Old=Text;

  if (size==0){
    Text=Empty_Text;
  }else{
    Text=new char[size+1];
  }
  bufsize=size;

  if (Text!=Empty_Text){
    memcpy(Text,Old,min(strlen(Old)+1,bufsize));
    Text[bufsize]=0;
  }
  if (Old!=Empty_Text) delete[] Old;
}
//---------------------------------------------------------------------------
EasyStr& EasyStr::Delete(int start,int count)
{
  start-=EASYSTR_BASE;
  if (count<=0 || start<0) return *this;

  int Len=(int)strlen(Text);
  if (start>=Len) return *this;

  if (start+count>Len){  //Delete to end of string
    Text[start]=0;
  }else{
    memmove(Text+start,Text+start+count,(Len-(start+count))+1); //strlen(Text+start+count)+1
  }

  return *this;
}
//---------------------------------------------------------------------------
EasyStr& EasyStr::Insert(EasyStr ToAdd,int Pos)
{
  int AddLen=strlen(ToAdd.Text),CurLen=strlen(Text);

  Pos-=EASYSTR_BASE;
  if (Pos<0 || Pos>CurLen || AddLen==0) return *this;

  if (size_t(CurLen+AddLen)>bufsize) ResizeBuf(CurLen+AddLen);
  memmove(Text+Pos+AddLen,Text+Pos,(CurLen-Pos)+1); //strlen(Text+Pos)+1
  memcpy(Text+Pos,ToAdd.Text,AddLen);

  return *this;
}
//---------------------------------------------------------------------------
char* EasyStr::Right()
{
  if (Text[0]){
    return Text+strlen(Text)-1;
  }else{
    return Text;
  }
}
//---------------------------------------------------------------------------
char* EasyStr::Rights(int numchars)
{
  if (Text[0]){
    int len=strlen(Text);
    return Text+len-max(min(numchars,len),0);
  }
  return Text;
}
//---------------------------------------------------------------------------
char EasyStr::RightChar()
{
  if (Text[0]) return Text[strlen(Text)-1];
  return 0;
}
//---------------------------------------------------------------------------
EasyStr EasyStr::Lefts(int NumChars)
{
  if (NumChars<=0) return "";
  if (NumChars>=(int)strlen(Text)) return Text;

  EasyStr Temp;
  Temp.SetLength(NumChars);
  memcpy(Temp.Text,Text,NumChars);
  return Temp;
}
//---------------------------------------------------------------------------
EasyStr EasyStr::Mids(int firstchar,int numchars)
{
  firstchar-=EASYSTR_BASE;
  if (firstchar>=(int)strlen(Text) || firstchar<0) return "";

  EasyStr temp(Text+firstchar);
  temp.SetLength(numchars);
  return temp;
}
//---------------------------------------------------------------------------
EasyStr EasyStr::UpperCase()
{
  EasyStr temp(Text);
  strupr(temp.Text);
  return temp;
}
//---------------------------------------------------------------------------
EasyStr EasyStr::LowerCase()
{
  EasyStr temp(Text);
  strlwr(temp.Text);
  return temp;
}
//---------------------------------------------------------------------------
EasyStr& EasyStr::LPad(int NewLen,char c)
{
  int CurLen=strlen(Text);
  if (NewLen<=CurLen) return *this;

  int LenToAdd=NewLen-CurLen;
  EasyStr AddChars;
  AddChars.SetLength(LenToAdd);
  memset(AddChars.Text,c,LenToAdd);
  Insert(AddChars,EASYSTR_BASE);
  return *this;
}
EasyStr& EasyStr::RPad(int NewLen,char c)
{
  int CurLen=strlen(Text);
  if (NewLen<=CurLen) return *this;

  SetLength(NewLen);
  memset(Text+CurLen,c,NewLen-CurLen);
  return *this;
}
int EasyStr::InStr(char *ss)
{
  char *Pos=strstr(Text,ss);
  if (Pos) return long(Pos)-long(Text);
  return -1;
}
//---------------------------------------------------------------------------
//----------------------------- Operators -----------------------------------
//---------------------------------------------------------------------------
EasyStr::operator char*(){ return Text; }
#if !defined(_VC_BUILD) && !defined(_MINGW_BUILD) && !defined(UNIX)
EasyStr::operator struct EasyStr_Dummy_Struct*(){ return NULL; }

char& EasyStr::operator[](unsigned int idx)
{
#if EASYSTR_BASE>0
  idx-=EASYSTR_BASE;
#endif
#ifdef EASYSTR_FOOLPROOF
  idx=max(min(idx,Length()),0);
#endif
  return Text[idx];
}
#endif

EASYSTR_OPERATORS(char*,p)
EASYSTR_OPERATORS(const char*,p)
EASYSTR_OPERATORS(EasyStr&,p.Text)
EASYSTR_OPERATORS(EasyStr*,p->Text)

EASYSTR_OPERATORS(signed int,itoa(p,numbuf,10))
EASYSTR_OPERATORS(signed short,itoa(p,numbuf,10))
EASYSTR_OPERATORS(signed long,itoa(p,numbuf,10))

EASYSTR_OPERATORS(unsigned int,ultoa(p,numbuf,10))
EASYSTR_OPERATORS(unsigned short,ultoa(p,numbuf,10))
EASYSTR_OPERATORS(unsigned long,ultoa(p,numbuf,10))
EASYSTR_OPERATORS(unsigned char,ultoa(p,numbuf,10))

EASYSTR_OPERATORS(bool,(char*)(p ? "1":"0"))
EASYSTR_OPERATORS(char,CharToString(p))

EasyStr& EasyStr::EqualsString(const char *nt)
{
  size_t len=strlen(nt);
  bool DoResize=0;
  if (len>bufsize || len<bufsize-128 || len==0) DoResize=true;
  if (nt>=Text && nt<Text+bufsize) DoResize=0; // Setting to itself
  if (DoResize){
    DeleteBuf();
    bufsize=len;
    if (len){
      Text=new char[bufsize+1];
    }else{
      Text=Empty_Text;
    }
  }
  if (Text!=Empty_Text) strcpy(Text,nt);
  return *this;
}
//---------------------------------------------------------------------------
EasyStr EasyStr::PlusString(const char *nt)
{
  return EasyStr(Text,(char*)nt);
}
//---------------------------------------------------------------------------
EasyStr& EasyStr::PlusEqualsString(const char *new_text)
{
  EasyStr nt=new_text;  // Just in case new_text is pointer in this string
  size_t len=nt.Length();
  if ((int)strlen(Text)+len>bufsize) ResizeBuf(len+strlen(Text));
  strcat(Text,nt);
  return *this;
}
//---------------------------------------------------------------------------
bool EasyStr::SameAsString(const char *otext)
{
  return !(bool)(strcmp(Text,otext));
}
//---------------------------------------------------------------------------
bool EasyStr::NotSameAsString(const char *otext)
{
  return (bool)strcmp(Text,otext);
}
//---------------------------------------------------------------------------
#endif

