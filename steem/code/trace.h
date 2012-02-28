#define TDE_BEFORE 1
#define TDE_AFTER 2
#define TDE_TEXT_ONLY 0x80

typedef struct{
  short when;
  bool regflag;
  MEM_ADDRESS ad;
  char name[100];
  int bytes;
  unsigned long val[2];
  unsigned long*ptr;
}trace_display_entry;

#define MAX_TRACE_DISPLAY_ENTRIES 50
trace_display_entry t_d_e[MAX_TRACE_DISPLAY_ENTRIES];
unsigned short trace_sr_before,trace_sr_after;
MEM_ADDRESS trace_pc;
int trace_entries=0;
mem_browser m_b_trace;
HWND trace_window_handle;
HWND trace_repeat_trace_button;
HWND trace_hLABEL[MAX_TRACE_DISPLAY_ENTRIES];
HWND trace_sr_before_display,trace_sr_after_display;
ScrollControlWin trace_scroller;
bool trace_show_window=true;

LRESULT __stdcall trace_window_WndProc(HWND,UINT,WPARAM,LPARAM);
void trace_window_init();
void trace_init();
void trace_add_entry(char*name1,char*name2,short when,bool regflag,
    int bytes,MEM_ADDRESS ad);
void trace_add_movem_block(char*name,int,short when,int bytes,MEM_ADDRESS ad,int count);
void trace_get_after();
void trace_display();
void trace();
void trace_exception_display(m68k_exception*exc);
void trace_add_text(char*tt);


const char*bombs_name[12]={"SSP after reset","PC after reset","bus error","address error",
                      "illegal instruction","division by zero","CHK instruction",
                      "TRAPV instruction","Privilege violation","Trace","Line-A","Line-F"};

const char*exception_action_name[4]={"read from","write to","fetch from","instruction execution"};


