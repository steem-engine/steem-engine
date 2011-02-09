#ifndef ICONGROUP_H
#define ICONGROUP_H

#define HIWORD(a) WORD(((DWORD)(a)) >> 16)
#define RGB(r,g,b) (BYTE(r) | (BYTE(g) << 8) | (BYTE(b) << 16))

#define IG_OK 0
#define IGERR_WRONGDEPTH 1
#define IGERR_COMPRESSEDBMP 2
#define IGERR_NOTBMP 3
#define IGERR_FILEERROR 4
#define IGERR_CANTCREATE 5
//---------------------------------------------------------------------------
typedef struct tagBITMAPINFOHEADER{
   DWORD  biSize;
   LONG   biWidth;
   LONG   biHeight;
//   WORD   biPlanes : 16,biBitCount : 16;
   DWORD  biPlanes_biBitCount;
   DWORD  biCompression; //BI_RGB==0
   DWORD  biSizeImage;
   LONG   biXPelsPerMeter;
   LONG   biYPelsPerMeter;
   DWORD  biClrUsed;
   DWORD  biClrImportant;
}BITMAPINFOHEADER;
//---------------------------------------------------------------------------
class IconGroup
{
private:
	DWORD CreateImages(BYTE *,DWORD,int,DWORD [256],DWORD);

  XImage *Img,*MaskImg;
  BITMAPINFOHEADER bi;
public:
  IconGroup() { Img=NULL;MaskImg=NULL;NumIcons=0;IconWidth=0;IconHeight=0;}
  ~IconGroup() { }

  DWORD LoadIcons(Display*,char*,int,DWORD=RGB(255,192,128));
  DWORD LoadIconsFromMemory(Display *,BYTE*,int,DWORD=RGB(255,192,128));
  bool DrawIcon(int,Drawable,GC,int,int);
  bool DrawIconMask(int,Drawable,GC,int,int);
  Pixmap CreateIconPixmap(int,GC);
  Pixmap CreateMaskBitmap(int);
  XImage *NewIconImage(Display*,int,int);
  void FreeIcons();

  Display *XD;

  int NumIcons;
  int IconWidth,IconHeight;
  int IconsPerRow;

  static long (*ColList)[2];
  static int ColListLen;
};

#endif

