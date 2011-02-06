MEM_ADDRESS dpc,old_dpc;

HWND DWin=NULL,HiddenParent=NULL;
HMENU menu,breakpoint_menu,monitor_menu,breakpoint_irq_menu;
HMENU insp_menu=NULL,insp_menu_reg_submenu[3];
HMENU mem_browser_menu,history_menu,logsection_menu;
HMENU menu1;
HMENU boiler_op_menu,shift_screen_menu;
HWND sr_display,DWin_edit;
mr_static *lpms_other_sp;
HWND DWin_trace_button,DWin_trace_over_button,DWin_run_button;

Str LogViewProg="notepad.exe";

void boiler_show_stack_display(int);
ScrollControlWin DWin_timings_scroller;
HWND DWin_right_display_combo;

WNDPROC Old_sr_display_WndProc;

mem_browser m_b_mem_disa,m_b_stack;

#define SIMULTRACE_CHOOSE ((HWND)(0xffffffff))
HWND simultrace=NULL;

bool d2_trace=false;

/////////////////////////////// insp menu ////////////////////////////////////
int insp_menu_subject_type;
void* insp_menu_subject;
long insp_menu_long[3];
char insp_menu_long_name[3][100];
int insp_menu_long_bytes[3];
void insp_menu_setup();
int insp_menu_col,insp_menu_row;

/////////////////////////////// breakpoints ////////////////////////////////////
int get_breakpoint_or_monitor(bool,MEM_ADDRESS);
void remove_breakpoint_or_monitor(bool,MEM_ADDRESS);
void set_breakpoint_or_monitor(bool,MEM_ADDRESS,WORD=0xffff,bool=0);
void breakpoint_menu_setup();
void logfile_wipe();

void breakpoint_menu_setup();
void logfile_wipe();

#define UPDATE_DO_MON_CHECK do_monitor_check=(monitor_mode>0 && num_monitors);
#define UPDATE_DO_BREAK_CHECK do_breakpoint_check=(breakpoint_mode>0 && num_breakpoints);

/////////////////////////////// logfile  ////////////////////////////////////

//////////////////////////////// routines //////////////////////////////////
void update_register_display(bool);
void disa_to_file(FILE*f,MEM_ADDRESS dstart,int dlen,bool);
//---------------------------------------------------------------------------
THistoryList HistList;


