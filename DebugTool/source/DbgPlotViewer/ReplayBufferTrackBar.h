#pragma once

#include "Event.h"

// ReplayBufferTrackBar

class ReplayBufferTrackBar : public CWnd
{
	DECLARE_DYNAMIC(ReplayBufferTrackBar)

public:
	ReplayBufferTrackBar();
	virtual ~ReplayBufferTrackBar();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

public:
	struct SeekEvent
	{
		double pos;
	};

	SoraDbgPlot::Event::Event<SeekEvent> EventSeek;

public:
	void SetViewWnd(double right, double range);
private:
	double _right;
	double _range;
};



