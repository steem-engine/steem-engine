#ifdef IN_EMU
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)
#endif

#define TB_TIME_WOBBLE (rand() & 4)

extern "C" void ASMCALL check_for_interrupts_pending();

EXT BYTE mfp_reg[24];
EXT BYTE mfp_gpip_no_interrupt INIT(0xf7);

#define MFPR_GPIP 0
#define MFPR_AER 1
#define MFPR_DDR 2
#define MFPR_IERA 3
#define MFPR_IERB 4
#define MFPR_IPRA 5
#define MFPR_IPRB 6
#define MFPR_ISRA 7
#define MFPR_ISRB 8
#define MFPR_IMRA 9
#define MFPR_IMRB 10
#define MFPR_VR 11
#define MFPR_TACR 12
#define MFPR_TBCR 13
#define MFPR_TCDCR 14
#define MFPR_TADR 15
#define MFPR_TBDR 16
#define MFPR_TCDR 17
#define MFPR_TDDR 18
#define MFPR_SCR 19
#define MFPR_UCR 20
#define MFPR_RSR 21
#define MFPR_TSR 22
#define MFPR_UDR 23

#define MFP_INT_MONOCHROME_MONITOR_DETECT 15
#define MFP_INT_RS232_RING_INDICATOR 14
#define MFP_INT_TIMER_A 13
#define MFP_INT_RS232_RECEIVE_BUFFER_FULL 12
#define MFP_INT_RS232_RECEIVE_ERROR 11
#define MFP_INT_RS232_TRANSMIT_BUFFER_EMPTY 10
#define MFP_INT_RS232_TRANSMIT_ERROR 9
#define MFP_INT_TIMER_B 8
#define MFP_INT_FDC_AND_DMA 7
#define MFP_INT_ACIA 6  // Vector at $118
#define MFP_INT_TIMER_C 5
#define MFP_INT_TIMER_D 4
#define MFP_INT_BLITTER 3
#define MFP_INT_RS232_CTS 2
#define MFP_INT_RS232_DCD 1
#define MFP_INT_CENTRONICS_BUSY 0

#define MFP_GPIP_COLOUR BYTE(0x80)
#define MFP_GPIP_NOT_COLOUR BYTE(~0x80)
#define MFP_GPIP_CTS BYTE(BIT_2)
#define MFP_GPIP_DCD BYTE(BIT_1)
#define MFP_GPIP_RING BYTE(BIT_6)

#define MFP_GPIP_CENTRONICS_BIT 0
#define MFP_GPIP_DCD_BIT 1
#define MFP_GPIP_CTS_BIT 2
#define MFP_GPIP_BLITTER_BIT 3
#define MFP_GPIP_ACIA_BIT 4
#define MFP_GPIP_FDC_BIT 5
#define MFP_GPIP_RING_BIT 6
#define MFP_GPIP_MONO_BIT 7

#ifdef IN_EMU

BYTE mfp_gpip_input_buffer=0;

#define MFP_CLK 2451

#define MFP_CLK_EXACT 2451134 // Between 2451168 and 2451226 cycles

// Number of MFP clock ticks per 8000000 CPU cycles, very accurately tested!
// This is the most accurate number but we use the one above because Lethal Xcess
// won't work with this one.
//#define MFP_CLK_EXACT 2451182 // Between 2451168 and 2451226 cycles

//#define MFP_CLK_EXACT 2451034 // Between 2450992 and 2451050 (erring high) old version before 12 cycle delay

//#define MFP_CLK_EXACT 2450780  old version, checked inaccurately
/*
#define MFP_CLK 2457
#define MFP_CLK_EXACT 2457600
*/

#if defined(STEVEN_SEAGAL) && defined(SS_MFP_RATIO)
// it's a variable! See SSE.h
#else
#define CPU_CYCLES_PER_MFP_CLK (8000000.0/double(MFP_CLK_EXACT))
#endif

#define CYCLES_FROM_START_OF_MFP_IRQ_TO_WHEN_PEND_IS_CLEARED 20
//#define CYCLES_FROM_START_OF_MFP_IRQ_TO_WHEN_PEND_IS_CLEARED 20
#define MFP_S_BIT (mfp_reg[MFPR_VR] & BIT_3)

//const int mfp_io_port_bit[16]={BIT_0,BIT_1,BIT_2,BIT_3,-1,-1,BIT_4,BIT_5,-1,-1,-1,-1,-1,-1,BIT_6,BIT_7};

inline BYTE mfp_get_timer_control_register(int);
const int mfp_timer_8mhz_prescale[16]={65535,4,10,16,50,64,100,200,65535,4,10,16,50,64,100,200};
const int mfp_timer_irq[4]={13,8,5,4};
void calc_time_of_next_timer_b();

int mfp_timer_prescale[16]={65535,4,10,16,50,64,100,200,65535,4,10,16,50,64,100,200};

int mfp_timer_counter[4];
int mfp_timer_timeout[4];
bool mfp_timer_enabled[4]={0,0,0,0};
int mfp_timer_period[4]={10000,10000,10000,10000};
bool mfp_timer_period_change[4]={0,0,0,0};
//int mfp_timer_prescale_counter[4]={0,0,0,0};

void mfp_set_timer_reg(int,BYTE,BYTE);
int mfp_calc_timer_counter(int);
void mfp_init_timers();
inline bool mfp_set_pending(int,int);

void mfp_gpip_set_bit(int,bool);

const int mfp_gpip_irq[8]={0,1,2,3,6,7,14,15};

//int mfp_gpip_timeout;

bool mfp_interrupt_enabled[16];
int mfp_time_of_start_of_last_interrupt[16];

int cpu_time_of_first_mfp_tick;


#define MFP_CALC_INTERRUPTS_ENABLED	\
{	\
  int mask=1;	\
  for (int n=0;n<8;n++){	\
    mfp_interrupt_enabled[n]=mfp_reg[MFPR_IERB] & mask; mask<<=1;	\
  }	\
  mask=1;	\
  for (int n=0;n<8;n++){	\
    mfp_interrupt_enabled[n+8]=mfp_reg[MFPR_IERA] & mask; mask<<=1;	\
  }	\
}

#define MFP_CALC_TIMERS_ENABLED	\
	for (int n=0;n<4;n++){	\
		mfp_timer_enabled[n]=mfp_interrupt_enabled[mfp_timer_irq[n]] && (mfp_get_timer_control_register(n) & 7);	\
	}

//int mfp_timer_tick_countdown[4];
void mfp_interrupt(int,int);
//bool mfp_interrupt_enabled(int irq);
void mfp_gpip_transition(int,bool);
void mfp_check_for_timer_timeouts(); // SS not defined

#define MFP_CALC_TIMER_PERIOD(t)  mfp_timer_period[t]=int(  \
          double(mfp_timer_prescale[mfp_get_timer_control_register(t)]* \
            int(BYTE_00_TO_256(mfp_reg[MFPR_TADR+t])))*CPU_CYCLES_PER_MFP_CLK);

#define mfp_interrupt_i_bit(irq) (BYTE(1 << (irq & 7)))
#define mfp_interrupt_i_ab(irq) (1-((irq & 8) >> 3))

#define mfp_interrupt_pend(irq,when_fired)                                       \
  if (mfp_interrupt_enabled[irq]){                             \
    LOG_ONLY( bool done= ) mfp_set_pending(irq,when_fired);   \
    LOG_ONLY( if (done==0) log_to(LOGSECTION_MFP_TIMERS,EasyStr("INTERRUPT: MFP IRQ #")+irq+" ("+    \
                                (char*)name_of_mfp_interrupt[irq]+") - can't set pending as MFP cleared "  \
                                "pending after timeout"); )             \
  } 

#endif

#undef EXT
#undef INIT

