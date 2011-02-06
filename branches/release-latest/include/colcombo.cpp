#define USING_BUILDER             DELETE THESE LINES IF YOU ARE NOT\
																	USING BORLAND'S C++ BUILDER!

#include "ColCombo.h"

#define BCB_CC_SETUPMESSAGEMAP   \
	void __fastcall CC_HandleMessage(TMessage Message);\
BEGIN_MESSAGE_MAP\
  MESSAGE_HANDLER(WM_DRAWITEM, TMessage, CC_HandleMessage)\
END_MESSAGE_MAP(TForm)

/*
How to use:
	Create an instance of TColCombo.
  Add the entries using AddItem.
  Use SetBounds to position the box and set Visible to true
  	when you want it to display.
  In your WndProc put a call to HandleDrawItem when you recieve
	  a WM_DRAWITEM message.
  Remember to call delete after the parent window has closed

  Fiddle with the variables until it looks just right.

Additional for BCB RAD:
  In the parent Form's header type "BCB_CC_SETUPMESSAGEMAP"
  just above the }; for the Form's class declaration

  Now put this procedure in the cpp of the parent Form
  replacing the things in []s:

	void __fastcall T[FORM NAME]::CC_HandleMessage(TMessage Message)
	{
    if ([THE COLOUR COMBOBOX HAS BEEN CREATED])
     	[POINTER TO THE COLOUR COMBOBOX]->BcbWMDrawItem(Message);

		//Do the same for each box

	  TForm::Dispatch(&Message);
	}

-----------------------------------------------------------------------
  Methods (Functions):

  TColCombo(HANDLE Parent_Window,bool Make_Tabstop,HFONT Font)

  Notes:  If Font is NULL, the parent window's font will be used. The
          font must be deleted by you after Free has been called (unless.
          NULL was passed as Font). Handle=0 if the initialization fails.


  HandleDrawItem(UINT wParam,long lParam)

  Notes:  Put in the WndProc of the parent window in response to
          WM_DRAWITEM messages.


  AddItem(char *Text,int RGB_Colour)

  Notes:  Adds an item to the list.


	DeleteItem(int Index)

  Notes:  Deletes the item at position Index.


  InsertItem(char *Text,int RGB_Colour,int Index)

  Notes:  Inserts an item at position Index.


  Clear()

  Notes:  Deletes all items.


  GetIndexFromInfo(char *Text,int RGB_Colour,bool Match_Case)

  Notes:  Returns the index of the item with it's Text the same as Text
	        and it's Col the same as RGB_Colour. If none match it returns
          -1. If RGB_Colour is -1 then the function will just match the
          text.


  SetBounds(int Left,int Top, int Width,int Height)

  Notes:  Positions the Combobox, Height is the maximum height of
  				the drop down list, the actual height of the box is related
          to the SelItemHeight property. Doesn't show the box if it's
          hidden.


  Update()

  Notes:  Redraws the box and Shows it if it should. When not using
          Borland's C++ Builder you should call this whenever you
          change a variable that effects the appearance of the Combobox.


  IsFocused()

  Notes:  Returns whether the Combobox has the focus.


  GetSelIndex(),   SetSelIndex()

  Notes:  Gets and Sets the selected item respectively.

-----------------------------------------------------------------------
	Properties (Members):  (> = Read Only, (B) = Borland's C++ Builder Only)

  Visible -  Visible is always initially false so the Combobox can be
             positioned and filled with items.
  >Item[] -  The list of RGB values (->Col) and text (->Text) that
  					 appear in the combobox.
  Left,Top,Width,Height - Height is the maximum height of the dropped
  												down list, not the actual height of the box.
  BoxGap - The gap between the colour box and the text
  BoxSurround - The gap around the colour box
  BoxWidth - The width of the colour box
  ItemHeight - The height of the items in the list (when dropped).
               Setting the value wont have any effect when not using
               Borland's C++ Builder.
  SelItemHeight - The height of the selected item box (not including the
  								(border). Usually ItemHeight+2. Setting the value wont
                  have any effect when not using Borland's C++ Builder.
  (B)SelIndex/(B)ItemIndex - The index of the currently selected item (-1 if
	                           nothing is selected).
  >(B)Focused - See IsFocused().
  >Id - The child window identifier
  >NumItems - The number of items in the combobox
  >Handle - The window handle of the combobox
  >Parent

*/
//---------------------------------------------------------------------------
TColCombo::TColCombo(HANDLE Par,bool Tabstop,HFONT fnt)
{
	VNumItems=0;for (int n=0;n<MAX_CC_ITEMS;n++)VItem[n]=NULL;
  VVisible=0;VLeft=0;VTop=0;VWidth=100;VHeight=200;
  VBoxGap=0;VBoxSurround=2;VBoxWidth=25;

  VParent=Par;
	for (VId=50000;VId<100000;VId++)if (GetDlgItem(VParent,VId)==NULL) break;
  Font=fnt;if (Font==NULL)Font=(HFONT)SendMessage(VParent,WM_GETFONT,0,0);

 	VHandle=CreateWindowEx(0,"COMBOBOX","",
  		CBS_DROPDOWNLIST | CBS_HASSTRINGS	| CBS_OWNERDRAWFIXED |
			WS_BORDER | (Tabstop==true ? WS_TABSTOP:0) | WS_CHILDWINDOW | WS_VSCROLL
     	,VLeft,VTop,VWidth,VHeight,
      VParent,(HANDLE) VId,(HANDLE) GetClassLong(VParent,GCL_HMODULE),0);

  if (IsWindow(VHandle)==0){VHandle=NULL;return;}

  SendMessage(VHandle,WM_SETFONT,(WPARAM)Font,0);
	HANDLE tempdc=CreateCompatibleDC(0);
  HANDLE oldfnt=SelectObject(tempdc,Font);
  SIZE sz;GetTextExtentPoint32(tempdc,"HjTTLMjpq",strlen("HjTTLMjpq"),&sz);
  SelectObject(tempdc,oldfnt);
  DeleteDC(tempdc);
  FontHeight=sz.cy;VItemHeight=FontHeight;VSelItemHeight=FontHeight+2;
	SendMessage(VHandle,CB_SETITEMHEIGHT,-1,VSelItemHeight);
	SendMessage(VHandle,CB_SETITEMHEIGHT,0,VItemHeight);
}
//---------------------------------------------------------------------------
void TColCombo::HandleDrawItem(UINT WinId,long Sptr)
{
	if ((int)WinId==Id) DrawItem((DRAWITEMSTRUCT*)Sptr);
}
//---------------------------------------------------------------------------
void TColCombo::BcbWMDrawItem(TMessage Mess)
{
	if (Mess.WParam==Id) DrawItem((DRAWITEMSTRUCT*)Mess.LParam);
}
//---------------------------------------------------------------------------
void TColCombo::DrawItem(DRAWITEMSTRUCT *di)
{
	if (VHandle==NULL)return;

  HANDLE br;
  int oldtextcol=GetTextColor(di->hDC),oldmode=GetBkMode(di->hDC);

  if (di->itemState & ODS_SELECTED){
    br=CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
    SetTextColor(di->hDC,GetSysColor(COLOR_HIGHLIGHTTEXT));
  }else{
    br=CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    SetTextColor(di->hDC,GetSysColor(COLOR_WINDOWTEXT));
  }

  SetBkMode(di->hDC,TRANSPARENT);
  FillRect(di->hDC,&(di->rcItem),br);
  DeleteObject(br);
  if ((di->itemID)<(UINT)NumItems){
    br=CreateSolidBrush(VItem[di->itemID]->Col);
    HANDLE pen=CreatePen(PS_SOLID,1,0);
    HANDLE oldpen=SelectObject(di->hDC,pen),oldbr=SelectObject(di->hDC,br);

    Rectangle(di->hDC,
              di->rcItem.left+BoxSurround,di->rcItem.top+BoxSurround,
              di->rcItem.left+(BoxWidth-BoxSurround),di->rcItem.bottom-BoxSurround);

    SelectObject(di->hDC,oldpen);SelectObject(di->hDC,oldbr);
    DeleteObject(pen);DeleteObject(br);

    RECT shiftrect=di->rcItem;shiftrect.left+=BoxWidth+BoxGap;
    DrawText(di->hDC,VItem[di->itemID]->Text,strlen(VItem[di->itemID]->Text),
              &shiftrect,DT_LEFT | DT_SINGLELINE | DT_VCENTER);
  }
  SetTextColor(di->hDC,oldtextcol);
  SetBkMode(di->hDC,oldmode);
  if (di->itemState & ODS_FOCUS)
    DrawFocusRect(di->hDC,&(di->rcItem));
}
//---------------------------------------------------------------------------
bool TColCombo::AddItem(char *text,int col)
{
	if (VNumItems>=MAX_CC_ITEMS || VHandle==NULL) return 0;

  VItem[VNumItems]=new CC_Info;
  memcpy(VItem[VNumItems]->Text,text,min((int)strlen(text),249));
  VItem[VNumItems]->Col=col;
  SendMessage(VHandle,CB_ADDSTRING,0,(long)text);
  VNumItems++;
  return true;
}
//---------------------------------------------------------------------------
bool TColCombo::DeleteItem(int idx)
{
 	if (VHandle==NULL || idx<0 || idx>=VNumItems)return 0;

  for (int n=idx;n<VNumItems-1;n++){
    strcpy(VItem[n]->Text,VItem[n+1]->Text);
    VItem[n]->Col=VItem[n+1]->Col;
  }
  delete[] VItem[VNumItems-1];
  SendMessage(VHandle,CB_DELETESTRING,idx,0);
  VNumItems--;
  return true;
}
//---------------------------------------------------------------------------
bool TColCombo::InsertItem(char *text,int col,int idx)
{
	if (VHandle==NULL || VNumItems>=MAX_CC_ITEMS)return 0;

  if (idx>=0 && idx<VNumItems){
    VItem[VNumItems]=new CC_Info;
    for (int n=VNumItems;n>idx;n--){
      strcpy(VItem[n]->Text,VItem[n-1]->Text);
      VItem[n]->Col=VItem[n-1]->Col;
    }
    memcpy(VItem[idx]->Text,text,min((int)strlen(text),249));
    VItem[idx]->Col=col;
    SendMessage(VHandle,CB_INSERTSTRING,idx,(long)text);
    VNumItems++;
    return true;
  }
  return AddItem(text,col);
}
//---------------------------------------------------------------------------
int TColCombo::GetIndexFromInfo(char *text,int col,bool MatchCase)
{
	int n;
	for (n=0;n<VNumItems;n++){
    if (MatchCase){
  		if (strcmp(text,VItem[n]->Text)==0 && (col==VItem[n]->Col || col==-1)){
      	return n;
      }
    }else{
  		if (strcmpi(text,VItem[n]->Text)==0 && (col==VItem[n]->Col || col==-1)){
      	return n;
      }
    }
  }
  return -1;
}
//---------------------------------------------------------------------------
void TColCombo::Clear()
{
	if (VHandle==NULL)return;

	for (int n=0;n<NumItems;n++)delete VItem[n];
	SendMessage(VHandle,CB_RESETCONTENT,0,0);
  VNumItems=0;

  InvalidateRect(VHandle,0,0);
}
//---------------------------------------------------------------------------
void TColCombo::SetBounds(int nl,int nt,int nw,int nh)
{
	if (VHandle==NULL) return;

	MoveWindow(VHandle,nl,nt,nw,nh,VVisible);
  VLeft=nl;VTop=nt;VWidth=nw;VHeight=nh;
}
//---------------------------------------------------------------------------
void TColCombo::Update()
{
	if (VHandle==NULL)return;

	MoveWindow(VHandle,VLeft,VTop,VWidth,VHeight,true);

 	if (IsWindowVisible(Handle)!=VVisible){
  	if (VVisible) ShowWindow(VHandle,SW_SHOWNA);
	  else ShowWindow(VHandle,SW_HIDE);
  }
  InvalidateRect(VHandle,0,0);
}
//---------------------------------------------------------------------------
TColCombo::~TColCombo()
{
	if (VHandle!=NULL){
		if (IsWindow(VHandle) && IsWindow(VParent))
    	DestroyWindow(VHandle);
	  for (int n=0;n<VNumItems;n++)delete VItem[n];
  }
}

