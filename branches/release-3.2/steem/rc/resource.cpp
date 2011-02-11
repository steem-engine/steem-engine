#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define ASMCALL __cdecl

char* try_to_load(char *fn)
{
  FILE *f=fopen(fn,"rb");
  if (f==NULL) return NULL;

  fseek(f,0,SEEK_END);
  int l=ftell(f);
  fseek(f,0,SEEK_SET);
    
  char *loaded_at=(char*)malloc(l);
  fread(loaded_at,1,l,f);
  fclose(f);
  return loaded_at;
}

extern "C" ASMCALL char* Get_icon16_bmp()
{
  static char* loaded_at=NULL;
  if (loaded_at==NULL) loaded_at=try_to_load("./code/rc/icon16.bmp");
  return loaded_at;
}

extern "C" ASMCALL char* Get_icon32_bmp()
{
  static char* loaded_at=NULL;
  if (loaded_at==NULL) loaded_at=try_to_load("./code/rc/icon32.bmp");
  return loaded_at;
}

extern "C" ASMCALL char* Get_icon64_bmp()
{
  static char* loaded_at=NULL;
  if (loaded_at==NULL) loaded_at=try_to_load("./code/rc/icon64.bmp");
  return loaded_at;
}

extern "C" ASMCALL char* Get_charset_blk()
{
  static char *loaded_at=NULL;
  if (loaded_at==NULL) loaded_at=try_to_load("./rc/charset.blk");
  return loaded_at;
}

extern "C" ASMCALL char* Get_st_charset_bmp() 
{ 
  static char *loaded_at=NULL; 
  if (loaded_at==NULL) loaded_at=try_to_load("./code/rc/ST_Chars_Mono.bmp"); 
  return loaded_at; 
} 

extern "C" ASMCALL char* Get_tos_flags_bmp() 
{ 
  static char *loaded_at=NULL; 
  if (loaded_at==NULL) loaded_at=try_to_load("./code/rc/Flags.bmp"); 
  return loaded_at; 
} 

