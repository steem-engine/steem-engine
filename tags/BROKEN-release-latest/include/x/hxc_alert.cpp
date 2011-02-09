#ifndef HXC_ALERT_CPP
#define HXC_ALERT_CPP

#include "hxc.h"
#include "hxc_alert.h"

hxc_alert::hxc_alert()
{
  for(int n=0;n<4;n++){
    but[n].owner=this;
  }
  n_buttons=0;
  handle=0;
  lp_big_ig=NULL;
  lp_small_ig=NULL;
}

void hxc_alert::set_icons(IconGroup*pass_lp_big_ig,int pass_big_icon_index,
                        IconGroup*pass_lp_small_ig,int pass_small_icon_index){

  lp_big_ig=pass_lp_big_ig;
  big_icon_index=pass_big_icon_index;

  lp_small_ig=pass_lp_small_ig;
  small_icon_index=pass_small_icon_index;
}

int hxc_alert::ask(Display*XD,char*text,char*caption,
          char*opts,int pass_default_option,int pass_close_option)
{
  hxc_alert::XD=XD;
  default_option=pass_default_option;
  close_option=pass_close_option;
  Pixmap IconPixmap=0;
  Pixmap IconMaskPixmap=0;

  hxc::load_res(XD);

  w=300;h=200;

  EasyStr butname[4];
  int butwidth[4];
  int max_butwidth=0,bw=0;
  n_buttons=0;
  if(opts[0]==0)opts="    Okay    ";
  int cp=0,cp2=0;
  for(int n=0;n<4;n++){
    for(cp2=cp;(opts[cp2]!='|') && (opts[cp2]);cp2++);
    butname[n]=opts+cp;butname[n]=butname[n].Lefts(cp2-cp);
    butwidth[n]=XTextWidth(hxc::font,butname[n],strlen(butname[n]));
    if(butwidth[n]<80)butwidth[n]=80;
    max_butwidth=max(max_butwidth,butwidth[n]);
    n_buttons++;

    cp=cp2+1;
    if(opts[cp2]==0){
      break;
    }
  }
  for(int n=0;n<n_buttons;n++){
    butwidth[n]=max_butwidth;
    bw+=butwidth[n]+(HXC_ALERT_BUTTON_PADDING*2);
    if(n)bw+=HXC_ALERT_BUTTON_SPACING;
  }


  td.border=0;td.pad_x=0;td.sy=0;
  td.textheight=(hxc::font->ascent)+(hxc::font->descent)+2;

  td.set_text(text);

  has_icon=(lp_big_ig!=NULL);
  indent_for_icon=0;
  int icon_size=0;
  if(has_icon){
    indent_for_icon=lp_big_ig->IconHeight+20;
    icon_size=lp_big_ig->IconHeight;
    if(!icon_size)has_icon=false;
  }

  w=max(w,bw+40);

	int llw=td.get_longest_word_width(hxc::font);
	
	if(llw>600)llw=600;
	w=max(llw+40+indent_for_icon,w);

  n_lines=td.wordwrap(w-40-indent_for_icon,hxc::font);
  while(n_lines>6 && w<600){
    w+=100;n_lines=td.wordwrap(w-40-indent_for_icon,hxc::font);
  }
  td.wordwrapped=true;
  if(has_icon){
    h=max(n_lines*(td.textheight),lp_big_ig->IconHeight+16)+60;
  }else{
    h=n_lines*(td.textheight)+60;
  }

  handle=hxc::create_modal_dialog(XD,w,h,caption,0);
  if (handle==0) return close_option;

  XSetForeground(XD,hxc::gc,hxc::col_black);
  XSetFont(XD,hxc::gc,hxc::font->fid);

  int tdh=n_lines*(td.textheight);
  int tdy=10,iy=10;
  if(has_icon){
    if(icon_size>tdh){
      tdy=10+(icon_size-tdh)/2;
    }else{
      iy=10+(tdh-icon_size)/2;
    }
  }

  td.create(XD,handle,indent_for_icon+20,tdy,
                      w-40-indent_for_icon,tdh,
                      hxc::col_bk,false);

  if (has_icon){
    icon.create(XD,handle,15,iy,icon_size,icon_size,NULL,this,
                            BT_ICON | BT_STATIC | BT_BORDER_NONE | BT_TEXT_CENTRE,
                            "!",0,hxc::col_grey);
    icon.set_icon(lp_big_ig,big_icon_index);
    if(lp_small_ig){
      IconPixmap=lp_small_ig->CreateIconPixmap(small_icon_index,hxc::gc);
      IconMaskPixmap=lp_small_ig->CreateMaskBitmap(small_icon_index);
    }else{
      IconPixmap=lp_big_ig->CreateIconPixmap(big_icon_index,hxc::gc);
      IconMaskPixmap=lp_big_ig->CreateMaskBitmap(big_icon_index);
    }
    SetWindowHints(XD,handle,True,NormalState,IconPixmap,IconMaskPixmap,0,0);
  }

  int x=(w-bw)/2;
  for (int n=0;n<n_buttons;n++){
    but[n].create(XD,handle,x,h-38,butwidth[n]+HXC_ALERT_BUTTON_PADDING*2,25,
          button_notify_proc,this,BT_TEXT|BT_TEXT_CENTRE,butname[n],n,hxc::col_grey);
    x+=butwidth[n]+(HXC_ALERT_BUTTON_SPACING+HXC_ALERT_BUTTON_PADDING*2);
  }

  chosen=hxc::show_modal_dialog(XD,handle,true)-1;

  hxc::destroy_modal_dialog(XD,handle);

  if (IconPixmap) XFreePixmap(XD,IconPixmap);
  IconPixmap=0;
  if (IconMaskPixmap) XFreePixmap(XD,IconMaskPixmap);
  IconMaskPixmap=0;

  hxc::free_res(XD);

  return chosen;
}

int hxc_alert::button_notify_proc(hxc_button *b,int mess,int*)
{
  if (mess==BN_CLICKED) hxc::modal_result=1+b->id;
  return 0;
}

#endif

