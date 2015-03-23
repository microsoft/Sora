#include "stdafx.h"
#include <math.h>
#include "BitmapTypeSettings.h"

double SettingGetBitmapRangeMax(bool isLog)
{
	double maxNormal = (double)INT_MAX * 10.0;
	return isLog ? log10(maxNormal) : maxNormal;
}

double SettingGetBitmapRangeMin(bool isLog)
{
	double maxNormal = (double)INT_MAX * 10.0;
	double minNormal = (double)INT_MIN * 10.0;
	return isLog ? -log10(maxNormal) : minNormal;
}

double SettingGetBitmapRangeHalfDelta(bool isLog)
{
	if (isLog)
		return 0.001;
	else
		return 1.0;
}
