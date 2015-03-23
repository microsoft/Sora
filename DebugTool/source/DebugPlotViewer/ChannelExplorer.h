#pragma once

#include "ViewTree.h"
#include "DebugPlotViewerDoc.h"

class CChannelExplorerWnd : public CDockablePane
{
public:
	CChannelExplorerWnd();
	virtual ~CChannelExplorerWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnApp(WPARAM wParam, LPARAM lParam);
	
	DECLARE_MESSAGE_MAP()

protected:
	//virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT * pResult);

private:
	CViewTree m_channelView;
	CImageList m_channelViewImages;

	void AdjustLayout();

	enum ChildWndId {
		TREECTRL_ID = 4,
	};

	CDebugPlotViewerDoc * doc;

	static UINT UpdateChannelTree(LPVOID lParam);
	void BuildViewTreeAndSwap();
	std::vector<DebuggingProcessProp *> * treeData;
public:
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};
