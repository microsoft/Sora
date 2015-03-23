#include "PlotWndPropTextType.h"

class PlotWndPropText : public PlotWndPropTextType
{
protected:
	virtual char * GetText(size_t index);
	virtual HRESULT CreateElementPlot(IXMLDOMDocument *pDom, IXMLDOMElement *pe);
	virtual BaseProperty * GetPropertyPage();
};
