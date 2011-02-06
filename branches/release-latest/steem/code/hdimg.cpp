//---------------------------------------------------------------------------
void hdimg_init_vectors()
{
  os_hdimg_init_vector=LPEEK(0x46a);
  os_hdimg_bpb_vector=LPEEK(0x472);
  os_hdimg_rw_vector=LPEEK(0x476);
  os_hdimg_boot_vector=LPEEK(0x47a);
  os_hdimg_mediach_vector=LPEEK(0x47e);
}
//---------------------------------------------------------------------------
void hdimg_reset()
{
  os_hdimg_init_vector=0;
  os_hdimg_bpb_vector=0;
  os_hdimg_rw_vector=0;
  os_hdimg_boot_vector=0;
  os_hdimg_mediach_vector=0;
}
//---------------------------------------------------------------------------
void hdimg_intercept(MEM_ADDRESS pc)
{
  if (pc==os_hdimg_init_vector){
    hdimg_intercept_init();
  }else if (pc==os_hdimg_bpb_vector){
  }else if (pc==os_hdimg_rw_vector){
  }else if (pc==os_hdimg_boot_vector){
  }else if (pc==os_hdimg_mediach_vector){
  }
}
//---------------------------------------------------------------------------
void hdimg_intercept_init()
{
}
//---------------------------------------------------------------------------

