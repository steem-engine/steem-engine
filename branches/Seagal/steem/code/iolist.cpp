/*---------------------------------------------------------------------------
FILE: iolist.cpp
MODULE: Steem
CONDITION: _DEBUG_BUILD
DESCRIPTION: A global list that stores information about various special
addresses and how they should be displayed in the debugger.
---------------------------------------------------------------------------*/

void iolist_add_entry(MEM_ADDRESS ad,char*name,int bytes,char*bitmask,BYTE*ptr)
{
  iolist_entry*p=iolist+iolist_length;
  p->ad=ad;
  if(name==NULL)p->name="";else p->name=name;
  p->bytes=bytes;
  p->ptr=ptr;
  if(bitmask==NULL){ //bitmask not significant
    p->bitmask="";//"F|E|D|C|B|A|9|8|7|6|5|4|3|2|1|0";
  }else if(*bitmask=='#'){  //special masks
    p->bitmask=bitmask;
  }else{
    p->bitmask="";
    int n=15;if(bytes==1)n=7;
    for(char*c=bitmask;*c;c++){
      if(*c=='|')n--;
    }
    for(;n>0;n--){
      p->bitmask+=".|"; //pad
    }
    p->bitmask+=bitmask;
  }
  iolist_length++;
};



void iolist_init()
{
  iolist_length=0;
  iolist_add_entry(0x0,"Initial SSP",4);
  iolist_add_entry(0x4,"Initial PC",4);
  iolist_add_entry(0x8,"Bus Error Vector",4);
  iolist_add_entry(0xc,"Address Error Vector",4);
  iolist_add_entry(0x10,"Illegal Instruction Vector",4);
  iolist_add_entry(0x14,"Divide By Zero Vector",4);
  iolist_add_entry(0x18,"CHK vector",4);
  iolist_add_entry(0x1c,"TRAPV vector",4);
  iolist_add_entry(0x20,"Privelege Violation Vector",4);
  iolist_add_entry(0x24,"Trace Exception Vector",4);
  iolist_add_entry(0x28,"Line-A Emulator",4);
  iolist_add_entry(0x2c,"Line-F Emulator",4);
  iolist_add_entry(0x60,"68000 Spurious Interrupt",4);
  iolist_add_entry(0x64,"68000 Level 1 Interrupt (not connected)",4);
  iolist_add_entry(0x68,"68000 Level 2 Interrupt (HBL)",4);
  iolist_add_entry(0x6c,"68000 Level 3 Interrupt (not connected)",4);
  iolist_add_entry(0x70,"68000 Level 4 Interrupt (VBL)",4);
  iolist_add_entry(0x74,"68000 Level 5 Interrupt (not connected)",4);
  iolist_add_entry(0x78,"68000 Level 6 Interrupt (MFP)",4);
  iolist_add_entry(0x7c,"68000 Level 7 Interrupt (not connected)",4);
  iolist_add_entry(0x80,"Trap #0",4);
  iolist_add_entry(0x84,"Trap #1 (GEMDOS)",4);
  iolist_add_entry(0x88,"Trap #2 (AES/VDI)",4);
  iolist_add_entry(0x8C,"Trap #3",4);
  iolist_add_entry(0x90,"Trap #4",4);
  iolist_add_entry(0x94,"Trap #5",4);
  iolist_add_entry(0x98,"Trap #6",4);
  iolist_add_entry(0x9C,"Trap #7",4);
  iolist_add_entry(0xA0,"Trap #8",4);
  iolist_add_entry(0xA4,"Trap #9",4);
  iolist_add_entry(0xA8,"Trap #10",4);
  iolist_add_entry(0xAC,"Trap #11",4);
  iolist_add_entry(0xB0,"Trap #12",4);
  iolist_add_entry(0xB4,"Trap #13 (BIOS)",4);
  iolist_add_entry(0xB8,"Trap #14 (XBIOS)",4);
  iolist_add_entry(0xBc,"Trap #15",4);

  iolist_add_entry(0x100,"MFP 0 - Centronics Busy",4);
  iolist_add_entry(0x104,"MFP 1 - RS232 DCD",4);
  iolist_add_entry(0x108,"MFP 2 - RS232 CTS",4);
  iolist_add_entry(0x10c,"MFP 3 - Blitter Done",4);
  iolist_add_entry(0x110,"MFP 4 - Timer D",4);
  iolist_add_entry(0x114,"MFP 5 - Timer C",4);
  iolist_add_entry(0x118,"MFP 6 - ACIA",4);
  iolist_add_entry(0x11c,"MFP 7 - FDC",4);
  iolist_add_entry(0x120,"MFP 8 - Timer B",4);
  iolist_add_entry(0x124,"MFP 9 - Send Error",4);
  iolist_add_entry(0x128,"MFP 10 - Send Buffer Empty",4);
  iolist_add_entry(0x12c,"MFP 11 - Receive Error",4);
  iolist_add_entry(0x130,"MFP 12 - Receive Buffer Full",4);
  iolist_add_entry(0x134,"MFP 13 - Timer A",4);
  iolist_add_entry(0x138,"MFP 14 - RS232 Ring Detect",4);
  iolist_add_entry(0x13c,"MFP 15 - Monochrome Detect",4);

  iolist_add_entry(0xff8001,"MMU mem config",1,"Bank0h|Bank0l|Bank1h|Bank1l");

  iolist_add_entry(0xff8201,"Screen Base High",1);
  iolist_add_entry(0xff8203,"Screen Base Mid",1);
  iolist_add_entry(0xff8205,"Vid Ptr High",1);
  iolist_add_entry(0xff8207,"Vid Ptr Mid",1);
  iolist_add_entry(0xff8209,"Vid Ptr Low",1);
  iolist_add_entry(0xff820a,"Freq",1,"50Hz|.");
  iolist_add_entry(0xff820d,"Screen Base Low",1);
  iolist_add_entry(0xff820f,"Scanline Word Skip Count",1);
  for(int n=0;n<16;n++){
    iolist_add_entry(0xff8240+n*2,EasyStr("pal")+n,2,"#COLOUR");
  }
  iolist_add_entry(0xff8260,"Res",1);
  iolist_add_entry(0xff8264,"HScroll (No Extra Fetch)",1);
  iolist_add_entry(0xff8265,"HScroll",1);

  iolist_add_entry(0xff8604,"FDC access/sector count",2);
  iolist_add_entry(0xff8606,"DMA mode/status",2);
  iolist_add_entry(0xff8609,"DMA high",1);
  iolist_add_entry(0xff860b,"DMA mid",1);
  iolist_add_entry(0xff860d,"DMA low",1);
  iolist_add_entry(0xff860e,"Freq/Density Control",2);
  iolist_add_entry(0xff8800,"PSG read data/reg sel",1);
  iolist_add_entry(0xff8802,"PSG write data",1);

  iolist_add_entry(0xff8901,"DMA Sound Control",1,"Loop|Play");
  iolist_add_entry(0xff8903,"DMA Sound Start High",1);
  iolist_add_entry(0xff8905,"DMA Sound Start Mid",1);
  iolist_add_entry(0xff8907,"DMA Sound Start Low",1);
  iolist_add_entry(0xff8909,"DMA Sound Current High",1);
  iolist_add_entry(0xff890b,"DMA Sound Current Mid",1);
  iolist_add_entry(0xff890d,"DMA Sound Current Low",1);
  iolist_add_entry(0xff890f,"DMA Sound End High",1);
  iolist_add_entry(0xff8911,"DMA Sound End Mid",1);
  iolist_add_entry(0xff8913,"DMA Sound End Low",1);
  iolist_add_entry(0xff8921,"DMA Sound Mode",1,"Mono|.|.|.|.|.|Freq|Freq");
  iolist_add_entry(0xff8922,"Microwire Data",1);
  iolist_add_entry(0xff8924,"Microwire Mask",1);

  for (int line=0;line<16;line++){
    iolist_add_entry(0xFF8A00+(line*2),EasyStr("Halftone RAM [")+line+"]",2);
  }
  iolist_add_entry(0xFF8A20,"SrcXInc",2);
  iolist_add_entry(0xFF8A22,"SrcYInc",2);
  iolist_add_entry(0xFF8A24,"SrcAdr",4);
  iolist_add_entry(0xFF8A28,"EndMask[0]",2);
  iolist_add_entry(0xFF8A2A,"EndMask[1]",2);
  iolist_add_entry(0xFF8A2C,"EndMask[2]",2);
  iolist_add_entry(0xFF8A2E,"DestXInc",2);
  iolist_add_entry(0xFF8A30,"DestYInc",2);
  iolist_add_entry(0xFF8A32,"DestAdr",4);
  iolist_add_entry(0xFF8A36,"XCount",2);
  iolist_add_entry(0xFF8A38,"YCount",2);
  iolist_add_entry(0xFF8A3A,"Halftone Op",1);
  iolist_add_entry(0xFF8A3B,"Op",1);
  iolist_add_entry(0xFF8A3C,"Flags1",1,"Busy|Hog|Smudge|.|LineNum3|LineNum2|LineNum1|LineNum0");
  iolist_add_entry(0xFF8A3D,"Flags2",1,"FXSR|NFSR|.|.|Skew3|Skew2|Skew1|Skew0");

  iolist_add_entry(0xfffa01,"MFP GPIP (1)",1,"mono|rs232 ring ind|FDC/HDC|ACIA|Blitter|rs232 CTS|rs232 data carrier|centronics busy");
  iolist_add_entry(0xfffa03,"MFP AER (2)",1,"mono|rs232 ring ind|FDC/HDC|ACIA|Blitter|rs232 CTS|rs232 data carrier|centronics busy");
  iolist_add_entry(0xfffa05,"MFP DDR (3)",1,"mono|rs232 ring ind|FDC/HDC|ACIA|Blitter|rs232 CTS|rs232 data carrier|centronics busy");
  iolist_add_entry(0xfffa07,"MFP IERA (4)",1,"mono|rs232 ring|TIMER A|RX buf full|RX error|TX buf empty|TX error|TIMER B");
  iolist_add_entry(0xfffa09,"MFP IERB (5)",1,"FDC|ACIA|TIMER C|TIMER D|Blitter|CTS|rs232con|LPT");
  iolist_add_entry(0xfffa0b,"MFP IPRA (6)",1,"mono|rs232 ring|TIMER A|RX buf full|RX error|TX buf empty|TX error|TIMER B");
  iolist_add_entry(0xfffa0d,"MFP IPRB (7)",1,"FDC|ACIA|TIMER C|TIMER D|Blitter|CTS|rs232con|LPT");
  iolist_add_entry(0xfffa0f,"MFP ISRA (8)",1,"mono|rs232 ring|TIMER A|RX buf full|RX error|TX buf empty|TX error|TIMER B");
  iolist_add_entry(0xfffa11,"MFP ISRB (9)",1,"FDC|ACIA|TIMER C|TIMER D|Blitter|CTS|rs232con|LPT");
  iolist_add_entry(0xfffa13,"MFP IMRA (10)",1,"mono|rs232 ring|TIMER A|RX buf full|RX error|TX buf empty|TX error|TIMER B");
  iolist_add_entry(0xfffa15,"MFP IMRB (11)",1,"FDC|ACIA|TIMER C|TIMER D|Blitter|CTS|rs232con|LPT");
  iolist_add_entry(0xfffa17,"MFP VR (12)",1,"V7|V6|V5|V4|S|.|.|.");
  iolist_add_entry(0xfffa19,"MFP TACR (13)",1,"low|extern|subdiv2|subdiv1|subdiv0");
  iolist_add_entry(0xfffa1b,"MFP TBCR (14)",1,"low|extern|subdiv2|subdiv1|subdiv0");
  iolist_add_entry(0xfffa1d,"MFP TCDCR (15)",1,".|Csubdiv2|Csubdiv1|Csubdiv0|.|Dsubdiv2|Dsubdiv1|Dsubdiv0");
  iolist_add_entry(0xfffa1f,"MFP TADR (16)",1);
  iolist_add_entry(0xfffa21,"MFP TBDR (17)",1);
  iolist_add_entry(0xfffa23,"MFP TCDR (18)",1);
  iolist_add_entry(0xfffa25,"MFP TDDR (19)",1);
  iolist_add_entry(0xfffa27,"MFP SCR (20)",1);
  iolist_add_entry(0xfffa29,"MFP UCR (21)",1,"freq/16|wordlen1|wordlen0|startstopbits&sync1|ss&s0|parity on|parity even|.");
  iolist_add_entry(0xfffa2b,"MFP RSR (22)",1,"full|overrun|parity err|frame err|break|match|sync strip|enable");
  iolist_add_entry(0xfffa2d,"MFP TSR (23)",1,"empty|underrun|auto turnaround|end|break|H|L|enable");
  iolist_add_entry(0xfffa2f,"MFP UDR (24)",1);

  iolist_add_entry(0xfffc00,"Keyboard ACIA Control",1,"IRQ|Parity Err|Rx Overrun|Framing Err|CTS|DCD|Tx Data Empty|Rx Data Full");
  iolist_add_entry(0xfffc02,"Keyboard ACIA Data",1);
  iolist_add_entry(0xfffc04,"MIDI ACIA Control",1,"IRQ|Parity Err|Rx Overrun|Framing Err|CTS|DCD|Tx Data Empty|Rx Data Full");
  iolist_add_entry(0xfffc06,"MIDI ACIA Data",1);

//----------------- pseudo addresses ------------------------------
  iolist_debug_add_pseudo_addresses();
}

iolist_entry* search_iolist(MEM_ADDRESS ad)
{
  if ((ad&0xff000000)!=IOLIST_PSEUDO_AD){
    ad&=0xffffff;
  }
  for (int n=iolist_length-1;n>=0;n--){
//    if(!(((iolist[n].ad)^ad)&~1)){ //even parts agree
    if ((iolist[n].ad & -2)==(ad & -2)){ //even parts agree
      if (iolist[n].bytes==2){
        return iolist+n;
      }else if (iolist[n].ad==ad){
        return iolist+n;
      }
    }
    if (iolist[n].ad<ad) return NULL;
  }
  return NULL;
}
//revise calc_wpl

void iolist_box_click(int xc,iolist_entry*il,BYTE*ptr){ //toggle bit clicked
  if(il->bitmask[0]!='#'){
    EasyStr bitdesc;
    int x=0,wid;
    int cells=0;

  //  long mask=1;
    for(int n=strlen(il->bitmask)-1;n>=0;n--){
      if((il->bitmask)[n]=='|'){
        cells++;
      }
    }
    int b=0;
    int nn=0;
    int c1=0,c2=0;
    while(il->bitmask[c2]){
      for(c2=c1;il->bitmask[c2]!='|' && il->bitmask[c2];c2++); //search out next delimeter or end of string
      bitdesc=il->bitmask.Mids(c1,c2-c1);
      wid=get_text_width(bitdesc.Text)+6;
      if(xc>=x && xc<x+wid){
        b=cells-nn;break;
      }
      x+=wid;
      c1=c2+1;
      nn++;
    }
    bool crashy=false;
    WORD mask=WORD(1<<b);
    if(ptr){ //force value
      if(il->bytes==1){ //one byte to edit
        (*ptr)^=LOBYTE(mask);
      }else{
        *((WORD*)ptr)^=mask;
      }
    }else if(!(il->ad&0xff000000)){ //not pseudo-address
      if(il->bytes==1){ //one byte to edit
        crashy=!d2_poke(il->ad,d2_peek(il->ad)^LOBYTE(mask));
      }else{
        crashy=!d2_dpoke(il->ad,d2_dpeek(il->ad)^mask);
      }
    }
    if(crashy)MessageBox(0,"A bus error occurred while trying|to edit this memory.","ттт",0);
  }
}

int iolist_box_width(iolist_entry*il){
  if(il->bitmask.NotEmpty()){
    if(il->bitmask[0]!='#'){ //not special display
      int x=0,wid;
      EasyStr bitdesc;
      long mask=1;
      for(int n=strlen(il->bitmask)-1;n>=0;n--){
        if((il->bitmask)[n]=='|'){
          mask<<=1;
        }
      }
      int c1=0,c2=0;
      while(il->bitmask[c2] && mask){
        for(c2=c1;il->bitmask[c2]!='|' && il->bitmask[c2];c2++); //search out next delimeter or end of string
        bitdesc=il->bitmask.Mids(c1,c2-c1);
        wid=get_text_width(bitdesc.Text)+4;
        mask>>=1;
        x+=wid;
        c1=c2+1;
      }
      return x;
    }else if(il->bitmask=="#COLOUR"){
      return 60;
    }
  }
  return 10;
}

int iolist_box_draw(HDC dc,int x1,int y1,int w,int h,iolist_entry *il,BYTE *ptr)
{
  if (il->bitmask.NotEmpty()){
    RECT rc={x1,y1,x1+w,y1+h+1};
    HRGN Rgn=CreateRectRgnIndirect(&rc);
    SelectObject(dc,Rgn);
    if (il->bitmask[0]!='#'){ //not special display
      HBRUSH bg_br=CreateSolidBrush(GetSysColor(COLOR_WINDOW));
      HBRUSH hi_br=CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));

      COLORREF hi_text_col=GetSysColor(COLOR_HIGHLIGHTTEXT);
      COLORREF std_text_col=GetSysColor(COLOR_WINDOWTEXT);

      HANDLE OldFont=SelectObject(dc,fnt);
      SetBkMode(dc,TRANSPARENT);

      EasyStr bitdesc;
      int x=0,wid;

//      int cells=1;
      long mask=1;
      for(int n=strlen(il->bitmask)-1;n>=0;n--){
        if((il->bitmask)[n]=='|'){
//          cells++;
          mask<<=1;
        }
      }
      long dat=0;
      if (ptr){  //force value
        if (il->bytes==2){
          dat=(long)*((WORD*)ptr);
        }else{
          dat=(long)*((BYTE*)ptr);
        }
      }else if (IS_IOLIST_PSEUDO_ADDRESS(il->ad)){
        if (il->ptr) dat=*((BYTE*)(il->ptr));
      }else{
        if (il->bytes==2){
          dat=d2_dpeek(il->ad);
        }else{
          dat=d2_peek(il->ad);
        }
      }
      int c1=0,c2=0;
      while (il->bitmask[c2] && mask){
        for (c2=c1;il->bitmask[c2]!='|' && il->bitmask[c2];c2++); //search out next delimeter or end of string
        bitdesc=il->bitmask.Mids(c1,c2-c1);
        wid=get_text_width(bitdesc.Text)+4;
        rc.left=x+x1;rc.top=y1;rc.right=x+x1+wid+1;rc.bottom=y1+h+1;
        FillRect(dc,&rc,HBRUSH((dat & mask) ? hi_br:bg_br));
        SetTextColor(dc,COLORREF((dat & mask) ? hi_text_col:std_text_col));
        TextOut(dc,x1+x+2,y1,bitdesc,c2-c1);
        FrameRect(dc,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));
        mask>>=1;
        x+=wid;
        c1=c2+1;
      }
      SelectObject(dc,OldFont);
      SelectClipRgn(dc,NULL);

      DeleteObject(hi_br);
      DeleteObject(bg_br);
      DeleteObject(Rgn);
      return x;
    //  EndPaint(Win,&ps);
    }else if(il->bitmask=="#COLOUR"){
      rc.right=x1+60;

      WORD dat;

      if(ptr){  //force value
        dat=*((WORD*)ptr);
      }else if(!(il->ad&0xff000000)){ //not pseudo-address
        dat=d2_dpeek(il->ad);
      }else{
        dat=0;
      }

//      STE colour to 24-bit RGB colour
//      700 -> e0 0000, 800 -> 10 0000
//      070 -> 00 e000, 080 -> 00 1000
//      007 -> 00 00e0, 008 -> 00 0010

/*    DWORD rgb=((dat&0x700)<<13) | ((dat&0x800)<<9) //red
              | ((dat&0x070)<<9)  | ((dat&0x080)<<5) //green
              | ((dat&0x007)<<5)  | ((dat&0x008)<<1); //blue

*/
      dat=WORD( ((dat&0x888)>>3) | ((dat&0x777)<<1) );  //fix up stupid rRRRgGGGbBBB colour pattern

      HBRUSH cl=CreateSolidBrush(RGB((dat&0xf00)>>4,(dat&0x0f0),(dat&0xf)<<4));
      FillRect(dc,&rc,cl);
      FrameRect(dc,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));

      SelectClipRgn(dc,NULL);

      DeleteObject(cl);
      DeleteObject(Rgn);
      return 60;
    }
  }
  return 10;
}



