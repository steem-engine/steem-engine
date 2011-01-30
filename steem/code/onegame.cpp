//---------------------------------------------------------------------------
MEM_ADDRESS write_string_to_memory_255(MEM_ADDRESS ad,char *str)
{
  BYTE c;
  do{
    c=BYTE(*(str++));
    if (c==0) c=255;
    m68k_poke(ad++,c);
  }while (c!=255);
  return ad;
}
//---------------------------------------------------------------------------
#define nt255 0x01000000

MEM_ADDRESS OGMemWrite_AndSetRef(MEM_ADDRESS ad,char *t,MEM_ADDRESS SetAd1,...)
{
  int t_len=(int)strlen(t)+1;
  bool NullEqu255=bool(ad & nt255);
  ad&=0xffffff;
  if (ad<OG_TEXT_ADDRESS || (ad+t_len)>=OG_TEXT_ADDRESS+OG_TEXT_LEN) return ad;
  char *WriteStart=OG_TextMem+ad-OG_TEXT_ADDRESS;
  strcpy(WriteStart,t);
  if (NullEqu255) *LPBYTE(WriteStart+t_len-1)=255;

  MEM_ADDRESS *lpSetAds=&SetAd1;
  while (*lpSetAds){
    m68k_lpoke(*(lpSetAds++),ad);
  }

  ad+=t_len;
  return ad;
}

#if ONEGAME_IDX==OG_AW1_IDX || ONEGAME_IDX==OG_AW2_IDX

#define NONULL 0x01000000

void OG_AW_WriteString(MEM_ADDRESS ad,char *str)
{
  bool NoNull=bool(ad & NONULL);
  ad&=0xffffff;
  BYTE c;
  do{
    c=BYTE(*(str++));
    if (c==0){
      if (NoNull) break;
      c=255;
    }else if (c>='A' && c<='Z') c=BYTE(0xb+(c-'A'));
    else if (c>='0' && c<='9') c=BYTE(0x25+(c-'0'));
    else if (c=='.') c=0x2f;
    else if (c=='-') c=0x30;
    else if (c==',') c=0x31;
    else if (c=='(') c=0x32;
    else if (c==')') c=0x33;
    else if (c=='!') c=0x34;
    else if (c=='/') c=0x35;
    else c=0;
    m68k_poke(ad++,c);
  }while (c!=255);
}

#endif
//---------------------------------------------------------------------------
void OGDrawSprite(int n,MEM_ADDRESS ad)
{
  if (pOGSprites==NULL) return;

  WORD *p=pOGSprites;
  for(;;){
    WORD size=*(p++);
    if (size==0) break;

    if ((n--)<=0){
      for (int n=0;n<size;n+=2){
        DPEEK(ad)=*(p++);
        ad+=2;
      }
      break;
    }
    p+=size/2;
  }
}
//---------------------------------------------------------------------------
bool OGInit()
{
  MemoryFile mf;
  mf.offset=0;

  mf.data=NULL;
  HRSRC res;
  HGLOBAL hglob=NULL;
  res=FindResource(NULL,"OG_DATA",RCNUM(300));
  if (res) hglob=LoadResource(NULL,res);
  if (hglob) mf.data=(void*)LockResource(hglob);
  if (mf.data==NULL) return 0;

  mf.size=SizeofResource(Inst,res);

  char *data_ptr;
  DWORD data_size;

  if (urarlib_get(&data_ptr,&data_size,ONEGAME_NAME ".st",(char*)&mf,"")){
    OGDiskPath.SetLength(MAX_PATH);
    char temppath[MAX_PATH+1];
    GetTempPath(MAX_PATH,temppath);
    GetTempFileName(temppath,"TMP",0,OGDiskPath.Text);

    FILE *f=fopen(OGDiskPath,"wb");
    if (f==NULL) return 0;
    fwrite(data_ptr,1,data_size,f);
    fclose(f);

    SetFileAttributes(OGDiskPath,FILE_ATTRIBUTE_READONLY);

    FloppyDrive[0].SetDisk(OGDiskPath);
    OGRestoreSectors();
  }

  if (urarlib_get(&data_ptr,&data_size,"tos.img",(char*)&mf,"")==0) return 0;
  memset(Rom,0xff,256*1024);
  for (DWORD n=0;n<tos_len;n++){
    ROM_PEEK(n)=(BYTE)*(data_ptr++);
  }

  if (urarlib_get(&data_ptr,&data_size,ONEGAME_NAME ".sts",(char*)&mf,"")==0) return 0;
  if (LoadSnapShot(data_ptr,true,0)==0) return 0;

  if (urarlib_get(&data_ptr,&data_size,ONEGAME_NAME ".stsprites",(char*)&mf,"")){
    pOGSprites=new WORD[data_size];
    memcpy(pOGSprites,data_ptr,data_size);
  }

#if ONEGAME_IDX==OG_AW1_IDX
  if (urarlib_get(&data_ptr,&data_size,ONEGAME_NAME "_load.raw",(char*)&mf,"")){
    pOGExtraScreen[0]=(WORD*)new BYTE[data_size];
    memcpy(pOGExtraScreen[0],data_ptr,data_size);
  }
  if (urarlib_get(&data_ptr,&data_size,ONEGAME_NAME "_end.raw",(char*)&mf,"")){
    pOGExtraScreen[1]=(WORD*)new BYTE[data_size];
    memcpy(pOGExtraScreen[1],data_ptr,data_size);
  }
#endif

  if (urarlib_get(&data_ptr,&data_size,ONEGAME_NAME "_title.raw",(char*)&mf,"")==0) return 0;
  pOGTitle=(WORD*)data_ptr;

/*
  Str SnapShot=RunDir+SLASH+ONEGAME_NAME+".sts";
  if (Exists(SnapShot)==0){
    if (Exists(SnapShot+".lnk")){
      WIN32_FIND_DATA wfd;
      SnapShot=GetLinkDest(SnapShot+".lnk",&wfd);
    }else{
      SnapShot="";
    }
  }
  if (SnapShot.Empty()) return 0;
*/

  SHORTCUTINFO si[5];
  int nSI=3;

  ClearSHORTCUTINFO(&(si[0])); si[0].Id[0]=MAKEWORD(101,10), si[0].Action=CUT_PRESSKEY, si[0].PressKey=VK_MENU;
  ClearSHORTCUTINFO(&(si[1])); si[1].Id[0]=MAKEWORD(102,10), si[1].Action=CUT_PRESSKEY, si[1].PressKey=VK_LSHIFT;
  ClearSHORTCUTINFO(&(si[2])); si[2].Id[0]=VK_END,           si[2].Action=CUT_PRESSKEY, si[2].PressKey=VK_ESCAPE;

#if ONEGAME_IDX==OG_AW1_IDX || ONEGAME_IDX==OG_AW2_IDX
  // Turn off colour changing
  m68k_dpoke(0x387C,0x4e71);
  m68k_dpoke(0x387C+2,0x4e71);
  m68k_dpoke(0x387C+4,0x4e71);
  m68k_dpoke(0x387C+6,0x4e71);
#endif

#if ONEGAME_IDX==OG_NM1_IDX

  nSI++;
  ClearSHORTCUTINFO(&(si[3])); si[3].Id[0]=VK_SPACE,           si[3].Action=CUT_PRESSKEY, si[3].PressKey=VK_MENU;

  m68k_dpoke(0x21AA,0xffff); // Set up hook

  m68k_dpoke(0x233C,0x4e71); // Disable VBL colour change

  m68k_dpoke(0x2420,0x4e71);
  m68k_dpoke(0x2422,0x4e71);
  m68k_dpoke(0x2424,0x4e71);

  m68k_dpoke(0x2448,0x4e71);
  m68k_dpoke(0x244a,0x4e71);
  m68k_dpoke(0x244c,0x4e71);
  m68k_dpoke(0x244e,0x4e71);
  m68k_dpoke(0x2450,0x4e71);
  m68k_dpoke(0x2452,0x4e71);
  m68k_dpoke(0x2454,0x4e71);
  m68k_dpoke(0x2456,0x4e71);

  m68k_dpoke(0x23D0,0x4e71);
  m68k_dpoke(0x23D2,0x4e71);
  m68k_dpoke(0x23D4,0x4e71);
  m68k_dpoke(0x23D6,0x4e71);

  // Turn off top overscan
  m68k_dpoke(0x235E,0x4e71);
  m68k_dpoke(0x2360,0x4e71);
  m68k_dpoke(0x2362,0x4e71);
  m68k_dpoke(0x2364,0x4e71);

  m68k_dpoke(0x236E,0x4e71);
  m68k_dpoke(0x2370,0x4e71);
  m68k_dpoke(0x2372,0x4e71);
  m68k_dpoke(0x2374,0x4e71);

  // Turn off bottom overscan
  m68k_dpoke(0x246A,0x4e71);
  m68k_dpoke(0x246C,0x4e71);
  m68k_dpoke(0x246E,0x4e71);
  m68k_dpoke(0x2470,0x4e71);

  m68k_dpoke(0x2478,0x4e71);
  m68k_dpoke(0x247a,0x4e71);
  m68k_dpoke(0x247c,0x4e71);
  m68k_dpoke(0x247e,0x4e71);

/*
  // PROGRAM BY IGNACIO ABRIL
  // GRAPHIC DESIGNER SNATCHO
  // MUSIC COMPOSED BY MAC
  // PRODUCED BY VICTOR RUIZ
  // COPYRIGHT [^]^ DINAMIC
  char *text_replace[5]={"PROGRAMA POR IGNACIO ABRIL","GRAFICOS POR SNATCHO","COPYRIGHT FX INTERACTIVE",
                          "PRODUCIDO POR VICTOR RUIZ"," "};
  MEM_ADDRESS text_read_ad[5]={0x20be,0x20E6,0x20D2,0x20FA,0x210E};
  MEM_ADDRESS x_pos_ad[5]={0x20B6,0x20DE,0x20CA,0x20F2,0x2106};

  MEM_ADDRESS text_ad=0x3061E;
  for (int n=0;n<5;n++){
    m68k_lpoke(text_read_ad[n],text_ad);
    m68k_dpoke(x_pos_ad[n],WORD(max(157-(int)strlen(text_replace[n])*6,0)));
    text_ad=write_string_to_memory(text_ad,text_replace[n]);
  }
*/

  if (OGInfinite){
    m68k_dpoke(0x3718,0x4e71); // nop
    m68k_dpoke(0x371a,0x4e71); // nop
    m68k_dpoke(0x371c,0x4e71); // nop
  }
  if (OGNumLives){
    m68k_dpoke(0x217a,(WORD)min(OGNumLives,0x7f));
    m68k_poke(0x30706,(BYTE)min(OGNumLives,0x7f));
  }
  if (OGEasyMode){
    // 0x3167A - first mines
    m68k_dpoke(0x3167A+3*2,0x2a00);
    m68k_dpoke(0x3167A+5*2,m68k_dpeek(0x3167A+6*2));
    m68k_dpoke(0x3167A+11*2,m68k_dpeek(0x3167A+10*2));

    // 0x31694 - second mines
    m68k_dpoke(0x31694+0*2,0x9c00);
    m68k_dpoke(0x31694+1*2,0x9c00);
    m68k_dpoke(0x31694+4*2,m68k_dpeek(0x31694+5*2));
    m68k_dpoke(0x31694+9*2,m68k_dpeek(0x31694+10*2));
    m68k_dpoke(0x31694+11*2,m68k_dpeek(0x31694+12*2));

    // 0x316B4 - first enemies
    for (MEM_ADDRESS ad=0x316B5;ad<0x316dc;ad+=2){
      if (m68k_peek(ad) & 0x10) m68k_poke(ad,0);
    }

    // 0x316DE - second enemies
    for (MEM_ADDRESS ad=0x316DF;ad<0x31716;ad+=2){
      if (m68k_peek(ad) & 0x10) m68k_poke(ad,0);
    }
  }


#elif ONEGAME_IDX==OG_NM2_IDX

  ClearSHORTCUTINFO(&(si[0])); si[0].Id[0]=MAKEWORD(101,10), si[0].Action=CUT_PRESSKEY, si[0].PressKey=VK_SPACE;
/*
  Can't do this, stops space from working in console!
  ClearSHORTCUTINFO(&(si[3])); si[3].Id[0]=VK_SPACE,         si[3].Action=CUT_PRESSKEY, si[3].PressKey=VK_MENU;
  ClearSHORTCUTINFO(&(si[4])); si[4].Id[0]=VK_MENU,          si[4].Action=CUT_PRESSKEY, si[4].PressKey=VK_SPACE;
  nSI+=2;
*/

  m68k_dpoke(0x216E,0xffff); // Set title hook
  m68k_dpoke(0x2178,0xffff); // Restore palette hook
  m68k_dpoke(0x387E,0xffff); // Skip get ready

  MEM_ADDRESS ad=OG_TEXT_ADDRESS;
  ad=OGMemWrite_AndSetRef(ad,"SISTEMA CONECTADO",0x71C6,0x71DA,0);  // 0x7572=READY
  ad=OGMemWrite_AndSetRef(ad,"INTRODUCE COMANDO",0x72B2,0);  // 0x7579=INSERT MESSAGE
  ad=OGMemWrite_AndSetRef(ad,"\rORDEN DE PRIORIDAD\rALPHA REQUIRIDA\rAUTORIZACION\r",0x739E,0);  // 0x7588=\rPRIORITY ORDER ALPHA\rAUTHORISATION REQUIRED\r
  ad=OGMemWrite_AndSetRef(ad,"\rORDEN DE PRIORIDAD\rBETA REQUIRIDA\rAUTORIZACION\r",0x73FA,0);  // 0x75B6=\rPRIORITY ORDER BETA\rAUTHORISATION REQUIRED\r
  ad=OGMemWrite_AndSetRef(ad,"INTRODUCE CODIGO DEL\rPRIMER OFICIAL",0x7404,0);  // 0x75E3=INSERT 1ST OFFICIAL\rCODE
  ad=OGMemWrite_AndSetRef(ad,"INTRODUCE CODIGO DEL\rOFICIAL DE MAQUINAS",0x7414,0);  // 0x75FC=INSERT MACHINES CODE
  ad=OGMemWrite_AndSetRef(ad,"INTRODUCE CODIGO DEL\rOFICIAL DE\rCOMUNICACIONES CODE",0x741E,0);  // 0x7611=INSERT TRANSMISSIONS\rCODE
  write_string_to_memory(0x762B,"BUSCANDO\r");  // 0x762B=SEARCHING\r
  ad=OGMemWrite_AndSetRef(ad,"0010101110 DISL  OK\r\rFOUND\rOPERACION ACEPTADA\r",0x74C6,0);  // 0x7672=0010101110 DISL  OK\r\rFOUND\rOPERATION ALLOWED\r
  ad=OGMemWrite_AndSetRef(ad,"1100110101 ACOP ERROR\r\rOPERATION DENEGADA\r",0x007512,0);  // 0x76A0=1100110101 ACOP FAILED\r\rOPERATION DENIED\r
  ad=OGMemWrite_AndSetRef(ad,"0010101110 DISL ERROR\r\rOPERATION DENEGADA\r",0x0074DC,0);  // 0x76CA=0010101110 DISL FAILED\r\rOPERATION DENIED\r
  ad=OGMemWrite_AndSetRef(ad,"ACCEDIENDO A SISTEMA\01PRINCIPAL PETICION DE\01ACTION ALTO SECRETO\01COMPROBANDO\01AUTORIZACION\r",0x00723C,0);  // 0x7778=MAIN FILE OPEN\01ACTION FILE TOP SECRET\01FILE CLOSED\r
  ad=OGMemWrite_AndSetRef(ad,"SISTEMA DE CIFRADO  OK",0x007508,0x00743A,0);  // 0x76F4=LOGICAL STATUS   OK
  ad=OGMemWrite_AndSetRef(ad,"SUBMARINO EN\rSUPERFICIE\r",0x007310,0);  // 0x770C=SUBMARINE HAS EMERGED\r
  ad=OGMemWrite_AndSetRef(ad,"PUERTA ABIERTA\r",0x007356,0);  // 0x7723=DOOR OPEN\r
  write_string_to_memory(0x772E,"MOTORES PARADOS\r");  // 0x772E=MACHINES STOPED\r
  ad=OGMemWrite_AndSetRef(ad,"MISILES PREPARADOS\rPARA EL LANZAMIENTO\r",0x0071d0,0);  // 0x774a=OABERBYAMDMISSILES READY\rTO LAUNCH\r
  ad=OGMemWrite_AndSetRef(ad,"INTRODUCE CODIGO\rDEFCOM",0x0073BC,0);  // 0x7764=INSERT DEFCOM CODE\r
  ad=OGMemWrite_AndSetRef(ad,"PETICION DENEGADA\r",0x00726C,0);  // 0x77AB=SYSTEM ERROR 0 \r
  write_string_to_memory(0x77BC,"FX INTERACTIVE");  // 0x77BC=DINAMIC SOFTWARE
  ad=OGMemWrite_AndSetRef(ad,"BIENVENIDO AL U*5544\rPOR FAVOR INTRODUCE EL\r   CODIGO DE ACCESO\r",0x007130,0);  // 0x77CD=WELCOME TO THE U*5544\r  PLEASE INSERT THE\r     ACCESS CODE\r
  write_string_to_memory(0x7810,"CODIGO ERRONEO\r\rDETECTADO\rINTRUSO\r\r    ' GAME OVER '    \r\r\r\r PULSA DISPARO");  // 0x7810=THE CODE IS WRONG\r\rYOU ARE AN ILLEGAL\rPLAYER\r\r    ' GAME OVER '    \r\r\r\r PRESS FIRE
  ad=OGMemWrite_AndSetRef(ad,"MENSAJE MAL CODIFICADO\r",0x007284,0);  // 0x7864=WRONG MESSAGE\r
  write_string_to_memory(0x7873,"\r\r      NAVY MOVES\rCOPYRIGHT FX INTERACTIVE\r\r\PROGRAMA POR IGNACIO ABRIL\rGRAFICOS POR SNATCHO\r PRODUCIDO POR VICTOR RUIZ\r\r\r\r\r\r\r");

  write_string_to_memory(0x12B80,"BOMBA PREPARADA"); // THE BOMB IS SET
  write_string_to_memory(0x12B90,"INICIO CUENTA ATRAS"); // THE COUNTDOWN BEGINS
  ad=OGMemWrite_AndSetRef(ad,"INICIADA CUENTA ATRAS",0x35C6,0);  // 0x12B90=THE COUNTDOWN BEGINS
  write_string_to_memory(0x12BA5,"LA MISION HA FRACASADO"); // "YOUR MISSION HAS FAILED"
  write_string_to_memory(0x12BBD," HAS PERDIDO"); // "YOU HAVE LOST"
//  write_string_to_memory(0x12BCB,"TUS EFECTIVOS"); // "YOUR WEAPONS"
  OGMemWrite_AndSetRef(ad,"TUS EFECTIVOS",0x3812,0);  // 0x12BCB="YOUR WEAPONS"
  write_string_to_memory(0x12BD8,"LA BOMBA HA EXPLOTADO"); // "THE BOMB HAS EXPLODED"
  write_string_to_memory(0x12BEE,"  MISION  CUMPLIDA"); // "MISSION ACCOMPLISHED"
  write_string_to_memory(0x12C03,"AHORA PUEDES DISFRUTAR DEL"); // "NOW YOU CAN LIE IN THE SUN"
  write_string_to_memory(0x12C1E,"SOL Y PREPARARTE PARA"); // "AND KEEP IN SHAPE FOR"
//  write_string_to_memory(0x12C34,"ARCTIC MOVES"); // "ARCTIC MOVES"

/*
  char *text_replace[]={"GET READY",                  // GET READY
                        "PLAYER ONE",                 // PLAYER ONE
                        "FX INTERACTIVE",             // COPYRIGHT [^]^ DINAMIC
                        "PROGRAMA POR IGNACIO ABRIL", // PROGRAM BY IGNACIO ABRIL
                        "GRAFICOS POR SNATCHO",       // MUSIC COMPOSED BY MAC
                        "PRODUCIDO POR VICTOR RUIZ",  // GRAPHIC DESIGNER SNATCHO
                        "COPYRIGHT [^]^",             // PRODUCED BY VICTOR RUIZ
                        "THE BOMB IS SET",            // THE BOMB IS SET
                        "THE COUNTDOWN BEGINS",       // THE COUNTDOWN BEGINS
                        "YOUR MISSION HAS FAILED",    // YOUR MISSION HAS FAILED
                        "YOU HAVE LOST YOUR WEAPONS", // YOU HAVE LOST YOUR WEAPONS
                        "THE BOMB HAS EXPLODED",      // THE BOMB HAS EXPLODED
                        "MISSION ACCOMPLISHED",       // MISSION ACCOMPLISHED
                        "NOW YOU CAN LIE IN THE SUN", // NOW YOU CAN LIE IN THE SUN
                        "AND KEEP IN SHAPE FOR",      // AND KEEP IN SHAPE FOR
                        "ARCTIC MOVES",               // ARCTIC MOVES
                        NULL};
  MEM_ADDRESS text_draw_ad[]={0xff003856,0xff00386A,0x210A,0x211e,0x2132,0x2146,0x215a,
                              0x35A6,0x35BA,0xff0037FC,0x7EB2,0x6A90,0x6AA4,0x6AB8,0x6ACC};
  MEM_ADDRESS text_ad=0x012AF4;
  int n=0;
  do{
    if (text_draw_ad[n] & 0xff000000){ // address first
      text_draw_ad[n]&=0xffffff;
      m68k_lpoke(text_draw_ad[n]+2,text_ad);
      m68k_dpoke(text_draw_ad[n]+10,WORD(max(157-(int)strlen(text_replace[n])*6,0)));
    }else{
      m68k_lpoke(text_draw_ad[n]+12,text_ad);
      m68k_dpoke(text_draw_ad[n]+4,WORD(max(157-(int)strlen(text_replace[n])*6,0)));
    }
    text_ad=write_string_to_memory(text_ad,text_replace[n++]);
  }while (text_replace[n]!=NULL);
*/

  if (OGInfinite){
    m68k_dpoke(0x4ad6,0x4e71); // nop
    m68k_dpoke(0x4ad8,0x4e71); // nop
    m68k_dpoke(0x4ada,0x4e71); // nop
  }
  if (OGNumLives){
    BYTE tens=BYTE(min(OGNumLives/10,9)+0x30);
    BYTE ones=BYTE(min(OGNumLives % 10,9)+0x30);

    m68k_poke(0x225a,tens);
    m68k_poke(0x225b,ones);
    m68k_poke(0x12C42,tens);
    m68k_poke(0x12C43,ones);
  }


#elif ONEGAME_IDX==OG_SAT1_IDX


  m68k_dpoke(0x153E,0xffff); // Title screen draw
  m68k_dpoke(0x154a,0xffff); // Restore palette

  m68k_dpoke(0x158e,0x4e71); // Cancel drawing of keys
  m68k_dpoke(0x1590,0x4e71); // Cancel drawing of keys
  m68k_dpoke(0x163a,0x4e71); // Cancel clearing of keys
  m68k_dpoke(0x163c,0x4e71); // Cancel clearing of keys

/*
  write_string_to_memory_255(0x1658,"COPYRIGHT         SOFTWARE");  // 0x1658=COPYRIGHT         SOFTWARE
  write_string_to_memory_255(0x1673,"CODED BY");  // 0x1673=CODED BY
  write_string_to_memory_255(0x167C,"GRAPHICS");  // 0x167C=GRAPHICS
  write_string_to_memory_255(0x1685,"DANIEL");  // 0x1685=DANIEL
  write_string_to_memory_255(0x168C,"PRODUCTION");  // 0x168C=PRODUCTION
  write_string_to_memory_255(0x1697,"RODRIGUEZ");  // 0x1697=RODRIGUEZ
  write_string_to_memory_255(0x16A1,"SNATCHO");  // 0x16A1=SNATCHO
  write_string_to_memory_255(0x16A9,"KEYS");  // 0x16A9=KEYS
  write_string_to_memory_255(0x16EE,"UP##CONTROL");  //  0x16EE=UP##CONTROL
  write_string_to_memory_255(0x16FA,"DOWN##L#SHIFT");  // 0x16FA=DOWN##L#SHIFT
  write_string_to_memory_255(0x1708,"RIGHT##R#SHIFT");  // 0x1708=RIGHT##R#SHIFT
  write_string_to_memory_255(0x1717,"LEFT##%");  // 0x1717=LEFT##%
  write_string_to_memory_255(0x171F,"FIRE#SPACE");  // 0x171F=FIRE#SPACE
  write_string_to_memory_255(0x172A,"PAUSE##F1");  // 0x172A=PAUSE##F1
  write_string_to_memory_255(0x1734,"ABORT##F10");  // 0x1734=ABORT##F10
*/

  MEM_ADDRESS ad=OG_TEXT_ADDRESS;
  m68k_dpoke(0x1984,0xffff); // Set up hook
  m68k_dpoke(0x1984+2,WORD(ad-OG_TEXT_ADDRESS)); // Set address
  ad=OGMemWrite_AndSetRef(ad | nt255,"PREPARATE PARA LA BATALLA",0);  //   0x19A6=GET READY FOR THE BATTLE

  m68k_dpoke(0x1278,0xffff); // Set up hook
  m68k_dpoke(0x1278+2,WORD(ad-OG_TEXT_ADDRESS)); // Set address
  ad=OGMemWrite_AndSetRef(ad | nt255,"FIN DEL TIEMPO",0);  //  0x19C0=TIME OVER

//  write_string_to_memory_255(0x19CA,"GAME OVER");  // 0x19CA=GAME OVER

  m68k_dpoke(0x17E8,0xffff); // Set up hook
  m68k_dpoke(0x17E8+2,WORD(ad-OG_TEXT_ADDRESS)); // Set address
  ad=OGMemWrite_AndSetRef(ad | nt255," BIEN HECHO GUERRERO",0);  // 0x19D4=WELL DONE WARRIOR

  m68k_dpoke(0x1802,0xffff); // Set up hook
  m68k_dpoke(0x1802+2,WORD(ad-OG_TEXT_ADDRESS)); // Set address
  ad=OGMemWrite_AndSetRef(ad | nt255,"EL CAMINO HACIA EL REINO",0);  // 0x19E6=THE WAY TO THE MAGIC

  m68k_dpoke(0x181C,0xffff); // Set up hook
  m68k_dpoke(0x181C+2,WORD(ad-OG_TEXT_ADDRESS)); // Set address
  ad=OGMemWrite_AndSetRef(ad | nt255,"DE LA MAGIA ESTA CERCA",0);  //  0x19FC=KINGDOM IS CLOSER

  write_string_to_memory_255(0x1A0E," FELICITACIONES");  // 0x1A0E=CONGRATULATIONS

  m68k_dpoke(0x18E0,0xffff); // Set up hook
  m68k_dpoke(0x18E0+2,WORD(ad-OG_TEXT_ADDRESS)); // Set address
  OGMemWrite_AndSetRef(ad | nt255,"LO HAS CONSEGUIDO",0);  // 0x1A1E=YOU HAVE GOT IT

  write_string_to_memory_255(0x1A2E,"    YA ERES UN MAGO");  // 0x1A2E=NOW YOU ARE A MAGICIAN

  OGDrawSprite(0,0x3ba98);

  if (OGInfinite){                   
    m68k_dpoke(0x55FC,0x4e71);
    m68k_dpoke(0x55FE,0x4e71);
  }


#elif ONEGAME_IDX==OG_SAT2_IDX


  m68k_dpoke(0x11c4,0xffff); // Title screen draw
  m68k_dpoke(0x11CE,0xffff); // Restore palette

  m68k_dpoke(0x4A32,0x4e71); // Cancel drawing/clearing of keys
  m68k_dpoke(0x4A34,0x4e71); // Cancel drawing/clearing of keys
  m68k_dpoke(0x4A44,0x4e71); // Cancel drawing/clearing of keys
  m68k_dpoke(0x4A46,0x4e71); // Cancel drawing/clearing of keys

  m68k_dpoke(0xDA42,0xff); // Remove drawing of logos and copyright line in shop

/*
  write_string_to_memory_255(0x94DC,"SUPER");  // 0x94DC=SUPER
  write_string_to_memory_255(0x94E2,"EXTRA");  // 0x94E2=EXTRA
  write_string_to_memory_255(0x94E8,"POWER");  // 0x94E8=POWER
  write_string_to_memory_255(0x94F6,"MEDIUM");  // 0x94F6=MEDIUM
  write_string_to_memory_255(0x94FD,"EXTRA");  // 0x94FD=EXTRA
  write_string_to_memory_255(0x9503,"POWER");  // 0x9503=POWER
  write_string_to_memory_255(0x9512,"LIGHT");  // 0x9512=LIGHT
  write_string_to_memory_255(0x9518,"EXTRA");  // 0x9518=EXTRA
  write_string_to_memory_255(0x951E,"POWER");  // 0x951E=POWER
  write_string_to_memory_255(0x952C,"FIRE");   // 0x952C=FIRE
  write_string_to_memory_255(0x9531,"SHIELD");  // 0x9531=SHIELD
  write_string_to_memory_255(0x9540,"LIGHT");  // 0x9540=LIGHT
  write_string_to_memory_255(0x9546,"SHIELD");  // 0x9546=SHIELD
  write_string_to_memory_255(0x9556,"TELECARD");  // 0x9556=TELECARD
  write_string_to_memory_255(0x9568,"ADVANCED");  // 0x9568=ADVANCED
  write_string_to_memory_255(0x9571,"SATAN");  // 0x9571=SATAN
  write_string_to_memory_255(0x9577,"SCANNER");  // 0x9577=SCANNER
  write_string_to_memory_255(0x9588,"SATAN");  // 0x9588=SATAN
  write_string_to_memory_255(0x958E,"SCANNER");  // 0x958E=SCANNER
  write_string_to_memory_255(0x959E,"ABOUT");  // 0x959E=ABOUT
  write_string_to_memory_255(0x95A4,"SATAN");  // 0x95A4=SATAN
  write_string_to_memory_255(0x95B2,"MAGIC");  // 0x95B2=MAGIC
  write_string_to_memory_255(0x95B8,"AXE");  // 0x95B8=AXE
  write_string_to_memory_255(0x95C4,"TRIPLE");  // 0x95C4=TRIPLE
  write_string_to_memory_255(0x95CB,"MAGIC");  // 0x95CB=MAGIC
  write_string_to_memory_255(0x95D1,"AXE");  // 0x95D1=AXE
*/

  MEM_ADDRESS ad=OG_TEXT_ADDRESS;
  m68k_dpoke(0x35BE,0xffff); // Set up hook
  m68k_dpoke(0x35BE+2,WORD(ad-OG_TEXT_ADDRESS)); // Set address
  ad=OGMemWrite_AndSetRef(ad | nt255,"SALIR",0);  // 0x95DE=EXIT
//  write_string_to_memory_255(0x95DE,"SALIR");  // 0x95DE=EXIT

  write_string_to_memory_255(0x9600,"     NO PUEDES COMPRAR");  // 0x9600=     YOU CANT BUY MORE
  write_string_to_memory_255(0x9617,"    INSUFICIENTE DINERO");  // 0x9617=  YOU HAVENT ENOUGHT MONEY
  write_string_to_memory_255(0x9632,"     YA LO HAS COMPRADO");  // 0x9632=     YOU ALREADY HAVE IT
  write_string_to_memory_255(0x964B,"");  // 0x964B= AUTHORS MARCOS AND SNATCHO
  write_string_to_memory_255(0x9667,"##ENHORABUENA");  // 0x9667=CONGRATULATIONS
  write_string_to_memory_255(0x9677,"AL#FIN#EL#MUNDO");  // 0x9677=AT#LAST#THE#WORLD

  ad=OGMemWrite_AndSetRef(ad | nt255,"ESTA#LIBRE#DEL#MAL",0x157a,0);  // 0x9689=IS#FREE#FROM#EVIL
//  write_string_to_memory_255(0x9689,"ESTA#LIBRE#DEL#MAL");  // 0x9689=IS#FREE#FROM#EVIL

  ad=OGMemWrite_AndSetRef(ad | nt255,"ESC#PARA#VOLVER",0x23C6,0);  // 0x969B=ESC#TO#RETURN
//  write_string_to_memory_255(0x969B,"ESC#PARA#VOLVER");  // 0x969B=ESC#TO#RETURN
  OGMemWrite_AndSetRef(ad | nt255,"NO#QUEDA#TIEMPO",0x14FA,0);  // 0x96A9=TIME#OVER
//  write_string_to_memory_255(0x96A9,"NO#QUEDA#TIEMPO");  // 0x96A9=TIME#OVER

//  write_string_to_memory_255(0x96B3,"GAME##OVER");  // 0x96B3=GAME##OVER

  OGDrawSprite(0,0x5F240);  
  OGDrawSprite(1,0x5f3f0);
  OGDrawSprite(2,0x5c390);
  OGDrawSprite(3,0x5bc50);
  OGDrawSprite(4,0x5c750);

/*
  write_string_to_memory_255(0x96BE,"COPYRIGHT##########SOFTWARE");  // 0x96BE=COPYRIGHT##########SOFTWARE
  write_string_to_memory_255(0x96DA,"CODED#BY");  // 0x96DA=CODED#BY
  write_string_to_memory_255(0x96E3,"GRAPHICS");  // 0x96E3=GRAPHICS
  write_string_to_memory_255(0x96EC,"MARCOS");  // 0x96EC=MARCOS
  write_string_to_memory_255(0x96F3,"PRODUCTION");  // 0x96F3=PRODUCTION
  write_string_to_memory_255(0x96FE,"JOURON");  // 0x96FE=JOURON
  write_string_to_memory_255(0x9705,"SNATCHO");  // 0x9705=SNATCHO
  write_string_to_memory_255(0x970D,"KEYS");  // 0x970D=KEYS
  write_string_to_memory_255(0x9712,"$$$$$$$$$$$$$");  // 0x9712=$$$$$$$$$$$$$
  write_string_to_memory_255(0x975C,"UP$$CONTROL");  // 0x975C=UP$$CONTROL
  write_string_to_memory_255(0x9768,"DOWN$$L$SHIFT");  // 0x9768=DOWN$$L$SHIFT
  write_string_to_memory_255(0x9776,"RIGHT$$R$SHIFT");  // 0x9776=RIGHT$$R$SHIFT
  write_string_to_memory_255(0x9785,"LEFT$$%");  // 0x9785=LEFT$$%
  write_string_to_memory_255(0x978D,"FIRE$$SPACE");  // 0x978D=FIRE$$SPACE
  write_string_to_memory_255(0x9799,"UP$DOWN$$ALT");  // 0x9799=UP$DOWN$$ALT
  write_string_to_memory_255(0x97A6,"TELEPORT$$T");  // 0x97A6=TELEPORT$$T
  write_string_to_memory_255(0x97B2,"PAUSE$$H");  // 0x97B2=PAUSE$$H
  write_string_to_memory_255(0x97BB,"ABORT$$UNDO");  // 0x97BB=ABORT$$UNDO
*/


#elif ONEGAME_IDX==OG_AW1_IDX

  m68k_dpoke(0xa0aa,0xffff); // Title screen
  m68k_dpoke(0xA0B0,0xffff); // Restore palette

  m68k_dpoke(0x8986,0xffff); // Load screen
  m68k_dpoke(0x8988,0xffff); // Load screen

  m68k_dpoke(0x8CEC,0xffff); // End screen
  m68k_dpoke(0x8CEE,0xffff); // End screen

  m68k_dpoke(0x9FFA,0x4e71); // Disable scrollers
  m68k_dpoke(0x9FFA+2,0x4e71); // Disable scrollers
  m68k_dpoke(0x9FFA+4,0x4e71); // Disable scrollers
  m68k_dpoke(0x9FFA+6,0x4e71); // Disable scrollers

  m68k_dpoke(0xa01a,0x4e71); // Disable Dinamic logo
  m68k_dpoke(0xa01a+2,0x4e71); // Disable Dinamic logo
  m68k_dpoke(0xa01a+4,0x4e71); // Disable Dinamic logo
  m68k_dpoke(0xa028,0x4e71); // Disable Dinamic logo
  m68k_dpoke(0xa028+2,0x4e71); // Disable Dinamic logo
  m68k_dpoke(0xa028+4,0x4e71); // Disable Dinamic logo

  m68k_dpoke(0x8996,0xffff); // Fade out before load hook
  m68k_dpoke(0x8998,0xffff); // "Do what above instruction should" hook

  m68k_dpoke(0x899e,0x4e71); // Disable normal fade out
  m68k_dpoke(0x899e+2,0x4e71); // Disable normal fade out
  m68k_dpoke(0x899e+4,0x4e71); // Disable normal fade out

/*
  0xa578="1. KEYBOARD"
  0xa584="2. KEYS AND JOYSTICK"
  0xa599="PRESS FIRE TO START"
  0xa5ad="INSERT DISK 2"
  0xa5bb="ACCESS CODE"
  0xa5c7="101069"
  0x20b86="AFTER THE WAR   COPYRIGHT DINAMIC SOFTWARE 1989   "
          "PROGRAM BY DANIEL RODRIGUEZ    MUSIC BY MAC   SOUND FX BY VICTOR RUIZ   "
          "ADDITIONAL SOUNDS, MAPS AND GRAPHICS BY JAVIER CUBEDO   "
          "ARTISTIC DESIGN BY RICARDO MACHUCA   GRAPHICS, DESIGN AND PRODUCTION BY SNATCHO      "
  0x20cd7="      KEYS AND JOYSTICK   ALT-KICK   SPACE-PUNCH   F1-PAUSE   F10-ABORT      "
  0x20d4e="      KEYBOARD   CTRL-JUMP   SHIFT-DUCK DOWN   /-LEFT   SHIFT-RIGHT   SPACE-PUNCH   ALT-KICK   F1-PAUSE   F10-ABORT      "
*/
//  OG_AW_WriteString(0x18796+00 | NONULL,"WAR.......");
  OG_AW_WriteString(0x18796+14 | NONULL,"..........");
  OG_AW_WriteString(0x18796+28 | NONULL,"..........");

/*
  OG_AW_WriteString(0xa578,"1. KEYBOARD");
  OG_AW_WriteString(0xa584,"2. KEYS AND JOYSTICK");
  OG_AW_WriteString(0xa599,"PRESS FIRE TO START");
  OG_AW_WriteString(0xa5ad,"INSERT DISK 2");
  OG_AW_WriteString(0xa5bb,"ACCESS CODE");
//  OG_AW_WriteString(0xa5c7,"101069");
  OG_AW_WriteString(0x20cd7,"      KEYS AND JOYSTICK   ALT-KICK   SPACE-PUNCH   "
                        "F1-PAUSE   F10-ABORT      ");
  OG_AW_WriteString(0x20d4e,"      KEYBOARD  CTRL-JUMP   SHIFT-DUCK DOWN   /-LEFT   "
                        "SHIFT-RIGHT   SPACE-PUNCH   ALT-KICK   F1-PAUSE   F10-ABORT      ");
*/
  OG_AW_WriteString(0x20b86,"             ");

  OGDrawSprite(0,0x1c506);
  OGDrawSprite(1,0x1cde6);
  OGDrawSprite(2,0x1d106);
  OGDrawSprite(3,0x1d286);
  OGDrawSprite(4,0x1c6de);
  OGDrawSprite(5,0x1bfe6);
  OGDrawSprite(6,0x1d346);
  OGDrawSprite(7,0x1d446);

  if (OGNumLives){
    m68k_dpoke(0x8E94+2,(WORD)min(OGNumLives+1,4));
  }
  if (OGInfinite){
    m68k_dpoke(0x0150FA,0x4e71);
    m68k_dpoke(0x0150FA+2,0x4e71);
  }

#elif ONEGAME_IDX==OG_AW2_IDX

  m68k_dpoke(0xCFDC,0xffff); // Title screen
  m68k_dpoke(0xCFE0,0xffff); // Restore palette

  m68k_dpoke(0xcf38,0x4e71); // Disable scroller
  m68k_dpoke(0xcf38+2,0x4e71); // Disable scroller
  m68k_dpoke(0xcf3c,0x4e71); // Disable scroller
  m68k_dpoke(0xcf3c+2,0x4e71); // Disable scroller

  m68k_dpoke(0xcf54,0x4e71); // Disable Dinamic logo
  m68k_dpoke(0xcf54+2,0x4e71); // Disable Dinamic logo
  m68k_dpoke(0xCF60,0x4e71); // Disable Dinamic logo
  m68k_dpoke(0xCF60+2,0x4e71); // Disable Dinamic logo

/*
  0x97d6="AFTER THE WAR    COPYRIGHT DINAMIC SOFTWARE 1989   PROGRAM BY JOSE LOPEZ   "
         "MUSIC BY MAC   SOUND FX BY VICTOR RUIZ   "
         "ADDITIONAL SOUNDS, MAPS AND GRAPHICS BY JAVIER CUBEDO   "
         "ARTISTIC DESIGN BY RICARDO MACHUCA   GRAPHICS, DESIGN AND PRODUCTION BY SNATCHO      "
  0x9920="      KEYBOARD    CTRL-RAISE GUN    SHIFT-LOWER GUN    /-LEFT    SHIFT-RIGHT    "
         "RETURN-CHANGE FLOOR    SPACE-SHOOT    ALT-DUCK DOWN    F1-PAUSE    F10-ABORT      "
  0x99e2="      JOYSTICK   SPACE-SHOOT   ALT-DUCK DOWN   F1-PAUSE   F10-ABORT      "
  0x9a54="      KEYS AND JOYSTICK   CTRL OR SHIFT-CHANGE FLOOR   SPACE-SHOOT   "
         "ALT-DUCK DOWN   F1-PAUSE   F10-ABORT      "
  0x9aec="  THIS IS AFTER THE WAR.IF YOU HAVE      ENJOYED IT GET READY FOR A.M.C.    "
         "ASTRO-MARINE CORPS BY DINAMIC      SOON ON YOUR SCREEN      "
  0x9b8d="1. KEYBOARD"
  0x9b99="2. JOYSTICK"
  0x9ba5="3. KEYS AND JOYSTICK"
  0x9bba="PRESS FIRE TO START"
  0x9bce="INPUT ACCESS CODE"
*/
//  OG_AW_WriteString(0x14f2e | NONULL,"WAR.......");
  OG_AW_WriteString(0x14f3c | NONULL,"..........");
  OG_AW_WriteString(0x14f4a | NONULL,"..........");
  OG_AW_WriteString(0x97d6,"    "); // This never appears, but changing it will make the hiscores appear quicker
/*
  OG_AW_WriteString(0x9920,"      KEYBOARD    CTRL-RAISE GUN    SHIFT-LOWER GUN    /-LEFT    SHIFT-RIGHT    "
         "RETURN-CHANGE FLOOR    SPACE-SHOOT    ALT-DUCK DOWN    F1-PAUSE    F10-ABORT      ");
  OG_AW_WriteString(0x99e2,"      JOYSTICK  SPACE-SHOOT   ALT-DUCK DOWN   F1-PAUSE   F10-ABORT      ");
  OG_AW_WriteString(0x9a54,"      KEYS AND JOYSTICK   CTRL OR SHIFT-CHANGE FLOOR   SPACE-SHOOT   "
         "ALT-DUCK DOWN   F1-PAUSE   F10-ABORT      ");
  OG_AW_WriteString(0x9b8d,"1. KEYBOARD");
  OG_AW_WriteString(0x9b99,"2. JOYSTICK");
  OG_AW_WriteString(0x9ba5,"3. KEYS AND JOYSTICK");
  OG_AW_WriteString(0x9bba,"PRESS FIRE TO START");
  OG_AW_WriteString(0x9bce,"INPUT ACCESS CODE");
                            0123456789012345678901234567890123456789*/
  OG_AW_WriteString(0x9aec,"       ESTO HA SIDO AFTER THE WAR       "
                           "     SI HAS DISFRUTADO, PREPARATE PARA  "
                           "             ASTRO-MARINE CORPS         "
                           "             PRONTO EN TU PC            ");

  OGDrawSprite(0,0x41D8E);
  OGDrawSprite(1,0x444ce);
  OGDrawSprite(2,0x40F1E);
  OGDrawSprite(3,0x44c1e);
  OGDrawSprite(4,0x41526);
  OGDrawSprite(5,0x4463e);
  OGDrawSprite(6,0x41fee+32);
  OGDrawSprite(7,0x4478e);
  OGDrawSprite(8,0x44a8e);
  OGDrawSprite(9,0x44d26);

  m68k_dpoke(0x01198E,0xffff); // rts from load routine (need to draw sprites 10-13 here)

  if (OGNumLives){
    m68k_poke(0x11898+3,(BYTE)min(OGNumLives,3));
  }
  if (OGInfinite){
    m68k_dpoke(0xE48C,0x4e71);
    m68k_dpoke(0xE48C+2,0x4e71);
  }

#endif
  SET_PC(PC32); // Might start at altered address, reset prefetch

  CurrentCuts.DeleteAll();
  for (int n=0;n<nSI;n++) CurrentCuts.Add(si[n]);
  ShortcutBox.CurrentCutSelType=2;

  return true;
}
//---------------------------------------------------------------------------
void OGSetRestart()
{
  if (runstate==RUNSTATE_RUNNING) PostRunMessage();
  OGStopAction=OG_RESTART;
}
//---------------------------------------------------------------------------
void OGVBL()
{
#if ONEGAME_IDX==OG_NM2_IDX
  if (OGUnlimitedAmmo){
    m68k_poke(0x12C46,0x39);
    m68k_poke(0x12C47,0x39);
  }
#elif ONEGAME_IDX==OG_SAT1_IDX
  if (OGInvincible) if (m68k_dpeek(0xA7DE)>0) m68k_dpoke(0xA7DE,0x40);
  if (OGUnlimitedTime) m68k_dpoke(0xA7E4,0);
#elif ONEGAME_IDX==OG_SAT2_IDX
  if (OGInvincible){
    m68k_dpoke(0x92C2,0x50);
    m68k_dpoke(0x92C6,0xa0);
  }
  if (OGUnlimitedTime) m68k_dpoke(0x94BA,0x50);
#elif ONEGAME_IDX==OG_AW1_IDX
  if (OGInvincible) m68k_dpoke(0x95B4,0x44);
  if (OGUnlimitedTime) m68k_dpoke(0x95BA,0);
#elif ONEGAME_IDX==OG_AW2_IDX
  if (OGInvincible) m68k_dpoke(0xD2Fa,0x0);
  if (OGUnlimitedTime) m68k_dpoke(0xD2BE,9999);
#endif
}
//---------------------------------------------------------------------------
void OGIntercept()
{
  MEM_ADDRESS OGTitleAd=0,OGPalAd=0;
  int OGBmpNum=0;
  switch (pc){
#if ONEGAME_IDX==OG_NM1_IDX
    case 0x21AA: // Title screen routine
      areg[0]=0x249E;
      SET_PC(0x21B0);
      OGTitleAd=xbios2;
      break;
#elif ONEGAME_IDX==OG_NM2_IDX
    case 0x216E: // Title screen routine
      areg[0]=0x8AB4;
      SET_PC(0x2174);

      OGTitleAd=xbios2;
      OGPalAd=areg[0];
      for (int i=0;i<16;i++) OGStorePal[i]=m68k_dpeek(OGPalAd + i*2);
      break;
    case 0x2178: // Restore palette
      m68k_lpoke(0x12E80,0x006A0098);
      SET_PC(0x2182);

      for (int i=0;i<16;i++) m68k_dpoke(0x8AB4 + i*2,OGStorePal[i]);
      break;
    case 0x387E: // Skip "Get ready.."
      SET_PC(0x389E);
      break;
#elif ONEGAME_IDX==OG_SAT1_IDX
    case 0x153E: // Title screen routine
      areg[0]=0xA67E;
      SET_PC(0x1544);

      OGTitleAd=xbios2;
      OGPalAd=areg[0];
      for (int i=0;i<16;i++) OGStorePal[i]=m68k_dpeek(OGPalAd + i*2);
      break;
    case 0x154A: // Restore palette
      m68k_dpoke(0x16ea,0x36B0);
      SET_PC(0x1552);

      for (int i=0;i<16;i++) m68k_dpoke(0xA67E + i*2,OGStorePal[i]);
      break;
    case 0x1984: // "Get ready for battle"
    case 0x1278: // "Time Over"
    case 0x17E8: // "Well done warrior"
    case 0x1802: // "The way to the magic"
    case 0x181C: // "Kingdom is closer"
    case 0x18E0: // "You got it"
      areg[0]=OG_TEXT_ADDRESS+m68k_dpeek(pc+2);
      SET_PC(pc+4);
      break;
#elif ONEGAME_IDX==OG_SAT2_IDX
    case 0x11c4: // Title screen routine
      areg[0]=0xD8C2;
      SET_PC(0x11ca);

      OGTitleAd=0x77300;
      OGPalAd=areg[0];
      for (int i=0;i<16;i++) OGStorePal[i]=m68k_dpeek(OGPalAd + i*2);
      break;
    case 0x11CE: // Restore palette
      m68k_dpoke(0x9758,0x64);
      SET_PC(0x11d6);
      for (int i=0;i<16;i++) m68k_dpoke(0xD8C2 + i*2,OGStorePal[i]);
      break;
    case 0x35BE: // Draw description of item in shop
      m68k_dpoke(0x93A6,0xe6);
      SET_PC(0x35C6);
      if (areg[0]==0x95DE) areg[0]=OG_TEXT_ADDRESS+m68k_dpeek(0x35BE+2);
      break;
#elif ONEGAME_IDX==OG_AW1_IDX
    case 0xa0aa: // Title screen routine
      areg[7]-=4;
      m68k_lpoke(areg[7],0xA0b0);
      SET_PC(0xA678);

      OGTitleAd=0x78000;
      OGPalAd=0x18832;
      for (int i=0;i<16;i++) OGStorePal[i]=m68k_dpeek(OGPalAd + i*2);
      break;
    case 0xA0b0: // Restore palette
      areg[0]=0x187CE;
      SET_PC(0xA0b6);

      for (int i=0;i<16;i++) m68k_dpoke(0x18832 + i*2,OGStorePal[i]);
      break;
    case 0x8986: // Load screen fade up
      areg[7]-=4;
      m68k_lpoke(areg[7],0x8988);
      SET_PC(0xA670);

      OGTitleAd=xbios2;
      OGPalAd=0x18812;
      OGBmpNum=1;
      for (int i=0;i<16;i++) OGStorePal[i]=m68k_dpeek(OGPalAd + i*2);
      break;
    case 0x8988: // Load screen restore pal
      SET_PC(0x898a);
      for (int i=0;i<16;i++) m68k_dpoke(0x18812 + i*2,OGStorePal[i]);
      break;

    case 0x8CEC: // End screen fade up
      areg[7]-=4;
      m68k_lpoke(areg[7],0x8CEE);
      SET_PC(0xA678);

      OGTitleAd=xbios2;
      OGPalAd=0x18832;
      OGBmpNum=2;
      for (int i=0;i<16;i++) OGStorePal[i]=m68k_dpeek(OGPalAd + i*2);
      break;
    case 0x8CEE: // End screen restore pal
      SET_PC(0x8CF2);
      for (int i=0;i<16;i++) m68k_dpoke(0x18832 + i*2,OGStorePal[i]);
      break;

    case 0x8996: // Fade out before load hook
      areg[7]-=4;
      m68k_lpoke(areg[7],0x8998); // return address
      SET_PC(0x17976);
      break;
    case 0x8998: // Do what above instruction should hook
      areg[7]-=4;
      m68k_lpoke(areg[7],0x899a); // return address
      SET_PC(0x8e24);
      break;

#elif ONEGAME_IDX==OG_AW2_IDX
    case 0xCFDC: // Title screen routine
      areg[7]-=4;
      m68k_lpoke(areg[7],0xCFE0); // return address
      SET_PC(0xfdf4);

      OGTitleAd=0x78000;
      OGPalAd=0x14312;
      for (int i=0;i<16;i++) OGStorePal[i]=m68k_dpeek(OGPalAd + i*2);
      break;
    case 0xCFE0: // Restore palette
      areg[0]=0x125B2;
      SET_PC(0xCFE4);

      for (int i=0;i<16;i++) m68k_dpoke(0x14312 + i*2,OGStorePal[i]);
      break;
    case 0x01198E: // Load enemy at end of level 2
      SET_PC(m68k_lpeek(r[15]));
      r[15]+=4;

      OGDrawSprite(10,0x69a62);
      OGDrawSprite(11,0x69B62);
      OGDrawSprite(12,0x69C22);
      OGDrawSprite(13,0x69CA2);
      break;
#endif
  }
  if (pOGTitle && OGTitleAd){
    MEM_ADDRESS ad=OGTitleAd & 0xffffff;
    MEM_ADDRESS ad_end=ad+32000;
    WORD *p=pOGTitle;
    if (OGBmpNum) if (pOGExtraScreen[OGBmpNum-1]) p=pOGExtraScreen[OGBmpNum-1];
    if (ad_end<=mem_len){
      while (ad<ad_end){
        DPEEK(ad)=*(p++);
        ad+=2;
      }
      if (OGBmpNum==0){
#if ONEGAME_IDX==OG_SAT2_IDX
        MEM_ADDRESS ad2=0x2B500;
#elif ONEGAME_IDX==OG_AW1_IDX
        MEM_ADDRESS ad2=0x3CE00;
#elif ONEGAME_IDX==OG_AW2_IDX
        MEM_ADDRESS ad2=0x70300;
#else
        MEM_ADDRESS ad2=0;
#endif
        if (ad2){
          ad=OGTitleAd & 0xffffff;
          while (ad<ad_end) PEEK((ad2++))=PEEK((ad++));
        }
      }
      if (OGPalAd){
        for (int i=0;i<16;i++){
          DPEEK(OGPalAd)=*(p++);
          OGPalAd+=2;
        }
      }else{
        for (int i=0;i<16;i++) STpal[i]=*(p++);
        palette_convert_all();
      }
    }
  }
}
//---------------------------------------------------------------------------
void OGLoadData(ConfigStoreFile *pCSF)
{
  OGInfinite=pCSF->GetInt("Cheats","InfiniteLives",OGInfinite);
  OGInvincible=pCSF->GetInt("Cheats","Invincibility",OGInvincible);
  OGNumLives=pCSF->GetInt("Cheats","NumLives",OGNumLives);
  OGUnlimitedTime=pCSF->GetInt("Cheats","UnlimitedTime",OGUnlimitedTime);
  OGUnlimitedAmmo=pCSF->GetInt("Cheats","UnlimitedAmmo",OGUnlimitedAmmo);
  OGEasyMode=pCSF->GetInt("Cheats","EasyMode",OGEasyMode);
  int RunSpeedPercent=pCSF->GetInt("Cheats","RunSpeed",0);
  if (RunSpeedPercent) run_speed_ticks_per_second=100000/RunSpeedPercent;
}
//---------------------------------------------------------------------------
void OGHandleQuit()
{
  if (OGStopAction==OG_QUIT){
    if (IDYES==Alert("Estás seguro?","Salir?",MB_ICONQUESTION | MB_YESNO)) QuitSteem();
  }else if (OGStopAction==OG_RESTART){
    if (OGInit()==0) QuitSteem();
  }
  if (Quitting==0){
    OGStopAction=0;
    PostRunMessage();
  }
}
//---------------------------------------------------------------------------
void OGCleanUp()
{
  if (pOGSprites) delete[] pOGSprites;
  pOGSprites=NULL;

  for (int n=0;n<2;n++){
    if (pOGExtraScreen[n]) delete[] pOGExtraScreen[n];
    pOGExtraScreen[n]=NULL;
  }

  FloppyDrive[0].RemoveDisk();
  if (OGDiskPath.NotEmpty()) DeleteFile(OGDiskPath);
  OGDiskPath="";
}
//---------------------------------------------------------------------------
void OGWriteSector(int Side,int Track,int Sector,int Bytes)
{
  FILE *f=fopen(WriteDir+SLASH+"hiscores","r+b");
  if (f==NULL) return;

  int SavePos;
  for(;;){
    int SavedSide=-1,SavedTrack=-1,SavedSector=-1,SavedBytes=-1;
    SavePos=ftell(f);
    fread(&SavedSide,1,sizeof(int),f);
    fread(&SavedTrack,1,sizeof(int),f);
    fread(&SavedSector,1,sizeof(int),f);
    fread(&SavedBytes,1,sizeof(int),f);
    if (SavedSide==-1 || SavedTrack==-1 || SavedSector==-1 || SavedBytes==-1) break;
    if (SavedSide==Side && SavedTrack==Track && SavedSector==Sector){
      if (SavedBytes==Bytes){
        // Can save over here
        break;
      }else{
        fseek(f,SavePos+sizeof(int)*2,SEEK_SET);
        SavedSector=256; // Clear this one
        fwrite(&SavedSector,1,sizeof(int),f);
        fseek(f,SavePos+sizeof(int)*4+SavedBytes,SEEK_SET);
      }
    }else{
      fseek(f,SavedBytes,SEEK_CUR);
    }
  }
  fseek(f,SavePos,SEEK_SET);
  fwrite(&Side,1,sizeof(int),f);
  fwrite(&Track,1,sizeof(int),f);
  fwrite(&Sector,1,sizeof(int),f);
  fwrite(&Bytes,1,sizeof(int),f);
  for (int n=0;n<Bytes;n++){
    if (dma_address+n < mem_len){
      BYTE b=PEEK(dma_address+n);
      fwrite(&b,1,1,f);
    }
  }
  fclose(f);
}
//---------------------------------------------------------------------------
void OGRestoreSectors()
{
  FILE *f=fopen(WriteDir+SLASH+"hiscores","rb");
  if (f==NULL) return;

  TFloppyImage *floppy=&(FloppyDrive[0]);
  BYTE Sector[1024];
  for(;;){
    int SavedSide=-1,SavedTrack=-1,SavedSector=-1,SavedBytes=-1;
    fread(&SavedSide,1,sizeof(int),f);
    fread(&SavedTrack,1,sizeof(int),f);
    fread(&SavedSector,1,sizeof(int),f);
    fread(&SavedBytes,1,sizeof(int),f);
    if (SavedSide==-1 || SavedTrack==-1 || SavedSector==-1 || SavedBytes==-1) break;
    if (SavedSector<256){
      if (floppy->SeekSector(SavedSide,SavedTrack,SavedSector,0)==0){
        fread(Sector,1,SavedBytes,f);
        fwrite(Sector,1,min(int(floppy->BytesPerSector),SavedBytes),floppy->f);
      }
    }else{
      fseek(f,SavedBytes,SEEK_CUR);
    }
  }
  fclose(f);
}
//---------------------------------------------------------------------------

