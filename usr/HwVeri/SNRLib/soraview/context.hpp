#pragma once

#include <windows.h>
#include "Radio.h"

#define MAX_SAMPLE_PER_SPOT 1024
static int g_spotTable[] = {6, 12, 5, 10, 14, 1, 7, 3, 14, 9, 3, 4, 1, 6, 3, 1, 13, 15, 4, 8, 5, 9, 6, 0, 12, 11, 5, 6, 6, 14, 15, 1, 15, 3, 9, 0, 3, 11, 12, 13, 7, 2, 2, 4, 0, 7, 8, 1, 13, 6, 12, 4, 14, 12, 6, 5, 6, 0, 15, 2, 0, 9, 7, 1, 12, 6, 5, 15, 4, 1, 1, 9, 12, 8, 14, 2, 8, 6, 12, 3, 11, 12, 14, 8, 11, 2, 4, 4, 8, 7, 8, 9, 8, 5, 15, 5, 1, 9, 10, 11, 15, 0, 3, 0, 9, 0, 14, 3, 10, 10, 0, 0, 1, 1, 10, 3, 7, 1, 13, 7, 0, 3, 12, 15, 3, 1, 12, 11, 2, 13, 11, 7, 0, 8, 9, 14, 10, 0, 11, 10, 13, 10, 2, 7, 9, 12, 15, 13, 15, 1, 7, 10, 10, 9, 15, 14, 11, 5, 12, 9, 1, 13, 1, 15, 12, 9, 0, 12, 4, 3, 4, 7, 2, 4, 4, 0, 8, 8, 1, 5, 8, 11, 10, 10, 5, 13, 8, 3, 3, 4, 15, 13, 6, 8, 5, 12, 6, 0, 0, 11, 5, 14, 6, 14, 8, 14, 0, 7, 9, 0, 3, 11, 12, 14, 8, 13, 13, 11, 15, 8, 7, 14, 9, 8, 11, 15, 5, 10, 13, 8, 3, 8, 11, 0, 5, 3, 9, 2, 14, 13, 5, 15, 4, 1, 1, 9, 12, 8, 14, 2, 8, 6, 3, 12, 4, 12, 14, 8, 11, 2, 4, 11, 7, 8, 7, 6, 7, 10, 0, 10, 10, 13, 8, 2, 7, 4, 3, 5, 3, 2, 13, 1, 14, 13, 0, 10, 6, 1, 10, 3, 7, 1, 13, 7, 0, 3, 12, 15, 3, 1, 3, 4, 13, 13, 11, 7, 0, 8, 9, 1, 5, 15, 4, 5, 2, 5, 13, 8, 3, 3, 5, 15, 13, 2, 9, 14, 13, 2, 5, 9, 10, 11, 14, 0, 8, 8, 1, 15, 12, 9, 0, 12, 4, 3, 4, 7, 2, 4, 11, 15, 7, 8, 1, 5, 8, 11, 10, 5, 10, 2, 7, 12, 12, 11, 0, 2, 3, 9, 14, 14, 13, 10, 0, 10, 11, 14, 15, 7, 9, 4, 6, 12, 10, 14, 3, 11, 12, 14, 8, 13, 13, 11, 15, 8, 7, 14, 6, 7, 4, 15, 5, 10, 13, 8, 3, 7, 4, 15, 10, 12, 6, 13, 1, 2, 13, 0, 10, 6, 1, 0, 14, 9, 4, 7, 3, 5, 14, 3, 15, 9, 12, 2, 11, 2, 4, 11, 7, 8, 7, 6, 7, 10, 0, 10, 5, 2, 7, 2, 7, 4, 3, 5, 3, 13, 2, 14, 1, 2, 15, 5, 9, 14, 11, 14, 9, 8, 8, 12, 6, 14, 3, 1, 6, 3, 8, 7, 7, 6, 5, 6, 0, 8, 9, 1, 5, 15, 4, 5, 2, 5, 13, 8, 12, 12, 10, 15, 13, 2, 9, 14, 13, 13, 10, 6, 5, 4, 1, 15, 7, 7, 4, 6, 14, 10, 14, 9, 15, 8, 7, 13, 9, 10, 9, 6, 4, 12, 15, 9, 8, 11, 10, 5, 10, 2, 7, 12, 12, 11, 0, 2, 12, 6, 1, 14, 13, 10, 0, 10, 11, 1, 0, 8, 6, 11, 9, 3, 5, 1, 3, 15, 8, 12, 2, 6, 7, 9, 6, 9, 3, 0, 8, 1, 1, 4, 10, 10, 13, 8, 3, 7, 4, 15, 10, 12, 6, 13, 1, 2, 2, 15, 5, 6, 1, 0, 14, 9, 4, 8, 12, 10, 1, 12, 0, 6, 3, 13, 7, 7, 9, 5, 6, 12, 4, 8, 1, 6, 11, 5, 3, 1, 7, 15, 8, 5, 3, 5, 3, 13, 2, 14, 1, 2, 15, 5, 9, 14, 4, 1, 6, 8, 8, 12, 6, 14, 3, 14, 9, 12, 7, 8, 8, 9, 10, 9, 6, 4, 8, 15, 9, 4, 1, 3, 1, 5, 0, 7, 5, 11, 0, 3, 11, 2, 9, 14, 13, 13, 10, 6, 5, 4, 1, 15, 7, 7, 11, 9, 1, 10, 14, 9, 15, 8, 7, 2, 6, 5, 6, 9, 11, 3, 0, 6, 1, 1, 3, 10, 10, 15, 7, 5, 11, 10, 12, 4, 6, 6, 10, 15, 12, 3, 0, 10, 11, 1, 0, 8, 6, 11, 9, 3, 5, 1, 12, 0, 7, 12, 2, 6, 7, 9, 6, 6, 12, 15, 7, 14, 14, 11, 5, 5, 1, 7, 5, 8, 5, 3, 0, 6, 6, 13, 0, 3, 7, 3, 13, 7, 12, 7, 14, 9, 7, 8, 12, 11, 1, 12, 1, 6, 3, 14, 8, 8, 5, 5, 6, 15, 4, 8, 0, 9, 4, 10, 12, 14, 8, 0, 7, 9, 11, 0, 7, 11, 2, 14, 10, 7, 3, 12, 8, 1, 10, 1, 5, 1, 8, 2, 9, 14, 3, 8, 9, 12, 12, 7, 7, 3, 5, 6, 0, 4, 8, 15, 9, 4, 5, 3, 1, 11, 15, 8, 15, 11, 0, 7, 11, 2, 8, 10, 7, 12, 3, 7, 5, 10, 1, 10, 1, 8, 7, 13, 2, 1, 6, 4};



#include "Message.h"
#include "Average.h"
#include "Plot.h"

#define NO_PRINTF

void Printf(const char * format, ...)
{
#if defined NO_PRINTF
	return; 
#endif

	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
}


typedef struct SPOT
{
	COMPLEX samples[MAX_SAMPLE_PER_SPOT];
	int sampleNum;

	void Reset()
	{
		sampleNum = 0;
	}
	void InsertSample(COMPLEX & complex)
	{
		samples[sampleNum] = complex;
		sampleNum++;
	}
} SPOT;
typedef struct RX_SAMPLE
{
	SPOT spots[16];
	int sampleNum;
	static const int MAX_SIZE = 864 /*768*/;
	//static const int MAX_SIZE = sizeof(g_spotTable)/sizeof(int);
	void Reset() {
		sampleNum = 0;
		for (int i = 0; i < 16; i++)
		{
			spots[i].Reset();
		}
	}
	void InsertSample(int spot, COMPLEX & complex)
	{
		spots[spot].InsertSample(complex);
		sampleNum++;
	}
	bool IsFull()
	{
		//const int MAX_SIZE = ;
		return sampleNum >= MAX_SIZE;
	}
} RX_SAMPLE;
class CContext {
public:
	int  id_;
	bool bPause_;
	bool bAbort_;
	int  reset_;
	
	int frameCount_; // # of frame processed

	int frameStart_; // the linear sample count of the frame
	int LTStart_;    // the linear sample count of LT symbol start

	int state_; // 
	
	// radio and channel states
	double freqOffset_; // freq. offset
	COMPLEX ch_[64];	// channel coefficients

	double accum_r_;  // accumulate rotation due to central freq offset
	double accum_sr_; // accumulate rotation due to sample freq offset

//	COMPLEX pilot_[4];

	// frame states
	bool bHdrFlag_;
	int  datarate_;
	int  framelen_;
	int  symCount_;
	int  symIdx_;

	RX_SAMPLE samples_;

	HWND hWnd_;
	
	PlotImpl<COMPLEX> * snrPlot;

	LARGE_INTEGER lastPerformanceCount;
	LARGE_INTEGER performanceFrequency;
	long long lastPerformanceInterval;
	FILETIME lastKernelTime;
	FILETIME lastUserTime;

	FILETIME creationTime;
	FILETIME exitTime;
	FILETIME kernelTime;
	FILETIME userTime;

	CContext () {
		QueryPerformanceFrequency(&performanceFrequency);
		frameCount_ = 0;
		Reset ();
	}

	void Abort () {
		bAbort_  = true;
	}
	void Pause () {
		bPause_ = true;
	}
	
	void Reset () {
		bPause_ = false;
		bAbort_ = false;
		reset_  = true;

		state_  = STATE_CS;
		
		frameStart_ = 0;
		LTStart_    = 0;
		freqOffset_ = 0;

		datarate_ = -1;
		framelen_ = 0;
		symCount_ = 0;
		symIdx_   = 0;

		accum_r_  = 0;
		accum_sr_ = 0;
		samples_.Reset();
	}

	int State () { return state_; }

// Events
	void CarrierSense ( int frameStart, double aVal, double eVal, double nf ) {
		frameStart_ = frameStart;
		frameCount_ ++;

		// change states
		state_ = STATE_TSYNC;
		
		Printf ( "\nFrame detected at sample %d\n", frameStart_ );
		Printf ( "Frame count %d\n", frameCount_);
		Printf ( "Energy %.3f (%lf dB) mod %lf, AutoCorr %.3f\n", 
			      eVal, 
			      10*log10(eVal),
			      sqrt(eVal),
			      aVal);
		
		Printf ( "Noise floor %lf (%lf dB) mod %lf\n", nf,
				 10 * log10 (nf), sqrt (nf) );
		Printf ( "Coarse SNR %lf\n\n", 10 * log10 (eVal / nf));
	}

	void TimeSyncDone ( int nSample ) {
		LTStart_ = nSample;

		Printf ( "LT symbol found: %d\n", nSample );

		state_ = STATE_FSYNC;
	}

	void SetFreqOffset ( double deg ) {
		freqOffset_ = deg;

		//Printf ( "Freq offset %lf rad per sample or %.3lf KHz (40MHz sampling)\n", 
		//	 	 deg, 
		//	 	 deg * 20 * 1024 / 3.14159265359 );

	}

	void FreqSyncDone () {
		state_ = STATE_HDR;

		Printf( "Freq Sync done!\n");

	}

	void HeaderDone () {
		state_ = STATE_DATA;
	}

	void UpdateSNR(double snr)
	{

		double * snrSend = new double[1];
		*snrSend = snr;
		::SendNotifyMessage(hWnd_, WM_UPDATE_DATA, DATA_SNR, (LPARAM)snrSend);
	}

	void UpdateFreqOffset()
	{
		double * freqSend = new double[1];
		double sampleRate = (double)Radio::Current()->GetConfig(Radio::SAMPLE_RATE);
		*freqSend = freqOffset_ * (sampleRate/2) * 1000000 / 3.14159265359 / 2;
		::SendNotifyMessage(hWnd_, WM_UPDATE_DATA, DATA_FREQ_OFFSET, (LPARAM)freqSend);	
	}

	//void PlotData(int re, int im)
	//{
	//	::PlotData(g_channelConstel, re, im);
	//}
	
	void ShowPerformance()
	{
		LARGE_INTEGER performanceCount;

		QueryPerformanceCounter(&performanceCount);
		long long interval = performanceCount.QuadPart - lastPerformanceCount.QuadPart;
		long long intervalIncrement = interval - lastPerformanceInterval;

		lastPerformanceInterval = interval;
		lastPerformanceCount = performanceCount;

		printf("performance: %f %f\n", 
			(double)interval/performanceFrequency.QuadPart, 
			(double)intervalIncrement/performanceFrequency.QuadPart);

		HANDLE currentProcess = GetCurrentProcess();
		HANDLE currentThread = GetCurrentThread();
		DWORD priorityClass = GetPriorityClass(currentProcess);
		int priority =  GetThreadPriority(currentThread);
		printf("priority: %x %d\n", priorityClass, priority);

		FILETIME creationTime;
		FILETIME exitTime;
		FILETIME kernelTime;
		FILETIME userTime;
		BOOL ret = GetProcessTimes(
			currentProcess,
			&creationTime,
			&exitTime,
			&kernelTime,
			&userTime
		);

		if (ret)
		{
			ULARGE_INTEGER currentTime, lastTime;

			currentTime.HighPart = userTime.dwHighDateTime;
			currentTime.LowPart = userTime.dwLowDateTime;
			
			lastTime.HighPart = lastUserTime.dwHighDateTime;
			lastTime.LowPart = lastUserTime.dwLowDateTime;

			printf("user time: %f\n", double(currentTime.QuadPart - lastTime.QuadPart) / (10*1000*1000));

			currentTime.HighPart = kernelTime.dwHighDateTime;
			currentTime.LowPart = kernelTime.dwLowDateTime;
			
			lastTime.HighPart = lastKernelTime.dwHighDateTime;
			lastTime.LowPart = lastKernelTime.dwLowDateTime;

			printf("kernel time: %f\n", double(currentTime.QuadPart - lastTime.QuadPart) / (10*1000*1000));

			lastUserTime = userTime;
			lastKernelTime = kernelTime;
		}
		else
		{
			printf("GetProcessTimes error\n");
		}
	}
};

