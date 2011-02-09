#ifdef IN_MAIN
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)
#endif

#define IOLIST_PSEUDO_AD 0x53000000
#define IOLIST_PSEUDO_AD_PSG (IOLIST_PSEUDO_AD+0x1000)
#define IOLIST_PSEUDO_AD_FDC (IOLIST_PSEUDO_AD+0x2000)
#define IOLIST_PSEUDO_AD_IKBD (IOLIST_PSEUDO_AD+0x3000)

#define IS_IOLIST_PSEUDO_ADDRESS(x) ((x&0xff000000)==IOLIST_PSEUDO_AD)

EXT void iolist_add_entry(MEM_ADDRESS,char*,int,char* INIT(NULL),BYTE* INIT(NULL));

#ifdef IN_MAIN

typedef struct{
  MEM_ADDRESS ad;
  EasyStr name;
  int bytes;
  EasyStr bitmask;
  BYTE*ptr;
}iolist_entry;

int iolist_length=0;

iolist_entry iolist[300];

void iolist_init();
iolist_entry*search_iolist(MEM_ADDRESS);
int iolist_box_draw(HDC,int,int,int,int,iolist_entry*,BYTE*);
void iolist_box_click(int,iolist_entry*,BYTE*); //bit number clicked, toggle bit
int iolist_box_width(iolist_entry*);

#endif

#undef EXT
#undef INIT

