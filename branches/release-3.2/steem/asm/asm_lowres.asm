; FILE: asm_lowres.asm
; MODULE: asm_draw
; DESCRIPTION: Various routines to draw the lowres ST screen to PC video memory.

%macro DRAWPIXEL_LOWRES_DW 1 ;bpp
  %if %1==1
    mov [edi],ax      ; write colour to screen address
  %elif %1==2
    mov [edi],eax     ; write colour to screen address
  %elif %1==3
    mov [edi],eax      ; write colour to screen address... and first byte of next pixel
    ror eax,8
    mov [edi+4],ax     ;finish off next pixel
    rol eax,8
  %elif %1==4
    mov [edi],eax      ; write colour to screen address
    mov [edi+4],eax
  %endif
  add edi,2*%1         ; next screen address
%endmacro

%macro DRAW_SCANLINE_PIXELWISE_LOWRES 2 ;bpp,dw
  push ebp
  mov ebp,esp

  SAVE_REGS
  GET_START 0,160
  GET_PC_DRAW_ADDR_INTO_EDI

  mov ebx,[p_border1]
  shr ebx,4  ;pixels of left border/16
  DRAW_BORDER %1,ebx,%2

  mov ebx,[p_border1]
  and ebx,15
  jz short %%finished_border_1
  mov eax,[pal0]
%%extra_pix_border_1:

%if %2==0
  DRAWPIXEL %1,0
%else
  DRAWPIXEL_LOWRES_DW %1
%endif

  dec ebx
  jnz short %%extra_pix_border_1
%%finished_border_1:

  mov ebx,[p_picture]
  or ebx,ebx
  jz near %%border_2

  mov eax,16
  mov ecx,[p_hscroll]  ;how many pixels to skip
  sub eax,ecx
  ;eax now contains how many pixels to draw in the first raster
  cmp eax,ebx           ;how many pixels to draw
  jl short %%no_more_reduction
  mov eax,ebx           ;there's less pixels to draw than remaining in the first raster
%%no_more_reduction:

  cmp eax,16
  je near %%middle_bit

  push eax ;store counter
  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA ;also increments source address
  shl ebx,cl
  shl edx,cl  ;skip initial pixels

  pop ecx ;number of pixels to draw in first raster in ecx
  sub [p_picture],ecx   ;reduce future number of pixels to draw

%%next_left_pixel:

  CALC_COL_LOWRES
%if %2==0
  DRAWPIXEL %1,0
%else
  DRAWPIXEL_LOWRES_DW %1
%endif

  dec ecx
  jnz near %%next_left_pixel

%%middle_bit:
  mov ecx,[p_picture] ;number of pixels left
  shr ecx,4           ;/16 to get number of full rasters

  jmp near %%next_raster_lowres
%%draw_raster_lowres:
  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA ;also increments source address
  %rep 15

    CALC_COL_LOWRES
%if %2==0
    DRAWPIXEL %1,1 ;carelessly
%else
    DRAWPIXEL_LOWRES_DW %1
%endif

  %endrep

  CALC_COL_LOWRES
%if %2==0
  DRAWPIXEL %1,0 ;carefully
%else
  DRAWPIXEL_LOWRES_DW %1
%endif

%%next_raster_lowres:
  dec ecx
  jns near %%draw_raster_lowres

  mov ecx,[p_picture]
  and ecx,15          ;extra pixels
  jz near %%border_2
  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA ;also increments source address

%%next_right_pixel:

  CALC_COL_LOWRES
%if %2==0
  DRAWPIXEL %1,0 ;carefully
%else
  DRAWPIXEL_LOWRES_DW %1
%endif

  dec ecx
  jnz near %%next_right_pixel

%%border_2:
  mov ebx,[p_border2]
  shr ebx,4  ;pixels of right border/16
  DRAW_BORDER %1,ebx,%2

  mov ebx,[p_border2]
  and ebx,15
  jz short %%finished_border_2
  mov eax,[pal0]
%%extra_pix_border_2:

%if %2==0
  DRAWPIXEL %1,0 ;carefully
%else
  DRAWPIXEL_LOWRES_DW %1
%endif

  dec ebx
  jnz short %%extra_pix_border_2
%%finished_border_2:

  mov [_draw_dest_ad],edi  ;save dest ad - new!!!!!

  RESTORE_REGS

  leave
%endmacro

draw_scanline_8_lowres_pixelwise:
  DRAW_SCANLINE_PIXELWISE_LOWRES 1,0
  ret

draw_scanline_16_lowres_pixelwise:
  DRAW_SCANLINE_PIXELWISE_LOWRES 2,0
  ret

draw_scanline_24_lowres_pixelwise:
  DRAW_SCANLINE_PIXELWISE_LOWRES 3,0
  ret

draw_scanline_32_lowres_pixelwise:
  DRAW_SCANLINE_PIXELWISE_LOWRES 4,0
  ret

; ---------------------------------------------------------------------------
draw_scanline_8_lowres_pixelwise_dw:
  DRAW_SCANLINE_PIXELWISE_LOWRES 1,1
  ret

draw_scanline_16_lowres_pixelwise_dw:
  DRAW_SCANLINE_PIXELWISE_LOWRES 2,1
  ret

draw_scanline_24_lowres_pixelwise_dw:
  DRAW_SCANLINE_PIXELWISE_LOWRES 3,1
  ret

draw_scanline_32_lowres_pixelwise_dw:
  DRAW_SCANLINE_PIXELWISE_LOWRES 4,1
  ret




; ---------------------------------------------------------------------------
; ------------------------------ _400 --------------------------------------
; ---------------------------------------------------------------------------
%macro GET_PC_DRAW_ADDR_INTO_EDI_400 0
  mov edi,[_draw_dest_ad]
  mov ecx,[_draw_line_length] ; offset for second line drawing offset
%endmacro
; ---------------------------------------------------------------------------
%macro DRAW_BORDER_400 2 ;bpp,how_many
  mov eax,[pal0]
%ifnidni %2,ebx
  mov ebx,%2
%endif

  jmp near %%next
%%for:
  %if %1==1
    mov [edi],ax
    %assign n 1
    %rep 15
      mov [edi+2*n],ax
      %assign n n+1
    %endrep
    mov [edi+ecx],ax
    %assign n 1
    %rep 15
      mov [edi+ecx+2*n],ax
      %assign n n+1
    %endrep
  %elif %1==2
    mov [edi],eax
    %assign n 1
    %rep 15
      mov [edi+4*n],eax
      %assign n n+1
    %endrep
    mov [edi+ecx],eax
    %assign n 1
    %rep 15
      mov [edi+ecx+4*n],eax
      %assign n n+1
    %endrep
  %elif %1==3
    mov [edi],eax
    %assign n 1
    %rep 30
      mov [edi+3*n],eax
      %assign n n+1
    %endrep
    mov [edi+3*31],ax
    bswap eax
    mov [edi+3*31+2],ah
    bswap eax
    mov [edi+ecx],eax
    %assign n 1
    %rep 30
      mov [edi+ecx+3*n],eax
      %assign n n+1
    %endrep
    mov [edi+ecx+3*31],ax
    bswap eax
    mov [edi+ecx+3*31+2],ah
    bswap eax
  %elif %1==4
    mov [edi],eax
    %assign n 1
    %rep 31
      mov [edi+4*n],eax
      %assign n n+1
    %endrep
    mov [edi+ecx],eax
    %assign n 1
    %rep 31
      mov [edi+ecx+4*n],eax
      %assign n n+1
    %endrep
  %endif
  add edi,32*%1
%%next:
  dec ebx
  jns near %%for
%endmacro

%macro DRAWPIXEL_LOWRES_400 1 ;bpp
  %if %1==1
    mov [edi],ax      ; write colour to screen address
    mov [edi+ecx],ax  ; write colour to screen address + draw_line_length
  %elif %1==2
    mov [edi],eax     ; write colour to screen address
    mov [edi+ecx],eax ; write colour to screen address + draw_line_length
  %elif %1==3
    mov [edi],eax      ; write colour to screen address... and first byte of next pixel
    mov [edi+ecx],eax  ; write colour to screen address + draw_line_length
    ror eax,8
    mov [edi+4],ax     ;finish off next pixel
    mov [edi+ecx+4],ax
    rol eax,8
  %elif %1==4
    mov [edi],eax      ; write colour to screen address
    mov [edi+ecx],eax  ; write colour to screen address + draw_line_length
    mov [edi+4],eax
    mov [edi+ecx+4],eax
  %endif
  add edi,2*%1         ; next screen address
%endmacro

%macro PUSHPIXEL_LOWRES 1 ;bpp
  %if %1==1
    mov [edi],ax
    push ax
  %elif %1==2
    mov [edi],eax     ; write colour to screen address
    push eax
  %elif %1==3
    mov [edi],eax
    push eax
    shr eax,8
    mov [edi+4],ax
    push ax
  %elif %1==4
    mov [edi],eax      ; write colour to screen address
    mov [edi+4],eax
    push eax
  %endif
  add edi,%1 * 2
%endmacro

%macro POPRASTER_LOWRES 1 ;bpp
  lea eax,[edi+ecx-16*2 * %1] ;look at next line
  %if %1==1
    %assign n 15
    %rep 15
      pop word[eax+2*n]
      %assign n n-1
    %endrep
    pop word[eax]
  %elif %1==2
    %assign n 15
    %rep 15
      pop dword[eax+4*n]
      %assign n n-1
    %endrep
    pop dword[eax]
  %elif %1==3
    %assign n 15
    %rep 15
      pop word[eax+6*n+4]
      pop dword[eax+6*n]
      %assign n n-1
    %endrep
    pop word[eax+4]
    pop dword[eax]
  %elif %1==4
    %assign n 15
    %rep 15
      pop ebx
      mov [eax+8*n],ebx
      mov [eax+8*n+4],ebx
      %assign n n-1
    %endrep
    pop ebx
    mov [eax],ebx
    mov [eax+4],ebx
  %endif
%endmacro

%macro POPPIXEL_LOWRES 1 ;bpp
;  lea eax,[edi+ecx-16*2 * %1] ;look at next line
  %if %1==1
    sub eax,2
    pop word[eax]
  %elif %1==2
    sub eax,4
    pop dword[eax]
  %elif %1==3
    sub eax,6
    pop word[eax+4]
    pop dword[eax]
  %elif %1==4
    pop ebx
    sub eax,8
    mov [eax],ebx
    mov [eax+4],ebx
  %endif
%endmacro




%macro DRAW_SCANLINE_LOWRES_PIXELWISE_400 1 ;bpp
  push ebp
  mov ebp,esp

  SAVE_REGS
  GET_START 1,160
  GET_PC_DRAW_ADDR_INTO_EDI_400

  mov ebx,[p_border1]
  shr ebx,4  ;pixels of left border/16
  DRAW_BORDER_400 %1,ebx

  mov ebx,[p_border1]
  and ebx,15
  jz short %%finished_border_1
  mov eax,[pal0]
%%extra_pix_border_1:
  DRAWPIXEL_LOWRES_400 %1
  dec ebx
  jnz short %%extra_pix_border_1
%%finished_border_1:

  mov ebx,[p_picture]
  or ebx,ebx
  jz near %%border_2

  mov eax,16
  sub eax,[p_hscroll]  ;how many pixels to skip
  ;eax now contains how many pixels to draw in the first raster
  cmp eax,ebx           ;how many pixels to draw
  jl short %%no_more_reduction
  mov eax,ebx           ;there's less pixels to draw than remaining in the first raster
%%no_more_reduction:

  cmp eax,16
  je near %%middle_bit

  sub [p_picture],eax   ;reduce future number of pixels to draw
  mov [counter],eax ;store counter

  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA ;also increments source address
  mov ecx,[p_hscroll]          ;copy to ecx ready for shl
  shl ebx,cl
  shl edx,cl  ;skip initial pixels

  mov ecx,[_draw_line_length] ; offset for second line drawing offset
  push dword[counter]

%%next_left_pixel:
  CALC_COL_LOWRES
  DRAWPIXEL_LOWRES_400 %1
  dec dword[esp]
  jnz near %%next_left_pixel
  pop eax

%%middle_bit:
  mov eax,[p_picture] ;number of pixels left
  shr eax,4           ;/16 to get number of full rasters
  push eax

  jmp near %%next_raster

%%draw_raster:
  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA ;also increments source address
  %rep 16
    CALC_COL_LOWRES
    PUSHPIXEL_LOWRES %1
  %endrep
  POPRASTER_LOWRES %1
%%next_raster:
  dec dword[esp]
  jns near %%draw_raster

  mov eax,[p_picture]
  and eax,15          ;extra pixels
  jz near %%finished_picture
  mov dword[esp],eax
  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA ;also increments source address

%%next_right_pixel:
  CALC_COL_LOWRES
  DRAWPIXEL_LOWRES_400 %1
  dec dword[esp]
  jnz near %%next_right_pixel

%%finished_picture:
  pop eax  ;correct stack, discard 0-counter

%%border_2:
  mov ebx,[p_border2]

  shr ebx,4  ;pixels of right border/16
  DRAW_BORDER_400 %1,ebx

  mov ebx,[p_border2]
  and ebx,15
  jz near %%finished_border_2
  mov eax,[pal0]
%%extra_pix_border_2:
  DRAWPIXEL_LOWRES_400 %1
  dec ebx
  jnz near %%extra_pix_border_2
%%finished_border_2:

  mov [_draw_dest_ad],edi  ;save dest ad - new!!!!!

  RESTORE_REGS

  leave
%endmacro

draw_scanline_8_lowres_pixelwise_400: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_LOWRES_PIXELWISE_400 1
  ret

draw_scanline_16_lowres_pixelwise_400: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_LOWRES_PIXELWISE_400 2
  ret

draw_scanline_24_lowres_pixelwise_400: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_LOWRES_PIXELWISE_400 3
  ret

draw_scanline_32_lowres_pixelwise_400: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_LOWRES_PIXELWISE_400 4
  ret
