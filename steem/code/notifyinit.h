void CreateNotifyInitWin();
void DestroyNotifyInitWin();

void SetNotifyInitText(char*);

#ifdef WIN32
LRESULT __stdcall NotifyInitWndProc(HWND,UINT,WPARAM,LPARAM);
HWND NotifyWin=NULL;
ONEGAME_ONLY( HWND NotifyWinParent; )
#endif
