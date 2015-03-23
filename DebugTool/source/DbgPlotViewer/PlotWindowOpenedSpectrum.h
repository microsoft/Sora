#pragma once

#include <memory>
#include <vector>
#include "BaseProperty.h"
#include "PlotWindowOpenedLineType.h"
#include "ChannelOpened.h"
#include "ChannelOpenedSpectrum.h"
#include "ChannelOpenedText.h"
#include "TaskQueue.h"

class PlotWindowOpenedSpectrum : public PlotWindowOpenedLineType
{
public:
	PlotWindowOpenedSpectrum(std::shared_ptr<ProcessOpened>);
	~PlotWindowOpenedSpectrum();
	virtual std::shared_ptr<BaseProperty> CreatePropertyPage();
	virtual bool Accept(std::shared_ptr<ChannelOpened>, CPoint pointIn, CPoint &);
	virtual void ModifyDataRange(size_t & range);
	virtual HRESULT AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent);
	virtual HRESULT LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement);
	virtual const wchar_t * GetTypeName();
	virtual void OnMouseWheel(bool bIsUp);
};
