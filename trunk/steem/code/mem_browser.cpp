/*---------------------------------------------------------------------------
FILE: mem_browser.cpp
MODULE: Steem
CONDITION: _DEBUG_BUILD
DESCRIPTION: The code for Steem's memory browsers, used heavily in the debug
build to view memory and I/O areas.
---------------------------------------------------------------------------*/

LRESULT __stdcall mem_browser_WndProc(HWND,UINT,UINT,long);
WNDPROC Old_mem_browser_WndProc;
//---------------------------------------------------------------------------
void mem_browser::listbox_add_line(HWND hlb,int i,char**txt,int count)
{
  LV_ITEM pitem;
  pitem.mask=LVIF_TEXT;
  pitem.iItem=i;
  pitem.iSubItem=0;
  pitem.pszText=txt[0];
  pitem.state=0;
  pitem.stateMask=0;
  pitem.lParam=text_column;
  pitem.iImage=0;
  SendMessage(hlb,LVM_INSERTITEM,0,LPARAM(&pitem));
  for (int m=0;m<max(count,1);m++){
    pitem.iSubItem=m;
    pitem.pszText=txt[m];
    SendMessage(hlb,LVM_SETITEM,0,LPARAM(&pitem));
  }
}
//---------------------------------------------------------------------------
void mem_browser::new_window(MEM_ADDRESS address,type_disp_type new_disp_type)
{
  int n,y;
  RECT rc;
  for (n=0;n<MAX_MEMORY_BROWSERS;n++) if (m_b[n]==NULL) break;
  if (n>=MAX_MEMORY_BROWSERS){
    MessageBox(NULL,"Can't open any more memory browsers. Surely that's enough!","No More!",
                  MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_TOPMOST);
  }else{
    Str Title="Memory";
    if (IS_IOLIST_PSEUDO_ADDRESS(address)){
      switch (address & 0xfffff000){
        case IOLIST_PSEUDO_AD_PSG:
          Title="PSG (YM-2149)";
          break;
        case IOLIST_PSEUDO_AD_FDC:
          Title="FDC (WD-1772) + DMA";
          break;
        case IOLIST_PSEUDO_AD_IKBD:
          Title="IKBD";
          break;
      }
    }else if (new_disp_type==DT_REGISTERS){
      Title="Registers";
    }
    owner=CreateWindowEx(ex_style,"Steem Mem Browser Window",Title,
        WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX,
        10,20,640,400,NULL,NULL,Inst,0);
    if (IsWindow(owner)==0 || owner==NULL){
      MessageBox(NULL,"Failed to open new window. Bad problem with Windows!!! Time to restart!!!!","Windows Error",
                    MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_TOPMOST);
    }else{
      update_icon();

      SetWindowLong(owner,GWL_USERDATA,(long)this);

      ad=address;
      disp_type=new_disp_type;
      mode=MB_MODE_STANDARD;
      if ((ad & 0xff000000)==IOLIST_PSEUDO_AD) mode=MB_MODE_IOLIST;
      editflag=true;
      editbox=NULL;

      GetClientRect(owner,&rc);
      y=30;
      if (new_disp_type==DT_REGISTERS || IS_IOLIST_PSEUDO_ADDRESS(ad)) y=2;

      //create the list box
      handle=CreateWindowEx(512,WC_LISTVIEW,"",
          LVS_REPORT | LVS_SHAREIMAGELISTS | LVS_NOSORTHEADER | LVS_OWNERDRAWFIXED |
          WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS,
          10,y,rc.right-20,rc.bottom-5-y,owner,(HMENU)1,Inst,NULL);

      SetWindowLong(handle,GWL_WNDPROC,(long)mem_browser_WndProc);

      init();
      update();

      if (disp_type==DT_REGISTERS){
        ad=0;
      }else{
        if (IS_IOLIST_PSEUDO_ADDRESS(ad)==0){
          editbox=new mr_static(/*label*/"",/*name*/"browser address",/*x*/10,/*y*/2,
              /*owner*/owner,/*id*/(HMENU)3,/*pointer*/(MEM_ADDRESS)&ad,
              /*bytes*/ 3,/*regflag*/ MST_MEM_BROWSER_ADDRESS, /*editflag*/true,
              /*mem_browser to update*/this);

          HWND Win=CreateWindowEx(512,"Combobox","",WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
                                        75,2,100,160,owner,(HMENU)2,Inst,NULL);
          SendMessage(Win,CB_ADDSTRING,0,(long)"Instructions");
          SendMessage(Win,CB_ADDSTRING,0,(long)"Memory");
          SendMessage(Win,CB_SETCURSEL,(WPARAM)disp_type,0);


          CreateWindow("Static","",WS_VISIBLE | WS_CHILD | SS_ETCHEDVERT,
              180,0,2,27,owner,(HMENU)7,Inst,NULL);

          Win=CreateWindowEx(512,"Edit","",WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,
              187,2,80,23,owner,(HMENU)8,Inst,NULL);
          SendMessage(Win,EM_LIMITTEXT,200,0);
          OldEditWndProc=(WNDPROC)SetWindowLong(Win,GWL_WNDPROC,(long)FindEditWndProc);

          TOOLINFO ti;
          ti.cbSize=sizeof(TOOLINFO);
          ti.uFlags=TTF_IDISHWND | TTF_SUBCLASS;
          ti.hwnd=owner;
          ti.uId=(UINT)Win;
          ti.lpszText="Type in value you want to search for. For ASCII search prefix with \" (case sensitive)."
                      "Hexadecimal ($ or 0x) and binary (%) are also accepted, the length "
                      "determining how many bytes to find ($00ff is longer than $ff). "
                      "For non-text searches add 'w' to make sure the number you find "
                      "starts on a word boundary.";
          SendMessage(ToolTip,TTM_ADDTOOL,0,(LPARAM)&ti);

          CreateWindow("Button","Find Up",WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
              272,2,65,23,owner,(HMENU)9,Inst,NULL);

          CreateWindow("Button","Find Down",WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
              342,2,65,23,owner,(HMENU)10,Inst,NULL);


          CreateWindow("Static","",WS_VISIBLE | WS_CHILD | SS_ETCHEDVERT,
              412,0,2,27,owner,NULL,Inst,NULL);

          CreateWindow("Button","Dump->",WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
              419,2,60,23,owner,(HMENU)4,Inst,NULL);

          Win=CreateWindowEx(512,"Combobox","",WS_VISIBLE | WS_CHILD | WS_VSCROLL | CBS_DROPDOWN,
                              484,2,65,300,owner,(HMENU)5,Inst,NULL);
          for (int n=5;n<=256;n+=5) CBAddString(Win,EasyStr(n)+"Kb",n*1024);
          CBAddString(Win,"512Kb",512*1024);
          CBAddString(Win,"640Kb",640*1024);
          CBAddString(Win,"1MB",1024*1024);
          CBAddString(Win,"2MB",2*1024*1024);
          CBAddString(Win,"2.5MB",(2048+512)*1024);
          CBAddString(Win,"4MB",4*1024*1024);
          SendMessage(Win,WM_SETTEXT,0,LPARAM("100Kb"));

          CreateWindow("Button","Load",WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,484+65+5,2,60,23,owner,(HMENU)6,Inst,NULL);
        }
      }
      SetWindowAndChildrensFont(owner,fnt);
      CentreWindow(owner,0);
      ShowWindow(owner,SW_SHOW);
      if (GetDlgItem(owner,5)) SetFocus(GetDlgItem(owner,5));
      SetFocus(handle);
    }
    m_b[n]=this;
  }
}
//---------------------------------------------------------------------------
void mem_browser::init()
{
  LV_COLUMN lc;
  ZeroMemory(&lc,sizeof(LV_COLUMN));

  lc.mask=LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  lc.fmt=LVCFMT_LEFT;

  for (int k=columns-1;k>=0;k--) SendMessage(handle,LVM_DELETECOLUMN,k,0);
  columns=0;
  text_column=-1;
  disa_column=-1;
  mon_column=-1;
  break_column=-1;
  hex_column=-1;

  if (disp_type==DT_INSTRUCTION){
    lc.cx=30;
    lc.pszText="Reg";
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);
    lc.cx=20;
    lc.pszText="Bk";
    break_column=columns;
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);
    lc.cx=35;
    lc.pszText="Mon";
    mon_column=columns;
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);
    lc.cx=65;
    lc.pszText="Address";
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);
    lc.cx=how_big_is_0000*2+8;
    lc.pszText="Hex";
    hex_column=columns;
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);
    lc.cx=240;
    lc.pszText="Disassembly";
    disa_column=columns;
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);
  }else if (disp_type==DT_MEMORY){
    bool InIO=(ad & 0xffffff)>MEM_IO_BASE;
    bool PSG=(ad & 0xfffff000)==IOLIST_PSEUDO_AD_PSG;
    bool FDC=(ad & 0xfffff000)==IOLIST_PSEUDO_AD_FDC;
    bool IKBD=(ad & 0xfffff000)==IOLIST_PSEUDO_AD_IKBD;
    bool Pseudo=(PSG || FDC || IKBD);

    lc.cx=30;
    if (Pseudo) lc.cx=0;
    lc.pszText="Reg";
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);

    lc.cx=20;
    if (InIO || init_text || Pseudo) lc.cx=0;
    lc.pszText="Bk";
    break_column=columns;
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);

    lc.cx=35;
    if (init_text || Pseudo) lc.cx=0;
    lc.pszText="Mon";
    mon_column=columns;
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);

    lc.cx=65;
    lc.pszText="Address";
    if (Pseudo) lc.cx=0;
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);

    lc.cx=how_big_is_0000*2+8;
    if (InIO) lc.cx=how_big_is_0000+8;
    if (init_text || Pseudo) lc.cx=0;
    lc.pszText="Hex";
    hex_column=columns;
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);

    lc.cx=150;
    lc.pszText="Disassembly";
    if (mode==MB_MODE_STACK) lc.cx=0;
    if (init_text) lc.cx=0;
    if (Pseudo){
      lc.pszText="Description";
    }else{
      disa_column=columns;
    }
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);

    lc.cx=50;
    if (InIO || Pseudo) lc.cx=0;
    if (init_text) lc.cx=800;
    lc.pszText="Text";
    text_column=columns;
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);

    lc.cx=120;
    lc.pszText="Decimal";
    if (init_text) lc.cx=0;
    if (Pseudo){
      lc.cx=how_big_is_0000*2; // Max 8 chars
      lc.pszText="Value";
    }
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);

    lc.cx=600;
    if (init_text) lc.cx=0;
    lc.pszText="Binary";
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);
  }else if (disp_type==DT_REGISTERS){
    lc.cx=70;
    lc.pszText="Reg";
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);
    lc.cx=how_big_is_0000*2+8;
    lc.pszText="Hex";
    hex_column=columns;
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);
    lc.cx=0;
    lc.pszText="Text";
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);
    lc.cx=180;
    lc.pszText="Decimal";
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);
    lc.cx=300;
    lc.pszText="Binary";
    SendMessage(handle,LVM_INSERTCOLUMN,columns++,(long)&lc);
  }
}
//---------------------------------------------------------------------------
int mem_browser::calculate_wpl()
{
  int cwid=SendMessage(handle,LVM_GETCOLUMNWIDTH,4,0)-8;
  if (cwid<=1) return 32;
  return max(1,(int)(cwid/how_big_is_0000)); //words per line
}
//---------------------------------------------------------------------------
char* mem_browser::get_mem_mon_string(void *p)
{
  DEBUG_ADDRESS *pda=(DEBUG_ADDRESS*)p;
  WORD mask=0;
  bool readonly=0;
  if (pda->bwr & BIT_2) mask|=pda->mask[1],readonly=true;
  if (pda->bwr & BIT_1) mask|=pda->mask[0],readonly=0;
  switch (mask){
    case 0xffff: return (readonly ? "E":"B");
    case 0xff00: return (readonly ? "F":"C");
    case 0x00ff: return (readonly ? "G":"D");
  }
  return ".";
}
//---------------------------------------------------------------------------
Str mem_browser::get_hex_map(MEM_ADDRESS ad)
{
  char word_map[3]={0,0,0};
  for (int i=0;i<debug_ads.NumItems;i++){
    if (debug_ads[i].ad==ad){
      if (debug_ads[i].bwr & BIT_1){
        if (debug_ads[i].mask[0] & 0xff00) word_map[0]|=1;
        if (debug_ads[i].mask[0] & 0x00ff) word_map[1]|=1;
      }
      if (debug_ads[i].bwr & BIT_2){
        if (debug_ads[i].mask[1] & 0xff00) word_map[0]|=2;
        if (debug_ads[i].mask[1] & 0x00ff) word_map[1]|=2;
      }
    }
  }
  word_map[0]+='0';
  word_map[1]+='0';
  return Str(word_map);
}
//---------------------------------------------------------------------------
void mem_browser::get_breakpoint_labels(MEM_ADDRESS ad,int bpl,char *t[3])
{
  ad&=0xffffff;

  char regname[20];strcpy(regname,"(pc)");
  MEM_ADDRESS rv=pc,ad2=(ad+bpl) & 0xffffff;
  int n=-1;

  t[0][0]=0; // registers
  while (n<8){
    if (rv>=ad && rv<ad2){
      if (strlen(t[0])) strcat(t[0]," ");
      if (rv>ad) strcat(t[0],STRS( (int)ad-(int)rv ));
      strcat(t[0],regname);
    }

    n++;
    regname[1]='a';regname[2]=char('0'+n);rv=r[n+8];
  }

  t[1][0]=0; // breakpoints
  t[2][0]=0; // monitors
  bool add_dotdot=0;
  for (;ad<ad2;ad+=2){
    ad&=0xffffff;
    for (int i=0;i<debug_ads.NumItems;i++){
      if (debug_ads[i].ad==ad){
        int bkmode=debug_ads[i].mode,monmode=bkmode;
        if (bkmode==1){
          bkmode=breakpoint_mode;
          monmode=monitor_mode;
        }
        if (debug_ads[i].bwr & BIT_0){
          if (add_dotdot) strcat(t[1],"..");
          strcat(t[1],(bkmode==0 ? "a":"A"));
        }else if (debug_ads[i].name[0]){
          if (add_dotdot) strcat(t[1],"..");
          strcat(t[1],"H");
        }
        if (debug_ads[i].bwr & (BIT_1 | BIT_2)){
          if (add_dotdot) strcat(t[2],"..");
          Str c=get_mem_mon_string(&debug_ads[i]);
          if (monmode==0) strlwr(c);
          strcat(t[2],c);
        }
      }
    }
    add_dotdot=true;
  }
}
//---------------------------------------------------------------------------
MEM_ADDRESS mem_browser::get_address_from_row(int row)
{
  char ttt[256];
  LV_ITEM item;
  item.iItem=row;
  item.iSubItem=3;
  item.mask=LVIF_TEXT;
  item.pszText=ttt;
  item.cchTextMax=250;
  SendMessage(handle,LVM_GETITEM,0,(LPARAM)&item);
  return HexToVal(ttt);
}
//---------------------------------------------------------------------------
void mem_browser_update_all()
{
  m_b_stack.update();
  m_b_mem_disa.update();
  m_b_trace.update();
  for (int n=0;n<MAX_MEMORY_BROWSERS;n++){
    if (m_b[n]) m_b[n]->update();
  }
}
//---------------------------------------------------------------------------
void mem_browser::draw(DRAWITEMSTRUCT *di)
{
  HBRUSH br;
  COLORREF oldtextcol,oldbkcol;
  char Text[1024];

  oldtextcol=GetTextColor(di->hDC);
  oldbkcol=GetBkMode(di->hDC);  //Color

  COLORREF BackCol=SendMessage(di->hwndItem,LVM_GETBKCOLOR,0,0);
  SetBkMode(di->hDC,TRANSPARENT);
  br=CreateSolidBrush(BackCol);
  SetTextColor(di->hDC,SendMessage(di->hwndItem,LVM_GETTEXTCOLOR,0,0));

  FillRect(di->hDC,&(di->rcItem),br);
  DeleteObject(br);

  if (di->itemID < 0xffff){
    LV_ITEM lvi;
    lvi.pszText=Text;
    lvi.cchTextMax=1023;
    lvi.iItem=di->itemID;

    int Wid=SendMessage(di->hwndItem,LVM_GETCOLUMNWIDTH,0,0),Col=0;
    di->rcItem.right=di->rcItem.left+Wid-2;

    LOOP{
      lvi.iSubItem=Col;
      lvi.mask=LVIF_TEXT;
      Text[0]=0;
      SendMessage(di->hwndItem,LVM_GETITEM,0,(LPARAM)&lvi);

      HFONT old_font=NULL;
      if (text_column==Col || (disa_column==Col && debug_monospace_disa)){
        old_font=(HFONT)SelectObject(di->hDC,GetStockObject(ANSI_FIXED_FONT));
      }

      di->rcItem.left+=2;

      if (strstr(Text,"$$IOL$$")){ //special display
        int x=di->rcItem.left+2;
        char*c1=Text;
        char*c2;
        LOOP{
          c2=strstr(c1,"$$IOL$$");
          if(c2==NULL){
            c2=c1+strlen(c1);
          }
          if(c2>c1){ //text to display
            *(c2-1)=0; //first part of string
            TextOut(di->hDC,x,di->rcItem.top,c1,strlen(c1));
            x+=SendMessage(handle,LVM_GETSTRINGWIDTH,0,(LPARAM)c1);
          }
          c1=c2;
          if(!*c1)break; //end of string
          //special box

          memcpy(d2_t_buf,c1+7,8); //copy out hex stuff
          d2_t_buf[8]=0;
          iolist_entry*il=(iolist_entry*)HexToVal(d2_t_buf);
          if (IsBadReadPtr((void*)il,28)){
            TextOut(di->hDC,x,di->rcItem.top,"BAD!",4);x+=100;
          }else{
            x+=iolist_box_draw(di->hDC,x,di->rcItem.top,di->rcItem.right - x,
              (di->rcItem.bottom - di->rcItem.top)-1,il,NULL);
          }
          c1+=(7+8);
        }
      }else{
        if (mon_column==Col || break_column==Col){
          COLORREF text_col=GetSysColor(COLOR_WINDOWTEXT);
          WIDTHHEIGHT wh=GetTextSize((HFONT)GetCurrentObject(di->hDC,OBJ_FONT),Text);
          int x=di->rcItem.left;
          int y=di->rcItem.top + (di->rcItem.bottom-di->rcItem.top)/2 - DEBUG_ICONS_H/2;
          for (size_t i=0;i<strlen(Text);i++){
            if (x>=di->rcItem.right) break;
            if (Text[i]=='.'){
              SetPixel(di->hDC,x,di->rcItem.bottom-2,text_col);
              x+=2;
            }else{
              int ico=toupper(Text[i])-'A';
              if (ico<0 || ico>=DEBUG_NUM_ICONS) ico=0;
              int w=DEBUG_ICONS_W;
              if (x+w>=di->rcItem.right) w-=x+w-di->rcItem.right;
              BitBlt(di->hDC,x,y,w,DEBUG_ICONS_H,icons_dc,ico*DEBUG_ICONS_W,0,SRCCOPY);
              if (islower(Text[i])){
                BitBlt(di->hDC,x,y,w,DEBUG_ICONS_H,icons_dc,8*DEBUG_ICONS_W,0,MERGEPAINT);
              }
              x+=DEBUG_ICONS_W+1;
            }
          }
        }else if (Col==hex_column){
          if (hex_map.NumStrings){
            RECT rc=di->rcItem;
            char *map=hex_map[di->itemID].String;
            char byte_text[3]={0,0,0},*t=Text;
            int next_l=rc.left+how_big_is_0000,last_l=rc.left;
            HFONT Font=(HFONT)GetCurrentObject(di->hDC,OBJ_FONT);

            COLORREF text_col=GetTextColor(di->hDC);
            int save_bk_mode=SetBkMode(di->hDC,TRANSPARENT);
            while (map[0]){
              while (t[0]==' '){
                rc.left=next_l;
                next_l=rc.left+how_big_is_0000;
                t++;
              }

              for (int i=0;i<2;i++) byte_text[i]=*(t++);

              COLORREF bk_col=0;
              if (map[0]=='1') bk_col=RGB(255,0,0);
              if (map[0]=='2') bk_col=RGB(0,0,255);
              if (map[0]=='3') bk_col=RGB(224,0,224);
              WIDTHHEIGHT wh=GetTextSize(Font,byte_text);
              if (rc.left+wh.Width+6>=rc.right){
                rc.left=last_l+1;
                for (int i=0;i<3;i++){
                  SetPixel(di->hDC,rc.left,rc.top+(rc.bottom-rc.top)/2+wh.Height/2-3,text_col);
                  rc.left+=3;
                }
                break;
              }
              if (bk_col){
                RECT bk_rc={rc.left,rc.top,rc.left+wh.Width,rc.bottom};
                HBRUSH br=CreateSolidBrush(bk_col);
                FillRect(di->hDC,&bk_rc,br);
                DeleteObject(br);
                SetTextColor(di->hDC,RGB(255,255,255));
              }
              DrawText(di->hDC,byte_text,2,&rc,DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
              if (bk_col) SetTextColor(di->hDC,text_col);
              rc.left+=wh.Width;
              last_l=rc.left;
              map++;
            }
            SetBkMode(di->hDC,save_bk_mode);
          }else{
            DrawText(di->hDC,Text,strlen(Text),&(di->rcItem),DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
          }
        }else{
          DrawText(di->hDC,Text,strlen(Text),&(di->rcItem),DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
        }
      }
      if (old_font) SelectObject(di->hDC,old_font);

      Col++;
      if (Col>columns) break;

      di->rcItem.left+=Wid-2;
      Wid=SendMessage(di->hwndItem,LVM_GETCOLUMNWIDTH,Col,0);
      di->rcItem.right=di->rcItem.left+Wid-1;
    }
  }
  SetBkMode(di->hDC,oldbkcol); //Color
  SetTextColor(di->hDC,oldtextcol);
}

void mem_browser::update()
{
  int x3,x1;
  char *c;
  iolist_entry* iol[30];
  MEM_ADDRESS ldpc2;
  MEM_ADDRESS save_dpc=dpc;

  char txt[12][1024];
  char *txt_p[12];
  char tbuffy[8];
  for (int i=0;i<12;i++){
    txt_p[i]=txt[i];
    txt[i][0]=0;
  }
  hex_map.Sort=eslNoSort;
  hex_map.DeleteAll();

  bool reg=(disp_type==DT_REGISTERS);

  if (handle){
    SendMessage(handle,WM_SETREDRAW,0,0);

    if (mode!=MB_MODE_FIXED) lb_height=SendMessage(handle,LVM_GETCOUNTPERPAGE,0,0);

//    if(mode==MB_MODE_PC)ad=pc; //**** CHANGED!!
    else if(mode==MB_MODE_IOLIST)ad=(ad&0xffffff)|IOLIST_PSEUDO_AD;

    // Delete all but 1 item (saves the horz scrollbar position)
    int nOldItems=SendMessage(handle,LVM_GETITEMCOUNT,0,0);
    for (int n=0;n<nOldItems-1;n++) SendMessage(handle,LVM_DELETEITEM,0,0);

    if (editbox) editbox->update();
    //      SetWindowText(editbox->handle,HEXSl(ad,6));
    dpc=ad;

    if (disp_type==DT_MEMORY || disp_type==DT_REGISTERS){
      if (reg){
        wpl=2;
      }else if(mode==MB_MODE_STANDARD){
        wpl=calculate_wpl();
      }else if(mode==MB_MODE_IOLIST){
        wpl=1;
      }
      int min_wpl=min(wpl,8);
      if (!reg) dpc=ad;

      for (int i=0;i<lb_height;i++){
        for (int n=0;n<12;n++) txt[n][0]=0;
        for (int n=0;n<30;n++) iol[n]=NULL;
        if (reg){
          ldpc2=ad+i;
        }else{
          MEM_ADDRESS ad_high=ad & 0xff000000;
          ldpc2=ad+i*2*wpl;
          ldpc2&=0xfffffe;
          ldpc2|=ad_high;
        }
        Str line_hex_map;
        // Add to list view
        if (reg){
          if (reg_browser_entry_name[ldpc2][0]==0) break;
          strcpy(txt[0],reg_browser_entry_name[ldpc2]);
          x3=2;x1=1;
        }else if (ldpc2>=MEM_IO_BASE || ldpc2<MEM_START_OF_USER_AREA){
          if ((ldpc2 & 0xff000000) == IOLIST_PSEUDO_AD){ //pseudo
            strcpy(txt[0],"-");
            strcpy(txt[1],"-");
            strcpy(txt[2],"-");
            if ((ldpc2 & 0xfffff000)==IOLIST_PSEUDO_AD_PSG && ldpc2<=(IOLIST_PSEUDO_AD_PSG + 30)){
              strcpy(txt[3],"#");
              strcat(txt[3],STRS((ldpc2 & 0xff)/2));
            }else{
              txt[3][0]=0;
            }
            strcpy(txt[5],"___");
            iol[0]=search_iolist(ldpc2);
            if (iol[0]) strcpy(txt[5],iol[0]->name);
            iol[1]=iol[0];
          }else{
            get_breakpoint_labels(ldpc2,min_wpl*2,txt_p); // Reg, Bk, Mon

            strcpy(txt[3],HEXSl(ldpc2,6)); // Address

            // Disassembly (description of address)
            txt[5][0]=0;
            bool skip=false;
            for (int n=0;n<min_wpl*2;n++){
              iol[n]=search_iolist(ldpc2+n);
              if (skip){
                skip=false;
              }else{
                if (n) strcat(txt[5],", ");
                if (iol[n]){
                  strcat(txt[5],iol[n]->name);
                  skip=iol[n]->bytes==2;
                }else{
                  strcat(txt[5],"__");
                }
              }
            }
          }
          x3=6;x1=4;
        }else{ //disassemble
          get_breakpoint_labels(ldpc2,min_wpl*2,txt_p);
          strcpy(txt[3],HEXSl(ldpc2,6));

          if (dpc>ldpc2) strcat(txt[5],"... ");
          Str dis;
          bool first=true;
          while (dpc<ldpc2+wpl*2){
            bool add_this=(dpc<ldpc2+min_wpl*2);
            dis=debug_parse_disa_for_display(disa_d2(dpc));
            if (add_this){
              if (first==0) strcat(txt[5],"     ");
              first=0;
              strcat(txt[5],dis);
            }else{
              strcat(txt[5],".");
            }
          }
          x3=6;x1=4;
        }
        short buf;unsigned short mask;

        //decimal
        if ((ldpc2&0xff000000) != IOLIST_PSEUDO_AD){
          for(int m=0;m<(min_wpl/2);m++){
            signed long buf2;
            d2_peekvalid=0;
            if(reg)buf2=*(reg_browser_entry_pointer[ldpc2]);
            else buf2=d2_lpeek(ldpc2+m*4);
            if(d2_peekvalid){
             if(m)strcat(txt[x3+1],", ");
             strcat(txt[x3+1],"ттт.l");
            }else{
              if(m)strcat(txt[x3+1],", ");
              strcat(txt[x3+1],STRS(buf2));
              strcat(txt[x3+1],".l");
            }
          }
          if(strlen(txt[x3+1]))  strcat(txt[x3+1],"  /  ");
          for(int m=0;m<min_wpl;m++){
            d2_peekvalid=0;
            if(reg) buf=*((WORD*)(reg_browser_entry_pointer[ldpc2])+1-m);
            else buf=d2_dpeek(ldpc2+m*2);
            if(d2_peekvalid){
              strcat(txt[x3+1]," ттт.w");
            }else{
              if(m)strcat(txt[x3+1],", ");
              strcat(txt[x3+1],STRS(buf));
              strcat(txt[x3+1],".w");
            }
          }
          strcat(txt[x3+1],"  /  ");
          for(int m=0;m<min_wpl;m++){
            d2_peekvalid=0;
            if(reg) buf=*((WORD*)(reg_browser_entry_pointer[ldpc2])+1-m);
            else buf=d2_dpeek(ldpc2+m*2);
            if(m)strcat(txt[x3+1],", ");
            if(d2_peekvalid&0xff00){
              strcat(txt[x3+1],"ттт");
            }else{
              strcat(txt[x3+1],STRS((unsigned char)HIBYTE(buf)));
            }
            strcat(txt[x3+1],".b, ");
            if(d2_peekvalid&0xff){
              strcat(txt[x3+1],"ттт");
            }else{
              strcat(txt[x3+1],STRS((unsigned char)LOBYTE(buf)));
            }
            strcat(txt[x3+1],".b");
          }
        }else{ //iolist
          if (iol[0]){
            if (iol[0]->ptr){
              strcpy(txt[x3+1],"$");
              strcat(txt[x3+1],HEXSl(*((BYTE*)(iol[0]->ptr)),2));
              strcat(txt[x3+1]," (");
              strcat(txt[x3+1],STRS( *((BYTE*)(iol[0]->ptr)) ));
              strcat(txt[x3+1],")");
            }
          }else{
            txt[x3+1][0]=0;
          }
        }

        // x1=hex column, x3=text column, x3+2=binary
        for (int m=0;m<wpl;m++){
          d2_peekvalid=0;
          if (mode==MB_MODE_IOLIST){
            if (iol[0]){
              buf=0;
              if (iol[0]->ptr) buf=*((BYTE*)(iol[0]->ptr));
              strcpy(tbuffy,HEXSl(buf,2));
              txt[x3][0]=buf;txt[x3][1]=0;
            }else{
              buf=0;
              tbuffy[0]=0;
              txt[x3][0]=0;
            }
          }else{
            if (reg) buf=*((WORD*)(reg_browser_entry_pointer[ldpc2])+1-m);
            else buf=d2_dpeek(ldpc2+m*2);

            strcpy(tbuffy,HEXSl(buf,4));
            c=tbuffy;
            for(mask=0x7f00;mask;mask>>=8){
              if(d2_peekvalid&mask){ //bus error
                *c++='x';*c++='x';  //т
              }else{
                c+=2;
              }
            }
            if (d2_peekvalid & 0xff00){
              txt[x3][m*2]='Т';
            }else if (HIBYTE(buf)){
              txt[x3][m*2]=HIBYTE(buf);
            }else{
              txt[x3][m*2]='.';
            }
            if (d2_peekvalid & 0xff){
              txt[x3][m*2+1]='Т';
            }else if (LOBYTE(buf)){
              txt[x3][m*2+1]=LOBYTE(buf);
            }else{
              txt[x3][m*2+1]='.';
            }
            txt[x3][m*2+2]=0;
          }
          if (m<min_wpl){
            if (m){
              strcat(txt[x1]," ");
              strcat(txt[x3+2]," ");
            }
            strcat(txt[x1],tbuffy);
            line_hex_map+=get_hex_map(((ldpc2+m*2) & 0xffffff));

            if (strlen(txt[x3+2])<200-20){
              mask=0x8000;
              if (IS_IOLIST_PSEUDO_ADDRESS(ad)) mask=0x80;
              if (m) strcat(txt[x3+2]," ");
              iolist_entry *last_iol=NULL;
              for(int b=0;b<2;b++){
                if(iol[m*2+b]){  //special display
                  if(iol[m*2+b]!=last_iol){
                    last_iol=iol[m*2+b];
                    if(iol[m*2+b]->bitmask==""){
                      for(int mm=0;mm<8;mm++){
                        strcat(txt[x3+2],LPSTR((buf & mask) ? "1":"0"));
                        mask>>=1;
                      }
                    }else{
                      strcat(txt[x3+2],"$$IOL$$");
                      strcat(txt[x3+2],HEXSl(DWORD(iol[m*2+b]),8)); //store long pointer in hex
                    }
                  }
                }else if(mode==MB_MODE_IOLIST){
                  txt[x3+2][0]=0;
                }else{
                  if(d2_peekvalid&mask){
                    strcat(txt[x3+2],"ттт");
                    mask>>=8;
                  }else{
                    for(int mm=0;mm<8;mm++){
                      strcat(txt[x3+2],LPSTR((buf & mask) ? "1":"0"));
                      mask>>=1;
                    }
                  }
                }
                if (b==0) strcat(txt[x3+2]," ");
              }
            }
          }
        }
        if (mode!=MB_MODE_IOLIST){
          // Pad to make sure text same length as that displayed
          int n=(strlen(txt[x1])+1)/10;
          strcat(txt[x1],"\01");
          for (int j=0;j<=n;j++) strcat(txt[x1],"0");
          hex_map.Add(line_hex_map);
        }
        txt[x3+3][0]=0;
        listbox_add_line(handle,i,txt_p,x3+3);
      }
    }else if (disp_type==DT_INSTRUCTION){
      wpl=1;
      for (int i=0;i<lb_height;i++){
        int n;
        for (n=0;n<12;n++) txt[n][0]=0;
  //      ldpc2=ad+i*2*wpl;
        //add to list view
        ldpc2=dpc;
        strcpy(txt[3],HEXSl(dpc,6));
        strcpy(txt[5],debug_parse_disa_for_display(disa_d2(dpc)).Text);
        get_breakpoint_labels(ldpc2,(int)dpc-(int)ldpc2,txt_p);

        Str line_hex_map;
        for (int m=0;m<(int)dpc-(int)ldpc2;m+=2){
          if (m) strcat(txt[4]," ");
          strcat(txt[4],HEXSl(d2_dpeek(ldpc2+m),4));
          line_hex_map+=get_hex_map((ldpc2+m) & 0xffffff);
        }
        // Pad to make sure text same length as that displayed
        n=(strlen(txt[4])+1)/10;
        strcat(txt[4],"\01");
        for (int j=0;j<=n;j++) strcat(txt[4],"0");
        hex_map.Add(line_hex_map);
        listbox_add_line(handle,i,txt_p,columns);
      }
    }
    dpc=save_dpc;

    if (nOldItems) SendMessage(handle,LVM_DELETEITEM,SendMessage(handle,LVM_GETITEMCOUNT,0,0)-1,0);

    SendMessage(handle,WM_SETREDRAW,1,0);
  }
}
//--------------------------------------------------------------------------
LRESULT __stdcall mem_browser_window_WndProc(HWND Win,UINT Mess,UINT wPar,long lPar)
{
  mem_browser *mb;
	switch (Mess){
    case WM_COMMAND:
    {
      switch (HIWORD(wPar)){
        case CBN_SELENDOK:
          if (LOWORD(wPar)==2){
            mb=(mem_browser*)GetWindowLong(Win,GWL_USERDATA);
            int l=SendMessage((HWND)lPar,CB_GETCURSEL,0,0);
            if(l==DT_INSTRUCTION){
              mb->disp_type=DT_INSTRUCTION;
            }else if(l==DT_MEMORY){
              mb->disp_type=DT_MEMORY;
            }
            mb->init();
            mb->update();
          }
          break;
        case BN_CLICKED:
          mb=(mem_browser*)GetWindowLong(Win,GWL_USERDATA);
          if (LOWORD(wPar)==4){
            Str sz_str;
            sz_str.SetLength(200);
            SendMessage(GetDlgItem(Win,5),WM_GETTEXT,200,LPARAM(sz_str.Text));
            strupr(sz_str);
            int coeff=1;
            char *postfix=strstr(sz_str,"MB");
            if (postfix) coeff=1024*1024,*postfix=0;
            postfix=strstr(sz_str,"KB");
            if (postfix) coeff=1024,*postfix=0;

            int bytes;
            bool hex=0;
            char *sz=sz_str;
            if (sz[0]=='0' && sz[1]=='x') sz+=2, hex=true;
            if (sz[0]=='$') sz++, hex=true;
            if (hex){
              bytes=HexToVal(sz);
            }else{
              bytes=atoi(sz);
            }
            bytes*=coeff;
            if (bytes<=0){
              MessageBeep(NULL);
              break;
            }

            EasyStr fn;
            if (mb->disp_type==DT_MEMORY){
              fn=FileSelect(Win,"Save Memory Block As...",WriteDir,"Memory Dump Files\0*.dmp\0All Files\0*.*\0\0",1,false,"dmp");
            }else if (mb->disp_type==DT_INSTRUCTION){
              fn=FileSelect(Win,"Save Disassembly As...",WriteDir,"Dissasembly Files\0*.s;*.dis\0All Files\0*.*\0\0",1,false,"s");
            }else{
              break;
            }
            if (fn.NotEmpty()){
              FILE *f=fopen(fn,"wb");
              if (f){
                if (mb->disp_type==DT_MEMORY){
                  STfile_write_from_ST_memory(f,mb->ad,bytes);
                }else{
                  bool as_s=IsSameStr_I(fn.Rights(2),".S");
                  disa_to_file(f,mb->ad,bytes,as_s);
                }
                fclose(f);
              }
            }
          }else if (LOWORD(wPar)==6){
            debug_load_file_to_address(Win,mb->ad & 0xffffff);
          }else if (LOWORD(wPar)==9 || LOWORD(wPar)==10){
            int dir=int((LOWORD(wPar)==9) ? -1:1);
            EasyStr Text;
            Text.SetLength(200);
            SendDlgItemMessage(Win,8,WM_GETTEXT,200,LPARAM(Text.Text));

            DynamicArray<BYTE> BytesToFind;
            bool WordOnly=0;
            acc_parse_search_string(Text,BytesToFind,WordOnly);
            if (BytesToFind.NumItems==0){
              MessageBeep(0);
              return 0;
            }

            MEM_ADDRESS ad=mb->ad & 0xffffff;
            if (ad>=rom_addr+tos_len && dir<0) ad=rom_addr+tos_len-BytesToFind.NumItems;
            if (mb->disp_type==DT_INSTRUCTION){
              ad=oi(ad & ~1,int(dir<0 ? -1:1));  //offset instruction
            }else{
              BYTE ToFind=BytesToFind[0];
//              try{
              TRY_M68K_EXCEPTION
                if (m68k_peek(ad)==ToFind || m68k_peek(ad+1)==ToFind) ad+=dir*2;
//              }catch(...){}
              CATCH_M68K_EXCEPTION
              END_M68K_EXCEPTION
            }
            ad+=dir*2;

            MEM_ADDRESS found_at=acc_find_bytes(BytesToFind,WordOnly,ad,dir);
            if (found_at<=0xffffff){
              if (mb->disp_type==DT_INSTRUCTION){
                mb->ad=oi((found_at & ~1)+2,-1) | (mb->ad & 0xff000000);  //offset instruction
              }else{
                mb->ad=(found_at & ~1) | (mb->ad & 0xff000000);
              }
              mb->update();
//              SetFocus(mb->handle);
            }else{
              Alert("It's not there mate","Find Failed",0);
            }
          }
          break;
      }
      break;
    }
    case WM_DRAWITEM:
    {
      if(wPar==1){ //mb
        mb=(mem_browser*)GetWindowLong(Win,GWL_USERDATA);
        mb->draw((DRAWITEMSTRUCT*)lPar);
        return 1;
      }else{
        break;
      }
    }
    case WM_SIZE:
      mb=(mem_browser*)GetWindowLong(Win,GWL_USERDATA);
      if(mb){
        if(IsWindow(mb->handle)){
          int y=30;
          if (mb->disp_type==DT_REGISTERS || IS_IOLIST_PSEUDO_ADDRESS(mb->ad)) y=2;
          MoveWindow(mb->handle,10,y,LOWORD(lPar)-20,HIWORD(lPar)-5-y,true);
          mb->update();
        }
      }
      break;
    case WM_DESTROY:
    {
      if (GetParent(DWin_edit)==Win){
        ShowWindow(DWin_edit,SW_HIDE);
        SetParent(DWin_edit,DWin);
      }

      mb=(mem_browser*)GetWindowLong(Win,GWL_USERDATA);
      if (mb){
        if (mb->disp_type!=DT_REGISTERS && IS_IOLIST_PSEUDO_ADDRESS(mb->ad)==0){
          TOOLINFO ti;
          ti.cbSize=sizeof(TOOLINFO);
          ti.hwnd=Win;
          ti.uId=(UINT)GetDlgItem(Win,8);
          SendMessage(ToolTip,TTM_DELTOOL,0,(LPARAM)&ti);
        }

        delete mb;
        mr_static_delete_children_of(Win);
      }
      break;
    }
    case WM_SETFOCUS:
      SetFocus(GetDlgItem(Win,1));
      break;
  }

	return DefWindowProc(Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
void mem_browser::vscroll(int of)
{
  if (of){
    MEM_ADDRESS ad_high=ad & 0xff000000;
    if (disp_type==DT_INSTRUCTION){
      ad=oi(ad & 0xffffff,of);  //offset instruction
    }else if (disp_type==DT_MEMORY && IS_IOLIST_PSEUDO_ADDRESS(ad)==0){
      ad+=2*(wpl)*of;
      ad&=0xfffffe;
    }else if(disp_type==DT_REGISTERS){
      ad+=of;
      ad=max(0,min(17,(int)(ad)));
    }
    ad|=ad_high;
    update();
  }
}
//---------------------------------------------------------------------------
mem_browser* mem_browser_get_pointer(HWND Win)
{
  if (Win==m_b_mem_disa.handle){
    return &m_b_mem_disa;
  }else if (Win==m_b_trace.handle){
    return &m_b_trace;
  }else if (Win==m_b_stack.handle){
    return &m_b_stack;
  }
  return (mem_browser*)GetWindowLong(GetParent(Win),GWL_USERDATA);
}
//---------------------------------------------------------------------------
void mem_browser::setup_contextmenu(int row,int col)
{
  insp_menu_subject_type=1; //0=mem_browser
  insp_menu_subject=(void*)this;
  insp_menu_row=row;
  insp_menu_col=col;

  DeleteAllMenuItems(insp_menu);
  MEM_ADDRESS row_ad=get_address_from_row(row) & 0xffffff;

  if (col<=mon_column){
    insp_menu_long[0]=row_ad;

    int wpl=1;
    if (disp_type==DT_MEMORY && mode==MB_MODE_STANDARD) wpl=min(calculate_wpl(),5);
    if (disp_type==DT_INSTRUCTION) wpl=max(int(oi(row_ad,1)-row_ad)/2,1);

    for (int w=0;w<wpl;w++){
      DEBUG_ADDRESS *pda=debug_find_address(row_ad);
      int id_base=3050+w*20;

      if (pda) if (pda->name[0]) AppendMenu(insp_menu,MF_BYCOMMAND | MF_DISABLED | MF_GRAYED,0,pda->name);
      AppendMenu(insp_menu,MF_BYCOMMAND,id_base+1,Str("Name address at $")+HEXSl(row_ad,6));
      bool is_bk=0;
      WORD mask[2]={0,0};
      if (pda){
        if (pda->bwr & BIT_0) is_bk=true;
        if (pda->bwr & BIT_1) mask[0]=pda->mask[0];
        if (pda->bwr & BIT_2) mask[1]=pda->mask[1];
      }
      if (is_bk){
        AppendMenu(insp_menu,MF_BYCOMMAND,id_base,Str("Clear breakpoint at $")+HEXSl(row_ad,6));
      }else{
        AppendMenu(insp_menu,MF_BYCOMMAND,id_base,Str("Set breakpoint at $")+HEXSl(row_ad,6));
      }

      if (row_ad<0xe00000 || row_ad>=MEM_IO_BASE){ // Monitors only on RAM and IO
        HMENU MonPop[2]={CreatePopupMenu(),CreatePopupMenu()};
        AppendMenu(insp_menu,MF_BYCOMMAND | MF_POPUP | (mask[0] ? MF_CHECKED:0),(UINT)MonPop[0],"Monitor writes to");
        AppendMenu(insp_menu,MF_BYCOMMAND | MF_POPUP | (mask[1] ? MF_CHECKED:0),(UINT)MonPop[1],"Monitor reads of");

        for (int n=0;n<2;n++){
          int mask_base=id_base+2+n*4;
          AppendMenu(MonPop[n],MF_BYCOMMAND,mask_base,"None");
          AppendMenu(MonPop[n],MF_BYCOMMAND,mask_base+1,Str("$")+HEXSl(row_ad,6)+".w");
          AppendMenu(MonPop[n],MF_BYCOMMAND,mask_base+2,Str("$")+HEXSl(row_ad,6)+".b");
          AppendMenu(MonPop[n],MF_BYCOMMAND,mask_base+3,Str("$")+HEXSl(row_ad+1,6)+".b");
          int check=mask_base;
          if (mask[n]==0xffff) check=mask_base+1;
          if (mask[n]==0xff00) check=mask_base+2;
          if (mask[n]==0x00ff) check=mask_base+3;
          CheckMenuRadioItem(MonPop[n],mask_base,mask_base+3,check,MF_BYCOMMAND);
        }
        if (pda){
          HMENU ActivatePop=CreatePopupMenu();
          AppendMenu(ActivatePop,MF_STRING,id_base+16,"Do nothing");
          AppendMenu(ActivatePop,MF_STRING,id_base+17,"Do global");
          AppendMenu(ActivatePop,MF_STRING,id_base+18,"Break");
          AppendMenu(ActivatePop,MF_STRING,id_base+19,"Log");
          AppendMenu(insp_menu,MF_BYCOMMAND | MF_POPUP,(UINT)ActivatePop,"On activation");
          CheckMenuRadioItem(ActivatePop,id_base+16,id_base+19,id_base+16+pda->mode,MF_BYCOMMAND);
        }
      }

      if (w+1>=wpl) break;
      AppendMenu(insp_menu,MF_SEPARATOR,0,NULL);
      row_ad+=2;
    }
  }else{
    if (disp_type!=DT_REGISTERS){
      strcpy(insp_menu_long_name[0],"address ");
      strcat(insp_menu_long_name[0],HEXSl(row_ad,6));
      insp_menu_long_bytes[0]=3;
      insp_menu_long[0]=row_ad;

      insp_menu_long[1]=d2_lpeek(row_ad) & 0xffffff;
      insp_menu_long_bytes[1]=4;
      strcpy(insp_menu_long_name[1],"(");strcat(insp_menu_long_name[1],HEXSl(row_ad,6));
      strcat(insp_menu_long_name[1],").L = ");
      strcat(insp_menu_long_name[1],HEXSl(insp_menu_long[1],6));

      insp_menu_long[2]=(long)d2_dpeek(row_ad);
      insp_menu_long_bytes[2]=4;
      strcpy(insp_menu_long_name[2],"(");strcat(insp_menu_long_name[2],HEXSl(row_ad,6));
      strcat(insp_menu_long_name[2],").W = ");
      strcat(insp_menu_long_name[2],HEXSl(insp_menu_long[2],6));

      insp_menu_col=4; // Edit target

    }else{
      int n=row+(ad);
      strcpy(insp_menu_long_name[0],reg_browser_entry_name[n]);
      strcat(insp_menu_long_name[0],".L = ");
      strcat(insp_menu_long_name[0],HEXS(*(reg_browser_entry_pointer[n])));
      insp_menu_long[0]=*(reg_browser_entry_pointer[n]);
      insp_menu_long_bytes[0]=4;

      strcpy(insp_menu_long_name[1],reg_browser_entry_name[n]);
      strcat(insp_menu_long_name[1],".W = ");
      strcat(insp_menu_long_name[1],HEXS(*(WORD*)(reg_browser_entry_pointer[n])));
      insp_menu_long[1]=(long)*(WORD*)(reg_browser_entry_pointer[n]);
      insp_menu_long_bytes[1]=2;

      insp_menu_long_bytes[2]=0;
      insp_menu_col=1;
    }
    insp_menu_setup();
    AppendMenu(insp_menu,MF_ENABLED | MF_STRING,3025,"Edit");
  }
  if (disp_type!=DT_REGISTERS){
    AppendMenu(insp_menu,MF_SEPARATOR,0,NULL);
    AppendMenu(insp_menu,MF_SEPARATOR,0,NULL);
    AppendMenu(insp_menu,MF_ENABLED | MF_STRING,3028,Str("Run to $")+HEXSl(row_ad,6));
    if (owner!=DWin) AppendMenu(insp_menu,MF_ENABLED | MF_STRING,3026,"Set browser name");
    if ((MEM_ADDRESS)(insp_menu_long[0])<himem) AppendMenu(insp_menu,MF_ENABLED | MF_STRING,3027,Str("Load file to $")+HEXSl(insp_menu_long[0],6));
  }
}
//---------------------------------------------------------------------------
LRESULT __stdcall mem_browser_WndProc(HWND Win,UINT Mess,UINT wPar,long lPar)
{
  mem_browser *mb;
	switch (Mess){
    case WM_LBUTTONUP:   
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
    case WM_MBUTTONUP:   case WM_MBUTTONDOWN:   case WM_MBUTTONDBLCLK:
      return 0;
    case WM_RBUTTONUP /*WM_CONTEXTMENU*/:case WM_LBUTTONDOWN:case WM_LBUTTONDBLCLK:{
      mb=mem_browser_get_pointer(Win);
      if (mb->handle!=NULL){

        int col,row;
        POINT m;RECT rc;
        m.x=LOWORD(lPar)+GetScrollPos(Win,SB_HORZ),m.y=HIWORD(lPar);
/*
        if (Mess==WM_CONTEXTMENU){
          ScreenToClient(Win,&m);
        }
*/
        int cx=SendMessage(Win,LVM_GETCOLUMNWIDTH,0,0),cx1=0;
        for(col=0;m.x>=cx && col<mb->columns;col++){
          cx1=cx;  //cx1 is the left x-pos of the clicked column
          cx+=SendMessage(Win,LVM_GETCOLUMNWIDTH,col+1,0);
        }
        rc.left=LVIR_BOUNDS;
        SendMessage(Win,LVM_GETITEMRECT,0,(LPARAM)&rc);
        int item_height=(rc.bottom-rc.top);
        row=(m.y-rc.top)/item_height;

        if (row>=0 && row<(mb->lb_height)){
          if ((col==1 || col==2) && (mb->disp_type)!=DT_REGISTERS){
            if (Mess==WM_LBUTTONDOWN || Mess==WM_LBUTTONDBLCLK){
              MEM_ADDRESS ad=mb->get_address_from_row(row);
              DEBUG_ADDRESS *pda=debug_find_address(ad);
              if (col==1){
                bool bk=0;
                if (pda) bk=(pda->bwr & BIT_0);
                debug_set_bk(ad,!bk);
              }else{
                if (pda){
                  int cur=pda->bwr & (BIT_1 | BIT_2);
                  if (cur==BIT_1 && pda->mask[0]==0xffff){
                    debug_set_mon(ad,0,0);
                    debug_set_mon(ad,true,0xffff);
                  }else if (cur==BIT_2 && pda->mask[1]==0xffff){
                    debug_set_mon(ad,true,0);
                  }else if (cur==0){
                    debug_set_mon(ad,0,0xffff);
                  }else{
                    Mess=WM_RBUTTONUP;
                  }
                }else{
                  debug_set_mon(ad,0,0xffff);
                }
              }
              if (Mess!=WM_RBUTTONUP) return 0;
            }
          }else if (bool((mb->disp_type==DT_REGISTERS) ? (col==1):(col==4)) ){ //hex
            if (Mess==WM_LBUTTONDOWN){
              if (mb->editflag){
                set_DWin_edit(1,(void*)mb,row,col);
                return 0;
              }
            }
          }else{
            char tb[200];
      	    LV_ITEM lvi;
      	    lvi.pszText=tb;
      	    lvi.cchTextMax=199;
      	    lvi.iItem=row;
//      	    int Wid=SendMessage(di->hwndItem,LVM_GETCOLUMNWIDTH,0,0),Col=0;
//      	    di->rcItem.right=di->rcItem.left+Wid;

            lvi.iSubItem=col;
            tb[0]=0;
            SendMessage(mb->handle,LVM_GETITEMTEXT,row,(LPARAM)&lvi);

            if(strstr(tb,"$$IOL$$")){ //special display
              
              
              //this section is adapted from the mem_browser::draw() method above.

              int x=cx1+6,ox; //start imaginary drawing from left of cell.
              char*c1=tb;
              char*c2;
              LOOP{
                c2=strstr(c1,"$$IOL$$");
                if(c2==NULL){
                  c2=c1+strlen(c1);
                }
                if(c2>c1){ //text to display
                  *(c2-1)=0; //first part of string
                  x+=SendMessage(mb->handle,LVM_GETSTRINGWIDTH,0,(LPARAM)c1);  //how wide was the text?
                }
                c1=c2;
                if(!*c1)break; //end of string
                if(x>m.x)break; //mouse missed boxes
                //special box

                memcpy(d2_t_buf,c1+7,8); //copy out hex stuff
                d2_t_buf[8]=0;
                iolist_entry*il=(iolist_entry*)HexToVal(d2_t_buf);
                if(IsBadReadPtr((void*)il,28)){
                  break; //give up if access error
                }else{
                  ox=x;
                  x+=iolist_box_width(il);
                  if(x>m.x){ //clicked the box
                    iolist_box_click(m.x-ox,il,NULL);
                    update_register_display(false);
                    break; //end loop
//                    MessageBox(0,EasyStr("Clicked ")+(il->name),"Clicky",0);
                  }
                }
                c1+=(7+8);
              }
            }
          }
          if (Mess==WM_RBUTTONUP /*WM_CONTEXTMENU*/){
            POINT pt;pt.x=LOWORD(lPar);pt.y=HIWORD(lPar);
            ClientToScreen(mb->handle,&pt);
            mb->setup_contextmenu(row,col);
            TrackPopupMenu(insp_menu,TPM_LEFTALIGN | TPM_LEFTBUTTON,pt.x,pt.y,0,DWin,NULL);

            return 0;
          }
        }
        SetFocus(mb->handle);
      }
      return 0;
    }case WM_KEYDOWN:
      mb=mem_browser_get_pointer(Win);
      if (mb->handle!=NULL){
        switch(wPar){
          case VK_PAGEUP:
            mb->vscroll(-(mb->lb_height)+1);
            break;
          case VK_PAGEDOWN:
            mb->vscroll((mb->lb_height)-1);
            break;
          case VK_UP:
            mb->vscroll(-1);
            break;
          case VK_DOWN:
            mb->vscroll(1);
            break;
          case VK_LEFT:case VK_RIGHT:
            if (mb->mode==MB_MODE_STANDARD){
              if (wPar==VK_LEFT) mb->ad-=2;
              else mb->ad+=2;
              mb->update();
            }
        }
      }
      return 0;
    case WM_PAINT:
      mb=mem_browser_get_pointer(Win);
      if (mb){
        if (mb->mode==MB_MODE_STANDARD){
          if (mb->disp_type==DT_MEMORY){
            int new_wpl=mb->calculate_wpl();
            if(mb->wpl!=new_wpl){
              mb->update();
            }
          }
        }
      }
      break;
  }
	return CallWindowProc(Old_mem_browser_WndProc,Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
LRESULT __stdcall mem_browser::FindEditWndProc(HWND Win,UINT Mess,UINT wPar,long lPar)
{
  if (Mess==WM_KEYDOWN){
    if (wPar==VK_RETURN || wPar==VK_DOWN || wPar==VK_PAGEDOWN){
      PostMessage(GetParent(Win),WM_COMMAND,MAKEWPARAM(10,BN_CLICKED),(LPARAM)GetDlgItem(GetParent(Win),10));
      return 0;
    }else if (wPar==VK_UP || wPar==VK_PAGEUP){
      PostMessage(GetParent(Win),WM_COMMAND,MAKEWPARAM(9,BN_CLICKED),(LPARAM)GetDlgItem(GetParent(Win),9));
      return 0;
    }
  }
  return CallWindowProc(OldEditWndProc,Win,Mess,wPar,lPar);
}
//---------------------------------------------------------------------------
void mem_browser::update_icon()
{
  if (owner) SetClassLong(owner,GCL_HICON,long(hGUIIcon[RC_ICO_STCLOSE]));
}
//---------------------------------------------------------------------------
mem_browser::~mem_browser()
{
  for (int n=0;n<MAX_MEMORY_BROWSERS;n++){
    if (m_b[n]==this){

      for (int m=n;m<MAX_MEMORY_BROWSERS-1;m++)
        m_b[m]=m_b[m+1];

      m_b[MAX_MEMORY_BROWSERS-1]=NULL;
      n--;
    }
  }
}
//---------------------------------------------------------------------------

