void (*m68k_high_nibble_jump_table[16])();
void (*m68k_jump_line_0[64])();
void (*m68k_jump_line_4[64])();
void (*m68k_jump_line_5[8])();
void (*m68k_jump_line_8[8])();
void (*m68k_jump_line_9[8])();
void (*m68k_jump_line_b[8])();
void (*m68k_jump_line_c[8])();
void (*m68k_jump_line_d[8])();
void (*m68k_jump_line_e[64])();
void (*m68k_jump_line_4_stuff[64])();
void (*m68k_jump_get_source_b[8])();
void (*m68k_jump_get_source_w[8])();
void (*m68k_jump_get_source_l[8])();
void (*m68k_jump_get_source_b_not_a[8])();
void (*m68k_jump_get_source_w_not_a[8])();
void (*m68k_jump_get_source_l_not_a[8])();
void (*m68k_jump_get_dest_b[8])();
void (*m68k_jump_get_dest_w[8])();
void (*m68k_jump_get_dest_l[8])();
void (*m68k_jump_get_dest_b_not_a[8])();
void (*m68k_jump_get_dest_w_not_a[8])();
void (*m68k_jump_get_dest_l_not_a[8])();
void (*m68k_jump_get_dest_b_not_a_or_d[8])();
void (*m68k_jump_get_dest_w_not_a_or_d[8])();
void (*m68k_jump_get_dest_l_not_a_or_d[8])();
void (*m68k_jump_get_dest_b_not_a_faster_for_d[8])();
void (*m68k_jump_get_dest_w_not_a_faster_for_d[8])();
void (*m68k_jump_get_dest_l_not_a_faster_for_d[8])();
bool (*m68k_jump_condition_test[16])();

#define m68k_READ_B_FROM_ADDR                         \
  abus&=0xffffff;                                   \
  if(abus>=himem){                                  \
    if(abus>=MEM_IO_BASE){            \
      if(SUPERFLAG)m68k_src_b=io_read_b(abus);           \
      else exception(BOMBS_BUS_ERROR,EA_READ,abus);         \
    }else if(abus>=0xfc0000){                             \
      if(tos_high && abus<(0xfc0000+192*1024))m68k_src_b=ROM_PEEK(abus-rom_addr);   \
      else if (abus<0xfe0000 || abus>=0xfe2000) exception(BOMBS_BUS_ERROR,EA_READ,abus);  \
    }else if(abus>=MEM_EXPANSION_CARTRIDGE){           \
      if(cart){                                             \
        m68k_src_b=CART_PEEK(abus-MEM_EXPANSION_CARTRIDGE);  \
      }else{                                                 \
        m68k_src_b=0xff;                                    \
      }                                                     \
    }else if (abus>=rom_addr){                         \
      if(abus<(0xe00000+256*1024))m68k_src_b=ROM_PEEK(abus-rom_addr);                           \
      else if (abus>=0xec0000) exception(BOMBS_BUS_ERROR,EA_READ,abus);          \
      else m68k_src_b=0xff;                                          \
    }else if (abus>=0xd00000 && abus<0xd80000){ \
      m68k_src_b=0xff;                                          \
    }else if(mmu_confused){                            \
      m68k_src_b=mmu_confused_peek(abus,true);                                         \
    }else if(abus>=FOUR_MEGS){                   \
      exception(BOMBS_BUS_ERROR,EA_READ,abus);                          \
    }else{                                                     \
      m68k_src_b=0xff;                                          \
    }                                                             \
  }else if(abus>=MEM_START_OF_USER_AREA){                                              \
    m68k_src_b=(BYTE)(PEEK(abus));                  \
  }else if(SUPERFLAG){     \
    m68k_src_b=(BYTE)(PEEK(abus));                  \
  }else exception(BOMBS_BUS_ERROR,EA_READ,abus);


#define m68k_READ_W_FROM_ADDR           \
  abus&=0xffffff;                                   \
  if(abus&1){                                      \
    exception(BOMBS_ADDRESS_ERROR,EA_READ,abus);    \
  }else if(abus>=himem){                                  \
    if(abus>=MEM_IO_BASE){            \
      if(SUPERFLAG)m68k_src_w=io_read_w(abus);           \
      else exception(BOMBS_BUS_ERROR,EA_READ,abus);         \
    }else if(abus>=0xfc0000){                             \
      if (tos_high && abus<(0xfc0000+192*1024)) m68k_src_w=ROM_DPEEK(abus-rom_addr);   \
      else if (abus<0xfe0000 || abus>=0xfe2000) exception(BOMBS_BUS_ERROR,EA_READ,abus);  \
    }else if(abus>=MEM_EXPANSION_CARTRIDGE){           \
      if(cart){                                             \
        m68k_src_w=CART_DPEEK(abus-MEM_EXPANSION_CARTRIDGE);  \
      }else{                                                 \
        m68k_src_w=0xffff;                                    \
      }                                                     \
    }else if(abus>=rom_addr){                         \
      if(abus<(0xe00000+256*1024)) m68k_src_w=ROM_DPEEK(abus-rom_addr);                           \
      else if (abus>=0xec0000) exception(BOMBS_BUS_ERROR,EA_READ,abus);          \
      else m68k_src_w=0xffff;                                          \
    }else if (abus>=0xd00000 && abus<0xd80000){ \
      m68k_src_w=0xffff;                                          \
    }else if(mmu_confused){                            \
      m68k_src_w=mmu_confused_dpeek(abus,true);                                         \
    }else if(abus>=FOUR_MEGS){                   \
      exception(BOMBS_BUS_ERROR,EA_READ,abus);                          \
    }else{                                                     \
      m68k_src_w=0xffff;                                          \
    }                                                             \
  }else if(abus>=MEM_START_OF_USER_AREA){                                              \
    m68k_src_w=DPEEK(abus);                  \
  }else if(SUPERFLAG){     \
    m68k_src_w=DPEEK(abus);                  \
  }else exception(BOMBS_BUS_ERROR,EA_READ,abus);

#define m68k_READ_L_FROM_ADDR                        \
  abus&=0xffffff;                                   \
  if(abus&1){                                      \
    exception(BOMBS_ADDRESS_ERROR,EA_READ,abus);    \
  }else if(abus>=himem){                                  \
    if(abus>=MEM_IO_BASE){           \
      if(SUPERFLAG)m68k_src_l=io_read_l(abus);          \
      else exception(BOMBS_BUS_ERROR,EA_READ,abus);         \
    }else if(abus>=0xfc0000){                             \
      if(tos_high && abus<(0xfc0000+192*1024-2)) m68k_src_l=ROM_LPEEK(abus-rom_addr);   \
      else if (abus<0xfe0000 || abus>=0xfe2000) exception(BOMBS_BUS_ERROR,EA_READ,abus);  \
    }else if(abus>=MEM_EXPANSION_CARTRIDGE){           \
      if(cart){                                             \
        m68k_src_l=CART_LPEEK(abus-MEM_EXPANSION_CARTRIDGE);  \
      }else{                                                 \
        m68k_src_l=0xffffffff;                                    \
      }                                                     \
    }else if(abus>=rom_addr){                         \
      if(abus<(0xe00000+256*1024-2)) m68k_src_l=ROM_LPEEK(abus-rom_addr);   \
      else if (abus>=0xec0000) exception(BOMBS_BUS_ERROR,EA_READ,abus);          \
      else m68k_src_l=0xffffffff;                                          \
    }else if (abus>=0xd00000 && abus<0xd80000-2){ \
      m68k_src_l=0xffffffff;                                          \
    }else if (mmu_confused){                            \
      m68k_src_l=mmu_confused_lpeek(abus,true);                                         \
    }else if (abus>=FOUR_MEGS){                   \
      exception(BOMBS_BUS_ERROR,EA_READ,abus);                          \
    }else{                                                     \
      m68k_src_l=0xffffffff;                                          \
    }                                                             \
  }else if(abus>=MEM_START_OF_USER_AREA){                                              \
    m68k_src_l=LPEEK(abus);                  \
  }else if(SUPERFLAG){     \
    m68k_src_l=LPEEK(abus);                  \
  }else exception(BOMBS_BUS_ERROR,EA_READ,abus);



#define m68k_READ_B(addr)                              \
  m68k_src_b=m68k_peek(addr);                           \

#define m68k_READ_W(addr)                              \
  m68k_src_w=m68k_dpeek(addr);                           \

#define m68k_READ_L(addr)                              \
  m68k_src_l=m68k_lpeek(addr);                           \

//---------------------------------------------------------------------------
inline void change_to_user_mode()
{
//  if(SUPERFLAG){
  compare_buffer=r[15];r[15]=other_sp;other_sp=compare_buffer;
  SR_CLEAR(SR_SUPER);
//  }
}
//---------------------------------------------------------------------------
inline void change_to_supervisor_mode()
{
//  if(!SUPERFLAG){
  compare_buffer=r[15];r[15]=other_sp;other_sp=compare_buffer;
  SR_SET(SR_SUPER);
//  }
}

void m68k_exception::init(int a,exception_action ea,MEM_ADDRESS _abus)
{
  bombs=a;
  _pc=PC32; //old_pc;
  crash_address=old_pc;
  address=_abus;
  _sr=::sr;_ir=::ir;
  action=ea;
}

#define LOGSECTION LOGSECTION_CRASH
//---------------------------------------------------------------------------
void ASMCALL perform_crash_and_burn()
{
  reset_st(0,0);
  osd_start_scroller(T("CRASH AND BURN - ST RESET"));
}
//---------------------------------------------------------------------------
#ifdef _DEBUG_BUILD
void log_registers()
{
  log_write("        Register dump:");
  log_write(EasyStr("        pc = ")+HEXSl(pc,8));
  log_write(EasyStr("        sr = ")+HEXSl(sr,4));
  for(int n=0;n<16;n++){
    log_write(EasyStr("        ")+("d\0a\0"+(n/8)*2)+(n&7)+" = "+HEXSl(r[n],8));
  }
}

void log_history(int bombs,MEM_ADDRESS crash_address)
{
  if (logsection_enabled[LOGSECTION] && logging_suspended==0){
    log("");
    log("****************************************");
    if (bombs){
      log(EasyStr(bombs)+" bombs");
    }else{
      log("Exception/interrupt");
    }
    log(EasyStr("Crash at ")+HEXSl(crash_address,6));
    int n=pc_history_idx-HIST_MENU_SIZE;
    if (n<0) n=HISTORY_SIZE+n;
    EasyStr Dissasembly;
    do{
      if (pc_history[n]!=0xffffff71){
        Dissasembly=disa_d2(pc_history[n]);
        log(EasyStr(HEXSl(pc_history[n],6))+" - "+Dissasembly);
      }
      n++; if (n>=HISTORY_SIZE) n=0;
    }while (n!=pc_history_idx);
    log("^^ Crash!");
    log("****************************************");
    log("");
  }
}
#endif
//---------------------------------------------------------------------------
inline void m68k_interrupt(MEM_ADDRESS ad) //not address, bus, illegal instruction or privilege violation interrupt
{
  WORD _sr=sr;
  if (!SUPERFLAG) change_to_supervisor_mode();
  m68k_PUSH_L(PC32);
  m68k_PUSH_W(_sr);
  SET_PC(ad);
//  log(EasyStr("interrupt - increasing interrupt depth from ")+interrupt_depth+" to "+(interrupt_depth+1));
  SR_CLEAR(SR_TRACE);
  interrupt_depth++;
}
//---------------------------------------------------------------------------
void m68k_exception::crash()
{
  DWORD bytes_to_stack=int((bombs==BOMBS_BUS_ERROR || bombs==BOMBS_ADDRESS_ERROR) ? (4+2+2+4+2):(4+2));
  MEM_ADDRESS sp=(MEM_ADDRESS)(SUPERFLAG ? (areg[7] & 0xffffff):(other_sp & 0xffffff));
  if (sp<bytes_to_stack || sp>himem){
    // Double bus error, CPU halt (we crash and burn)
    // This only has to be done here, m68k_PUSH_ will cause bus error if invalid
    DEBUG_ONLY( log_history(bombs,crash_address) );
    perform_crash_and_burn();
  }else{
    cpu_cycles&=-4;
    if (bombs==BOMBS_ILLEGAL_INSTRUCTION || bombs==BOMBS_PRIVILEGE_VIOLATION){
      INSTRUCTION_TIME_ROUND(0); //Round first for interrupts
      if (!SUPERFLAG) change_to_supervisor_mode();
      m68k_PUSH_L((crash_address & 0x00ffffff) | pc_high_byte);
      INSTRUCTION_TIME_ROUND(8);
      m68k_PUSH_W(_sr);
      INSTRUCTION_TIME_ROUND(4); //Round first for interrupts
      MEM_ADDRESS ad=LPEEK(bombs*4);
      if (ad & 1){
        bombs=BOMBS_ADDRESS_ERROR;
        address=ad;
        action=EA_FETCH;
      }else{
        SET_PC(ad);
        SR_CLEAR(SR_TRACE);
        INSTRUCTION_TIME_ROUND(22);
        interrupt_depth++;                                        // Is this necessary?
      }
    }
    if (bombs==BOMBS_BUS_ERROR || bombs==BOMBS_ADDRESS_ERROR){
      if (!SUPERFLAG) change_to_supervisor_mode();
      // Big page in stack
//      try{
      TRY_M68K_EXCEPTION
        m68k_PUSH_L(_pc);
        m68k_PUSH_W(_sr);
        m68k_PUSH_W(_ir);
        m68k_PUSH_L(address);
        WORD x=0;
        if (action!=EA_WRITE) x|=B6_010000;
        if (action==EA_FETCH){
          x|=WORD((_sr & SR_SUPER) ? FC_SUPERVISOR_PROGRAM:FC_USER_PROGRAM);
        }else{
          x|=WORD((_sr & SR_SUPER) ? FC_SUPERVISOR_DATA:FC_USER_DATA);
        }
        m68k_PUSH_W(x);
//      }catch (m68k_exception&){
      CATCH_M68K_EXCEPTION
        r[15]=0xf000;
      END_M68K_EXCEPTION

      SET_PC(LPEEK(bombs*4));
      SR_CLEAR(SR_TRACE);

      INSTRUCTION_TIME_ROUND(0); //Round first for interrupts
      INSTRUCTION_TIME_ROUND(50); //Round for fetch
    }
    DEBUG_ONLY(log_history(bombs,crash_address));
  }
  PeekEvent(); // Stop exception freeze
}

#undef LOGSECTION

inline void m68k_poke_abus(BYTE x){
  abus&=0xffffff;
  if(abus>=MEM_IO_BASE){
    if(SUPERFLAG)
      io_write_b(abus,x);
    else
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);
  }else if(abus>=himem){
    if (mmu_confused){
      mmu_confused_set_dest_to_addr(1,true);
      m68k_DEST_B=x;
    }else if (abus>=FOUR_MEGS){
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);
    } //otherwise throw away
  }else{
    if (SUPERFLAG && abus>=MEM_FIRST_WRITEABLE)
      PEEK(abus)=x;
    else if (abus>=MEM_START_OF_USER_AREA)
      PEEK(abus)=x;
    else exception(BOMBS_BUS_ERROR,EA_WRITE,abus);
  }
}

inline void m68k_dpoke_abus(WORD x){
  abus&=0xffffff;
  if(abus&1)exception(BOMBS_ADDRESS_ERROR,EA_WRITE,abus);
  else if(abus>=MEM_IO_BASE){
    if(SUPERFLAG)
      io_write_w(abus,x);
    else
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);
  }else if(abus>=himem){
    if(mmu_confused){
      mmu_confused_set_dest_to_addr(2,true);
      m68k_DEST_W=x;
    }else if(abus>=FOUR_MEGS){
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);
    } //otherwise throw away
  }else{
    if(SUPERFLAG && abus>=MEM_FIRST_WRITEABLE)
      DPEEK(abus)=x;
    else if(abus>=MEM_START_OF_USER_AREA)
      DPEEK(abus)=x;
    else exception(BOMBS_BUS_ERROR,EA_WRITE,abus);
  }
}

inline void m68k_lpoke_abus(LONG x){
  abus&=0xffffff;
  if(abus&1)exception(BOMBS_ADDRESS_ERROR,EA_WRITE,abus);
  else if(abus>=MEM_IO_BASE){
    if(SUPERFLAG)
      io_write_l(abus,x);
    else
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);
  }else if(abus>=himem){
    if(mmu_confused){
      mmu_confused_set_dest_to_addr(4,true);
      m68k_DEST_L=x;
    }else if(abus>=FOUR_MEGS){
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);
    } //otherwise throw away
  }else{
    if(SUPERFLAG && abus>=MEM_FIRST_WRITEABLE)
      LPEEK(abus)=x;
    else if(abus>=MEM_START_OF_USER_AREA)
      LPEEK(abus)=x;
    else exception(BOMBS_BUS_ERROR,EA_WRITE,abus);
  }
}

inline void m68k_poke(MEM_ADDRESS ad,BYTE x){
  abus=ad;
  m68k_poke_abus(x);
}

inline void m68k_dpoke(MEM_ADDRESS ad,WORD x){
  abus=ad;
  m68k_dpoke_abus(x);
}

inline void m68k_lpoke(MEM_ADDRESS ad,LONG x){
  abus=ad;
  m68k_lpoke_abus(x);
}


BYTE m68k_peek(MEM_ADDRESS ad){
  ad&=0xffffff;
  if (ad>=himem){
    if (ad>=MEM_IO_BASE){
      if(SUPERFLAG)return io_read_b(ad);
      else exception(BOMBS_BUS_ERROR,EA_READ,ad);
    }else if(ad>=0xfc0000){
      if(tos_high && ad<(0xfc0000+192*1024))return ROM_PEEK(ad-rom_addr);
      else if (ad<0xfe0000 || ad>=0xfe2000) exception(BOMBS_BUS_ERROR,EA_READ,ad);
    }else if(ad>=MEM_EXPANSION_CARTRIDGE){
      if (cart) return CART_PEEK(ad-MEM_EXPANSION_CARTRIDGE);
      else return 0xff;
    }else if(ad>=rom_addr){
      if (ad<(0xe00000+256*1024)) return ROM_PEEK(ad-rom_addr);
      if (ad>=0xec0000) exception(BOMBS_BUS_ERROR,EA_READ,ad);
      return 0xff;
    }else if (ad>=0xd00000 && ad<0xd80000){
      return 0xff;
    }else if (mmu_confused){
      return mmu_confused_peek(ad,true);
    }else if (ad>=FOUR_MEGS){
      exception(BOMBS_BUS_ERROR,EA_READ,ad);
    }else{
      return 0xff;
    }
  }else if(ad>=MEM_START_OF_USER_AREA){
    return (BYTE)(PEEK(ad));
  }else if(SUPERFLAG){
    return (BYTE)(PEEK(ad));
  }else exception(BOMBS_BUS_ERROR,EA_READ,ad);
  return 0;
}

WORD m68k_dpeek(MEM_ADDRESS ad){
  ad&=0xffffff;
  if(ad&1)exception(BOMBS_ADDRESS_ERROR,EA_READ,ad);
  else if(ad>=himem){
    if(ad>=MEM_IO_BASE){
      if(SUPERFLAG)return io_read_w(ad);
      else exception(BOMBS_BUS_ERROR,EA_READ,ad);
    }else if(ad>=0xfc0000){
      if(tos_high && ad<(0xfc0000+192*1024))return ROM_DPEEK(ad-rom_addr);
      else if (ad<0xfe0000 || ad>=0xfe2000) exception(BOMBS_BUS_ERROR,EA_READ,ad);
    }else if(ad>=MEM_EXPANSION_CARTRIDGE){
      if (cart) return CART_DPEEK(ad-MEM_EXPANSION_CARTRIDGE);
      else return 0xffff;
    }else if(ad>=rom_addr){
      if (ad<(0xe00000+256*1024)) return ROM_DPEEK(ad-rom_addr);
      if (ad>=0xec0000) exception(BOMBS_BUS_ERROR,EA_READ,ad);
      return 0xffff;
    }else if (ad>=0xd00000 && ad<0xd80000){
      return 0xffff;
    }else if(mmu_confused){
      return mmu_confused_dpeek(ad,true);
    }else if(ad>=FOUR_MEGS){
      exception(BOMBS_BUS_ERROR,EA_READ,ad);
    }else{
      return 0xffff;
    }
  }else if(ad>=MEM_START_OF_USER_AREA){
    return DPEEK(ad);
  }else if(SUPERFLAG){
    return DPEEK(ad);
  }else exception(BOMBS_BUS_ERROR,EA_READ,ad);
  return 0;
}

LONG m68k_lpeek(MEM_ADDRESS ad){
  ad&=0xffffff;
  if(ad&1)exception(BOMBS_ADDRESS_ERROR,EA_WRITE,ad);
  else if(ad>=himem){
    if(ad>=MEM_IO_BASE){
      if(SUPERFLAG)return io_read_l(ad);
      else exception(BOMBS_BUS_ERROR,EA_READ,ad);
    }else if(ad>=0xfc0000){
      if(tos_high && ad<(0xfc0000+192*1024-2))return ROM_LPEEK(ad-rom_addr);
      else if (ad<0xfe0000 || ad>=0xfe2000) exception(BOMBS_BUS_ERROR,EA_READ,ad);
    }else if (ad>=MEM_EXPANSION_CARTRIDGE){
      if (cart) return CART_LPEEK(ad-MEM_EXPANSION_CARTRIDGE);
      else return 0xffffffff;
    }else if (ad>=rom_addr){
      if (ad<(0xe00000+256*1024-2)) return ROM_LPEEK(ad-rom_addr);
      if (ad>=0xec0000) exception(BOMBS_BUS_ERROR,EA_READ,ad);
      return 0xffffffff;
    }else if (ad>=0xd00000 && ad<0xd80000){
      return 0xffffffff;
    }else if (mmu_confused){
      return mmu_confused_lpeek(ad,true);
    }else if (ad>=FOUR_MEGS){
      exception(BOMBS_BUS_ERROR,EA_READ,ad);
    }else{
      return 0xffffffff;
    }
  }else if (ad>=MEM_START_OF_USER_AREA){
    return LPEEK(ad);
  }else if (SUPERFLAG){
    return LPEEK(ad);
  }else exception(BOMBS_BUS_ERROR,EA_READ,ad);
  return 0;
}

BYTE m68k_fetchB()
{
  WORD ret;
  FETCH_W(ret)
  return LOBYTE(ret);
}

WORD m68k_fetchW()
{
  WORD ret;
  FETCH_W(ret)
  return ret;
}

LONG m68k_fetchL()
{
  LONG ret;
  FETCH_W(*LPHIWORD(ret));
  FETCH_W(*LPLOWORD(ret));
  return ret;
}

void m68k_unrecognised()
{
  exception(BOMBS_ILLEGAL_INSTRUCTION,EA_INST,0);
}


BYTE m68k_read_dest_b(){
  BYTE x;
  switch(ir&BITS_543){
  case BITS_543_000:
    return LOBYTE(r[PARAM_M]);
  case BITS_543_001:
    m68k_unrecognised();break;
  case BITS_543_010:
    INSTRUCTION_TIME_ROUND(4);
    return m68k_peek(areg[PARAM_M]);
  case BITS_543_011:
    INSTRUCTION_TIME_ROUND(4);
    x=m68k_peek(areg[PARAM_M]);areg[PARAM_M]++;
    if(PARAM_M==7)areg[7]++;
    return x;
  case BITS_543_100:
    areg[PARAM_M]--;
    if (PARAM_M==7) areg[7]--;
    INSTRUCTION_TIME_ROUND(6);
    return m68k_peek(areg[PARAM_M]);
  case BITS_543_101:{
    INSTRUCTION_TIME_ROUND(8);
    register MEM_ADDRESS ad=areg[PARAM_M]+(signed short)m68k_fetchW();pc+=2;
    x=m68k_peek(ad);
    return x;
  }case BITS_543_110:
    INSTRUCTION_TIME_ROUND(10);
    m68k_iriwo=m68k_fetchW();pc+=2;
    if(m68k_iriwo&BIT_b){  //.l
      return m68k_peek(areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12]);
    }else{         //.w
      return m68k_peek(areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12]);
    }
  case BITS_543_111:
    switch(ir&0x7){
    case 0:{
      INSTRUCTION_TIME_ROUND(8);
      register MEM_ADDRESS ad=0xffffff&(unsigned long)((signed long)((signed short)m68k_fetchW()));
      pc+=2;
      x=m68k_peek(ad);
      return x;
    }case 1:{
      INSTRUCTION_TIME_ROUND(12);
      register MEM_ADDRESS ad=m68k_fetchL()&0xffffff;
      pc+=4;
      x=m68k_peek(ad);
      return x;
    }default:
      m68k_unrecognised();
    }
  }
  return 0;
}

WORD m68k_read_dest_w(){
  WORD x;
  switch(ir&BITS_543){
  case BITS_543_000:
    return LOWORD(r[PARAM_M]);
  case BITS_543_001:
    m68k_unrecognised();break;
  case BITS_543_010:
    INSTRUCTION_TIME_ROUND(4);
    return m68k_dpeek(areg[PARAM_M]);
  case BITS_543_011:
    INSTRUCTION_TIME_ROUND(4);
    x=m68k_dpeek(areg[PARAM_M]);areg[PARAM_M]+=2;
    return x;
  case BITS_543_100:
    INSTRUCTION_TIME_ROUND(6);
    areg[PARAM_M]-=2;
    return m68k_dpeek(areg[PARAM_M]);
  case BITS_543_101:{
    INSTRUCTION_TIME_ROUND(8);
    register MEM_ADDRESS ad=areg[PARAM_M]+(signed short)m68k_fetchW();pc+=2;
    x=m68k_dpeek(ad);
    return x;
  }case BITS_543_110:
    INSTRUCTION_TIME_ROUND(10);
    m68k_iriwo=m68k_fetchW();pc+=2;
    if(m68k_iriwo&BIT_b){  //.l
      return m68k_dpeek(areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12]);
    }else{         //.w
      return m68k_dpeek(areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12]);
    }
  case BITS_543_111:
    switch(ir&ir&0x7){
    case 0:{
      INSTRUCTION_TIME_ROUND(8);
      register MEM_ADDRESS ad=0xffffff&(unsigned long)((signed long)((signed short)m68k_fetchW()));
      pc+=2;
      x=m68k_dpeek(ad);
      return x;
    }case 1:{
      INSTRUCTION_TIME_ROUND(12);
      register MEM_ADDRESS ad=m68k_fetchL()&0xffffff;
      pc+=4;
      x=m68k_dpeek(ad);
      return x;
    }default:
      m68k_unrecognised();
    }
  }
  return 0;
}

LONG m68k_read_dest_l(){
  LONG x;
  switch(ir&BITS_543){
  case BITS_543_000:
    return (r[PARAM_M]);
  case BITS_543_001:
    m68k_unrecognised();break;
  case BITS_543_010:
    INSTRUCTION_TIME_ROUND(8);
    return m68k_lpeek(areg[PARAM_M]);
  case BITS_543_011:
    INSTRUCTION_TIME_ROUND(8);
    x=m68k_lpeek(areg[PARAM_M]);areg[PARAM_M]+=4;
    return x;
  case BITS_543_100:
    INSTRUCTION_TIME_ROUND(10);
    areg[PARAM_M]-=4;
    return m68k_lpeek(areg[PARAM_M]);
  case BITS_543_101:{
    INSTRUCTION_TIME_ROUND(12);
    register MEM_ADDRESS ad=areg[PARAM_M]+(signed short)m68k_fetchW();pc+=2;
    x=m68k_lpeek(ad);
    return x;
  }case BITS_543_110:
    m68k_iriwo=m68k_fetchW();pc+=2;
    INSTRUCTION_TIME_ROUND(14);
    if(m68k_iriwo&BIT_b){  //.l
      return m68k_lpeek(areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12]);
    }else{         //.w
      return m68k_lpeek(areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12]);
    }
  case BITS_543_111:
    switch(ir&0x7){
    case 0:{
      INSTRUCTION_TIME_ROUND(12);
      register MEM_ADDRESS ad=0xffffff&(unsigned long)((signed long)((signed short)m68k_fetchW()));
      pc+=2;
      x=m68k_lpeek(ad);
      return x;
    }case 1:{
      INSTRUCTION_TIME_ROUND(16);
      register MEM_ADDRESS ad=m68k_fetchL()&0xffffff;
      pc+=4;
      x=m68k_lpeek(ad);
      return x;
    }default:
      m68k_unrecognised();
    }
  }
  return 0;
}

#define HANDLE_IOACCESS(tracefunc) \
  if (ioaccess){                             \
    switch (ioaccess & IOACCESS_NUMBER_MASK){                        \
      case 1: io_write_b(ioad,LOBYTE(iobuffer)); break;    \
      case 2: io_write_w(ioad,LOWORD(iobuffer)); break;    \
      case 4: io_write_l(ioad,iobuffer); break;      \
      case TRACE_BIT_JUST_SET: tracefunc; break;                                        \
    }                                             \
    if (ioaccess & IOACCESS_FLAG_DELAY_MFP){ \
      ioaccess&=~IOACCESS_FLAG_DELAY_MFP;  \
      ioaccess|=IOACCESS_FLAG_FOR_CHECK_INTRS; \
    }else if (ioaccess & IOACCESS_FLAG_FOR_CHECK_INTRS_MFP_CHANGE){ \
      ioaccess|=IOACCESS_FLAG_DELAY_MFP;  \
    } \
    if (ioaccess & IOACCESS_INTERCEPT_OS2){ \
      ioaccess&=~IOACCESS_INTERCEPT_OS2;  \
      ioaccess|=IOACCESS_FLAG_FOR_CHECK_INTRS; \
    }else if (ioaccess & IOACCESS_INTERCEPT_OS){ \
      ioaccess|=IOACCESS_INTERCEPT_OS2; \
    } \
    if (ioaccess & IOACCESS_FLAG_FOR_CHECK_INTRS){   \
      check_for_interrupts_pending();          \
      CHECK_STOP_USER_MODE_NO_INTR \
    }                                             \
    if (ioaccess & IOACCESS_FLAG_DO_BLIT) Blitter_Start_Now(); \
    /* These flags stay until the next instruction to stop interrupts */  \
    ioaccess=ioaccess & (IOACCESS_FLAG_DELAY_MFP | IOACCESS_INTERCEPT_OS2);                                   \
  }

#define m68k_PROCESS \
  LOG_CPU  \
  old_pc=pc;  \
  FETCH_W(ir)       \
  pc+=2;               \
  m68k_high_nibble_jump_table[ir>>12]();    \
  HANDLE_IOACCESS(m68k_trace();)           \
  DEBUG_ONLY( debug_first_instruction=0 );

#define LOGSECTION LOGSECTION_TRACE
extern "C" ASMCALL void m68k_trace() //execute instruction with trace bit set
{
#ifdef _DEBUG_BUILD
  pc_history[pc_history_idx++]=pc;
  if (pc_history_idx>=HISTORY_SIZE) pc_history_idx=0;
  EasyStr Dissasembly=disa_d2(pc);
  log(EasyStr("TRACE: ")+HEXSl(pc,6)+" - "+Dissasembly);
#endif

  LOG_CPU

  old_pc=pc;

  DEBUG_ONLY( if (do_breakpoint_check) breakpoint_check(); )



  ir=m68k_fetchW();
  pc+=2;

  // Store blitter and interrupt check bits, set trace exception bit, lose everything else
  int store_ioaccess=ioaccess & (IOACCESS_FLAG_DO_BLIT | IOACCESS_FLAG_FOR_CHECK_INTRS |
                                  IOACCESS_FLAG_FOR_CHECK_INTRS_MFP_CHANGE);
  ioaccess=0;
  m68k_do_trace_exception=true;
  m68k_high_nibble_jump_table[ir>>12]();

  if (m68k_do_trace_exception){
    // This flag is used for exceptions that we don't want to do a proper exception
    // for but should really. i.e Line-A/F, they are just as serious as illegal
    // instruction but are called reguarly, we don't want to slow things down.
    INSTRUCTION_TIME_ROUND(0); // Round first for interrupts
    INSTRUCTION_TIME_ROUND(34);
    m68k_interrupt(LPEEK(BOMBS_TRACE_EXCEPTION*4));
  }
  ioaccess|=store_ioaccess;

  // In case of IOACCESS_FLAG_FOR_CHECK_INTRS interrupt must happen after trace
  HANDLE_IOACCESS(;)
}
#undef LOGSECTION


void m68k_get_effective_address()
{
  // Note: The timings in this routine are completely wrong. It is only used for
  //       lea, pea, jsr and jmp, they all have different EA times so they all
  //       correct for these bogus times. Do not use this routine somewhere that
  //       doesn't correct timings!
  switch (ir & BITS_543){
    case BITS_543_010:
      INSTRUCTION_TIME_ROUND(0);
      effective_address=areg[PARAM_M];
      break;
    case BITS_543_101:
      INSTRUCTION_TIME_ROUND(4);
      effective_address=areg[PARAM_M]+(signed short)m68k_fetchW();
      pc+=2;
      break;
    case BITS_543_110:
      INSTRUCTION_TIME_ROUND(8);
      m68k_iriwo=m68k_fetchW();pc+=2;
      if (m68k_iriwo & BIT_b){  //.l
        effective_address=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12];
      }else{         //.w
        effective_address=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12];
      }
      break;
    case BITS_543_111:
      switch (ir & 0x7){
      case 0:
        INSTRUCTION_TIME_ROUND(4);
        effective_address=(signed long)(signed short)m68k_fetchW();
        pc+=2;
        break;
      case 1:
        INSTRUCTION_TIME_ROUND(8);
        effective_address=m68k_fetchL();
        pc+=4;
        break;
      case 2:
        INSTRUCTION_TIME_ROUND(4);
        effective_address=(PC_RELATIVE_PC+(signed short)m68k_fetchW()) | pc_high_byte;
        PC_RELATIVE_MONITOR(effective_address);
        pc+=2;
        break;
      case 3:
        INSTRUCTION_TIME_ROUND(8);
        m68k_iriwo=m68k_fetchW();
        if (m68k_iriwo & BIT_b){  //.l
          effective_address=(PC_RELATIVE_PC+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12]) | pc_high_byte;
        }else{         //.w
          effective_address=(PC_RELATIVE_PC+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12]) | pc_high_byte;
        }
        PC_RELATIVE_MONITOR(effective_address);
        pc+=2;
        break;       //what do bits 8,9,a  of extra word do?  (not always 0)
      default:
        m68k_unrecognised();
        break;
      }
      break;
    default:
      m68k_unrecognised();
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//////////////////////////    GET SOURCE     ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void m68k_get_source_000_b(){ m68k_src_b=(BYTE)(r[PARAM_M]); }
void m68k_get_source_000_w(){ m68k_src_w=(WORD)(r[PARAM_M]); }
void m68k_get_source_000_l(){ m68k_src_l=(long)(r[PARAM_M]); }
void m68k_get_source_001_b(){ m68k_src_b=(BYTE)(areg[PARAM_M]); }
void m68k_get_source_001_w(){ m68k_src_w=(WORD)(areg[PARAM_M]); }
void m68k_get_source_001_l(){ m68k_src_l=(long)(areg[PARAM_M]); }
void m68k_get_source_010_b(){ INSTRUCTION_TIME_ROUND(4);m68k_READ_B(areg[PARAM_M]) }
void m68k_get_source_010_w(){ INSTRUCTION_TIME_ROUND(4);m68k_READ_W(areg[PARAM_M]) }
void m68k_get_source_010_l(){ INSTRUCTION_TIME_ROUND(8);m68k_READ_L(areg[PARAM_M]) }
void m68k_get_source_011_b(){ INSTRUCTION_TIME_ROUND(4);m68k_READ_B(areg[PARAM_M]) areg[PARAM_M]++; if(PARAM_M==7)areg[7]++;}
void m68k_get_source_011_w(){ INSTRUCTION_TIME_ROUND(4);m68k_READ_W(areg[PARAM_M]) areg[PARAM_M]+=2; }
void m68k_get_source_011_l(){ INSTRUCTION_TIME_ROUND(8);m68k_READ_L(areg[PARAM_M]) areg[PARAM_M]+=4; }
void m68k_get_source_100_b(){ INSTRUCTION_TIME_ROUND(6);/* 6 */ areg[PARAM_M]--;if(PARAM_M==7)areg[7]--;m68k_READ_B(areg[PARAM_M]) }
void m68k_get_source_100_w(){ INSTRUCTION_TIME_ROUND(6);/* 6 */areg[PARAM_M]-=2;m68k_READ_W(areg[PARAM_M])  }
void m68k_get_source_100_l(){ INSTRUCTION_TIME_ROUND(10);/* 10 */areg[PARAM_M]-=4;m68k_READ_L(areg[PARAM_M])  }
void m68k_get_source_101_b(){
  INSTRUCTION_TIME_ROUND(8);
  register int fw=(signed short)m68k_fetchW(); pc+=2;
  m68k_READ_B(areg[PARAM_M]+fw);

}
void m68k_get_source_101_w(){
  INSTRUCTION_TIME_ROUND(8);
  register int fw=(signed short)m68k_fetchW(); pc+=2;
  m68k_READ_W(areg[PARAM_M]+fw);
}
void m68k_get_source_101_l(){
  INSTRUCTION_TIME_ROUND(12);
  register int fw=(signed short)m68k_fetchW(); pc+=2;
  m68k_READ_L(areg[PARAM_M]+fw);
}
void m68k_get_source_110_b(){
  INSTRUCTION_TIME_ROUND(10);
  WORD w=m68k_fetchW();pc+=2;
  if(w&BIT_b){  //.l
    abus=areg[PARAM_M]+(signed char)LOBYTE(w)+(int)r[w>>12];
  }else{         //.w
    abus=areg[PARAM_M]+(signed char)LOBYTE(w)+(signed short)r[w>>12];
  }
  m68k_READ_B_FROM_ADDR
}
void m68k_get_source_110_w(){
  INSTRUCTION_TIME_ROUND(10);
  WORD w=m68k_fetchW();pc+=2;
  if(w&BIT_b){  //.l
    abus=areg[PARAM_M]+(signed char)LOBYTE(w)+(int)r[w>>12];
  }else{         //.w
    abus=areg[PARAM_M]+(signed char)LOBYTE(w)+(signed short)r[w>>12];
  }
  m68k_READ_W_FROM_ADDR
}
void m68k_get_source_110_l(){
  INSTRUCTION_TIME_ROUND(14);
  WORD w=m68k_fetchW();pc+=2;
  if(w&BIT_b){  //.l
    abus=areg[PARAM_M]+(signed char)LOBYTE(w)+(int)r[w>>12];
  }else{         //.w
    abus=areg[PARAM_M]+(signed char)LOBYTE(w)+(signed short)r[w>>12];
  }
  m68k_READ_L_FROM_ADDR
}
void m68k_get_source_111_b(){
  switch(ir&0x7){
  case 0:{
    INSTRUCTION_TIME_ROUND(8);
    register signed int fw=(signed short)m68k_fetchW();pc+=2;
    m68k_READ_B(fw)
    break;
  }case 1:{
    INSTRUCTION_TIME_ROUND(12);
    register MEM_ADDRESS fl=m68k_fetchL();pc+=4;
    m68k_READ_B(fl)
    break;
  }case 2:{
    INSTRUCTION_TIME_ROUND(8);
    register MEM_ADDRESS ad=PC_RELATIVE_PC+(signed short)m68k_fetchW();pc+=2;
    PC_RELATIVE_MONITOR(ad);
    m68k_READ_B(ad)
    break;
  }case 3:{
    INSTRUCTION_TIME_ROUND(10);
    m68k_iriwo=m68k_fetchW();
    if(m68k_iriwo&BIT_b){  //.l
      abus=PC_RELATIVE_PC+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12];
    }else{         //.w
      abus=PC_RELATIVE_PC+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12];
    }
    PC_RELATIVE_MONITOR(abus);
    pc+=2;
    m68k_READ_B_FROM_ADDR
    break;       //what do bits 8,9,a  of extra word do?  (not always 0)
  }case 4:{
    INSTRUCTION_TIME_ROUND(4);
    m68k_src_b=m68k_fetchB();
    pc+=2;
    break;
  }default:
    ILLEGAL;
  }
}
void m68k_get_source_111_w(){
  switch(ir&0x7){
  case 0:{
    INSTRUCTION_TIME_ROUND(8);
    register signed int fw=(signed short)m68k_fetchW();pc+=2;
    m68k_READ_W(fw)

    break;
  }case 1:{
    INSTRUCTION_TIME_ROUND(12);
    register MEM_ADDRESS fl=m68k_fetchL();pc+=4;
    m68k_READ_W(fl)
    break;
  }case 2:{
    INSTRUCTION_TIME_ROUND(8);
    register MEM_ADDRESS ad=PC_RELATIVE_PC+(signed short)m68k_fetchW();pc+=2;
    PC_RELATIVE_MONITOR(ad);
    m68k_READ_W(ad)
    break;
  }case 3:{
    INSTRUCTION_TIME_ROUND(10);
    m68k_iriwo=m68k_fetchW();
    if(m68k_iriwo&BIT_b){  //.l
      abus=PC_RELATIVE_PC+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12];
    }else{         //.w
      abus=PC_RELATIVE_PC+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12];
    }
    PC_RELATIVE_MONITOR(abus);
    pc+=2;
    m68k_READ_W_FROM_ADDR
    break;       //what do bits 8,9,a  of extra word do?  (not always 0)
  }case 4:{
    INSTRUCTION_TIME_ROUND(4);
//      ap=m68k_fetchL();pc+=4;
    m68k_src_w=m68k_fetchW();
    pc+=2;
    break;
  }default:
    ILLEGAL;
  }
}
void m68k_get_source_111_l(){
  switch(ir&0x7){
  case 0:{
    INSTRUCTION_TIME_ROUND(12);
    register signed int fw=(signed short)m68k_fetchW();pc+=2;
    m68k_READ_L(fw)
    break;
  }case 1:{
    INSTRUCTION_TIME_ROUND(16);
    register MEM_ADDRESS fl=m68k_fetchL();pc+=4;
    m68k_READ_L(fl)
    break;
  }case 2:{
    INSTRUCTION_TIME_ROUND(12);
    register MEM_ADDRESS ad=PC_RELATIVE_PC+(signed short)m68k_fetchW();pc+=2;
    PC_RELATIVE_MONITOR(ad);
    m68k_READ_L(ad)
    break;
  }case 3:{
    INSTRUCTION_TIME_ROUND(14);
    m68k_iriwo=m68k_fetchW();
    if(m68k_iriwo&BIT_b){  //.l
      abus=PC_RELATIVE_PC+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12];
    }else{         //.w
      abus=PC_RELATIVE_PC+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12];
    }
    PC_RELATIVE_MONITOR(abus);
    pc+=2;
    m68k_READ_L_FROM_ADDR
    break;       //what do bits 8,9,a  of extra word do?  (not always 0)
  }case 4:{
    INSTRUCTION_TIME_ROUND(8);
    m68k_src_l=m68k_fetchL();
    pc+=4;
    break;
  }default:
    ILLEGAL;
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//////////////////////////    GET DEST       ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


void m68k_get_dest_000_b(){ m68k_dest=r+PARAM_M; }
void m68k_get_dest_000_w(){ m68k_dest=r+PARAM_M; }
void m68k_get_dest_000_l(){ m68k_dest=r+PARAM_M; }
void m68k_get_dest_001_b(){ m68k_dest=areg+PARAM_M; }
void m68k_get_dest_001_w(){ m68k_dest=areg+PARAM_M; }
void m68k_get_dest_001_l(){ m68k_dest=areg+PARAM_M; }
void m68k_get_dest_010_b(){ INSTRUCTION_TIME_ROUND(4); m68k_SET_DEST_B(areg[PARAM_M]); }
void m68k_get_dest_010_w(){ INSTRUCTION_TIME_ROUND(4); m68k_SET_DEST_W(areg[PARAM_M]); }
void m68k_get_dest_010_l(){ INSTRUCTION_TIME_ROUND(8); m68k_SET_DEST_L(areg[PARAM_M]); }
void m68k_get_dest_011_b(){ INSTRUCTION_TIME_ROUND(4); m68k_SET_DEST_B(areg[PARAM_M]); areg[PARAM_M]+=1; if(PARAM_M==7)areg[7]++;}
void m68k_get_dest_011_w(){ INSTRUCTION_TIME_ROUND(4); m68k_SET_DEST_W(areg[PARAM_M]); areg[PARAM_M]+=2; }
void m68k_get_dest_011_l(){ INSTRUCTION_TIME_ROUND(8); m68k_SET_DEST_L(areg[PARAM_M]); areg[PARAM_M]+=4; }
void m68k_get_dest_100_b(){ INSTRUCTION_TIME_ROUND(6); areg[PARAM_M]-=1; if(PARAM_M==7)areg[7]--; m68k_SET_DEST_B(areg[PARAM_M]); }
void m68k_get_dest_100_w(){ INSTRUCTION_TIME_ROUND(6); areg[PARAM_M]-=2; m68k_SET_DEST_W(areg[PARAM_M]); }
void m68k_get_dest_100_l(){ INSTRUCTION_TIME_ROUND(10); areg[PARAM_M]-=4; m68k_SET_DEST_L(areg[PARAM_M]); }
void m68k_get_dest_101_b(){
  INSTRUCTION_TIME_ROUND(8);
  register signed int fw=(signed short)m68k_fetchW();pc+=2;
  m68k_SET_DEST_B(areg[PARAM_M]+fw);
}
void m68k_get_dest_101_w(){
  INSTRUCTION_TIME_ROUND(8);
  register signed int fw=(signed short)m68k_fetchW();pc+=2;
  m68k_SET_DEST_W(areg[PARAM_M]+fw);
}
void m68k_get_dest_101_l(){
  INSTRUCTION_TIME_ROUND(12);
  register signed int fw=(signed short)m68k_fetchW();pc+=2;
  m68k_SET_DEST_L(areg[PARAM_M]+fw);
}
void m68k_get_dest_110_b(){
  INSTRUCTION_TIME_ROUND(10);
  m68k_iriwo=m68k_fetchW();pc+=2;
  if(m68k_iriwo&BIT_b){  //.l
    abus=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12];
  }else{         //.w
    abus=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12];
  }
  m68k_SET_DEST_B_TO_ADDR
}
void m68k_get_dest_110_w(){
  INSTRUCTION_TIME_ROUND(10);
  m68k_iriwo=m68k_fetchW();pc+=2;
  if(m68k_iriwo&BIT_b){  //.l
    abus=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12];
  }else{         //.w
    abus=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12];
  }
  m68k_SET_DEST_W_TO_ADDR
}
void m68k_get_dest_110_l(){
  INSTRUCTION_TIME_ROUND(14);
  m68k_iriwo=m68k_fetchW();pc+=2;
  if(m68k_iriwo&BIT_b){  //.l
    abus=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12];
  }else{         //.w
    abus=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12];
  }
  m68k_SET_DEST_L_TO_ADDR
}
void m68k_get_dest_111_b(){
  switch(ir&0x7){
  case 0:{
    INSTRUCTION_TIME_ROUND(8);
    register signed int fw=(signed short)m68k_fetchW();pc+=2;
    m68k_SET_DEST_B(fw)
    break;
  }case 1:{
    INSTRUCTION_TIME_ROUND(12);
    register MEM_ADDRESS fw=m68k_fetchL();pc+=4;
    m68k_SET_DEST_B(fw)
    break;
  }default:
    ILLEGAL;
  }
}
void m68k_get_dest_111_w(){
  switch(ir&0x7){
  case 0:{
    INSTRUCTION_TIME_ROUND(8);
    register signed int fw=(signed short)m68k_fetchW();pc+=2;
    m68k_SET_DEST_W(fw)
    break;
  }case 1:{
    INSTRUCTION_TIME_ROUND(12);
    register MEM_ADDRESS fw=m68k_fetchL();pc+=4;
    m68k_SET_DEST_W(fw)
    break;
  }default:
    ILLEGAL;
  }
}
void m68k_get_dest_111_l(){
  switch(ir&0x7){
  case 0:{
    INSTRUCTION_TIME_ROUND(12);
    register signed int fw=(signed short)m68k_fetchW();pc+=2;
    m68k_SET_DEST_L(fw)
    break;
  }case 1:{
    INSTRUCTION_TIME_ROUND(16);
    register MEM_ADDRESS fw=m68k_fetchL();pc+=4;
    m68k_SET_DEST_L(fw)
    break;
  }default:
    ILLEGAL;
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//////////////////////////    GET DEST       ///////////////////////////////////
//////////////////////////  faster for D,    ///////////////////////////////////
//////////////////////////  extra fetch for  ///////////////////////////////////
//////////////////////////  others           ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


void m68k_get_dest_000_b_faster(){ INSTRUCTION_TIME(-4); m68k_dest=r+PARAM_M; }
void m68k_get_dest_000_w_faster(){ INSTRUCTION_TIME(-4); m68k_dest=r+PARAM_M; }
void m68k_get_dest_000_l_faster(){ INSTRUCTION_TIME(-4); m68k_dest=r+PARAM_M; }
void m68k_get_dest_001_b_faster(){ m68k_dest=areg+PARAM_M; EXTRA_PREFETCH}
void m68k_get_dest_001_w_faster(){ m68k_dest=areg+PARAM_M; EXTRA_PREFETCH}
void m68k_get_dest_001_l_faster(){ m68k_dest=areg+PARAM_M; EXTRA_PREFETCH}
void m68k_get_dest_010_b_faster(){ INSTRUCTION_TIME_ROUND(4); m68k_SET_DEST_B(areg[PARAM_M]); EXTRA_PREFETCH}
void m68k_get_dest_010_w_faster(){ INSTRUCTION_TIME_ROUND(4); m68k_SET_DEST_W(areg[PARAM_M]); EXTRA_PREFETCH}
void m68k_get_dest_010_l_faster(){ INSTRUCTION_TIME_ROUND(8); m68k_SET_DEST_L(areg[PARAM_M]); EXTRA_PREFETCH}
void m68k_get_dest_011_b_faster(){ INSTRUCTION_TIME_ROUND(4); m68k_SET_DEST_B(areg[PARAM_M]); areg[PARAM_M]+=1; if(PARAM_M==7)areg[7]++; EXTRA_PREFETCH}
void m68k_get_dest_011_w_faster(){ INSTRUCTION_TIME_ROUND(4); m68k_SET_DEST_W(areg[PARAM_M]); areg[PARAM_M]+=2; EXTRA_PREFETCH}
void m68k_get_dest_011_l_faster(){ INSTRUCTION_TIME_ROUND(8); m68k_SET_DEST_L(areg[PARAM_M]); areg[PARAM_M]+=4; EXTRA_PREFETCH}
void m68k_get_dest_100_b_faster(){ INSTRUCTION_TIME_ROUND(6); areg[PARAM_M]-=1; if(PARAM_M==7)areg[7]--; m68k_SET_DEST_B(areg[PARAM_M]); EXTRA_PREFETCH}
void m68k_get_dest_100_w_faster(){ INSTRUCTION_TIME_ROUND(6); areg[PARAM_M]-=2; m68k_SET_DEST_W(areg[PARAM_M]); EXTRA_PREFETCH}
void m68k_get_dest_100_l_faster(){ INSTRUCTION_TIME_ROUND(10); areg[PARAM_M]-=4; m68k_SET_DEST_L(areg[PARAM_M]); EXTRA_PREFETCH}
void m68k_get_dest_101_b_faster(){
  INSTRUCTION_TIME_ROUND(8);
  register signed int fw=(signed short)m68k_fetchW();pc+=2;
  m68k_SET_DEST_B(areg[PARAM_M]+fw);
  EXTRA_PREFETCH
}
void m68k_get_dest_101_w_faster(){
  INSTRUCTION_TIME_ROUND(8);
  register signed int fw=(signed short)m68k_fetchW();pc+=2;
  m68k_SET_DEST_W(areg[PARAM_M]+fw);
  EXTRA_PREFETCH
}
void m68k_get_dest_101_l_faster(){
  INSTRUCTION_TIME_ROUND(12);
  register signed int fw=(signed short)m68k_fetchW();pc+=2;
  m68k_SET_DEST_L(areg[PARAM_M]+fw);
  EXTRA_PREFETCH
}
void m68k_get_dest_110_b_faster(){
  INSTRUCTION_TIME_ROUND(10);
  m68k_iriwo=m68k_fetchW();pc+=2;
  if(m68k_iriwo&BIT_b){  //.l
    abus=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12];
  }else{         //.w
    abus=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12];
  }
  m68k_SET_DEST_B_TO_ADDR
  EXTRA_PREFETCH
}
void m68k_get_dest_110_w_faster(){
  INSTRUCTION_TIME_ROUND(10);
  m68k_iriwo=m68k_fetchW();pc+=2;
  if(m68k_iriwo&BIT_b){  //.l
    abus=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12];
  }else{         //.w
    abus=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12];
  }
  m68k_SET_DEST_W_TO_ADDR
  EXTRA_PREFETCH
}
void m68k_get_dest_110_l_faster(){
  INSTRUCTION_TIME_ROUND(14);
  m68k_iriwo=m68k_fetchW();pc+=2;
  if(m68k_iriwo&BIT_b){  //.l
    abus=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12];
  }else{         //.w
    abus=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12];
  }
  m68k_SET_DEST_L_TO_ADDR
  EXTRA_PREFETCH
}
void m68k_get_dest_111_b_faster(){
  switch(ir&0x7){
  case 0:{
    INSTRUCTION_TIME_ROUND(8);
    register signed int fw=(signed short)m68k_fetchW();pc+=2;
    m68k_SET_DEST_B(fw)
    EXTRA_PREFETCH
    break;
  }case 1:{
    INSTRUCTION_TIME_ROUND(12);
    register MEM_ADDRESS fw=m68k_fetchL();pc+=4;
    m68k_SET_DEST_B(fw)
    EXTRA_PREFETCH
    break;
  }default:
    ILLEGAL;
  }
}
void m68k_get_dest_111_w_faster(){
  switch(ir&0x7){
  case 0:{
    INSTRUCTION_TIME_ROUND(8);
    register signed int fw=(signed short)m68k_fetchW();pc+=2;
    m68k_SET_DEST_W(fw)
    EXTRA_PREFETCH
    break;
  }case 1:{
    INSTRUCTION_TIME_ROUND(12);
    register MEM_ADDRESS fw=m68k_fetchL();pc+=4;
    m68k_SET_DEST_W(fw)
    EXTRA_PREFETCH
    break;
  }default:
    ILLEGAL;
  }
}
void m68k_get_dest_111_l_faster(){
  switch(ir&0x7){
  case 0:{
    INSTRUCTION_TIME_ROUND(12);
    register signed int fw=(signed short)m68k_fetchW();pc+=2;
    m68k_SET_DEST_L(fw)
    EXTRA_PREFETCH
    break;
  }case 1:{
    INSTRUCTION_TIME_ROUND(16);
    register MEM_ADDRESS fw=m68k_fetchL();pc+=4;
    m68k_SET_DEST_L(fw)
    EXTRA_PREFETCH
    break;
  }default:
    ILLEGAL;
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////        CONDITION TESTS        //////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



bool m68k_condition_test_t(){return true;}
bool m68k_condition_test_f(){return false;}
bool m68k_condition_test_hi(){return ((sr&(SR_C+SR_Z))==0);}
bool m68k_condition_test_ls(){return (bool)((sr&(SR_C+SR_Z)));}
bool m68k_condition_test_cc(){return ((sr&SR_C)==0);}
bool m68k_condition_test_cs(){return (bool)(sr&SR_C);}
bool m68k_condition_test_ne(){return ((sr&SR_Z)==0);}
bool m68k_condition_test_eq(){return (bool)(sr&SR_Z);}
bool m68k_condition_test_vc(){return ((sr&SR_V)==0);}
bool m68k_condition_test_vs(){return (bool)(sr&SR_V);}
bool m68k_condition_test_pl(){return ((sr&SR_N)==0);}
bool m68k_condition_test_mi(){return (bool)(sr&SR_N);}
bool m68k_condition_test_ge(){return ((sr&(SR_N+SR_V))==0 || (sr&(SR_N+SR_V))==(SR_N+SR_V));}
bool m68k_condition_test_lt(){return ((sr&(SR_N+SR_V))==SR_V || (sr&(SR_N+SR_V))==SR_N);}
bool m68k_condition_test_gt(){return (!(sr&SR_Z) && ( ((sr&(SR_N+SR_V))==0) || ((sr&(SR_N+SR_V))==SR_N+SR_V) ));}
bool m68k_condition_test_le(){return ((sr&SR_Z) || ( ((sr&(SR_N+SR_V))==SR_N) || ((sr&(SR_N+SR_V))==SR_V) ));}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////  LINE-0 IMMEDIATE ROUTINES    //////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



void                              m68k_ori_b(){
  FETCH_TIMING;
  if ((ir & B6_111111)==B6_111100){  //to sr
    INSTRUCTION_TIME(16);
    CCR|=m68k_IMMEDIATE_B;
    sr&=SR_VALID_BITMASK;
    pc+=2;
  }else{
    INSTRUCTION_TIME(8);
    m68k_GET_IMMEDIATE_B;
    m68k_GET_DEST_B_NOT_A_FASTER_FOR_D;
    m68k_DEST_B|=m68k_src_b;
    SR_CLEAR(SR_V+SR_C+SR_N+SR_Z);
    SR_CHECK_Z_AND_N_B
  }
}
void                              m68k_ori_w(){
  FETCH_TIMING;
  if ((ir & B6_111111)==B6_111100){  //to sr
    if (SUPERFLAG){
      INSTRUCTION_TIME(16);
      sr|=m68k_IMMEDIATE_W;
      sr&=SR_VALID_BITMASK;
      pc+=2;
      DETECT_TRACE_BIT;
    }else{
      exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
    }
  }else{
    INSTRUCTION_TIME(8);
    m68k_GET_IMMEDIATE_W;
    m68k_GET_DEST_W_NOT_A_FASTER_FOR_D;
    m68k_DEST_W|=m68k_src_w;
    SR_CLEAR(SR_V+SR_C+SR_N+SR_Z);
    SR_CHECK_Z_AND_N_W
  }
}
void                              m68k_ori_l(){
  FETCH_TIMING;
  INSTRUCTION_TIME(16);
  m68k_GET_IMMEDIATE_L;
  m68k_GET_DEST_L_NOT_A_FASTER_FOR_D;
  m68k_DEST_L|=m68k_src_l;
  SR_CLEAR(SR_V+SR_C+SR_N+SR_Z);
  SR_CHECK_Z_AND_N_L
}
void                              m68k_andi_b(){
  FETCH_TIMING;
  if((ir&B6_111111)==B6_111100){  //to sr
    INSTRUCTION_TIME(16);
    CCR&=m68k_IMMEDIATE_B;
    pc+=2;
  }else{
    INSTRUCTION_TIME(8);
    m68k_GET_IMMEDIATE_B;
    m68k_GET_DEST_B_NOT_A_FASTER_FOR_D;
    m68k_DEST_B&=m68k_src_b;
    SR_CLEAR(SR_V+SR_C+SR_N+SR_Z);
    SR_CHECK_Z_AND_N_B
  }
}
void                              m68k_andi_w(){
  FETCH_TIMING;
  if((ir&B6_111111)==B6_111100){  //to sr
    if(SUPERFLAG){
      DEBUG_ONLY( int debug_old_sr=sr; )

      INSTRUCTION_TIME(16);
      sr&=m68k_IMMEDIATE_W;
      DETECT_CHANGE_TO_USER_MODE;
      pc+=2;
      ioaccess|=IOACCESS_FLAG_FOR_CHECK_INTRS;
//      check_for_interrupts_pending(); //in case we've lowered the IPL level

      CHECK_STOP_ON_USER_CHANGE
    }else exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
  }else{
    INSTRUCTION_TIME(8);
    m68k_GET_IMMEDIATE_W;
    m68k_GET_DEST_W_NOT_A_FASTER_FOR_D;
    m68k_DEST_W&=m68k_src_w;
    SR_CLEAR(SR_V+SR_C+SR_N+SR_Z);
    SR_CHECK_Z_AND_N_W
  }
}
void                              m68k_andi_l(){
  FETCH_TIMING;
  INSTRUCTION_TIME(16);
  m68k_GET_IMMEDIATE_L;
  m68k_GET_DEST_L_NOT_A_FASTER_FOR_D;
  m68k_DEST_L&=m68k_src_l;
  SR_CLEAR(SR_V+SR_C+SR_N+SR_Z);
  SR_CHECK_Z_AND_N_L
}
void                              m68k_subi_b(){
  FETCH_TIMING;
  INSTRUCTION_TIME(8);
  m68k_GET_IMMEDIATE_B;
  m68k_GET_DEST_B_NOT_A_FASTER_FOR_D;
//  SR_CLEAR(SR_USER_BYTE);
//  if((unsigned char)m68k_IMMEDIATE_B>(unsigned char)m68k_DEST_B)SR_SET(SR_C+SR_X);
  m68k_old_dest=m68k_DEST_B;
  m68k_DEST_B-=m68k_src_b;
  SR_SUB_B(SR_X);
/*
  if(m68k_DEST_B&0x80){
    SR_SET(SR_N);
  }else{
    if(wasnegative){
      SR_SET(SR_V);
    }
    if(m68k_DEST_B==0){
      SR_SET(SR_Z);
    }
  }
*/
}
void                              m68k_subi_w(){
  FETCH_TIMING;
  INSTRUCTION_TIME(8);
  m68k_GET_IMMEDIATE_W;
  m68k_GET_DEST_W_NOT_A_FASTER_FOR_D;
  m68k_old_dest=m68k_DEST_W;
  m68k_DEST_W-=m68k_src_w;
  SR_SUB_W(SR_X);
}
void                              m68k_subi_l(){
  FETCH_TIMING;
  INSTRUCTION_TIME(16);
  m68k_GET_IMMEDIATE_L;
  m68k_GET_DEST_L_NOT_A_FASTER_FOR_D;
  m68k_old_dest=m68k_DEST_L;
  m68k_DEST_L-=m68k_src_l;
  SR_SUB_L(SR_X);
}
void                              m68k_addi_b(){
  FETCH_TIMING;
  INSTRUCTION_TIME(8);
  m68k_GET_IMMEDIATE_B;
  m68k_GET_DEST_B_NOT_A_FASTER_FOR_D;
  m68k_old_dest=m68k_DEST_B;
  m68k_DEST_B+=m68k_src_b;
  SR_ADD_B;
}
void                              m68k_addi_w(){
  FETCH_TIMING;
  INSTRUCTION_TIME(8);
  m68k_GET_IMMEDIATE_W;
  m68k_GET_DEST_W_NOT_A_FASTER_FOR_D;
  m68k_old_dest=m68k_DEST_W;
  m68k_DEST_W+=m68k_src_w;
  SR_ADD_W;
}

void                              m68k_addi_l(){
  FETCH_TIMING;
  INSTRUCTION_TIME(16);
  m68k_GET_IMMEDIATE_L;
  m68k_GET_DEST_L_NOT_A_FASTER_FOR_D;
  m68k_old_dest=m68k_DEST_L;
  m68k_DEST_L+=m68k_src_l;
  SR_ADD_L;
}

void                              m68k_btst(){
  FETCH_TIMING;
  m68k_GET_IMMEDIATE_B;
  if ((ir&BITS_543)==BITS_543_000){
    INSTRUCTION_TIME(6);
    m68k_src_b&=31;
    if((r[PARAM_M]>>m68k_src_b)&1){
      SR_CLEAR(SR_Z);
    }else{
      SR_SET(SR_Z);
    }
  }else{
    INSTRUCTION_TIME(4);
    m68k_ap=(short)(m68k_src_b & 7);
//    m68k_GET_DEST_B_NOT_A;
    if((ir&(BIT_5+BIT_4+BIT_3+BIT_2+BIT_1+BIT_0))==B6_111100){  //immediate mode is the only one not allowed -
      m68k_unrecognised();
    }else{
      m68k_GET_SOURCE_B_NOT_A;
      if((m68k_src_b>>m68k_ap)&1){
        SR_CLEAR(SR_Z);
      }else{
        SR_SET(SR_Z);
      }
    }
  }
}
void                              m68k_bchg(){
  FETCH_TIMING;
  m68k_GET_IMMEDIATE_B;
  if((ir&BITS_543)==BITS_543_000){
    m68k_src_b&=31;
    if (m68k_src_b>=16){
      INSTRUCTION_TIME(8); //MAXIMUM VALUE
    }else{
      INSTRUCTION_TIME(6);
    }
    m68k_src_l=1<<m68k_src_b;
    if(r[PARAM_M]&m68k_src_l){
      SR_CLEAR(SR_Z);
    }else{
      SR_SET(SR_Z);
    }
    r[PARAM_M]^=m68k_src_l;
  }else{
    INSTRUCTION_TIME(8);
    m68k_src_b&=7;
    m68k_GET_DEST_B_NOT_A;
    m68k_src_b=(BYTE)(1<<m68k_src_b);
    if(m68k_DEST_B&m68k_src_b){
      SR_CLEAR(SR_Z);
    }else{
      SR_SET(SR_Z);
    }
    m68k_DEST_B^=(BYTE)m68k_src_b;
  }
}
void                              m68k_bclr(){
  FETCH_TIMING;
  m68k_GET_IMMEDIATE_B;
  if((ir&BITS_543)==BITS_543_000){
    m68k_src_b&=31;
    if (m68k_src_b>=16){
      INSTRUCTION_TIME(10); //MAXIMUM VALUE
    }else{
      INSTRUCTION_TIME(8);
    }
    m68k_src_l=1<<m68k_src_b;
    if(r[PARAM_M]&m68k_src_l){
      SR_CLEAR(SR_Z);
    }else{
      SR_SET(SR_Z);
    }
    r[PARAM_M]&=~m68k_src_l;
  }else{
    INSTRUCTION_TIME(8);
    m68k_src_b&=7;
    m68k_GET_DEST_B_NOT_A;
    m68k_src_b=(BYTE)(1<<m68k_src_b);
    if(m68k_DEST_B&m68k_src_b){
      SR_CLEAR(SR_Z);
    }else{
      SR_SET(SR_Z);
    }
    m68k_DEST_B&=(BYTE)(~m68k_src_b);
  }
}
void                              m68k_bset(){
  FETCH_TIMING;
  m68k_GET_IMMEDIATE_B;
  if ((ir&BITS_543)==BITS_543_000){
    m68k_src_b&=31;
    if (m68k_src_b>=16){
      INSTRUCTION_TIME(8); //MAXIMUM VALUE
    }else{
      INSTRUCTION_TIME(6);
    }
    m68k_src_l=1 << m68k_src_b;
    if (r[PARAM_M] & m68k_src_l){
      SR_CLEAR(SR_Z);
    }else{
      SR_SET(SR_Z);
    }
    r[PARAM_M]|=m68k_src_l;
  }else{
    INSTRUCTION_TIME(8);
    m68k_src_b&=7;
    m68k_GET_DEST_B_NOT_A;
    m68k_src_b=(BYTE)(1<<m68k_src_b);
    if(m68k_DEST_B&m68k_src_b){
      SR_CLEAR(SR_Z);
    }else{
      SR_SET(SR_Z);
    }
    m68k_DEST_B|=(BYTE)m68k_src_b;
  }
}
void                              m68k_eori_b(){
  FETCH_TIMING;
  if((ir&B6_111111)==B6_111100){  //to sr
    INSTRUCTION_TIME(16);
    CCR^=m68k_IMMEDIATE_B;
    sr&=SR_VALID_BITMASK;
    pc+=2;
  }else{
    INSTRUCTION_TIME(8);
    m68k_GET_IMMEDIATE_B;
    m68k_GET_DEST_B_NOT_A_FASTER_FOR_D;
    m68k_DEST_B^=m68k_src_b;
    SR_CLEAR(SR_V+SR_C+SR_N+SR_Z);
    SR_CHECK_Z_AND_N_B
  }
}
void                              m68k_eori_w(){
  FETCH_TIMING;
  if((ir&B6_111111)==B6_111100){  //to sr
    if(SUPERFLAG){
      DEBUG_ONLY( int debug_old_sr=sr; )

      INSTRUCTION_TIME(16);
      sr^=m68k_IMMEDIATE_W;
      sr&=SR_VALID_BITMASK;
      pc+=2;
      DETECT_CHANGE_TO_USER_MODE
      DETECT_TRACE_BIT;
      // Interrupts must come after trace exception
      ioaccess|=IOACCESS_FLAG_FOR_CHECK_INTRS;
//      check_for_interrupts_pending();

      CHECK_STOP_ON_USER_CHANGE;
    }else exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
  }else{
    INSTRUCTION_TIME(8);
    m68k_GET_IMMEDIATE_W;
    m68k_GET_DEST_W_NOT_A_FASTER_FOR_D;
    m68k_DEST_W^=m68k_src_w;
    SR_CLEAR(SR_V+SR_C+SR_N+SR_Z);
    SR_CHECK_Z_AND_N_W;
  }
}
void                              m68k_eori_l(){
  FETCH_TIMING;
  INSTRUCTION_TIME(16);
  m68k_GET_IMMEDIATE_L;
  m68k_GET_DEST_L_NOT_A_FASTER_FOR_D;
  m68k_DEST_L^=m68k_src_l;
  SR_CLEAR(SR_V+SR_C+SR_N+SR_Z);
  SR_CHECK_Z_AND_N_L;
}

void                              m68k_cmpi_b(){
  FETCH_TIMING;
  INSTRUCTION_TIME(4);
  m68k_GET_IMMEDIATE_B;
  m68k_old_dest=m68k_read_dest_b();
  compare_buffer=m68k_old_dest;
  m68k_dest=&compare_buffer;
  m68k_DEST_B-=m68k_src_b;

  SR_SUB_B(0);
}
void                              m68k_cmpi_w(){
  FETCH_TIMING;
  INSTRUCTION_TIME(4);
  m68k_GET_IMMEDIATE_W;
  m68k_old_dest=m68k_read_dest_w();
  compare_buffer=m68k_old_dest;
  m68k_dest=&compare_buffer;
  m68k_DEST_W-=m68k_src_w;
  SR_SUB_W(0);
}
void                              m68k_cmpi_l(){
  FETCH_TIMING;
  m68k_GET_IMMEDIATE_L;
  if(DEST_IS_REGISTER){INSTRUCTION_TIME(10);} else {INSTRUCTION_TIME(8);}
  m68k_old_dest=m68k_read_dest_l();
  compare_buffer=m68k_old_dest;
  m68k_dest=&compare_buffer;
  m68k_DEST_L-=m68k_src_l;
  SR_SUB_L(0);
}

void                              m68k_movep_w_to_dN_or_btst(){
  FETCH_TIMING;
  if((ir&BITS_543)==BITS_543_001){
    MEM_ADDRESS addr=areg[PARAM_M]+(signed short)m68k_fetchW();
    INSTRUCTION_TIME(4);

    pc+=2;
    m68k_READ_B(addr);
    DWORD_B_1(&r[PARAM_N])=m68k_src_b; //high byte
    INSTRUCTION_TIME(4);

    m68k_READ_B(addr+2);
    DWORD_B_0(&r[PARAM_N])=m68k_src_b; //low byte
    INSTRUCTION_TIME(4);

//    *( ((BYTE*)(&r[PARAM_N])) +1)=m68k_src_b; //high byte
//    m68k_READ_B(addr+2);
//    *( ((BYTE*)(&r[PARAM_N]))   )=m68k_src_b; //low byte

  }else{
    if ((ir&BITS_543)==BITS_543_000){  //btst to data register
      INSTRUCTION_TIME(2);
      if ((r[PARAM_M] >> (31 & r[PARAM_N])) & 1){
        SR_CLEAR(SR_Z);
      }else{
        SR_SET(SR_Z);
      }
    }else{ // btst memory
      m68k_GET_SOURCE_B_NOT_A;   //even immediate mode is allowed!!!!
      if( (m68k_src_b >> (7 & r[PARAM_N])) & 1){
        SR_CLEAR(SR_Z);
      }else{
        SR_SET(SR_Z);
      }
    }
  }
}

void                              m68k_movep_l_to_dN_or_bchg(){
  FETCH_TIMING;
  if((ir&BITS_543)==BITS_543_001){
    MEM_ADDRESS addr=areg[PARAM_M]+(signed short)m68k_fetchW();
    pc+=2;
    INSTRUCTION_TIME(4);

    ///// Should handle blitter here, blitter would start 1 word after busy is set
    m68k_READ_B(addr)
    DWORD_B_3(&r[PARAM_N])=m68k_src_b;
    INSTRUCTION_TIME(4);

    m68k_READ_B(addr+2)
    DWORD_B_2(&r[PARAM_N])=m68k_src_b;
    INSTRUCTION_TIME(4);

    m68k_READ_B(addr+4)
    DWORD_B_1(&r[PARAM_N])=m68k_src_b;
    INSTRUCTION_TIME(4);

    m68k_READ_B(addr+6)
    DWORD_B_0(&r[PARAM_N])=m68k_src_b;
    INSTRUCTION_TIME(4);

  }else{
    if((ir&BITS_543)==BITS_543_000){
      m68k_src_w=BYTE(LOBYTE(r[PARAM_N]) & 31);
      if (m68k_src_w>=16){
        INSTRUCTION_TIME(4); //MAXIMUM VALUE
      }else{
        INSTRUCTION_TIME(2);
      }
      if((r[PARAM_M]>>(m68k_src_w))&1){
        SR_CLEAR(SR_Z);
      }else{
        SR_SET(SR_Z);
      }
      r[PARAM_M]^=(1<<m68k_src_w);
    }else{
      INSTRUCTION_TIME(4);
      m68k_GET_DEST_B_NOT_A;
      if((m68k_DEST_B>>(7&r[PARAM_N]))&1){
        SR_CLEAR(SR_Z);
      }else{
        SR_SET(SR_Z);
      }
      m68k_DEST_B^=(signed char)(1<<(7&r[PARAM_N]));
    }
  }
}
void                              m68k_movep_w_from_dN_or_bclr(){
  FETCH_TIMING;
  if ((ir & BITS_543)==BITS_543_001){
    MEM_ADDRESS ad=areg[PARAM_M]+(short)m68k_fetchW();
    pc+=2;
    INSTRUCTION_TIME(4);

    INSTRUCTION_TIME(4);
    m68k_poke(ad,DWORD_B_1(&r[PARAM_N]));
    ad+=2;

    INSTRUCTION_TIME(4);
    m68k_poke(ad,DWORD_B_0(&r[PARAM_N]));
  }else{
    if((ir&BITS_543)==BITS_543_000){
      m68k_src_w=BYTE(LOBYTE(r[PARAM_N]) & 31);
      if (m68k_src_w>=16){
        INSTRUCTION_TIME(6); //MAXIMUM VALUE
      }else{
        INSTRUCTION_TIME(4);
      }
      if((r[PARAM_M]>>(m68k_src_w))&1){
        SR_CLEAR(SR_Z);
      }else{
        SR_SET(SR_Z);
      }
      r[PARAM_M]&=(long)~((long)(1<<m68k_src_w));

      //length = .l
    }else{
      INSTRUCTION_TIME(4);
      m68k_GET_DEST_B_NOT_A;
      if((m68k_DEST_B>>(7&r[PARAM_N]))&1){
        SR_CLEAR(SR_Z);
      }else{
        SR_SET(SR_Z);
      }
      m68k_DEST_B&=(signed char)~(1<<(7&r[PARAM_N]));
    }
  }
}
void                              m68k_movep_l_from_dN_or_bset(){
  FETCH_TIMING;
  if ((ir&BITS_543)==BITS_543_001){
    MEM_ADDRESS ad=areg[PARAM_M]+(signed short)m68k_fetchW();
    pc+=2;
    INSTRUCTION_TIME(4);

    BYTE *p=(BYTE*)(&r[PARAM_N]);
    INSTRUCTION_TIME(4);
    m68k_poke(ad,DWORD_B_3(p));
    ad+=2;

    INSTRUCTION_TIME(4);
    m68k_poke(ad,DWORD_B_2(p));
    ad+=2;

    INSTRUCTION_TIME(4);
    m68k_poke(ad,DWORD_B_1(p));
    ad+=2;

    INSTRUCTION_TIME(4);
    m68k_poke(ad,DWORD_B_0(p));
  }else{
    if((ir&BITS_543)==BITS_543_000){
      m68k_src_w=BYTE(LOBYTE(r[PARAM_N]) & 31);
      if (m68k_src_w>=16){
        INSTRUCTION_TIME(4); //MAXIMUM VALUE
      }else{
        INSTRUCTION_TIME(2);
      }
      if((r[PARAM_M]>>(m68k_src_w))&1){
        SR_CLEAR(SR_Z);
      }else{
        SR_SET(SR_Z);
      }

      r[PARAM_M]|=(1<<m68k_src_w);


    //    m68k_dest=// D2_dM;
      //length = .l
    }else{
      INSTRUCTION_TIME(4);
      m68k_GET_DEST_B_NOT_A;
      if((m68k_DEST_B>>(7&r[PARAM_N]))&1){
        SR_CLEAR(SR_Z);
      }else{
        SR_SET(SR_Z);
      }
      m68k_DEST_B|=(signed char)(1<<(7&r[PARAM_N]));
    }
  }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////        LINE-4 ROUTINES        //////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


void                              m68k_negx_b(){
  FETCH_TIMING;
  m68k_GET_DEST_B_NOT_A;
  m68k_old_dest=m68k_DEST_B;
  m68k_DEST_B=(BYTE)-m68k_DEST_B;
  if(sr&SR_X)m68k_DEST_B--;
  SR_CLEAR(SR_X+SR_N+SR_V+SR_C);
  if(m68k_DEST_B)SR_CLEAR(SR_Z);
  if(m68k_old_dest&m68k_DEST_B&MSB_B)SR_SET(SR_V);
  if((m68k_old_dest|m68k_DEST_B)&MSB_B)SR_SET(SR_C+SR_X);
  if(m68k_DEST_B & MSB_B)SR_SET(SR_N);
}void                             m68k_negx_w(){
  FETCH_TIMING;
  m68k_GET_DEST_W_NOT_A;
  m68k_old_dest=m68k_DEST_W;
  m68k_DEST_W=(WORD)-m68k_DEST_W;
  if(sr&SR_X)m68k_DEST_W--;
  SR_CLEAR(SR_X+SR_N+SR_V+SR_C);
  if(m68k_DEST_W)SR_CLEAR(SR_Z);
  if(m68k_old_dest&m68k_DEST_W&MSB_W)SR_SET(SR_V);
  if((m68k_old_dest|m68k_DEST_W)&MSB_W)SR_SET(SR_C+SR_X);
  if(m68k_DEST_W & MSB_W)SR_SET(SR_N);
}void                             m68k_negx_l(){
  FETCH_TIMING;
  INSTRUCTION_TIME(2);
  m68k_GET_DEST_L_NOT_A;
  m68k_old_dest=m68k_DEST_L;
  m68k_DEST_L=-m68k_DEST_L;
  if(sr&SR_X)m68k_DEST_L-=1;
  SR_CLEAR(SR_X+SR_N+SR_V+SR_C);
  if(m68k_DEST_L)SR_CLEAR(SR_Z);
  if(m68k_old_dest&m68k_DEST_L&MSB_L)SR_SET(SR_V);
  if((m68k_old_dest|m68k_DEST_L)&MSB_L)SR_SET(SR_C+SR_X);
  if(m68k_DEST_L & MSB_L)SR_SET(SR_N);
}
void                              m68k_clr_b(){
  FETCH_TIMING;
  if (DEST_IS_REGISTER==0){INSTRUCTION_TIME(4);}
  m68k_GET_DEST_B_NOT_A;
  m68k_DEST_B=0;
  SR_CLEAR(SR_N+SR_V+SR_C);
  SR_SET(SR_Z);
}void                             m68k_clr_w(){
  FETCH_TIMING;
  if(DEST_IS_REGISTER==0){INSTRUCTION_TIME(4);}
  m68k_GET_DEST_W_NOT_A;
  m68k_DEST_W=0;
  SR_CLEAR(SR_N+SR_V+SR_C);
  SR_SET(SR_Z);
}void                             m68k_clr_l(){
  FETCH_TIMING;
  if(DEST_IS_REGISTER){INSTRUCTION_TIME(2);}else {INSTRUCTION_TIME(8);}
  m68k_GET_DEST_L_NOT_A;
  m68k_DEST_L=0;
  SR_CLEAR(SR_N+SR_V+SR_C);
  SR_SET(SR_Z);
}
void                              m68k_neg_b(){
  FETCH_TIMING;
  if(DEST_IS_REGISTER==0){INSTRUCTION_TIME(4);}
  m68k_GET_DEST_B_NOT_A;
  m68k_old_dest=m68k_DEST_B;
  m68k_DEST_B=(BYTE)-m68k_DEST_B;
  SR_CLEAR(SR_USER_BYTE);
  if(m68k_old_dest&m68k_DEST_B&MSB_B)SR_SET(SR_V);
  if((m68k_old_dest|m68k_DEST_B)&MSB_B)SR_SET(SR_C+SR_X);
  SR_CHECK_Z_AND_N_B;
}void                             m68k_neg_w(){
  FETCH_TIMING;
  if(DEST_IS_REGISTER==0){INSTRUCTION_TIME(4);}
  m68k_GET_DEST_W_NOT_A;
  m68k_old_dest=m68k_DEST_W;
  m68k_DEST_W=(WORD)-m68k_DEST_W;
  SR_CLEAR(SR_USER_BYTE);
  if(m68k_old_dest&m68k_DEST_W&MSB_W)SR_SET(SR_V);
  if((m68k_old_dest|m68k_DEST_W)&MSB_W)SR_SET(SR_C+SR_X);
  SR_CHECK_Z_AND_N_W;
}void                             m68k_neg_l(){
  FETCH_TIMING;
  if(DEST_IS_REGISTER){INSTRUCTION_TIME(2);}else {INSTRUCTION_TIME(8);}
  m68k_GET_DEST_L_NOT_A;
  m68k_old_dest=m68k_DEST_L;
  m68k_DEST_L=-m68k_DEST_L;
  SR_CLEAR(SR_USER_BYTE);
  if(m68k_old_dest&m68k_DEST_L&MSB_L)SR_SET(SR_V);
  if((m68k_old_dest|m68k_DEST_L)&MSB_L)SR_SET(SR_C+SR_X);
  SR_CHECK_Z_AND_N_L;
}
void                              m68k_not_b(){
  FETCH_TIMING;
  if(DEST_IS_REGISTER==0){INSTRUCTION_TIME(4);}
  m68k_GET_DEST_B_NOT_A;
  m68k_DEST_B=(BYTE)~m68k_DEST_B;
  SR_CLEAR(SR_N+SR_Z+SR_V+SR_C);
  SR_CHECK_Z_AND_N_B;
}void                             m68k_not_w(){
  FETCH_TIMING;
  if(DEST_IS_REGISTER==0){INSTRUCTION_TIME(4);}
  m68k_GET_DEST_W_NOT_A;
  m68k_DEST_W=(WORD)~m68k_DEST_W;
  SR_CLEAR(SR_N+SR_Z+SR_V+SR_C);
  SR_CHECK_Z_AND_N_W;
}void                             m68k_not_l(){
  FETCH_TIMING;
  if (DEST_IS_REGISTER){INSTRUCTION_TIME(2);}else {INSTRUCTION_TIME(8);}
  m68k_GET_DEST_L_NOT_A;
  m68k_DEST_L=~m68k_DEST_L;
  SR_CLEAR(SR_N+SR_Z+SR_V+SR_C);
  SR_CHECK_Z_AND_N_L;
}
void                              m68k_tst_b(){
  FETCH_TIMING;
  BYTE x=m68k_read_dest_b();
  SR_CLEAR(SR_N+SR_Z+SR_V+SR_C);
  if(!x)SR_SET(SR_Z);
  if(x&MSB_B)SR_SET(SR_N);
}void                             m68k_tst_w(){
  FETCH_TIMING;
  WORD x=m68k_read_dest_w();
  SR_CLEAR(SR_N+SR_Z+SR_V+SR_C);
  if(!x)SR_SET(SR_Z);
  if(x&MSB_W)SR_SET(SR_N);
}void                             m68k_tst_l(){
  FETCH_TIMING;
  LONG x=m68k_read_dest_l();
  SR_CLEAR(SR_N+SR_Z+SR_V+SR_C);
  if(!x)SR_SET(SR_Z);
  if(x&MSB_L)SR_SET(SR_N);
}
void                              m68k_tas(){
  if((ir&B6_111111)==B6_111100){
    ILLEGAL;
  }else{
    FETCH_TIMING;
    m68k_GET_DEST_B_NOT_A;
    if(DEST_IS_REGISTER==0){INSTRUCTION_TIME(10);} /// Should this be 6?
    SR_CLEAR(SR_N+SR_Z+SR_V+SR_C);
    SR_CHECK_Z_AND_N_B;
    m68k_DEST_B|=MSB_B;
  }
}
void                              m68k_move_from_sr(){
  FETCH_TIMING;
  if (DEST_IS_REGISTER){
    INSTRUCTION_TIME(2);
  }else{
    INSTRUCTION_TIME(4);
  }
  m68k_GET_DEST_W_NOT_A;
  m68k_DEST_W=sr;
}
void                              m68k_move_from_ccr(){
  ILLEGAL;  //68010 only!!!
/*
  m68k_GET_DEST_B_NOT_A;
  m68k_DEST_B=LOBYTE(sr);
*/
}
void                              m68k_move_to_ccr(){
  if((ir&BITS_543)==BITS_543_001){
    m68k_unrecognised();
  }else{
    FETCH_TIMING;
    m68k_GET_SOURCE_W;
    CCR=LOBYTE(m68k_src_w);
    sr&=SR_VALID_BITMASK;
    INSTRUCTION_TIME(8);
  }
}
void                              m68k_move_to_sr(){
  if(SUPERFLAG){
    if((ir&BITS_543)==BITS_543_001){ //address register
      m68k_unrecognised();
    }else{
      DEBUG_ONLY( int debug_old_sr=sr; )

      FETCH_TIMING;
      INSTRUCTION_TIME(8);
      m68k_GET_SOURCE_W;
      sr=m68k_src_w;
      sr&=SR_VALID_BITMASK;

      DETECT_CHANGE_TO_USER_MODE;
      DETECT_TRACE_BIT;
      // Interrupts must come after trace exception
      ioaccess|=IOACCESS_FLAG_FOR_CHECK_INTRS;
//      check_for_interrupts_pending();

      CHECK_STOP_ON_USER_CHANGE;
    }
  }else{
    exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
  }
}
void                              m68k_nbcd(){
  FETCH_TIMING;
  if (DEST_IS_REGISTER){INSTRUCTION_TIME(2);}else {INSTRUCTION_TIME(4);}
  m68k_GET_DEST_B_NOT_A;
  int m=m68k_DEST_B,n=0;
  if(m&0xff) n=0xa0;
  if(m&0xf)n=0x9a;
  if(sr&SR_X)n=0x99;

  SR_CLEAR(SR_X+SR_C);
  if(m)SR_SET(SR_X+SR_C); //there will be a carry
  m68k_DEST_B=(BYTE)(n-m);
  if(m68k_DEST_B){SR_CLEAR(SR_Z);}
}
void                              m68k_pea_or_swap(){
  FETCH_TIMING;
  if((ir&BITS_543)==BITS_543_000){
    r[PARAM_M]=MAKELONG(HIWORD(r[PARAM_M]),LOWORD(r[PARAM_M]));
    SR_CLEAR(SR_N+SR_Z+SR_V+SR_C);
    if(!r[PARAM_M])SR_SET(SR_Z);
    if(r[PARAM_M]&MSB_L)SR_SET(SR_N);
  }else{
    // pea instruction times table.
    // ad.mode  time  Steem EA time difference
    // (aN)     12    0             12
    // D(aN)    16    4             12
    // D(aN,dM) 22    8             14
    // xxx.w    16    4             12
    // xxx.l    20    8             12
    // D(pc)    16    4             12
    // D(pc,dM) 22    8             14
    if ((ir & B6_111111)==B6_111011 || (ir & B6_111000)==B6_110000){ INSTRUCTION_TIME(2); } //iriwo
    m68k_get_effective_address();

    INSTRUCTION_TIME_ROUND(8); // Round before writing to memory
    m68k_PUSH_L(effective_address);

    FETCH_TIMING;
  }
}
void                              m68k_movem_w_from_regs_or_ext_w(){
  FETCH_TIMING;
  if((ir&BITS_543)==BITS_543_000){
    SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
    m68k_dest=&(r[PARAM_M]);
    m68k_DEST_W=(signed short)((signed char)LOBYTE(r[PARAM_M]));
    SR_CHECK_Z_AND_N_W;
  }else if ((ir & BITS_543)==BITS_543_100){ //predecrement
    m68k_src_w=m68k_fetchW();pc+=2;
    INSTRUCTION_TIME(4);

    MEM_ADDRESS ad=areg[PARAM_M];
    DWORD areg_hi=(areg[PARAM_M] & 0xff000000);
    short mask=1,BlitterStart=0;
    for (int n=0;n<16;n++){
      if (m68k_src_w & mask){
        ad-=2;
        m68k_dpoke(ad,LOWORD(r[15-n]));
        INSTRUCTION_TIME(4);
        if (ioaccess & IOACCESS_FLAG_DO_BLIT){
          // After word that starts blitter must write one more word, then blit
          if ((++BlitterStart)==2){
            Blitter_Start_Now();
            BlitterStart=0;
          }
        }
      }
      mask<<=1;
    }
    // The register written to memory should be the original one, so
    // predecrement afterwards.
    areg[PARAM_M]=ad | areg_hi;
  }else{
    m68k_src_w=m68k_fetchW();pc+=2;
    INSTRUCTION_TIME(4);
    MEM_ADDRESS ad;

    switch (ir & BITS_543){
    case BITS_543_010:
      ad=areg[PARAM_M];
      break;
    case BITS_543_101:
      INSTRUCTION_TIME(4);
      ad=areg[PARAM_M]+(signed short)m68k_fetchW();
      pc+=2;
      break;
    case BITS_543_110:
      INSTRUCTION_TIME(6);
      m68k_iriwo=m68k_fetchW();pc+=2;
      if(m68k_iriwo&BIT_b){  //.l
        ad=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12];
      }else{         //.w
        ad=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12];
      }
      break;
    case BITS_543_111:
      switch(ir&0x7){
      case 0:
        INSTRUCTION_TIME(4);
        ad=0xffffff&(unsigned long)((signed long)((signed short)m68k_fetchW()));
        pc+=2;
        break;
      case 1:
        INSTRUCTION_TIME(8);
        ad=0xffffff & m68k_fetchL();
        pc+=4;
        break;
      default:
        ad=0; // This is to stop an annoying warning
        m68k_unrecognised();
        break;
      }
      break;
    default:
      ad=0; // This is to stop an annoying warning
      m68k_unrecognised();
    }
    short mask=1,BlitterStart=0;
    for (int n=0;n<16;n++){
      if (m68k_src_w & mask){
        m68k_dpoke(ad,LOWORD(r[n]));
        INSTRUCTION_TIME(4);
        ad+=2;
        if (ioaccess & IOACCESS_FLAG_DO_BLIT){
          // After word that starts blitter must write one more word, then blit
          if ((++BlitterStart)==2){
            Blitter_Start_Now();
            BlitterStart=0;
          }
        }
      }
      mask<<=1;
    }
  }
  if (ioaccess & IOACCESS_FLAG_PSG_BUS_JAM_W){  //oh dear, writing multiple words to the PSG
    int s=count_bits_set_in_word(m68k_src_w);
    if (s>4) BUS_JAM_TIME((s-1) & -4);  //we've already had a bus jam of 4, for s=5..8 want extra bus jam of 4
  }
}

void                              m68k_movem_l_from_regs_or_ext_l(){
  FETCH_TIMING;
  if((ir&BITS_543)==BITS_543_000){  //ext.l
    SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
    m68k_dest=&(r[PARAM_M]);
    m68k_DEST_L=(signed long)((signed short)LOWORD(r[PARAM_M]));
    SR_CHECK_Z_AND_N_L;
  }else if((ir&BITS_543)==BITS_543_100){ //predecrement
    m68k_src_w=m68k_fetchW();pc+=2;
    INSTRUCTION_TIME(4);

    MEM_ADDRESS ad=areg[PARAM_M];
    DWORD areg_hi=(areg[PARAM_M] & 0xff000000);
    short mask=1;
    for (int n=0;n<16;n++){
      if (m68k_src_w & mask){
        ad-=4;
        INSTRUCTION_TIME(4);
        m68k_lpoke(ad,r[15-n]);
        INSTRUCTION_TIME(4);
        if (ioaccess & IOACCESS_FLAG_DO_BLIT) Blitter_Start_Now();
      }
      mask<<=1;
    }
    // The register written to memory should be the original one, so
    // predecrement afterwards.
    areg[PARAM_M]=ad | areg_hi;
  }else{
    m68k_src_w=m68k_fetchW();pc+=2;
    INSTRUCTION_TIME(4);
    MEM_ADDRESS ad;

    switch(ir&BITS_543){
    case BITS_543_010:
      ad=areg[PARAM_M];
      break;
    case BITS_543_101:
      INSTRUCTION_TIME(4);
      ad=areg[PARAM_M]+(signed short)m68k_fetchW();
      pc+=2;
      break;
    case BITS_543_110:
      m68k_ap=m68k_fetchW();pc+=2;
      INSTRUCTION_TIME(6);
      if(m68k_ap&BIT_b){  //.l
        ad=areg[PARAM_M]+(signed char)LOBYTE(m68k_ap)+(int)r[m68k_ap>>12];
      }else{         //.w
        ad=areg[PARAM_M]+(signed char)LOBYTE(m68k_ap)+(signed short)r[m68k_ap>>12];
      }
      break;
    case BITS_543_111:
      switch(ir&0x7){
      case 0:
        ad=0xffffff&(unsigned long)((signed long)((signed short)m68k_fetchW()));
        INSTRUCTION_TIME(4);
        pc+=2;
        break;
      case 1:
        ad=0xffffff&m68k_fetchL();
        INSTRUCTION_TIME(8);
        pc+=4;
        break;
      default:
        ad=0; // This is to stop an annoying warning
        m68k_unrecognised();
        break;
      }
      break;
    default:
      ad=0; // This is to stop an annoying warning
      m68k_unrecognised();
    }
    short mask=1;
    for (int n=0;n<16;n++){
      if (m68k_src_w&mask){
        INSTRUCTION_TIME_ROUND(4);
        m68k_lpoke(ad,r[n]);
        ad+=4;
        INSTRUCTION_TIME(4);
      }
      mask<<=1;
    }
  }
  if (ioaccess & IOACCESS_FLAG_PSG_BUS_JAM_W){  //oh dear, writing multiple longs to the PSG
    int s=count_bits_set_in_word(m68k_src_w)*2; //number of words to write
    if(s>4)BUS_JAM_TIME((s-1)&-4);  //we've already had a bus jam of 4, for s=5..8 want extra bus jam of 4
  }

}
void                              m68k_movem_l_to_regs(){
  FETCH_TIMING;
  bool postincrement=false;
  m68k_src_w=m68k_fetchW();pc+=2;
  INSTRUCTION_TIME(4);

  MEM_ADDRESS ad;
  switch(ir&BITS_543){
  case BITS_543_010:
    ad=areg[PARAM_M];
    break;
  case BITS_543_011:
    ad=areg[PARAM_M];
    postincrement=true;
    break;
  case BITS_543_101:
    ad=areg[PARAM_M]+(signed short)m68k_fetchW();
    INSTRUCTION_TIME(4);
    pc+=2;
    break;
  case BITS_543_110:
    INSTRUCTION_TIME(6);
    m68k_iriwo=m68k_fetchW();pc+=2;
    if(m68k_iriwo&BIT_b){  //.l
      ad=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12];
    }else{         //.w
      ad=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12];
    }
    break;
  case BITS_543_111:
    switch(ir&0x7){
    case 0:
      INSTRUCTION_TIME(4);
      ad=0xffffff&(unsigned long)((signed long)((signed short)m68k_fetchW()));
      pc+=2;
      break;
    case 1:
      INSTRUCTION_TIME(8);
      ad=0xffffff&m68k_fetchL();
      pc+=4;
      break;
    case 2:
      INSTRUCTION_TIME(4);
      ad=pc+(signed short)m68k_fetchW();
      pc+=2;
      break;
    case 3:
      INSTRUCTION_TIME(6);
      m68k_iriwo=m68k_fetchW();
      if(m68k_iriwo&BIT_b){  //.l
        ad=pc+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12];
      }else{         //.w
        ad=pc+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12];
      }
      pc+=2;
      break;
    default:
      ad=0; // This is to stop an annoying warning
      m68k_unrecognised();
      break;
    }
    break;
  default:
    ad=0; // This is to stop an annoying warning
    m68k_unrecognised();
  }
  DWORD areg_hi=(areg[PARAM_M] & 0xff000000);
  short mask=1;
  for (int n=0;n<16;n++){
    if (m68k_src_w & mask){
      INSTRUCTION_TIME_ROUND(4);
      r[n]=m68k_lpeek(ad);
      INSTRUCTION_TIME(4);
      ad+=4;
    }
    mask<<=1;
  }
  if (postincrement) areg[PARAM_M]=ad | areg_hi;
  m68k_dpeek(ad); //extra word read (discarded)
  INSTRUCTION_TIME_ROUND(4);
  if (ioaccess & IOACCESS_FLAG_PSG_BUS_JAM_R){  //oh dear, reading multiple longs from the PSG
    int s=count_bits_set_in_word(m68k_src_w)*2+1; //number of words read
    if(s>4)BUS_JAM_TIME((s-1)&-4);  //we've already had a bus jam of 4, for s=5..8 want extra bus jam of 4
  }
}
void                              m68k_movem_w_to_regs(){
  FETCH_TIMING;
  bool postincrement=false;
  INSTRUCTION_TIME(4);
  m68k_src_w=m68k_fetchW();pc+=2;

  MEM_ADDRESS ad;
  switch (ir & BITS_543){
  case BITS_543_010:
    ad=areg[PARAM_M];
    break;
  case BITS_543_011:
    ad=areg[PARAM_M];
    postincrement=true;
    break;
  case BITS_543_101:
    INSTRUCTION_TIME(4);
    ad=areg[PARAM_M]+(signed short)m68k_fetchW();
    pc+=2;
    break;
  case BITS_543_110:
    INSTRUCTION_TIME(6);
    m68k_iriwo=m68k_fetchW();pc+=2;
    if(m68k_iriwo&BIT_b){  //.l
      ad=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12];
    }else{         //.w
      ad=areg[PARAM_M]+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12];
    }
    break;
  case BITS_543_111:
    switch(ir&0x7){
    case 0:
      INSTRUCTION_TIME(4);
      ad=0xffffff&(unsigned long)((signed long)((signed short)m68k_fetchW()));
      pc+=2;
      break;
    case 1:
      INSTRUCTION_TIME(8);
      ad=0xffffff&m68k_fetchL();
      pc+=4;
      break;
    case 2:
      INSTRUCTION_TIME(4);
      ad=pc+(signed short)m68k_fetchW();
      pc+=2;
      break;
    case 3:
      INSTRUCTION_TIME(6);

      m68k_iriwo=m68k_fetchW();
      if(m68k_iriwo&BIT_b){  //.l
        ad=pc+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12];
      }else{         //.w
        ad=pc+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12];
      }
      pc+=2;
      // m68k_src_w=// D2_IRIWO_PC;
      break;
    default:
      ad=0; // This is to stop an annoying warning
      m68k_unrecognised();
      break;
    }
    break;
  default:
    ad=0; // This is to stop an annoying warning
    m68k_unrecognised();
  }
  DWORD areg_hi=(areg[PARAM_M] & 0xff000000);
  short mask=1;
  for(int n=0;n<16;n++){
    if (m68k_src_w & mask){
      r[n]=(signed long)((signed short)m68k_dpeek(ad));
      INSTRUCTION_TIME_ROUND(4);
      ad+=2;
    }
    mask<<=1;
  }
  if (postincrement) areg[PARAM_M]=ad | areg_hi;
  m68k_dpeek(ad); //extra word read (discarded)
  INSTRUCTION_TIME_ROUND(4);
  if (ioaccess & IOACCESS_FLAG_PSG_BUS_JAM_R){  //oh dear, reading multiple words from the PSG
    int s=count_bits_set_in_word(m68k_src_w)+1; //number of words read
    if(s>4)BUS_JAM_TIME((s-1)&-4);  //we've already had a bus jam of 4, for s=5..8 want extra bus jam of 4
  }
}
void                              m68k_jsr()
{
  // see jmp instruction times table and +8.
  if((ir&B6_111111)==B6_111001){ INSTRUCTION_TIME(0);}
  else if((ir&BITS_543)==BITS_543_010){ INSTRUCTION_TIME(4);}
  else {INSTRUCTION_TIME(2);}

  m68k_get_effective_address();
  INSTRUCTION_TIME_ROUND(8);
  m68k_PUSH_L(PC32);
  FETCH_TIMING; // Fetch from new address before setting PC
  m68k_READ_W(effective_address); // Check for bus/address errors
  SET_PC(effective_address);
  intercept_os();
}
void                              m68k_jmp()
{
  // jmp instruction times table.
  // ad.mode  time  Steem EA time difference
  // (aN)     8     0             8
  // D(aN)    10    4             6
  // D(aN,dM) 14    8             6
  // xxx.w    10    4             6
  // xxx.l    12    8             4
  // D(pc)    10    4             6
  // D(pc,dM) 14    8             6
  if((ir&B6_111111)==B6_111001){ INSTRUCTION_TIME(0);}  // abs.l
  else if((ir&BITS_543)==BITS_543_010){ INSTRUCTION_TIME(4);} // (an)
  else {INSTRUCTION_TIME(2);}

  m68k_get_effective_address();
  FETCH_TIMING; // Fetch from new address before setting PC
  m68k_READ_W(effective_address); // Check for bus/address errors
  SET_PC(effective_address);
  intercept_os();
}
void                              m68k_chk(){
  FETCH_TIMING;
  if((ir&BITS_543)==BITS_543_001){
    m68k_unrecognised();
  }else{
    m68k_GET_SOURCE_W;
    INSTRUCTION_TIME(4);
    if(r[PARAM_N]&0x8000){
      SR_SET(SR_N);
      m68k_interrupt(LPEEK(BOMBS_CHK*4));
      INSTRUCTION_TIME_ROUND(40);
    }else if((signed short)LOWORD(r[PARAM_N])>(signed short)m68k_src_w){
      SR_CLEAR(SR_N);
      m68k_interrupt(LPEEK(BOMBS_CHK*4));
      INSTRUCTION_TIME_ROUND(40);
    }
  }
}
void                              m68k_lea(){
  // lea instruction times table.
  // ad.mode  time  Steem EA time difference
  // (aN)     4     0             4
  // D(aN)    8     4             4
  // D(aN,dM) 14    8             6
  // xxx.w    8     4             4
  // xxx.l    12    8             4
  // D(pc)    8     4             4
  // D(pc,dM) 14    8             6

  if ((ir & B6_111111)==B6_111011 || (ir & B6_111000)==B6_110000){ INSTRUCTION_TIME(2); }
  m68k_get_effective_address();
  areg[PARAM_N]=effective_address;
  FETCH_TIMING; /// This seems strange but it is right, it fetches after instruction
}


void                              m68k_line_4_stuff(){
  m68k_jump_line_4_stuff[ir&(BITS_543|0x7)]();
}

#define LOGSECTION LOGSECTION_TRAP
void                              m68k_trap(){
  INSTRUCTION_TIME_ROUND(8); // Time to read address to jump to
  MEM_ADDRESS Vector=LPEEK( 0x80+((ir & 0xf)*4) );

  switch (ir & 0xf){
    case 1: //GEMDOS
      if (os_gemdos_vector==0) if (Vector>=rom_addr) os_gemdos_vector=Vector;
      break;
    case 13: // BIOS
      if (os_bios_vector==0) if (Vector>=rom_addr) os_bios_vector=Vector;
      break;
    case 14: // XBIOS
      if (os_xbios_vector==0) if (Vector>=rom_addr) os_xbios_vector=Vector;
      break;
  }
  m68k_interrupt(Vector);
  INSTRUCTION_TIME_ROUND(26);
  intercept_os();
  debug_check_break_on_irq(BREAK_IRQ_TRAP_IDX);
}
#undef LOGSECTION

void                              m68k_link(){
  FETCH_TIMING;
  INSTRUCTION_TIME(12);
  m68k_GET_IMMEDIATE_W;
  m68k_PUSH_L(areg[PARAM_M]);
  areg[PARAM_M]=r[15];
  r[15]+=(signed short)m68k_src_w;
}
void                              m68k_unlk(){
  FETCH_TIMING;
  INSTRUCTION_TIME(8);
  r[15]=areg[PARAM_M];
  abus=r[15];m68k_READ_L_FROM_ADDR;
  areg[PARAM_M]=m68k_src_l;
  r[15]+=4;
}
void                              m68k_move_to_usp(){
  FETCH_TIMING;
  if (SUPERFLAG){
    other_sp=areg[PARAM_M];
  }else{
    exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
  }
}
void                              m68k_move_from_usp(){
  FETCH_TIMING;
  if (SUPERFLAG){
    areg[PARAM_M]=other_sp;
  }else{
    exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
  }
}
void                              m68k_reset(){
  FETCH_TIMING;
  if (SUPERFLAG){
    reset_peripherals();
    INSTRUCTION_TIME(128);
  }else{
    exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
  }
}
void                              m68k_nop(){
  FETCH_TIMING;
  prefetched_2=false;
  prefetch_buf[0]=*(lpfetch-MEM_DIR);  //flush prefetch queue
}
void                              m68k_stop(){
  if (SUPERFLAG){
    if (cpu_stopped==0){
      FETCH_TIMING;
      m68k_GET_IMMEDIATE_W;
      INSTRUCTION_TIME_ROUND(4); // time for immediate fetch

      DEBUG_ONLY( int debug_old_sr=sr; )

      sr=m68k_src_w;
      sr&=SR_VALID_BITMASK;
      DETECT_CHANGE_TO_USER_MODE;
      cpu_stopped=true;

      SET_PC((pc-4) | pc_high_byte);

      DETECT_TRACE_BIT;
      // Interrupts must come after trace exception
      ioaccess|=IOACCESS_FLAG_FOR_CHECK_INTRS;
//      check_for_interrupts_pending();

      CHECK_STOP_ON_USER_CHANGE;
    }else{
      SET_PC((pc-2) | pc_high_byte);
      // If we have got here then there were no interrupts pending when the IPL
      // was changed. Now unless we are in a blitter loop nothing can possibly
      // happen until cpu_cycles<=0.
      if (Blit.Busy){
        INSTRUCTION_TIME_ROUND(4);
      }else if (cpu_cycles>0){
        cpu_cycles=0; // It takes 0 cycles to unstop
      }
    }
  }else{
    exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
  }
}
void                              m68k_rte(){
  bool dont_intercept_os=false;
  if (SUPERFLAG){

    DEBUG_ONLY( int debug_old_sr=sr; )

    INSTRUCTION_TIME_ROUND(20);

    M68K_PERFORM_RTE(;);

    log_to(LOGSECTION_INTERRUPTS,Str("INTERRUPT: ")+HEXSl(old_pc,6)+" - RTE to "+HEXSl(pc,6)+" sr="+HEXSl(sr,4)+
                                  " at "+ABSOLUTE_CPU_TIME+" idepth="+interrupt_depth);
    if (on_rte){
      if (on_rte_interrupt_depth==interrupt_depth){  //is this the RTE we want?
        switch (on_rte){
#ifdef _DEBUG_BUILD
          case ON_RTE_STOP:
            if (runstate==RUNSTATE_RUNNING){
              runstate=RUNSTATE_STOPPING;
              SET_WHY_STOP(HEXSl(old_pc,6)+": RTE");
            }
            on_rte=ON_RTE_RTE;
            break;
#endif
#ifndef DISABLE_STEMDOS
          case ON_RTE_STEMDOS:
            stemdos_rte();
            dont_intercept_os=true;
            break;
#endif

#ifndef NO_CRAZY_MONITOR
          case ON_RTE_LINE_A:
            on_rte=ON_RTE_RTE;
            SET_PC(on_rte_return_address);
            extended_monitor_hack();

            break;
          case ON_RTE_DONE_MALLOC_FOR_EM:
//            log_write(HEXSl(pc,6)+EasyStr(": Malloc done - returned $")+HEXSl(r[0],8));
            xbios2=(r[0]+255)&-256;
            LPEEK(SV_v_bas_ad)=xbios2;
            LPEEK(SVscreenpt)=xbios2;
            memcpy(r,save_r,16*4);
            on_rte=ON_RTE_RTE;
//            log_write_stack();
            break;
          case ON_RTE_EMHACK:
            on_rte=ON_RTE_RTE;
            extended_monitor_hack();
            break;
#endif
        }
      }
    }
//    log(EasyStr("RTE - decreasing interrupt depth from ")+interrupt_depth+" to "+(interrupt_depth-1));
    interrupt_depth--;
    ioaccess|=IOACCESS_FLAG_FOR_CHECK_INTRS;
//    check_for_interrupts_pending();
    if (!dont_intercept_os) intercept_os();

    CHECK_STOP_ON_USER_CHANGE;
  }else{
    exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
  }
}
void                              m68k_rtd(){
//  INSTRUCTION_TIME_ROUND(20);
//  m68k_GET_IMMEDIATE_W;
  m68k_unrecognised();
}
void                              m68k_rts(){
  INSTRUCTION_TIME_ROUND(16);

  effective_address=m68k_lpeek(r[15]);
  r[15]+=4;
  m68k_READ_W(effective_address); // Check for bus/address errors
  SET_PC(effective_address);
  intercept_os();
}
void                              m68k_trapv(){
  if (sr & SR_V){
    m68k_interrupt(LPEEK(BOMBS_TRAPV*4));
    INSTRUCTION_TIME_ROUND(0); //Round first for interrupts
    INSTRUCTION_TIME_ROUND(34);
  }else{
    INSTRUCTION_TIME(4);
  }
}
void                              m68k_rtr(){
  INSTRUCTION_TIME_ROUND(20);
  CCR=LOBYTE(m68k_dpeek(r[15]));r[15]+=2;
  sr&=SR_VALID_BITMASK;

  effective_address=m68k_lpeek(r[15]);r[15]+=4;
  m68k_READ_W(effective_address); // Check for bus/address errors
  SET_PC(effective_address);
  intercept_os();
}
void                              m68k_movec(){
  m68k_unrecognised();
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/////////////////  LINE 5 - ADDQ, SUBQ, SCC, DBCC     //////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



void                              m68k_addq_b(){
  FETCH_TIMING;
  INSTRUCTION_TIME(4);
  m68k_src_b=(BYTE)PARAM_N;if(m68k_src_b==0)m68k_src_b=8;
  m68k_GET_DEST_B_NOT_A_FASTER_FOR_D;
  m68k_old_dest=m68k_DEST_B;
  m68k_DEST_B+=m68k_src_b;
  SR_ADD_B;
}void                             m68k_addq_w(){
  FETCH_TIMING;
  m68k_src_w=(WORD)PARAM_N;if(m68k_src_w==0)m68k_src_w=8;
  if((ir&BITS_543)==BITS_543_001){ //addq.w to address register
    INSTRUCTION_TIME(4);
    areg[PARAM_M]+=m68k_src_w;
  }else{
    INSTRUCTION_TIME(4);
    m68k_GET_DEST_W_NOT_A_FASTER_FOR_D;
    m68k_old_dest=m68k_DEST_W;
    m68k_DEST_W+=m68k_src_w;
    SR_ADD_W;
  }
}void                             m68k_addq_l(){
  FETCH_TIMING;
  m68k_src_l=(LONG)PARAM_N;if(m68k_src_l==0)m68k_src_l=8;
  if((ir&BITS_543)==BITS_543_001){ //addq.l to address register
    INSTRUCTION_TIME(4);
    areg[PARAM_M]+=m68k_src_l;
  }else{
    INSTRUCTION_TIME(8);
    m68k_GET_DEST_L_NOT_A_FASTER_FOR_D;
    m68k_old_dest=m68k_DEST_L;
    m68k_DEST_L+=m68k_src_l;
    SR_ADD_L;
  }
}
void                              m68k_subq_b(){
  FETCH_TIMING;
  m68k_src_b=(BYTE)PARAM_N;if(m68k_src_b==0)m68k_src_b=8;
  INSTRUCTION_TIME(4);
  m68k_GET_DEST_B_NOT_A_FASTER_FOR_D;
  m68k_old_dest=m68k_DEST_B;
  m68k_DEST_B-=m68k_src_b;
  SR_SUB_B(SR_X);
}void                             m68k_subq_w(){
  FETCH_TIMING;
  m68k_src_w=(WORD)PARAM_N;if(m68k_src_w==0)m68k_src_w=8;
  if((ir&BITS_543)==BITS_543_001){ //subq.w to address register
    INSTRUCTION_TIME(4);
    areg[PARAM_M]-=m68k_src_w;
  }else{
    INSTRUCTION_TIME(4);
    m68k_GET_DEST_W_NOT_A_FASTER_FOR_D;
    m68k_old_dest=m68k_DEST_W;
    m68k_DEST_W-=m68k_src_w;
    SR_SUB_W(SR_X);
  }
}void                             m68k_subq_l(){
  FETCH_TIMING;
  m68k_src_l=(LONG)PARAM_N;if(m68k_src_l==0)m68k_src_l=8;
  if((ir&BITS_543)==BITS_543_001){ //subq.l to address register
    areg[PARAM_M]-=m68k_src_l;
    INSTRUCTION_TIME(4);
  }else{
    INSTRUCTION_TIME(8);
    m68k_GET_DEST_L_NOT_A_FASTER_FOR_D;
    m68k_old_dest=m68k_DEST_L;
    m68k_DEST_L-=m68k_src_l;
    SR_SUB_L(SR_X);
  }
}
void                              m68k_dbCC_or_sCC(){
  if ((ir&BITS_543)==BITS_543_001){
    INSTRUCTION_TIME(6);
    m68k_GET_IMMEDIATE_W;
    if (!m68k_CONDITION_TEST){
      (*((WORD*)(&(r[PARAM_M]))))--;
      if( (*( (signed short*)(&(r[PARAM_M]) ))) != (signed short)(-1) ){
        MEM_ADDRESS new_pc=(pc+(signed short)m68k_src_w-2) | pc_high_byte;
        m68k_READ_W(new_pc); // Check for bus/address errors
        SET_PC(new_pc);
      }else{
        INSTRUCTION_TIME(4);
      }
    }else{
      INSTRUCTION_TIME(2);
    }
    FETCH_TIMING;
  }else{
    FETCH_TIMING;
    m68k_GET_DEST_B;
    if(m68k_CONDITION_TEST){
      m68k_DEST_B=0xff;
      if(DEST_IS_REGISTER){INSTRUCTION_TIME(2);}else {INSTRUCTION_TIME(4);}
    }else{
      m68k_DEST_B=0;
      if(DEST_IS_REGISTER==0){INSTRUCTION_TIME(4);}
    }
  }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////  Line 8 - or, div, sbcd   //////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void                              m68k_or_b_to_dN(){
  FETCH_TIMING;
  m68k_GET_SOURCE_B_NOT_A;
  m68k_dest=&(r[PARAM_N]);
  m68k_DEST_B|=m68k_src_b;
  SR_CLEAR(SR_Z+SR_N+SR_V+SR_C);
  SR_CHECK_Z_AND_N_B;
}void                             m68k_or_w_to_dN(){
  FETCH_TIMING;
  m68k_GET_SOURCE_W_NOT_A;
  m68k_dest=&(r[PARAM_N]);
  m68k_DEST_W|=m68k_src_w;
  SR_CLEAR(SR_Z+SR_N+SR_V+SR_C);
  SR_CHECK_Z_AND_N_W;
}void                             m68k_or_l_to_dN(){
  FETCH_TIMING;
  m68k_GET_SOURCE_L_NOT_A;
  if (SOURCE_IS_REGISTER_OR_IMMEDIATE){INSTRUCTION_TIME(4);}
  else {INSTRUCTION_TIME(2);}
  m68k_dest=&(r[PARAM_N]);
  m68k_DEST_L|=m68k_src_l;
  SR_CLEAR(SR_Z+SR_N+SR_V+SR_C);
  SR_CHECK_Z_AND_N_L;
}
void                              m68k_divu(){
  log_to(LOGSECTION_DIV,Str("DIV: ")+HEXSl(old_pc,6)+" - "+disa_d2(old_pc));
  FETCH_TIMING;
  m68k_GET_SOURCE_W_NOT_A;
  if (m68k_src_w==0){
    m68k_interrupt(LPEEK(BOMBS_DIVISION_BY_ZERO*4));
    INSTRUCTION_TIME_ROUND(0); //Round first for interrupts
    INSTRUCTION_TIME_ROUND(38);
  }else{
    INSTRUCTION_TIME(m68k_divu_cycles);
    unsigned long q=(((unsigned long)(r[PARAM_N]))/(unsigned long)((unsigned short)m68k_src_w));
    if(q&0xffff0000){
      SR_SET(SR_V);
    }else{
      SR_CLEAR(SR_Z+SR_N+SR_C+SR_V);
      if(q&MSB_W)SR_SET(SR_N);
      if(q==0)SR_SET(SR_Z);
      r[PARAM_N]=((((unsigned long)r[PARAM_N])%((unsigned short)m68k_src_w))<<16)+q;
    }
  }

}
void                              m68k_or_b_from_dN_or_sbcd(){
  FETCH_TIMING;
  switch(ir&BITS_543){
  case BITS_543_000:case BITS_543_001:{  //sbcd
    if((ir&BITS_543)==BITS_543_000){
      INSTRUCTION_TIME(2);
      m68k_src_b=LOBYTE(r[PARAM_M]);
      m68k_dest=&(r[PARAM_N]);
    }else{
      INSTRUCTION_TIME_ROUND(14);
      areg[PARAM_M]--;areg[PARAM_N]--;
      if(PARAM_M==7)areg[PARAM_M]--;
      if(PARAM_N==7)areg[PARAM_N]--;
      m68k_src_b=m68k_peek(areg[PARAM_M]);
      m68k_SET_DEST_B(areg[PARAM_N]);
    }
    int n=100+
       ( ((m68k_DEST_B&0xf0)>>4)*10+(m68k_DEST_B&0xf) )
      -( ((m68k_src_b&0xf0)>>4)*10+(m68k_src_b&0xf) );
    if(sr&SR_X)n--;
    SR_CLEAR(SR_X+SR_C+SR_N);
    if(n<100)SR_SET(SR_X+SR_C); //if a carry occurs
    n%=100;
    if(n)SR_CLEAR(SR_Z);
    m68k_DEST_B=(BYTE)( (((n/10)%10)<<4)+(n%10) );
    break;
  }default:
    INSTRUCTION_TIME(4);
    EXTRA_PREFETCH;
    m68k_GET_DEST_B_NOT_A;
    m68k_src_b=LOBYTE(r[PARAM_N]);
    m68k_DEST_B|=m68k_src_b;
    SR_CLEAR(SR_Z+SR_N+SR_V+SR_C);
    SR_CHECK_Z_AND_N_B;
  }
}void                             m68k_or_w_from_dN(){
  FETCH_TIMING;
  switch(ir&BITS_543){
  case BITS_543_000:
  case BITS_543_001:
    m68k_unrecognised();
    break;
  default:
    INSTRUCTION_TIME(4);
    EXTRA_PREFETCH;
    m68k_GET_DEST_W_NOT_A;
    m68k_src_w=LOWORD(r[PARAM_N]);
    m68k_DEST_W|=m68k_src_w;
    SR_CLEAR(SR_Z+SR_N+SR_V+SR_C);
    SR_CHECK_Z_AND_N_W;
  }
}void                             m68k_or_l_from_dN(){
  FETCH_TIMING;
  switch(ir&BITS_543){
  case BITS_543_000:
  case BITS_543_001:
    m68k_unrecognised();
    break;
  default:
    INSTRUCTION_TIME(8);
    EXTRA_PREFETCH;
    m68k_GET_DEST_L_NOT_A;
    m68k_src_l=r[PARAM_N];
    m68k_DEST_L|=m68k_src_l;
    SR_CLEAR(SR_Z+SR_N+SR_V+SR_C);
    SR_CHECK_Z_AND_N_L;
  }
}
void                              m68k_divs(){
  log_to(LOGSECTION_DIV,Str("DIV: ")+HEXSl(old_pc,6)+" - "+disa_d2(old_pc));
  FETCH_TIMING;
  m68k_GET_SOURCE_W_NOT_A;
  if (m68k_src_w==0){
    m68k_interrupt(LPEEK(BOMBS_DIVISION_BY_ZERO*4));
    INSTRUCTION_TIME_ROUND(0); //Round first for interrupts
    INSTRUCTION_TIME_ROUND(38);
  }else{
    INSTRUCTION_TIME(m68k_divs_cycles);
    signed long q=(signed long)((signed long)r[PARAM_N])/(signed long)((signed short)m68k_src_w);
    if(q<-32768 || q>32767){
      SR_SET(SR_V);
    }else{
      SR_CLEAR(SR_Z+SR_N+SR_C+SR_V);
      if(q&MSB_W)SR_SET(SR_N);
      if(q==0)SR_SET(SR_Z);
      r[PARAM_N]=((((signed long)r[PARAM_N])%((signed short)m68k_src_w))<<16)|((long)LOWORD(q));
    }
  }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////  line 9 - sub            ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void                              m68k_sub_b_to_dN(){
  FETCH_TIMING;
  m68k_GET_SOURCE_B_NOT_A;
  m68k_dest=&r[PARAM_N];
  m68k_old_dest=m68k_DEST_B;
  m68k_DEST_B-=m68k_src_b;
  SR_SUB_B(SR_X);
}void                             m68k_sub_w_to_dN(){
  FETCH_TIMING;
  m68k_GET_SOURCE_W;   //A is allowed
  m68k_dest=&r[PARAM_N];
  m68k_old_dest=m68k_DEST_W;
  m68k_DEST_W-=m68k_src_w;
  SR_SUB_W(SR_X);
}void                             m68k_sub_l_to_dN(){

  FETCH_TIMING;
  m68k_GET_SOURCE_L;   //A is allowed
  if(SOURCE_IS_REGISTER_OR_IMMEDIATE){INSTRUCTION_TIME(4);}
  else {INSTRUCTION_TIME(2);}
  m68k_dest=&r[PARAM_N];
  m68k_old_dest=m68k_DEST_L;
  m68k_DEST_L-=m68k_src_l;
  SR_SUB_L(SR_X);
}void                             m68k_suba_w(){
  FETCH_TIMING;
  m68k_GET_SOURCE_W;
  INSTRUCTION_TIME(4);

  m68k_src_l=(signed long)((signed short)m68k_src_w);
  areg[PARAM_N]-=m68k_src_l;
}
void                              m68k_sub_b_from_dN(){
  FETCH_TIMING;
  switch(ir&BITS_543){
  case BITS_543_000:
  case BITS_543_001:
    if((ir&BITS_543)==BITS_543_000){
      m68k_src_b=LOBYTE(r[PARAM_M]);
      m68k_dest=&(r[PARAM_N]);
    }else{
      INSTRUCTION_TIME_ROUND(14);
      areg[PARAM_M]--;      if(PARAM_M==7)areg[PARAM_M]--;
      m68k_src_b=m68k_peek(areg[PARAM_M]);
      areg[PARAM_N]--;      if(PARAM_N==7)areg[PARAM_N]--;
      m68k_SET_DEST_B(areg[PARAM_N]);
    }
    m68k_old_dest=m68k_DEST_B;
    m68k_DEST_B-=m68k_src_b;
    if(sr&SR_X)m68k_DEST_B--;
    SR_SUBX_B;
    break;
  default:
    INSTRUCTION_TIME(4);
    EXTRA_PREFETCH;
    m68k_src_b=LOBYTE(r[PARAM_N]);
    m68k_GET_DEST_B_NOT_A;
    m68k_old_dest=m68k_DEST_B;
    m68k_DEST_B-=m68k_src_b;
    SR_SUB_B(SR_X);
  }
}void                              m68k_sub_w_from_dN(){
  FETCH_TIMING;
  switch(ir&BITS_543){
  case BITS_543_000:
  case BITS_543_001:
    if((ir&BITS_543)==BITS_543_000){
      m68k_src_w=LOWORD(r[PARAM_M]);
      m68k_dest=&(r[PARAM_N]);
    }else{
      INSTRUCTION_TIME_ROUND(14);
      areg[PARAM_M]-=2;m68k_src_w=m68k_dpeek(areg[PARAM_M]);
      areg[PARAM_N]-=2;m68k_SET_DEST_W(areg[PARAM_N]);
    }
    m68k_old_dest=m68k_DEST_W;
    m68k_DEST_W-=m68k_src_w;
    if(sr&SR_X)m68k_DEST_W--;
    SR_SUBX_W;
    break;
  default: //to memory
    INSTRUCTION_TIME(4);
    EXTRA_PREFETCH;
    m68k_src_w=LOWORD(r[PARAM_N]);
    m68k_GET_DEST_W_NOT_A;
    m68k_old_dest=m68k_DEST_W;
    m68k_DEST_W-=m68k_src_w;
    SR_SUB_W(SR_X);
  }
}void                              m68k_sub_l_from_dN(){
  FETCH_TIMING;
  switch(ir&BITS_543){
  case BITS_543_000:
  case BITS_543_001:
    if((ir&BITS_543)==BITS_543_000){
      INSTRUCTION_TIME(4);
      m68k_src_l=r[PARAM_M];
      m68k_dest=&(r[PARAM_N]);
    }else{
      INSTRUCTION_TIME_ROUND(26);
      areg[PARAM_M]-=4;m68k_src_l=m68k_lpeek(areg[PARAM_M]);
      areg[PARAM_N]-=4;m68k_SET_DEST_L(areg[PARAM_N]);
    }
    m68k_old_dest=m68k_DEST_L;
    m68k_DEST_L-=m68k_src_l;
    if(sr&SR_X)m68k_DEST_L--;
    SR_SUBX_L;
    break;
  default:
    INSTRUCTION_TIME(8);
    EXTRA_PREFETCH;
    m68k_src_l=r[PARAM_N];
    m68k_GET_DEST_L_NOT_A;
    m68k_old_dest=m68k_DEST_L;
    m68k_DEST_L-=m68k_src_l;
    SR_SUB_L(SR_X);
  }
}void                             m68k_suba_l(){
  FETCH_TIMING;
  m68k_GET_SOURCE_L;
  if (SOURCE_IS_REGISTER_OR_IMMEDIATE){INSTRUCTION_TIME(4);}
  else {INSTRUCTION_TIME(2);}
  areg[PARAM_N]-=m68k_src_l;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////  line b - cmp, eor       ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void                              m68k_cmp_b(){
  FETCH_TIMING;
  m68k_GET_SOURCE_B;
  m68k_old_dest=LOBYTE(r[PARAM_N]);
  compare_buffer=m68k_old_dest;
  m68k_dest=&compare_buffer;
  m68k_DEST_B-=m68k_src_b;
  SR_SUB_B(0);
}void                             m68k_cmp_w(){
  FETCH_TIMING;
  m68k_GET_SOURCE_W;
  m68k_old_dest=LOWORD(r[PARAM_N]);
  compare_buffer=m68k_old_dest;
  m68k_dest=&compare_buffer;
  m68k_DEST_W-=m68k_src_w;
  SR_SUB_W(0);
}void                             m68k_cmp_l(){
  FETCH_TIMING;
  m68k_GET_SOURCE_L;
  INSTRUCTION_TIME(2);
  m68k_old_dest=r[PARAM_N];
  compare_buffer=m68k_old_dest;
  m68k_dest=&compare_buffer;
  m68k_DEST_L-=m68k_src_l;
  SR_SUB_L(0);
}void                             m68k_cmpa_w(){
  FETCH_TIMING;
  m68k_GET_SOURCE_W;
  INSTRUCTION_TIME(2);
  m68k_src_l=(signed long)((signed short)m68k_src_w);
  m68k_old_dest=areg[PARAM_N];
  compare_buffer=m68k_old_dest;
  m68k_dest=&compare_buffer;
  m68k_DEST_L-=m68k_src_l;
  SR_SUB_L(0);
}
void                              m68k_eor_b(){
  FETCH_TIMING;
  if((ir&BITS_543)==BITS_543_001){  //cmpm
    INSTRUCTION_TIME_ROUND(8);
    m68k_src_b=m68k_peek(areg[PARAM_M]);areg[PARAM_M]++; if(PARAM_M==7)areg[PARAM_M]++;
    m68k_old_dest=m68k_peek(areg[PARAM_N]);areg[PARAM_N]++; if(PARAM_N==7)areg[PARAM_N]++;
    compare_buffer=m68k_old_dest;
    m68k_dest=&compare_buffer;
    m68k_DEST_B-=m68k_src_b;
    SR_SUB_B(0);
  }else{
    INSTRUCTION_TIME(4);
    m68k_GET_DEST_B_NOT_A_FASTER_FOR_D;
    m68k_DEST_B^=LOBYTE(r[PARAM_N]);
    SR_CLEAR(SR_Z+SR_N+SR_V+SR_C);
    SR_CHECK_Z_AND_N_B;
  }
}void                             m68k_eor_w(){
  FETCH_TIMING;
  if((ir&BITS_543)==BITS_543_001){  //cmpm
    INSTRUCTION_TIME_ROUND(8);
    m68k_src_w=m68k_dpeek(areg[PARAM_M]);areg[PARAM_M]+=2;
    m68k_old_dest=m68k_dpeek(areg[PARAM_N]);areg[PARAM_N]+=2;
    compare_buffer=m68k_old_dest;
    m68k_dest=&compare_buffer;
    m68k_DEST_W-=m68k_src_w;
    SR_SUB_W(0);
  }else{
    INSTRUCTION_TIME(4);
    m68k_GET_DEST_W_NOT_A_FASTER_FOR_D;
    m68k_DEST_W^=LOWORD(r[PARAM_N]);
    SR_CLEAR(SR_Z+SR_N+SR_V+SR_C);
    SR_CHECK_Z_AND_N_W;
  }
}void                             m68k_eor_l(){
  FETCH_TIMING;
  if((ir&BITS_543)==BITS_543_001){  //cmpm
    INSTRUCTION_TIME_ROUND(16);
    m68k_src_l=m68k_lpeek(areg[PARAM_M]);areg[PARAM_M]+=4;
    m68k_old_dest=m68k_lpeek(areg[PARAM_N]);areg[PARAM_N]+=4;
    compare_buffer=m68k_old_dest;
    m68k_dest=&compare_buffer;
    m68k_DEST_L-=m68k_src_l;
    SR_SUB_L(0);
  }else{
    INSTRUCTION_TIME(8);
    m68k_GET_DEST_L_NOT_A_FASTER_FOR_D;
    m68k_DEST_L^=r[PARAM_N];
    SR_CLEAR(SR_Z+SR_N+SR_V+SR_C);
    SR_CHECK_Z_AND_N_L;
  }
}
void                             m68k_cmpa_l(){
  FETCH_TIMING;
  m68k_GET_SOURCE_L;
  INSTRUCTION_TIME(2);
  m68k_old_dest=areg[PARAM_N];
  compare_buffer=m68k_old_dest;
  m68k_dest=&compare_buffer;
  m68k_DEST_L-=m68k_src_l;
  SR_SUB_L(0);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////  Line C - and, abcd, exg, mul   ///////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void                              m68k_and_b_to_dN(){
  FETCH_TIMING;
  m68k_GET_SOURCE_B_NOT_A;
  m68k_dest=&(r[PARAM_N]);
  m68k_DEST_B&=m68k_src_b;
  SR_CLEAR(SR_Z+SR_N+SR_V+SR_C);
  SR_CHECK_Z_AND_N_B;
}void                             m68k_and_w_to_dN(){
  FETCH_TIMING;
  m68k_GET_SOURCE_W_NOT_A;
  m68k_dest=&(r[PARAM_N]);
  m68k_DEST_W&=m68k_src_w;
  SR_CLEAR(SR_Z+SR_N+SR_V+SR_C);
  SR_CHECK_Z_AND_N_W;
}void                             m68k_and_l_to_dN(){
  FETCH_TIMING;
  m68k_GET_SOURCE_L_NOT_A;
  if(SOURCE_IS_REGISTER_OR_IMMEDIATE){INSTRUCTION_TIME(4);}
  else {INSTRUCTION_TIME(2);}
  m68k_dest=&(r[PARAM_N]);
  m68k_DEST_L&=m68k_src_l;
  SR_CLEAR(SR_Z+SR_N+SR_V+SR_C);
  SR_CHECK_Z_AND_N_L;
}
void                              m68k_mulu(){
  FETCH_TIMING;

  m68k_GET_SOURCE_W_NOT_A;
  INSTRUCTION_TIME(34);

  ///// Hey, this is right apparently
  for (WORD Val=m68k_src_w;Val;Val>>=1){
    if (Val & 1) INSTRUCTION_TIME(2);
  }

  m68k_dest=&(r[PARAM_N]);
  m68k_DEST_L=(unsigned long)LOWORD(r[PARAM_N])*(unsigned long)((unsigned short)m68k_src_w);
  SR_CLEAR(SR_Z+SR_N+SR_C+SR_V);
  SR_CHECK_Z_AND_N_L;
}
void                              m68k_and_b_from_dN_or_abcd(){
  FETCH_TIMING;
  switch (ir & BITS_543){
  case BITS_543_000:case BITS_543_001:{
    if((ir&BITS_543)==BITS_543_000){
      INSTRUCTION_TIME(2);
      m68k_src_b=LOBYTE(r[PARAM_M]);
      m68k_dest=&(r[PARAM_N]);
    }else{
      INSTRUCTION_TIME_ROUND(14);
      areg[PARAM_M]--;
            if(PARAM_M==7)areg[PARAM_M]--;
      areg[PARAM_N]--;
            if(PARAM_N==7)areg[PARAM_N]--;
      m68k_src_b=m68k_peek(areg[PARAM_M]);
      m68k_SET_DEST_B(areg[PARAM_N]);
    }
    int n=
       ( ((m68k_DEST_B&0xf0)>>4)*10+(m68k_DEST_B&0xf) )
      +( ((m68k_src_b&0xf0)>>4)*10+(m68k_src_b&0xf) );
    if(sr&SR_X)n++;
    SR_CLEAR(SR_X+SR_C+SR_N);
    if(n>=100)SR_SET(SR_X+SR_C); //if a carry occurs
    n%=100;
    if(n)SR_CLEAR(SR_Z);
    m68k_DEST_B=(BYTE)( (((n/10)%10)<<4)+(n%10) );
    break;
  }default:
    INSTRUCTION_TIME(4);
    EXTRA_PREFETCH;
    m68k_GET_DEST_B_NOT_A;
    m68k_src_b=LOBYTE(r[PARAM_N]);
    m68k_DEST_B&=m68k_src_b;
    SR_CLEAR(SR_Z+SR_N+SR_V+SR_C);
    SR_CHECK_Z_AND_N_B;
  }
}void                             m68k_and_w_from_dN_or_exg_like(){
  FETCH_TIMING;
  switch(ir&BITS_543){
  case BITS_543_000:
    INSTRUCTION_TIME(2);
    compare_buffer=r[PARAM_N];
    r[PARAM_N]=r[PARAM_M];
    r[PARAM_M]=compare_buffer;
    break;
  case BITS_543_001:
    INSTRUCTION_TIME(2);
    compare_buffer=areg[PARAM_N];
    areg[PARAM_N]=areg[PARAM_M];
    areg[PARAM_M]=compare_buffer;
    break;
  default:
    INSTRUCTION_TIME(4);
    EXTRA_PREFETCH;
    m68k_GET_DEST_W_NOT_A;
    m68k_src_w=LOWORD(r[PARAM_N]);
    m68k_DEST_W&=m68k_src_w;
    SR_CLEAR(SR_Z+SR_N+SR_V+SR_C);
    SR_CHECK_Z_AND_N_W;
  }
}void                             m68k_and_l_from_dN_or_exg_unlike(){
  FETCH_TIMING;
  switch(ir&BITS_543){
  case BITS_543_000:
    m68k_unrecognised();
    // m68k_command="exg";
    // m68k_src_l=// D2_aN;
    // m68k_dest=// D2_dM;
    break;
  case BITS_543_001:
    INSTRUCTION_TIME(2);
    compare_buffer=areg[PARAM_M];
    areg[PARAM_M]=r[PARAM_N];
    r[PARAM_N]=compare_buffer;
    break;
  default:
    INSTRUCTION_TIME(8);
    EXTRA_PREFETCH;
    m68k_GET_DEST_L_NOT_A;
    m68k_src_l=r[PARAM_N];
    m68k_DEST_L&=m68k_src_l;
    SR_CLEAR(SR_Z+SR_N+SR_V+SR_C);
    SR_CHECK_Z_AND_N_L;
  }
}
void                              m68k_muls(){
  FETCH_TIMING;

  m68k_GET_SOURCE_W_NOT_A;

  INSTRUCTION_TIME(34);
  ///// Hey, this is right apparently
  int LastLow=0;
  int Val=WORD(m68k_src_w);
  for (int n=0;n<16;n++){
    if ((Val & 1)!=LastLow) INSTRUCTION_TIME(2);
    LastLow=(Val & 1);
    Val>>=1;
  }
  m68k_dest=&(r[PARAM_N]);
  m68k_DEST_L=((signed long)((signed short)LOWORD(r[PARAM_N])))*((signed long)((signed short)m68k_src_w));
  SR_CLEAR(SR_Z+SR_N+SR_C+SR_V);
  SR_CHECK_Z_AND_N_L;
}




////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////  line D - add            ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void                              m68k_add_b_to_dN(){
  FETCH_TIMING;
  m68k_GET_SOURCE_B_NOT_A;
  m68k_dest=&r[PARAM_N];
  m68k_old_dest=m68k_DEST_B;
  m68k_DEST_B+=m68k_src_b;
  SR_ADD_B;
}void                             m68k_add_w_to_dN(){
  FETCH_TIMING;
  m68k_GET_SOURCE_W;   //A is allowed
  m68k_dest=&r[PARAM_N];
  m68k_old_dest=m68k_DEST_W;
  m68k_DEST_W+=m68k_src_w;
  SR_ADD_W;
}void                             m68k_add_l_to_dN(){
  FETCH_TIMING;
  m68k_GET_SOURCE_L;   //A is allowed
  if(SOURCE_IS_REGISTER_OR_IMMEDIATE){INSTRUCTION_TIME(4);}
  else {INSTRUCTION_TIME(2);}
  m68k_dest=&r[PARAM_N];
  m68k_old_dest=m68k_DEST_L;
  m68k_DEST_L+=m68k_src_l;
  SR_ADD_L;
}void                             m68k_adda_w(){
  FETCH_TIMING;
  m68k_GET_SOURCE_W;
  INSTRUCTION_TIME(4);
  m68k_src_l=(signed long)((signed short)m68k_src_w);
  areg[PARAM_N]+=m68k_src_l;
}
void                              m68k_add_b_from_dN(){
  FETCH_TIMING;
  switch(ir&BITS_543){
  case BITS_543_000:
  case BITS_543_001:
    if((ir&BITS_543)==BITS_543_000){
      m68k_src_b=LOBYTE(r[PARAM_M]);
      m68k_dest=&(r[PARAM_N]);
    }else{
      INSTRUCTION_TIME_ROUND(14);
      areg[PARAM_M]--;
            if(PARAM_M==7)areg[PARAM_M]--;
      m68k_src_b=m68k_peek(areg[PARAM_M]);
      areg[PARAM_N]--;
            if(PARAM_N==7)areg[PARAM_N]--;
      m68k_SET_DEST_B(areg[PARAM_N]);
    }
    m68k_old_dest=m68k_DEST_B;
    m68k_DEST_B+=m68k_src_b;
    if(sr&SR_X)m68k_DEST_B++;
    SR_ADDX_B;
    break;
  default:
    INSTRUCTION_TIME(4);
    EXTRA_PREFETCH;
    m68k_src_b=LOBYTE(r[PARAM_N]);
    m68k_GET_DEST_B_NOT_A;
    m68k_old_dest=m68k_DEST_B;
    m68k_DEST_B+=m68k_src_b;
    SR_ADD_B;
  }
}void                              m68k_add_w_from_dN(){
  FETCH_TIMING;
  switch(ir&BITS_543){
  case BITS_543_000:
  case BITS_543_001:
    if((ir&BITS_543)==BITS_543_000){
      m68k_src_w=LOWORD(r[PARAM_M]);
      m68k_dest=&(r[PARAM_N]);
    }else{
      INSTRUCTION_TIME_ROUND(14);
      areg[PARAM_M]-=2;m68k_src_w=m68k_dpeek(areg[PARAM_M]);
      areg[PARAM_N]-=2;m68k_SET_DEST_W(areg[PARAM_N]);
    }
    m68k_old_dest=m68k_DEST_W;
    m68k_DEST_W+=m68k_src_w;
    if(sr&SR_X)m68k_DEST_W++;
    SR_ADDX_W;
    break;
  default:
    INSTRUCTION_TIME(4);
    EXTRA_PREFETCH
    m68k_src_w=LOWORD(r[PARAM_N]);
    m68k_GET_DEST_W_NOT_A;
    m68k_old_dest=m68k_DEST_W;
    m68k_DEST_W+=m68k_src_w;
    SR_ADD_W;
  }
}void                              m68k_add_l_from_dN(){
  FETCH_TIMING;
  switch (ir&BITS_543){
  case BITS_543_000:
  case BITS_543_001:
    if((ir&BITS_543)==BITS_543_000){
      INSTRUCTION_TIME(4);
      m68k_src_l=r[PARAM_M];
      m68k_dest=&(r[PARAM_N]);
    }else{
      INSTRUCTION_TIME_ROUND(26);
      areg[PARAM_M]-=4;m68k_src_l=m68k_lpeek(areg[PARAM_M]);
      areg[PARAM_N]-=4;m68k_SET_DEST_L(areg[PARAM_N]);
    }
    m68k_old_dest=m68k_DEST_L;
    m68k_DEST_L+=m68k_src_l;
    if(sr&SR_X)m68k_DEST_L++;
    SR_ADDX_L;
    break;
  default:
    INSTRUCTION_TIME(8);
    EXTRA_PREFETCH;
    m68k_src_l=r[PARAM_N];
    m68k_GET_DEST_L_NOT_A;
    m68k_old_dest=m68k_DEST_L;
    m68k_DEST_L+=m68k_src_l;
    SR_ADD_L;
  }
}void                             m68k_adda_l(){
  FETCH_TIMING;
  m68k_GET_SOURCE_L;
  if (SOURCE_IS_REGISTER_OR_IMMEDIATE){
    INSTRUCTION_TIME(4);
  }else{
    INSTRUCTION_TIME(2);
  }
  areg[PARAM_N]+=m68k_src_l;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////  line e - bit shift      ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


void                              m68k_asr_b_to_dM(){
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(2+2*m68k_src_w);
  if(m68k_src_w>31)m68k_src_w=31;
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
  if (m68k_src_w){
    if( m68k_DEST_B & (BYTE)( 1 << min(m68k_src_w-1,7) )  ){
      SR_SET(SR_C+SR_X);
    }else{
      SR_CLEAR(SR_C+SR_X);
    }
  }
  *((signed char*)m68k_dest)>>=m68k_src_w;
  SR_CHECK_Z_AND_N_B;
}void                             m68k_lsr_b_to_dM(){
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(2+2*m68k_src_w);
  if(m68k_src_w>31)m68k_src_w=31;
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
  if (m68k_src_w){
    if(m68k_src_w>8){
      SR_CLEAR(SR_C+SR_X);
    }else{
      if( m68k_DEST_B&(BYTE)( 1<<(m68k_src_w-1) )  ){
        SR_SET(SR_C+SR_X);
      }else{
        SR_CLEAR(SR_C+SR_X);
      }
    }
  }
  *((unsigned char*)m68k_dest)>>=m68k_src_w;
  SR_CHECK_Z_AND_N_B;
}void                             m68k_roxr_b_to_dM(){ //okay
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(2+2*m68k_src_w);
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);if(sr&SR_X)SR_SET(SR_C);
  for(int n=0;n<m68k_src_w;n++){
    bool old_x=(sr&SR_X);
    if(m68k_DEST_B&1){
      SR_SET(SR_X+SR_C)
    }else{
      SR_CLEAR(SR_X+SR_C)
    }
    *((unsigned char*)m68k_dest)>>=1;if(old_x)m68k_DEST_B|=MSB_B;
  }
  SR_CHECK_Z_AND_N_B;
}void                             m68k_ror_b_to_dM(){  //okay!
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(2+2*m68k_src_w);
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
  for(int n=0;n<m68k_src_w;n++){
    bool old_x=m68k_DEST_B&1;
    if(old_x){
      SR_SET(SR_C)
    }else{
      SR_CLEAR(SR_C)
    }
    *((unsigned char*)m68k_dest)>>=1;if(old_x)m68k_DEST_B|=MSB_B;
  }
  SR_CHECK_Z_AND_N_B;
}
void                              m68k_asr_w_to_dM(){
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(2+2*m68k_src_w);
  if(m68k_src_w>31)m68k_src_w=31;
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
  if(m68k_src_w){
    if( m68k_DEST_W&(WORD)( 1<<min(m68k_src_w-1,15) )  ){
      SR_SET(SR_C+SR_X);
    }else{
      SR_CLEAR(SR_C+SR_X);
    }
    *((signed short*)m68k_dest)>>=m68k_src_w;
  }
  SR_CHECK_Z_AND_N_W;
}void                             m68k_lsr_w_to_dM(){  //okay!
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(2+2*m68k_src_w);
  if(m68k_src_w>31)m68k_src_w=31;
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
  if(m68k_src_w){
    if(m68k_src_w>16){
      SR_CLEAR(SR_C+SR_X);
    }else{
      if( m68k_DEST_W & (WORD)( 1<<(m68k_src_w-1) )  ){
        SR_SET(SR_C+SR_X);
      }else{
        SR_CLEAR(SR_C+SR_X);
      }
    }
  }
  *((unsigned short*)m68k_dest)>>=m68k_src_w;
  SR_CHECK_Z_AND_N_W;
}void                             m68k_roxr_w_to_dM(){          //okay
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(2+2*m68k_src_w);
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);if(sr&SR_X)SR_SET(SR_C);
  for(int n=0;n<m68k_src_w;n++){
    bool old_x=(sr&SR_X);
    if(m68k_DEST_W&1){
      SR_SET(SR_X+SR_C)
    }else{
      SR_CLEAR(SR_X+SR_C)
    }
    *((unsigned short*)m68k_dest)>>=1;if(old_x)m68k_DEST_W|=MSB_W;
  }
  SR_CHECK_Z_AND_N_W;
}void                             m68k_ror_w_to_dM(){
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(2+2*m68k_src_w);
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
  for(int n=0;n<m68k_src_w;n++){
    bool old_x=m68k_DEST_W&1;
    if(old_x){
      SR_SET(SR_C)
    }else{
      SR_CLEAR(SR_C)
    }
    *((unsigned short*)m68k_dest)>>=1;if(old_x)m68k_DEST_W|=MSB_W;
  }
  SR_CHECK_Z_AND_N_W;
}
void                              m68k_asr_l_to_dM(){
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(4+2*m68k_src_w);
  m68k_dest=&(r[PARAM_M]);

  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
  if (m68k_src_w){
    // If shift by 31 or more then test MSB as this is copied to the whole long
    if ( m68k_DEST_L & (1 << min(m68k_src_w-1,31)) ){
      SR_SET(SR_C+SR_X);
    }else{
      SR_CLEAR(SR_C+SR_X);
    }
    // Because MSB->LSB, MSB has been copied to all other bits so 1 extra shift
    // will make no difference.
    if (m68k_src_w>31) m68k_src_w=31;
    *((signed long*)m68k_dest)>>=m68k_src_w;
  }
  SR_CHECK_Z_AND_N_L;
}
void                             m68k_lsr_l_to_dM(){
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(4+2*m68k_src_w);
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
  if(m68k_src_w){
    if(m68k_src_w>32){
      SR_CLEAR(SR_C+SR_X);
    }else{
      if( m68k_DEST_L&(DWORD)( 1<<(m68k_src_w-1) )  ){
        SR_SET(SR_C+SR_X);
      }else{
        SR_CLEAR(SR_C+SR_X);
      }
    }
  }
  *((unsigned long*)m68k_dest)>>=m68k_src_w;
  if(m68k_src_w>31)m68k_DEST_L=0;
  SR_CHECK_Z_AND_N_L;
}void                             m68k_roxr_l_to_dM(){   //okay!
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(4+2*m68k_src_w);
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);if(sr&SR_X)SR_SET(SR_C);
  for(int n=0;n<m68k_src_w;n++){
    bool old_x=(sr&SR_X);
    if(m68k_DEST_L&1){
      SR_SET(SR_X+SR_C)
    }else{
      SR_CLEAR(SR_X+SR_C)
    }
    *((unsigned long*)m68k_dest)>>=1;if(old_x)m68k_DEST_L|=MSB_L;
  }
  SR_CHECK_Z_AND_N_L;
}void                             m68k_ror_l_to_dM(){   //okay!
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(4+2*m68k_src_w);
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
  for(int n=0;n<m68k_src_w;n++){
    bool old_x=m68k_DEST_L&1;
    if(old_x){
      SR_SET(SR_C)
    }else{
      SR_CLEAR(SR_C)
    }
    *((unsigned long*)m68k_dest)>>=1;if(old_x)m68k_DEST_L|=MSB_L;
  }
  SR_CHECK_Z_AND_N_L;
}
void                              m68k_asl_b_to_dM(){
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(2+2*m68k_src_w);
  if (m68k_src_w>31) m68k_src_w=31;
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
  if (m68k_src_w){
    SR_CLEAR(SR_C+SR_X);
    if (m68k_src_w<=8){
      if ( m68k_DEST_B & (BYTE)( MSB_B >> (m68k_src_w-1) )  ){
        SR_SET(SR_C+SR_X);
      }
    }
    if(m68k_src_w<=7){
      signed char mask=(signed char)(((signed char)(MSB_B))>>(m68k_src_w));
      // mask:  m  m-1 m-2 m-3 ... m-r m-r-1 ...
      //        1   1   1   1       1   0    0...

      if((mask&(m68k_DEST_B))!=0 && ((mask&(m68k_DEST_B))^mask)!=0){
        SR_SET(SR_V);
      }
    }else if(m68k_DEST_B){
      SR_SET(SR_V);
    }
  }
  *((signed char*)m68k_dest)<<=m68k_src_w;
  SR_CHECK_Z_AND_N_B;
}void                             m68k_lsl_b_to_dM(){
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(2+2*m68k_src_w);
  if(m68k_src_w>31)m68k_src_w=31;
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
  if(m68k_src_w){
    SR_CLEAR(SR_X);
    if(m68k_src_w<=8){
      if( m68k_DEST_B&(BYTE)( MSB_B>>(m68k_src_w-1) )  ){
        SR_SET(SR_C+SR_X);
      }
    }
  }
  *((unsigned char*)m68k_dest)<<=m68k_src_w;
  SR_CHECK_Z_AND_N_B;
}void                             m68k_roxl_b_to_dM(){
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;

  INSTRUCTION_TIME(2+2*m68k_src_w);
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);if(sr&SR_X)SR_SET(SR_C);
  for(int n=0;n<m68k_src_w;n++){
    bool old_x=(sr&SR_X);
    if(m68k_DEST_B&MSB_B){
      SR_SET(SR_X+SR_C)
    }else{
      SR_CLEAR(SR_X+SR_C)
    }
    *((unsigned char*)m68k_dest)<<=1;if(old_x)m68k_DEST_B|=1;
  }
  SR_CHECK_Z_AND_N_B;
}void                             m68k_rol_b_to_dM(){
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(2+2*m68k_src_w);
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
  for(int n=0;n<m68k_src_w;n++){
    bool old_x=m68k_DEST_B&MSB_B;
    if(old_x){
      SR_SET(SR_C)
    }else{
      SR_CLEAR(SR_C)
    }
    *((unsigned char*)m68k_dest)<<=1;if(old_x)m68k_DEST_B|=1;
  }
  SR_CHECK_Z_AND_N_B;
}
void                              m68k_asl_w_to_dM(){       //okay!
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(2+2*m68k_src_w);
  if(m68k_src_w>31)m68k_src_w=31;
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
  if(m68k_src_w){
    SR_CLEAR(SR_C+SR_X);
    if(m68k_src_w<=16){
      if( m68k_DEST_W&(WORD)( MSB_W>>(m68k_src_w-1) )  ){
        SR_SET(SR_C+SR_X);
      }
    }
    if(m68k_src_w<=15){
      signed short mask=(signed short)(((signed short)(MSB_W))>>(m68k_src_w));
      if((mask&(m68k_DEST_W))!=0 && ((mask&(m68k_DEST_W))^mask)!=0){
        SR_SET(SR_V);
      }
    }else if(m68k_DEST_W){
      SR_SET(SR_V);
    }
  }
  *((signed short*)m68k_dest)<<=m68k_src_w;
  SR_CHECK_Z_AND_N_W;
}void                             m68k_lsl_w_to_dM(){
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(2+2*m68k_src_w);
  if(m68k_src_w>31)m68k_src_w=31;
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
  if(m68k_src_w){
    SR_CLEAR(SR_X);
    if(m68k_src_w<=16){
      if( m68k_DEST_W&(WORD)( MSB_W>>(m68k_src_w-1) )  ){
        SR_SET(SR_C+SR_X);
      }
    }
  }
  *((unsigned short*)m68k_dest)<<=m68k_src_w;

  SR_CHECK_Z_AND_N_W;
}void                             m68k_roxl_w_to_dM(){
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(2+2*m68k_src_w);
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);if(sr&SR_X)SR_SET(SR_C);
  for(int n=0;n<m68k_src_w;n++){
    bool old_x=(sr&SR_X);
    if(m68k_DEST_W&MSB_W){
      SR_SET(SR_X+SR_C)
    }else{
      SR_CLEAR(SR_X+SR_C)
    }
    *((unsigned short*)m68k_dest)<<=1;if(old_x)m68k_DEST_W|=1;
  }
  SR_CHECK_Z_AND_N_W;
}void                             m68k_rol_w_to_dM(){
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(2+2*m68k_src_w);
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
  for(int n=0;n<m68k_src_w;n++){
    bool old_x=m68k_DEST_W&MSB_W;
    if(old_x){
      SR_SET(SR_C)
    }else{
      SR_CLEAR(SR_C)
    }
    *((unsigned short*)m68k_dest)<<=1;if(old_x)m68k_DEST_W|=1;
  }
  SR_CHECK_Z_AND_N_W;
}
void                              m68k_asl_l_to_dM(){    //okay!
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(4+2*m68k_src_w);
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
  if(m68k_src_w){
    SR_CLEAR(SR_C+SR_X);
    if(m68k_src_w<=32){
      if( m68k_DEST_L&(LONG)( MSB_L>>(m68k_src_w-1) )  ){
        SR_SET(SR_C+SR_X);
      }
    }
    if(m68k_src_w<=31){
      signed long mask=(((signed long)(MSB_L))>>(m68k_src_w));
      if((mask&(m68k_DEST_L))!=0 && ((mask&(m68k_DEST_L))^mask)!=0){
        SR_SET(SR_V);
      }
    }else if(m68k_DEST_L){
      SR_SET(SR_V);
    }
  }
  *((signed long*)m68k_dest)<<=m68k_src_w;
  if(m68k_src_w>31)m68k_DEST_L=0;
  SR_CHECK_Z_AND_N_L;
}void                             m68k_lsl_l_to_dM(){
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(4+2*m68k_src_w);
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
  if(m68k_src_w){
    SR_CLEAR(SR_X);
    if(m68k_src_w<=32){
      if( m68k_DEST_L&(LONG)( MSB_L>>(m68k_src_w-1) )  ){
        SR_SET(SR_C+SR_X);
      }
    }
  }
  *((unsigned long*)m68k_dest)<<=m68k_src_w;
  if(m68k_src_w>31)m68k_DEST_L=0;
  SR_CHECK_Z_AND_N_L;
}void                             m68k_roxl_l_to_dM(){
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(4+2*m68k_src_w);
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);if(sr&SR_X)SR_SET(SR_C);
  for(int n=0;n<m68k_src_w;n++){
    bool old_x=(sr&SR_X);
    if(m68k_DEST_L&MSB_L){
      SR_SET(SR_X+SR_C)
    }else{
      SR_CLEAR(SR_X+SR_C)
    }
    *((unsigned long*)m68k_dest)<<=1;if(old_x)m68k_DEST_L|=1;
  }
  SR_CHECK_Z_AND_N_L;
}void                             m68k_rol_l_to_dM(){
  FETCH_TIMING;
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE;
  INSTRUCTION_TIME(4+2*m68k_src_w);
  m68k_dest=&(r[PARAM_M]);
  SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);
  for(int n=0;n<m68k_src_w;n++){
    bool old_x=m68k_DEST_L&MSB_L;
    if(old_x){
      SR_SET(SR_C)
    }else{
      SR_CLEAR(SR_C)
    }
    *((unsigned long*)m68k_dest)<<=1;if(old_x)m68k_DEST_L|=1;
  }
  SR_CHECK_Z_AND_N_L;
}
void                              m68k_bit_shift_right_to_mem(){
  FETCH_TIMING;
  INSTRUCTION_TIME(4);
  m68k_GET_DEST_W_NOT_A_OR_D;
  switch(ir&BITS_ba9){
  case BITS_ba9_000:
    SR_CLEAR(SR_N+SR_V+SR_Z+SR_C+SR_X);
    if(m68k_DEST_W&1){
      SR_SET(SR_C+SR_X);
    }
    *((signed short*)m68k_dest)>>=1;
    SR_CHECK_Z_AND_N_W;
    break;
  case BITS_ba9_001:
    SR_CLEAR(SR_N+SR_V+SR_Z+SR_C+SR_X);
    if(m68k_DEST_W&1){
      SR_SET(SR_C+SR_X);

    }
    *((unsigned short*)m68k_dest)>>=1;
    SR_CHECK_Z_AND_N_W;
    break;
  case BITS_ba9_010:{
    SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);if(sr&SR_X)SR_SET(SR_C);
    bool old_x=(sr&SR_X);
    if(m68k_DEST_W&1){
      SR_SET(SR_X+SR_C)
    }else{
      SR_CLEAR(SR_X+SR_C)
    }
    *((unsigned short*)m68k_dest)>>=1;if(old_x)m68k_DEST_W|=MSB_W;
    SR_CHECK_Z_AND_N_W;
    break;
  }case BITS_ba9_011:{
    SR_CLEAR(SR_N+SR_V+SR_Z);
    bool old_x=m68k_DEST_W&1;
    if(old_x){
      SR_SET(SR_C)
    }else{
      SR_CLEAR(SR_C)
    }
    *((unsigned short*)m68k_dest)>>=1;if(old_x)m68k_DEST_W|=MSB_W;
    SR_CHECK_Z_AND_N_W;
    break;

  }default:
    m68k_unrecognised();
    break;
  }
}
void                              m68k_bit_shift_left_to_mem(){
  FETCH_TIMING;
  INSTRUCTION_TIME(4);
  m68k_GET_DEST_W_NOT_A_OR_D;
  switch(ir&BITS_ba9){
  case BITS_ba9_000: //asl
    SR_CLEAR(SR_N+SR_V+SR_Z+SR_C+SR_X);
//    if( m68k_DEST_W&(WORD)( MSB_W>>(m68k_src_w-1) )  ){
    if( m68k_DEST_W&(WORD)( MSB_W )  ){
      SR_SET(SR_C+SR_X);
    }
    if((m68k_DEST_W&0xc000)==0x8000 || (m68k_DEST_W&0xc000)==0x4000){
      SR_SET(SR_V);
    }
    *((signed short*)m68k_dest)<<=1;
    SR_CHECK_Z_AND_N_W;
    break;
  case BITS_ba9_001:
    SR_CLEAR(SR_N+SR_V+SR_Z+SR_C+SR_X);
    if(m68k_DEST_W&MSB_W){
      SR_SET(SR_C+SR_X);
    }
    *((unsigned short*)m68k_dest)<<=1;
    SR_CHECK_Z_AND_N_W;
    break;
  case BITS_ba9_010:{
    SR_CLEAR(SR_N+SR_V+SR_Z+SR_C);if(sr&SR_X)SR_SET(SR_C);
    bool old_x=(sr&SR_X);
    if(m68k_DEST_W&MSB_W){
      SR_SET(SR_X+SR_C)
    }else{
      SR_CLEAR(SR_X+SR_C)
    }
    *((unsigned short*)m68k_dest)<<=1;if(old_x)m68k_DEST_W|=1;
    SR_CHECK_Z_AND_N_W;
    break;
  }case BITS_ba9_011:{
    SR_CLEAR(SR_N+SR_V+SR_Z);
    bool old_x=m68k_DEST_W&MSB_W;
    if(old_x){
      SR_SET(SR_C)
    }else{
      SR_CLEAR(SR_C)
    }
    *((unsigned short*)m68k_dest)<<=1;if(old_x)m68k_DEST_W|=1;
    SR_CHECK_Z_AND_N_W;
    break;
  }default:
    m68k_unrecognised();
    break;
  }
}






////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////  HIGH NIBBLE ROUTINES    ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


extern "C" void m68k_0000(){ //immediate stuff
  m68k_jump_line_0[(ir&(BITS_876|BITS_ba9))>>6]();
}

void m68k_0001(){  //move.b
  INSTRUCTION_TIME(4); // I don't think this should be here, does move read on cycle 0?
  m68k_GET_SOURCE_B;
  if((ir&BITS_876)==BITS_876_000){
    SR_CLEAR(SR_V+SR_C+SR_N+SR_Z);
    m68k_dest=&(r[PARAM_N]);
    m68k_DEST_B=m68k_src_b;
    SR_CHECK_Z_AND_N_B;
  }else if((ir&BITS_876)==BITS_876_001){
    m68k_unrecognised();
  }else{   //to memory
    bool refetch=0;
    switch(ir&BITS_876){
    case BITS_876_010:
//      INSTRUCTION_TIME(8-4-4);
      abus=areg[PARAM_N];
      break;
    case BITS_876_011:
//      INSTRUCTION_TIME(8-4-4);
      abus=areg[PARAM_N];
      areg[PARAM_N]++;
      if(PARAM_N==7)areg[7]++;
      break;
    case BITS_876_100:
//      INSTRUCTION_TIME(8-4-4);
      areg[PARAM_N]--;
      if(PARAM_N==7)areg[7]--;
      abus=areg[PARAM_N];
      break;
    case BITS_876_101:
      INSTRUCTION_TIME(12-4-4);
      abus=areg[PARAM_N]+(signed short)m68k_fetchW();
      pc+=2;
      break;
    case BITS_876_110:
      INSTRUCTION_TIME(14-4-4);
      m68k_iriwo=m68k_fetchW();pc+=2;
      if(m68k_iriwo&BIT_b){  //.l
        abus=areg[PARAM_N]+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12];
      }else{         //.w
        abus=areg[PARAM_N]+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12];
      }
      break;
    case BITS_876_111:
      if (SOURCE_IS_REGISTER_OR_IMMEDIATE==0) refetch=true;
      switch (ir & BITS_ba9){
        case BITS_ba9_000:
          INSTRUCTION_TIME(12-4-4);
          abus=0xffffff & (unsigned long)((signed long)((signed short)m68k_fetchW()));
          pc+=2;
          break;
        case BITS_ba9_001:
          INSTRUCTION_TIME(16-4-4);
          abus=m68k_fetchL() & 0xffffff;
          pc+=4;
          break;
        default:
          m68k_unrecognised();
      }
    }
    SR_CLEAR(SR_Z+SR_N+SR_C+SR_V);
    if(!m68k_src_b){
      SR_SET(SR_Z);
    }
    if(m68k_src_b&MSB_B){
      SR_SET(SR_N);
    }

    m68k_poke_abus(m68k_src_b);
    FETCH_TIMING;  // move fetches after instruction
    if (refetch) prefetch_buf[0]=*(lpfetch-MEM_DIR);
  }
}

void m68k_0010()  //move.l
{
  INSTRUCTION_TIME(4);
  m68k_GET_SOURCE_L;
  if ((ir & BITS_876)==BITS_876_000){
    SR_CLEAR(SR_V+SR_C+SR_N+SR_Z);
    m68k_dest=&(r[PARAM_N]);
    m68k_DEST_L=m68k_src_l;
    SR_CHECK_Z_AND_N_L;
  }else if ((ir & BITS_876)==BITS_876_001){
//    INSTRUCTION_TIME(4);
    areg[PARAM_N]=m68k_src_l;
  }else{   //to memory
    bool refetch=0;
    switch(ir&BITS_876){
    case BITS_876_010:
      INSTRUCTION_TIME(12-4-4);
      abus=areg[PARAM_N];
      break;
    case BITS_876_011:
      INSTRUCTION_TIME(12-4-4);
      abus=areg[PARAM_N];
      areg[PARAM_N]+=4;
      break;
    case BITS_876_100:
      INSTRUCTION_TIME(12-4-4);
      areg[PARAM_N]-=4;
      abus=areg[PARAM_N];
      break;
    case BITS_876_101:
      INSTRUCTION_TIME(16-4-4);
      abus=areg[PARAM_N]+(signed short)m68k_fetchW();
      pc+=2;
      break;
    case BITS_876_110:
      INSTRUCTION_TIME(18-4-4);
      m68k_iriwo=m68k_fetchW();pc+=2;
      if(m68k_iriwo&BIT_b){  //.l
        abus=areg[PARAM_N]+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12];
      }else{         //.w
        abus=areg[PARAM_N]+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12];
      }
      break;
    case BITS_876_111:
      if (SOURCE_IS_REGISTER_OR_IMMEDIATE==0) refetch=true;
      switch(ir&BITS_ba9){
      case BITS_ba9_000:
        INSTRUCTION_TIME(16-4-4);
        abus=0xffffff&(unsigned long)((signed long)((signed short)m68k_fetchW()));
        pc+=2;
        break;
      case BITS_ba9_001:
        INSTRUCTION_TIME(20-4-4);
        abus=m68k_fetchL()&0xffffff;
        pc+=4;
        break;
      default:
        m68k_unrecognised();
      }
    }
    SR_CLEAR(SR_Z+SR_N+SR_C+SR_V);
    if(!m68k_src_l){
      SR_SET(SR_Z);
    }
    if(m68k_src_l&MSB_L){
      SR_SET(SR_N);
    }

    m68k_lpoke_abus(m68k_src_l);
    FETCH_TIMING; // move fetches after instruction
    if (refetch) prefetch_buf[0]=*(lpfetch-MEM_DIR);
  }
}

void m68k_0011() //move.w
{
  INSTRUCTION_TIME(4);
  m68k_GET_SOURCE_W;
  if ((ir & BITS_876)==BITS_876_000){
//    INSTRUCTION_TIME(4);
    SR_CLEAR(SR_V+SR_C+SR_N+SR_Z);
    m68k_dest=&(r[PARAM_N]);
    m68k_DEST_W=m68k_src_w;
    SR_CHECK_Z_AND_N_W;
  }else if ((ir & BITS_876)==BITS_876_001){
//    INSTRUCTION_TIME(4);
    areg[PARAM_N]=(signed long)((signed short)m68k_src_w);
  }else{   //to memory
    bool refetch=0;
    switch (ir & BITS_876){
    case BITS_876_010:
//      INSTRUCTION_TIME(8-4);
      abus=areg[PARAM_N];
      break;
    case BITS_876_011:
//      INSTRUCTION_TIME(8-4);
      abus=areg[PARAM_N];
      areg[PARAM_N]+=2;
      break;
    case BITS_876_100:
//      INSTRUCTION_TIME(8-4);
      areg[PARAM_N]-=2;
      abus=areg[PARAM_N];
      break;
    case BITS_876_101:
      INSTRUCTION_TIME(12-4-4);
      abus=areg[PARAM_N]+(signed short)m68k_fetchW();
      pc+=2;
      break;
    case BITS_876_110:
      INSTRUCTION_TIME(14-4-4);
      m68k_iriwo=m68k_fetchW();pc+=2;
      if(m68k_iriwo&BIT_b){  //.l
        abus=areg[PARAM_N]+(signed char)LOBYTE(m68k_iriwo)+(int)r[m68k_iriwo>>12];
      }else{         //.w
        abus=areg[PARAM_N]+(signed char)LOBYTE(m68k_iriwo)+(signed short)r[m68k_iriwo>>12];
      }
      break;
    case BITS_876_111:
      if (SOURCE_IS_REGISTER_OR_IMMEDIATE==0) refetch=true;
      switch (ir & BITS_ba9){
      case BITS_ba9_000:
        INSTRUCTION_TIME(12-4-4);
        abus=0xffffff&(unsigned long)((signed long)((signed short)m68k_fetchW()));
        pc+=2;
        break;
      case BITS_ba9_001:
        INSTRUCTION_TIME(16-4-4);
        abus=m68k_fetchL()&0xffffff;
        pc+=4;
        break;
      default:
        m68k_unrecognised();
      }
    }
    SR_CLEAR(SR_Z+SR_N+SR_C+SR_V);
    if (!m68k_src_w){
      SR_SET(SR_Z);
    }
    if (m68k_src_w & MSB_W){
      SR_SET(SR_N);
    }

    m68k_dpoke_abus(m68k_src_w);
    FETCH_TIMING; // move fetches after instruction
    if (refetch) prefetch_buf[0]=*(lpfetch-MEM_DIR);
  }
}

extern "C" void m68k_0100(){
  m68k_jump_line_4[(ir&(BITS_ba9|BITS_876))>>6]();
}

extern "C" void m68k_0101(){
  m68k_jump_line_5[(ir&BITS_876)>>6]();
}

extern "C" void m68k_0110(){  //bCC
  if (LOBYTE(ir)){
    MEM_ADDRESS new_pc=(pc+(signed long)((signed char)LOBYTE(ir))) | pc_high_byte;
    if ((ir & 0xf00)==0x100){ //bsr
      m68k_PUSH_L(PC32);
      m68k_READ_W(new_pc); // Check for bus/address errors
      SET_PC(new_pc);
      INSTRUCTION_TIME_ROUND(18); // round for fetch
    }else{
      if (m68k_CONDITION_TEST){
        m68k_READ_W(new_pc); // Check for bus/address errors
        SET_PC(new_pc);
        INSTRUCTION_TIME_ROUND(10); // round for fetch
      }else{
        INSTRUCTION_TIME_ROUND(8);
      }
    }
  }else{
    if ((ir & 0xf00)==0x100){ //bsr.l
      m68k_PUSH_L(PC32+2);

      MEM_ADDRESS new_pc=(pc+(signed long)((signed short)m68k_fetchW())) | pc_high_byte;
      // stacked pc is always instruction pc+2 due to prefetch (pc doesn't increase before new_pc is read)
      m68k_READ_W(new_pc); // Check for bus/address errors
      SET_PC(new_pc);
      INSTRUCTION_TIME_ROUND(18); // round for fetch
    }else{ // Bcc.l
      MEM_ADDRESS new_pc=(pc+(signed long)((signed short)m68k_fetchW())) | pc_high_byte;
      if (m68k_CONDITION_TEST){
        // stacked pc is always instruction pc+2 due to prefetch (pc doesn't increase before new_pc is read)
        m68k_READ_W(new_pc); // Check for bus/address errors
        SET_PC(new_pc);
        INSTRUCTION_TIME_ROUND(10);
      }else{
        pc+=2;
        INSTRUCTION_TIME_ROUND(12);
      }
    }
  }
}

extern "C" void m68k_0111(){  //moveq
  if(ir&BIT_8){
    m68k_unrecognised();
  }else{
    FETCH_TIMING;
    m68k_dest=&(r[PARAM_N]);
    m68k_DEST_L=(signed long)((signed char)LOBYTE(ir));
    SR_CLEAR(SR_Z+SR_N+SR_C+SR_V);
    SR_CHECK_Z_AND_N_L;
  }
}

extern "C" void m68k_1000(){ //or, div, sbcd
  m68k_jump_line_8[(ir&BITS_876)>>6]();
}

extern "C" void m68k_1001(){ //sub
  m68k_jump_line_9[(ir&BITS_876)>>6]();
}

extern "C" void m68k_1010() //line-a
{
  pc-=2;  //pc not incremented for illegal instruction

//  log_write("CPU sees line-a instruction");
//  intercept_line_a();
  INSTRUCTION_TIME_ROUND(0);  // Round first for interrupts
  INSTRUCTION_TIME_ROUND(34);
  m68k_interrupt(LPEEK(BOMBS_LINE_A*4));
  m68k_do_trace_exception=0;
  debug_check_break_on_irq(BREAK_IRQ_LINEA_IDX);
}

extern "C" void m68k_1011(){ //cmp, eor
  m68k_jump_line_b[(ir&BITS_876)>>6]();
}

extern "C" void m68k_1100(){ // and, abcd, exg, mul
  m68k_jump_line_c[(ir&BITS_876)>>6]();
}

extern "C" void m68k_1101(){   //add
  m68k_jump_line_d[(ir&BITS_876)>>6]();
}

extern "C" void m68k_1110(){  //bit shift
  m68k_jump_line_e[(ir&(BITS_876|BITS_543))>>3]();
}

extern "C" void m68k_1111(){  //line-f emulator
  pc-=2;  //pc not incremented for illegal instruction

#ifdef ONEGAME
  if (ir==0xffff){
    OGIntercept();
    return;
  }
#endif

  INSTRUCTION_TIME_ROUND(0);  // Round first for interrupts
  INSTRUCTION_TIME_ROUND(34);
  m68k_interrupt(LPEEK(BOMBS_LINE_F*4));
  m68k_do_trace_exception=0;
  debug_check_break_on_irq(BREAK_IRQ_LINEF_IDX);
}
//       check PC-relative addressing


#include "cpuinit.cpp"


