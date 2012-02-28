/*---------------------------------------------------------------------------
FILE: infobox.cpp
MODULE: Steem
DESCRIPTION: Steem's general information dialog.
---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
void TGeneralInfo::CreatePage(int pg)
{
  switch (pg){
    case INFOPAGE_ABOUT:
      CreateAboutPage();
      break;
    case INFOPAGE_DRAWSPEED:
      CreateSpeedPage();
      break;
    case INFOPAGE_LINKS:
      CreateLinksPage();
      break;
    case INFOPAGE_README:
    case INFOPAGE_HOWTO_DISK:
    case INFOPAGE_HOWTO_CART:
    case INFOPAGE_FAQ:
    case INFOPAGE_UNIXREADME:
      CreateReadmePage(pg);
      break;
  }
}

const char *Credits[40]={
  "Jorge Cwik for the brilliant pasti library and finding lots of Steem bugs",
  "Keili for the fantastic disk image database",
  "Stephen Ware for finding lots of emulation and MIDI bugs",
  "Tomi Kivelä for the move.l timing bug and lots of STE help",
  "Xavier Joubert for finding the illegal instruction bug",
  "Sébastien Molines for the abs mouse button flags and prefetch bugs",
  "Hans Härröd for finding many blitter bugs (fixing Obsession)",
  "Kimmo Hakala for finding many CPU bugs",
  "Christian Gandler for the Pexec clearing heap bug",
  "Sengan Baring-Gould for the Pexec mode 3/4 bug",
  "Simon Owen for the FDC CRC generation code",
  "Zorg for the fantastic MSA Converter",
  "Heinz Rudolf for his help with overscans",
  "Aggression for their many insights into little known STE features",
  "Dima Sobolev for making MIDI possible",
  "Jon Brown for finding the MIDI time code bug",
  "Torsten Keltsch, RGO and anyone else who has run test programs for us",
  "Joseph Newton for his many tests to find the mysterious slowdown bug",
  "Robert Hagenström for his fantastic icons",
  "David E. Gervais for his gold Steem icon",

  "",
  "Libraries:",
#ifdef UNIX
  "The makers of PortAudio the Portable Real-Time Audio Library",
  "PortAudio Copyright (c) 1999-2000 Phil Burk and Ross Bencina",
  "",
  "RtAudio: A set of realtime audio I/O C++ classes",
  "Copyright (c) 2001-2004 Gary P. Scavone",
  "",
  "Jean-Loup Gailly and Mark Adler for the zlib library",
  "Gilles Vollant for the zlib minizip addition",
#endif

#ifdef WIN32
  "Christian Ghisler for his unzip library",
  "Floris van den Berg and everyone else who helped create the FreeImage library",
#endif

  "Christian Scheurer and Johannes Winkelmann for the UniquE RAR File Library",
  "",

  "Technical Info:",
  "Dan Hollis/MicroImages Software for their hardware register list",
  "Damien Burke for describing the MSA disk image format",
  "Bruno Mathieu for his info on the DIM disk image format",
  NULL};

void TGeneralInfo::GetHyperlinkLists(EasyStringList &desc_sl,EasyStringList &link_sl)
{
  desc_sl.Sort=eslNoSort;
  link_sl.Sort=eslNoSort;
  desc_sl.Add(T("Official Steem website"),0);
  link_sl.Add(STEEM_WEB);

  // Steem links
  FILE *f=fopen(RunDir+SLASH "steem.new","rt");
  if (f){
    char buf[5000];
    EasyStr LinkDesc;
    bool InLinks=0;
    while (fgets(buf,5000,f)){
      if (strstr(buf,"[LINKS]")==buf){
        InLinks=true;
      }else if (strstr(buf,"[LINKSEND]")==buf){
        break;
      }else if (InLinks){
        LinkDesc=buf;
        while (LinkDesc.RightChar()=='\r' || LinkDesc.RightChar()=='\n') *(LinkDesc.Right())=0;
        if (LinkDesc.NotEmpty()){
          if (LinkDesc.Lefts(3)=="..."){
            desc_sl.Add(LinkDesc.Text+3,1);
            link_sl.Add("");
          }else{
            if (fgets(buf,5000,f)){
              Str Link=buf;
              while (Link.RightChar()=='\r' || Link.RightChar()=='\n') *(Link.Right())=0;

              desc_sl.Add(LinkDesc,0);
              link_sl.Add(Link);
            }
          }
        }
      }
    }
    fclose(f);
  }
}

#ifdef WIN32

#define INFOBOX_HEIGHT 420
//---------------------------------------------------------------------------
TGeneralInfo::TGeneralInfo()
{
  page_l=160;page_w=500;

  Left=(GetSystemMetrics(SM_CXSCREEN)-(3+page_l+page_w+10+3))/2;
  Top=(GetSystemMetrics(SM_CYSCREEN)-(INFOBOX_HEIGHT+GetSystemMetrics(SM_CYCAPTION)))/2;

  FSLeft=(640-(3+page_l+page_w+10+3))/2;
  FSTop=(480-(INFOBOX_HEIGHT+GetSystemMetrics(SM_CYCAPTION)))/2;

  Section="GeneralInfo";

  BkBrush=CreateSolidBrush(GetSysColor(COLOR_WINDOW));
  il=NULL;
  MaxLinkID=0;

  Page=INFOPAGE_README;
}
//---------------------------------------------------------------------------
void TGeneralInfo::ManageWindowClasses(bool Unreg)
{
  char *ClassName="Steem General Info";
  if (Unreg){
    UnregisterClass(ClassName,Inst);
  }else{
    RegisterMainClass(WndProc,ClassName,RC_ICO_INFO);
  }
}
//---------------------------------------------------------------------------
void TGeneralInfo::LoadIcons()
{
  if (Handle==NULL) return;

  HIMAGELIST old_il=il;

  il=ImageList_Create(18,20,BPPToILC[BytesPerPixel] | ILC_MASK,7,7);
  if (il){
    ImageList_AddPaddedIcons(il,PAD_ALIGN_RIGHT,hGUIIcon[RC_ICO_INFO],hGUIIcon[RC_ICO_INFO_CLOCK],
                              hGUIIcon[RC_ICO_FUJILINK],hGUIIcon[RC_ICO_TEXT],
                              hGUIIcon[RC_ICO_INFO] /*unused*/,hGUIIcon[RC_ICO_DISK_HOWTO],
                              hGUIIcon[RC_ICO_CART_HOWTO],hGUIIcon[RC_ICO_INFO_FAQ],0);
  }
  SendMessage(PageTree,TVM_SETIMAGELIST,TVSIL_NORMAL,(LPARAM)il);
  if (old_il) ImageList_Destroy(old_il);
}
//---------------------------------------------------------------------------
void TGeneralInfo::DestroyCurrentPage()
{
  HWND LinkParent=Scroller.GetControlPage();
  if (LinkParent){
    ToolsDeleteAllChildren(ToolTip,LinkParent);
    MaxLinkID=0;
  }
  TStemDialog::DestroyCurrentPage();
}
//---------------------------------------------------------------------------
void TGeneralInfo::Show()
{
  if (Handle!=NULL){
    ShowWindow(Handle,SW_SHOWNORMAL);
    SetForegroundWindow(Handle);
    return;
  }

  ManageWindowClasses(SD_REGISTER);
  Handle=CreateWindowEx(WS_EX_CONTROLPARENT | WS_EX_APPWINDOW,"Steem General Info",T("General Info"),
                          WS_CAPTION | WS_SYSMENU,Left,Top,0,0,
                          ParentWin,NULL,HInstance,NULL);
  if (HandleIsInvalid()){
    ManageWindowClasses(SD_UNREGISTER);
    return;
  }

  SetWindowLong(Handle,GWL_USERDATA,(long)this);

  MakeParent(HWND(FullScreen ? StemWin:NULL));

  PageTree=CreateWindowEx(512,WC_TREEVIEW,"",WS_CHILD | WS_VISIBLE | WS_TABSTOP |
                        TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP,
                        0,0,page_l-10,INFOBOX_HEIGHT,Handle,(HMENU)60000,Inst,NULL);

  LoadIcons();
  SendMessage(PageTree,TVM_SETIMAGELIST,TVSIL_NORMAL,(LPARAM)il);


  if (Exists(RunDir+"\\readme.txt")) AddPageLabel(T("Readme"),INFOPAGE_README);
  if (Exists(RunDir+"\\faq.txt")) AddPageLabel("FAQ",INFOPAGE_FAQ);
  AddPageLabel(T("About"),INFOPAGE_ABOUT);
#ifdef _DEBUG_BUILD
  if (avg_frame_time && FullScreen==0) AddPageLabel(T("Draw Speed"),INFOPAGE_DRAWSPEED);
#endif
  AddPageLabel(T("Links"),INFOPAGE_LINKS);
  if (Exists(RunDir+"\\disk image howto.txt")) AddPageLabel("Disk Image Howto",INFOPAGE_HOWTO_DISK);
  if (Exists(RunDir+"\\cart image howto.txt")) AddPageLabel("Cartridge Image Howto",INFOPAGE_HOWTO_CART);

  page_l=2+TreeGetMaxItemWidth(PageTree)+5+2+10;
  page_w=min(page_w,620-page_l);
  SetWindowPos(Handle,NULL,0,0,3+page_l+page_w+10+3,GetSystemMetrics(SM_CYCAPTION)+3+INFOBOX_HEIGHT+3,SWP_NOZORDER | SWP_NOMOVE);
  SetWindowPos(PageTree,NULL,0,0,page_l-10,INFOBOX_HEIGHT,SWP_NOZORDER | SWP_NOMOVE);

  Focus=NULL;
  while (TreeSelectItemWithData(PageTree,Page)==NULL) Page=INFOPAGE_ABOUT;

  ShowWindow(Handle,SW_SHOW);
  SetFocus(Focus);

  if (StemWin) PostMessage(StemWin,WM_USER,1234,0);
}
//---------------------------------------------------------------------------
int TGeneralInfo::DrawColumn(int x,int y,int id,char *t1,...)
{
  char **t=&t1;
  while (**t!='*'){
    if (**t=='-'){
      y+=10;
    }else{
      CreateWindow("Static",*t,WS_CHILD | WS_VISIBLE,x,y,min(180,page_l+page_w-x),17,
                          Handle,(HMENU)id,HInstance,NULL);
      y+=28;id++;
    }
    t++;
  }
  return id;
}
//---------------------------------------------------------------------------
EasyStr TGeneralInfo::dp4_disp(int val)
{
  EasyStr ret;
  ret=(val % 10000);
  while (ret.Length()<4){
    ret.Insert("0",0);
  }
  ret.Insert(".",0);
  ret.Insert(EasyStr(val/10000),0);
  return ret;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void TGeneralInfo::CreateAboutPage()
{
  int y=10,h=4;
  HWND Win;

  EasyStr Text=EasyStr("Steem Engine v")+(char*)stem_version_text+" (built " __DATE__" " +"- "__TIME__")\n";
  Text+="Written by Anthony && Russell Hayward\n";
  Text+="Copyright 2000-2004\n";
  if (TranslateBuf){
    Text+="\n";
    Text+=T("Translation by [Your Name]");
    h+=2;
  }
  int TextHeight=GetTextSize(Font,"HyITljq").Height;
  h*=TextHeight;
  CreateWindowEx(0,"Static",Text,WS_CHILD | WS_VISIBLE,
                      page_l,y,page_w,h,Handle,(HMENU)200,HInstance,NULL);
  y+=h;

  Text=T("Thanks To");
  CreateWindowEx(0,"Steem HyperLink",Text,WS_CHILD | WS_VISIBLE | HL_STATIC,
                          page_l,y,page_w,TextHeight,Handle,(HMENU)204,HInstance,NULL);
  y+=TextHeight;

  Scroller.CreateEx(512,WS_CHILD | WS_VSCROLL | WS_HSCROLL,page_l,y,page_w,INFOBOX_HEIGHT-y-(TextHeight+10+10),
                      Handle,203,HInstance);
  Scroller.SetBkColour(GetSysColor(COLOR_WINDOW));

  y=5;
  for (int n=0;;n++){
    if (Credits[n]==NULL) break;

    Win=CreateWindowEx(0,"Steem HyperLink",Credits[n],WS_CHILD | WS_VISIBLE | HL_STATIC | HL_WINDOWBK,
                       5,y,500,30,Scroller,HMENU(100+n),HInstance,NULL);
    SendMessage(Win,WM_SETFONT,WPARAM(Font),0);
    y+=TextHeight+2;
  }
  Scroller.AutoSize(2,2);
  ShowWindow(Scroller,SW_SHOW);

  y=INFOBOX_HEIGHT-TextHeight-10;
  CreateWindowEx(0,"Steem HyperLink",STEEM_WEB,
                      WS_CHILD | WS_VISIBLE,page_l,y,page_w,TextHeight,
                      Handle,(HMENU)201,HInstance,NULL);

  if (Focus==NULL) Focus=PageTree;
  SetPageControlsFont();
  ShowPageControls();
}
//---------------------------------------------------------------------------
void TGeneralInfo::CreateSpeedPage()
{
  if (avg_frame_time){
    DWORD draw_time,unlock_time,blit_time,tt; //cpu_time;
    DWORD inst_per_second;

    CreateWindowEx(0,"STATIC",T("Timings per VBL (screen refresh)"),
                        WS_CHILD | WS_VISIBLE,page_l,45,250,26,
                        Handle,(HMENU)300,HInstance,NULL);
    int id=DrawColumn(page_l,80,301,CStrT("Drawing time:"),CStrT("Unlocking time:"),CStrT("Blitting time:"),
                        CStrT("Total draw time:"),"-",
//                        CStrT("CPU time:"),
                        CStrT("Instructions per second:"),"-",
                        CStrT("Total frame time:"),CStrT("% ST VBL rate"),"*");

    draw_end();

    tt=timeGetTime();
    for (int trial=0;trial<10;trial++){
      draw_begin();
      draw_end();
    }
    unlock_time=timeGetTime()-tt;

    tt=timeGetTime();
    for (int trial=0;trial<12;trial++){
      draw(false);
    }
    draw_time=timeGetTime()-unlock_time-tt;

    tt=timeGetTime();
    for (int trial=0;trial<12;trial++){
      draw_blit();
    }
    blit_time=timeGetTime()-tt;

    unlock_time/=max(frameskip,1);
    draw_time/=max(frameskip,1);
    blit_time/=max(frameskip,1);

//    cpu_time=avg_frame_time-(draw_time+unlock_time+blit_time);

    inst_per_second=(((n_cpu_cycles_per_second/4)/shifter_freq)*12000)/avg_frame_time;

    EasyStr Secs=EasyStr(" ")+T("seconds");
    DrawColumn(page_l+page_w/2,80,id,(dp4_disp(draw_time*10/12)+Secs).Text,(dp4_disp(unlock_time*10/12)+Secs).Text,
              (dp4_disp(blit_time*10/12)+Secs).Text,(dp4_disp((draw_time+unlock_time+blit_time)*10/12)+Secs).Text,
              "-",
//              (dp4_disp(cpu_time*10/12)+Secs).Text,
              EasyStr(inst_per_second).Text,"-",
              (dp4_disp(avg_frame_time*10/12)+Secs).Text,
              (dp4_disp((((10000*12000)/avg_frame_time)*100)/shifter_freq)).Text,"*");

    if (Focus==NULL) Focus=PageTree;
    SetPageControlsFont();
    ShowPageControls();
  }
}
//---------------------------------------------------------------------------
void TGeneralInfo::CreateLinksPage()
{
  Scroller.Create(WS_CHILD | WS_VSCROLL | WS_HSCROLL,page_l-10,0,page_w+20,INFOBOX_HEIGHT,Handle,400,HInstance);

  int TextHeight=GetTextSize(Font,"HyITljq").Height;
  int y=10,id=INFOPAGE_LINK_ID_BASE,group_id=3000;
  HWND Win;

  EasyStringList desc_sl,link_sl;
  GetHyperlinkLists(desc_sl,link_sl);

  for (int n=0;n<desc_sl.NumStrings;n++){
    if (desc_sl[n].Data[0]==0){
      Win=CreateWindowEx(0,"Steem HyperLink",Str(desc_sl[n].String)+"|"+link_sl[n].String,
                          WS_CHILD | WS_VISIBLE,10,y,250,15,
                          Scroller,(HMENU)id++,HInstance,NULL);
      ToolAddWindow(ToolTip,Win,link_sl[n].String);
      y+=TextHeight+5;
    }else{
      y+=10;
      CreateWindowEx(0,"Steem HyperLink",desc_sl[n].String,
                          HL_STATIC | WS_CHILD | WS_VISIBLE,
                          10,y,250,15,Scroller,(HMENU)group_id++,HInstance,NULL);
      y+=TextHeight+5;
    }
  }

  MaxLinkID=id;

  SetWindowAndChildrensFont(Scroller.GetControlPage(),Font);

  Scroller.AutoSize(2,2);

  if (Focus==NULL) Focus=PageTree;
  SetPageControlsFont();
  ShowPageControls();
}
//---------------------------------------------------------------------------
void TGeneralInfo::CreateReadmePage(int p)
{
  if (GetDlgItem(Handle,500)==NULL){
    int wid_of_search=get_text_width(T("Search"));
    int wid_of_find=get_text_width(T("Find"))+20;
    CreateWindow("Static",T("Search"),WS_CHILD | WS_VISIBLE,
                            page_l,14,wid_of_search,23,Handle,(HMENU)503,HInstance,NULL);

    CreateWindowEx(512,"Edit",SearchText,WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                            page_l+wid_of_search+5,10,page_w-wid_of_find-5-wid_of_search-5,23,
                            Handle,(HMENU)504,HInstance,NULL);

    CreateWindow("Button",T("Find"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                      page_l+page_w-wid_of_find,10,wid_of_find,23,Handle,(HMENU)502,HInstance,NULL);
/*
    HWND Win=CreateWindowEx(512,"Edit","",WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_AUTOVSCROLL,
                            page_l,40,page_w,INFOBOX_HEIGHT-50,Handle,(HMENU)500,HInstance,NULL);
*/
    CreateTextDisplay(Handle,page_l,40,page_w,INFOBOX_HEIGHT-50,500);

//    MakeEditNoCaret(Win);
  }

  Str TextFile=RunDir+SLASH;
  switch (p){
    case INFOPAGE_README: TextFile+="readme.txt"; break;
    case INFOPAGE_HOWTO_DISK: TextFile+="disk image howto.txt"; break;
    case INFOPAGE_HOWTO_CART: TextFile+="cart image howto.txt"; break;
    case INFOPAGE_FAQ: TextFile+="faq.txt"; break;
  }
  FILE *f=fopen(TextFile,"rb");
  if (f){
    char text[64000];
    text[fread(text,1,64000,f)]=0;
    fclose(f);

    SendMessage(GetDlgItem(Handle,500),WM_SETTEXT,0,(LPARAM)text);
  }

  if (Focus==NULL) Focus=GetDlgItem(Handle,504);
  SetPageControlsFont();
  ShowPageControls();
}
//---------------------------------------------------------------------------
void TGeneralInfo::Hide()
{
  if (Handle==NULL) return;

  ShowWindow(Handle,SW_HIDE);
  if (FullScreen) SetFocus(StemWin);

  DestroyCurrentPage();
  DestroyWindow(Handle);Handle=NULL;

  ImageList_Destroy(il); il=NULL;

  if (StemWin) PostMessage(StemWin,WM_USER,1234,0);

  ManageWindowClasses(SD_UNREGISTER);
}
//---------------------------------------------------------------------------
#define GET_THIS This=(TGeneralInfo*)GetWindowLong(Win,GWL_USERDATA);

LRESULT __stdcall TGeneralInfo::WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar)
{
  LRESULT Ret=DefStemDialogProc(Win,Mess,wPar,lPar);
  if (StemDialog_RetDefVal) return Ret;

  TGeneralInfo *This;
  switch (Mess){
    case WM_COMMAND:
      GET_THIS;

      switch (LOWORD(wPar)){
        case 504:
          if (HIWORD(wPar)==EN_CHANGE){
            int SearchTextLen=SendMessage(GetDlgItem(Win,504),WM_GETTEXTLENGTH,0,0);
            This->SearchText.SetLength(SearchTextLen);
            if (SearchTextLen){
              SendMessage(GetDlgItem(Win,504),WM_GETTEXT,SearchTextLen+1,LPARAM(This->SearchText.Text));
            }
          }
          break;
        case IDOK:case 502:
          if (This->SearchText.Length()){
            HWND te=GetDlgItem(Win,500);
            int TextLen=SendMessage(te,WM_GETTEXTLENGTH,0,0);
            char *Text=new char[TextLen+1];
            SendMessage(te,WM_GETTEXT,TextLen+1,LPARAM(Text));
            strupr(Text);

            EasyStr SText=This->SearchText.UpperCase();

            int tp=LOWORD(SendMessage(te,EM_GETSEL,0,0));
            int n;
            for (n=0;n<2;n++){
              char *textpos=strstr(Text+tp+1,SText);
              if (textpos){
                tp=(int)(textpos-Text);
                if (tp<TextLen){
                  SendMessage(te,EM_SETSEL,tp,tp+This->SearchText.Length());
                  int first_line=SendMessage(te,EM_GETFIRSTVISIBLELINE,0,0);
                  int sel_line=SendMessage(te,EM_LINEFROMCHAR,tp,0);
                  SendMessage(te,EM_LINESCROLL,0,max(sel_line-5,0)-first_line);
                  break;
                }
              }
              tp=-1;
            }
            if (n==2) MessageBeep(0);
            delete[] Text;
          }
          break;
      }
      break;
    case WM_NOTIFY:
    {
      NMHDR *pnmh=(NMHDR*)lPar;
      if (wPar==60000){
        GET_THIS;
        switch (pnmh->code){
          case TVN_SELCHANGING:
          {
            NM_TREEVIEW *Inf=(NM_TREEVIEW*)lPar;

            if (Inf->action==4096) return true; //DODGY!!!!!! Undocumented!!!!!
            return 0;
          }
          case TVN_SELCHANGED:
          {
            NM_TREEVIEW *Inf=(NM_TREEVIEW*)lPar;

            if (Inf->itemNew.hItem){
              TV_ITEM tvi;

              tvi.mask=TVIF_PARAM;
              tvi.hItem=(HTREEITEM)Inf->itemNew.hItem;
              SendMessage(This->PageTree,TVM_GETITEM,0,(LPARAM)&tvi);

              bool Destroy=true;
              if (GetDlgItem(Win,500)){ // On text page
                switch (tvi.lParam){
                  case INFOPAGE_README:case INFOPAGE_HOWTO_DISK:
                  case INFOPAGE_HOWTO_CART:case INFOPAGE_FAQ:
                    Destroy=0;
                    break;
                }
              }
              if (Destroy) This->DestroyCurrentPage();
              This->Page=tvi.lParam;
              This->CreatePage(This->Page);
            }
            break;
          }
        }
      }
      break;
    }
    case (WM_USER+1011):
    {
      GET_THIS;

      HWND NewParent=(HWND)lPar;
      if (NewParent){
        This->CheckFSPosition(NewParent);
        SetWindowPos(Win,NULL,This->FSLeft,This->FSTop,0,0,SWP_NOZORDER | SWP_NOSIZE);
      }else{
        SetWindowPos(Win,NULL,This->Left,This->Top,0,0,SWP_NOZORDER | SWP_NOSIZE);
      }
      This->ChangeParent(NewParent);
      break;
    }
    case WM_CLOSE:
      GET_THIS;
      This->Hide();
      return 0;
    case DM_GETDEFID:
      return MAKELONG(502,DC_HASDEFID);
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}
#undef GET_THIS
#endif

#ifdef UNIX
#include "x/x_infobox.cpp"
#endif

