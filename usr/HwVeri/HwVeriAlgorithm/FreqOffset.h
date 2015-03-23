#pragma once

#include "SoraThread.h"
#include "Radio.h"
#include "SineWaveTest.h"

#include <vector>

class FrequencyOffset
{
public:
	FrequencyOffset(Radio * radio);
	void Reset();
	bool SampleFinished();
	void SetNextX();
	void SetNextY(double y);
	HRESULT Calibration();
private:

	std::vector<double> x;
	std::vector<double> y;
	unsigned int index;

	double slope;
	double intercept;

	Logger * logger;
};

