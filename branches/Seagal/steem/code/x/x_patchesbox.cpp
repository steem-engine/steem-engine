//---------------------------------------------------------------------------
TPatchesBox::TPatchesBox()
{
	PatchList.owner=this;
	ApplyBut.owner=this;
	PatchDirBut.owner=this;

  Section="Patches";
}
//---------------------------------------------------------------------------
void TPatchesBox::Show()
{
  if (Handle) return;

  if (StandardShow(500,420,T("Patches"),
      ICO16_PATCHES,0,(LPWINDOWPROC)WinProc)) return;

  PatchLabel.create(XD,Handle,10,10,200,25,NULL,this,
                    BT_LABEL,T("Available Patches"),0,BkCol);

  PatchList.create(XD,Handle,10,35,200,340,ListviewNotifyHandler,this);

  DescLabel.create(XD,Handle,220,10,270,25,NULL,this,
                    BT_LABEL,T("Description"),0,BkCol);

	DescText.create(XD,Handle,220,35,270,80,WhiteCol);

  ApplyWhenLabel.create(XD,Handle,220,125,270,25,NULL,this,
                    BT_LABEL,T("Apply When"),0,BkCol);

	ApplyWhenText.create(XD,Handle,220,150,270,40,WhiteCol);

  VersionLabel.create(XD,Handle,220,200,270,25,NULL,this,
                    BT_LABEL,T("Version"),0,BkCol);

	VersionText.create(XD,Handle,220,225,270,40,WhiteCol);

  AuthorLabel.create(XD,Handle,220,275,270,25,NULL,this,
                    BT_LABEL,T("Patch Author"),0,BkCol);

	AuthorText.create(XD,Handle,220,300,270,40,WhiteCol);

  ApplyBut.create(XD,Handle,220,350,270,25,ButtonNotifyHandler,this,
                    BT_TEXT,T("Apply Now"),100,BkCol);


  PatchDirLabel.create(XD,Handle,10,385,0,25,NULL,this,
                    BT_LABEL,T("Patch folder"),0,BkCol);

  PatchDirBut.create(XD,Handle,490,385,0,25,ButtonNotifyHandler,this,
                    BT_TEXT,T("Choose"),200,BkCol);
  PatchDirBut.x-=PatchDirBut.w;
  XMoveWindow(XD,PatchDirBut.handle,PatchDirBut.x,PatchDirBut.y);

  PatchDirText.create(XD,Handle,15+PatchDirLabel.w,385,
  									PatchDirBut.x-10-(15+PatchDirLabel.w),25,NULL,this,
                    BT_STATIC | BT_BORDER_INDENT | BT_TEXT_PATH | BT_TEXT,
                    PatchDir,0,WhiteCol);


	RefreshPatchList();

  if (StemWin) PatBut.set_check(true);

  XMapWindow(XD,Handle);
  XFlush(XD);
}
//---------------------------------------------------------------------------
void TPatchesBox::ShowPatchFile()
{
  Str Text[4];
  GetPatchText(PatchDir+SLASH+SelPatch+".stp",Text);
  DescText.set_text(Text[0]);      DescText.draw();
  ApplyWhenText.set_text(Text[1]); ApplyWhenText.draw();
  VersionText.set_text(Text[2]);   VersionText.draw();
  AuthorText.set_text(Text[3]);    AuthorText.draw();
}
//---------------------------------------------------------------------------
void TPatchesBox::Hide()
{
  if (XD==NULL || Handle==0) return;

  StandardHide();

  if (StemWin) PatBut.set_check(0);
}
//---------------------------------------------------------------------------
int TPatchesBox::WinProc(TPatchesBox *This,Window Win,XEvent *Ev)
{
  switch (Ev->type){
    case ClientMessage:
      if (Ev->xclient.message_type==hxc::XA_WM_PROTOCOLS){
        if (Atom(Ev->xclient.data.l[0])==hxc::XA_WM_DELETE_WINDOW){
          This->Hide();
        }
      }
      break;
  }
  return PEEKED_MESSAGE;
}
//---------------------------------------------------------------------------
int TPatchesBox::ListviewNotifyHandler(hxc_listview* LV,int Mess,int I)
{
  TPatchesBox *This=(TPatchesBox*)(LV->owner);
  if (LV->sel>=0){
    if (NotSameStr_I(LV->sl[LV->sel].String,This->SelPatch)){
      This->SelPatch=LV->sl[LV->sel].String;
      This->ShowPatchFile();
    }
  }		
  return 0;
}
//---------------------------------------------------------------------------
int TPatchesBox::ButtonNotifyHandler(hxc_button* But,int Mess,int I[])
{
  TPatchesBox *This=(TPatchesBox*)(But->owner);
  if (Mess==BN_CLICKED){
  	if (But->id==100){
	    This->ApplyPatch();
	  }else if (But->id==200){
			fileselect.set_corner_icon(&Ico16,ICO16_FOLDER);
		  EasyStr Path=fileselect.choose(XD,This->PatchDir,"",T("Pick a Folder"),
		    FSM_CHOOSE_FOLDER | FSM_CONFIRMCREATE,folder_parse_routine,"");  	
		  if (Path.NotEmpty()){
        NO_SLASH(Path);
		  	This->PatchDir=Path;
		  	CreateDirectory(This->PatchDir,NULL);
		  	This->PatchDirText.set_text(This->PatchDir);
		  	
			  This->DescText.set_text("");This->DescText.draw();
			  This->ApplyWhenText.set_text("");This->ApplyWhenText.draw();
			  This->VersionText.set_text("");This->VersionText.draw();
			  This->AuthorText.set_text("");This->AuthorText.draw();
		  	This->RefreshPatchList();
		  }
	  }
  }
  return 0;
}

