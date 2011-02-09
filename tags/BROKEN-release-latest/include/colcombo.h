#ifndef MAX_CC_ITEMS
  #define MAX_CC_ITEMS 255
#endif
#ifndef MAX_CC_TEXTLENGTH
  #define MAX_CC_TEXTLENGTH 250
#endif
typedef struct {
	char Text[MAX_CC_TEXTLENGTH];
  int Col;
}CC_Info;
//---------------------------------------------------------------------------
class TColCombo
{
private:
	void DrawItem(DRAWITEMSTRUCT*);
	//private variables
	HANDLE Font;int FontHeight;

#if (defined USING_BUILDER)
  CC_Info* GetItem(int idx){return VItem[idx];}

  void SetVar(int,int),SetVisible(bool);

	//real variables of the properties
  CC_Info *VItem[MAX_CC_ITEMS];
  int VLeft,VTop,VWidth,VHeight,VBoxGap,VBoxSurround,VBoxWidth,VNumItems,VId,
  		VItemHeight,VSelItemHeight;
	bool VVisible;
  HANDLE VParent,VHandle;
#endif
public:
	TColCombo(HWND,bool,HFONT);
  ~TColCombo();
  bool AddItem(char *,int),DeleteItem(int),InsertItem(char*,int,int);
  bool IsFocused(){if (VHandle==0)return 0; return GetFocus()==VHandle;}
  bool SetFocus(){if (VHandle==0)return 0; return ::SetFocus(VHandle);}
  void SetBounds(int,int,int,int),Update(),Clear(),
  		 HandleDrawItem(UINT,long),BcbWMDrawItem(TMessage);
  int GetIndexFromInfo(char*,int,bool);
  int GetSelIndex(){if (VHandle==NULL)return -1;return SendMessage(VHandle,CB_GETCURSEL,0,0);}
  void SetSelIndex(int tosel){if (VHandle==NULL)return;SendMessage(VHandle,CB_SETCURSEL,tosel,0);}

#ifndef USING_BUILDER
	union{
		struct{
	    CC_Info *VItem[MAX_CC_ITEMS];
		  int VLeft,VTop,VWidth,VHeight,VBoxGap,VBoxSurround,VBoxWidth,VNumItems,VId,
		  		VItemHeight,VSelItemHeight;
			bool VVisible;
		  HANDLE VParent,VHandle;
    };
    struct{
	    CC_Info *Item[MAX_CC_ITEMS];
		  int Left,Top,Width,Height,BoxGap,BoxSurround,BoxWidth,NumItems,Id,
		  		ItemHeight,SelItemHeight;
			bool Visible;
		  HANDLE Parent,Handle;
    };
  };
  int SelIndex,ItemIndex;

#else  //YOU ARE USING BUILDER
  __property CC_Info* Item[int]={read=GetItem};

  __property int Left={read=VLeft,write=SetVar,index=1};
  __property int Top={read=VTop,write=SetVar,index=2};
  __property int Width={read=VWidth,write=SetVar,index=3};
  __property int Height={read=VHeight,write=SetVar,index=4};

  __property bool Visible={read=VVisible,write=SetVisible};

  __property int BoxGap={read=VBoxGap,write=SetVar,index=5};
  __property int BoxSurround={read=VBoxSurround,write=SetVar,index=6};
  __property int BoxWidth={read=VBoxWidth,write=SetVar,index=7};
  __property int NumItems={read=VNumItems};
  __property int ItemHeight={read=VItemHeight,write=SetVar,index=10};
  __property int SelItemHeight={read=VSelItemHeight,write=SetVar,index=11};
  __property int SelIndex={read=GetSelIndex,write=SetSelIndex};
  __property int ItemIndex={read=GetSelIndex,write=SetSelIndex};
  __property int Focused={read=IsFocused};
  __property int HasFocus={read=IsFocused};

  __property int Id={read=VId};
  __property HANDLE Handle={read=VHandle};
  __property HANDLE Parent={read=VParent};
#endif
};
//---------------------------------------------------------------------------

#if (defined USING_BUILDER)
void TColCombo::SetVar(int idx,int value)
{
	bool changed=0;
  switch (idx){
  case 1:if (VLeft!=value){VLeft=value;changed=true;} break;
  case 2:if (VTop!=value){VTop=value;changed=true;} break;
  case 3:if (VWidth!=value){VWidth=value;changed=true;} break;
  case 4:if (VHeight!=value){VHeight=value;changed=true;} break;
  case 5:if (VBoxGap!=value){VBoxGap=value;changed=true;} break;
  case 6:if (VBoxSurround!=value){VBoxSurround=value;changed=true;} break;
  case 7:if (VBoxWidth!=value){VBoxWidth=value;changed=true;} break;
  case 10:
		value=max(value,FontHeight);
  	if (VItemHeight!=value){
    	VItemHeight=value;
	  	SendMessage(Handle,CB_SETITEMHEIGHT,0,value); idx=7;
    }
    break;
  case 11:
		value=max(value,FontHeight);
  	if (VSelItemHeight!=value){
    	VSelItemHeight=value;
			SendMessage(Handle,CB_SETITEMHEIGHT,-1,value); idx=7;
    }
    break;
	}
	if (Handle!=NULL && changed){
	  if (idx<=4)
			MoveWindow(Handle,VLeft,VTop,VWidth,VHeight,VVisible);
	  else if (idx<=7)
	    InvalidateRect(Handle,0,0);
  }
}
//---------------------------------------------------------------------------
void TColCombo::SetVisible(bool value)
{
	VVisible=value;
	if (Handle==NULL)return;
 	if (IsWindowVisible(VHandle)==VVisible) return;
 	if (VVisible) ShowWindow(VHandle,SW_SHOWNA);
  else ShowWindow(VHandle,SW_HIDE);
}
#endif

