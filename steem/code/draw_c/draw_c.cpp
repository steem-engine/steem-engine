/*---------------------------------------------------------------------------
FILE: draw_c.cpp
MODULE: draw_c
DESCRIPTION: Alternative C++ drawing routines for systems that don't support
Steem's faster x86 assembler versions.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#define IN_DRAW_C

#include "easystr.h"
typedef EasyStr Str;

#include "conditions.h"
#include "steemh.h"
#include "draw.h"
#include "display.h"
#include "emulator.h"
#include "osd.h"

long PCpal_array[16];

extern "C" long* ASMCALL Get_PCpal(){
  return PCpal_array;
}

DWORD palette_convert_RRRRGGGGBBBB(DWORD dw){
  int rp,gp,bp;
  if(BytesPerPixel==1){
    return 255;
  }else if (BytesPerPixel==2){
    if(rgb555){
      rp=14;gp=9;bp=4;
    }else{
      rp=15;gp=10;bp=4;
    }
  }else{
    rp=31;gp=23;bp=15;
  }
  return ((dw&0xf00)<<(rp-11)) | ((dw&0xf0)<<(gp-7)) | ((dw&0xf)<<(bp-3));
}

#define OSD_PIXEL(mask)  \
  if(dw1&mask){    \
    OSD_DRAWPIXEL(colour);  \
  }else if(dw0&mask){    \
    OSD_DRAWPIXEL(0);     \
  }else{              \
    dadd+=bpp; }

#define bpp 1
#define OSD_DRAWPIXEL(c) *(dadd++)=(BYTE)c;
extern "C" void ASMCALL osd_draw_char_8(long*source_ad,BYTE*draw_mem,long x,long y,int draw_line_length,long colour,long h){
#include "draw_c_osd_draw_char.cpp"
}

#undef OSD_DRAWPIXEL
#undef bpp

#define bpp 2
#define OSD_DRAWPIXEL(c) *(((WORD*)dadd)++)=(WORD)c;
extern "C" void ASMCALL osd_draw_char_16(long*source_ad,BYTE*draw_mem,long x,long y,int draw_line_length,long colour,long h){
#include "draw_c_osd_draw_char.cpp"
}
extern "C" void ASMCALL osd_blueize_line_16_555(int x,int y,int w){
#include "draw_c_osd_blueize_line.cpp"
}
extern "C" void ASMCALL osd_blueize_line_16_565(int x,int y,int w){
#include "draw_c_osd_blueize_line.cpp"
}
#undef OSD_DRAWPIXEL
#undef bpp

#define bpp 3
#define OSD_DRAWPIXEL(c) *(dadd++)=(BYTE)(c&255);*(dadd++)=(BYTE)(c>>8);*(dadd++)=(BYTE)(c>>16)
extern "C" void ASMCALL osd_draw_char_24(long*source_ad,BYTE*draw_mem,long x,long y,int draw_line_length,long colour,long h){
#include "draw_c_osd_draw_char.cpp"
}
#undef RAINBOW
extern "C" void ASMCALL osd_blueize_line_24(int x,int y,int w){
#include "draw_c_osd_blueize_line.cpp"
}
#undef OSD_DRAWPIXEL
#undef bpp

#define bpp 4
#define OSD_DRAWPIXEL(c) *(((LONG*)dadd)++)=c;
extern "C" void ASMCALL osd_draw_char_32(long*source_ad,BYTE*draw_mem,long x,long y,int draw_line_length,long colour,long h){
#include "draw_c_osd_draw_char.cpp"
}
extern "C" void ASMCALL osd_blueize_line_32(int x,int y,int w){
#include "draw_c_osd_blueize_line.cpp"
}
#undef OSD_DRAWPIXEL
#undef bpp

extern "C" void ASMCALL osd_draw_char_clipped_8(long*,BYTE*,long,long,int,long,long,RECT*){}
extern "C" void ASMCALL osd_draw_char_clipped_16(long*,BYTE*,long,long,int,long,long,RECT*){}
extern "C" void ASMCALL osd_draw_char_clipped_24(long*,BYTE*,long,long,int,long,long,RECT*){}
extern "C" void ASMCALL osd_draw_char_clipped_32(long*,BYTE*,long,long,int,long,long,RECT*){}

extern "C" void ASMCALL osd_draw_char_transparent_8(long*,BYTE*,long,long,int,long,long){}
extern "C" void ASMCALL osd_draw_char_transparent_16(long*,BYTE*,long,long,int,long,long){}
extern "C" void ASMCALL osd_draw_char_transparent_24(long*,BYTE*,long,long,int,long,long){}
extern "C" void ASMCALL osd_draw_char_transparent_32(long*,BYTE*,long,long,int,long,long){}

extern "C" void ASMCALL osd_draw_char_clipped_transparent_8(long*,BYTE*,long,long,int,long,long,RECT*){}
extern "C" void ASMCALL osd_draw_char_clipped_transparent_16(long*,BYTE*,long,long,int,long,long,RECT*){}
extern "C" void ASMCALL osd_draw_char_clipped_transparent_24(long*,BYTE*,long,long,int,long,long,RECT*){}
extern "C" void ASMCALL osd_draw_char_clipped_transparent_32(long*,BYTE*,long,long,int,long,long,RECT*){}

extern "C" void ASMCALL osd_black_box_8(void*,int,int,int,int,long,long){}
extern "C" void ASMCALL osd_black_box_16(void*,int,int,int,int,long,long){}
extern "C" void ASMCALL osd_black_box_24(void*,int,int,int,int,long,long){}
extern "C" void ASMCALL osd_black_box_32(void*,int,int,int,int,long,long){}


extern "C" void ASMCALL palette_convert_16_555(int n){
  long col=((STpal[n] & 0x888) >> 3)+((STpal[n] & 0x777) << 1);  //correct STE colour
  PCpal[n]=((col & 0x00f) << 1) | ((col & 0x0f0) << 2) | ((col & 0xf00) << 3);
  *((WORD*)(&PCpal[n])+1)=LOWORD(PCpal[n]); //copy to long
}

extern "C" void ASMCALL palette_convert_16_565(int n){
  long col=((STpal[n] & 0x888) >> 3)+((STpal[n] & 0x777) << 1);  //correct STE colour
  PCpal[n]=((col & 0x00f) << 1) | ((col & 0x0f0) << 3) | ((col & 0xf00) << 4);
  *((WORD*)(&PCpal[n])+1)=LOWORD(PCpal[n]); //copy to long
}

extern "C" void ASMCALL palette_convert_24(int n){
  long col=((STpal[n] & 0x888) >> 3)+((STpal[n] & 0x777) << 1);  //correct STE colour
  PCpal[n]=((col & 0x00f) << 4) | ((col & 0x0f0) << 8)
         |((col & 0xf00) << 12) | ((col & 0x00f) << 28);
}

extern "C" void ASMCALL palette_convert_32(int n){
  long col=((STpal[n] & 0x888) >> 3)+((STpal[n] & 0x777) << 1);  //correct STE colour
  PCpal[n]=((col & 0x00f) << 4) | ((col & 0x0f0) << 8) | ((col & 0xf00) << 12);
}



#define GET_START(doubleflag,increase)  \
  source=shifter_draw_pointer&0xffffff; \
  while(source+increase>mem_len)source-=mem_len;  \

#define DRAW_BORDER(n)    \
  for(;n>0;n--){                       \
    DRAW_2_BORDER_PIXELS                \
    DRAW_2_BORDER_PIXELS                 \
    DRAW_2_BORDER_PIXELS                                  \
    DRAW_2_BORDER_PIXELS                                  \
    DRAW_2_BORDER_PIXELS                                  \
    DRAW_2_BORDER_PIXELS                                  \
    DRAW_2_BORDER_PIXELS                                  \
    DRAW_2_BORDER_PIXELS                                  \
  }

#define GET_SCREEN_DATA_INTO_REGS_AND_INC_SA \
  w0=DPEEK(source);w1=DPEEK(source+2); \
  w2=DPEEK(source+4);w3=DPEEK(source+6); \
  source+=8;

#define GET_SCREEN_DATA_INTO_REGS_AND_INC_SA_MEDRES \
  w0=DPEEK(source);w1=DPEEK(source+2); \
  source+=4;

#define GET_SCREEN_DATA_INTO_REGS_AND_INC_SA_HIRES \
  w0=DPEEK(source);      \
  source+=2;

#define CALC_COL_LOWRES_AND_DRAWPIXEL(mask)  \
  if(w3&mask){                                      \
    if(w2&mask){                                     \
      if(w1&mask){                                    \
        if(w0&mask){ /*1111    */DRAWPIXEL(PCpal+15)     \
        }else{       /*1110    */ DRAWPIXEL(PCpal+14)     \
        }                                                \
      }else{                                              \
        if(w0&mask){ /*1101   */DRAWPIXEL(PCpal+13)          \
        }else{       /*1100   */   DRAWPIXEL(PCpal+12)        \
        }                                                    \
      }                                                       \
    }else{                                                     \
      if(w1&mask){                                              \
        if(w0&mask){ /*1011   */DRAWPIXEL(PCpal+11)                \
        }else{       /*1010  */DRAWPIXEL(PCpal+10)                  \
        }                                                          \
      }else{                                                        \
        if(w0&mask){ /*1001 */DRAWPIXEL(PCpal+9)                       \
        }else{       /*1000   */DRAWPIXEL(PCpal+8)                      \
        }                                                              \
      }                                                                 \
    }                                                                    \
  }else{           \
    if(w2&mask){    \
      if(w1&mask){   \
        if(w0&mask){ /*0111    */DRAWPIXEL(PCpal+7) \
        }else{       /*0110    */ DRAWPIXEL(PCpal+6) \
        }                                           \
      }else{                                         \
        if(w0&mask){ /*0101   */DRAWPIXEL(PCpal+5)      \
        }else{       /*0100   */   DRAWPIXEL(PCpal+4)    \
        }                                               \
      }                                                  \
    }else{                                                \
      if(w1&mask){                                         \
        if(w0&mask){ /*0011   */DRAWPIXEL(PCpal+3)            \
        }else{       /*0010  */DRAWPIXEL(PCpal+2)              \
        }                                                     \
      }else{                                                   \
        if(w0&mask){ /*0001 */DRAWPIXEL(PCpal+1)                  \
        }else{       /*0000   */DRAWPIXEL(PCpal)                   \
        }                                                         \
      }     \
    }        \
  }

#define CALC_COL_MEDRES_AND_DRAWPIXEL(mask)  \
  if(w1&mask){                                      \
    if(w0&mask){                                     \
      DRAWPIXEL_MEDRES(PCpal+3)    \
    }else{                    \
      DRAWPIXEL_MEDRES(PCpal+2)    \
    }                                      \
  }else{                                    \
    if(w0&mask){                                     \
      DRAWPIXEL_MEDRES(PCpal+1)    \
    }else{                    \
      DRAWPIXEL_MEDRES(PCpal)    \
    }                                        \
  }

#define CALC_COL_HIRES_AND_DRAWPIXEL(mask)  \
  if(w0&mask){                                     \
    DRAWPIXEL(fore)    \
  }else{                    \
    DRAWPIXEL(back)    \
  }                                      \

#define DRAW_2_BORDER_PIXELS  *LPWORD(draw_dest_ad)=*LPWORD(PCpal);draw_dest_ad+=2;
#define DRAWPIXEL(s_add)  *(draw_dest_ad++)=*LPBYTE(s_add);
#define DRAWPIXEL_MEDRES(s) DRAWPIXEL(s)
extern "C" void ASMCALL draw_scanline_8_lowres_pixelwise(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}
extern "C" void ASMCALL draw_scanline_8_medres_pixelwise(int border1,int picture,int border2,int hscroll){
  border1*=2;border2*=2;
#include "draw_c_medres_scanline.cpp"
}
#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL
#undef DRAWPIXEL_MEDRES

#define DRAW_2_BORDER_PIXELS  *LPDWORD(draw_dest_ad)=*LPDWORD(PCpal);draw_dest_ad+=4;
#define DRAWPIXEL(s_add)  *LPWORD(draw_dest_ad)=*LPWORD(s_add);draw_dest_ad+=2;
#define DRAWPIXEL_MEDRES(s) DRAWPIXEL(s)
extern "C" void ASMCALL draw_scanline_16_lowres_pixelwise(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}
extern "C" void ASMCALL draw_scanline_16_medres_pixelwise(int border1,int picture,int border2,int hscroll){
  border1*=2;border2*=2;
#include "draw_c_medres_scanline.cpp"
}
#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL
#undef DRAWPIXEL_MEDRES

#define DRAW_2_BORDER_PIXELS  \
              *(draw_dest_ad++)=*LPBYTE(PCpal); \
              *(draw_dest_ad++)=*(LPBYTE(PCpal)+1); \
              *(draw_dest_ad++)=*(LPBYTE(PCpal)+2); \
              *(draw_dest_ad++)=*LPBYTE(PCpal); \
              *(draw_dest_ad++)=*(LPBYTE(PCpal)+1); \
              *(draw_dest_ad++)=*(LPBYTE(PCpal)+2);
#define DRAWPIXEL(s_add)  \
              *(draw_dest_ad++)=*LPBYTE(s_add); \
              *(draw_dest_ad++)=*(LPBYTE(s_add)+1); \
              *(draw_dest_ad++)=*(LPBYTE(s_add)+2);
#define DRAWPIXEL_MEDRES(s) DRAWPIXEL(s)
extern "C" void ASMCALL draw_scanline_24_lowres_pixelwise(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}
extern "C" void ASMCALL draw_scanline_24_medres_pixelwise(int border1,int picture,int border2,int hscroll){
  border1*=2;border2*=2;
#include "draw_c_medres_scanline.cpp"
}
#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL
#undef DRAWPIXEL_MEDRES

#define DRAW_2_BORDER_PIXELS  \
          *LPDWORD(draw_dest_ad)=*LPDWORD(PCpal);draw_dest_ad+=4; \
          *LPDWORD(draw_dest_ad)=*LPDWORD(PCpal);draw_dest_ad+=4;
#define DRAWPIXEL(s_add)  *LPDWORD(draw_dest_ad)=*LPDWORD(s_add);draw_dest_ad+=4;
#define DRAWPIXEL_MEDRES(s) DRAWPIXEL(s)
extern "C" void ASMCALL draw_scanline_32_lowres_pixelwise(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}
extern "C" void ASMCALL draw_scanline_32_medres_pixelwise(int border1,int picture,int border2,int hscroll){
  border1*=2;border2*=2;
#include "draw_c_medres_scanline.cpp"
}
#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL
#undef DRAWPIXEL_MEDRES

///////////////////////////////////////////////////////////
////////////////////////// _dw  ///////////////////////////
///////////////////////////////////////////////////////////

#define DRAW_2_BORDER_PIXELS *(((WORD*)(draw_dest_ad))++)=*(WORD*)(PCpal);*(((WORD*)(draw_dest_ad))++)=*(WORD*)(PCpal);
#define DRAWPIXEL(s_add) *(((WORD*)(draw_dest_ad))++)=*(WORD*)(s_add);
extern "C" void ASMCALL draw_scanline_8_lowres_pixelwise_dw(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}
#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL

#define DRAW_2_BORDER_PIXELS  *(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(PCpal);*(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(PCpal);
#define DRAWPIXEL(s_add)  *(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(s_add);
extern "C" void ASMCALL draw_scanline_16_lowres_pixelwise_dw(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}
#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL

#define DRAW_2_BORDER_PIXELS \
  {                              \
   for(int r=3*3;r>0;r-=3){                                                                                         \
    *(draw_dest_ad++)=*(BYTE*)(PCpal);*(draw_dest_ad++)=*((BYTE*)(PCpal)+1);*(draw_dest_ad++)=*((BYTE*)(PCpal)+2);   \
   } \
  }
#define DRAWPIXEL(s_add)  \
  *(draw_dest_ad++)=*(BYTE*)(PCpal);*(draw_dest_ad++)=*(((BYTE*)(PCpal))+1);*(draw_dest_ad++)=*(((BYTE*)(PCpal))+2);   \
  *(draw_dest_ad++)=*(BYTE*)(PCpal);*(draw_dest_ad++)=*(((BYTE*)(PCpal))+1);*(draw_dest_ad++)=*(((BYTE*)(PCpal))+2); 

extern "C" void ASMCALL draw_scanline_24_lowres_pixelwise_dw(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}
#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL

#define DRAW_2_BORDER_PIXELS *(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(PCpal);*(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(PCpal);*(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(PCpal);*(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(PCpal);
#define DRAWPIXEL(s_add)  *(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(s_add);*(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(s_add);
extern "C" void ASMCALL draw_scanline_32_lowres_pixelwise_dw(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}
#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL

///////////////////////////////////////////////////////////
////////////////////////// _400 ///////////////////////////
///////////////////////////////////////////////////////////

#define DRAW_2_BORDER_PIXELS  *(((WORD*)(draw_dest_ad+draw_line_length)))=*(WORD*)(PCpal);*(((WORD*)(draw_dest_ad+draw_line_length+2)))=*(WORD*)(PCpal); \
                              *(((WORD*)(draw_dest_ad))++)=*(WORD*)(PCpal);*(((WORD*)(draw_dest_ad))++)=*(WORD*)(PCpal);
#define DRAWPIXEL(s_add)  *((WORD*)(draw_dest_ad+draw_line_length))=*(WORD*)(s_add);*(((WORD*)(draw_dest_ad))++)=*(WORD*)(s_add);
#define DRAWPIXEL_MEDRES(s_add)  *((BYTE*)(draw_dest_ad+draw_line_length))=*(BYTE*)(s_add);*((BYTE*)(draw_dest_ad)++)=*(BYTE*)(s_add);
extern "C" void ASMCALL draw_scanline_8_lowres_pixelwise_400(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}
extern "C" void ASMCALL draw_scanline_8_medres_pixelwise_400(int border1,int picture,int border2,int hscroll){
#include "draw_c_medres_scanline.cpp"
}
#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL
#undef DRAWPIXEL_MEDRES

#define DRAW_2_BORDER_PIXELS  *(((DWORD*)(draw_dest_ad+draw_line_length)))=*(DWORD*)(PCpal);*(((DWORD*)(draw_dest_ad+draw_line_length+4)))=*(DWORD*)(PCpal);*(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(PCpal);*(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(PCpal);
#define DRAWPIXEL(s_add)  *(((DWORD*)(draw_dest_ad+draw_line_length)))=*(DWORD*)(s_add);*(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(s_add);
#define DRAWPIXEL_MEDRES(s_add)  *(((WORD*)(draw_dest_ad+draw_line_length)))=*(WORD*)(s_add);*(((WORD*)(draw_dest_ad))++)=*(WORD*)(s_add);
extern "C" void ASMCALL draw_scanline_16_lowres_pixelwise_400(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}
extern "C" void ASMCALL draw_scanline_16_medres_pixelwise_400(int border1,int picture,int border2,int hscroll){
#include "draw_c_medres_scanline.cpp"
}
#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL
#undef DRAWPIXEL_MEDRES

#define DRAW_2_BORDER_PIXELS \
  {BYTE* dadd=draw_dest_ad+draw_line_length;                                                                    \
   for(int r=3*3;r>0;r-=3){                                                                                      \
    *(dadd++)=*(BYTE*)(PCpal);*(dadd++)=*((BYTE*)(PCpal)+1);*(dadd++)=*((BYTE*)(PCpal)+2);                        \
   }                                                                                                               \
   for(int r=3*3;r>0;r-=3){                                                                                         \
    *(draw_dest_ad++)=*(BYTE*)(PCpal);*(draw_dest_ad++)=*((BYTE*)(PCpal)+1);*(draw_dest_ad++)=*((BYTE*)(PCpal)+2);   \
   } \
  }
#define DRAWPIXEL(s_add)  \
  {BYTE* dadd=draw_dest_ad+draw_line_length;                                                                    \
  *(dadd++)=*(BYTE*)(PCpal);*(dadd++)=*(((BYTE*)(PCpal))+1);*(dadd++)=*(((BYTE*)(PCpal))+2);                        \
  *(dadd++)=*(BYTE*)(PCpal);*(dadd++)=*(((BYTE*)(PCpal))+1);*(dadd++)=*(((BYTE*)(PCpal))+2);                        \
  *(draw_dest_ad++)=*(BYTE*)(PCpal);*(draw_dest_ad++)=*(((BYTE*)(PCpal))+1);*(draw_dest_ad++)=*(((BYTE*)(PCpal))+2);   \
  *(draw_dest_ad++)=*(BYTE*)(PCpal);*(draw_dest_ad++)=*(((BYTE*)(PCpal))+1);*(draw_dest_ad++)=*(((BYTE*)(PCpal))+2); \
  }
#define DRAWPIXEL_MEDRES(s_add)  \
  {BYTE* dadd=draw_dest_ad+draw_line_length;                                                                    \
  *(dadd++)=*(BYTE*)(s_add);*(dadd++)=*(((BYTE*)(s_add))+1);*(dadd++)=*(((BYTE*)(s_add))+2);                        \
  *(draw_dest_ad++)=*(BYTE*)(s_add);*(draw_dest_ad++)=*(((BYTE*)(s_add))+1);*(draw_dest_ad++)=*(((BYTE*)(s_add))+2);   \
  }

extern "C" void ASMCALL draw_scanline_24_lowres_pixelwise_400(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}
extern "C" void ASMCALL draw_scanline_24_medres_pixelwise_400(int border1,int picture,int border2,int hscroll){
#include "draw_c_medres_scanline.cpp"
}
#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL
#undef DRAWPIXEL_MEDRES

#define DRAW_2_BORDER_PIXELS \
  {DWORD* dadd=(DWORD*)(draw_dest_ad+draw_line_length);                                                                    \
   *(dadd++)=*(DWORD*)(PCpal);*(dadd++)=*(DWORD*)(PCpal);*(dadd++)=*(DWORD*)(PCpal);*(dadd++)=*(DWORD*)(PCpal);                                                                                \
   *(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(PCpal);*(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(PCpal);*(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(PCpal);*(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(PCpal); \
  }
#define DRAWPIXEL(s_add)  *(((DWORD*)(draw_dest_ad+draw_line_length)))=*(DWORD*)(s_add); \
                          *(((DWORD*)(draw_dest_ad+draw_line_length+4)))=*(DWORD*)(s_add); \
                          *(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(s_add);*(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(s_add);
#define DRAWPIXEL_MEDRES(s_add)  *(((DWORD*)(draw_dest_ad+draw_line_length)))=*(DWORD*)(s_add); \
                          *(((DWORD*)(draw_dest_ad))++)=*(DWORD*)(s_add);
extern "C" void ASMCALL draw_scanline_32_lowres_pixelwise_400(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}
extern "C" void ASMCALL draw_scanline_32_medres_pixelwise_400(int border1,int picture,int border2,int hscroll){
#include "draw_c_medres_scanline.cpp"
}
#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL
#undef DRAWPIXEL_MEDRES

#define DRAWPIXEL(col) *(draw_dest_ad++)=BYTE(col);
extern "C" void ASMCALL draw_scanline_8_hires(int border1,int picture,int border2,int){
#include "draw_c_hires_scanline.cpp"
}
#undef DRAWPIXEL
#define DRAWPIXEL(col) *(((WORD*)draw_dest_ad)++)=WORD(col);
extern "C" void ASMCALL draw_scanline_16_hires(int border1,int picture,int border2,int){
#include "draw_c_hires_scanline.cpp"
}
#undef DRAWPIXEL
#define DRAWPIXEL(col) *(draw_dest_ad++)=BYTE(col);*(draw_dest_ad++)=BYTE(col);*(draw_dest_ad++)=BYTE(col);
extern "C" void ASMCALL draw_scanline_24_hires(int border1,int picture,int border2,int){
#include "draw_c_hires_scanline.cpp"
}
#undef DRAWPIXEL
#define DRAWPIXEL(col) *(((DWORD*)draw_dest_ad)++)=DWORD(col);
extern "C" void ASMCALL draw_scanline_32_hires(int border1,int picture,int border2,int){
#include "draw_c_hires_scanline.cpp"
}
#undef DRAWPIXEL

