#ifdef IN_EMU
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)
#endif

/*  tone frequency
     fT = fMaster
          -------
           16TP


ie. period = 16TP / 2 000 000
= TP/125000

 = TP / (8*15625)


 the envelope repetition frequency fE is obtained as follows from the
envelope setting period value EP (decimal):

       fE = fMaster        (fMaster if the frequency of the master clock)
            -------
             256EP

The period of the actual frequency fEA used for the envelope generated is
1/32 of the envelope repetition period (1/fE).

ie. period of whole envelope = 256EP/2 000 000 = 2*EP / 15625 (think this is one go through, eg. /)
Period of envelope change frequency is 1/32 of this, so EP/ 16*15625
Scale by 64 and multiply by frequency of buffer to get final period, 4*EP/15625

*/

/*      New scalings - 6/7/01

__TONE__  Period = TP/(8*15625) Hz = (TP*sound_freq)/(8*15625) samples
          Period in samples goes up to 1640.6.  Want this to correspond to 2000M, so
          scale up by 2^20 = 1M.  So period to initialise counter is
          (TP*sound_freq*2^17)/15625, which ranges up to 1.7 thousand million.  The
          countdown each time is 2^20, but we double this to 2^21 so that we can
          spot each half of the period.  To initialise the countdown, bit 0 of
          (time*2^21)/tonemodulo gives high or low.  The counter is initialised
          to tonemodulo-(time*2^21 mod tonemodulo).

__NOISE__ fudged similarly to tone.

__ENV__   Step period = 2EP/(15625*32) Hz.  Scale by 2^17 to scale 13124.5 to 1.7 thousand
          million.  Step period is (EP*sound_freq*2^13)/15625.

*/

#define SOUND_DESIRED_LQ_FREQ (50066/2)

#ifdef ENABLE_VARIABLE_SOUND_DAMPING
EXT int sound_variable_a INIT(32);
EXT int sound_variable_d INIT(208);
#endif

#define SOUND_MODE_MUTE         0
#define SOUND_MODE_CHIP         1
#define SOUND_MODE_EMULATED     2
#define SOUND_MODE_SHARPSAMPLES 3

EXT bool sound_internal_speaker INIT(false);
EXT int sound_freq INIT(50066),sound_comline_freq INIT(0),sound_chosen_freq INIT(50066);
EXT int sound_mode INIT(SOUND_MODE_CHIP),sound_last_mode INIT(SOUND_MODE_CHIP);
EXT BYTE sound_num_channels INIT(1),sound_num_bits INIT(8);
EXT int sound_bytes_per_sample INIT(1);
EXT DWORD MaxVolume INIT(0xffff);
EXT bool sound_low_quality INIT(0);
EXT bool sound_write_primary INIT( NOT_ONEGAME(0) ONEGAME_ONLY(true) );
EXT bool sound_click_at_start INIT(0);
EXT int sound_time_method INIT(0);
EXT bool sound_record INIT(false);
EXT DWORD sound_record_start_time; //by timer variable = timeGetTime()
EXT int psg_write_n_screens_ahead INIT(3 UNIX_ONLY(+7) );

EXT void SoundStopInternalSpeaker();

EXT int psg_voltage,psg_dv;

#define PSGR_PORT_A 14
#define PSGR_PORT_B 15

EXT int psg_reg_select;
EXT BYTE psg_reg[16],psg_reg_data;

EXT FILE *wav_file INIT(NULL);

#if defined(ENABLE_LOGFILE) || defined(SHOW_WAVEFORM)
EXT DWORD min_write_time;
EXT DWORD play_cursor,write_cursor;
#endif

#define DEFAULT_SOUND_BUFFER_LENGTH (32768*SCREENS_PER_SOUND_VBL)
EXT int sound_buffer_length INIT(DEFAULT_SOUND_BUFFER_LENGTH);
EXT DWORD SoundBufStartTime;

#define PSG_BUF_LENGTH sound_buffer_length

#if SCREENS_PER_SOUND_VBL == 1
//#define MOD_PSG_BUF_LENGTH &(PSG_BUF_LENGTH-1)
#define MOD_PSG_BUF_LENGTH %PSG_BUF_LENGTH
#else
#define MOD_PSG_BUF_LENGTH %PSG_BUF_LENGTH
EXT int cpu_time_of_last_sound_vbl INIT(0);
#endif

EXT DWORD psg_last_play_cursor;
EXT DWORD psg_last_write_time;
EXT DWORD psg_time_of_start_of_buffer;
EXT DWORD psg_time_of_last_vbl_for_writing,psg_time_of_next_vbl_for_writing;
EXT int psg_n_samples_this_vbl;

EXT WORD dma_sound_last_sample_l,dma_sound_last_sample_r;

#ifdef SHOW_WAVEFORM

EXT int temp_waveform_display_counter;
#define MAX_temp_waveform_display_counter PSG_BUF_LENGTH
EXT BYTE temp_waveform_display[DEFAULT_SOUND_BUFFER_LENGTH];
EXT void draw_waveform();
EXT DWORD temp_waveform_play_counter;
//#define TEMP_WAVEFORM_INTERVAL 31

#endif


#ifdef IN_EMU

#define ONE_MILLION 1048576
#define TWO_MILLION 2097152
//two to the power of 21
#define TWO_TO_SIXTEEN 65536
#define TWO_TO_SEVENTEEN 131072
#define TWO_TO_EIGHTEEN 262144

bool sound_first_vbl=0;

void sound_record_to_wav(int,DWORD,bool,int*);
//---------------------------------------------------------------------------
HRESULT Sound_VBL();
//--------------------------------------------------------------------------- DMA Sound
void event_dma_sound_hit_end();
void dma_sound_prepare_for_end(MEM_ADDRESS,int,bool);

BYTE dma_sound_control,dma_sound_mode;
MEM_ADDRESS dma_sound_start=0,next_dma_sound_start=0,
            dma_sound_end=0,next_dma_sound_end=0;

int dma_sound_start_time;

WORD MicroWire_Mask=0x07ff;
WORD MicroWire_Data=0;
int MicroWire_StartTime=0;
#define CPU_CYCLES_PER_MW_SHIFT 8

void dma_sound_write_to_buffer(int);

const double dma_sound_mode_to_cycles_per_byte_stereo_8mhz[4]={8000000.0 / (6540.0*2.0),
                                                               8000000.0 / (12800.0*2.0),
                                                               8000000.0 / (25180.0*2.0),
                                                               8000000.0 / (50066.0*2.0)};

const double dma_sound_mode_to_cycles_per_byte_mono_8mhz[4]={8000000.0 / 6900.0,
                                                             8000000.0 / 13140.0,
                                                             8000000.0 / 25600.0,
                                                             8000000.0 / 50360.0};
/*
const double dma_sound_mode_to_cycles_per_byte_stereo_8mhz[4]={8000000.0 / (6258.0*2.0),
                                                               8000000.0 / (12517.0*2.0),
                                                               8000000.0 / (25033.0*2.0),
                                                               8000000.0 / (50066.0*2.0)};

const double dma_sound_mode_to_cycles_per_byte_mono_8mhz[4]={8000000.0 / 6258.0,
                                                             8000000.0 / 12517.0,
                                                             8000000.0 / 25033.0,
                                                             8000000.0 / 50066.0};
*/

double dma_sound_mode_to_cycles_per_byte_stereo[4]={dma_sound_mode_to_cycles_per_byte_stereo_8mhz[0],
                                                    dma_sound_mode_to_cycles_per_byte_stereo_8mhz[1],
                                                    dma_sound_mode_to_cycles_per_byte_stereo_8mhz[2],
                                                    dma_sound_mode_to_cycles_per_byte_stereo_8mhz[3]};

double dma_sound_mode_to_cycles_per_byte_mono[4]={dma_sound_mode_to_cycles_per_byte_mono_8mhz[0],
                                                    dma_sound_mode_to_cycles_per_byte_mono_8mhz[1],
                                                    dma_sound_mode_to_cycles_per_byte_mono_8mhz[2],
                                                    dma_sound_mode_to_cycles_per_byte_mono_8mhz[3]};


int dma_sound_mode_to_freq[2][4]={/*stereo*/{6540,12800,25180,50066},
                                    /*mono*/{6900,13140,25600,50360}};

//int dma_sound_mode_to_freq[2][4]={/*stereo*/{6258,12517,25033,50066},
//                                    /*mono*/{6258,12517,25033,50066}};
int dma_sound_freq,dma_sound_countdown;

// Max frequency/lowest refresh *2 for stereo
#define DMA_SOUND_BUFFER_LENGTH (((int(double(100000/50)*1.3) & ~3)*SCREENS_PER_SOUND_VBL)*2)
WORD dma_sound_channel_buf[DMA_SOUND_BUFFER_LENGTH+16];
int dma_sound_end_cpu_time;
DWORD dma_sound_channel_buf_last_write_t;
MEM_ADDRESS dma_sound_addr_to_read_next;

#define DMA_SOUND_CHECK_TIMER_A \
    if (mfp_reg[MFPR_TACR]==8){ \
      mfp_timer_counter[0]-=64;  \
      if (mfp_timer_counter[0]<64){ \
        mfp_timer_counter[0]=BYTE_00_TO_256(mfp_reg[MFPR_TADR])*64; \
        mfp_interrupt_pend(MFP_INT_TIMER_A,ABSOLUTE_CPU_TIME); \
      }                                 \
    }

int dma_sound_mixer=1,dma_sound_volume=40;
int dma_sound_l_volume=20,dma_sound_r_volume=20;
int dma_sound_l_top_val=128,dma_sound_r_top_val=128;

//---------------------------------- PSG ------------------------------------

void psg_write_buffer(int,DWORD);

//make constant for dimming waveform display

//#define PSG_BUF_LENGTH (32768*SCREENS_PER_SOUND_VBL)

#define PSG_NOISE_ARRAY 8192
#define MOD_PSG_NOISE_ARRAY & 8191

#ifndef ONEGAME
#define PSG_WRITE_EXTRA 300
#else
#define PSG_WRITE_EXTRA OGExtraSamplesPerVBL
#endif

//#define PSG_WRITE_EXTRA 10

//#define PSG_CHANNEL_BUF_LENGTH (2048*SCREENS_PER_SOUND_VBL)
#define PSG_CHANNEL_BUF_LENGTH (8192*SCREENS_PER_SOUND_VBL)

#define VOLTAGE_ZERO_LEVEL 0
#define VOLTAGE_FIXED_POINT 256
//must now be fixed at 256!
#define VOLTAGE_FP(x) ((x) << 8)

//BYTE psg_channel_buf[3][PSG_CHANNEL_BUF_LENGTH];
int psg_channels_buf[PSG_CHANNEL_BUF_LENGTH+16];
int psg_buf_pointer[3];
DWORD psg_tone_start_time[3];


//DWORD psg_tonemodulo_2,psg_noisemodulo;
//const short volscale[16]=  {0,1,1,2,3,4,5,7,12,20,28,44,70,110,165,255};

char psg_noise[PSG_NOISE_ARRAY];



#define PSGR_NOISE_PERIOD 6
#define PSGR_MIXER 7
#define PSGR_AMPLITUDE_A 8
#define PSGR_AMPLITUDE_B 9
#define PSGR_AMPLITUDE_C 10
#define PSGR_ENVELOPE_PERIOD 11
#define PSGR_ENVELOPE_PERIOD_LOW 11
#define PSGR_ENVELOPE_PERIOD_HIGH 12
#define PSGR_ENVELOPE_SHAPE 13

#define PSG_ENV_SHAPE_HOLD BIT_0
#define PSG_ENV_SHAPE_ALT BIT_1
#define PSG_ENV_SHAPE_ATTACK BIT_2
#define PSG_ENV_SHAPE_CONT BIT_3

#define PSG_CHANNEL_AMPLITUDE 60

//#define PSG_VOLSCALE(vl) (volscale[vl]/4+VOLTAGE_ZERO_LEVEL)
/*
#define PSG_ENV_DOWN 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
#define PSG_ENV_UP   00,01,02,03,04,05,6,7,8,9,10,11,12,13,14,15
#define PSG_ENV_0    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
#define PSG_ENV_LOUD 15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15

const BYTE psg_envelopes[8][32]={
    {PSG_ENV_DOWN,PSG_ENV_DOWN},
    {PSG_ENV_DOWN,PSG_ENV_0},
    {PSG_ENV_DOWN,PSG_ENV_UP},
    {PSG_ENV_DOWN,PSG_ENV_LOUD},
    {PSG_ENV_UP,PSG_ENV_UP},
    {PSG_ENV_UP,PSG_ENV_LOUD},
    {PSG_ENV_UP,PSG_ENV_DOWN},
    {PSG_ENV_UP,PSG_ENV_0}};

*/
#define VFP VOLTAGE_FIXED_POINT
#define VZL VOLTAGE_ZERO_LEVEL
#define VA VFP*PSG_CHANNEL_AMPLITUDE
const int psg_flat_volume_level[16]={0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,5*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,50*VA/1000+VZL*VFP,65*VA/1000+VZL*VFP,80*VA/1000+VZL*VFP,100*VA/1000+VZL*VFP,125*VA/1000+VZL*VFP,178*VA/1000+VZL*VFP,250*VA/1000+VZL*VFP,354*VA/1000+VZL*VFP,510*VA/1000+VZL*VFP,707*VA/1000+VZL*VFP,1000*VA/1000+VZL*VFP};

const int psg_envelope_level[8][64]={
    {1000*VA/1000+VZL*VFP,841*VA/1000+VZL*VFP,707*VA/1000+VZL*VFP,590*VA/1000+VZL*VFP,510*VA/1000+VZL*VFP,420*VA/1000+VZL*VFP,354*VA/1000+VZL*VFP,290*VA/1000+VZL*VFP,250*VA/1000+VZL*VFP,210*VA/1000+VZL*VFP,178*VA/1000+VZL*VFP,149*VA/1000+VZL*VFP,125*VA/1000+VZL*VFP,110*VA/1000+VZL*VFP,100*VA/1000+VZL*VFP,88*VA/1000+VZL*VFP,80*VA/1000+VZL*VFP,70*VA/1000+VZL*VFP,65*VA/1000+VZL*VFP,55*VA/1000+VZL*VFP,50*VA/1000+VZL*VFP,30*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,5*VA/1000+VZL*VFP,3*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,1*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,
    1000*VA/1000+VZL*VFP,841*VA/1000+VZL*VFP,707*VA/1000+VZL*VFP,590*VA/1000+VZL*VFP,510*VA/1000+VZL*VFP,420*VA/1000+VZL*VFP,354*VA/1000+VZL*VFP,290*VA/1000+VZL*VFP,250*VA/1000+VZL*VFP,210*VA/1000+VZL*VFP,178*VA/1000+VZL*VFP,149*VA/1000+VZL*VFP,125*VA/1000+VZL*VFP,110*VA/1000+VZL*VFP,100*VA/1000+VZL*VFP,88*VA/1000+VZL*VFP,80*VA/1000+VZL*VFP,70*VA/1000+VZL*VFP,65*VA/1000+VZL*VFP,55*VA/1000+VZL*VFP,50*VA/1000+VZL*VFP,30*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,5*VA/1000+VZL*VFP,3*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,1*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP},
    {1000*VA/1000+VZL*VFP,841*VA/1000+VZL*VFP,707*VA/1000+VZL*VFP,590*VA/1000+VZL*VFP,510*VA/1000+VZL*VFP,420*VA/1000+VZL*VFP,354*VA/1000+VZL*VFP,290*VA/1000+VZL*VFP,250*VA/1000+VZL*VFP,210*VA/1000+VZL*VFP,178*VA/1000+VZL*VFP,149*VA/1000+VZL*VFP,125*VA/1000+VZL*VFP,110*VA/1000+VZL*VFP,100*VA/1000+VZL*VFP,88*VA/1000+VZL*VFP,80*VA/1000+VZL*VFP,70*VA/1000+VZL*VFP,65*VA/1000+VZL*VFP,55*VA/1000+VZL*VFP,50*VA/1000+VZL*VFP,30*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,5*VA/1000+VZL*VFP,3*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,1*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,
    VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP},
    {1000*VA/1000+VZL*VFP,841*VA/1000+VZL*VFP,707*VA/1000+VZL*VFP,590*VA/1000+VZL*VFP,510*VA/1000+VZL*VFP,420*VA/1000+VZL*VFP,354*VA/1000+VZL*VFP,290*VA/1000+VZL*VFP,250*VA/1000+VZL*VFP,210*VA/1000+VZL*VFP,178*VA/1000+VZL*VFP,149*VA/1000+VZL*VFP,125*VA/1000+VZL*VFP,110*VA/1000+VZL*VFP,100*VA/1000+VZL*VFP,88*VA/1000+VZL*VFP,80*VA/1000+VZL*VFP,70*VA/1000+VZL*VFP,65*VA/1000+VZL*VFP,55*VA/1000+VZL*VFP,50*VA/1000+VZL*VFP,30*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,5*VA/1000+VZL*VFP,3*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,1*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,
    0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,1*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,3*VA/1000+VZL*VFP,5*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,30*VA/1000+VZL*VFP,50*VA/1000+VZL*VFP,55*VA/1000+VZL*VFP,65*VA/1000+VZL*VFP,70*VA/1000+VZL*VFP,80*VA/1000+VZL*VFP,88*VA/1000+VZL*VFP,100*VA/1000+VZL*VFP,110*VA/1000+VZL*VFP,125*VA/1000+VZL*VFP,149*VA/1000+VZL*VFP,178*VA/1000+VZL*VFP,210*VA/1000+VZL*VFP,250*VA/1000+VZL*VFP,290*VA/1000+VZL*VFP,354*VA/1000+VZL*VFP,420*VA/1000+VZL*VFP,510*VA/1000+VZL*VFP,590*VA/1000+VZL*VFP,707*VA/1000+VZL*VFP,841*VA/1000+VZL*VFP,1000*VA/1000+VZL*VFP},
    {1000*VA/1000+VZL*VFP,841*VA/1000+VZL*VFP,707*VA/1000+VZL*VFP,590*VA/1000+VZL*VFP,510*VA/1000+VZL*VFP,420*VA/1000+VZL*VFP,354*VA/1000+VZL*VFP,290*VA/1000+VZL*VFP,250*VA/1000+VZL*VFP,210*VA/1000+VZL*VFP,178*VA/1000+VZL*VFP,149*VA/1000+VZL*VFP,125*VA/1000+VZL*VFP,110*VA/1000+VZL*VFP,100*VA/1000+VZL*VFP,88*VA/1000+VZL*VFP,80*VA/1000+VZL*VFP,70*VA/1000+VZL*VFP,65*VA/1000+VZL*VFP,55*VA/1000+VZL*VFP,50*VA/1000+VZL*VFP,30*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,5*VA/1000+VZL*VFP,3*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,1*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,
    VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP},
    {0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,1*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,3*VA/1000+VZL*VFP,5*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,30*VA/1000+VZL*VFP,50*VA/1000+VZL*VFP,55*VA/1000+VZL*VFP,65*VA/1000+VZL*VFP,70*VA/1000+VZL*VFP,80*VA/1000+VZL*VFP,88*VA/1000+VZL*VFP,100*VA/1000+VZL*VFP,110*VA/1000+VZL*VFP,125*VA/1000+VZL*VFP,149*VA/1000+VZL*VFP,178*VA/1000+VZL*VFP,210*VA/1000+VZL*VFP,250*VA/1000+VZL*VFP,290*VA/1000+VZL*VFP,354*VA/1000+VZL*VFP,420*VA/1000+VZL*VFP,510*VA/1000+VZL*VFP,590*VA/1000+VZL*VFP,707*VA/1000+VZL*VFP,841*VA/1000+VZL*VFP,1000*VA/1000+VZL*VFP,
    0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,1*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,3*VA/1000+VZL*VFP,5*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,30*VA/1000+VZL*VFP,50*VA/1000+VZL*VFP,55*VA/1000+VZL*VFP,65*VA/1000+VZL*VFP,70*VA/1000+VZL*VFP,80*VA/1000+VZL*VFP,88*VA/1000+VZL*VFP,100*VA/1000+VZL*VFP,110*VA/1000+VZL*VFP,125*VA/1000+VZL*VFP,149*VA/1000+VZL*VFP,178*VA/1000+VZL*VFP,210*VA/1000+VZL*VFP,250*VA/1000+VZL*VFP,290*VA/1000+VZL*VFP,354*VA/1000+VZL*VFP,420*VA/1000+VZL*VFP,510*VA/1000+VZL*VFP,590*VA/1000+VZL*VFP,707*VA/1000+VZL*VFP,841*VA/1000+VZL*VFP,1000*VA/1000+VZL*VFP},
    {0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,1*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,3*VA/1000+VZL*VFP,5*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,30*VA/1000+VZL*VFP,50*VA/1000+VZL*VFP,55*VA/1000+VZL*VFP,65*VA/1000+VZL*VFP,70*VA/1000+VZL*VFP,80*VA/1000+VZL*VFP,88*VA/1000+VZL*VFP,100*VA/1000+VZL*VFP,110*VA/1000+VZL*VFP,125*VA/1000+VZL*VFP,149*VA/1000+VZL*VFP,178*VA/1000+VZL*VFP,210*VA/1000+VZL*VFP,250*VA/1000+VZL*VFP,290*VA/1000+VZL*VFP,354*VA/1000+VZL*VFP,420*VA/1000+VZL*VFP,510*VA/1000+VZL*VFP,590*VA/1000+VZL*VFP,707*VA/1000+VZL*VFP,841*VA/1000+VZL*VFP,1000*VA/1000+VZL*VFP,
    VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP},
    {0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,1*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,3*VA/1000+VZL*VFP,5*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,30*VA/1000+VZL*VFP,50*VA/1000+VZL*VFP,55*VA/1000+VZL*VFP,65*VA/1000+VZL*VFP,70*VA/1000+VZL*VFP,80*VA/1000+VZL*VFP,88*VA/1000+VZL*VFP,100*VA/1000+VZL*VFP,110*VA/1000+VZL*VFP,125*VA/1000+VZL*VFP,149*VA/1000+VZL*VFP,178*VA/1000+VZL*VFP,210*VA/1000+VZL*VFP,250*VA/1000+VZL*VFP,290*VA/1000+VZL*VFP,354*VA/1000+VZL*VFP,420*VA/1000+VZL*VFP,510*VA/1000+VZL*VFP,590*VA/1000+VZL*VFP,707*VA/1000+VZL*VFP,841*VA/1000+VZL*VFP,1000*VA/1000+VZL*VFP,
    1000*VA/1000+VZL*VFP,841*VA/1000+VZL*VFP,707*VA/1000+VZL*VFP,590*VA/1000+VZL*VFP,510*VA/1000+VZL*VFP,420*VA/1000+VZL*VFP,354*VA/1000+VZL*VFP,290*VA/1000+VZL*VFP,250*VA/1000+VZL*VFP,210*VA/1000+VZL*VFP,178*VA/1000+VZL*VFP,149*VA/1000+VZL*VFP,125*VA/1000+VZL*VFP,110*VA/1000+VZL*VFP,100*VA/1000+VZL*VFP,88*VA/1000+VZL*VFP,80*VA/1000+VZL*VFP,70*VA/1000+VZL*VFP,65*VA/1000+VZL*VFP,55*VA/1000+VZL*VFP,50*VA/1000+VZL*VFP,30*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,5*VA/1000+VZL*VFP,3*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,1*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP},
    {0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,1*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,3*VA/1000+VZL*VFP,5*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,30*VA/1000+VZL*VFP,50*VA/1000+VZL*VFP,55*VA/1000+VZL*VFP,65*VA/1000+VZL*VFP,70*VA/1000+VZL*VFP,80*VA/1000+VZL*VFP,88*VA/1000+VZL*VFP,100*VA/1000+VZL*VFP,110*VA/1000+VZL*VFP,125*VA/1000+VZL*VFP,149*VA/1000+VZL*VFP,178*VA/1000+VZL*VFP,210*VA/1000+VZL*VFP,250*VA/1000+VZL*VFP,290*VA/1000+VZL*VFP,354*VA/1000+VZL*VFP,420*VA/1000+VZL*VFP,510*VA/1000+VZL*VFP,590*VA/1000+VZL*VFP,707*VA/1000+VZL*VFP,841*VA/1000+VZL*VFP,1000*VA/1000+VZL*VFP,
    VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP}};

#undef VFP
#undef VZL
#undef VA

DWORD psg_quantize_time(int,DWORD);
void psg_set_reg(int,BYTE,BYTE&);

DWORD psg_envelope_start_time=0xfffff000;
//---------------------------------------------------------------------------

#endif

#undef EXT
#undef INIT

