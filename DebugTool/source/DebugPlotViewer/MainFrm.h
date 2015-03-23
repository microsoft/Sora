
// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "OutputWnd.h"
#include "PropertiesWnd.h"
#include "ChannelExplorer.h"
#include "PlayControl.h"
#include "AutoLayoutPanel.h"
#include <set>

#include "SharedSerialNumGenerator.h"

class CMainFrame : public CFrameWndEx
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

	void StartReadingThread();
	void KillReadingThread();

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
#ifdef ENABLE_STATUSBAR
	CMFCStatusBar     m_wndStatusBar;
#endif
	CMFCToolBarImages m_UserImages;

	CPropertiesWnd    m_wndProperties;
	CChannelExplorerWnd	m_wndChannelExplorer;
	AutoLayoutPanel		m_wndAutoLayout;
	CPlayControlWnd		m_wndPlayControl;

#ifdef ENABLE_OUTPUT_PANEL
	COutputWnd			m_wndOutput;
#endif

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg LRESULT OnApp(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);

private:
	static void __stdcall ReadDataFunction(const char * buffer, int size, void * userData, __int32 userId);

	friend void __cdecl ThreadReadData(PVOID param);
	HANDLE hReadingThread;
	int stopFlag;
	int timeIntervalMs;

	bool enableTimerFlag;

	IChannelBufferReadable * _channelBufferReadable;

private:
	enum {
		TIMER_CONTROLLER,
		TIMER_UPDATE_TREE,
		TIMER_UPDATE_GRAPH,
	};

public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	set<CWnd *> registeredWnd;

	bool closeFlag;
	HANDLE hWaitableTimer;

	SoraDbgPlot::Common::SharedSerialNumGenerator * _serialGen;
};



