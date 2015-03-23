#pragma once

// AutoLayoutPanel

class AutoLayoutPanel : public CDockablePane
{
	DECLARE_DYNAMIC(AutoLayoutPanel)

public:
	AutoLayoutPanel();
	virtual ~AutoLayoutPanel();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

private:
	static const int ID_SLIDER = 1;
	CSliderCtrl slider;

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
