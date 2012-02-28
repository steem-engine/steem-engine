//---------------------------------------------------------------------------
void TOptionBox::CreatePage(int n)
{
	switch (n){
    case 9:CreateMachinePage();break;
    case 10:CreateTOSPage();break;
    case 13:CreateMacrosPage();break;
    case 12:CreatePortsPage();break;

    case 0:CreateGeneralPage();break;
    case 5:CreateSoundPage();break;
    case 1:CreateDisplayPage();break;
    case 14:CreateOSDPage();break;
    case 2:CreateBrightnessPage();break;
    case 11:CreateProfilesPage();break;
    case 6:CreateStartupPage();break;
    case 15:CreatePathsPage();break;
	}
}
//---------------------------------------------------------------------------
void TOptionBox::CreateMachinePage()
{
  int y=10;

#if defined(STEVEN_SEAGAL) && defined(SS_STF)
  // Switch STF/STE
  st_type_label.create(XD,page_p,page_l,y,0,25,NULL,this,
          					BT_LABEL,T("ST Model"),0,BkCol);
  st_type_dd.make_empty();
  st_type_dd.additem(T("STF"));
  st_type_dd.additem(T("STE"));
  st_type_dd.changesel(ST_type);

  st_type_dd.create(XD,page_p,page_l+5+bo_label.w,y,
  						page_w-(5+bo_label.w),210,dd_notify_proc,this);
  y+=35;

#endif

  int Wid=hxc::get_text_width(XD,T("ST CPU speed"));
	cpu_boost_label.create(XD,page_p,page_l,y,Wid,25,NULL,this,BT_STATIC | BT_TEXT,T("ST CPU speed"),0,BkCol);
  cpu_boost_dd.id=8;

  cpu_boost_dd.make_empty();
  EasyStr Mhz=T("Megahertz");
  cpu_boost_dd.additem(EasyStr("8 ")+Mhz+" ("+T("ST standard")+")",8000000);
  cpu_boost_dd.additem(EasyStr("9 ")+Mhz,9000000);
  cpu_boost_dd.additem(EasyStr("10 ")+Mhz,10000000);
  cpu_boost_dd.additem(EasyStr("11 ")+Mhz,11000000);
  cpu_boost_dd.additem(EasyStr("12 ")+Mhz,12000000);
  cpu_boost_dd.additem(EasyStr("14 ")+Mhz,14000000);
  cpu_boost_dd.additem(EasyStr("16 ")+Mhz,16000000);
  cpu_boost_dd.additem(EasyStr("20 ")+Mhz,20000000);
  cpu_boost_dd.additem(EasyStr("24 ")+Mhz,24000000);
  cpu_boost_dd.additem(EasyStr("28 ")+Mhz,28000000);
  cpu_boost_dd.additem(EasyStr("32 ")+Mhz,32000000);
  cpu_boost_dd.additem(EasyStr("36 ")+Mhz,36000000);
  cpu_boost_dd.additem(EasyStr("40 ")+Mhz,40000000);
  cpu_boost_dd.additem(EasyStr("44 ")+Mhz,44000000);
  cpu_boost_dd.additem(EasyStr("48 ")+Mhz,48000000);
  cpu_boost_dd.additem(EasyStr("56 ")+Mhz,56000000);
  cpu_boost_dd.additem(EasyStr("64 ")+Mhz,64000000);
  cpu_boost_dd.additem(EasyStr("80 ")+Mhz,80000000);
  cpu_boost_dd.additem(EasyStr("96 ")+Mhz,96000000);
  cpu_boost_dd.additem(EasyStr("128 ")+Mhz,128000000);

  if (cpu_boost_dd.select_item_by_data(n_cpu_cycles_per_second)<0){
    EasyStr Cycles=n_cpu_cycles_per_second;
    Cycles=Cycles.Lefts(Cycles.Length()-6);
    cpu_boost_dd.additem(Cycles+" "+Mhz,n_cpu_cycles_per_second);
    cpu_boost_dd.changesel(cpu_boost_dd.lv.sl.NumStrings-1);
  }

  cpu_boost_dd.create(XD,page_p,page_l+5+Wid,y,400-(15+Wid+10),350,dd_notify_proc,this);
  y+=35;

  memory_label.create(XD,page_p,page_l,y,0,25,NULL,this,BT_LABEL,T("Memory size"),0,BkCol);

  memory_dd.id=910;
  memory_dd.make_empty();
  memory_dd.lv.sl.Add("512Kb",MEMCONF_512,MEMCONF_0);
  memory_dd.lv.sl.Add("1 MB",MEMCONF_512,MEMCONF_512);
  memory_dd.lv.sl.Add("2 MB",MEMCONF_2MB,MEMCONF_0);
  memory_dd.lv.sl.Add("4 MB",MEMCONF_2MB,MEMCONF_2MB);
  memory_dd.lv.sl.Add("14 MB",MEMCONF_7MB,MEMCONF_7MB);
	memory_dd.create(XD,page_p,page_l+5+memory_label.w,y,page_w-(5+memory_label.w),200,dd_notify_proc,this);
	y+=35;

	monitor_label.create(XD,page_p,page_l,y,0,25,NULL,this,BT_LABEL,
													T("Monitor"),0,BkCol);

  monitor_dd.id=920;
  monitor_dd.make_empty();
  monitor_dd.additem(T("Colour")+" ("+T("Low/Med Resolution")+")");
  monitor_dd.additem(T("Monochrome")+" ("+T("High Resolution")+")");
#ifndef NO_CRAZY_MONITOR
  for(int n=0;n<EXTMON_RESOLUTIONS;n++){
    monitor_dd.additem(T("Extended Monitor At")+" "+extmon_res[n][0]+"x"+extmon_res[n][1]+"x"+extmon_res[n][2]);
  }
#endif

  monitor_dd.create(XD,page_p,page_l+5+monitor_label.w,y,page_w-(5+monitor_label.w),200,dd_notify_proc,this);
  y+=35;

  hxc_button *kg=new hxc_button(XD,page_p,page_l,y,page_w,85,NULL,this,
					BT_GROUPBOX,T("Keyboard"),0,hxc::col_bk);

  keyboard_language_dd.id=940;
  keyboard_language_dd.make_empty();
  keyboard_language_dd.additem(T("United States"),MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US));
  keyboard_language_dd.additem(T("United Kingdom"),MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_UK));
  keyboard_language_dd.additem(T("Australia (UK TOS)"),MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_AUS));
  keyboard_language_dd.additem(T("German"),MAKELANGID(LANG_GERMAN,SUBLANG_GERMAN));
  keyboard_language_dd.additem(T("French"),MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH));
  keyboard_language_dd.additem(T("Spanish"),MAKELANGID(LANG_SPANISH,SUBLANG_SPANISH));
  keyboard_language_dd.additem(T("Italian"),MAKELANGID(LANG_ITALIAN,SUBLANG_ITALIAN));
  keyboard_language_dd.additem(T("Swedish"),MAKELANGID(LANG_SWEDISH,SUBLANG_SWEDISH));
  keyboard_language_dd.additem(T("Norwegian"),MAKELANGID(LANG_NORWEGIAN,SUBLANG_NEUTRAL));
  keyboard_language_dd.additem(T("Belgian"),MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH_BELGIAN));
  if (keyboard_language_dd.select_item_by_data(KeyboardLangID)<0){
    // if can't find the language
    keyboard_language_dd.sel=0;
    KeyboardLangID=MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US);
  }
  keyboard_language_dd.grandfather=page_p;
  Wid=hxc::get_text_width(XD,T("Language"));
	keyboard_language_label.create(XD,kg->handle,10,20,Wid,25,NULL,this,
                                  BT_TEXT | BT_STATIC | BT_BORDER_NONE,
																	T("Language"),0,BkCol);
  keyboard_language_dd.create(XD,kg->handle,15+Wid,20,page_w-20-(5+Wid),200,dd_notify_proc,this);

  keyboard_sc_but.set_check(EnableShiftSwitching);
  keyboard_sc_but.create(XD,kg->handle,10,50,0,25,button_notify_proc,this,
    BT_CHECKBOX,T("Shift and alternate correction"),960,BkCol);
  hints.add(keyboard_sc_but.handle,T("When checked this allows Steem to emulate all keys correctly, it does this by changing the shift and alternate state of the ST when you press them.")+" "+
                              T("This could interfere with games and other programs, only use it if you are doing lots of typing.")+" "+
                              T("Please note that instead of pressing Alt-Gr or Control to access characters on the right-hand side of a key, you have to press Alt or Alt+Shift (this is how it was done on an ST)."),
                              page_p);
  y+=95;

  cart_group.create(XD,page_p,page_l,y,page_w,90,NULL,this,BT_GROUPBOX,T("Cartridge"),0,BkCol);

  cart_display.create(XD,cart_group.handle,10,25,
    page_w-20,25,NULL,this,BT_STATIC|BT_TEXT|BT_BORDER_INDENT|BT_TEXT_PATH,
    CartFile.Text,0,WhiteCol);

  cart_change_but.create(XD,cart_group.handle,10,55,
    page_w/2-10-5,25,button_notify_proc,this,BT_TEXT,T("Choose"),737,BkCol);

  cart_remove_but.create(XD,cart_group.handle,page_w/2+5,55,
    page_w/2-10-5,25,button_notify_proc,this,BT_TEXT,T("Remove"),747,BkCol);
  y+=100;

  mustreset_td.text=T("Memory and monitor changes don't take effect until the next cold reset of the ST");
  mustreset_td.sy=0;
  mustreset_td.wordwrapped=false;
	mustreset_td.create(XD,page_p,page_l,y,page_w,45,hxc::col_white,0);
  y+=55;

  coldreset_but.create(XD,page_p,page_l,y,page_w,25,button_notify_proc,this,
            BT_TEXT,T("Perform Cold Reset Now"),1000,hxc::col_bk);

  MachineUpdateIfVisible();

  XFlush(XD);
}
//---------------------------------------------------------------------------
void TOptionBox::MachineUpdateIfVisible()
{
  TOSRefreshBox();

  int memconf=4;
  if (NewMemConf0<0){
    if (mem_len<1024*1024){
      memconf=0;
    }else if (mem_len<2048*1024){
      memconf=1;
    }else if (mem_len<4096*1024){
      memconf=2;
    }else if (mem_len<14*1024*1024){
      memconf=3;
    }
  }else{
    if (NewMemConf0==MEMCONF_512) memconf=int((NewMemConf1==MEMCONF_512) ? 1:0); // 1Mb:512Kb
    if (NewMemConf0==MEMCONF_2MB) memconf=int((NewMemConf1==MEMCONF_2MB) ? 3:2); // 4Mb:2Mb
  }

  memory_dd.changesel(memconf);

  int monitor_sel=NewMonitorSel;
  if (monitor_sel<0) monitor_sel=GetCurrentMonitorSel();
  monitor_dd.changesel(monitor_sel);

  cart_display.set_text(CartFile.Text);
}
//---------------------------------------------------------------------------
void TOptionBox::CreateTOSPage()
{
	int y=10,tosbox_h=OPTIONS_HEIGHT-10-35-10-35-55-25-10;

	hxc_button *label=new hxc_button(XD,page_p,page_l,y,0,25,NULL,this,
                      BT_TEXT | BT_STATIC | BT_BORDER_NONE,
											T("Sort by"),0,BkCol);

  tos_sort_dd.make_empty();
  tos_sort_dd.lv.sl.Add(T("Version (Ascending)"),eslSortByData0,0);
  tos_sort_dd.lv.sl.Add(T("Version (Descending)"),eslSortByData0,1);
  tos_sort_dd.lv.sl.Add(T("Language"),eslSortByData1,0);
  tos_sort_dd.lv.sl.Add(T("Date (Ascending)"),eslSortByData2,0);
  tos_sort_dd.lv.sl.Add(T("Date (Descending)"),eslSortByData2,1);
  tos_sort_dd.lv.sl.Add(T("Name (Ascending)"),eslSortByNameI,0);
  tos_sort_dd.lv.sl.Add(T("Name (Descending)"),eslSortByNameI,1);
  bool Found=0;
	for (int i=0;i<tos_sort_dd.lv.sl.NumStrings;i++){
		if (tos_sort_dd.lv.sl[i].Data[0]==(long)eslTOS_Sort){
			if (tos_sort_dd.lv.sl[i].Data[1]==(long)eslTOS_Descend){
				Found=true;
				tos_sort_dd.sel=i;
				break;
			}
		}
	}
	if (Found==0){
		tos_sort_dd.sel=0;
		eslTOS_Sort=eslSortByData0;
		eslTOS_Descend=0;
	}
	tos_sort_dd.create(XD,page_p,page_l+label->w+5,y,page_w-(label->w+5),200,
									dd_notify_proc,this);
  tos_sort_dd.id=1020;
  y+=35;

  tos_lv.id=1000;
  tos_lv.create(XD,page_p,page_l,y,page_w,tosbox_h,listview_notify_proc,this);
  y+=tosbox_h+5;


	tosadd_but.create(XD,page_p,page_l,y,page_w/2-5,25,button_notify_proc,this,
            BT_TEXT,T("Add To List"),1010,hxc::col_bk);

  tosrefresh_but.create(XD,page_p,page_l+page_w/2+5,y,page_w/2-5,25,button_notify_proc,this,
            BT_TEXT,T("Refresh"),1011,hxc::col_bk);
  y+=35;

  mustreset_td.text=T("TOS changes don't take effect until the next cold reset of the ST");
  mustreset_td.sy=0;
  mustreset_td.wordwrapped=false;
	mustreset_td.create(XD,page_p,page_l,y,page_w,45,hxc::col_white,0);
  y+=55;

  coldreset_but.create(XD,page_p,page_l,y,page_w,25,button_notify_proc,this,
            BT_TEXT,T("Perform Cold Reset Now"),1000,hxc::col_bk);

  XFlush(XD);

  TOSRefreshBox();
}
//---------------------------------------------------------------------------
void TOptionBox::CreateMacrosPage()
{
	int y=10;
  dir_lv.ext_sl.DeleteAll();
  dir_lv.ext_sl.Add(3,T("Parent Directory"),1,1,0);
  dir_lv.ext_sl.Add(3,"",ICO16_FOLDER,ICO16_FOLDERLINK,0);
  dir_lv.ext_sl.Add(3,"stmac",ICO16_MACROS,ICO16_MACROLINK,0);
  dir_lv.lpig=&Ico16;
  dir_lv.base_fol=MacroDir;
  dir_lv.fol=MacroDir;
  dir_lv.allow_type_change=0;
  dir_lv.show_broken_links=0;
  dir_lv.lv.sel=-1;
  if (MacroSel.NotEmpty()){
		dir_lv.fol=MacroSel;
		RemoveFileNameFromPath(dir_lv.fol,REMOVE_SLASH);
	}
  dir_lv.id=2000;
  int dir_lv_h=OPTIONS_HEIGHT-10-10-30-20-30-30;
  dir_lv.create(XD,page_p,page_l,y,page_w,dir_lv_h-5,
                dir_lv_notify_proc,this);
  y+=dir_lv_h;

  hxc_button *p_but,*p_grp;

  new hxc_button(XD,page_p,page_l,y,page_w/2-5,25,button_notify_proc,this,
  										BT_TEXT,T("New Macro"),2001,BkCol);

  new hxc_button(XD,page_p,page_l+page_w/2+5,y,page_w/2-5,25,
                      button_notify_proc,this,BT_TEXT,
                      T("Change Store Folder"),2002,BkCol);
  y+=30;

  p_grp=new hxc_button(XD,page_p,page_l,y,page_w,20+30+30,NULL,this,
                      BT_GROUPBOX,T("Controls"),2009,BkCol);
  y=20;

  int x=10;
  p_but=new hxc_button(XD,p_grp->handle,x,y,25,25,
                      button_notify_proc,this,BT_ICON,"",2010,BkCol);
  p_but->set_icon(&Ico32,ICO32_RECORD);
  x+=p_but->w+5;

  p_but=new hxc_button(XD,p_grp->handle,x,y,25,25,
                      button_notify_proc,this,BT_ICON,"",2011,BkCol);
  p_but->set_icon(&Ico32,ICO32_PLAY);
  x+=p_but->w+5;

  p_but=new hxc_button(XD,p_grp->handle,x,y,0,25,NULL,this,
                      BT_LABEL,T("Mouse speed"),0,BkCol);
  x+=p_but->w+5;

  hxc_dropdown *p_dd=new hxc_dropdown(XD,p_grp->handle,x,y,
                          page_w-10-x,300,dd_notify_proc,this);
	p_dd->id=2012;
	p_dd->make_empty();
  p_dd->additem(T("Safe"),15);
  p_dd->additem(T("Slow"),32);
  p_dd->additem(T("Medium"),64);
  p_dd->additem(T("Fast"),96);
  p_dd->additem(T("V.Fast"),127);
  p_dd->select_item_by_data(127);
  y+=30;

  x=10;
  p_but=new hxc_button(XD,p_grp->handle,x,y,0,25,NULL,this,
                      BT_LABEL,T("Playback event delay"),0,BkCol);
  x+=p_but->w+5;

  p_dd=new hxc_dropdown(XD,p_grp->handle,x,y,page_w-10-x,300,
                    dd_notify_proc,this);
	p_dd->id=2013;
	p_dd->make_empty();
  p_dd->additem(T("As Recorded"),0);
  EasyStr Ms=Str(" ")+T("Milliseconds");
  for (int n=1;n<=25;n++) p_dd->additem(Str(n*20)+Ms,n);
  p_dd->select_item_by_data(1);

	if (MacroSel.NotEmpty()){
		dir_lv.select_item_by_name(GetFileNameFromPath(MacroSel));
	}
  UpdateMacroRecordAndPlay();

  XFlush(XD);
}
//---------------------------------------------------------------------------
void TOptionBox::CreatePortsPage()
{
	int y=10;
	PortGroup[0].create(XD,page_p,page_l,y,page_w,25+90+5,NULL,this,BT_GROUPBOX,
												T("MIDI Port"),0,BkCol);
	y+=25+90+5+10;
	PortGroup[1].create(XD,page_p,page_l,y,page_w,25+90+5,NULL,this,BT_GROUPBOX,
												T("Parallel Port"),0,BkCol);
	y+=25+90+5+10;
	PortGroup[2].create(XD,page_p,page_l,y,page_w,25+90+5,NULL,this,BT_GROUPBOX,
												T("Serial Port"),0,BkCol);

	for (int p=0;p<3;p++){
    int IDBase=1200+p*20;
		y=25;
		ConnectLabel[p].create(XD,PortGroup[p].handle,10,y,0,25,NULL,this,BT_LABEL,
														T("Connect to"),0,BkCol);

		ConnectDD[p].make_empty();
    ConnectDD[p].additem(T("None"),PORTTYPE_NONE);
    ConnectDD[p].additem(T("MIDI Port Device"),PORTTYPE_MIDI);
//    ConnectDD[p].additem(T("MIDI Sequencer Device"),PORTTYPE_UNIX_SEQUENCER);
    if (AllowLPT) ConnectDD[p].additem(T("Parallel Port Device"),PORTTYPE_PARALLEL);
    if (AllowCOM) ConnectDD[p].additem(T("Serial Port Device"),PORTTYPE_COM);
    ConnectDD[p].additem(T("Named Pipes"),PORTTYPE_LAN);
    ConnectDD[p].additem(T("Other Device"),PORTTYPE_UNIX_OTHER);
    ConnectDD[p].additem(T("File"),PORTTYPE_FILE);
    ConnectDD[p].additem(T("Loopback (Output->Input)"),PORTTYPE_LOOP);
		ConnectDD[p].select_item_by_data(STPort[p].Type);
		ConnectDD[p].grandfather=page_p;
		ConnectDD[p].id=IDBase+0;
		ConnectDD[p].create(XD,PortGroup[p].handle,15+ConnectLabel[p].w,
														y,page_w-10-(15+ConnectLabel[p].w),200,dd_notify_proc,this);
		y+=30;

    //---------------------------------------------------------------------------
		IOGroup[p].create(XD,PortGroup[p].handle,10,y,PortGroup[p].w-20,60,NULL,this,
														BT_STATIC,"",0,BkCol);

		IOChooseBut[p].create(XD,IOGroup[p].handle,IOGroup[p].w,0,0,25,
												button_notify_proc,this,BT_TEXT,T("Choose"),IDBase+1,BkCol);
		IOChooseBut[p].x-=IOChooseBut[p].w;
		XMoveResizeWindow(XD,IOChooseBut[p].handle,IOChooseBut[p].x,IOChooseBut[p].y,
																IOChooseBut[p].w,IOChooseBut[p].h);

		IODevEd[p].create(XD,IOGroup[p].handle,0,0,IOChooseBut[p].x-10,25,edit_notify_proc,this);
		IODevEd[p].id=IDBase+2;

		IOAllowIOBut[p][0].create(XD,IOGroup[p].handle,0,30,0,25,
												button_notify_proc,this,BT_CHECKBOX,
												T("Output"),IDBase+3,BkCol);

		IOAllowIOBut[p][1].create(XD,IOGroup[p].handle,IOGroup[p].w/3,30,
												0,25,button_notify_proc,this,
												BT_CHECKBOX,T("Input"),IDBase+4,BkCol);

		IOOpenBut[p].create(XD,IOGroup[p].handle,(IOGroup[p].w/3)*2,30,
												(IOGroup[p].w/3),25,button_notify_proc,this,
												BT_TEXT,T("Open"),IDBase+5,BkCol);

    //---------------------------------------------------------------------------
		LANGroup[p].create(XD,PortGroup[p].handle,10,y,PortGroup[p].w-20,60,NULL,this,
														BT_STATIC,"",0,BkCol);

		hxc_button *p_but=new hxc_button(XD,LANGroup[p].handle,LANGroup[p].w,0,0,55,
												button_notify_proc,this,BT_TEXT,T("Open"),IDBase+14,BkCol);
		p_but->x-=p_but->w;
		XMoveResizeWindow(XD,p_but->handle,p_but->x,p_but->y,p_but->w,p_but->h);
    int lan_wid=p_but->x-5;

		p_but=new hxc_button(XD,LANGroup[p].handle,lan_wid,0,0,25,
												button_notify_proc,this,BT_TEXT,T("Choose"),IDBase+11,BkCol);
		p_but->x-=p_but->w;
		XMoveResizeWindow(XD,p_but->handle,p_but->x,p_but->y,p_but->w,p_but->h);

		hxc_button *p_lab=new hxc_button(XD,LANGroup[p].handle,0,0,0,25,NULL,this,
                                    BT_LABEL,T("Output"),0,BkCol);

		hxc_edit *p_ed=new hxc_edit(XD,LANGroup[p].handle,p_lab->w+5,0,p_but->x-5-(p_lab->w+5),25,
                                  edit_notify_proc,this);
    p_ed->id=IDBase+10;

		p_but=new hxc_button(XD,LANGroup[p].handle,lan_wid,30,0,25,
												button_notify_proc,this,BT_TEXT,T("Choose"),IDBase+13,BkCol);
		p_but->x-=p_but->w;
		XMoveResizeWindow(XD,p_but->handle,p_but->x,p_but->y,p_but->w,p_but->h);

		p_lab=new hxc_button(XD,LANGroup[p].handle,0,30,0,25,NULL,this,
                                    BT_LABEL,T("Input"),0,BkCol);

		p_ed=new hxc_edit(XD,LANGroup[p].handle,p_lab->w+5,30,p_but->x-5-(p_lab->w+5),25,
                                  edit_notify_proc,this);
    p_ed->id=IDBase+12;

    //---------------------------------------------------------------------------
		FileGroup[p].create(XD,PortGroup[p].handle,10,y,PortGroup[p].w-20,60,NULL,this,
														BT_STATIC,"",0,BkCol);



		FileDisplay[p].create(XD,FileGroup[p].handle,0,0,FileGroup[p].w,25,
												NULL,this,BT_TEXT | BT_BORDER_INDENT | BT_STATIC | BT_TEXT_PATH,
                        STPort[p].File,0,WhiteCol);

		FileChooseBut[p].create(XD,FileGroup[p].handle,0,30,FileGroup[p].w/2-5,25,
												button_notify_proc,this,BT_TEXT,T("Choose"),IDBase+6,BkCol);

		FileEmptyBut[p].create(XD,FileGroup[p].handle,FileGroup[p].w/2+5,30,FileGroup[p].w/2-5,25,
												button_notify_proc,this,BT_TEXT,T("Empty"),IDBase+7,BkCol);

		UpdatePortDisplay(p);
	}

  XFlush(XD);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void TOptionBox::CreateGeneralPage()
{
	int y=10;
  RunSpeedLabel.create(XD,page_p,page_l,y,page_w,25,NULL,this,
      BT_STATIC | BT_TEXT | BT_BORDER_NONE | BT_TEXT_CENTRE,"",0,BkCol);
  y+=25;

  RunSpeedSB.horizontal=true;
  RunSpeedSB.init(189+10,10,((100000/run_speed_ticks_per_second)-50) / 5); // 1 tick per 5%
  RunSpeedSB.create(XD,page_p,page_l,y,page_w,25,scrollbar_notify_proc,this);
  RunSpeedSB.id=2;
  y+=35;

  SMSpeedLabel.create(XD,page_p,page_l,y,page_w,25,NULL,this,
      BT_STATIC | BT_TEXT | BT_BORDER_NONE | BT_TEXT_CENTRE,"",0,BkCol);
  y+=25;

  SMSpeedSB.horizontal=true;
  SMSpeedSB.init(79+5,5,(slow_motion_speed-10)/10);
  SMSpeedSB.create(XD,page_p,page_l,y,page_w,25,scrollbar_notify_proc,this);
  SMSpeedSB.id=0;
  y+=35;

  FFMaxSpeedLabel.create(XD,page_p,page_l,y,page_w,25,NULL,this,
        BT_STATIC | BT_TEXT | BT_BORDER_NONE | BT_TEXT_CENTRE,"",0,BkCol);
  y+=25;

  FFMaxSpeedSB.horizontal=true;
  FFMaxSpeedSB.init(18+4,4,(1000/max(fast_forward_max_speed,50))-2);
  FFMaxSpeedSB.create(XD,page_p,page_l,y,page_w,25,scrollbar_notify_proc,this);
  FFMaxSpeedSB.id=1;
  y+=35;
  scrollbar_notify_proc(&SMSpeedSB,SBN_SCROLL,SMSpeedSB.pos);

  hxc_button *p_but=new hxc_button(XD,page_p,page_l,y,0,25,button_notify_proc,this,
        BT_CHECKBOX,T("Show pop-up hints"),121,BkCol);
  p_but->set_check(ShowTips);
  y+=35;

  high_priority_but.create(XD,page_p,page_l,y,0,25,button_notify_proc,this,
        BT_CHECKBOX,T("Make Steem high priority"),120,BkCol);
  high_priority_but.set_check(HighPriority);
  hints.add(high_priority_but.handle,T("When this option is ticked Steem will get first use of the CPU ahead of other applications, this means Steem will still run smoothly even if you start doing something else at the same time, but everything else will run slower."),
              page_p);
  y+=35;

  pause_inactive_but.create(XD,page_p,page_l,y,0,25,button_notify_proc,this,
        BT_CHECKBOX,T("Pause emulation when inactive"),110,BkCol);
  pause_inactive_but.set_check(PauseWhenInactive);
  y+=35;

  ff_on_fdc_but.create(XD,page_p,page_l,y,0,25,
          button_notify_proc,this,BT_CHECKBOX,
          T("Automatic fast forward on disk access"),130,BkCol);
  ff_on_fdc_but.set_check(floppy_access_ff);
  y+=35;

  start_click_but.create(XD,page_p,page_l,y,0,25,
          button_notify_proc,this,BT_CHECKBOX,
          T("Start emulation on mouse click"),140,BkCol);
  start_click_but.set_check(StartEmuOnClick);
  hints.add(start_click_but.handle,T("When this option is ticked clicking a mouse button on Steem's main window will start emulation."),
              page_p);

#if defined(STEVEN_SEAGAL) && defined(SS_VARIOUS)
  // Option Specific hacks
  y+=30;
  specific_hacks_but.create(XD,page_p,page_l,y,0,25,
          button_notify_proc,this,BT_CHECKBOX,
          T("Hacks"),160,BkCol);
  specific_hacks_but.set_check(SpecificHacks);
  hints.add(specific_hacks_but.handle,T("If checked, specific hacks targetting known programs are used. Those hacks may break other programs."),
              page_p);

#endif

  XFlush(XD);
}
//---------------------------------------------------------------------------
void TOptionBox::CreateSoundPage()
{
	int y=10;

	sound_mode_label.create(XD,page_p,page_l,y,0,25,NULL,this,BT_LABEL,T("Output type"),0,BkCol);

	sound_mode_dd.create(XD,page_p,page_l+5+sound_mode_label.w,y,
							page_w-(5+sound_mode_label.w),200,dd_notify_proc,this);
  sound_mode_dd.id=5001;
  sound_mode_dd.make_empty();
  sound_mode_dd.additem(T("None (Mute)"));
  sound_mode_dd.additem(T("Simulated ST Speaker"));
  sound_mode_dd.additem(T("Direct"));
  sound_mode_dd.additem(T("Sharp STFM Samples"));
	sound_mode_dd.changesel(sound_mode);
	y+=35;

  sound_group.create(XD,page_p,page_l,y,page_w,210,NULL,this,BT_GROUPBOX,
  											T("Configuration"),0,BkCol);

	int sgy=25;

  hxc_button *but=new hxc_button(XD,sound_group.handle,10,sgy,0,25,NULL,this,BT_LABEL,T("Library"),0,BkCol);

  hxc_dropdown *dd=new hxc_dropdown(XD,sound_group.handle,15+but->w,sgy,(sound_group.w-10-(15+but->w)),300,dd_notify_proc,this);
  dd->id=5067;
  dd->additem("None",0);
#ifndef NO_RTAUDIO
  dd->additem("RtAudio",XS_RT);
#endif
#ifndef NO_PORTAUDIO
  dd->additem("PortAudio",XS_PA);
#endif
  dd->select_item_by_data(x_sound_lib);
  sgy+=30;


  device_label.create(XD,sound_group.handle,10,sgy,0,25,NULL,this,
											BT_LABEL,T("Device"),0,BkCol);

  dd=new hxc_dropdown(XD,sound_group.handle,15+device_label.w,sgy,
                        (sound_group.w-10-(15+device_label.w)),300,dd_notify_proc,this);
  dd->id=5000;
  sgy+=30;

  but=new hxc_button(XD,sound_group.handle,10,sgy,0,25,NULL,this,BT_LABEL,T("Timing Method"),0,BkCol);

  dd=new hxc_dropdown(XD,sound_group.handle,15+but->w,sgy,(sound_group.w-10-(15+but->w)),300,dd_notify_proc,this);
  dd->id=5005;
  dd->additem("Write Cursor",0);
  dd->additem("Clock",2);
  dd->select_item_by_data(sound_time_method);
  sgy+=30;

  but=new hxc_button(XD,sound_group.handle,10,sgy,0,25,NULL,this,BT_LABEL,T("Delay"),0,BkCol);

  dd=new hxc_dropdown(XD,sound_group.handle,15+but->w,sgy,(sound_group.w-10-(15+but->w)),300,dd_notify_proc,this);
  dd->id=5004;
  EasyStr Ms=T("Milliseconds");
  for (int i=20;i<=800;i+=20) dd->additem(Str(i)+" "+Ms,i/20);
  dd->sel=0;
  dd->select_item_by_data(psg_write_n_screens_ahead);
  sgy+=30;

	sound_freq_label.create(XD,sound_group.handle,10,sgy,0,25,NULL,this,
											BT_LABEL,T("Frequency"),0,BkCol);

	sound_freq_dd.create(XD,sound_group.handle,15+sound_freq_label.w,
							sgy,370-(15+sound_freq_label.w),200,dd_notify_proc,this);
  sound_freq_dd.id=5002;
  sound_freq_dd.make_empty();
  if (sound_comline_freq){
    sound_freq_dd.additem(Str(sound_comline_freq)+"Hz",sound_comline_freq);
  }
  sound_freq_dd.additem("50066Hz",50066);
  sound_freq_dd.additem("44100Hz",44100);
  sound_freq_dd.additem("25033Hz",25033);
  sound_freq_dd.additem("22050Hz",22050);
	sound_freq_dd.select_item_by_data(sound_chosen_freq);
	sound_freq_dd.grandfather=page_p;
  sgy+=30;

	sound_format_label.create(XD,sound_group.handle,10,sgy,0,25,NULL,this,
											BT_LABEL,T("Format"),0,BkCol);

	sound_format_dd.create(XD,sound_group.handle,15+sound_format_label.w,
							sgy,370-(15+sound_format_label.w),200,dd_notify_proc,this);
  sound_format_dd.id=5003;

  FillSoundDevicesDD();

  y+=220;

  record_group.create(XD,page_p,page_l,y,page_w,90,NULL,this,
  											BT_GROUPBOX,T("Record"),0,BkCol);

  record_but.set_icon(&Ico32,ICO32_RECORD);
	record_but.create(XD,record_group.handle,10,25,25,25,button_notify_proc,this,
											BT_ICON,"",5100,BkCol);
	record_but.set_check(sound_record);

	wav_choose_but.create(XD,record_group.handle,record_group.w-10,25,0,25,button_notify_proc,this,
											BT_TEXT,T("Choose"),5101,BkCol);
  wav_choose_but.x-=wav_choose_but.w;
  XMoveWindow(XD,wav_choose_but.handle,
              wav_choose_but.x,wav_choose_but.y);

	wav_output_label.create(XD,record_group.handle,40,25,wav_choose_but.x-10-40,25,NULL,this,
											BT_TEXT | BT_BORDER_INDENT | BT_STATIC | BT_TEXT_PATH,
											WAVOutputFile,0,WhiteCol);

	overwrite_ask_but.create(XD,record_group.handle,10,55,0,25,button_notify_proc,this,
				BT_CHECKBOX,T("Warn before overwrite"),5102,BkCol);
	overwrite_ask_but.set_check(RecordWarnOverwrite);
	y+=100;

	internal_speaker_but.create(XD,page_p,page_l,y,0,25,button_notify_proc,this,
					BT_CHECKBOX,T("Internal speaker sound"),5300,BkCol);
	internal_speaker_but.set_check(sound_internal_speaker);

#if defined(STEVEN_SEAGAL) && defined(SS_VARIOUS)
	keyboard_click_but.create(XD,page_p,page_l,y,0,25,button_notify_proc,this,
					BT_CHECKBOX,T("Keyboard click"),5301,BkCol);
	keyboard_click_but.set_check(mute_keyboard_click);
#endif

  XFlush(XD);
}
//---------------------------------------------------------------------------
void TOptionBox::FillSoundDevicesDD()
{
  hxc_dropdown *dd=(hxc_dropdown*)hxc::find(sound_group.handle,5000);
  dd->make_empty();
#ifndef NO_PORTAUDIO
  if (UseSound==XS_PA){
    int c=Pa_GetDeviceCount(),isel=Pa_GetDefaultOutputDevice();
    for (PaDeviceIndex i=0;i<c;i++){
      const PaDeviceInfo *pdev=Pa_GetDeviceInfo(i);
      if (pdev->maxOutputChannels>0){
        dd->additem((char*)(pdev->name),i);
        if (IsSameStr_I(pdev->name,sound_device_name)) isel=i;
      }
    }
    dd->select_item_by_data(max(isel,0));
    sound_device_name=dd->sl[dd->sel].String; // Just in case changed to default
  }
#endif
#ifndef NO_RTAUDIO
  if (UseSound==XS_RT){
    RtAudio::DeviceInfo radi;
    int c=rt_audio->getDeviceCount(),isel=0; //isel=default device, find while walking through list
    for (int i=1;i<=c;i++){
      radi=rt_audio->getDeviceInfo(i);
      if (radi.outputChannels>0){
        dd->additem((char*)(radi.name.c_str()),i);
        if (radi.isDefaultOutput) isel=i;
      }
    }
    for (int i=1;i<=c;i++){
      if (IsSameStr_I(radi.name.c_str(),sound_device_name)) isel=i;
    }
    dd->select_item_by_data(max(isel,0));
    sound_device_name=dd->sl[dd->sel].String; // Just in case changed to default
  }
#endif
  if (UseSound==0) dd->additem(T("None"));

#ifdef NO_RTAUDIO
  int rt_unsigned_8bit=0;
#endif
  sound_format_dd.make_empty();
  sound_format_dd.additem(T("8-Bit Mono"),MAKEWORD(8,1));
  sound_format_dd.additem(T("8-Bit Stereo"),MAKEWORD(8,2));
  if (x_sound_lib==XS_RT){
    sound_format_dd.additem(T("8-Bit Mono Unsigned"),MAKELONG(MAKEWORD(8,1),1));
    sound_format_dd.additem(T("8-Bit Stereo Unsigned"),MAKELONG(MAKEWORD(8,2),1));
  }
  sound_format_dd.additem(T("16-Bit Mono"),MAKEWORD(16,1));
  sound_format_dd.additem(T("16-Bit Stereo"),MAKEWORD(16,2));
  sound_format_dd.sel=-1;
	sound_format_dd.select_item_by_data(MAKELONG(MAKEWORD(sound_num_bits,sound_num_channels),rt_unsigned_8bit));
  if (sound_format_dd.sel==-1){
  	sound_format_dd.select_item_by_data(MAKEWORD(sound_num_bits,sound_num_channels));
  }
}
//---------------------------------------------------------------------------
void TOptionBox::CreateDisplayPage()
{
	int y=10;

#if defined(STEVEN_SEAGAL) && defined(SS_VID_BORDERS)
  // Option large border
 border_size_label.create(XD,page_p,page_l,y,0,25,NULL,this,
          					BT_LABEL,T("Border size"),0,BkCol);
  border_size_dd.make_empty();
  border_size_dd.additem(T("Normal (384x270)"));
  border_size_dd.additem(T("Large (400x275)"));
  border_size_dd.additem(T("Very large (416x280)"));
  border_size_dd.changesel(BorderSize);
  border_size_dd.create(XD,page_p,page_l+5+bo_label.w,y,
  						page_w-(5+bo_label.w),170,dd_notify_proc,this);
  y+=35;

#endif

  fs_label.create(XD,page_p,page_l,y,0,25,
				          NULL,this,BT_LABEL,T("Frameskip"),0,BkCol);

  frameskip_dd.make_empty();
  frameskip_dd.additem(T("Draw Every Frame"));
  frameskip_dd.additem(T("Draw Every Second Frame"));
  frameskip_dd.additem(T("Draw Every Third Frame"));
  frameskip_dd.additem(T("Draw Every Fourth Frame"));
  frameskip_dd.additem(T("Auto Frameskip"));
  frameskip_dd.changesel(min(frameskip-1,4));

  frameskip_dd.create(XD,page_p,page_l+5+fs_label.w,y,page_w-(5+fs_label.w),
	  200,dd_notify_proc,this);
	y+=35;

  bo_label.create(XD,page_p,page_l,y,0,25,NULL,this,
          					BT_LABEL,T("Borders"),0,BkCol);

  border_dd.make_empty();
  border_dd.additem(T("Never Show Borders"));
  border_dd.additem(T("Always Show Borders"));
  border_dd.additem(T("Auto Borders"));
  border_dd.changesel(min(border,2));

  border_dd.create(XD,page_p,page_l+5+bo_label.w,y,
  						page_w-(5+bo_label.w),210,dd_notify_proc,this);
  y+=35;





  hxc_button *p_but=new hxc_button(XD,page_p,page_l,y,0,25,button_notify_proc,this,
                  BT_CHECKBOX,T("Scanline Grille"),210,BkCol);
  p_but->set_check(draw_fs_fx==DFSFX_GRILLE);
  y+=35;

  p_but=new hxc_button(XD,page_p,page_l,y,0,25,button_notify_proc,this,
          BT_CHECKBOX,T("Asynchronous blitting (can be faster)"),220,BkCol);
  p_but->set_check(Disp.DoAsyncBlit);
  y+=35;

  {
    size_group.create(XD,page_p,page_l,y,page_w,120,
  												NULL,this,BT_STATIC | BT_TEXT | BT_BORDER_OUTDENT |
  												BT_TEXT_VTOP,T("Window Size"),0,BkCol);

    lowres_doublesize_but.create(XD,size_group.handle,10,25,0,25,
            button_notify_proc,this,BT_CHECKBOX,T("Low-res double size"),
            250,BkCol);
    lowres_doublesize_but.set_check(WinSizeForRes[0]);

    medres_doublesize_but.create(XD,size_group.handle,10,55,0,25,
            button_notify_proc,this,BT_CHECKBOX,T("Med-res double height"),
            251,BkCol);
    medres_doublesize_but.set_check(WinSizeForRes[1]);

    hxc_button *p_but=new hxc_button(XD,size_group.handle,10,85,0,25,
            button_notify_proc,this,BT_CHECKBOX,T("Fullscreen 640x400 (never show borders only)"),
            253,BkCol);
    p_but->set_check(prefer_res_640_400);
    hints.add(p_but->handle,T("When this option is ticked Steem will use the 600x400 PC screen resolution in fullscreen if it can"),page_p);

    y+=130;
  }


	screenshots_group.create(XD,page_p,page_l,y,page_w,60,
												NULL,this,BT_STATIC | BT_TEXT | BT_BORDER_OUTDENT |
												BT_TEXT_VTOP,T("Screenshots"),0,BkCol);

  screenshots_fol_label.create(XD,screenshots_group.handle,10,25,0,25,NULL,this,
                    BT_LABEL,T("Folder"),0,BkCol);

  screenshots_fol_but.create(XD,screenshots_group.handle,screenshots_group.w-10,
                    25,0,25,button_notify_proc,this,BT_TEXT,T("Choose"),252,BkCol);
  screenshots_fol_but.x-=screenshots_fol_but.w;
  XMoveWindow(XD,screenshots_fol_but.handle,
              screenshots_fol_but.x,screenshots_fol_but.y);

  screenshots_fol_display.create(XD,screenshots_group.handle,
                    15+screenshots_fol_label.w,25,
                    screenshots_fol_but.x-10-(15+screenshots_fol_label.w),25,NULL,this,
                    BT_STATIC | BT_BORDER_INDENT | BT_TEXT_PATH | BT_TEXT,
                    ScreenShotFol,0,WhiteCol);

  XFlush(XD);
}
//---------------------------------------------------------------------------
void TOptionBox::CreateOSDPage()
{
	int y=10;
  hxc_button *p_but;
  hxc_dropdown *p_dd;

  p_but=new hxc_button(XD,page_p,page_l,y,0,25,button_notify_proc,this,
                          BT_CHECKBOX,T("Floppy disk access light"),12000,BkCol);
  p_but->set_check(osd_show_disk_light);
  y+=35;

  int *p_element[4]={&osd_show_plasma,&osd_show_speed,&osd_show_icons,&osd_show_cpu};
  Str osd_name[4];
  osd_name[0]=T("Logo");
  osd_name[1]=T("Speed bar");
  osd_name[2]=T("State icons");
  osd_name[3]=T("CPU speed indicator");
  for (int i=0;i<4;i++){
    p_but=new hxc_button(XD,page_p,page_l,y,0,25,NULL,NULL,BT_LABEL,osd_name[i],0,BkCol);

    p_dd=new hxc_dropdown(XD,page_p,page_l+p_but->w+5,y,page_w-(p_but->w+5),200,dd_notify_proc,this);
    p_dd->id=12010+i;
    p_dd->additem(T("Off"),0);
    p_dd->additem(Str("2 ")+T("Seconds"),2);
    p_dd->additem(Str("3 ")+T("Seconds"),3);
    p_dd->additem(Str("4 ")+T("Seconds"),4);
    p_dd->additem(Str("5 ")+T("Seconds"),5);
    p_dd->additem(Str("6 ")+T("Seconds"),6);
    p_dd->additem(Str("8 ")+T("Seconds"),8);
    p_dd->additem(Str("10 ")+T("Seconds"),10);
    p_dd->additem(Str("12 ")+T("Seconds"),12);
    p_dd->additem(Str("15 ")+T("Seconds"),15);
    p_dd->additem(Str("20 ")+T("Seconds"),20);
    p_dd->additem(Str("30 ")+T("Seconds"),30);
    p_dd->additem(T("Always Shown"),OSD_SHOW_ALWAYS);
    if (p_dd->select_item_by_data(*(p_element[i]),0)<0) p_dd->changesel(0);
    y+=35;
  }

  p_but=new hxc_button(XD,page_p,page_l,y,0,25,button_notify_proc,this,
                          BT_CHECKBOX,T("Scrolling messages"),12020,BkCol);
  p_but->set_check(osd_show_scrollers);
  y+=35;

  osd_disable_but.create(XD,page_p,page_l,y,0,25,button_notify_proc,this,
                          BT_CHECKBOX,T("Disable on screen display"),12030,BkCol);
  osd_disable_but.set_check(osd_disable);

  XFlush(XD);
}
//---------------------------------------------------------------------------
void TOptionBox::DrawBrightnessBitmap(XImage *Img)
{
  if (Img==NULL) return;

  int w=Img->width,h=Img->height;

  int band_w=w/16;
  int col_h=h/4;
  int BytesPP=(Img->bits_per_pixel+7)/8;
  ZeroMemory(Img->data,w*h*BytesPP);
  if (BytesPP>1){
    BYTE *pMem=LPBYTE(Img->data);
    for (int y=0;y<h;y++){
      for (int i=0;i<w;i++){
        int r=((i/band_w) >> 1)+(((i/band_w) & 1) << 3),g=r,b=r;
        if (y>col_h*3){
          g=0,b=0;
        }else if (y>col_h*2){
          r=0,b=0;
        }else if (y>col_h){
          r=0,g=0;
        }
        long Col=palette_table[r | (g << 4) | (b << 8)];
        switch (BytesPP){
          case 1:
            *pMem=BYTE(Col);
            break;
          case 2:
            *LPWORD(pMem)=WORD(Col);
            break;
          case 3:case 4:
            *LPDWORD(pMem)=DWORD(Col);
            break;
        }
        pMem+=BytesPP;
      }
    }
  }
}
//---------------------------------------------------------------------------
void TOptionBox::FullscreenBrightnessBitmap()
{
  int sw=XDisplayWidth(XD,XDefaultScreen(XD));
  int sh=XDisplayHeight(XD,XDefaultScreen(XD));

  XSetWindowAttributes swa;
  swa.backing_store=NotUseful;
  swa.override_redirect=True;
  Window handle=XCreateWindow(XD,XDefaultRootWindow(XD),0,0,sw,sh,0,
                           CopyFromParent,InputOutput,CopyFromParent,
                           CWBackingStore | CWOverrideRedirect,&swa);

  SetProp(XD,handle,cWinProc,(DWORD)WinProc);
  SetProp(XD,handle,cWinThis,(DWORD)this);
  SetProp(XD,handle,hxc::cModal,(DWORD)0xffffffff);

  XSelectInput(XD,handle,KeyPressMask | KeyReleaseMask |
                            ButtonPressMask | ButtonReleaseMask |
                            FocusChangeMask | LeaveWindowMask);

  brightness_image=brightness_ig.NewIconImage(XD,sw,sh);
	DrawBrightnessBitmap(brightness_image);

  hxc_button *p_but=new hxc_button(XD,handle,0,0,sw,sh,NULL,this,
                      BT_STATIC | BT_ICON | BT_BORDER_NONE | BT_NOBACKGROUND,"",0,BkCol);
  p_but->set_icon(&brightness_ig,0);

  XMapWindow(XD,handle);
  XGrabPointer(XD,handle,False,ButtonPressMask | ButtonReleaseMask,
                GrabModeAsync,GrabModeAsync,None,None,CurrentTime);
  XEvent Ev;
  LOOP{
    if (hxc::wait_for_event(XD,&Ev)){
      if (Ev.xany.window==handle){
        break;
      }else{
        ProcessEvent(&Ev);
      }
    }
  }
  hxc::destroy_children_of(handle);

  hxc::RemoveProp(XD,handle,cWinProc);
  hxc::RemoveProp(XD,handle,cWinThis);
 	hxc::RemoveProp(XD,handle,hxc::cModal);
  hxc::kill_timer(handle,HXC_TIMER_ALL_IDS);
  XDestroyWindow(XD,handle);

	brightness_image=brightness_ig.NewIconImage(XD,136+136,120);
	DrawBrightnessBitmap(brightness_image);
}
//---------------------------------------------------------------------------
void TOptionBox::CreateBrightnessPage()
{
	int y=10;

  make_palette_table(brightness,contrast);
	brightness_image=brightness_ig.NewIconImage(XD,136+136,120);
	DrawBrightnessBitmap(brightness_image);

  brightness_picture.set_icon(&brightness_ig,0);
	brightness_picture.create(XD,page_p,page_l + page_w/2 - 137,y,
														137+137,122,button_notify_proc,this,
                            BT_STATIC | BT_ICON | BT_BORDER_INDENT | BT_NOBACKGROUND,
														"",122,BkCol);
  hints.add(brightness_picture.handle,T("Click to view fullscreen"),page_p);
	y+=125;

  brightness_picture_label.create(XD,page_p,page_l,y,page_w,25,NULL,this,BT_STATIC | BT_TEXT | BT_BORDER_NONE | BT_TEXT_CENTRE,
											  T("There should be 16 vertical strips (one black)"),0,BkCol);
  y+=25;

  brightness_label.create(XD,page_p,page_l,y,page_w,25,NULL,this,BT_STATIC | BT_TEXT | BT_BORDER_NONE | BT_TEXT_CENTRE,"",0,BkCol);
  y+=30;

  brightness_sb.horizontal=true;
  brightness_sb.init(256+10,10,brightness+128);

  brightness_sb.create(XD,page_p,page_l,y,page_w,25,scrollbar_notify_proc,this);
  brightness_sb.id=10;
  y+=35;

  contrast_label.create(XD,page_p,page_l,y,page_w,25,NULL,this,
        BT_STATIC | BT_TEXT | BT_BORDER_NONE | BT_TEXT_CENTRE,"",0,BkCol);
  y+=30;

  contrast_sb.horizontal=true;
  contrast_sb.init(256+10,10,contrast+128);
  contrast_sb.create(XD,page_p,page_l,y,page_w,25,scrollbar_notify_proc,this);
  contrast_sb.id=11;

  y+=35;

  scrollbar_notify_proc(&contrast_sb,SBN_SCROLL,contrast_sb.pos); // update the label text

  XFlush(XD);
}
//---------------------------------------------------------------------------
void TOptionBox::CreateStartupPage()
{
	int y=10;

  auto_sts_but.create(XD,page_p,page_l,y,0,25,
          button_notify_proc,this,BT_CHECKBOX,T("Restore previous state"),100,BkCol);
  auto_sts_but.set_check(AutoLoadSnapShot);
  y+=35;

  auto_sts_filename_label.create(XD,page_p,page_l,y,0,25,NULL,this,
                    BT_LABEL,T("Filename"),0,BkCol);

  auto_sts_filename_edit.create(XD,page_p,page_l+5+auto_sts_filename_label.w,y,
  									page_w-(5+auto_sts_filename_label.w),25,edit_notify_proc,this);
  auto_sts_filename_edit.set_text(AutoSnapShotName);
  auto_sts_filename_edit.id=100;

  y+=40;

  no_shm_but.create(XD,page_p,page_l,y,0,25,
              button_notify_proc,this,BT_CHECKBOX,
              T("Never use shared memory extension"),101,BkCol);
  no_shm_but.set_check(GetCSFInt("Options","NoSHM",0,INIFile));
  y+=35;

  XFlush(XD);
}
//---------------------------------------------------------------------------
void TOptionBox::CreateProfilesPage()
{
	int y=10;
  dir_lv.ext_sl.DeleteAll();
  dir_lv.ext_sl.Add(3,T("Parent Directory"),1,1,0);
  dir_lv.ext_sl.Add(3,"",ICO16_FOLDER,ICO16_FOLDERLINK,0);
  dir_lv.ext_sl.Add(3,"ini",ICO16_PROFILE,ICO16_PROFILELINK,0);
  dir_lv.lpig=&Ico16;
  dir_lv.base_fol=ProfileDir;
  dir_lv.fol=ProfileDir;
  dir_lv.allow_type_change=0;
  dir_lv.show_broken_links=0;
  dir_lv.lv.sel=-1;
	if (ProfileSel.NotEmpty()){
		dir_lv.fol=ProfileSel;
		RemoveFileNameFromPath(dir_lv.fol,REMOVE_SLASH);
	}
  dir_lv.id=2100;
  int dir_lv_h=OPTIONS_HEIGHT-10-10-30-20-30-130;
  dir_lv.create(XD,page_p,page_l,y,page_w,dir_lv_h-5,
                dir_lv_notify_proc,this);
  y+=dir_lv_h;

  hxc_button *p_grp;

  new hxc_button(XD,page_p,page_l,y,page_w/2-5,25,button_notify_proc,this,
  										BT_TEXT,T("Save New Profile"),2101,BkCol);

  new hxc_button(XD,page_p,page_l+page_w/2+5,y,page_w/2-5,25,
                      button_notify_proc,this,BT_TEXT,
                      T("Change Store Folder"),2102,BkCol);
  y+=30;

  p_grp=new hxc_button(XD,page_p,page_l,y,page_w,20+30+130,NULL,this,
                      BT_GROUPBOX,T("Controls"),2109,BkCol);
  y=20;
  Window par=p_grp->handle;
  int par_l=10,par_w=page_w-20;

  new hxc_button(XD,par,par_l,y,par_w/2-5,25,button_notify_proc,this,
  										BT_TEXT,T("Load Profile"),2110,BkCol);

  new hxc_button(XD,par,par_l+par_w/2+5,y,par_w/2-5,25,
                      button_notify_proc,this,BT_TEXT,
                      T("Save Over Profile"),2111,BkCol);
  y+=30;

  profile_sect_lv.lpig=&Ico16;
  profile_sect_lv.display_mode=1;
  profile_sect_lv.checkbox_mode=true;
  profile_sect_lv.id=2112;
  profile_sect_lv.sl.DeleteAll();
  profile_sect_lv.sl.Sort=eslNoSort;
  for (int i=0;ProfileSection[i].Name!=NULL;i++){
    profile_sect_lv.sl.Add(T(ProfileSection[i].Name),0,ProfileSection[i].ID);
  }
  profile_sect_lv.create(XD,par,par_l,y,par_w,125,listview_notify_proc,this);

	if (ProfileSel.NotEmpty()){
		dir_lv.select_item_by_name(GetFileNameFromPath(ProfileSel));
	}
  UpdateProfileDisplay();

  XFlush(XD);
}
//---------------------------------------------------------------------------
void TOptionBox::CreatePathsPage()
{
	int y=10;
  hxc_edit *p_ed;
  hxc_button *p_but;
  
  Str Comline_Desc[NUM_COMLINES];
  Comline_Desc[COMLINE_HTTP]=T("Web");
  Comline_Desc[COMLINE_FTP]=T("FTP");
  Comline_Desc[COMLINE_MAILTO]=T("E-mail");
  Comline_Desc[COMLINE_FM]=T("File Manager");
  Comline_Desc[COMLINE_FIND]=T("Find File(s)");

  for (int i=0;i<NUM_COMLINES;i++){
    p_but=new hxc_button(XD,page_p,page_l,y,0,25,NULL,this,BT_LABEL,Comline_Desc[i],0,BkCol);

    p_ed=new hxc_edit(XD,page_p,page_l+p_but->w+5,y,page_w-p_but->w-5-20,25,edit_notify_proc,this);
    p_ed->set_text(Comlines[i]);
    p_ed->id=15000+i*10;
  
    p_but=new hxc_button(XD,page_p,page_l+page_w-20,y,20,25,button_notify_proc,this,
    										BT_ICON,"",15001+i*10,BkCol);
    p_but->set_icon(NULL,1);
    y+=30;
  }
  
  XFlush(XD);
}
//---------------------------------------------------------------------------

