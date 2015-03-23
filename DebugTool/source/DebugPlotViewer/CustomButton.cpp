// CustomButton.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "CustomButton.h"

static Bitmap* bmpPlay = 0;
static Bitmap* bmpPause = 0;
static Bitmap* bmpPlus = 0;
static Bitmap* bmpMinus = 0;
static Bitmap* bmpAutoReplay = 0;
static Bitmap* bmpAutoPause = 0;
static bool bitmapInitialized = false;
// CustomButton

static Bitmap * GetBitmap(enum ButtonType type)
{
	if (!bitmapInitialized)
	{
		bmpPlay = Bitmap::FromResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_BUTTON_PLAY));
		bmpPause = Bitmap::FromResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_BUTTON_PAUSE));
		bmpPlus = Bitmap::FromResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_BUTTON_PLUS));
		bmpMinus = Bitmap::FromResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_BUTTON_MINUS));
		bmpAutoReplay = Bitmap::FromResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_BUTTON_AUTO));
		bmpAutoPause = Bitmap::FromResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_BUTTON_AUTO_PAUSE));
		bitmapInitialized = true;
	}

	switch(type)
	{
	case TYPE_PAUSE:
		return bmpPause;
	case TYPE_PLAY:
		return bmpPlay;
	case TYPE_PLUS:
		return bmpPlus;
	case TYPE_MINUS:
		return bmpMinus;
	case TYPE_AUTO_REPLAY:
		return bmpAutoReplay;
	case TYPE_AUTO_PAUSE:
		return bmpAutoPause;
	default:
		return 0;
	}
}

IMPLEMENT_DYNAMIC(CustomButton, CButton)

CustomButton::CustomButton()
{
	this->bmp = 0;
}

CustomButton::~CustomButton()
{
}


BEGIN_MESSAGE_MAP(CustomButton, CButton)
END_MESSAGE_MAP()



// CustomButton message handlers




//void CustomButton::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
//{
//
//	// TODO:  Add your code to draw the specified item
//}

void CustomButton::SetType(enum ButtonType type)
{
	this->bmp = ::GetBitmap(type);
	if (this->m_hWnd)
		this->Invalidate(1);
}

void CustomButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{

	// TODO:  Add your code to draw the specified item
   UINT uStyle = DFCS_BUTTONPUSH;

   // This code only works with buttons.
   ASSERT(lpDrawItemStruct->CtlType == ODT_BUTTON);

   // If drawing selected, add the pushed style to DrawFrameControl.
   if (lpDrawItemStruct->itemState & ODS_SELECTED)
      uStyle |= DFCS_PUSHED;

   // Draw the button frame.
   ::DrawFrameControl(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, 
      DFC_BUTTON, uStyle);

    Graphics graphics(lpDrawItemStruct->hDC);
	Bitmap* bitmap = this->bmp;
	//bitmap = Bitmap::FromResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_BUTTON_AUTO));
    //bitmap = Bitmap::FromResource(AfxGetApp()->m_hInstance, L"IDB_CHANNEL");
	if (bitmap)
	{
		CachedBitmap cachedBitmap(bitmap, &graphics);
		graphics.DrawCachedBitmap(&cachedBitmap, 10, 12);
	}

}
