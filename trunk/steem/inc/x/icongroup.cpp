#ifndef ICONGROUP_CPP
#define ICONGROUP_CPP

#include "icongroup.h"
//---------------------------------------------------------------------------
long (*IconGroup::ColList)[2]=NULL;
int IconGroup::ColListLen=0;
//---------------------------------------------------------------------------
DWORD IconGroup::LoadIconsFromMemory(Display *pass_Disp,BYTE *Mem,int IcoWid,DWORD RGBTransparent)
{
  BITMAPINFOHEADER loaded_bi;
  BYTE *Pixels=NULL;
  DWORD Pal[256];
  int BmpBitsPerPixel=0;
  DWORD NumBytes,Err=0;

  memcpy(&loaded_bi,Mem+14,sizeof(loaded_bi));
  if (loaded_bi.biCompression==0 /*BI_RGB*/){
	  if (HIWORD(loaded_bi.biPlanes_biBitCount)==24){
		  BmpBitsPerPixel=24;
	  }else if (HIWORD(loaded_bi.biPlanes_biBitCount)==8){
		  BmpBitsPerPixel=8;
		  memcpy(Pal,Mem+14+sizeof(loaded_bi),256*4);
    }else if (HIWORD(loaded_bi.biPlanes_biBitCount)==1){
		  BmpBitsPerPixel=1;
    }else{
      Err=IGERR_WRONGDEPTH;
    }
  }else{
    Err=IGERR_COMPRESSEDBMP;
  }
  if (Err==0){
    bi=loaded_bi;
    XD=pass_Disp;
    IconWidth=IcoWid;

    NumBytes=(bi.biWidth*bi.biHeight*BmpBitsPerPixel + 7)/8;
    Pixels=new BYTE[NumBytes];
    memcpy(Pixels,Mem + *LPDWORD(Mem+10) /*fiOffset*/,NumBytes);

    FreeIcons();
    Err=CreateImages(Pixels,NumBytes,BmpBitsPerPixel,Pal,RGBTransparent);

    delete[] Pixels;
  }
  return Err;
}

DWORD IconGroup::LoadIcons(Display *pass_Disp,char *File,int IcoWid,DWORD RGBTransparent)
{
  FILE *f=fopen(File,"rb");
  if (f){
    BITMAPINFOHEADER loaded_bi;
    BYTE *Pixels=NULL;
    DWORD NumBytes,Err=0;
    BYTE FileInfo[14];
	  DWORD Pal[256];
	  int BmpBitsPerPixel=0;

    if (fread(&FileInfo,1,sizeof(FileInfo),f)==sizeof(FileInfo)){
      if (fread(&loaded_bi,1,sizeof(loaded_bi),f)==sizeof(loaded_bi)){
        if (loaded_bi.biCompression==0 /*BI_RGB*/){
	        if (HIWORD(loaded_bi.biPlanes_biBitCount)==24){
	        	BmpBitsPerPixel=24;
      	  }else if (HIWORD(loaded_bi.biPlanes_biBitCount)==8){
      		  BmpBitsPerPixel=8;
      		  if (fread(Pal,1,256*4,f)!=256*4) Err=IGERR_NOTBMP;
          }else if (HIWORD(loaded_bi.biPlanes_biBitCount)==1){
      		  BmpBitsPerPixel=1;
          }else{
	          Err=IGERR_WRONGDEPTH;
          }
        }else{
          Err=IGERR_COMPRESSEDBMP;
        }
      }else{
        Err=IGERR_NOTBMP;
      }
    }else{
      Err=IGERR_NOTBMP;
    }
    if (Err==0){
      NumBytes=(loaded_bi.biWidth*loaded_bi.biHeight*BmpBitsPerPixel + 7)/8;
      Pixels=new BYTE[NumBytes];
      fseek(f,*LPDWORD(FileInfo+10) /*bfOffBits*/,SEEK_SET);
      if (fread(Pixels,1,NumBytes,f)==NumBytes){
        FreeIcons();
        bi=loaded_bi;
        XD=pass_Disp;
        IconWidth=IcoWid;
        Err=CreateImages(Pixels,NumBytes,BmpBitsPerPixel,Pal,RGBTransparent);
      }else{
        Err=IGERR_FILEERROR;
      }
      delete[] Pixels;
    }
    fclose(f);

    return Err;
  }else{
    return IGERR_FILEERROR;
  }
}

DWORD IconGroup::CreateImages(BYTE *Pixels,DWORD NumBytes,int PixelBpp,DWORD Pal[256],DWORD RGBTransparent)
{
  if (Pixels==NULL) return IGERR_CANTCREATE;

  DWORD BGRTransparent=RGB_TO_BGR(RGBTransparent);

  int Scr=XDefaultScreen(XD);
  int Depth=XDefaultDepth(XD,Scr);
  int BytesPerPixel=(Depth+7)/8;
  if (Depth>=24) BytesPerPixel=4;

  int w=bi.biWidth,h=abs(bi.biHeight);
  int ImgNumBytes=w*h*BytesPerPixel;

  char *ImgPixels=(char*)malloc(ImgNumBytes);
  Img=XCreateImage(XD,XDefaultVisual(XD,Scr),
                     Depth,ZPixmap,0,ImgPixels,
                     w,h,BytesPerPixel*8,0);
  if (Img){
    char *MaskImgPixels=(char*)malloc(ImgNumBytes);
    MaskImg=XCreateImage(XD,XDefaultVisual(XD,Scr),
                          Depth,ZPixmap,0,MaskImgPixels,
                          w,h,BytesPerPixel*8,0);
    if (MaskImg){
      IconHeight=h;
      NumIcons=w/IconWidth;
      IconsPerRow=NumIcons;

			int nBGRBits[3]={0,0,0};
 			int nBGRShift[3]={0,0,0};
      DWORD WhiteCol=(Img->red_mask | Img->green_mask | Img->blue_mask);
      if (Depth==8){
      	WhiteCol=255;
      }else{
  			DWORD BGRMask[3]={Img->blue_mask,Img->green_mask,Img->red_mask};
  			for (int bgr=0;bgr<3;bgr++){
  				for (int n=0;n<32;n++){
  					if (BGRMask[bgr] & (1 << n)){
  					  if (nBGRBits[bgr]==0) nBGRShift[bgr]=n;
  						nBGRBits[bgr]++;
  					}
  				}
  			}
  		}

      memset(ImgPixels,0,ImgNumBytes);
      memset(MaskImgPixels,0,ImgNumBytes);

      int x=0,y=h-1;
      BYTE *p=Pixels,*PixelsEnd=Pixels+NumBytes;
      DWORD BGRCol;
      BYTE *pBGRColByte=LPBYTE(&BGRCol);
      BYTE Bit=128;
      while (p<PixelsEnd){
        if (PixelBpp==8){
          BGRCol=Pal[*(p++)];
        }else if (PixelBpp==24){
          BGRCol=*LPDWORD(p);
          p+=PixelBpp;
        }else{
          BGRCol=((*p & Bit) ? 0xffffffff:0);
          Bit>>=1;
          if (Bit==0){
            p++;
            Bit=128;
          }
        }
        BGRCol &= 0xffffff;
        if (BGRCol==BGRTransparent){
          XPutPixel(MaskImg,x,y,WhiteCol);
        }else{
		      if (ColList==NULL){
		      	DWORD Pix=0;
		      	for (int bgr=0;bgr<3;bgr++){
							DWORD Col=pBGRColByte[bgr];
							Col >>= 8-nBGRBits[bgr];
							Col <<= nBGRShift[bgr];
							Pix|=Col;
		      	}
  	        XPutPixel(Img,x,y,Pix);
  	      }else{
  	        int c=0;
            for (int n=0;n<ColListLen;n++){
              if (BGRCol==DWORD(ColList[n][1])) c=ColList[n][0];
            }
  	        XPutPixel(Img,x,y,c);
  	      }
        }
        if (++x >= w){
          x=0;
          y--;
        }
      }
      return 0;
    }else{
      free(MaskImgPixels);
    }
  }else{ //Img==0
    free(ImgPixels);
  }
  FreeIcons();
  return IGERR_CANTCREATE;
}
//---------------------------------------------------------------------------
XImage *IconGroup::NewIconImage(Display *XD,int w,int h)
{
	if (Img) FreeIcons();

	IconGroup::XD=XD;
	IconWidth=w;
	IconHeight=h;
	NumIcons=1;
  IconsPerRow=NumIcons;

  int Scr=XDefaultScreen(XD);
  int Depth=XDefaultDepth(XD,Scr);
  int BytesPerPixel=(Depth+7)/8;
  if (Depth>=24) BytesPerPixel=4;

  int ImgNumBytes=w*h*BytesPerPixel;
  char *ImgPixels=(char*)malloc(ImgNumBytes);
  Img=XCreateImage(XD,XDefaultVisual(XD,Scr),
                     Depth,ZPixmap,0,ImgPixels,
                     w,h,BytesPerPixel*8,0);
  return Img;
}
//---------------------------------------------------------------------------
bool IconGroup::DrawIcon(int IcoIdx,Drawable DrawTo,GC gc,int x,int y)
{
  if (Img==NULL || IcoIdx>=NumIcons || IcoIdx<0) return 0;

  int IcoX=(IcoIdx % IconsPerRow) * IconWidth;
  int IcoY=(IcoIdx/IconsPerRow) * IconHeight;

	if (MaskImg){
	  XSetFunction(XD,gc,GXand);
	  XPutImage(XD,DrawTo,gc,MaskImg,IcoX,IcoY,x,y,IconWidth,IconHeight);
	  XSetFunction(XD,gc,GXor);
	}
  XPutImage(XD,DrawTo,gc,Img,IcoX,IcoY,x,y,IconWidth,IconHeight);
	if (MaskImg) XSetFunction(XD,gc,GXcopy);

  return true;
}
//---------------------------------------------------------------------------
bool IconGroup::DrawIconMask(int IcoIdx,Drawable DrawTo,GC gc,int x,int y)
{
  if (MaskImg==NULL || IcoIdx>=NumIcons || IcoIdx<0) return 0;

  int IcoX=(IcoIdx % IconsPerRow) * IconWidth;
  int IcoY=(IcoIdx/IconsPerRow) * IconHeight;

  XSetFunction(XD,gc,GXand);
  XPutImage(XD,DrawTo,gc,MaskImg,IcoX,IcoY,x,y,IconWidth,IconHeight);
  XSetFunction(XD,gc,GXcopy);
  return true;
}
//---------------------------------------------------------------------------
Pixmap IconGroup::CreateIconPixmap(int IcoIdx,GC gc)
{
  if (Img==NULL || IcoIdx>=NumIcons || IcoIdx<0) return 0;

  int IcoX=(IcoIdx % IconsPerRow) * IconWidth;
  int IcoY=(IcoIdx/IconsPerRow) * IconHeight;

  int Depth=XDefaultDepth(XD,XDefaultScreen(XD));
  Pixmap IconPixmap=XCreatePixmap(XD,XDefaultRootWindow(XD),IconWidth,IconWidth,Depth);
  XPutImage(XD,IconPixmap,gc,Img,IcoX,IcoY,0,0,IconWidth,IconHeight);
  return IconPixmap;
}
//---------------------------------------------------------------------------
Pixmap IconGroup::CreateMaskBitmap(int IcoIdx)
{
  if (MaskImg==NULL || IcoIdx>=NumIcons || IcoIdx<0) return 0;

  int IcoX=(IcoIdx % IconsPerRow) * IconWidth;
  int IcoY=(IcoIdx/IconsPerRow) * IconHeight;

  char *MonoDat=new char[IconWidth*IconHeight / 8];
  char *pDat=MonoDat;
  memset(MonoDat,0,IconWidth*IconHeight / 8);
  for (int y=IcoY;y<IcoY+IconHeight;y++){
    BYTE Mask=1;
    for (int x=IcoX;x<IcoX+IconWidth;x++){
      if ((MaskImg->f.get_pixel(MaskImg,x,y))){
        *pDat &= ~Mask;
      }else{
        *pDat |= Mask;
      }
      Mask<<=1;
      if (Mask==0){
        Mask=1;
        pDat++;
      }
    }
  }
  Pixmap Ret=XCreateBitmapFromData(XD,XDefaultRootWindow(XD),MonoDat,IconWidth,IconHeight);
  delete[] MonoDat;
  return Ret;
}
//---------------------------------------------------------------------------
void IconGroup::FreeIcons()
{
  if (Img)     { XDestroyImage(Img);Img=NULL; }
  if (MaskImg) { XDestroyImage(MaskImg);MaskImg=NULL; }
}
//---------------------------------------------------------------------------
#endif

