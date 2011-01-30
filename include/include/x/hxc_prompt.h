#ifndef HXC_PROMPT_H
#define HXC_PROMPT_H

class hxc_prompt{
private:
  static int edit_notify_proc(hxc_edit*,int,int);
public:
  hxc_prompt();
  ~hxc_prompt(){ };
  EasyStr ask(Display*,char*,char*);
};

#endif

