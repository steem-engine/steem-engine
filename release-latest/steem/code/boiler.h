MEM_ADDRESS dpc,old_dpc;

HWND DWin=NULL,HiddenParent=NULL;
HMENU menu,breakpoint_menu,monitor_menu,breakpoint_irq_menu;
HMENU insp_menu=NULL;
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

void debug_load_file_to_address(HWND,MEM_ADDRESS);

Str debug_parse_disa_for_display(Str);
bool debug_monospace_disa=0,debug_uppercase_disa=0;

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
typedef struct{
  MEM_ADDRESS ad;
  int mode; // 0=off, 1=global, 2=break, 3=log
  int bwr; // & 1=break, & 2=write, & 4=read
  WORD mask[2]; // write mask, read mask
  char name[64];
}DEBUG_ADDRESS;

DEBUG_ADDRESS *debug_find_address(MEM_ADDRESS);
void debug_remove_address(MEM_ADDRESS);
void debug_update_bkmon();
void breakpoint_menu_setup();
void logfile_wipe();
void debug_set_bk(MEM_ADDRESS,bool);
void debug_set_mon(MEM_ADDRESS,bool,WORD);

DynamicArray<DEBUG_ADDRESS> debug_ads;

/////////////////////////////// logfile  ////////////////////////////////////

//////////////////////////////// routines //////////////////////////////////
void update_register_display(bool);
void disa_to_file(FILE*f,MEM_ADDRESS dstart,int dlen,bool);
//---------------------------------------------------------------------------
THistoryList HistList;

void debug_plugin_load(),debug_plugin_free();



