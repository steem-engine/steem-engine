/*---------------------------------------------------------------------------
FILE: stjoy_directinput.cpp
MODULE: Steem
DESCRIPTION: Code to read PC joysticks using DirectInput.
---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
void DIFreeJoysticks()
{
  for (int j=0;j<MAX_PC_JOYS;j++){
    if (DIJoy[j]){
      DIJoy[j]->Unacquire();
      DIJoy[j]->Release();
      DIJoy[j]=NULL;
    }
    if (DIJoy2[j]){
      DIJoy2[j]->Release();
      DIJoy2[j]=NULL;
    }
  }
  if (DIObj){
    DIObj->Release();
    DIObj=NULL;
  }
}
//---------------------------------------------------------------------------
BOOL CALLBACK DIEnumJoysticksCallback(LPCDIDEVICEINSTANCE pInst,LPVOID)
{
  HRESULT hr;

  hr=DIObj->CreateDevice(pInst->guidInstance,&DIJoy[NumJoysticks],NULL);
  if (FAILED(hr)) return DIENUM_CONTINUE;

  // Get IDirectInputDevice2 - we need this to poll the joystick
  DIJoy[NumJoysticks]->QueryInterface(IID_IDirectInputDevice2,(LPVOID*)&DIJoy2[NumJoysticks]);

  if ((++NumJoysticks)>=MAX_PC_JOYS) return DIENUM_STOP;

  return DIENUM_CONTINUE;
}
//---------------------------------------------------------------------------
void DIGetAxisInfo(int j,int offset,int nAxis)
{
  DIPROPRANGE diprg;
  HRESULT hRet;

  diprg.diph.dwSize=sizeof(DIPROPRANGE);
  diprg.diph.dwHeaderSize=sizeof(DIPROPHEADER);
  diprg.diph.dwHow=DIPH_BYOFFSET;
  diprg.diph.dwObj=offset;
  diprg.lMin=0;
  diprg.lMax=65535;
  DIJoy[j]->SetProperty(DIPROP_RANGE,&diprg.diph);

  hRet=DIJoy[j]->GetProperty(DIPROP_RANGE,&diprg.diph);
  if (hRet==DI_OK){
    JoyInfo[j].AxisExists[nAxis]=true;
    
    if (diprg.lMin>diprg.lMax){
      UINT temp=diprg.lMax;
      diprg.lMax=diprg.lMin;
      diprg.lMin=temp;
    }
    if (diprg.lMin<0){
      DIAxisNeg[j][nAxis]=(-diprg.lMin);
      diprg.lMax+=(-diprg.lMin);
      diprg.lMin=0;
    }else{
      DIAxisNeg[j][nAxis]=0;
    }
    JoyInfo[j].AxisMin[nAxis]=(UINT)(diprg.lMin);
    JoyInfo[j].AxisMax[nAxis]=(UINT)(diprg.lMax);
    JoyInfo[j].AxisMid[nAxis]=(JoyInfo[j].AxisMax[nAxis]+JoyInfo[j].AxisMin[nAxis])/2;
    JoyInfo[j].AxisLen[nAxis]=JoyInfo[j].AxisMax[nAxis]-JoyInfo[j].AxisMin[nAxis];
  }
}
//---------------------------------------------------------------------------
SET_GUID(CLSID_DirectInput,      0x25E609E0,0xB259,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
SET_GUID(IID_IDirectInputA,     0x89521360,0xAA8A,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
SET_GUID(IID_IDirectInputDevice2A,0x5944E682,0xC92E,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
SET_GUID(GUID_XAxis,   0xA36D02E0,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
SET_GUID(GUID_YAxis,   0xA36D02E1,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
SET_GUID(GUID_ZAxis,   0xA36D02E2,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
SET_GUID(GUID_RxAxis,  0xA36D02F4,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
SET_GUID(GUID_RyAxis,  0xA36D02F5,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
SET_GUID(GUID_RzAxis,  0xA36D02E3,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
SET_GUID(GUID_Slider,  0xA36D02E4,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
SET_GUID(GUID_Button,  0xA36D02F0,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
SET_GUID(GUID_POV,     0xA36D02F2,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);

#define DIFT_ALLOWNOTEXIST 0x80000000
#define DIJOY_MAX_DATAOBJECT 43
DIOBJECTDATAFORMAT DIJoyDataFormatObjects[DIJOY_MAX_DATAOBJECT+1];
DIDATAFORMAT DIJoyDataFormat={sizeof(DIDATAFORMAT),sizeof(DIOBJECTDATAFORMAT),DIDF_ABSAXIS,
                              sizeof(DIJOYSTATE),0,DIJoyDataFormatObjects};

void DIInitJoysticks()
{
  HRESULT hr;

  hr=CoCreateInstance(CLSID_DirectInput,NULL,CLSCTX_ALL,IID_IDirectInput,(void**)&DIObj);
  if (FAILED(hr)) return;

  hr=DIObj->Initialize(GetModuleHandle(NULL),DIRECTINPUT_VERSION);
  if (FAILED(hr)){
    DIFreeJoysticks();return;
  }

  hr=DIObj->EnumDevices(DIDEVTYPE_JOYSTICK,DIEnumJoysticksCallback,NULL,DIEDFL_ALLDEVICES);
  if (FAILED(hr) || NumJoysticks==0){
    DIFreeJoysticks();return;
  }

  // Set up data format
  const GUID* AxisList[]={&GUID_XAxis,&GUID_YAxis,&GUID_ZAxis,&GUID_RxAxis,&GUID_RyAxis,
                    &GUID_RzAxis,&GUID_Slider,&GUID_Slider,NULL};
  int Offset=FIELD_OFFSET(DIJOYSTATE,lX);
  int i=0;
  while (AxisList[i]){
    DIJoyDataFormatObjects[i].pguid=AxisList[i];
    DIJoyDataFormatObjects[i].dwOfs=Offset;
    DIJoyDataFormatObjects[i].dwType=DIDFT_AXIS | DIFT_ALLOWNOTEXIST | DIDFT_ANYINSTANCE;
    DIJoyDataFormatObjects[i].dwFlags=DIDOI_ASPECTPOSITION;
    Offset+=sizeof(LONG);
    i++;
  }
  for (int n=0;n<4;n++){
    DIJoyDataFormatObjects[i].pguid=&GUID_POV;
    DIJoyDataFormatObjects[i].dwOfs=FIELD_OFFSET(DIJOYSTATE,rgdwPOV[n]);
    DIJoyDataFormatObjects[i].dwType=DIDFT_POV | DIFT_ALLOWNOTEXIST | DIDFT_ANYINSTANCE;
    DIJoyDataFormatObjects[i].dwFlags=0;
    i++;
  }
  for (int n=0;n<32;n++){
    DIJoyDataFormatObjects[i].pguid=NULL;
    DIJoyDataFormatObjects[i].dwOfs=FIELD_OFFSET(DIJOYSTATE,rgbButtons[n]);
    DIJoyDataFormatObjects[i].dwType=DIDFT_BUTTON | DIFT_ALLOWNOTEXIST | DIDFT_ANYINSTANCE;
    DIJoyDataFormatObjects[i].dwFlags=0;
    i++;
    if (i>DIJOY_MAX_DATAOBJECT) break;
  }
  DIJoyDataFormat.dwNumObjs=i;

  for (int j=0;j<MAX_PC_JOYS;j++){
    if (DIJoy[j]){
      JoyExists[j]=true;
      DIJoy[j]->SetDataFormat(&DIJoyDataFormat);
      DIJoy[j]->SetCooperativeLevel(StemWin,DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

      // Set device wide options
      DIPROPDWORD dip={{sizeof(DIPROPDWORD),sizeof(DIPROPHEADER),0,DIPH_DEVICE},0};

      dip.dwData=DIPROPAXISMODE_ABS;
      DIJoy[j]->SetProperty(DIPROP_AXISMODE,&dip.diph);
      dip.dwData=0;
      DIJoy[j]->SetProperty(DIPROP_DEADZONE,&dip.diph);
      dip.dwData=10000;
      DIJoy[j]->SetProperty(DIPROP_SATURATION,&dip.diph);

      for (int a=0;a<7;a++) JoyInfo[j].AxisExists[a]=0;

      DIGetAxisInfo(j,DIJOFS_X,AXIS_X);
      DIGetAxisInfo(j,DIJOFS_Y,AXIS_Y);
      DIGetAxisInfo(j,DIJOFS_Z,AXIS_Z);
      DIGetAxisInfo(j,DIJOFS_SLIDER(0),AXIS_U);
      DIGetAxisInfo(j,DIJOFS_SLIDER(1),AXIS_V);

      int rofs[3]={DIJOFS_RZ,DIJOFS_RX,DIJOFS_RY};
      DI_RnMap[j][0]=-1, DI_RnMap[j][1]=-1, DI_RnMap[j][2]=-1;
      for (int i=0;i<3;i++){
        int axnum=-1;
        for (int a=0;a<AXIS_POV;a++){
          if (JoyInfo[j].AxisExists[a]==0){
            axnum=a;
            break;
          }
        }
        if (axnum<0) break;

        DIGetAxisInfo(j,rofs[i],axnum);
        if (JoyInfo[j].AxisExists[axnum]) DI_RnMap[j][i]=axnum;
      }

      DIPOVNum=-1;
      for (int i=0;i<4;i++){
        DIDEVICEOBJECTINSTANCE didoi;
        didoi.dwSize=sizeof(DIDEVICEOBJECTINSTANCE);
        if (DIJoy[j]->GetObjectInfo(&didoi,DIJOFS_POV(i),DIPH_BYOFFSET)==DI_OK){
          DIPOVNum=i;
          JoyInfo[j].AxisExists[AXIS_POV]=true;
          break;
        }
      }

      // This is just in case getting positions fails silently somehow.
      DIJoyPos[j].lX=JoyInfo[j].AxisMid[AXIS_X];
      DIJoyPos[j].lY=JoyInfo[j].AxisMid[AXIS_Y];
      DIJoyPos[j].lZ=JoyInfo[j].AxisMid[AXIS_Z];
      DIJoyPos[j].rglSlider[0]=JoyInfo[j].AxisMid[AXIS_U];
      DIJoyPos[j].rglSlider[1]=JoyInfo[j].AxisMid[AXIS_V];
      for (int p=0;p<4;p++) DIJoyPos[j].rgdwPOV[p]=0xffffffff;

      if (DI_RnMap[j][0]>=0) DIJoyPos[j].lRz=JoyInfo[j].AxisMid[DI_RnMap[j][0]];
      if (DI_RnMap[j][1]>=0) DIJoyPos[j].lRx=JoyInfo[j].AxisMid[DI_RnMap[j][1]];
      if (DI_RnMap[j][2]>=0) DIJoyPos[j].lRy=JoyInfo[j].AxisMid[DI_RnMap[j][2]];

      JoyInfo[j].NumButtons=31;
      ZeroMemory(DIJoyPos[j].rgbButtons,sizeof(DIJoyPos[j].rgbButtons));

      DIDisconnected[j]=50;

      DIJoy[j]->Acquire();
    }
  }
}
//---------------------------------------------------------------------------
void DIJoyGetPos(int j)
{
  if (DIJoy[j]){
    HRESULT Ret;

    if (DIDisconnected[j]>0){
      DIDisconnected[j]--;

      DIDEVCAPS didc;
      didc.dwSize=sizeof(DIDEVCAPS);
      DIJoy[j]->GetCapabilities(&didc);  // This doesn't update until you Poll again
      if (didc.dwFlags & DIDC_ATTACHED) DIDisconnected[j]=0;
    }
    if (DIDisconnected[j]>0){
      JoyPosReset(j); // Nothing stuck on
      return;
    }

    for (int i=0;i<2;i++){
      if (DIJoy2[j]) DIJoy2[j]->Poll();
      Ret=DIJoy[j]->GetDeviceState(sizeof(DIJOYSTATE),&DIJoyPos[j]);
      if ((Ret==DIERR_INPUTLOST || Ret==DIERR_NOTACQUIRED) && i==0){
        DIJoy[j]->Acquire();
      }else{
        if (Ret!=DI_OK) DIDisconnected[j]=int((runstate==RUNSTATE_RUNNING) ? 200:100);
        break;
      }
    }

    JoyPos[j].dwXpos=DWORD(DIJoyPos[j].lX+DIAxisNeg[j][AXIS_X]);
    JoyPos[j].dwYpos=DWORD(DIJoyPos[j].lY+DIAxisNeg[j][AXIS_Y]);
    JoyPos[j].dwZpos=DWORD(DIJoyPos[j].lZ+DIAxisNeg[j][AXIS_Z]);
    JoyPos[j].dwUpos=DWORD(DIJoyPos[j].rglSlider[0]+DIAxisNeg[j][AXIS_U]);
    JoyPos[j].dwVpos=DWORD(DIJoyPos[j].rglSlider[1]+DIAxisNeg[j][AXIS_V]);

    long *rn_val[3]={&(DIJoyPos[j].lRz),&(DIJoyPos[j].lRx),&(DIJoyPos[j].lRy)};
    DWORD *axis_val[6]={&(JoyPos[j].dwXpos),&(JoyPos[j].dwYpos),&(JoyPos[j].dwZpos),
                          &(JoyPos[j].dwRpos),&(JoyPos[j].dwUpos),&(JoyPos[j].dwVpos)};
    for (int i=0;i<3;i++){
      int a=DI_RnMap[j][i];
      if (a>=0) *(axis_val[a])=DWORD(*(rn_val[i]) + DIAxisNeg[j][a]);
    }

    if (DIPOVNum>-1){
      JoyPos[j].dwPOV=DIJoyPos[j].rgdwPOV[DIPOVNum];
      if (JoyPos[j].dwPOV<0xffff) JoyPos[j].dwPOV%=32000;
    }else{
      JoyPos[j].dwPOV=0xffffffff;
    }

    JoyPos[j].dwButtons=0;
    int Bit=1;
    for (int n=0;n<31;n++){
      if (DIJoyPos[j].rgbButtons[n]) JoyPos[j].dwButtons|=Bit;
      Bit<<=1;
    }
  }
}
//---------------------------------------------------------------------------

