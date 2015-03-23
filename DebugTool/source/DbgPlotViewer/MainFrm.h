
// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include "DockableWndContainer.h"
#include "BaseProperty.h"
#include "PassiveTaskQueue.h"

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

public:
	void EnableTimer(bool bEnable);
	bool _bTimeEnabled;

public:
	void FlushMsg();

private:
	LRESULT OnExeTaskQueue(WPARAM wParam, LPARAM lParam);

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
	CMFCToolBar       _playControlToolBar;
	CMFCToolBarImages m_UserImages;

	std::shared_ptr<DockableWndContainer> _channelExplorerPanel;
	std::shared_ptr<DockableWndContainer> _autoLayoutPanel;
	std::shared_ptr<DockableWndContainer> _propertyPanel;
	std::shared_ptr<DockableWndContainer> _controlPanel;

	CImageList _channelViewImages;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewCustomize();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg LRESULT OnRealClose(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPlayPauseAll();
	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
	BOOL DockDockpanels();
	void SetDockingWindowIcons(BOOL bHiColorIcons);

private:
	std::vector<std::pair<int, std::function<void(void)> > > _timerHandlers;

private:
	void InitWorkWhenCreated();
	void DeinitWorkWhenClosed();

public:
	void OpenDocument(const CString & path);
	void SaveDocument(const CString & path);
	void ClearDocument();

private:
	enum PlayPauseState
	{
		STATE_PLAY,
		STATE_PAUSE
	};

	enum PlayPauseState  _playPauseState;

	PassiveTaskQueue _pvTaskQueue;

public:
	afx_msg void OnClose();


	afx_msg void OnTimer(UINT_PTR nIDEvent);
};


