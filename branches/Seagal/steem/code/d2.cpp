/*---------------------------------------------------------------------------
FILE: d2.cpp
MODULE: Steem
CONDITION: _DEBUG_BUILD
DESCRIPTION: A Motorola 68000 disassembler. This is laid out exactly as
cpu.cpp but rather than performing the operations it simply generates a
disassembly. The only functions of interest to the outside program are
disa_d2 (see bottom of file) to disassemble an instruction and
d2_routines_init to initialise the debugger.
---------------------------------------------------------------------------*/


void (*d2_high_nibble_jump_table[16])();
void (*d2_jump_line_0[64])();
void (*d2_jump_line_4[64])();
void (*d2_jump_line_5[8])();
void (*d2_jump_line_8[8])();
void (*d2_jump_line_9[8])();
void (*d2_jump_line_b[8])();
void (*d2_jump_line_c[8])();
void (*d2_jump_line_d[8])();
void (*d2_jump_line_e[64])();
void (*d2_jump_line_4_stuff[64])();
void (*d2_jump_get_source_b[16])();
void (*d2_jump_get_source_w[16])();
void (*d2_jump_get_source_l[16])();
void (*d2_jump_get_dest_b[16])();
void (*d2_jump_get_dest_w[16])();
void (*d2_jump_get_dest_l[16])();
void (*d2_jump_get_dest_b_not_a[16])();
void (*d2_jump_get_dest_w_not_a[16])();
void (*d2_jump_get_dest_l_not_a[16])();
void (*d2_jump_get_dest_b_not_a_or_d[16])();
void (*d2_jump_get_dest_w_not_a_or_d[16])();
void (*d2_jump_get_dest_l_not_a_or_d[16])();
void (*d2_jump_get_dest_b_not_a_with_sr[16])();
void (*d2_jump_get_dest_w_not_a_with_sr[16])();
void (*d2_jump_get_dest_l_not_a_with_sr[16])();


EasyStr d2_signed_offset(short a){
  if(a>-100 && a<100)return EasyStr((short)a);
  else if(a>0)return EasyStr("+$")+HEXS(a);
  else return EasyStr("-$")+HEXS(-a);
}


EasyStr d2_iriwo(){
  EasyStr et=itoa((signed char)LOBYTE(d2_ap),d2_t_buf,10);
  et+="(";et+=D2_aM;et+=",";et+=reg_name(((unsigned short)d2_ap)>>12);
  et+=".";et+=LPSTR(BTSTb(d2_ap) ? "L)":"W)");
  return et;
}

EasyStr d2_iriwo_N(){
  EasyStr et=itoa((signed char)LOBYTE(d2_ap),d2_t_buf,10);
  et+="(";et+=D2_aN;et+=",";et+=reg_name(((unsigned short)d2_ap)>>12);
  et+=".";et+=LPSTR(BTSTb(d2_ap) ? "L)":"W)");
  return et;
}

EasyStr d2_iriwo_pc(){
  EasyStr et=itoa((signed char)LOBYTE(d2_ap),d2_t_buf,10);
  et+="(pc,";
  et+=reg_name(((unsigned short)d2_ap)>>12);
  et+=".";et+=LPSTR(BTSTb(d2_ap) ? "L)":"W)");
  return et;
}


MEM_ADDRESS d2_calc_iriwo(MEM_ADDRESS aaa){
//  EasyStr(itoa((signed char)LOBYTE(d2_ap),d2_t_buf,10))+
//        "("+D2_aM+",r"+itoa((d2_ap>>12),d2_t_buf,10)+"."+
//        (BTSTb(d2_ap)?"L":"W")+")"
  MEM_ADDRESS ad=aaa;
  if(BTSTb(d2_ap))ad+=(signed long)r[d2_ap>>12];
  else ad+=(signed short)LOWORD(r[d2_ap>>12]);
  ad+=(signed char)LOBYTE(d2_ap);

  trace_add_entry("source offset register: ",reg_name(d2_ap>>12),TDE_BEFORE,true,int(BTSTb(d2_ap) ? 4:2),(MEM_ADDRESS)&r[d2_ap>>12]);
  trace_add_entry("immediate offset: ","",TDE_BEFORE,false,1,dpc+1);

  return ad;
}

BYTE d2_fetchB()
{
  return d2_peek(dpc+1);
//  dpc+=2;
/*
  if(dpc>=mem_len){
    dpc&=0xffffff;
    if(dpc>=MEM_IO_BASE)return 0;
    else if(dpc>=MEM_EXPANSION_CARTRIDGE){
      if(dpc>=0xfc0000){
        if(tos_high)return (BYTE)ROM_PEEK(dpc+1-rom_addr);
        else return 0;
      }
    }else if(dpc>=rom_addr){
      return (BYTE)ROM_PEEK(dpc+1-rom_addr);
    }else return 0;
  }else{
    return (BYTE)PEEK(dpc+1);
  }
  return 0;
*/
}

WORD d2_fetchW()
{
//  dpc+=2;
  return d2_dpeek(dpc);
/*
  if(dpc>=mem_len){
    dpc&=0xffffff;
    if(dpc>=MEM_IO_BASE)return 0;
    else if(dpc>=MEM_EXPANSION_CARTRIDGE){
      if(dpc>=0xfc0000){
        if(tos_high)return (WORD)ROM_DPEEK(dpc-rom_addr);
        else return 0;
      }
    }else if(dpc>=rom_addr){
      return (WORD)ROM_DPEEK(dpc-rom_addr);
    }else return 0;
  }else{
    return (WORD)DPEEK(dpc);
  }
  return 0;
*/
}

LONG d2_fetchL()
{
//  dpc+=4;
  return d2_lpeek(dpc);
/*
  if(dpc>=mem_len){
    dpc&=0xffffff;
    if(dpc>=MEM_IO_BASE)return 0;
    else if(dpc>=MEM_EXPANSION_CARTRIDGE){
      if(dpc>=0xfc0000){
        if(tos_high)return (LONG)ROM_LPEEK(dpc-rom_addr);
        else return 0;
      }
    }else if(dpc>=rom_addr){
      return (LONG)ROM_LPEEK(dpc-rom_addr);
    }else return 0;
  }else{
    return (LONG)LPEEK(dpc);
  }
  return 0;
*/
}

BYTE d2_peek(MEM_ADDRESS ad){
  d2_peekvalid=0; //all valid
  if(ad>=himem){
    ad&=0xffffff;
    if(ad>=MEM_IO_BASE){
      BYTE x;
      int old_mode=mode;
      mode=STEM_MODE_INSPECT;
      TRY_M68K_EXCEPTION
//      try{
        x=io_read_b(ad);
//      }catch(m68k_exception){
      CATCH_M68K_EXCEPTION
        d2_peekvalid=0xff; //byte invalid
      END_M68K_EXCEPTION
      mode=old_mode;
      return x;
    }else if(ad>=MEM_EXPANSION_CARTRIDGE){
      if(ad>=0xfc0000){
        if(tos_high && ad<(0xfc0000+192*1024))return ROM_PEEK(ad-rom_addr);
        else{d2_peekvalid=0xff;return 0;}
      }else if(cart){
        return CART_PEEK(ad-MEM_EXPANSION_CARTRIDGE);
      }
    }else if(ad>=rom_addr){
      if(ad<=(0xe00000+256*1024))return ROM_PEEK(ad-rom_addr);
      else{d2_peekvalid=0xff;return 0;}
    }else if(mmu_confused){
      return mmu_confused_peek(ad,false);
    }
  }else{
    return PEEK(ad);
  }
  d2_peekvalid=0xff;return 0;
}


WORD d2_dpeek(MEM_ADDRESS ad)
{
  d2_peekvalid=0; //all valid
  if(ad>=mem_len){
    ad&=0xffffff;
    if(ad>=MEM_IO_BASE){
      int old_mode=mode;
      mode=STEM_MODE_INSPECT;
      WORD x=0;
      for(int nn=0;nn<2;nn++){
        x<<=8;
        d2_peekvalid<<=8;
        TRY_M68K_EXCEPTION
          x|=io_read_b(ad+nn);
        CATCH_M68K_EXCEPTION
          d2_peekvalid|=0xff; //byte invalid
        END_M68K_EXCEPTION
      }
      mode=old_mode;
      return x;
    }else if(ad>=MEM_EXPANSION_CARTRIDGE){
      if(ad>=0xfc0000){
        if(tos_high && ad<(0xfc0000+192*1024))return (WORD)ROM_DPEEK(ad-rom_addr);
        else {d2_peekvalid=0xffff;return 0;}
      }else if(cart){
        return CART_DPEEK(ad-MEM_EXPANSION_CARTRIDGE);
      }
    }else if(ad>=rom_addr){
      if(ad<=(0xe00000+256*1024))      return (WORD)ROM_DPEEK(ad-rom_addr);
      else {d2_peekvalid=0xffff;return 0;}
    }else if(mmu_confused){
      return mmu_confused_dpeek(ad,false);
    }
  }else{
    return (WORD)DPEEK(ad);
  }
  d2_peekvalid=0xffff;return 0;
}

LONG d2_lpeek(MEM_ADDRESS ad){
  d2_peekvalid=0; //all valid
  if (ad>=mem_len){
    ad&=0xffffff;
    if (ad>=MEM_IO_BASE){
      int old_mode=mode;
      mode=STEM_MODE_INSPECT;
      LONG x=0;
      for (int nn=0;nn<4;nn++){
        x<<=8;
        d2_peekvalid<<=8;
//        try{
        TRY_M68K_EXCEPTION
          x|=io_read_b(ad+nn);
//        }catch(m68k_exception){
        CATCH_M68K_EXCEPTION
          d2_peekvalid|=0xff; //byte invalid
//        }
        END_M68K_EXCEPTION
      }
      mode=old_mode;
      return x;
    }else if(ad>=MEM_EXPANSION_CARTRIDGE){
      if(ad>=0xfc0000){
        if(tos_high && ad<(0xfc0000+192*1024))return (LONG)ROM_LPEEK(ad-rom_addr);
        else {d2_peekvalid=0xffffffff;return 0;}
      }else if(cart){
        return CART_LPEEK(ad-MEM_EXPANSION_CARTRIDGE);
      }
    }else if(ad>=rom_addr){
      if(ad<=(0xe00000+256*1024))      return (LONG)ROM_LPEEK(ad-rom_addr);
      else {d2_peekvalid=0xffffffff;return 0;}
    }else if(mmu_confused){
      return mmu_confused_lpeek(ad,false);
    }
  }else{
    return (LONG)LPEEK(ad);
  }
  d2_peekvalid=0xffffffff;return 0;
}

bool d2_poke(MEM_ADDRESS ad,BYTE val){
  ad&=0xffffff;
  if(ad>=mem_len){
    if(ad>=MEM_IO_BASE){
//      try{
      bool Ret=true;
      TRY_M68K_EXCEPTION
        io_write_b(ad,val);
//      }catch(m68k_exception){
      CATCH_M68K_EXCEPTION
        Ret=0;
//      }
      END_M68K_EXCEPTION
      return Ret;
    }else if(ad>=MEM_EXPANSION_CARTRIDGE){
      if(ad>=0xfc0000){
        if(tos_high && ad<(0xfc0000+192*1024)){ROM_PEEK(ad-rom_addr)=val;return true;}
        else return false;
      }
    }else if(ad>=rom_addr  && ad<(0xe00000+256*1024)){
      ROM_PEEK(ad-rom_addr)=val;return true;
    }
  }else{
    PEEK(ad)=val;return true;
  }
  return false;
}

bool d2_dpoke(MEM_ADDRESS ad,WORD val){
  ad&=0xffffff;
  if(ad&1)return false;
  if(ad>=mem_len){
    if(ad>=MEM_IO_BASE){
//      try{
      bool Ret=true;
      TRY_M68K_EXCEPTION
        io_write_w(ad,val);
//      }catch(m68k_exception){
      CATCH_M68K_EXCEPTION
        Ret=0;
//      }
      END_M68K_EXCEPTION
      return Ret;
    }else if(ad>=MEM_EXPANSION_CARTRIDGE){
      if(ad>=0xfc0000){
        if(tos_high){ROM_DPEEK(ad-rom_addr)=val;return true;}
        else return false;
      }
    }else if(ad>=rom_addr){
      ROM_DPEEK(ad-rom_addr)=val;return true;
    }
  }else{
    DPEEK(ad)=val;return true;
  }
  return false;
}

bool d2_lpoke(MEM_ADDRESS ad,LONG val){
  ad&=0xffffff;
  if(ad&1)return false;
  if(ad>=mem_len){
    if(ad>=MEM_IO_BASE){
//      try{
      bool Ret=true;
      TRY_M68K_EXCEPTION
        io_write_l(ad,val);
//      }catch(m68k_exception){
      CATCH_M68K_EXCEPTION
        Ret=0;
//      }
      END_M68K_EXCEPTION
      return Ret;
    }else if(ad>=MEM_EXPANSION_CARTRIDGE){
      if(ad>=0xfc0000){
        if(tos_high){ROM_LPEEK(ad-rom_addr)=val;return true;}
        else return false;
      }
    }else if(ad>=rom_addr){
      ROM_LPEEK(ad-rom_addr)=val;return true;
    }
  }else{
    LPEEK(ad)=val;return true;
  }
  return false;
}

void d2_unrecognised(){
  d2_src_b="";
  d2_src_w="";
  d2_src_l="";
  d2_dest="";
  d2_command=EasyStr("dc.w $")+itoa(ir,d2_t_buf,16);
  dpc=old_dpc; //in case we've got more gumpf for the source
  if(d2_trace)trace_entries=0;
}

EasyStr d2_get_movem_regs(bool backwards){
//  return disa_get_movem_regs(backwards);
  EasyStr dt="";
  int regs=d2_dpeek(dpc);dpc+=2;
  bool had_one=false;
  d2_n_movem_regs=0;
  int msk=1,old_msk;if(backwards)msk=0x8000;
  for(int da=0;da<=1;da++){
    int mn=999;
    for(int r=0;r<8;r++){
      if(regs&msk){
//        dt+=EasyStr("[")+reg_name(da*8+r)+"]";
        d2_n_movem_regs++;
        if(mn==999)mn=r;
      }else{
        if(mn<r-1){
          if(had_one)dt+="/";
          dt+=EasyStr(reg_name(da*8+mn))+"-";
          dt+=itoa((r-1),d2_t_buf,10);
          mn=999;had_one=true;
        }else if(mn==r-1){
          if(had_one)dt+="/";
          dt+=EasyStr(reg_name(da*8+mn));
          mn=999;had_one=true;
        }
      }
      old_msk=msk;
      if(backwards)msk>>=1;else msk<<=1;
    }
    if(regs&old_msk){
      if(mn<7){
        if(had_one)dt+="/";
        dt+=EasyStr(reg_name(da*8+mn))+"-7";
        had_one=true;
      }else if(mn==7){
        if(had_one)dt+="/";
        dt+=EasyStr(reg_name(da*8+mn));
        had_one=true;
      }
    }

  }
  if(d2_trace){
    int msk=1;if(backwards)msk=0x8000;
    d2_n_movem_regs=0;
    for(int rn=0;rn<16;rn++){
      if(regs&msk){
        d2_n_movem_regs++;
        trace_add_entry("register in movem: ",reg_name(rn),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[rn]);
      }
      if(backwards)msk>>=1;else msk<<=1;
    }
  }
  return dt;
}
EasyStr d2_effective_address(){
  EasyStr ret;
  switch(ir&BITS_543){
  case BITS_543_010:
    trace_add_entry("source register: ",reg_name(PARAM_M+8),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    trace_add_entry("predecrement stack pointer: ","sp=a7",TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[15]);
    return EasyStr("(")+D2_aM+")";
  case BITS_543_101:
    trace_add_entry("immediate offset: ","",TDE_BEFORE,false,2,dpc);
    trace_add_entry("source register: ",reg_name(PARAM_M+8),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    trace_add_entry("predecrement stack pointer: ","sp=a7",TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[15]);
    ret=d2_signed_offset(d2_fetchW())+D2_BRACKETS_aM;
    dpc+=2;
    return ret;
  case BITS_543_110:
    d2_ap=d2_fetchW();
    ret=D2_IRIWO;
    trace_add_entry("source address register: ",reg_name(PARAM_M+8),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    trace_add_entry("source offset register: ",reg_name(d2_ap>>12),TDE_BEFORE,true,int(BTSTb(d2_ap) ? 4:2),(MEM_ADDRESS)&r[d2_ap>>12]);
    trace_add_entry("immediate offset: ","",TDE_BEFORE,false,1,dpc+1);
    trace_add_entry("predecrement stack pointer: ","sp=a7",TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[15]);
    dpc+=2;
    return ret;
  case BITS_543_111:
    switch(ir&0x7){
    case 0:
      ret=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+".w";
      trace_add_entry("effective absolute address: ",d2_src_w.c_str(),TDE_BEFORE,false,2,dpc);
      trace_add_entry("predecrement stack pointer: ","sp=a7",TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[15]);
      dpc+=2;
      return ret;
    case 1:
      ret=EasyStr("$")+itoa(d2_fetchL(),d2_t_buf,16);
      trace_add_entry("effective absolute address: ",d2_src_w.c_str(),TDE_BEFORE,false,4,dpc);
      trace_add_entry("predecrement stack pointer: ","sp=a7",TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[15]);
      dpc+=4;
      return ret;
    case 2:
      ret=EasyStr("+$")+itoa(d2_fetchW(),d2_t_buf,16)+"(pc)";
      d2_pc_rel_ex+=EasyStr(" {$")+HEXSl(dpc+(signed short)d2_fetchW(),6)+"}";
      trace_add_entry("immediate offset: ","",TDE_BEFORE,false,2,dpc);
      trace_add_entry("predecrement stack pointer: ","sp=a7",TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[15]);
      dpc+=2;
      return ret;
    case 3:
      d2_ap=d2_fetchW();
      ret=D2_IRIWO_PC;
      trace_add_entry("source offset register: ",reg_name(d2_ap>>12),TDE_BEFORE,true,int(BTSTb(d2_ap) ? 4:2),(MEM_ADDRESS)&r[d2_ap>>12]);
      trace_add_entry("immediate offset: ","",TDE_BEFORE,false,1,dpc+1);
      trace_add_entry("predecrement stack pointer: ","sp=a7",TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[15]);
      dpc+=2;
      return ret;
//      break;       //what do bits 8,9,a  of extra word do?  (not always 0)
    default:
      d2_unrecognised();
      return "";
    }
  default:
    d2_unrecognised();
    return "";
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

void d2_get_source_000_b(){ d2_src_b=D2_dM;
  trace_add_entry("source register: ",reg_name(PARAM_M),TDE_BEFORE,true,1,(MEM_ADDRESS)&r[PARAM_M]);}
void d2_get_source_000_w(){ d2_src_w=D2_dM;
  trace_add_entry("source register: ",reg_name(PARAM_M),TDE_BEFORE,true,2,(MEM_ADDRESS)&r[PARAM_M]);}
void d2_get_source_000_l(){ d2_src_l=D2_dM;
  trace_add_entry("source register: ",reg_name(PARAM_M),TDE_BEFORE,true,4,(MEM_ADDRESS)&r[PARAM_M]);}
void d2_get_source_001_b(){ d2_src_b=D2_aM;
  trace_add_entry("source register: ",reg_name(8+PARAM_M),TDE_BEFORE,true,1,(MEM_ADDRESS)&areg[PARAM_M]);}
void d2_get_source_001_w(){ d2_src_w=D2_aM;
  trace_add_entry("source register: ",reg_name(8+PARAM_M),TDE_BEFORE,true,2,(MEM_ADDRESS)&areg[PARAM_M]);}
void d2_get_source_001_l(){ d2_src_l=D2_aM;
  trace_add_entry("source register: ",reg_name(8+PARAM_M),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_M]);}
void d2_get_source_010_b(){ d2_src_b=D2_BRACKETS_aM;
  trace_add_entry("source memory: ",d2_src_b.c_str(),TDE_BEFORE,false,1,areg[PARAM_M]);
}
void d2_get_source_010_w(){ d2_src_w=D2_BRACKETS_aM;
  trace_add_entry("source memory: ",d2_src_w.c_str(),TDE_BEFORE,false,2,areg[PARAM_M]);
}
void d2_get_source_010_l(){ d2_src_l=D2_BRACKETS_aM;
  trace_add_entry("source memory: ",d2_src_l.c_str(),TDE_BEFORE,false,4,areg[PARAM_M]);
}
void d2_get_source_011_b(){ d2_src_b=D2_BRACKETS_aM+"+";
  trace_add_entry("source memory: ",d2_src_b.c_str(),TDE_BEFORE,false,1,areg[PARAM_M]);
  trace_add_entry("postincrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
}
void d2_get_source_011_w(){ d2_src_w=D2_BRACKETS_aM+"+";
  trace_add_entry("source memory: ",d2_src_w.c_str(),TDE_BEFORE,false,2,areg[PARAM_M]);
  trace_add_entry("postincrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
}
void d2_get_source_011_l(){ d2_src_l=D2_BRACKETS_aM+"+";
  trace_add_entry("source memory: ",d2_src_l.c_str(),TDE_BEFORE,false,4,areg[PARAM_M]);
  trace_add_entry("postincrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
}
void d2_get_source_100_b(){ d2_src_b=EasyStr("-")+D2_BRACKETS_aM;
  trace_add_entry("source memory: ",d2_src_b.c_str(),TDE_BEFORE,false,1,areg[PARAM_M]-1);
  trace_add_entry("predecrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
}
void d2_get_source_100_w(){ d2_src_w=EasyStr("-")+D2_BRACKETS_aM;
  trace_add_entry("source memory: ",d2_src_w.c_str(),TDE_BEFORE,false,2,areg[PARAM_M]-2);
  trace_add_entry("predecrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
}
void d2_get_source_100_l(){ d2_src_l=EasyStr("-")+D2_BRACKETS_aM;
  trace_add_entry("source memory: ",d2_src_l.c_str(),TDE_BEFORE,false,2,areg[PARAM_M]-4);
  trace_add_entry("predecrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
}
void d2_get_source_101_b(){
  d2_src_b=d2_signed_offset((short)d2_fetchW())+D2_BRACKETS_aM;
  trace_add_entry("immediate offset: ","",TDE_BEFORE,false,2,dpc);
  trace_add_entry("source memory: ",d2_src_b.c_str(),TDE_BEFORE,false,1,areg[PARAM_M]+(signed short)d2_fetchW());
  dpc+=2;
}
void d2_get_source_101_w(){
  d2_src_w=d2_signed_offset((short)d2_fetchW())+D2_BRACKETS_aM;
  trace_add_entry("immediate offset: ","",TDE_BEFORE,false,2,dpc);
  trace_add_entry("source memory: ",d2_src_w.c_str(),TDE_BEFORE,false,2,areg[PARAM_M]+(signed short)d2_fetchW());
  dpc+=2;
}
void d2_get_source_101_l(){
  d2_src_l=d2_signed_offset((short)d2_fetchW())+D2_BRACKETS_aM;
  trace_add_entry("immediate offset: ","",TDE_BEFORE,false,2,dpc);
  trace_add_entry("source memory: ",d2_src_l.c_str(),TDE_BEFORE,false,4,areg[PARAM_M]+(signed short)d2_fetchW());
  dpc+=2;
}
void d2_get_source_110_b(){
  d2_ap=d2_fetchW();
  d2_src_b=D2_IRIWO;
  trace_add_entry("source address register: ",reg_name(PARAM_M+8),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
  trace_add_entry("source memory: ",d2_src_b.c_str(),TDE_BEFORE,false,1,d2_calc_iriwo(areg[PARAM_M]));
  dpc+=2;
}
void d2_get_source_110_w(){
  d2_ap=d2_fetchW();
  d2_src_w=D2_IRIWO;
  trace_add_entry("source address register: ",reg_name(PARAM_M+8),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
  trace_add_entry("source memory: ",d2_src_w.c_str(),TDE_BEFORE,false,2,d2_calc_iriwo(areg[PARAM_M]));
  dpc+=2;
}
void d2_get_source_110_l(){
  d2_ap=d2_fetchW();
  d2_src_l=D2_IRIWO;
  trace_add_entry("source address register: ",reg_name(PARAM_M+8),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
  trace_add_entry("source memory: ",d2_src_l.c_str(),TDE_BEFORE,false,4,d2_calc_iriwo(areg[PARAM_M]));
  dpc+=2;
}
void d2_get_source_111_b(){
  switch(ir&0x7){
  case 0:
    d2_src_b=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+".W";
    trace_add_entry("source memory: ",d2_src_b.c_str(),TDE_BEFORE,false,1,(MEM_ADDRESS)0xffffff&((signed long)((signed short)d2_fetchW())));
    dpc+=2;
    break;
  case 1:
    d2_src_b=EasyStr("$")+itoa(d2_fetchL(),d2_t_buf,16);
    trace_add_entry("source memory: ",d2_src_b.c_str(),TDE_BEFORE,false,1,(MEM_ADDRESS)0xffffff&(d2_fetchL()));
    dpc+=4;
    break;
  case 2:
    d2_src_b=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+"(pc)";
    d2_pc_rel_ex+=EasyStr(" {$")+HEXSl(dpc+(signed short)d2_fetchW(),6)+"}";
    trace_add_entry("source memory: ",d2_src_b.c_str(),TDE_BEFORE,false,1,D2_PC_RELATIVE_PC+(signed short)d2_fetchW());
    dpc+=2;
    break;
  case 3:
    d2_ap=d2_fetchW();
    d2_src_b=D2_IRIWO_PC;
    trace_add_entry("source memory: ",d2_src_b.c_str(),TDE_BEFORE,false,1,d2_calc_iriwo(D2_PC_RELATIVE_PC));
    dpc+=2;
    break;       //what do bits 8,9,a  of extra word do?  (not always 0)
  case 4:
//      ap=d2_fetchL();dpc+=4;
    d2_src_b=EasyStr("#$")+itoa(d2_fetchB(),d2_t_buf,16);
    trace_add_entry("source immediate: ",d2_src_b.c_str(),TDE_BEFORE,false,1,dpc+1);
    dpc+=2;
    break;
  default:
    d2_unrecognised();
/*    d2_command=EasyStr("dc.w $")+itoa(ir,d2_t_buf,16);
    d2_src_b="";
*/  break;
  }
}
void d2_get_source_111_w(){
  switch(ir&0x7){
  case 0:
    d2_src_w=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+".W";
    if(d2_trace){
      trace_add_entry("source memory: ",d2_src_w.c_str(),TDE_BEFORE,false,2,(MEM_ADDRESS)0xffffff&((signed long)((signed short)d2_fetchW())));
    }
    dpc+=2;
    break;
  case 1:
    d2_src_w=EasyStr("$")+itoa(d2_fetchL(),d2_t_buf,16);
    if(d2_trace){
      trace_add_entry("source memory: ",d2_src_w.c_str(),TDE_BEFORE,false,2,(MEM_ADDRESS)0xffffff&(d2_fetchL()));
    }
    dpc+=4;
    break;
  case 2:
    d2_src_w=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+"(pc)";
    d2_pc_rel_ex+=EasyStr(" {$")+HEXSl(dpc+(signed short)d2_fetchW(),6)+"}";
    trace_add_entry("source memory: ",d2_src_w.c_str(),TDE_BEFORE,false,2,D2_PC_RELATIVE_PC+(signed short)d2_fetchW());
    dpc+=2;
    break;
  case 3:
    d2_ap=d2_fetchW();
    d2_src_w=D2_IRIWO_PC;
    trace_add_entry("source memory: ",d2_src_w.c_str(),TDE_BEFORE,false,2,d2_calc_iriwo(D2_PC_RELATIVE_PC));
    dpc+=2;
    break;       //what do bits 8,9,a  of extra word do?  (not always 0)
  case 4:
//      ap=d2_fetchL();dpc+=4;
    d2_src_w=EasyStr("#$")+itoa(d2_fetchW(),d2_t_buf,16);
    trace_add_entry("source immediate: ",d2_src_w.c_str(),TDE_BEFORE,false,2,dpc);
    dpc+=2;
    break;
  default:
    d2_unrecognised();
    break;
  }
}
void d2_get_source_111_l(){
  switch(ir&0x7){
  case 0:
    d2_src_l=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+".W";
    if(d2_trace){
      trace_add_entry("source memory: ",d2_src_l.c_str(),TDE_BEFORE,false,4,(MEM_ADDRESS)0xffffff&((signed long)((signed short)d2_fetchW())));
    }
    dpc+=2;
    break;
  case 1:
    d2_src_l=EasyStr("$")+itoa(d2_fetchL(),d2_t_buf,16);
    if(d2_trace){
      trace_add_entry("source memory: ",d2_src_l.c_str(),TDE_BEFORE,false,4,(MEM_ADDRESS)0xffffff&(d2_fetchL()));
    }
    dpc+=4;
    break;
  case 2:
    d2_src_l=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+"(pc)";
    d2_pc_rel_ex+=EasyStr(" {$")+HEXSl(dpc+(signed short)d2_fetchW(),6)+"}";
    trace_add_entry("source memory: ",d2_src_w.c_str(),TDE_BEFORE,false,4,D2_PC_RELATIVE_PC+(signed short)d2_fetchW());
    dpc+=2;
    break;
  case 3:
    d2_ap=d2_fetchW();
    d2_src_l=D2_IRIWO_PC;
    trace_add_entry("source memory: ",d2_src_l.c_str(),TDE_BEFORE,false,2,d2_calc_iriwo(D2_PC_RELATIVE_PC));
    dpc+=2;
    break;       //what do bits 8,9,a  of extra word do?  (not always 0)
  case 4:
//      ap=d2_fetchL();dpc+=4;
    d2_src_l=EasyStr("#$")+itoa(d2_fetchL(),d2_t_buf,16);
    trace_add_entry("source immediate: ",d2_src_l.c_str(),TDE_BEFORE,false,4,dpc);
    dpc+=4;
    break;
  default:
    d2_unrecognised();
    break;
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


void d2_get_dest_000_b(){ d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,1,(MEM_ADDRESS)&r[PARAM_M]);
}
void d2_get_dest_000_w(){ d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_M]);
}
void d2_get_dest_000_l(){ d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_M]);
}
void d2_get_dest_001_b(){ d2_dest=D2_aM;
  trace_add_entry("dest register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
}
void d2_get_dest_001_w(){ d2_dest=D2_aM;
  trace_add_entry("dest register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
}
void d2_get_dest_001_l(){ d2_dest=D2_aM;
  trace_add_entry("dest register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
}
void d2_get_dest_010_b(){ d2_dest=D2_BRACKETS_aM;
  trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,areg[PARAM_M]);
}
void d2_get_dest_010_w(){ d2_dest=D2_BRACKETS_aM;
  trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,2,areg[PARAM_M]);
}
void d2_get_dest_010_l(){ d2_dest=D2_BRACKETS_aM;
  trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,4,areg[PARAM_M]);
}
void d2_get_dest_011_b(){ d2_dest=D2_BRACKETS_aM+"+";
  trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,areg[PARAM_M]);
  trace_add_entry("postincrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
}
void d2_get_dest_011_w(){ d2_dest=D2_BRACKETS_aM+"+";
  trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,2,areg[PARAM_M]);
  trace_add_entry("postincrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
}
void d2_get_dest_011_l(){ d2_dest=D2_BRACKETS_aM+"+";
  trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,4,areg[PARAM_M]);
  trace_add_entry("postincrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
}
void d2_get_dest_100_b(){ d2_dest=EasyStr("-")+D2_BRACKETS_aM;
  trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,areg[PARAM_M]-1);
  trace_add_entry("predecrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
}
void d2_get_dest_100_w(){ d2_dest=EasyStr("-")+D2_BRACKETS_aM;
  trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,2,areg[PARAM_M]-2);
  trace_add_entry("predecrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
}
void d2_get_dest_100_l(){ d2_dest=EasyStr("-")+D2_BRACKETS_aM;
  trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,4,areg[PARAM_M]-4);
  trace_add_entry("predecrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
}
void d2_get_dest_101_b(){
  d2_dest=d2_signed_offset((short)d2_fetchW())+D2_BRACKETS_aM;
  trace_add_entry("immediate offset: ","",TDE_BEFORE,false,2,dpc);
  trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,areg[PARAM_M]+(signed short)d2_fetchW());
  dpc+=2;
}
void d2_get_dest_101_w(){
  d2_dest=d2_signed_offset((short)d2_fetchW())+D2_BRACKETS_aM;
  trace_add_entry("immediate offset: ","",TDE_BEFORE,false,2,dpc);
  trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,2,areg[PARAM_M]+(signed short)d2_fetchW());
  dpc+=2;
}
void d2_get_dest_101_l(){
  d2_dest=d2_signed_offset((short)d2_fetchW())+D2_BRACKETS_aM;
  trace_add_entry("immediate offset: ","",TDE_BEFORE,false,2,dpc);
  trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,4,areg[PARAM_M]+(signed short)d2_fetchW());
  dpc+=2;
}
void d2_get_dest_110_b(){
  d2_ap=d2_fetchW();
  d2_dest=D2_IRIWO;
  trace_add_entry("dest base address register: ",reg_name(PARAM_M+8),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
  trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,d2_calc_iriwo(areg[PARAM_M]));
  dpc+=2;
}
void d2_get_dest_110_w(){
  d2_ap=d2_fetchW();
  d2_dest=D2_IRIWO;
  trace_add_entry("dest base address register: ",reg_name(PARAM_M+8),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
  trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,2,d2_calc_iriwo(areg[PARAM_M]));
  dpc+=2;
}
void d2_get_dest_110_l(){
  d2_ap=d2_fetchW();
  d2_dest=D2_IRIWO;
  trace_add_entry("dest base address register: ",reg_name(PARAM_M+8),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
  trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,4,d2_calc_iriwo(areg[PARAM_M]));
  dpc+=2;
}
void d2_get_dest_111_b(){
  switch(ir&0x7){
  case 0:
    d2_dest=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+".W";
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,(MEM_ADDRESS)0xffffff&((signed long)((signed short)d2_fetchW())));
    dpc+=2;
    break;
  case 1:
    d2_dest=EasyStr("$")+itoa(d2_fetchL(),d2_t_buf,16);
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,(MEM_ADDRESS)0xffffff&(d2_fetchL()));
    dpc+=4;
    break;
  default:
    d2_unrecognised();
    break;
  }
}
void d2_get_dest_111_w(){
  switch(ir&0x7){
  case 0:
    d2_dest=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+".W";
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,2,(MEM_ADDRESS)0xffffff&((signed long)((signed short)d2_fetchW())));
    dpc+=2;
    break;
  case 1:
    d2_dest=EasyStr("$")+itoa(d2_fetchL(),d2_t_buf,16);
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,2,(MEM_ADDRESS)0xffffff&(d2_fetchL()));
    dpc+=4;
    break;
  default:
    d2_unrecognised();
    break;
  }
}
void d2_get_dest_111_l(){
  switch(ir&0x7){
  case 0:
    d2_dest=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+".W";
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,4,(MEM_ADDRESS)0xffffff&((signed long)((signed short)d2_fetchW())));
    dpc+=2;
    break;
  case 1:
    d2_dest=EasyStr("$")+itoa(d2_fetchL(),d2_t_buf,16);
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,4,(MEM_ADDRESS)0xffffff&(d2_fetchL()));
    dpc+=4;
    break;
  default:
    d2_unrecognised();
    break;
  }
}
void d2_get_dest_111_b_with_sr(){
  switch(ir&0x7){
  case 0:
    d2_dest=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+".W";
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,(MEM_ADDRESS)0xffffff&((signed long)((signed short)d2_fetchW())));
    dpc+=2;
    break;
  case 1:
    d2_dest=EasyStr("$")+itoa(d2_fetchL(),d2_t_buf,16);
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,(MEM_ADDRESS)0xffffff&(d2_fetchL()));
    dpc+=4;
    break;
  case 4:
    d2_dest="ccr";
    break;
  default:
    d2_unrecognised();
    break;
  }
}
void d2_get_dest_111_w_with_sr(){
  switch(ir&0x7){
  case 0:
    d2_dest=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+".W";
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,2,(MEM_ADDRESS)0xffffff&((signed long)((signed short)d2_fetchW())));
    dpc+=2;
    break;
  case 1:
    d2_dest=EasyStr("$")+itoa(d2_fetchL(),d2_t_buf,16);
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,2,(MEM_ADDRESS)0xffffff&(d2_fetchL()));
    dpc+=4;
    break;
  case 4:
    d2_dest="sr";
    break;
  default:
    d2_unrecognised();
    break;
  }
}



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

void                              d2_ori_b(){
  d2_command="ori.b";
  D2_GET_IMMEDIATE_B;
  D2_GET_DEST_B_NOT_A_WITH_SR;
}void                             d2_ori_w(){
  d2_command="ori.w";
  D2_GET_IMMEDIATE_W;
  D2_GET_DEST_W_NOT_A_WITH_SR;
}void                             d2_ori_l(){
  d2_command="ori.l";
  D2_GET_IMMEDIATE_L;
  D2_GET_DEST_L_NOT_A_WITH_SR;
}
void                              d2_andi_b(){
  d2_command="andi.b";
  D2_GET_IMMEDIATE_B;
  D2_GET_DEST_B_NOT_A_WITH_SR;
}void                             d2_andi_w(){
  d2_command="andi.w";
  D2_GET_IMMEDIATE_W;
  D2_GET_DEST_W_NOT_A_WITH_SR;
}void                             d2_andi_l(){
  d2_command="andi.l";
  D2_GET_IMMEDIATE_L;
  D2_GET_DEST_L_NOT_A_WITH_SR;
}
void                              d2_subi_b(){
  d2_command="subi.b";
  D2_GET_IMMEDIATE_B;
  D2_GET_DEST_B_NOT_A;
}void                             d2_subi_w(){
  d2_command="subi.w";
  D2_GET_IMMEDIATE_W;
  D2_GET_DEST_W_NOT_A;
}void                             d2_subi_l(){
  d2_command="subi.l";
  D2_GET_IMMEDIATE_L;
  D2_GET_DEST_L_NOT_A;
}
void                              d2_addi_b(){
  d2_command="addi.b";
  D2_GET_IMMEDIATE_B;
  D2_GET_DEST_B_NOT_A;
}void                             d2_addi_w(){
  d2_command="addi.w";
  D2_GET_IMMEDIATE_W;
  D2_GET_DEST_W_NOT_A;
}void                             d2_addi_l(){
  d2_command="addi.l";
  D2_GET_IMMEDIATE_L;
  D2_GET_DEST_L_NOT_A;
}
void                              d2_btst(){
  d2_command="btst";
  D2_GET_IMMEDIATE_W;
  if((ir&BITS_543)==BITS_543_000){
    d2_dest=D2_dM;
    trace_add_entry("btst register: ",reg_name(PARAM_M),TDE_BEFORE,true,4,(MEM_ADDRESS)&r[PARAM_M]);
    //length = .l
  }else if((ir&BITS_543)==BITS_543_001){
    d2_unrecognised();
  }else{
    EasyStr a=d2_src_b;
    D2_GET_SOURCE_B;
    d2_dest=d2_src_b;
    d2_src_b=a;
  }
}void                             d2_bchg(){
  d2_command="bchg";
  D2_GET_IMMEDIATE_W;
  if((ir&BITS_543)==BITS_543_000){
    d2_dest=D2_dM;
    trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_M]);
    //length = .l
  }else{
    D2_GET_DEST_B_NOT_A;
  }
}void                             d2_bclr(){
  d2_command="bclr";
  D2_GET_IMMEDIATE_W;
  if((ir&BITS_543)==BITS_543_000){
    d2_dest=D2_dM;
    trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_M]);
    //length = .l
  }else{
    D2_GET_DEST_B_NOT_A;
  }
}void                             d2_bset(){
  d2_command="bset";
  D2_GET_IMMEDIATE_W;
  if((ir&BITS_543)==BITS_543_000){
    d2_dest=D2_dM;
    trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_M]);
    //length = .l
  }else{
    D2_GET_DEST_B_NOT_A;
  }
}
void                              d2_eori_b(){
  d2_command="eori.b";
  D2_GET_IMMEDIATE_B;
  D2_GET_DEST_B_NOT_A_WITH_SR;
}void                             d2_eori_w(){
  d2_command="eori.w";
  D2_GET_IMMEDIATE_W;
  D2_GET_DEST_W_NOT_A_WITH_SR;
}void                             d2_eori_l(){
  d2_command="eori.l";
  D2_GET_IMMEDIATE_L;
  D2_GET_DEST_L_NOT_A_WITH_SR;
}
void                              d2_cmpi_b(){
  d2_command="cmpi.b";
  D2_GET_IMMEDIATE_B;
  D2_GET_DEST_B_NOT_A_WITH_SR;
}void                             d2_cmpi_w(){
  d2_command="cmpi.w";
  D2_GET_IMMEDIATE_W;
  D2_GET_DEST_W_NOT_A_WITH_SR;
}void                             d2_cmpi_l(){
  d2_command="cmpi.l";
  D2_GET_IMMEDIATE_L;
  D2_GET_DEST_L_NOT_A_WITH_SR;
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


void                              d2_negx_b(){
  d2_command="negx.b";
  D2_GET_DEST_B_NOT_A;
}void                             d2_negx_w(){
  d2_command="negx.w";
  D2_GET_DEST_W_NOT_A;
}void                             d2_negx_l(){
  d2_command="negx.l";
  D2_GET_DEST_L_NOT_A;
}
void                              d2_clr_b(){
  d2_command="clr.b";
  D2_GET_DEST_B_NOT_A;
}void                             d2_clr_w(){
  d2_command="clr.w";
  D2_GET_DEST_W_NOT_A;
}void                             d2_clr_l(){
  d2_command="clr.l";
  D2_GET_DEST_L_NOT_A;
}
void                              d2_neg_b(){
  d2_command="neg.b";
  D2_GET_DEST_B_NOT_A;
}void                             d2_neg_w(){
  d2_command="neg.w";
  D2_GET_DEST_W_NOT_A;
}void                             d2_neg_l(){
  d2_command="neg.l";
  D2_GET_DEST_L_NOT_A;
}
void                              d2_not_b(){
  d2_command="not.b";
  D2_GET_DEST_B_NOT_A;
}void                             d2_not_w(){
  d2_command="not.w";
  D2_GET_DEST_W_NOT_A;
}void                             d2_not_l(){
  d2_command="not.l";
  D2_GET_DEST_L_NOT_A;
}
void                              d2_tst_b(){
  d2_command="tst.b";
  D2_GET_DEST_B_NOT_A;
}void                             d2_tst_w(){
  d2_command="tst.w";
  D2_GET_DEST_W_NOT_A;
}void                             d2_tst_l(){
  d2_command="tst.l";
  D2_GET_DEST_L_NOT_A;
}
void                              d2_tas(){
  if((ir&B6_111111)==B6_111100){
    d2_command="illegal";
  }else{
    d2_command="tas.b";
    D2_GET_DEST_B_NOT_A;
  }
}
void                              d2_move_from_sr(){
  d2_command="move";
  d2_src_w="sr";
  D2_GET_DEST_W_NOT_A;
}
void                              d2_move_from_ccr(){
  d2_command="move";
  d2_src_w="ccr";
  D2_GET_DEST_B_NOT_A;
}
void                              d2_move_to_ccr(){
  d2_command="move";
  d2_dest="ccr";
  if((ir&BITS_543)==BITS_543_001){
//source is address register
    d2_unrecognised();
  }else{
    D2_GET_SOURCE_B;
  }
}
void                              d2_move_to_sr(){
  d2_command="move";
  d2_dest="sr";
  if((ir&BITS_543)==BITS_543_001){
//source is address register
    d2_unrecognised();
  }else{
    D2_GET_SOURCE_W;
  }
}
void                              d2_nbcd(){
  d2_command="nbcd";
  D2_GET_DEST_B_NOT_A;
}
void                              d2_pea_or_swap(){
  switch(ir&BITS_543){
  case BITS_543_000:
    d2_command="swap";
    d2_dest=D2_dM;
    trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_M]);
    break;
  case BITS_543_001:case BITS_543_011:case BITS_543_100:
    d2_unrecognised();
  default:
    d2_command="pea";
    d2_src_w=d2_effective_address();
  }
}
void                              d2_movem_w_from_regs_or_ext_w(){
  switch(ir&BITS_543){
  case BITS_543_000:
    d2_command="ext.w";
    d2_dest=D2_dM;
    trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_M]);
    break;
  case BITS_543_010:
    d2_command="movem.w";
    d2_src_w=d2_get_movem_regs(false);
    d2_dest=D2_BRACKETS_aM;
    trace_add_movem_block("dest memory: ",PARAM_M,TDE_BEFORE|TDE_AFTER,2,areg[PARAM_M],d2_n_movem_regs);
    break;
  case BITS_543_100:
    d2_command="movem.w";
    d2_src_w=d2_get_movem_regs(true);
    d2_dest=EasyStr("-")+D2_BRACKETS_aM;
    trace_add_movem_block("dest memory: ",PARAM_M,TDE_BEFORE|TDE_AFTER,2,areg[PARAM_M]-2*d2_n_movem_regs,d2_n_movem_regs);
    trace_add_entry("predecrement address register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    break;
  case BITS_543_101:
    d2_command="movem.w";
    d2_src_w=d2_get_movem_regs(false);
    d2_dest=EasyStr("+$")+itoa(d2_fetchW(),d2_t_buf,16)+D2_BRACKETS_aM;
    trace_add_entry("immediate offset: ","",TDE_BEFORE,false,2,dpc);
    trace_add_movem_block("dest memory: ",PARAM_M,TDE_BEFORE|TDE_AFTER,2,areg[PARAM_M]+(signed short)d2_fetchW(),d2_n_movem_regs);
    dpc+=2;
    break;
  case BITS_543_110:
    d2_command="movem.w";
    d2_src_w=d2_get_movem_regs(false);
    d2_ap=d2_fetchW();
    d2_dest=D2_IRIWO;
    trace_add_entry("dest address register: ",reg_name(PARAM_M+8),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    trace_add_movem_block("dest memory: ",PARAM_M,TDE_BEFORE|TDE_AFTER,2,d2_calc_iriwo(areg[PARAM_M]),d2_n_movem_regs);
    dpc+=2;
    break;
  case BITS_543_111:
    d2_command="movem.w";
    d2_src_w=d2_get_movem_regs(false);
    switch(ir&0x7){
    case 0:
      d2_dest=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+".W";
      trace_add_movem_block("dest memory: ",-1,TDE_BEFORE|TDE_AFTER,2,0xffffff&(unsigned long)((signed long)((signed short)d2_fetchW())),d2_n_movem_regs);
      dpc+=2;
      break;
    case 1:
      d2_dest=EasyStr("$")+itoa(d2_fetchL(),d2_t_buf,16);
      trace_add_movem_block("dest memory: ",-1,TDE_BEFORE|TDE_AFTER,2,0xffffff&d2_fetchL(),d2_n_movem_regs);
      dpc+=4;
      break;
    default:
      d2_unrecognised();
      break;
    }
    break;
  default:
    d2_unrecognised();
  }
}
void                              d2_movem_l_from_regs_or_ext_l(){
  switch(ir&BITS_543){
  case BITS_543_000:
    d2_command="ext.l";
    d2_dest=D2_dM;
    trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_M]);
    break;
  case BITS_543_010:
    d2_command="movem.l";
    d2_src_l=d2_get_movem_regs(false);
    d2_dest=D2_BRACKETS_aM;
    trace_add_movem_block("dest memory: ",PARAM_M,TDE_BEFORE|TDE_AFTER,4,areg[PARAM_M],d2_n_movem_regs);
    break;
  case BITS_543_100:
    d2_command="movem.l";
    d2_src_l=d2_get_movem_regs(true);
    d2_dest=EasyStr("-")+D2_BRACKETS_aM;
    trace_add_movem_block("dest memory: ",PARAM_M,TDE_BEFORE|TDE_AFTER,4,areg[PARAM_M]-4*d2_n_movem_regs,d2_n_movem_regs);
    trace_add_entry("predecrement address register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    break;
  case BITS_543_101:
    d2_command="movem.l";
    d2_src_l=d2_get_movem_regs(false);
    d2_dest=EasyStr("+$")+itoa(d2_fetchW(),d2_t_buf,16)+D2_BRACKETS_aM;
    trace_add_entry("immediate offset: ","",TDE_BEFORE,false,2,dpc);
    trace_add_movem_block("dest memory: ",PARAM_M,TDE_BEFORE|TDE_AFTER,4,areg[PARAM_M]+(signed short)d2_fetchW(),d2_n_movem_regs);
    dpc+=2;
    break;
  case BITS_543_110:
    d2_command="movem.l";
    d2_src_l=d2_get_movem_regs(false);
    d2_ap=d2_fetchW();
    d2_dest=D2_IRIWO;
    trace_add_entry("dest address register: ",reg_name(PARAM_M+8),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    trace_add_movem_block("dest memory: ",PARAM_M,TDE_BEFORE|TDE_AFTER,4,d2_calc_iriwo(areg[PARAM_M]),d2_n_movem_regs);
    dpc+=2;
    break;
  case BITS_543_111:
    d2_command="movem.l";
    d2_src_l=d2_get_movem_regs(false);
    switch(ir&0x7){
    case 0:
      d2_dest=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+".W";
      trace_add_movem_block("dest memory: ",-1,TDE_BEFORE|TDE_AFTER,4,0xffffff&(unsigned long)((signed long)((signed short)d2_fetchW())),d2_n_movem_regs);
      dpc+=2;
      break;
    case 1:
      d2_dest=EasyStr("$")+itoa(d2_fetchL(),d2_t_buf,16);
      trace_add_movem_block("dest memory: ",-1,TDE_BEFORE|TDE_AFTER,4,0xffffff&d2_fetchL(),d2_n_movem_regs);
      dpc+=4;
      break;
    default:
      d2_unrecognised();
      break;
    }
    break;
  default:
    d2_unrecognised();
  }
}
void                              d2_movem_l_to_regs(){
  switch(ir&BITS_543){
  case BITS_543_010:
    d2_command="movem.l";
    d2_src_l=D2_BRACKETS_aM;
    d2_dest=d2_get_movem_regs(false);
    trace_add_movem_block("source memory: ",PARAM_M,TDE_BEFORE,4,areg[PARAM_M],d2_n_movem_regs);
    break;
  case BITS_543_011:
    d2_command="movem.l";
    d2_src_l=D2_BRACKETS_aM+"+";
    d2_dest=d2_get_movem_regs(false);
    trace_add_movem_block("source memory: ",PARAM_M,TDE_BEFORE|TDE_AFTER,4,areg[PARAM_M],d2_n_movem_regs);
    trace_add_entry("postincrement address register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    break;
  case BITS_543_101:
    d2_command="movem.l";
    d2_dest=d2_get_movem_regs(false);
    d2_src_l=EasyStr("+$")+itoa(d2_fetchW(),d2_t_buf,16)+D2_BRACKETS_aM;
    trace_add_entry("immediate offset: ","",TDE_BEFORE,false,2,dpc);
    trace_add_movem_block("source memory: ",PARAM_M,TDE_BEFORE,4,areg[PARAM_M]+(signed short)d2_fetchW(),d2_n_movem_regs);
    dpc+=2;
    break;
  case BITS_543_110:
    d2_command="movem.l";
    d2_dest=d2_get_movem_regs(false);
    d2_ap=d2_fetchW();
    d2_src_l=D2_IRIWO;
    trace_add_entry("source address register: ",reg_name(PARAM_M+8),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    trace_add_movem_block("source memory: ",PARAM_M,TDE_BEFORE,4,d2_calc_iriwo(areg[PARAM_M]),d2_n_movem_regs);
    dpc+=2;
    break;
  case BITS_543_111:
    d2_command="movem.l";
    d2_dest=d2_get_movem_regs(false);
    switch(ir&0x7){
    case 0:
      d2_src_l=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+".W";
      trace_add_movem_block("source memory: ",-1,TDE_BEFORE,4,0xffffff&(unsigned long)((signed long)((signed short)d2_fetchW())),d2_n_movem_regs);
      dpc+=2;
      break;
    case 1:
      d2_src_l=EasyStr("$")+itoa(d2_fetchL(),d2_t_buf,16);
      trace_add_movem_block("source memory: ",-1,TDE_BEFORE,4,0xffffff&d2_fetchL(),d2_n_movem_regs);
      dpc+=4;
      break;
    case 2:
      d2_src_l=EasyStr("+$")+itoa(d2_fetchW(),d2_t_buf,16)+"(pc)";
      d2_pc_rel_ex+=EasyStr(" {$")+HEXSl(dpc+(signed short)d2_fetchW(),6)+"}";
//////////********************************** where is the pc-relative from????? ***************
      trace_add_movem_block("source memory : ",-1,TDE_BEFORE,4,D2_PC_RELATIVE_PC+(signed short)d2_fetchW(),d2_n_movem_regs);
      dpc+=2;
      break;
    case 3:
      d2_ap=d2_fetchW();
      d2_src_l=D2_IRIWO_PC;
      trace_add_movem_block("source memory : ",-1,TDE_BEFORE,4,d2_calc_iriwo(D2_PC_RELATIVE_PC),d2_n_movem_regs);
      dpc+=2;
      break;
    default:
      d2_unrecognised();
      break;
    }
    break;
  default:
    d2_unrecognised();
  }
}
void                              d2_movem_w_to_regs(){
  switch(ir&BITS_543){
  case BITS_543_010:
    d2_command="movem.w";
    d2_src_w=D2_BRACKETS_aM;
    d2_dest=d2_get_movem_regs(false);
    trace_add_movem_block("source memory: ",PARAM_M,TDE_BEFORE,2,areg[PARAM_M],d2_n_movem_regs);
    break;
  case BITS_543_011:
    d2_command="movem.w";
    d2_src_w=D2_BRACKETS_aM+"+";
    d2_dest=d2_get_movem_regs(false);
    trace_add_movem_block("source memory: ",PARAM_M,TDE_BEFORE|TDE_AFTER,2,areg[PARAM_M],d2_n_movem_regs);
    trace_add_entry("postincrement address register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    break;
  case BITS_543_101:
    d2_command="movem.w";
    d2_dest=d2_get_movem_regs(false);
    d2_src_w=EasyStr("+$")+itoa(d2_fetchW(),d2_t_buf,16)+D2_BRACKETS_aM;
    trace_add_entry("immediate offset: ","",TDE_BEFORE,false,2,dpc);
    trace_add_movem_block("source memory: ",PARAM_M,TDE_BEFORE,2,areg[PARAM_M]+(signed short)d2_fetchW(),d2_n_movem_regs);
    dpc+=2;
    break;
  case BITS_543_110:
    d2_command="movem.w";
    d2_dest=d2_get_movem_regs(false);
    d2_ap=d2_fetchW();
    d2_src_w=D2_IRIWO;
    trace_add_entry("source address register: ",reg_name(PARAM_M+8),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    trace_add_movem_block("source memory: ",PARAM_M,TDE_BEFORE,2,d2_calc_iriwo(areg[PARAM_M]),d2_n_movem_regs);
    dpc+=2;
    break;
  case BITS_543_111:
    d2_command="movem.w";
    d2_dest=d2_get_movem_regs(false);
    switch(ir&0x7){
    case 0:
      d2_src_w=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+".W";
      trace_add_movem_block("source memory: ",-1,TDE_BEFORE,2,0xffffff&(unsigned long)((signed long)((signed short)d2_fetchW())),d2_n_movem_regs);
      dpc+=2;
      break;
    case 1:
      d2_src_w=EasyStr("$")+itoa(d2_fetchL(),d2_t_buf,16);
      trace_add_movem_block("source memory: ",-1,TDE_BEFORE,2,0xffffff&d2_fetchL(),d2_n_movem_regs);
      dpc+=4;
      break;
    case 2:
      d2_src_w=EasyStr("+$")+itoa(d2_fetchW(),d2_t_buf,16)+"(pc)";
      d2_pc_rel_ex+=EasyStr(" {$")+HEXSl(dpc+(signed short)d2_fetchW(),6)+"}";
      trace_add_movem_block("source memory : ",-1,TDE_BEFORE,2,D2_PC_RELATIVE_PC+(signed short)d2_fetchW(),d2_n_movem_regs);
      dpc+=2;
      break;
    case 3:
      d2_ap=d2_fetchW();
      d2_src_w=D2_IRIWO_PC;
      trace_add_movem_block("source memory : ",-1,TDE_BEFORE,2,d2_calc_iriwo(D2_PC_RELATIVE_PC),d2_n_movem_regs);
      dpc+=2;
      break;
    default:
      d2_unrecognised();
      break;
    }
    break;
  default:
    d2_unrecognised();
  }
}
void                              d2_jsr(){
  switch(ir&BITS_543){
  case BITS_543_000:case BITS_543_001:case BITS_543_011:case BITS_543_100:
    d2_unrecognised();
  default:
    d2_command="jsr";
    trace_add_entry("program counter: ","pc",TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&pc);
    d2_src_w=d2_effective_address();
  }
}
void                              d2_jmp(){
  switch(ir&BITS_543){
  case BITS_543_000:case BITS_543_001:case BITS_543_011:case BITS_543_100:
    d2_unrecognised();
  default:
    d2_command="jmp";
    trace_add_entry("program counter: ","pc",TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&pc);
    d2_src_w=d2_effective_address();
  }
}
void                              d2_chk(){
  d2_command="chk";
  if((ir&BITS_543)==BITS_543_001){
    d2_unrecognised();
  }else{
    D2_GET_SOURCE_W;
    d2_dest=D2_dN;
    trace_add_entry("dest data register: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&(r[PARAM_M]));
  }
}
void                              d2_lea(){
  switch(ir&BITS_543){
  case BITS_543_000:case BITS_543_001:case BITS_543_011:case BITS_543_100:
    d2_unrecognised();
  default:
    d2_command="lea";
    d2_dest=D2_aN;
    trace_add_entry("dest address register: ",reg_name(PARAM_N+8),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    d2_src_w=d2_effective_address();
  }
}
void                              d2_line_4_stuff(){
  d2_jump_line_4_stuff[ir&(BITS_543|0x7)]();
}
void                              d2_trap(){
  d2_command="trap";
  d2_src=EasyStr("#")+itoa((ir&0xf),d2_t_buf,10);
}
void                              d2_link(){
  d2_command="link";
  d2_src_l=D2_aM;
  trace_add_entry("predecrement stack pointer: ","sp=a7",TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[15]);
  trace_add_entry("link address register: ",reg_name(PARAM_M+8),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
  d2_dest=EasyStr("#")+itoa((signed short)d2_fetchW(),d2_t_buf,10);
  trace_add_entry("immediate offset: ","",TDE_BEFORE,false,2,dpc);
  dpc+=2;
}
void                              d2_unlk(){
  d2_command="unlk";
  d2_dest=D2_aM;
  trace_add_entry("stack pointer: ","sp=a7",TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[15]);
  trace_add_entry("link address register: ",reg_name(PARAM_M+8),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);

}
void                              d2_move_to_usp(){
  d2_command="move";
  d2_src_l=D2_aM;
  trace_add_entry("source register: ",reg_name(8+PARAM_M),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
  d2_dest="usp";
  trace_add_entry("dest: ","usp",TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)lpUSP);
}
void                              d2_move_from_usp(){
  d2_command="move";
  d2_src_l="usp";
  trace_add_entry("source: ","usp",TDE_BEFORE,true,4,(MEM_ADDRESS)lpUSP);
  d2_dest=D2_aM;
  trace_add_entry("dest register: ",reg_name(8+PARAM_M),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
}
void                              d2_reset(){
  d2_command="reset";
}
void                              d2_nop(){
  d2_command="nop";
}
void                              d2_stop(){
  d2_command="stop";
  d2_src_w=EasyStr("#$")+itoa(d2_fetchW(),d2_t_buf,16);
  trace_add_entry("immediate source: ","",TDE_BEFORE,false,2,dpc);
  dpc+=2;
}
void                              d2_rte(){
  d2_command="rte";
  trace_add_entry("program counter: ","pc",TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&pc);
}
void                              d2_rtd(){
  d2_command="rtd";
  d2_src_w=EasyStr("#$")+itoa(d2_fetchW(),d2_t_buf,16);
  trace_add_entry("immediate source: ","",TDE_BEFORE,false,2,dpc);
  trace_add_entry("program counter: ","pc",TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&pc);
  dpc+=2;
}
void                              d2_rts(){
  d2_command="rts";
  trace_add_entry("program counter: ","pc",TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&pc);
}
void                              d2_trapv(){
  d2_command="trapv";
}
void                              d2_rtr(){
  d2_command="rtr";
  trace_add_entry("program counter: ","pc",TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&pc);
}
void                              d2_movec(){
  EasyStr dtt;
  d2_command="movec";
  d2_ap=d2_fetchW();dpc+=2;
  switch(d2_ap&0xfff){
  case 0:
    dtt="sfc";break;
  case 1:
    dtt="dfc";break;
  case 2:
    dtt="cacr";break;
  default:
    dtt="???";
  }
  if((ir&0x7)==2){
    d2_src_w=dtt;d2_dest=EasyStr("r")+itoa(((d2_ap&0xf000)>>12),d2_t_buf,10);
  }else{
    d2_dest=dtt;d2_src_w=EasyStr("r")+itoa(((d2_ap&0xf000)>>12),d2_t_buf,10);
  }
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



void                              d2_addq_b(){
  d2_command="addq.b";
  d2_src_b=EasyStr("#")+STRS(int(PARAM_N ? PARAM_N:8));
  D2_GET_DEST_B;
}void                             d2_addq_w(){
  d2_command="addq.w";
  d2_src_w=EasyStr("#")+STRS(int(PARAM_N ? PARAM_N:8));
  D2_GET_DEST_W;
}void                             d2_addq_l(){
  d2_command="addq.l";
  d2_src_l=EasyStr("#")+STRS(int(PARAM_N ? PARAM_N:8));
  D2_GET_DEST_L;
}
void                              d2_subq_b(){
  d2_command="subq.b";
  d2_src_b=EasyStr("#")+STRS(int(PARAM_N ? PARAM_N:8));
  D2_GET_DEST_B;
}void                             d2_subq_w(){
  d2_command="subq.w";
  d2_src_w=EasyStr("#")+STRS(int(PARAM_N ? PARAM_N:8));
  D2_GET_DEST_W;
}void                             d2_subq_l(){
  d2_command="subq.l";
  d2_src_l=EasyStr("#")+STRS(int(PARAM_N ? PARAM_N:8));
  D2_GET_DEST_L;
}
void                              d2_dbCC_or_sCC(){
  if((ir&BITS_543)==BITS_543_001){
    d2_command="db";
    d2_command+=("t\0\0ra\0hi\0ls\0cc\0cs\0ne\0eq\0vc\0vs\0pl\0mi\0ge\0lt\0gt\0le\0"
                     +3*((ir&0xf00)>>8));
    d2_src_w=D2_dM;
    trace_add_entry("counter register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_M]);
    d2_ap=d2_fetchW();
    if (d2_ap & 0x8000){
      d2_dest=EasyStr("-")+STRS(0x10000-d2_ap);
      d2_ap=WORD(-(0x10000-d2_ap));
    }else{
      d2_dest=EasyStr("+")+STRS(d2_ap);
    }
    trace_add_entry("immediate offset: ",d2_dest.c_str(),TDE_BEFORE,false,2,dpc);
    d2_dest+=Str(" {$")+HEXSl(dpc+(signed short)d2_ap,6)+"}";
    dpc+=2;
  }else{
    d2_command="s  ";
    memcpy(d2_command.Text+1,("t  f  hi ls cc cs ne eq vc vs pl mi ge lt gt le "
                               +3*((ir&0xf00)>>8)),2);
    D2_GET_DEST_B;
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

void                              d2_or_b_to_dN(){
  d2_command="or.b";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,1,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_B;
}void                             d2_or_w_to_dN(){
  d2_command="or.w";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_W;
}void                             d2_or_l_to_dN(){
  d2_command="or.l";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_L;
}
void                              d2_divu(){
  d2_command="divu";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_W;
}
void                              d2_or_b_from_dN_or_sbcd(){
  switch(ir&BITS_543){
  case BITS_543_000:
    d2_command="sbcd";
    d2_src_b=D2_dM;
    trace_add_entry("source register: ",reg_name(PARAM_M),TDE_BEFORE,true,1,(MEM_ADDRESS)&r[PARAM_M]);
    d2_dest=D2_dN;
    trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,1,(MEM_ADDRESS)&r[PARAM_N]);
    break;
  case BITS_543_001:
    d2_command="sbcd";
    d2_src_b=EasyStr("-")+D2_BRACKETS_aM;
    trace_add_entry("source memory: ",d2_src_b.c_str(),TDE_BEFORE,false,1,areg[PARAM_M]-1);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    d2_dest=EasyStr("-")+D2_BRACKETS_aN;
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,areg[PARAM_N]-1);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    break;
  default:
    d2_command="or.b";
    d2_src_b=D2_dN;
    trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,1,(MEM_ADDRESS)&r[PARAM_N]);
    D2_GET_DEST_B;
  }
}void                             d2_or_w_from_dN(){
  switch(ir&BITS_543){
  case BITS_543_000:
  case BITS_543_001:
    d2_unrecognised();
    break;
  default:
    d2_command="or.w";
    d2_src_w=D2_dN;
    trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,2,(MEM_ADDRESS)&r[PARAM_N]);
    D2_GET_DEST_W;
  }
}void                             d2_or_l_from_dN(){
  switch(ir&BITS_543){
  case BITS_543_000:
  case BITS_543_001:
    d2_unrecognised();
    break;
  default:
    d2_command="or.l";
    d2_src_l=D2_dN;
    trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,4,(MEM_ADDRESS)&r[PARAM_N]);
    D2_GET_DEST_L;
  }
}
void                              d2_divs(){
  d2_command="divs";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_W;
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

void                              d2_sub_b_to_dN(){
  d2_command="sub.b";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,1,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_B;
}void                             d2_sub_w_to_dN(){
  d2_command="sub.w";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_W;
}void                             d2_sub_l_to_dN(){
  d2_command="sub.l";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_L;
}void                             d2_suba_w(){
  d2_command="suba.w";
  d2_dest=D2_aN;
  trace_add_entry("dest register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
  D2_GET_SOURCE_W;
}
void                              d2_sub_b_from_dN(){
  switch(ir&BITS_543){
  case BITS_543_000:
    d2_command="subx.b";
    d2_src_b=D2_dM;
    trace_add_entry("source register: ",reg_name(PARAM_M),TDE_BEFORE,true,1,(MEM_ADDRESS)&r[PARAM_M]);
    d2_dest=D2_dN;
    trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,1,(MEM_ADDRESS)&r[PARAM_N]);
    break;
  case BITS_543_001:
    d2_command="subx.b";
    d2_src_b=EasyStr("-")+D2_BRACKETS_aM;
    trace_add_entry("source memory: ",d2_src_b.c_str(),TDE_BEFORE,false,1,areg[PARAM_M]-1);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    d2_dest=EasyStr("-")+D2_BRACKETS_aN;
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,areg[PARAM_N]-1);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    break;
  default:
    d2_command="sub.b";
    d2_src_b=D2_dN;
    trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,1,(MEM_ADDRESS)&r[PARAM_N]);
    D2_GET_DEST_B;
  }
}void                              d2_sub_w_from_dN(){
  switch(ir&BITS_543){
  case BITS_543_000:
    d2_command="subx.w";
    d2_src_w=D2_dM;
    trace_add_entry("source register: ",reg_name(PARAM_M),TDE_BEFORE,true,2,(MEM_ADDRESS)&r[PARAM_M]);
    d2_dest=D2_dN;
    trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_N]);
    break;
  case BITS_543_001:
    d2_command="subx.w";
    d2_src_w=EasyStr("-")+D2_BRACKETS_aM;
    trace_add_entry("source memory: ",d2_src_b.c_str(),TDE_BEFORE,false,2,areg[PARAM_M]-2);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    d2_dest=EasyStr("-")+D2_BRACKETS_aN;
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,2,areg[PARAM_N]-2);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    break;
  default:
    d2_command="sub.w";
    d2_src_w=D2_dN;
    trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,2,(MEM_ADDRESS)&r[PARAM_N]);
    D2_GET_DEST_W;
  }
}void                              d2_sub_l_from_dN(){
  switch(ir&BITS_543){
  case BITS_543_000:
    d2_command="subx.l";
    d2_src_l=D2_dM;
    trace_add_entry("source register: ",reg_name(PARAM_M),TDE_BEFORE,true,4,(MEM_ADDRESS)&r[PARAM_M]);
    d2_dest=D2_dN;
    trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_N]);
    break;
  case BITS_543_001:
    d2_command="subx.l";
    d2_src_l=EasyStr("-")+D2_BRACKETS_aM;
    trace_add_entry("source memory: ",d2_src_b.c_str(),TDE_BEFORE,false,4,areg[PARAM_M]-4);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    d2_dest=EasyStr("-")+D2_BRACKETS_aN;
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,4,areg[PARAM_N]-4);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    break;
  default:
    d2_command="sub.l";
    d2_src_l=D2_dN;
    trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,4,(MEM_ADDRESS)&r[PARAM_N]);
    D2_GET_DEST_L;
  }
}void                             d2_suba_l(){
  d2_command="suba.l";
  d2_dest=D2_aN;
  trace_add_entry("dest register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
  D2_GET_SOURCE_L;
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

void                              d2_cmp_b(){
  d2_command="cmp.b";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE,true,1,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_B;
}void                             d2_cmp_w(){
  d2_command="cmp.w";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE,true,2,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_W;
}void                             d2_cmp_l(){
  d2_command="cmp.l";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE,true,4,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_L;
}void                             d2_cmpa_w(){
  d2_command="cmpa.w";
  d2_dest=D2_aN;
  trace_add_entry("dest register: ",reg_name(8+PARAM_N),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
  D2_GET_SOURCE_W;
}
void                              d2_eor_b(){
  if((ir&BITS_543)==BITS_543_001){
    d2_command="cmpm.b";
    d2_src_b=D2_BRACKETS_aM+"+";
    trace_add_entry("source memory: ",d2_src_b.c_str(),TDE_BEFORE,false,1,areg[PARAM_M]);
    trace_add_entry("postdecrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    d2_dest=D2_BRACKETS_aN+"+";
    trace_add_entry("dest memory: ",d2_src_b.c_str(),TDE_BEFORE,false,1,areg[PARAM_N]);
    trace_add_entry("postdecrement register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
  }else{
    d2_command="eor.b";
    d2_src_b=D2_dN;
    trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,1,(MEM_ADDRESS)&r[PARAM_N]);
    D2_GET_DEST_B;
  }
}void                             d2_eor_w(){
  if((ir&BITS_543)==BITS_543_001){
    d2_command="cmpm.w";
    d2_src_w=D2_BRACKETS_aM+"+";
    trace_add_entry("source memory: ",d2_src_w.c_str(),TDE_BEFORE,false,2,areg[PARAM_M]);
    trace_add_entry("postdecrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    d2_dest=D2_BRACKETS_aN+"+";
    trace_add_entry("dest memory: ",d2_src_w.c_str(),TDE_BEFORE,false,2,areg[PARAM_M]);
    trace_add_entry("postdecrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
  }else{
    d2_command="eor.w";
    d2_src_w=D2_dN;
    trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,2,(MEM_ADDRESS)&r[PARAM_N]);
    D2_GET_DEST_W;
  }
}void                             d2_eor_l(){
  if((ir&BITS_543)==BITS_543_001){
    d2_command="cmpm.l";
    d2_src_l=D2_BRACKETS_aM+"+";
    trace_add_entry("source memory: ",d2_src_l.c_str(),TDE_BEFORE,false,4,areg[PARAM_M]);
    trace_add_entry("postdecrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    d2_dest=D2_BRACKETS_aN+"+";
    trace_add_entry("dest memory: ",d2_src_l.c_str(),TDE_BEFORE,false,4,areg[PARAM_N]);
    trace_add_entry("postdecrement register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
  }else{
    d2_command="eor.l";
    d2_src_l=D2_dN;
    trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,4,(MEM_ADDRESS)&r[PARAM_N]);
    D2_GET_DEST_L;
  }
}
void                             d2_cmpa_l(){
  d2_command="cmpa.l";
  d2_dest=D2_aN;
  trace_add_entry("dest register: ",reg_name(8+PARAM_N),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
  D2_GET_SOURCE_L;
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

void                              d2_and_b_to_dN(){
  d2_command="and.b";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,1,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_B;
}void                             d2_and_w_to_dN(){
  d2_command="and.w";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_W;
}void                             d2_and_l_to_dN(){
  d2_command="and.l";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_L;
}
void                              d2_mulu(){
  d2_command="mulu";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_W;
}
void                              d2_and_b_from_dN_or_abcd(){
  switch(ir&BITS_543){
  case BITS_543_000:
    d2_command="abcd";
    d2_src_b=D2_dM;
    trace_add_entry("source register: ",reg_name(PARAM_M),TDE_BEFORE,true,1,(MEM_ADDRESS)&r[PARAM_M]);
    d2_dest=D2_dN;
    trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,1,(MEM_ADDRESS)&r[PARAM_N]);
    break;
  case BITS_543_001:
    d2_command="abcd";
    d2_src_b=EasyStr("-")+D2_BRACKETS_aM;
    trace_add_entry("source memory: ",d2_src_b.c_str(),TDE_BEFORE,false,1,areg[PARAM_M]-1);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    d2_dest=EasyStr("-")+D2_BRACKETS_aN;
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,areg[PARAM_N]-1);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    break;
  default:
    d2_command="and.b";
    d2_src_b=D2_dN;
    trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,1,(MEM_ADDRESS)&r[PARAM_N]);
    D2_GET_DEST_B;
  }
}void                             d2_and_w_from_dN_or_exg_like(){
  switch(ir&BITS_543){
  case BITS_543_000:
    d2_command="exg";
    d2_src_l=D2_dN;
    trace_add_entry("exg register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_N]);
    d2_dest=D2_dM;
    trace_add_entry("exg register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_M]);
    break;
  case BITS_543_001:
    d2_command="exg";
    d2_src_l=D2_aN;
    trace_add_entry("exg register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    d2_dest=D2_aM;
    trace_add_entry("exg register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    break;
  default:
    d2_command="and.w";
    d2_src_w=D2_dN;
    trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,2,(MEM_ADDRESS)&r[PARAM_N]);
    D2_GET_DEST_W;
  }
}void                             d2_and_l_from_dN_or_exg_unlike(){
  switch(ir&BITS_543){
  case BITS_543_000:
/*
    d2_command="exg";
    d2_src_l=D2_aN;
    trace_add_entry("exg register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    d2_dest=D2_dM;
    trace_add_entry("exg register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_M]);
*/
    d2_unrecognised();
    break;
  case BITS_543_001:
    d2_command="exg";
    d2_src_l=D2_dN;
    trace_add_entry("exg register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_N]);
    d2_dest=D2_aM;
    trace_add_entry("exg register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    break;
  default:
    d2_command="and.l";
    d2_src_l=D2_dN;
    trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,4,(MEM_ADDRESS)&r[PARAM_N]);
    D2_GET_DEST_L;
  }
}
void                              d2_muls(){
  d2_command="muls";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_W;
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

void                              d2_add_b_to_dN(){
  d2_command="add.b";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,1,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_B;
}void                             d2_add_w_to_dN(){
  d2_command="add.w";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_W;
}void                             d2_add_l_to_dN(){
  d2_command="add.l";
  d2_dest=D2_dN;
  trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_N]);
  D2_GET_SOURCE_L;
}void                             d2_adda_w(){
  d2_command="adda.w";
  d2_dest=D2_aN;
  trace_add_entry("dest register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
  D2_GET_SOURCE_W;
}
void                              d2_add_b_from_dN(){
  switch(ir&BITS_543){
  case BITS_543_000:
    d2_command="addx.b";
    d2_src_b=D2_dM;
    trace_add_entry("source register: ",reg_name(PARAM_M),TDE_BEFORE,true,1,(MEM_ADDRESS)&r[PARAM_M]);
    d2_dest=D2_dN;
    trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,1,(MEM_ADDRESS)&r[PARAM_N]);
    break;
  case BITS_543_001:
    d2_command="addx.b";
    d2_src_b=EasyStr("-")+D2_BRACKETS_aM;
    trace_add_entry("source memory: ",d2_src_b.c_str(),TDE_BEFORE,false,1,areg[PARAM_M]-1);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    d2_dest=EasyStr("-")+D2_BRACKETS_aN;
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,areg[PARAM_N]-1);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    break;
  default:
    d2_command="add.b";
    d2_src_b=D2_dN;
    trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,1,(MEM_ADDRESS)&r[PARAM_N]);
    D2_GET_DEST_B;
  }
}void                              d2_add_w_from_dN(){
  switch(ir&BITS_543){
  case BITS_543_000:
    d2_command="addx.w";
    d2_src_w=D2_dM;
    trace_add_entry("source register: ",reg_name(PARAM_M),TDE_BEFORE,true,2,(MEM_ADDRESS)&r[PARAM_M]);
    d2_dest=D2_dN;
    trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_N]);
    break;
  case BITS_543_001:
    d2_command="addx.w";
    d2_src_w=EasyStr("-")+D2_BRACKETS_aM;
    trace_add_entry("source memory: ",d2_src_b.c_str(),TDE_BEFORE,false,2,areg[PARAM_M]-2);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    d2_dest=EasyStr("-")+D2_BRACKETS_aN;
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,2,areg[PARAM_N]-2);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    break;
  default:
    d2_command="add.w";
    d2_src_w=D2_dN;
    trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,2,(MEM_ADDRESS)&r[PARAM_N]);
    D2_GET_DEST_W;
  }
}void                              d2_add_l_from_dN(){
  switch(ir&BITS_543){
  case BITS_543_000:
    d2_command="addx.l";
    d2_src_l=D2_dM;
    trace_add_entry("source register: ",reg_name(PARAM_M),TDE_BEFORE,true,4,(MEM_ADDRESS)&r[PARAM_M]);
    d2_dest=D2_dN;
    trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_N]);
    break;
  case BITS_543_001:
    d2_command="addx.l";
    d2_src_l=EasyStr("-")+D2_BRACKETS_aM;
    trace_add_entry("source memory: ",d2_src_b.c_str(),TDE_BEFORE,false,4,areg[PARAM_M]-4);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_M]);
    d2_dest=EasyStr("-")+D2_BRACKETS_aN;
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,4,areg[PARAM_N]-4);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    break;
  default:
    d2_command="add.l";
    d2_src_l=D2_dN;
    trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,4,(MEM_ADDRESS)&r[PARAM_N]);
    D2_GET_DEST_L;
  }
}void                             d2_adda_l(){
  d2_command="adda.l";
  d2_dest=D2_aN;
  trace_add_entry("dest register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
  D2_GET_SOURCE_L;
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


void                              d2_asr_b_to_dM(){
  d2_command="asr.b";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,1,(MEM_ADDRESS)&r[PARAM_M]);
}void                             d2_lsr_b_to_dM(){
  d2_command="lsr.b";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,1,(MEM_ADDRESS)&r[PARAM_M]);
}void                             d2_roxr_b_to_dM(){
  d2_command="roxr.b";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,1,(MEM_ADDRESS)&r[PARAM_M]);
}void                             d2_ror_b_to_dM(){
  d2_command="ror.b";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,1,(MEM_ADDRESS)&r[PARAM_M]);
}
void                              d2_asr_w_to_dM(){
  d2_command="asr.w";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_M]);
}void                             d2_lsr_w_to_dM(){
  d2_command="lsr.w";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_M]);
}void                             d2_roxr_w_to_dM(){
  d2_command="roxr.w";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_M]);
}void                             d2_ror_w_to_dM(){
  d2_command="ror.w";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_M]);
}
void                              d2_asr_l_to_dM(){
  d2_command="asr.l";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_M]);
}void                             d2_lsr_l_to_dM(){
  d2_command="lsr.l";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_M]);
}void                             d2_roxr_l_to_dM(){
  d2_command="roxr.l";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_M]);
}void                             d2_ror_l_to_dM(){
  d2_command="ror.l";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_M]);
}
void                              d2_asl_b_to_dM(){
  d2_command="asl.b";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,1,(MEM_ADDRESS)&r[PARAM_M]);
}void                             d2_lsl_b_to_dM(){
  d2_command="lsl.b";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,1,(MEM_ADDRESS)&r[PARAM_M]);
}void                             d2_roxl_b_to_dM(){
  d2_command="roxl.b";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,1,(MEM_ADDRESS)&r[PARAM_M]);
}void                             d2_rol_b_to_dM(){
  d2_command="rol.b";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,1,(MEM_ADDRESS)&r[PARAM_M]);
}
void                              d2_asl_w_to_dM(){
  d2_command="asl.w";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_M]);
}void                             d2_lsl_w_to_dM(){
  d2_command="lsl.w";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_M]);
}void                             d2_roxl_w_to_dM(){
  d2_command="roxl.w";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_M]);
}void                             d2_rol_w_to_dM(){
  d2_command="rol.w";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_M]);
}
void                              d2_asl_l_to_dM(){
  d2_command="asl.l";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_M]);
}void                             d2_lsl_l_to_dM(){
  d2_command="lsl.l";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_M]);
}void                             d2_roxl_l_to_dM(){
  d2_command="roxl.l";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_M]);
}void                             d2_rol_l_to_dM(){
  d2_command="rol.l";
  D2_BIT_SHIFT_TO_dM_GET_SOURCE;
  d2_dest=D2_dM;
  trace_add_entry("dest register: ",reg_name(PARAM_M),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_M]);
}
void                              d2_bit_shift_right_to_mem(){
  D2_GET_DEST_W_NOT_A_OR_D;
  switch(ir&BITS_ba9){
  case BITS_ba9_000:
    d2_command="asr";
    break;
  case BITS_ba9_001:
    d2_command="lsr";
    break;
  case BITS_ba9_010:
    d2_command="roxr";
    break;
  case BITS_ba9_011:
    d2_command="ror";
    break;
  default:
    d2_unrecognised();
    break;
  }
}
void                              d2_bit_shift_left_to_mem(){
  switch(ir&BITS_ba9){
  case BITS_ba9_000:
    d2_command="asl";
    break;
  case BITS_ba9_001:
    d2_command="lsl";
    break;
  case BITS_ba9_010:
    d2_command="roxl";
    break;
  case BITS_ba9_011:
    d2_command="rol";
    break;
  default:
    d2_unrecognised();
    break;
  }
  D2_GET_DEST_W_NOT_A_OR_D;
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


void d2_0000(){ //immediate stuff
  switch(ir&BITS_876){
  case BITS_876_100:
    if((ir&BITS_543)==BITS_543_001){
      d2_command="movep.w";
      d2_src_w=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+D2_BRACKETS_aM;
      trace_add_entry("source memory: ",d2_src_w.c_str(),TDE_BEFORE,false,1,areg[PARAM_M]+(signed short)d2_fetchW());
      trace_add_entry("source memory: 2+",d2_src_w.c_str(),TDE_BEFORE,false,1,areg[PARAM_M]+(signed short)d2_fetchW()+2);
      dpc+=2;
      d2_dest=D2_dN;
      trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_N]);
    }else{
      d2_command="btst";
      d2_src_b=D2_dN;
      trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,2,(MEM_ADDRESS)&r[PARAM_N]);
      EasyStr a=d2_src_b;
      D2_GET_SOURCE_B;
      d2_dest=d2_src_b;
      d2_src_b=a;
    }
    break;
  case BITS_876_101:
    if((ir&BITS_543)==BITS_543_001){
      d2_command="movep.l";
      d2_src_l=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+D2_BRACKETS_aM;
      trace_add_entry("source memory: ",d2_src_w.c_str(),TDE_BEFORE,false,1,areg[PARAM_M]+(signed short)d2_fetchW());
      trace_add_entry("source memory: 2+",d2_src_w.c_str(),TDE_BEFORE,false,1,areg[PARAM_M]+(signed short)d2_fetchW()+2);
      trace_add_entry("source memory: 4+",d2_src_w.c_str(),TDE_BEFORE,false,1,areg[PARAM_M]+(signed short)d2_fetchW()+4);
      trace_add_entry("source memory: 6+",d2_src_w.c_str(),TDE_BEFORE,false,1,areg[PARAM_M]+(signed short)d2_fetchW()+6);
      dpc+=2;
      d2_dest=D2_dN;
      trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_N]);
    }else{
      d2_command="bchg";
      d2_src_b=D2_dN;
      trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,2,(MEM_ADDRESS)&r[PARAM_N]);
      D2_GET_DEST_B;
    }
    break;
  case BITS_876_110:
    if((ir&BITS_543)==BITS_543_001){
      d2_command="movep.w";
      d2_src_w=D2_dN;
      trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,2,(MEM_ADDRESS)&r[PARAM_N]);
      d2_dest=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+D2_BRACKETS_aM;
      trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,areg[PARAM_M]+(signed short)d2_fetchW());
      trace_add_entry("dest memory: 2+",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,areg[PARAM_M]+(signed short)d2_fetchW()+2);
      dpc+=2;
    }else{
      d2_command="bclr";
      d2_src_b=D2_dN;
      trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,2,(MEM_ADDRESS)&r[PARAM_N]);
      D2_GET_DEST_B;
    }
    break;
  case BITS_876_111:
    if((ir&BITS_543)==BITS_543_001){
      d2_command="movep.l";
      d2_src_l=D2_dN;
      trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,4,(MEM_ADDRESS)&r[PARAM_N]);
      d2_dest=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+D2_BRACKETS_aM;
      trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,areg[PARAM_M]+(signed short)d2_fetchW());
      trace_add_entry("dest memory: 2+",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,areg[PARAM_M]+(signed short)d2_fetchW()+2);
      trace_add_entry("dest memory: 4+",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,areg[PARAM_M]+(signed short)d2_fetchW()+4);
      trace_add_entry("dest memory: 6+",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,areg[PARAM_M]+(signed short)d2_fetchW()+6);
      dpc+=2;
    }else{
      d2_command="bset";
      d2_src_b=D2_dN;
      trace_add_entry("source register: ",reg_name(PARAM_N),TDE_BEFORE,true,2,(MEM_ADDRESS)&r[PARAM_N]);
      D2_GET_DEST_B;
    }
    break;
  default:
    d2_jump_line_0[(ir&(BITS_876|BITS_ba9))>>6]();
  }

}

void d2_0001(){  //move.b
  d2_command="move.b";
  D2_GET_SOURCE_B;
  switch(ir&BITS_876){
  case BITS_876_000:
    d2_dest=D2_dN;
    trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,1,(MEM_ADDRESS)&r[PARAM_N]);
    break;
  case BITS_876_001:
    d2_unrecognised();
    break;
  case BITS_876_010:
    d2_dest=D2_BRACKETS_aN;
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,areg[PARAM_N]);
    break;
  case BITS_876_011:
    d2_dest=D2_BRACKETS_aN+"+";
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,areg[PARAM_N]);
    trace_add_entry("postincrement register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    break;
  case BITS_876_100:
    d2_dest=EasyStr("-")+D2_BRACKETS_aN;
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,areg[PARAM_N]-1);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    break;
  case BITS_876_101:
    d2_dest=d2_signed_offset((short)d2_fetchW())+D2_BRACKETS_aN;
    trace_add_entry("immediate offset: ","",TDE_BEFORE,false,2,dpc);
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,areg[PARAM_N]+(signed short)d2_fetchW());
    dpc+=2;
    break;
  case BITS_876_110:
    d2_ap=d2_fetchW();
    d2_dest=D2_IRIWO_N;
    trace_add_entry("dest base address register: ",reg_name(PARAM_N+8),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,d2_calc_iriwo(areg[PARAM_N]));
    dpc+=2;
    break;
  case BITS_876_111:
    switch(ir&BITS_ba9){
    case BITS_ba9_000:
      d2_dest=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+".w";
      trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,(MEM_ADDRESS)0xffffff&((signed long)((signed short)d2_fetchW())));
      dpc+=2;
      break;
    case BITS_ba9_001:
      d2_dest=EasyStr("$")+itoa(d2_fetchL(),d2_t_buf,16);
      trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,1,(MEM_ADDRESS)0xffffff&d2_fetchL());
      dpc+=4;
      break;
    default:
      d2_unrecognised();
    }
  }
}

void d2_0010(){
  d2_command="move.l";
  D2_GET_SOURCE_L;
  switch(ir&BITS_876){
  case BITS_876_000:
    d2_dest=D2_dN;
    trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_N]);
    break;
  case BITS_876_001:
    d2_command="movea.l";
    d2_dest=D2_aN;
    trace_add_entry("dest register: ",reg_name(PARAM_N+8),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    break;
  case BITS_876_010:
    d2_dest=D2_BRACKETS_aN;
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,4,areg[PARAM_N]);
    break;
  case BITS_876_011:
    d2_dest=D2_BRACKETS_aN+"+";
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,4,areg[PARAM_N]);
    trace_add_entry("postincrement register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    break;
  case BITS_876_100:
    d2_dest=EasyStr("-")+D2_BRACKETS_aN;
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,4,areg[PARAM_N]-4);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    break;
  case BITS_876_101:
    d2_dest=d2_signed_offset((short)d2_fetchW())+D2_BRACKETS_aN;
    trace_add_entry("immediate offset: ","",TDE_BEFORE,false,2,dpc);
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,4,areg[PARAM_N]+(signed short)d2_fetchW());
    dpc+=2;
    break;
  case BITS_876_110:
    d2_ap=d2_fetchW();
    d2_dest=D2_IRIWO_N;
    trace_add_entry("dest base address register: ",reg_name(PARAM_N+8),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,4,d2_calc_iriwo(areg[PARAM_N]));
    dpc+=2;
    break;
  case BITS_876_111:
    switch(ir&BITS_ba9){
    case BITS_ba9_000:
      d2_dest=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+".w";
      trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,4,(MEM_ADDRESS)0xffffff&((signed long)((signed short)d2_fetchW())));
      dpc+=2;
      break;
    case BITS_ba9_001:
      d2_dest=EasyStr("$")+itoa(d2_fetchL(),d2_t_buf,16);
      trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,4,(MEM_ADDRESS)0xffffff&d2_fetchL());
      dpc+=4;
      break;
    default:
      d2_unrecognised();
    }
  }
}

void d2_0011(){
  d2_command="move.w";
  D2_GET_SOURCE_W;
  switch(ir&BITS_876){
  case BITS_876_000:
    d2_dest=D2_dN;
    trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,2,(MEM_ADDRESS)&r[PARAM_N]);
    break;
  case BITS_876_001:
    d2_command="movea.w";
    d2_dest=D2_aN;
    trace_add_entry("dest register: ",reg_name(PARAM_N+8),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    break;
  case BITS_876_010:
    d2_dest=D2_BRACKETS_aN;
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,2,areg[PARAM_N]);
    break;
  case BITS_876_011:
    d2_dest=D2_BRACKETS_aN+"+";
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,2,areg[PARAM_N]);
    trace_add_entry("postincrement register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    break;
  case BITS_876_100:
    d2_dest=EasyStr("-")+D2_BRACKETS_aN;
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,2,areg[PARAM_N]-2);
    trace_add_entry("predecrement register: ",reg_name(8+PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    break;
  case BITS_876_101:
    d2_dest=d2_signed_offset((short)d2_fetchW())+D2_BRACKETS_aN;
    trace_add_entry("immediate offset: ","",TDE_BEFORE,false,2,dpc);
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,2,areg[PARAM_N]+(signed short)d2_fetchW());
    dpc+=2;
    break;
  case BITS_876_110:
    d2_ap=d2_fetchW();
    d2_dest=D2_IRIWO_N;
    trace_add_entry("dest base address register: ",reg_name(PARAM_N+8),TDE_BEFORE,true,4,(MEM_ADDRESS)&areg[PARAM_N]);
    trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,2,d2_calc_iriwo(areg[PARAM_N]));
    dpc+=2;
    break;
  case BITS_876_111:
    switch(ir&BITS_ba9){
    case BITS_ba9_000:
      d2_dest=EasyStr("$")+itoa(d2_fetchW(),d2_t_buf,16)+".w";
      trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,2,(MEM_ADDRESS)0xffffff&((signed long)((signed short)d2_fetchW())));
      dpc+=2;
      break;
    case BITS_ba9_001:
      d2_dest=EasyStr("$")+itoa(d2_fetchL(),d2_t_buf,16);
      trace_add_entry("dest memory: ",d2_dest.c_str(),TDE_BEFORE|TDE_AFTER,false,2,(MEM_ADDRESS)0xffffff&d2_fetchL());
      dpc+=4;
      break;
    default:
      d2_unrecognised();
    }
  }
}

void d2_0100(){
  d2_jump_line_4[(ir&(BITS_ba9|BITS_876))>>6]();
}

void d2_0101(){
  d2_jump_line_5[(ir&BITS_876)>>6]();
}

void d2_0110(){  //bCC
  d2_command="b  ";
  memcpy(d2_command.Text+1,("ra sr hi ls cc cs ne eq vc vs pl mi ge lt gt le "
                             +3*((ir&0xf00)>>8)),2);
  trace_add_entry("program counter: ","pc",TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&pc);
  d2_ap=LOBYTE(ir);
  if (d2_ap){
    d2_command+=".s";
    if (d2_ap & 0x80){
      d2_dest=EasyStr("-")+STRS(0x100-d2_ap);
      d2_ap=WORD(-(0x100-d2_ap));
    }else{
      d2_dest=EasyStr("+")+STRS(d2_ap);
    }
    d2_dest+=Str(" {$")+HEXSl(dpc+(signed short)d2_ap,6)+"}";
  }else{
    d2_ap=d2_fetchW();
    if (d2_ap & 0x8000){
      d2_dest=EasyStr(".l -$")+HEXS(0x10000-d2_ap);
      d2_ap=WORD(-(0x10000-d2_ap));
    }else{
      d2_dest=EasyStr(".l +$")+HEXS(d2_ap);
    }
    trace_add_entry("branch offset: ","",TDE_BEFORE,false,2,dpc);
    d2_dest+=Str(" {$")+HEXSl(dpc+(signed short)d2_ap,6)+"}";
    dpc+=2;
  }
}

void d2_0111(){  //moveq
  if(ir&BIT_8){
    d2_unrecognised();
  }else{
    d2_command="moveq";
    d2_src_b=EasyStr("#")+STRS((signed char)LOBYTE(ir));
    d2_dest=D2_dN;
    trace_add_entry("dest register: ",reg_name(PARAM_N),TDE_BEFORE|TDE_AFTER,true,4,(MEM_ADDRESS)&r[PARAM_N]);
  }
}

void d2_1000(){ //or, div, sbcd
  d2_jump_line_8[(ir&BITS_876)>>6]();
}

void d2_1001(){ //sub
  d2_jump_line_9[(ir&BITS_876)>>6]();
}

void d2_1010(){  //line-a
  d2_command="line-A";
  d2_src_w=EasyStr("#$")+HEXS(ir&0xfff);
}

void d2_1011(){ //cmp, eor
  d2_jump_line_b[(ir&BITS_876)>>6]();
}

void d2_1100(){ // and, abcd, exg, mul
  d2_jump_line_c[(ir&BITS_876)>>6]();
}

void d2_1101(){   //add
  d2_jump_line_d[(ir&BITS_876)>>6]();
}

void d2_1110(){  //bit shift
  d2_jump_line_e[(ir&(BITS_876|BITS_543))>>3]();
}

void d2_1111(){
  d2_command="line-F";
  d2_src_w=EasyStr("#$")+HEXS(ir&0xfff);
}



void d2_routines_init(){
  d2_high_nibble_jump_table[0]=d2_0000;
  d2_high_nibble_jump_table[1]=d2_0001;
  d2_high_nibble_jump_table[2]=d2_0010;
  d2_high_nibble_jump_table[3]=d2_0011;
  d2_high_nibble_jump_table[4]=d2_0100;
  d2_high_nibble_jump_table[5]=d2_0101;
  d2_high_nibble_jump_table[6]=d2_0110;
  d2_high_nibble_jump_table[7]=d2_0111;
  d2_high_nibble_jump_table[8]=d2_1000;
  d2_high_nibble_jump_table[9]=d2_1001;
  d2_high_nibble_jump_table[10]=d2_1010;
  d2_high_nibble_jump_table[11]=d2_1011;
  d2_high_nibble_jump_table[12]=d2_1100;
  d2_high_nibble_jump_table[13]=d2_1101;
  d2_high_nibble_jump_table[14]=d2_1110;
  d2_high_nibble_jump_table[15]=d2_1111;

  d2_jump_get_source_b[0]=d2_get_source_000_b;
  d2_jump_get_source_w[0]=d2_get_source_000_w;
  d2_jump_get_source_l[0]=d2_get_source_000_l;
  d2_jump_get_source_b[1]=d2_get_source_001_b;
  d2_jump_get_source_w[1]=d2_get_source_001_w;
  d2_jump_get_source_l[1]=d2_get_source_001_l;
  d2_jump_get_source_b[2]=d2_get_source_010_b;
  d2_jump_get_source_w[2]=d2_get_source_010_w;
  d2_jump_get_source_l[2]=d2_get_source_010_l;
  d2_jump_get_source_b[3]=d2_get_source_011_b;
  d2_jump_get_source_w[3]=d2_get_source_011_w;
  d2_jump_get_source_l[3]=d2_get_source_011_l;
  d2_jump_get_source_b[4]=d2_get_source_100_b;
  d2_jump_get_source_w[4]=d2_get_source_100_w;
  d2_jump_get_source_l[4]=d2_get_source_100_l;
  d2_jump_get_source_b[5]=d2_get_source_101_b;
  d2_jump_get_source_w[5]=d2_get_source_101_w;
  d2_jump_get_source_l[5]=d2_get_source_101_l;
  d2_jump_get_source_b[6]=d2_get_source_110_b;
  d2_jump_get_source_w[6]=d2_get_source_110_w;
  d2_jump_get_source_l[6]=d2_get_source_110_l;
  d2_jump_get_source_b[7]=d2_get_source_111_b;
  d2_jump_get_source_w[7]=d2_get_source_111_w;
  d2_jump_get_source_l[7]=d2_get_source_111_l;


  d2_jump_get_dest_b[0]=d2_get_dest_000_b;
  d2_jump_get_dest_w[0]=d2_get_dest_000_w;
  d2_jump_get_dest_l[0]=d2_get_dest_000_l;
  d2_jump_get_dest_b[1]=d2_get_dest_001_b;
  d2_jump_get_dest_w[1]=d2_get_dest_001_w;
  d2_jump_get_dest_l[1]=d2_get_dest_001_l;
  d2_jump_get_dest_b[2]=d2_get_dest_010_b;
  d2_jump_get_dest_w[2]=d2_get_dest_010_w;
  d2_jump_get_dest_l[2]=d2_get_dest_010_l;
  d2_jump_get_dest_b[3]=d2_get_dest_011_b;
  d2_jump_get_dest_w[3]=d2_get_dest_011_w;
  d2_jump_get_dest_l[3]=d2_get_dest_011_l;
  d2_jump_get_dest_b[4]=d2_get_dest_100_b;
  d2_jump_get_dest_w[4]=d2_get_dest_100_w;
  d2_jump_get_dest_l[4]=d2_get_dest_100_l;
  d2_jump_get_dest_b[5]=d2_get_dest_101_b;
  d2_jump_get_dest_w[5]=d2_get_dest_101_w;
  d2_jump_get_dest_l[5]=d2_get_dest_101_l;
  d2_jump_get_dest_b[6]=d2_get_dest_110_b;
  d2_jump_get_dest_w[6]=d2_get_dest_110_w;
  d2_jump_get_dest_l[6]=d2_get_dest_110_l;
  d2_jump_get_dest_b[7]=d2_get_dest_111_b;
  d2_jump_get_dest_w[7]=d2_get_dest_111_w;
  d2_jump_get_dest_l[7]=d2_get_dest_111_l;
  for(int n=0;n<8;n++){
    d2_jump_get_dest_b_not_a[n]=d2_jump_get_dest_b[n];
    d2_jump_get_dest_w_not_a[n]=d2_jump_get_dest_w[n];
    d2_jump_get_dest_l_not_a[n]=d2_jump_get_dest_l[n];
    d2_jump_get_dest_b_not_a_or_d[n]=d2_jump_get_dest_b[n];
    d2_jump_get_dest_w_not_a_or_d[n]=d2_jump_get_dest_w[n];
    d2_jump_get_dest_l_not_a_or_d[n]=d2_jump_get_dest_l[n];
    d2_jump_get_dest_b_not_a_with_sr[n]=d2_jump_get_dest_b[n];
    d2_jump_get_dest_w_not_a_with_sr[n]=d2_jump_get_dest_w[n];
    d2_jump_get_dest_l_not_a_with_sr[n]=d2_jump_get_dest_l[n];
  }
  d2_jump_get_dest_b_not_a[1]=d2_unrecognised;
  d2_jump_get_dest_w_not_a[1]=d2_unrecognised;
  d2_jump_get_dest_l_not_a[1]=d2_unrecognised;
  d2_jump_get_dest_b_not_a_or_d[0]=d2_unrecognised;
  d2_jump_get_dest_w_not_a_or_d[0]=d2_unrecognised;
  d2_jump_get_dest_l_not_a_or_d[0]=d2_unrecognised;
  d2_jump_get_dest_b_not_a_or_d[1]=d2_unrecognised;
  d2_jump_get_dest_w_not_a_or_d[1]=d2_unrecognised;
  d2_jump_get_dest_l_not_a_or_d[1]=d2_unrecognised;
  d2_jump_get_dest_b_not_a_with_sr[1]=d2_unrecognised;
  d2_jump_get_dest_w_not_a_with_sr[1]=d2_unrecognised;
  d2_jump_get_dest_l_not_a_with_sr[1]=d2_unrecognised;

  d2_jump_get_dest_b_not_a_with_sr[7]=d2_get_dest_111_b_with_sr;
  d2_jump_get_dest_w_not_a_with_sr[7]=d2_get_dest_111_w_with_sr;

  for(int n=0;n<64;n++){
    d2_jump_line_0[n]=d2_unrecognised;
    d2_jump_line_4[n]=d2_unrecognised;
    d2_jump_line_4_stuff[n]=d2_unrecognised;
    d2_jump_line_e[n]=d2_unrecognised;
  }

  d2_jump_line_0[B6_000000]=d2_ori_b;
  d2_jump_line_0[B6_000001]=d2_ori_w;
  d2_jump_line_0[B6_000010]=d2_ori_l;
  d2_jump_line_0[B6_001000]=d2_andi_b;
  d2_jump_line_0[B6_001001]=d2_andi_w;
  d2_jump_line_0[B6_001010]=d2_andi_l;
  d2_jump_line_0[B6_010000]=d2_subi_b;
  d2_jump_line_0[B6_010001]=d2_subi_w;
  d2_jump_line_0[B6_010010]=d2_subi_l;
  d2_jump_line_0[B6_011000]=d2_addi_b;
  d2_jump_line_0[B6_011001]=d2_addi_w;
  d2_jump_line_0[B6_011010]=d2_addi_l;
  d2_jump_line_0[B6_100000]=d2_btst;
  d2_jump_line_0[B6_100001]=d2_bchg;
  d2_jump_line_0[B6_100010]=d2_bclr;
  d2_jump_line_0[B6_100011]=d2_bset;
  d2_jump_line_0[B6_101000]=d2_eori_b;
  d2_jump_line_0[B6_101001]=d2_eori_w;
  d2_jump_line_0[B6_101010]=d2_eori_l;
  d2_jump_line_0[B6_110000]=d2_cmpi_b;
  d2_jump_line_0[B6_110001]=d2_cmpi_w;
  d2_jump_line_0[B6_110010]=d2_cmpi_l;

  d2_jump_line_4[B6_000000]=d2_negx_b;
  d2_jump_line_4[B6_000001]=d2_negx_w;
  d2_jump_line_4[B6_000010]=d2_negx_l;
  d2_jump_line_4[B6_000011]=d2_move_from_sr;
  d2_jump_line_4[B6_001000]=d2_clr_b;
  d2_jump_line_4[B6_001001]=d2_clr_w;
  d2_jump_line_4[B6_001010]=d2_clr_l;
  d2_jump_line_4[B6_001011]=d2_move_from_ccr;
  d2_jump_line_4[B6_010000]=d2_neg_b;
  d2_jump_line_4[B6_010001]=d2_neg_w;
  d2_jump_line_4[B6_010010]=d2_neg_l;
  d2_jump_line_4[B6_010011]=d2_move_to_ccr;
  d2_jump_line_4[B6_011000]=d2_not_b;
  d2_jump_line_4[B6_011001]=d2_not_w;
  d2_jump_line_4[B6_011010]=d2_not_l;
  d2_jump_line_4[B6_011011]=d2_move_to_sr;
  d2_jump_line_4[B6_100000]=d2_nbcd;
  d2_jump_line_4[B6_100001]=d2_pea_or_swap;
  d2_jump_line_4[B6_100010]=d2_movem_w_from_regs_or_ext_w;
  d2_jump_line_4[B6_100011]=d2_movem_l_from_regs_or_ext_l;
  d2_jump_line_4[B6_101000]=d2_tst_b;
  d2_jump_line_4[B6_101001]=d2_tst_w;
  d2_jump_line_4[B6_101010]=d2_tst_l;
  d2_jump_line_4[B6_101011]=d2_tas;
  d2_jump_line_4[B6_110010]=d2_movem_w_to_regs;
  d2_jump_line_4[B6_110011]=d2_movem_l_to_regs;
  d2_jump_line_4[B6_111001]=d2_line_4_stuff;
  d2_jump_line_4[B6_111010]=d2_jsr;
  d2_jump_line_4[B6_111011]=d2_jmp;

  for(int n=0;n<8;n++){
    d2_jump_line_4[(n<<3)+6]=d2_chk;
    d2_jump_line_4[(n<<3)+7]=d2_lea;
    d2_jump_line_4_stuff[BITS_543_000+n]=d2_trap;
    d2_jump_line_4_stuff[BITS_543_001+n]=d2_trap;
    d2_jump_line_4_stuff[BITS_543_010+n]=d2_link;
    d2_jump_line_4_stuff[BITS_543_011+n]=d2_unlk;
    d2_jump_line_4_stuff[BITS_543_100+n]=d2_move_to_usp;
    d2_jump_line_4_stuff[BITS_543_101+n]=d2_move_from_usp;
  }

  d2_jump_line_4_stuff[B6_110000]=d2_reset;
  d2_jump_line_4_stuff[B6_110001]=d2_nop;
  d2_jump_line_4_stuff[B6_110010]=d2_stop;
  d2_jump_line_4_stuff[B6_110011]=d2_rte;
  d2_jump_line_4_stuff[B6_110100]=d2_rtd;
  d2_jump_line_4_stuff[B6_110101]=d2_rts;
  d2_jump_line_4_stuff[B6_110110]=d2_trapv;
  d2_jump_line_4_stuff[B6_110111]=d2_rtr;
  d2_jump_line_4_stuff[B6_111010]=d2_movec;
  d2_jump_line_4_stuff[B6_111011]=d2_movec;

  d2_jump_line_5[0]=d2_addq_b;
  d2_jump_line_5[1]=d2_addq_w;
  d2_jump_line_5[2]=d2_addq_l;
  d2_jump_line_5[3]=d2_dbCC_or_sCC;
  d2_jump_line_5[4]=d2_subq_b;
  d2_jump_line_5[5]=d2_subq_w;
  d2_jump_line_5[6]=d2_subq_l;
  d2_jump_line_5[7]=d2_dbCC_or_sCC;

  d2_jump_line_8[0]=d2_or_b_to_dN;
  d2_jump_line_8[1]=d2_or_w_to_dN;
  d2_jump_line_8[2]=d2_or_l_to_dN;
  d2_jump_line_8[3]=d2_divu;
  d2_jump_line_8[4]=d2_or_b_from_dN_or_sbcd;
  d2_jump_line_8[5]=d2_or_w_from_dN;
  d2_jump_line_8[6]=d2_or_l_from_dN;
  d2_jump_line_8[7]=d2_divs;

  d2_jump_line_9[0]=d2_sub_b_to_dN;
  d2_jump_line_9[1]=d2_sub_w_to_dN;
  d2_jump_line_9[2]=d2_sub_l_to_dN;
  d2_jump_line_9[3]=d2_suba_w;
  d2_jump_line_9[4]=d2_sub_b_from_dN;
  d2_jump_line_9[5]=d2_sub_w_from_dN;
  d2_jump_line_9[6]=d2_sub_l_from_dN;
  d2_jump_line_9[7]=d2_suba_l;

  d2_jump_line_b[0]=d2_cmp_b;
  d2_jump_line_b[1]=d2_cmp_w;
  d2_jump_line_b[2]=d2_cmp_l;
  d2_jump_line_b[3]=d2_cmpa_w;
  d2_jump_line_b[4]=d2_eor_b;
  d2_jump_line_b[5]=d2_eor_w;
  d2_jump_line_b[6]=d2_eor_l;
  d2_jump_line_b[7]=d2_cmpa_l;

  d2_jump_line_c[0]=d2_and_b_to_dN;
  d2_jump_line_c[1]=d2_and_w_to_dN;
  d2_jump_line_c[2]=d2_and_l_to_dN;
  d2_jump_line_c[3]=d2_mulu;
  d2_jump_line_c[4]=d2_and_b_from_dN_or_abcd;
  d2_jump_line_c[5]=d2_and_w_from_dN_or_exg_like;
  d2_jump_line_c[6]=d2_and_l_from_dN_or_exg_unlike;
  d2_jump_line_c[7]=d2_muls;

  d2_jump_line_d[0]=d2_add_b_to_dN;
  d2_jump_line_d[1]=d2_add_w_to_dN;
  d2_jump_line_d[2]=d2_add_l_to_dN;
  d2_jump_line_d[3]=d2_adda_w;
  d2_jump_line_d[4]=d2_add_b_from_dN;
  d2_jump_line_d[5]=d2_add_w_from_dN;
  d2_jump_line_d[6]=d2_add_l_from_dN;
  d2_jump_line_d[7]=d2_adda_l;

  d2_jump_line_e[B6_000000]=d2_asr_b_to_dM;
  d2_jump_line_e[B6_000001]=d2_lsr_b_to_dM;
  d2_jump_line_e[B6_000010]=d2_roxr_b_to_dM;
  d2_jump_line_e[B6_000011]=d2_ror_b_to_dM;
  d2_jump_line_e[B6_001000]=d2_asr_w_to_dM;
  d2_jump_line_e[B6_001001]=d2_lsr_w_to_dM;
  d2_jump_line_e[B6_001010]=d2_roxr_w_to_dM;
  d2_jump_line_e[B6_001011]=d2_ror_w_to_dM;
  d2_jump_line_e[B6_010000]=d2_asr_l_to_dM;
  d2_jump_line_e[B6_010001]=d2_lsr_l_to_dM;
  d2_jump_line_e[B6_010010]=d2_roxr_l_to_dM;
  d2_jump_line_e[B6_010011]=d2_ror_l_to_dM;
  d2_jump_line_e[B6_100000]=d2_asl_b_to_dM;
  d2_jump_line_e[B6_100001]=d2_lsl_b_to_dM;
  d2_jump_line_e[B6_100010]=d2_roxl_b_to_dM;
  d2_jump_line_e[B6_100011]=d2_rol_b_to_dM;
  d2_jump_line_e[B6_101000]=d2_asl_w_to_dM;
  d2_jump_line_e[B6_101001]=d2_lsl_w_to_dM;
  d2_jump_line_e[B6_101010]=d2_roxl_w_to_dM;
  d2_jump_line_e[B6_101011]=d2_rol_w_to_dM;
  d2_jump_line_e[B6_110000]=d2_asl_l_to_dM;
  d2_jump_line_e[B6_110001]=d2_lsl_l_to_dM;
  d2_jump_line_e[B6_110010]=d2_roxl_l_to_dM;
  d2_jump_line_e[B6_110011]=d2_rol_l_to_dM;

  for(int a=B6_000000;a<=B6_111000;a+=B6_001000){
    for(int b=B6_000000;b<=B6_000011;b+=B6_000001){
      d2_jump_line_e[a+b+B6_000100]=d2_jump_line_e[a+b]; //for bit shifting to data registers, immediate mode and dN, mode point to same function
    }
  }
  for(int n=2;n<8;n++){
    d2_jump_line_e[B6_011000+n]=d2_bit_shift_right_to_mem;
    d2_jump_line_e[B6_111000+n]=d2_bit_shift_left_to_mem;
  }

}


EasyStr disa_d2(MEM_ADDRESS new_dpc)
{
  EasyStr dt;
  dpc=new_dpc & 0xffffff;
  ir=d2_fetchW(); //fetch
  dpc+=2;
  if (d2_peekvalid) return "Not valid address";
  old_dpc=dpc;
  d2_src="";
  d2_dest="";
  d2_command="";
  d2_pc_rel_ex="";
  d2_high_nibble_jump_table[ir>>12]();     //execute
  if (strstr(d2_command.c_str(),"dc.w")==NULL){
    dt=d2_command+" ";
    if (d2_src.IsEmpty()){
      dt+=d2_dest;
    }else{
      if (d2_dest.IsEmpty()){
        dt+=d2_src;
      }else{
        dt+=d2_src+","+d2_dest;
      }
    }
  }else{
    dt=d2_command;
    dpc=old_dpc;
  }
  dt+=d2_pc_rel_ex;
  return dt;
}

