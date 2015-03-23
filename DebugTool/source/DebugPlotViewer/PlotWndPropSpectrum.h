#pragma once

#include "BitmapTypePlotWndProp.h"
#include "PlotWndPropLineType.h"

class PlotWndPropSpectrum : public PlotWndPropLineType 
{
public:
	PlotWndPropSpectrum();
	virtual HRESULT CreateElementPlot(IXMLDOMDocument *pDom,IXMLDOMElement *pe);
	virtual BaseProperty * GetPropertyPage();
	virtual bool Accept(SeriesProp * series);
	virtual void ModifyDataRange(size_t & range);
};

