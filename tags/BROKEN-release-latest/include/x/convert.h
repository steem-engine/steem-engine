#ifndef LINUXCONV_H
#define LINUXCONV_H

#ifdef UNIX
#include <string.h>
#include <ctype.h> 
#include <unistd.h> 

#define max(a,b) (a>b ? a:b)
#define min(a,b) (a>b ? b:a)
#define strcmpi strcasecmp

char *itoa(int i,char *s,int radix)
{
  if (radix==10) sprintf(s,"%i",i);
  if (radix==16) sprintf(s,"%x",i);
  return s;
}
char *ultoa(unsigned long l,char *s,int radix)
{
  if (radix==10) sprintf(s,"%u",(unsigned int)l);
  if (radix==16) sprintf(s,"%x",(unsigned int)l);
  return s;
}
char *strupr(char *s)
{
  int n;
  for (n=0;n<(int)strlen(s);n++) s[n]=(char)(islower(s[n]) ? toupper(s[n]):s[n]);
  return s;
}
char *strlwr(char *s)
{
  int n;
  for (n=0;n<(int)strlen(s);n++) s[n]=(char)(isupper(s[n]) ? tolower(s[n]):s[n]);
  return s;
}

typedef long long LONGLONG;
typedef unsigned long long DWORDLONG;
#endif

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;

typedef char* LPSTR;
typedef unsigned char* LPBYTE;
typedef unsigned short* LPWORD;
typedef unsigned long* LPDWORD;
typedef long* LPLONG;
typedef void* LPVOID;

typedef unsigned int UINT;

#endif



