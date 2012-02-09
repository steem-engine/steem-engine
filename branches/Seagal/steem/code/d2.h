#define D2_PC_RELATIVE_PC dpc

#define D2_GET_SOURCE_B d2_jump_get_source_b[(ir&BITS_543)>>3]()
#define D2_GET_SOURCE_W d2_jump_get_source_w[(ir&BITS_543)>>3]()
#define D2_GET_SOURCE_L d2_jump_get_source_l[(ir&BITS_543)>>3]()

#define D2_GET_DEST_B d2_jump_get_dest_b[(ir&BITS_543)>>3]()
#define D2_GET_DEST_W d2_jump_get_dest_w[(ir&BITS_543)>>3]()
#define D2_GET_DEST_L d2_jump_get_dest_l[(ir&BITS_543)>>3]()

#define D2_GET_DEST_B_NOT_A d2_jump_get_dest_b_not_a[(ir&BITS_543)>>3]()
#define D2_GET_DEST_W_NOT_A d2_jump_get_dest_w_not_a[(ir&BITS_543)>>3]()
#define D2_GET_DEST_L_NOT_A d2_jump_get_dest_l_not_a[(ir&BITS_543)>>3]()

#define D2_GET_DEST_B_NOT_A_OR_D d2_jump_get_dest_b_not_a_or_d[(ir&BITS_543)>>3]()
#define D2_GET_DEST_W_NOT_A_OR_D d2_jump_get_dest_w_not_a_or_d[(ir&BITS_543)>>3]()
#define D2_GET_DEST_L_NOT_A_OR_D d2_jump_get_dest_l_not_a_or_d[(ir&BITS_543)>>3]()


#define D2_GET_DEST_B_NOT_A_WITH_SR d2_jump_get_dest_b_not_a_with_sr[(ir&BITS_543)>>3]()
#define D2_GET_DEST_W_NOT_A_WITH_SR d2_jump_get_dest_w_not_a_with_sr[(ir&BITS_543)>>3]()
#define D2_GET_DEST_L_NOT_A_WITH_SR d2_jump_get_dest_l_not_a_with_sr[(ir&BITS_543)>>3]()

#define D2_GET_IMMEDIATE_B \
  d2_src_b=EasyStr("#$")+itoa(d2_fetchB(),d2_t_buf,16); \
  trace_add_entry("source immediate: ",d2_src_b.c_str(),TDE_BEFORE,false,1,dpc+1);\
  dpc+=2
#define D2_GET_IMMEDIATE_W \
  d2_src_w=EasyStr("#$")+itoa(d2_fetchW(),d2_t_buf,16); \
  trace_add_entry("source immediate: ",d2_src_w.c_str(),TDE_BEFORE,false,2,dpc);\
  dpc+=2

#define D2_GET_IMMEDIATE_L \
  d2_src_l=EasyStr("#$")+itoa(d2_fetchL(),d2_t_buf,16); \
  trace_add_entry("source immediate: ",d2_src_l.c_str(),TDE_BEFORE,false,4,dpc);\
  dpc+=4

#define d2_src_b d2_src
#define d2_src_w d2_src
#define d2_src_l d2_src
#define D2_BIT_SHIFT_TO_dM_GET_SOURCE         \
  if(ir&BIT_5){                               \
    d2_src_w=D2_dN;                           \
    trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,2,(MEM_ADDRESS)&r[PARAM_N]); \
  }else{                                      \
    d2_src_w=EasyStr("#")+((((PARAM_N)-1)&7)+1);   \
  }

#define D2_dN EasyStr("d")+("0\0001\0002\0003\0004\0005\0006\0007\000"+2*PARAM_N)
#define D2_aN EasyStr("a")+("0\0001\0002\0003\0004\0005\0006\0007\000"+2*PARAM_N)
#define D2_dM EasyStr("d")+("0\0001\0002\0003\0004\0005\0006\0007\000"+2*PARAM_M)
#define D2_aM EasyStr("a")+("0\0001\0002\0003\0004\0005\0006\0007\000"+2*PARAM_M)
#define D2_BRACKETS_aN EasyStr("(a")+("0\0001\0002\0003\0004\0005\0006\0007\000"+2*PARAM_N)+")"
#define D2_BRACKETS_aM EasyStr("(a")+("0\0001\0002\0003\0004\0005\0006\0007\000"+2*PARAM_M)+")"

#define D2_IRIWO d2_iriwo()
#define D2_IRIWO_N d2_iriwo_N()
#define D2_IRIWO_PC d2_iriwo_pc()

EasyStr disa_d2(MEM_ADDRESS);

LONG d2_peekvalid;

BYTE d2_peek(MEM_ADDRESS ad);
WORD d2_dpeek(MEM_ADDRESS ad);
LONG d2_lpeek(MEM_ADDRESS ad);

EasyStr d2_src,d2_dest,d2_command,d2_pc_rel_ex;
WORD d2_ap;
int d2_n_movem_regs;

