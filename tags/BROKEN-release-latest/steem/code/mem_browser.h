

enum type_disp_type{DT_INSTRUCTION=0,DT_MEMORY,DT_REGISTERS};// disp_type;
enum type_mode{MB_MODE_STANDARD=0,MB_MODE_PC,MB_MODE_STACK,MB_MODE_FIXED,MB_MODE_IOLIST};//mode;

class mr_static;

class mem_browser
{
  static LRESULT __stdcall FindEditWndProc(HWND,UINT,UINT,long);
  static WNDPROC OldEditWndProc;
  void listbox_add_line(HWND,int,char**,int);
  char* get_mem_mon_string(WORD,bool);
public:
//  bool active;
  HWND owner;
  HWND handle;
  type_disp_type disp_type;
  MEM_ADDRESS ad;
  type_mode mode;
  int wpl,lb_height,columns,text_column;
  bool editflag,init_text;
  mr_static *editbox;

  mem_browser(){
    owner=NULL;handle=NULL;disp_type=DT_MEMORY;ad=0;mode=MB_MODE_STANDARD;
    wpl=1;lb_height=3;editflag=true;editbox=NULL;columns=0;init_text=0;
  }
  mem_browser(MEM_ADDRESS new_ad,type_disp_type dt){
    init_text=0;columns=0;new_window(new_ad,dt);
  }
  void new_window(MEM_ADDRESS,type_disp_type);
  void update();
  void init();
  void draw(DRAWITEMSTRUCT*);
  int calculate_wpl();
  void get_breakpoint_labels(MEM_ADDRESS ad,int bpl,char*t[3]);
  MEM_ADDRESS get_address_from_row(int row);
  void vscroll(int of);
  void setup_contextmenu(int row,int col);
  void update_icon();

  ~mem_browser();

  static DWORD ex_style;
};
WNDPROC mem_browser::OldEditWndProc;
DWORD mem_browser::ex_style=WS_EX_TOOLWINDOW;


#define MAX_MEMORY_BROWSERS 20
mem_browser *m_b[MAX_MEMORY_BROWSERS];

char reg_browser_entry_name[20][8];
unsigned long *reg_browser_entry_pointer[20];
#define NUM_REGISTERS_IN_REGISTER_BROWSER 18

