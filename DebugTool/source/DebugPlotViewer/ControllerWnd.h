#pragma once

#include <vector>
#include "SeriesObj.h"
#include "AppMessage.h"
#include "SeriesObj.h"
#include "CustomButton.h"

// ControllerWnd

class ControllerWnd : public CWnd
{
	DECLARE_DYNAMIC(ControllerWnd)

public:
	ControllerWnd();
	virtual ~ControllerWnd();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	std::vector<NewData *> data;

	static const int OVERVIEW_MARGIN_TOP = 5;
	static const int OVERVIEW_MARGIN_BOTTOM = 2;

	// layout
	// Ops...
	void SetLayout(
		int pidLeft,
		int pidRight,
		int barLeft,
		int barRight,
		//int speedUpLeft,
		//int speedUpRight,
		//int speedDownLeft,
		//int speedDownRight,
		int playLeft,
		int playRight
		//int autoPlayLeft,
		//int autoPlayRight		
	);

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

private:
	// mouse control
	CustomButton buttonPlay;
	//CustomButton buttonSpeedUp;
	//CustomButton buttonSpeedDown;
	//CustomButton buttonAutoPlay;

	static const int ID_PLAY_BUTTON = 3;
	//static const int ID_SPEED_UP_BUTTON = 4;
	//static const int ID_SPEED_DOWN_BUTTON = 5;
	//static const int ID_AUTO_PLAY_BUTTON = 6;

	// Draw Functions
	void DrawCaption(Graphics * g);
	void DrawGraphTrackbar(Graphics * g);
	void DrawSourceTrackbar(Graphics * g);
	void DrawSourceTrackbarTop(Graphics * g, CRect * rect);
	void DrawSourceTrackbarBottom(Graphics * g, CRect * rect);

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg LRESULT OnApp(WPARAM wParam, LPARAM lParam);

public:
	void SetProcess(DebuggingProcessProp * process);
	void ReleaseProcess();

private:
	DebuggingProcessProp * process;
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void PostNcDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

private:
	double * overviewDispData;
	int overviewDispDataLen;
public:
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};



