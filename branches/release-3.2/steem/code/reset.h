extern void power_on();
extern void reset_peripherals(bool);

#define RESET_COLD 0
#define RESET_CHANGESETTINGS 0
#define RESET_STOP 0
#define RESET_BACKUP 0

#define RESET_WARM BIT_0
#define RESET_NOCHANGESETTINGS BIT_1
#define RESET_NOSTOP BIT_2
#define RESET_NOBACKUP BIT_3


extern void reset_st(DWORD);

