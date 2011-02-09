//---------------------------------------------------------------------------
bool macro_mvi_blank(MACROVBLINFO *mvi)
{
  if (mvi->xdiff || mvi->ydiff || mvi->keys) return 0;
  for (int i=0;i<8;i++) if (mvi->stick[i]) return 0;
  for (int i=0;i<2;i++) if (mvi->jagpad[i]) return 0;
  return true;
}
//---------------------------------------------------------------------------
void macro_file_options(int GetSet,char *Path,MACROFILEOPTIONS *lpMFO,FILE *f)
{
  if (GetSet==MACRO_FILE_GET){
    lpMFO->add_mouse_together=MACRO_DEFAULT_ADD_MOUSE;
    lpMFO->allow_same_vbls=MACRO_DEFAULT_ALLOW_VBLS;
    lpMFO->max_mouse_speed=MACRO_DEFAULT_MAX_MOUSE;
  }

  if (Path){
    if (GetSet==MACRO_FILE_GET){
      f=fopen(Path,"rb");
    }else{
      if (Exists(Path)){
        f=fopen(Path,"r+b");
      }else{
        f=fopen(Path,"wb");
      }
    }
  }
  if (f){
    unsigned int Version=2;
    if (GetSet==MACRO_FILE_SET && GetFileLength(f)==0){ // Blank file?
      int Dummy=0;
      fwrite(&Version,1,4,f);fwrite(&Dummy,1,4,f);fwrite(&Dummy,1,4,f);fwrite(&Dummy,1,4,f);
    }
    fseek(f,0,SEEK_SET);
    fread(&Version,1,4,f);
    if (Version>=2){
      fseek(f,4+4+4+4,SEEK_SET);

      #define LoadSave(var) if (GetSet==MACRO_FILE_SET) fwrite(&(var),1,4,f); else fread(&(var),1,4,f);

      LoadSave(lpMFO->add_mouse_together);
      LoadSave(lpMFO->max_mouse_speed);
      LoadSave(lpMFO->allow_same_vbls);

      #undef LoadSave
    }
    if (Path) fclose(f);
  }
}
//---------------------------------------------------------------------------
void macro_advance(int StartCode)
{
  int max_mouse=0;
  if (macro_record || (StartCode & MACRO_STARTRECORD)){
    bool advance=true;
    if (macro_record==0){
      macro_record_store.Resize(shifter_freq*MACRO_RECORD_BUF_INC_SECS);
      MACROFILEOPTIONS MFO;
      macro_file_options(MACRO_FILE_GET,macro_record_file,&MFO);
      max_mouse=MFO.max_mouse_speed;
    }else if (macro_record==1){ // Don't move on to 2 until there is input
      if (macro_mvi_blank(mrsc)) advance=0;
    }else if (macro_record>=macro_record_store.GetSize()){
      macro_record_store.Resize(macro_record_store.GetSize() + shifter_freq*MACRO_RECORD_BUF_INC_SECS);
    }
    if (advance){
      mrsc=&(macro_record_store[macro_record]);
      macro_record++;
    }
    mrsc->keys=0;
    mrsc->xdiff=0xffff;
  }
  if (macro_play || (StartCode & MACRO_STARTPLAY)){
    if (macro_play==0){
      if (macro_play_start()==0) return;
      if (macro_play_has_mouse) max_mouse=macro_play_max_mouse_speed;
    }
    if (macro_play>=macro_play_until){
      macro_end(MACRO_ENDPLAY);
      return;
    }
    mpsc=&(macro_play_store[macro_play]);
    macro_play++;
  }
  if (max_mouse){
    mousek=0;
    ikbd_mouse_move(-MACRO_LEFT_INIT_MOVE,-MACRO_UP_INIT_MOVE,mousek,max_mouse);
    macro_start_after_ikbd_read_count=keyboard_buffer_length;
  }
  if (StartCode) OptionBox.UpdateMacroRecordAndPlay();
}
//---------------------------------------------------------------------------
void macro_end(int EndCode)
{
  if (macro_record && (EndCode & MACRO_ENDRECORD)){
    // Cut off current frame if it hasn't been set yet
    // (macro_record=current frame +1, should be last recorded frame +1)
    if (macro_record_store[macro_record-1].xdiff==0xffff) macro_record--;

    // Cut blank frames off the end
    for (int n=macro_record-1;n>=0;n--){
      if (macro_mvi_blank(&macro_record_store[n])){
        macro_record--;
      }else{
        break;
      }
    }

    if (macro_record>0){
      MACROFILEOPTIONS MFO;
      macro_file_options(MACRO_FILE_GET,macro_record_file,&MFO);

      FILE *f=fopen(macro_record_file,"wb");
      if (f){
        unsigned int Version=2,SizeMVI=sizeof(MACROVBLINFO),StructOffset=7*4;
        fwrite(&Version,1,4,f);
        fwrite(&SizeMVI,1,4,f);
        fwrite(&StructOffset,1,4,f);
        fwrite(&macro_record,1,4,f);
        fwrite(&MFO.add_mouse_together,1,4,f);
        fwrite(&MFO.max_mouse_speed,1,4,f);
        fwrite(&MFO.allow_same_vbls,1,4,f);
        for (int n=0;n<macro_record;n++) fwrite(&(macro_record_store[n]),1,SizeMVI,f);
        fclose(f);
      }
    }
    macro_record=0;
    macro_record_store.DeleteAll();
  }
  if (EndCode & MACRO_ENDPLAY){
    macro_play=0;
    macro_play_store.DeleteAll();
    macro_play_has_mouse=0;
    macro_play_has_keys=0;
    macro_play_has_joys=0;
  }
  if (macro_play==0 && macro_record==0) macro_start_after_ikbd_read_count=0;
  OptionBox.UpdateMacroRecordAndPlay();
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void macro_record_joy()
{
  for (int Port=0;Port<8;Port++) mrsc->stick[Port]=stick[Port];
  mrsc->jagpad[0]=macro_jagpad[0];
  mrsc->jagpad[1]=macro_jagpad[1];
}
//---------------------------------------------------------------------------
void macro_record_mouse(int x_change,int y_change)
{
  mrsc->xdiff=x_change;
  mrsc->ydiff=y_change;
}
//---------------------------------------------------------------------------
void macro_record_key(BYTE STCode)
{
  if (mrsc->keys<32) mrsc->keycode[mrsc->keys++]=STCode;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool macro_play_start()
{
  FILE *f=fopen(macro_play_file,"rb");
  if (f==NULL) return 0;

  unsigned int Version=0,SizeMVI=0,StructOffset=0;
  fread(&Version,1,4,f);
  fread(&SizeMVI,1,4,f);
  fread(&StructOffset,1,4,f);
  if (Version==0 || SizeMVI==0 || StructOffset==0){
    fclose(f);
    return 0;
  }

  fread(&macro_play_until,1,4,f);
  macro_play_store.Resize(macro_play_until+16);

  MACROFILEOPTIONS MFO;
  macro_file_options(MACRO_FILE_GET,NULL,&MFO,f);
  macro_play_max_mouse_speed=MFO.max_mouse_speed;

  fseek(f,StructOffset,SEEK_SET);

  int idx=0;
  MACROVBLINFO last_cut;
  int mxa=0,mya=0,no_event_vbls=0xffff,cut_vbls=0;
  BYTE oldstick[8]={0,0,0,0,0,0,0,0};
  DWORD oldjag[2]={0,0};
  for (int n=0;n<macro_play_until;n++){
    MACROVBLINFO *lpMVI=&(macro_play_store[idx]);
    ZeroMemory(lpMVI,sizeof(MACROVBLINFO)); // Zero what isn't loaded
    fread(lpMVI,1,min(sizeof(MACROVBLINFO),SizeMVI),f);
    if (sizeof(MACROVBLINFO)<SizeMVI){
      fseek(f,SizeMVI-sizeof(MACROVBLINFO),SEEK_CUR);
    }

    bool new_event=0;
    if (lpMVI->keys > 0){
      new_event=true;
      macro_play_has_keys=true;
    }
    for (int Port=0;Port<8;Port++){
      if (lpMVI->stick[Port]!=oldstick[Port]){
        new_event=true;
        macro_play_has_joys=true;
      }
      oldstick[Port]=lpMVI->stick[Port];
    }
    for (int Jag=0;Jag<2;Jag++){
      if (lpMVI->jagpad[Jag]!=oldjag[Jag]){
        new_event=true;
        macro_play_has_joys=true;
      }
      oldjag[Jag]=lpMVI->jagpad[Jag];
    }

    bool CanCutFrame=0;
    if (MFO.allow_same_vbls>0){
      if (new_event){
        no_event_vbls=0;
      }else{
        no_event_vbls++;
        if (no_event_vbls>=MFO.allow_same_vbls) CanCutFrame=true;
      }
    }

    if (CanCutFrame){
      mxa+=lpMVI->xdiff;
      mya+=lpMVI->ydiff;

      // Store stick and jagpad settings (mouse and keys ignored)
      if (MFO.allow_same_vbls>0) last_cut=*lpMVI;

      cut_vbls++;
    }else{
      if (mxa || mya || lpMVI->xdiff || lpMVI->ydiff) macro_play_has_mouse=true;
      if (MFO.allow_same_vbls>0 && cut_vbls){
        // Store current frame
        MACROVBLINFO MVI=macro_play_store[idx];
        // Must insert frame(s) before pressing button to move mouse to new position
        cut_vbls=min(cut_vbls,MFO.allow_same_vbls);
        for (int n=0;n<cut_vbls;n++){
          // Make all the movement occur on the first frame
          last_cut.xdiff=mxa;mxa=0;
          last_cut.ydiff=mya;mya=0;
          macro_play_store[idx++]=last_cut;
        }
        macro_play_store[idx]=MVI;
      }
      idx++; // Insert current frame
      cut_vbls=0;
      if (idx>=macro_play_store.GetSize()){
        macro_play_store.Resize(macro_play_store.GetSize()+16);
      }
    }
  }
  if (mxa || mya){ // Left over movement
    macro_play_has_mouse=true;
    macro_play_store[idx].xdiff=mxa;
    macro_play_store[idx].ydiff=mya;
    idx++;
  }
  fclose(f);
  macro_play_until=idx;
  return true;
}
//---------------------------------------------------------------------------
void macro_play_joy()
{
  for (int Port=0;Port<8;Port++) stick[Port]=(BYTE)(mpsc->stick[Port]);
  macro_jagpad[0]=mpsc->jagpad[0];
  macro_jagpad[1]=mpsc->jagpad[1];
}
//---------------------------------------------------------------------------
void macro_play_mouse(int &x_change,int &y_change)
{
  x_change=mpsc->xdiff;
  y_change=mpsc->ydiff;
}
//---------------------------------------------------------------------------
void macro_play_keys()
{
  for (DWORD n=0;n<mpsc->keys;n++){
    keyboard_buffer_write(mpsc->keycode[n]);
  }
}
//---------------------------------------------------------------------------

