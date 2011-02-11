#ifdef IN_MAIN
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)
#endif

#ifdef ENABLE_LOGFILE

  #define log(s)  \
   {if(logsection_enabled[LOGSECTION]){ \
      if(!logging_suspended){            \
        log_write(s);                 \
      }                               \
   }}

  #define logc(s)      \
    if (mode==STEM_MODE_CPU && logsection_enabled[LOGSECTION]) log_write(s);

  #define log_stack  \
   {if(logsection_enabled[LOGSECTION]){ \
      if(!logging_suspended){            \
        log_write_stack();                 \
      }                               \
   }}

  EXT void log_write(EasyStr);
  EXT void log_os_call(int trap);
  #define log_to_section(section,s) if (logsection_enabled[section] && logging_suspended==0) log_write(s);
  #define log_to(section,s)  if (logsection_enabled[section] && logging_suspended==0) log_write(s);
  EXT void log_write_stack();
  EXT bool logging_suspended INIT(false);
  EXT bool logsection_enabled[100];
  EXT void log_io_write(MEM_ADDRESS,BYTE);

  #define CPU_INSTRUCTIONS_TO_LOG 10000
  EXT int log_cpu_count INIT(0);
  EXT void stop_cpu_log();

#ifdef _DEBUG_BUILD
  #define LOG_CPU \
    if (log_cpu_count){ \
      log_to_section(LOGSECTION_CPU,HEXSl(pc,6)+": "+disa_d2(pc)); \
      if ((--log_cpu_count)==0) stop_cpu_log(); \
    }
#else
  #define LOG_CPU
#endif

  #define LOGSECTION_ALWAYS 0
  #define LOGSECTION_FDC 1
  #define LOGSECTION_IO 2
  #define LOGSECTION_MFP_TIMERS 3
  #define LOGSECTION_INIT 4
  #define LOGSECTION_CRASH 5
  #define LOGSECTION_STEMDOS 6
  #define LOGSECTION_IKBD 7
  #define LOGSECTION_AGENDA 8
  #define LOGSECTION_INTERRUPTS 9
  #define LOGSECTION_TRAP 10
  #define LOGSECTION_SOUND 11
  #define LOGSECTION_VIDEO 12
  #define LOGSECTION_BLITTER 13
  #define LOGSECTION_MIDI 14
  #define LOGSECTION_TRACE 15
  #define LOGSECTION_SHUTDOWN 16
  #define LOGSECTION_SPEEDLIMIT 17
  #define LOGSECTION_CPU 18
  #define LOGSECTION_INIFILE 19
  #define LOGSECTION_GUI 20
  #define LOGSECTION_DIV 21
  #define LOGSECTION_PASTI 22
  #define NUM_LOGSECTIONS 23

  extern const char *name_of_mfp_interrupt[21];

#ifdef IN_MAIN
  struct{
    char *Name;
    int Index;
  }logsections[NUM_LOGSECTIONS+8]={{"Always",LOGSECTION_ALWAYS},
                                    {"Video",LOGSECTION_VIDEO},
                                    {"MFP Timers",LOGSECTION_MFP_TIMERS},
                                    {"Interrupts",LOGSECTION_INTERRUPTS},
                                    {"IO",LOGSECTION_IO},
                                    {"Crash",LOGSECTION_CRASH},
                                    {"CPU",LOGSECTION_CPU},
                                    {"Div Instructions",LOGSECTION_DIV},
                                    {"Trace",LOGSECTION_TRACE},
                                    {"-",-1},
                                    {"FDC",LOGSECTION_FDC},
                                    {"Pasti",LOGSECTION_PASTI},
                                    {"Stemdos",LOGSECTION_STEMDOS},
                                    {"Trap",LOGSECTION_TRAP},
                                    {"-",-1},
                                    {"IKBD",LOGSECTION_IKBD},
                                    {"Tasks",LOGSECTION_AGENDA},
                                    {"Blitter",LOGSECTION_BLITTER},
                                    {"Sound",LOGSECTION_SOUND},
                                    {"MIDI",LOGSECTION_MIDI},
                                    {"-",-1},
                                    {"Speed Limiting",LOGSECTION_SPEEDLIMIT},
                                    {"Startup",LOGSECTION_INIT},
                                    {"Shutdown",LOGSECTION_SHUTDOWN},
                                    {"INI File",LOGSECTION_INIFILE},
                                    {"GUI",LOGSECTION_GUI},
                                    {"*",-1}};

  const char *name_of_mfp_interrupt[21]={"Centronics","RS232 DCD","RS232 CTS","Blitter",
        "Timer D","Timer C","ACIAs","FDC","Timer B","RS232 TX Error","RS232 RX Buf Empty",
        "RS232 RX Error","RS232 RX Buf Full","Timer A","RS232 Ring Detector","Mono Monitor",
        "HBL","VBL","Line-A","Line-F","Trap"};

  //////////////////////////////// names of OS calls //////////////////////////////////
  const char* gemdos_calls[0x58]={"Pterm0","Conin","Conout(c=&)","Cauxin","Cauxout (c=&)",
        "Cprnout(c=&)","Raw con io Crawio(c=&)","Crawcin","Cnecic","Print line(text=%)",
        "ReadLine(buf=%)","Constat","","","SetDrv(drv=&)","","Conout stat","Printer status",
        "inp?(serial)","out?(serial)","","","","","","GetDrv","SetDTA(buf=%)","","","","","",
        "super(%)","","","","","","","","","","GetDate","SetDate(date=&)","Gettime",
        "Settime(time=&)","","GetDTA","Get version number","PtermRes(keepcnt=%,retcode=&)",
        "","","","","Dfree(buf=%,drive=&)","","","Mkdir(path=%=$)","Rmdir(path=%=$)",
        "Chdir(path=%)","Fcreate(fname=%=$,attr=&)","Fopen(fname=%=$,mode=&)","Fclose(handle=&)",
        "Fread(handle=&,count=%,buf=%)","Fwrite(handle=&,count=%,buf=%)","Fdelete(fname=%=$)",
        "Fseek(offest=%,handle=&,seekmode=&)","Fattrib(fname=%=$,flag=&,attrib=&)",
        "","Fdup(handle=&)","Fforce(stdh=&,nonstdh=&)","DgetPath(buf=%,drive=&)",
        "Malloc(%)","Mfree(addr=%)","Mshrink(dummy=&,block=%,newsize=%)","Pexec(mode=&,%=$,%,%)",
        "Pterm(retcode=&)","","Fsfirst(fnam=%=$,attr=&)","Fsnext","","","","","","",
        "Frename(dummy=&,oldname=%=$,newname=%=$)","Fdatime(timeptr=%,handle=&,flag=&)"};

  const char* bios_calls[12]={"GetMBP(pointer=%)","Bconstat(dev=&)","Bconin(dev=&)",
        "Bconout(dev=&,c=&)","Rwabs(rwflag=&,buffer=%,number=&,recno=&,dev=&), read/write disk sector",
        "Setexec(number=&,vector=%), set exception vector","Tickcal","Getbpb(dev=&)","Bcostat(dev=&)",
        "Mediach(dev=&)","Drvmap","Kbshift(mode=&)"};

  const char* xbios_calls[40]={"InitMouse(type=&,parameter=%,vector=%)","Ssbrk(number=%)",
        "Physbase","Logbase","Getrez","setscreen(log=%,phys=%,res=&)","Setpalette(ptr=%)",
        "Setcolor(colornum=&,color=&)","Floprd(buffer=%,filler=%,dev=&,sector=&,track=&,side=&,count=&)",
        "Flopwr(buffer=%,filler=%,dev=&,sector=&,track=&,side=&,count=&)",
        "Flopfmt(buffer=%,filler=%,dev=&,spt=&,track=&,side=&,interleave=&,magic=&,virgin=&)","",
        "Midiws(count=&,ptr=%)","Mfpint(number=&,vector=%)","Iorec(dev=&)",
        "Rsconf(baud=&,ctrl=&,ucr=&,rsr=&,tsr=&,scr=&)","Keytbl(unshift=%,shift=%,caps=%)",
        "random","protobt(buffer=%,serialno=%,disktype=&,execflag=&)",
        "Flopver(buffer=%,filler=%,dev=&,sector=&,track=&,side=&,count=&)","Scrdmp",
        "Cursconf(function=&,rate=&)","Settime(time=%)","Gettime","Bioskeys","Ikbdws(number=&,ptr=%)",
        "Jdisint(number=&)","Jenabint(number=&)","Giaccess(data=&,register=&)","Offgibit(bitnumber=&)",
        "Ongibit(bitnumber=&)","Xbtimer(timer=&,control=&,data=&,vector=%)","Dosound(pointer=%)",
        "Setprt(config=&)","Kbdvbase","Kbrate(delay=&,repeat=&)","Prtblk(parameter=%)",
        "Vsync","Supexec(%)","Puntaes"};


  FILE *logfile=NULL;
  EasyStr LogFileName;
#endif

#else
  #define log(s)
  #define logc(s)
  #define log_stack ;
#ifdef UNIX
  #define log_write(s) printf(s);printf("\n");
#else
  #define log_write(s)
#endif
  #define log_io_write(a,b)
  #define log_to_section(section,s)
  #define log_to(section,s)
  #define log_write_stack() ;
  #define LOG_CPU
#endif

#define log_DELETE_SOON(s) log_write(s);

EXT EasyStr HEXSl(long,int);
EXT int count_bits_set_in_word(unsigned short w);

EXT EasyStr read_string_from_memory(MEM_ADDRESS,int);
EXT MEM_ADDRESS write_string_to_memory(MEM_ADDRESS,char*);
EXT MEM_ADDRESS get_sp_before_trap(bool* DEFVAL(NULL));
EXT void acc_parse_search_string(Str,DynamicArray<BYTE>&,bool&);
EXT MEM_ADDRESS acc_find_bytes(DynamicArray<BYTE> &,bool,MEM_ADDRESS,int);

EXT bool STfile_read_error INIT(0);

EXT WORD STfile_read_word(FILE*f);
EXT LONG STfile_read_long(FILE*f);
EXT void STfile_read_to_ST_memory(FILE*f,MEM_ADDRESS ad,int n_bytes);
EXT void STfile_write_from_ST_memory(FILE*f,MEM_ADDRESS ad,int n_bytes);
EXT long colour_convert(int,int,int);

#ifdef ENABLE_LOGFILE
EXT Str scanline_cycle_log();
#endif

#ifdef IN_MAIN
void GetTOSKeyTableAddresses(MEM_ADDRESS *,MEM_ADDRESS *);
EasyStr time_or_times(int n);

char *reg_name(int n);
int get_text_width(char*t);

MEM_ADDRESS oi(MEM_ADDRESS,int);

int how_big_is_0000;

char _reg_name_buf[8];

int file_read_num(FILE*);

EasyStr DirID_to_text(int ID,bool st_key);

bool has_extension_list(char*,char*,...);
bool has_extension(char*,char*);
bool MatchesAnyString(char *,char *,...);
bool MatchesAnyString_I(char *,char *,...);

EasyStr GetEXEDir();
EasyStr GetCurrentDir();
EasyStr GetEXEFileName();

#endif

#undef EXT
#undef INIT

