long palette_table[4096];

int brightness,contrast;

void make_palette_table(int brightness,int contrast){
  contrast+=256;
  long c;
  for(int n=0;n<4096;n++){
    c=0;
    for(int r=0;r<3;r++){ //work out levels
      l=(n>>((2-r)*4))&15;
      l=((l<<5)+(l<<1))&(15<<4); //unscramble bits and multiply by 16
                //to get number between 0 and 240
      l*=contrast;
      l/=256;
      l+=brightness;
      if(l<0)l=0;else if(l>255)l=255;
      if(BytesPerPixel==2){
        if(rgb555 || (r!=1)){
          c+=(l&31);
          c<<=5;
        }else{ //565 mode, green
          c+=(l&63);
          c<<=6;
        }
      }else{ //24/32 mode
        c+=l;
        c<<=8;
      }
    }
    if(BytesPerPixel==2){
      c|=(c<<16);
    }
    palette_table[n]=c;
  }
}
