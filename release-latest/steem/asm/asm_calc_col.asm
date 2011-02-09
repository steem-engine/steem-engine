; FILE: asm_calc_col.asm
; MODULE: asm_draw
; DESCRIPTION: Routine to get the PC colour data from ST screen data

;%macro CALC_COL_LOWRES_SHORT 0
;  mov eax,pal0
;
;  test dx,dx
;  jns short %%Label1
;  add eax,32
;%%Label1:
;  test bx,bx
;  jns short %%Label2
;  add eax,8
;%%Label2:
;  add edx,edx
;  jnc short %%Label3
;  add eax,16
;%%Label3:
;  add ebx,ebx
;  jnc short %%Label4
;  add eax,4
;%%Label4:
;  mov eax,[eax]
;%endmacro

%macro CALC_COL_LOWRES 0
  %if CALC_COL_METHOD==0
    CALC_COL_LOWRES_SHORT
  %elif CALC_COL_METHOD==1
    ;new version by Ant, 24/1/2001

    test dx,dx
    jns short %%ccl_0xxx
  %%ccl_1xxx:
    test bx,bx
    jns short %%ccl_1x0x
  %%ccl_1x1x:
    add edx,edx
    jnc short %%ccl_101x
  %%ccl_111x:
    add ebx,ebx
    jnc short %%ccl_1110
  %%ccl_1111:
    mov eax,[pal15]
    jmp short %%ccl_finished
  %%ccl_1110:
    mov eax,[pal14]
    jmp short %%ccl_finished
  %%ccl_101x:
    add ebx,ebx
    jnc short %%ccl_1010
  %%ccl_1011:
    mov eax,[pal11]
    jmp short %%ccl_finished
  %%ccl_1010:
    mov eax,[pal10]
    jmp short %%ccl_finished
  %%ccl_1x0x:
    add edx,edx
    jnc short %%ccl_100x
  %%ccl_110x:
    add ebx,ebx
    jnc short %%ccl_1100
  %%ccl_1101:
    mov eax,[pal13]
    jmp short %%ccl_finished
  %%ccl_1100:
    mov eax,[pal12]
    jmp short %%ccl_finished
  %%ccl_100x:
    add ebx,ebx
    jnc short %%ccl_1000
  %%ccl_1001:
    mov eax,[pal9]
    jmp short %%ccl_finished
  %%ccl_1000:
    mov eax,[pal8]
    jmp short %%ccl_finished
  %%ccl_0xxx:
    test bx,bx
    jns short %%ccl_0x0x
  %%ccl_0x1x:
    add edx,edx
    jnc short %%ccl_001x
  %%ccl_011x:
    add ebx,ebx
    jnc short %%ccl_0110
  %%ccl_0111:
    mov eax,[pal7]
    jmp short %%ccl_finished
  %%ccl_0110:
    mov eax,[pal6]
    jmp short %%ccl_finished
  %%ccl_001x:
    add ebx,ebx
    jnc short %%ccl_0010
  %%ccl_0011:
    mov eax,[pal3]
    jmp short %%ccl_finished
  %%ccl_0010:
    mov eax,[pal2]
    jmp short %%ccl_finished
  %%ccl_0x0x:
    add edx,edx
    jnc short %%ccl_000x
  %%ccl_010x:
    add ebx,ebx
    jnc short %%ccl_0100
  %%ccl_0101:
    mov eax,[pal5]
    jmp short %%ccl_finished
  %%ccl_0100:
    mov eax,[pal4]
    jmp short %%ccl_finished
  %%ccl_000x:
    add ebx,ebx
    jnc short %%ccl_0000
  %%ccl_0001:
    mov eax,[pal1]
    jmp short %%ccl_finished
  %%ccl_0000:
    mov eax,[pal0]
  %%ccl_finished:

  %elif CALC_COL_METHOD==2
    ;new version by Ant 30/1/2001
    ;needs colour nibble in order 3120 instead of 3210.

    xor eax,eax   ;zero counter
    cmp dx,8000h
    rcl eax,1
    cmp bx,8000h
    rcl eax,1
    add edx,edx
    rcl eax,1
    add ebx,ebx
    rcl eax,3
    mov eax,[eax + pal0]

  %elif CALC_COL_METHOD==3
    ;new version by Ant, 20/1/2001

    test dh,80h
    jz short %%ccl_0xxx
  %%ccl_1xxx:
    test bh,80h
    jz short %%ccl_1x0x
  %%ccl_1x1x:
    add edx,edx
    jnc short %%ccl_101x
  %%ccl_111x:
    add ebx,ebx
    jnc short %%ccl_1110
  %%ccl_1111:
    mov eax,[pal15]
    jmp near %%ccl_finished
  %%ccl_1110:
    mov eax,[pal14]
    jmp near %%ccl_finished
  %%ccl_101x:
    add ebx,ebx
    jnc short %%ccl_1010
  %%ccl_1011:
    mov eax,[pal11]
    jmp near %%ccl_finished
  %%ccl_1010:
    mov eax,[pal10]
    jmp short %%ccl_finished
  %%ccl_1x0x:
    add edx,edx
    jnc short %%ccl_100x
  %%ccl_110x:
    add ebx,ebx
    jnc short %%ccl_1100
  %%ccl_1101:
    mov eax,[pal13]
    jmp short %%ccl_finished
  %%ccl_1100:
    mov eax,[pal12]
    jmp short %%ccl_finished
  %%ccl_100x:
    add ebx,ebx
    jnc short %%ccl_1000
  %%ccl_1001:
    mov eax,[pal9]
    jmp short %%ccl_finished
  %%ccl_1000:
    mov eax,[pal8]
    jmp short %%ccl_finished
  %%ccl_0xxx:
    test bh,80h
    jz short %%ccl_0x0x
  %%ccl_0x1x:
    add edx,edx
    jnc short %%ccl_001x
  %%ccl_011x:
    add ebx,ebx
    jnc short %%ccl_0110
  %%ccl_0111:
    mov eax,[pal7]
    jmp short %%ccl_finished
  %%ccl_0110:
    mov eax,[pal6]
    jmp short %%ccl_finished
  %%ccl_001x:
    add ebx,ebx
    jnc short %%ccl_0010
  %%ccl_0011:
    mov eax,[pal3]
    jmp short %%ccl_finished
  %%ccl_0010:
    mov eax,[pal2]
    jmp short %%ccl_finished
  %%ccl_0x0x:
    add edx,edx
    jnc short %%ccl_000x
  %%ccl_010x:
    add ebx,ebx
    jnc short %%ccl_0100
  %%ccl_0101:
    mov eax,[pal5]
    jmp short %%ccl_finished
  %%ccl_0100:
    mov eax,[pal4]
    jmp short %%ccl_finished
  %%ccl_000x:
    add ebx,ebx
    jnc short %%ccl_0000
  %%ccl_0001:
    mov eax,[pal1]
    jmp short %%ccl_finished
  %%ccl_0000:
    mov eax,[pal0]
  %%ccl_finished:

  %endif
%endmacro
