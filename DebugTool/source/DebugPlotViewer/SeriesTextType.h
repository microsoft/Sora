#pragma once

#include "SeriesObj.h"

class TextTypeSeriesProp : public SeriesProp
{
public:
	virtual size_t DataSize() = 0;
	virtual char * GetData(size_t) = 0;
};
