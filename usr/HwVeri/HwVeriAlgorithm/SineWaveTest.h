#ifndef _SINE_WAVE_TEST_H_
#define _SINE_WAVE_TEST_H_

//#include "Logger.h"
//#include "UMXSample.h"

#include "Send.h"
#include "Receive.h"
#include "Complex1.h"
#include "Average.h"
#include "AGC.h"
#include "Plot.h"

class SineDataProcessor : public IDataProcessor
{
public:
	void ProcessData(PRX_BLOCK, int);

public:
	SineDataProcessor(HWND, Radio * radio);
	~SineDataProcessor();
	void Init();
	void Deinit();

	void SetAutoGain(bool autogain);

private:
	HWND m_hWnd;
// UI operation
	static const int PKG_MAX = 500;
	static const int DUMP_SAMPLE_MAX = 256*1024;	//256K complex16, 1M bytes

	void removeDCOffset(Complex * Data, int SampleCount);
	void calEnergy(Complex* input, double *output, int sampleNum);
	int searchPkg(double* input, int *pkg_start, int *pkg_end);
	int detectOnePeriod(Complex* dumpData, int pkg_end, int start_peak, int re_flag, double *max, double *min);
	void detectPeriod(Complex* dumpData, int pkg_num, int total, int re_flag);
	double detectPhaseDiff(Complex* dumpData, int pkg_num, int total);
	double CentralFreq(int pkgNum);

	static const int SEND_FREQ = (1000*1000);	// 1MHz

	int dumpSampleCount;
	COMPLEX16 dumpRawData[DUMP_SAMPLE_MAX];
	Complex dumpData[DUMP_SAMPLE_MAX];
	double egyData[DUMP_SAMPLE_MAX];
	double egyDataRaw[DUMP_SAMPLE_MAX];

	static const int EGY_AVG_COUNT = 512;
	static const int EGY_PLOT_COUNT = DUMP_SAMPLE_MAX/EGY_AVG_COUNT;
	double egyDataPlot[EGY_PLOT_COUNT];

	int overviewSnapshotCnt;

	int pkg_total_num;
	int pkg_start[PKG_MAX];
	int pkg_end[PKG_MAX];
	bool pkgValid[PKG_MAX];

	double period_re[PKG_MAX];
	double period_im[PKG_MAX];

	double A_re[PKG_MAX];
	double A_im[PKG_MAX];
	double phaseDiff[PKG_MAX];

	void UpdateUI(double, double, double, double);

	AGC * agc;

	Series<double> * seriesOverview;
	CAnimWnd * wndOverview;
	DrawLine * drawline;

	Series<COMPLEX> * seriesConstel;
	CAnimWnd * wndConstel;
	DrawDots * drawdots;

	PlotImpl<double> * plotOverview;
	PlotImpl<COMPLEX> * plotConstel;

	PlotPlayer * plotPlayer;

	static const int NUM_16M = 16*1024*1024;
	static const int DUMP_BUF_LEN = NUM_16M - (NUM_16M % sizeof(RX_BLOCK));
	char * dumpBuf;
	int dumpBufValidCnt;
	
	bool autogain;
	bool lastAutoGain;

	static const int SAMPLE_FREQ = 40 * 1000 * 1000;
	static const int FFT_LEN = 32768;
	static const int FFT_ALIGN = 0x0f;
	char * fftInput;
	char * fftOutput;
	COMPLEX16 * fftInputStart;
	COMPLEX16 * fftOutputStart;

	Logger * logger;

	static const int STATUS_TOTAL_PKG_NUM = 256;
	int statusTotalPkgNum;
	int status40MPkgNum;
	int status44MPkgNum;
	int statusValidPkgNum;

	bool sampleRateNotMatchMessagePrinted;

	bool saturation;
};

class SineDataReader : public IDataReader
{
public:
	SineDataReader();
	int ReadData(char *, int);
	void Reset();
private:
	bool m_bFirstRead;
};

#endif
