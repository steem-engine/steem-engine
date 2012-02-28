#ifdef IN_MAIN
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)
#endif

#define T(s)       Translation(s)
#define StrT(s)    Translation(s)
#define CStrT(s)   Translation(s).Text
#define StaticT(s) (TranslateString=Translation(s))

EXT EasyStr TranslateString,TranslateFileName;
EXT char *TranslateBuf INIT(NULL),*TranslateUpperBuf INIT(NULL);
EXT int TranslateBufLen INIT(0);

EXT EasyStr Translation(char *s);

#ifdef IN_MAIN
EasyStr StripAndT(char *s)
{
  EasyStr Ret=Translation(s);
  for(;;){
    int i=Ret.InStr("&");
    if (i<0) break;
    Ret.Delete(i,1);
  }
  return Ret;
}
#else
extern EasyStr StripAndT(char*);
#endif

#undef EXT
#undef INIT

