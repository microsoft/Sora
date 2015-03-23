#pragma once

#define DOUBLE_MAX  (1.0e30)
#define ID_REFRESH  1000

#include <deque>

struct COMPLEX;

class CBareWnd {
protected:
   BOOL    m_bAutoDelete;
   HINSTANCE m_hInst;
   HWND    m_hWnd; // key window handler

   BOOL    m_bClose; // useful when ShowWindowModel 
   BOOL    m_bRedrawBackground;
   
public:
   CBareWnd ();
   CBareWnd ( BOOL bAuto );
   ~CBareWnd ();

public:
    HWND HWnd () { return m_hWnd; };

public:
   virtual void Paint ( HDC hDC ); // Paint on the DC
   virtual void RegisterWindowClass ( );
   virtual LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

public:
    static LRESULT CALLBACK stWndProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	inline static CBareWnd *GetThisFromWindow(HWND hWnd)
    {
        return (CBareWnd *)GetWindowLong(hWnd, GWL_USERDATA);
    }

public:
	BOOL Create ( char * szTitle, DWORD dwStyles, RECT * rc );
	BOOL Create ( char * szTitle, int width, int height );
	BOOL Create ( char * szTitle, int x, int y, int w, int h );
	BOOL Create ( char * szTitle ); // quick create wnd with all default

	static void DoYield (); // message loop
	BOOL ShowWindowModal ();
	BOOL ShowWindow ();
	BOOL HideWindow ();
	BOOL UpdateWnd (); 

	void SetRefresh ( int msec ); // set refresh timer

	virtual void Destroy ();

public:
	virtual BOOL OnCreate ();
	
};


class CPlotWnd : public CBareWnd {
protected:
   HDC     m_hBkDC;
   HBITMAP m_hBkMap;
   HBITMAP m_hSaveMap;
   
   COLORREF m_bkColor;
   COLORREF m_penColor;
   HFONT    m_hFont;
   
   // Canvas size	
   int m_nWidth;
   int m_nHeight;
   
   
   RECT     m_rc;

static const int m_nMargin = 15;
static const int m_nFontHeight = 12;
static const int m_nFontWidth = 5;

public:
	CPlotWnd ();
	CPlotWnd ( int bAuto ); 
	virtual ~CPlotWnd ();

public:
	HDC DC () { return m_hBkDC; };
	int Width ()  { return m_nWidth; }
	int Height () { return m_nHeight; }
public:
	virtual void Paint ( HDC hDC );
	virtual BOOL OnCreate ();

	void PlotLine ( double * pData, int nSize );

	virtual void Destroy ();
	void Initialize ();
	
};

class CAnimWnd : public CPlotWnd {
public:
	double m_dMax;
	double m_dMin;
	CAnimWnd () : CPlotWnd () {
		m_dMax =-1; m_dMin = 0;
	}
	void ResetScale () { m_dMax =-1; m_dMin = 0;};
	void SetScale ( int nMax, int nMin ) {
		m_dMax = nMax;
		m_dMin = nMin;
	}
	
	void CheckScale ( int & nMax, int & nMin, double * pData, int nSize ) {
		double dMax, dMin;
		dMax = dMin = * pData;
		for (int i=1; i < nSize; i++ ) {
			if ( dMax < pData[i] ) dMax = pData[i];
			if ( dMin > pData[i] ) dMin = pData[i];
		}
		nMax = (int) dMax;
		nMin = (int) dMin;
	}
	
	void EraseCanvas (bool bDrawBorder = true);
	void UpdateCanvas ();
	
	void PlotLine ( double * pData, int nSize, int nIndex, COLORREF cr = RGB (0,255,0) );
	void PlotLine ( const std::deque<double> & pData, int nSize, int nIndex, COLORREF cr = RGB (0,255,0) );
	void PlotLine ( const std::deque<double> & pData, const std::deque<bool> & pValid, COLORREF cr = RGB(0,255,0));
	void PlotDots ( COMPLEX * pData, int nSize, COLORREF cr = RGB (0,255,0) );
	void PlotDots ( const std::deque<COMPLEX> & pData, int nSize, COLORREF cr = RGB (0,255,0));
};

// Short hand API
CPlotWnd * PlotLine ( int nSize, double * pData, char* szTitle );
CPlotWnd * PlotLine ( int nSize, double * pData, char* szTitle, int w, int h ); 
CPlotWnd * PlotLine ( int nSize, double * pData, char* szTitle, int x, int y, int w, int h );
BOOL PauseConsole (char * prompt = NULL);
