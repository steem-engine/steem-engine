/*---------------------------------------------------------------------------
FILE: palette.cpp
MODULE: Steem
DESCRIPTION: General palette utility functions. This covers both the code
to create the ST palette and also the code to add to the PC palette.
---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
void make_palette_table(int brightness,int contrast)
{
  contrast+=256;
  long c;
  for (int n=0;n<4096;n++){
    c=0;
    for (int r=0;r<3;r++){ //work out levels
      int l=(n >> ((2-r)*4)) & 15;
      l=((l << 5)+(l << 1)) & (15 << 4); // unscramble bits and multiply by 16
                                          // to get number between 0 and 240
      l*=contrast;
      l/=256;
      l+=brightness;
      if (l<0){
        l=0;
      }else if (l>255){
        l=255;
      }
      if (BytesPerPixel==2){
        if (rgb555==0 && r==1){ // green 565
          c<<=6;
          c+=(l >> 2);
        }else{
          c<<=5;
          c+=(l >> 3);
        }
      }else{ //24/32 mode, 8-bit too (value used in call to add to palette)
        c<<=8;
        c+=l;
      }
    }
    if (BytesPerPixel==2){
      c|=(c << 16);
    }else if (BytesPerPixel==3){
      c|=(c << 24);
    }else if (rgb32_bluestart_bit){ // 0xRRGGBB00
      c<<=8;
    }
    palette_table[n]=c;
  }
}
//---------------------------------------------------------------------------



void palette_prepare(bool get)
{
  palette_remove();
#ifdef WIN32
  if (BytesPerPixel==1){
    PalDC=CreateCompatibleDC(NULL);

    if (get){
      GetSystemPaletteEntries(PalDC,1,255,(PALETTEENTRY*)(logpal+1));
      logpal[0]=MAKELONG(0x300,254);
      for(int n=10;n<246;n++){
        ((PALETTEENTRY*)logpal+n)->peFlags=PC_RESERVED;
      }
    }
    winpal=CreatePalette((LOGPALETTE*)logpal);

    oldwinpal=SelectPalette(PalDC,winpal,1);
    SetSystemPaletteUse(PalDC,SYSPAL_NOSTATIC);
    RealizePalette(PalDC);
  }
#endif
}

void palette_flip()
{
  if (palette_changed){
    WIN_ONLY( AnimatePalette(winpal,palhalf+10,118,(PALETTEENTRY*)(logpal+palhalf+10)); )
    UNIX_ONLY( XStoreColors(XD,colormap,new_pal+palhalf,palnum); )
    palette_changed=false;
  }
#ifdef WIN32
  if (runstate==RUNSTATE_RUNNING) palhalf^=118;
#else
//  printf("palhalf was %i palnum was %i\n",palhalf,palnum);
  if (runstate==RUNSTATE_RUNNING) palhalf^=128;
#endif
  palnum=0;
}

void palette_remove()
{
#ifdef WIN32
  if (winpal){
    log_to_section(LOGSECTION_SHUTDOWN,"SHUTDOWN: Destroying palette");
    SetSystemPaletteUse(PalDC,SYSPAL_STATIC);
    SelectPalette(PalDC,oldwinpal,1);

    DeleteDC(PalDC);
    DeleteObject(winpal);

    winpal=NULL;
  }
#endif
}



void palette_copy()
{
  int n_cols=16;

  switch (screen_res){
    case 1:n_cols=4;break;
    case 2:return;  ///// Added by Tarq
#ifndef NO_CRAZY_MONITOR
    case 3: //extended monitor
      if (em_planes==1) return;
      n_cols=(1<<em_planes);
      break;
#endif
  }
  for (int n=0;n<n_cols;n++){
    PCpal[n]=palette_add_entry(palette_table[(STpal[n] & 0xfff)]);
  }
}


WORD palette_add_entry(DWORD col) // Add BGR colour to palette
{
#ifdef WIN32
  PALETTEENTRY pbuf;
  int n,m;
  pbuf.peFlags=PC_RESERVED;
  pbuf.peRed=  BYTE((col & 0xff0000) >> 16);
  pbuf.peGreen=BYTE((col & 0x00ff00) >> 8);
  pbuf.peBlue= BYTE((col & 0x0000ff));

  n=10+palhalf+palnum;
  if (palnum<=117){
    if (*((LONG*)(logpal+n))==*((LONG*)&pbuf)){
      palnum++;
      n++;
      return WORD(n | (n<<8));
    }
    palnum++;
  }else{
    n=10+palhalf;
    for (m=0;m<palnum;m++){
      if (*(LONG*)(logpal+10+palhalf+m)==*((LONG*)&pbuf)){
        n=10+palhalf+m;
        n++;
        return WORD(n|(n<<8));
      }
    }
  }

  *((PALETTEENTRY*)(logpal+n))=pbuf;
  palette_changed=true;

  n++;
  return WORD(n|(n<<8));
#elif defined(UNIX)
  if (XD==NULL || colormap==0) return 0;
  int n;
  if(palnum<=127){
    while (DWORD(logpal[palhalf+palnum])==0xffffffff) palnum++;
  }
  if(palnum<=127){
    n=palhalf+palnum;
    new_pal[n].red=(col & 0xff0000)>>8;
    new_pal[n].green=(col & 0xff00);
    new_pal[n].blue=(col & 0xff)<<8;
    logpal[n]=col;
 //   XStoreColor(XD,colormap,&xc);
    palnum++;
    palette_changed=true;
    return WORD(n|(n<<8));
  }else{
    n=palhalf;
    for (int m=0;m<palnum;m++){
      if (*(DWORD*)(logpal+palhalf+m)==col){
        n=palhalf+m;
        return WORD(n|(n<<8));
      }
    }
  }
  return 0;
/*
  if (XD==NULL) return 0;

  XColor xc={0,(col & 0xf00)<< 4,(col & 0x0f0) << 8,(col & 0x00f)<< 12};
  XAllocColor(XD,XDefaultColormap(XD,XDefaultScreen(XD)),&xc);
  return WORD(BYTE(xc.pixel) | (xc.pixel << 8));
*/
#endif
}

/*
  WORD palette_add_entry(WORD col){ //add STE colour to palette
    PALETTEENTRY*pe;
    int n;

    n=10+palhalf+palnum;
    if(palnum>117){
      n=10+palhalf;
    }else{
      palnum++;
    }

    pe=(PALETTEENTRY*)(logpal+n);

    pe->peRed=BYTE((col&0xf00)>>4);
    pe->peGreen=BYTE((col&0x0f0));
    pe->peBlue=BYTE((col&0x00f)<<4);

    palette_changed=true;

    n++;
    return WORD(n|(n<<8));
  }
*/


void palette_convert(int n)
{
//  long col=((STpal[n] & 0x888) >> 3)+((STpal[n] & 0x777) << 1);  //correct STE colour
  if (BytesPerPixel==1){
    if (draw_lock) PCpal[n]=palette_add_entry(palette_table[(STpal[n] & 0xfff)]);
  }else{
    PCpal[n]=palette_table[(STpal[n] & 0xfff)];
  }
}

void palette_convert_all()
{
  for (int n=0;n<16;n++) palette_convert(n);
  for (int n=0;n<256;n++) emudetect_falcon_palette_convert(n);
}

