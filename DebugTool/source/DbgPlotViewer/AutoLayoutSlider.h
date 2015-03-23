#pragma once


// AutoLayoutSlider

class AutoLayoutSlider : public CWnd
{
	DECLARE_DYNAMIC(AutoLayoutSlider)

public:
	AutoLayoutSlider();
	virtual ~AutoLayoutSlider();

	SoraDbgPlot::Event::Event<int> EventAutoScale;

private:
	CSliderCtrl _sliderCtrl;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual void PostNcDestroy();
};


