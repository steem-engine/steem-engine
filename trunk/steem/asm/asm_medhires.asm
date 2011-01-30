; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! MEDIUM RES !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
cglobal draw_scanline_8_medres_pixelwise,draw_scanline_16_medres_pixelwise
cglobal draw_scanline_24_medres_pixelwise,draw_scanline_32_medres_pixelwise

cglobal draw_scanline_8_medres_pixelwise_400,draw_scanline_16_medres_pixelwise_400
cglobal draw_scanline_24_medres_pixelwise_400,draw_scanline_32_medres_pixelwise_400


%macro GET_SCREEN_DATA_INTO_REGS_AND_INC_SA_MEDRES 0
  sub esi,4
  mov ebx,[esi]
%endmacro

; new version by Ant 14/1/2001
%macro CALC_COL_MEDRES 0
  test bh,80h
  jz short %%ccm_0x
%%ccm_1x:
  add ebx,ebx
  jnc short %%ccm_10
%%ccm_11:
  mov eax,[pal3]
  jmp short %%ccm_finished
%%ccm_10:
  mov eax,[pal2]
  jmp short %%ccm_finished
%%ccm_0x:
  add ebx,ebx
  jnc short %%ccm_00
%%ccm_01:
  mov eax,[pal1]
  jmp short %%ccm_finished
%%ccm_00:
  mov eax,[pal0]
%%ccm_finished:

%endmacro

%macro DRAWPIXEL_MEDRES_400 2 ;bpp, carelessly
  %if %1==1
    mov [edi],al
    mov [edi+ecx],al
  %elif %1==2
    mov [edi],ax
    mov [edi+ecx],ax
  %elif %1==3
    %if %2
    ;carelessly
      mov [edi],eax
      mov [edi+ecx],eax
    %else
      mov [edi],ax
      mov [edi+ecx],ax
      bswap eax
      mov [edi+2],ah
      mov [edi+ecx+2],ah
      bswap eax
    %endif
  %elif %1==4
    mov [edi],eax
    mov [edi+ecx],eax
  %endif
  add edi,%1
%endmacro

%macro DRAW_SCANLINE_PIXELWISE_MEDRES 1 ;bpp
  push ebp
  mov ebp,esp

  SAVE_REGS
  GET_START 1,160
  GET_PC_DRAW_ADDR_INTO_EDI

  mov ebx,[p_border1]
  shr ebx,4  ;pixels of left border/16
  DRAW_BORDER %1,ebx,1

  mov ebx,[p_border1]
  and ebx,15
  jz %%finished_border_1
  mov eax,[pal0]
%%extra_pix_border_1:
  DRAWPIXEL %1,1
  DRAWPIXEL %1,0
  dec ebx
  jnz %%extra_pix_border_1
%%finished_border_1:

  mov ebx,[p_picture]
  add ebx,ebx
  mov [p_picture],ebx  ;double the low res pixels to get medres pixels

  or ebx,ebx
  jz near %%border_2

  mov eax,16
  mov ecx,[p_hscroll]  ;how many pixels to skip
  sub eax,ecx
  ;eax now contains how many pixels to draw in the first raster
  cmp eax,ebx           ;how many pixels to draw
  jl %%no_more_reduction
  mov eax,ebx           ;there's less pixels to draw than remaining in the first raster
%%no_more_reduction:

  cmp eax,16
  je %%middle_bit

  push eax ;store counter
  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA_MEDRES ;also increments source address
  shl ebx,cl
   ;skip initial pixels

  pop ecx ;number of pixels to draw in first raster in ecx
  sub [p_picture],ecx   ;reduce future number of pixels to draw

%%next_left_pixel:
  CALC_COL_MEDRES
  DRAWPIXEL %1,0
  dec ecx
  jnz %%next_left_pixel

%%middle_bit:
  mov ecx,[p_picture] ;number of pixels left
  shr ecx,4           ;/16 to get number of full rasters

  jmp near %%next_raster
%%draw_raster:
  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA_MEDRES ;also increments source address
  %rep 15
    CALC_COL_MEDRES
    DRAWPIXEL %1,1    ;draw pixel carelessly
  %endrep
  CALC_COL_MEDRES
  DRAWPIXEL %1,0    ;draw pixel carefully

%%next_raster:
  dec ecx
  jns near %%draw_raster

  mov ecx,[p_picture]
  and ecx,15          ;extra pixels
  jz %%border_2
  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA_MEDRES ;also increments source address

%%next_right_pixel:
  CALC_COL_MEDRES
  DRAWPIXEL %1,0
  dec ecx
  jnz near %%next_right_pixel

%%border_2:
  mov ebx,[p_border2]
  shr ebx,4  ;pixels of right border/16
  DRAW_BORDER %1,ebx,1

  mov ebx,[p_border2]
  and ebx,15
  jz %%finished_border_2
  mov eax,[pal0]
%%extra_pix_border_2:
  DRAWPIXEL %1,1
  DRAWPIXEL %1,0
  dec ebx
  jnz %%extra_pix_border_2
%%finished_border_2:

  mov [_draw_dest_ad],edi  ;save dest ad - new!!!!!

  RESTORE_REGS

  leave
%endmacro


draw_scanline_8_medres_pixelwise:
  DRAW_SCANLINE_PIXELWISE_MEDRES 1
  ret

draw_scanline_16_medres_pixelwise:
  DRAW_SCANLINE_PIXELWISE_MEDRES 2
  ret

draw_scanline_24_medres_pixelwise:
  DRAW_SCANLINE_PIXELWISE_MEDRES 3
  ret

draw_scanline_32_medres_pixelwise:
  DRAW_SCANLINE_PIXELWISE_MEDRES 4
  ret

; ---------------------------------------------------------------------------
; ------------------------------ _400 --------------------------------------
; ---------------------------------------------------------------------------
%macro PUSHPIXEL_MEDRES 2 ;bpp,carelessly
  %if %1==1
    push ax
    mov [edi],al
    inc edi
  %elif %1==2
    push ax
    mov [edi],ax
    add edi,2
  %elif %1==3
    push eax      ;save colour on stack
    %if %2
      mov [edi],eax ;write first line
    %else
      mov [edi],ax  ;write first line
      bswap eax
      mov [edi+2],ah
    %endif
    add edi,3
  %elif %1==4
    push eax      ;save colour on stack
    mov [edi],ax  ;write first line
    bswap eax
    mov [edi+2],ah
    add edi,4
  %endif
%endmacro

%macro POPRASTER_MEDRES 1 ;bpp
  lea eax,[edi+ecx-16 * %1] ;look at next line
  %if %1==1
    %assign n 15
    %rep 15
      pop bx
      mov [eax+n],bl
      %assign n n-1
    %endrep
    pop bx
    mov [eax],bl
  %elif %1==2
    %assign n 15
    %rep 15
      pop word[eax+2*n]
      %assign n n-1
    %endrep
    pop word[eax]
  %elif %1==3
    %assign n 15
    %rep 16
      pop ebx
      mov [eax+3*n],bx
      bswap ebx
      mov [eax+(3*n)+2],bh
      %assign n n-1
    %endrep
  %elif %1==4
    %assign n 15
    %rep 15
      pop dword[eax+4*n]
      %assign n n-1
    %endrep
    pop dword[eax]
  %endif
%endmacro

%macro DRAW_SCANLINE_MEDRES_400 1 ;bpp
  push ebp
  mov ebp,esp

  SAVE_REGS
  GET_START 0,160
  GET_PC_DRAW_ADDR_INTO_EDI_400
	DRAW_BORDER_400 %1,[border1]
  mov edx,[picture]
  jmp near %%next_raster
%%draw_raster:
  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA_MEDRES
  %rep 15
    CALC_COL_MEDRES
    PUSHPIXEL_MEDRES %1,1 ;push pixel carelessly
  %endrep
  CALC_COL_MEDRES
  PUSHPIXEL_MEDRES %1,0  ;push pixel carefully
  POPRASTER_MEDRES %1
%%next_raster:
  dec edx  ;edx is counter here cos ecx is offset
  jns near %%draw_raster
	DRAW_BORDER_400 %1,[border2]
  RESTORE_REGS

  leave
%endmacro

draw_scanline_8_medres_400: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_MEDRES_400 1
  ret

draw_scanline_16_medres_400: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_MEDRES_400 2
  ret

draw_scanline_24_medres_400: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_MEDRES_400 3
  ret

draw_scanline_32_medres_400: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_MEDRES_400 4
  ret


%macro DRAW_SCANLINE_MEDRES_PIXELWISE_400 1 ;bpp
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
  jz %%finished_border_1
  mov eax,[pal0]
%%extra_pix_border_1:
  DRAWPIXEL_LOWRES_400 %1
  dec ebx
  jnz %%extra_pix_border_1
%%finished_border_1:

  mov ebx,[p_picture]
  add ebx,ebx
  mov [p_picture],ebx
  or ebx,ebx
  jz near %%border_2

  mov eax,16
  sub eax,[p_hscroll]  ;how many pixels to skip
  ;eax now contains how many pixels to draw in the first raster
  cmp eax,ebx           ;how many pixels to draw
  jl %%no_more_reduction
  mov eax,ebx           ;there's less pixels to draw than remaining in the first raster
%%no_more_reduction:

  cmp eax,16
  je near %%middle_bit

  sub [p_picture],eax   ;reduce future number of pixels to draw
  mov [counter],eax ;store counter

  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA_MEDRES ;also increments source address
  mov ecx,[p_hscroll]          ;copy to ecx ready for shl
  shl ebx,cl
    ;skip initial pixels

  mov ecx,[_draw_line_length] ; offset for second line drawing offset
  push dword[counter]

%%next_left_pixel:
  CALC_COL_MEDRES
  DRAWPIXEL_MEDRES_400 %1,0
  dec dword[esp]
  jnz near %%next_left_pixel
  pop eax

%%middle_bit:
  mov eax,[p_picture] ;number of pixels left
  shr eax,4           ;/16 to get number of full rasters
  push eax

  jmp near %%next_raster

%%draw_raster:
  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA_MEDRES
  %rep 15
    CALC_COL_MEDRES
    PUSHPIXEL_MEDRES %1,1
  %endrep
  CALC_COL_MEDRES
  PUSHPIXEL_MEDRES %1,0
  POPRASTER_MEDRES %1
%%next_raster:
  dec dword[esp]
  jns near %%draw_raster

  mov eax,[p_picture]
  and eax,15          ;extra pixels
  jz %%finished_picture
  mov dword[esp],eax
  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA_MEDRES ;also increments source address

%%next_right_pixel:
  CALC_COL_MEDRES
  DRAWPIXEL_MEDRES_400 %1,0
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

draw_scanline_8_medres_pixelwise_400: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_MEDRES_PIXELWISE_400 1
  ret

draw_scanline_16_medres_pixelwise_400: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_MEDRES_PIXELWISE_400 2
  ret

draw_scanline_24_medres_pixelwise_400: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_MEDRES_PIXELWISE_400 3
  ret

draw_scanline_32_medres_pixelwise_400: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_MEDRES_PIXELWISE_400 4
  ret


; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! HIGH RES !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
cglobal draw_scanline_8_hires,draw_scanline_16_hires
cglobal draw_scanline_24_hires,draw_scanline_32_hires

%macro GET_BLACK_AND_WHITE_INTO_EBX_EDX 0
  xor ebx,ebx
  xor edx,edx
  mov eax,[_STpal]
  and eax,1
  jnz short %%ebx_black
  not ebx
  jmp short %%end
%%ebx_black:
  not edx
%%end:
%endmacro

%macro GET_SCREEN_DATA_INTO_REGS_AND_INC_SA_HIRES 0
  ; Get ST screen memory into regs
  sub esi,2
  mov ax,[esi]
%endmacro

%macro DRAW_BORDER_HIRES 2 ;bpp,how_many
  mov eax,%2

  jmp short %%next
%%for:
  %if %1==1
    mov [edi],dx
    %assign n 1
    %rep 7
      mov [edi+2*n],dx
      %assign n n+1
    %endrep
  %elif %1==2
    mov [edi],edx
    %assign n 1
    %rep 7
      mov [edi+4*n],edx
      %assign n n+1
    %endrep
  %elif %1==3
    ; want a total of 16*3=12*4 bytes of edx
    mov [edi],edx
    %assign n 1
    %rep 11
      mov [edi+4*n],edx
      %assign n n+1
    %endrep
  %elif %1==4
    mov [edi],edx
    %assign n 1
    %rep 15
      mov [edi+4*n],edx
      %assign n n+1
    %endrep
  %endif
  add edi,16*%1
%%next:
  dec eax
  jns short %%for
%endmacro

%macro DRAWPIXEL_HIRES 2 ;bpp,carelessly
  %if %1==1
    jnc short %%no_carry
    mov [edi],bl
    jmp short %%endtest
%%no_carry:
    mov [edi],dl
%%endtest:
    inc edi
  %elif %1==2
    jnc short %%no_carry
    mov [edi],bx
    jmp short %%endtest
%%no_carry:
    mov [edi],dx
%%endtest:
    add edi,2
  %elif %1==3
    %if %2 ;carelessly
      jnc short %%no_carry
      mov [edi],ebx
      jmp short %%endtest
%%no_carry:
      mov [edi],edx
    %else
      jnc short %%no_carry
      mov [edi],bx
      mov [edi+2],bl
      jmp short %%endtest
%%no_carry:
      mov [edi],dx
      mov [edi+2],dl
    %endif
%%endtest:
    add edi,3
  %elif %1==4
    jnc short %%no_carry
    mov [edi],ebx
    jmp short %%endtest
%%no_carry:
    mov [edi],edx
%%endtest:
    add edi,4
  %endif
%endmacro

%macro DRAW_SCANLINE_HIRES 1 ;bpp
  push ebp
  mov ebp,esp

  SAVE_REGS
  GET_START 0,80
  GET_PC_DRAW_ADDR_INTO_EDI
  GET_BLACK_AND_WHITE_INTO_EBX_EDX
  DRAW_BORDER_HIRES %1,[border1]
  mov ecx,[picture]
  jmp near %%next_raster
%%draw_raster:
  GET_SCREEN_DATA_INTO_REGS_AND_INC_SA_HIRES
  %rep 15
    add ax,ax
    DRAWPIXEL_HIRES %1,1
  %endrep
  add ax,ax
  DRAWPIXEL_HIRES %1,0
%%next_raster:
  dec ecx
  jns near %%draw_raster
  DRAW_BORDER_HIRES %1,[border2]
  RESTORE_REGS

  leave
%endmacro



draw_scanline_8_hires: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_HIRES 1
  ret

draw_scanline_16_hires: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_HIRES 2
  ret

draw_scanline_24_hires: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_HIRES 3
  ret

draw_scanline_32_hires: ; Proc C border1:DWORD,picture:DWORD,border2:DWORD
  DRAW_SCANLINE_HIRES 4
  ret



