//
// PASTI DLL
// FDC Emulator
//	external interface definitions
//

#ifndef DllExport
#define DllExport __declspec( dllimport )
#endif

#ifndef WINVER
typedef void *HWND;
#endif

#define PASTI_VERSION		0x0002

// Dll flags

#define PASTI_DFDEBUG		0x01
#define PASTI_DFBETA		0x8000

// Application flags

#define PASTI_AFUSEHBL		0x01
#define PASTI_AFUPDONLY		0x02
#define PASTI_AFRDMA		0x04

// I/O mode

#define PASTI_IOUPD			0x00
#define PASTI_IOREAD		0x01
#define PASTI_IOWRITE		0x02

// Image load/save modes

#define PASTI_LDQUERYSIZE	0x00
#define PASTI_LDFNAME		0x01
#define PASTI_LDMEM			0x02

#define PASTI_LDUSEGEOMETRY	0x100

// Image types

#define PASTI_ITNODISK		0x00
#define PASTI_ITST			0x01
#define PASTI_ITMSA			0x02
#define PASTI_ITPROT		0x03

#define PASTI_ITUNPROT		0x01

// Configuration flags

#define PASTI_CFDRIVES		0x01
#define PASTI_CFOPTIONS		0x02
#define PASTI_CFDETAILSPEED	0x04
#define PASTI_CFSIMPLESPEED	0x08

#define PASTI_CFAPPOPTIONS	(PAST_CFDRIVES)
#define PASTI_CFUSROPTIONS	(PASTI_CFOPTIONS | PASTI_CFDETAILSPEED)
#define PASTI_CFALL			(PASTI_CFAPPOPTIONS | PASTI_CFUSROPTIONS)

// Options setting

#define PASTI_OPWPOFF		0x100
#define PASTI_OPNORAND		0x200
#define PASTI_OPFORMAT		0x400

// Warnings

#define PASTI_WNUNIMG		0x01
#define PASTI_WNRDTRK		0x02
#define PASTI_WNWRITEPROT	0x04
#define PASTI_WNUNIMPLWRT	0x08
#define PASTI_WNONCEPTRK	0x10


// Dialogs flags

#define PASTI_DFNOINIT		0x01
#define PASTI_DFNOAPPLY		0x02

// Load Save config modes

#define PASTI_LCSTRINGS		0

// pastiBreakpoint subfunc codes

#define PASTI_BRK_SET		1
#define PASTI_BRK_GET		2
#define PASTI_BRK_DEL		3
#define PASTI_BRK_KILL		4

// Error values

#define pastiErrNoerr		0
#define pastiErrNotInited	-1
#define pastiErrGeneric		-2
#define pastiErrOsErr		-3
#define pastiErrNoMem		-4
#define pastiErrFileFmt		-5
#define pastiErrUnimpl		-6
#define pastiErrInvalidDrv	-7
#define pastiErrNoDisk		-8
#define pastiErrInvParam	-9
#define pastiErrIncompatVersion	-10

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


struct pastiFUNCS
{
	BOOL ( *Io) ( int mode, struct pastiIOINFO *);
	BOOL ( *WritePorta) ( unsigned data, long cycles);

	BOOL ( *Config) ( struct pastiCONFIGINFO *);
	BOOL ( *GetConfig) ( struct pastiCONFIGINFO *pInfo);
	BOOL ( *HwReset) ( BOOL bPowerUp);
	int ( *GetLastError) ( void);

	BOOL ( *ImgLoad) ( int drive, BOOL bWprot, BOOL bDelay, long cycles,
		struct pastiDISKIMGINFO *);
	BOOL ( *SaveImg) ( int drive, BOOL bAlways, struct pastiDISKIMGINFO *);
	BOOL ( *Eject) ( int drive, long cycles);
	BOOL ( *GetBootSector) ( int drive, struct pastiBOOTSECTINFO *);
	int ( *GetFileExtensions) ( char *buf, int bufSize, BOOL bAll);

	BOOL ( *SaveState) ( struct pastiSTATEINFO *);
	BOOL ( *LoadState) ( struct pastiSTATEINFO *);

	BOOL ( *LoadConfig )( struct pastiLOADINI *, pastiCONFIGINFO *);
	BOOL ( *SaveConfig) ( struct pastiLOADINI *, const pastiCONFIGINFO *);

	BOOL ( *Peek) ( struct pastiPEEKINFO *);
	BOOL ( *Breakpoint) ( unsigned subfunc, int n, struct pastiBREAKINFO *);

	BOOL ( *DlgConfig) ( HWND hWnd, unsigned flags, struct pastiCONFIGINFO *);
	BOOL ( *DlgBreakpoint) ( HWND hWnd);
	BOOL ( *DlgStatus) ( HWND hWnd);
	BOOL ( *DlgFileProps) ( HWND hWnd, const char *fileName);

	BOOL ( *Extra) ( unsigned code, void *ptr);
};

struct pastiCALLBACKS
{
	BOOL ( *DmaXfer) ( const struct pastiDMAXFERINFO *);
	void ( *MotorOn) ( BOOL bOn);
	void ( *IntrqChg) ( BOOL bOn);

	void ( *loadDelay) ( int drive, BOOL bStart);

	void ( *LogMsg) ( const char *msg);
	void ( *WarnMsg) ( const char *msg);

	void ( *BreakHit) ( int n);
};

struct pastiINITINFO
{
	unsigned dwSize;

	unsigned applFlags;
	unsigned applVersion;

	const struct pastiCALLBACKS *cBacks;

	unsigned dllFlags;
	unsigned dllVersion;

	// array of dll public func pointers
	const struct pastiFUNCS *funcs;
};

struct pastiSPEEDINFO
{
	BOOL slowSectRead;
	BOOL slowTrackRead;

	BOOL slowRotation;
	BOOL slowSpinup;
	BOOL slowSettle;

	BOOL slowSeek;
	BOOL slowVerify;
	BOOL slowSnf;
};

struct pastiCONFIGINFO
{
	unsigned flags;						// Which options to configure and which not

	int ndrives;
	unsigned drvFlags;					// drivesides & tracks

	int logLevel;
	unsigned options;
	unsigned warnings;

	unsigned reserved;


	BOOL slowSpeed;						// Simplified speed settings

	BOOL fastUnprotDisk;				// Not copy-protected disks
	BOOL fastUnprotTracks;				// Not copy-protected tracks

	struct pastiSPEEDINFO SpeedInfo;	// Detailed speed settings

};

struct pastiGEOMETRY
{
	unsigned nSides;
	unsigned sectsPerTrack;
	unsigned nTracks;
	unsigned bytesPerSector;
};

struct pastiDISKIMGINFO
{
	unsigned mode;

	unsigned imgType;
	const char *fileName;
	void *fileBuf;

	long fileLength;
	long bufSize;

	struct pastiGEOMETRY geometry;

	BOOL bDirty;
};

struct pastiDMAXFERINFO
{
	BOOL memToDisk;
	unsigned xferLen;
	void *xferBuf;
	unsigned xferSTaddr;
};

struct pastiIOINFO
{
	unsigned addr;
	unsigned data;

	long stPC;				// Only for debugging
	long cycles;			// Current ST main clock cycles counter

	long updateCycles;		// Need update after these cycles

	BOOL intrqState;
	BOOL haveXfer;
	BOOL brkHit;

	struct pastiDMAXFERINFO xferInfo;
};

struct pastiLOADINI
{
	int mode;
	const char *name;
	void *buffer;
	unsigned bufSize;
};

struct pastiSTATEINFO
{
	void *buffer;
	unsigned long bufSize;
	unsigned long cycles;
};

struct pastiBOOTSECTINFO
{
	unsigned nBoots;
	unsigned readSize;
	void *buffer;
};

struct pastiPEEKINFO
{
	BOOL motorOn;
	BOOL intrqState;

	unsigned char commandReg;
	unsigned char statusReg;
	unsigned char sectorReg;
	unsigned char trackReg;
	unsigned char dataReg;

	unsigned char drvaTrack;
	unsigned char drvbTrack;

	unsigned char drvSelect;

	unsigned long dmaBase;
	unsigned dmaControl;
	unsigned dmaStatus;
	unsigned dmaCount;
};

struct pastiBREAKINFO
{
	BOOL enabled;

	// fdc registers
	unsigned char cmdRegValue;
	unsigned char cmdRegMask;
	unsigned char trackRegMin;
	unsigned char trackRegMax;
	unsigned char sectRegMin;
	unsigned char sectRegMax;
	unsigned char statusRegValue;
	unsigned char statusRegMask;

	unsigned char dataRegMin;
	unsigned char dataRegMax;

	// select bits
	unsigned char drvSelValue;
	unsigned char drvSelMask;

	// track head position
	unsigned char trkHeadMin;
	unsigned char trkHeadMax;

	// PC
	unsigned long pcMin;
	unsigned long pcMax;

	// dmabase
	unsigned long dmaAddrMin;
	unsigned long dmaAddrMax;
};


// typedef prototype for casting GetProcAddress pointer

typedef BOOL PASTIINITPROC( struct pastiINITINFO *);
typedef PASTIINITPROC *LPPASTIINITPROC;

DllExport BOOL pastiInit( struct pastiINITINFO *);

DllExport BOOL pastiConfig( struct pastiCONFIGINFO *);
DllExport BOOL pastiGetConfig( struct pastiCONFIGINFO *);
DllExport BOOL pastiHwReset( BOOL bPowerUp);
DllExport int pastiGetLastError( void);

DllExport BOOL pastiIo( int mode, struct pastiIOINFO *);
DllExport BOOL pastiWritePorta( unsigned data, long cycles);

DllExport BOOL pastiImgLoad( int drive, BOOL bWprot, BOOL bDelay, long cycles,
				struct pastiDISKIMGINFO *);
DllExport BOOL pastiSaveImg( int drive, BOOL bAlways, pastiDISKIMGINFO *);
DllExport BOOL pastiEject( int drive, long cycles);
DllExport int pastiGetFileExtensions( char *newExts, int bufSize, BOOL bAll);
DllExport BOOL pastiGetBootSector( int drive, struct pastiBOOTSECTINFO *);

DllExport BOOL pastiPeek( struct pastiPEEKINFO *);
DllExport BOOL pastiBreakpoint( unsigned subfunc, int n, struct pastiBREAKINFO *);

DllExport BOOL pastiSaveState( struct pastiSTATEINFO *);
DllExport BOOL pastiLoadState( struct pastiSTATEINFO *);

DllExport BOOL pastiLoadConfig( struct pastiLOADINI *, struct pastiCONFIGINFO *);
DllExport BOOL pastiSaveConfig( struct pastiLOADINI *, const struct pastiCONFIGINFO *);

DllExport BOOL pastiDlgConfig( HWND hWnd, unsigned flags,
											struct pastiCONFIGINFO *);
DllExport BOOL pastiDlgBreakpoint( HWND hWnd);
DllExport BOOL pastiDlgStatus( HWND hWnd);
DllExport BOOL pastiDlgFileProps( HWND hWnd, const char *fileName);

DllExport BOOL pastiExtra( unsigned code, void *ptr);


#ifdef __cplusplus
}				/* End of extern "C" { */
#endif
