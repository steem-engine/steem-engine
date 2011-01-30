//---------------------------------------------------------------------------
int SteemIntro()
{
	EasyStr caption=T("Welcome to Steem"),text;

  text=T("Thank you for running the Steem Engine. We hope you will get many hours of enjoyment from this program. Before you can start there's just a few things to set up.");
#ifdef WIN32
  int Ret;
  MSGBOXPARAMS mbp;
  mbp.cbSize=sizeof(MSGBOXPARAMS);
  mbp.hwndOwner=NULL;
  mbp.hInstance=Inst;
  mbp.dwContextHelpId=0;
  mbp.lpfnMsgBoxCallback=NULL;
  mbp.dwLanguageId=GetUserDefaultLangID();
  mbp.lpszIcon=RCNUM(RC_ICO_APP);

  text+=" ";
  text+=T("Do you want to put a shortcut to Steem in your Start Menu?");
  mbp.lpszCaption=caption;
  mbp.lpszText=text.Text;
  mbp.dwStyle=MB_USERICON | MB_YESNO;
  if (MessageBoxIndirect(&mbp)==IDYES){
    ITEMIDLIST *idl;
    if (SHGetSpecialFolderLocation(NULL,CSIDL_PROGRAMS,&idl)==NOERROR){
      IMalloc *Mal;SHGetMalloc(&Mal);
      EasyStr Path;Path.SetLength(MAX_PATH);
      SHGetPathFromIDList(idl,Path);
      Mal->Free(idl);

      EasyStr ThisExeName=GetEXEFileName();
      CreateDirectory(Path+"\\Steem Engine",NULL);
      CreateLink(Path+"\\Steem Engine\\Steem Engine.lnk",ThisExeName,"The STE Emulating Engine");
      CreateLink(Path+"\\Steem Engine\\Readme.lnk",RunDir+"\\readme.txt","Steem Engine Info");
    }
  }
#elif defined(UNIX)
  alert.set_icons(&Ico64,ICO64_STEEM,&Ico16,ICO16_STEEM);
  if (1==alert.ask(XD,text,caption,T("Continue")+"|"+T("Abort"),0,1)){
		return 1;
  }
#endif

  caption=T("TOS Image");
  text=T("The most important file Steem needs to run is an image of the ST operating system TOS. If you haven't got one you cannot run Steem. They are easily downloadable from the internet, probably from the same place that you downloaded Steem. Alternatively you can get ST programs that will save out the TOS from an ST. After clicking OK find a TOS image file and click open. This setting, and all the other settings you are about to set, can be easily changed at any time when running Steem.");
#ifdef WIN32
	mbp.lpszText=text;
	mbp.lpszCaption=caption;
  mbp.dwStyle=MB_USERICON | MB_OK;
  MessageBoxIndirect(&mbp);
#elif defined(UNIX)
	alert.ask(XD,text,caption,T("Okay"),0,0);
#endif
  LOOP{
#ifdef WIN32
    ROMFile=FileSelect(NULL,T("Select TOS Image"),RunDir,FSTypes(3,NULL),1,true,"img");
#elif defined(UNIX)
		fileselect.set_corner_icon(&Ico16,ICO16_CHIP);
    ROMFile=fileselect.choose(XD,RunDir,"",T("Select TOS Image"),FSM_LOAD | FSM_LOADMUSTEXIST,
    												romfile_parse_routine,".img");
#endif
    if (ROMFile.IsEmpty()) return 1;

    if (load_TOS(ROMFile)==0) break;

    MessageBox((WINDOWTYPE)0,ROMFile+" "+T("is not a valid TOS"),T("Error Loading OS"),
                MB_ICONEXCLAMATION | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
  }


	caption=T("Disk Images");
  text=T("The next few settings regard the way Steem emulates disks. Steem, and all other ST emulators, use files with the extension ST, STT, DIM or MSA on a PC drive for its floppy disks. This is the format most things you download for the ST will be in. If you have some of these files already then you can tell Steem which folder they are in. This will become your home folder and makes it easy to switch disks in and out of drives. If you don't have any then select a suitable folder on your PC hard disk and Steem will create a blank disk for you there.")+"\n\n"+
         T("Click cancel if you'd rather set up the home folder later.");

	bool proceed;
#ifdef WIN32
  mbp.lpszCaption=caption;
  mbp.lpszText=text;
  mbp.dwStyle=MB_USERICON | MB_OKCANCEL;
  Ret=MessageBoxIndirect(&mbp);
  proceed=(Ret==IDOK);
#elif defined(UNIX)
	proceed=!alert.ask(XD,text,caption,T("Okay")+"|"+T("Cancel"),0,1);
#endif

  if (proceed){
  	EasyStr Path;
#ifdef WIN32
    Path=ChooseFolder(NULL,T("Pick a Folder"),RunDir);
    if (Path.Empty()) Path=RunDir;
#elif defined(UNIX)
		fileselect.set_corner_icon(&Ico16,ICO16_FOLDER);
	  Path=fileselect.choose(XD,RunDir,"",T("Pick a Folder"),
	    FSM_CHOOSE_FOLDER | FSM_CONFIRMCREATE,folder_parse_routine,"");
#endif
		if (Path.NotEmpty()){
      DiskMan.HomeFol=Path;
      NO_SLASH(DiskMan.HomeFol);
      DiskMan.DisksFol=DiskMan.HomeFol;

    	bool Found=0;
      {
	      DirSearch ds;
		    if (ds.Find(DiskMan.HomeFol+SLASH+"*.*")){
		    	do{
            if (FileIsDisk(ds.Name)){
							Found=true;
							break;
		    		}
		    	}while (ds.Next());
		    }
	    }

      if (Found==0){
        if (DiskMan.CreateDiskImage(DiskMan.HomeFol+SLASH+T("Blank Disk")+".st",
                                    80*9*2,9,2)){
          FloppyDrive[0].SetDisk(DiskMan.HomeFol+SLASH+T("Blank Disk")+".st");
          FloppyDrive[0].DiskName=T("Blank Disk");
          DiskMan.InsertHistoryAdd(0,FloppyDrive[0].DiskName,FloppyDrive[0].GetDisk(),"");
        }
      }
    }
  }

  caption=T("Hard Drives");
  text=T("Steem can also emulate hard drives on the ST. This isn't as reliable as disk images, but it works for most things. You can have up to 10 hard drives, each one is mapped to a folder on your PC hard drive.")+"\n\n"+
         T("Would you like to select a folder to be ST hard drive C now?");
 	proceed=false;
#ifdef WIN32
  mbp.lpszText=text;
  mbp.lpszCaption=caption;
  mbp.dwStyle=MB_USERICON | MB_YESNO;
  Ret=MessageBoxIndirect(&mbp);
  if (Ret==IDYES){
  	proceed=true;
  }
#elif defined(UNIX)
	proceed=!alert.ask(XD,text,caption,T("Yes")+"|"+T("No"),0,1);
#endif
	if (proceed){
    int Let=0;
    EasyStr Mess[9];
    Mess[0]=T("Would you like to select a folder to be ST hard drive D now?");
    Mess[1]=T("Would you like to select a folder to be ST hard drive E now?");
    Mess[2]=T("Would you like to select a folder to be ST hard drive F now?");
    Mess[3]=T("Would you like to select a folder to be ST hard drive G now?");
    Mess[4]=T("Would you like to select a folder to be ST hard drive H now?");
    Mess[5]=T("Would you like to select a folder to be ST hard drive I now?");
    Mess[6]=T("Would you like to select a folder to be ST hard drive J now?");
    Mess[7]=T("Would you like to select a folder to be ST hard drive K now?");
    Mess[8]=T("Would you like to select a folder to be ST hard drive L now?");
    EasyStr Path=RunDir;
#ifdef WIN32
    do{
      Path=ChooseFolder(NULL,T("Pick a Folder"),Path);
      if (Path.Empty()) break;
		  NO_SLASH(Path);

      if (Let>=9) break;
      HardDiskMan.NewDrive(Path);
      mbp.lpszText=Mess[Let++];
      mbp.dwStyle=MB_USERICON | MB_YESNO;
      Ret=MessageBoxIndirect(&mbp);
    }while (Ret==IDYES);
#elif defined(UNIX)
		fileselect.set_corner_icon(&Ico16,ICO16_HARDDRIVE);
		do{
		  Path=fileselect.choose(XD,RunDir,"",T("Pick a Folder"),
		    FSM_CHOOSE_FOLDER | FSM_CONFIRMCREATE,folder_parse_routine,"");
		  if (Path.Empty()) break;
		  NO_SLASH(Path);

      if (Let>=9) break;
      HardDiskMan.NewDrive(Path);
			proceed=!alert.ask(XD,Mess[Let++],caption,T("Yes")+"|"+T("No"),1,1);
		}while(proceed);
#endif
  }

	caption=T("Get Ready For Steem!");
	text=T("Congratulations, Steem is now ready to go. Click on the green play button to start emulation. To release the PC mouse press the Pause/Break key. To stop emulation click on the run button again or press Shift + Pause/Break.")+
         "\n\n"+T("Have fun!");

#ifdef WIN32
  mbp.lpszCaption=caption;
  mbp.lpszText=text;
  mbp.dwStyle=MB_USERICON | MB_OK;
  MessageBoxIndirect(&mbp);
  return 0;
#elif defined(UNIX)
	alert.ask(XD,text,caption,T("Okay"),0,0);
  return 0;
#endif
}
//---------------------------------------------------------------------------

