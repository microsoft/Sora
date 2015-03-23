#pragma once


// SeriesPropertySheet

class SeriesPropertySheet : public CMFCPropertyGridCtrl
{
	DECLARE_DYNAMIC(SeriesPropertySheet)

public:
	SeriesPropertySheet();
	virtual ~SeriesPropertySheet();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


