#ifndef HXC_FILESELECT_H
#define HXC_FILESELECT_H

#ifndef T
#define T(s) s
#endif

#include <easystringlist.h>

class hxc_fileselect;

typedef int HXC_FILESELECTPARSE(char*,struct stat*);
typedef HXC_FILESELECTPARSE* LPHXC_FILESELECTPARSE;

extern bool read_directory_list(char *,EasyStringList &,LPHXC_FILESELECTPARSE=NULL);

#define FSM_OK 1
#define FSM_LOAD 2
#define FSM_SAVE 4
#define FSM_CHOOSE_FOLDER 8

#define FSM_CONFIRMOVERWRITE 64
#define FSM_CONFIRMLOAD 128
#define FSM_LOADMUSTEXIST 256
#define FSM_CONFIRMCREATE 512
#define FSM_CONFIRMCREATEONLOAD FSM_CONFIRMCREATE

#define FS_FTYPE_REJECT 0
#define FS_FTYPE_FOLDER 50
#define FS_FTYPE_FILE 100
#define FS_FTYPE_FILE_ICON 101

class hxc_fileselect
{
private:
//  void read_directory_list(char*,EasyStringList&);
  void set_path(EasyStr);
  static int WinProc(hxc_fileselect*,Window,XEvent*);
  static int listview_notify_handler(hxc_listview*,int,int);
  static int button_notify_handler(hxc_button*,int,int*);
  static int edit_notifyproc(hxc_edit*,int,int);

	void attempt_to_choose(int);

  Display *XD;
  Window Win;
//  XFontStruct *FontInfo;
//  GC gc;
//  DWORD col_grey,col_black,col_white;
  EasyStr DisplayDir;
  int Close;

  IconGroup*lp_corner_ig;
  int corner_icon_index;

  hxc_listview lv;
  hxc_button UpBut,OkBut,CancelBut,LoadBut,SaveBut,FolBut;
  hxc_edit DirOutput;
  hxc_edit filename_ed;
  LPHXC_FILESELECTPARSE parse_routine;

  hxc_alert alert;
public:
  EasyStr T_doesnt_exist;
  EasyStr T_folder_doesnt_exist;
  EasyStr T_cant_find;
  EasyStr T_okay;
  EasyStr T_do_you_wanna;
  EasyStr T_confirm_load;
  EasyStr T_yes;
  EasyStr T_no;
  EasyStr T_create_new_question;
  EasyStr T_confirm_create;
  EasyStr T_overwrite_it;
  EasyStr T_confirm_overwrite;
	EasyStr T_load_but;
	EasyStr T_save_but;
	EasyStr T_okay_but;
	EasyStr T_cancel_but;
  EasyStr T_choose_folder;
  EasyStr T_create_new_folder_question;
  EasyStr T_error;
  EasyStr T_failed_to_create_folder;

  hxc_fileselect();
  ~hxc_fileselect() {};

  EasyStr chose_filename;
  EasyStr chose_path;
  int chose_option;
	EasyStr default_extension;
  	
  int mode;
	bool one_choice;

  IconGroup*lpig;

  EasyStr choose(Display*,char*,char*,char*,int=FSM_OK,
  								LPHXC_FILESELECTPARSE=(LPHXC_FILESELECTPARSE)1,
  								char* =NULL);
	void set_corner_icon(IconGroup*p,int idx);
	void set_alert_box_icons(IconGroup*lp_big_ig,IconGroup*lp_small_ig);
	
};

#endif

