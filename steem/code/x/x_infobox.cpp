//---------------------------------------------------------------------------
TGeneralInfo::TGeneralInfo()
{
  Section="GeneralInfo";
//  for(int n=0;n<5;n++)tab[n].owner=this;
	page_lv.owner=this;
  Page=INFOPAGE_README;
}
//---------------------------------------------------------------------------
void TGeneralInfo::Show()
{
  if (Handle) return;

  bool ShowXReadme=Exists(RunDir+"/README");
  bool ShowFAQ=Exists(RunDir+"/FAQ");
  bool ShowReadme=Exists(RunDir+"/win32.help");
  bool ShowDrawSpeed=0; /*(avg_frame_time && FullScreen==0);*/
  bool ShowDiskHowto=Exists(RunDir+"/disk image howto.txt");
  bool ShowCartHowto=Exists(RunDir+"/cart image howto.txt");

  page_lv.sl.DeleteAll();
  page_lv.sl.Sort=eslNoSort;
  if (ShowXReadme) page_lv.sl.Add(EasyStr("X ")+T("Readme"),101+ICO16_README,INFOPAGE_UNIXREADME);
  if (ShowReadme) page_lv.sl.Add(EasyStr("Win32 ")+T("Readme"),101+ICO16_README,INFOPAGE_README);
  if (ShowFAQ) page_lv.sl.Add("FAQ",101+ICO16_FAQ,INFOPAGE_FAQ);
  page_lv.sl.Add(T("About"),101+ICO16_GENERALINFO,INFOPAGE_ABOUT);
  if (ShowDrawSpeed) page_lv.sl.Add(T("Draw Speed"),101+ICO16_DRAWSPEED,INFOPAGE_DRAWSPEED);
  page_lv.sl.Add(T("Links"),101+ICO16_LINKS,INFOPAGE_LINKS);
  if (ShowDiskHowto) page_lv.sl.Add("Disk Image Howto",101+ICO16_DISK,INFOPAGE_HOWTO_DISK);
  if (ShowCartHowto) page_lv.sl.Add("Cartridge Image Howto",101+ICO16_CHIP,INFOPAGE_HOWTO_CART);

  page_lv.lpig=&Ico16;
  page_lv.display_mode=1;
  int page_lv_w=page_lv.get_max_width(XD);

  if (StandardShow(page_lv_w+500,460,T("General Info"),
      ICO16_GENERALINFO,0,(LPWINDOWPROC)WinProc))return;

  while (page_lv.select_item_by_data(Page,1)==-1) Page=INFOPAGE_ABOUT;
  page_lv.id=60000;
  page_lv.create(XD,Handle,0,0,page_lv_w,460,listview_notify_proc,this);

  gb.create(XD,Handle,page_lv.w,0,500,460,
            NULL,this,BT_STATIC | BT_BORDER_NONE,"",-4,hxc::col_bk);

  CreatePage(Page);
  if (StemWin) InfBut.set_check(true);
            	
  XMapWindow(XD,Handle);
  XFlush(XD);
}

void TGeneralInfo::CreateAboutPage()
{
  EasyStr Text=EasyStr("Steem Engine v")+(char*)stem_version_text+"\n";
	Text+="Built " __DATE__ "\n";
  Text+="Written by Anthony & Russell Hayward";
  if (TranslateBuf){
    Text+="\n\n";
    Text+=T("Translation by [Your Name]");
  }
  about.border=0;about.pad_x=0;about.sy=0;
  about.textheight=(hxc::font->ascent)+(hxc::font->descent)+2;

	about.set_text(Text);
  // hxc::fontinfo has been set up by the creation of the tab buttons causing a load_res
	int n_lines=about.wordwrap(gb.w-20,hxc::font);
	int h=n_lines*about.textheight;
	about.create(XD,gb.handle,10,10,gb.w-20,h,hxc::col_bk);

	EasyStr ta=Credits[0];
	int nta=1;
	while (Credits[nta]){
		ta+=EasyStr("\n")+Credits[nta];
		nta++;
	}
	thanks.set_text(ta);

  thanks_label.create(XD,gb.handle,10,20+h,0,0,NULL,this,
                    BT_LABEL,T("Thanks To"),-59,hxc::col_bk);
  thanks.create(XD,gb.handle,10,thanks_label.y+thanks_label.h,
                  gb.w-20,gb.h-h-70,hxc::col_white);

	steem_link.create(XD,gb.handle,10,thanks.y+thanks.h+10,0,0,
            hyperlink_np,this,BT_LINK|BT_TEXT,STEEM_WEB,-56,
            hxc::col_bk);
}


void TGeneralInfo::CreateSpeedPage()
{
}

void TGeneralInfo::CreateLinksPage()
{
	sa.create(XD,gb.handle,1,1,gb.w-2,gb.h-2);
	int y=10;
  EasyStringList desc_sl,link_sl;
  GetHyperlinkLists(desc_sl,link_sl);

  hxc_button *b;
  for (int n=0;n<desc_sl.NumStrings;n++){
    if (desc_sl[n].Data[0]==0){
      b=new hxc_button(XD,sa.handle,10,y,0,0,
          hyperlink_np,this,BT_LINK|BT_TEXT,
          Str(desc_sl[n].String)+"|"+link_sl[n].String,0,hxc::col_bk);
      y+=b->h+5;
    }else{
      y+=10;
      b=new hxc_button(XD,sa.handle,10,y,0,0,
          NULL,this,BT_STATIC|BT_TEXT,
          desc_sl[n].String,0,hxc::col_bk);
      y+=b->h+5;
    }
  }
  sa.adjust();
}

void TGeneralInfo::CreateReadmePage(int p)
{
	switch (p){
		case INFOPAGE_README: ShowTheReadme("win32.help"); break;
		case INFOPAGE_HOWTO_DISK: ShowTheReadme("disk image howto.txt"); break;
		case INFOPAGE_HOWTO_CART: ShowTheReadme("cart image howto.txt"); break;
		case INFOPAGE_FAQ: ShowTheReadme("FAQ"); break;
		case INFOPAGE_UNIXREADME:	ShowTheReadme("README",true); break;
	}
}

void TGeneralInfo::ShowTheReadme(char* fn,bool stripreturns)
{
  FILE *f=fopen(RunDir+"/"+fn,"rb");
  if (f){
    int Len;
    fseek(f,0,SEEK_END);
    Len=ftell(f);
    fseek(f,0,SEEK_SET);

		readme.text.SetLength(Len);
    fread(readme.text.Text,1,Len,f);
    fclose(f);
		if (stripreturns){
			int ocn;
			for (int cn=0;cn<Len;cn++){
				if (*(readme.text.Text+cn)=='\n'){
					ocn=cn++;
					if (*(readme.text.Text+cn)!='.'){
  					while (*(readme.text.Text+cn)=='\n')cn++;
  					if (cn==ocn+1){ //just one return
  						*(readme.text.Text+ocn)=' ';
  					}
  				}
				}
			}
		}
  }
  if (hxc::find(gb.handle,500)==NULL){
    hxc_button *p_lab=new hxc_button(XD,gb.handle,5,5,0,25,NULL,NULL,BT_LABEL,T("Search"),0,hxc::col_bk);
    
    int w=hxc::get_text_width(XD,T("Find"))+8;
    hxc_button *p_but=new hxc_button(XD,gb.handle,gb.w-5-w,5,w,25,button_notifyproc,this,BT_TEXT,T("Find"),500,hxc::col_bk);
    
    hxc_edit *ed=new hxc_edit(XD,gb.handle,p_lab->x+p_lab->w+5,5,p_but->x-5-(p_lab->x+p_lab->w+5),25,
                              edit_notify_proc,this);
    ed->id=501;
    ed->set_text(GetCSFStr("Info","SearchText","",INIFile));
  }

  readme.sy=0;
  readme.wordwrapped=false;
	readme.create(XD,gb.handle,5,35,gb.w-10,gb.h-40,hxc::col_white,true);
  last_find_idx=0;
}
//---------------------------------------------------------------------------
void TGeneralInfo::Hide()
{
  if (XD==NULL || Handle==0) return;

  StandardHide();

  if (StemWin) InfBut.set_check(0);
}
//---------------------------------------------------------------------------
int TGeneralInfo::WinProc(TGeneralInfo *This,Window Win,XEvent *Ev)
{
  switch (Ev->type){
    case ClientMessage:
      if (Ev->xclient.message_type==hxc::XA_WM_PROTOCOLS){
        if (Atom(Ev->xclient.data.l[0])==hxc::XA_WM_DELETE_WINDOW){
          This->Hide();
        }
      }
      break;
  }

  return PEEKED_MESSAGE;
}
//---------------------------------------------------------------------------
int TGeneralInfo::button_notifyproc(hxc_button *b,int mess,int*ip)
{
  if (b->id==500){
    TGeneralInfo *This=(TGeneralInfo*)(b->owner);  
    if (mess==BN_CLICKED) edit_notify_proc((hxc_edit*)hxc::find(This->gb.handle,501),EDN_RETURN,0);
    return 0;
  }
	return 0;
}
//---------------------------------------------------------------------------
int TGeneralInfo::listview_notify_proc(hxc_listview *LV,int Mess,int I)
{
  TGeneralInfo *This=(TGeneralInfo*)(LV->owner);
  if (Mess==LVN_SELCHANGE || Mess==LVN_SINGLECLICK){
    int NewPage=LV->sl[LV->sel].Data[1];
    if (This->Page!=NewPage){
      hxc::destroy_children_of(This->gb.handle);
      This->Page=NewPage;
      This->CreatePage(This->Page);
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
int TGeneralInfo::edit_notify_proc(hxc_edit *ed,int mess,int i)
{
  TGeneralInfo *This=(TGeneralInfo*)(ed->owner);
  if (mess==EDN_CHANGE){
    WriteCSFStr("Info","SearchText",ed->text,INIFile);
    This->last_find_idx=0;
  }else if (mess==EDN_RETURN){
    if (ed->text.Empty()) return 0;
    
    Str t=This->readme.text.Text;
    strupr(t);
    for (int n=0;n<2;n++){
      char *s=strstr(t.Text + This->last_find_idx,ed->text.UpperCase());
      if (s){
        int line=This->readme.get_line_from_character_index(DWORD(s-t.Text));
        This->readme.highlight_lines.DeleteAll();
        This->readme.highlight_lines.Add(line);
        This->readme.scrollto(max(line-3,0)*This->readme.textheight);
        This->readme.draw(true);

        This->last_find_idx=int(s-t.Text)+1;
        if (line<This->readme.n_lines-1){
          while (This->readme.get_line_from_character_index(This->last_find_idx)==line) This->last_find_idx++;
        }       
        break;
      }else{
        if (This->last_find_idx==0) break;
        This->last_find_idx=0;
      }
    }
  }
  return 0;
}
//---------------------------------------------------------------------------

