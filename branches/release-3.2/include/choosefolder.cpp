#ifndef CHOOSEFOLDER_H
#define CHOOSEFOLDER_H
#include <shlobj.h>

#ifndef NO_SLASH
#define CHOOSEFOLDER_UNDEF_NO_SLASH
#define NO_SLASH(c) if(c[0]) if (c[strlen(c)-1]=='\\' || c[strlen(c)-1]=='/') c[strlen(c)-1]=0;
#endif
//---------------------------------------------------------------------------
int __stdcall ChooseFolder_BrowseCallbackProc(HWND Win,UINT Mess,LPARAM,LPARAM lParUser)
{
  switch (Mess){
    case BFFM_INITIALIZED:
    {
      RECT box;
			POINT pt;
			HWND TreeView;
			char Path[MAX_PATH+1];
			int Len;

			// Centre dialog
      GetWindowRect(Win,&box);
      box.right-=box.left;
      box.bottom-=box.top;
      SetWindowPos(Win,0,(GetSystemMetrics(SM_CXSCREEN)/2)-(box.right/2),
                    (GetSystemMetrics(SM_CYSCREEN)/2)-(box.bottom/2),
                    0,0,SWP_NOSIZE | SWP_NOZORDER);

			// Select passed folder
			strcpy(Path,(char*)lParUser);
      NO_SLASH(Path);
			Len=strlen(Path);
			if (Path[Len-1]==':'){
				Path[Len++]='\\';
				Path[Len]=0;
			}
      SendMessage(Win,BFFM_SETSELECTION,TRUE,(LPARAM)Path);

			// Change Tree to make it work better
      pt.x=box.right/2;
			pt.y=box.bottom/2;
      TreeView=ChildWindowFromPoint(Win,pt);
      SetWindowLong(TreeView,GWL_STYLE,GetWindowLong(TreeView,GWL_STYLE) | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP);

      break;
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
EasyStr ChooseFolder(HWND Win,char *Title,char *FolToSel)
{
	ITEMIDLIST *RetIDL,*RootIDL=NULL;
	char FolName[MAX_PATH+1];
 	IMalloc *Mal;
  BROWSEINFO bi;

	SHGetMalloc(&Mal);

 	bi.hwndOwner=Win;
 	bi.pidlRoot=RootIDL;              //Directory to start from (NULL=desktop)
	bi.pszDisplayName=FolName;        //Doesn't return full path
  bi.lpszTitle=Title;
	bi.ulFlags=BIF_RETURNONLYFSDIRS;  //No false folders (like DUN)
	bi.lpfn=ChooseFolder_BrowseCallbackProc; // Function to handle various notification
  bi.lParam=(LPARAM)FolToSel;         // What to call that func with
  bi.iImage=0;

  RetIDL=SHBrowseForFolder(&bi);
  if (RetIDL==NULL) return "";

  EasyStr SelFol;
  SelFol.SetLength(MAX_PATH);
  SHGetPathFromIDList(RetIDL,SelFol);
	NO_SLASH(SelFol);

  Mal->Free(RetIDL);

	return SelFol;
}

#ifdef CHOOSEFOLDER_UNDEF_NO_SLASH
#undef CHOOSEFOLDER_UNDEF_NO_SLASH
#undef NO_SLASH
#endif

#endif
