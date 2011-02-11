segment .data

charset_blk:    INCBIN "./rc/charset.blk"

icon16_bmp:     INCBIN "./rc/icon16.bmp"

icon32_bmp:     INCBIN "./rc/icon32.bmp"

icon64_bmp:     INCBIN "./rc/icon64.bmp"

st_charset_bmp: INCBIN "./rc/st_chars_mono.bmp"

flags_bmp:      INCBIN "./rc/flags_256.bmp"

segment .text

%ifdef WIN32
%define UNDERSCORES 1
%endif

%ifdef UNDERSCORES

%macro cextern 1-*
%rep %0
extern _%1
%rotate 1
%endrep
%endmacro

%macro cglobal 1-*
%rep %0
%define %1 _%1
global _%1
%rotate 1
%endrep
%endmacro

%else

%macro cextern 1-*
%rep %0
extern %1
%define _%1 %1
%rotate 1
%endrep
%endmacro

%define cglobal global

%endif

cglobal Get_st_charset_bmp,Get_charset_blk,Get_tos_flags_bmp
cglobal Get_icon16_bmp,Get_icon32_bmp,Get_icon64_bmp

Get_st_charset_bmp:
  mov eax,st_charset_bmp
  ret

Get_charset_blk:
  mov eax,charset_blk
  ret

Get_tos_flags_bmp:
  mov eax,flags_bmp
  ret

Get_icon16_bmp:
  mov eax,icon16_bmp
  ret

Get_icon32_bmp:
  mov eax,icon32_bmp
  ret

Get_icon64_bmp:
  mov eax,icon64_bmp
  ret
