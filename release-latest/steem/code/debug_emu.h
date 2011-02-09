#ifdef IN_EMU
#define EXT
#define INIT(s) =s
extern EasyStr disa_d2(MEM_ADDRESS);
void breakpoint_check();
extern int debug_get_ad_mode(MEM_ADDRESS);
extern WORD debug_get_ad_mask(MEM_ADDRESS,bool);
#else
#define EXT extern
#define INIT(s)
#ifdef IN_MAIN
int debug_get_ad_mode(MEM_ADDRESS);
WORD debug_get_ad_mask(MEM_ADDRESS,bool);
#endif
#endif

EXT bool debug_in_trace INIT(0),debug_wipe_log_on_reset;
EXT bool redraw_on_stop INIT(0),redraw_after_trace INIT(0);
EXT COLORREF debug_gun_pos_col INIT( RGB(255,0,0) );

EXT void debug_update_cycle_counts();
EXT void debug_update_drawing_position(int* DEFVAL(NULL));
EXT void update_display_after_trace();

#define CRASH_NOTIFICATION_ALWAYS 0
#define CRASH_NOTIFICATION_BOMBS_DISPLAYED 1
#define CRASH_NOTIFICATION_NEVER 2

EXT void stop_new_program_exec();
EXT void debug_check_break_on_irq(int);
EXT void debug_run_start(),debug_run_end();
EXT void debug_trace_crash(m68k_exception &);
EXT void debug_vbl();
EXT void logfile_wipe();
EXT void debug_reset();

EXT int crash_notification INIT(CRASH_NOTIFICATION_BOMBS_DISPLAYED);
EXT bool stop_on_blitter_flag INIT(false);
EXT bool stop_on_ipl_7 INIT(false);
EXT int stop_on_user_change INIT(0);
EXT int stop_on_next_program_run INIT(0);
EXT bool debug_first_instruction INIT(0);
EXT Str runstate_why_stop;
EXT DWORD debug_cycles_since_VBL,debug_cycles_since_HBL;
EXT MEM_ADDRESS debug_VAP;
EXT int debug_time_to_timer_timeout[4];
EXT void debug_check_for_events();
EXT void debug_trace_event_plan_init();

EXT void iolist_debug_add_pseudo_addresses();

#define CYCLE_COL_SPEED 4
EXT int debug_cycle_colours INIT(0);
EXT int debug_screen_shift INIT(0);

//---------------------------------------------------------------------------
EXT void debug_hit_mon(MEM_ADDRESS,int);
EXT void debug_hit_io_mon_write(MEM_ADDRESS,int);

#define DEBUG_CHECK_WR_B(ad,num,adarr,maskarr,hit,wr) \
  if (num){ \
    WORD mask=WORD((ad & 1) ? 0x00ff:0xff00); \
    MEM_ADDRESS test_ad=ad & ~1;  \
    for (int i=0;i<num;i++){ \
      if (adarr[i]==test_ad){ \
        if (mask & maskarr[i]){ \
          hit(ad,wr); \
          break; \
        }   \
      }         \
    }            \
  }

#define DEBUG_CHECK_WR_W(ad,num,adarr,hit,wr) \
  if (num){ \
    for (int i=0;i<num;i++){ \
      if (adarr[i]==ad){ \
        hit(ad,wr); \
        break; \
      }         \
    }            \
  }

#define DEBUG_CHECK_WR_L(ad,num,adarr,hit,wr) \
  if (num){ \
    for (int i=0;i<num;i++){ \
      if (adarr[i]==ad){ \
        hit(ad,wr); \
        break; \
      }         \
      if (adarr[i]==ad+2){ \
        hit(ad+2,wr); \
        break; \
      }         \
    }            \
  }


#define DEBUG_CHECK_WRITE_B(ad) DEBUG_CHECK_WR_B(ad,debug_num_mon_writes,debug_mon_write_ad,debug_mon_write_mask,debug_hit_mon,0)
#define DEBUG_CHECK_WRITE_W(ad) DEBUG_CHECK_WR_W(ad,debug_num_mon_writes,debug_mon_write_ad,debug_hit_mon,0)
#define DEBUG_CHECK_WRITE_L(ad) DEBUG_CHECK_WR_L(ad,debug_num_mon_writes,debug_mon_write_ad,debug_hit_mon,0)
#define DEBUG_CHECK_READ_B(ad) DEBUG_CHECK_WR_B(ad,debug_num_mon_reads,debug_mon_read_ad,debug_mon_read_mask,debug_hit_mon,1)
#define DEBUG_CHECK_READ_W(ad) DEBUG_CHECK_WR_W(ad,debug_num_mon_reads,debug_mon_read_ad,debug_hit_mon,1)
#define DEBUG_CHECK_READ_L(ad) DEBUG_CHECK_WR_L(ad,debug_num_mon_reads,debug_mon_read_ad,debug_hit_mon,1)

#define DEBUG_CHECK_WRITE_IO_B(ad,v) DEBUG_CHECK_WR_B(ad,debug_num_mon_writes_io,debug_mon_write_ad_io,debug_mon_write_mask_io,debug_hit_io_mon_write,v)
#define DEBUG_CHECK_WRITE_IO_W(ad,v) DEBUG_CHECK_WR_W(ad,debug_num_mon_writes_io,debug_mon_write_ad_io,debug_hit_io_mon_write,v)
#define DEBUG_CHECK_WRITE_IO_L(ad,v) DEBUG_CHECK_WR_L(ad,debug_num_mon_writes_io,debug_mon_write_ad_io,debug_hit_io_mon_write,v)
#define DEBUG_CHECK_READ_IO_B(ad) DEBUG_CHECK_WR_B(ad,debug_num_mon_reads_io,debug_mon_read_ad_io,debug_mon_read_mask_io,debug_hit_mon,1)
#define DEBUG_CHECK_READ_IO_W(ad) DEBUG_CHECK_WR_W(ad,debug_num_mon_reads_io,debug_mon_read_ad_io,debug_hit_mon,1)
#define DEBUG_CHECK_READ_IO_L(ad) DEBUG_CHECK_WR_L(ad,debug_num_mon_reads_io,debug_mon_read_ad_io,debug_hit_mon,1)

#define MAX_BREAKPOINTS 30
EXT int debug_num_bk INIT(0),debug_num_mon_reads INIT(0),debug_num_mon_writes INIT(0);
EXT int debug_num_mon_reads_io INIT(0),debug_num_mon_writes_io INIT(0);

EXT MEM_ADDRESS debug_bk_ad[MAX_BREAKPOINTS],debug_mon_read_ad[MAX_BREAKPOINTS],debug_mon_write_ad[MAX_BREAKPOINTS];
EXT MEM_ADDRESS debug_mon_read_ad_io[MAX_BREAKPOINTS],debug_mon_write_ad_io[MAX_BREAKPOINTS];
EXT WORD debug_mon_read_mask[MAX_BREAKPOINTS],debug_mon_write_mask[MAX_BREAKPOINTS];
EXT WORD debug_mon_read_mask_io[MAX_BREAKPOINTS],debug_mon_write_mask_io[MAX_BREAKPOINTS];
//---------------------------------------------------------------------------
EXT MEM_ADDRESS trace_over_breakpoint INIT(0xffffffff);

#define DRU_OFF 0
#define DRU_VBL 1
#define DRU_SCANLINE 2
#define DRU_CYCLE 3
#define DRU_INSTCHANGE 4

EXT int debug_run_until INIT(DRU_OFF),debug_run_until_val;

EXT int monitor_mode INIT(2),breakpoint_mode INIT(2);

#define BREAK_IRQ_HBL_IDX 16
#define BREAK_IRQ_VBL_IDX 17
#define BREAK_IRQ_LINEA_IDX 18
#define BREAK_IRQ_LINEF_IDX 19
#define BREAK_IRQ_TRAP_IDX 20
#define NUM_BREAK_IRQS 21

#ifdef IN_EMU
bool break_on_irq[NUM_BREAK_IRQS]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#else
extern bool break_on_irq[NUM_BREAK_IRQS];
#endif

#define HISTORY_SIZE 1000
#define HIST_MENU_SIZE 20

EXT MEM_ADDRESS pc_history[HISTORY_SIZE];
EXT int pc_history_idx INIT(0);
EXT BYTE debug_send_alt_keys INIT(0),debug_send_alt_keys_vbl_countdown INIT(0);

EXT int __stdcall debug_plugin_read_mem(DWORD,BYTE*,int);
EXT int __stdcall debug_plugin_write_mem(DWORD,BYTE*,int);

#ifdef IN_MAIN
typedef void __stdcall DEBUGPLUGIN_INITPROC(void**,char*);
typedef void __stdcall DEBUGPLUGIN_ACTIVATEPROC(int);
typedef void __stdcall DEBUGPLUGIN_CLOSEPROC();

typedef struct{
  HINSTANCE hDll;
  DEBUGPLUGIN_INITPROC *Init;
  DEBUGPLUGIN_ACTIVATEPROC *Activate;
  DEBUGPLUGIN_CLOSEPROC *Close;
  char Menu[512];
}DEBUGPLUGININFO;

void *debug_plugin_routines[]={(void*)2,(void*)debug_plugin_read_mem,(void*)debug_plugin_write_mem};
DynamicArray<DEBUGPLUGININFO> debug_plugins;
#endif

#undef EXT
#undef INIT
//---------------------------------------------------------------------------

