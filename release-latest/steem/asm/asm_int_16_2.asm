segment .text

global _int_16_2

_int_16_2:    
  mov ah,2
  int 16h
  ret
