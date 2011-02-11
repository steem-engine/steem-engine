#ifndef HXC_POPUPHINTS_H
#define HXC_POPUPHINTS_H

#include "hxc.h"

class hxc_popuphints;

typedef int HXC_POPUPHINTNOTIFYPROC(hxc_popuphints*,int,int);
typedef HXC_POPUPHINTNOTIFYPROC* LPHXC_POPUPHINTNOTIFYPROC;

class hxc_popuphints
{
private:
	bool create();
  void draw();
  void close();

	static int WinProc(hxc_popuphints*,Window,XEvent*);
	static int timerproc(hxc_popuphints*,Window,int);
  void get_current_lines(EasyStringList*);
public:
	hxc_popuphints();
	~hxc_popuphints() {};

  void add(Window,EasyStr,Window);
  void remove(Window),remove_all_children(Window);
  void change(Window,EasyStr);
  void start(),stop();

	EasyStringList win_list;
  EasyStr current_text;
	void *owner;
  int border;
  int x,y,w,h,max_width;
  int delay_before_show,time_to_show;
  Display *XD;
  LPHXC_POPUPHINTNOTIFYPROC notifyproc;
  Window handle;
  Window mouse_in_win;
  int mouse_in_count;
};

#endif

