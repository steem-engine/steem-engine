LRESULT __stdcall mr_static_WndProc(HWND,UINT,UINT,long);

enum _mr_static_type{MST_MEMORY=0,MST_REGISTER,MST_HISTORIC_MEMORY,MST_MEM_BROWSER_ADDRESS,
                    MST_ADDRESS,MST_IOLIST,MST_HISTORIC_IOLIST,MST_DECIMAL};

class mr_static{
public:
  bool active;
  HWND owner;
  HWND handle;
  HWND hLABEL;
  _mr_static_type type;
  char name[128];
  long*ptr;
  MEM_ADDRESS ad;
  int bytes;
  bool editflag;
  mem_browser *mem_browser_update;
  iolist_entry *ile;

  mr_static(char*label,char*name,int x,int y,
      HWND owner,HMENU id,MEM_ADDRESS ad,int bytes,_mr_static_type type,
      bool editflag,mem_browser*mem_browser_update);
  void update();
  void edit(char*ttt);
  void setup_contextmenu();

  ~mr_static();
};

#define MAX_MR_STATICS 120
mr_static *m_s[MAX_MR_STATICS];

void mr_static_update_all();

