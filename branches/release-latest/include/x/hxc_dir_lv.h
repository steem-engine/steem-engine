#ifndef HXC_DIR_LV_H
#define HXC_DIR_LV_H

#include "hxc.h"
#include "hxc_popup.h"

// DLV = dir_lv + D=data/F=flag/N=notify

#define DLVD_ICON 0
#define DLVD_SORT 1 // 0=parent, 1=folder, 2=file
#define DLVD_TYPE 2 // corresponds to the extension in ext_sl
#define DLVD_FLAGS 3

#define DLVF_LINKMASK   3 //0=no link, 1=link, 2=broken link
#define DLVF_EXTREMOVED 0x100
#define DLVF_READONLY   0x200

#define DLVN_GETTYPE 0
#define DLVN_DROP 1
#define DLVN_DOUBLECLICK 2
#define DLVN_SELCHANGE 3
#define DLVN_RETURN 4
#define DLVN_NAMECHANGED 5
#define DLVN_CONTEXTMENU 6
#define DLVN_POPCHOOSE 7
#define DLVN_FOLDERCHANGE 8

#define DLVN_TYPECHANGE 10
#define DLVN_FOLDERMOVED 11
#define DLVN_ITEMDELETED 12
#define DLVN_CONTENTSCHANGE 13

#define DLVCCN_DELETE 0
#define DLVCCN_MOVE 1
#define DLVCCN_RENAME 2

#define DLVCCN_BEFORE 0
#define DLVCCN_AFTER 1

typedef struct{
  int action,time,flags;
  EasyStr path,new_path;
  bool success;
}dlv_ccn_struct;

#define MCL_MOVE 1
#define MCL_COPY 2
#define MCL_LINK 3

class hxc_dir_lv;

typedef int HXC_DIRLVNOTIFYPROC(hxc_dir_lv*,int,int);
typedef HXC_DIRLVNOTIFYPROC* LPHXC_DIRLVNOTIFYPROC;

class hxc_dir_lv
{
private:
	static int lv_notifyproc(hxc_listview*,int,int);
  static int drag_popup_notifyproc(hxc_popup*,int,int);
public:
	hxc_dir_lv();
	~hxc_dir_lv() {};

	bool create(Display *,Window,int,int,int,int,LPHXC_DIRLVNOTIFYPROC,void*);
	bool refresh_fol();
	EasyStr get_item_path(int,bool=0);
	EasyStr get_item_name(int);
	EasyStr movecopylink_item(int,EasyStr,EasyStr);
	void select_item_by_name(char*);
  void delete_item(),rename_item();

	EasyStr fol,base_fol;
	hxc_listview lv;
	bool allow_type_change,show_broken_links,choose_only;
	EasyStringList ext_sl;

  hxc_popup pop;
  EasyStr drag_src,drag_dest;

	IconGroup* &lpig;
	EasyStringList &sl;
	LPHXC_DIRLVNOTIFYPROC notifyproc;
	void *owner;
  int id;
};

#endif
