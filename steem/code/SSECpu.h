// Steem Steven Seagal Edition
// SSECpu.h


////////////////////////////////
// inlined (hopefully) macros //
////////////////////////////////

#if defined(SS_CPU)

// forward 
#ifdef _DEBUG_BUILD
#define DEBUG_CHECK_IOACCESS \
  if (ioaccess & IOACCESS_DEBUG_MEM_WRITE_LOG){ \
    int val=int((debug_mem_write_log_bytes==1) ? int(m68k_peek(debug_mem_write_log_address)):int(m68k_dpeek(debug_mem_write_log_address))); \
    log_write(HEXSl(old_pc,6)+": Write to address $"+HEXSl(debug_mem_write_log_address,6)+ \
                  ", new value is "+val+" ($"+HEXSl(val,debug_mem_write_log_bytes*2)+")"); \
  }
#else
#define DEBUG_CHECK_IOACCESS
#endif
//void set_pc(MEM_ADDRESS ad); 
#if defined(STEVEN_SEAGAL) && defined(_VC_BUILD)
extern "C" void ASMCALL m68k_trace();
#else
extern "C" ASMCALL void m68k_trace();
#endif
extern void (*m68k_high_nibble_jump_table[16])();
void HandleIOAccess();
void ASMCALL perform_crash_and_burn();

inline void FetchTiming() {
#if !defined(_DEBUG_BUILD)
  ASSERT(!Cpu.NextIrFetched);
#endif
#if defined(SS_DEBUG)
  Cpu.NextIrFetched=true;
#endif
  INSTRUCTION_TIME(4); 
  cpu_cycles&=-4;
}
#define FETCH_TIMING FetchTiming();


inline void FetchWord(WORD &dest_word) {
  // Replacingmacro FETCH_W(dest_word)
  dest_word=prefetch_buf[0];
  if(prefetched_2)
  {
    if(prefetch_buf[1]!=*lpfetch)
    {
      TRACE("Prefetched IRC:%X current:%X\n",prefetch_buf[1],*lpfetch);
//      prefetch_buf[1]=*lpfetch; // cancel prefetch - Anomaly, TB2 would never start
    }
    prefetch_buf[0]=prefetch_buf[1];
    prefetched_2=false;
  }
  else
    prefetch_buf[0]=*lpfetch; // next instr
  lpfetch+=MEM_DIR; // advance the fetch pointer
  if(lpfetch MEM_GE lpfetch_bound) // <=
    exception(BOMBS_BUS_ERROR,EA_FETCH,pc);
}
#define FETCH_W(dest_word) FetchWord(dest_word);


inline void HandleIOAccess() {
  // HANDLE_IOACCESS extended
  if(ioaccess) 
  {                             
    switch(ioaccess & IOACCESS_NUMBER_MASK)
    {                        
    case 1: 
      io_write_b(ioad,LOBYTE(iobuffer)); 
      break;    
    case 2: 
      io_write_w(ioad,LOWORD(iobuffer)); 
      break;    
    case 4: 
      io_write_l(ioad,iobuffer); 
      break;      
    case TRACE_BIT_JUST_SET: 
      m68k_trace();
      break;                                      
    }                                             
    if(ioaccess & IOACCESS_FLAG_DELAY_MFP)
    { 
      ioaccess&=~IOACCESS_FLAG_DELAY_MFP;  
      ioaccess|=IOACCESS_FLAG_FOR_CHECK_INTRS; 
    }
    else if(ioaccess & IOACCESS_FLAG_FOR_CHECK_INTRS_MFP_CHANGE)
      ioaccess|=IOACCESS_FLAG_DELAY_MFP;  
    if(ioaccess & IOACCESS_INTERCEPT_OS2)
    { 
      ioaccess&=~IOACCESS_INTERCEPT_OS2;  
      ioaccess|=IOACCESS_FLAG_FOR_CHECK_INTRS; 
    }
    else if(ioaccess & IOACCESS_INTERCEPT_OS)
      ioaccess|=IOACCESS_INTERCEPT_OS2; 
    if(ioaccess & IOACCESS_FLAG_FOR_CHECK_INTRS)
    {   
      check_for_interrupts_pending();          
      CHECK_STOP_USER_MODE_NO_INTR 
    }                                             
    DEBUG_CHECK_IOACCESS; 
    if (ioaccess & IOACCESS_FLAG_DO_BLIT) 
      Blitter_Start_Now(); 
    // These flags stay until the next instruction to stop interrupts  
    ioaccess=ioaccess & (IOACCESS_FLAG_DELAY_MFP | IOACCESS_INTERCEPT_OS2);                                   \
  }
}


inline void m68k_perform_rte() {
  // replacing macro M68K_PERFORM_RTE(checkints)
  MEM_ADDRESS pushed_return_address=m68k_lpeek(r[15]+2);
  // An Illegal routine could manipulate this value.
  set_pc(pushed_return_address);        
  sr=m68k_dpeek(r[15]);r[15]+=6;      
  sr&=SR_VALID_BITMASK;               
  DETECT_CHANGE_TO_USER_MODE;         
  DETECT_TRACE_BIT;                                           
}


inline void m68k_Process() {
  // Replacing macro m68k_PROCESS.
  LOG_CPU  
  old_pc=pc;  
#if defined(SS_DEBUG)
  Cpu.PreviousIr=ir;
  Cpu.nInstr++;
  Cpu.NextIrFetched=false;
#endif
#if defined(SS_CPU_PREFETCH)
  ASSERT(prefetched_2);
#if defined(SS_DEBUG)
  Cpu.PrefetchClass=0; // default - not used
#endif
#endif
  FetchWord(ir); // IR->IRD
  pc+=2; 
//  ASSERT(ir!=0xE7F9); // dbg: break on opcode...
  m68k_high_nibble_jump_table[ir>>12](); // go to instruction...
  // Macro inlined: HANDLE_IOACCESS(m68k_trace(););
  HandleIOAccess();
  DEBUG_ONLY( debug_first_instruction=0 );
}


// SET PC
#if defined(_VC_BUILD)
inline void set_pc(MEM_ADDRESS ad) {
#else
/*inline*/ void set_pc(MEM_ADDRESS ad) {
#endif
  pc=ad;                               
  pc_high_byte=pc & 0xff000000;     
  pc&=0xffffff;                    
  lpfetch=lpDPEEK(0);  //Default to instant bus error when fetch
  lpfetch_bound=lpDPEEK(0);         
//  ASSERT(pc);
  if(pc>=himem)
  {                                                       
    if(pc<MEM_IO_BASE)
    {           
      if(pc>=MEM_EXPANSION_CARTRIDGE)
      {                                
        if(pc>=0xfc0000)
        {                                                   
          if(tos_high && pc<(0xfc0000+192*1024))
          {         
            lpfetch=lpROM_DPEEK(pc-0xfc0000); 
            lpfetch_bound=lpROM_DPEEK(192*1024);         
          }                                                                          
        }
        else if(cart)
        {
          lpfetch=lpCART_DPEEK(pc-MEM_EXPANSION_CARTRIDGE); 
          lpfetch_bound=lpCART_DPEEK(128*1024);         
        }                                                                          
      }
      else if(pc>=rom_addr)
      {                                                      
        if(pc<(0xe00000 + 256*1024))
        {         
          lpfetch=lpROM_DPEEK(pc-0xe00000); 
          lpfetch_bound=lpROM_DPEEK(256*1024);         
        }                                                                          
      }                            
    }
    else
    {   
      if(pc>=0xff8240 && pc<0xff8260)
      {         
        lpfetch=lpPAL_DPEEK(pc-0xff8240); 
        lpfetch_bound=lpPAL_DPEEK(64+PAL_EXTRA_BYTES);         
      }                              
    }                                                                           
  }
  else // SS in normal ram
  {                                                                         
    lpfetch=lpDPEEK(pc);
    lpfetch_bound=lpDPEEK(mem_len+(MEM_EXTRA_BYTES/2));         
  }                                         
#if defined(SS_CPU_PREFETCH)
  prefetch_buf[0]=*lpfetch; // ir
  // prefetch queue is reloaded
  prefetch_buf[1]=*(lpfetch+MEM_DIR); 
  prefetched_2=true;
  if(Cpu.CallPrefetch) // JSR
  {
    ASSERT(Cpu.PrefetchedOpcode==prefetch_buf[0]); // asserts if it worked
    prefetch_buf[0]=Cpu.PrefetchedOpcode;
    Cpu.CallPrefetch=FALSE;
  }
  lpfetch+=MEM_DIR;  //let's not cause exceptions here
#else
  //PREFETCH_SET_PC:
  prefetched_2=false; // will have prefetched 1 word
  prefetch_buf[0]=*lpfetch;            
  lpfetch+=MEM_DIR;  //let's not cause exceptions here
#endif
}


inline void m68kReadBFromAddr() {
  // Replacing macro m68k_READ_B_FROM_ADDR.
  abus&=0xffffff;
  if(abus>=himem)
  {                                  
    if(abus>=MEM_IO_BASE)
    {            
      if(SUPERFLAG)
        m68k_src_b=io_read_b(abus);           
      else 
        exception(BOMBS_BUS_ERROR,EA_READ,abus);         
    }
    else if(abus>=0xfc0000)
    {                             
      if(tos_high && abus<(0xfc0000+192*1024))
        m68k_src_b=ROM_PEEK(abus-rom_addr);   
      else if (abus<0xfe0000 || abus>=0xfe2000) 
        exception(BOMBS_BUS_ERROR,EA_READ,abus);  
    }
    else if(abus>=MEM_EXPANSION_CARTRIDGE)
    {           
      if(cart)
        m68k_src_b=CART_PEEK(abus-MEM_EXPANSION_CARTRIDGE);  
      else
        m68k_src_b=(BYTE)0xff;
    }
    else if (abus>=rom_addr)
    {                         
      if(abus<(0xe00000+256*1024))
        m68k_src_b=ROM_PEEK(abus-rom_addr);                           
      else if (abus>=0xec0000)
        exception(BOMBS_BUS_ERROR,EA_READ,abus);          
      else
        m68k_src_b=(BYTE)0xff;                                          
    }
    else if (abus>=0xd00000 && abus<0xd80000)
      m68k_src_b=(BYTE)0xff;                                          
    else if(mmu_confused)
      m68k_src_b=mmu_confused_peek(abus,true);                                         
    else if(abus>=FOUR_MEGS)
      exception(BOMBS_BUS_ERROR,EA_READ,abus);                          
    else
      m68k_src_b=(BYTE)0xff;                                          
  }
  else if(abus>=MEM_START_OF_USER_AREA)
  {                                              
    DEBUG_CHECK_READ_B(abus);  
    m68k_src_b=(BYTE)(PEEK(abus));                  
  }
  else if(SUPERFLAG)
  {     
    DEBUG_CHECK_READ_B(abus);  
    m68k_src_b=(BYTE)(PEEK(abus));                  
  }
  else
    exception(BOMBS_BUS_ERROR,EA_READ,abus);
}
#define m68k_READ_B_FROM_ADDR m68kReadBFromAddr();        


inline void m68kReadWFromAddr() {
  // Replacing macro m68k_READ_W_FROM_ADDR.
  abus&=0xffffff;                                   
  if(abus&1)
    exception(BOMBS_ADDRESS_ERROR,EA_READ,abus);    
  else if(abus>=himem)
  {                                  
    if(abus>=MEM_IO_BASE)
    {            
      if(SUPERFLAG)
        m68k_src_w=io_read_w(abus);           
      else 
        exception(BOMBS_BUS_ERROR,EA_READ,abus);         
    }
    else if(abus>=0xfc0000)
    {                             
      if(tos_high && abus<(0xfc0000+192*1024))
        m68k_src_w=ROM_DPEEK(abus-rom_addr);   
      else if(abus<0xfe0000 || abus>=0xfe2000) 
        exception(BOMBS_BUS_ERROR,EA_READ,abus);  
    }
    else if(abus>=MEM_EXPANSION_CARTRIDGE)
    {           
      if(cart)
        m68k_src_w=CART_DPEEK(abus-MEM_EXPANSION_CARTRIDGE);  
      else
        m68k_src_w=(WORD)0xffff;                                    
    }
    else if(abus>=rom_addr)
    {                         
      if(abus<(0xe00000+256*1024)) 
        m68k_src_w=ROM_DPEEK(abus-rom_addr);                           
      else if (abus>=0xec0000) 
        exception(BOMBS_BUS_ERROR,EA_READ,abus);          
      else 
        m68k_src_w=(WORD)0xffff;                                          
    }
    else if (abus>=0xd00000 && abus<0xd80000)
      m68k_src_w=(WORD)0xffff;                                          
    else if(mmu_confused)
      m68k_src_w=mmu_confused_dpeek(abus,true);                                         
    else if(abus>=FOUR_MEGS)
      exception(BOMBS_BUS_ERROR,EA_READ,abus);                          
    else
      m68k_src_w=(WORD)0xffff;                                          
  }
  else if(abus>=MEM_START_OF_USER_AREA)
  {                                              
    DEBUG_CHECK_READ_W(abus);  
    m68k_src_w=DPEEK(abus);                  
  }
  else if(SUPERFLAG)
  {     
    DEBUG_CHECK_READ_W(abus);  
    m68k_src_w=DPEEK(abus);                  
  }
  else 
    exception(BOMBS_BUS_ERROR,EA_READ,abus);
}
#define m68k_READ_W_FROM_ADDR m68kReadWFromAddr();


inline void m68kReadLFromAddr() {
  // Replacing macro m68k_READ_L_FROM_ADDR.
  abus&=0xffffff;                                   
  if(abus&1)
    exception(BOMBS_ADDRESS_ERROR,EA_READ,abus);    
  else if(abus>=himem)
  {                                  
    if(abus>=MEM_IO_BASE)
    {           
      if(SUPERFLAG)
        m68k_src_l=io_read_l(abus);          
      else
        exception(BOMBS_BUS_ERROR,EA_READ,abus);         
    }
    else if(abus>=0xfc0000)
    {                             
      if(tos_high && abus<(0xfc0000+192*1024-2)) 
        m68k_src_l=ROM_LPEEK(abus-rom_addr);   
      else if(abus<0xfe0000 || abus>=0xfe2000)
        exception(BOMBS_BUS_ERROR,EA_READ,abus);  
    }
    else if(abus>=MEM_EXPANSION_CARTRIDGE)
    {           
      if(cart)
        m68k_src_l=CART_LPEEK(abus-MEM_EXPANSION_CARTRIDGE);  
      else
        m68k_src_l=0xffffffff;                                    
    }
    else if(abus>=rom_addr)
    {                         
      if(abus<(0xe00000+256*1024-2)) 
        m68k_src_l=ROM_LPEEK(abus-rom_addr);   
      else if(abus>=0xec0000)
        exception(BOMBS_BUS_ERROR,EA_READ,abus);          
      else
        m68k_src_l=0xffffffff;                                          
    }
    else if(abus>=0xd00000 && abus<0xd80000-2)
      m68k_src_l=0xffffffff;                                          
    else if (mmu_confused)
      m68k_src_l=mmu_confused_lpeek(abus,true);                                         
    else if(abus>=FOUR_MEGS)
      exception(BOMBS_BUS_ERROR,EA_READ,abus);                          
    else
      m68k_src_l=0xffffffff;                                          
  }
  else if(abus>=MEM_START_OF_USER_AREA)
  {                                              
    DEBUG_CHECK_READ_L(abus);  
    m68k_src_l=LPEEK(abus);                  
  }
  else if(SUPERFLAG)
  {     
    DEBUG_CHECK_READ_L(abus);  
    m68k_src_l=LPEEK(abus);                  
  }
  else
    exception(BOMBS_BUS_ERROR,EA_READ,abus);
}
#define m68k_READ_L_FROM_ADDR m68kReadLFromAddr();                   


inline void m68kSetDestBToAddr() {
  // Replacing macro m68k_SET_DEST_B_TO_ADDR
  abus&=0xffffff;                                   
  if(abus>=MEM_IO_BASE)
  {               
    if(SUPERFLAG)
    {                        
      ioaccess&=IOACCESS_FLAGS_MASK; 
      ioaccess|=1;                     
      ioad=abus;                        
      m68k_dest=&iobuffer;               
      DWORD_B_0(&iobuffer)=io_read_b(abus);        
    }
    else exception(BOMBS_BUS_ERROR,EA_WRITE,abus);             
  }
  else if(abus>=himem)
  {                               
    if(mmu_confused)                              
      mmu_confused_set_dest_to_addr(1,true);           
    else if(abus>=FOUR_MEGS)
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);                               
    else
      m68k_dest=&iobuffer;                             
  }
  else
  {                                            
    DEBUG_CHECK_WRITE_B(abus); 
    if (SUPERFLAG && abus>=MEM_FIRST_WRITEABLE)
      m68k_dest=lpPEEK(abus);           
    else if(abus>=MEM_START_OF_USER_AREA)
      m68k_dest=lpPEEK(abus);           
    else                                      
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);       
  }
}
#define m68k_SET_DEST_B_TO_ADDR m68kSetDestBToAddr();       


inline void m68kSetDestWToAddr() {
  // Replacing macro m68k_SET_DEST_W_TO_ADDR  
  abus&=0xffffff;                                   
  if(abus&1)
    exception(BOMBS_ADDRESS_ERROR,EA_WRITE,abus);    
  else if(abus>=MEM_IO_BASE)
  {               
    if(SUPERFLAG)
    {                        
      ioaccess&=IOACCESS_FLAGS_MASK; 
      ioaccess|=2;                     
      ioad=abus;                        
      m68k_dest=&iobuffer;               
      *((WORD*)&iobuffer)=io_read_w(abus);        
    }
    else 
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);                                
  }
  else if(abus>=himem)
  {                               
    if(mmu_confused)                               
      mmu_confused_set_dest_to_addr(2,true);           
    else if(abus>=FOUR_MEGS)                                                
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);                               
    else                                                        
      m68k_dest=&iobuffer;                             
  }
  else
  {                               
    DEBUG_CHECK_WRITE_W(abus);  
    if(SUPERFLAG && abus>=MEM_FIRST_WRITEABLE)                       
      m68k_dest=lpDPEEK(abus);           
    else if(abus>=MEM_START_OF_USER_AREA) 
      m68k_dest=lpDPEEK(abus);           
    else                                      
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);       
  }
}
#define m68k_SET_DEST_W_TO_ADDR m68kSetDestWToAddr();   


inline void m68kSetDestLToAddr() {
  // Replacing macro m68k_SET_DEST_L_TO_ADDR
  abus&=0xffffff;                                   
  if(abus&1)                                      
    exception(BOMBS_ADDRESS_ERROR,EA_WRITE,abus);    
  else if(abus>=MEM_IO_BASE)
  {               
    if(SUPERFLAG)
    {                        
      ioaccess&=IOACCESS_FLAGS_MASK; 
      ioaccess|=4;                     
      ioad=abus;                         
      m68k_dest=&iobuffer;               
      iobuffer=io_read_l(abus);        
    }
    else
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);                                 
  }
  else if(abus>=himem)
  {                               
    if(mmu_confused)                               
      mmu_confused_set_dest_to_addr(4,true);           
    else if(abus>=FOUR_MEGS)                                                
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);                               
    else                                                        
      m68k_dest=&iobuffer;                                              
  }
  else
  {                               
    DEBUG_CHECK_WRITE_L(abus);  
    if(SUPERFLAG && abus>=MEM_FIRST_WRITEABLE)                       
      m68k_dest=lpLPEEK(abus);           
    else if(abus>=MEM_START_OF_USER_AREA) 
      m68k_dest=lpLPEEK(abus);           
    else                                      
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);       
  }
}
#define m68k_SET_DEST_L_TO_ADDR m68kSetDestLToAddr();   


inline void m68kSetDestB(unsigned long addr) {
  // Replacing macro m68k_SET_DEST_B(addr)
  abus=addr;   
  m68kSetDestBToAddr();
}
#define m68k_SET_DEST_B(addr) m68kSetDestB(addr);


inline void m68kSetDestW(unsigned long addr) {
  // Replacing macro m68k_SET_DEST_W(addr)
  abus=addr;   
  m68kSetDestWToAddr();
}
#define m68k_SET_DEST_W(addr) m68kSetDestW(addr);


inline void m68kSetDestL(unsigned long addr) {
  // Replacing macro m68k_SET_DEST_L(addr)
  abus=addr;   
  m68kSetDestLToAddr();
}
#define m68k_SET_DEST_L(addr) m68kSetDestL(addr);

#endif // CPU


///////////////
// HBL & VBL //
///////////////

// It's here to avoid forwards (TODO)

#if defined(STEVEN_SEAGAL) && defined(SS_INTERRUPT)

#if defined(SS_VID_HATARI) && defined(SS_JITTER) 
#include "..\..\3rdparty\hatari\screen.h"
#include "..\..\3rdparty\hatari\video.h" 
#endif

// normally we use the "jitter" version anyway
inline void InterruptStartTimeWobble() {
  INSTRUCTION_TIME_ROUND(0);
#if defined(SS_MFP_RATIO)
  INSTRUCTION_TIME((CpuNormalHz-(ABSOLUTE_CPU_TIME-shifter_cycle_base)) % 10);
#else
  INSTRUCTION_TIME((8000000-(ABSOLUTE_CPU_TIME-shifter_cycle_base)) % 10);
#endif
}
//#define INTERRUPT_START_TIME_WOBBLE InterruptStartTimeWobble()


inline void HblInterrupt() {
  hbl_pending=false;
  log_to_section(LOGSECTION_INTERRUPTS,Str("INTERRUPT: HBL at PC=")+HEXSl(pc,6)+" "+scanline_cycle_log());
  M68K_UNSTOP;
#if defined(SS_JITTER) 
//  INSTRUCTION_TIME(HblJitterArray[HblJitterIndex]); // Hatari's way
#else
  InterruptStartTimeWobble(); // Steem's way
#endif
  time_of_last_hbl_interrupt=ABSOLUTE_CPU_TIME;
  INSTRUCTION_TIME_ROUND(HBL_CYCLES);
  m68k_interrupt(LPEEK(0x0068));       
#if defined(SS_JITTER) 
  INSTRUCTION_TIME(HblJitterArray[HblJitterIndex]); // Hatari's way
#endif
  sr=(sr & (WORD)(~SR_IPL)) | (WORD)(SR_IPL_2); 
  debug_check_break_on_irq(BREAK_IRQ_HBL_IDX); 
}
#define HBL_INTERRUPT HblInterrupt();


inline void VblInterrupt() {
  vbl_pending=false;                                             
  log_to_section(LOGSECTION_INTERRUPTS,EasyStr("INTERRUPT: VBL at PC=")+HEXSl(pc,6)+" time is "+ABSOLUTE_CPU_TIME+" ("+(ABSOLUTE_CPU_TIME-cpu_time_of_last_vbl)+" cycles into screen)");
  M68K_UNSTOP;
#if defined(SS_JITTER) 
//  INSTRUCTION_TIME(VblJitterArray[VblJitterIndex]);
#else
  InterruptStartTimeWobble();
#endif
  INSTRUCTION_TIME_ROUND(VBL_CYCLES);
  m68k_interrupt(LPEEK(0x0070)); // this sets PC
#if defined(SS_JITTER) 
  INSTRUCTION_TIME(VblJitterArray[VblJitterIndex]);
#endif
  sr=(sr& (WORD)(~SR_IPL))|(WORD)(SR_IPL_4);
  debug_check_break_on_irq(BREAK_IRQ_VBL_IDX);
}
#define VBL_INTERRUPT VblInterrupt();

#endif


//////////////
// Prefetch //
//////////////

// We enforce ijor's rules (http://pasti.fxatari.com/68kdocs/68kPrefetch.html)
// while staying in the Steem system to avoid duplication.
// Note that prefetch was good enough in Steem 3.2 to pass known cases 
// (some demos).

#if defined(SS_CPU_PREFETCH)

inline void PrefetchIrc() {
  ASSERT(!prefetched_2); // only once per instruction
//  if(prefetched_2) TRACE("prefetched_2 ir %X\n",ir);
//  if(!prefetched_2)
  {
    prefetch_buf[1]=*lpfetch;
    prefetched_2= TRUE;
  }
}
#define EXTRA_PREFETCH
#define PREFETCH_IRC PrefetchIrc();


inline void RefetchIr() {
  ASSERT(prefetch_buf[0]==*(lpfetch+1)); // detect cases
  prefetch_buf[0]=*(lpfetch-MEM_DIR);
}
#define REFETCH_IR RefetchIr();
#else
#define PREFETCH_IRC
#define REFETCH_IR
#endif