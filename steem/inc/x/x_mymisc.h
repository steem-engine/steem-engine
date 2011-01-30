extern void unix_non_resizable_window(Display*Disp,Window Handle);
extern bool IsFocussed(Display*disp,Window win);
extern void SetWindowHints(Display *Disp,Window Win,bool Input,int State,
                    Pixmap IconPixmap,Pixmap IconMaskPixmap,XID Group,
                    bool Urgent);
extern void SetWindowNormalSize(Display *Disp,Window Win,int min_w,int min_h,
													int max_w=0,int max_h=0,int w_inc=1,int h_inc=1,
													int grav=NorthWestGravity);
extern void SetWindowGravity(Display *Disp,Window Win,int Grav);
extern char *RemoveIllegalFromName(char *Name,bool RemoveWild=true,char ReplaceChar='-');
extern char *RemoveIllegalFromPath(char *Path,bool DriveIncluded,bool RemoveWild=true,char ReplaceChar='-',bool STPath=0);
extern char *GetFileNameFromPath(char *fil);
extern bool MoveFile(char *From,char *To);
extern DWORD timeGetTime();
#define GetTickCount() timeGetTime()
extern void Sleep(DWORD milli);

#define FILE_ATTRIBUTE_ARCHIVE    0
#define FILE_ATTRIBUTE_HIDDEN     0
#define FILE_ATTRIBUTE_NORMAL     0
#define FILE_ATTRIBUTE_OFFLINE    0 
#define FILE_ATTRIBUTE_READONLY   1
#define FILE_ATTRIBUTE_SYSTEM     0 
#define FILE_ATTRIBUTE_TEMPORARY  0 
#define FILE_ATTRIBUTE_COMPRESSED 0 
#define FILE_ATTRIBUTE_DIRECTORY  2 

/*
`S_IRUSR' Read permission bit for the owner of the file.
`S_IWUSR' Write permission bit for the owner of the file.
`S_IXUSR' Execute (for ordinary files) or search (for directories)
          permission bit for the owner of the file.
`S_IRWXU' (S_IRUSR | S_IWUSR | S_IXUSR)

`S_IRGRP' Read permission bit for the group owner of the file.
`S_IWGRP' Write permission bit for the group owner of the file.
`S_IXGRP' Execute or search permission bit for the group owner of the file.
`S_IRWXG' (S_IRGRP | S_IWGRP | S_IXGRP)

`S_IROTH' Read permission bit for other users.
`S_IWOTH' Write permission bit for other users.
`S_IXOTH' Execute or search permission bit for other users.
`S_IRWXO' (S_IROTH | S_IWOTH | S_IXOTH)

`S_ISUID' Set-user-ID on execute bit
`S_ISGID' Set-group-ID on execute bit

`S_ISVTX' "sticky" bit
*/

extern bool SetFileAttributes(char *File,DWORD Attrib);
extern DWORD GetFileAttributes(char *File);
extern bool CreateDirectory(char *Path,void*);
extern bool RemoveDirectory(char *Path);
extern bool matches_mask(char*_t,char*_mask);
extern bool filename_matches_mask(char*_t,char*_mask);
extern EasyStr find_file_i(EasyStr path,EasyStr f); //search for file f in path
//---------------------------------------------------------------------------
extern bool SetProp(Display *Disp,Window Win,XContext Prop,DWORD Val);
extern DWORD GetProp(Display *Disp,Window Win,XContext Prop);
extern DWORD RemoveProp(Display *Disp,Window Win,XContext Prop);
extern DWORD GetColourValue(Display *Disp,WORD R,WORD G,WORD B,DWORD Default);
//---------------------------------------------------------------------------
extern void ShowHideWindow(Display *,Window,bool);
extern bool copy_file_byte_by_byte(char*,char*);

