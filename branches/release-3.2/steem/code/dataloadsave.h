#define SEC(n) if (SecDisabled[int(n)]==0)

#ifndef ONEGAME

#define UPDATE \
    if (Handle) Hide();  \
    LoadPosition(pCSF); \
    if (pCSF->GetInt(Section,"Visible",0)) Show();

#else
#define UPDATE
#endif

void LoadAllDialogData(bool,Str,bool* = NULL,GoodConfigStoreFile* = NULL);
void SaveAllDialogData(bool,Str,ConfigStoreFile* = NULL);

#define PSEC_SNAP 0
#define PSEC_PASTE 1
#define PSEC_CUT 2
#define PSEC_PATCH 3
#define PSEC_MACHINETOS 4
#define PSEC_MACRO 5
#define PSEC_PORTS 6
#define PSEC_GENERAL 7
#define PSEC_SOUND 8
#define PSEC_DISPFULL 9
#define PSEC_STARTUP 10
#define PSEC_AUTOUP 11
#define PSEC_JOY 12
#define PSEC_HARDDRIVES 13
#define PSEC_DISKEMU 14
#define PSEC_POSSIZE 15
#define PSEC_DISKGUI 16
#define PSEC_PCJOY 17
#define PSEC_OSD 18

typedef struct{
  char *Name;
  int ID;
}ProfileSectionData;

ProfileSectionData ProfileSection[20]=
      {{"Machine and TOS",PSEC_MACHINETOS},{"Ports and MIDI",PSEC_PORTS},
       {"General",PSEC_GENERAL},{"Display, Fullscreen, Brightness and Contrast",PSEC_DISPFULL},
       {"On Screen Display",PSEC_OSD},{"Steem Window Position and Size",PSEC_POSSIZE},
       {"Disk Emulation",PSEC_DISKEMU},{"Disk Manager",PSEC_DISKGUI},
       {"Hard Drives",PSEC_HARDDRIVES},{"Joysticks",PSEC_JOY},
#ifdef UNIX
       {"PC Joysticks",PSEC_PCJOY},
#endif
       {"Sound",PSEC_SOUND},{"Shortcuts",PSEC_CUT},{"Macros",PSEC_MACRO},
       {"Patches",PSEC_PATCH},{"Startup",PSEC_STARTUP},
       {"Auto Update and File Associations",PSEC_AUTOUP},
       {"Memory Snapshots",PSEC_SNAP},{"Paste Delay",PSEC_PASTE},
       {NULL,-1}};

Str ProfileSectionGetStrFromID(int ID)
{
  for (int i=0;;i++){
    if (ProfileSection[i].Name==NULL) break;
    if (ProfileSection[i].ID==ID) return ProfileSection[i].Name;
  }
  return "";
}
