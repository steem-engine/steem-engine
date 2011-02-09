//void draw_scanline_hires_pixelwise_8(int border1,int picture,int border2,int hscroll){
  int n;
  WORD w0;
  DWORD fore,back;
  if (STpal[0] & 1){
    back=0xffffffff;fore=0;
  }else{
    fore=0xffffffff;back=0;
  }

  MEM_ADDRESS source;
  GET_START(0,80)
  for(n=border1;n>0;n--){
    DRAWPIXEL(back);DRAWPIXEL(back);DRAWPIXEL(back);DRAWPIXEL(back);
    DRAWPIXEL(back);DRAWPIXEL(back);DRAWPIXEL(back);DRAWPIXEL(back);
    DRAWPIXEL(back);DRAWPIXEL(back);DRAWPIXEL(back);DRAWPIXEL(back);
    DRAWPIXEL(back);DRAWPIXEL(back);DRAWPIXEL(back);DRAWPIXEL(back);
  }
  for(n=picture;n>0;n--){
    GET_SCREEN_DATA_INTO_REGS_AND_INC_SA_HIRES
    CALC_COL_HIRES_AND_DRAWPIXEL(BIT_15);
    CALC_COL_HIRES_AND_DRAWPIXEL(BIT_14);
    CALC_COL_HIRES_AND_DRAWPIXEL(BIT_13);
    CALC_COL_HIRES_AND_DRAWPIXEL(BIT_12);
    CALC_COL_HIRES_AND_DRAWPIXEL(BIT_11);
    CALC_COL_HIRES_AND_DRAWPIXEL(BIT_10);
    CALC_COL_HIRES_AND_DRAWPIXEL(BIT_9);
    CALC_COL_HIRES_AND_DRAWPIXEL(BIT_8);
    CALC_COL_HIRES_AND_DRAWPIXEL(BIT_7);
    CALC_COL_HIRES_AND_DRAWPIXEL(BIT_6);
    CALC_COL_HIRES_AND_DRAWPIXEL(BIT_5);
    CALC_COL_HIRES_AND_DRAWPIXEL(BIT_4);
    CALC_COL_HIRES_AND_DRAWPIXEL(BIT_3);
    CALC_COL_HIRES_AND_DRAWPIXEL(BIT_2);
    CALC_COL_HIRES_AND_DRAWPIXEL(BIT_1);
    CALC_COL_HIRES_AND_DRAWPIXEL(BIT_0);
  }
  for(n=border2;n>0;n--){
    DRAWPIXEL(back);DRAWPIXEL(back);DRAWPIXEL(back);DRAWPIXEL(back);
    DRAWPIXEL(back);DRAWPIXEL(back);DRAWPIXEL(back);DRAWPIXEL(back);
    DRAWPIXEL(back);DRAWPIXEL(back);DRAWPIXEL(back);DRAWPIXEL(back);
    DRAWPIXEL(back);DRAWPIXEL(back);DRAWPIXEL(back);DRAWPIXEL(back);
  }

