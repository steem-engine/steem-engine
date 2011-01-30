#ifdef IN_EMU
#define EXT
#define EXTC
#define INIT(s) =s
#else
#define EXT extern
#define EXTC extern "C"
#define INIT(s)
#endif

#define BORDER_TOP 30
#define BORDER_BOTTOM 40
#define BORDER_SIDE 32

#define DFSM_FLIP 0
#define DFSM_STRAIGHTBLIT 1
#define DFSM_STRETCHBLIT 2
#define DFSM_LAPTOP 3

#define DFSFX_NONE 0
#define DFSFX_GRILLE 1
#define DFSFX_BLUR 2

#define DFSM_LAPTOP 3

EXT bool draw_routines_init();

EXT void init_screen();

EXT void draw_begin();
EXT void draw_end();
EXT bool draw_blit();
EXT void draw_set_jumps_and_source();

EXT void draw(bool);

EXT HRESULT change_fullscreen_display_mode(bool resizeclippingwindow);
EXT void change_window_size_for_border_change(int oldborder,int newborder);

EXT void res_change();

EXT int bad_drawing INIT(0);
EXT int draw_fs_blit_mode INIT( UNIX_ONLY(DFSM_STRAIGHTBLIT) WIN_ONLY(DFSM_STRETCHBLIT) );
EXT int draw_fs_fx INIT(DFSFX_NONE),draw_grille_black INIT(6);
EXT RECT draw_blit_source_rect;
EXT int draw_fs_topgap INIT(0);

#define DWM_STRETCH 0
#define DWM_NOSTRETCH 1
#define DWM_GRILLE 2
WIN_ONLY( EXT int draw_win_mode[2]; ) // Inited by draw_fs_blit_mode


EXT bool FullScreen INIT(0);
EXT bool draw_lock;

extern "C"
{
EXT BYTE *draw_mem;
EXT int draw_line_length;
EXT long *PCpal;
EXT WORD STpal[16];
EXT BYTE *draw_dest_ad,*draw_dest_next_scanline;
}

#define OVERSCAN_MAX_COUNTDOWN 25

EXT int border INIT(2),border_last_chosen INIT(2);
EXT bool display_option_8_bit_fs INIT(false);
EXT bool prefer_res_640_400 INIT(0),using_res_640_400 INIT(0);
extern int prefer_pc_hz[2][3];
extern WORD tested_pc_hz[2][3];

EXT void get_fullscreen_rect(RECT *);
EXT int overscan INIT(0),stfm_borders INIT(0);

UNIX_ONLY( EXT int x_draw_surround_count INIT(4); )

WIN_ONLY( EXT HWND ClipWin; )



#ifdef IN_EMU

int prefer_pc_hz[2][3]={{0,0,0},{0,0,0}};
WORD tested_pc_hz[2][3]={{0,0,0},{0,0,0}};

#define CYCLES_FROM_HBL_TO_LEFT_BORDER_OPEN 84
#define CYCLES_FROM_HBL_TO_RIGHT_BORDER_CLOSE (CYCLES_FROM_HBL_TO_LEFT_BORDER_OPEN+320)

int cpu_cycles_from_hbl_to_timer_b;

#define CALC_CYCLES_FROM_HBL_TO_TIMER_B(freq) \
  switch (freq){ \
    case MONO_HZ: cpu_cycles_from_hbl_to_timer_b=192;break; \
    case 60: cpu_cycles_from_hbl_to_timer_b=(CYCLES_FROM_HBL_TO_LEFT_BORDER_OPEN+320-4);break; \
    default: cpu_cycles_from_hbl_to_timer_b=(CYCLES_FROM_HBL_TO_LEFT_BORDER_OPEN+320); \
  }

#define CYCLES_FROM_START_VBL_TO_INTERRUPT 1544

#define SCANLINE_TIME_IN_CPU_CYCLES_50HZ 512
#define SCANLINES_ABOVE_SCREEN_50HZ 63
#define SCANLINES_BELOW_SCREEN_50HZ 50
#define CYCLES_FOR_VERTICAL_RETURN_IN_50HZ 444

#define SCANLINE_TIME_IN_CPU_CYCLES_60HZ 508
#define SCANLINES_ABOVE_SCREEN_60HZ 34
#define SCANLINES_BELOW_SCREEN_60HZ 29
#define CYCLES_FOR_VERTICAL_RETURN_IN_60HZ 444

#define HBLS_PER_SECOND_AVE 15700 // Average between 50 and 60hz

#define SCANLINE_TIME_IN_CPU_CYCLES_70HZ 224
#define SCANLINES_ABOVE_SCREEN_70HZ 61
#define SCANLINES_BELOW_SCREEN_70HZ 40
#define CYCLES_FOR_VERTICAL_RETURN_IN_70HZ 200

#define HBLS_PER_SECOND_MONO (501.0*71.42857)

const int scanlines_above_screen[4]={SCANLINES_ABOVE_SCREEN_50HZ,
                                    SCANLINES_ABOVE_SCREEN_60HZ,
                                    SCANLINES_ABOVE_SCREEN_70HZ,
                                    16};

const int scanline_time_in_cpu_cycles_8mhz[4]={SCANLINE_TIME_IN_CPU_CYCLES_50HZ,
                                                SCANLINE_TIME_IN_CPU_CYCLES_60HZ,
                                                SCANLINE_TIME_IN_CPU_CYCLES_70HZ,
                                                128};

int scanline_time_in_cpu_cycles[4]={SCANLINE_TIME_IN_CPU_CYCLES_50HZ,
                                    SCANLINE_TIME_IN_CPU_CYCLES_60HZ,
                                    SCANLINE_TIME_IN_CPU_CYCLES_70HZ,
                                    128};

int draw_dest_increase_y;

int res_vertical_scale=1;
int draw_first_scanline_for_border,draw_last_scanline_for_border; //calculated from BORDER_TOP, BORDER_BOTTOM and res_vertical_scale

int draw_first_possible_line=0,draw_last_possible_line=200;

void inline draw_scanline_to_end();
void inline draw_scanline_to(int);
int scanline_drawn_so_far;
int cpu_cycles_when_shifter_draw_pointer_updated;
int left_border=BORDER_SIDE,right_border=BORDER_SIDE;
bool right_border_changed=0;
int overscan_add_extra;

//LPSCANPROC draw_scanline;

typedef void ASMCALL PIXELWISESCANPROC(int,int,int,int);
typedef PIXELWISESCANPROC* LPPIXELWISESCANPROC;
//LPRASTERWISESCANPROC jump_draw_scanline[2][4][3],draw_scanline;
LPPIXELWISESCANPROC jump_draw_scanline[3][4][3],draw_scanline,draw_scanline_lowres,draw_scanline_medres;
void ASMCALL draw_scanline_dont(int,int,int,int);

//LPPALETTECONVERTPROC palette_convert_entry=NULL;
//void palette_convert_entry_dont(int){}
//void palette_convert_16_555(int);
//void palette_convert_16_565(int);
//void palette_convert_24(int);
//void palette_convert_32(int);

extern "C"{

long* ASMCALL Get_PCpal();
//void palette_convert_16_555(int), palette_convert_16_565(int);
//void palette_convert_24(int),     palette_convert_32(int);

//void draw_scanline_8_lowres(int,int,int), draw_scanline_16_lowres(int,int,int);
//void draw_scanline_24_lowres(int,int,int),draw_scanline_32_lowres(int,int,int);

//void draw_scanline_8_lowres_400(int,int,int), draw_scanline_16_lowres_400(int,int,int);
//void draw_scanline_24_lowres_400(int,int,int),draw_scanline_32_lowres_400(int,int,int);

void ASMCALL draw_scanline_8_lowres_pixelwise(int,int,int,int), draw_scanline_16_lowres_pixelwise(int,int,int,int);
void ASMCALL draw_scanline_24_lowres_pixelwise(int,int,int,int),draw_scanline_32_lowres_pixelwise(int,int,int,int);

void ASMCALL draw_scanline_8_lowres_pixelwise_dw(int,int,int,int), draw_scanline_16_lowres_pixelwise_dw(int,int,int,int);
void ASMCALL draw_scanline_24_lowres_pixelwise_dw(int,int,int,int),draw_scanline_32_lowres_pixelwise_dw(int,int,int,int);

void ASMCALL draw_scanline_8_lowres_pixelwise_400(int,int,int,int), draw_scanline_16_lowres_pixelwise_400(int,int,int,int);
void ASMCALL draw_scanline_24_lowres_pixelwise_400(int,int,int,int),draw_scanline_32_lowres_pixelwise_400(int,int,int,int);

//void draw_scanline_8_lowres_scrolled(int,int,int), draw_scanline_16_lowres_scrolled(int,int,int);
//void draw_scanline_24_lowres_scrolled(int,int,int),draw_scanline_32_lowres_scrolled(int,int,int);

//void draw_scanline_8_lowres_scrolled_400(int,int,int), draw_scanline_16_lowres_scrolled_400(int,int,int);
//void draw_scanline_24_lowres_scrolled_400(int,int,int),draw_scanline_32_lowres_scrolled_400(int,int,int);

//void draw_scanline_8_medres(int,int,int), draw_scanline_16_medres(int,int,int);
//void draw_scanline_24_medres(int,int,int),draw_scanline_32_medres(int,int,int);

void ASMCALL draw_scanline_8_medres_pixelwise(int,int,int,int), draw_scanline_16_medres_pixelwise(int,int,int,int);
void ASMCALL draw_scanline_24_medres_pixelwise(int,int,int,int),draw_scanline_32_medres_pixelwise(int,int,int,int);

//void draw_scanline_8_medres_400(int,int,int), draw_scanline_16_medres_400(int,int,int);
//void draw_scanline_24_medres_400(int,int,int),draw_scanline_32_medres_400(int,int,int);

void ASMCALL draw_scanline_8_medres_pixelwise_400(int,int,int,int), draw_scanline_16_medres_pixelwise_400(int,int,int,int);
void ASMCALL draw_scanline_24_medres_pixelwise_400(int,int,int,int),draw_scanline_32_medres_pixelwise_400(int,int,int,int);

void ASMCALL draw_scanline_8_hires(int,int,int,int), draw_scanline_16_hires(int,int,int,int);
void ASMCALL draw_scanline_24_hires(int,int,int,int),draw_scanline_32_hires(int,int,int,int);


}


#define OVERSCAN_ADD_EXTRA_FOR_LEFT_BORDER_REMOVAL 2
#define OVERSCAN_ADD_EXTRA_FOR_SMALL_LEFT_BORDER_REMOVAL 2
#define OVERSCAN_ADD_EXTRA_FOR_GREAT_BIG_RIGHT_BORDER -106
#define OVERSCAN_ADD_EXTRA_FOR_EARLY_RIGHT_BORDER -2
#define OVERSCAN_ADD_EXTRA_FOR_RIGHT_BORDER_REMOVAL 28

#define ADD_EXTRA_TO_SHIFTER_DRAW_POINTER_AT_END_OF_LINE(s) \
          s+=(shifter_fetch_extra_words)*2;     \
          if (shifter_skip_raster_for_hscroll){                \
            if (left_border){ \
              s+=8;                     \
            }else{     \
              s+=2;                     \
            } \
          }                           \
          s+=overscan_add_extra;

int shifter_freq_change_time[32];
int shifter_freq_change[32];
int shifter_freq_change_idx=0;

#ifdef WIN32
// This is for the new scanline buffering (v2.6). If you write a lot direct
// to video memory it can be very slow due to recasching, so if the surface is
// in vid mem we set draw_buffer_complex_scanlines. This means that in
// draw_scanline_to we change draw_dest_ad to draw_temp_line_buf and
// set draw_scanline to draw_scanline_1_line. In draw_scanline_to_end
// we then copy from draw_temp_line_buf to the old draw_dest_ad and
// restore draw_scanline.
BYTE draw_temp_line_buf[800*4+16];
BYTE *draw_store_dest_ad=NULL;
LPPIXELWISESCANPROC draw_scanline_1_line[2],draw_store_draw_scanline;
bool draw_buffer_complex_scanlines;
#endif

bool draw_med_low_double_height;

#define ADD_SHIFTER_FREQ_CHANGE(f) \
  {shifter_freq_change_idx++;shifter_freq_change_idx&=31; \
  shifter_freq_change_time[shifter_freq_change_idx]=ABSOLUTE_CPU_TIME; \
  shifter_freq_change[shifter_freq_change_idx]=f;                    \
  log_to_section(LOGSECTION_VIDEO,EasyStr("VIDEO: Change to freq ")+f+      \
            " at time "+ABSOLUTE_CPU_TIME);}

bool freq_change_this_scanline=false;

void draw_check_border_removal();

#endif

#undef EXT
#undef INIT

