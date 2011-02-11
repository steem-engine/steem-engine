#ifndef STEEMKEYTEST
// These are the characters that are produced by pressing Alt+[shift]+key.
// They were really hard to extract from TOS!
// BYTE(STCode),BYTE(Modifiers, bit 0=shift 1=alt),BYTE(STAscii code),BYTE(0)
DWORD AltKeys_French[8]={MAKELONG(MAKEWORD(0x1a,2),'['),  MAKELONG(MAKEWORD(0x1b,2),']'),
                         MAKELONG(MAKEWORD(0x1a,3),'{'),  MAKELONG(MAKEWORD(0x1b,3),'}'),
                         MAKELONG(MAKEWORD(0x2b,2),'@'),  MAKELONG(MAKEWORD(0x2b,3),'~'),
                         MAKELONG(MAKEWORD(0x28,2),'\\'), 0};
DWORD AltKeys_German[7]={MAKELONG(MAKEWORD(0x1a,2),'@'),  MAKELONG(MAKEWORD(0x1a,3),'\\'),
                         MAKELONG(MAKEWORD(0x27,2),'['),  MAKELONG(MAKEWORD(0x28,2),']'),
                         MAKELONG(MAKEWORD(0x27,3),'{'),  MAKELONG(MAKEWORD(0x28,3),'}'),
                         0};
DWORD AltKeys_Spanish[8]={MAKELONG(MAKEWORD(0x1a,2),'['),  MAKELONG(MAKEWORD(0x1b,2),']'),
                          MAKELONG(MAKEWORD(0x1a,3),'{'),  MAKELONG(MAKEWORD(0x1b,3),'}'),
                          MAKELONG(MAKEWORD(0x2b,2),'#'),  MAKELONG(MAKEWORD(0x2b,3),'@'),
                          MAKELONG(MAKEWORD(0x28,2),129/*ü*/),  0};
DWORD AltKeys_Italian[8]={MAKELONG(MAKEWORD(0x1a,2),'['),  MAKELONG(MAKEWORD(0x1b,2),']'),
                          MAKELONG(MAKEWORD(0x1a,3),'{'),  MAKELONG(MAKEWORD(0x1b,3),'}'),
                          MAKELONG(MAKEWORD(0x2b,2),248/*°*/),MAKELONG(MAKEWORD(0x2b,3),'~'),
                          MAKELONG(MAKEWORD(0x60,2),'`'),  0};
DWORD AltKeys_Swedish[9]={MAKELONG(MAKEWORD(0x1a,2),'['),  MAKELONG(MAKEWORD(0x1b,2),']'),
                          MAKELONG(MAKEWORD(0x1a,3),'{'),  MAKELONG(MAKEWORD(0x1b,3),'}'),
                          MAKELONG(MAKEWORD(0x28,2),'`'),  MAKELONG(MAKEWORD(0x28,3),'~'),
                          MAKELONG(MAKEWORD(0x2b,2),'^'),  MAKELONG(MAKEWORD(0x2b,2),'@'),
                          0};
DWORD AltKeys_Swiss[10]={MAKELONG(MAKEWORD(0x1a,2),'@'),  MAKELONG(MAKEWORD(0x1a,3),'\\'),
                         MAKELONG(MAKEWORD(0x1b,2),'#'),
                         MAKELONG(MAKEWORD(0x27,2),'['),  MAKELONG(MAKEWORD(0x28,2),']'),
                         MAKELONG(MAKEWORD(0x27,3),'{'),  MAKELONG(MAKEWORD(0x28,3),'}'),
                         MAKELONG(MAKEWORD(0x2b,2),'~'),  MAKELONG(MAKEWORD(0x2b,3),'|'),
                         0};
                         
extern LANGID KeyboardLangID;
//---------------------------------------------------------------------------
void GetTOSKeyTableAddresses(MEM_ADDRESS *lpUnshiftTable,MEM_ADDRESS *lpShiftTable)
{
	MEM_ADDRESS addr=0;
	while (addr<tos_len){
		if (ROM_PEEK(addr++)=='u'){
			if (ROM_PEEK(addr)=='i'){
				addr++;
				if (ROM_PEEK(addr)=='o'){
					addr++;
					if (ROM_PEEK(addr)=='p'){
						*lpUnshiftTable=addr-25;
						break;
					}
				}
			}
		}
	}
	addr=(*lpUnshiftTable)+127;
	while (addr<tos_len){
		if (ROM_PEEK(addr++)==27){
			*lpShiftTable=addr-2;
			break;
		}
	}
}
//---------------------------------------------------------------------------
void GetAvailablePressChars(DynamicArray<DWORD> *lpChars)
{
  MEM_ADDRESS UnshiftTableAddr,ShiftTableAddr;
  GetTOSKeyTableAddresses(&UnshiftTableAddr,&ShiftTableAddr);

  MEM_ADDRESS TableAddr=UnshiftTableAddr;
  int Shift=0;
  for (int t=0;t<2;t++){
    // Go through every entry of both tables
    for (int STCode=0;STCode<128;STCode++){
      // Ignore keypad codes, they just confuse things and are the same on every ST
      if ((STCode<0x63 || STCode>0x72) && STCode!=0x4a && STCode!=0x4e){
        BYTE STAscii=ROM_PEEK(TableAddr+STCode);
        if (STAscii>32 && STAscii!=127){ // Viewable character and not delete
          DWORD Code=MAKELONG(MAKEWORD(STCode,Shift),STAscii);
          lpChars->Add(Code);
        }
      }
    }
    TableAddr=ShiftTableAddr;
    Shift=1;
  }

  // Handle characters typed while holding Alt, these aren't
  // in any key table
  DWORD *Alts=NULL;
  switch (ROM_PEEK(0x1d)){ // Country code
    case 5:  Alts=AltKeys_French; break;
    case 9:  Alts=AltKeys_Spanish; break;
    case 3:  Alts=AltKeys_German; break;
    case 11: Alts=AltKeys_Italian; break;
    case 13: Alts=AltKeys_Swedish; break;
    case 17: Alts=AltKeys_Swiss; break;
  }
  if (Alts){
    while (*Alts) lpChars->Add(*(Alts++));
  }
}

#endif

WORD *shift_key_table[4]={NULL,NULL,NULL,NULL};

bool EnableShiftSwitching=0,ShiftSwitchingAvailable=0;
//---------------------------------------------------------------------------
#ifdef WIN32
void SetSTKeys(char *Letters,int Val1,...)
{
  int *lpVals=&Val1;
  int l=0;
  WORD Code;
  do{
    Code=VkKeyScan(Letters[l]);
    if (HIBYTE(Code)==0){ //No shift required to type character #Letters[l]
      key_table[LOBYTE(Code)]=LOBYTE(lpVals[l]);
    }
  }while (Letters[++l]);
}
#elif defined(UNIX)
KeyCode Key_Pause,Key_Quit;

void SetSTKeys(char *Letters,int Val1,...)
{
  int *lpVals=&Val1;
  int l=0;
  KeyCode Code;
  do{
    // Somehow KeySym codes are exactly the same as Windows standard ASCII codes!
    KeySym ks=(KeySym)((unsigned char)(Letters[l])); // Don't sign extend this!
    Code=XKeysymToKeycode(XD,ks);

    // Now assign this X scan code to the ST scan code, we should only do this if you
    // do not need shift/alt to access the character. However, in a vain attempt to
    // improve mapping, we assign it anyway if the code isn't already assigned.
    if (XKeycodeToKeysym(XD,Code,0)==ks || key_table[LOBYTE(Code)]==0){
      key_table[LOBYTE(Code)]=LOBYTE(lpVals[l]);
    }
  }while (Letters[++l]);
}
//---------------------------------------------------------------------------
void SetSTKey(KeySym Sym,BYTE STCode,bool CanOverwrite=0)
{
  KeyCode Code=XKeysymToKeycode(XD,Sym);
  if (key_table[BYTE(Code)]==0 || CanOverwrite){
    key_table[BYTE(Code)]=STCode;
  }
}
#endif
//---------------------------------------------------------------------------
#define PC_SHIFT 1
#define NO_PC_SHIFT 0
#define PC_ALT 2
#define NO_PC_ALT 0

#define ST_SHIFT 1
#define NO_ST_SHIFT 0
#define ST_ALT 2
#define NO_ST_ALT 0

void AddToShiftSwitchTable(int PCModifiers,int PCAscii,BYTE STModifier,BYTE STCode)
{
  BYTE Code;

  ShiftSwitchingAvailable=true;
#ifdef WIN32
	Code=LOBYTE(VkKeyScan((BYTE)PCAscii));
#elif defined(UNIX)
  Code=(BYTE)XKeysymToKeycode(XD,(KeySym)PCAscii);
#endif
	if (shift_key_table[PCModifiers]) shift_key_table[PCModifiers][Code]=MAKEWORD(STCode,STModifier);
}
//---------------------------------------------------------------------------
/*
bool GetPCKeyForSTCode(bool ShiftTable,BYTE STCode,BYTE *VKCode,bool *NeedShift)
{
  WORD Dat=0xffff;
  BYTE Ascii=ROM_PEEK((MEM_ADDRESS)(ShiftTable ? tos_shift_key_table:tos_key_table) + STCode);
  if (Ascii>127) Ascii=STCharToPCChar[Ascii-128];
  if (Ascii>32){
    Dat=VkKeyScan(Ascii);
    if (Dat!=0xffff){
      *VKCode=LOBYTE(Dat);
      *NeedShift=HIBYTE(Dat) & 1;
      return true;
    }
  }
  return 0;
}

void GenerateAutomaticKeyTable()
{
  bool DoShiftTable[128];
  BYTE VKCode,Shift_VKCode;
  bool NeedShift;

  memset(DoShiftTable,0xff,sizeof(DoShiftTable));
  for (int STCode=0;STCode<128;STCode++){
    if (STCode ISNT_NUMPAD_KEY){
      if (GetPCKeyForSTCode(0,STCode,&VKCode,&NeedShift)){
        if (NeedShift==0){
      	  key_table[VkCode]=STCode;

          if (GetPCKeyForSTCode(true,STCode,&VKCode,&NeedShift)){
            if (Shift_VKCode==VKCode && NeedShift) DoShiftTable[STCode]=0;
          }
        }else{
        	if (shift_key_table[1]) shift_key_table[1][VKCode]=STCode;
          ShiftSwitchingAvailable=true;
        }
      }
    }
  }

  for (int STCode=0;STCode<128;STCode++){
    if (STCode ISNT_NUMPAD_KEY && DoShiftTable[STCode]){
      if (GetPCKeyForSTCode(true,STCode,&VKCode,&NeedShift)){
        int PCShift=int(NeedShift ? PC_SHIFT:NO_PC_SHIFT);
      	if (shift_key_table[PCShift]) shift_key_table[PCShift][VKCode]=STCode | ST_SHIFT;
        ShiftSwitchingAvailable=true;
      }
    }
  }
}
*/
//---------------------------------------------------------------------------
void DestroyKeyTable()
{
  for (int i=0;i<4;i++){
    if (shift_key_table[i]){
      free(shift_key_table[i]);
      shift_key_table[i]=NULL;
    }
  }
}
//---------------------------------------------------------------------------
void InitKeyTable()
{
  long Language,SubLang;

  DestroyKeyTable();

  ShiftSwitchingAvailable=0;
  if (EnableShiftSwitching){
    for (int i=0;i<4;i++){
      // i: BIT 0=shift, BIT 1=alt
      shift_key_table[i]=(WORD*)malloc(sizeof(WORD)*256);
      ZeroMemory(shift_key_table[i],sizeof(WORD)*256);
    }
  }

#ifdef WIN32
  Language=PRIMARYLANGID(KeyboardLangID);
  SubLang=SUBLANGID(KeyboardLangID);
  /*
   Any keys with VK_ constants plus 'A'-'Z' and '0' to '9' will be the same
   on all keyboards/languages, these are set up in ikbd.h. The rest are put
   into the key_table here.

   NOTE: On Windows SetSTKeys doesn't put it in the table if modifiers are
         required to produce the characters.
  */

  SetSTKeys("-=[];" "\'",0x0c,0x0d,0x1a,0x1b,0x27, 0x28);
  SetSTKeys("`#,./",0x29,0x2b,0x33,0x34,0x35);

#elif defined(UNIX)

  Language=LOWORD(KeyboardLangID);
  SubLang=HIWORD(KeyboardLangID);
  ZeroMemory(key_table,sizeof(key_table));

  SetSTKeys("1234567890-=",0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD);
  SetSTKeys("qwertyuiop[]",0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B);
  SetSTKeys("asdfghjkl;'",0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28);
  SetSTKeys("zxcvbnm,./",0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,0x33,0x34,0x35);
  SetSTKey(XK_Escape,0x1);
  SetSTKey(XK_grave,0x29);
  SetSTKey(XK_BackSpace,0xE);
  SetSTKey(XK_Tab,0xF);
  SetSTKey(XK_Return,0x1C);
  SetSTKey(XK_Delete,0x53);
  key_table[VK_CONTROL]=0x1d;
  SetSTKey(XK_Control_L,0x1D);
  SetSTKey(XK_Control_R,0x1D);
  // Should never get VK_SHIFT
  SetSTKey(XK_Shift_L,0x2A);
  SetSTKey(XK_Shift_R,0x36);
  key_table[VK_MENU]=0x38;
  SetSTKey(XK_Alt_L,0x38);
  SetSTKey(XK_Alt_R,0x38);
  SetSTKey(XK_space,0x39);
  SetSTKey(XK_Caps_Lock,0x3A);
  SetSTKey(XK_F1,0x3B);
  SetSTKey(XK_F2,0x3C);
  SetSTKey(XK_F3,0x3D);
  SetSTKey(XK_F4,0x3E);
  SetSTKey(XK_F5,0x3F);
  SetSTKey(XK_F6,0x40);
  SetSTKey(XK_F7,0x41);
  SetSTKey(XK_F8,0x42);
  SetSTKey(XK_F9,0x43);
  SetSTKey(XK_F10,0x44);

  SetSTKey(XK_Help,0x62);
  SetSTKey(XK_Undo,0x61);
  // If you don't have help and undo keys use Page Up and Page Down
  SetSTKey(XK_Page_Up,0x62);
  SetSTKey(XK_Page_Down,0x61);
  SetSTKey(XK_Insert,0x52);
  SetSTKey(XK_Home,0x47);
  SetSTKey(XK_Up,0x48);
  SetSTKey(XK_Left,0x4B);
  SetSTKey(XK_Down,0x50);
  SetSTKey(XK_Right,0x4D);

  SetSTKey(XK_F11,0x63);  // (
  SetSTKey(XK_F12,0x64);  // )
  SetSTKey(XK_KP_Divide,0x65);
  SetSTKey(XK_KP_Multiply,0x66);
  SetSTKey(XK_KP_7,0x67);
  SetSTKey(XK_KP_8,0x68);
  SetSTKey(XK_KP_9,0x69);
  SetSTKey(XK_KP_Subtract,0x4A);
  SetSTKey(XK_KP_4,0x6A);
  SetSTKey(XK_KP_5,0x6B);
  SetSTKey(XK_KP_6,0x6C);
  SetSTKey(XK_KP_Add,0x4E);
  SetSTKey(XK_KP_1,0x6D);
  SetSTKey(XK_KP_2,0x6E);
  SetSTKey(XK_KP_3,0x6F);
  SetSTKey(XK_KP_0,0x70);
  SetSTKey(XK_KP_Decimal,0x71);
  SetSTKey(XK_KP_Enter,0x72);
#endif

  if (SubLang==SUBLANG_ENGLISH_UK){
    SetSTKeys("\\" "#",0x60, 0x2b);
  }else if (SubLang==SUBLANG_ENGLISH_AUS){
    SetSTKeys("\\",0x60);  // # not unshifted on Aus keyboard, might overwrite something if we map
  }else{
    SetSTKeys("\\",0x2b);
  }

  switch (Language){
    case LANG_ENGLISH:
      if (SubLang==SUBLANG_ENGLISH_AUS){
        AddToShiftSwitchTable(PC_SHIFT, '2',ST_SHIFT,0x28);     // Shift+"2" = Shift+"'" = "@"
        AddToShiftSwitchTable(PC_SHIFT, '\'',ST_SHIFT,0x3);     // Shift+"'" = Shift+"2" =  "
        AddToShiftSwitchTable(PC_SHIFT, '3',NO_ST_SHIFT,0x2b);  // Shift+"3" =       "#" = "#"
        AddToShiftSwitchTable(PC_SHIFT, '`',ST_SHIFT,0x2b);     // Shift+"`" = Shift+"#" = "~"
      }
      break;
    case LANG_FRENCH:
      if (SubLang==SUBLANG_FRENCH_BELGIAN){
        SetSTKeys( "&" "é" "\"" "\'"  "(" "§" "è" "!" "ç" "à" ")" "-",
                   0x2,0x3, 0x4, 0x5, 0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd);
        SetSTKeys("az^$",0x10,0x11,0x1a,0x1b);
        SetSTKeys("qmùµ",30,39,40,0x29);
        SetSTKeys("<w,;:=",96,44,50,51,52,0x35);

        AddToShiftSwitchTable(PC_ALT, '&',ST_SHIFT,0x2b);        // |
        AddToShiftSwitchTable(PC_ALT, 'é',ST_ALT,0x2b);          // @
        AddToShiftSwitchTable(PC_ALT,'\"',0,0x2b);               // #
        AddToShiftSwitchTable(PC_ALT, 'ç',ST_ALT+ST_SHIFT,0x1a); // {
        AddToShiftSwitchTable(PC_ALT, 'à',ST_ALT+ST_SHIFT,0x1b); // }
        AddToShiftSwitchTable(PC_ALT, '^',ST_ALT,0x1a);          // [
        AddToShiftSwitchTable(PC_ALT, '$',ST_ALT,0x1b);          // ]
        AddToShiftSwitchTable(PC_ALT, 'µ',0,0x29);               // `
        AddToShiftSwitchTable(PC_ALT, '<',ST_ALT,0x28);          /* \ */
        AddToShiftSwitchTable(PC_ALT, '=',ST_ALT+ST_SHIFT,0x2b); // ~
      }else{
        SetSTKeys( "&" "é" "\"" "\'"  "(" "-" "è" "_" "ç" "à" ")" "=",
                   0x2,0x3, 0x4, 0x5, 0x6,0x7,0x8,0x9,0xa,0xb,0xc,0x35);
        SetSTKeys("az^$",0x10,0x11,0x1a,0x1b);
        SetSTKeys("qmù*",30,39,40,0x66);
        SetSTKeys("<w,;:!",96,44,50,51,52,0x9);

        AddToShiftSwitchTable(0,'-',0,0xd);                // -
        AddToShiftSwitchTable(PC_SHIFT,'-',ST_SHIFT,0x7);  // 6
        AddToShiftSwitchTable(0,'_',ST_SHIFT,0xd);         // _
        AddToShiftSwitchTable(PC_SHIFT,'_',ST_SHIFT,0x9);  // 8
        AddToShiftSwitchTable(0,'!',0,0x9);                // !
        AddToShiftSwitchTable(PC_SHIFT,'!',0,0x7);         // §
        AddToShiftSwitchTable(PC_SHIFT,'$',ST_SHIFT,0x29); // £

        AddToShiftSwitchTable(PC_ALT, 'é',ST_ALT+ST_SHIFT,0x2b);  // ~
        AddToShiftSwitchTable(PC_ALT,'\"',0,0x2b);                // #
        AddToShiftSwitchTable(PC_ALT,'\'',ST_ALT+ST_SHIFT,0x1a);  // {
        AddToShiftSwitchTable(PC_ALT, '(',ST_ALT,0x1a);           // [
        AddToShiftSwitchTable(PC_ALT, '-',ST_SHIFT,0x2b);         // |
        AddToShiftSwitchTable(PC_ALT, 'è',0,0x29);                // `
        AddToShiftSwitchTable(PC_ALT, '_',ST_ALT,0x28);           /* \ */
        AddToShiftSwitchTable(PC_ALT, 'ç',0,0x1a);                // ^
        AddToShiftSwitchTable(PC_ALT, 'à',ST_ALT,0x2b);           // @
        AddToShiftSwitchTable(PC_ALT, ')',ST_ALT,0x1b);           // ]
        AddToShiftSwitchTable(PC_ALT, '=',ST_ALT+ST_SHIFT,0x1b);  // }
        AddToShiftSwitchTable(PC_ALT, '$',ST_SHIFT,0xc);          // ¤ !
      }
      break;
    case LANG_GERMAN:
    {
      SetSTKeys("ß" "\'" "zü+öä#~y-<",12, 13, 21,26,27,39,40,41,43,44,53,96);
      /*                                                          ___
        Key #220 = ASCII '^' (#94) = ST keycode 0x2b             ; / ;
        Key #221 = ASCII '´' (#180) = ST keycode 0xd        #180=;   ;
                                                                  ---
      */
#ifdef WIN32
      SetSTKeys("^´",0x2b,0xd);
#else
      SetSTKey(XK_dead_circumflex,0x2b,true);
      SetSTKey(XK_dead_acute,0xd,true);
#endif

      /*
        Shift + Key #191 = ASCII '#' (#35) = ST keycode No Shift+ 0xd
        No shift + Key #220 = ASCII '^' (#94) = ST keycode Shift+ 0x29
        Shift + Key #220 = ASCII '^' (#94) = ST keycode No Shift+ 0x2b
      */
      AddToShiftSwitchTable(PC_SHIFT,'#',NO_ST_SHIFT,0xd);  // '
      AddToShiftSwitchTable(NO_PC_SHIFT,'^',ST_SHIFT,0x29); // ^
      AddToShiftSwitchTable(PC_SHIFT,'^',NO_ST_SHIFT,0x2b); // ~

      // PC alt to no ST alt
      AddToShiftSwitchTable(PC_ALT,'+',0,0x2b);        /* ~ */
      AddToShiftSwitchTable(PC_ALT,'<',ST_SHIFT,0x2b); /* | */

      // PC alt to ST alt (but moved)
      AddToShiftSwitchTable(PC_ALT,'Q',ST_ALT,0x1a);           /* @ */
      AddToShiftSwitchTable(PC_ALT,'ß',ST_ALT+ST_SHIFT,0x1a);  /* \ */
      AddToShiftSwitchTable(PC_ALT,'8',ST_ALT,0x27);           /* [ */
      AddToShiftSwitchTable(PC_ALT,'9',ST_ALT,0x28);           /* ] */
      AddToShiftSwitchTable(PC_ALT,'7',ST_ALT+ST_SHIFT,0x27);  /* { */
      AddToShiftSwitchTable(PC_ALT,'0',ST_ALT+ST_SHIFT,0x28);  /* } */

      break;
    }
    case LANG_SPANISH:
    case LANG_CATALAN:
    case LANG_BASQUE:
      SetSTKeys("\'" "`´ñ;ç" "\\" ".°<[{^",26 ,0x1b,0x1a,39,40,41, 43, 0x34,0x35,0x60,0x0c,0x28,0x1b);

      AddToShiftSwitchTable(PC_SHIFT,'1',ST_SHIFT,0x34);               // !
      AddToShiftSwitchTable(PC_SHIFT,'2',ST_SHIFT,0x1a);               // "
      AddToShiftSwitchTable(PC_SHIFT,'3',NO_ST_SHIFT,0x71);            // · (central .)
      AddToShiftSwitchTable(PC_SHIFT,'6',ST_SHIFT,0x8);                // &
      AddToShiftSwitchTable(PC_SHIFT,'7',ST_SHIFT,0x7);                // /
      AddToShiftSwitchTable(PC_SHIFT,'8',ST_SHIFT,0xa);                // (     ___
      AddToShiftSwitchTable(PC_SHIFT,'9',ST_SHIFT,0xb);                // )    ; o ;
      AddToShiftSwitchTable(PC_SHIFT,   '\''/*39 */,ST_SHIFT,0x33);    // ?    ;   ;
      AddToShiftSwitchTable(NO_PC_SHIFT,'º' /*186*/,NO_ST_SHIFT,0x35); // º     --- #186
      AddToShiftSwitchTable(PC_SHIFT,   'º' /*186*/,NO_ST_SHIFT,0x2b); /* \ */
      AddToShiftSwitchTable(NO_PC_SHIFT,'¡' /*161*/,ST_SHIFT,   0x2);  // ¡
      AddToShiftSwitchTable(PC_SHIFT,   '¡' /*161*/,ST_SHIFT,   0x3);  // ? (upside down)
      AddToShiftSwitchTable(NO_PC_SHIFT,'+' /*43 */,ST_SHIFT,   0xd);  // +     ___
      AddToShiftSwitchTable(PC_SHIFT,   '+' /*43 */,NO_ST_SHIFT,0x66); // *    ; . ;
      AddToShiftSwitchTable(PC_SHIFT,   ',' /*44 */,NO_ST_SHIFT,0x28); // ;    ; | ;
      AddToShiftSwitchTable(PC_SHIFT,   '.' /*46 */,ST_SHIFT,   0x28); // :     --- #161
      AddToShiftSwitchTable(PC_SHIFT,   '0',        NO_ST_SHIFT,0x0d); // =

      AddToShiftSwitchTable(PC_ALT,'º' /*186*/ ,0,0x2b);              /* \ */
      AddToShiftSwitchTable(PC_ALT,'1',ST_SHIFT,0x2b);                // |    ___
      AddToShiftSwitchTable(PC_ALT,'2',ST_ALT+ST_SHIFT,0x2b);         // @   ; C ;
      AddToShiftSwitchTable(PC_ALT,'3',ST_ALT,0x2b);                  // #   ; j ;
      AddToShiftSwitchTable(PC_ALT,'`',ST_ALT,0x1a);                  // [    --- #231
      AddToShiftSwitchTable(PC_ALT,'+',ST_ALT,0x1b);                  // ]
      AddToShiftSwitchTable(PC_ALT,'´' /*180*/,ST_ALT+ST_SHIFT,0x1a); // { 180=horz flip `
      AddToShiftSwitchTable(PC_ALT,'ç' /*231*/,ST_ALT+ST_SHIFT,0x1b); // }
      break;
    case LANG_ITALIAN:
      SetSTKeys("ìèòàù\\.-<" "\'" "+",13,26,39,40,41,43,52,53,96, 12, 27);
      break;
    case LANG_SWEDISH:
      SetSTKeys("+éåüöä" "\'" "\\" "-<",12,13,26,27,39,40, 41, 43, 53,96);
      AddToShiftSwitchTable(PC_ALT,'+',NO_ST_SHIFT,0x2b); /* \ */
      AddToShiftSwitchTable(PC_ALT,'2',ST_ALT,0x2b);      // @
/*
      AddToShiftSwitchTable(PC_ALT,'3',0,0);      // £
      AddToShiftSwitchTable(PC_ALT,'4',0,0);      // $
      AddToShiftSwitchTable(PC_ALT,'7',0,0);      // {
      AddToShiftSwitchTable(PC_ALT,'8',0,0);      // [
      AddToShiftSwitchTable(PC_ALT,'9',0,0);      // ]
      AddToShiftSwitchTable(PC_ALT,'0',0,0);      // }
*/
      break;
    case LANG_NORWEGIAN:
      SetSTKeys("ß'zü+öä#~y-<",12,13,0x2c,26,27,39,40,41,43,0x15,53,96);
      {
        /*
          But the key #186 has a ^ shifted, not £. There is one key that didn't
          come out: key 219 = \, and ` shifted, and one for which
          I didn't find the right unshifted key: key #220, which is |, shifted §.

          Key #186 = ASCII ¨ (#-88) = ST keycode 0x1b
          Key #187 = ASCII + (#43) = ST keycode 0xc
          Key #191 = ASCII ' (#39) = ST keycode 0x29
          Key #192 = ASCII ø (#-8) = ST keycode 0x28
          Key #221 = ASCII å (#-27) = ST keycode 0x1a
          Key #222 = ASCII æ (#-26) = ST keycode 0x27
        */
        char char_list[]=  {168, 43, 39,  248, 229, 230,   0};
        SetSTKeys(char_list,0x1b,0xc,0x29,0x28,0x1a,0x27);

      }
      break;
  }

#ifndef STEEMKEYTEST
  if (ShiftSwitchingAvailable==0) DestroyKeyTable();
#endif
}

#ifdef UNIX
void UNIX_get_fake_VKs()
{
  VK_LBUTTON=0xf0;VK_RBUTTON=0xf1;VK_MBUTTON=0xf2;

  VK_F11=XKeysymToKeycode(XD,XK_F11);
  VK_F12=XKeysymToKeycode(XD,XK_F12);
  VK_END=XKeysymToKeycode(XD,XK_End);

  VK_LEFT=XKeysymToKeycode(XD,XK_Left);
  VK_RIGHT=XKeysymToKeycode(XD,XK_Right);
  VK_UP=XKeysymToKeycode(XD,XK_Up);
  VK_DOWN=XKeysymToKeycode(XD,XK_Down);
  VK_TAB=XKeysymToKeycode(XD,XK_Tab);

  VK_SHIFT=0xf3;
  VK_LSHIFT=XKeysymToKeycode(XD,XK_Shift_L);
  VK_RSHIFT=XKeysymToKeycode(XD,XK_Shift_R);

  VK_CONTROL=0xf4;
  VK_RCONTROL=XKeysymToKeycode(XD,XK_Control_R);
  VK_LCONTROL=XKeysymToKeycode(XD,XK_Control_L);

  VK_MENU=0xf5;
  VK_LMENU=XKeysymToKeycode(XD,XK_Alt_L);
  VK_RMENU=XKeysymToKeycode(XD,XK_Alt_R);

  VK_SCROLL=0xf6;
  VK_NUMLOCK=0xf7;

  Key_Pause=XKeysymToKeycode(XD,XK_Pause);
}
#endif

