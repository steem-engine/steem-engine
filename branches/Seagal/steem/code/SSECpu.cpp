// Steem Steven Seagal Edition
// SSECpu.cpp

#include "..\..\3rdparty\pasti\div68kCycleAccurate.c"

TCpu Cpu; // singleton

TCpu::TCpu() {
#if defined(SS_DEBUG)    
  nExceptions=0;
  nInstr=0;
#endif
#if defined(SS_CPU_PREFETCH_CALL)
  CallPrefetch=FALSE;
#endif
}


////////////////
// Exceptions //
////////////////

#if defined(STEVEN_SEAGAL) && defined(SS_CPU_EXCEPTION)

void m68k_exception::crash()
{
  DWORD bytes_to_stack=int((bombs==BOMBS_BUS_ERROR || bombs==BOMBS_ADDRESS_ERROR) ? (4+2+2+4+2):(4+2));
  MEM_ADDRESS sp=(MEM_ADDRESS)(SUPERFLAG ? (areg[7] & 0xffffff):(other_sp & 0xffffff));
#if defined(SS_CPU_PREFETCH) && defined(SS_DEBUG)
  Cpu.PrefetchClass=2;
#endif
#if defined(SS_DEBUG)   // report in TRACE, helped a lot
  Cpu.nExceptions++;
  if(Cpu.nExceptions<=EXCEPTIONS_REPORTED
    &&!(bombs==2 && pc==0xfc0f38)) // blitter test
  {
#ifdef _DEBUG_BUILD
    EasyStr instr=disa_d2(old_pc);
    TRACE("PC:%X-Op:%X-Ins: %s -SR:%X-Bus:%X\n",old_pc,ir,instr.Text,sr,abus);
#else
    TRACE("PC:%X-Op:%X-SR:%X-Bus:%X\n",old_pc,ir,sr,abus);
#endif
    TRACE("Regs");
    int i;
    for(i=0;i<8;i++) // D0-D7
      TRACE(" D%d:%X",i,r[i]);
    TRACE("\nRegs");
    for(i=0;i<8;i++) // A0-A7 (A7 when the exception occured)
      TRACE(" A%d:%X",i,areg[i]);
    TRACE("\nException Nr %d, %d (",Cpu.nExceptions,bombs);
    switch(bombs)
    {  
    case 2:
      TRACE("BOMBS_BUS_ERROR"); 
      break;
    case 3:
      TRACE("BOMBS_ADDRESS_ERROR"); 
      break;
    case 4:
      TRACE("BOMBS_ILLEGAL_INSTRUCTION"); 
      break;
    case 5:
      TRACE("BOMBS_DIVISION_BY_ZERO"); 
      break;
    case 6:
      TRACE("BOMBS_CHK"); 
      break;
    case 7:
      TRACE("BOMBS_TRAPV"); 
      break;
    case 8:
      TRACE("BOMBS_PRIVILEGE_VIOLATION"); 
      break;
    case 9:
      TRACE("BOMBS_TRACE_EXCEPTION"); 
      break;
    case 10:
      TRACE("BOMBS_LINE_A"); 
      break;
    case 11:
      TRACE("BOMBS_LINE_F"); 
      break;
    }//sw
    TRACE(")-Vector: %X\n",LPEEK(bombs*4));
    if(Cpu.nExceptions==EXCEPTIONS_REPORTED)
      TRACE("%d exceptions. Stopping reporting exceptions.\n",Cpu.nExceptions);
  }
#endif//SS_DEBUG
  if (sp<bytes_to_stack || sp>FOUR_MEGS)
  {
    // Double bus error, CPU halt (we crash and burn)
    // This only has to be done here, m68k_PUSH_ will cause bus error if invalid
  //  DEBUG_ONLY( log_history(bombs,crash_address) );
    TRACE("Double bus error SP:%X\n",sp);
    perform_crash_and_burn();
  }
  else
  {
    INSTRUCTION_TIME_ROUND(0); //Round first for interrupts
    if(bombs==BOMBS_ILLEGAL_INSTRUCTION || bombs==BOMBS_PRIVILEGE_VIOLATION)
    {
      if(!SUPERFLAG) 
        change_to_supervisor_mode();
      m68k_PUSH_L((crash_address & 0x00ffffff) | pc_high_byte); 
      INSTRUCTION_TIME_ROUND(8);
      m68k_PUSH_W(_sr); // Status register 
      INSTRUCTION_TIME_ROUND(4); //Round first for interrupts
      MEM_ADDRESS ad=LPEEK(bombs*4); // Get the vector
      if(ad & 1) // bad vector!
      {
        // Very rare, rather indicates emulation bug
        bombs=BOMBS_ADDRESS_ERROR;
#if defined(SS_DEBUG)
        BRK(odd exception vector);
        Cpu.nExceptions++;
        TRACE("->%d bombs\n",bombs);
#endif
        address=ad;
        action=EA_FETCH;
      }
      else
      {
        set_pc(ad);
        SR_CLEAR(SR_TRACE);
        INSTRUCTION_TIME_ROUND(22); 
        interrupt_depth++; // Is this necessary?
      }
    }
    if(bombs==BOMBS_BUS_ERROR||bombs==BOMBS_ADDRESS_ERROR)
    {
      if(!SUPERFLAG) 
        change_to_supervisor_mode();
      TRY_M68K_EXCEPTION
      {
        // MOVE.L ad hoc hacks - TODO: other instructions?
        if((_ir & 0xf000)==(b00100000 << 8))
        {
          int offset=0;
          switch(_ir) // opcode
          {
          case 0x2285: // fixes War Heli, already in Steem 3.2.
            TRACE("War Heli?\n");
            offset=2;
            break;
          case 0x21F8: // TB2 loader fix #1, Phaleon
            TRACE("Phaleon or TB2 protection?\n");
            offset=-2;
            break;
          case 0x2210:
            TRACE("Use Pasti for Delirious IV loader\n");
            break;
          case 0x2ABB:
#if defined(SS_VAR_PROG_ID)
            SetProgram(LX_STF); // lots of time, so a prg ID
#endif
            break;
          case 0x2235: 
            TRACE("Super Neo Show demo?\n"); // strange beast
            break;
          case 0x20BC:
          case 0x2089:
            TRACE("Cuddly demo?\n");
            break;
          case 0x2390:
            TRACE("Delirious IV esc?\n");
            break;
          default:
            TRACE("MOVE.L crash - Opcode %X\n",_ir);
            break;
          }//sw
          _pc+=offset;
        }
        WORD x=WORD(_ir & 0xffe0); // status
        if(action!=EA_WRITE) x|=B6_010000;
        if(action==EA_FETCH)
          x|=WORD((_sr & SR_SUPER) ? FC_SUPERVISOR_PROGRAM:FC_USER_PROGRAM);
        else
          x|=WORD((_sr & SR_SUPER) ? FC_SUPERVISOR_DATA:FC_USER_DATA);
        m68k_PUSH_L(_pc);
        m68k_PUSH_W(_sr);
        m68k_PUSH_W(_ir);
        m68k_PUSH_L(address);
        m68k_PUSH_W(x);
      }
      CATCH_M68K_EXCEPTION
      {
        TRACE("Exception during exception...\n");
        r[15]=0xf000; // R15=A7
      }
      END_M68K_EXCEPTION
      SET_PC(LPEEK(bombs*4)); // includes final prefetch
      SR_CLEAR(SR_TRACE);
      INSTRUCTION_TIME_ROUND(50); //Round for fetch
    }
///    DEBUG_ONLY(log_history(bombs,crash_address));
  }
  PeekEvent(); // Stop exception freeze
}

#endif


//////////////
// Prefetch //
//////////////

#if defined(SS_CPU_PREFETCH)

WORD TCpu::FetchForCall(MEM_ADDRESS ad) {
  // fetch PC before pushing return address in JSR, fixes nothing
  ad&=0xffffff; 
  PrefetchAddress=lpDPEEK(0); // default
  if(ad>=himem)
  {
    if(ad<MEM_IO_BASE)
    {           
      if(ad>=MEM_EXPANSION_CARTRIDGE)
      {                                
        if(ad>=0xfc0000)
        {                                                   
          if(tos_high && ad<(0xfc0000+192*1024)) 
            PrefetchAddress=lpROM_DPEEK(ad-0xfc0000); 
        }
        else if(cart)
          PrefetchAddress=lpCART_DPEEK(ad-MEM_EXPANSION_CARTRIDGE); 
      }
      else if(ad>=rom_addr)
      {                                                      
        if(ad<(0xe00000 + 256*1024))
          PrefetchAddress=lpROM_DPEEK(ad-0xe00000); 
      }                            
    }
    else
    {   
      if(ad>=0xff8240 && ad<0xff8260)      
        PrefetchAddress=lpPAL_DPEEK(ad-0xff8240);
    }                                                                           
  }
  else //  in normal ram
    PrefetchAddress=lpDPEEK(ad);
  prefetched_2=false; // will have prefetched 1 word / cancels eventual prefetch
  PrefetchedOpcode=*(PrefetchAddress); // fetch instruction!
  CallPrefetch=TRUE;
  return PrefetchedOpcode;
}

#endif//prefetch


/////////////////////////
// Redone instructions //
/////////////////////////

#if defined(SS_CPU_CLR)
// CLR reads before writing but we leave timings unchanged (Decade Menu)
void                              m68k_clr_b(){
  FETCH_TIMING;
  if (DEST_IS_REGISTER==0){INSTRUCTION_TIME(4);}
  m68k_GET_DEST_B_NOT_A;
  PREFETCH_IRC;
  int save=cpu_cycles;
  if (DEST_IS_REGISTER==0) 
    m68kReadBFromAddr();
  cpu_cycles=save;
  m68k_DEST_B=0;
  SR_CLEAR(SR_N+SR_V+SR_C);
  SR_SET(SR_Z);
}void                             m68k_clr_w(){
  FETCH_TIMING;
  if(DEST_IS_REGISTER==0){INSTRUCTION_TIME(4);}
  m68k_GET_DEST_W_NOT_A;
  PREFETCH_IRC;
  int save=cpu_cycles;
  if (DEST_IS_REGISTER==0) 
    m68kReadWFromAddr();
  cpu_cycles=save;
  m68k_DEST_W=0;
  SR_CLEAR(SR_N+SR_V+SR_C);
  SR_SET(SR_Z);
}void                             m68k_clr_l(){
  FETCH_TIMING;
  if(DEST_IS_REGISTER){INSTRUCTION_TIME(2);}else {INSTRUCTION_TIME(8);}
  m68k_GET_DEST_L_NOT_A;
  PREFETCH_IRC;
  int save=cpu_cycles;
  if (DEST_IS_REGISTER==0) 
    m68kReadLFromAddr();
  cpu_cycles=save;
  m68k_DEST_L=0;
  SR_CLEAR(SR_N+SR_V+SR_C);
  SR_SET(SR_Z);
}

#endif

#if defined(SS_CPU_DIV) 
// using ijor's timings, what they fix proves them correct

void                              m68k_divu(){
  FETCH_TIMING;
  m68k_GET_SOURCE_W_NOT_A;
  if (m68k_src_w==0){ // div by 0
    // Clear V flag when dividing by zero. Fixes...?
    SR_CLEAR(SR_V);
    m68k_interrupt(LPEEK(BOMBS_DIVISION_BY_ZERO*4));
    INSTRUCTION_TIME_ROUND(0); //Round first for interrupts
    INSTRUCTION_TIME_ROUND(38);
  }else{
    unsigned long q;
    unsigned long dividend = (unsigned long) (r[PARAM_N]);
    unsigned short divisor = (unsigned short) m68k_src_w;
    int cycles_for_instr=getDivu68kCycles(dividend,divisor) -4; // -prefetch
    INSTRUCTION_TIME(cycles_for_instr); // fixes Pandemonium loader
    q=(unsigned long)((unsigned long)dividend)/(unsigned long)((unsigned short)divisor);
    PREFETCH_IRC;
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


void                              m68k_divs(){
  FETCH_TIMING;
  m68k_GET_SOURCE_W_NOT_A;
  if (m68k_src_w==0){
    /* Clear V flag when dividing by zero - Alcatraz Odyssey demo depends
     * on this (actually, it's doing a DIVS).  */
    SR_CLEAR(SR_V);
    m68k_interrupt(LPEEK(BOMBS_DIVISION_BY_ZERO*4));
    INSTRUCTION_TIME_ROUND(0); //Round first for interrupts
    INSTRUCTION_TIME_ROUND(38);
  }else{
    signed long q;
    signed long dividend = (signed long) (r[PARAM_N]);
    signed short divisor = (signed short) m68k_src_w;
    int cycles_for_instr=getDivs68kCycles(dividend,divisor)-4; // -prefetch
    INSTRUCTION_TIME(cycles_for_instr);   // fixes Dragonnels loader
    q=(signed long)((signed long)dividend)/(signed long)((signed short)divisor);
    PREFETCH_IRC;
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

#endif

#if defined(SS_CPU_MOVE_B)

void m68k_0001() {  // move.b
  INSTRUCTION_TIME(4);
#if defined(SS_CPU_PREFETCH)
  Cpu.PrefetchClass=1; // by default 
#endif
#if defined(SS_CPU_EXCEPTION)
  // We consider ILLEGAL before bus/address error - TB2 loader fix #2
  BOOL PostIncrement=FALSE;
  if( (ir&BITS_876)==BITS_876_001
    || (ir&BITS_876)==BITS_876_111
    && (ir&BITS_ba9)!=BITS_ba9_000
    && (ir&BITS_ba9)!=BITS_ba9_001 )
    m68k_unrecognised();
#endif
  // Source
  m68k_GET_SOURCE_B;
  // Destination
  if((ir&BITS_876)==BITS_876_000)
  { // Dn
    SR_CLEAR(SR_V+SR_C+SR_N+SR_Z);
    m68k_dest=&(r[PARAM_N]);
    m68k_DEST_B=m68k_src_b; // move completed
    SR_CHECK_Z_AND_N_B; // update flags
  }
  else if((ir&BITS_876)==BITS_876_001)
  {BRK(impossible illegal in move.b);} // m68k_unrecognised(); // Steem 3.2's way
  else
  {   //to memory
    bool refetch=false;
    switch(ir&BITS_876)
    {
    case BITS_876_010: // (An)
      abus=areg[PARAM_N];
      break;
    case BITS_876_011: // (An)+
      abus=areg[PARAM_N];
#if defined(SS_CPU_EXCEPTION)
      PostIncrement=TRUE;
#else
      areg[PARAM_N]++; 
      if(PARAM_N==7)
        areg[7]++;
#endif
      break;
    case BITS_876_100: // -(An)
#if defined(SS_CPU_PREFETCH)
      Cpu.PrefetchClass=0; 
      PREFETCH_IRC;
      FETCH_TIMING;
#endif  
      areg[PARAM_N]--;
      if(PARAM_N==7)
        areg[7]--;
      abus=areg[PARAM_N];
      break;
    case BITS_876_101: // (d16, An)
      INSTRUCTION_TIME(12-4-4);
      abus=areg[PARAM_N]+(signed short)m68k_fetchW();
      pc+=2;
      break;
    case BITS_876_110: // (d8, An, Xn)
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
        case BITS_ba9_000: // (xxx).W
      INSTRUCTION_TIME(12-4-4);
          abus=0xffffff & (unsigned long)((signed long)((signed short)m68k_fetchW()));
          pc+=2;
          break;
        case BITS_ba9_001: // (xxx).L
#if defined(SS_CPU_PREFETCH)
          if(refetch)
            Cpu.PrefetchClass=2; // only .L
#endif
          INSTRUCTION_TIME(16-4-4);
          abus=m68k_fetchL() & 0xffffff;
          pc+=4;
          break;
        default:
          BRK(impossible illegal in move.b); //m68k_unrecognised();
      }
    }
    // Set flags
    SR_CLEAR(SR_Z+SR_N+SR_C+SR_V);
    if(!m68k_src_b){
      SR_SET(SR_Z);
    }
    if(m68k_src_b&MSB_B){
      SR_SET(SR_N);
    }
    m68k_poke_abus(m68k_src_b); // write; could crash
#if defined(SS_CPU_EXCEPTION)
    if(PostIncrement)
    {
      areg[PARAM_N]++; 
      if(PARAM_N==7)
        areg[7]++;
    }
#endif
#if defined(SS_CPU_PREFETCH)
    if(Cpu.PrefetchClass==2)
    {
      REFETCH_IR;
      PREFETCH_IRC;
    }
    if(Cpu.PrefetchClass) // -(An), already fetched
      FETCH_TIMING;
#else
    if (refetch) prefetch_buf[0]=*(lpfetch-MEM_DIR);
    FETCH_TIMING;  // move fetches after instruction
#endif  
  }// to memory
#if defined(SS_CPU_PREFETCH)
  if(Cpu.PrefetchClass==1) // classes 0 & 2 already handled
    PREFETCH_IRC;
#endif
}

#endif

#if defined(SS_CPU_MOVE_L)

void m68k_0010()  //move.l
{
  INSTRUCTION_TIME(4); 
#if defined(SS_CPU_PREFETCH)
  Cpu.PrefetchClass=1; // by default 
#endif
#if defined(SS_CPU_EXCEPTION)
  BOOL PostIncrement=FALSE;
  if( (ir&BITS_876)==BITS_876_111
    && (ir&BITS_ba9)!=BITS_ba9_000
    && (ir&BITS_ba9)!=BITS_ba9_001 )
      m68k_unrecognised();
#endif
  m68k_GET_SOURCE_L;
  // Destination
  if ((ir & BITS_876)==BITS_876_000) // Dn
  { 
    SR_CLEAR(SR_V+SR_C+SR_N+SR_Z);
    m68k_dest=&(r[PARAM_N]);
    m68k_DEST_L=m68k_src_l;
    SR_CHECK_Z_AND_N_L;
  }
  else if ((ir & BITS_876)==BITS_876_001) // An
    areg[PARAM_N]=m68k_src_l; // MOVEA
  else
  {   //to memory
    bool refetch=0;
    switch(ir&BITS_876)
    {
    case BITS_876_010: // (An)
      INSTRUCTION_TIME(12-4-4);
      abus=areg[PARAM_N];
      break;
    case BITS_876_011: // (An)+
      INSTRUCTION_TIME(12-4-4);
      abus=areg[PARAM_N];
#if defined(SS_CPU_EXCEPTION)
      PostIncrement=TRUE;
#else
      areg[PARAM_N]+=4;
#endif
      break;
    case BITS_876_100: // -(An)
      INSTRUCTION_TIME(12-4-4);
#if defined(SS_CPU_PREFETCH)
      Cpu.PrefetchClass=0; 
      PREFETCH_IRC;
      FETCH_TIMING;
#endif  
      areg[PARAM_N]-=4;
      abus=areg[PARAM_N];
      break;
    case BITS_876_101: // (d16, An)
      INSTRUCTION_TIME(16-4-4);
      abus=areg[PARAM_N]+(signed short)m68k_fetchW();
      pc+=2;
      break;
    case BITS_876_110: // (d8, An, Xn)
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
      case BITS_ba9_000: // (xxx).W
        INSTRUCTION_TIME(16-4-4);
        abus=0xffffff&(unsigned long)((signed long)((signed short)m68k_fetchW()));
        pc+=2;
        break;
      case BITS_ba9_001: // (xxx).L
        INSTRUCTION_TIME(20-4-4);
#if defined(SS_CPU_PREFETCH)
        if(refetch) 
          Cpu.PrefetchClass=2; 
#endif 
        abus=m68k_fetchL()&0xffffff;
        pc+=4;
        break;
      default:
        BRK(impossible illegal in move.l); ///m68k_unrecognised();
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
#if defined(SS_CPU_EXCEPTION)
    if(PostIncrement)
      areg[PARAM_N]+=4;
#endif
#if defined(SS_CPU_PREFETCH)
    if(Cpu.PrefetchClass==2)
    {
      REFETCH_IR;
      PREFETCH_IRC;
    }
    if(Cpu.PrefetchClass) // -(An), already fetched
      FETCH_TIMING;
#else
    if (refetch) prefetch_buf[0]=*(lpfetch-MEM_DIR);
    FETCH_TIMING;  // move fetches after instruction
#endif  
  }// to memory
#if defined(SS_CPU_PREFETCH)
  if(Cpu.PrefetchClass==1)
    PREFETCH_IRC;
#endif
}

#endif

#if defined(SS_CPU_MOVE_W)

void m68k_0011() //move.w
{
  INSTRUCTION_TIME(4);
#if defined(SS_CPU_PREFETCH)
  Cpu.PrefetchClass=1; // by default 
#endif
#if defined(SS_CPU_EXCEPTION)
  BOOL PostIncrement=FALSE;
    if( (ir&BITS_876)==BITS_876_111
      && (ir&BITS_ba9)!=BITS_ba9_000
      && (ir&BITS_ba9)!=BITS_ba9_001 )
      m68k_unrecognised();
#endif
  m68k_GET_SOURCE_W;
  // Destination
  if ((ir & BITS_876)==BITS_876_000) // Dn
  {
    SR_CLEAR(SR_V+SR_C+SR_N+SR_Z);
    m68k_dest=&(r[PARAM_N]);
    m68k_DEST_W=m68k_src_w;
    SR_CHECK_Z_AND_N_W;
  }
  else if ((ir & BITS_876)==BITS_876_001) // An
    areg[PARAM_N]=(signed long)((signed short)m68k_src_w); // movea
  else
  {   //to memory
    bool refetch=0;
    switch (ir & BITS_876)
    {
    case BITS_876_010: // (An)
      abus=areg[PARAM_N];
      break;
    case BITS_876_011:  // (An)+
      abus=areg[PARAM_N];
#if defined(SS_CPU_EXCEPTION)
      PostIncrement=TRUE; // fixes Beyond loader
#else
      areg[PARAM_N]+=2;
#endif
      break;
    case BITS_876_100: // -(An)
#if defined(SS_CPU_PREFETCH)
      Cpu.PrefetchClass=0; 
      PREFETCH_IRC;
      FETCH_TIMING;
#endif  
      areg[PARAM_N]-=2;
      abus=areg[PARAM_N];
      break;
    case BITS_876_101: // (d16, An)
      INSTRUCTION_TIME(12-4-4);
      abus=areg[PARAM_N]+(signed short)m68k_fetchW();
      pc+=2;
      break;
    case BITS_876_110: // (d8, An, Xn)
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
      case BITS_ba9_000: // (xxx).W
        INSTRUCTION_TIME(12-4-4);
        abus=0xffffff&(unsigned long)((signed long)((signed short)m68k_fetchW()));
        pc+=2;
        break;
      case BITS_ba9_001: // (xxx).L
        INSTRUCTION_TIME(16-4-4);
#if defined(SS_CPU_PREFETCH)
        if(refetch) 
          Cpu.PrefetchClass=2; 
#endif  
        abus=m68k_fetchL()&0xffffff;
        pc+=4;
        break;
      default:
        BRK(impossible illegal in move.w);///        m68k_unrecognised();
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
#if defined(SS_CPU_EXCEPTION)
    if(PostIncrement)
      areg[PARAM_N]+=2;
#endif

#if defined(SS_CPU_PREFETCH)
    if(Cpu.PrefetchClass==2)
    {
      REFETCH_IR;
      PREFETCH_IRC;
    }
    if(Cpu.PrefetchClass) // -(An), already fetched
      FETCH_TIMING;
#else
    if (refetch) prefetch_buf[0]=*(lpfetch-MEM_DIR);
    FETCH_TIMING;  // move fetches after instruction
#endif  
  }// to memory
#if defined(SS_CPU_PREFETCH)
  if(Cpu.PrefetchClass==1)
    PREFETCH_IRC;
#endif
}

#endif

