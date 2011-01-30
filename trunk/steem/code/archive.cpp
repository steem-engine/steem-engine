zipclass::zipclass()
{
  type[0]=0;
  is_open=false;
  current_file_n=0;
  err=0;
}

bool zipclass::first(char *name)
{
  if (enable_zip==0) return ZIPPY_FAIL;

  if (is_open) close();

  type[0]=0;
  char *dot=strrchr(name,'.');
  if (dot){
    if (strlen(dot+1)<11) strcpy(type,dot+1);
  }
  if (type[0]==0) strcpy(type,"ZIP");
  strupr(type);

  if (strcmp(type,"ZIP")==0){
#ifdef UNIX
    uf=unzOpen(name);
    if (uf==NULL){
      last_error="Couldn't open ";
      last_error+=name;
      return ZIPPY_FAIL;
    }else{
      is_open=true;
      err=unzGetGlobalInfo(uf,&gi);
      current_file_n=0;
      current_file_offset=0;
      err=unzGetCurrentFileInfo(uf,&fi,filename_inzip,
            sizeof(filename_inzip),NULL,0,NULL,0);
      if (err){
        close();
        return ZIPPY_FAIL;
      }
    }
#endif
#ifdef WIN32
    ZeroMemory(&PackInfo,sizeof(PackInfo));

    if (GetFirstInZip(name,&PackInfo)!=UNZIP_Ok){
      return ZIPPY_FAIL;
    }
    is_open=true;
    current_file_n=0;
    current_file_offset=PackInfo.HeaderOffset;
    attrib=PackInfo.Attr;
    crc=PackInfo.Crc;
#endif

    return ZIPPY_SUCCEED;

#ifdef RAR_SUPPORT
  }else if (strcmp(type,"RAR")==0){
    if (urarlib_list(name,(ArchiveList_struct*)&rar_list)<=0) return ZIPPY_FAIL;

    // There are some files in this RAR
    while (rar_list){
      if ((rar_list->item.FileAttr & 0x10)==0) break; // Not directory
      rar_list=rar_list->next;
    }
    if (rar_list==NULL) return ZIPPY_FAIL;

    is_open=true;
    rar_current=rar_list;
    current_file_n=0;
    current_file_offset=0;
    attrib=WORD(rar_current->item.FileAttr);

    return ZIPPY_SUCCEED;
#endif
  }
  return ZIPPY_FAIL;
}

bool zipclass::next()
{
  if (enable_zip==0) return ZIPPY_FAIL;

  if (strcmp(type,"ZIP")==0){
#ifdef UNIX
    if (is_open==0) return ZIPPY_FAIL;
  	if (current_file_n>=(int)(gi.number_entry-1)) return ZIPPY_FAIL;
    err=unzGoToNextFile(uf);
    if (err) return ZIPPY_FAIL;
    err=unzGetCurrentFileInfo(uf,&fi,filename_inzip,
          sizeof(filename_inzip),NULL,0,NULL,0);
    if (err) return ZIPPY_FAIL;
    current_file_n++;
    current_file_offset=current_file_n;
#endif
#ifdef WIN32
    err=GetNextInZip(&PackInfo);
    if (err!=UNZIP_Ok) return ZIPPY_FAIL;
    attrib=PackInfo.Attr;
    crc=PackInfo.Crc;
    current_file_n++;
    current_file_offset=PackInfo.HeaderOffset;
#endif

    return ZIPPY_SUCCEED;

#ifdef RAR_SUPPORT
  }else if (strcmp(type,"RAR")==0){
    if (is_open==0 || rar_current==NULL) return ZIPPY_FAIL;

    do{
      rar_current=rar_current->next;
      current_file_n++;
      if (rar_current==NULL) return ZIPPY_FAIL;
    }while (rar_current->item.FileAttr & 0x10); // Skip if directory

    current_file_offset=current_file_n;
    attrib=WORD(rar_current->item.FileAttr);

    return ZIPPY_SUCCEED;
#endif
  }
  return ZIPPY_FAIL;
}

char* zipclass::filename_in_zip()
{
  if (enable_zip==0) return "";

  if (strcmp(type,"ZIP")==0){
#ifdef UNIX
    return filename_inzip;
#endif
#ifdef WIN32
    return PackInfo.FileName;
#endif
#ifdef RAR_SUPPORT
  }else if (strcmp(type,"RAR")==0){
    if (rar_current) return rar_current->item.Name;
#endif
  }
  return "";
}

bool zipclass::close()
{
  if (enable_zip==0) return ZIPPY_FAIL;

  if (is_open){
    if (strcmp(type,"ZIP")==0){
      UNIX_ONLY( unzClose(uf); )
      WIN_ONLY( CloseZipFile(&PackInfo); )

      is_open=false;
      return ZIPPY_SUCCEED;

#ifdef RAR_SUPPORT
    }else if (strcmp(type,"RAR")==0){
      is_open=false;
      return ZIPPY_SUCCEED;
#endif
    }
  }
  return ZIPPY_FAIL;
}

void zipclass::list_contents(char *name,EasyStringList *eslp,bool st_disks_only)
{
  if (enable_zip==0) return;

  eslp->DeleteAll();

  if (first(name)==0){
    do{
      EasyStr a=filename_in_zip();
      bool addflag=true;
      if (st_disks_only){
        if (FileIsDisk(a)!=DISK_UNCOMPRESSED) addflag=false;
      }
      if (addflag) eslp->Add(a,current_file_offset,attrib,crc);
    }while (next()==0);
  }
  close();
}

bool zipclass::extract_file(char *fn,int offset,char *dest_dir,bool hide,DWORD attrib)
{
  if (enable_zip==0) return ZIPPY_FAIL;

  if (strcmp(type,"ZIP")==0){
    if (is_open) close();

#ifdef UNIX
    uf=unzOpen(fn);
    if (uf==NULL){
      last_error=Str("Couldn't open ")+fn;
      return ZIPPY_FAIL;
    }
    is_open=true;
    err=unzGetGlobalInfo(uf,&gi);
    if (err){
      close();
      return ZIPPY_FAIL;
    }

    unz_global_info gi;
    int err=unzGetGlobalInfo(uf,&gi);
    if (err!=UNZ_OK){
      close();
      last_error="couldn't get global info";
      return ZIPPY_FAIL;
    }
    if (offset>=(int)gi.number_entry){
      close();
      last_error="too few files in zip";
      return ZIPPY_FAIL;
    }
  //  unzGoToFirstFile(uf);
    for (int i=0;i<offset;i++) unzGoToNextFile(uf);

#define UNZIP_BUF_SIZE 8192
    BYTE buf[UNZIP_BUF_SIZE];
    err=unzGetCurrentFileInfo(uf,&fi,filename_inzip,
          sizeof(filename_inzip),NULL,0,NULL,0);
    if (err) return ZIPPY_FAIL;

    EasyStr dest_file=dest_dir;
    if (dest_dir[0]==0 || dest_dir[strlen(dest_dir)-1]=='/'){
      char *t=strrchr(filename_inzip,'/');
      if (t){
        t++;
      }else{
        t=strrchr(filename_inzip,'\\');
        if(t){
          t++;
        }else{
          t=filename_inzip;
        }
      }
      dest_file+=t;
    }

    err=unzOpenCurrentFile(uf);
    if (err){
      close();
      last_error="error opening file in ZIP";
      printf("%s\n",last_error.Text);
      return ZIPPY_FAIL;
    }

    FILE *fout=fopen(dest_file,"wb");
    if (fout==NULL){
      close();unzCloseCurrentFile(uf);
      last_error="error opening file ";last_error+=dest_file.Text;
      printf("%s\n",last_error.Text);
      return ZIPPY_FAIL;
    }

    do{
      err=unzReadCurrentFile(uf,buf,UNZIP_BUF_SIZE);
      if (err<0){
        last_error=EasyStr("error #")+err+" with zipfile in unzReadCurrentFile";
        printf("%s\n",last_error.Text);
        break;
      }else if(err>0){
        fwrite(buf,err,1,fout);
      }
    }while (err>0);
    fclose(fout);

    err=unzCloseCurrentFile(uf);
    close();
    if (err) return ZIPPY_FAIL;

#elif defined(WIN32)
    EasyStr dest_file=dest_dir;
    err=UnzipFile(fn,dest_file.Text,(WORD)(hide ? 2:attrib),offset,NULL,0);
    if (err!=UNZIP_Ok) return ZIPPY_FAIL;
#endif

    return ZIPPY_SUCCEED;

#ifdef RAR_SUPPORT
  }else if (strcmp(type,"RAR")==0){
    if (is_open) close();

    if (first(fn)==0){
      while (offset > 0){
        if (next()){ // Failed
          close();
          return ZIPPY_FAIL;
        }
        offset--;
      }
    }

    char *data_ptr;
    DWORD data_size;
    if (urarlib_get(&data_ptr,&data_size,rar_current->item.Name,fn,"")==0) return ZIPPY_FAIL;
    close();

    EasyStr dest_file=dest_dir;
    FILE *f=fopen(dest_file,"wb");
    if (f==NULL) return ZIPPY_FAIL;
    fwrite(data_ptr,1,data_size,f);
    fclose(f);

#ifdef WIN32
    if (hide){
      SetFileAttributes(dest_file,FILE_ATTRIBUTE_HIDDEN);
    }else{
      SetFileAttributes(dest_file,attrib);
    }
#endif

    return ZIPPY_SUCCEED;

#endif
  }
  return ZIPPY_FAIL;
}

