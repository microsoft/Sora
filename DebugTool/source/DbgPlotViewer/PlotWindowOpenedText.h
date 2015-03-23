#pragma once

#include <memory>
#include "PlotWindowOpenedTextType.h"
#include "TaskQueue.h"

class PlotWindowOpenedText : public PlotWindowOpenedTextType
{
public:
	PlotWindowOpenedText(std::shared_ptr<ProcessOpened>);
	~PlotWindowOpenedText();
	const wchar_t * GetTypeName();

protected:
	virtual bool Accept(std::shared_ptr<ChannelOpened>, CPoint pointIn, CPoint & pointOut);
	virtual HRESULT AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent);
	virtual HRESULT LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement);
};