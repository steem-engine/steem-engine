#ifdef IN_EMU
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)
#endif

extern "C"
{
EXT WORD ir;                  //0
EXT unsigned short sr;        //2
EXT MEM_ADDRESS pc;           //4
EXT signed int r[16];         //8
EXT MEM_ADDRESS old_pc;       //96  //all wrong from this point onwards
EXT MEM_ADDRESS pc_high_byte; //100
EXT signed int other_sp;      //104
//---------------------     CPU emulation
EXT int cpu_cycles;           //108
EXT int ioaccess;             //112
EXT long iobuffer;            //116
EXT MEM_ADDRESS ioad;         //120
//---------------------    memory
EXT unsigned long himem;      //124
EXT MEM_ADDRESS rom_addr;     //128
EXT unsigned long tos_len;    //132
EXT unsigned long mem_len;    //136
EXT bool tos_high;            //140
EXT bool mmu_confused;        //144
EXT unsigned long hbl_count INIT(0);

// Don't forget to update this in the resource file too!
EXT const char *stem_version_text INIT("3.2b1");
EXT const char *stem_x_version_text INIT("12");

#define STEEM_EMAIL "steem@gmx.net"
#define STEEM_WEB "http:/""/steem.atari.st/"
#define MSACONV_WEB "http:/""/pageperso.aol.fr/zorg63/"

#define MEM_EXTRA_BYTES 320

EXT BYTE *Mem_End,
     *Mem_End_minus_1,
     *Mem_End_minus_2,
     *Mem_End_minus_4,
     *Rom_End,
     *Rom_End_minus_1,
     *Rom_End_minus_2,
     *Rom_End_minus_4,
     *cart INIT(NULL),
     *Cart_End_minus_1,
     *Cart_End_minus_2,
     *Cart_End_minus_4;
}

#define PAL_EXTRA_BYTES 16
EXT BYTE palette_exec_mem[64+PAL_EXTRA_BYTES];

EXT long palette_table[4096];

#define STEM_MODE_CPU 0
#define STEM_MODE_D2 1
#define STEM_MODE_INSPECT 2

#define BIT_0 0x1
#define BIT_1 0x2
#define BIT_2 0x4
#define BIT_3 0x8
#define BIT_4 0x10
#define BIT_5 0x20
#define BIT_6 0x40
#define BIT_7 0x80
#define BIT_8 0x100
#define BIT_9 0x200
#define BIT_a 0x400
#define BIT_b 0x800
#define BIT_c 0x1000
#define BIT_d 0x2000
#define BIT_e 0x4000
#define BIT_f 0x8000
#define BIT_10 0x400
#define BIT_11 0x800
#define BIT_12 0x1000
#define BIT_13 0x2000
#define BIT_14 0x4000
#define BIT_15 0x8000

#define BIT_16 0x00010000
#define BIT_17 0x00020000
#define BIT_18 0x00040000
#define BIT_19 0x00080000
#define BIT_20 0x00100000
#define BIT_21 0x00200000
#define BIT_22 0x00400000
#define BIT_23 0x00800000
#define BIT_24 0x01000000
#define BIT_25 0x02000000
#define BIT_26 0x04000000
#define BIT_27 0x08000000
#define BIT_28 0x10000000
#define BIT_29 0x20000000
#define BIT_30 0x40000000
#define BIT_31 0x80000000

#if defined(PEEK_RANGE_TEST) && defined(_DEBUG_BUILD)

void RangeError(DWORD &ad,DWORD hi_ad)
{
//  ad/=0;
  ad=hi_ad-1;
}

// Have to allow pointer to last byte to be returned for lpDPEEK (SET_PC)

#define RANGE_CHECK_MESSAGE(hi,len,hiadd) if (ad<0 || (ad+(len))>=((hi)+(hiadd))) RangeError(ad,hi-len)

BYTE& PEEK(DWORD ad){ RANGE_CHECK_MESSAGE(mem_len+MEM_EXTRA_BYTES,0,0);return *LPBYTE(Mem_End_minus_1-ad); }
WORD& DPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(mem_len+MEM_EXTRA_BYTES,1,0);return *LPWORD(Mem_End_minus_2-ad); }
DWORD& LPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(mem_len+MEM_EXTRA_BYTES,3,0);return *LPDWORD(Mem_End_minus_4-ad); }
BYTE* lpPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(mem_len+MEM_EXTRA_BYTES,0,0);return LPBYTE(Mem_End_minus_1-ad); }
WORD* lpDPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(mem_len+MEM_EXTRA_BYTES,1,0);return LPWORD(Mem_End_minus_2-ad); }
DWORD* lpLPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(mem_len+MEM_EXTRA_BYTES,3,0);return LPDWORD(Mem_End_minus_4-ad); }

BYTE& ROM_PEEK(DWORD ad){ RANGE_CHECK_MESSAGE(tos_len,0,0);return *LPBYTE(Rom_End_minus_1-ad); }
WORD& ROM_DPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(tos_len,1,0);return *LPWORD(Rom_End_minus_2-ad); }
DWORD& ROM_LPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(tos_len,3,0);return *LPDWORD(Rom_End_minus_4-ad); }
BYTE* lpROM_PEEK(DWORD ad){ RANGE_CHECK_MESSAGE(tos_len,0,0);return LPBYTE(Rom_End_minus_1-ad); }
WORD* lpROM_DPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(tos_len,1,2);return LPWORD(Rom_End_minus_2-ad); }
DWORD* lpROM_LPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(tos_len,3,0);return LPDWORD(Rom_End_minus_4-ad); }

BYTE& CART_PEEK(DWORD ad){ RANGE_CHECK_MESSAGE(128*1024,0,0);return *LPBYTE(Cart_End_minus_1-ad); }
WORD& CART_DPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(128*1024,1,0);return *LPWORD(Cart_End_minus_2-ad); }
DWORD& CART_LPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(128*1024,3,0);return *LPDWORD(Cart_End_minus_4-ad); }
BYTE* lpCART_PEEK(DWORD ad){ RANGE_CHECK_MESSAGE(128*1024,0,0);return LPBYTE(Cart_End_minus_1-ad); }
WORD* lpCART_DPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(128*1024,1,2);return LPWORD(Cart_End_minus_2-ad); }
DWORD* lpCART_LPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(128*1024,3,0);return LPDWORD(Cart_End_minus_4-ad); }

#define PAL_DPEEK(l)   *(WORD*)(palette_exec_mem+64+PAL_EXTRA_BYTES-2-(l))
#define lpPAL_DPEEK(l) (WORD*)(palette_exec_mem+64+PAL_EXTRA_BYTES-2-(l))

#else

#ifndef BIG_ENDIAN_PROCESSOR
//little endian version

#define PEEK(l)    *(BYTE*)(Mem_End_minus_1-(l))
#define DPEEK(l)   *(WORD*)(Mem_End_minus_2-(l))
#define LPEEK(l)   *(DWORD*)(Mem_End_minus_4-(l))
#define lpPEEK(l)  (BYTE*)(Mem_End_minus_1-(l))
#define lpDPEEK(l) (WORD*)(Mem_End_minus_2-(l))
#define lpLPEEK(l) (DWORD*)(Mem_End_minus_4-(l))

#define ROM_PEEK(l)    *(BYTE*)(Rom_End_minus_1-(l))
#define ROM_DPEEK(l)   *(WORD*)(Rom_End_minus_2-(l))
#define ROM_LPEEK(l)   *(DWORD*)(Rom_End_minus_4-(l))
#define lpROM_PEEK(l)  (BYTE*)(Rom_End_minus_1-(l))
#define lpROM_DPEEK(l) (WORD*)(Rom_End_minus_2-(l))
#define lpROM_LPEEK(l) (DWORD*)(Rom_End_minus_4-(l))

#define CART_PEEK(l)    *(BYTE*)(Cart_End_minus_1-(l))
#define CART_DPEEK(l)   *(WORD*)(Cart_End_minus_2-(l))
#define CART_LPEEK(l)   *(DWORD*)(Cart_End_minus_4-(l))
#define lpCART_PEEK(l)  (BYTE*)(Cart_End_minus_1-(l))
#define lpCART_DPEEK(l) (WORD*)(Cart_End_minus_2-(l))
#define lpCART_LPEEK(l) (DWORD*)(Cart_End_minus_4-(l))

#define PAL_DPEEK(l)   *(WORD*)(palette_exec_mem+64+PAL_EXTRA_BYTES-2-(l))
#define lpPAL_DPEEK(l) (WORD*)(palette_exec_mem+64+PAL_EXTRA_BYTES-2-(l))

#else

#define PEEK(l)    *(BYTE*)(Mem+(l))
#define DPEEK(l)   *(WORD*)(Mem+(l))
#define LPEEK(l)   *(DWORD*)(Mem+(l))
#define lpPEEK(l)  (BYTE*)(Mem+(l))
#define lpDPEEK(l) (WORD*)(Mem+(l))
#define lpLPEEK(l) (DWORD*)(Mem+(l))

#define ROM_PEEK(l)    *(BYTE*)(Rom+(l))
#define ROM_DPEEK(l)   *(WORD*)(Rom+(l))
#define ROM_LPEEK(l)   *(DWORD*)(Rom+(l))
#define lpROM_PEEK(l)  (BYTE*)(Rom+(l))
#define lpROM_DPEEK(l) (WORD*)(Rom+(l))
#define lpROM_LPEEK(l) (DWORD*)(Rom+(l))

#define CART_PEEK(l)    *(BYTE*)(cart+(l))
#define CART_DPEEK(l)   *(WORD*)(cart+(l))
#define CART_LPEEK(l)   *(DWORD*)(cart+(l))
#define lpCART_PEEK(l)  (BYTE*)(cart+(l))
#define lpCART_DPEEK(l) (WORD*)(cart+(l))
#define lpCART_LPEEK(l) (DWORD*)(cart+(l))

#define PAL_DPEEK(l)   *(WORD*)(palette_exec_mem+(l))
#define lpPAL_DPEEK(l) (WORD*)(palette_exec_mem+(l))

#endif

#endif

#define MEM_IO_BASE 0xff8000
#define MEM_EXPANSION_CARTRIDGE 0xfa0000
#define MEM_START_OF_USER_AREA 0x800

#if defined(WIN32) && !defined(_VC_BUILD) && !defined(_MINGW_BUILD)
extern void _RTLENTRY __int__(int);
#define INTR(i) __int__(i)
#endif

EXT char d2_t_buf[200];
#define STRS(a) itoa((a),d2_t_buf,10)
#define HEXS(a) itoa((a),d2_t_buf,16)
//----------------------------------------------------------------------
//---------------------------------------------------------------------------
EXT void* m68k_dest;
EXT MEM_ADDRESS abus;
EXT long m68k_old_dest;
EXT MEM_ADDRESS effective_address;

EXT int cpu_timer;
EXT WORD m68k_ap,m68k_iriwo;
EXT short m68k_src_w;
EXT long m68k_src_l;
EXT char m68k_src_b;
//---------------------------------------------------------------------------

// #define DPEEK(l) *(WORD*)(Mem+l)
// #define LPEEK(l) MAKELONG(*(WORD*)(Mem+l+2),*(WORD*)(Mem+l))

// #define DPEEK(l) *(WORD*)(Mem+l)
// #define LPEEK(l) MAKELONG(*(WORD*)(Mem+l+2),*(WORD*)(Mem+l))

#define DOT_B 0
#define DOT_W 1
#define DOT_L 2

#define BITS_ba9 0xe00
#define BITS_876 0x1c0
#define BITS_543 0x038

#define BITS_ba9_000 0x000
#define BITS_ba9_001 0x200
#define BITS_ba9_010 0x400
#define BITS_ba9_011 0x600
#define BITS_ba9_100 0x800
#define BITS_ba9_101 0xa00
#define BITS_ba9_110 0xc00
#define BITS_ba9_111 0xe00

#define BITS_876_000 0x000
#define BITS_876_001 0x040
#define BITS_876_010 0x080
#define BITS_876_011 0x0c0
#define BITS_876_100 0x100
#define BITS_876_101 0x140
#define BITS_876_110 0x180
#define BITS_876_111 0x1c0

#define BITS_543_000 0x00
#define BITS_543_001 0x08
#define BITS_543_010 0x10
#define BITS_543_011 0x18
#define BITS_543_100 0x20
#define BITS_543_101 0x28
#define BITS_543_110 0x30
#define BITS_543_111 0x38

#define B6_000000 0
#define B6_000001 1
#define B6_000010 2
#define B6_000011 3
#define B6_000100 4
#define B6_000101 5
#define B6_000110 6
#define B6_000111 7
#define B6_001000 8
#define B6_001001 9
#define B6_001010 10
#define B6_001011 11
#define B6_001100 12
#define B6_001101 13
#define B6_001110 14
#define B6_001111 15
#define B6_010000 16
#define B6_010001 17
#define B6_010010 18
#define B6_010011 19
#define B6_010100 20
#define B6_010101 21
#define B6_010110 22
#define B6_010111 23
#define B6_011000 24
#define B6_011001 25
#define B6_011010 26
#define B6_011011 27
#define B6_011100 28
#define B6_011101 29
#define B6_011110 30
#define B6_011111 31
#define B6_100000 32
#define B6_100001 33
#define B6_100010 34
#define B6_100011 35
#define B6_100100 36
#define B6_100101 37
#define B6_100110 38
#define B6_100111 39
#define B6_101000 40
#define B6_101001 41
#define B6_101010 42
#define B6_101011 43
#define B6_101100 44
#define B6_101101 45
#define B6_101110 46
#define B6_101111 47
#define B6_110000 48
#define B6_110001 49
#define B6_110010 50
#define B6_110011 51
#define B6_110100 52
#define B6_110101 53
#define B6_110110 54
#define B6_110111 55
#define B6_111000 56
#define B6_111001 57
#define B6_111010 58
#define B6_111011 59
#define B6_111100 60
#define B6_111101 61
#define B6_111110 62
#define B6_111111 63



#define BTST(n,b) (bool)((n>>b)&1)

#define BTST0(n) (bool)(n&1)
#define BTST1(n) (bool)(n&2)
#define BTST2(n) (bool)(n&4)
#define BTST3(n) (bool)(n&8)
#define BTST4(n) (bool)(n&16)
#define BTST5(n) (bool)(n&32)
#define BTST6(n) (bool)(n&64)
#define BTST7(n) (bool)(n&128)
#define BTST8(n) (bool)(n&256)
#define BTST9(n) (bool)(n&512)
#define BTSTa(n) (bool)(n&1024)
#define BTSTb(n) (bool)(n&2048)
#define BTSTc(n) (bool)(n&4096)
#define BTSTd(n) (bool)(n&8192)
#define BTSTe(n) (bool)(n&0x4000)
#define BTSTf(n) (bool)(n&32768)

#define PARAM_N ((ir&BITS_ba9)>>9)
#define PARAM_M (ir&0x7)

#define MSB_B BYTE(0x80)
#define MSB_W 0x8000
#define MSB_L 0x80000000

#define BYTE_00_TO_256(x) ( (int) ((unsigned char) (( (unsigned char)x )-1))  +1 )

#undef EXT
#undef INIT

