#ifdef WIN32

typedef struct{
  char InternalUse[12];    // Used internally by the dll
  int Time;                // File time
  int Size;                // File size
  int CompressSize;        // Size in zipfile
  int HeaderOffset;        // File offset in zip
  long Crc;                // CRC, sort of checksum
  char FileName[260];      // File name
  WORD PackMethod;         /* 0=Stored, 1=Shrunk, 2=Reduced 1, 3=Reduced 2, 4=Reduced 3, 5=Reduced 4,
                              6=Imploded,7=Tokenized (format does not exist), 8=Deflated,
                              More than 8=Unknown method.
                              For this DLL this number can only be 0, 8, or more than 8
                              */
  WORD Attr;               // File attributes
  WORD Flags;              // Only used by ARJ unpacker (LOBYTE: arj_flags, HIBYTE: file_type)
}PackStruct;

int (_stdcall *GetFirstInZip)(char*,PackStruct*); //find out what files are in the ZIP file (first file)
int (_stdcall *GetNextInZip)(PackStruct*);        //get next file in ZIP
void (_stdcall *CloseZipFile)(PackStruct*);       //free buffers and close ZIP after GetFirstInZip()
BYTE (_stdcall *isZip)(char*);                    //determine if a file is a ZIP file
int (_stdcall *UnzipFile)(char*,char*,WORD,long,void*,long);        //unzipping

#define UNZIP_Ok           0               // Unpacked ok
#define UNZIP_CRCErr       1               // CRC error
#define UNZIP_WriteErr     2               // Error writing out file: maybe disk full
#define UNZIP_ReadErr      3               // Error reading zip file
#define UNZIP_ZipFileErr   4               // Error in zip structure
#define UNZIP_UserAbort    5               // Aborted by user
#define UNZIP_NotSupported 6               // ZIP Method not supported!
#define UNZIP_Encrypted    7               // Zipfile encrypted
#define UNZIP_InUse        -1              // DLL in use by other program!
#define UNZIP_DLLNotFound  -2              // DLL not loaded!

HINSTANCE hUnzip=NULL;
//---------------------------------------------------------------------------
void LoadUnzipDLL()
{
	hUnzip=LoadLibrary("unzipd32.dll");
	enable_zip=(hUnzip!=NULL);
	if (hUnzip){
		GetFirstInZip=(int(_stdcall*)(char*,PackStruct*))GetProcAddress(hUnzip,"GetFirstInZip");
		GetNextInZip=(int(_stdcall*)(PackStruct*))GetProcAddress(hUnzip,"GetNextInZip");
		CloseZipFile=(void(_stdcall*)(PackStruct*))GetProcAddress(hUnzip,"CloseZipFile");
		isZip=(BYTE(_stdcall*)(char*))GetProcAddress(hUnzip,"isZip");
		UnzipFile=(int(_stdcall*)(char*,char*,WORD,long,void*,long))GetProcAddress(hUnzip,"unzipfile");

		if (!(GetFirstInZip && GetNextInZip && CloseZipFile && isZip && UnzipFile)){
			FreeLibrary(hUnzip);
      hUnzip=NULL;
      enable_zip=false;
	  }
	}
}
//---------------------------------------------------------------------------
#endif
