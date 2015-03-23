#pragma once

#include <Windows.h>
#include "Complex1.h"
#include "Radio.h"
#include "Average.h"

class AGC
{
public:
	AGC();
	~AGC();

	void Reset();
	void CalcData(Complex * buf, int count);
	int AdjustByRawData(int db);

	double GetRatioByRange(double min, double max);
	double SearchSampleByTop(double ratio);
	double SearchSampleByBottom(double ratio);

private:
	int lastRxDB;

	static const int HISTO_LEN = 1000;
	static const int SAMPLE_VALUE_MAX = 32767;

	int histogram[HISTO_LEN];
	int integral[HISTO_LEN];

	FIFO_Average<double> * satRatioAverager;
	FIFO_Average<double> * highEnergyRatioAverager;
	FIFO_Average<double> * gainAverager;

	Logger * logger;
};


