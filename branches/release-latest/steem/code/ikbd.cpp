#define LOGSECTION LOGSECTION_IKBD
//---------------------------------------------------------------------------
void ikbd_run_start(bool Cold)
{
  if (Cold){
    disable_mouse_until=timeGetTime()+2000;
    keyboard_buffer_length=0;
    keyboard_buffer[0]=0;
//    ZeroMemory(ST_Key_Down,sizeof(ST_Key_Down));
  }else{
    disable_mouse_until=0;
    UpdateSTKeys();
  }
  mouse_move_since_last_interrupt_x=0;
  mouse_move_since_last_interrupt_y=0;
  mouse_change_since_last_interrupt=false;
  JoyGetPoses();
}
//---------------------------------------------------------------------------
bool ikbd_keys_disabled()
{
  return ikbd.joy_mode>=100;
}
//---------------------------------------------------------------------------
void IKBD_VBL()
{
  if ((++ikbd.clock_vbl_count)>=shifter_freq_at_start_of_vbl){
    ikbd.clock_vbl_count=0;

    for (int n=5;n>=0;n--){
      int val=(ikbd.clock[n] >> 4)*10 + (ikbd.clock[n] & 0xf);
      int max_val=ikbd_clock_max_val[n];
      if (max_val==0){
        int mon=min((ikbd.clock[1] >> 4)*10 + (ikbd.clock[1] & 0xf),12);
        max_val=ikbd_clock_days_in_mon[mon];
      }
      bool increase_next=0;
      if ((++val)>max_val){
        val=0;if (n==1) val=1;
        increase_next=true;
      }
      if (n==0) val%=100;
      ikbd.clock[n]=BYTE((val % 10) | ((val/10) << 4));
      if (increase_next==0) break;
    }
  }
  if (macro_start_after_ikbd_read_count) return;

  // ikbd_poll_scanline determines where on the screen the IKBD has polled the keyboard,
  // joystick and mouse. Steem always polls at the VBL but that can cause problems for
  // some programs, so we delay sending the notifications.
  int max_line=scanlines_above_screen[shifter_freq_idx]+(MONO ? 400:200);
  ikbd_joy_poll_line+=527;
  ikbd_joy_poll_line%=max_line;
  ikbd_key_poll_line+=793;
  ikbd_key_poll_line%=max_line;
  ikbd_mouse_poll_line+=379;
  ikbd_mouse_poll_line%=max_line;

  if (macro_play_has_keys) macro_play_keys();

  static BYTE old_stick[2];
  old_stick[0]=stick[0];
  old_stick[1]=stick[1];
  bool old_joypar1_bit4=(stick[N_JOY_PARALLEL_1] & BIT_4)!=0;

  if (macro_play_has_joys){
    macro_play_joy();
  }else{
    joy_read_buttons();
    for (int Port=0;Port<8;Port++) stick[Port]=joy_get_pos(Port);
    if (IsJoyActive(N_JOY_PARALLEL_0)) stick[N_JOY_PARALLEL_0]|=BIT_4;
    if (IsJoyActive(N_JOY_PARALLEL_1)) stick[N_JOY_PARALLEL_1]|=BIT_4;
  }

  if (disable_mouse_until==0 || timer>disable_mouse_until){
    switch (ikbd.joy_mode){
      case IKBD_JOY_MODE_DURATION:
        keyboard_buffer_write( BYTE(int((stick[0] & MSB_B) ? BIT_1:0) | int((stick[1] & MSB_B) ? BIT_0:0)) );
        keyboard_buffer_write( BYTE(((stick[0] & 0xf) << 4) | (stick[1] & 0xf)) );
        break;
      case IKBD_JOY_MODE_AUTO_NOTIFY:
      {
        int j=int(ikbd.port_0_joy ? 0:1);
        for (;j<2;j++){
          BYTE os=old_stick[j],s=stick[j];
          // If mouse active then joystick button never down
          if (ikbd.port_0_joy==0) os&=0x0f, s&=0x0f;
          if (os!=s) agenda_add(ikbd_send_joystick_message,ikbd_joy_poll_line,j);
        }
        break;
      }
      case IKBD_JOY_MODE_FIRE_BUTTON_DURATION:
        if (stick[1] & MSB_B){
          keyboard_buffer_write_string(0xff,0xff,0xff,0xff,0xff,0xff,-1);
        }else{
          keyboard_buffer_write_string(0,0,0,0,0,0,-1);
        }
        break;
      case IKBD_JOY_MODE_CURSOR_KEYS:
      {
        if (stick[0] & (~old_stick[0]) & 0xc){ //new press left/right
          ikbd.cursor_key_joy_ticks[0]=timeGetTime(); //reset timer left/right
          ikbd.cursor_key_joy_ticks[2]=timeGetTime(); //last report
          if (stick[0] & 4){
            keyboard_buffer_write(0x4b);
            keyboard_buffer_write(0x4b | MSB_B);
          }else{
            keyboard_buffer_write(0x4d);
            keyboard_buffer_write(0x4d | MSB_B);
          }
        }else if (stick[0] & (~old_stick[0]) & 0x3){
          ikbd.cursor_key_joy_ticks[1]=timeGetTime(); //reset timer up/down
          ikbd.cursor_key_joy_ticks[3]=timeGetTime(); //last report
          if (stick[0] & 1){
            keyboard_buffer_write(0x48);
            keyboard_buffer_write(0x48 | MSB_B);
          }else{
            keyboard_buffer_write(0x50);
            keyboard_buffer_write(0x50 | MSB_B);
          }
        }else if (stick[0]){
          for (int xy=0;xy<2;xy++){
            BYTE s=stick[0] & BYTE(0xc >> (xy*2));
            if (s){ //one of these directions pressed
              DWORD interval=(timeGetTime()-ikbd.cursor_key_joy_ticks[2+xy])/100;
              DWORD elapsed=(timeGetTime()-ikbd.cursor_key_joy_ticks[xy])/100;
              bool report=false;
              BYTE key;
              if (elapsed>ikbd.cursor_key_joy_time[xy]){ //>Rx
                if (interval>ikbd.cursor_key_joy_time[2+xy]){ //Tx
                  report=true;
                }
              }else if (interval>ikbd.cursor_key_joy_time[4+xy]){ //Vx
                report=true;
              }
              if (report){
                if (s & 8) key=0x4d;
                else if (s & 4) key=0x4b;
                else if (s & 2) key=0x50;
                else key=0x48;
                keyboard_buffer_write(key);
                keyboard_buffer_write(key | MSB_B);
                ikbd.cursor_key_joy_ticks[2+xy]=timeGetTime();
              }
            }
          }
        }
        break;
      }
    }
  }

  if (macro_record){
    macro_jagpad[0]=GetJagPadDown(N_JOY_STE_A_0,0xffffffff);
    macro_jagpad[1]=GetJagPadDown(N_JOY_STE_B_0,0xffffffff);
    macro_record_joy();
  }

  // Handle io line for parallel port joystick 1 (busy bit cleared if fire is pressed)
  if (stick[N_JOY_PARALLEL_1] & BIT_4){
    mfp_gpip_set_bit(0,bool((stick[N_JOY_PARALLEL_1] & BIT_7))==0);
  }else if (old_joypar1_bit4){
    UpdateCentronicsBusyBit();
  }

  {
    int old_mousek=mousek;
    mousek=0;
    if (stick[0] & 128) mousek|=BIT_LMB;
    if (stick[1] & 128) mousek|=BIT_RMB;

    if (stem_mousemode==STEM_MOUSEMODE_WINDOW){
      POINT pt;
      GetCursorPos(&pt);
      if (pt.x!=window_mouse_centre_x || pt.y!=window_mouse_centre_y){
        mouse_move_since_last_interrupt_x+=(pt.x-window_mouse_centre_x);
        mouse_move_since_last_interrupt_y+=(pt.y-window_mouse_centre_y);
        if (mouse_speed!=10){
          int x_if_0=0;
          if (mouse_move_since_last_interrupt_x<0) x_if_0=-1;
          if (mouse_move_since_last_interrupt_x>0) x_if_0=1;

          int y_if_0=0;
          if (mouse_move_since_last_interrupt_y<0) y_if_0=-1;
          if (mouse_move_since_last_interrupt_y>0) y_if_0=1;

          mouse_move_since_last_interrupt_x*=mouse_speed;
          mouse_move_since_last_interrupt_y*=mouse_speed;
          mouse_move_since_last_interrupt_x/=10;
          mouse_move_since_last_interrupt_y/=10;
          if (mouse_move_since_last_interrupt_x==0) mouse_move_since_last_interrupt_x=x_if_0;
          if (mouse_move_since_last_interrupt_y==0) mouse_move_since_last_interrupt_y=y_if_0;
        }
        if (ikbd.mouse_upside_down){
          mouse_move_since_last_interrupt_y=-mouse_move_since_last_interrupt_y;
        }
        mouse_change_since_last_interrupt=true;

#ifdef CYGWIN
        window_mouse_centre_x=pt.x;
        window_mouse_centre_y=pt.y;
#else
        SetCursorPos(window_mouse_centre_x,window_mouse_centre_y);
#endif
      }
    }
    if (macro_record){
      macro_record_mouse(mouse_move_since_last_interrupt_x,mouse_move_since_last_interrupt_y);
    }
    if (macro_play_has_mouse){
      mouse_change_since_last_interrupt=0;
      macro_play_mouse(mouse_move_since_last_interrupt_x,mouse_move_since_last_interrupt_y);
      if (mouse_move_since_last_interrupt_x || mouse_move_since_last_interrupt_y){
        mouse_change_since_last_interrupt=true;
      }
    }

    int report_button_abs=0;
    if (mousek!=old_mousek){
      bool send_change_for_button=true;

      // Handle absolute mouse button flags
      if (RMB_DOWN(mousek) && RMB_DOWN(old_mousek)==0) report_button_abs|=BIT_0;
      if (RMB_DOWN(mousek)==0 && RMB_DOWN(old_mousek)) report_button_abs|=BIT_1;
      if (LMB_DOWN(mousek) && LMB_DOWN(old_mousek)==0) report_button_abs|=BIT_2;
      if (LMB_DOWN(mousek)==0 && LMB_DOWN(old_mousek)) report_button_abs|=BIT_3;
      ikbd.abs_mousek_flags|=report_button_abs;

      // Handle mouse buttons as keys
      if (ikbd.mouse_button_press_what_message & BIT_2){
        if (2 & (mousek^old_mousek)) keyboard_buffer_write(BYTE((mousek & 2) ? 0x74:0xf4)); //if mouse button 1
        if (1 & (mousek^old_mousek)) keyboard_buffer_write(BYTE((mousek & 1) ? 0x75:0xf5)); //if mouse button 2
        send_change_for_button=0; // Don't send mouse packet if you haven't moved mouse
        report_button_abs=0; // No ABS reporting
      }else if (ikbd.mouse_mode==IKBD_MOUSE_MODE_ABSOLUTE){
        if ((ikbd.mouse_button_press_what_message & BIT_0)==0){ // Don't report ABS on press
          report_button_abs&=~(BIT_0 | BIT_2);
        }
        if ((ikbd.mouse_button_press_what_message & BIT_1)==0){ // Don't report ABS on release
          report_button_abs&=~(BIT_1 | BIT_3);
        }
      }else{
        report_button_abs=0;
      }

      if (send_change_for_button) mouse_change_since_last_interrupt=true;
    }

    if (mouse_change_since_last_interrupt){
      if (disable_mouse_until==0 || timer>=disable_mouse_until){
        int max_mouse_move=IKBD_DEFAULT_MOUSE_MOVE_MAX;
        if (macro_play_has_mouse) max_mouse_move=macro_play_max_mouse_speed;
        ikbd_mouse_move(mouse_move_since_last_interrupt_x,mouse_move_since_last_interrupt_y,mousek,max_mouse_move);
      }
      mouse_change_since_last_interrupt=false;
      mouse_move_since_last_interrupt_x=0;
      mouse_move_since_last_interrupt_y=0;
    }
    if (report_button_abs){
      for (int bit=BIT_0;bit<=BIT_3;bit<<=1){
        if (report_button_abs & bit) ikbd_report_abs_mouse(report_button_abs & bit);
      }
    }
  }

  if (macro_play_has_keys==0){
    // Check modifier keys, it's simpler to check them like this rather than
    // respond to messages
    MODIFIERSTATESTRUCT mss=GetLRModifierStates();
    bool StemWinActive=GUICanGetKeys(); 

    if (joy_is_key_used(VK_SHIFT) || CutDisableKey[VK_SHIFT] || StemWinActive==0){
      mss.LShift=0;
      mss.RShift=0;
    }
    if (joy_is_key_used(VK_LSHIFT) || CutDisableKey[VK_LSHIFT]) mss.LShift=0;
    if (joy_is_key_used(VK_RSHIFT) || CutDisableKey[VK_RSHIFT]) mss.RShift=0;

    if (joy_is_key_used(VK_CONTROL) || CutDisableKey[VK_CONTROL] || StemWinActive==0){
      mss.LCtrl=0;
      mss.RCtrl=0;
    }
    if (joy_is_key_used(VK_LCONTROL) || CutDisableKey[VK_LCONTROL]) mss.LCtrl=0;
    if (joy_is_key_used(VK_RCONTROL) || CutDisableKey[VK_RCONTROL]) mss.RCtrl=0;

    if (joy_is_key_used(VK_MENU) || CutDisableKey[VK_MENU] || StemWinActive==0){
      mss.LAlt=0;
      mss.RAlt=0;
    }
    if (joy_is_key_used(VK_LMENU) || CutDisableKey[VK_LMENU]) mss.LAlt=0;
    if (joy_is_key_used(VK_RMENU) || CutDisableKey[VK_RMENU]) mss.RAlt=0;

    int ModDown=ExternalModDown | CutModDown;
    if (ModDown & b00000001) mss.LShift=true;
    if (ModDown & b00000010) mss.RShift=true;
    if (ModDown & b00001100) mss.LCtrl=true;
    if (ModDown & b00110000) mss.LAlt=true;

    if (ST_Key_Down[key_table[VK_LSHIFT]]!=mss.LShift){
      HandleKeyPress(VK_LSHIFT,mss.LShift==0,IGNORE_EXTEND);
    }
    if (ST_Key_Down[key_table[VK_RSHIFT]]!=mss.RShift){
      HandleKeyPress(VK_RSHIFT,mss.RShift==0,IGNORE_EXTEND);
    }
    if (ST_Key_Down[key_table[VK_CONTROL]]!=(mss.LCtrl || mss.RCtrl)){
      HandleKeyPress(VK_CONTROL,(mss.LCtrl || mss.RCtrl)==0,IGNORE_EXTEND);
    }
    if (ST_Key_Down[key_table[VK_MENU]]!=(mss.LAlt || mss.RAlt)){
      HandleKeyPress(VK_MENU,(mss.LAlt || mss.RAlt)==0,IGNORE_EXTEND);
    }

#if !defined(ONEGAME) && defined(WIN32)
    if (TaskSwitchDisabled){
      BYTE n=0,Key;
      while (TaskSwitchVKList[n]){
        Key=TaskSwitchVKList[n];
        if (joy_is_key_used(Key)==0 && CutDisableKey[Key]==0 && CutTaskSwitchVKDown[n]==0){
          if (ST_Key_Down[key_table[Key]] != (GetAsyncKeyState(Key)<0)){
            HandleKeyPress(Key,GetAsyncKeyState(Key)>=0,IGNORE_EXTEND);
          }
        }
        n++;
      }
    }
#endif
  }

  macro_advance();
}
//---------------------------------------------------------------------------
void ikbd_inc_hack(int &hack_val,int inc_val)
{
  if (ikbd.resetting==0) return; 
  if (hack_val==inc_val){
    hack_val=inc_val+1;
  }else{
    hack_val=-1;
  }
}
//---------------------------------------------------------------------------
void agenda_ikbd_process(int src)    //intelligent keyboard handle byte
{
  log(EasyStr("IKBD: At ")+hbl_count+" receives $"+HEXSl(src,2));

  ikbd.send_nothing=0;  // This should only happen if valid command is received!
  if (ikbd.command_read_count){
    if (ikbd.command!=0x50){ //load memory rubbish
      ikbd.command_param[ikbd.command_parameter_counter++]=(BYTE)src;
    }else{
      // Save into IKBD RAM, this is in the strange range $0080 to $00FF (128 bytes)
      if (ikbd.load_memory_address>=0x80 && ikbd.load_memory_address<=0xff){
        ikbd.ram[ikbd.load_memory_address-0x80]=(BYTE)src;
      }
      ikbd.load_memory_address++;
    }
    ikbd.command_read_count--;
    if (ikbd.command_read_count<=0){
      switch (ikbd.command){
      case 0x7: // Set what package is returned when mouse buttons are pressed
        ikbd.mouse_button_press_what_message=ikbd.command_param[0];
        break;
      case 0x9: // Absolute mouse mode
        ikbd.mouse_mode=IKBD_MOUSE_MODE_ABSOLUTE;
        ikbd.abs_mouse_max_x=MAKEWORD(ikbd.command_param[1],ikbd.command_param[0]);
        ikbd.abs_mouse_max_y=MAKEWORD(ikbd.command_param[3],ikbd.command_param[2]);
        ikbd.abs_mouse_x=ikbd.abs_mouse_max_x/2;
        ikbd.abs_mouse_y=ikbd.abs_mouse_max_y/2;
        ikbd.port_0_joy=false;

        ikbd.abs_mousek_flags=0;
        if (RMB_DOWN(mousek)) ikbd.abs_mousek_flags|=BIT_0;
        if (LMB_DOWN(mousek)) ikbd.abs_mousek_flags|=BIT_2;
        break;
      case 0xa: // Return mouse movements as cursor keys
        ikbd.mouse_mode=IKBD_MOUSE_MODE_CURSOR_KEYS;
        ikbd.cursor_key_mouse_pulse_count_x=max(int(ikbd.command_param[0]),1);
        ikbd.cursor_key_mouse_pulse_count_y=max(int(ikbd.command_param[1]),1);
        ikbd.port_0_joy=false;
        agenda_delete(ikbd_report_abs_mouse);
        break;
      case 0xb: // Set relative mouse threshold
        ikbd.relative_mouse_threshold_x=ikbd.command_param[0];
        ikbd.relative_mouse_threshold_y=ikbd.command_param[1];
        ikbd_inc_hack(ikbd.psyg_hack_stage,1);
        break;
      case 0xc://set absolute mouse threshold
        ikbd.abs_mouse_scale_x=ikbd.command_param[0];
        ikbd.abs_mouse_scale_y=ikbd.command_param[1];
        break;
      case 0xe://set mouse position in IKBD
        ikbd.abs_mouse_x=MAKEWORD(ikbd.command_param[2],ikbd.command_param[1]);
        ikbd.abs_mouse_y=MAKEWORD(ikbd.command_param[4],ikbd.command_param[3]);
        break;
      case 0x17://joystick duration
        log("IKBD: Joysticks set to duration mode");
        ikbd.joy_mode=IKBD_JOY_MODE_DURATION;

        ikbd.duration=ikbd.command_param[0]*10; //in 1000ths of a second
        ikbd.mouse_mode=IKBD_MOUSE_MODE_OFF;  //disable mouse
        ikbd.port_0_joy=true;
        agenda_delete(ikbd_send_joystick_message); // just in case sending other type of packet
        agenda_delete(ikbd_report_abs_mouse);
        break;
      case 0x19://cursor key simulation mode for joystick 0
        ikbd.joy_mode=IKBD_JOY_MODE_CURSOR_KEYS;
        for(int n=0;n<6;n++){
          ikbd.cursor_key_joy_time[n]=ikbd.command_param[n];
        }
        ikbd.cursor_key_joy_ticks[0]=timeGetTime();
        ikbd.cursor_key_joy_ticks[1]=ikbd.cursor_key_joy_ticks[0];
        ikbd.mouse_mode=IKBD_MOUSE_MODE_OFF;  //disable mouse
        ikbd.port_0_joy=true;
        agenda_delete(ikbd_send_joystick_message); // just in case sending other type of packet
        agenda_delete(ikbd_report_abs_mouse);
        break;
      case 0x1b://set clock time
        log("IKBD: Set clock to... ");
        for (int n=0;n<6;n++){
          int newval=ikbd.command_param[n];
          if ((newval & 0xf0)>=0xa0){ // Invalid high nibble
            newval&=0xf;
            newval|=ikbd.clock[n] & 0xf;
          }
          if ((newval & 0xf)>=0xa){ // Invalid low nibble
            newval&=0xf0;
            newval|=ikbd.clock[n] & 0xf0;
          }
          int val=(newval >> 4)*10 + (newval & 0xf);
          int max_val=ikbd_clock_max_val[n];
          if (max_val==0){
            int mon=min((ikbd.clock[1] >> 4)*10 + (ikbd.clock[1] & 0xf),12);
            max_val=ikbd_clock_days_in_mon[mon];
          }
          if (val>max_val){
            val=0;if (n==1) val=1;
          }
          ikbd.clock[n]=BYTE((val % 10) | ((val/10) << 4));

          log(HEXSl(ikbd.clock[n],2));
        }
        ikbd.clock_vbl_count=0;
        break;
      case 0x20:  //load memory
        ikbd.command=0x50; // Ant's command about loading memory
        ikbd.load_memory_address=MAKEWORD(ikbd.command_param[1],ikbd.command_param[0]);
        ikbd.command_read_count=ikbd.command_param[2]; //how many bytes to load
        log(Str("IKBD: Loading next ")+ikbd.command_read_count+" bytes into IKBD memory address "+
              HEXSl(ikbd.load_memory_address,4));
        break;
      case 0x50:
        log("IKBD: Finished loading memory");
        break;    //but instead just throw it away!
      case 0x21:  //read memory
      {
        WORD adr=MAKEWORD(ikbd.command_param[1],ikbd.command_param[0]);
        log(Str("IKBD: Reading 6 bytes of IKBD memory, address ")+HEXSl(adr,4));
        keyboard_buffer_write_string(0xf6,0x20,(-1));
        for (int n=0;n<6;n++){
          BYTE b=0;
          if (adr>=0x80 && adr<=0xff) b=ikbd.ram[adr-0x80];
          keyboard_buffer_write(b);
        }
        break;
      }
      case 0x22:  //execute routine
        log(Str("IKBD: Blimey! Executing IKBD routine at ")+
              HEXSl(MAKEWORD(ikbd.command_param[1],ikbd.command_param[0]),4));
        break;    //it worked!
      case 0x80:  //reset
        if (src==0x01) ikbd_reset(0);
        break;
      }
    }
  }else{ //new command
    if (ikbd.joy_mode==IKBD_JOY_MODE_FIRE_BUTTON_DURATION) ikbd.joy_mode=IKBD_JOY_MODE_OFF;
    if (ikbd.resetting && src!=0x08 && src!=0x14) ikbd.reset_0814_hack=-1;
    if (ikbd.resetting && src!=0x12 && src!=0x14) ikbd.reset_1214_hack=-1;
    if (ikbd.resetting && src!=0x08 && src!=0x0B && src!=0x14) ikbd.psyg_hack_stage=-1;
    if (ikbd.resetting && src!=0x12 && src!=0x1A) ikbd.reset_121A_hack=-1;
    ikbd.command=(BYTE)src;
    switch (src){  //how many bytes of parameters do we want?
      case 0x7:case 0x17:case 0x80:ikbd.command_read_count=1;break;
      case 0x8: //return relative mouse position from now on
        ikbd.mouse_mode=IKBD_MOUSE_MODE_RELATIVE;
        ikbd.port_0_joy=false;
        ikbd_inc_hack(ikbd.psyg_hack_stage,0);
        ikbd_inc_hack(ikbd.reset_0814_hack,0);
        agenda_delete(ikbd_report_abs_mouse);
        break;
      case 0x9:ikbd.command_read_count=4;break;
      case 0xa:case 0xb:case 0xc:case 0x21:case 0x22:ikbd.command_read_count=2;break;
      case 0xd: //read absolute mouse position
        // This should be ignored if you aren't in absolute mode!
        if (ikbd.mouse_mode!=IKBD_MOUSE_MODE_ABSOLUTE) break;
        ikbd.port_0_joy=false;
        // Ignore command if already calcing and sending packet
        if (agenda_get_queue_pos(ikbd_report_abs_mouse)>=0) break;
        agenda_add(ikbd_report_abs_mouse,IKBD_SCANLINES_FROM_ABS_MOUSE_POLL_TO_SEND,-1);
        break;
      case 0xe:ikbd.command_read_count=5;break;
      case 0xf: //mouse goes upside down
        ikbd.mouse_upside_down=true;
        break;
      case 0x10: //mouse goes right way up
        ikbd.mouse_upside_down=false;
        break;
      case 0x11: //okay to send!
        log("IKBD turned on");
        ikbd.send_nothing=false;
        break;
      case 0x12: //turn mouse off
        log("IKBD: Mouse turned off");
        ikbd.mouse_mode=IKBD_MOUSE_MODE_OFF;
        ikbd.port_0_joy=true;
        ikbd_inc_hack(ikbd.reset_1214_hack,0);
        ikbd_inc_hack(ikbd.reset_121A_hack,0);
        agenda_delete(ikbd_report_abs_mouse);
        break;
      case 0x13: //stop data transfer to main processor
        log("IKBD turned off");
        ikbd.send_nothing=true;
        break;
      case 0x14: //return joystick movements
        log("IKBD: Changed joystick mode to change notification");
        ikbd.port_0_joy=true;
        ikbd.mouse_mode=IKBD_MOUSE_MODE_OFF;  //disable mouse
        agenda_delete(ikbd_report_abs_mouse);

        if (ikbd.joy_mode!=IKBD_JOY_MODE_AUTO_NOTIFY){
          agenda_delete(ikbd_send_joystick_message); // just in case sending other type of packet
          ikbd.joy_mode=IKBD_JOY_MODE_AUTO_NOTIFY;
        }

        // In the IKBD this resets old_stick to 0
        for (int j=0;j<2;j++){
          if (stick[j]) ikbd_send_joystick_message(j);
        }
        ikbd_inc_hack(ikbd.psyg_hack_stage,2);
        ikbd_inc_hack(ikbd.reset_0814_hack,1);
        ikbd_inc_hack(ikbd.reset_1214_hack,1);
        break;
      case 0x15: //don't return joystick movements
        log("IKBD: Joysticks set to only report when asked");
        ikbd.port_0_joy=true;
        ikbd.mouse_mode=IKBD_MOUSE_MODE_OFF;  //disable mouse
        agenda_delete(ikbd_report_abs_mouse);
        if (ikbd.joy_mode!=IKBD_JOY_MODE_ASK){
          ikbd.joy_mode=IKBD_JOY_MODE_ASK;
          agenda_delete(ikbd_send_joystick_message); // just in case sending other type of packet
        }
        break;
      case 0x16: //read joystick
        if (ikbd.joy_mode!=IKBD_JOY_MODE_OFF){
          // Ignore command if already calcing and sending packet
          if (agenda_get_queue_pos(ikbd_send_joystick_message)>=0) break;
          agenda_add(ikbd_send_joystick_message,IKBD_SCANLINES_FROM_JOY_POLL_TO_SEND,-1);
        }
        break;
      case 0x18: //fire button duration, constant high speed joystick button test
        log("IKBD: Joysticks set to fire button duration mode!");
        ikbd.joy_mode=IKBD_JOY_MODE_FIRE_BUTTON_DURATION;
        ikbd.mouse_mode=IKBD_MOUSE_MODE_OFF;  //disable mouse
        agenda_delete(ikbd_report_abs_mouse);
        ikbd.port_0_joy=true;
        agenda_delete(ikbd_send_joystick_message); // just in case sending other type of packet
        break;
      //    case 0x19: //cursor key mode for joystick 0 (=mouse)
      case 0x1a: //turn off joysticks
        log("IKBD: Joysticks turned off");
//        ikbd.mouse_mode=IKBD_MOUSE_MODE_OFF;  //disable mouse
        ikbd.port_0_joy=0;
        ikbd.joy_mode=IKBD_JOY_MODE_OFF;
        stick[0]=0;stick[1]=0;
        ikbd_inc_hack(ikbd.reset_121A_hack,1);
        agenda_delete(ikbd_send_joystick_message); // just in case sending other type of packet
        break;
      case 0x1b:case 0x19:ikbd.command_read_count=6;break;
      case 0x1c: //read clock time
        keyboard_buffer_write(0xfc);
        for (int n=0;n<6;n++){
          keyboard_buffer_write(ikbd.clock[n]);
        }
        break;
      case 0x20:ikbd.command_read_count=3;break;

      case 0x87: //return what happens when mouse buttons are pressed
        keyboard_buffer_write_string(0xf6,0x7,ikbd.mouse_button_press_what_message,0,0,0,0,0,(-1));
        break;
      case 0x88:case 0x89:case 0x8a:
        keyboard_buffer_write(0xf6);
        keyboard_buffer_write(BYTE(ikbd.mouse_mode));
        if (ikbd.mouse_mode==0x9){
          keyboard_buffer_write_string(HIBYTE(ikbd.abs_mouse_max_x),LOBYTE(ikbd.abs_mouse_max_x),
                                        HIBYTE(ikbd.abs_mouse_max_y),LOBYTE(ikbd.abs_mouse_max_y),
                                        0,0,(-1));
        }else if (ikbd.mouse_mode==0xa){
          keyboard_buffer_write_string(ikbd.cursor_key_mouse_pulse_count_x,
                                        ikbd.cursor_key_mouse_pulse_count_y,
                                        0,0,0,0,(-1));
        }else{
          keyboard_buffer_write_string(0,0,0,0,0,0,(-1));
        }
        break;
      case 0x8b: //x, y threshhold for relative mouse movement messages
        keyboard_buffer_write_string(0xf6,0xb,ikbd.relative_mouse_threshold_x,
                                      ikbd.relative_mouse_threshold_y,
                                      0,0,0,0,(-1));
        break;
      case 0x8c: //x,y scaling of mouse for absolute mouse
        keyboard_buffer_write_string(0xf6,0xc,ikbd.abs_mouse_scale_x,
                                      ikbd.abs_mouse_scale_y,
                                      0,0,0,0,(-1));
        break;
      case 0x8d: /*DEAD*/ break;
      case 0x8e: /*DEAD*/ break;
      case 0x8f:case 0x90: //return 0xf if mouse is upside down, 0x10 otherwise
        keyboard_buffer_write(0xf6);
        if (ikbd.mouse_upside_down){
          keyboard_buffer_write(0xf);
        }else{
          keyboard_buffer_write(0x10);
        }
        keyboard_buffer_write_string(0,0,0,0,0,0,(-1));
        break;
      case 0x91: /*DEAD*/ break;
      case 0x92:  //is mouse off?
        keyboard_buffer_write(0xf6);
        if (ikbd.mouse_mode==IKBD_MOUSE_MODE_OFF){
          keyboard_buffer_write(0x12);
        }else{
          keyboard_buffer_write(0);
        }
        keyboard_buffer_write_string(0,0,0,0,0,0,(-1));
        break;
      case 0x93: /*DEAD*/ break;
      case 0x94:case 0x95:case 0x99:
      {
        keyboard_buffer_write(0xf6);
        // if joysticks are disabled then return previous state. We don't store that.
        BYTE mode=BYTE(ikbd.joy_mode);
        if (mode==IKBD_JOY_MODE_OFF) mode=IKBD_JOY_MODE_AUTO_NOTIFY;
        keyboard_buffer_write(mode);
        if (ikbd.joy_mode==0x19){
          for (int n=0;n<6;n++) keyboard_buffer_write(BYTE(ikbd.cursor_key_joy_time[n]));
        }else{
          keyboard_buffer_write_string(0,0,0,0,0,0,(-1));
        }
        break;
      }
      case 0x96: /*DEAD*/ break;
      case 0x97: /*DEAD*/ break;
      case 0x98: /*DEAD*/ break;
      case 0x9a:  //is joystick off?
        keyboard_buffer_write(0xf6);
        if (ikbd.joy_mode==IKBD_JOY_MODE_OFF){
          keyboard_buffer_write(0x1a);
        }else{
          keyboard_buffer_write(0);
        }
        keyboard_buffer_write_string(0,0,0,0,0,0,(-1));
        break;
        // > 0x9a all DEAD (tested up to 0xac)
    }
    ikbd.command_parameter_counter=0;
  }
}
//---------------------------------------------------------------------------
void agenda_keyboard_replace(int)
{
  log(EasyStr("IKBD: agenda_keyboard_replace at time=")+hbl_count+" with keyboard_buffer_length="+keyboard_buffer_length);

  if (keyboard_buffer_length){
    if (ikbd.send_nothing==0){
      keyboard_buffer_length--;
      if (ikbd.joy_packet_pos>=keyboard_buffer_length) ikbd.joy_packet_pos=-1;
      if (ikbd.mouse_packet_pos>=keyboard_buffer_length) ikbd.mouse_packet_pos=-1;

      if (ACIA_IKBD.rx_not_read){
        log("IKBD: Overrun on keyboard ACIA");
        // discard data and set overrun
        if (ACIA_IKBD.overrun!=ACIA_OVERRUN_YES) ACIA_IKBD.overrun=ACIA_OVERRUN_COMING;
      }else{
        ACIA_IKBD.data=keyboard_buffer[keyboard_buffer_length]; //---------------------------------------------------------------------------
        ACIA_IKBD.rx_not_read=true;
      }
      if (ACIA_IKBD.rx_irq_enabled){
        log(EasyStr("IKBD: Changing ACIA IRQ bit from ")+ACIA_IKBD.irq+" to 1");
        ACIA_IKBD.irq=true;
      }
      mfp_gpip_set_bit(MFP_GPIP_ACIA_BIT,!(ACIA_IKBD.irq || ACIA_MIDI.irq));
    }
    if (keyboard_buffer_length) agenda_add(agenda_keyboard_replace,ACIAClockToHBLS(ACIA_IKBD.clock_divide),0);
  }
  if (macro_start_after_ikbd_read_count) macro_start_after_ikbd_read_count--;
}

void keyboard_buffer_write_n_record(BYTE src)
{
  keyboard_buffer_write(src);
  if (macro_record) macro_record_key(src);
}

void keyboard_buffer_write(BYTE src)
{
  if (keyboard_buffer_length<MAX_KEYBOARD_BUFFER_SIZE){
    if (keyboard_buffer_length){
      memmove(keyboard_buffer+1,keyboard_buffer,keyboard_buffer_length);
    }else{
      // new chars in keyboard so time them out, +1 for middle of scanline
      agenda_add(agenda_keyboard_replace,ACIAClockToHBLS(ACIA_IKBD.clock_divide)+1,0);
    }
    keyboard_buffer_length++;
    keyboard_buffer[0]=src;
    log(EasyStr("IKBD: Wrote $")+HEXSl(src,2)+" keyboard buffer length="+keyboard_buffer_length);
    if (ikbd.joy_packet_pos>=0) ikbd.joy_packet_pos++;
    if (ikbd.mouse_packet_pos>=0) ikbd.mouse_packet_pos++;
  }else{
    log("IKBD: Keyboard buffer overflow");
  }
}

void keyboard_buffer_write_string(int s1,...)
{
  int *ptr;
  for (ptr=&s1;*ptr!=-1;ptr++){
    keyboard_buffer_write(LOBYTE(*ptr));
  }
}

void ikbd_mouse_move(int x,int y,int mousek,int max_mouse_move)
{
//  log(EasyStr("Mouse moves ")+x+","+y);
  if (ikbd.joy_mode<100 || ikbd.port_0_joy==0) {  //not in duration mode or joystick mode
    if (ikbd.mouse_mode==IKBD_MOUSE_MODE_ABSOLUTE){
      ikbd.abs_mouse_x+=x;
      if(ikbd.abs_mouse_x<0)ikbd.abs_mouse_x=0;
      else if(ikbd.abs_mouse_x>ikbd.abs_mouse_max_x)ikbd.abs_mouse_x=ikbd.abs_mouse_max_x;
      ikbd.abs_mouse_y+=y;
      if(ikbd.abs_mouse_y<0)ikbd.abs_mouse_y=0;
      else if(ikbd.abs_mouse_y>ikbd.abs_mouse_max_y)ikbd.abs_mouse_y=ikbd.abs_mouse_max_y;
    }else if (ikbd.mouse_mode==IKBD_MOUSE_MODE_RELATIVE){
      int x1=0;int y1=0;
      while (abs(x-x1)>max_mouse_move || abs(y-y1)>max_mouse_move){
        int x2=min(max_mouse_move,max(-max_mouse_move,x-x1));
        int y2=min(max_mouse_move,max(-max_mouse_move,y-y1));
        keyboard_buffer_write(BYTE(0xf8+(mousek & 3)));
        keyboard_buffer_write(LOBYTE(x2));
        keyboard_buffer_write(LOBYTE(y2));
        x1+=x2;
        y1+=y2;
      }
      keyboard_buffer_write(BYTE(0xf8+(mousek & 3)));
      keyboard_buffer_write(LOBYTE(x-x1));
      keyboard_buffer_write(LOBYTE(y-y1));
    }else if (ikbd.mouse_mode==IKBD_MOUSE_MODE_CURSOR_KEYS){
      while(abs(x)>ikbd.cursor_key_mouse_pulse_count_x || abs(y)>ikbd.cursor_key_mouse_pulse_count_y){
        if(x>ikbd.cursor_key_mouse_pulse_count_x){
          keyboard_buffer_write(0x4d);
          keyboard_buffer_write(0x4d|MSB_B);
          x-=ikbd.cursor_key_mouse_pulse_count_x;
        }else if(x<-ikbd.cursor_key_mouse_pulse_count_x){
          keyboard_buffer_write(0x4b);
          keyboard_buffer_write(0x4b|MSB_B);
          x+=ikbd.cursor_key_mouse_pulse_count_x;
        }
        if(y>ikbd.cursor_key_mouse_pulse_count_y){
          keyboard_buffer_write(0x50);
          keyboard_buffer_write(0x50|MSB_B);
          y-=ikbd.cursor_key_mouse_pulse_count_y;
        }else if(y<-ikbd.cursor_key_mouse_pulse_count_y){
          keyboard_buffer_write(0x48);
          keyboard_buffer_write(0x48|MSB_B);
          y+=ikbd.cursor_key_mouse_pulse_count_y;
        }
      }
      if(mousek&2)keyboard_buffer_write(0x74);else keyboard_buffer_write(0x74|MSB_B);
      if(mousek&1)keyboard_buffer_write(0x75);else keyboard_buffer_write(0x75|MSB_B);
    }
  }
}
//---------------------------------------------------------------------------
void ikbd_set_clock_to_correct_time()
{
  time_t timer=time(NULL);
  struct tm *lpTime=localtime(&timer);
  ikbd.clock[5]=BYTE((lpTime->tm_sec % 10) | ((lpTime->tm_sec/10) << 4));
  ikbd.clock[4]=BYTE((lpTime->tm_min % 10) | ((lpTime->tm_min/10) << 4));
  ikbd.clock[3]=BYTE((lpTime->tm_hour % 10) | ((lpTime->tm_hour/10) << 4));
  ikbd.clock[2]=BYTE((lpTime->tm_mday % 10) | ((lpTime->tm_mday/10) << 4));
  int m=    lpTime->tm_mon +1; //month is 0-based in C RTL
  ikbd.clock[1]=BYTE((m % 10) | ((m/10) << 4));
  int y= (lpTime->tm_year);
  y %= 100;
//  lpTime->tm_year %= 100;
//  ikbd.clock[0]=BYTE((lpTime->tm_year % 10) | ((lpTime->tm_year/10) << 4));
  ikbd.clock[0]=BYTE((y % 10) | ((y/10) << 4));
  ikbd.clock_vbl_count=0;
}

void ikbd_reset(bool Cold)
{
  agenda_delete(agenda_keyboard_reset);

  if (Cold){

    ikbd_set_clock_to_correct_time();

    ikbd.command_read_count=0;
    agenda_delete(agenda_keyboard_replace);
    keyboard_buffer_length=0;
    keyboard_buffer[0]=0;
    ikbd.joy_packet_pos=-1;
    ikbd.mouse_packet_pos=-1;
    agenda_keyboard_reset(0);
    ZeroMemory(ST_Key_Down,sizeof(ST_Key_Down));
  }else{
    agenda_keyboard_reset(0);

    ikbd.resetting=true;
    agenda_add(agenda_keyboard_reset,MILLISECONDS_TO_HBLS(50),true);
  }
}
//---------------------------------------------------------------------------
void agenda_keyboard_reset(int SendF0)
{
  if (SendF0==0){
    ikbd.mouse_button_press_what_message=0;
    ikbd.mouse_mode=IKBD_MOUSE_MODE_RELATIVE;
    ikbd.joy_mode=IKBD_JOY_MODE_AUTO_NOTIFY;
    ikbd.cursor_key_mouse_pulse_count_x=3;
    ikbd.cursor_key_mouse_pulse_count_y=3;
    ikbd.relative_mouse_threshold_x=1;
    ikbd.relative_mouse_threshold_y=1;
    ikbd.abs_mouse_scale_x=1;
    ikbd.abs_mouse_scale_y=1;
    ikbd.abs_mouse_x=shifter_x/2;
    ikbd.abs_mouse_y=shifter_y/2;
    ikbd.abs_mouse_max_x=shifter_x;
    ikbd.abs_mouse_max_y=shifter_y;
    ikbd.mouse_upside_down=false;
    ikbd.send_nothing=false;
    ikbd.port_0_joy=false;
    ikbd.abs_mousek_flags=0;

    ikbd.psyg_hack_stage=0;
    ikbd.reset_0814_hack=0;
    ikbd.reset_1214_hack=0;
    ikbd.reset_121A_hack=0;

    ZeroMemory(ikbd.ram,sizeof(ikbd.ram));

    agenda_delete(ikbd_send_joystick_message); // just in case sending other type of packet
    agenda_delete(ikbd_report_abs_mouse); // just in case sending other type of packet

    stick[0]=0;
    stick[1]=0;
  }else{
    log(EasyStr("IKBD: Finished reset at ")+hbl_count);
    keyboard_buffer_write(IKBD_RESET_MESSAGE);

    if (ikbd.psyg_hack_stage==3 || ikbd.reset_0814_hack==2 || ikbd.reset_1214_hack==2){
      log("IKBD: HACK ACTIVATED - turning mouse on.");
      ikbd.mouse_mode=IKBD_MOUSE_MODE_RELATIVE;
      ikbd.port_0_joy=false;
    }
    if (ikbd.reset_121A_hack==2){ // Turned both mouse and joystick off, but they should be on.
      log("IKBD: HACK ACTIVATED - turning mouse and joystick on.");
      ikbd.mouse_mode=IKBD_MOUSE_MODE_RELATIVE;
      ikbd.joy_mode=IKBD_JOY_MODE_AUTO_NOTIFY;
      ikbd.port_0_joy=false;
    }
    ikbd.mouse_button_press_what_message=0; // Hack to fix No Second Prize
    ikbd.send_nothing=0; // Fix Just Bugging (probably correct though)

    for (int n=1;n<118;n++){
      // Send break codes for "stuck" keys
      if (ST_Key_Down[n]) keyboard_buffer_write(BYTE(0x80 | n));
    }
  }
  ikbd.resetting=0;
}
//---------------------------------------------------------------------------
void ikbd_report_abs_mouse(int abs_mousek_flags)
{
  bool use_current_mousek=(abs_mousek_flags==-1);
  if (use_current_mousek) abs_mousek_flags=ikbd.abs_mousek_flags;

  if (ikbd.mouse_packet_pos>=0){
    keyboard_buffer[ikbd.mouse_packet_pos-1]|=LOBYTE(abs_mousek_flags); // Must |= this or could lose button presses
    keyboard_buffer[ikbd.mouse_packet_pos-2]=HIBYTE(ikbd.abs_mouse_x);
    keyboard_buffer[ikbd.mouse_packet_pos-3]=LOBYTE(ikbd.abs_mouse_x);
    keyboard_buffer[ikbd.mouse_packet_pos-4]=HIBYTE(ikbd.abs_mouse_y);
    keyboard_buffer[ikbd.mouse_packet_pos-5]=LOBYTE(ikbd.abs_mouse_y);
  }else{
    keyboard_buffer_write_string(0xf7,LOBYTE(abs_mousek_flags),
                    HIBYTE(ikbd.abs_mouse_x),LOBYTE(ikbd.abs_mouse_x),
                    HIBYTE(ikbd.abs_mouse_y),LOBYTE(ikbd.abs_mouse_y),-1);
    ikbd.mouse_packet_pos=5;
  }
  if (use_current_mousek) ikbd.abs_mousek_flags=0;
}
//---------------------------------------------------------------------------
void ikbd_send_joystick_message(int jn)
{
  BYTE s[2]={stick[0],stick[1]};
  // If mouse active then joystick never sends button down
  if (ikbd.port_0_joy==0) s[0]&=0x0f, s[1]&=0x0f;
  if (jn==-1){ // requested packet
    if (ikbd.joy_packet_pos>=0){
      keyboard_buffer[ikbd.joy_packet_pos-1]=s[0];
      keyboard_buffer[ikbd.joy_packet_pos-2]=s[1];
    }else{
      keyboard_buffer_write_string(0xfd,s[0],s[1],-1);
      ikbd.joy_packet_pos=2; //0=stick 1, 1=stick 0, 2=header
    }
  }else{
    keyboard_buffer_write_string((BYTE)(0xfe + jn),s[jn],-1);
    log(EasyStr("IKBD: Notified joystick movement, stick[")+jn+"]="+s[jn]);
  }
}
//---------------------------------------------------------------------------
#undef LOGSECTION

