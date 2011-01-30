; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! LOW RES !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
cglobal draw_scanline_8_lowres_scrolled,draw_scanline_16_lowres_scrolled
cglobal draw_scanline_24_lowres_scrolled,draw_scanline_32_lowres_scrolled

cglobal draw_scanline_8_lowres_scrolled_400,draw_scanline_16_lowres_scrolled_400
cglobal draw_scanline_24_lowres_scrolled_400,draw_scanline_32_lowres_scrolled_400
; -------------------------------------------------------------------------
%macro GET_SCREEN_DATA_INTO_REGS_AND_INC_SA_AND_HSCROLL_FOR_LEFT 0
  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA
  mov ecx,[_shifter_hscroll]
  shl ebx,cl
  shl edx,cl
  neg ecx
  add ecx,16
%endmacro
; -------------------------------------------------------------------------
%macro DRAW_SCANLINE_LOWRES_SCROLLED 1 ;bpp
  push ebp
  mov ebp,esp

  SAVE_REGS
  GET_START 1,168
  GET_PC_DRAW_ADDR_INTO_EDI
  DRAW_BORDER %1,[border1]
  mov eax,[picture]
  mov [counter],eax
  or eax,eax
  jz near %%scrolled_finished
  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA_AND_HSCROLL_FOR_LEFT
  dec dword[counter]
%%draw_pixel_on_left:
  CALC_COL_LOWRES_SHORT
  DRAWPIXEL %1,1  ;draw pixel carelessly
  loop %%draw_pixel_on_left

  jmp near %%next_raster
%%draw_raster:
  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA
  %rep 16
    CALC_COL_LOWRES
    DRAWPIXEL %1,1  ;draw pixel carelessly
  %endrep
%%next_raster:
  dec dword[counter]
  jns near %%draw_raster

  GET_SCREEN_DATA_INTO_REGS
  mov ecx,[_shifter_hscroll]
%%draw_pixel_on_right:
  CALC_COL_LOWRES_SHORT
  DRAWPIXEL %1,0 ;draw the last few carefully
  loop %%draw_pixel_on_right

%%scrolled_finished:
  DRAW_BORDER %1,[border2]
  RESTORE_REGS

  leave
%endmacro

draw_scanline_8_lowres_scrolled: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_LOWRES_SCROLLED 1
  ret

draw_scanline_16_lowres_scrolled: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_LOWRES_SCROLLED 2
  ret

draw_scanline_24_lowres_scrolled: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_LOWRES_SCROLLED 3
  ret

draw_scanline_32_lowres_scrolled: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_LOWRES_SCROLLED 4
  ret

; ---------------------------------------------------------------------------
; ------------------------------ _400 --------------------------------------
; ---------------------------------------------------------------------------
%macro DRAW_SCANLINE_LOWRES_SCROLLED_400 1 ;bpp
  push ebp
  mov ebp,esp

  SAVE_REGS
  GET_START 1,168
  GET_PC_DRAW_ADDR_INTO_EDI_400
  DRAW_BORDER_400 %1,[border1]
  mov eax,[picture]
  or eax,eax
  jz near %%scrolled_finished
  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA
  push ecx
  mov ecx,[_shifter_hscroll]
  shl ebx,cl
  shl edx,cl
  mov eax,16
  sub eax,ecx
  pop ecx
  mov [counter],eax
%%draw_pixel_on_left:
  CALC_COL_LOWRES
  DRAWPIXEL_LOWRES_400 %1
  dec dword[counter]
  jnz %%draw_pixel_on_left

  mov eax,[picture]
  mov [counter],eax
  or eax,eax
  jz near %%scrolled_finished
  dec dword[counter]

  jmp near %%next_raster
%%draw_raster:
  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA
  %rep 16
    CALC_COL_LOWRES
    PUSHPIXEL_LOWRES %1
  %endrep
  POPRASTER_LOWRES %1
%%next_raster:
  dec dword[counter]
  jns near %%draw_raster

  GET_SCREEN_DATA_INTO_REGS
  mov eax,[_shifter_hscroll]
  mov [counter],eax
%%draw_pixel_on_right:
  CALC_COL_LOWRES
  DRAWPIXEL_LOWRES_400 %1
  dec dword[counter]
  jnz near %%draw_pixel_on_right

%%scrolled_finished:
  DRAW_BORDER_400 %1,[border2]
  RESTORE_REGS

  leave
%endmacro

draw_scanline_8_lowres_scrolled_400: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_LOWRES_SCROLLED_400 1
  ret

draw_scanline_16_lowres_scrolled_400: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_LOWRES_SCROLLED_400 2
  ret

draw_scanline_24_lowres_scrolled_400: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_LOWRES_SCROLLED_400 3
  ret

draw_scanline_32_lowres_scrolled_400: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_LOWRES_SCROLLED_400 4
  ret

