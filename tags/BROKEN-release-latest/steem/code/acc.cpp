// Porting: GetNearestPaletteIndex  get_text_width flush_message_queue

/*void inline log(EasyStr a){
#ifdef _DEBUG_BUILD
  log_write(a);
#endif
}*/
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
int count_bits_set_in_word(unsigned short w)
{
  int t=0;
  for(int n=15;n>=0;n--){
    if(w&1)t++;w>>=1;
  }
  return t;
}

#define INVALID_SP (mem_len-128)

// This always returns a valid address
MEM_ADDRESS get_sp_before_trap(bool *pInvalid)
{
  MEM_ADDRESS sp=(areg[7] & 0xffffff)+6;
  // sp now points to first byte after the exception stack frame
  if (sp<mem_len){
    // First byte on stack is high byte of sr
    if ( (PEEK(areg[7] & 0xffffff) & BIT_5) == 0 ){
      // Supervisor bit not set in stacked sr
      sp=(other_sp & 0xffffff);
    }
  }
  if (sp>=mem_len){
    if (pInvalid) *pInvalid=true;
    return INVALID_SP;
  }
  return sp;
}

#ifdef UNIX
EasyStr GetEXEDir()
{
  EasyStr Path=_argv[0];
  RemoveFileNameFromPath(Path,REMOVE_SLASH);
  return Path;
}
//---------------------------------------------------------------------------
EasyStr GetCurrentDir()
{
  EasyStr Path=GetEXEDir();
  Path.SetLength(MAX_PATH);
  getcwd(Path,MAX_PATH);
  return Path;
}
//---------------------------------------------------------------------------
EasyStr GetEXEFileName()
{
  return _argv[0];
}
//---------------------------------------------------------------------------
#endif

EasyStr time_or_times(int n)
{
  if(n==1)return T("time");
  return T("times");
}

long colour_convert(int red,int green,int blue)
{
  long col;

  switch (BytesPerPixel){
  case 1:
#ifdef WIN32
    if((red|green|blue)==0)return 0;
    if((red&green&blue)==255)return 0xffff;
    col=GetNearestPaletteIndex(winpal,RGB(red,green,blue))+1;
    return (col<<8)|col;
#elif defined(UNIX)
/*
  {
    XColor xc={0,red << 8,green << 8,blue << 8};
    XAllocColor(XD,XDefaultColormap(XD,XDefaultScreen(XD)),&xc);
    return WORD(BYTE(xc.pixel) | (xc.pixel << 8));
  }
*/
    return 0;
#endif
  case 2:
    //.RrrrrGggggBbbbb
    //        Ccccc...
    if (rgb555){                                  //%-000001111100000 1555
      col=((red&0xf8)<<7) | ((green&0xf8)<<2) | ((blue)>>3);
    }else{                                        //%00000011111100000 565
      col=((red&0xf8)<<8) | ((green&0xfc)<<3) | ((blue)>>3);
    }
    return (col<<16)|col;
  case 3:case 4:
    return ((red << 16) | (green << 8) | blue) << rgb32_bluestart_bit;
  }
  return 0;
}


void flush_message_queue()
{
  while (PeekEvent()==PEEKED_MESSAGE);
}

EasyStr read_string_from_memory(MEM_ADDRESS ad,int max_len)
{
  if (ad==0) return "";

  EasyStr a;a.SetLength(max_len);
  int n;
  char i;
  for (n=0;n<max_len;n++){
    if (ad<himem){
      i=PEEK(ad);
    }else if (ad>=rom_addr && ad<rom_addr+tos_len){
      i=ROM_PEEK(ad-rom_addr);
    }else{
      break;
    }
    ad++;
    if (i==0) break;
    a[n]=i;
  }
  a[n]=0;
  return a;
}

MEM_ADDRESS write_string_to_memory(MEM_ADDRESS ad,char*c)
{
  if (ad<MEM_FIRST_WRITEABLE) return 0;
  do{
    m68k_poke(ad++,*c);
  }while (*(c++));
  return ad;
}


WORD STfile_read_word(FILE*f){

  WORD result;
#ifdef BIG_ENDIAN_PROCESSOR
  fread(&result,1,2,f);
#else
  fread((BYTE*)(&result)+1,1,1,f); //high byte in file ->high byte in word
  fread((BYTE*)(&result),1,1,f); //low byte in file ->low byte in word
#endif
  return result;
}

LONG STfile_read_long(FILE*f){
  LONG result;
#ifdef BIG_ENDIAN_PROCESSOR
  if(!fread(&result,1,4,f)){STfile_read_error=true;return 0;};
#else
  STfile_read_error=0;
  if(!fread((BYTE*)(&result)+3,1,1,f)){STfile_read_error=true;return 0;} //high byte in file ->high byte in word
  if(!fread((BYTE*)(&result)+2,1,1,f)){STfile_read_error=true;return 0;}; //mid-high byte in file ->mid-high byte in word
  if(!fread((BYTE*)(&result)+1,1,1,f)){STfile_read_error=true;return 0;}; //mid-low byte in file ->mid-low byte in word
  if(!fread((BYTE*)(&result)+0,1,1,f)){STfile_read_error=true;return 0;}; //low byte in file ->low byte in word
#endif
  return result;
}

void STfile_read_to_ST_memory(FILE*f,MEM_ADDRESS ad,int n_bytes)
{
  log_to(LOGSECTION_STEMDOS,EasyStr("STEMDOS: Reading ")+n_bytes+" bytes into "+HEXSl(ad,6));
  BYTE buf;
  for(int n=0;n<n_bytes;n++){
    fread(&buf,1,1,f);
    m68k_poke(ad,buf);
    ad++;
  }
}

void STfile_write_from_ST_memory(FILE *f,MEM_ADDRESS ad,int n_bytes)
{
  TRY_M68K_EXCEPTION
    for (int n=0;n<n_bytes;n++){
      fputc(m68k_peek(ad),f);
      ad++;
    }
  CATCH_M68K_EXCEPTION
  END_M68K_EXCEPTION
}

#ifdef ENABLE_LOGFILE
void log_write(EasyStr a)
{
  if (logfile){
    fprintf(logfile,"%s\r\n",a.Text);
    fflush(logfile);
  }
}

void log_io_write(MEM_ADDRESS addr,BYTE io_src_b)
{
  if (logsection_enabled[LOGSECTION_IO]){
    if (!logging_suspended){
      EasyStr a=Str("IO: ")+HEXSl(old_pc,6)+" - wrote byte "+io_src_b+" to IO address "+HEXSl(addr,6);
#ifdef _DEBUG_BUILD
      iolist_entry *iol=search_iolist(addr);
      if (iol) a+=EasyStr(" (")+(iol->name)+")";
#endif
      log_write(a);
    }
  }
}
//---------------------------------------------------------------------------
void stop_cpu_log()
{
  logsection_enabled[LOGSECTION_CPU]=0;
  DEBUG_ONLY( CheckMenuItem(logsection_menu,300+LOGSECTION_CPU,MF_BYCOMMAND | MF_UNCHECKED); )
}
//---------------------------------------------------------------------------
void log_os_call(int trap)
{
  if (logging_suspended) return;

  EasyStr l="",a="";
  long lpar;

  bool Invalid=0;
  MEM_ADDRESS sp=get_sp_before_trap(&Invalid);
  if (Invalid) return;
  MEM_ADDRESS spp=sp+2;

  unsigned int call=m68k_dpeek(sp);
  l=HEXSl(old_pc,6)+": ";
  if (trap==1){
    if (call<0x58ul) a=(char*)gemdos_calls[call];
    l+="GEMDOS $";
    l+=HEXSl(call,2);
  }else if (trap==13){
    if (call<12ul) a=(char*)bios_calls[call];
    l+="BIOS ";
    l+=(int)call;
  }else if (trap==14){
    if (call<40ul) a=(char*)xbios_calls[call];
    l+="XBIOS ";
    l+=(int)call;
  }else if (trap==2){
    if(r[0]==0x73){
      a="VDI with opcode ";
      MEM_ADDRESS adddd=m68k_lpeek(r[1]); //r[1] has vdi parameter block.  a points to the control array
      a+=m68k_dpeek(adddd+0); //opcode
      a+=", subopcode ";
      a+=m68k_dpeek(adddd+10); //subopcode
    }
  }
  if (a.IsEmpty()){
    l+=" (unrecognised)";
  }else{
    l+="  ";
    for(int i=0;i<a.Length();i++){
      if(a[i]=='%'){
        lpar=m68k_lpeek(spp);spp+=4;
        l+=HEXSl(lpar,8);
      }else if(a[i]=='&'){
        l+=HEXSl(m68k_dpeek(spp),4);spp+=2;
      }else if(a[i]=='$'){
        char c;
        int ii;
        for (ii=0;ii<30;ii++){
          c=(char)m68k_peek(lpar+ii);
          if(!c)break;
          l+=c;
        }
        if(ii>=30)l+="...";
      }else{
        l+=a[i];
      }
    }
  }
  log_write(l);
}

void log_write_stack()
{
#ifdef _DEBUG_BUILD
  log_write(EasyStr("A7 = ")+HEXSl(areg[7],6));
  log_write(EasyStr("stack = ")+HEXSl(d2_dpeek(areg[7]),4));
  log_write(EasyStr("        ")+HEXSl(d2_dpeek(areg[7]+2),4));
  log_write(EasyStr("        ")+HEXSl(d2_dpeek(areg[7]+4),4));
  log_write(EasyStr("        ")+HEXSl(d2_dpeek(areg[7]+6),4));
  log_write(EasyStr("        ")+HEXSl(d2_dpeek(areg[7]+8),4));
  log_write(EasyStr("        ")+HEXSl(d2_dpeek(areg[7]+10),4));
  log_write(EasyStr("        ")+HEXSl(d2_dpeek(areg[7]+12),4));
  log_write(EasyStr("        ")+HEXSl(d2_dpeek(areg[7]+14),4));
  log_write(EasyStr("        ")+HEXSl(d2_dpeek(areg[7]+16),4));
  log_write(EasyStr("        ")+HEXSl(d2_dpeek(areg[7]+18),4));
#endif
}
#endif

EasyStr HEXSl(long n,int ln){
  char bf[17];
  strcpy(bf,"00000000");
  itoa(n,bf+8,16);
  strupr(bf);
  return bf+8-ln+strlen(bf+8);
}

#ifdef _DEBUG_BUILD
char *reg_name(int n){
  _reg_name_buf[0]="da"[int((n & 8) ? 1:0)];
  _reg_name_buf[1]=(char)('0'+(n & 7));
  _reg_name_buf[2]=0;
  return _reg_name_buf;

}
#endif

#ifndef UNIX
int get_text_width(char *t)
{
#ifdef WIN32
  SIZE sz;
  HDC dc=GetDC(StemWin);
  HANDLE oldfnt=SelectObject(dc,fnt);
  GetTextExtentPoint32(dc,t,strlen(t),&sz);
  SelectObject(dc,oldfnt);
  ReleaseDC(StemWin,dc);
  return sz.cx+1; // For grayed string
#elif defined(UNIX)
  return XTextWidth(GUIFont,t,strlen(t));
#endif
}
#endif


int file_read_num(FILE*f){
  char tb[50];
  fgets(tb,49,f);
  return atoi(tb);

}

#ifdef _DEBUG_BUILD
MEM_ADDRESS oi(MEM_ADDRESS ad,int of) //offset by instruction
{
  if (of==0) return ad;
  MEM_ADDRESS save_dpc=dpc;

  if (of>0){
    dpc=ad;
    for (int n=0;n<of;n++) disa_d2(dpc);
    MEM_ADDRESS next_inst=dpc & 0xffffff;
    dpc=save_dpc;
    if (next_inst<ad) return 0;
    return next_inst;
  }else if (of<0){
    for (int n=0;n>of;n--){
      // longest instruction is 10 bytes, start from there
      MEM_ADDRESS sad=(ad-10) & 0xffffff;
      if (sad>ad) ad+=0x1000000;
      while (sad<ad){
        disa_d2(sad & 0xffffff);
        if (dpc==(ad & 0xffffff)) break;
        sad+=2;
      }
      if (sad==ad){ // If nothing turns into a decent instruction just guess
        ad-=2;
      }else{
        ad=sad;
      }
    }
    dpc=save_dpc;
    return ad & 0xffffff;
  }
  return 0;
}

#endif

