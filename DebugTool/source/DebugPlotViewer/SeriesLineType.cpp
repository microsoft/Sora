#include "stdafx.h"
#include "SeriesLineType.h"
#include "PlotWndPropLineType.h"
#include "HelperFunc.h"

bool LineTypeSeriesProp::CalcMaxMin(const CRect & rect, size_t start, size_t size, double & max, double &min)
{
	auto plotWndProp = (PlotWndPropLineType *)(this->GetPlotWndProp());
	double maxValue, minValue;

	int width = rect.Width();

	bool initialized = false;

	for (int i = 0; i < width; i++)
	{
		size_t mappedIdx = (start+size) - i * size / width;
		if (mappedIdx >= this->DataSize())
			continue;

		int y;
		bool succ = this->GetData(mappedIdx, y);
		double yT = ::TransformCoordinate(y, plotWndProp->isLog);

		if (!initialized)
		{
			maxValue = yT;
			minValue = yT;
			initialized = true;
		}
		else
		{
			maxValue = max(maxValue, yT);
			minValue = min(minValue, yT);
		}
	}

	if (initialized)
	{
		max = maxValue;
		min = minValue;
	}

	return initialized;
}
