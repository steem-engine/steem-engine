//---------------------------------------------------------------
hxc_textdisplay::hxc_textdisplay()  //: linebreak(100)
{
  can_have_children=true; //the scrollbar
  destroyproc=(LPHXC_DESTROYPROC)destroy;
  deleteproc=(LPHXC_DELETEPROC)delete_hxc_textdisplay;

  force_scrollbar=0;
  has_scrollbar=false;
  sb.owner=this;
  wordwrapped=false;
  n_lines=0;
  sy=0;
  border=1;
  pad_x=5;
  textheight=20;
  top_left_char=0;
  fix_top_left_char=false;

//  font=NULL;
}
//---------------------------------------------------------------------------
bool hxc_textdisplay::create(Display*d,Window daddy,int x,int y,
                      int w,int h,DWORD pass_col_bk,bool pass_force_scrollbar)
{
  if (XD!=NULL) destroy(this);
  XD=d;
  parent=daddy;
  force_scrollbar=pass_force_scrollbar;
  highlight_lines.DeleteAll();

  col_td_bk=pass_col_bk;
  load_res(XD);


  XSetWindowAttributes swa;
  swa.backing_store=NotUseful;
  swa.background_pixel=col_td_bk;
  handle=XCreateWindow(XD,parent,x,y,w,h,0,
                           CopyFromParent,InputOutput,CopyFromParent,
                           CWBackingStore|CWBackPixel,&swa);
	hxc::x=x;hxc::y=y;hxc::w=w;hxc::h=h;

  SetProp(XD,handle,cWinProc,(DWORD)WinProc);
  SetProp(XD,handle,cWinThis,(DWORD)this);

  XSelectInput(XD,handle,KeyPressMask|KeyReleaseMask|
                            ButtonPressMask|ButtonReleaseMask|
                            ExposureMask|FocusChangeMask|
                            StructureNotifyMask);

  XSetForeground(XD,gc,col_black);

  textheight=(font->ascent)+(font->descent)+2;

  draw(true);

  XMapWindow(XD,handle);

  return false;
}


int hxc_textdisplay::linesperscreen()
{
  return (h-border*2)/textheight;
}

int hxc_textdisplay::get_line_from_character_index(int cn){
  if(wordwrapped){
    int n;
    for(n=0;n<n_lines;n++){
      if(linebreak[n]>cn){
        return n-1;
      }
    }
    return n_lines-1;
  }else{
    return 0;
  }
}

void hxc_textdisplay::set_text(char*t){
  text=t;
  wordwrapped=false;
  fix_top_left_char=false;
  highlight_lines.DeleteAll();
  linebreak.DeleteAll(true); //resize
}

void hxc_textdisplay::append_text(char*t){
  text+=t;
  wordwrapped=false;
  fix_top_left_char=false;
  linebreak.DeleteAll(true); //resize
}

void hxc_textdisplay::scrollto(int newsy)
{
  if (XD==NULL) return;

  if (newsy<0) newsy=0;
  int ch=h-border*2;
  int maxsy=n_lines*textheight-ch;
  if (maxsy<0) maxsy=0;
  if (newsy>maxsy) newsy=maxsy;
  if (sy!=newsy){
    sy=newsy;
    sb.pos=sy;
    draw(true);
    XFlush(XD);
  }
}

int hxc_textdisplay::wordwrap(int w,XFontStruct *fi,int give_up_if_longer_than){
  char*t; //,buf;
  if(fi==NULL){
    fi=font;
    if(fi==NULL){
      return 0; //no font for size checking!
    }
  }
  if(w<20)return 0;
  if(text[0]==0)return 0;
  linebreak.DeleteAll(false);
  linebreak.Add(0);
  int last_linebreak=0;
  int last_length=0;
  int last_break_char=0;
  int length;
  int cn,cut_cn;
  int text_length=strlen(text);
  for(cn=0;cn<=text_length;cn++){
    char c=text[cn];
    if(c==' ' || c=='-' || c==0 || c=='\n'){ //breaking character
      if(c==' ' || c==0){
        cut_cn=cn;
      }else if(c=='\n'){
        if(text[cn-1]=='\r'){
          cut_cn=cn-1;
        }else{
          cut_cn=cn;
        }
      }else{ //visible breaking character
        cut_cn=cn+1;
      }
//      buf=text[cut_cn];
//      text[cut_cn]=0;
      t=text.Text+last_linebreak;
//      t=&(text[last_linebreak]);
      if(cut_cn>last_linebreak){
        length=XTextWidth(fi,t,cut_cn-last_linebreak);
      }else{
        length=0;
      }
//      text[cut_cn]=buf;
      if(length>w){ //wrap
        if(last_length==0){ //nowhere to break the line!
          do{
//            text[cut_cn]=buf;
            cut_cn--;
            if(cut_cn<=last_linebreak){
              n_lines=0;
              return 0; //something's gone badly wrong!
            }else if(cut_cn==last_linebreak+1){
              break;
            }
//            buf=text[cut_cn];
//            text[cut_cn]=0;
            length= XTextWidth(fi,t,cut_cn-last_linebreak);
          }while(length>w);
//          text[cut_cn]=buf; //restore text
          linebreak.Add(cut_cn);
          last_linebreak=cut_cn;
          cn=cut_cn;last_length=0;last_break_char=cn;
          if(--give_up_if_longer_than<0){
            n_lines=linebreak.NumItems-1;
            return n_lines;
          }
        }else{ //there is a breaking character in the line
          cn=last_break_char+1;
          while(text[cn]==' ')cn++;
          linebreak.Add(cn); //leave space or - on end of previous line
          last_linebreak=cn;
          last_length=0;last_break_char=last_linebreak;
          if(--give_up_if_longer_than<0){
            n_lines=linebreak.NumItems-1;
            return n_lines;
          }
        }
      }else{ //don't need to wrap yet
        if(c=='\n'){   //return
          linebreak.Add(cn+1);
          last_linebreak=cn+1;
          last_length=0;last_break_char=last_linebreak;
          if(--give_up_if_longer_than<0){
            n_lines=linebreak.NumItems-1;
            return n_lines;
          }
        }else{
          last_length=length;
          last_break_char=cn;
        }
      }
    }
    if(text[cn]==0){ //reached end
      break;
    }
  } //next cn

  n_lines=linebreak.NumItems;
  linebreak.Add(cn);
  return n_lines;
}

int hxc_textdisplay::get_longest_line_width(XFontStruct *fi)
{
  char*t; //,buf;
  if(fi==NULL){
    fi=font;
    if(fi==NULL){
      return 0; //no font for size checking!
    }
  }
  int text_length=strlen(text);
  int length,longest_length=0;
  int last_line_start=0,cut_cn;
  for(int cn=0;cn<=text_length;cn++){
    char c=text[cn];
    if(c=='\n' || c==0){ //breaking character
      if(text[cn-1]=='\r'){
        cut_cn=cn-1;
      }else{
        cut_cn=cn;
      }
			t=text.Text+last_line_start;
      if(cut_cn>last_line_start){
        length=XTextWidth(fi,t,cut_cn-last_line_start);
      }else{
        length=0;
      }
      last_line_start=cut_cn+1;
      if(length>longest_length){
      	longest_length=length;
      }
    }
    if(text[cn]==0){ //reached end
      break;
    }
  } //next cn

  return longest_length;
}

int hxc_textdisplay::get_longest_word_width(XFontStruct *fi)
{
  char*t; //,buf;
  if(fi==NULL){
    fi=font;
    if(fi==NULL){
      return 0; //no font for size checking!
    }
  }
  int text_length=strlen(text);
  int length,longest_length=0;
  int last_word_start=0,cut_cn;
  for(int cn=0;cn<=text_length;cn++){
    char c=text[cn];
    if(c=='\n' || c==0 || c==' ' || c=='-'){ //breaking character
      if(text[cn-1]=='\r'){
        cut_cn=cn-1;
      }else if(c=='-'){
      	cut_cn=cn+1;
      }else{
        cut_cn=cn;
      }
			t=text.Text+last_word_start;
      if(cut_cn>last_word_start){
        length=XTextWidth(fi,t,cut_cn-last_word_start);
      }else{
        length=0;
      }
      last_word_start=cn+1;
      if(length>longest_length){
      	longest_length=length;
      }
    }
    if(text[cn]==0){ //reached end
      break;
    }
  } //next cn

  return longest_length;
}

void hxc_textdisplay::destroy(hxc_textdisplay*This)
{
  if (This->XD==NULL) return;

  if(This->has_scrollbar){
    This->sb.destroy(&(This->sb));
    This->has_scrollbar=false;
  }
  RemoveProp(This->XD,This->handle,cWinProc);
  RemoveProp(This->XD,This->handle,cWinThis);
  XDestroyWindow(This->XD,This->handle);

  free_res(This->XD);

  This->handle=0;
  This->XD=NULL;
}

void hxc_textdisplay::draw(bool draw_contents)
{
  if (XD==NULL) return;

  int cw=w-border*2;
  int ch=h-border*2;
  if (draw_contents){
    if(!wordwrapped){
//      printf("Text was not wordwrapped\n");
      bool should_have_scrollbar=false;
      if(force_scrollbar){
        should_have_scrollbar=true;
      }else{
        int max_lines=ch/textheight;
        n_lines=wordwrap(cw-pad_x*2,NULL,max_lines);
        wordwrapped=true;
        should_have_scrollbar=(n_lines*textheight>ch);
//        printf("% lines, text %i high, total height %i, have scrollbar? %i\n",n_lines,textheight,ch,should_have_scrollbar);
      }
      if(should_have_scrollbar){ //should have scrollbar
        n_lines=wordwrap(cw-pad_x*2-HXC_SCROLLBAR_WIDTH); //rewrap to smaller size
        wordwrapped=true;
        if(!has_scrollbar){
          sb.create(XD,handle,cw-HXC_SCROLLBAR_WIDTH+border,border,
                          HXC_SCROLLBAR_WIDTH,ch,scrollbar_notify_proc,this);
        }
        sb.init(n_lines*textheight,ch,sy);
//        printf("Initialising scrollbar on textdisplay, %i high\n",n_lines*textheight);
        has_scrollbar=true;
      }else{ //shouldn't have scrollbar
        if(has_scrollbar){
          sb.destroy(&sb);
          has_scrollbar=false;
        }
      }
    }
    if(fix_top_left_char){
      fix_top_left_char=false;
      int yo=textheight-(sy%textheight);
      scrollto(get_line_from_character_index(top_left_char)*textheight-yo);
    }
    int maxsy=n_lines*textheight-ch;
    if(sy>maxsy)scrollto(maxsy);

//    if(has_scrollbar)sb.pos=sy;

    XSetForeground(XD,gc,col_td_bk);
    XFillRectangle(XD,handle,gc,border,border,cw,ch);
    XSetForeground(XD,gc,col_black);
    if(text[0]){
      int cut_cn,line_start;
      char buf,*t;

      int first_line=(sy+textheight-1)/textheight;
      if(first_line<0)first_line=0;
      if(first_line>n_lines)first_line=n_lines;
      top_left_char=linebreak[first_line];

      int n1=max(sy/textheight,0); //index of first visible line
      int n2=min(n_lines,(sy+ch+textheight-1)/textheight); //index of line after the last one to draw
      for (int n=n1;n<n2;n++){
        cut_cn=linebreak[n+1];line_start=linebreak[n];
        while ((unsigned char)(text[cut_cn-1])<=(unsigned char)' '){
          //we don't want these control & space characters to be displayed
          cut_cn--;
          if (cut_cn<=line_start)break;
        }
        if (cut_cn>line_start){
          buf=text[cut_cn];text[cut_cn]=0;
          t=text.Text+line_start;

          bool reset=0;
          for (int i=0;i<highlight_lines.NumItems;i++){
            if (highlight_lines[i]==n){
              XSetForeground(XD,gc,col_sel_back);
              XFillRectangle(XD,handle,gc,border,border+n*textheight-sy,cw,font->ascent+font->descent);
              XSetForeground(XD,gc,col_sel_fore);
              reset=true;
              break;
            }
          }
          XDrawString(XD,handle,gc,border+pad_x,border+n*textheight-sy+(font->ascent),t,strlen(t));
          if (reset) XSetForeground(XD,gc,col_black);
          text[cut_cn]=buf; //restore first char of next line
        }
      }
    }
//    XFlush(XD);
    if(has_scrollbar){
      sb.draw();
    }
  }
  if(border){
    Window Foc=0;
    int RevertTo;
    XGetInputFocus(XD,&Foc,&RevertTo);
    if(Foc==handle){
      draw_border(XD,handle,gc,0,0,w,h,border,col_black,col_black);
    }else{
      draw_border(XD,handle,gc,0,0,w,h,border,col_border_dark,col_border_light);
    }
  }
}

int hxc_textdisplay::WinProc(hxc_textdisplay *This,Window Win,XEvent *Ev)
{
  if (This->XD==NULL) return 0;
	This->common_winproc(Ev);

  switch(Ev->type){
    case FocusIn:
      This->draw(false);
      break;
    case FocusOut:
      This->draw(true);
      break;
    case ButtonPress:
      if (Ev->xbutton.button==Button1){
        if (This->border) XSetInputFocus(This->XD,Win,RevertToParent,CurrentTime);
      }else if (Ev->xbutton.button==Button4){      // Wheel up
        This->scrollto(This->sy-(This->h-This->border*2-This->textheight));
      }else if (Ev->xbutton.button==Button5){      // Wheel down
        This->scrollto(This->sy+(This->h-This->border*2-This->textheight));
      }
      break;
    case KeyPress:
      This->handle_keypress(This->XD,Ev->xkey.keycode);
      break;
    case Expose:
      hxc::clip_to_expose_rect(This->XD,&(Ev->xexpose));
      This->draw(true);
      XSetClipMask(This->XD,hxc::gc,None);
      break;
    case ConfigureNotify:
    {
      XWindowAttributes wa;
      XGetWindowAttributes(This->XD,Win,&wa);
      if (This->has_scrollbar){
        This->sb.init((This->n_lines)*(This->textheight),
                wa.height-(This->border)*2,This->sy);
        XMoveResizeWindow(This->XD,This->sb.handle,
                        wa.width-HXC_SCROLLBAR_WIDTH-(This->border),(This->border),
                        HXC_SCROLLBAR_WIDTH,max(wa.height-This->border*2,10));
      }
      This->fix_top_left_char=true;
      This->wordwrapped=false;
      break;
    }
  }
  return 0;
}

int hxc_textdisplay::scrollbar_notify_proc(hxc_scrollbar* sb,int mess,int i)
{
  hxc_textdisplay*This=(hxc_textdisplay*)(sb->owner);
  if(sb->XD==NULL)return -1;
  switch(mess){
  case SBN_SCROLLBYONE:
    This->scrollto((This->sy)+i*(This->textheight));
    break;
  case SBN_SCROLL:
    This->scrollto(i);
    break;
  }
  return 0;
}

void hxc_textdisplay::handle_keypress(Display *XD,int keycode)
{
  if (XD==NULL) return;
  KeySym ks;
  ks=XKeycodeToKeysym(XD, keycode, 0);
  int newsy=sy;
  switch (ks){
    case XK_Up:
      newsy-=textheight;
      break;
    case XK_Down:
      newsy+=textheight;
      break;
    case XK_Page_Up:
    case XK_Page_Down:
    {
      int np=h-border*2-textheight;
      if (ks==XK_Page_Down) newsy+=np;
      if (ks==XK_Page_Up) newsy-=np;
      break;
    }
    case XK_Home:
      newsy=0;
      break;
    case XK_End:
      newsy=n_lines*textheight;
      break;
  }
  scrollto(newsy);
}

