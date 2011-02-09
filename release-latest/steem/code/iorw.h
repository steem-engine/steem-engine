#ifdef IN_EMU
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)
#endif

extern DWORD get_shifter_draw_pointer(int);

extern "C" {

BYTE ASMCALL io_read_b(MEM_ADDRESS);
WORD ASMCALL io_read_w(MEM_ADDRESS);
DWORD ASMCALL io_read_l(MEM_ADDRESS);
void ASMCALL io_write_b(MEM_ADDRESS,BYTE);
void ASMCALL io_write_w(MEM_ADDRESS,WORD);
void ASMCALL io_write_l(MEM_ADDRESS,LONG);

}

EXT bool io_word_access INIT(0);


#define BUS_JAM_TIME(t) INSTRUCTION_TIME_ROUND(t)
//#define BUS_JAM_TIME(t) INSTRUCTION_TIME(t)


#undef EXT
#undef INIT

