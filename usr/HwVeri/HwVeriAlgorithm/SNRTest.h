#ifndef _SNR_TEST_H_
#define _SNR_TEST_H_

#include "Complex1.h"
#include "Send.h"
#include "Receive.h"
#include "AGC.h"
#include "Plot.h"

class SNRDataProcessor : public IDataProcessor
{
public:
	void ProcessData(PRX_BLOCK, int);

public:
	SNRDataProcessor(HWND hWnd);
	~SNRDataProcessor();

	void Init();
	void Deinit();

	void SetAutoGain(bool autogain);

private:
	HWND m_hWnd;
	char * m_filename;

	int m_channelEnergy;
	int m_channelConstel;

	static const int DUMP_SAMPLE_MAX = 256*1024;//64*1024/sizeof(COMPLEX16);

	int dumpSampleCount;
	COMPLEX16 dumpRawData[DUMP_SAMPLE_MAX];
	Complex dumpData[DUMP_SAMPLE_MAX];
	double energy[DUMP_SAMPLE_MAX];

	bool m_offlineDone;

	AGC * agc;

	Series<double> * seriesOverview;
	CAnimWnd * wndOverview;
	DrawLine * drawline;
	int overviewSnapshotCnt;

	Series<COMPLEX> * seriesConstel;
	CAnimWnd * wndConstel;
	DrawDots * drawdots;
	int constelSnapshotCnt;

	PlotImpl<double> * plotOverview;
	PlotImpl<COMPLEX> * plotConstel;

	PlotPlayer * plotPlayer;

	static const int NUM_16M = 16*1024*1024;
	static const int DUMP_BUF_LEN = NUM_16M - (NUM_16M % sizeof(RX_BLOCK));
	char * dumpBuf;
	int dumpBufValidCnt;

	bool autogain;
	bool lastAutoGain;

	Logger * logger;

	static const int EGY_AVG_COUNT = 512;
	static const int EGY_PLOT_COUNT = DUMP_SAMPLE_MAX/EGY_AVG_COUNT;
	double egyDataPlot[EGY_PLOT_COUNT];
};

class SNRDataReader : public IDataReader
{
public:
	int ReadData(char *, int);

public:
	SNRDataReader(const wchar_t * filename);
	~SNRDataReader();
	void Reset();
private:
	bool m_bFirstRead;
	wchar_t * m_fileName;

	void StoreFileName(const wchar_t * filename);
	wchar_t * GetFileName();
	void ReleaseFileName();

	Logger * logger;
};

#endif
