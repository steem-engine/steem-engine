/*---------------------------------------------------------------------------
FILE: mr_static.cpp
MODULE: Steem
CONDITION: _DEBUG_BUILD
DESCRIPTION: A class to encapsulate the labels that contain various data
in the debugger.
---------------------------------------------------------------------------*/

mr_static::~mr_static()
{
  active=false;
  for (int n=0;n<MAX_MR_STATICS;n++){
    if (m_s[n]==this){
      for(int m=n;m<MAX_MR_STATICS-1;m++)m_s[m]=m_s[m+1];
      m_s[MAX_MR_STATICS-1]=NULL;
      n--;
    }
  }
  DestroyWindow(handle);
  if (hLABEL) DestroyWindow(hLABEL);
}

void mr_static_delete_children_of(HWND Win)
{
  for (int n=0;n<MAX_MR_STATICS;n++){
    if (m_s[n]!=NULL){
      if (m_s[n]->owner==Win){
        delete m_s[n];
        n--;
      }
    }
  }
}


mr_static::mr_static(char*new_label,char*new_name,int x,int y,
    HWND new_owner,HMENU id,MEM_ADDRESS new_ad,int new_bytes,_mr_static_type new_type,
    bool new_editflag,mem_browser*new_mem_browser_update)
{
  int n,label_width,text_width;
  for(n=0;n<MAX_MR_STATICS && m_s[n]!=NULL;n++);

  if(n<MAX_MR_STATICS){

    bytes=new_bytes;
    editflag=new_editflag;
    owner=new_owner;
    type=new_type;

    label_width=get_text_width(new_label);

    if (*new_label){
      hLABEL=CreateWindowEx(0,"Static",new_label,WS_VISIBLE | WS_CHILD | SS_RIGHT,
          x,y+3,label_width,22,owner,id,Inst,NULL);
      SendMessage(hLABEL,WM_SETFONT,(UINT)fnt,0);
      x+=label_width;
    }else{
      hLABEL=NULL;
    }

    iolist_entry *il=NULL;
    if (type==MST_IOLIST || type==MST_HISTORIC_IOLIST){
      il=(iolist_entry*)new_ad;
      text_width=iolist_box_width(il)+7;
    }else{
      int nchars=new_bytes*2;
      if (type==MST_DECIMAL) nchars=strlen(Str(1 << min(new_bytes*8,31)))+(new_bytes!=3); //+1 for -
      char tb[30];
      memset(tb,'8',nchars);
      tb[nchars]=0;
      text_width=get_text_width(tb);
      if (type!=MST_DECIMAL){
        text_width+=max(nchars,4); // D is bigger than 8
      }else{
        text_width+=4; // 2 to the left and right
      }
    }

    handle=CreateWindowEx(512,"Steem Mr Static Control","",WS_VISIBLE | WS_CHILD | SS_NOTIFY | SS_CENTER,
        x,y,text_width+4,22,owner,id,Inst,NULL);

    TOOLINFO ti;
    ti.cbSize=sizeof(TOOLINFO);
    ti.uFlags=TTF_IDISHWND | TTF_SUBCLASS;
    // This should be owner but it is easier if the TTN_NEEDTEXT is sent
    // straight to the Mr. Static.
    ti.hwnd=handle;
    ti.uId=(UINT)handle;
    ti.lpszText=LPSTR_TEXTCALLBACK;
    SendMessage(ToolTip,TTM_ADDTOOL,0,(LPARAM)&ti);

    SetWindowLong(handle,GWL_USERDATA,(long)this);
    mem_browser_update=NULL;ile=NULL;
    if(*new_name){
      strcpy(name,new_name);
      if(name[strlen(name)-1]=='=')name[strlen(name)-1]=0;
    }
    if(type==MST_MEMORY){
      ptr=NULL;ad=new_ad;
    }else if(type==MST_HISTORIC_MEMORY){
      ptr=(long*)new_mem_browser_update;ad=new_ad;
    }else if(type==MST_HISTORIC_IOLIST){
      ptr=(long*)new_mem_browser_update;ad=il->ad;
      ile=il; 
    }else if(type==MST_IOLIST){
      ptr=NULL;ad=il->ad;
      ile=il;
    }else{
      ptr=(long*)new_ad;ad=0;
      mem_browser_update=new_mem_browser_update;
    }
    active=true;
    update();

    m_s[n]=this;
  }
//  return &(m_s[n]);
}


void mr_static::setup_contextmenu()
{
  char tb[80];
  insp_menu_subject_type=0; //0=mr_static
  insp_menu_subject=(void*)this;

  DeleteAllMenuItems(insp_menu);

  strcpy(insp_menu_long_name[0],name);
  strcat(insp_menu_long_name[0]," = ");GetWindowText(handle,tb,80);
  strcat(insp_menu_long_name[0],tb);
  insp_menu_long_bytes[0]=bytes;
  insp_menu_long[0]=HexToVal(tb);

  insp_menu_long_bytes[1]=0;
  insp_menu_long_bytes[2]=0;
  if(type==MST_MEMORY || type==MST_HISTORIC_MEMORY || type==MST_IOLIST || type==MST_HISTORIC_IOLIST){
    strcpy(insp_menu_long_name[1],"address of this, ");
    strcat(insp_menu_long_name[1],HEXSl(ad,6));
    insp_menu_long_bytes[1]=3;
    insp_menu_long[1]=ad;

  }else if(type==MST_REGISTER){
    strcpy(insp_menu_long_name[1],name);strcat(insp_menu_long_name[1],".W");
    insp_menu_long_bytes[1]=2;
    insp_menu_long[1]=(long)*(WORD*)(ptr);
    AppendMenu(insp_menu,MF_ENABLED | MF_STRING,3003,
      "Register browser");
  }
  insp_menu_setup();
  if(type!=MST_IOLIST && type!=MST_HISTORIC_IOLIST){
    AppendMenu(insp_menu,MF_ENABLED | MF_STRING,3016,"Edit");
  }

}


LRESULT __stdcall mr_static_WndProc(HWND Win,UINT Mess,UINT wPar,long lPar)
{
  static EasyStr ToolText;
  mr_static *ms;

	switch (Mess){
    case WM_SETTEXT:
      InvalidateRect(Win,NULL,0);
      break;
    case WM_PAINT:
    {
      RECT box;
      char *Text;
      int Len;
      ms=(mr_static*)GetWindowLong(Win,GWL_USERDATA);

      GetClientRect(Win,&box);

      if (ms->type==MST_IOLIST || ms->type==MST_HISTORIC_IOLIST){ // these can change on the fly
        InvalidateRect(Win,NULL,true);
      }

      PAINTSTRUCT ps;
      BeginPaint(Win,&ps);

      SelectObject(ps.hdc,fnt);

      if (ms->type==MST_IOLIST){ //special draw
        iolist_box_draw(ps.hdc,0,0,box.right,15,ms->ile,NULL);
      }else if(ms->type==MST_HISTORIC_IOLIST){ //special draw
        iolist_box_draw(ps.hdc,0,0,box.right,15,ms->ile,(BYTE*)(ms->ptr));
      }else{
        if (ms->editflag){
          HBRUSH Br=CreateSolidBrush(MidGUIRGB);
          FillRect(ps.hdc,&box,Br);
          DeleteObject(Br);
        }else{
          HBRUSH Br=CreateSolidBrush(DkMidGUIRGB);
          FillRect(ps.hdc,&box,Br);
          DeleteObject(Br);
        }
        Len=DefWindowProc(Win,WM_GETTEXTLENGTH,0,0)+1;
        Text=new char[Len+1];
        DefWindowProc(Win,WM_GETTEXT,Len,(long)Text);

        SetBkMode(ps.hdc,TRANSPARENT);
        SetTextColor(ps.hdc,GetSysColor(COLOR_BTNTEXT));
        if (ms->type==MST_DECIMAL) box.left+=2, box.right-=4;
        DrawText(ps.hdc,Text,-1,&box,int((ms->type==MST_DECIMAL) ? 0:DT_CENTER) | DT_VCENTER | DT_SINGLELINE);

        delete[] Text;
      }
      EndPaint(Win,&ps);
      return 0;
    }
    case WM_ERASEBKGND:
      return 1;
    case WM_CONTEXTMENU:
    {
      ms=(mr_static*)GetWindowLong(Win,GWL_USERDATA);
      ms->setup_contextmenu();
      TrackPopupMenu(insp_menu,TPM_LEFTALIGN	| TPM_LEFTBUTTON,
                      LOWORD(lPar),HIWORD(lPar),0,DWin,NULL);
      return 0;
    }
    case WM_NOTIFY:
      if (((NMHDR*)lPar)->code==TTN_NEEDTEXT){
        int Len=DefWindowProc(Win,WM_GETTEXTLENGTH,0,0)+1;
        char *Text=new char[Len+1];
        DefWindowProc(Win,WM_GETTEXT,Len,(long)Text);

        ms=(mr_static*)GetWindowLong(Win,GWL_USERDATA);
        DWORD Val;
        if (ms->type==MST_DECIMAL){
          Val=(DWORD)atoi(Text);
        }else{
          Val=HexToVal(Text);
        }

        delete[] Text;

        TOOLTIPTEXT *ttt=(TOOLTIPTEXT*)lPar;

        ttt->szText[0]=0;
        ttt->hinst=NULL;

        ToolText=(EasyStr("L:")+Val).RPad(13,' ');
        ToolText+=(EasyStr("W:")+WORD(Val)).RPad(8,' ');
        ToolText+=(EasyStr("B:")+BYTE(Val)).RPad(6,' ');

        EasyStr BinBuf;
        BinBuf.SetLength(40);
        ultoa(Val,BinBuf,2);
        BinBuf.LPad(32,'0');
        BinBuf.Insert("  -  ",24);
        BinBuf.Insert("     ",16);
        BinBuf.Insert("  -  ",8);
        ToolText+=BinBuf;

        ttt->lpszText=ToolText.Text;
      }
      break;
    case WM_LBUTTONDOWN:
      ms=(mr_static*)GetWindowLong(Win,GWL_USERDATA);
      if(ms->type==MST_IOLIST || ms->type==MST_HISTORIC_IOLIST){
        int x=LOWORD(lPar);
        if(ms->type==MST_IOLIST)iolist_box_click(x,ms->ile,NULL);
        else iolist_box_click(x,ms->ile,(BYTE*)(ms->ptr));
//        InvalidateRect(Win,NULL,false);
        update_register_display(false);
      }else if (ms->editflag){
        set_DWin_edit(0,ms,0,0);
      }
      return 0;
//      break;
    case WM_DESTROY:
    {
      ms=(mr_static*)GetWindowLong(Win,GWL_USERDATA);
      ms->active=false;

      TOOLINFO ti;
      ti.cbSize=sizeof(TOOLINFO);
      ti.hwnd=Win;
      ti.uId=(UINT)Win;
      SendMessage(ToolTip,TTM_DELTOOL,0,(LPARAM)&ti);
      break;
    }
  }
	return DefWindowProc(Win,Mess,wPar,lPar);
}


void mr_static::edit(char*ttt)
{
  char tb[42];
  long val=HexToVal(ttt);
  if (mem_browser_update) val&=~1; // Can't be odd!
  val&=(unsigned long)(-1)>>(8*(4-(bytes)));
  if (type==MST_MEMORY){
    switch(bytes){
      case 1:
        d2_poke(ad,(BYTE)val);break;
      case 2:
        d2_dpoke(ad,(WORD)val);break;
      case 4:
        d2_lpoke(ad,(LONG)val);break;
    }
  }else{
    switch (bytes){
      case 1:
        *(BYTE*)(ptr)=LOBYTE(val);break;
      case 2:
        *(WORD*)(ptr)=LOWORD(val);break;
      case 3:case 4:
        *(ptr)=val;break;
    }
  }
  update();

  if (type==MST_REGISTER){
    for (int n=0;n<MAX_MEMORY_BROWSERS;n++){
      if (m_b[n]!=NULL){
        if (m_b[n]->disp_type==DT_REGISTERS){
          m_b[n]->update();
        }
      }
    }
  }


  if (mem_browser_update!=NULL) mem_browser_update->update();
  if ((MEM_ADDRESS*)ptr==&xbios2) draw(false);
  if ((MEM_ADDRESS*)ptr==&pc){
  	SET_PC(PC32);
  }
  SendMessage(handle,WM_GETTEXT,40,(long)tb);
  SetWindowText(DWin_edit,tb);
}

void mr_static_update_all()
{
  for(int n=0;n<MAX_MR_STATICS;n++){
    if(m_s[n]!=NULL){
      if(m_s[n]->active){
        m_s[n]->update();
      }
    }
  }
}

void mr_static::update()
{
  char tb[50];
  if (type==MST_MEMORY || type==MST_IOLIST){ //memory
    switch(bytes){
    case 1:strcpy(tb,HEXSl(d2_peek(ad),2));break;
    case 2:strcpy(tb,HEXSl(d2_dpeek(ad),4));break;
    case 4:strcpy(tb,HEXSl(d2_lpeek(ad),8));break;
    }
  }else if (type==MST_MEM_BROWSER_ADDRESS){ //check if pseudo address
    if (mem_browser_update->mode==MB_MODE_IOLIST){
      strcpy(tb,""); // Not seen
    }else{
      strcpy(tb,HEXSl(*(ptr),6));
    }
  }else{  //ie. MST_REGISTER,MST_HISTORIC_MEMORY,MST_ADDRESS,MST_HISTORIC_IOLIST,
    DWORD num=*ptr;
    if (bytes<=3) num&=(1 << (bytes*8))-1;
    if (type==MST_DECIMAL){
      Str text_conv="%i";
      if (bytes==1) text_conv="%ci";
      if (bytes==2) text_conv="%hi";
      sprintf(tb,text_conv,num);
    }else{
      strcpy(tb,HEXSl(num,bytes*2));
    }
  }
  SetWindowText(handle,tb);
}

