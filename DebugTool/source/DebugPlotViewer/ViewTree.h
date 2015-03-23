
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CViewTree window

#include "AppMessage.h"
#include "DebugPlotViewerDoc.h"
#include "SeriesObj.h"
#include <windows.h>
#include <hash_map>
#include <map>
#include <string>
#include "ShareMemHelper.h"
//#include "_share_mem_if.h"
#include "TargetTestable.h"

class CViewTree : public CTreeCtrl
{
// Construction
public:
	CViewTree();

// Overrides
protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

// Implementation
public:
	virtual ~CViewTree();
private:
	bool isDragging;
	CWnd * lastFoundWnd;
	TargetTestable * _lastTestable;

	void HighlightWindow(CWnd * wnd, bool highlight);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg LRESULT OnApp(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCloseChannel();
	afx_msg void OnChannelSaveSelection();
	afx_msg void OnChannelSaveAll();
	afx_msg void OnTextChannelExport();
	afx_msg void OnTextChannelClose();
	void BuildTree();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
private:
	enum TIMER_ID {
		TIMER_UPDATE_TREE,
		TIMER_UPDATE_GRAPH,
	};

	void UpdateProcessTree(DebuggingProcessProp * prop);
	void UpdateChannel(HTREEITEM item, SeriesProp * prop);
	CDebugPlotViewerDoc * doc;

	SeriesProp * dragSeriesProp;
	SeriesProp * contextMenuProp;

	bool FindTargetWnd(TargetWnd * targetWnd);
	TargetTestable * FindTarget();
	void RebuildTree();
	
	void UpdateTree();

public:
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);

	std::map<int, DebuggingProcessProp *> allActiveProcessObjs;
	std::map<int, SeriesProp *> allActiveSeriesObjs;
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnNMRClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	
private:
	bool ShowFileDialog(CString & filename);
};

