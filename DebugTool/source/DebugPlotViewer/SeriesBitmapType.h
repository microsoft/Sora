#pragma once

#include <Windows.h>
#include "SeriesObj.h"

class BitmapTypeSeriesProp : public SeriesProp
{
public:
	virtual bool GetTimeStamp(size_t index, unsigned long long & out) = 0;
	virtual size_t DataSize() = 0;
};
