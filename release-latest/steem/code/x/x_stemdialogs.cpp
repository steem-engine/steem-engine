//---------------------------------------------------------------------------
TStemDialog::TStemDialog()
{
  Handle=0;
  IconPixmap=0;
  IconMaskPixmap=0;

  if (nStemDialogs<MAX_DIALOGS) DialogList[nStemDialogs++]=this;
}
//---------------------------------------------------------------------------
void TStemDialog::StandardHide()
{
  if (Handle){
    hxc::destroy_children_of(Handle);

    RemoveProp(XD,Handle,cWinProc);
    RemoveProp(XD,Handle,cWinThis);
    XDestroyWindow(XD,Handle);

    if(IconPixmap)XFreePixmap(XD,IconPixmap);
    if(IconMaskPixmap)XFreePixmap(XD,IconMaskPixmap);

    Handle=0;
  }
}


bool TStemDialog::StandardShow(int w,int h,char* name,
      int icon_index,long input_mask,LPWINDOWPROC WinProc,bool resizable)
{
  XSetWindowAttributes swa;
  swa.backing_store=NotUseful;
  swa.background_pixel=BkCol;
	swa.colormap=colormap;
  Handle=XCreateWindow(XD,XDefaultRootWindow(XD),
                           (GetScreenWidth()-w)/2,(GetScreenHeight()-h)/2,
                           w,h,0,CopyFromParent,InputOutput,CopyFromParent,
                           CWBackingStore | CWBackPixel |
                           int(colormap ? CWColormap:0),&swa);
  if (Handle==0) return true; //fail

  Atom Prots[1]={hxc::XA_WM_DELETE_WINDOW};
  XSetWMProtocols(XD,Handle,Prots,1);

  if(!resizable)unix_non_resizable_window(XD,Handle);

  IconPixmap=Ico16.CreateIconPixmap(icon_index,DispGC);
  IconMaskPixmap=Ico16.CreateMaskBitmap(icon_index);
  SetWindowHints(XD,Handle,True,NormalState,IconPixmap,IconMaskPixmap,SteemWindowGroup,0);

  XStoreName(XD,Handle,name);

  XSelectInput(XD,Handle,input_mask);

  SetProp(XD,Handle,cWinProc,(DWORD)WinProc);
  SetProp(XD,Handle,cWinThis,(DWORD)this);

  return false; //no error
}
