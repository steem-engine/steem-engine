; ---------------------------------------------------------------------------
; INFO
;
; Registers
;   eax ecx edx   ebx esi edi ebp esp
;   the first three are always available
;   the other five have to be saved and restored after use.

; Procedures
;   Procedure_Name:
;     enter [LOCAL_BYTES],0
;     ...
;     leave
;     ret
;
;   if LOCAL_BYTES==0 then can replace enter with faster:
;     push ebp
;     mov ebp,esp
;
;   first parameter = ebp+8 (last parameter pushed), second is ebp+12
;   all params are at least 32-bits for 32-bit processors
;   local variable space is at ebp-1 to ebp-LOCAL_BYTES

; When used all imported variables begin with an _, but the declaration
; in the cextern should not have one
; ---------------------------------------------------------------------------
; TODO
;
; Make counter local? (for _400 and scrolled)
; ---------------------------------------------------------------------------
%include "macros.asm"

%define CALC_COL_METHOD 3
; 0 for jump & add & move                       <- This okay
; 1 for jump jump jump & move, using test ax,ax <- about as fast as 3
; 2 for no jumps                                <- Still slower than 3
; 3 like 1 but using test instead of or.        <- This is the best
;---------------------------------------------------------------------------
%ifndef MINGW

%ifdef WIN32
segment .bss public align=4 class=bss use32
%else
segment .bss
%endif

%else
segment .bss
%endif

; Internal variables
end_adr:                         resd 1  ; end ST address
counter:                         resd 1

pal0:                            resd 1
pal1:                            resd 1
pal2:                            resd 1
pal3:                            resd 1
pal4:                            resd 1
pal5:                            resd 1
pal6:                            resd 1
pal7:                            resd 1
pal8:                            resd 1
pal9:                            resd 1
pal10:                           resd 1
pal11:                           resd 1
pal12:                           resd 1
pal13:                           resd 1
pal14:                           resd 1
pal15:                           resd 1

; Steem variables

cextern draw_mem,draw_line_length,draw_dest_ad,STpal,rgb555
cextern Mem_End,Mem_End_minus_1,Mem_End_minus_2,Mem_End_minus_4,mem_len
cextern shifter_draw_pointer,shifter_hscroll
;---------------------------------------------------------------------------
%ifndef MINGW

%ifdef WIN32
segment .text public align=1 class=code use32
%else
segment .text
%endif

%else
segment .text
%endif
; ---------------------------------------------------------------------------
; ----------------------- Palette Convert Macros ----------------------------
; ---------------------------------------------------------------------------
cglobal Get_PCpal

Get_PCpal:
  mov eax,pal0
  ret


; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! LOW RES !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
cglobal draw_scanline_8_lowres_pixelwise,draw_scanline_16_lowres_pixelwise
cglobal draw_scanline_24_lowres_pixelwise,draw_scanline_32_lowres_pixelwise

cglobal draw_scanline_8_lowres_pixelwise_dw,draw_scanline_16_lowres_pixelwise_dw
cglobal draw_scanline_24_lowres_pixelwise_dw,draw_scanline_32_lowres_pixelwise_dw

cglobal draw_scanline_8_lowres_pixelwise_400,draw_scanline_16_lowres_pixelwise_400
cglobal draw_scanline_24_lowres_pixelwise_400,draw_scanline_32_lowres_pixelwise_400


%define border1 ebp+8
%define picture ebp+12
%define border2 ebp+16

%define p_border1 ebp+8
%define p_picture ebp+12
%define p_border2 ebp+16
%define p_hscroll ebp+20


%macro GET_START 2 ; doubleflag,increase
  mov eax,[_shifter_draw_pointer]
  and eax,0ffffffh

  mov esi,[_Mem_End]
  sub esi,eax

  add eax,%2 ;increase

%%keep_checking:
  cmp eax,[_mem_len]
  jle short %%end

  add esi,[_mem_len]  ;loop to low mem
  sub eax,[_mem_len]
  jmp %%keep_checking

;  mov ebx,[picture]
;%if %1 ;doubleflag
;  add ebx,ebx
;%endif
;  add [border1],ebx
;  xor ebx,ebx
;  mov [picture],ebx

%%end:
%endmacro

%macro GET_PC_DRAW_ADDR_INTO_EDI 0
  mov edi,[_draw_dest_ad]
%endmacro

%macro GET_SCREEN_DATA_INTO_REGS 0
  ; Get ST screen memory into regs
  mov edx,[esi-8]
  mov ebx,[esi-4]
%endmacro

%macro GET_SCREEN_DATA_INTO_REGS_AND_INC_SA 0
  sub esi,8
  mov edx,[esi]
  mov ebx,[esi+4]
%endmacro

; -------------------------------------------------------------------------
%macro DRAWPIXEL 2 ;bpp,carelessly
  %if %1==1
    mov [edi],al
    inc edi
  %elif %1==2
    mov [edi],ax      ; colour into screen addr
    add edi,2         ; inc screen addr
  %elif %1==3
    %if %2 ;carelessly
      mov [edi],eax    ; write colour to PC screen + bonus byte!
    %else
      mov [edi],ax    ; write colour to PC screen
      bswap eax
      mov [edi+2],ah   ;last byte of RGB colour
      bswap eax       ;restore eax in case we're drawing a border
    %endif
    add edi,3
  %elif %1==4
    mov [edi],eax     ; colour into screen addr
    add edi,4         ; next screen addr
  %endif
%endmacro

%macro DRAW_BORDER 3 ;bpp,how_many,doubleflag
  mov eax,[pal0]
%ifnidni %2,ebx
  mov ebx,%2 ;how_many
%endif
%if %3 ;doubleflag
  add ebx,ebx
%endif

  jmp short %%next
%%for:
  %if %1==1
    mov [edi],ax
    %assign n 1
    %rep 7
      mov [edi+2*n],ax
      %assign n n+1
    %endrep
  %elif %1==2
    mov [edi],eax
    %assign n 1
    %rep 7
      mov [edi+4*n],eax
      %assign n n+1
    %endrep
  %elif %1==3
    mov [edi],eax
    %assign n 1
    %rep 15
      mov [edi+3*n],eax
      %assign n n+1
    %endrep
    mov [edi+3*15],ax
    bswap eax
    mov [edi+3*15+2],ah
    bswap eax
  %elif %1==4
    mov [edi],eax
    %assign n 1
    %rep 15
      mov [edi+4*n],eax
      %assign n n+1
    %endrep
  %endif
  add edi,16*%1
%%next:
  dec ebx
  jns short %%for

%endmacro

%include "asm_calc_col.asm"
%include "asm_lowres.asm"
%include "asm_medhires.asm"

; --------------------------------------------------------------------------
; %include "asm_draw_scrolled.asm"
; ---------------------------------------------------------------------------

