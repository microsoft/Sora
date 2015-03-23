#pragma once

#include "BitmapTypePlotWndProp.h"

class PlotWndPropDots : public BitmapTypePlotWndProp
{
//
private:
	static const size_t MAX_RANGE;

public:	// serialize
	bool autoScale;
	bool showGrid;
	bool luminescence;
	int dotShowCount;

	int GetDispMaxValue() {
		return this->maxValue;
	}

	void SetDispMaxValue(int value) {
		this->maxValue = value;
	}

	int GetMaxValue() {
		return this->maxValue;
	}

	void SetMaxValue(int value) {
		this->maxValue = value;
	}

	double autoScaleMaxValue;

	static const int MAX_DOTS_SHOWN = 1024;

private:
	int maxValue;

public:
	PlotWndPropDots();
	virtual HRESULT CreateElementPlot(IXMLDOMDocument *pDom, IXMLDOMElement *pe);
	virtual BaseProperty * GetPropertyPage();
	virtual Bitmap * CreateBitmap();
	virtual size_t MaxDataSize();

private:
	void DrawGrid(Graphics * g, const CRect & clientRect);
	void DrawBackground(Graphics * g);
	COMPLEX16 GetClientDot(COMPLEX16 data);
	void DrawDot(Graphics * g, COMPLEX16 data, Color& color);

public:
	virtual void ModifyDataRange(size_t & range);

public:
	virtual bool Accept(SeriesProp * series);
};
