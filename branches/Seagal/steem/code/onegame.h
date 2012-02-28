#ifdef IN_MAIN
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)

#endif

EXT void OGVBL(),OGIntercept(),OGWriteSector(int,int,int,int),OGHandleQuit(),OGSetRestart();

#define OG_TEXT_ADDRESS 0xffa000
#define OG_TEXT_LEN 0x1000
EXT char OG_TextMem[OG_TEXT_LEN];
EXT int OGExtraSamplesPerVBL INIT(300);

#ifdef IN_MAIN
bool OGInit();
void OGRestoreSectors();
void OGLoadData(ConfigStoreFile*);
void OGCleanUp();

Str OGDiskPath;
bool OGInfinite=0,OGInvincible=0,OGUnlimitedTime=0,OGUnlimitedAmmo=0,OGEasyMode=0;
int OGNumLives=0;
WORD *pOGTitle,*pOGExtraScreen[2]={NULL,NULL},*pOGSprites=NULL;
WORD OGStorePal[16];

#define OG_QUIT 1
#define OG_RESTART 2
EXT int OGStopAction INIT(0);

#endif

#undef EXT
#undef INIT

