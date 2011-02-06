#ifdef IN_EMU
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)
#endif

#define FLOPPY_MAX_BYTESPERSECTOR 512
#define FLOPPY_MAX_TRACK_NUM      85
#define FLOPPY_MAX_SECTOR_NUM     26

EXT int floppy_current_drive();
EXT BYTE floppy_current_side();
EXT void fdc_add_to_crc(WORD &,BYTE);

EXT MEM_ADDRESS dma_address;
EXT bool floppy_instant_sector_access INIT(true);
EXT int num_connected_floppies INIT(2);
EXT int floppy_mediach[2];
EXT bool floppy_access_ff INIT(0);
EXT DWORD disk_light_off_time INIT(0);
EXT bool floppy_access_started_ff INIT(0);

EXT void agenda_fdc_spun_up(int);
EXT void agenda_fdc_motor_flag_off(int),agenda_fdc_finished(int);
EXT void agenda_floppy_seek(int);
EXT void agenda_floppy_readwrite_sector(int);
EXT void agenda_floppy_read_address(int);
EXT void agenda_floppy_read_track(int);
EXT void agenda_floppy_write_track(int);

#ifdef IN_EMU

#define FDC_STR_BUSY               BIT_0
#define FDC_STR_T1_INDEX_PULSE     BIT_1
#define FDC_STR_T23_DATA_REQUEST   BIT_1
#define FDC_STR_T1_TRACK_0         BIT_2
#define FDC_STR_T23_LOST_DATA      BIT_2
#define FDC_STR_CRC_ERROR          BIT_3
#define FDC_STR_SEEK_ERROR         BIT_4
#define FDC_STR_T1_SPINUP_COMPLETE BIT_5
#define FDC_STR_T23_SECTOR_TYPE    BIT_5
#define FDC_STR_WRITE_PROTECT      BIT_6
#define FDC_STR_MOTOR_ON           BIT_7

#define FDC_CR_TYPE_1_VERIFY      BIT_2
#define FDC_VERIFY                (fdc_cr & FDC_CR_TYPE_1_VERIFY)

WORD dma_mode;
BYTE dma_status;
int dma_sector_count;
int dma_bytes_written_for_sector_count=0;

BYTE fdc_cr,fdc_tr,fdc_sr,fdc_str,fdc_dr;
bool fdc_last_step_inwards_flag;
BYTE floppy_head_track[2];
int fdc_spinning_up=0;
int floppy_type1_command_active=2;  // Default to type 1 status
int floppy_write_track_bytes_done;

void floppy_fdc_command(BYTE);
void fdc_execute();

#define FLOPPY_FF_VBL_COUNT 20
int floppy_access_ff_counter=0;

#define FLOPPY_IRQ_YES 9
#define FLOPPY_IRQ_ONESEC 10
#define FLOPPY_IRQ_NOW 417
int floppy_irq_flag=0;

bool floppy_track_index_pulse_active();

int fdc_step_time_to_hbls[4]={94,188,32,47};

int fdc_read_address_buffer_len=0;
BYTE fdc_read_address_buffer[20];

#endif

#undef EXT
#undef INIT
