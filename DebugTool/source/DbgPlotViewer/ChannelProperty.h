#pragma once

#include "BaseProperty.h"
#include "Event.h"

// ChannelProperty

class ChannelProperty : public BaseProperty
{
	DECLARE_DYNAMIC(ChannelProperty)

public:
	ChannelProperty(const std::wstring &, const std::wstring &, COLORREF);
	virtual ~ChannelProperty();

protected:
	virtual void OnCloseProperty();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPropertyChanged(CMFCPropertyGridProperty* pProp) const;


public:
	void SetColor(COLORREF color);
	SoraDbgPlot::Event::Event<COLORREF> EventColor;

private:
	std::wstring _typename;
	std::wstring _name;
	COLORREF _color;
};


