#pragma once

#include "PlotWindowOpenedTextType.h"

class PlotWindowOpenedLog : public PlotWindowOpenedTextType
{
public:
	PlotWindowOpenedLog(std::shared_ptr<ProcessOpened>);
	~PlotWindowOpenedLog();
	
	virtual const wchar_t * GetTypeName();

protected:
	virtual bool Accept(std::shared_ptr<ChannelOpened>, CPoint pointIn, CPoint & pointOut);
	virtual HRESULT AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent);
	virtual HRESULT LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement);

};
