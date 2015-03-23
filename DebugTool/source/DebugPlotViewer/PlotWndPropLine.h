#pragma once

#include "BitmapTypePlotWndProp.h"
#include "PlotWndPropLineType.h"

class PlotWndPropLine : public PlotWndPropLineType
{
public:
	virtual HRESULT CreateElementPlot(IXMLDOMDocument *pDom,IXMLDOMElement *pe);
	virtual BaseProperty * GetPropertyPage();
	virtual bool Accept(SeriesProp * series);
};


