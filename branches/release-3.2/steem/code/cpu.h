#ifdef IN_EMU
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)
#endif

#define BOMBS_BUS_ERROR 2
#define BOMBS_ADDRESS_ERROR 3
#define BOMBS_ILLEGAL_INSTRUCTION 4
#define BOMBS_DIVISION_BY_ZERO 5
#define BOMBS_CHK 6
#define BOMBS_TRAPV 7
#define BOMBS_PRIVILEGE_VIOLATION 8
#define BOMBS_TRACE_EXCEPTION 9
#define BOMBS_LINE_A 10
#define BOMBS_LINE_F 11

enum exception_action{EA_READ=0,EA_WRITE,EA_FETCH,EA_INST};

class m68k_exception
{
public:
  int bombs;
  MEM_ADDRESS _pc;
  MEM_ADDRESS crash_address;
  MEM_ADDRESS address;
  WORD _sr,_ir;
  exception_action action;

  m68k_exception() {}
  ~m68k_exception() {}

  void init(int,exception_action,MEM_ADDRESS);
  void crash();
};

EXT void exception(int,exception_action,MEM_ADDRESS);

// GCC try/catch fix:
EXT m68k_exception ExceptionObject;
EXT jmp_buf *pJmpBuf INIT(NULL);

// This is exactly the same as the C++ exceptions except you mustn't return or break
// out of the exception block without executing the END_M68K_EXCEPTION. If you have
// to you can just add pJmpBuf=oldpJmpBuf before the return/break and it will work.

// This could be fixed by making a wrapper class for jmp_buf so it will call the
// destructor when it goes out of scope, but GCC seems flakey on that sort of thing.

#define TRY_M68K_EXCEPTION jmp_buf temp_excep_jump;jmp_buf *oldpJmpBuf=pJmpBuf;pJmpBuf=&temp_excep_jump;if (setjmp(temp_excep_jump)==0){
#define CATCH_M68K_EXCEPTION }else{
#define END_M68K_EXCEPTION }pJmpBuf=oldpJmpBuf;

#define areg (r+8)

EXT BYTE  m68k_peek(MEM_ADDRESS ad);
EXT WORD  m68k_dpeek(MEM_ADDRESS ad);
EXT LONG  m68k_lpeek(MEM_ADDRESS ad);

EXT void cpu_routines_init();

EXT int m68k_divu_cycles INIT(124),m68k_divs_cycles INIT(140); // +4 for overall time

#define PC32 ( (pc&0xffffff)|(pc_high_byte) )
#define FOUR_MEGS 0x400000
#define FOURTEEN_MEGS 0xE00000
#define MEM_FIRST_WRITEABLE 8

#define SR_IPL (BIT_a+BIT_9+BIT_8)
#define SR_IPL_7 (BIT_a+BIT_9+BIT_8)
#define SR_IPL_6 (BIT_a+BIT_9       )
#define SR_IPL_5 (BIT_a+      BIT_8 )
#define SR_IPL_4 (BIT_a             )
#define SR_IPL_3 (      BIT_9+BIT_8 )
#define SR_IPL_2 (      BIT_9       )
#define SR_IPL_1 (            BIT_8 )
#define SR_IPL_0                    0

#define SR_C 1
#define SR_V 2
#define SR_Z 4
#define SR_N 8
#define SR_X 16
#define SR_SUPER BIT_d
#define SR_USER_BYTE 31
#define SR_TRACE (WORD(BIT_f))

#define SUPERFLAG ((bool)(sr&SR_SUPER))
#define SSP ((MEM_ADDRESS)((SUPERFLAG) ? r[15]:other_sp))
#define USP ((MEM_ADDRESS)((SUPERFLAG) ? other_sp:r[15]))
#define lpSSP ((MEM_ADDRESS*)((SUPERFLAG) ? &(r[15]):&other_sp))
#define lpUSP ((MEM_ADDRESS*)((SUPERFLAG) ? &other_sp:&(r[15])))

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#if defined(IN_EMU)

inline void m68k_poke(MEM_ADDRESS ad,BYTE x);
inline void m68k_dpoke(MEM_ADDRESS ad,WORD x);
inline void m68k_lpoke(MEM_ADDRESS ad,LONG x);

#define INSTRUCTION_TIME(t) {cpu_cycles-=(t);}
#define INSTRUCTION_TIME_ROUND(t) {INSTRUCTION_TIME(t); cpu_cycles&=-4;}
#define FETCH_TIMING {INSTRUCTION_TIME(4); cpu_cycles&=-4;}

#ifdef _DEBUG_BUILD

/*
This is for the "stop on change to user mode" and "stop on next program run" options.
Whenever an instruction that can change SR is executed we check for SUPER change. If
a change to user mode has happened then stop_on_user_change is set to 2. Then when we
check for interrupts (after the current instruction) we see if an interrupt has happened
and if it hasn't we stop. If it has happened we return stop_on_user_change to 1 and wait
for the next change to user mode (when the interrupt has finished).
*/

#define CHECK_STOP_ON_USER_CHANGE  \
            if (stop_on_user_change){ \
              if ((debug_old_sr & SR_SUPER) && (sr & SR_SUPER)==0) stop_on_user_change=2;  \
            }

#define CHECK_STOP_USER_MODE_NO_INTR \
            if (stop_on_user_change==2){  \
              if ((sr & SR_SUPER)==0){  \
                if (runstate==RUNSTATE_RUNNING){ \
                  runstate=RUNSTATE_STOPPING; \
                  SET_WHY_STOP( HEXSl(old_pc,6)+": Switch to user mode" ) \
                } \
                if (stop_on_next_program_run==2) stop_new_program_exec(); \
              }else{ \
                stop_on_user_change=1; \
              }  \
            }

#ifndef _RELEASE_BUILD

MEM_ADDRESS pc_rel_stop_on_ref=0;

#define PC_RELATIVE_MONITOR(ad) \
  if (pc_rel_stop_on_ref){ \
    if ((ad & 0xffffff)==pc_rel_stop_on_ref){ \
      if (runstate==RUNSTATE_RUNNING){ \
        runstate=RUNSTATE_STOPPING; \
        runstate_why_stop=HEXSl(old_pc,6)+": Referencing "+HEXSl(pc_rel_stop_on_ref,6); \
      } \
    } \
  }

#else
#define PC_RELATIVE_MONITOR(ad)
#endif

#else
#define CHECK_STOP_ON_USER_CHANGE
#define CHECK_STOP_USER_MODE_NO_INTR
#define debug_check_break_on_irq(irq)
#define PC_RELATIVE_MONITOR(ad)
#endif

#define M68K_PERFORM_RTE(checkints)             \
            SET_PC(m68k_lpeek(r[15]+2));        \
            sr=m68k_dpeek(r[15]);r[15]+=6;      \
            sr&=SR_VALID_BITMASK;               \
            DETECT_CHANGE_TO_USER_MODE;         \
            DETECT_TRACE_BIT;                   \
            checkints;                          \

#define DEST_IS_REGISTER ((ir&BITS_543)<=BITS_543_001)
#define DEST_IS_MEMORY ((ir&BITS_543)>BITS_543_001)

#define SOURCE_IS_REGISTER_OR_IMMEDIATE ((ir & BITS_543)<=BITS_543_001 || ((ir&b00111111)==b00111100) )


WORD*lpfetch,*lpfetch_bound;
bool prefetched_2=false;
WORD prefetch_buf[2];

#define PREFETCH_SET_PC                       \
  prefetched_2=false; /*will have prefetched 1 word*/ \
  prefetch_buf[0]=*lpfetch;               \
  lpfetch+=MEM_DIR;  /*let's not cause exceptions here*/

#define EXTRA_PREFETCH                    \
  prefetch_buf[1]=*lpfetch;              \
  prefetched_2=true;

#define EXTRA_PREFETCH_IF_TO_MEM \
  if(DEST_IS_MEMORY){           \
    EXTRA_PREFETCH               \
  }

#define FETCH_W(dest_word)              \
  if(prefetched_2){                     \
    dest_word=prefetch_buf[0];            \
    prefetch_buf[0]=prefetch_buf[1];       \
    prefetched_2=false;                           \
  }else{ /* if(prefetched==1) */             \
    dest_word=prefetch_buf[0];                \
    prefetch_buf[0]=*lpfetch;              \
  }                                            \
  lpfetch+=MEM_DIR;                             \
  if(lpfetch MEM_GE lpfetch_bound)exception(BOMBS_BUS_ERROR,EA_FETCH,pc);

#define SET_PC(ad)        \
    pc=ad;                               \
    pc_high_byte=pc & 0xff000000;     \
    pc&=0xffffff;                    \
    lpfetch=lpDPEEK(0);          /*Default to instant bus error when fetch*/   \
    lpfetch_bound=lpDPEEK(0);         \
                                        \
    if (pc>=himem){                                                       \
      if (pc<MEM_IO_BASE){           \
        if (pc>=MEM_EXPANSION_CARTRIDGE){                                \
          if (pc>=0xfc0000){                                                   \
            if (tos_high && pc<(0xfc0000+192*1024)){         \
              lpfetch=lpROM_DPEEK(pc-0xfc0000); \
              lpfetch_bound=lpROM_DPEEK(192*1024);         \
            }                                                                          \
          }else if (cart){                                                          \
            lpfetch=lpCART_DPEEK(pc-MEM_EXPANSION_CARTRIDGE); \
            lpfetch_bound=lpCART_DPEEK(128*1024);         \
          }                                                                          \
        }else if(pc>=rom_addr){                                                      \
          if (pc<(0xe00000 + 256*1024)){         \
            lpfetch=lpROM_DPEEK(pc-0xe00000); \
            lpfetch_bound=lpROM_DPEEK(256*1024);         \
          }                                                                          \
        }                            \
      }else{   \
        if (pc>=0xff8240 && pc<0xff8260){         \
          lpfetch=lpPAL_DPEEK(pc-0xff8240); \
          lpfetch_bound=lpPAL_DPEEK(64+PAL_EXTRA_BYTES);         \
        }                              \
      }                                                                           \
    }else{                                                                         \
      lpfetch=lpDPEEK(pc); \
      lpfetch_bound=lpDPEEK(mem_len+(MEM_EXTRA_BYTES/2));         \
    }                                         \
    PREFETCH_SET_PC

#define IOACCESS_FLAGS_MASK  0xFFFFFFC0
#define IOACCESS_NUMBER_MASK 0x0000003F

#define IOACCESS_FLAG_FOR_CHECK_INTRS BIT_6
#define IOACCESS_FLAG_PSG_BUS_JAM_R BIT_7
#define IOACCESS_FLAG_PSG_BUS_JAM_W BIT_8
#define IOACCESS_FLAG_DO_BLIT BIT_9
#define IOACCESS_FLAG_FOR_CHECK_INTRS_MFP_CHANGE BIT_10
#define IOACCESS_FLAG_DELAY_MFP BIT_11
#define IOACCESS_INTERCEPT_OS BIT_12
#define IOACCESS_INTERCEPT_OS2 BIT_13

#ifdef ENABLE_LOGFILE
#define IOACCESS_DEBUG_MEM_WRITE_LOG BIT_14
MEM_ADDRESS debug_mem_write_log_address;
int debug_mem_write_log_bytes;
#endif

#define STOP_INTS_BECAUSE_INTERCEPT_OS bool(ioaccess & (IOACCESS_INTERCEPT_OS | IOACCESS_INTERCEPT_OS2))

void m68k_interrupt(MEM_ADDRESS);  //non-address or bus error interrupt
void change_to_user_mode();
void change_to_supervisor_mode();

bool cpu_stopped=false,m68k_do_trace_exception;

signed int compare_buffer;


#define m68k_SET_DEST_B_TO_ADDR        \
  abus&=0xffffff;                                   \
  if(abus>=MEM_IO_BASE){               \
    if(SUPERFLAG){                        \
      ioaccess&=IOACCESS_FLAGS_MASK; \
      ioaccess|=1;                     \
      ioad=abus;                        \
      m68k_dest=&iobuffer;               \
      DWORD_B_0(&iobuffer)=io_read_b(abus);        \
    }else exception(BOMBS_BUS_ERROR,EA_WRITE,abus);             \
  }else if(abus>=himem){                               \
    if(mmu_confused){                               \
      mmu_confused_set_dest_to_addr(1,true);           \
    }else if(abus>=FOUR_MEGS){                                                \
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);                               \
    }else{                                                        \
      m68k_dest=&iobuffer;                             \
    }                                       \
  }else{                                            \
    DEBUG_CHECK_WRITE_B(abus); \
    if (SUPERFLAG && abus>=MEM_FIRST_WRITEABLE){                             \
      m68k_dest=lpPEEK(abus);           \
    }else if(abus>=MEM_START_OF_USER_AREA){ \
      m68k_dest=lpPEEK(abus);           \
    }else{                                      \
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);       \
    }                                           \
  }

#define m68k_SET_DEST_W_TO_ADDR        \
  abus&=0xffffff;                                   \
  if(abus&1){                                      \
    exception(BOMBS_ADDRESS_ERROR,EA_WRITE,abus);    \
  }else if(abus>=MEM_IO_BASE){               \
    if(SUPERFLAG){                        \
      ioaccess&=IOACCESS_FLAGS_MASK; \
      ioaccess|=2;                     \
      ioad=abus;                        \
      m68k_dest=&iobuffer;               \
      *((WORD*)&iobuffer)=io_read_w(abus);        \
    }else exception(BOMBS_BUS_ERROR,EA_WRITE,abus);                                \
  }else if(abus>=himem){                               \
    if(mmu_confused){                               \
      mmu_confused_set_dest_to_addr(2,true);           \
    }else if(abus>=FOUR_MEGS){                                                \
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);                               \
    }else{                                                        \
      m68k_dest=&iobuffer;                             \
    }                                       \
  }else{                               \
    DEBUG_CHECK_WRITE_W(abus);  \
    if(SUPERFLAG && abus>=MEM_FIRST_WRITEABLE){                       \
      m68k_dest=lpDPEEK(abus);           \
    }else if(abus>=MEM_START_OF_USER_AREA){ \
      m68k_dest=lpDPEEK(abus);           \
    }else{                                      \
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);       \
    }                                           \
  }

#define m68k_SET_DEST_L_TO_ADDR        \
  abus&=0xffffff;                                   \
  if(abus&1){                                      \
    exception(BOMBS_ADDRESS_ERROR,EA_WRITE,abus);    \
  }else if(abus>=MEM_IO_BASE){               \
    if(SUPERFLAG){                        \
      ioaccess&=IOACCESS_FLAGS_MASK; \
      ioaccess|=4;                     \
      ioad=abus;                         \
      m68k_dest=&iobuffer;               \
      iobuffer=io_read_l(abus);        \
    }else exception(BOMBS_BUS_ERROR,EA_WRITE,abus);                                 \
  }else if(abus>=himem){                               \
    if(mmu_confused){                               \
      mmu_confused_set_dest_to_addr(4,true);           \
    }else if(abus>=FOUR_MEGS){                                                \
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);                               \
    }else{                                                        \
      m68k_dest=&iobuffer;                             \
    }                                       \
  }else{                               \
    DEBUG_CHECK_WRITE_L(abus);  \
    if(SUPERFLAG && abus>=MEM_FIRST_WRITEABLE){                       \
      m68k_dest=lpLPEEK(abus);           \
    }else if(abus>=MEM_START_OF_USER_AREA){ \
      m68k_dest=lpLPEEK(abus);           \
    }else{                                      \
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);       \
    }                                           \
  }


#define m68k_SET_DEST_B(addr)           \
  abus=addr;                            \
  m68k_SET_DEST_B_TO_ADDR;

#define m68k_SET_DEST_W(addr)           \
  abus=addr;                            \
  m68k_SET_DEST_W_TO_ADDR;

#define m68k_SET_DEST_L(addr)           \
  abus=addr;                            \
  m68k_SET_DEST_L_TO_ADDR;






#define PC_RELATIVE_PC pc
//(old_pc+2)
//(old_dpc+2)


#define m68k_GET_SOURCE_B m68k_jump_get_source_b[(ir&BITS_543)>>3]()
#define m68k_GET_SOURCE_W m68k_jump_get_source_w[(ir&BITS_543)>>3]()
#define m68k_GET_SOURCE_L m68k_jump_get_source_l[(ir&BITS_543)>>3]()

#define m68k_GET_SOURCE_B_NOT_A m68k_jump_get_source_b_not_a[(ir&BITS_543)>>3]()
#define m68k_GET_SOURCE_W_NOT_A m68k_jump_get_source_w_not_a[(ir&BITS_543)>>3]()
#define m68k_GET_SOURCE_L_NOT_A m68k_jump_get_source_l_not_a[(ir&BITS_543)>>3]()

#define m68k_GET_DEST_B m68k_jump_get_dest_b[(ir&BITS_543)>>3]()
#define m68k_GET_DEST_W m68k_jump_get_dest_w[(ir&BITS_543)>>3]()
#define m68k_GET_DEST_L m68k_jump_get_dest_l[(ir&BITS_543)>>3]()

#define m68k_GET_DEST_B_NOT_A m68k_jump_get_dest_b_not_a[(ir&BITS_543)>>3]()
#define m68k_GET_DEST_W_NOT_A m68k_jump_get_dest_w_not_a[(ir&BITS_543)>>3]()
#define m68k_GET_DEST_L_NOT_A m68k_jump_get_dest_l_not_a[(ir&BITS_543)>>3]()

#define m68k_GET_DEST_B_NOT_A_OR_D m68k_jump_get_dest_b_not_a_or_d[(ir&BITS_543)>>3]()
#define m68k_GET_DEST_W_NOT_A_OR_D m68k_jump_get_dest_w_not_a_or_d[(ir&BITS_543)>>3]()
#define m68k_GET_DEST_L_NOT_A_OR_D m68k_jump_get_dest_l_not_a_or_d[(ir&BITS_543)>>3]()

#define m68k_GET_DEST_B_NOT_A_FASTER_FOR_D m68k_jump_get_dest_b_not_a_faster_for_d[(ir&BITS_543)>>3]()
#define m68k_GET_DEST_W_NOT_A_FASTER_FOR_D m68k_jump_get_dest_w_not_a_faster_for_d[(ir&BITS_543)>>3]()
#define m68k_GET_DEST_L_NOT_A_FASTER_FOR_D m68k_jump_get_dest_l_not_a_faster_for_d[(ir&BITS_543)>>3]()

#define m68k_CONDITION_TEST m68k_jump_condition_test[(ir&0xf00)>>8]()

#define m68k_PUSH_W(x)                   \
    r[15]-=2;abus=r[15];                 \
    m68k_SET_DEST_W_TO_ADDR;             \
    m68k_DEST_W=x;


#define m68k_PUSH_L(x)                   \
    r[15]-=4;abus=r[15];                 \
    m68k_SET_DEST_L_TO_ADDR;             \
    m68k_DEST_L=x;


#define m68k_BIT_SHIFT_TO_dM_GET_SOURCE         \
  if(ir&BIT_5){                               \
    m68k_src_w=(WORD)(r[PARAM_N]&63);                 \
  }else{                                      \
    m68k_src_w=(WORD)PARAM_N;                       \
    if(!m68k_src_w)m68k_src_w=8;              \
  }

#define m68k_DEST_B (*((signed char*)m68k_dest))
#define m68k_DEST_W (*((short*)m68k_dest))
#define m68k_DEST_L (*((long*)m68k_dest))

#define CCR WORD_B_0(&sr)

#define SR_CLEAR(f) sr&=(unsigned short)(~((unsigned short)f));
#define SR_SET(f) sr|=(unsigned short)(f);

#define SR_VALID_BITMASK 0xa71f

#define SR_CHECK_Z_AND_N_B                   \
  if(m68k_DEST_B&BIT_7){                     \
    SR_SET(SR_N);                            \
  }else if(m68k_DEST_B==0){                  \
    SR_SET(SR_Z);                            \
  }

#define SR_CHECK_Z_AND_N_W                   \
  if(m68k_DEST_W&BIT_f){                     \
    SR_SET(SR_N);                            \
  }else if(m68k_DEST_W==0){                  \
    SR_SET(SR_Z);                            \
  }

#define SR_CHECK_Z_AND_N_L                   \
  if((signed long)m68k_DEST_L<0){            \
    SR_SET(SR_N);                            \
  }else if(m68k_DEST_L==0){                  \
    SR_SET(SR_Z);                            \
  }


#define SR_ADD_B                                                        \
  SR_CLEAR(SR_USER_BYTE)                                                \
  if( ( (( m68k_src_b)&( m68k_old_dest)&(~m68k_DEST_B))|                \
        ((~m68k_src_b)&(~m68k_old_dest)&( m68k_DEST_B)) ) & MSB_B){     \
    SR_SET(SR_V);                                                       \
  }                                                                     \
  if( ( (( m68k_src_b)&( m68k_old_dest)) |                              \
        ((~m68k_DEST_B)&( m68k_old_dest))|                              \
        (( m68k_src_b)&(~m68k_DEST_B)) ) & MSB_B){                      \
    SR_SET(SR_C+SR_X);                                                  \
  }                                                                     \
  if(!m68k_DEST_B)SR_SET(SR_Z);                                         \
  if(m68k_DEST_B & MSB_B)SR_SET(SR_N);                                  \


#define SR_ADDX_B                                                       \
  SR_CLEAR(SR_X+SR_N+SR_V+SR_C)                                         \
  if( ( (( m68k_src_b)&( m68k_old_dest)&(~m68k_DEST_B))|                \
        ((~m68k_src_b)&(~m68k_old_dest)&( m68k_DEST_B)) ) & MSB_B){     \
    SR_SET(SR_V);                                                       \
  }                                                                     \
  if( ( (( m68k_src_b)&( m68k_old_dest)) |                              \
        ((~m68k_DEST_B)&( m68k_old_dest))|                              \
        (( m68k_src_b)&(~m68k_DEST_B)) ) & MSB_B){                      \
    SR_SET(SR_C+SR_X);                                                  \
  }                                                                     \
  if(sr&SR_Z)if(m68k_DEST_B)SR_CLEAR(SR_Z);                             \
  if(m68k_DEST_B & MSB_B)SR_SET(SR_N);                                  \


#define SR_ADD_W                                                        \
  SR_CLEAR(SR_USER_BYTE)                                                \
  if( ( (( m68k_src_w)&( m68k_old_dest)&(~m68k_DEST_W))|                \
        ((~m68k_src_w)&(~m68k_old_dest)&( m68k_DEST_W)) ) & MSB_W){     \
    SR_SET(SR_V);                                                       \
  }                                                                     \
  if( ( (( m68k_src_w)&( m68k_old_dest)) |                              \
        ((~m68k_DEST_W)&( m68k_old_dest))|                              \
        (( m68k_src_w)&(~m68k_DEST_W)) ) & MSB_W){                      \
    SR_SET(SR_C+SR_X);                                                  \
  }                                                                     \
  if(!m68k_DEST_W)SR_SET(SR_Z);                                         \
  if(m68k_DEST_W & MSB_W)SR_SET(SR_N);                                  \


#define SR_ADDX_W                                                       \
  SR_CLEAR(SR_X+SR_N+SR_V+SR_C) \
  if( ( (( m68k_src_w)&( m68k_old_dest)&(~m68k_DEST_W))|  \
        ((~m68k_src_w)&(~m68k_old_dest)&( m68k_DEST_W)) ) & MSB_W){  \
    SR_SET(SR_V);                                                     \
  }                                                                 \
  if( ( (( m68k_src_w)&( m68k_old_dest)) |  \
        ((~m68k_DEST_W)&( m68k_old_dest))|  \
        (( m68k_src_w)&(~m68k_DEST_W)) ) & MSB_W){  \
    SR_SET(SR_C+SR_X);                                                 \
  }                                                                     \
  if(sr&SR_Z)if(m68k_DEST_W)SR_CLEAR(SR_Z);                                      \
  if(m68k_DEST_W & MSB_W)SR_SET(SR_N);                               \


#define SR_ADD_L           \
  SR_CLEAR(SR_USER_BYTE) \
  if( ( (( m68k_src_l)&( m68k_old_dest)&(~m68k_DEST_L))|  \
        ((~m68k_src_l)&(~m68k_old_dest)&( m68k_DEST_L)) ) & MSB_L){  \
    SR_SET(SR_V);                                                     \
  }                                                                 \
  if( ( (( m68k_src_l)&( m68k_old_dest)) |  \
        ((~m68k_DEST_L)&( m68k_old_dest))|  \
        (( m68k_src_l)&(~m68k_DEST_L)) ) & MSB_L){  \
    SR_SET(SR_C+SR_X);                                                 \
  }                                                                     \
  if(!m68k_DEST_L)SR_SET(SR_Z);                                      \
  if(m68k_DEST_L & MSB_L)SR_SET(SR_N);                               \


#define SR_ADDX_L           \
  SR_CLEAR(SR_X+SR_N+SR_V+SR_C) \
  if( ( (( m68k_src_l)&( m68k_old_dest)&(~m68k_DEST_L))|  \
        ((~m68k_src_l)&(~m68k_old_dest)&( m68k_DEST_L)) ) & MSB_L){  \
    SR_SET(SR_V);                                                     \
  }                                                                 \
  if( ( (( m68k_src_l)&( m68k_old_dest)) |  \
        ((~m68k_DEST_L)&( m68k_old_dest))|  \
        (( m68k_src_l)&(~m68k_DEST_L)) ) & MSB_L){  \
    SR_SET(SR_C+SR_X);                                                 \
  }                                                                     \
  if(sr&SR_Z)if(m68k_DEST_L)SR_CLEAR(SR_Z);                                      \
  if(m68k_DEST_L & MSB_L)SR_SET(SR_N);                               \




#define SR_SUB_B(extend_flag)           \
  SR_CLEAR(SR_N+SR_Z+SR_V+SR_C+extend_flag) \
  if( ( ((~m68k_src_b)&( m68k_old_dest)&(~m68k_DEST_B))|  \
        (( m68k_src_b)&(~m68k_old_dest)&( m68k_DEST_B)) ) & MSB_B){  \
    SR_SET(SR_V);                                                     \
  }                                                                 \
  if( ( (( m68k_src_b)&(~m68k_old_dest)) |  \
        (( m68k_DEST_B)&(~m68k_old_dest))|  \
        (( m68k_src_b)&( m68k_DEST_B)) ) & MSB_B){  \
    SR_SET(SR_C+extend_flag);                                                 \
  }                                                                     \
  if(!m68k_DEST_B)SR_SET(SR_Z);                                      \
  if(m68k_DEST_B & MSB_B)SR_SET(SR_N);                               \

#define SR_SUBX_B                                                      \
  SR_CLEAR(SR_X+SR_N+SR_V+SR_C) \
  if( ( ((~m68k_src_b)&( m68k_old_dest)&(~m68k_DEST_B))|  \
        (( m68k_src_b)&(~m68k_old_dest)&( m68k_DEST_B)) ) & MSB_B){  \
    SR_SET(SR_V);                                                     \
  }                                                                 \
  if( ( (( m68k_src_b)&(~m68k_old_dest)) |  \
        (( m68k_DEST_B)&(~m68k_old_dest))|  \
        (( m68k_src_b)&( m68k_DEST_B)) ) & MSB_B){  \
    SR_SET(SR_C+SR_X);                                                 \
  }                                                                     \
  if(sr&SR_Z)if(m68k_DEST_B)SR_CLEAR(SR_Z);                                      \
  if(m68k_DEST_B & MSB_B)SR_SET(SR_N);                               \

#define SR_SUB_W(extend_flag)           \
  SR_CLEAR(SR_N+SR_Z+SR_V+SR_C+extend_flag)                             \
  if( ( ((~m68k_src_w)&( m68k_old_dest)&(~m68k_DEST_W))|  \
        (( m68k_src_w)&(~m68k_old_dest)&( m68k_DEST_W)) ) & MSB_W){  \
    SR_SET(SR_V);                                                     \
  }                                                                 \
  if( ( (( m68k_src_w)&(~m68k_old_dest)) |  \
        (( m68k_DEST_W)&(~m68k_old_dest))|  \
        (( m68k_src_w)&( m68k_DEST_W)) ) & MSB_W){  \
    SR_SET(SR_C+extend_flag);                                                 \
  }                                                                     \
  if(!m68k_DEST_W)SR_SET(SR_Z);                                      \
  if(m68k_DEST_W & MSB_W)SR_SET(SR_N);                               \

#define SR_SUBX_W           \
  SR_CLEAR(SR_X+SR_N+SR_V+SR_C) \
  if( ( ((~m68k_src_w)&( m68k_old_dest)&(~m68k_DEST_W))|  \
        (( m68k_src_w)&(~m68k_old_dest)&( m68k_DEST_W)) ) & MSB_W){  \
    SR_SET(SR_V);                                                     \
  }                                                                 \
  if( ( (( m68k_src_w)&(~m68k_old_dest)) |  \
        (( m68k_DEST_W)&(~m68k_old_dest))|  \
        (( m68k_src_w)&( m68k_DEST_W)) ) & MSB_W){  \
    SR_SET(SR_C+SR_X);                                                 \
  }                                                                     \
  if(sr&SR_Z)if(m68k_DEST_W)SR_CLEAR(SR_Z);                                      \
  if(m68k_DEST_W & MSB_W)SR_SET(SR_N);                               \


#define SR_SUB_L(extend_flag)           \
  SR_CLEAR(SR_N+SR_Z+SR_V+SR_C+extend_flag)                             \
  if( ( ((~m68k_src_l)&( m68k_old_dest)&(~m68k_DEST_L))|  \
        (( m68k_src_l)&(~m68k_old_dest)&( m68k_DEST_L)) ) & MSB_L){  \
    SR_SET(SR_V);                                                     \
  }                                                                 \
  if( ( (( m68k_src_l)&(~m68k_old_dest)) |  \
        (( m68k_DEST_L)&(~m68k_old_dest))|  \
        (( m68k_src_l)&( m68k_DEST_L)) ) & MSB_L){  \
    SR_SET(SR_C+extend_flag);                                                 \
  }                                                                     \
  if(!m68k_DEST_L)SR_SET(SR_Z);                                      \
  if(m68k_DEST_L & MSB_L)SR_SET(SR_N);                               \

#define SR_SUBX_L           \
  SR_CLEAR(SR_X+SR_N+SR_V+SR_C) \
  if( ( ((~m68k_src_l)&( m68k_old_dest)&(~m68k_DEST_L))|  \
        (( m68k_src_l)&(~m68k_old_dest)&( m68k_DEST_L)) ) & MSB_L){  \
    SR_SET(SR_V);                                                     \
  }                                                                 \
  if( ( (( m68k_src_l)&(~m68k_old_dest)) |  \
        (( m68k_DEST_L)&(~m68k_old_dest))|  \
        (( m68k_src_l)&( m68k_DEST_L)) ) & MSB_L){  \
    SR_SET(SR_C+SR_X);                                                 \
  }                                                                     \
  if(sr&SR_Z)if(m68k_DEST_L)SR_CLEAR(SR_Z);                                      \
  if(m68k_DEST_L & MSB_L)SR_SET(SR_N);                               \



#define m68k_GET_IMMEDIATE_B m68k_src_b=m68k_fetchB();pc+=2
#define m68k_GET_IMMEDIATE_W m68k_src_w=m68k_fetchW();pc+=2
#define m68k_GET_IMMEDIATE_L m68k_src_l=m68k_fetchL();pc+=4
#define m68k_IMMEDIATE_B (signed char)m68k_fetchB()
#define m68k_IMMEDIATE_W (short)m68k_fetchW()
#define m68k_IMMEDIATE_L (long)m68k_fetchL()

#define DETECT_CHANGE_TO_USER_MODE  \
          if (!SUPERFLAG) change_to_user_mode();

#define ILLEGAL  exception(BOMBS_ILLEGAL_INSTRUCTION,EA_INST,0);

#ifndef DETECT_TRACE_BIT
#define DETECT_TRACE_BIT {if (sr & SR_TRACE) ioaccess=TRACE_BIT_JUST_SET | (ioaccess & IOACCESS_FLAGS_MASK);}
#endif

#define TRACE_BIT_JUST_SET 0x2b

//function codes on the m68k
#define FC_USER_DATA 1
#define FC_USER_PROGRAM 2
#define FC_SUPERVISOR_DATA 5
#define FC_SUPERVISOR_PROGRAM 6
#define FC_INTERRUPT_VERIFICATION 7

void sr_check_z_n_l_for_r0()
{
  m68k_dest=&r[0];
  SR_CHECK_Z_AND_N_L;
}

#else

#define SET_PC(ad) set_pc(ad);
extern void set_pc(MEM_ADDRESS);
#define M68K_PERFORM_RTE(s) perform_rte();
extern void perform_rte();
extern void sr_check_z_n_l_for_r0();
extern void m68k_process();

#define m68k_poke m68k_poke_noinline
#define m68k_dpoke m68k_dpoke_noinline
#define m68k_lpoke m68k_lpoke_noinline
extern void m68k_poke_noinline(MEM_ADDRESS ad,BYTE x);
extern void m68k_dpoke_noinline(MEM_ADDRESS ad,WORD x);
extern void m68k_lpoke_noinline(MEM_ADDRESS ad,LONG x);

#endif

#undef EXT
#undef INIT

