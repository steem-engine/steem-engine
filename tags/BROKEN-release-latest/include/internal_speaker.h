#define SPEAKER_PORT  0x61
#define PIT_CONTROL   0x43
#define PIT_CHANNEL_2 0x42
#define PIT_FREQ      0x1234DD

#ifndef ASMCALL
#define ASMCALL
#define __UNDEF_ASMCALL
#endif

extern "C"{
  void ASMCALL port_out(int,int);
  long ASMCALL port_in(int);
}

#ifdef __UNDEF_ASMCALL
#undef ASMCALL
#undef __UNDEF_ASMCALL
#endif

void internal_speaker_sound(int frequency){

  if(frequency){

  //  { Program the PIT chip }
    int counter = PIT_FREQ / frequency;

    port_out(PIT_CONTROL,0xB6);
    port_out(PIT_CHANNEL_2,LOBYTE(counter));
    port_out(PIT_CHANNEL_2,HIBYTE(counter));

    BYTE sp=(BYTE)port_in(SPEAKER_PORT);
    if((sp&3)!=3)port_out(SPEAKER_PORT,sp|3);
  }else{
    BYTE sp=(BYTE)port_in(SPEAKER_PORT);
    port_out(SPEAKER_PORT,sp&0xfc);
  }
}

void internal_speaker_sound_by_period(int counter){

  if(counter){

  //  { Program the PIT chip }
    port_out(PIT_CONTROL,0xB6);
    port_out(PIT_CHANNEL_2,LOBYTE(counter));
    port_out(PIT_CHANNEL_2,HIBYTE(counter));

    BYTE sp=(BYTE)port_in(SPEAKER_PORT);
    if((sp&3)!=3)port_out(SPEAKER_PORT,sp|3);
  }else{
    BYTE sp=(BYTE)port_in(SPEAKER_PORT);
    port_out(SPEAKER_PORT,sp&0xfc);
  }
}

