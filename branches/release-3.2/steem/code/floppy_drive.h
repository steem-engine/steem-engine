#define FIMAGE_OK                  0
#define FIMAGE_WRONGFORMAT         1
#define FIMAGE_CANTOPEN            2
#define FIMAGE_FILEDOESNTEXIST     3
#define FIMAGE_NODISKSINZIP        4
#define FIMAGE_CORRUPTZIP          5
#define FIMAGE_DIMNOMAGIC          6
#define FIMAGE_DIMTYPENOTSUPPORTED 7

typedef struct{
  BYTE Track,Side,SectorNum,SectorLen,CRC1,CRC2;
}FDC_IDField;

typedef struct{
  int BytesPerSector,Sectors,SectorsPerTrack,Sides;
}BPBINFO;

class TFloppyImage
{
private:
  EasyStr ImageFile,MSATempFile,ZipTempFile,FormatTempFile;
public:
  TFloppyImage()             { f=NULL;Format_f=NULL;PastiDisk=0;PastiBuf=NULL;RemoveDisk();}
  ~TFloppyImage()            { RemoveDisk(); }

  int SetDisk(EasyStr,EasyStr="",BPBINFO* = NULL,BPBINFO* = NULL);
  EasyStr GetDisk()  { return ImageFile; }
  bool ReinsertDisk();
  void RemoveDisk(bool=0);
  bool DiskInDrive() { return f!=NULL || PastiDisk; }
  bool NotEmpty() { return DiskInDrive(); }
  bool Empty()       { return DiskInDrive()==0; }
  bool IsMSA()       { return MSATempFile.NotEmpty(); }
  bool IsZip()       { return ZipTempFile.NotEmpty(); }
  bool BeenFormatted() { return Format_f!=NULL; }
  bool NotBeenFormatted() { return Format_f==NULL; }
  bool SeekSector(int,int,int,bool=0);
  long GetLogicalSector(int,int,int,bool=0);
  int GetIDFields(int,int,FDC_IDField*);
  int GetRawTrackData(int,int);
  bool OpenFormatFile();
  bool ReopenFormatFile();

  FILE *f,*Format_f;
  bool ReadOnly;
  short BytesPerSector,Sides,SectorsPerTrack,TracksPerSide;
  EasyStr DiskName,DiskInZip;
  DWORD DiskFileLen;

  BYTE *PastiBuf;
  int PastiBufLen;
  bool STT_File,PastiDisk;
  DWORD STT_TrackStart[2][FLOPPY_MAX_TRACK_NUM+1];
  WORD STT_TrackLen[2][FLOPPY_MAX_TRACK_NUM+1];

  bool DIM_File,ValidBPB;

  bool TrackIsFormatted[2][FLOPPY_MAX_TRACK_NUM+1];
  int FormatMostSectors,FormatLargestSector;

  bool WrittenTo;
};

#ifdef IN_MAIN
TFloppyImage FloppyDrive[2];
bool FloppyArchiveIsReadWrite=0;
#else
extern TFloppyImage FloppyDrive[2];
#endif

