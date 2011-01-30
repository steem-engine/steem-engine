#ifndef HXC_ALERT_H
#define HXC_ALERT_H

#define HXC_ALERT_BUTTON_SPACING 10
#define HXC_ALERT_BUTTON_PADDING 10

class hxc_alert{
private:
  static int button_notify_proc(hxc_button*,int,int*);
public:
  Display *XD;
  Window handle;

  int default_option,close_option;
  int w,h;
  bool has_icon;
  int indent_for_icon;
  int n_lines;
  int n_buttons;
  int chosen;

  hxc_textdisplay td;
  hxc_button but[4];
  hxc_button icon;

  IconGroup*lp_big_ig,*lp_small_ig;
  int big_icon_index,small_icon_index;

  void set_icons(IconGroup*,int,IconGroup* =NULL,int=0);

  hxc_alert();
  ~hxc_alert(){ };
  int ask(Display*,char*,char*,char*,int,int);
};

#endif

