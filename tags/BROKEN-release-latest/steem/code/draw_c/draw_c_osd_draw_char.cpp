BYTE* dadd=draw_mem+x*bpp+y*draw_line_length,*daddl=dadd;
DWORD dw0,dw1;

for (int yy=h;yy>0;yy--){
  dw0=*source_ad;
  dw1=*(source_ad+1);
  source_ad+=2;
  OSD_PIXEL(BIT_31);
  OSD_PIXEL(BIT_30);
  OSD_PIXEL(BIT_29);
  OSD_PIXEL(BIT_28);
  OSD_PIXEL(BIT_27);
  OSD_PIXEL(BIT_26);
  OSD_PIXEL(BIT_25);
  OSD_PIXEL(BIT_24);
  OSD_PIXEL(BIT_23);
  OSD_PIXEL(BIT_22);
  OSD_PIXEL(BIT_21);
  OSD_PIXEL(BIT_20);
  OSD_PIXEL(BIT_19);
  OSD_PIXEL(BIT_18);
  OSD_PIXEL(BIT_17);
  OSD_PIXEL(BIT_16);
  OSD_PIXEL(BIT_15);
  OSD_PIXEL(BIT_14);
  OSD_PIXEL(BIT_13);
  OSD_PIXEL(BIT_12);
  OSD_PIXEL(BIT_11);
  OSD_PIXEL(BIT_10);
  OSD_PIXEL(BIT_9);
  OSD_PIXEL(BIT_8);
  OSD_PIXEL(BIT_7);
  OSD_PIXEL(BIT_6);
  OSD_PIXEL(BIT_5);
  OSD_PIXEL(BIT_4);
  OSD_PIXEL(BIT_3);
  OSD_PIXEL(BIT_2);
  OSD_PIXEL(BIT_2);
  OSD_PIXEL(BIT_1);
  OSD_PIXEL(BIT_0);
  daddl+=draw_line_length;
  dadd=daddl;
}

