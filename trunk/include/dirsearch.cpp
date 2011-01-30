/*---------------------------------------------------------------------------
FILE: dirsearch.cpp
MODULE: helper
DESCRIPTION: Cross-platform class to retrieve directory contents. 
---------------------------------------------------------------------------*/

#ifndef DIRSEARCH_CPP
#define DIRSEARCH_CPP

#include "dirsearch.h"

#ifdef WIN32

DirSearch::DirSearch() : Attrib(FindDat.dwFileAttributes),CreationTime(FindDat.ftCreationTime),
              LastAccessTime(FindDat.ftLastAccessTime),LastWriteTime(FindDat.ftLastWriteTime),
              SizeHigh(FindDat.nFileSizeHigh),SizeLow(FindDat.nFileSizeLow)
    { hFind=NULL; FoundFile=0; }
DirSearch::DirSearch(char *Dir) : Attrib(FindDat.dwFileAttributes),CreationTime(FindDat.ftCreationTime),
              LastAccessTime(FindDat.ftLastAccessTime),LastWriteTime(FindDat.ftLastWriteTime),
              SizeHigh(FindDat.nFileSizeHigh),SizeLow(FindDat.nFileSizeLow)
    { hFind=NULL;Find(Dir); }
DirSearch::~DirSearch()         { Close(); }

bool DirSearch::Find(char *Dir)
{
  Close();
  hFind=FindFirstFile(Dir,&FindDat);
  if (hFind!=INVALID_HANDLE_VALUE){
    SetFoundFile();
    return true;
  }
  hFind=NULL;
  FoundFile=0;
  return 0;
}

bool DirSearch::Next()
{
  if (hFind==NULL) return 0;

  if (FindNextFile(hFind,&FindDat)){
    SetFoundFile();
    return true;
  }
  FoundFile=0;
  return 0;
}

void DirSearch::SetFoundFile()
{
  FoundFile=true;

  Name=FindDat.cFileName;
  // If Name is 8.3 we can use that as shortname, otherwise use AlternateFileName;
  // This is done to preserve special characters that the AlternateName destroys
  if (FindDat.cAlternateFileName[0]){
    bool UseAlt=0;
    EasyStr Temp=Name;
    char *dot=strrchr(Temp,'.');
    if (dot){
      if (strlen(dot)>4) UseAlt=true;
      *dot=0;
    }
    if (strlen(Temp)>8) UseAlt=true;
    if (strchr(Name,' ')!=NULL) UseAlt=true; // Don't allow spaces in 8.3 names, confuses DOS/TOS!

    ShortName=(UseAlt ? FindDat.cAlternateFileName:Name);
  }else{
    ShortName=Name;
  }
}

void DirSearch::Close()
{
  if (hFind==NULL) return;
  FindClose(hFind);
  hFind=NULL;
}

//---------------------------------------------------------------------------
#else
//---------------------------------------------------------------------------

DirSearch::DirSearch()          { dp=NULL;FoundFile=0;st_only=false;}
DirSearch::DirSearch(char *Dir) { dp=NULL;st_only=false;Find(Dir);}
DirSearch::~DirSearch()         { Close(); }

bool DirSearch::Find(char *Dir)
{
  Close();

  fullpath=Dir;
  char *slash=strrchr(fullpath,'/');
  if (slash){
    mask=slash+1;
    *(slash+1)=0;
//	  	*slash=0;
  }

  dp=opendir(fullpath);
  if (dp){
    Next();
  }else{
    FoundFile=0;
  }
  return FoundFile;
}

bool DirSearch::Next()
{
  if (dp==NULL) return 0;

  FoundFile=0;
  	
  EasyStr FullFilePath;
  for (;;){
    ep=readdir(dp);
    if (ep==NULL) break;

    bool matches=true;
    if (!filename_matches_mask(ep->d_name,mask)){
      matches=false;
    }
    if(st_only){
      EasyStr cap_sn=ep->d_name;
      strupr(cap_sn.Text);
      //check 8.3
      char*c_dot=strchr(cap_sn,'.');
      if(c_dot){ //there's a dot in the filename
        if(int(c_dot-cap_sn.Text)>8){matches=false;}
        if(strlen(c_dot)>4){matches=false;}
      }else{
        if(strlen(cap_sn.Text)>8){matches=false;}
      }      		
    }
    if (matches){
      FullFilePath=fullpath+ep->d_name;
      if (stat(FullFilePath,&s)==0){
        SetFoundFile(FullFilePath);
        break;
      }
    }
  }
  return FoundFile;
}

void DirSearch::SetFoundFile(char *FullFilePath)
{
  if (IsSameStr_I(ep->d_name,".") || IsSameStr_I(ep->d_name,"..")){
    Attrib=FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM;
    SizeHigh=0;SizeLow=0;
  }else{
    Attrib=0;
    if (S_ISDIR(s.st_mode)) Attrib|=FILE_ATTRIBUTE_DIRECTORY;

    uid_t cur_uid=geteuid();
    gid_t cur_gid=getegid();
    if (cur_uid==s.st_uid){
      if ((s.st_mode & S_IWUSR)==0) Attrib|=FILE_ATTRIBUTE_READONLY;
    }else if (cur_gid==s.st_gid){
      if ((s.st_mode & S_IWGRP)==0) Attrib|=FILE_ATTRIBUTE_READONLY;
    }else{
      if ((s.st_mode & S_IWOTH)==0) Attrib|=FILE_ATTRIBUTE_READONLY;
    }

    SizeHigh=0;SizeLow=s.st_size;

    struct tm *pStm;
    pStm=localtime(&s.st_mtime);
    LastWriteTime=TMToDOSDateTime(pStm);
    pStm=localtime(&s.st_atime);
    LastAccessTime=TMToDOSDateTime(pStm);
    pStm=localtime(&s.st_ctime);
    CreationTime=TMToDOSDateTime(pStm);
  }
	 	
  Name=ep->d_name;
  ShortName=ep->d_name;
  char *ShortNameExt=strrchr(ShortName,'.');
  if (ShortNameExt){
    if (strlen(ShortNameExt)>4) ShortNameExt[4]=0;
    ShortNameExt[0]=0;
    if (strlen(ShortName)>8){
      ShortName[8]=0;
      ShortName+=EasyStr(".")+(ShortNameExt+1);
    }
    ShortNameExt[0]='.';
  }else{
    if (strlen(ShortName)>8) ShortName[8]=0;
  }

  FoundFile=true;
}

void DirSearch::Close()
{
  if (dp) closedir(dp);
  dp=NULL;
}
#endif

#endif

