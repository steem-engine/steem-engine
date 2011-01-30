#ifndef HXC_PROMPT_CPP
#define HXC_PROMPT_CPP

#include "hxc.h"
#include "hxc_prompt.h"

hxc_prompt::hxc_prompt()
{
}

EasyStr hxc_prompt::ask(Display *XD,char *text,char *caption)
{
  hxc::load_res(XD);

  int w=300,h=10+35+10;

  Window handle=hxc::create_modal_dialog(XD,w,h,caption,true);
  if (handle==0) return "";

  hxc_edit *pEd=new hxc_edit(XD,handle,10,10,w-20,25,edit_notify_proc,this);
  pEd->set_text(text,true);

  EasyStr ret;
  if (hxc::show_modal_dialog(XD,handle,true)==1) ret=pEd->text;

  hxc::destroy_modal_dialog(XD,handle);

  hxc::free_res(XD);

  return ret;
}

int hxc_prompt::edit_notify_proc(hxc_edit *e,int mess,int)
{
  if (mess==EDN_RETURN) hxc::modal_result=1;
  return 0;
}

#endif

