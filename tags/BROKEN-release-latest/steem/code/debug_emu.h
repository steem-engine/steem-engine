#ifdef IN_EMU
#define EXT
#define INIT(s) =s
extern EasyStr disa_d2(MEM_ADDRESS);
inline MEM_ADDRESS monitor_check();
inline void breakpoint_check();
#else
#define EXT extern
#define INIT(s)
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
EXT void debug_check_monitors();
EXT void debug_check_for_events();
EXT void debug_trace_event_plan_init();
EXT void debug_check_io_monitor(MEM_ADDRESS,bool,BYTE=0);

EXT void iolist_debug_add_pseudo_addresses();

#define CYCLE_COL_SPEED 4
EXT int debug_cycle_colours INIT(0);
EXT int debug_screen_shift INIT(0);

#define MAX_BREAKPOINTS 30
EXT int num_breakpoints INIT(0),num_monitors INIT(0),num_io_monitors INIT(0);
EXT MEM_ADDRESS breakpoint[MAX_BREAKPOINTS],monitor_ad[MAX_BREAKPOINTS],monitor_io_ad[MAX_BREAKPOINTS];
EXT WORD monitor_contents[MAX_BREAKPOINTS],monitor_mask[MAX_BREAKPOINTS],monitor_io_mask[MAX_BREAKPOINTS];
EXT bool monitor_io_readflag[MAX_BREAKPOINTS];
EXT MEM_ADDRESS trace_over_breakpoint INIT(0xffffffff);

#define DRU_OFF 0
#define DRU_VBL 1
#define DRU_SCANLINE 2
#define DRU_CYCLE 3

EXT int debug_run_until INIT(DRU_OFF),debug_run_until_val;

EXT int monitor_mode INIT(1),breakpoint_mode INIT(1);
EXT bool do_monitor_check INIT(0),do_breakpoint_check INIT(0);

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

#undef EXT
#undef INIT
//---------------------------------------------------------------------------

