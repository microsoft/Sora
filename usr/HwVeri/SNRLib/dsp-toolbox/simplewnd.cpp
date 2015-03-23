//#include "stdafx.h"

#include <windows.h>

#include <conio.h>
#include <stdio.h>
#include "dsplib.h"
#include "simplewnd.h"

const char szBareClass[] = "BareWindows";
static BOOL bBareClassRegistered = FALSE;

void CBareWnd::RegisterWindowClass ( )
{
	WNDCLASSEX wcex;


	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= CBareWnd::stWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= m_hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL; /// MAKEINTRESOURCE(IDC_SRVIEW);
	wcex.lpszClassName	= (char*)szBareClass;
	wcex.hIconSm		= NULL;
	
	RegisterClassEx(&wcex);
}


CBareWnd::CBareWnd () : m_hWnd (NULL), m_bAutoDelete(FALSE)
{
    m_hInst = GetModuleHandle (NULL);

	if ( !bBareClassRegistered ) {
		RegisterWindowClass ();
		bBareClassRegistered = TRUE;
	}

	m_bClose = FALSE;
}

CBareWnd::~CBareWnd () {
	// Clean up
	if ( m_bAutoDelete ) 
		m_bAutoDelete = FALSE;

	if ( m_hWnd) {
		::DestroyWindow ( m_hWnd );
		m_hWnd = NULL;
	}
}

CBareWnd::CBareWnd ( BOOL bAuto ) : m_hWnd (NULL) {
	m_hInst = GetModuleHandle (NULL);

	if ( !bBareClassRegistered ) {
		RegisterWindowClass ();
		bBareClassRegistered = TRUE;
	}

	m_bClose = FALSE;
	m_bAutoDelete = bAuto;
}

void CBareWnd::Paint ( HDC hDC ) {
}

BOOL CBareWnd::ShowWindow () {
	if ( m_hWnd == NULL ) 
		return FALSE;

	::ShowWindow (m_hWnd, SW_SHOW);
	return TRUE;
}

BOOL CBareWnd::HideWindow () {
	if ( m_hWnd == NULL ) 
		return FALSE;
	
	::ShowWindow (m_hWnd, SW_HIDE);
	return TRUE;	
}

void CBareWnd::SetRefresh ( int msec ) {
	if ( m_hWnd == NULL ) return;

	if ( msec == 0 ) {
		KillTimer (m_hWnd, ID_REFRESH );
	} else {
		SetTimer ( m_hWnd, ID_REFRESH, msec, NULL );
	}
}

BOOL CBareWnd::UpdateWnd () {
	RECT rc;
	if ( m_hWnd == NULL ) return FALSE;

	::GetClientRect ( m_hWnd, &rc );
	::InvalidateRect ( m_hWnd,  &rc, FALSE );
	::UpdateWindow (m_hWnd);
	return TRUE;
}

BOOL CBareWnd::ShowWindowModal ()
{
	if ( !ShowWindow () ) {
		return FALSE;
	}
	m_bClose = FALSE;
	while ( !m_bClose ) {
		DoYield ();
	}
	return TRUE; 
}

BOOL CBareWnd::Create ( char * szTitle, DWORD dwStyles, RECT * rc )
{
	if ( m_hWnd != NULL ) 
		return FALSE;

	m_hWnd = CreateWindow ( szBareClass, szTitle, dwStyles,
		rc->left, rc->top, rc->right-rc->left, rc->bottom-rc->top,
		NULL, NULL, m_hInst, (void*) this );
	if ( m_hWnd == NULL ) return FALSE;
	return OnCreate () ;
}

BOOL CBareWnd::Create ( char * szTitle )
{
	if ( m_hWnd != NULL ) 
		return FALSE;

	m_hWnd = CreateWindow ( szBareClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		NULL, NULL, m_hInst, (void*) this );

	if ( m_hWnd == NULL ) return FALSE;
	return OnCreate () ;
}

BOOL CBareWnd::Create ( char * szTitle, int width, int height )
{
	if ( m_hWnd != NULL ) 
		return FALSE;

	m_hWnd = CreateWindow ( szBareClass, szTitle, 0,
		CW_USEDEFAULT, 0, width, height,
		NULL, NULL, m_hInst, (void*) this );

	if ( m_hWnd == NULL ) return FALSE;
	return OnCreate () ;
}

BOOL CBareWnd::Create ( char * szTitle, int x, int y, int w, int h )
{
	if ( m_hWnd != NULL ) 
		return FALSE;

	m_hWnd = CreateWindow ( szBareClass, szTitle, WS_SYSMENU ,
		x, y, w, h,
		NULL, NULL, m_hInst, (void*) this );

	if ( m_hWnd == NULL ) return FALSE;
	return OnCreate () ;

}


void CBareWnd::Destroy () {
	m_hWnd = NULL;

	if ( m_bAutoDelete ) {
		delete this;
	}
}

BOOL CBareWnd::OnCreate () {
	return TRUE;
}

LRESULT CALLBACK CBareWnd::WndProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) 
{
	//int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (uMsg)
	{
	case WM_PAINT:
		hdc = BeginPaint(m_hWnd, &ps);
		
		Paint  ( hdc );

		EndPaint(m_hWnd, &ps);
		break;
	case WM_DESTROY:
		// Destroy the window
		// reset the global window handle to null
		m_bClose = TRUE;
		Destroy ();
		
		break;
	case WM_SIZE:
		m_bRedrawBackground = true;
		break;
	case WM_TIMER:
		if ( wParam == ID_REFRESH ) {
			RECT rc;
			GetClientRect (hWnd, &rc);
			InvalidateRect (hWnd,&rc,FALSE);
			UpdateWindow (hWnd);
		}
		break;
	case WM_CLOSE:
		::ShowWindow(hWnd, FALSE);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK CBareWnd::stWndProc ( HWND hWnd, UINT uMsg, 
	                                  WPARAM wParam, LPARAM lParam )
{
	CBareWnd * pWnd;
	if ( uMsg == WM_NCCREATE ) {
		SetWindowLong ( hWnd, GWL_USERDATA, 
			(long) ((LPCREATESTRUCT(lParam))->lpCreateParams));
	}

	pWnd = GetThisFromWindow ( hWnd );
	if ( pWnd ) {
		return pWnd->WndProc ( hWnd, uMsg, wParam, lParam);
	} else {
		return DefWindowProc ( hWnd, uMsg, wParam, lParam );
	}
}

void CBareWnd::DoYield ()
{
	// Just message loop
	MSG msg; 
    while(::PeekMessage (&msg, NULL, 0, 0, PM_REMOVE) ) {
	    ::TranslateMessage(&msg); 
	    ::DispatchMessage(&msg); 

	}
}

// CPlotWnd
CPlotWnd::CPlotWnd (): CBareWnd (), m_hBkDC (NULL), m_hBkMap (NULL), m_hSaveMap (NULL) {
	Initialize ();
}

CPlotWnd::CPlotWnd (BOOL bAuto ): CBareWnd ( bAuto ), m_hBkDC (NULL), m_hBkMap (NULL), m_hSaveMap (NULL) {
	Initialize ();
}

void CPlotWnd::Initialize ()
{
	m_bkColor = RGB (0,0,0);
	m_penColor = RGB (0,255,0);
	
	m_nWidth = m_nHeight = 0;
	m_hFont  = CreateFont (
        m_nFontHeight,      // height of font
        m_nFontWidth,       // average character width
        0,                  // angle of escapement
        0,                  // base-line orientation angle
        FW_DONTCARE,        // font weight
        FALSE,              // italic attribute option
        FALSE,              // underline attribute option
        FALSE,              // strikeout attribute option
        ANSI_CHARSET,                   // character set identifier
        OUT_CHARACTER_PRECIS,           // output precision
        CLIP_CHARACTER_PRECIS,          // clipping precision
        DEFAULT_QUALITY,                // output quality
        DEFAULT_PITCH | FF_DONTCARE,    // pitch and family
        "Arial"                         // typeface name
    );
}

CPlotWnd::~CPlotWnd () {
	Destroy ();
}

void CPlotWnd::Destroy () {
	if ( m_hBkDC ) {
		SelectObject ( m_hBkDC, m_hSaveMap );
		DeleteObject ( m_hBkMap );
		DeleteDC ( m_hBkDC );
		DeleteObject ( m_hFont );
		
		m_hBkDC = NULL;
	}

	CBareWnd::Destroy ();
}

void CPlotWnd::Paint ( HDC hDC ) {
	RECT rc;
	GetClientRect (m_hWnd, &rc );

	HBRUSH  hBR     = CreateSolidBrush ( m_bkColor );
	HBRUSH  hOldBr  = (HBRUSH) SelectObject( hDC, hBR );


	if ( m_bRedrawBackground ) {
		Rectangle ( hDC, rc.left, rc.top, rc.right, rc.bottom );
		m_bRedrawBackground = false;
	}

	BitBlt ( hDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
		     m_hBkDC, 0, 0, SRCCOPY);
	
	SelectObject (hDC, hOldBr );
	DeleteObject ( hBR );
}

BOOL CPlotWnd::OnCreate () {
	// Create bk DC and 
	if ( m_hBkDC == NULL ) {
	    Initialize ();

		HDC hDC = GetWindowDC ( m_hWnd );
		GetClientRect ( m_hWnd, &m_rc );
		m_nWidth = m_rc.right - m_rc.left;
		m_nHeight = m_rc.bottom - m_rc.top;
	
		m_hBkDC  = CreateCompatibleDC(hDC);
		m_hBkMap = CreateCompatibleBitmap( hDC, m_nWidth, m_nHeight );
    	m_hSaveMap = (HBITMAP) SelectObject( m_hBkDC, m_hBkMap);
		HBRUSH hBR =  CreateSolidBrush ( m_bkColor );
		HBRUSH hOldBr = (HBRUSH) SelectObject( m_hBkDC, hBR );

		Rectangle ( m_hBkDC, m_rc.left, m_rc.top, m_rc.right, m_rc.bottom );
		
		SelectObject ( m_hBkDC, hOldBr );
		ReleaseDC ( m_hWnd, hDC );
	}

	return TRUE;
}

void CPlotWnd::PlotLine ( double * pData, int nSize ) {
	if ( m_hBkDC == NULL ) {
		return;
	}

	HBRUSH  hBR     = CreateSolidBrush ( m_bkColor );
	HBRUSH  hOldBr  = (HBRUSH) SelectObject( m_hBkDC, hBR );
	Rectangle ( m_hBkDC, 0, 0, m_nWidth, m_nHeight );


	HPEN hPen = CreatePen ( PS_SOLID, 1, m_penColor );
	HPEN hOldPen = (HPEN) SelectObject (m_hBkDC, hPen );
	HFONT hOldFont = (HFONT) SelectObject ( m_hBkDC, m_hFont );

	int w = m_nWidth  - 2*m_nMargin;
	int h = m_nHeight - 2*m_nMargin;

	// Border
	Rectangle ( m_hBkDC, m_nMargin, m_nMargin, m_nMargin + w, m_nMargin + h );
	
	// try to scale
	double data_max, data_min;
	data_max = data_min = pData[0] ;
	for ( int i =1; i< nSize; i++) {
		if ( pData[i] > data_max ) data_max = pData[i];
		if ( pData[i] < data_min ) data_min = pData[i];
	}
	double data_span = data_max - data_min;
	
	double step = w * 1.0 / ( nSize -1 );
	int x = m_nMargin + 0;
	int y = m_nMargin + (int) ( (1 - (pData[0]-data_min) / data_span)*h ); 
		
	MoveToEx ( m_hBkDC, x, y, NULL); 
	for ( int i=1; i< nSize; i++ ) {
		x = m_nMargin + (int) (step * i );
		y = m_nMargin + (int) ((1 - (pData[i] - data_min) / data_span)*h );
		LineTo ( m_hBkDC, x, y );
	}

	// Draw x-axis
	y = m_nMargin + (int) ((1 + data_min / data_span)*h );
	MoveToEx ( m_hBkDC, m_nMargin, y, NULL );
	LineTo ( m_hBkDC, m_nMargin + w, y );

	// Draw label
	SetBkColor ( m_hBkDC, m_bkColor );
	SetTextColor ( m_hBkDC, m_penColor );
	char labelTxt [120];
	sprintf_s ( labelTxt, "%.3e", data_min );
	TextOut ( m_hBkDC, 0, m_nMargin+h+2, labelTxt, strlen (labelTxt) );

	sprintf_s ( labelTxt, "%.3e", data_max );
	TextOut ( m_hBkDC, 0, 2, labelTxt, strlen (labelTxt) );

	sprintf_s ( labelTxt, "%d", nSize );
	TextOut ( m_hBkDC, m_rc.right - m_nMargin - m_nFontWidth*strlen (labelTxt), m_nMargin+h+2, labelTxt, strlen (labelTxt) );
	
	SelectObject ( m_hBkDC, hOldPen );
	SelectObject ( m_hBkDC, hOldBr );
	SelectObject ( m_hBkDC, hOldFont );
	DeleteObject ( hPen );
	DeleteObject ( hBR );

	InvalidateRect ( m_hWnd, &m_rc, FALSE );
	UpdateWindow   ( m_hWnd );
}


void CAnimWnd::EraseCanvas (bool bDrawBorder)
{
	if ( m_hBkDC == NULL ) {
		return;
	}

	RECT rc;

	GetClientRect (m_hWnd,&rc);
	
	HBRUSH  hBR     = CreateSolidBrush ( m_bkColor );
	HBRUSH  hOldBr;
	/*
	if ( m_bRedrawBackground ) {
		
		HDC hDC = GetDC ( m_hWnd );
		hOldBr	= (HBRUSH) SelectObject( hDC, hBR );	
		Rectangle ( hDC, 0, 0, rc.right, rc.bottom );		
		
		SelectObject (hDC, hOldBr );
		ReleaseDC (m_hWnd, hDC );

		m_bRedrawBackground = false;
	}*/
	
	hOldBr	= (HBRUSH) SelectObject( m_hBkDC, hBR );


	Rectangle ( m_hBkDC, 0, 0, m_nWidth, m_nHeight );


	HPEN hPen = CreatePen ( PS_SOLID, 1, m_penColor );
	
	HPEN hOldPen = (HPEN) SelectObject (m_hBkDC, hPen );
	HFONT hOldFont = (HFONT) SelectObject ( m_hBkDC, m_hFont );

	int w = m_nWidth  - 2*m_nMargin;
	int h = m_nHeight - 2*m_nMargin;

	// Border
	Rectangle ( m_hBkDC, m_nMargin, m_nMargin, m_nMargin + w, m_nMargin + h );

	SelectObject ( m_hBkDC, hOldPen );
	SelectObject ( m_hBkDC, hOldBr );
	SelectObject ( m_hBkDC, hOldFont );
	DeleteObject ( hPen );
	DeleteObject ( hBR );

}

void CAnimWnd::PlotDots ( COMPLEX * pData, int nSize, 
	            COLORREF cr)
{
	if ( m_hBkDC == NULL ) {
		return;
	}
	HBRUSH  hBR     = CreateSolidBrush ( m_bkColor );
	HBRUSH  hOldBr  = (HBRUSH) SelectObject( m_hBkDC, hBR );

	HPEN hDrawPen = CreatePen ( PS_SOLID, 1, cr );
	
	HPEN hOldPen   = (HPEN) SelectObject ( m_hBkDC, hDrawPen );
	HFONT hOldFont = (HFONT) SelectObject ( m_hBkDC, m_hFont );

	int w = m_nWidth  - 2*m_nMargin;
	int h = m_nHeight - 2*m_nMargin;


	double cx = w / 2 + m_nMargin;
	double cy = h / 2 + m_nMargin;
	
	for ( int i=0; i< nSize; i++ ) {
		int x = (int) (cx + pData[i].re / m_dMax * w / 2);
		int y = (int) (cy + pData[i].im / m_dMax * h / 2);
		MoveToEx ( m_hBkDC, x-1, y, NULL );
		LineTo   ( m_hBkDC, x+2, y );
		
		MoveToEx ( m_hBkDC, x, y-1, NULL );
		LineTo   ( m_hBkDC, x, y+2 );
	}

	// Draw label
	/*
	SetBkColor   ( m_hBkDC, m_bkColor );
	SetTextColor ( m_hBkDC, m_penColor );
	char labelTxt [120];
	sprintf ( labelTxt, "-%.3e", ymax );
	TextOut ( m_hBkDC, 0, m_nMargin+h+2, labelTxt, strlen (labelTxt) );

	sprintf ( labelTxt, "%.3e", ymax );
	TextOut ( m_hBkDC, 0, 2, labelTxt, strlen (labelTxt) );
	*/
	
	SelectObject ( m_hBkDC, hOldPen );
	SelectObject ( m_hBkDC, hOldBr );
	SelectObject ( m_hBkDC, hOldFont );
	DeleteObject ( hBR );
	DeleteObject ( hDrawPen );

}

void CAnimWnd::PlotDots ( const std::deque<COMPLEX> & pData, int nSize, 
	            COLORREF cr)
{
	if ( m_hBkDC == NULL ) {
		return;
	}
	HBRUSH  hBR     = CreateSolidBrush ( m_bkColor );
	HBRUSH  hOldBr  = (HBRUSH) SelectObject( m_hBkDC, hBR );

	HPEN hDrawPen = CreatePen ( PS_SOLID, 1, cr );
	
	HPEN hOldPen   = (HPEN) SelectObject ( m_hBkDC, hDrawPen );
	HFONT hOldFont = (HFONT) SelectObject ( m_hBkDC, m_hFont );

	int w = m_nWidth  - 2*m_nMargin;
	int h = m_nHeight - 2*m_nMargin;


	double cx = w / 2 + m_nMargin;
	double cy = h / 2 + m_nMargin;
	
	for ( int i=0; i< nSize; i++ ) {
		int x = (int) (cx + pData[i].re / m_dMax * w / 2);
		int y = (int) (cy + pData[i].im / m_dMax * h / 2);
		MoveToEx ( m_hBkDC, x-1, y, NULL );
		LineTo   ( m_hBkDC, x+2, y );
		
		MoveToEx ( m_hBkDC, x, y-1, NULL );
		LineTo   ( m_hBkDC, x, y+2 );
	}

	// Draw label
	/*
	SetBkColor   ( m_hBkDC, m_bkColor );
	SetTextColor ( m_hBkDC, m_penColor );
	char labelTxt [120];
	sprintf ( labelTxt, "-%.3e", ymax );
	TextOut ( m_hBkDC, 0, m_nMargin+h+2, labelTxt, strlen (labelTxt) );

	sprintf ( labelTxt, "%.3e", ymax );
	TextOut ( m_hBkDC, 0, 2, labelTxt, strlen (labelTxt) );
	*/
	
	SelectObject ( m_hBkDC, hOldPen );
	SelectObject ( m_hBkDC, hOldBr );
	SelectObject ( m_hBkDC, hOldFont );
	DeleteObject ( hBR );
	DeleteObject ( hDrawPen );

}

void CAnimWnd::PlotLine ( double * pData, int nSize, 
	                      int nIndex, COLORREF cr ) 
{
	if ( m_hBkDC == NULL ) {
		return;
	}

	HBRUSH  hBR     = CreateSolidBrush ( m_bkColor );
	HBRUSH  hOldBr  = (HBRUSH) SelectObject( m_hBkDC, hBR );

	HPEN hPen     = CreatePen ( PS_SOLID, 1, m_penColor );
	HPEN hDrawPen = CreatePen ( PS_SOLID, 1, cr );
	
	HPEN hOldPen   = (HPEN) SelectObject (m_hBkDC, hPen );
	HFONT hOldFont = (HFONT) SelectObject ( m_hBkDC, m_hFont );

	int w = m_nWidth  - 2*m_nMargin;
	int h = m_nHeight - 2*m_nMargin;

	// select draw pen
	SelectObject ( m_hBkDC, hDrawPen );
	
	// try to scale
	double data_max, data_min;
	data_max = data_min = pData[0] ;
	for ( int i =1; i< nSize; i++) {
		if ( pData[i] > data_max ) data_max = pData[i];
		if ( pData[i] < data_min ) data_min = pData[i];
	}

	if ( m_dMax < m_dMin ) {
		m_dMax = data_max;
		m_dMin = data_min;
	} else {
		if ( m_dMax < data_max ) {
			m_dMax = data_max;
		}
		if ( m_dMin > data_min ) {
			m_dMin = data_min;
		}
	}
	
	double data_span = m_dMax - m_dMin;
	
	double step = w * 1.0 / ( nSize -1 );
	int x = m_nMargin + 0;
	int y = m_nMargin + (int) ( (1 - (pData[nIndex]-m_dMin) / data_span)*h ); 
		
	MoveToEx ( m_hBkDC, x, y, NULL); 
	for ( int i=1; i< nSize; i++ ) {
		x = m_nMargin + (int) (step * i );
		y = m_nMargin + (int) ((1 - (pData[(nIndex+i)%nSize] - m_dMin) / data_span)*h );
		LineTo ( m_hBkDC, x, y );
	}

	// Draw label
	SetBkColor ( m_hBkDC, m_bkColor );
	SetTextColor ( m_hBkDC, m_penColor );
	char labelTxt [120];
	sprintf_s ( labelTxt, "%.3e", m_dMin );
	TextOut ( m_hBkDC, 0, m_nMargin+h+2, labelTxt, strlen (labelTxt) );

	sprintf_s ( labelTxt, "%.3e", m_dMax );
	TextOut ( m_hBkDC, 0, 2, labelTxt, strlen (labelTxt) );

	sprintf_s ( labelTxt, "%d", nSize );
	TextOut ( m_hBkDC, m_rc.right - m_nMargin - m_nFontWidth*strlen (labelTxt), m_nMargin+h+2, labelTxt, strlen (labelTxt) );
	
	SelectObject ( m_hBkDC, hOldPen );
	SelectObject ( m_hBkDC, hOldBr );
	SelectObject ( m_hBkDC, hOldFont );
	DeleteObject ( hPen );
	DeleteObject ( hBR );
	DeleteObject ( hDrawPen );
}


void CAnimWnd::PlotLine ( const std::deque<double> & pData, const std::deque<bool> & pValid, COLORREF cr)
{
	//PlotLine(pData, pData.size(), 0, cr);

	if ( m_hBkDC == NULL ) {
		return;
	}
	int nSize = pData.size();
	if (nSize != pValid.size())
		return;

	int nIndex = -1;

	for (int i = 0; i < nSize; i++)
	{
		if (pValid[i] == true)
		{
			nIndex = i;
			break;
		}
	}

	if (nIndex < 0)
		return;

	HBRUSH  hBR     = CreateSolidBrush ( m_bkColor );
	HBRUSH  hOldBr  = (HBRUSH) SelectObject( m_hBkDC, hBR );

	HPEN hPen     = CreatePen ( PS_SOLID, 1, m_penColor );
	HPEN hDrawPen = CreatePen ( PS_SOLID, 1, cr );
	
	HPEN hOldPen   = (HPEN) SelectObject (m_hBkDC, hPen );
	HFONT hOldFont = (HFONT) SelectObject ( m_hBkDC, m_hFont );

	int w = m_nWidth  - 2*m_nMargin;
	int h = m_nHeight - 2*m_nMargin;

	// select draw pen
	SelectObject ( m_hBkDC, hDrawPen );
	
	// try to scale
	double data_max, data_min;
	data_max = data_min = pData[0] ;
	for (int i = nIndex; i < nSize; i++) {

		if (pValid[i] == false)
			continue;
		
		if ( pData[i] > data_max ) data_max = pData[i];
		if ( pData[i] < data_min ) data_min = pData[i];
	}

	// ugly...
	if (data_max == data_min)
	{
		data_min = 0.0;

		if (data_max == 0.0)
		{
			data_max = 1.0;
		}
	}

	if ( m_dMax < m_dMin ) {
		m_dMax = data_max;
		m_dMin = data_min;
	} else {
		if ( m_dMax < data_max ) {
			m_dMax = data_max;
		}
		if ( m_dMin > data_min ) {
			m_dMin = data_min;
		}
	}
	
	double data_span = m_dMax - m_dMin;
	data_span *= 4;
	
	double step = w * 1.0 / ( nSize -1 );
	int x = m_nMargin + (int) (step * nIndex );
	int y = m_nMargin + (int) ( (1 - (pData[nIndex]-m_dMin) / data_span)*h ) - h / 2; 
		
	bool lastValid = true;
	MoveToEx ( m_hBkDC, x, y, NULL); 
	for ( int i= nIndex + 1; i < nSize; i++ ) {
		x = m_nMargin + (int) (step * i );
		y = m_nMargin + (int) ((1 - (pData[i] - m_dMin) / data_span)*h ) - h / 2;
		if (lastValid == true)
		{
			if (pValid[i] == true)
				LineTo ( m_hBkDC, x, y );
		}
		else
		{
			if (pValid[i] == true)
				MoveToEx (m_hBkDC, x, y, NULL);
		}

		lastValid = pValid[i];
	}

	// Draw label
	SetBkColor ( m_hBkDC, m_bkColor );
	SetTextColor ( m_hBkDC, m_penColor );
	char labelTxt [120];
	//sprintf_s ( labelTxt, "%.3e", m_dMin );
	sprintf_s ( labelTxt, "%.3e", m_dMin - 2 * (m_dMax - m_dMin) );
	TextOut ( m_hBkDC, 0, m_nMargin+h+2, labelTxt, strlen (labelTxt) );

	//sprintf_s ( labelTxt, "%.3e", m_dMax );
	sprintf_s ( labelTxt, "%.3e", m_dMin + 2 * (m_dMax - m_dMin) );
	TextOut ( m_hBkDC, 0, 2, labelTxt, strlen (labelTxt) );

	sprintf_s ( labelTxt, "%d", nSize );
	TextOut ( m_hBkDC, m_rc.right - m_nMargin - m_nFontWidth*strlen (labelTxt), m_nMargin+h+2, labelTxt, strlen (labelTxt) );
	
	SelectObject ( m_hBkDC, hOldPen );
	SelectObject ( m_hBkDC, hOldBr );
	SelectObject ( m_hBkDC, hOldFont );
	DeleteObject ( hPen );
	DeleteObject ( hBR );
	DeleteObject ( hDrawPen );

}

void CAnimWnd::PlotLine ( const std::deque<double> & pData, int nSize, 
	                      int nIndex, COLORREF cr ) 
{
	if ( m_hBkDC == NULL ) {
		return;
	}

	HBRUSH  hBR     = CreateSolidBrush ( m_bkColor );
	HBRUSH  hOldBr  = (HBRUSH) SelectObject( m_hBkDC, hBR );

	HPEN hPen     = CreatePen ( PS_SOLID, 1, m_penColor );
	HPEN hDrawPen = CreatePen ( PS_SOLID, 1, cr );
	
	HPEN hOldPen   = (HPEN) SelectObject (m_hBkDC, hPen );
	HFONT hOldFont = (HFONT) SelectObject ( m_hBkDC, m_hFont );

	int w = m_nWidth  - 2*m_nMargin;
	int h = m_nHeight - 2*m_nMargin;

	// select draw pen
	SelectObject ( m_hBkDC, hDrawPen );
	
	// try to scale
	double data_max, data_min;
	data_max = data_min = pData[0] ;
	for ( int i =1; i< nSize; i++) {
		if ( pData[i] > data_max ) data_max = pData[i];
		if ( pData[i] < data_min ) data_min = pData[i];
	}

	if ( m_dMax < m_dMin ) {
		m_dMax = data_max;
		m_dMin = data_min;
	} else {
		if ( m_dMax < data_max ) {
			m_dMax = data_max;
		}
		if ( m_dMin > data_min ) {
			m_dMin = data_min;
		}
	}
	
	double data_span = m_dMax - m_dMin;
	data_span *= 4;
	
	double step = w * 1.0 / ( nSize -1 );
	int x = m_nMargin + 0;
	int y = m_nMargin + (int) ( (1 - (pData[nIndex]-m_dMin) / data_span)*h ) - h / 2; 
		
	MoveToEx ( m_hBkDC, x, y, NULL); 
	for ( int i=1; i< nSize; i++ ) {
		x = m_nMargin + (int) (step * i );
		y = m_nMargin + (int) ((1 - (pData[(nIndex+i)%nSize] - m_dMin) / data_span)*h ) - h / 2;
		LineTo ( m_hBkDC, x, y );
	}

	// Draw label
	SetBkColor ( m_hBkDC, m_bkColor );
	SetTextColor ( m_hBkDC, m_penColor );
	char labelTxt [120];
	//sprintf_s ( labelTxt, "%.3e", m_dMin );
	sprintf_s ( labelTxt, "%.3e", m_dMin - 2 * (m_dMax - m_dMin) );
	TextOut ( m_hBkDC, 0, m_nMargin+h+2, labelTxt, strlen (labelTxt) );

	//sprintf_s ( labelTxt, "%.3e", m_dMax );
	sprintf_s ( labelTxt, "%.3e", m_dMin + 2 * (m_dMax - m_dMin) );
	TextOut ( m_hBkDC, 0, 2, labelTxt, strlen (labelTxt) );

	sprintf_s ( labelTxt, "%d", nSize );
	TextOut ( m_hBkDC, m_rc.right - m_nMargin - m_nFontWidth*strlen (labelTxt), m_nMargin+h+2, labelTxt, strlen (labelTxt) );
	
	SelectObject ( m_hBkDC, hOldPen );
	SelectObject ( m_hBkDC, hOldBr );
	SelectObject ( m_hBkDC, hOldFont );
	DeleteObject ( hPen );
	DeleteObject ( hBR );
	DeleteObject ( hDrawPen );
}


//void CAnimWnd::PlotLine ( double * pData, int nSize, 
//	                      int nIndex, COLORREF cr ) 
//{
//	if ( m_hBkDC == NULL ) {
//		return;
//	}
//
//	HBRUSH  hBR     = CreateSolidBrush ( m_bkColor );
//	HBRUSH  hOldBr  = (HBRUSH) SelectObject( m_hBkDC, hBR );
//
//	HPEN hPen     = CreatePen ( PS_SOLID, 1, m_penColor );
//	HPEN hDrawPen = CreatePen ( PS_SOLID, 1, cr );
//	
//	HPEN hOldPen   = (HPEN) SelectObject (m_hBkDC, hPen );
//	HFONT hOldFont = (HFONT) SelectObject ( m_hBkDC, m_hFont );
//
//	int w = m_nWidth  - 2*m_nMargin;
//	int h = m_nHeight - 2*m_nMargin;
//
//	// select draw pen
//	SelectObject ( m_hBkDC, hDrawPen );
//	
//	// try to scale
//	double data_max, data_min;
//	data_max = data_min = pData[0] ;
//	for ( int i =1; i< nSize; i++) {
//		if ( pData[i] > data_max ) data_max = pData[i];
//		if ( pData[i] < data_min ) data_min = pData[i];
//	}
//
//	if ( m_dMax < m_dMin ) {
//		m_dMax = data_max;
//		m_dMin = data_min;
//	} else {
//		if ( m_dMax < data_max ) {
//			m_dMax = data_max;
//		}
//		if ( m_dMin > data_min ) {
//			m_dMin = data_min;
//		}
//	}
//	
//	double data_span = m_dMax - m_dMin;
//	
//	double step = w * 1.0 / ( nSize -1 );
//	int x = m_nMargin + 0;
//	int y = m_nMargin + (int) ( (1 - (pData[nIndex]-m_dMin) / data_span)*h ); 
//		
//	MoveToEx ( m_hBkDC, x, y, NULL); 
//	for ( int i=1; i< nSize; i++ ) {
//		x = m_nMargin + (int) (step * i );
//		y = m_nMargin + (int) ((1 - (pData[(nIndex+i)%nSize] - m_dMin) / data_span)*h );
//		LineTo ( m_hBkDC, x, y );
//	}
//
//	// Draw label
//	SetBkColor ( m_hBkDC, m_bkColor );
//	SetTextColor ( m_hBkDC, m_penColor );
//	char labelTxt [120];
//	sprintf ( labelTxt, "%.3e", m_dMin );
//	TextOut ( m_hBkDC, 0, m_nMargin+h+2, labelTxt, strlen (labelTxt) );
//
//	sprintf ( labelTxt, "%.3e", m_dMax );
//	TextOut ( m_hBkDC, 0, 2, labelTxt, strlen (labelTxt) );
//
//	sprintf ( labelTxt, "%d", nSize );
//	TextOut ( m_hBkDC, m_rc.right - m_nMargin - m_nFontWidth*strlen (labelTxt), m_nMargin+h+2, labelTxt, strlen (labelTxt) );
//	
//	SelectObject ( m_hBkDC, hOldPen );
//	SelectObject ( m_hBkDC, hOldBr );
//	SelectObject ( m_hBkDC, hOldFont );
//	DeleteObject ( hPen );
//	DeleteObject ( hBR );
//	DeleteObject ( hDrawPen );
//}


void CAnimWnd::UpdateCanvas ()
{
	InvalidateRect ( m_hWnd, &m_rc, FALSE );
	UpdateWindow   ( m_hWnd );
}

////////////////////////////////////////////////////////////////
// 

CPlotWnd * PlotLine ( int nSize, double * pData, char* szTitle ) 
{
	CPlotWnd * pWnd = new CPlotWnd ( TRUE );
	pWnd->Create ( szTitle );
	pWnd->PlotLine ( pData, nSize );
	pWnd->ShowWindow ();
	return pWnd;
}

CPlotWnd * PlotLine ( int nSize, double * pData, char* szTitle, int x, int y, int w, int h ) 
{
	CPlotWnd * pWnd = new CPlotWnd ( TRUE );
	pWnd->Create ( szTitle, x, y, w, h );
	pWnd->PlotLine ( pData, nSize );
	pWnd->ShowWindow ();
	return pWnd;
}

CPlotWnd * PlotLine ( int nSize, double * pData, char* szTitle, int w, int h ) 
{
	CPlotWnd * pWnd = new CPlotWnd ( TRUE );
	pWnd->Create ( szTitle, w, h );
	pWnd->PlotLine ( pData, nSize );
	pWnd->ShowWindow ();
    return pWnd;
	
}

BOOL PauseConsole ( char * prompt) {
	int ch;
	if ( prompt ) {
		printf ( "%s:>\n", prompt );
	}
	while (1) {
		if ( _kbhit ()) {
			ch = _getch ();
			break;
		}
		CBareWnd::DoYield ();
	}
	
	if ( ch == 0x1b ) // ESC
		return TRUE;

	return FALSE;
	
}

