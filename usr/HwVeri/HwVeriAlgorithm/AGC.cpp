#include "AGC.h"
#include "Message.h"

AGC::AGC()
{
	logger = Logger::GetLogger(L"alg");

	lastRxDB = -1;

	satRatioAverager = new FIFO_Average<double>(5);
	highEnergyRatioAverager = new FIFO_Average<double>(5);
	gainAverager = new FIFO_Average<double>(10);
}

AGC::~AGC()
{
	delete satRatioAverager;
	delete highEnergyRatioAverager;
	delete gainAverager;
}

void AGC::Reset()
{
	lastRxDB = -1;
	satRatioAverager->Clear();
	highEnergyRatioAverager->Clear();
	gainAverager->Clear();
}

void AGC::CalcData(Complex * buf, int count)
{
	for (int i = 0; i < HISTO_LEN; i++)
	{
		histogram[i] = 0;
	}

	for (int i = 0; i < count; i++)
	{
		double re = buf[i].re;
		double im = buf[i].im;

		double radius = sqrt(re*re + im*im);
		int index = (int)(radius*HISTO_LEN/(SAMPLE_VALUE_MAX+1));
		index = min(HISTO_LEN-1, index);
		histogram[index]++;
	}

	integral[0] = histogram[0];
	for (int i = 1; i < HISTO_LEN; i++)
	{
		integral[i]	= histogram[i] + integral[i-1];
	}
}

double AGC::SearchSampleByTop(double ratio)
{
	return SearchSampleByBottom(1.0 - ratio);
}

double AGC::SearchSampleByBottom(double ratio)
{
	ratio = max(0.0, ratio);
	ratio = min(1.0, ratio);

	int total = integral[HISTO_LEN-1];
	int threshold = (int)(total * ratio);

	int sampleValue = HISTO_LEN-1;

	for (int i = 0; i < HISTO_LEN; i++)
	{
		if (integral[i] >= threshold)
		{
			sampleValue = i;
			break;
		}
	}

	return (double)sampleValue/HISTO_LEN;
}

double AGC::GetRatioByRange(double min, double max)
{
	if (min > max)
		return 0.0;

	min = max(min, 0.0);
	max = min(max, 1.0);

	int minIdx = (int)((HISTO_LEN-1) * min);
	int maxIdx = (int)((HISTO_LEN-1) * max);
	int count = integral[maxIdx] - integral[minIdx];

	int total = integral[HISTO_LEN-1];
	double ratio = (double)count/total;

	return ratio;
}

int AGC::AdjustByRawData(int db)
{
	double thresholdHigh, thresholdLow;
	if (db >= 32.0)
	{
		thresholdHigh = 0.90;
		thresholdLow = 0.25;
	}
	else
	{
		thresholdHigh = 0.85;
		thresholdLow = 0.55;
	}

	double saturateRatio = GetRatioByRange(thresholdHigh, 1.0);
	double saturateRatio2 = GetRatioByRange(thresholdLow, 1.0);

	satRatioAverager->Add(saturateRatio);
	highEnergyRatioAverager->Add(saturateRatio2);

	double avgSaturateRatio = satRatioAverager->GetAverage();
	double avgHighEnergyRatio = highEnergyRatioAverager->GetAverage();

	if (avgSaturateRatio > 0.03)	// saturation
	{
		db-=2;
	}
	else if (avgHighEnergyRatio < 0.03)	// signal
	{
		db+=2;
	}
	else
	{
		gainAverager->Add(db);
		return db;
	}

	gainAverager->Add(db);

	double dbSet = gainAverager->GetAverage();
	int dbInt = (int)dbSet;

	if (dbInt != lastRxDB)
	{
		lastRxDB = dbInt;
	}

	return dbInt;
}
