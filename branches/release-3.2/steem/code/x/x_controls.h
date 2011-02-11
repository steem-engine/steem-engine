class hxc_buttonpicker;

#define BPN_CHANGE 1
#define BPN_FOCUSCHANGE 2

typedef int (HXC_BUTTONPICKERNOTIFYPROC)(hxc_buttonpicker*,int,int);
typedef HXC_BUTTONPICKERNOTIFYPROC* LPHXC_BUTTONPICKERNOTIFYPROC;

class hxc_buttonpicker : public hxc
{
private:
  static int WinProc(hxc_buttonpicker*,Window,XEvent*);
  static void delete_hxc_buttonpicker(hxc *p){ delete (hxc_buttonpicker*)p; }
  static int timer_notify_proc(void*,Window,int);
public:
  hxc_buttonpicker() { init(); }
  hxc_buttonpicker(Display *_d,Window _p,int _x,int _y,int _w,int _h,
  						LPHXC_BUTTONPICKERNOTIFYPROC np,void *o,int i)
  {
    dynamically_allocated=true;
    init();
    create(_d,_p,_x,_y,_w,_h,np,o,i);
  }

  ~hxc_buttonpicker() {  }

  void init();
  bool create(Display*,Window,int,int,int,int,LPHXC_BUTTONPICKERNOTIFYPROC,void*,int);
  static void destroy(hxc_buttonpicker*);
  void draw(bool=true);
  static int key_to_modbit(KeySym);
  static void get_joystick_down(DWORD*,DWORD*);
  static WORD axis_mask_to_dir_id(int,DWORD);
  static WORD button_mask_to_dir_id(int,DWORD);

  EasyStr text;
  int mod_down;

  operator Window() { return handle; }

  int DirID;
  bool st_keys_only,allow_joy;
  LPHXC_BUTTONPICKERNOTIFYPROC notifyproc;
  static hxc_listview lv;
  static DWORD old_joy_axis_down[MAX_PC_JOYS],old_joy_button_down[MAX_PC_JOYS];
};
hxc_listview hxc_buttonpicker::lv;
DWORD hxc_buttonpicker::old_joy_axis_down[MAX_PC_JOYS],hxc_buttonpicker::old_joy_button_down[MAX_PC_JOYS];

