#define LOGSECTION LOGSECTION_FDC

int TFloppyImage::SetDisk(EasyStr File,EasyStr CompressedDiskName,BPBINFO *pDetectBPB,BPBINFO *pFileBPB)
{
  if (IsSameStr_I(File,ImageFile) && IsSameStr_I(CompressedDiskName,DiskInZip)) return 0;

  if (Exists(File)==0) return FIMAGE_FILEDOESNTEXIST;

  EasyStr OriginalFile=File,NewZipTemp;

  bool FileIsReadOnly=bool(GetFileAttributes(File) & FILE_ATTRIBUTE_READONLY);

  bool MSA=0,STT=0,DIM=0;
  char *dot=strrchr(File,'.');
  if (dot){
    MSA=IsSameStr_I(dot,".MSA");
    STT=IsSameStr_I(dot,".STT");
    DIM=IsSameStr_I(dot,".DIM");
  }

  // NewDiskInZip will be blank for default disk, RealDiskInZip will be the
  // actual name of the file in the zip that is a disk image
  EasyStr NewDiskInZip,RealDiskInZip;

  if (ExtensionIsDisk(dot)==DISK_COMPRESSED){
    if (!enable_zip) return FIMAGE_WRONGFORMAT;

    int HOffset=-1;
    bool CorruptZip=true;
    if (zippy.first(File)==0){
      CorruptZip=0;
      do{
        EasyStr fn=zippy.filename_in_zip();
        if (FileIsDisk(fn)==DISK_UNCOMPRESSED){
          if (CompressedDiskName.Empty() || IsSameStr_I(CompressedDiskName,fn.Text)){
            // Blank DiskInZip name means default disk (first in zip)
            MSA=has_extension(fn,"MSA");
            STT=has_extension(fn,"STT");
            DIM=has_extension(fn,"DIM");
            HOffset=zippy.current_file_offset;
            NewDiskInZip=CompressedDiskName;
            RealDiskInZip=fn.Text;
            break;
          }
        }
      }while (zippy.next()==0);
    }
    zippy.close();

    if (HOffset!=-1){
      NewZipTemp.SetLength(MAX_PATH);
      GetTempFileName(WriteDir,"ZIP",0,NewZipTemp);
      if (zippy.extract_file(File,HOffset,NewZipTemp,true /*bool hide*/)){
        DeleteFile(NewZipTemp);
        return FIMAGE_WRONGFORMAT;
      }
      File=NewZipTemp;
      if (FloppyArchiveIsReadWrite){
        FileIsReadOnly=0;
      }else{
        FileIsReadOnly=true;
      }
    }else{
      if (CorruptZip) return FIMAGE_CORRUPTZIP;
      return FIMAGE_NODISKSINZIP;
    }
  }

  // Open for read for an MSA (going to convert to ST and write to that)
  // and if the file is read-only, otherwise open for update
  FILE *nf=fopen(File,LPSTR((MSA || FileIsReadOnly) ? "rb":"r+b"));
  if (nf==NULL){
    if (NewZipTemp.NotEmpty()) DeleteFile(NewZipTemp);
    return FIMAGE_CANTOPEN;
  }

  if (GetFileLength(nf)<512){
    fclose(nf);
    if (NewZipTemp.NotEmpty()) DeleteFile(NewZipTemp);
    return FIMAGE_WRONGFORMAT;
  }

  fseek(nf,0,SEEK_SET);

  EasyStr NewMSATemp="";
  short MSA_SecsPerTrack,MSA_EndTrack,MSA_Sides;
  if (MSA){
    NewMSATemp.SetLength(MAX_PATH);
    GetTempFileName(WriteDir,"MSA",0,NewMSATemp);

    FILE *tf=fopen(NewMSATemp,"wb");
    if (tf){
      bool Err=0;
      short ID,StartTrack;

      fseek(nf,0,SEEK_SET);

      // Read header
      fread(&ID,2,1,nf);               SWAPBYTES(ID);
      fread(&MSA_SecsPerTrack,2,1,nf); SWAPBYTES(MSA_SecsPerTrack);
      fread(&MSA_Sides,2,1,nf);        SWAPBYTES(MSA_Sides);
      fread(&StartTrack,2,1,nf);       SWAPBYTES(StartTrack);
      fread(&MSA_EndTrack,2,1,nf);     SWAPBYTES(MSA_EndTrack);

      if (MSA_SecsPerTrack<1 || MSA_SecsPerTrack>FLOPPY_MAX_SECTOR_NUM ||
          MSA_Sides<0 || MSA_Sides>1 ||
          StartTrack<0 || StartTrack>FLOPPY_MAX_TRACK_NUM || StartTrack>=MSA_EndTrack ||
          MSA_EndTrack<1 || MSA_EndTrack>FLOPPY_MAX_TRACK_NUM){
        Err=true;
      }

      if (Err==0){
        // Read data
        WORD Len,NumRepeats;
        BYTE *TrackData=new BYTE[(MSA_SecsPerTrack*512)+16];
        BYTE *pDat,*pEndDat,dat;
        BYTE *STBuf=new BYTE[(MSA_SecsPerTrack*512)+16];
        BYTE *pSTBuf,*pSTBufEnd=STBuf+(MSA_SecsPerTrack*512)+8;
        for (int n=0;n<=MSA_EndTrack;n++){
          for (int s=0;s<=MSA_Sides;s++){
            if (n>=StartTrack){
              Len=0;
              fread(&Len,1,2,nf); SWAPBYTES(Len);
              if (Len>MSA_SecsPerTrack*512 || Len==0){
                Err=true;break;
              }
              if (WORD(fread(TrackData,1,Len,nf))<Len){
                Err=true;break;
              }
              if (Len==(MSA_SecsPerTrack*512)){
                fwrite(TrackData,Len,1,tf);
              }else{
                // Convert compressed MSA format track in TrackData to ST format in STBuf
                pSTBuf=STBuf;
                pDat=TrackData;
                pEndDat=TrackData+Len;
                while (pDat<pEndDat && pSTBuf<pSTBufEnd){
                  dat=*(pDat++);
                  if (dat==0xE5){
                    dat=*(pDat++);
                    NumRepeats=*LPWORD(pDat);pDat+=2;
                    SWAPBYTES(NumRepeats);
                    for (int s=0;s<NumRepeats && pSTBuf<pSTBufEnd;s++) *(pSTBuf++)=dat;
                  }else{
                    *(pSTBuf++)=dat;
                  }
                }
                if (pSTBuf>=pSTBufEnd){
                  Err=true;break;
                }
                fwrite(STBuf,MSA_SecsPerTrack*512,1,tf);
              }
            }else{
              ZeroMemory(TrackData,MSA_SecsPerTrack*512);
              if (n==0 && s==0){   // Write BPB
                *LPWORD(TrackData+11)=512;
                TrackData[13]=2;           // SectorsPerCluster

                *LPWORD(TrackData+17)=112; // nDirEntries
                *LPWORD(TrackData+19)=WORD(MSA_EndTrack * MSA_SecsPerTrack);

                *LPWORD(TrackData+22)=3;   // SectorsPerFAT
                *LPWORD(TrackData+24)=MSA_SecsPerTrack;
                *LPWORD(TrackData+26)=MSA_Sides;
                *LPWORD(TrackData+28)=0;
              }
              fwrite(TrackData,MSA_SecsPerTrack*512,1,tf);
            }
          }
          if (Err) break;
        }
        delete[] TrackData;
        delete[] STBuf;
      }

      fclose(tf);
      fclose(nf);

      if (Err==0){
        SetFileAttributes(NewMSATemp,FILE_ATTRIBUTE_HIDDEN);
        nf=fopen(NewMSATemp,"r+b");
        Err=(nf==NULL);
      }

      if (Err){
        DeleteFile(NewMSATemp);
        if (NewZipTemp.NotEmpty()) DeleteFile(NewZipTemp);
        return FIMAGE_WRONGFORMAT;
      }
    }else{
      // Couldn't open NewMSATemp
      fclose(nf);
      if (NewZipTemp.NotEmpty()) DeleteFile(NewZipTemp);
      return FIMAGE_CANTOPEN;
    }
  }

  bool f_ValidBPB=true;
  DWORD f_DiskFileLen=GetFileLength(nf);
  if (STT){
    bool Err=0;
    DWORD Magic;
    WORD Version,Flags,AllTrackFlags,NumTracks,NumSides;

    fread(&Magic,4,1,nf);
    fread(&Version,2,1,nf);
    fread(&Flags,2,1,nf);
    fread(&AllTrackFlags,2,1,nf);
    fread(&NumTracks,2,1,nf);
    fread(&NumSides,2,1,nf);
    Err=(Magic!=MAKECHARCONST('S','T','E','M') || Version!=1 || (AllTrackFlags & BIT_0)==0);
    if (Err==0){
      ZeroMemory(STT_TrackStart,sizeof(STT_TrackStart));
      ZeroMemory(STT_TrackLen,sizeof(STT_TrackLen));
      for (int s=0;s<NumSides;s++){
        for (int t=0;t<NumTracks;t++){
          fread(&STT_TrackStart[s][t],4,1,nf);
          fread(&STT_TrackLen[s][t],2,1,nf);
        }
      }
    }else{
      fclose(nf);

      if (NewMSATemp.NotEmpty()) DeleteFile(NewMSATemp);
      if (NewZipTemp.NotEmpty()) DeleteFile(NewZipTemp);

      return FIMAGE_WRONGFORMAT;
    }

    fseek(nf,0,SEEK_SET);

    RemoveDisk();
    f=nf;
    STT_File=true;
    TracksPerSide=NumTracks;
    Sides=NumSides;

    BytesPerSector=512;
    SectorsPerTrack=0xff; // Variable
  }else{
    BPBINFO bpbi={0,0,0,0};
    int HeaderLen=int(DIM ? 32:0);
    f_DiskFileLen-=HeaderLen;

    if (DIM){
      int Err=0;

      fseek(nf,0,SEEK_SET);
      WORD Magic;
      fread(&Magic,1,2,nf);
      if (Magic!=0x4242){
        Err=FIMAGE_DIMNOMAGIC;
      }else{
        BYTE UsedSectors;
        fseek(nf,3,SEEK_SET);
        fread(&UsedSectors,1,1,nf);
        if (UsedSectors!=0) Err=FIMAGE_DIMTYPENOTSUPPORTED;
      }

      if (Err){
        fclose(nf);

        if (NewMSATemp.NotEmpty()) DeleteFile(NewMSATemp);
        if (NewZipTemp.NotEmpty()) DeleteFile(NewZipTemp);

        return Err;
      }
    }

    // Always append the name of the real disk file in the zip to the name
    // of the steembpb file, even if we are using default disk
    // This is so we don't need 2 .steembpb files for the default disk
    EasyStr BPBFile=OriginalFile+RealDiskInZip+".steembpb";
    bool HasBPBFile=(GetCSFInt("BPB","Sides",0,BPBFile)!=0);

    if (MSA){
      bpbi.BytesPerSector=512;
    }else{
      fseek(nf,HeaderLen+11,SEEK_SET);
      fread(&bpbi.BytesPerSector,2,1,nf);
    }
    fseek(nf,HeaderLen+19,SEEK_SET);
    fread(&bpbi.Sectors,2,1,nf);
    fseek(nf,HeaderLen+24,SEEK_SET);
    fread(&bpbi.SectorsPerTrack,2,1,nf);
    fread(&bpbi.Sides,2,1,nf);
    if (pFileBPB) *pFileBPB=bpbi; // Store BPB exactly as it is in the file (for DiskMan)

    // A BPB is corrupt when one of its fields is totally wrong
    bool BPBCorrupt=0;
    if (bpbi.BytesPerSector!=128 && bpbi.BytesPerSector!=256 &&
          bpbi.BytesPerSector!=512 && bpbi.BytesPerSector!=1024) BPBCorrupt=true;
    if (bpbi.SectorsPerTrack<1 || bpbi.SectorsPerTrack>FLOPPY_MAX_SECTOR_NUM) BPBCorrupt=true;
    if (bpbi.Sides<1 || bpbi.Sides>2) BPBCorrupt=true;

    // Has to be exact length for Steem to accept it
    if (DWORD(bpbi.Sectors*bpbi.BytesPerSector)!=f_DiskFileLen || BPBCorrupt){
      f_ValidBPB=0;
      // If the BPB is only a few sectors out then we don't want to destroy
      // the value in BytesPerSector.
      if (BPBCorrupt) bpbi.BytesPerSector=512; // 99.9% of ST disks used sectors this size
    }

    if (f_ValidBPB==0){
      if (MSA){
        // Probably got a better chance of being right than guessing
        bpbi.SectorsPerTrack=MSA_SecsPerTrack;
        bpbi.Sides=short(MSA_Sides+1);
        bpbi.Sectors=short((MSA_EndTrack+1)*bpbi.SectorsPerTrack*bpbi.Sides);
      }else{
        // BPB's wrong, time to guess the format
        bpbi.SectorsPerTrack=0;

        bpbi.Sectors=short(f_DiskFileLen/bpbi.BytesPerSector);
        bpbi.Sides=WORD((bpbi.Sectors<1100) ? 1:2); // Total guess

        // Work out bpbi.SectorsPerTrack from bpbi.Sides and bpbi.Sectors
        bool Found=0;
        for (;;){
          for (int t=75;t<=FLOPPY_MAX_TRACK_NUM;t++){
            for (int s=8;s<=13;s++){
              if (bpbi.Sectors==(t+1)*s*bpbi.Sides){
                bpbi.SectorsPerTrack=WORD(s);
                Found=true;
                break;
              }
            }
            if (Found) break;
          }
          if (Found) break;

          if (bpbi.Sectors<10) break;

          bpbi.Sectors--;
        }
        if (bpbi.SectorsPerTrack==0 && HasBPBFile==0){
          fclose(nf);

          if (NewMSATemp.NotEmpty()) DeleteFile(NewMSATemp);
          if (NewZipTemp.NotEmpty()) DeleteFile(NewZipTemp);

          return FIMAGE_WRONGFORMAT;
        }
      }
    }
    if (pDetectBPB) *pDetectBPB=bpbi; // Steem's best guess (or the BPB if it is valid)

    if (HasBPBFile){
      // User specified disk parameters
      ConfigStoreFile CSF(BPBFile);
      bpbi.Sides=CSF.GetInt("BPB","Sides",2);
      bpbi.SectorsPerTrack=CSF.GetInt("BPB","SectorsPerTrack",9);
      bpbi.BytesPerSector=CSF.GetInt("BPB","BytesPerSector",512);
      bpbi.Sectors=CSF.GetInt("BPB","Sectors",1440);
      CSF.Close();
    }

    fseek(nf,HeaderLen,SEEK_SET);

    RemoveDisk();

    f=nf;
    BytesPerSector=short(bpbi.BytesPerSector);
    SectorsPerTrack=short(bpbi.SectorsPerTrack);
    Sides=short(bpbi.Sides);

    TracksPerSide=short(short(bpbi.Sectors/SectorsPerTrack)/Sides);

    DIM_File=DIM;
  }

  ReadOnly=FileIsReadOnly;
  ZipTempFile=NewZipTemp;
  DiskInZip=NewDiskInZip;
  MSATempFile=NewMSATemp;
  ImageFile=OriginalFile;
  ValidBPB=f_ValidBPB;
  DiskFileLen=f_DiskFileLen;
  WrittenTo=0;

  // Media change, write protect for 10 VBLs, unprotect for 10 VBLs, wp for 10
  if (this==&FloppyDrive[0]) floppy_mediach[0]=30;
  if (this==&FloppyDrive[1]) floppy_mediach[1]=30;

  log("");
  log(EasyStr("FDC: Inserted disk ")+OriginalFile);
  log(EasyStr("     Into drive ")+LPSTR(floppy_current_drive() ? "B":"A")+" its BPB was "+LPSTR(ValidBPB ? "valid.":"invalid."));
  log(EasyStr("     BytesPerSector=")+BytesPerSector+", SectorsPerTrack="+SectorsPerTrack+
      ", Sides="+Sides);
  log(Str("     TracksPerSide=")+TracksPerSide+", ReadOnly="+ReadOnly);
  log("");

  return 0;
}
//---------------------------------------------------------------------------
bool TFloppyImage::ReinsertDisk()
{
  if (Empty()) return 0;

  fclose(f);
  if (ZipTempFile.NotEmpty()) ReadOnly=(FloppyArchiveIsReadWrite==0);
  if (MSATempFile.NotEmpty()){
    f=fopen(MSATempFile,"r+b");
  }else if (ZipTempFile.NotEmpty()){
    f=fopen(ZipTempFile,LPSTR(ReadOnly ? "rb":"r+b"));
  }else{
    f=fopen(ImageFile,LPSTR(ReadOnly ? "rb":"r+b"));
  }
  if (f==NULL){
    DiskMan.EjectDisk(this==&FloppyDrive[0]?0:1);
    return 0;
  }
  return true;
}
//---------------------------------------------------------------------------
bool TFloppyImage::OpenFormatFile()
{
  if (f==NULL || ReadOnly || Format_f || STT_File) return 0;

  // The format file is just a max size ST file, any formatted tracks
  // go in here and then are merged with unformatted tracks when
  // the disk is removed from the drive
  FormatTempFile.SetLength(MAX_PATH);
  GetTempFileName(WriteDir,"FMT",0,FormatTempFile);

  // Create it
  Format_f=fopen(FormatTempFile,"wb");
  if (Format_f==NULL) return 0;
  fclose(Format_f);

  SetFileAttributes(FormatTempFile,FILE_ATTRIBUTE_HIDDEN);
  Format_f=fopen(FormatTempFile,"r+b");
  if (Format_f==NULL) return 0;

  char zeros[FLOPPY_MAX_BYTESPERSECTOR];
  ZeroMemory(zeros,sizeof(zeros));
  for (int Side=0;Side<2;Side++){
    for (int Track=0;Track<=FLOPPY_MAX_TRACK_NUM;Track++){
      for (int Sector=1;Sector<=FLOPPY_MAX_SECTOR_NUM;Sector++){
        fwrite(zeros,FLOPPY_MAX_BYTESPERSECTOR,1,Format_f);
      }
    }
  }
  fflush(Format_f);

  return true;
}
//---------------------------------------------------------------------------
bool TFloppyImage::ReopenFormatFile()
{
  if (Format_f==NULL || f==NULL || ReadOnly) return 0;

  fclose(Format_f);

  Format_f=fopen(FormatTempFile,"r+b");
  if (Format_f) return true;

  return 0;
}
//---------------------------------------------------------------------------
// Seek in the disk image to the start of the required sector
//---------------------------------------------------------------------------
bool TFloppyImage::SeekSector(int Side,int Track,int Sector,bool Format)
{
  if (Format_f==NULL) Format=0;

  if (Empty()){
    return true;
  }else if (Side<0 || Track<0 || Side>1){
    log(EasyStr("FDC: Seek Failed - Side ")+Side+" track "+Track+" sector "+Sector+" - negative values!");
    return true;
  }else if (Side>=int(Format ? 2:Sides)){
    log(EasyStr("FDC: Seek Failed - Can't seek to side ")+Side);
    return true;
  }else if (Track>=int(Format ? FLOPPY_MAX_TRACK_NUM+1:TracksPerSide)){
    log(EasyStr("FDC: Seek Failed - Can't seek to track ")+Track+" on side "+Side);
    return true;
  }
  if (STT_File){
    DWORD TrackStart=STT_TrackStart[Side][Track],Magic=0;
    WORD DataFlags;

    if (TrackStart==0) return true; // Track doesn't exist
    fseek(f,TrackStart,SEEK_SET);
    if (fread(&Magic,4,1,f)==0){
      if (ReinsertDisk()==0) return true;
      if ((TrackStart=STT_TrackStart[Side][Track])==0) return true;
      fseek(f,TrackStart,SEEK_SET);
      fread(&Magic,4,1,f);
    }
    if (Magic!=MAKECHARCONST('T','R','C','K')) return true;
    fread(&DataFlags,2,1,f);

    bool Failed=true;
    if (DataFlags & BIT_0){       //Sectors
      WORD Offset,Flags,NumSectors;
      fread(&Offset,2,1,f);
      fread(&Flags,2,1,f);
      fread(&NumSectors,2,1,f);

      BYTE TrackNum,SideNum,SectorNum,LenIdx,CRC1,CRC2;
      WORD SectorOffset,SectorLen;
      for (int n=0;n<NumSectors;n++){
        fread(&TrackNum,1,1,f);
        fread(&SideNum,1,1,f);
        fread(&SectorNum,1,1,f);
        fread(&LenIdx,1,1,f);
        fread(&CRC1,1,1,f);
        fread(&CRC2,1,1,f);
        fread(&SectorOffset,2,1,f);
        fread(&SectorLen,2,1,f);

        // I'm not sure but it is very possible changing sides during a disk operation
        // would cause it to immediately start reading the other side
        if (TrackNum==Track && SideNum==floppy_current_side() && SectorNum==Sector && SectorLen!=0){
          fseek(f,TrackStart+SectorOffset,SEEK_SET);
          BytesPerSector=SectorLen;
          Failed=0;
          break;
        }
      }
    }else if (DataFlags & BIT_1){ // Raw track data (with bad syncs)
    }

    LOG_ONLY( if (Failed) log(EasyStr("FDC: Seek Failed - Can't find sector ")+Sector+" in track "+Track+" on side "+Side); )

    return Failed;
  }else{
    if (Sector==0 || Sector>int(Format ? FLOPPY_MAX_SECTOR_NUM:SectorsPerTrack)){
      log(EasyStr("FDC: Seek Failed - Can't seek to sector ")+Sector+" of track "+Track+" on side "+Side);
      return true;
    }

    if (Format==0){
      int HeaderLen=int(DIM_File ? 32:0);
      fseek(f,HeaderLen+(GetLogicalSector(Side,Track,Sector)*BytesPerSector),SEEK_SET);
    }else{
      fseek(Format_f,GetLogicalSector(Side,Track,Sector,true)*FLOPPY_MAX_BYTESPERSECTOR,SEEK_SET);
    }
    return false;  //no error!
  }
}
//---------------------------------------------------------------------------
long TFloppyImage::GetLogicalSector(int Side,int Track,int Sector,bool FormatFile)
{
  if (Empty()) return 0;

  if (FormatFile==0 || Format_f==NULL){
    return ((Track*Sides*SectorsPerTrack)+(Side*SectorsPerTrack)+(Sector-1));
  }

  return (Track*2*FLOPPY_MAX_SECTOR_NUM)+(Side*FLOPPY_MAX_SECTOR_NUM)+(Sector-1);
}
//---------------------------------------------------------------------------
int TFloppyImage::GetIDFields(int Side,int Track,FDC_IDField IDList[30])
{
  if (Empty()) return 0;

  if (STT_File){
    DWORD TrackStart=STT_TrackStart[Side][Track],Magic=0;
    WORD DataFlags;

    if (TrackStart==0) return 0;
    fseek(f,TrackStart,SEEK_SET);
    if (fread(&Magic,4,1,f)==0){
      if (ReinsertDisk()==0) return 0;
      if ((TrackStart=STT_TrackStart[Side][Track])==0) return 0;
      fseek(f,TrackStart,SEEK_SET);
      fread(&Magic,4,1,f);
    }
    if (Magic!=MAKECHARCONST('T','R','C','K')) return 0;
    fread(&DataFlags,2,1,f);

    if (DataFlags & BIT_0){       //Sectors
      DWORD Dummy;
      WORD Offset,Flags,NumSectors;
      fread(&Offset,2,1,f);
      fread(&Flags,2,1,f);
      fread(&NumSectors,2,1,f);
      for (int n=0;n<NumSectors;n++){
        fread(&IDList[n].Track,1,1,f);
        fread(&IDList[n].Side,1,1,f);
        fread(&IDList[n].SectorNum,1,1,f);
        fread(&IDList[n].SectorLen,1,1,f);
        fread(&IDList[n].CRC1,1,1,f);
        fread(&IDList[n].CRC2,1,1,f);
        fread(&Dummy,4,1,f); // SectorOffset, SectorLen
      }
      return NumSectors;
    }else if (DataFlags & BIT_1){ //Raw track
    }
    return 0;
  }else{
    bool Format=0;
    if (Track<=FLOPPY_MAX_TRACK_NUM) Format=TrackIsFormatted[Side][Track];
    if (Side>=int(Format ? 2:Sides)){
      return 0;
    }else if (Track>=int(Format ? FLOPPY_MAX_TRACK_NUM+1:TracksPerSide)){
      return 0;
    }
    for (int n=0;n<int(Format ? FormatMostSectors:SectorsPerTrack);n++){
      IDList[n].Track=BYTE(Track);
      IDList[n].Side=BYTE(Side);
      IDList[n].SectorNum=BYTE(1+n);
      switch (BytesPerSector){
        case 128:  IDList[n].SectorLen=0; break;
        case 256:  IDList[n].SectorLen=1; break;
        case 1024: IDList[n].SectorLen=3; break;
        default:   IDList[n].SectorLen=2; break;
      }

      WORD CRC=0xffff;
      fdc_add_to_crc(CRC,0xa1);
      fdc_add_to_crc(CRC,0xa1);
      fdc_add_to_crc(CRC,0xa1);
      fdc_add_to_crc(CRC,0xfe);
      fdc_add_to_crc(CRC,IDList[n].Track);
      fdc_add_to_crc(CRC,IDList[n].Side);
      fdc_add_to_crc(CRC,IDList[n].SectorNum);
      fdc_add_to_crc(CRC,IDList[n].SectorLen);
      IDList[n].CRC1=HIBYTE(CRC);
      IDList[n].CRC2=LOBYTE(CRC);
    }
    return int(Format ? FormatMostSectors:SectorsPerTrack);
  }
}
//---------------------------------------------------------------------------
int TFloppyImage::GetRawTrackData(int Side,int Track)
{
  if (STT_File){
    DWORD TrackStart=STT_TrackStart[Side][Track],Magic;
    WORD DataFlags;

    if (TrackStart==0) return 0;
    fseek(f,TrackStart,SEEK_SET);
    if (fread(&Magic,4,1,f)==0){
      if (ReinsertDisk()==0) return 0;
      if ((TrackStart=STT_TrackStart[Side][Track])==0) return 0;
      fseek(f,TrackStart,SEEK_SET);
      fread(&Magic,4,1,f);
    }
    if (Magic!=MAKECHARCONST('T','R','C','K')) return 0;
    fread(&DataFlags,2,1,f);

    if (DataFlags & BIT_0){ // Skip this section if it exists
      WORD Offset;
      fread(&Offset,2,1,f);
      fseek(f,TrackStart+Offset,SEEK_SET);
    }
    if (DataFlags & BIT_1){ //Raw
      WORD Offset,Flags,TrackDataOffset,TrackDataLen;
      fread(&Offset,2,1,f);
      fread(&Flags,2,1,f);
      fread(&TrackDataOffset,2,1,f);
      fread(&TrackDataLen,2,1,f);

      fseek(f,TrackStart+TrackDataOffset,SEEK_SET);
      return TrackDataLen;
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
void TFloppyImage::RemoveDisk(bool LoseChanges)
{
  static bool Removing=0;

  if (Removing) return;
  Removing=true;

  if (f && ReadOnly==0 && LoseChanges==0 && WrittenTo && ZipTempFile.Empty()){
    short MSASecsPerTrack,MSAStartTrack=0,MSAEndTrack,MSASides;
    bool MSAResize=0;
    if (Format_f){
      if (FormatLargestSector>0){ // Formatted any track properly?
        // Try to merge the formatted data on Format_f with the old data on f
        int MaxTrack=0;
        for (int Side=0;Side<2;Side++){
          for (int Track=FLOPPY_MAX_TRACK_NUM;Track>0;Track--){
            if (TrackIsFormatted[Side][Track]){
              if (Track>MaxTrack) MaxTrack=Track;
              break;
            }
          }
        }
        if (MaxTrack>0){
          bool CanShrink=true,WipeOld=true;
          // Only shrink if all tracks were written
          for (int Track=MaxTrack;Track>=1;Track--){
            if (TrackIsFormatted[0][Track]==0){
              CanShrink=0;
              WipeOld=0;
              break;
            }
          }
          if (MaxTrack<70){ // Might want some old data left on the end
            CanShrink=0;
            WipeOld=0;
          }

          // Should we make it single sided?
          int NewSides=1;
          for (int Track=MaxTrack;Track>=0;Track--){
            if (TrackIsFormatted[1][Track]){
              NewSides=2;
            }else{
              // Don't wipe if haven't formatted over all sectors
              WipeOld=0;
            }
          }
          if (CanShrink==0) NewSides=max(NewSides,int(Sides));

          int NewTracksPerSide=int(CanShrink ? MaxTrack+1:max((int)TracksPerSide,MaxTrack+1));
          int NewSectorsPerTrack=int(CanShrink ? FormatMostSectors:max((int)SectorsPerTrack,FormatMostSectors));
          int NewBytesPerSector=int(WipeOld ? FormatLargestSector:max((int)BytesPerSector,FormatLargestSector));
          int NewBytesPerTrack=NewBytesPerSector*NewSectorsPerTrack;

          log("FDC: Formatted disk removed, copying data to disk image");
          log(EasyStr("  New format: Sides=")+NewSides+"  Tracks per side="+NewTracksPerSide+
                      "     SectorsPerTrack="+NewSectorsPerTrack);

          int HeaderLen=int(DIM_File ? 32:0);
          BYTE *NewDiskBuf=new BYTE[HeaderLen + NewBytesPerSector*NewSectorsPerTrack*NewTracksPerSide*NewSides];
          BYTE *lpNewDisk=NewDiskBuf;
          ZeroMemory(NewDiskBuf,HeaderLen + NewBytesPerSector*NewSectorsPerTrack*NewTracksPerSide*NewSides);
          if (HeaderLen){
            // Keep the header if there is one
            fseek(f,0,SEEK_SET);
            fread(lpNewDisk,HeaderLen,1,f);
            lpNewDisk+=HeaderLen;
          }

          for (int t=0;t<NewTracksPerSide;t++){
            for (int Side=0;Side<NewSides;Side++){
              int Countdown=3;
              if (TrackIsFormatted[Side][t]){
                // Read a track from the format file
                for (int s=1;s<=NewSectorsPerTrack;s++){
                  bool NextSector=true;
                  SeekSector(Side,t,s,true);
                  if (fread(lpNewDisk,1,NewBytesPerSector,Format_f)<size_t(NewBytesPerSector)){
                    if ((Countdown--)>0){
                      if (ReopenFormatFile()){
                        s--; // Try to redo a sector 3 times
                        NextSector=0;
                      }
                    }
                  }
                  if (NextSector) lpNewDisk+=NewBytesPerSector;
                }
              }else if (t<TracksPerSide && Side<Sides){
                // Copy information from the old disk onto the new disk
                for (int s=1;s<=min(int(SectorsPerTrack),NewSectorsPerTrack);s++){
                  bool NextSector=true;
                  SeekSector(Side,t,s,0);
                  if (fread(lpNewDisk,1,min(int(BytesPerSector),NewBytesPerSector),f)<size_t(min(int(BytesPerSector),NewBytesPerSector))){
                    if ((Countdown--)>0){
                      if (ReinsertDisk()){
                        s--; // Try to redo a sector 3 times
                        NextSector=0;
                      }
                    }
                  }
                  if (NextSector) lpNewDisk+=NewBytesPerSector;
                }
                // If getting bigger then skip
                for (int s=SectorsPerTrack;s<NewSectorsPerTrack;s++) lpNewDisk+=NewBytesPerSector;
              }else{
                lpNewDisk+=NewBytesPerTrack;
              }
            }
          }

          // Write it back to the original file (finally)
          int Countdown=3;
          LOOP{
            fclose(f);

            SectorsPerTrack=short(NewSectorsPerTrack);
            Sides=short(NewSides);
            TracksPerSide=short(NewTracksPerSide);
            BytesPerSector=short(NewBytesPerSector);

            if (MSATempFile.NotEmpty()){
              MSASecsPerTrack=short(NewSectorsPerTrack);
              MSAEndTrack=short(NewTracksPerSide+1);
              MSASides=short(NewSides-1);
              MSAResize=true;
              f=fopen(MSATempFile,"wb");
            }else{
              f=fopen(ImageFile,"wb");
            }
            if (f){
              fseek(f,0,SEEK_SET);
              int NewDiskSize=HeaderLen + NewBytesPerSector*NewSectorsPerTrack*NewTracksPerSide*NewSides;
              if (fwrite(NewDiskBuf,1,NewDiskSize,f)==size_t(NewDiskSize)){
                ConfigStoreFile CSF(ImageFile+".steembpb");
                CSF.SetStr("BPB","Sides",Str(Sides));
                CSF.SetStr("BPB","SectorsPerTrack",Str(SectorsPerTrack));
                CSF.SetStr("BPB","BytesPerSector",Str(BytesPerSector));
                CSF.SetStr("BPB","Sectors",Str(SectorsPerTrack*TracksPerSide*Sides));
                CSF.Close();
                break;
              }else{
                if ((--Countdown)<0){
                  log_write("Error writing to disk image after format! All data lost!");
                  break;
                }
              }
            }else{
              log_write("Error opening disk image after format! All data lost!");
              break;
            }
          }

          delete[] NewDiskBuf;
        }
      }
    }
    if (MSATempFile.NotEmpty() && f){
      // Write ST format MSATempFile to MSA format ImageFile
      WIN_ONLY( if (stem_mousemode!=STEM_MOUSEMODE_WINDOW) SetCursor(LoadCursor(NULL,IDC_WAIT)); )

      FILE *MSA=fopen(ImageFile,"r+b");
      if (MSA){
        BYTE Temp;
        fseek(MSA,2,SEEK_SET); //Seek past ID
        if (MSAResize==0){
          fread(&MSASecsPerTrack,2,1,MSA); SWAPBYTES(MSASecsPerTrack);
          fread(&MSASides,2,1,MSA);        SWAPBYTES(MSASides);
          fseek(MSA,2,SEEK_CUR);        // Skip StartTrack
          fread(&MSAEndTrack,2,1,MSA);     SWAPBYTES(MSAEndTrack);

          fseek(MSA,6,SEEK_SET);
          Temp=HIBYTE(MSAStartTrack);   fwrite(&Temp,1,1,MSA);
          Temp=LOBYTE(MSAStartTrack);   fwrite(&Temp,1,1,MSA);
        }else{
          // Write out MSA file info (in big endian)
          Temp=HIBYTE(MSASecsPerTrack); fwrite(&Temp,1,1,MSA);
          Temp=LOBYTE(MSASecsPerTrack); fwrite(&Temp,1,1,MSA);
          Temp=HIBYTE(MSASides);        fwrite(&Temp,1,1,MSA);
          Temp=LOBYTE(MSASides);        fwrite(&Temp,1,1,MSA);
          Temp=HIBYTE(MSAStartTrack);   fwrite(&Temp,1,1,MSA);
          Temp=LOBYTE(MSAStartTrack);   fwrite(&Temp,1,1,MSA);
          Temp=HIBYTE(MSAEndTrack);     fwrite(&Temp,1,1,MSA);
          Temp=LOBYTE(MSAEndTrack);     fwrite(&Temp,1,1,MSA);
        }

        fseek(MSA,10,SEEK_SET); // Past header

        fseek(f,0,SEEK_SET);

        int Len=WORD(MSASecsPerTrack*512);

        // Convert ST format f to MSA format MSA (uncompressed)
        int ReinsertAttempts=0;
        BYTE *MSADataBuf=new BYTE[(MSAEndTrack+1)*(MSASides+1)*MSASecsPerTrack*(512+2)];
        BYTE *pD=MSADataBuf;
        for (int t=0;t<=MSAEndTrack;t++){
          for (int s=0;s<=MSASides;s++){
            *(pD++)=HIBYTE(Len);
            *(pD++)=LOBYTE(Len);
            for (int sec=1;sec<=MSASecsPerTrack;sec++){
              SeekSector(s,t,sec,0);
              if (fread(pD,1,512,f)==512){ // Read sector from ST file
                pD+=512;
              }else if (ReinsertAttempts<5){
                ReinsertDisk();
                sec--;
                ReinsertAttempts++;
              }else{ // All else has failed, write an empty sector
                ZeroMemory(pD,512);
                pD+=512;
              }
            }
          }
        }
        fwrite(MSADataBuf,1,long(pD)-long(MSADataBuf),MSA);
        fclose(MSA);
        
        delete[] MSADataBuf;
      }
      WIN_ONLY( if (stem_mousemode!=STEM_MOUSEMODE_WINDOW) SetCursor(PCArrow); )
    }
  }
  if (f) fclose(f);
  f=NULL;
  if (Format_f) fclose(Format_f);
  Format_f=NULL;

  if (ZipTempFile.NotEmpty())    DeleteFile(ZipTempFile);
  if (MSATempFile.NotEmpty())    DeleteFile(MSATempFile);
  if (FormatTempFile.NotEmpty()) DeleteFile(FormatTempFile);

  ImageFile="";MSATempFile="";ZipTempFile="";FormatTempFile="";
  DiskName="";
  ReadOnly=true;
  BytesPerSector=0;Sides=0;SectorsPerTrack=0;TracksPerSide=0;
  ZeroMemory(TrackIsFormatted,sizeof(TrackIsFormatted));

  FormatMostSectors=0;FormatLargestSector=0;
  STT_File=0;
  DIM_File=0;

  Removing=0;
}
//---------------------------------------------------------------------------
#undef LOGSECTION

