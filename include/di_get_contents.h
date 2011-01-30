#define GC_ONAMBIGUITY_FAIL 0
#define GC_ONAMBIGUITY_GUESS 1
#define GC_ONAMBIGUITY_ASK 2

#define GC_TOOSMALL -1
#define GC_NOLISTS -2
#define GC_CANTFINDCONTENTS 0

DWORD GetContentsFromDiskImage(char *,char *,int,int);
void GetContents_SearchDatabase(char *,char *,int);

