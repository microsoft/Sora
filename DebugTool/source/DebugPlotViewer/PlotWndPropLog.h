#pragma once

#include "PlotWndProp.h"
#include "BaseProperty.h"
#include "PlotWndPropTextType.h"

class PlotWndPropLog : public PlotWndPropTextType
{
protected:
	virtual char * GetText(size_t index);
	virtual HRESULT CreateElementPlot(IXMLDOMDocument *pDom, IXMLDOMElement *pe);
	virtual BaseProperty * GetPropertyPage();
};
