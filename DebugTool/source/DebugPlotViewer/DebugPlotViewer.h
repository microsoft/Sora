
// DebugPlotViewer.h : main header file for the DebugPlotViewer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CDebugPlotViewerApp:
// See DebugPlotViewer.cpp for the implementation of this class
//

class CDebugPlotViewerApp : public CWinAppEx
{
public:
	CDebugPlotViewerApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

private:
	ULONG_PTR m_gdiplusToken;	// GDI+

	HANDLE singleInstanceMutex;
};

extern CDebugPlotViewerApp theApp;
