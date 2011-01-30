#ifndef HXC_DIR_LV_CPP
#define HXC_DIR_LV_CPP

#include "hxc_dir_lv.h"
#include "hxc_alert.h"
#include "hxc_prompt.h"
//---------------------------------------------------------------------------
hxc_dir_lv::hxc_dir_lv() : lpig(lv.lpig),sl(lv.sl)
{
  ext_sl.Sort=eslNoSort;
	allow_type_change=0;
  show_broken_links=0;
  choose_only=0;
}
//---------------------------------------------------------------------------
bool hxc_dir_lv::create(Display *d,Window daddy,int x,int y,
                      int w,int h,LPHXC_DIRLVNOTIFYPROC np,void *o)
{
	notifyproc=np;
	owner=o;

	refresh_fol();
	if (choose_only==0) lv.allow_drag=true;
	if (allow_type_change) lv.checkbox_mode=true;
	lv.display_mode=1;
  lv.columns.DeleteAll();
  lv.columns.Add(-1);
	lv.create(d,daddy,x,y,w,h,lv_notifyproc,this);
  return false;
}
//---------------------------------------------------------------------------
bool hxc_dir_lv::refresh_fol()
{
  DIR *dp;
  struct dirent *ep;
  EasyStr ext,name;
  char link_path[MAX_PATH+1];
  struct stat s;

  EasyStr old_sel=GetFileNameFromPath(get_item_path(lv.sel));

  dp=opendir(fol+"/");
  if (dp==NULL) return 0;

	lv.sl.DeleteAll();
  lv.sl.Sort=eslSortByData1;
  lv.sl.Sort2=eslSortByNameI;

  uid_t cur_uid=geteuid();
  gid_t cur_gid=getegid();

  for(;;){
    ep=readdir(dp);
    if (ep==NULL) break;

    if (IsSameStr(ep->d_name,".")){
    }else if (IsSameStr(ep->d_name,"..")){
 			if (NotSameStr_I(fol,base_fol)){
        EasyStr full_path=fol;
        RemoveFileNameFromPath(full_path,REMOVE_SLASH);
        lv.sl.Add(4,EasyStr(ext_sl[0].String)+"\01"+full_path,101+ext_sl[0].Data[0],0,0,0);
      }
    }else{
      EasyStr full_path=fol+SLASH+ep->d_name;
      int is_link=readlink(full_path,link_path,MAX_PATH);
      if (is_link>-1){
        link_path[is_link]=0;
        if (link_path[0]!='/'){
#ifdef CYGWIN
          if (link_path[1]!=':')
#endif
            strcpy(link_path,fol+"/"+link_path);
        }
        is_link=1;
        struct stat tmp_s;
        if (stat(link_path,&tmp_s)==-1) is_link=2;
      }else{
        is_link=0;
      }
      if (is_link<2 || show_broken_links){
        stat(full_path,&s);
        if (S_ISDIR(s.st_mode)){
          lv.sl.Add(4,EasyStr(ep->d_name)+"\01"+full_path,101+ext_sl[1].Data[is_link],1,1,is_link);
        }else if (S_ISREG(s.st_mode)){
          EasyStr notify_path=full_path;
          long flags=DLVF_EXTREMOVED | is_link;
          ext="";
          name=GetFileNameFromPath(full_path);
          char *dot=strrchr(name,'.');
          if (dot){
            ext=dot+1;
            *dot=0;
          }
          char *link_ext=NULL;
          if (is_link){
            link_ext=strrchr(GetFileNameFromPath(link_path),'.');
            if (link_ext) link_ext++;
            notify_path=link_path;
          }
          if (cur_uid==s.st_uid){
            if ((s.st_mode & S_IWUSR)==0) flags|=DLVF_READONLY;
          }else if (cur_gid==s.st_gid){
            if ((s.st_mode & S_IWGRP)==0) flags|=DLVF_READONLY;
          }else{
            if ((s.st_mode & S_IWOTH)==0) flags|=DLVF_READONLY;
          }
          for (int n=2;n<ext_sl.NumStrings;n++){
            bool match=IsSameStr_I(ext,ext_sl[n].String);
            if (link_ext && match==0) match=IsSameStr_I(link_ext,ext_sl[n].String);
            if (match){
              if (notifyproc){
                int newn=notifyproc(this,DLVN_GETTYPE,int(notify_path.Text));
                if (newn) n=newn;
              }
              int icon_idx=is_link;
              if (ext_sl[n].NumData>3 && icon_idx==0){
                if (flags & DLVF_READONLY) icon_idx=3;
              }
              lv.sl.Add(4,EasyStr(name)+"\01"+full_path,101+ext_sl[n].Data[icon_idx],2,n,flags);
              break;
            }
          }
        }
      }
    }
  }
  closedir(dp);

  if (old_sel.NotEmpty()) select_item_by_name(old_sel);
  lv.draw(true,true);

  return true;
}
//---------------------------------------------------------------------------
EasyStr hxc_dir_lv::get_item_path(int i,bool link_target)
{
	if (i<0 || i>=lv.sl.NumStrings) return "";

  EasyStr path=strrchr(lv.sl[i].String,'\01')+1;
	if ((lv.sl[i].Data[DLVD_FLAGS] & DLVF_LINKMASK) && link_target){
    EasyStr target;
    target.SetLength(MAX_PATH);
    int nchars=readlink(path,target,MAX_PATH);
    if (nchars<0) return path;
    target[nchars]=0;
    return target;
  }
	return path;
}
//---------------------------------------------------------------------------
EasyStr hxc_dir_lv::get_item_name(int i)
{
	if (i<0 || i>=lv.sl.NumStrings) return "";

  EasyStr name=lv.sl[i].String;
  char *name_end=strchr(name,'\01');
  if (name_end) *name_end=0;
  return name;
}
//---------------------------------------------------------------------------
void hxc_dir_lv::select_item_by_name(char *name)
{
	for (int i=0;i<lv.sl.NumStrings;i++){
		EasyStr sl_name=GetFileNameFromPath(strrchr(lv.sl[i].String,'\01')+1);
  	if (IsSameStr_I(sl_name,name)){
  		lv.changesel(i);
			return;
  	}
	}
	if (lv.sel<0) lv.changesel(0);
}
//---------------------------------------------------------------------------
EasyStr hxc_dir_lv::movecopylink_item(int action,EasyStr src,EasyStr dest_fol)
{
  if (action==0) return "";
  NO_SLASH(dest_fol);

  EasyStr dest_path;
  if (action==MCL_MOVE){
    dest_path=dest_fol+"/"+GetFileNameFromPath(src);
    // Don't add file name here, if moving a folder and it exists mv will put it inside instead of overwrite
    dlv_ccn_struct ccn;
    ccn.success=0;
    if (notifyproc){
      ccn.action=DLVCCN_MOVE;
      ccn.time=DLVCCN_BEFORE;
      ccn.flags=0;
      ccn.path=src;
      ccn.new_path=dest_path;
      notifyproc(this,DLVN_CONTENTSCHANGE,int(&ccn));
    }
    system(EasyStr("mv -f \"")+src+"\" \""+dest_fol+"/"+"\"");
    struct stat s;
    ccn.success=(stat(src,&s)==-1);
    if (notifyproc){
      ccn.time=DLVCCN_AFTER;
      notifyproc(this,DLVN_CONTENTSCHANGE,int(&ccn));
    }
  }else if (action==MCL_COPY){
    dest_path=GetUniquePath(dest_fol,GetFileNameFromPath(src));
    system(EasyStr("cp -f -R \"")+src+"\" \""+dest_path+"\"");
  }else{
    dest_path=GetUniquePath(dest_fol,GetFileNameFromPath(src));
    symlink(src,dest_path);
  }
	refresh_fol();
  return dest_path;
}
//---------------------------------------------------------------------------
int hxc_dir_lv::lv_notifyproc(hxc_listview *lv,int mess,int i)
{
	hxc_dir_lv *This=(hxc_dir_lv*)lv->owner;
	switch (mess){
		case LVN_DOUBLECLICK:
		case LVN_RETURN:
  		if (i<0) break;
  		if (lv->sl[i].Data[DLVD_TYPE]<2){
        EasyStr new_fol=This->get_item_path(i);
        int ret=0;
        if (This->notifyproc) ret=This->notifyproc(This,DLVN_FOLDERCHANGE,int(new_fol.Text));
        if (ret==0){
          This->fol=new_fol;
    			This->refresh_fol();
          lv->changesel(0);
          if (This->notifyproc) This->notifyproc(This,DLVN_SELCHANGE,0);
        }
  		}else{
  			if (mess==LVN_DOUBLECLICK){
	 				if (This->notifyproc) return This->notifyproc(This,DLVN_DOUBLECLICK,(int)i);
	 			}else{
	 				if (This->notifyproc) return This->notifyproc(This,DLVN_RETURN,(int)i);
	 			}
  		}
			break;
		case LVN_SINGLECLICK:
		case LVN_SELCHANGE:
			if (This->notifyproc) This->notifyproc(This,DLVN_SELCHANGE,(int)i);
			return 0;
		case LVN_ICONCLICK:
    {
  		if (i<0) return 0;
      int cur_type=lv->sl[i].Data[DLVD_TYPE];
  		if (cur_type>=2){
    		EasyStr ext=This->ext_sl[cur_type].String;
        for (int n=max((cur_type+1) % This->ext_sl.NumStrings,2);n!=cur_type;n=max((n+1) % This->ext_sl.NumStrings,2)){
          if (IsSameStr(This->ext_sl[n].String,ext)){
            lv->sl[i].Data[DLVD_TYPE]=n;
            int is_link=lv->sl[i].Data[DLVD_FLAGS] & DLVF_LINKMASK;
            lv->sl[i].Data[DLVD_ICON]=101+This->ext_sl[n].Data[is_link];
            lv->draw(0);
            if (This->notifyproc) This->notifyproc(This,DLVN_TYPECHANGE,i);
            break;
          }
        }
				return 1;
  		}
			return 0;
    }
    case LVN_CONTEXTMENU:
    {
      if (This->choose_only) return 0;
      
      This->pop.menu.DeleteAll();
      This->pop.lpig=NULL;
      
      if (i>-1){
        if (i!=lv->sel){
          lv->changesel(i);
          if (This->notifyproc) This->notifyproc(This,DLVN_SELCHANGE,i);
        }
        if (lv->sl[lv->sel].Data[DLVD_TYPE]>0){
          This->pop.menu.Add(T("Delete")+"  (DEL)",-1,60200);
          This->pop.menu.Add(StripAndT("&Rename")+"  (F2)",-1,60201);
          This->pop.menu.Add("-",-1,0);
        }
      }
      This->pop.menu.Add(T("Refresh")+"  (ESC)",-1,60203);
      This->pop.menu.Add("-",-1,0);
      This->pop.menu.Add(T("New Folder"),-1,60202);
      if (This->notifyproc) This->notifyproc(This,DLVN_CONTEXTMENU,i);

      This->pop.create(This->lv.XD,0,POP_CURSORPOS,0,This->drag_popup_notifyproc,This);
      return 0;
    }
    case LVN_KEYPRESS:
      if (i==XK_Escape) This->refresh_fol();
      if (This->choose_only) return 0;

      if (lv->sel>-1){
        if (lv->sl[lv->sel].Data[DLVD_TYPE]>0){
          if (i==XK_Delete) This->delete_item();
          if (i==XK_F2) This->rename_item();
        }
      }
      return 0;

		case LVN_CANTDROP:
  		if (i<0) return 1;
  		if (lv->sl[i].Data[DLVD_TYPE]>=2) return 1;
  		return 0;
    case LVN_CANTDRAG:
  		if (i<0) return 1;
  		if (lv->sl[i].Data[DLVD_TYPE]==0) return 1;
  		return 0;
		case LVN_DROP:
		{
			hxc_listview_drop_struct *ds=(hxc_listview_drop_struct*)i;

			if (This->notifyproc && ds->on>=0) This->notifyproc(This,DLVN_SELCHANGE,ds->on);

 			if (ds->in_lv==0){
 				if (This->notifyproc) This->notifyproc(This,DLVN_DROP,(int)ds);
 				break;
 			}

			This->drag_dest=This->fol;
  		if (ds->on>=0 && ds->dragged!=ds->on){
  			if (lv->sl[ds->on].Data[DLVD_TYPE]<2) This->drag_dest=This->get_item_path(ds->on);
  		}
      bool same_fol=(This->drag_dest==This->fol);

      This->drag_src=This->get_item_path(ds->dragged);

			if (ds->button==Button1){
        if (same_fol==0) This->movecopylink_item(MCL_MOVE,This->drag_src,This->drag_dest);
        break;
      }

      // Ask if link=0: (if not same fol) Move Here, Copy Here, Link Here
      // Ask if link: Move Target Here, Copy Target Here, -,
      //	(if not same fol) Move Link Here, Copy Link Here.
      // Allow customisation.

      // 0=no link 1=good link 2=broken
      int link=lv->sl[ds->dragged].Data[DLVD_FLAGS] & DLVF_LINKMASK;
      This->pop.menu.DeleteAll();
      if (link==1){
        This->pop.menu.Add(T("Move Target Here"),-1,60100+MCL_MOVE);
        This->pop.menu.Add(T("Copy Target Here"),-1,60100+MCL_COPY);
        This->pop.menu.Add("-",-1,0);
      }
      if (link){
        if (same_fol==0) This->pop.menu.Add(T("Move Link Here"),-1,60000+MCL_MOVE);
        This->pop.menu.Add(T("Copy Link Here"),-1,60000+MCL_COPY);
        This->pop.menu.Add("-",-1,60000+MCL_LINK);
      }else{
        if (same_fol==0) This->pop.menu.Add(StripAndT("&Move Here"),-1,60000+MCL_MOVE);
        This->pop.menu.Add(StripAndT("&Copy Here"),-1,60000+MCL_COPY);
        This->pop.menu.Add(T("Link Here"),-1,60000+MCL_LINK);
        This->pop.menu.Add("-",-1,0);
      }
      This->pop.menu.Add(T("Cancel"),-1,0);

      This->pop.create(This->lv.XD,This->lv.handle,ds->dx,ds->dy,This->drag_popup_notifyproc,This);
			break;
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
void hxc_dir_lv::delete_item()
{
  if (lv.sel<0) return;

  EasyStr file=get_item_path(lv.sel);
  int type=lv.sl[lv.sel].Data[DLVD_TYPE];

  hxc_alert alert;
  alert.set_icons(NULL,0,NULL,0);
  if (type==1){
    if (1==alert.ask(lv.XD,T("Are you sure you want to delete this folder and all its contents?")+"\n\n"+file,
                      T("Delete Folder"),T("Yes")+"|"+T("No"),1,1)) return;
  }else{
    if (1==alert.ask(lv.XD,T("Are you sure you want to delete this file?")+"\n\n"+file,
                      T("Delete File"),T("Yes")+"|"+T("No"),1,1)) return;
  }

  int old_sel=lv.sel;
  if (lv.sel==lv.sl.NumStrings-1){
    lv.sel--;
  }else{
    lv.sel++;
  }
  dlv_ccn_struct ccn;
  ccn.success=0;
  if (notifyproc){
    notifyproc(this,DLVN_SELCHANGE,lv.sel);

    ccn.action=DLVCCN_DELETE;
    ccn.time=DLVCCN_BEFORE;
    ccn.flags=0;
    ccn.path=file;
    notifyproc(this,DLVN_CONTENTSCHANGE,int(&ccn));
  }

  if (type==1){ // Folder, delete it and all its contents!
    system(EasyStr("rm -f -r \"")+file+"\"");
    rmdir(file);
    struct stat s;
    ccn.success=(stat(file,&s)==-1);
  }else if (type>=2){
    ccn.success=(unlink(file)!=-1);
  }

  if (notifyproc){
    ccn.time=DLVCCN_AFTER;
    notifyproc(this,DLVN_CONTENTSCHANGE,int(&ccn));
  }

  if (ccn.success==0){
    lv.sel=old_sel;
    if (notifyproc) notifyproc(this,DLVN_SELCHANGE,lv.sel);
    return;
  }

  refresh_fol();
  if (notifyproc) notifyproc(this,DLVN_ITEMDELETED,int(file.Text));
}
//---------------------------------------------------------------------------
void hxc_dir_lv::rename_item()
{
  EasyStr file=get_item_path(lv.sel);
  EasyStr name=GetFileNameFromPath(file),ext;
  long flags=lv.sl[lv.sel].Data[DLVD_FLAGS];
  long type=lv.sl[lv.sel].Data[DLVD_TYPE];
  if (flags & DLVF_EXTREMOVED){
    char *dot=strrchr(name,'.');
    if (dot){
      ext=dot;
      *dot=0;
    }
  }

  hxc_prompt prompt;
  name=prompt.ask(lv.XD,name,T("Enter Name"));
  if (name.NotEmpty()){
    EasyStr new_file=fol+SLASH+name+ext;

    dlv_ccn_struct ccn;
    ccn.success=0;
    if (notifyproc){
      ccn.action=DLVCCN_RENAME;
      ccn.time=DLVCCN_BEFORE;
      ccn.flags=flags;
      ccn.path=file;
      ccn.new_path=new_file;
      notifyproc(this,DLVN_CONTENTSCHANGE,int(&ccn));
    }

    struct stat s;
    if (stat(new_file,&s)==-1){
      ccn.success=(rename(file,new_file)==0);
    }
    if (notifyproc){
      ccn.time=DLVCCN_AFTER;
      notifyproc(this,DLVN_CONTENTSCHANGE,int(&ccn));
    }
    if (ccn.success==0) return;

    refresh_fol();
    select_item_by_name(GetFileNameFromPath(new_file));
    if (notifyproc){
      if (type==1){
        notifyproc(this,DLVN_FOLDERMOVED,int(file.Text));
      }
      notifyproc(this,DLVN_NAMECHANGED,lv.sel);
    }
  }
}
//---------------------------------------------------------------------------
int hxc_dir_lv::drag_popup_notifyproc(hxc_popup *pop,int mess,int i)
{
	hxc_dir_lv *This=(hxc_dir_lv*)pop->owner;
	if (mess!=POP_CHOOSE || i<0) return 0;

  int action=pop->menu[i].Data[1];
  if (action==0) return 0;

  if (action>=60200){ // Right click menu
    switch (action-60200){
      case 0: This->delete_item(); break;
      case 1: This->rename_item(); break;
      case 2:
      {
        hxc_prompt prompt;
        EasyStr name=prompt.ask(This->lv.XD,T("New Folder"),T("Enter Name"));
        if (name.NotEmpty()){
          EasyStr path=GetUniquePath(This->fol,name);
          if (mkdir(path,S_IRWXU | S_IRWXG | S_IRWXO)!=-1){
            This->refresh_fol();
            This->select_item_by_name(GetFileNameFromPath(path));
            if (This->notifyproc) This->notifyproc(This,DLVN_SELCHANGE,This->lv.sel);
          }
        }
        break;
      }
      case 3:
        This->refresh_fol();
        break;
    }
  }else if (action>=60000){ // Drag op
    if (action>=60100){
      // action on target of link at drag_src
      EasyStr target;
      target.SetLength(MAX_PATH);
      int nchars=readlink(This->drag_src,target,MAX_PATH);
      if (nchars<0) return 0;
      target[nchars]=0;

      EasyStr new_target=This->movecopylink_item(action-60100,target,This->drag_dest);
      if (action==60100+MCL_MOVE) symlink(new_target,This->drag_src);
    }else{
      This->movecopylink_item(action-60000,This->drag_src,This->drag_dest);
    }
  }else{
    if (This->notifyproc) This->notifyproc(This,DLVN_POPCHOOSE,i);
  }
  return 0;
}
//---------------------------------------------------------------------------
#endif

