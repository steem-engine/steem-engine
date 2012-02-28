//#define UNIX_FAKE_JOY
//---------------------------------------------------------------------------
void XOpenJoystick(int j)
{
  XCloseJoystick(j);

  if (JoyInfo[j].On==0) return;

#ifndef UNIX_FAKE_JOY

#ifdef LINUX
  JoyInfo[j].Dev=open(JoyInfo[j].DeviceFile,O_RDONLY | O_NONBLOCK);
  if (JoyInfo[j].Dev==-1) return;

  JoyExists[j]=true;

  int ret;
#ifdef JSIOCGVERSION
  ret=-1;
	ioctl(JoyInfo[j].Dev,JSIOCGVERSION,&ret);
  JoyInfo[j].NoEvent=(ret==-1);
#else
  JoyInfo[j].NoEvent=true;
#endif
  if (JoyInfo[j].NoEvent==0) JoyInfo[j].Range=65535;

  if (JoyInfo[j].NumButtons<0){
    if (JoyInfo[j].NoEvent) JoyInfo[j].Range=10000;
    JoyInitAxisRange(j); // Reset min and max

    if (JoyInfo[j].NoEvent){
      for (int n=0;n<6;n++) JoyInfo[j].AxisExists[n]=(n<2);
      JoyInfo[j].NumButtons=2;
    }else{
#ifdef JSIOCGAXES
      ret=0;
      ioctl(JoyInfo[j].Dev,JSIOCGAXES,&ret);
      for (int n=0;n<6;n++) JoyInfo[j].AxisExists[n]=(n<ret);

      ret=0;
      ioctl(JoyInfo[j].Dev,JSIOCGBUTTONS,&ret);
      JoyInfo[j].NumButtons=ret;
#endif
    }
  }
#else
  JoyInfo[j].Dev=-1;
  return;
#endif

#else
  JoyInfo[j].Dev=-1;
  JoyExists[j]=true;
  JoyInfo[j].NoEvent=0;
  if (JoyInfo[j].NoEvent==0) JoyInfo[j].Range=65535;
  if (JoyInfo[j].NumButtons<0){
    if (JoyInfo[j].NoEvent) JoyInfo[j].Range=10000;
    JoyInitAxisRange(j); // Reset min and max

    if (JoyInfo[j].NoEvent){
      for (int n=0;n<6;n++) JoyInfo[j].AxisExists[n]=(n<2);
      JoyInfo[j].NumButtons=2;
    }else{
      for (int n=0;n<6;n++) JoyInfo[j].AxisExists[n]=true;
      JoyInfo[j].NumButtons=32;
    }
  }
#endif

  // This is in case the joystick was already configured when NoEvent==0
  if (JoyInfo[j].NoEvent){
    for (int n=2;n<6;n++) JoyInfo[j].AxisExists[n]=0;
  }
  JoyPosReset(j); // Make sure changing range hasn't messed anything up
  NumJoysticks++;
}
//---------------------------------------------------------------------------
void XCloseJoystick(int j)
{
  if (JoyExists[j]==0) return;

  if (JoyInfo[j].Dev!=-1) close(JoyInfo[j].Dev);
  JoyInfo[j].Dev=-1;
  JoyExists[j]=0;
  JoyPosReset(j); // Make sure nothing stuck on
  NumJoysticks--;
}
//---------------------------------------------------------------------------
void FreeJoysticks()
{
  for (int j=0;j<MAX_PC_JOYS;j++) XCloseJoystick(j);
}
//---------------------------------------------------------------------------
void InitJoysticks(int Method)
{
  NumJoysticks=0;

  if (Method!=PCJOY_READ_DONT){
    for (int j=0;j<MAX_PC_JOYS;j++){
      XOpenJoystick(j);
      if (JoyExists[j]==0) JoyInfo[j].On=0;
    }
    JoyGetPoses();
  }
}
//---------------------------------------------------------------------------
void JoyGetPoses()
{
  for (int n=0;n<MAX_PC_JOYS;n++){
    if (JoyExists[n]){
#if defined(UNIX_FAKE_JOY)==0

#ifdef LINUX
      if (JoyInfo[n].NoEvent){
      	struct JS_DATA_TYPE js;
    		if (read(JoyInfo[n].Dev,&js,JS_RETURN)==JS_RETURN){
          JoyPos[n].dwButtons=DWORD(js.buttons);

          JoystickInfo &ji=JoyInfo[n];
          for (int a=0;a<2;a++){
            DWORD val=DWORD((a==0) ? js.x:js.y);
            if (val>ji.AxisMax[a]) val=ji.AxisMax[a];
            if (val<ji.AxisMin[a]) val=ji.AxisMin[a];
            int dzlen=((ji.AxisLen[a]/2)*ji.AxisDZ[a])/100;
            if (val>ji.AxisMid[a]-dzlen && val<ji.AxisMid[a]+dzlen){
              val=ji.AxisMid[a];
            }
            if (a==0) JoyPos[n].dwXpos=val;
            if (a==1) JoyPos[n].dwYpos=val;
          }
  	   	}
      }else{
        struct js_event js;
        if (read(JoyInfo[n].Dev,&js,sizeof(struct js_event))==sizeof(struct js_event)){

          switch (js.type & ~JS_EVENT_INIT){

            case JS_EVENT_BUTTON:
              if (js.number>31) break;
              if (js.value){
                JoyPos[n].dwButtons|=1 << js.number;
              }else{
                JoyPos[n].dwButtons&=~(1 << js.number);
              }
              break;
            case JS_EVENT_AXIS:
            {
              if (js.number>5) break;

              JoystickInfo &ji=JoyInfo[n];

              int a=js.number;
              // js.value is a number between -32767 and +32768
              UINT val=js.value + 32767; // make unsigned
              if (val>ji.AxisMax[a]) val=ji.AxisMax[a];
              if (val<ji.AxisMin[a]) val=ji.AxisMin[a];
              int dzlen=((ji.AxisLen[a]/2)*ji.AxisDZ[a])/100;
              if (val>ji.AxisMid[a]-dzlen && val<ji.AxisMid[a]+dzlen){
                val=ji.AxisMid[a];
              }
              switch (js.number){
                case 0: JoyPos[n].dwXpos=val; break;
                case 1: JoyPos[n].dwYpos=val; break;
                case 2: JoyPos[n].dwZpos=val; break;
                case 3: JoyPos[n].dwRpos=val; break;
                case 4: JoyPos[n].dwUpos=val; break;
                case 5: JoyPos[n].dwVpos=val; break;
              }
              break;
            }
          }
        }
      }
#endif

#else
      JoystickInfo &ji=JoyInfo[n];
      for (int a=0;a<6;a++){
        UINT val=rand() % 65535;
        if (val>ji.AxisMax[a]) val=ji.AxisMax[a];
        if (val<ji.AxisMin[a]) val=ji.AxisMin[a];
        int dzlen=((ji.AxisLen[a]/2)*ji.AxisDZ[a])/100;
        if (val>ji.AxisMid[a]-dzlen && val<ji.AxisMid[a]+dzlen){
        	val=ji.AxisMid[a];
        }
        switch (a){
          case 0: JoyPos[n].dwXpos=val; break;
          case 1: JoyPos[n].dwYpos=val; break;
          case 2: JoyPos[n].dwZpos=val; break;
          case 3: JoyPos[n].dwRpos=val; break;
          case 4: JoyPos[n].dwUpos=val; break;
          case 5: JoyPos[n].dwVpos=val; break;
        }
      }
      JoyPos[n].dwButtons=rand();
#endif
    }
  }
}
//---------------------------------------------------------------------------
void JoyInitAxisRange(int j)
{
  for (int a=0;a<6;a++){
    JoyInfo[j].AxisMin[a]=0;
    JoyInfo[j].AxisMax[a]=JoyInfo[j].Range;
    JoyInfo[j].AxisMid[a]=JoyInfo[j].AxisMax[a]/2;
    JoyInfo[j].AxisLen[a]=JoyInfo[j].Range;
  }
}
//---------------------------------------------------------------------------
void JoyInitCalibration(int j)
{
  JoyInitAxisRange(j);
  for (int a=0;a<7;a++){
    JoyInfo[j].AxisDZ[a]=25;
    JoyInfo[j].AxisExists[6]=0;
  }
  JoyInfo[j].NumButtons=-1;
  JoyInfo[j].On=0;
}
//---------------------------------------------------------------------------
TJoystickConfig::TJoystickConfig()
{
  Section="Joysticks";

  for (int j=0;j<MAX_PC_JOYS;j++){
    JoyInfo[j].Range=65535;
    JoyInitCalibration(j);
    JoyInfo[j].DeviceFile=Str("/dev/js")+char('0'+j);
    JoyInfo[j].Dev=-1;
  }

  ConfigST=true;
  PCJoyEdit=0;

  for (int n=0;n<256;n++) KeyboardButtonName[n]="";
  for (int s=0;s<2;s++){
    for (int n=0;n<6;n++) picker[s][n].allow_joy=true;
  }
}
//---------------------------------------------------------------------------
void TJoystickConfig::Show()
{
  if (Handle) return;

  int win_h=405;
  if (StandardShow(520,win_h,T("Joysticks"),ICO16_JOY,ExposureMask,
      (LPWINDOWPROC)WinProc)) return;

  config_group.create(XD,Handle,10,20,500,win_h-20-10-25-10,NULL,this,
                      BT_GROUPBOX,"",0,BkCol);
  int page_w=config_group.w-20;

#ifndef X_NO_PC_JOYSTICKS
  config_dd.make_empty();
  config_dd.additem(T("Configure PC Joysticks"));
  config_dd.additem(T("Configure ST Joysticks"));
  config_dd.changesel(ConfigST);
  config_dd.id=6200;
  config_dd.create(XD,Handle,20,10,page_w/2-5,200,dd_notify_proc,this);
#endif

  setup_dd.make_empty();
  setup_dd.additem(Str("ST ")+T("Joystick Setup")+" #1");
  setup_dd.additem(Str("ST ")+T("Joystick Setup")+" #2");
  setup_dd.additem(Str("ST ")+T("Joystick Setup")+" #3");
  setup_dd.changesel(nJoySetup);
  setup_dd.id=6201;
  setup_dd.create(XD,Handle,20+page_w/2+5,10,page_w/2-5,200,dd_notify_proc,this);

  pc_group.create(XD,config_group.handle,10,25,page_w,config_group.h-30-2,
                    NULL,this,BT_STATIC | BT_TEXT | BT_BORDER_NONE,"",0,BkCol);

  //---------------------------------------------------------------------------

  Window par=pc_group.handle;
  int x=0,y=0,w;
  
  hxc_button *p_lab=new hxc_button(XD,par,page_w,y,0,25,button_notify_proc,this,BT_TEXT | BT_TOGGLE,T("Open"),6002,BkCol);
  p_lab->w+=20;
  p_lab->x-=p_lab->w;
  XMoveResizeWindow(XD,p_lab->handle,p_lab->x,p_lab->y,p_lab->w,p_lab->h);
  w=p_lab->w;

  p_lab=new hxc_button(XD,par,x,y,0,25,
                    NULL,this,BT_LABEL,T("Edit"),0,BkCol);
  x+=p_lab->w+5;

  hxc_dropdown *p_dd=new hxc_dropdown(XD,par,x,y,
                page_w-10-w-x,300,dd_notify_proc,this);
	p_dd->id=6000;
	p_dd->make_empty();
  for (int j=0;j<MAX_PC_JOYS;j++) p_dd->additem(T("PC Joystick")+" "+Str("#")+(j+1));

  y+=30;

  x=0;
  p_lab=new hxc_button(XD,par,x,y,0,25,NULL,this,BT_LABEL,T("Device"),0,BkCol);
  x+=p_lab->w+5;

  int but_w=hxc::get_text_width(XD,T("Choose"))+10;
  p_lab=new hxc_button(XD,par,page_w-but_w,y,but_w,25,
                        button_notify_proc,this,BT_TEXT,T("Choose"),6001,BkCol);

  device_ed.create(XD,par,x,y,p_lab->x-x-5,25,edit_notify_proc,this);
  y+=30;

  x=0;
  p_lab=new hxc_button(XD,par,x,y,0,25,
                    NULL,this,BT_LABEL,T("Axes"),6005,BkCol);
  x+=p_lab->w+5;

  p_dd=new hxc_dropdown(XD,par,x,y,page_w/2-5-x,300,dd_notify_proc,this);
	p_dd->id=6003;
  x+=p_dd->w+10;

  p_lab=new hxc_button(XD,par,x,y,0,25,NULL,this,BT_LABEL,T("Buttons"),6006,BkCol);
  x+=p_lab->w+5;

  p_dd=new hxc_dropdown(XD,par,x,y,page_w-x,300,dd_notify_proc,this);
	p_dd->id=6004;
	p_dd->make_empty();
  for (int b=0;b<=32;b++) p_dd->additem(Str(b));
  y+=35;

  int bar_y=y;
  int bar_w=332;
  for (int a=0;a<6;a++){
    x=0;
    p_lab=new hxc_button(XD,par,x,bar_y,0,25,NULL,this,
                BT_LABEL,Str(AxisToName[a])+" -",6107+a*10,BkCol);
    x+=p_lab->w+2;

    p_lab=new hxc_button(XD,par,bar_w-5-hxc::get_text_width(XD,"+"),bar_y,0,25,
                NULL,this,BT_LABEL,"+",6108+a*10,BkCol);

    hxc_button *p_grp=new hxc_button(XD,par,x,bar_y,bar_w-x-5-p_lab->w-2,25,NULL,this,
                BT_STATIC | BT_BORDER_INDENT,"",6101+a*10,hxc::col_border_dark);

    p_lab=new hxc_button(XD,par,0,bar_y-5,5,5,NULL,this,
                BT_ICON | BT_STATIC,"",6100+a*10,BkCol);
    p_lab->set_icon(NULL,1);

    p_lab=new hxc_button(XD,p_grp->handle,0,1,5,23,NULL,this,
                  BT_STATIC | BT_TEXT | BT_BORDER_NONE,
                  "Deadzone",6102+a*10+4,hxc::col_sel_back);
    p_lab->col_text=hxc::col_sel_fore;
    for (int i=0;i<4;i++){
      p_lab=new hxc_button(XD,p_grp->handle,0,1,5,23,button_notify_proc,this,
                  0,"",6102+a*10+i,BkCol);
      p_lab->want_drag_notify=true;
    }
    bar_y+=35;
  }

  x=bar_w+10;
  int but_y=y;
  for (int b=0;b<32;b++){
    new hxc_button(XD,par,x,but_y,25,25,NULL,this,
                  BT_STATIC | BT_TEXT,EasyStr(b+1),6300+b,BkCol);
    x+=28;
    if (x+25>page_w){
      x=bar_w+10;
      but_y+=28;
    }
  }

  y=pc_group.h-30;
  x=0;
  p_lab=new hxc_button(XD,par,x,y,0,25,NULL,this,BT_LABEL,T("Axis range"),6008,BkCol);
  x+=p_lab->w+5;

  p_dd=new hxc_dropdown(XD,par,x,y,bar_w-5-x,300,dd_notify_proc,this);
	p_dd->id=6007;
	p_dd->make_empty();
  p_dd->additem("0-100",100);
  p_dd->additem("0-256",256);
  p_dd->additem("0-512",512);
  for (int n=1000;n<65000;n+=1000){
    p_dd->additem(Str("0-")+n,n);
  }
  p_dd->additem("0-65535",65535);
  p_dd->w=p_dd->get_min_width()+5;
  XMoveResizeWindow(XD,p_dd->handle,p_dd->x,p_dd->y,p_dd->w,p_dd->h);
  x+=p_dd->w+10;

  //---------------------------------------------------------------------------

  st_group.create(XD,config_group.handle,10,25,page_w,config_group.h-30-2,NULL,this,
                    BT_STATIC | BT_BORDER_NONE,"",0,BkCol);

  par=st_group.handle;
  int st_y=0;

  p_dd=new hxc_dropdown(XD,par,0,st_y,st_group.w,300,dd_notify_proc,this);
	p_dd->id=6202;
	p_dd->make_empty();
  p_dd->additem(T("Standard Ports"));
  p_dd->additem(T("STE Port A"));
  p_dd->additem(T("STE Port B"));
  p_dd->additem(T("Parallel Ports"));
  p_dd->sel=BasePort/2;

  hxc_button *p_but=new hxc_button(XD,par,page_w,st_y,0,25,button_notify_proc,this,
                          BT_CHECKBOX,T("JagPad"),6203,BkCol);
  p_but->x-=p_but->w;
  XMoveWindow(XD,p_but->handle,p_but->x,p_but->y);
  st_y+=35;

	int GroupWid=page_w/2-5;
  int xx1=0;
  for (int s=0;s<2;s++){
  	joy_group[s].create(XD,st_group.handle,xx1,st_y,GroupWid,270,NULL,this,BT_GROUPBOX,"",0,BkCol);

    dir_par[s].create(XD,st_group.handle,xx1+10,st_y+20,GroupWid-20,30+25+40+30,NULL,this,
                        BT_BORDER_NONE | BT_STATIC,"",0,BkCol);
    par=dir_par[s].handle;
    int par_w=dir_par[s].w;
    y=0;

    enable_but[s].create(XD,par,0,y,0,25,
            button_notify_proc,this,BT_CHECKBOX,T("Active"),5000+s,BkCol);
    y+=30;

    picker[s][0].create(XD,par,par_w/2-45,y,90,25,
        picker_notify_proc,this,s*64);
    y+=25;

    picker[s][2].create(XD,par,0,y+40/2-12,90,25,
        picker_notify_proc,this,s*64+2);

		centre_icon[s].create(XD,par,par_w/2-16,y+40/2-16,32,32,
            NULL,this,BT_ICON | BT_STATIC,"",-7,BkCol);
		centre_icon[s].set_icon(&Ico32,ICO32_JOYDIR);

    picker[s][3].create(XD,par,par_w-90,y+40/2-12,90,25,
        picker_notify_proc,this,s*64+3);
    y+=40;

    picker[s][1].create(XD,par,par_w/2-45,y,90,25,
        picker_notify_proc,this,s*64+1);
    y+=30;

    fire_par[s].create(XD,st_group.handle,xx1+10,dir_par[s].y+dir_par[s].h,
                        GroupWid-20,30+30+30,NULL,this,
                        BT_BORDER_NONE | BT_STATIC,"",0,BkCol);
    par=fire_par[s].handle;
    par_w=fire_par[s].w;
    y=0;

    fire_but_label[s].create(XD,par,0,y,0,25,NULL,this,
    										BT_STATIC | BT_TEXT,T("Fire"),0,BkCol);

    picker[s][4].create(XD,par,fire_but_label[s].w+5,y,90,25,
                          picker_notify_proc,this,s*64+4);
    picker[s][4].allow_joy=true;
    y+=30;

    autofire_but_label[s][0].create(XD,par,0,y,0,25,NULL,this,
    										BT_STATIC | BT_TEXT,T("Autofire speed"),0,BkCol);

	  autofire_dd[s].make_empty();
	  autofire_dd[s].additem(T("Off"));
	  autofire_dd[s].additem(T("V.Fast"));
	  autofire_dd[s].additem(T("Fast"));
	  autofire_dd[s].additem(T("Medium"));
	  autofire_dd[s].additem(T("Slow"));
	  autofire_dd[s].additem(T("V.Slow"));
	  autofire_dd[s].create(XD,par,autofire_but_label[s][0].w+5,y,
	  				max(autofire_dd[s].get_min_width()+5,90),200,dd_notify_proc,this);
		autofire_dd[s].id=s;
    y+=30;

    autofire_but_label[s][1].create(XD,par,0,y,0,25,NULL,this,
    										BT_STATIC | BT_TEXT,T("Autofire button"),0,BkCol);

    picker[s][5].create(XD,par,autofire_but_label[s][1].w+5,y,90,25,
						        picker_notify_proc,this,s*64+5);

    hxc_buttonpicker *p_bp;
    if (s==0){
      jagpad_par[s].create(XD,st_group.handle,xx1+10,dir_par[s].y+dir_par[s].h,
                              GroupWid-20,30+30+30,NULL,this,
                              BT_BORDER_NONE | BT_STATIC,"",0,BkCol);
      par=jagpad_par[s].handle;
      par_w=jagpad_par[s].w;
      y=5;

      for (int i=0;i<3;i++){
        p_lab=new hxc_button(XD,par,0,y,0,25,NULL,this,
                        BT_LABEL,T("Button")+" "+char('A'+i),0,BkCol);
        p_bp=new hxc_buttonpicker(XD,par,5+p_lab->w,y,90,25,picker_notify_proc,this,6+i);
        p_bp->allow_joy=true;
        y+=30;
      }
    }else{
      jagpad_par[s].create(XD,st_group.handle,xx1+10,dir_par[s].y,GroupWid-20,
                            30*7+30,NULL,this,
                            BT_BORDER_NONE | BT_STATIC,"",0,BkCol);
      par=jagpad_par[s].handle;
      par_w=jagpad_par[s].w;
      y=0;

      EasyStr Option=T("Option"),Pause=T("Pause");
      char *JagButName[14]={Option,Pause,"0","1","2","3","4","5","6","7","8","9","#","*"};
      int x=0;
      for (int n=0;n<14;n++){
        p_lab=new hxc_button(XD,par,x,y,0,25,NULL,this,
                        BT_LABEL,JagButName[n],0,BkCol);
        p_bp=new hxc_buttonpicker(XD,par,x+2+p_lab->w,y,90,25,picker_notify_proc,this,9+n);
        p_bp->allow_joy=true;

        x+=par_w/2;
        if ((n & 1) || n<2){
          x=0;
          y+=30;
        }
      }
    }
    xx1+=GroupWid+10;
  }

  ShowAndUpdatePage();

  y=config_group.y+config_group.h+10;
  MouseSpeedLabel[0].create(XD,Handle,10,y,0,25,NULL,this,
    										BT_LABEL,T("Mouse speed")+": "+T("Min"),0,BkCol);

  int Wid=hxc::get_text_width(XD,T("Max"));
  MouseSpeedLabel[1].create(XD,Handle,config_group.x+config_group.w-Wid,y,Wid,25,NULL,this,
    										BT_LABEL,T("Max"),0,BkCol);

  MouseSpeedSB.horizontal=true;
  MouseSpeedSB.init(19+4,4,mouse_speed-1);
  MouseSpeedSB.create(XD,Handle,15+MouseSpeedLabel[0].w,y,
                      config_group.x+config_group.w-(15+MouseSpeedLabel[0].w)-(Wid+5),
                      25,scrollbar_notify_proc,this);

  XMapWindow(XD,Handle);

  if (StemWin) JoyBut.set_check(true);
}
//---------------------------------------------------------------------------
void TJoystickConfig::ShowAndUpdatePage()
{
  if (ConfigST==0){
    hxc_dropdown *p_dd=(hxc_dropdown*)hxc::find(pc_group.handle,6000);
    p_dd->changesel(PCJoyEdit);
    p_dd->draw();

    device_ed.set_text(JoyInfo[PCJoyEdit].DeviceFile);
    device_ed.draw();

    hxc_button *p_but=(hxc_button*)hxc::find(pc_group.handle,6002);
    p_but->set_check(JoyInfo[PCJoyEdit].On);

    int n=0;
    for (int a=0;a<6;a++){
      if (JoyInfo[PCJoyEdit].AxisExists[a]){
        n++;
      }else{
        break;
      }
    }
    p_dd=(hxc_dropdown*)hxc::find(pc_group.handle,6003);
    int max_axes=6;
    if (JoyInfo[PCJoyEdit].NoEvent) max_axes=2;
    if (p_dd->sl.NumStrings!=max_axes+1){
      p_dd->sl.DeleteAll();
      for (int i=0;i<=max_axes;i++) p_dd->sl.Add(Str(i));
    }
    p_dd->changesel(n);
    p_dd->draw();
    ShowHideWindow(XD,p_dd->handle,JoyExists[PCJoyEdit]);
    ShowHideWindow(XD,((hxc_button*)(hxc::find(pc_group.handle,6005)))->handle,JoyExists[PCJoyEdit]);

    p_dd=(hxc_dropdown*)hxc::find(pc_group.handle,6004);
    p_dd->changesel(JoyInfo[PCJoyEdit].NumButtons);
    p_dd->draw();
    ShowHideWindow(XD,p_dd->handle,JoyExists[PCJoyEdit]);
    ShowHideWindow(XD,((hxc_button*)(hxc::find(pc_group.handle,6006)))->handle,JoyExists[PCJoyEdit]);

    p_dd=(hxc_dropdown*)hxc::find(pc_group.handle,6007);
    p_dd->select_item_by_data(JoyInfo[PCJoyEdit].Range);
    p_dd->draw();
    ShowHideWindow(XD,p_dd->handle,JoyExists[PCJoyEdit] && JoyInfo[PCJoyEdit].NoEvent);
    ShowHideWindow(XD,((hxc_button*)(hxc::find(pc_group.handle,6008)))->handle,
                            JoyExists[PCJoyEdit] && JoyInfo[PCJoyEdit].NoEvent);

    UpdateJoyPos();

    for (int a=0;a<6;a++){
      hxc_button *pointer=(hxc_button*)(hxc::find(pc_group.handle,6100+a*10));
      hxc_button *base=(hxc_button*)(hxc::find(pc_group.handle,6101+a*10));
      hxc_button *lab1=(hxc_button*)(hxc::find(pc_group.handle,6107+a*10));
      hxc_button *lab2=(hxc_button*)(hxc::find(pc_group.handle,6108+a*10));
      ShowHideWindow(XD,pointer->handle,JoyInfo[PCJoyEdit].AxisExists[a] && JoyExists[PCJoyEdit]);
      ShowHideWindow(XD,base->handle,JoyInfo[PCJoyEdit].AxisExists[a] && JoyExists[PCJoyEdit]);
      ShowHideWindow(XD,lab1->handle,JoyInfo[PCJoyEdit].AxisExists[a] && JoyExists[PCJoyEdit]);
      ShowHideWindow(XD,lab2->handle,JoyInfo[PCJoyEdit].AxisExists[a] && JoyExists[PCJoyEdit]);
    }
    for (int b=0;b<32;b++){
      hxc_button *p_but=(hxc_button*)(hxc::find(pc_group.handle,6300+b));
      ShowHideWindow(XD,p_but->handle,b<JoyInfo[PCJoyEdit].NumButtons && JoyExists[PCJoyEdit]);
    }
    if (JoyExists[PCJoyEdit]==0){
      pc_group.text=T("Device not opened");
    }else{
      pc_group.text="";
    }

    XUnmapWindow(XD,setup_dd.handle);
    XMapWindow(XD,pc_group.handle);
    XUnmapWindow(XD,st_group.handle);
    pc_group.draw();

    hxc::set_timer(Handle,0,50,timerproc,this);
  }else{
    hxc::kill_timer(Handle,0);
    
    Str PortName[8]={T("Port 0 (mouse)"),T("Port 1"),
                         T("Stick 0"),T("Stick 1"),
                         T("Stick 0"),T("Stick 1"),
                         T("Parallel 0"),T("Parallel 1")};

    hxc_dropdown *p_dd=(hxc_dropdown*)hxc::find(st_group.handle,6202);
    hxc_button *p_but=(hxc_button*)hxc::find(st_group.handle,6203);
    if (BasePort==2 || BasePort==4){
      XMoveResizeWindow(XD,p_dd->handle,p_dd->x,p_dd->y,p_but->x-p_dd->x-10,p_dd->h);
    }else{
      XMoveResizeWindow(XD,p_dd->handle,p_dd->x,p_dd->y,st_group.w,p_dd->h);
    }

    if (Joy[BasePort].Type==JOY_TYPE_JAGPAD){
      PortName[BasePort]=T("Pad");
      PortName[BasePort+1]=T("Pad Keyboard");
      p_but->set_check(true);
    }else{
      p_but->set_check(0);
    }
    ShowHideWindow(XD,p_but->handle,(BasePort==2 || BasePort==4));
    for (int s=0;s<2;s++){
      int j=BasePort+s;
      joy_group[s].set_text(PortName[j]);
      joy_group[s].draw();

      if (Joy[BasePort].Type==JOY_TYPE_JAGPAD){
        ShowHideWindow(XD,jagpad_par[s].handle,true);
        ShowHideWindow(XD,fire_par[s].handle,0);
        ShowHideWindow(XD,dir_par[s].handle,(s==0));
        j=BasePort;
      }else{
        ShowHideWindow(XD,fire_par[s].handle,true);
        ShowHideWindow(XD,dir_par[s].handle,true);
        ShowHideWindow(XD,jagpad_par[s].handle,0);
      }

      enable_but[s].set_check(bool(Joy[j].ToggleKey));
      for (int i=0;i<6;i++){
        picker[s][i].DirID=Joy[j].DirID[i];
        picker[s][i].draw();
      }
      autofire_dd[s].sel=Joy[j].AutoFireSpeed;
      autofire_dd[s].draw();
      
      if (s==0){
        for (int i=0;i<3;i++){
          hxc_buttonpicker *bp=(hxc_buttonpicker*)hxc::find(jagpad_par[s].handle,6+i);
          bp->DirID=Joy[j].JagDirID[i];
        }
      }else{
        for (int i=0;i<14;i++){
          hxc_buttonpicker *bp=(hxc_buttonpicker*)hxc::find(jagpad_par[s].handle,9+i);
          bp->DirID=Joy[j].JagDirID[3+i];
        }
      }
    }

    XMapWindow(XD,setup_dd.handle);
    XMapWindow(XD,st_group.handle);
    XUnmapWindow(XD,pc_group.handle);
  }
}
//---------------------------------------------------------------------------
void TJoystickConfig::UpdateJoyPos()
{
  if (Handle==0 || JoyExists[PCJoyEdit]==0) return;

  hxc_button *pointer,*base,*amin,*amax,*dzmin,*dzmax,*dzfill;
  for (int a=0;a<6;a++){
    if (JoyInfo[PCJoyEdit].AxisExists[a]){
      pointer=(hxc_button*)(hxc::find(pc_group.handle,6100+a*10));
      base=(hxc_button*)(hxc::find(pc_group.handle,6101+a*10));
      amin=(hxc_button*)(hxc::find(base->handle,6102+a*10));
      amax=(hxc_button*)(hxc::find(base->handle,6103+a*10));
      dzmin=(hxc_button*)(hxc::find(base->handle,6104+a*10));
      dzmax=(hxc_button*)(hxc::find(base->handle,6105+a*10));
      dzfill=(hxc_button*)(hxc::find(base->handle,6106+a*10));

      int w=base->w-amin->w-2;
      double conv=double(JoyInfo[PCJoyEdit].Range) / w;

      int pos=int(double(GetAxisPosition(a,&(JoyPos[PCJoyEdit])))/conv);
      pos=min(pos,w);
      if (base->x+1+amin->w+pos-2!=pointer->x){
        XMoveWindow(XD,pointer->handle,base->x+1+amin->w+pos-2,pointer->y);
      }

      pos=int(double(JoyInfo[PCJoyEdit].AxisMin[a])/conv)+1;
      if (pos!=amin->x) XMoveWindow(XD,amin->handle,pos,amin->y);

      pos=int(double(JoyInfo[PCJoyEdit].AxisMax[a])/conv)+1;
      if (pos!=amax->x) XMoveWindow(XD,amax->handle,pos,amax->y);

      pos=int(double(JoyInfo[PCJoyEdit].AxisMid[a]-
            ((JoyInfo[PCJoyEdit].AxisLen[a]/2)*JoyInfo[PCJoyEdit].AxisDZ[a])/100)/conv)+1;
      if (pos!=dzmin->x) XMoveWindow(XD,dzmin->handle,pos,dzmin->y);

      int pos2=int(double(JoyInfo[PCJoyEdit].AxisMid[a]+
            ((JoyInfo[PCJoyEdit].AxisLen[a]/2)*JoyInfo[PCJoyEdit].AxisDZ[a])/100)/conv)+1;
      if (pos2!=dzmax->x) XMoveWindow(XD,dzmax->handle,pos2,dzmax->y);

      if (pos2-pos-5!=dzfill->w || pos+5!=dzfill->x){
        if (pos2-pos-5>0){
          XMapWindow(XD,dzfill->handle);
          XMoveResizeWindow(XD,dzfill->handle,pos+5,dzfill->y,pos2-pos-5,dzfill->h);
        }else{
          XUnmapWindow(XD,dzfill->handle);
        }
      }
    }
    for (int b=0;b<JoyInfo[PCJoyEdit].NumButtons;b++){
      pointer=(hxc_button*)(hxc::find(pc_group.handle,6300+b));
      int old_type=pointer->type;
      pointer->type&=~BT_BORDER_MASK;
      pointer->type|=int(bool(JoyPos[PCJoyEdit].dwButtons & (1 << b)) ? BT_BORDER_INDENT:0);
      if (pointer->type!=old_type) pointer->draw();
    }
  }
}
//---------------------------------------------------------------------------
bool TJoystickConfig::AttemptOpenJoy(int j)
{
  XOpenJoystick(j);
  return JoyExists[PCJoyEdit];
}
//---------------------------------------------------------------------------
void TJoystickConfig::Hide()
{
  if (XD==NULL || Handle==0) return;

  hxc::kill_timer(Handle,HXC_TIMER_ALL_IDS);
  StandardHide();

  if (StemWin) JoyBut.set_check(0);
}
//---------------------------------------------------------------------------
int TJoystickConfig::button_notify_proc(hxc_button*b,int mess,int*ip)
{
  TJoystickConfig *This=(TJoystickConfig*)(b->owner);
	if (mess==BN_CLICKED){
    if((b->id)>=5000 && (b->id)<=5001){
      Joy[(b->id)-5000].ToggleKey=b->checked;
    }else if (b->id==6001){ // Choose
      b->set_check(true);
      fileselect.set_corner_icon(&Ico16,ICO16_JOY);
      Str CurFol=JoyInfo[This->PCJoyEdit].DeviceFile;
      RemoveFileNameFromPath(CurFol,REMOVE_SLASH);
      EasyStr fn=fileselect.choose(XD,CurFol,
                    GetFileNameFromPath(JoyInfo[This->PCJoyEdit].DeviceFile),
                    T("Choose Device"),FSM_OK | FSM_LOADMUSTEXIST,NULL,"");
      if (fileselect.chose_option==FSM_OK){
        This->device_ed.set_text(fn);
        edit_notify_proc(&(This->device_ed),EDN_CHANGE,0);
      }
      b->set_check(0);
    }else if (b->id==6002){ // Open
      XCloseJoystick(This->PCJoyEdit);
      JoyInfo[This->PCJoyEdit].On=b->checked;
      bool OpenFailed=0;
      if (b->checked){
        if (This->AttemptOpenJoy(This->PCJoyEdit)==0){
          JoyInfo[This->PCJoyEdit].On=0;
          b->set_check(0);
          OpenFailed=true;
        }
      }
      This->ShowAndUpdatePage();
      if (OpenFailed) This->pc_group.set_text(T("ERROR: Can't open joystick device."));
    }else if (b->id==6203){
      Joy[BasePort].Type=int((b->checked) ? JOY_TYPE_JAGPAD:JOY_TYPE_JOY);
      This->ShowAndUpdatePage();
    }
  }else if (mess==BN_MOTION){
    XEvent *Ev=(XEvent*)ip;
    int mv=Ev->xmotion.x-3;
    int a=(b->id-6100)/10;

    hxc_button *base=(hxc_button*)(hxc::find(This->pc_group.handle,6101+a*10));
    hxc_button *amin=(hxc_button*)(hxc::find(base->handle,6102+a*10));
    int w=base->w - amin->w - 2;
    UINT r=JoyInfo[This->PCJoyEdit].Range;
    UINT min_l=r/8;
    double conv=double(r) / w;
    int val=int((b->x+mv)*conv);
    switch (b->id % 10){
      case 2:
      {
        int max_val=JoyInfo[This->PCJoyEdit].AxisMax[a]-min_l;
        int min_val=0;
        if (val>max_val) val=max_val;
        if (val<min_val) val=min_val;
        JoyInfo[This->PCJoyEdit].AxisMin[a]=val;
        break;
      }
      case 3:
      {
        int max_val=JoyInfo[This->PCJoyEdit].AxisMin[a]+min_l;
        int min_val=r;
        if (val>max_val) val=max_val;
        if (val<min_val) val=min_val;
        JoyInfo[This->PCJoyEdit].AxisMax[a]=val;
        break;
      }
      case 4:
      case 5:
      {
        int midpos=1+int(double(JoyInfo[This->PCJoyEdit].AxisMid[a])/conv);
        int pos=abs(midpos-(b->x+mv));
        pos*=100;
        pos/=w/2;
        JoyInfo[This->PCJoyEdit].AxisDZ[a]=max(min(pos,90),10);
        break;
      }
    }
    JoyInfo[This->PCJoyEdit].AxisLen[a]=(JoyInfo[This->PCJoyEdit].AxisMax[a]-JoyInfo[This->PCJoyEdit].AxisMin[a]);
    JoyInfo[This->PCJoyEdit].AxisMid[a]=JoyInfo[This->PCJoyEdit].AxisMin[a]+JoyInfo[This->PCJoyEdit].AxisLen[a]/2;
    This->UpdateJoyPos();
  }
  return 0;
}

int TJoystickConfig::dd_notify_proc(hxc_dropdown *dd,int mess,int i)
{
  TJoystickConfig *This=(TJoystickConfig*)(dd->owner);
  if (mess==DDN_SELCHANGE){
    if (dd->id<2){
      Joy[This->BasePort+dd->id].AutoFireSpeed=dd->sel;
      CreateJoyAnyButtonMasks();
    }else if (dd->id==6000){
      This->PCJoyEdit=dd->sel;
      This->ShowAndUpdatePage();
    }else if (dd->id==6003){
      for (int a=0;a<6;a++) JoyInfo[This->PCJoyEdit].AxisExists[a]=(a < dd->sel);
      This->ShowAndUpdatePage();
    }else if (dd->id==6004){
      JoyInfo[This->PCJoyEdit].NumButtons=dd->sel;
      This->ShowAndUpdatePage();
    }else if (dd->id==6007){
      JoyInfo[This->PCJoyEdit].Range=dd->sl[dd->sel].Data[0];
      JoyInitAxisRange(This->PCJoyEdit);
      JoyGetPoses();
      This->ShowAndUpdatePage();
    }else if (dd->id==6200){
      This->ConfigST=bool(dd->sel);
      This->ShowAndUpdatePage();
    }else if (dd->id==6201){ // ST Config
      for (int n=0;n<8;n++) JoySetup[nJoySetup][n]=Joy[n];
      nJoySetup=dd->sel;
      for (int n=0;n<8;n++) Joy[n]=JoySetup[nJoySetup][n];
      This->ShowAndUpdatePage();
    }else if (dd->id==6202){ // Change BasePort
      BasePort=dd->sel*2;
      This->ShowAndUpdatePage();
    }
  }
  return 0;
}

int TJoystickConfig::picker_notify_proc(hxc_buttonpicker *bp,int mess,int i)
{
  TJoystickConfig *This=(TJoystickConfig*)(bp->owner);
  if (mess==BPN_CHANGE){
    int jn=(bp->id) / 64 + This->BasePort;
    int bn=(bp->id) & 63;
    if (bn<6){
      Joy[jn].DirID[bn]=i;
    }else{ // JagPad
      Joy[jn].JagDirID[bn-6]=i;
    }
    CreateJoyAnyButtonMasks();
  }
  return 0;
}

int TJoystickConfig::edit_notify_proc(hxc_edit *ed,int mess,int i)
{
  TJoystickConfig *This=(TJoystickConfig*)(ed->owner);
  if (mess==EDN_CHANGE){
    XCloseJoystick(This->PCJoyEdit);
    JoyInfo[This->PCJoyEdit].DeviceFile=ed->text;
    JoyInfo[This->PCJoyEdit].On=0;
    JoyInitCalibration(This->PCJoyEdit);
    This->ShowAndUpdatePage();
  }
  return 0;
}

int TJoystickConfig::scrollbar_notify_proc(hxc_scrollbar *SB,int Mess,int I)
{
	if (Mess==SBN_SCROLL){
    SB->pos=I;
		mouse_speed=SB->pos+1;
		SB->draw();
	}else if (Mess==SBN_SCROLLBYONE){
		mouse_speed=max(min(mouse_speed+I,19),1);
		SB->pos=mouse_speed-1;
		SB->draw();
	}
	return 0;
}
//---------------------------------------------------------------------------
int TJoystickConfig::WinProc(TJoystickConfig *This,Window Win,XEvent *Ev)
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
int TJoystickConfig::timerproc(void *t,Window,int)
{
  TJoystickConfig *This=(TJoystickConfig*)t;
  This->UpdateJoyPos();
  return HXC_TIMER_REPEAT;
}
//---------------------------------------------------------------------------

