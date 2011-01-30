//---------------------------------------------------------------------------
int TPortIO::Open(char *PortDev,bool AllowOutput,bool AllowInput,int PortType)
{
	Close();

  if (AllowOutput==0 && AllowInput==0) return 1;

	int flags=O_NOCTTY; // | O_NONBLOCK;
  if ((AllowInput && AllowOutput) || PortType==TPORTIO_TYPE_PIPE){
		flags|=O_RDWR;
  }else	if (AllowInput==0){
		flags|=O_WRONLY;
	}else{
		flags|=O_RDONLY;
	}
	iCom=open(PortDev,flags);
  if (iCom==-1) return 1;

  Type=PortType;
  InPause=true;
  OutPause=true;

	SetupCOM(115200,0,RTS_CONTROL_DISABLE,DTR_CONTROL_DISABLE,0,0,ONESTOPBIT,8);

  int status;
  if (AllowInput){
    if (InpBuf.Create(TPORTIO_BUF_SIZE)==0){
      Close();
      return 1;
    }
  	status=pthread_create(&iInThread,NULL,InThreadFunc,this);
  	if (status==-1){
  		Close();
  		return 1;
  	}
 	}
 	if (AllowOutput){
	  if (OutBuf.Create(TPORTIO_BUF_SIZE)==0){
	    Close();
	    return 1;
	  }

		pthread_cond_init(&OutWaitCond,NULL);
		pthread_mutex_init(&OutWaitMutex,NULL);
  	status=pthread_create(&iOutThread,NULL,OutThreadFunc,this);
  	if (status==-1){
  		Close();
  		return 1;
  	}
/*
#ifdef PRIO_MAX
  	struct sched_param sp;
  	sp.sched_priority=PRIO_MAX;
		pthread_setschedparam(iOutThread,SCHED_RR,&sp);
#endif
*/
	}

  return 0;
}
//---------------------------------------------------------------------------
void* TPortIO::InThreadFunc(void *t)
{
  TPortIO *This=(TPortIO*)t;

  This->InThreadClosed=0;
  BYTE TempIn;

  fd_set InputPendSet;
	struct timeval Timeout;
  while (This->Closing==0){
    if (This->InPause){
    	Sleep(20);
    }else{
		  FD_ZERO(&InputPendSet);
	  	FD_SET(This->iCom,&InputPendSet);
			Timeout.tv_sec=0;
			Timeout.tv_usec=200*1000; // 1000=1ms
    	if (select(This->iCom+1,&InputPendSet,NULL,NULL,&Timeout)>0){
      	if (FD_ISSET(This->iCom,&InputPendSet)){
          read(This->iCom,&TempIn,1);
          bool FirstByte=(This->InpBuf.AreBytesInBuffer()==0);
          This->InpBuf.AddByte(TempIn);
          if (FirstByte){
            if (This->lpInFirstByteProc) This->lpInFirstByteProc();
          }
          This->InCount++;
        }
      }
    }
  }
  This->InThreadClosed=true;
  return NULL;
}
//---------------------------------------------------------------------------
void* TPortIO::OutThreadFunc(void *t)
{
  TPortIO *This=(TPortIO*)t;

  This->OutThreadClosed=0;
  BYTE TempOut;
  while (This->Closing==0){
  	if (This->OutPause){
  		Sleep(20);
  	}else{
	    if (This->Outputting){
        TempOut=This->OutBuf.ReadByte();
        write(This->iCom,&TempOut,1);
        if (This->OutBuf.AreBytesInBuffer()){
          This->OutBuf.NextByte();
        }else{
          This->Outputting=0;
  	      if (This->lpOutFinishedProc) This->lpOutFinishedProc();
        }
        This->OutCount++;
      }else{
	      // Wait for OutWaitCond to be signalled (when closing
	      // or Outputting is set)
      	pthread_cond_wait(&(This->OutWaitCond),&(This->OutWaitMutex));
      }
    }
  }
  This->OutThreadClosed=true;
  return 0;
}
//---------------------------------------------------------------------------
bool TPortIO::StartBreak()
{
	if (iCom==-1) return 0;
	if (Type!=TPORTIO_TYPE_SERIAL) return 0;

#if defined(TIOCSBRK) && defined(TIOCCBRK)
	ioctl(iCom,TIOCSBRK,0); // start break
#endif

  return true;
}
//---------------------------------------------------------------------------
bool TPortIO::EndBreak()
{
	if (iCom==-1) return 0;
	if (Type!=TPORTIO_TYPE_SERIAL) return 0;

#if defined(TIOCSBRK) && defined(TIOCCBRK)
	ioctl(iCom,TIOCCBRK,0); // end break
#endif

  return true;
}
//---------------------------------------------------------------------------
void TPortIO::SetupCOM(int BaudRate,bool bXOn_XOff,int RTS,int DTR,bool bParity,BYTE ParityType,BYTE StopBits,BYTE WordLength)
{
	if (iCom==-1) return;
	if (Type!=TPORTIO_TYPE_SERIAL) return;

  struct termios options;
  tcgetattr(iCom,&options); // get the current options

  options.c_cflag     |= (CLOCAL | CREAD | HUPCL);
  options.c_cflag     &= ~(PARENB | CRTSCTS | CSTOPB | PARODD);
#ifdef EXTA
  options.c_cflag&=~EXTA;
#endif
#ifdef EXTB
  options.c_cflag&=~EXTB;
#endif
  options.c_lflag     &= ~(ISIG | ICANON | ECHO | ECHOE |
  													ECHOK | ECHONL | NOFLSH | ECHOCTL |
  													ECHOKE | TOSTOP);
#ifdef XCASE
  options.c_lflag&=~XCASE;
#endif
#ifdef ECHOPRT
  options.c_lflag&=~ECHOPRT;
#endif
#ifdef PENDIN
  options.c_lflag&=~PENDIN;
#endif
  options.c_iflag     &= ~(INPCK | PARMRK | IGNPAR | ISTRIP | IXON |
  													IXOFF | IXANY | IGNBRK | BRKINT | INLCR |
  													ICRNL | IUCLC | IMAXBEL);
  options.c_oflag     &= ~(OPOST | OLCUC | ONLCR | OCRNL |
														ONLRET | OFILL | OFDEL | NLDLY |
														NL0 | NL1 | CRDLY | CR0 | CR1 | CR2 |
														CR3 | TABDLY | TAB0 | TAB1 | TAB2 | TAB3 |
														BSDLY | BS0 | BS1 | VT0 | VT1 |
														FFDLY | FF0 | FF1);
	options.c_cc[VMIN]  = 0;
  options.c_cc[VTIME] = 1;	// 1/10th of a second before timeout

	int bauds[]={B50,50,B75,75,B110,110,B134,134,
								B150,150,B200,200,B300,300,B600,600,B1200,1200,
								B1800,1800,B2400,2400,B4800,4800,B9600,9600,
								B19200,19200,B38400,38400,B57600,57600,
								B115200,115200,-1,-1};
	int *bauds_p=bauds;
	int diff=999999;									
	int closest=B0;
	while (bauds_p[1]>=0){
		if (abs(bauds_p[1]-BaudRate)<diff){
			diff=abs(bauds_p[1]-BaudRate);
			closest=bauds_p[0];
		}
		bauds_p+=2;
	}
	cfsetispeed(&options,closest);
	cfsetospeed(&options,closest);

  if (bXOn_XOff) options.c_iflag|=(IXON | IXOFF);
  if (RTS) options.c_cflag|=CRTSCTS;
  if (bParity){
		// Set modem parity check bit
	  options.c_cflag|=PARENB;
	  // Mark parity errors, check parity on input
		options.c_lflag|=(PARMRK | INPCK | ISTRIP);
	}
	if (ParityType==ODDPARITY) options.c_cflag|=PARODD;
	if (StopBits==TWOSTOPBITS) options.c_cflag|=CSTOPB;

	options.c_cflag &= ~CSIZE;
	switch (WordLength){
		case 5: options.c_cflag |= CS5; break;
		case 6: options.c_cflag |= CS6; break;
		case 7: options.c_cflag |= CS7; break;
		default: options.c_cflag |= CS8;
	}

  // Set the options when current input/output buffers are empty
  tcsetattr(iCom,TCSAFLUSH,&options);
}
//---------------------------------------------------------------------------
DWORD TPortIO::GetModemFlags()
{
	if (iCom==-1) return 0;
	if (Type!=TPORTIO_TYPE_SERIAL) return MS_CTS_ON;

	DWORD Ret=MS_CTS_ON;
#ifdef TIOCMGET
	int status;
  if (ioctl(iCom,TIOCMGET,&status)!=-1){
//	  if (status & TIOCM_DTR) Ret|=MS_DTR_ON;
//	  if (status & TIOCM_RTS) Ret|=MS_RTS_ON;
    if (status & TIOCM_CTS) Ret|=MS_CTS_ON;
    if (status & TIOCM_CAR) Ret|=MS_RLSD_ON; // DCD
    if (status & TIOCM_RI) Ret|=MS_RING_ON;
    if (status & TIOCM_DSR) Ret|=MS_DSR_ON;
  }
#endif

  return Ret;
}
//---------------------------------------------------------------------------
bool TPortIO::SetDTR(bool Val)
{
	if (iCom==-1) return 0;
	if (Type!=TPORTIO_TYPE_SERIAL) return 0;

#ifdef TIOCMGET
	int status;
	ioctl(iCom,TIOCMGET,&status);
	status|=TIOCM_DTR;
	ioctl(iCom,TIOCMSET,status);
  return true;
#else
  return 0;
#endif
}
//---------------------------------------------------------------------------
bool TPortIO::SetRTS(bool Val)
{
	if (iCom==-1) return 0;
	if (Type!=TPORTIO_TYPE_SERIAL) return 0;

#ifdef TIOCMGET
	int status;
	ioctl(iCom,TIOCMGET,&status);
	status|=TIOCM_RTS;
	ioctl(iCom,TIOCMSET,status);
  return true;
#else
  return 0;
#endif
}
//---------------------------------------------------------------------------

