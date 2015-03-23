
#pragma once

#include "PlotWndArea.h"
//#include "DebugPlotViewerDoc.h"
#include "BaseProperty.h"
#include "LineSeriesProperty.h"

class CPropertiesToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CPropertiesWnd : public CDockablePane
{
// Construction
public:
	CPropertiesWnd();

	void AdjustLayout();
	ObjProp * GetCurrentObj();

// Attributes
public:
	//void SetVSDotNetLook(BOOL bSet)
	//{
	//	plotWndAreaProperty.SetVSDotNetLook(bSet);
	//	plotWndAreaProperty.SetGroupNameFullWidth(bSet);
	//}

protected:
	CFont m_fntPropList;
	//ViewWndProperty plotWndAreaProperty;
	//LineSeriesProperty plotWndAreaProperty;

	BaseProperty * currentPropertyGrid;

// Implementation
public:
	virtual ~CPropertiesWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnExpandAllProperties();
	afx_msg void OnUpdateExpandAllProperties(CCmdUI* pCmdUI);
	afx_msg void OnSortProperties();
	afx_msg void OnUpdateSortProperties(CCmdUI* pCmdUI);
	afx_msg void OnProperties1();
	afx_msg void OnUpdateProperties1(CCmdUI* pCmdUI);
	afx_msg void OnProperties2();
	afx_msg void OnUpdateProperties2(CCmdUI* pCmdUI);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg LRESULT OnApp(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

	void InitPropList();
	void SetPropListFont();
	void ChangeProperty(BaseProperty * property);
public:
	afx_msg void OnPaint();
};


