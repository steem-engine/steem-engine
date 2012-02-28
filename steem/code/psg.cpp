/*---------------------------------------------------------------------------
FILE: psg.cpp
MODULE: emu
DESCRIPTION: Steem's Programmable Sound Generator (Yamaha 2149) and STE
DMA sound output emulation. Sound_VBL is the main function writing one
frame of sound to the output buffer. The I/O code isn't included here, see
ior.cpp and iow.cpp for the lowest level emulation.
---------------------------------------------------------------------------*/

#define LOGSECTION LOGSECTION_SOUND
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HRESULT Sound_Start()
{
#ifdef UNIX
	if (sound_internal_speaker){
		console_device=open("/dev/console",O_RDONLY | O_NDELAY,0);
		if (console_device==-1){
		  printf("Couldn't open console for internal speaker output\n");
		  sound_internal_speaker=false;
      GUIUpdateInternalSpeakerBut();
		}
	}
#endif

  if (sound_mode==SOUND_MODE_MUTE) return DS_OK;
  if (UseSound==0) return DSERR_GENERIC;  // Not initialised
  if (SoundActive()) return DS_OK;        // Already started

  if (fast_forward || slow_motion || runstate!=RUNSTATE_RUNNING) return DSERR_GENERIC;

  sound_first_vbl=true;

  log("SOUND: Starting sound buffers and initialising PSG variables");

  // Work out startup voltage
  int envshape=psg_reg[13] & 15;
  int flatlevel=0;
  for (int abc=0;abc<3;abc++){
    if ((psg_reg[8+abc] & BIT_4)==0){
      flatlevel+=psg_flat_volume_level[psg_reg[8+abc] & 15];
    }else if (envshape==b1011 || envshape==b1101){
      flatlevel+=psg_flat_volume_level[15];
    }
  }
  psg_voltage=flatlevel;psg_dv=0;

  WORD dma_l,dma_r;
  dma_sound_get_last_sample(&dma_l,&dma_r);
  int current_l=HIBYTE(flatlevel)+HIBYTE(dma_l),current_r=HIBYTE(flatlevel)+HIBYTE(dma_r);
  if (sound_num_bits==16){
    current_l^=128;current_r^=128;
  }
  if (SoundStartBuffer((signed char)current_l,(signed char)current_r)!=DS_OK){
    return DDERR_GENERIC;
  }
  for (int n=PSG_NOISE_ARRAY-1;n>=0;n--) psg_noise[n]=(BYTE)random(2);

#ifdef ONEGAME
  // Make sure sound is still good(ish) if you are running below 80% speed
  OGExtraSamplesPerVBL=300;
  if (run_speed_ticks_per_second>1000){
    // Get the number of extra ms of sound per "second", change that to number
    // of samples, divide to get the number of samples per VBL and add extra.
    OGExtraSamplesPerVBL=((((run_speed_ticks_per_second-1000)*sound_freq)/1000)/shifter_freq)+300;
  }
#endif

  psg_time_of_start_of_buffer=0;
  psg_last_play_cursor=0;
//  psg_last_write_time=0; not used now?
  psg_time_of_last_vbl_for_writing=0;
  psg_time_of_next_vbl_for_writing=0;

/*
  psg_time_of_start_of_buffer=(0xffffffff-(sound_freq*4)) &(-PSG_BUF_LENGTH);
  psg_last_play_cursor=psg_time_of_start_of_buffer;
  psg_last_write_time=psg_time_of_start_of_buffer;
  psg_time_of_last_vbl_for_writing=psg_time_of_start_of_buffer;
  psg_time_of_next_vbl_for_writing=psg_time_of_start_of_buffer;
*/

  for (int abc=2;abc>=0;abc--){
    psg_buf_pointer[abc]=0;
    psg_tone_start_time[abc]=0;
  }
  for (int i=0;i<PSG_CHANNEL_BUF_LENGTH;i++) psg_channels_buf[i]=VOLTAGE_FP(VOLTAGE_ZERO_LEVEL);
  psg_envelope_start_time=0xff000000;

  if (sound_record){
    timer=timeGetTime();
    sound_record_start_time=timer+200; //start recording in 200ms time
    sound_record_open_file();
  }

  return DS_OK;
}
//---------------------------------------------------------------------------
void SoundStopInternalSpeaker()
{
  internal_speaker_sound_by_period(0);
}
//---------------------------------------------------------------------------
#ifdef ENABLE_VARIABLE_SOUND_DAMPING

#define CALC_V_CHIP  \
                if (v!=*source_p || dv){                            \
                  v+=dv;                                            \
                  dv-=(v-(*source_p))*sound_variable_a >> 8;        \
                  dv*=sound_variable_d;                                           \
                  dv>>=8;                                           \
                }

#define CALC_V_CHIP_25KHZ  \
                if (v!=*source_p || dv){                            \
                  v+=dv;                                            \
                  dv-=(v-(*source_p))*sound_variable_a >> 8;        \
                  dv*=sound_variable_d;                                           \
                  dv>>=8;                                           \
                }

#else

#define CALC_V_CHIP  \
                if (v!=*source_p || dv){                            \
                  v+=dv;                                            \
                  dv-=(v-(*source_p)) >> 3;                         \
                  dv*=13;                                           \
                  dv>>=4;                                           \
                }

#define CALC_V_CHIP_25KHZ  \
                if (v!=*source_p || dv){                            \
                  v+=dv;                                            \
                  dv-=((v-(*source_p)) *3) >>3;                         \
                  dv*=3;                                           \
                  dv>>=2;                                           \
                }

//60, C0

#endif

#define CALC_V_EMU  v=*source_p;

#ifdef SHOW_WAVEFORM
  #define WAVEFORM_SET_VAL(v) (val=(v))
  #define WAVEFORM_ONLY(x) x
#else
  #define WAVEFORM_SET_VAL(v) v
  #define WAVEFORM_ONLY(x)
#endif



#ifdef WRITE_ONLY_SINE_WAVE

#define SINE_ONLY(s) s

#define WRITE_SOUND_LOOP(Alter_V)         \
	          while (c>0){                                                  \
                *(p++)=WAVEFORM_SET_VAL(BYTE(sin((double)t*(M_PI/64))*120+128)); \
                t++;                                                       \
                WAVEFORM_ONLY( temp_waveform_display[((int)(source_p-psg_channels_buf)+psg_time_of_last_vbl_for_writing) % MAX_temp_waveform_display_counter]=(BYTE)val; ) \
    	          *(source_p++)=VOLTAGE_FP(VOLTAGE_ZERO_LEVEL);                 \
                c--;    \
	          }

#else

#define SINE_ONLY(s)

#define WRITE_SOUND_LOOP(Alter_V,Out_P,Size,GetSize)         \
	          while (c>0){                                                  \
              Alter_V                                                     \
              val=v + *lp_dma_sound_channel;                           \
  	          if (val<VOLTAGE_FP(0)){                                     \
                val=VOLTAGE_FP(0); \
      	      }else if (val>VOLTAGE_FP(255)){                            \
        	      val=VOLTAGE_FP(255);                    \
  	          }                                                            \
              *(Out_P++)=Size(GetSize(&val)); \
              if (sound_num_channels==2){         \
                val=v + *(lp_dma_sound_channel+1);                                            \
                if (val<VOLTAGE_FP(0)){                                     \
                  val=VOLTAGE_FP(0); \
                }else if (val>VOLTAGE_FP(255)){                            \
                  val=VOLTAGE_FP(255);                    \
                }                                                            \
                *(Out_P++)=Size(GetSize(&val)); \
              }     \
    	        WAVEFORM_ONLY(temp_waveform_display[((int)(source_p-psg_channels_buf)+psg_time_of_last_vbl_for_writing) % MAX_temp_waveform_display_counter]=WORD_B_1(&val)); \
  	          *(source_p++)=VOLTAGE_FP(VOLTAGE_ZERO_LEVEL);                 \
              if (lp_dma_sound_channel<lp_max_dma_sound_channel) lp_dma_sound_channel+=2; \
      	      c--;                                                          \
	          }

#endif

#define WRITE_TO_WAV_FILE_B(val) fputc(BYTE(WORD_B_1(&(val))),wav_file);
#define WRITE_TO_WAV_FILE_W(val) val^=MSB_W;fputc(LOBYTE(val),wav_file);fputc(HIBYTE(val),wav_file);

#define SOUND_RECORD(Alter_V,WRITE)         \
            while (c>0){                        \
              Alter_V;                          \
              val=v + *lp_dma_sound_channel;                           \
              if (val<VOLTAGE_FP(0)){                                     \
                val=VOLTAGE_FP(0); \
              }else if (val>VOLTAGE_FP(255)){                            \
                val=VOLTAGE_FP(255); \
              }                                                            \
              WRITE(val); \
              if (sound_num_channels==2){         \
                val=v + *(lp_dma_sound_channel+1);                           \
                if (val<VOLTAGE_FP(0)){                                     \
                  val=VOLTAGE_FP(0); \
                }else if (val>VOLTAGE_FP(255)){                            \
                  val=VOLTAGE_FP(255); \
                }                                                            \
                WRITE(val); \
              }  \
              source_p++;                       \
              SINE_ONLY( t++ );                                   \
              if (lp_dma_sound_channel<lp_max_dma_sound_channel) lp_dma_sound_channel+=2; \
              c--;                                          \
            }

void sound_record_to_wav(int c,DWORD SINE_ONLY(t),bool chipmode,int *source_p)
{
  if (timer<sound_record_start_time) return;

  int v=psg_voltage,dv=psg_dv; //restore from last time
  WORD *lp_dma_sound_channel=dma_sound_channel_buf;
  WORD *lp_max_dma_sound_channel=dma_sound_channel_buf+dma_sound_channel_buf_last_write_t;
  int val;
  if (sound_num_bits==8){
    if (chipmode){
      if (sound_low_quality==0){
        SOUND_RECORD(CALC_V_CHIP,WRITE_TO_WAV_FILE_B);
      }else{
        SOUND_RECORD(CALC_V_CHIP_25KHZ,WRITE_TO_WAV_FILE_B);
      }
    }else{
      SOUND_RECORD(CALC_V_EMU,WRITE_TO_WAV_FILE_B);
    }
  }else{
    if (chipmode){
      if (sound_low_quality==0){
        SOUND_RECORD(CALC_V_CHIP,WRITE_TO_WAV_FILE_W);
      }else{
        SOUND_RECORD(CALC_V_CHIP_25KHZ,WRITE_TO_WAV_FILE_W);
      }
    }else{
      SOUND_RECORD(CALC_V_EMU,WRITE_TO_WAV_FILE_W);
    }
  }
}

HRESULT Sound_VBL()
{
#if SCREENS_PER_SOUND_VBL != 1
  static int screens_countdown=SCREENS_PER_SOUND_VBL;
  screens_countdown--;if (screens_countdown>0) return DD_OK;
  screens_countdown=SCREENS_PER_SOUND_VBL;
  cpu_time_of_last_sound_vbl=ABSOLUTE_CPU_TIME;
#endif
  if (sound_internal_speaker){
    static double op=0;
    int abc,chan=-1,max_vol=0,vol;

    // Find loudest channel
    for (abc=0;abc<3;abc++){
      if ((psg_reg[PSGR_MIXER] & (1 << abc))==0){ // Channel enabled in mixer
        vol=(psg_reg[PSGR_AMPLITUDE_A+abc] & 15);
        if (vol>max_vol){
          chan=abc;
          max_vol=vol;
        }
      }
    }
    if (chan==-1){ //no sound
      internal_speaker_sound_by_period(0);
      op=0;
    }else{
      double p=((((int)psg_reg[chan*2+1] & 0xf) << 8) + psg_reg[chan*2]);
      p*=(1193181.0/125000.0);
      if (op!=p){
        op=p;
        internal_speaker_sound_by_period((int)p);
      }
    }
  }

  if (psg_capture_file){
    psg_capture_check_boundary();
  }

  // This just clears up some clicks when Sound_VBL is called very soon after Sound_Start
  if (sound_first_vbl){
    sound_first_vbl=0;
    return DS_OK;
  }

  if (sound_mode==SOUND_MODE_MUTE) return DS_OK;
  if (UseSound==0) return DSERR_GENERIC;  // Not initialised
  if (SoundActive()==0) return DS_OK;        // Not started

  log("");
  log("SOUND: Start of Sound_VBL");

  void *DatAdr[2]={NULL,NULL};
  DWORD LockLength[2]={0,0};
  DWORD s_time,write_time_1,write_time_2;
  HRESULT Ret;

  int *source_p;

  DWORD n_samples_per_vbl=(sound_freq*SCREENS_PER_SOUND_VBL)/shifter_freq;

  log(EasyStr("SOUND: Calculating time; psg_time_of_start_of_buffer=")+psg_time_of_start_of_buffer);


  s_time=SoundGetTime();
  //we have data from time_of_last_vbl+PSG_WRITE_N_SCREENS_AHEAD*n_samples_per_vbl up to
  //wherever we want

  write_time_1=psg_time_of_last_vbl_for_writing; //3 screens ahead of where the cursor was
//  write_time_1=max(write_time_1,min_write_time); //minimum time for new write

  write_time_2=max(write_time_1+(n_samples_per_vbl+PSG_WRITE_EXTRA),
                   s_time+(n_samples_per_vbl+PSG_WRITE_EXTRA));
  if ((write_time_2-write_time_1)>PSG_CHANNEL_BUF_LENGTH){
    write_time_2=write_time_1+PSG_CHANNEL_BUF_LENGTH;
  }

//  psg_last_write_time=write_time_2;

  DWORD time_of_next_vbl_to_write=max(s_time+n_samples_per_vbl*psg_write_n_screens_ahead,psg_time_of_next_vbl_for_writing);
  if (time_of_next_vbl_to_write>s_time+n_samples_per_vbl*(psg_write_n_screens_ahead+2)){  //
    time_of_next_vbl_to_write=s_time+n_samples_per_vbl*(psg_write_n_screens_ahead+2);     // new bit added by Ant 9/1/2001 to stop the sound lagging behind
  }                                                                                    // get rid of it if it is causing problems

  log(EasyStr("   writing from ")+write_time_1+" to "+write_time_2+"; current play cursor at "+s_time+" ("+play_cursor+"); minimum write at "+min_write_time+" ("+write_cursor+")");

//  log_write(EasyStr("writing ")+(write_time_1-s_time)+" samples ahead of play cursor, "+(write_time_1-min_write_time)+" ahead of min write");

#ifdef SHOW_WAVEFORM
  temp_waveform_display_counter=write_time_1 MOD_PSG_BUF_LENGTH;
  temp_waveform_play_counter=play_cursor;
#endif
  log("SOUND: Working out data up to the end of this VBL plus a bit more for all channels");
  for (int abc=2;abc>=0;abc--){
    psg_write_buffer(abc,time_of_next_vbl_to_write+PSG_WRITE_EXTRA);
  }
  if (dma_sound_on_this_screen){
    WORD w[2]={dma_sound_channel_buf[dma_sound_channel_buf_last_write_t-2],dma_sound_channel_buf[dma_sound_channel_buf_last_write_t-1]};
    for (int i=0;i<PSG_WRITE_EXTRA;i++){
      if (dma_sound_channel_buf_last_write_t>=DMA_SOUND_BUFFER_LENGTH) break;
      dma_sound_channel_buf[dma_sound_channel_buf_last_write_t++]=w[0];
      dma_sound_channel_buf[dma_sound_channel_buf_last_write_t++]=w[1];
    }
  }else{
    WORD w1,w2;
    dma_sound_get_last_sample(&w1,&w2);
    dma_sound_channel_buf[0]=w1;
    dma_sound_channel_buf[1]=w2;
    dma_sound_channel_buf_last_write_t=0;
  }

  // write_time_1 and 2 are sample variables, convert to bytes
  DWORD StartByte=(write_time_1 MOD_PSG_BUF_LENGTH)*sound_bytes_per_sample;
  DWORD NumBytes=((write_time_2-write_time_1)+1)*sound_bytes_per_sample;
  log(EasyStr("SOUND: Trying to lock from ")+StartByte+", length "+NumBytes);
  Ret=SoundLockBuffer(StartByte,NumBytes,&DatAdr[0],&LockLength[0],&DatAdr[1],&LockLength[1]);
  if (Ret!=DSERR_BUFFERLOST){
    if (Ret!=DS_OK){
      log_write("SOUND: Lock totally failed, disaster!");
      return SoundError("Lock for PSG Buffer Failed",Ret);
    }
    log(EasyStr("SOUND: Locked lengths ")+LockLength[0]+", "+LockLength[1]);
    int i=min(max(int(write_time_1-psg_time_of_last_vbl_for_writing),0),PSG_CHANNEL_BUF_LENGTH-10);
    int v=psg_voltage,dv=psg_dv; //restore from last time
    log(EasyStr("SOUND: Zeroing channels buffer up to ")+i);
    for (int j=0;j<i;j++){
      psg_channels_buf[j]=VOLTAGE_FP(VOLTAGE_ZERO_LEVEL); //zero the start of the buffer
    }
    source_p=psg_channels_buf+i;
    int samples_left_in_buffer=max(PSG_CHANNEL_BUF_LENGTH-i,0);
    int countdown_to_storing_values=max((int)(time_of_next_vbl_to_write-write_time_1),0);
    //this is set when we are counting down to the start time of the next write
    bool store_values=false,chipmode=bool((sound_mode==SOUND_MODE_EMULATED) ? false:true);
    if (sound_mode==SOUND_MODE_SHARPSAMPLES) chipmode=(psg_reg[PSGR_MIXER] & b00111111)!=b00111111;
    if (sound_mode==SOUND_MODE_SHARPCHIP)    chipmode=(psg_reg[PSGR_MIXER] & b00111111)==b00111111;
    if (sound_record){
      sound_record_to_wav(countdown_to_storing_values,write_time_1,chipmode,source_p);
    }

#ifdef WRITE_ONLY_SINE_WAVE
    DWORD t=write_time_1;
#endif

    int val;
    log("SOUND: Starting to write to buffers");
    WORD *lp_dma_sound_channel=dma_sound_channel_buf;
    WORD *lp_max_dma_sound_channel=dma_sound_channel_buf+dma_sound_channel_buf_last_write_t;
    BYTE *pb;
    WORD *pw;
    for (int n=0;n<2;n++){
      if (DatAdr[n]){
        pb=(BYTE*)(DatAdr[n]);
        pw=(WORD*)(DatAdr[n]);
        int c=min(int(LockLength[n]/sound_bytes_per_sample),samples_left_in_buffer),oc=c;
        if (c>countdown_to_storing_values){
          c=countdown_to_storing_values;
          oc-=countdown_to_storing_values;
          store_values=true;
        }
        for (;;){
          if (sound_num_bits==8){
            if (chipmode){
              if (sound_low_quality==0){
                WRITE_SOUND_LOOP(CALC_V_CHIP,pb,BYTE,DWORD_B_1);
              }else{
                WRITE_SOUND_LOOP(CALC_V_CHIP_25KHZ,pb,BYTE,DWORD_B_1);
              }
            }else{
              WRITE_SOUND_LOOP(CALC_V_EMU,pb,BYTE,DWORD_B_1);
            }
          }else{
            if (chipmode){
              if (sound_low_quality==0){
                WRITE_SOUND_LOOP(CALC_V_CHIP,pw,WORD,MSB_W ^ DWORD_W_0);
              }else{
                WRITE_SOUND_LOOP(CALC_V_CHIP_25KHZ,pw,WORD,MSB_W ^ DWORD_W_0);
              }
            }else{
              WRITE_SOUND_LOOP(CALC_V_EMU,pw,WORD,MSB_W ^ DWORD_W_0);
            }
          }

          if (store_values){
            c=oc;
            psg_voltage=v;
            psg_dv=dv;
            store_values=false;
            countdown_to_storing_values=0x7fffffff; //don't store the values again.
          }else{
            countdown_to_storing_values-=LockLength[n]/sound_bytes_per_sample;
            break;
          }
        }
        samples_left_in_buffer-=LockLength[n]/sound_bytes_per_sample;
      }
    }
    SoundUnlock(DatAdr[0],LockLength[0],DatAdr[1],LockLength[1]);
    while (source_p < (psg_channels_buf+PSG_CHANNEL_BUF_LENGTH)){
      *(source_p++)=VOLTAGE_FP(VOLTAGE_ZERO_LEVEL); //zero the rest of the buffer
    }
  }

  psg_buf_pointer[0]=0;
  psg_buf_pointer[1]=0;
  psg_buf_pointer[2]=0;

  psg_time_of_last_vbl_for_writing=time_of_next_vbl_to_write;

  psg_time_of_next_vbl_for_writing=max(s_time+n_samples_per_vbl*(psg_write_n_screens_ahead+1),
                                       time_of_next_vbl_to_write+n_samples_per_vbl);
  psg_time_of_next_vbl_for_writing=min(psg_time_of_next_vbl_for_writing,
                                       s_time+(PSG_BUF_LENGTH/2));
  log(EasyStr("SOUND: psg_time_of_next_vbl_for_writing=")+psg_time_of_next_vbl_for_writing);

  psg_n_samples_this_vbl=psg_time_of_next_vbl_for_writing-psg_time_of_last_vbl_for_writing;

  log("SOUND: End of Sound_VBL");
  log("");

  return DS_OK;
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*                                DMA SOUND                                  */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void dma_sound_set_control(BYTE io_src_b)
{
  if ((dma_sound_control & BIT_0) && (io_src_b & BIT_0)==0){  //Stopping
    dma_sound_start=next_dma_sound_start;
    dma_sound_end=next_dma_sound_end;
    dma_sound_fetch_address=dma_sound_start;
  }else if ((dma_sound_control & BIT_0)==0 && (io_src_b & BIT_0)){ //Start playing
    dma_sound_start=next_dma_sound_start;
    dma_sound_end=next_dma_sound_end;
    dma_sound_fetch_address=dma_sound_start;
    if (dma_sound_on_this_screen==0){
      // Pad buffer with last byte from VBL to current position
      bool Mono=bool(dma_sound_mode & BIT_7);
      int freq_idx=0;
      if (shifter_freq_at_start_of_vbl==60) freq_idx=1;
      if (shifter_freq_at_start_of_vbl==MONO_HZ) freq_idx=2;
      WORD w1,w2;
      dma_sound_get_last_sample(&w1,&w2);
      for (int y=-scanlines_above_screen[freq_idx];y<scan_y;y++){
        if (Mono){  //play half as many words
          dma_sound_samples_countdown+=dma_sound_freq*scanline_time_in_cpu_cycles_at_start_of_vbl/2;
        }else{ //stereo, 1 word per sample
          dma_sound_samples_countdown+=dma_sound_freq*scanline_time_in_cpu_cycles_at_start_of_vbl;
        }
        int loop=int(Mono ? 2:1);
        while (dma_sound_samples_countdown>=0){
          for (int i=0;i<loop;i++){
            dma_sound_output_countdown+=sound_freq;
            while (dma_sound_output_countdown>=0){
              if (dma_sound_channel_buf_last_write_t>=DMA_SOUND_BUFFER_LENGTH) break;
              dma_sound_channel_buf[dma_sound_channel_buf_last_write_t++]=w1;
              dma_sound_channel_buf[dma_sound_channel_buf_last_write_t++]=w2;
              dma_sound_output_countdown-=dma_sound_freq;
            }
          }
          dma_sound_samples_countdown-=n_cpu_cycles_per_second;
        }
      }
      dma_sound_on_this_screen=1;
    }
  }
  log_to(LOGSECTION_SOUND,EasyStr("SOUND: ")+HEXSl(old_pc,6)+" - DMA sound control set to "+(io_src_b & 3)+" from "+(dma_sound_control & 3));
  dma_sound_control=io_src_b;
  if (tos_version>=0x106) mfp_gpip_set_bit(MFP_GPIP_MONO_BIT,bool(COLOUR_MONITOR)^bool(dma_sound_control & BIT_0));
}
//---------------------------------------------------------------------------
void dma_sound_set_mode(BYTE new_mode)
{
  dma_sound_mode=new_mode;
  dma_sound_freq=dma_sound_mode_to_freq[dma_sound_mode & 3];
  log_to(LOGSECTION_SOUND,EasyStr("SOUND: ")+HEXSl(old_pc,6)+" - DMA sound mode set to $"+HEXSl(dma_sound_mode,2)+" freq="+dma_sound_freq);
}
//---------------------------------------------------------------------------
void dma_sound_fetch()
{
  bool Playing=bool(dma_sound_control & BIT_0);
  bool Mono=bool(dma_sound_mode & BIT_7);

  //we want to play a/b samples, where a is the DMA sound frequency
  //and b is the number of scanlines a second

  int left_vol_top_val=dma_sound_l_top_val,right_vol_top_val=dma_sound_r_top_val;
  //this a/b is the same as dma_sound_freq*scanline_time_in_cpu_cycles/8million
  if (Mono){  //play half as many words
    dma_sound_samples_countdown+=dma_sound_freq*scanline_time_in_cpu_cycles_at_start_of_vbl/2;
    left_vol_top_val=(dma_sound_l_top_val >> 1)+(dma_sound_r_top_val >> 1);
    right_vol_top_val=left_vol_top_val;
  }else{ //stereo, 1 word per sample
    dma_sound_samples_countdown+=dma_sound_freq*scanline_time_in_cpu_cycles_at_start_of_vbl;
  }
  bool vol_change_l=(left_vol_top_val<128),vol_change_r=(right_vol_top_val<128);
  while (dma_sound_samples_countdown>=0){
    //play word from buffer
    if (dma_sound_internal_buf_len>0){
      dma_sound_last_word=dma_sound_internal_buf[0];
      for (int i=0;i<3;i++) dma_sound_internal_buf[i]=dma_sound_internal_buf[i+1];
      dma_sound_internal_buf_len--;

      if (vol_change_l){
        int b1=(signed char)(HIBYTE(dma_sound_last_word));
        b1*=left_vol_top_val;
        b1/=128;
        dma_sound_last_word&=0x00ff;
        dma_sound_last_word|=WORD(BYTE(b1) << 8);
      }
      if (vol_change_r){
        int b2=(signed char)(LOBYTE(dma_sound_last_word));
        b2*=right_vol_top_val;
        b2/=128;
        dma_sound_last_word&=0xff00;
        dma_sound_last_word|=BYTE(b2);
      }
      dma_sound_last_word^=WORD((128 << 8) | 128); //unsign
    }
    dma_sound_output_countdown+=sound_freq;
    WORD w1;
    WORD w2;
    if (Mono){       //mono, play half as many words
      w1=WORD((dma_sound_last_word & 0xff00) >> 2);
      w2=WORD((dma_sound_last_word & 0x00ff) << 6);
      // dma_sound_channel_buf always stereo, so put each mono sample in twice
      while (dma_sound_output_countdown>=0){
        if (dma_sound_channel_buf_last_write_t>=DMA_SOUND_BUFFER_LENGTH) break;
        dma_sound_channel_buf[dma_sound_channel_buf_last_write_t++]=w1;
        dma_sound_channel_buf[dma_sound_channel_buf_last_write_t++]=w1;
        dma_sound_output_countdown-=dma_sound_freq;
      }
      dma_sound_output_countdown+=sound_freq;
      while (dma_sound_output_countdown>=0){
        if (dma_sound_channel_buf_last_write_t>=DMA_SOUND_BUFFER_LENGTH) break;
        dma_sound_channel_buf[dma_sound_channel_buf_last_write_t++]=w2;
        dma_sound_channel_buf[dma_sound_channel_buf_last_write_t++]=w2;
        dma_sound_output_countdown-=dma_sound_freq;
      }
    }else{//stereo , 1 word per sample
      if (sound_num_channels==1){
        //average the channels out
        w1=WORD(((dma_sound_last_word & 255)+(dma_sound_last_word >> 8)) << 5);
        w2=0; // skipped
      }else{
        w1=WORD((dma_sound_last_word & 0xff00) >> 2);
        w2=WORD((dma_sound_last_word & 0x00ff) << 6);
      }
      while (dma_sound_output_countdown>=0){
        if (dma_sound_channel_buf_last_write_t>=DMA_SOUND_BUFFER_LENGTH) break;
        dma_sound_channel_buf[dma_sound_channel_buf_last_write_t++]=w1;
        dma_sound_channel_buf[dma_sound_channel_buf_last_write_t++]=w2;
        dma_sound_output_countdown-=dma_sound_freq;
      }
    }
    dma_sound_samples_countdown-=n_cpu_cycles_per_second;
  }
  if (Playing==0) return;
  if (dma_sound_internal_buf_len>=4) return;
  if (dma_sound_fetch_address>=himem) return;
  for (int i=0;i<4;i++){
    if (dma_sound_fetch_address>=dma_sound_end){
      dma_sound_start=next_dma_sound_start;
      dma_sound_end=next_dma_sound_end;
      dma_sound_fetch_address=dma_sound_start;
      dma_sound_control&=~BIT_0;

      DMA_SOUND_CHECK_TIMER_A
      if (tos_version>=0x106) mfp_gpip_set_bit(MFP_GPIP_MONO_BIT,bool(COLOUR_MONITOR)^bool(dma_sound_control & BIT_0));

      if (dma_sound_control & BIT_1){
        dma_sound_control|=BIT_0; //Playing again immediately
        if (tos_version>=0x106) mfp_gpip_set_bit(MFP_GPIP_MONO_BIT,bool(COLOUR_MONITOR)^bool(dma_sound_control & BIT_0));
      }else{
        break;
      }
    }
    dma_sound_internal_buf[dma_sound_internal_buf_len++]=DPEEK(dma_sound_fetch_address);
    dma_sound_fetch_address+=2;
    if (dma_sound_internal_buf_len>=4) break;
  }
}
//---------------------------------------------------------------------------
void dma_sound_get_last_sample(WORD *pw1,WORD *pw2)
{
  if (dma_sound_mode & BIT_7){
    // ST plays HIBYTE, LOBYTE, so last sample is LOBYTE
    *pw1=WORD((dma_sound_last_word & 0x00ff) << 6);
    *pw2=*pw1; // play the same in both channels, or ignored in when sound_num_channels==1
  }else{
    if (sound_num_channels==1){
      //average the channels out
      *pw1=WORD(((dma_sound_last_word & 255)+(dma_sound_last_word >> 8)) << 5);
      *pw2=0; // skipped
    }else{
      *pw1=WORD((dma_sound_last_word & 0xff00) >> 2);
      *pw2=WORD((dma_sound_last_word & 0x00ff) << 6);
    }
  }
}
//---------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*                                PSG SOUND                                  */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#define PSG_CALC_VOLTAGE_ENVELOPE                                     \
      {																																	\
        envstage=(t64-est64)/envmodulo2;           \
        if (envstage>=32 && envdeath!=-1){                           \
          *(p++)+=envdeath;                                             \
        }else{                                                       \
          *(p++)+=psg_envelope_level[envshape][envstage & 63];            \
        }																															\
      }

#define PSG_PULSE_NOISE(ntn) (psg_noise[(ntn) MOD_PSG_NOISE_ARRAY] )
#define PSG_PULSE_TONE  ((t*128 / psg_tonemodulo) & 1)
#define PSG_PULSE_TONE_t64  ((t*64 / psg_tonemodulo_2) & 1)


#define PSG_PREPARE_ENVELOPE                                \
      int envperiod=max( (((int)psg_reg[PSGR_ENVELOPE_PERIOD_HIGH]) <<8) + psg_reg[PSGR_ENVELOPE_PERIOD_LOW],1);  \
      af=envperiod;                              \
      af*=sound_freq;                  \
      af*=((double)(1<<13))/15625;                               \
      psg_envmodulo=(int)af; \
      bf=(((DWORD)t)-psg_envelope_start_time); \
      bf*=(double)(1<<17); \
      psg_envstage=(int)floor(bf/af); \
      bf=fmod(bf,af); /*remainder*/ \
      psg_envcountdown=psg_envmodulo-(int)bf; \
      envdeath=-1;                                                                  \
      if ((psg_reg[PSGR_ENVELOPE_SHAPE] & PSG_ENV_SHAPE_CONT)==0 ||                  \
           (psg_reg[PSGR_ENVELOPE_SHAPE] & PSG_ENV_SHAPE_HOLD)){                      \
        if(psg_reg[PSGR_ENVELOPE_SHAPE]==11 || psg_reg[PSGR_ENVELOPE_SHAPE]==13){      \
          envdeath=psg_flat_volume_level[15];                                           \
        }else{                                                                           \
          envdeath=psg_flat_volume_level[0];                                              \
        }                                                                                   \
      }                                                                                      \
      envshape=psg_reg[PSGR_ENVELOPE_SHAPE] & 7;                    \
      if (psg_envstage>=32 && envdeath!=-1){                           \
        envvol=envdeath;                                             \
      }else{                                                       \
        envvol=psg_envelope_level[envshape][psg_envstage & 63];            \
      }																															\



#define PSG_PREPARE_NOISE                                \
      int noiseperiod=(1+(psg_reg[PSGR_NOISE_PERIOD]&0x1f));      \
      af=((int)noiseperiod*sound_freq);                              \
      af*=((double)(1<<17))/15625; \
      psg_noisemodulo=(int)af; \
      bf=t; \
      bf*=(double)(1<<20); \
      psg_noisecounter=(int)floor(bf/af); \
      psg_noisecounter &= (PSG_NOISE_ARRAY-1); \
      bf=fmod(bf,af); \
      psg_noisecountdown=psg_noisemodulo-(int)bf; \
      psg_noisetoggle=psg_noise[psg_noisecounter];

      /*
      if (abc==0) log_write(Str("toneperiod=")+toneperiod+" sound_freq="+sound_freq+" psg_tonemodulo_2="+psg_tonemodulo_2); \
      */

#define PSG_PREPARE_TONE                                 \
      af=((int)toneperiod*sound_freq);                              \
      af*=((double)(1<<17))/15625;                               \
      psg_tonemodulo_2=(int)af; \
      bf=(((DWORD)t)-psg_tone_start_time[abc]); \
      bf*=(double)(1<<21); \
      bf=fmod(bf,af*2); \
      af=bf-af;               \
      if(af>=0){                  \
        psg_tonetoggle=false;       \
        bf=af;                      \
      }                           \
      psg_tonecountdown=psg_tonemodulo_2-(int)bf; \

#define PSG_TONE_ADVANCE                                   \
          psg_tonecountdown-=TWO_MILLION;  \
          while (psg_tonecountdown<0){           \
            psg_tonecountdown+=psg_tonemodulo_2;             \
            psg_tonetoggle=!psg_tonetoggle;                   \
          }

#define PSG_NOISE_ADVANCE                           \
          psg_noisecountdown-=ONE_MILLION;   \
          while (psg_noisecountdown<0){   \
            psg_noisecountdown+=psg_noisemodulo;      \
            psg_noisecounter++;                        \
            if(psg_noisecounter>=PSG_NOISE_ARRAY){      \
              psg_noisecounter=0;                        \
            }                                             \
            psg_noisetoggle=psg_noise[psg_noisecounter];   \
          }

#define PSG_ENVELOPE_ADVANCE                                   \
          psg_envcountdown-=TWO_TO_SEVENTEEN;  \
          while (psg_envcountdown<0){           \
            psg_envcountdown+=psg_envmodulo;             \
            psg_envstage++;                   \
            if (psg_envstage>=32 && envdeath!=-1){                           \
              envvol=envdeath;                                             \
            }else{                                                       \
              envvol=psg_envelope_level[envshape][psg_envstage & 63];            \
            }																															\
          }


  //            envvol=(psg_envstage&255)*64;

void psg_write_buffer(int abc,DWORD to_t)
{
  //buffer starts at time time_of_last_vbl
  //we've written up to psg_buf_pointer[abc]
  //so start at pointer and write to to_t,
  int psg_tonemodulo_2,psg_noisemodulo;
  int psg_tonecountdown,psg_noisecountdown;
  int psg_noisecounter;
  double af,bf;
  bool psg_tonetoggle=true,psg_noisetoggle;
  int *p=psg_channels_buf+psg_buf_pointer[abc];
  DWORD t=(psg_time_of_last_vbl_for_writing+psg_buf_pointer[abc]);
  to_t=max(to_t,t);
  to_t=min(to_t,psg_time_of_last_vbl_for_writing+PSG_CHANNEL_BUF_LENGTH);
  int count=max(min((int)(to_t-t),PSG_CHANNEL_BUF_LENGTH-psg_buf_pointer[abc]),0);
  int toneperiod=(((int)psg_reg[abc*2+1] & 0xf) << 8) + psg_reg[abc*2];

  if ((psg_reg[abc+8] & BIT_4)==0){ // Not Enveloped
    int vol=psg_flat_volume_level[psg_reg[abc+8] & 15];
    if ((psg_reg[PSGR_MIXER] & (1 << abc))==0 && (toneperiod>9)){ //tone enabled
      PSG_PREPARE_TONE
      if ((psg_reg[PSGR_MIXER] & (8 << abc))==0){ //noise enabled

        PSG_PREPARE_NOISE
        for (;count>0;count--){
          if(psg_tonetoggle || psg_noisetoggle){
            p++;
          }else{
            *(p++)+=vol;
          }
          PSG_TONE_ADVANCE
          PSG_NOISE_ADVANCE
        }
      }else{ //tone only
        for (;count>0;count--){
          if(psg_tonetoggle){
            p++;
          }else{
            *(p++)+=vol;
          }
          PSG_TONE_ADVANCE
        }
      }
    }else if ((psg_reg[PSGR_MIXER] & (8 << abc))==0){ //noise enabled
      PSG_PREPARE_NOISE
      for (;count>0;count--){
        if(psg_noisetoggle){
          p++;
        }else{
          *(p++)+=vol;
        }
        PSG_NOISE_ADVANCE
      }

    }else{ //nothing enabled
      for (;count>0;count--){
        *(p++)+=vol;
      }
    }
    psg_buf_pointer[abc]=to_t-psg_time_of_last_vbl_for_writing;
    return;
  }else{  // Enveloped
//    DWORD est64=psg_envelope_start_time*64;
    int envdeath,psg_envstage,envshape;
    int psg_envmodulo,envvol,psg_envcountdown;

    PSG_PREPARE_ENVELOPE;

    if ((psg_reg[PSGR_MIXER] & (1 << abc))==0 && (toneperiod>9)){ //tone enabled
      PSG_PREPARE_TONE
      if ((psg_reg[PSGR_MIXER] & (8 << abc))==0){ //noise enabled
        PSG_PREPARE_NOISE
        for (;count>0;count--){
          if(psg_tonetoggle || psg_noisetoggle){
            p++;
          }else{
            *(p++)+=envvol;
          }
          PSG_TONE_ADVANCE
          PSG_NOISE_ADVANCE
          PSG_ENVELOPE_ADVANCE
        }
      }else{ //tone only
        for (;count>0;count--){
          if(psg_tonetoggle){
            p++;
          }else{
            *(p++)+=envvol;
          }
          PSG_TONE_ADVANCE
          PSG_ENVELOPE_ADVANCE
        }
      }
    }else if ((psg_reg[PSGR_MIXER] & (8 << abc))==0){ //noise enabled
      PSG_PREPARE_NOISE
      for (;count>0;count--){
        if(psg_noisetoggle){
          p++;
        }else{
          *(p++)+=envvol;
        }
        PSG_NOISE_ADVANCE
        PSG_ENVELOPE_ADVANCE
      }
    }else{ //nothing enabled
      for (;count>0;count--){
        *(p++)+=envvol;
        PSG_ENVELOPE_ADVANCE
      }
    }
    psg_buf_pointer[abc]=to_t-psg_time_of_last_vbl_for_writing;
  }
}
//---------------------------------------------------------------------------
DWORD psg_quantize_time(int abc,DWORD t)
{
  int toneperiod=(((int)psg_reg[abc*2+1] & 0xf) << 8) + psg_reg[abc*2];
  if (toneperiod<=1) return t;

  double a,b;
  a=toneperiod*sound_freq;
  a/=(15625*8); //125000
  b=(t-psg_tone_start_time[abc]);
//		b=a-fmod(b,a);
  a=fmod(b,a);
  b-=a;
  t=psg_tone_start_time[abc]+DWORD(b);
  return t;
}

DWORD psg_adjust_envelope_start_time(DWORD t,DWORD new_envperiod)
{
  double b,c;
  int envperiod=max( (((int)psg_reg[PSGR_ENVELOPE_PERIOD_HIGH]) <<8) + psg_reg[PSGR_ENVELOPE_PERIOD_LOW],1);
//  a=envperiod;
//  a*=sound_freq;

  b=(t-psg_envelope_start_time);

  c=b*(double)new_envperiod;
  c/=(double)envperiod;
  c=t-c;         //new env start time

//  a/=7812.5; //that's 2000000/256
//  b+=a;
//  a=fmod(b,a);
//  b-=a;
//  t=psg_envelope_start_time+DWORD(b);


  return DWORD(c);
}

void psg_set_reg(int reg,BYTE old_val,BYTE &new_val)
{
  // suggestions for global variables:  n_samples_per_vbl=sound_freq/shifter_freq,   shifter_y+(SCANLINES_ABOVE_SCREEN+SCANLINES_BELOW_SCREEN)
  if (reg==1 || reg==3 || reg==5 || reg==13){
    new_val&=15;
  }else if (reg==6 || (reg>=8 && reg<=10)){
    new_val&=31;
  }
  if (reg>=PSGR_PORT_A) return;
  if (old_val==new_val && reg!=PSGR_ENVELOPE_SHAPE) return;

  if (psg_capture_file){
    psg_capture_check_boundary();
    DWORD cycle=int(ABSOLUTE_CPU_TIME-psg_capture_cycle_base);
    if (n_millions_cycles_per_sec!=8){
      cycle*=8; // this is safe, max 128000000*8
      cycle/=n_millions_cycles_per_sec;
    }
    BYTE reg_byte=BYTE(reg);

//    log_write(Str("--- cycle=")+cycle+" - reg="+reg_byte+" - val="+new_val);
    fwrite(&cycle,1,sizeof(cycle),psg_capture_file);
    fwrite(&reg_byte,1,sizeof(reg_byte),psg_capture_file);
    fwrite(&new_val,1,sizeof(new_val),psg_capture_file);
  }

  if (SoundActive()==0){
    log(Str("SOUND: ")+HEXSl(old_pc,6)+" - PSG reg "+reg+" changed to "+new_val+" at "+scanline_cycle_log());
    return;
  }

  int cpu_cycles_per_vbl=n_cpu_cycles_per_second/shifter_freq;
#if SCREENS_PER_SOUND_VBL != 1
  cpu_cycles_per_vbl*=SCREENS_PER_SOUND_VBL;
  DWORDLONG a64=(ABSOLUTE_CPU_TIME-cpu_time_of_last_sound_vbl);
#else
  DWORDLONG a64=(ABSOLUTE_CPU_TIME-cpu_time_of_last_vbl);
#endif

  a64*=psg_n_samples_this_vbl;
  a64/=cpu_cycles_per_vbl;
  DWORD t=psg_time_of_last_vbl_for_writing+(DWORD)a64;

  log(EasyStr("SOUND: PSG reg ")+reg+" changed to "+new_val+" at "+scanline_cycle_log()+"; samples "+t+"; vbl was at "+psg_time_of_last_vbl_for_writing);
  switch (reg){
    case 0:case 1:
    case 2:case 3:
    case 4:case 5:
    {
      int abc=reg/2;
      // Freq is double bufferred, it cannot change until the PSG reaches the end of the current square wave.
      // psg_tone_start_time[abc] is set to the last end of wave, so if it is in future don't do anything.
      // Overflow will be a problem, however at 50Khz that will take a day of non-stop output.
      if (t>psg_tone_start_time[abc]){
        t=psg_quantize_time(abc,t);
        psg_write_buffer(abc,t);
        psg_tone_start_time[abc]=t;
      }
      break;
    }
    case 6:  //changed noise
      psg_write_buffer(0,t);
      psg_write_buffer(1,t);
      psg_write_buffer(2,t);
      break;
    case 7:  //mixer
//      new_val|=b00111110;

      psg_write_buffer(0,t);
      psg_write_buffer(1,t);
      psg_write_buffer(2,t);
      break;
    case 8:case 9:case 10:  //channel A,B,C volume
//      new_val&=0xf;

      // ST doesn't quantize, it changes the level straight away.
//        t=psg_quantize_time(reg-8,t);
      psg_write_buffer(reg-8,t);
//        psg_tone_start_time[reg-8]=t;
      break;
    case 11: //changing envelope period low
    {
      psg_write_buffer(0,t);
      psg_write_buffer(1,t);
      psg_write_buffer(2,t);
      int new_envperiod=max( (((int)psg_reg[PSGR_ENVELOPE_PERIOD_HIGH]) <<8) + new_val,1);
      psg_envelope_start_time=psg_adjust_envelope_start_time(t,new_envperiod);
      break;
    }
    case 12: //changing envelope period high
    {
      psg_write_buffer(0,t);
      psg_write_buffer(1,t);
      psg_write_buffer(2,t);
      int new_envperiod=max( (((int)new_val) <<8) + psg_reg[PSGR_ENVELOPE_PERIOD_LOW],1);
      psg_envelope_start_time=psg_adjust_envelope_start_time(t,new_envperiod);
      break;
    }
    case 13: //envelope shape
    {
/*
      DWORD abc_t[3]={t,t,t};
      for (int abc=0;abc<3;abc++){
        if (psg_reg[8+abc] & 16) abc_t[abc]=psg_quantize_time(abc,t);
      }
*/
//        t=psg_quantize_envelope_time(t,0,&new_envelope_start_time);
      psg_write_buffer(0,t);
      psg_write_buffer(1,t);
      psg_write_buffer(2,t);
      psg_envelope_start_time=t;
/*
      for (int abc=0;abc<3;abc++){
        if (psg_reg[8+abc] & 16) psg_tone_start_time[abc]=abc_t[abc];
      }
*/
      break;
    }
  }
}

void psg_capture(bool start,Str file)
{
  if (psg_capture_file){
    fclose(psg_capture_file);
    psg_capture_file=NULL;
  }
  if (start){
    psg_capture_file=fopen(file,"wb");
    if (psg_capture_file){
      WORD magic=0x2149;
      DWORD data_start=sizeof(WORD)+sizeof(DWORD)+sizeof(WORD)+sizeof(BYTE)*14;
      WORD version=1;

      fwrite(&magic,1,sizeof(magic),psg_capture_file);
      fwrite(&data_start,1,sizeof(data_start),psg_capture_file);
      fwrite(&version,1,sizeof(version),psg_capture_file);
      fwrite(psg_reg,14,sizeof(BYTE),psg_capture_file);

      psg_capture_cycle_base=ABSOLUTE_CPU_TIME;
    }
  }
}

void psg_capture_check_boundary()
{
  if (int(ABSOLUTE_CPU_TIME-psg_capture_cycle_base)>=(int)n_cpu_cycles_per_second){
    psg_capture_cycle_base+=n_cpu_cycles_per_second;

//    log_write(Str("--- second boundary"));
    DWORD cycle=0;
    BYTE reg_byte=0xff;
    BYTE new_val=0xff;
    fwrite(&cycle,1,sizeof(cycle),psg_capture_file);
    fwrite(&reg_byte,1,sizeof(reg_byte),psg_capture_file);
    fwrite(&new_val,1,sizeof(new_val),psg_capture_file);
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#undef LOGSECTION

