#define FI_ENUM(x)      enum x
#define FI_STRUCT(x)	struct x

FI_STRUCT (FIBITMAP) { void *data; };

FI_ENUM(FREE_IMAGE_FORMAT) {
	FIF_UNKNOWN = -1,
	FIF_BMP = 0,
	FIF_ICO,
	FIF_JPEG,
	FIF_JNG,
	FIF_KOALA,
	FIF_LBM,
	FIF_MNG,
	FIF_PBM,
	FIF_PBMRAW,
	FIF_PCD,
	FIF_PCX,
	FIF_PGM,
	FIF_PGMRAW,
	FIF_PNG,
	FIF_PPM,
	FIF_PPMRAW,
	FIF_RAS,
	FIF_TARGA,
	FIF_TIFF,
	FIF_WBMP,
	FIF_PSD,
	FIF_CUT,
	FIF_IFF = FIF_LBM,
};

#define BMP_DEFAULT         0
#define BMP_SAVE_RLE        1
#define CUT_DEFAULT         0
#define ICO_DEFAULT         0
#define ICO_FIRST           0
#define ICO_SECOND          0
#define ICO_THIRD           0
#define IFF_DEFAULT         0
#define JPEG_DEFAULT        0
#define JPEG_FAST           1
#define JPEG_ACCURATE       2
#define JPEG_QUALITYSUPERB  0x80
#define JPEG_QUALITYGOOD    0x100
#define JPEG_QUALITYNORMAL  0x200
#define JPEG_QUALITYAVERAGE 0x400
#define JPEG_QUALITYBAD     0x800
#define KOALA_DEFAULT       0
#define LBM_DEFAULT         0
#define MNG_DEFAULT         0
#define PCD_DEFAULT         0
#define PCD_BASE            1
#define PCD_BASEDIV4        2
#define PCD_BASEDIV16       3
#define PCX_DEFAULT         0
#define PNG_DEFAULT         0
#define PNG_IGNOREGAMMA		  1		// avoid gamma correction
#define PNM_DEFAULT         0
#define PNM_SAVE_RAW        0       // If set the writer saves in RAW format (i.e. P4, P5 or P6)
#define PNM_SAVE_ASCII      1       // If set the writer saves in ASCII format (i.e. P1, P2 or P3)
#define RAS_DEFAULT         0
#define TARGA_DEFAULT       0
#define TARGA_LOAD_RGB888   1       // If set the loader converts RGB555 and ARGB8888 -> RGB888.
#define TARGA_LOAD_RGB555   2       // This flag is obsolete
#define TIFF_DEFAULT        0
#define WBMP_DEFAULT        0
#define PSD_DEFAULT         0

typedef void (__stdcall *FI_INITPROC)(BOOL);
typedef void (__stdcall *FI_DEINITPROC)();
typedef FIBITMAP* (__stdcall *FI_CONVFROMRAWPROC)(BYTE*,int,int,int,UINT,UINT,UINT,UINT,BOOL);
typedef BOOL (__stdcall *FI_SAVEPROC)(FREE_IMAGE_FORMAT,FIBITMAP*,const char *,int);
typedef void (__stdcall *FI_FREEPROC)(FIBITMAP*);
typedef BOOL (__stdcall *FI_SUPPORTBPPPROC)(FREE_IMAGE_FORMAT,int);

FI_INITPROC FreeImage_Initialise;
FI_DEINITPROC FreeImage_DeInitialise;
FI_CONVFROMRAWPROC FreeImage_ConvertFromRawBits;
FI_SAVEPROC FreeImage_Save;
FI_FREEPROC FreeImage_Free;
FI_SUPPORTBPPPROC FreeImage_FIFSupportsExportBPP;

