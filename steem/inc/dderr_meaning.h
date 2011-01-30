//USERC("DDERR_Strings.RC");
//---------------------------------------------------------------------------
unsigned short UNMAKE_DDHRESULT(long code)
{
  long FACDD=0x876;
  return (unsigned short)((FACDD >> 16)^code);
}
//---------------------------------------------------------------------------
//If the function succeeds, the return value is the number of bytes
//copied into the buffer, not including the null-terminating character,
//or zero if the error does not exist.
int DDGetErrorDescription(HRESULT Error,char *buf,int size)
{
  return LoadString(HInstance,UNMAKE_DDHRESULT(Error),buf,size);
}
//---------------------------------------------------------------------------
