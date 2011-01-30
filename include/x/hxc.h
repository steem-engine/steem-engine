#ifndef _HXC_CONTROLS_H
#define _HXC_CONTROLS_H

#ifndef UNIX
#define UNIX
#endif

#include <dynamicarray.h>
#include <easystr.h>
#include <easystringlist.h>
#include <mymisc.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <x/icongroup.h>

#define Button123Mask (Button1Mask | Button2Mask | Button3Mask)

typedef int WINDOWPROC(void*,Window,XEvent*);
typedef WINDOWPROC* LPWINDOWPROC;
//---------------------------------------------------------------
class hxc_listview;
class hxc_scrollbar;
class hxc_button;
class hxc_dropdown;
class hxc_edit;
class hxc_scrollarea;
typedef int HXC_LISTVIEWNOTIFYPROC(hxc_listview*,int,int);
typedef int HXC_SCROLLBARNOTIFYPROC(hxc_scrollbar*,int,int);
typedef int HXC_BUTTONNOTIFYPROC(hxc_button*,int,int*);
typedef int HXC_DROPDOWNNOTIFYPROC(hxc_dropdown*,int,int);
typedef int HXC_EDITNOTIFYPROC(hxc_edit*,int,int);
typedef int HXC_SCROLLAREANOTIFYPROC(hxc_scrollarea*,int,int);

typedef HXC_LISTVIEWNOTIFYPROC* LPHXC_LISTVIEWNOTIFYPROC;
typedef HXC_SCROLLBARNOTIFYPROC* LPHXC_SCROLLBARNOTIFYPROC;
typedef HXC_BUTTONNOTIFYPROC* LPHXC_BUTTONNOTIFYPROC;
typedef HXC_DROPDOWNNOTIFYPROC* LPHXC_DROPDOWNNOTIFYPROC;
typedef HXC_EDITNOTIFYPROC* LPHXC_EDITNOTIFYPROC;
typedef HXC_SCROLLAREANOTIFYPROC* LPHXC_SCROLLAREANOTIFYPROC;
//---------------------------------------------------------------
//---------------------------------------------------------------
class hxc;

typedef void HXC_DESTROYPROC(void*);
typedef HXC_DESTROYPROC* LPHXC_DESTROYPROC;

typedef void HXC_DELETEPROC(void*);
typedef HXC_DELETEPROC* LPHXC_DELETEPROC;

typedef int HXC_TIMERPROC(void*,Window,int);
typedef HXC_TIMERPROC* LPHXC_TIMERPROC;

typedef struct hxc_timer_type{
  DWORD time;
  Window win;
  int interval,id;
  LPHXC_TIMERPROC notifyproc;
  void *data;
  hxc_timer_type *next,*prev;
}hxc_timer;


#define HXC_TIMER_ALL_IDS -31971
#define HXC_TIMER_ALL_WINS 56712851

#define HXC_TIMER_REPEAT 0
#define HXC_TIMER_STOP 1

class hxc
{
private:
public:
  static bool SetProp(Display *XD,Window Win,XContext Prop,DWORD Val);
  static DWORD GetProp(Display *XD,Window Win,XContext Prop);
  static void RemoveProp(Display *XD,Window Win,XContext Prop);
  static DWORD GetColourValue(Display *XD,WORD R,WORD G,WORD B,DWORD Default);
  static void draw_triangle(Display*,Drawable,GC,int,int,int,int);
  static void draw_border(Display* XD,Drawable handle,GC gc,
                      int x,int y,int w,int h,int,DWORD col_top,DWORD col_bottom);
	static void send_event(Display*,Window,Atom,long=0,long=0,long=0,long=0,long=0);
	static void modal_children(Display*,Window,Window);
	static bool suppress_mess_for_modal(Display*,XEvent*);

  static hxc* find(Window,int);

  static void destroy_children_of(Window);
  static void default_destroy_proc(void*){return;}
  static void default_delete_proc(void*)
  {
    printf("HXC problem: Someone's forgotton to set a delete proc! Memory leak!\n");
  }
  static void destroy(hxc*);

  static void clip_to_expose_rect(Display*,XExposeEvent*,GC=0);

//  static XContext cWinProc,cWinThis;
//  static struct hxc_res_struct res;
//  res!!!!!
  static int res_users;
  static EasyStringList font_sl;
  static XFontStruct *font;
  static DWORD col_grey,col_black,col_white,col_border_dark,col_border_light,
        col_bk,col_sel_back,col_sel_fore;
  static GC gc;
  static Colormap colormap;
  static Cursor arrow_cursor;

	static XContext cModal;
  static Atom XA_WM_PROTOCOLS,XA_WM_DELETE_WINDOW;
//

  static void load_res(Display*);
  static void free_res(Display*);
  static void alloc_colours(Display*);
  static void free_colours(Display*);
  static void (*alloc_colours_vector)(Display*);
  static void (*free_colours_vector)(Display*);

  static int get_text_width(Display*,char*t);
	void common_winproc(XEvent*);

  static Window create_modal_dialog(Display*,int,int,char*,bool);
  static int modal_dialog_winproc(Display*,Window,XEvent*);
  static int modal_but_np(hxc_button*,int,int*);
  static int show_modal_dialog(Display*,Window,bool,Window=0);
  static void destroy_modal_dialog(Display*,Window);

  static int modal_result;
  static Window modal_focus_win;
  static void (*modal_notifyproc)(bool);

  static Window popup_active;

  void *owner;
  Window parent;
  Window handle;
  Display *XD;
  int id;
  bool dynamically_allocated;

  int x,y,w,h;

  bool can_have_children;
  LPHXC_DESTROYPROC destroyproc;
  LPHXC_DELETEPROC deleteproc;
  hxc *previous_hxc,*next_hxc;
  hxc();
  ~hxc();

  static hxc *first_hxc,*last_hxc;

  static hxc_timer *first_timer,*last_timer;

  static void set_timer(Window,int,int,LPHXC_TIMERPROC,void *);
  static void kill_hxc_timer(hxc_timer *);
  static void kill_timer(Window,int);
  static void kill_all_timers();
  static void check_timers();
  static bool wait_for_event(Display*,XEvent*,DWORD=0);
};
//---------------------------------------------------------------------------
//------------------------------ Button/Static ------------------------------
//---------------------------------------------------------------------------
#define BT_TEXT             1
#define BT_ICON             2
#define BT_STATIC           4
#define BT_UPDOWNNOTIFY     8
#define BT_TOGGLE          16
#define BT_LINK            32
#define BT_HOLDREPEAT      64
#define BT_NOBACKGROUND    128
#define BT_CHECK           256

#define BT_GROUPBOX (BT_STATIC | BT_TEXT | BT_BORDER_OUTDENT | BT_TEXT_VTOP)
#define BT_LABEL    (BT_STATIC | BT_TEXT | BT_BORDER_NONE | BT_TEXT_LEFT)
#define BT_STATICICON (BT_STATIC | BT_ICON | BT_BORDER_NONE)
#define BT_CHECKBOX   (BT_CHECK | BT_STATIC | BT_TEXT | BT_BORDER_NONE | BT_TEXT_LEFT | BT_TOGGLE)

#define BT_OBJECTPOS_HMASK 0x00030000
#define BT_OBJECTPOS_VMASK 0x000c0000

#define BT_TEXT_CENTRE    0
#define BT_TEXT_LEFT      0x00010000
#define BT_TEXT_RIGHT     0x00020000
#define BT_TEXT_PATH      0x00030000
#define BT_TEXT_VCENTRE   0
#define BT_TEXT_VTOP      0x00040000
#define BT_TEXT_VBOTTOM   0x00080000

#define BT_ICON_CENTRE    0
#define BT_ICON_LEFT      0x00010000
#define BT_ICON_RIGHT     0x00020000
#define BT_ICON_VCENTRE   0
#define BT_ICON_VTOP      0x00040000
#define BT_ICON_VBOTTOM   0x00080000

#define BT_BORDER_MASK    0x00f00000
#define BT_BORDER_NONE    0
#define BT_BORDER_INDENT  0x00200000
#define BT_BORDER_OUTDENT 0x00100000

#define BN_CLICKED 1
#define BN_DOWN 2
#define BN_UP 3
#define BN_MOTION 4

class hxc_button:public hxc
{
private:
  static int WinProc(hxc_button*,Window,XEvent*);
  static void delete_hxc_button(hxc *p){ delete (hxc_button*)p; }
  int get_border_state();

  bool MouseIn;
  unsigned long MouseDown;
  Atom button_timer_atom;
	clock_t next_move_time;
  Pixmap drop_shadow_bitmap;
public:
  LPHXC_BUTTONNOTIFYPROC notifyproc;
  int type,icon_index;
  EasyStr text;
  bool checked;
  IconGroup *picons;
  bool want_drag_notify;
  DWORD col_text,col_background;

  static IconGroup *pcheck_ig;
  static int check_on_icon,check_off_icon;

  XFontStruct *but_font;

  hxc_button() { init(); }
  hxc_button(void *o) { owner=o;init(); }
  hxc_button(Display *_d,Window _p,int _x,int _y,int _w,int _h,LPHXC_BUTTONNOTIFYPROC np,void *o,
            		int tp,char *txt,int i,DWORD bk,XFontStruct *fs=NULL)
  {
    dynamically_allocated=true;
    init();
    create(_d,_p,_x,_y,_w,_h,np,o,tp,txt,i,bk,fs);
  }
  void init();

  ~hxc_button() {};

  bool create(Display*,Window,int,int,int,int,LPHXC_BUTTONNOTIFYPROC,void*,
            		int,char*,int,DWORD,XFontStruct* = NULL);
  void set_check(bool);
  void set_text(char*);
  void set_icon(IconGroup*,int);
  static void destroy(hxc_button*);
  void draw() { WinProc(this,handle,NULL); }

  operator Window() { return handle; }
};
//---------------------------------------------------------------
//---------------------------------------------------------------
#define SBN_SCROLL 0
#define SBN_SCROLLBYONE 1

#define HXC_SCROLLBAR_WIDTH 12

class hxc_scrollbar:public hxc
{
private:
  static int WinProc(hxc_scrollbar*,Window,XEvent*);
  static int button_notify_proc(hxc_button*,int,int*);
  static void delete_hxc_scrollbar(hxc *p){ delete (hxc_scrollbar*)p; }
public:
  int arrowheight;
  int range,pos,viewrange;
  int ty,th;
  int dragstage,drag_y;
  LPHXC_SCROLLBARNOTIFYPROC notifyproc;
  bool horizontal;
  hxc_button UpBut,DownBut;

  hxc_scrollbar();
  ~hxc_scrollbar(){};
  bool create(Display*,Window,int,int,int,int,LPHXC_SCROLLBARNOTIFYPROC=NULL);
  bool create(Display *_d,Window _p,int _x,int _y,int _w,int _h,LPHXC_SCROLLBARNOTIFYPROC np,void *o){
    owner=o;
    return create(_d,_p,_x,_y,_w,_h,np);
  }
  void init(int,int,int);
  void rangecheck();
  void draw();
  static void destroy(hxc_scrollbar*);
  void notify_reposition(int);
  void do_drag(int y,bool by_top); //pass the y-position of the top of the scrolling box, or the middle

  operator Window() { return handle; }
};
//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
#define LVN_SELCHANGE 1
#define LVN_RETURN 2
#define LVN_SINGLECLICK 3
#define LVN_DOUBLECLICK 4
#define LVN_CANTDROP 6
#define LVN_DROP 7
#define LVN_ICONCLICK 8
#define LVN_CONTEXTMENU 9
#define LVN_KEYPRESS 10
#define LVN_CANTDRAG 11

#define LVN_CB_RETRACT 5

#define LVTTM_ELLIPSIS 1
#define LVTTM_CUT 2

#define LV_SB_W 12
#define LV_SB_BUT_H 12

typedef struct{
	int dx,dy,on,dragged;
	unsigned int button;
	bool in_lv;
}hxc_listview_drop_struct;

class hxc_listview:public hxc
{
private:
  static int WinProc(hxc_listview*,Window,XEvent*);
  static void delete_hxc_listview(hxc *p){ delete (hxc_listview*)p; }

  static Atom sb_but_timer_atom;
  static clock_t sb_but_down_time;
  Pixmap text_pix;
  GC text_gc;
public:
//  Display *XD;
//  Window handle,parent;
//  hxc_scrollbar scrollbar;
  bool has_scrollbar;
  int sb_up_state,sb_down_state,sb_drag_y,sb_drag_bar_y,sb_but_down;

  void draw_scrollbar(int,int);
  bool scrollbar_process_event(XEvent*);
  void sb_calc_bar_pos(int,int &,int &);

  int sx,sy; //scroll pos
  int sel;
  EasyStringList sl;
  int itemheight;
  int border;
  Time last_click_time;
  int last_click_x,last_click_y;
  int display_mode;
  IconGroup *lpig;
	DWORD col_bg;

	DynamicArray<int> columns;
  int text_trunc_mode;

	bool allow_drag,checkbox_mode;
	int dragging;
	unsigned int drag_button;
	Cursor drag_cursor;
	int clicked_in;
	bool in_combo;

  EasyStr searchtext;
  Time searchtext_timeout;
  XComposeStatus searchtext_xcompose_status;

  LPHXC_LISTVIEWNOTIFYPROC notifyproc;
  static int scrollbar_notify_proc(hxc_scrollbar*,int,int);
  hxc_listview();
  ~hxc_listview() {  }
  bool create(Display*,Window,int,int,int,int,LPHXC_LISTVIEWNOTIFYPROC=NULL);
  bool create(Display *_d,Window _p,int _x,int _y,int _w,int _h,LPHXC_LISTVIEWNOTIFYPROC np,void *o){
    owner=o;
    return create(_d,_p,_x,_y,_w,_h,np);
  }
  static void destroy(hxc_listview*);

  int get_max_width(Display*);
  void additem(char*);
  void additem(char*,int);
  void draw(bool,bool=true,bool=true);
  void changesel(int);
  void highlightitem(int,int);
  void draw_item(int,int,int,unsigned long,unsigned long);
	void make_item_visible(int);
  void scrollto(int);
  void handle_keypress(Display*,XKeyEvent*);
  int match_string(int,char*,bool);
  int select_item_by_data(int,int=0);
  void contents_change();
  int getitemsperscreen();

	void drag_start(int),drag_move(int,int),drag_end(int,int);
	bool is_dropped_in(hxc_listview_drop_struct *,hxc *);

  operator Window() { return handle; }
};
//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
#define DDN_SELCHANGE 1
#define DDN_DROPWHERE 2

typedef struct{
	Window parent;
	int x,y,w,h;
}dd_drop_position;

class hxc_dropdown:public hxc
{
private:
  static int WinProc(hxc_dropdown*,Window,XEvent*);
  static void delete_hxc_dropdown(hxc *p){ delete (hxc_dropdown*)p; }

//  GC gc;
//  DWORD col_grey,col_black,col_white,col_border_dark,col_border_light;
public:
//  Display *XD;
//  Window handle,parent;
  hxc_listview lv;
  bool dropped;
  int sel;
  int border;
  int undropped_h;
  int dropped_h,dropped_w;
  EasyStringList& sl;
	Window grandfather;
	IconGroup *lpig;

  LPHXC_DROPDOWNNOTIFYPROC notifyproc;
  static int listview_notify_proc(hxc_listview*,int,int);
  hxc_dropdown() : sl(lv.sl) { init(); };
  hxc_dropdown(Display *_d,Window _p,int _x,int _y,
  				int _w,int _h,LPHXC_DROPDOWNNOTIFYPROC np,
  				void *o) : sl(lv.sl)
  {
    dynamically_allocated=true;
  	init();
    owner=o;
    create(_d,_p,_x,_y,_w,_h,np);
  };

  ~hxc_dropdown() {  }

  void init();
  bool create(Display*,Window,int,int,int,int,LPHXC_DROPDOWNNOTIFYPROC=NULL);
  bool create(Display *_d,Window _p,int _x,int _y,int _w,int _h,LPHXC_DROPDOWNNOTIFYPROC np,void *o){
    owner=o;
    return create(_d,_p,_x,_y,_w,_h,np);
  }
  static void destroy(hxc_dropdown*);
  void additem(char*);
  void additem(char*,int);
  void draw(bool=true);
  void changesel(int);
  void drop();
  void retract();
  void make_empty();
  int select_item_by_data(int,int=0);
	int get_min_width();

  operator Window() { return handle; }
};
//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
class hxc_textdisplay:public hxc
{
private:
  static int WinProc(hxc_textdisplay*,Window,XEvent*);
  static void delete_hxc_textdisplay(hxc *p){ delete (hxc_textdisplay*)p; }

//  XFontStruct *font;
//  GC gc;
//  DWORD col_grey,col_black,col_white,col_border_dark,col_border_light;
public:
  hxc_scrollbar sb;
  bool has_scrollbar;
  bool force_scrollbar;
  int sy;
  int border;
  int textheight;
  int pad_x;
  int col_td_bk;
  bool wordwrapped;
  EasyStr text;
  int n_lines;
  int top_left_char;
  bool fix_top_left_char;
  DynamicArray<int> linebreak,highlight_lines;

  hxc_textdisplay();
  ~hxc_textdisplay() {  }
  bool create(Display*,Window,int,int,int,int,DWORD,bool=false);
  static void destroy(hxc_textdisplay*);
  void draw(bool=true);
  void scrollto(int newsy);
  void set_text(char*);
  void append_text(char*);
//  int get_required_height(int line_width); done by wordwrap!
  int wordwrap(int,XFontStruct* =NULL,int=2000000000);
	int get_longest_line_width(XFontStruct*);
	int get_longest_word_width(XFontStruct*);
  int linesperscreen();
  int get_line_from_character_index(int cn);
  void handle_keypress(Display*,int);

  static int scrollbar_notify_proc(hxc_scrollbar* sb,int mess,int i);

  operator Window() { return handle; }
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define EDN_RETURN 1
#define EDN_CHANGE 2
#define EDN_LOSTFOCUS 3

class hxc_edit:public hxc
{
private:
  static int WinProc(hxc_edit*,Window,XEvent*);
  static void delete_hxc_edit(hxc *p){ delete (hxc_edit*)p; }
public:
  int sx;
  int pad_x;
  int border;
  int cursor;
  EasyStr text;

  bool selflag;
  int sel1,sel2;

  bool dragging;

  EasyStr clipboard;

  Time last_click_time;
  int last_click_x,last_click_y;

  LPHXC_EDITNOTIFYPROC notifyproc;
  XComposeStatus xcompose_status;

  void init();
  hxc_edit() { init(); }
  ~hxc_edit() {  }
  hxc_edit(Display *_d,Window _p,int _x,int _y,
  				int _w,int _h,LPHXC_EDITNOTIFYPROC np,
  				void *o)
  {
    dynamically_allocated=true;
  	init();
    owner=o;
    create(_d,_p,_x,_y,_w,_h,np);
  }

  bool create(Display*,Window,int,int,int,int,LPHXC_EDITNOTIFYPROC=NULL);
  bool create(Display *_d,Window _p,int _x,int _y,int _w,int _h,LPHXC_EDITNOTIFYPROC np,void *o){
    owner=o;
    return create(_d,_p,_x,_y,_w,_h,np);
  }

  static void destroy(hxc_edit*);
  void draw(bool=true);
  void scrollto(int);
  void set_text(char*,bool=false);
	void select_all();

//  int wordwrap(int,XFontStruct* =NULL,int=2000000000);
  bool is_alphanumeric_character(char);
  int character_index_from_x_coordinate(int);
  void replace_selection(char*);
  void select_word(int cn);

  void copy_selection_to_clipboard();
  bool get_clipboard_text();
  void copy();
  void cut();
  void paste();

  void handle_keypress(Display*,XKeyEvent*);

  operator Window() { return handle; }
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define SAN_CLICK 1

class hxc_scrollarea:public hxc
{
private:
  static int frame_WinProc(hxc_scrollarea*,Window,XEvent*);
  static int surface_WinProc(hxc_scrollarea*,Window,XEvent*);
  static int scrollbar_notify_proc(hxc_scrollbar*,int,int);
  static void delete_hxc_scrollarea(hxc *p){ delete (hxc_scrollarea*)p; }

public:
	int sx,sy;
	int ww,hh;
	bool hscroll,vscroll;
	hxc_scrollbar hsb,vsb;
	Window frame; //,cornerbox;
  LPHXC_SCROLLAREANOTIFYPROC notifyproc;

	void adjust();
	void rangecheck();
	void scrollto(int,int);
	void resize(int,int);

  hxc_scrollarea();
  ~hxc_scrollarea(){};
  bool create(Display*,Window,int,int,int,int,LPHXC_SCROLLAREANOTIFYPROC=NULL);
  bool create(Display *_d,Window _p,int _x,int _y,int _w,int _h,LPHXC_SCROLLAREANOTIFYPROC np,void *o){
    owner=o;
    return create(_d,_p,_x,_y,_w,_h,np);
  }
  static void destroy(hxc_scrollarea*);

  operator Window() { return handle; }
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#endif

