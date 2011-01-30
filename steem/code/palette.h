#ifdef IN_MAIN
#define EXT
#else
#define EXT extern
#endif

EXT void palette_copy();
EXT void palette_flip();
EXT WORD palette_add_entry(DWORD col);
EXT void palette_convert_all();
EXT void palette_convert(int);

#ifdef IN_MAIN
EXT void palette_prepare(bool);
EXT void palette_remove();
EXT bool palette_changed=false;

EXT void make_palette_table(int brightness,int contrast);

EXT int brightness=0,contrast=0;
int palhalf=0,palnum=0;

#ifdef WIN32
HPALETTE winpal=NULL,oldwinpal;
HDC PalDC;
#elif defined(UNIX)
const long standard_palette[18][2]={
	{0,0},
	{8,0xff0000},{9,0x00ff00},{10,0x0000ff},
	{11,0xffff00},{12,0x00ffff},{13,0xff00ff},
	{13,0xc0c0c0},
	{14,0xe0e0e0},
	{247,0x0000c0},
	{248,0x800000},{249,0x008000},{250,0x000080},
	{251,0x808000},{252,0x008080},{253,0x800080},
	{254,0x808080},
	{255,0xffffff}};
Colormap colormap=0;
XColor new_pal[257];

#endif

long logpal[257];
#endif

#undef EXT
