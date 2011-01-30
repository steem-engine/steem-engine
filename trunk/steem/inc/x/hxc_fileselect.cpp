#ifndef HXC_FILESELECT_CPP
#define HXC_FILESELECT_CPP

#include "hxc.h"
#include "hxc_alert.h"
#include "hxc_fileselect.h"
//---------------------------------------------------------------------------
hxc_fileselect::hxc_fileselect()
{
  XD=NULL;
  parse_routine=NULL;
  lpig=NULL;
  mode=FSM_OK;
  lp_corner_ig=NULL;
	DirOutput.id=0;
	filename_ed.id=1;
	one_choice=true;
	default_extension="";
}

void hxc_fileselect::set_corner_icon(IconGroup*p,int idx)
{
	lp_corner_ig=p;
	corner_icon_index=idx;
}

void hxc_fileselect::set_alert_box_icons(IconGroup*lp_big_ig,IconGroup*lp_small_ig)
{
	alert.lp_big_ig=lp_big_ig;
	alert.lp_small_ig=lp_small_ig;
}	


EasyStr hxc_fileselect::choose(Display *d,char *Dir,char*name,char *Title,
                            int pass_mode,LPHXC_FILESELECTPARSE pr,char*defext)
{
  XD=d;
  DisplayDir=Dir;
  mode=pass_mode;
  Pixmap IconPixmap=0;
  Pixmap IconMaskPixmap=0;

	
  T_doesnt_exist=T("This file doesn't seem to exist.  Choose a different one.");
  T_folder_doesnt_exist=T("This folder doesn't seem to exist.  Choose a different one.");
  T_cant_find=T("Can't find file");
  T_okay=T("Okay");
  T_do_you_wanna=T("Do you want to load the file");
  T_confirm_load=T("Confirm Load");
  T_yes=T("Yes");
  T_no=T("No");
  T_create_new_question=T("You have tried to open a file that doesn't exist.  Do you want to create a new file with this name?");
  T_confirm_create=T("Confirm Create");
  T_overwrite_it=T("This file already exists.  Do you want to overwrite it?");
  T_confirm_overwrite=T("Overwrite File?");
  T_choose_folder=T("Choose folder");
  T_create_new_folder_question=T("This folder doesn't exist.  Do you want to create a new folder with this name?");
  T_error=T("Error");
  T_failed_to_create_folder=T("Couldn't create the folder.  Sorry!");
	if(!(T_load_but[0]))T_load_but=T("Load");
	if(!(T_save_but[0]))T_save_but=T("Save");
	if(!(T_okay_but[0]))T_okay_but=T("Okay");
	if(!(T_cancel_but[0]))T_cancel_but=T("Cancel");
	
  if (pr!=(LPHXC_FILESELECTPARSE)1) parse_routine=pr;

  chose_filename="";
  chose_path="";
  chose_option=0;
	
	if(defext){
		default_extension=defext;
		if(defext[0]!='.')default_extension.Insert(".",0);
	}else{
		default_extension="";
	}

  hxc::load_res(XD);

	int scr=XDefaultScreen(XD);
  Win=XCreateSimpleWindow(XD,DefaultRootWindow(XD),
  												XDisplayWidth(XD,scr)/2-250,XDisplayHeight(XD,scr)/2-200,500,400,
                          0,hxc::col_grey,hxc::col_grey);
  if(hxc::colormap!=0){
    XSetWindowColormap(XD,Win,hxc::colormap);
  }
  hxc::SetProp(XD,Win,cWinProc,(DWORD)WinProc);
  hxc::SetProp(XD,Win,cWinThis,(DWORD)this);

  Atom Prots[1]={hxc::XA_WM_DELETE_WINDOW};
  XSetWMProtocols(XD,Win,Prots,1);

  XSelectInput(XD,Win,StructureNotifyMask | FocusChangeMask | ExposureMask);

	if(!lp_corner_ig){
		if(lpig){
			lp_corner_ig=lpig;
			corner_icon_index=0;
		}
	}
  if(lp_corner_ig){
    IconPixmap=lp_corner_ig->CreateIconPixmap(corner_icon_index,hxc::gc);
    IconMaskPixmap=lp_corner_ig->CreateMaskBitmap(corner_icon_index);
	  SetWindowHints(XD,Win,True,NormalState,IconPixmap,IconMaskPixmap,0,0);
	}



  lv.create(XD,Win,10,45,480,275,listview_notify_handler,this);
  lv.display_mode=1; //files

  if(lpig){
    UpBut.create(XD,Win,10,10,25,25,
              button_notify_handler,this,BT_ICON,"Up",1,hxc::col_bk);
    UpBut.set_icon(lpig,1);
    lv.lpig=lpig;
  }else{
    UpBut.create(XD,Win,10,10,25,25,
              button_notify_handler,this,BT_TEXT,"Up",1,hxc::col_bk);
    lv.lpig=NULL;
  }

  int xp=400;

  CancelBut.create(XD,Win,xp,365,90,25,
              button_notify_handler,this,BT_TEXT,T_cancel_but,3,hxc::col_bk);
  SetWindowGravity(XD,CancelBut.handle,SouthEastGravity);

  int bs=0;
  xp-=100;
  if(mode&FSM_CHOOSE_FOLDER){
    xp-=30;
	  FolBut.create(XD,Win,xp,365,120,25,
              button_notify_handler,this,BT_TEXT,T_choose_folder,6,hxc::col_bk);
	  SetWindowGravity(XD,FolBut.handle,SouthEastGravity);
    xp-=100;bs++;
  }
  if(mode&FSM_OK){
	  OkBut.create(XD,Win,xp,365,90,25,
              button_notify_handler,this,BT_TEXT,T_okay_but,2,hxc::col_bk);
	  SetWindowGravity(XD,OkBut.handle,SouthEastGravity);
    xp-=100;bs++;
  }
  if(mode&FSM_SAVE){
	  SaveBut.create(XD,Win,xp,365,90,25,
              button_notify_handler,this,BT_TEXT,T_save_but,5,hxc::col_bk);
	  SetWindowGravity(XD,SaveBut.handle,SouthEastGravity);
    xp-=100;bs++;
  }
  if(mode&FSM_LOAD){
	  LoadBut.create(XD,Win,xp,365,90,25,
              button_notify_handler,this,BT_TEXT,T_load_but,4,hxc::col_bk);
	  SetWindowGravity(XD,LoadBut.handle,SouthEastGravity);
    xp-=100;bs++;
  }
  one_choice=(bs==1);

	DirOutput.set_text(DisplayDir);
  DirOutput.create(XD,Win,40,10,420,25,edit_notifyproc,this);
	
	if(name)filename_ed.set_text(name,true);
	else{
	  filename_ed.set_text("");
	}
  filename_ed.create(XD,Win,10,lv.h+lv.y+10,lv.w,25,edit_notifyproc,this);

  XStoreName(XD,Win,Title);

  XSetTransientForHint(XD,Win,Win);

	hxc::modal_children(XD,Win,Win);
 	hxc::SetProp(XD,Win,hxc::cModal,Win);

  XMapWindow(XD,Win);

  set_path(DisplayDir);

  XFlush(XD);

  if (hxc::modal_notifyproc) hxc::modal_notifyproc(true);

  Close=0;
  LPWINDOWPROC WinProc;
  XEvent Ev;
  LOOP{
    if (hxc::wait_for_event(XD,&Ev)){
      WinProc=(LPWINDOWPROC)  hxc::GetProp(XD,Ev.xany.window,cWinProc);
      if (WinProc){
  //    	bool suppress=false;
      	if(hxc::GetProp(XD,Ev.xany.window,hxc::cModal)!=Win){ //not modal
      		if (hxc::suppress_mess_for_modal(XD,&Ev)==0){
  		    	WinProc((void*)  hxc::GetProp(XD,Ev.xany.window,cWinThis),Ev.xany.window,&Ev);
      		}else{
      			if(Ev.type==FocusIn){
      				XRaiseWindow(XD,Win);
      			}
      		}
      	}else{
  	    	WinProc((void*)  hxc::GetProp(XD,Ev.xany.window,cWinThis),Ev.xany.window,&Ev);
  			}
      }
      if (Close){
        if (XPending(XD)==0) break;
      }
    }
  }

  if (Close==3){
    DisplayDir="";
  }

	hxc::modal_children(XD,Win,0);

  hxc::destroy_children_of(Win);
  hxc::RemoveProp(XD,Win,cWinProc);
  hxc::RemoveProp(XD,Win,cWinThis);
 	hxc::RemoveProp(XD,Win,hxc::cModal);
  XDestroyWindow(XD,Win);

  if (IconPixmap) XFreePixmap(XD,IconPixmap);
  IconPixmap=0;
  if (IconMaskPixmap) XFreePixmap(XD,IconMaskPixmap);
  IconMaskPixmap=0;

  hxc::free_res(XD);

  if (hxc::modal_notifyproc) hxc::modal_notifyproc(0);

  return DisplayDir;
}
//-----------------------------------------------------------------
bool read_directory_list(char *dir,EasyStringList &sl,LPHXC_FILESELECTPARSE parse_routine)
{
  DIR *dp;
  struct dirent *ep;
  struct stat s;
  char fullpath[5000],*fullpathend;
  int ftype;

  strcpy(fullpath,dir);
  NO_SLASH(fullpath);
  fullpathend=fullpath+strlen(fullpath);
  *(fullpathend++)='/';
  *fullpathend=0;

  sl.Sort=eslSortByData0;
  sl.Sort2=eslSortByNameI;

  dp=opendir(fullpath);
  if (dp){
    for(;;){
      ep=readdir(dp);
      if (ep==NULL) break;

      if (NotSameStr(ep->d_name,".") && NotSameStr(ep->d_name,"..")){
      	bool is_dir=0,do_stat=true;
      	if (parse_routine==NULL){
#ifdef DT_UNKNOWN
      		if (ep->d_type==DT_DIR){
      			is_dir=true;
      		}else if (ep->d_type!=DT_UNKNOWN){
	      		do_stat=0;
      		}
#endif
      	}
      	if (do_stat){
	        strcpy(fullpathend,ep->d_name);
	        do_stat=stat(fullpath,&s);
	        is_dir=S_ISDIR(s.st_mode);
	      }
	      if (do_stat==0){
          ftype=FS_FTYPE_FILE;
          if (parse_routine==NULL){
            if (ep->d_name[0]=='.'){
              ftype=0;
            }else if (is_dir){
              ftype=FS_FTYPE_FOLDER;
            }
          }else{
            ftype=parse_routine(fullpath,&s);
          }
          if (ftype) sl.Add(fullpathend,ftype);
        }
      }
    }
    closedir(dp);
    return false;
  }else{
    return true;
  }
}

void hxc_fileselect::set_path(EasyStr new_path)
{
  NO_SLASH(new_path);
  DisplayDir=new_path;
  if(DisplayDir[0]!='/' && DisplayDir[0]!=0){
  	DisplayDir.Insert("/",0);
  }

  if(mode&FSM_CHOOSE_FOLDER){
    filename_ed.set_text("");
  }

  DirOutput.set_text(DisplayDir+"/",true);
  lv.sl.DeleteAll();
  if(read_directory_list(DisplayDir,lv.sl,parse_routine)){
  	alert.set_icons(alert.lp_big_ig,2,alert.lp_small_ig,2);
  	alert.ask(XD,EasyStr("The directory ")+DisplayDir+" is not accessible.","Access Error",
          "   Okay   ",0,0);
    lv.col_bg=hxc::col_bk;
  }else{
    lv.col_bg=hxc::col_white;
  }

  lv.sel=-1;
  lv.contents_change();
	hxc::modal_children(XD,lv.handle,Win);
  XFlush(XD);
}

int hxc_fileselect::WinProc(hxc_fileselect *This,Window Win,XEvent *Ev)
{
  if (Ev->type==ConfigureNotify){
    XWindowAttributes wa;
    XGetWindowAttributes(This->XD,Win,&wa);

    int w=wa.width,h=wa.height;
    if (This->DirOutput.handle) XResizeWindow(This->XD,This->DirOutput.handle,max(w-50,30),25);
    if (This->lv.XD){
    	XResizeWindow(This->XD,This->lv.handle,max(w-20,10),max(h-90-35,10));
		  if (This->filename_ed.handle){
        XMoveResizeWindow(This->XD,This->filename_ed.handle,
                  10,(This->lv.h)+(This->lv.y)+10,max(w-20,10),25);
      }
		}
    XSync(This->XD,0);
  }else if (Ev->type==ClientMessage){
    if (Ev->xclient.message_type==hxc::XA_WM_PROTOCOLS){
      if ((Atom)(Ev->xclient.data.l[0])==hxc::XA_WM_DELETE_WINDOW){
        This->Close=3;
      }
    }
  }else if (Ev->type==Expose){
    if (Ev->xexpose.count>0) return 0;

    XWindowAttributes wa;
    XGetWindowAttributes(This->XD,Win,&wa);
    hxc::draw_border(This->XD,Win,hxc::gc,0,0,wa.width,wa.height,2,hxc::col_border_light,hxc::col_border_dark);
  }
  return 0;
}

int hxc_fileselect::edit_notifyproc(hxc_edit* ed,int mess,int i){
	hxc_fileselect*This=(hxc_fileselect*)(ed->owner);	
	if((ed->id)==0){ //path
		if(mess==EDN_RETURN){
			This->set_path(ed->text);
		}
	}else if((ed->id)==1){  //filename
	  if(mess==EDN_RETURN){
	    int count=0;
	    for(int msk=1;msk<64;msk<<=1){
	      if((This->mode)&msk)count++;
	    }
	    if(count==1){
	      This->attempt_to_choose((This->mode)&63);
	    }
	  }
	}
	return 0;
}

int hxc_fileselect::listview_notify_handler(hxc_listview* lv,int mess,int i)
{
  hxc_fileselect *This=(hxc_fileselect*)hxc::GetProp(lv->XD,lv->parent,cWinThis);
  if (mess==LVN_DOUBLECLICK || mess==LVN_RETURN){
    if (lv->sl[i].Data[0]==FS_FTYPE_FOLDER){ //folder
      This->set_path(This->DisplayDir+"/"+(lv->sl[i].String));
    }else{
	    int count=0;
	    for (int msk=1;msk<64;msk<<=1) if (This->mode & msk) count++;
      // Don't allow double click on file when choosing folder
      if (This->mode & FSM_CHOOSE_FOLDER) count=0;
      if (count==1) This->attempt_to_choose(This->mode & 63);
    }
  }else if(mess==LVN_SELCHANGE || mess==LVN_SINGLECLICK){
    if( ((This->mode)&FSM_CHOOSE_FOLDER) || (lv->sl[i].Data[0]!=FS_FTYPE_FOLDER)){
	  	This->filename_ed.set_text(lv->sl[i].String,true);
	  }
  }
  return 0;
}

void hxc_fileselect::attempt_to_choose(int md)
{
	EasyStr fn;
  if (filename_ed.text[0]=='/'){ //absolute path
  	fn=filename_ed.text;
  }else{
    if (md!=FSM_CHOOSE_FOLDER || filename_ed.text.Text[0]){
    	fn=DisplayDir+EasyStr("/")+filename_ed.text;
    }else{ //mode is choose folder and text is empty
    	fn=DisplayDir;
    }
  }
  Close=1;
	if (md!=FSM_CHOOSE_FOLDER){
  	if (default_extension[0] && strchr(GetFileNameFromPath(fn),'.')==NULL){
  //		if(md==FSM_LOAD){
  		if (!Exists(fn)){
  			fn+=default_extension;
  		}
  /*		}else if(md==FSM_SAVE){
  			if(!Exists(fn)){
  				fn+=default_extension;
  			}
  		} */
  	}
  }
/*
#define FSM_CONFIRMOVERWRITE 64
#define FSM_CONFIRMLOAD 128
#define FSM_MUSTEXIST 256
#define FSM_CONFIRMCREATEONLOAD 512
2=! 3=? 4=STOP 5=i  */
  if (md!=FSM_CHOOSE_FOLDER || fn[0]){ //okay to choose "/" for pick folder
    if (md==FSM_CHOOSE_FOLDER){
      struct stat s;
      if (stat(fn,&s)==0){
        if (!S_ISDIR(s.st_mode)){
  //you've chosen a file which exists and isn't a directory
          fn=DisplayDir;  //chop off the filename
        }
      }
    }
    if ((mode&FSM_LOADMUSTEXIST) && ((md==FSM_LOAD) || (md==FSM_OK) || (md==FSM_CHOOSE_FOLDER)) ){
  		if (!Exists(fn)){
    		alert.set_icons(alert.lp_big_ig,4,alert.lp_small_ig,4);
    		EasyStr a;
    		if(md==FSM_CHOOSE_FOLDER){
    		  a=fn+"\n\n"+T_folder_doesnt_exist;
    		}else{
    		  a=fn+"\n\n"+T_doesnt_exist;
    		}
    		alert.ask(XD,a,T_cant_find,
              T_okay,0,0);
        Close=0;
      }
    }
    if (Close){
      if (md==FSM_LOAD){
      	if (mode&FSM_CONFIRMLOAD){
      		alert.set_icons(alert.lp_big_ig,4,alert.lp_small_ig,4);
      		if(alert.ask(XD,T_do_you_wanna+"\n"+fn+"?",T_confirm_load,
                T_yes+"|"+T_no,0,1)==1){
    	      Close=0;
    	    }
      	}else if (mode & FSM_CONFIRMCREATE){
    			if(!Exists(fn)){
    	  		alert.set_icons(alert.lp_big_ig,4,alert.lp_small_ig,4);
        		if(alert.ask(XD,fn+"\n\n"+T_create_new_question,
  								T_confirm_create,
                  T_yes+"|"+T_no,0,1)==1){
      	      Close=0;
      	    }
    			}
      	}
      }else if (md==FSM_SAVE){
      	if (mode & FSM_CONFIRMOVERWRITE){
    			if (Exists(fn)){
    	  		alert.set_icons(alert.lp_big_ig,2,alert.lp_small_ig,2);
        		if (alert.ask(XD,fn+"\n\n"+T_overwrite_it,T_confirm_overwrite,
                  T_yes+"|"+T_no,1,1)==1){
      	      Close=0;
      	    }
    			}
      	}
      }else if (md==FSM_CHOOSE_FOLDER){
        if (mode & FSM_CONFIRMCREATE){
          struct stat s;
          if (stat(fn,&s)){ //folder doesn't exist
    	  		alert.set_icons(alert.lp_big_ig,4,alert.lp_small_ig,4);
        		if (alert.ask(XD,fn+"\n\n"+T_create_new_folder_question,
  								T_confirm_create,
                  T_yes+"|"+T_no,0,1)==1){
      	      Close=0;
      	    }else{
      	      if (mkdir(fn,S_IRWXU|S_IRWXG|S_IRWXO)){
        	  		alert.set_icons(alert.lp_big_ig,2,alert.lp_small_ig,2);
      	        alert.ask(XD,T_failed_to_create_folder,
  								T_error,
                  T_okay,0,0);
      	        Close=0;
      	      }
      	    }
          }
        }
      }
    }
  }
	if (Close){
  	chose_option=md;
  	chose_filename=GetFileNameFromPath(fn); //(This->lv.sl)[This->lv.sel].String;
  	chose_path=DisplayDir;
  	DisplayDir=fn;
  	if (mode==FSM_CHOOSE_FOLDER) fn+="/";
	}          	
}

int hxc_fileselect::button_notify_handler(hxc_button *But,int mess,int *Inf)
{
  hxc_fileselect *This=(hxc_fileselect*)(But->owner);
  if (mess==BN_CLICKED){
    switch (But->id){
      case 1: //up
      {
        char *slashpos=strrchr(This->DisplayDir,'/');
        if (slashpos!=NULL){
          *slashpos=0;
        }else{
          This->DisplayDir="";
        }
        This->set_path(This->DisplayDir);
        break;
      }
      case 2: //okay
       	if (This->filename_ed.text[0]) This->attempt_to_choose(FSM_OK);
        break;
      case 3: //cancel
        This->Close=3;
        break;
      case 4: //load
       	if (This->filename_ed.text[0]) This->attempt_to_choose(FSM_LOAD);
        break;
      case 5: //save
       	if (This->filename_ed.text[0]) This->attempt_to_choose(FSM_SAVE);
        break;
      case 6: //choose folder
        This->attempt_to_choose(FSM_CHOOSE_FOLDER);
        break;
    }
  }
  return 0;
}

#endif

