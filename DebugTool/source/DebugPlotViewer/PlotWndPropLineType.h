#pragma once

#pragma once

#include "BitmapTypePlotWndProp.h"

class PlotWndPropLineType : public BitmapTypePlotWndProp
{
public:	// serialize
	bool autoScale;
	bool showGrid;
	bool isLog;

	bool autoScaleReset;

	double GetDispMaxValue();
	double GetDispMinValue();
	void SetDispMaxValue(double value);
	void SetDispMinValue(double value);
	double GetMaxValue();
	double GetMinValue();
	void SetMaxValue(double value);
	void SetMinValue(double value);

public:
	double maxValue;
	double minValue;

public:
	PlotWndPropLineType();
	virtual HRESULT CreateElementPlot(IXMLDOMDocument *pDom,IXMLDOMElement *pe) = 0;
	virtual BaseProperty * GetPropertyPage() = 0;
	Bitmap * CreateBitmap();
	void SeekTextWnd();
	
	// methods for draw graphics
public:
	Gdiplus::REAL GetClientY(double y, const CRect & clientRect);
private:
	void DrawGrid(Graphics * g, const CRect &);
	int CalcOptimizedGridSize(const CRect & rect);
	void DrawGridLine(Graphics * g, __int64 yData, const CRect &);
	//void DrawXAxis(Graphics * g, const CRect &);
	void DrawBackground(Graphics * g, const CRect &);
	size_t MaxDataSize();
};


