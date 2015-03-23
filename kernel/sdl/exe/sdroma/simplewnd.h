#pragma once

#define DOUBLE_MAX  (1.0e30)

class CBareWnd {
protected:
   BOOL    m_bAutoDelete;
   HINSTANCE m_hInst;
   HWND    m_hWnd; // key window handler

   BOOL    m_bClose; // useful when ShowWindowModel 
   
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

// Short hand API
CPlotWnd * PlotLine ( int nSize, double * pData, char* szTitle );
CPlotWnd * PlotLine ( int nSize, double * pData, char* szTitle, int w, int h ); 
CPlotWnd * PlotLine ( int nSize, double * pData, char* szTitle, int x, int y, int w, int h );
BOOL PauseConsole ();
