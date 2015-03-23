#pragma once

#include <memory>
#include <vector>
#include "BaseProperty.h"
#include "PlotWindowOpenedLineType.h"
#include "TaskQueue.h"
#include "TaskSimple.h"

class PlotWindowOpenedLine : public PlotWindowOpenedLineType
{
public:
	PlotWindowOpenedLine(std::shared_ptr<ProcessOpened>);
	~PlotWindowOpenedLine();
	virtual std::shared_ptr<BaseProperty> CreatePropertyPage();
	virtual bool PlotWindowOpenedLine::Accept(std::shared_ptr<ChannelOpened>, CPoint pointIn, CPoint & pointOut);
	virtual HRESULT AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent);
	virtual HRESULT LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement);
	virtual const wchar_t * GetTypeName();

protected:
	virtual bool IsRangeSettingEnabled();
	virtual void OnMouseWheel(bool bIsUp);
};
