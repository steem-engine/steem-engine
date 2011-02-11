%include "macros.asm"

%ifndef MINGW

%ifdef WIN32
segment .bss public align=4 class=bss use32
%else
segment .bss
%endif

%else
segment .bss
%endif
; ---------------------------------------------------------------------------
osd_x:                           resd 1
osd_y:                           resd 1
osd_xi:                          resd 1
osd_w:                           resd 1
osd_yi:                          resd 1
osd_h:                           resd 1
; ---------------------------------------------------------------------------
cextern draw_mem,draw_line_length,rgb555
; ---------------------------------------------------------------------------
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
cglobal osd_draw_char_clipped_8,osd_draw_char_clipped_16
cglobal osd_draw_char_clipped_24,osd_draw_char_clipped_32

%define RECT_LEFT 0
%define RECT_TOP 4
%define RECT_RIGHT 8
%define RECT_BOTTOM 12

cliphandle: ; Proc lpcliprect:DWORD  ;;;,lpxy:DWORD;;;

  mov ecx,osd_x           ;[ecx] faster than [osd_x]?
  mov dword[ecx+8],0
  mov dword[ecx+12],32
  mov dword[ecx+16],0
  mov dword[ecx+20],32

  mov edx,[esp+4]         ;lpcliprect

  mov eax,[edx+RECT_LEFT] ;x-min
  sub eax,[ecx]           ;osd_x
  jl short check_right
  ; off left
  cmp eax,32
  jl short not_too_far_off_left
  mov eax,-32
  ret                     ;error

not_too_far_off_left:
  mov [ecx+8],eax         ;osd_xi
  sub [ecx+12],eax        ;osd_w
  mov eax,[edx+RECT_LEFT]
  mov [ecx],eax           ;set osd_x to clipped position

check_right:
  mov eax,[ecx]           ;osd_x
  add eax,32              ;right edge
  sub eax,[edx+RECT_RIGHT]
  jle short check_top

  cmp eax,32
  jl short not_too_far_off_right
  mov eax,-32
  ret                     ;error

not_too_far_off_right:
  sub [ecx+12],eax        ;osd_w

check_top:
  mov eax,[edx+RECT_TOP]  ;y-min
  sub eax,[ecx+4]         ;osd_y
  jl short check_bottom
  ; off top
  cmp eax,32
  jl short not_too_far_off_top
  mov eax,-32
  ret                     ;error

not_too_far_off_top:
  mov [ecx+16],eax        ;start further down
  sub [ecx+20],eax        ;draw less lines
  mov eax,[edx+RECT_TOP]
  mov [ecx+4],eax         ;set osd_y to clipped position

check_bottom:
  mov eax,[ecx+4]         ;osd_y
  add eax,32              ;bottom edge
  sub eax,[edx+RECT_BOTTOM]
  jle short done_clipping

  cmp eax,32
  jl short not_too_far_off_bottom
  mov eax,-32
  ret                     ;error

not_too_far_off_bottom:
  sub [ecx+20],eax

done_clipping:
  xor eax,eax
  ret

; ---------------------------------------------------------------------------
%define source_data ebp+8
%define dest_screen ebp+12
%define xp          ebp+16
%define yp          ebp+20
%define line_length ebp+24
%define col         ebp+28
%define char_h			ebp+32
%define lpcliprect  ebp+36

%define d_ad        ebp-4
%define end_ad      ebp-8
%define advance     ebp-12

%macro OSD_DRAW_CHAR_CLIPPED 2 ;bpp
  enter 12,0

; source data format: 1 long mask, 1 long data x 32

  pushad

  mov eax,[xp]
  mov [osd_x],eax
  mov eax,[yp]
  mov [osd_y],eax

  push dword[lpcliprect]
  call cliphandle
  add esp,4

;  mov dword[osd_xi],0  ;tes
;  mov dword[osd_w],32  ;   ting
;  mov dword[osd_yi],0  ;        without
;  mov dword[osd_h],32  ;                cliphandle
;  mov eax,0

  cmp eax,0
  jne near %%the_end           ;error exit

  mov ebx,[osd_x]
  mov eax,[osd_y]         ;y position
  mul dword[line_length]  ;times bytes per line of dest
  %if %1==2
    add ebx,ebx
  %elif %1==3
    add ebx,ebx
    add ebx,[osd_x]
  %elif %1==4
    shl ebx,2
  %endif

  add eax,ebx
  add eax,[dest_screen]     ;eax now has pixel address
  mov [d_ad],eax            ;dest address
  mov edi,eax               ;use edi as draw pointer

  mov eax,[osd_yi]          ;number of lines to skip in pattern
  shl eax,3                 ;*8 bytes per line
  add eax,[source_data]     ;yeilds source data
  mov esi,eax               ;esi is source

  mov ecx,[line_length]
  mov ebx,[osd_w]           ;number of pixels per line
  %if %1==2
    add ebx,ebx
  %elif %1==3
    add ebx,ebx
    add ebx,[osd_w]
  %elif %1==4
    shl ebx,2
  %endif
  sub ecx,ebx               ;number of bytes to advance after end of line
  mov [advance],ecx         ;into variable "advance"

  %if %2==0
    mov ebx,[col]             ;use ebx as colour
  %endif

%%draw_line_clipped:
  mov ecx,[osd_xi]          ;how much to knock off start
  mov eax,[esi]             ;mask
  shl eax,cl                ;skip the first few
  mov edx,[esi+4]           ;data
  shl edx,cl                ;skip the first few

  add esi,8

  mov ecx,[osd_w]  ;counter
%%draw_pixel:
  add eax,eax
  jnc short %%eax_no_carry
  %if %2
    push eax
    %if %1==2
      call darken_pixel_to_eax_16
    %elif %1==3
      call darken_pixel_to_eax_24
    %elif %1==4
      call darken_pixel_to_eax_32
    %endif
  %endif
  %if %1==1
    mov byte[edi],0
  %elif %1==2
    %if %2==0
      mov word[edi],0
    %else
      mov word[edi],ax
    %endif
  %elif %1==3
    %if %2==0
      mov word[edi],0
      mov byte[edi+2],0
    %else
      mov word[edi],ax
      bswap eax
      mov byte[edi+2],ah
    %endif
  %elif %1==4
    %if %2==0
      mov dword[edi],0
    %else
      mov dword[edi],eax
    %endif
  %endif
  %if %2
    pop eax
  %endif

%%eax_no_carry:

  add edx,edx
  jnc short %%edx_no_carry
  %if %2
    push eax
    %if %1==2
      call lighten_pixel_to_ebx_16
    %elif %1==3
      call lighten_pixel_to_ebx_24
    %elif %1==4
      call lighten_pixel_to_ebx_32
    %endif
  %endif
  %if %1==1
    mov [edi],bl
  %elif %1==2
    mov [edi],bx
  %elif %1==3
    mov [edi],bx
    bswap ebx
    mov [edi+2],bh
    bswap ebx
  %elif %1==4
    mov [edi],ebx
  %endif
  %if %2
    pop eax
  %endif

%%edx_no_carry:

  %if %1==1
    inc edi
  %else
    add edi,%1
  %endif
  loop %%draw_pixel

  add edi,dword[advance] ;point to start of next line
  dec dword[char_h]
  jnle short %%draw_line_clipped

%%the_end:
  popad
  leave

%endmacro


osd_draw_char_clipped_8: ; Proc C source_data:DWORD, dest_screen:DWORD,xp:DWORD,yp:DWORD,line_length:DWORD,col:DWORD,lpcliprect:DWORD
  OSD_DRAW_CHAR_CLIPPED 1,0
  ret

osd_draw_char_clipped_16: ; Proc C source_data:DWORD, dest_screen:DWORD,xp:DWORD,yp:DWORD,line_length:DWORD,col:DWORD,lpcliprect:DWORD
  OSD_DRAW_CHAR_CLIPPED 2,0
  ret

osd_draw_char_clipped_24: ; Proc C source_data:DWORD, dest_screen:DWORD,xp:DWORD,yp:DWORD,line_length:DWORD,col:DWORD,lpcliprect:DWORD
  OSD_DRAW_CHAR_CLIPPED 3,0
  ret

osd_draw_char_clipped_32: ; Proc C source_data:DWORD, dest_screen:DWORD,xp:DWORD,yp:DWORD,line_length:DWORD,col:DWORD,lpcliprect:DWORD
  OSD_DRAW_CHAR_CLIPPED 4,0
  ret

; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
cglobal osd_draw_char_8,osd_draw_char_16
cglobal osd_draw_char_24,osd_draw_char_32

%macro GENERATE_LABEL 4
osd_draw_char_pixel_%1_%2_%3_%4:
%endmacro

darken_pixel_to_eax_16:
  mov ax,[edi]

  xor ebx,ebx

  mov bx,ax
  and bx,0000000000011111b
  cmp bx,0x8
  jg  short .no_overflow_b_16
  xor bx,bx
  jmp short .done_b_16
.no_overflow_b_16:
  sub bx,0x8
.done_b_16:
  and ax,1111111111100000b
  or ax,bx

  test word[_rgb555],1
  jnz short .is555

  mov bx,ax
  and bx,0000011111100000b
  cmp bx,0x200
  jg  short .no_overflow_g_16_565
  xor bx,bx
  jmp short .done_g_16_565
.no_overflow_g_16_565:
  sub bx,0x200
.done_g_16_565:
  and ax,1111100000011111b
  or  ax,bx

  mov bx,ax
  and bx,1111100000000000b
  cmp ebx,0x4000
  jg  short .no_overflow_r_16_565
  xor bx,bx
  jmp short .done_r_16_565
.no_overflow_r_16_565:
  sub bx,0x4000
.done_r_16_565:
  and ax,0000011111111111b
  or ax,bx
  ret

.is555:
  mov bx,ax
  and bx,0000001111100000b
  cmp bx,0x100
  jg  short .no_overflow_g_16_555
  xor bx,bx
  jmp short .done_g_16_555
.no_overflow_g_16_555:
  sub bx,0x100
.done_g_16_555:
  and ax,1111110000011111b
  or  ax,bx

  mov bx,ax
  and bx,0111110000000000b
  cmp ebx,0x2000
  jg  short .no_overflow_r_16_555
  xor bx,bx
  jmp short .done_r_16_555
.no_overflow_r_16_555:
  sub bx,0x2000
.done_r_16_555:
  and ax,0000011111111111b
  or ax,bx
  ret

%macro DARKEN_PIXEL_24_32 1
  %if %1==3
    mov ax,[edi]
    bswap eax
    mov ah,[edi+2]
    bswap eax
  %else
    mov eax,[edi]
  %endif

  mov ebx,eax
  and ebx,0x0000ff
  cmp ebx,0x80
  jg  short %%no_overflow_b
  xor ebx,ebx
  jmp short %%done_b
%%no_overflow_b:
  sub ebx,0x80
%%done_b:
  and eax,0xffff00
  or eax,ebx

  mov ebx,eax
  and ebx,0x00ff00
  cmp ebx,0x8000
  jg  short %%no_overflow_g
  xor ebx,ebx
  jmp short %%done_g
%%no_overflow_g:
  sub ebx,0x8000
%%done_g:
  and eax,0xff00ff
  or eax,ebx

  mov ebx,eax
  and ebx,0xff0000
  cmp ebx,0x800000
  jg  short %%no_overflow_r
  xor ebx,ebx
  jmp short %%done_r
%%no_overflow_r:
  sub ebx,0x800000
%%done_r:
  and eax,0x00ffff
  or eax,ebx
%endmacro

darken_pixel_to_eax_24:
  DARKEN_PIXEL_24_32 3
  ret

darken_pixel_to_eax_32:
  DARKEN_PIXEL_24_32 4
  ret


lighten_pixel_to_ebx_16:
  mov bx,[edi]

  xor eax,eax

  mov ax,bx
  and ax,0000000000011111b
  cmp ax,0000000000011111b-0x10
  jl  short .no_overflow_b_16
  mov ax,0000000000011111b
  jmp short .done_b_16
.no_overflow_b_16:
  add ax,0x10
.done_b_16:
  and bx,1111111111100000b
  or bx,ax

  test word[_rgb555],1
  jnz short .is555

  mov ax,bx
  and ax,0000011111100000b
  cmp ax,0000011111100000b-0x400
  jl  short .no_overflow_g_16_565
  mov ax,0000011111100000b
  jmp short .done_g_16_565
.no_overflow_g_16_565:
  add ax,0x400
.done_g_16_565:
  and bx,1111100000011111b
  or  bx,ax

  mov ax,bx
  and ax,1111100000000000b
  cmp eax,1111100000000000b-0x8000
  jl  short .no_overflow_r_16_565
  mov ax,1111100000000000b
  jmp short .done_r_16_565
.no_overflow_r_16_565:
  add ax,0x8000
.done_r_16_565:
  and bx,0000011111111111b
  or bx,ax

  ret

.is555:
  mov ax,bx
  and ax,0000001111100000b
  cmp ax,0000001111100000b-0x100
  jl  short .no_overflow_g_16_555
  mov ax,0000001111100000b
  jmp short .done_g_16_555
.no_overflow_g_16_555:
  add ax,0x100
.done_g_16_555:
  and bx,1111110000011111b
  or  bx,ax

  mov ax,bx
  and ax,0111110000000000b
  cmp ax,0111110000000000b-0x2000
  jl  short .no_overflow_r_16_555
  mov ax,0111110000000000b
  jmp short .done_r_16_555
.no_overflow_r_16_555:
  add ax,0x2000
.done_r_16_555:
  and bx,0000011111111111b
  or bx,ax

  ret

%macro LIGHTEN_PIXEL_24_32 1
  %if %1==3
    mov bx,[edi]
    bswap ebx
    mov bh,[edi+2]
    bswap ebx
  %else
    mov ebx,[edi]
  %endif

  mov eax,ebx
  and eax,0x0000ff
  cmp eax,0x0000ff-0x80
  jl  short %%no_overflow_b
  mov eax,0x0000ff
  jmp short %%done_b
%%no_overflow_b:
  add eax,0x80
%%done_b:
  and ebx,0xffff00
  or ebx,eax

  mov eax,ebx
  and eax,0x00ff00
  cmp eax,0x00ff00-0x8000
  jl  short %%no_overflow_g
  mov eax,0x00ff00
  jmp short %%done_g
%%no_overflow_g:
  add eax,0x8000
%%done_g:
  and ebx,0xff00ff
  or ebx,eax

  mov eax,ebx
  and eax,0xff0000
  cmp eax,0xff0000-0x800000
  jl  short %%no_overflow_r
  mov eax,0xff0000
  jmp short %%done_r
%%no_overflow_r:
  add eax,0x800000
%%done_r:
  and ebx,0x00ffff
  or ebx,eax
%endmacro

lighten_pixel_to_ebx_24:
  LIGHTEN_PIXEL_24_32 3
  ret

lighten_pixel_to_ebx_32:
  LIGHTEN_PIXEL_24_32 4
  ret

%macro OSD_DRAW_CHAR 3 ;bpp,rainbow,transparent
  %if %1==1 && %3
    enter 16,0
  %else
    enter 12,0
  %endif

; source data format: 1 long mask, 1 long data x 32

  pushad

  mov ebx,[xp]             ;x position
  mov eax,[yp]             ;y position
  mul dword[line_length]   ;times bytes per line of dest
  %if %1==2
    add ebx,ebx
  %elif %1==3
    add ebx,ebx
    add ebx,[xp]
  %elif %1==4
    shl ebx,2
  %endif

  add eax,ebx
  add eax,[dest_screen]    ;eax now has pixel address
  mov [d_ad],eax           ;dest address
  mov edi,eax              ;use edi as draw pointer
  mov esi,[source_data]    ;esi is source

  mov ecx,[line_length]
  sub ecx,31*%1            ;number of bytes to advance after end of line
  mov [advance],ecx        ;into variable "advance"

  mov ebx,eax              ;eax is source address
  mov eax,[line_length]
	mul dword[char_h]
  add eax,ebx              ;eax points to end of char as dest address
  mov [end_ad],eax

  %if %3==0 || %1==1
    %if %2
      mov edx,[col]
      mov eax,[edx]          ;get current colour
      mov ebx,eax
      %if %1==2
        call advance_colour_16
      %elif %1==3 || %1==4
        call advance_colour_24
      %endif
      mov [edx],ebx ;update colour for next time
      mov ebx,eax   ;use ebx as colour
    %else
      mov ebx,[col]   ;use ebx as colour
    %endif
    xor eax,eax              ;use eax to store 0
  %endif
  %if %1==1 && %3
    mov dword[ebp-16],0
  %endif

%%draw_line:
  mov ecx,[esi]            ;mask
  mov edx,[esi+4]          ;data
  add esi,8

  %if %1==1 && %3
    xor dword[ebp-16],1
    jnz short %%dont_check
    add ecx,ecx
    add edx,edx
%%dont_check:
  %endif

  %assign n 1
  %assign numreps 32
  %if %1==1 && %3
    %assign numreps 16
  %endif
  %rep numreps
    add ecx,ecx
    %if %1==1 && %3 && n>1
      add ecx,ecx
    %endif
    jnc short .ecx_no_carry
    %if %3
      %if %1==2
        call darken_pixel_to_eax_16
      %elif %1==3
        call darken_pixel_to_eax_24
      %elif %1==4
        call darken_pixel_to_eax_32
      %endif
    %endif
    %if %1==1
      mov [edi],al
    %elif %1==2
      mov [edi],ax
    %elif %1==3
      mov [edi],ax
      mov [edi+2],al
    %elif %1==4
      mov [edi],eax
    %endif
.ecx_no_carry:
    add edx,edx
    %if %1==1 && %3 && n>1
      add edx,edx
    %endif
    jnc short .edx_no_carry
    %if %3
      %if %1==2
        call lighten_pixel_to_ebx_16
      %elif %1==3
        call lighten_pixel_to_ebx_24
      %elif %1==4
        call lighten_pixel_to_ebx_32
      %endif
    %endif
    %if %1==1
      mov [edi],bl
    %elif %1==2
      mov [edi],bx
    %elif %1==3
      mov [edi],bx
      bswap ebx
      mov [edi+2],bh
      bswap ebx
    %elif %1==4
      mov [edi],ebx
    %endif
.edx_no_carry:

    %if n<32
      GENERATE_LABEL %1,%2,%3,n
      %assign n n+1
      %if %1==1 && %3
        add edi,2
      %else
        add edi,%1
      %endif
    %endif
  %endrep

  %if %2
    %if %1==2
      call advance_colour_16
    %elif %1==3 || %1==4
      call advance_colour_24
    %endif
  %endif

  %if %1==1 && %3
    test dword[ebp-16],1
    jnz short %%dont_return
    sub edi,2
%%dont_return:
  %endif

  add edi,[advance] ;point to start of next line
  cmp edi,[end_ad]
  jl near %%draw_line

  popad
  leave
%endmacro

osd_draw_char_8: ; Proc C source_data:DWORD, dest_screen:DWORD,x:DWORD,y:DWORD,line_length:DWORD,col:DWORD
  OSD_DRAW_CHAR 1,0,0
  ret

osd_draw_char_16: ; Proc C source_data:DWORD, dest_screen:DWORD,x:DWORD,y:DWORD,line_length:DWORD,col:DWORD
  OSD_DRAW_CHAR 2,0,0
  ret

osd_draw_char_24: ; Proc C source_data:DWORD, dest_screen:DWORD,x:DWORD,y:DWORD,line_length:DWORD,col:DWORD
  OSD_DRAW_CHAR 3,0,0
  ret

osd_draw_char_32: ; Proc C source_data:DWORD, dest_screen:DWORD,x:DWORD,y:DWORD,line_length:DWORD,col:DWORD
  OSD_DRAW_CHAR 4,0,0
  ret
; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
cglobal osd_draw_char_transparent_8,osd_draw_char_transparent_16
cglobal osd_draw_char_transparent_24,osd_draw_char_transparent_32

cglobal osd_draw_char_clipped_transparent_8,osd_draw_char_clipped_transparent_16
cglobal osd_draw_char_clipped_transparent_24,osd_draw_char_clipped_transparent_32

osd_draw_char_transparent_8: ; Proc C source_data:DWORD, dest_screen:DWORD,x:DWORD,y:DWORD,line_length:DWORD,col:DWORD
;  OSD_DRAW_CHAR 1,0,1
  ret

osd_draw_char_transparent_16: ; Proc C source_data:DWORD, dest_screen:DWORD,x:DWORD,y:DWORD,line_length:DWORD,col:DWORD
  OSD_DRAW_CHAR 2,0,1
  ret

osd_draw_char_transparent_24: ; Proc C source_data:DWORD, dest_screen:DWORD,x:DWORD,y:DWORD,line_length:DWORD,col:DWORD
  OSD_DRAW_CHAR 3,0,1
  ret

osd_draw_char_transparent_32: ; Proc C source_data:DWORD, dest_screen:DWORD,x:DWORD,y:DWORD,line_length:DWORD,col:DWORD
  OSD_DRAW_CHAR 4,0,1
  ret


osd_draw_char_clipped_transparent_8:
;  OSD_DRAW_CHAR_CLIPPED 1,1
  ret

osd_draw_char_clipped_transparent_16:
  OSD_DRAW_CHAR_CLIPPED 2,1
  ret

osd_draw_char_clipped_transparent_24:
  OSD_DRAW_CHAR_CLIPPED 3,1
  ret

osd_draw_char_clipped_transparent_32:
  OSD_DRAW_CHAR_CLIPPED 4,1
  ret


%undef source_data
%undef dest_screen
%undef xp
%undef yp
%undef line_length
%undef col
%undef lpcliprect
%undef char_h

%undef d_ad
%undef end_ad
%undef advance
; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
cglobal osd_blueize_line_16_555,osd_blueize_line_16_565
cglobal osd_blueize_line_24,osd_blueize_line_32

%define x1    ebp+8
%define y     ebp+12
%define w     ebp+16

osd_blueize_line_16_555: ; Proc C x1:DWORD, y:DWORD, w:DWORD
  push ebp
  mov ebp,esp

  pushad
  mov eax,[y]
  mul dword[_draw_line_length]
  add eax,[_draw_mem]
  add eax,[x1]
  add eax,[x1]

  mov ecx,[w]
  or ecx,ecx
  js short enough_16_555
  jnp short dont_do_first_pixel_16_555

;color is 0Rrr rrGg gggB bbbb
;ie.       red=0x7c green=0x3e     blue=0x1f

  mov bx,[eax]
  and bx,7BC0h
  shr bx,1
  or bx,0018h
  mov [eax],bx
  add eax,2

dont_do_first_pixel_16_555:
  shr ecx,1
  jz short enough_16_555

blueize_16_555_2pix:
  mov ebx,[eax]
  and ebx,7BC07BC0h
  shr ebx,1
  or ebx,00180018h
  mov [eax],ebx
  add eax,4
  loop blueize_16_555_2pix

enough_16_555:
  popad
  leave
  ret


osd_blueize_line_16_565: ; Proc C x1:DWORD, y:DWORD, w:DWORD
  push ebp
  mov ebp,esp

  pushad
  mov eax,[y]
  mul dword[_draw_line_length]
  add eax,[_draw_mem]
  add eax,[x1]
  add eax,[x1]

  mov ecx,[w]
  or ecx,ecx
  js short enough_16_565
  jnp short dont_do_first_pixel_16_565

  mov bx,[eax]
  and bx,0F7C0h
  shr bx,1
  or bx,0018h
  mov [eax],bx
  add eax,2

dont_do_first_pixel_16_565:
  shr ecx,1
  jz short enough_16_565

blueize_16_565_2pix:
  mov ebx,[eax]
  and ebx,0F7C0F7C0h
  shr ebx,1
  or ebx,00180018h
  mov [eax],ebx
  add eax,4
  loop blueize_16_565_2pix

enough_16_565:
  popad
  leave
  ret


osd_blueize_line_24: ; Proc C x1:DWORD, y:DWORD, w:DWORD
  push ebp
  mov ebp,esp

  pushad
  mov eax,[y]
  mul dword[_draw_line_length]
  add eax,[_draw_mem]

  mov ebx,[x1]
  add eax,ebx
  add eax,ebx
  add eax,ebx

  mov ecx,[w]
  cmp ecx,0
  jle short enough_24

blueize_24:
  mov byte[eax],0D3h
  inc eax
  mov bx,[eax]
  and bx,0FEFEh
  shr bx,1
  mov [eax],bx
  add eax,2
  loop blueize_24

enough_24:
  popad
  leave
  ret

osd_blueize_line_32: ; Proc C x1:DWORD, y:DWORD, w:DWORD
  push ebp
  mov ebp,esp

  pushad
  mov eax,[y]
  mul dword[_draw_line_length]
  add eax,[_draw_mem]

  mov ebx,[x1]
  lea eax,[eax+ebx*4]

  mov ecx,[w]
  cmp ecx,0
  jle short enough_32

blueize_32:
  mov ebx,[eax]
  and ebx,0FEFE00h
  shr ebx,1
  mov bl,0D3h
  mov [eax],ebx
  add eax,4
  loop blueize_32

enough_32:
  popad
  leave
  ret

%undef x1
%undef y
%undef w
; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
cglobal osd_black_box_8,osd_black_box_16,osd_black_box_24,osd_black_box_32;

%define dest_screen ebp+8
%define x           ebp+12
%define y           ebp+16
%define w           ebp+20
%define h           ebp+24
%define line_length ebp+28
%define col 				  ebp+32

%macro DRAWBLACKPIXEL 1
  %if %1==1
    mov [eax],dl
  %elif %1==2
    mov [eax],dx
  %elif %1==3
    mov [eax],dx
    mov [eax+2],dl
  %elif %1==4
    mov [eax],edx
  %endif
%endmacro

%macro DRAW_BLACK_BOX 1
  push ebp
  mov ebp,esp

  pushad

  dec word[w]
  js short %%finished_black_rect		; jump signed (negative)
  jz short %%finished_black_rect		

	; get w offset into ebx
  mov ax,[w]       
	dec ax						;to start of right line
  cwde							;sign extend
  mov ebx,eax				;to ebx
  %if %1==2
    add ebx,ebx
  %elif %1==3
    add ebx,ebx
    add ebx,eax
  %elif %1==4
    shl ebx,2
  %endif

	; get x position into ecx
  mov ax,[x]        ;x position
  cwde              ;sign extend
  mov ecx,eax       ;to ecx
  %if %1==2
    add ecx,ecx
  %elif %1==3
    add ecx,ecx
    add ecx,eax
  %elif %1==4
    shl ecx,2
  %endif

  mov ax,[y]              ;y position
  cwde                    ;to long
  mul dword[line_length]  ;times bytes per line of dest
  add eax,ecx				  ;add x position
  add eax,[dest_screen]   ;eax now has pixel address

  xor edx,edx				  ;edx contains colour

  dec word[h]
  js short %%finished_black_rect		; jump signed (negative)

	mov edi,eax					;save address
  mov cx,[w]
%%top_rect_line:
  DRAWBLACKPIXEL %1
  %if %1==1
    inc eax
  %else
    add eax,%1
  %endif
  dec cx
  ja short %%top_rect_line
  add edi,[line_length]

  dec word[h]
  jz short %%do_bottom_line
  js short %%finished_black_rect		; jump signed (negative)

  mov cx,[h]
%%middle_lines:
	mov eax,edi
  DRAWBLACKPIXEL %1
  add eax,ebx
  DRAWBLACKPIXEL %1
  add edi,[line_length]
  dec cx
  ja short %%middle_lines

%%do_bottom_line:
	mov eax,edi
	mov cx,[w]
%%bottom_rect_line:
  DRAWBLACKPIXEL %1
  %if %1==1
    inc eax
  %else
    add eax,%1
  %endif
  dec cx
  ja short %%bottom_rect_line

%%finished_black_rect:
  popad
  leave
%endmacro

osd_black_box_8: ; Proc C dest_screen:DWORD,x:WORD,y:WORD,w:WORD,h:WORD,line_length:DWORD
  DRAW_BLACK_BOX 1
  ret

osd_black_box_16: ; Proc C dest_screen:DWORD,x:WORD,y:WORD,w:WORD,h:WORD,line_length:DWORD
  DRAW_BLACK_BOX 2
  ret

osd_black_box_24: ; Proc C dest_screen:DWORD,x:WORD,y:WORD,w:WORD,h:WORD,line_length:DWORD
  DRAW_BLACK_BOX 3
  ret

osd_black_box_32: ; Proc C dest_screen:DWORD,x:WORD,y:WORD,w:WORD,h:WORD,line_length:DWORD
  DRAW_BLACK_BOX 4
  ret

%undef dest_screen
%undef x
%undef y
%undef w
%undef h
%undef line_length 
%undef col

