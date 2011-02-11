#ifdef IN_EMU
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)
#endif

extern void RS232_VBL(int DEFVAL(0));
extern void RS232_CalculateBaud(bool,BYTE,bool);
extern BYTE RS232_ReadReg(int);
extern void RS232_WriteReg(int,BYTE);

EXT bool UpdateBaud INIT(0);

EXT BYTE rs232_recv_byte INIT(0);
EXT bool rs232_recv_overrun INIT(0);
EXT int rs232_bits_per_word INIT(8),rs232_hbls_per_word INIT(1);

extern void agenda_serial_sent_byte(int);
extern void agenda_serial_break_boundary(int);
extern void agenda_serial_loopback_byte(int);


#undef EXT
#undef INIT

