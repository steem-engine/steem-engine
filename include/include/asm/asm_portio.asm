%ifdef WIN32
segment .text public align=1 class=code use32
%else
segment .text
%endif

global _port_out,_port_in    ;DOS OUT command

_port_out:    ;long PORT at esp+4, long VAL at esp+8 - send byte
  mov edx,[esp+4]
  mov eax,[esp+8]
  out dx,al
  ret

_port_in:    ;long PORT at esp+4,
  mov edx,[esp+4]
  in al,dx
  and eax,0xff
  ret

