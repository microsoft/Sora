#include <Windows.h>
#include <stdio.h>
#include <math.h>
#include <SetupAPI.h>
#include "FreqOffset.h"
#include "SoraThread.h"
#include "SineWaveTest.h"
#include "Message.h"

static void LeastSquares(double * slope, double * intercept, double x[], double y[], int count)
{
	// pass 1
	double sumX = 0;
	double sumY = 0;
	double sumXY = 0;
	double sumX2 = 0;

	for (int i = 0; i < count; i++)
	{
		sumX += x[i];
		sumX2 += x[i] * x[i];
		sumY += y[i];
		sumXY += x[i] * y[i];
	}

	double interceptVal = ((sumX2*sumY - sumX*sumXY) / (count*sumX2 - sumX*sumX));
	double slopeVal = ((count*sumXY - sumX*sumY) / (count*sumX2 - sumX*sumX));

	double maxDeltaY = 0.0;
	int mI = 0;

	for (int i = 0; i < count; i++)
	{
		double yVal = x[i] * slopeVal + interceptVal;
		double deltaY = abs(yVal - y[i]);
		
		if (deltaY > maxDeltaY)
		{
			maxDeltaY = deltaY;
			mI = i;
		}
	}

	// pass 2
	sumX = 0;
	sumY = 0;
	sumXY = 0;
	sumX2 = 0;

	for (int i = 0; i < count; i++)
	{
		if (i == mI)
			continue;

		sumX += x[i];
		sumX2 += x[i] * x[i];
		sumY += y[i];
		sumXY += x[i] * y[i];
	}

	interceptVal = ((sumX2*sumY - sumX*sumXY) / ( (count-1) *sumX2 - sumX*sumX));
	slopeVal = (((count-1)*sumXY - sumX*sumY) / ( (count-1) *sumX2 - sumX*sumX));

	*slope = slopeVal;
	*intercept = interceptVal;
}


FrequencyOffset::FrequencyOffset(Radio * radio)
{

	logger = Logger::GetLogger(L"alg");

	Reset();

	const int FREQ_MIN = -65536;
	const int FREQ_MAX = 65536;
	const int FREQ_STEP = 65536;

	for (int freqOut = FREQ_MIN; freqOut <= FREQ_MAX; freqOut += FREQ_STEP)
	{
		for (int freq = freqOut - 10*256; freq <= freqOut + 10*256; freq += 256)
		{
			x.push_back(freq);
		}
	}

	y.resize(x.size(), 0.0);
}

void FrequencyOffset::Reset()
{
	index = 0;
}

bool FrequencyOffset::SampleFinished()
{
	return index == x.size();
}

void FrequencyOffset::SetNextX()
{
	if (index < x.size())
	{
		Radio::Current()->SetConfig(Radio::FREQ_OFFSET, (long long)x[index]);
	}
}

void FrequencyOffset::SetNextY(double yValue)
{
	if (index < y.size())
	{
		y[index] = yValue;
		index++;

		logger->Log(LOG_DEFAULT, L"Result central frequency is %f.\n", yValue);
	}
}

HRESULT FrequencyOffset::Calibration()
{
	HRESULT ret = -1;

	LeastSquares(&slope, &intercept, &x[0], &y[0], x.size());

	logger->Log(LOG_INFO, L"Calibration result:\r\ny = %fx + %f\r\n", slope, intercept);

	Radio::Current()->SetCalibrationLineSlope(slope);
	Radio::Current()->SetCalibrationLineIntercept(intercept);

	//ret = AddCalibrationToRegistry(slope, intercept);
	return ret; 
}
