#pragma once

#include <functional>

// PlayTrackBar

class PlayTrackBar : public CWnd
{
	DECLARE_DYNAMIC(PlayTrackBar)

public:
	PlayTrackBar();
	virtual ~PlayTrackBar();
	void SetDispWnd(double rightPos, double wndWidth);
	void RegisterSeek(std::function<void(double)>, void *);


protected:
	DECLARE_MESSAGE_MAP()
};


