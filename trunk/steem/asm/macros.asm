%macro SAVE_REGS 0
  mov [esp-4],ebx
  mov [esp-8],edi
  mov [esp-12],esi
  sub esp,12
%endmacro

%macro RESTORE_REGS 0
  mov ebx,[esp+8]
  mov edi,[esp+4]
  mov esi,[esp]
  add esp,12
%endmacro

%macro RESTORE_AND_RET 0
  RESTORE_REGS
  ret
%endm

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

%else

%macro cextern 1-*
%rep %0
extern %1
%define _%1 %1
%rotate 1
%endrep
%endmacro

%endif


%ifdef _VC_BUILD
%define UNDERSCORES 1
%endif


%ifdef UNDERSCORES

%macro cglobal 1-*
%rep %0
%define %1 _%1
global _%1
%rotate 1
%endrep
%endmacro

%else

%define cglobal global

%endif

