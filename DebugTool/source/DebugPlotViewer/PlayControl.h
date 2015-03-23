#pragma once

#include "DebugPlotViewerDoc.h"
#include "ControllerWnd.h"

class CPlayControlWnd : public CDockablePane
{
public:
	CPlayControlWnd();
	virtual ~CPlayControlWnd();


protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnApp(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	CDebugPlotViewerDoc * doc;
	//ControllerWnd controllerWnd;


	std::vector<ControllerWnd *> controllerWnds;
	std::vector<DebuggingProcessProp *> process;

	// Layout
// layout
	int pidLeft;
	int pidRight;
	int barLeft;
	int barRight;
	//int speedUpLeft;
	//int speedUpRight;
	//int speedDownLeft;
	//int speedDownRight;
	int playLeft;
	int playRight;
	//int autoPlayLeft;
	//int autoPlayRight;

	// Layout Functions
	void AdjustLayout();
	void SetLayout(ControllerWnd * controllerWnd);
	void CalcYLayout(CRect * rect, int index);
	void CalcClientRect(CRect * rect);
	void CalcCaptionRect(CRect * rect, int index);
	void CalcBarRect(CRect * rect, int index);
	//void CalcSpeedUpButtonRect(CRect * rect, int index);
	//void CalcSpeedDownButtonRect(CRect * rect, int index);
	void CalcPlayButtonRect(CRect * rect, int index);

	void UpdateProcessList();
	virtual void PostNcDestroy();
};




