#pragma once

#include "SeriesBitmapType.h"

class LineTypeSeriesProp : public BitmapTypeSeriesProp
{
public:
	virtual bool CalcMaxMin(const CRect & rect, size_t start, size_t size, double & max, double &min);
	virtual void Draw(Graphics * g, const CRect & rect, size_t start, size_t size) = 0;
	virtual bool GetData(size_t index, int & out) = 0;
	virtual bool GetMaxMinRange(size_t in, size_t & out) = 0;
};

