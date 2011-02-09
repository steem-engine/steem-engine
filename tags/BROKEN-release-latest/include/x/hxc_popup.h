#ifndef HXC_POPUP_H
#define HXC_POPUP_H

#include "hxc.h"

#define POP_CHOOSE 0
#define POP_CANCEL 1

#define POP_CURSORPOS -9999

class hxc_popup;

typedef int HXC_POPUPNOTIFYPROC(hxc_popup*,int,int);
typedef HXC_POPUPNOTIFYPROC* LPHXC_POPUPNOTIFYPROC;

class hxc_popup
{
private:
	static int WinProc(hxc_popup*,Window,XEvent*);
public:
	hxc_popup();
	~hxc_popup() {};

	bool create(Display *,Window,int,int,LPHXC_POPUPNOTIFYPROC,void*);
  void draw(int=-1);
  void close(bool);

	IconGroup* lpig;
	EasyStringList menu;
	void *owner;
  int border,sy,sel,itemheight;
  int x,y,w,h;
  Display *XD;
  LPHXC_POPUPNOTIFYPROC notifyproc;
  bool clicked_in;
  Window handle;
};

#endif

