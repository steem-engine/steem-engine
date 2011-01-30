typedef struct{
	int x1,y1,w,h;
}Sprite;

void init_sprites(char*,int,int),put(HANDLE,Sprite*,int,int),destroy_sprites();

HBITMAP s_bmp,s_old,s_maskbmp,s_maskold;
HDC s_dc,s_maskdc;
int s_bmp_w,s_bmp_h;
//---------------------------------------------------------------------------
void init_sprites(char *bitmap_file,int w,int h)
{
  long c,c2;
  s_bmp_w=w;s_bmp_h=h;

  HDC a=GetDC(0);
  s_dc=CreateCompatibleDC(a);
  s_maskdc=CreateCompatibleDC(a);
  s_maskbmp=CreateCompatibleBitmap(a,s_bmp_w,s_bmp_h);
  ReleaseDC(0,a);

  s_bmp=(HBITMAP)LoadImage(0,bitmap_file,IMAGE_BITMAP,0,0,LR_DEFAULTSIZE | LR_LOADFROMFILE);
  s_old=(HBITMAP)SelectObject(s_dc,s_bmp);
  s_maskold=(HBITMAP)SelectObject(s_maskdc,s_maskbmp);

  long bkcol=GetPixel(s_dc,0,0);
  for (int x=0;x<s_bmp_w;x++){
    for (int y=0;y<s_bmp_h;y++){
      c=GetPixel(s_dc,x,y);
      if (c==bkcol){
        c2=RGB(255,255,255);c=0;
      }else
        c2=0;

      SetPixel(s_maskdc,x,y,c2);
      SetPixel(s_dc,x,y,c);
    }
	}
}
//---------------------------------------------------------------------------
void put(HDC dc,Sprite *sp,int x,int y)
{
  BitBlt(dc,x,y,sp->w,sp->h,s_maskdc,sp->x1,sp->y1,SRCAND);
  BitBlt(dc,x,y,sp->w,sp->h,s_dc,sp->x1,sp->y1,SRCINVERT);
}
//---------------------------------------------------------------------------
void destroy_sprites()
{
  SelectObject(s_dc,s_old);
  DeleteObject(s_bmp);
  DeleteDC(s_dc);

  SelectObject(s_maskdc,s_maskold);
  DeleteObject(s_maskbmp);
  DeleteDC(s_maskdc);
}

