#pragma once

#include <Windows.h>
#include "LoggerImpl.h"
#include "TaskQueue.h"
#include "Radio.h"

#include "sora.h"
#include "SineWaveTest.h"
#include "SNRTest.h"
#include "FreqOffset.h"
#include "SoraThread.h"

#include "Average.h"


class Application
{
public:
	Application();
	~Application();
	void Init(HWND hwnd);
	void Deinit();

	void OnTimer();
	void ReconfigUI();
	void ReconfigUIDisableAll();
	void EnableUIByState();
	void EnableUISineTest();
	void EnableUISnrTest();
	void EnableUIModeSelection();
	void EnableUIDirectionSelection();
	void EnableUIParaSetting();
	void EnableStartButton(bool enable);

	void UpdateUIPara();

	// UI Operations
	void SetGain(int sel, int max);
	void OnGainSliderChange(int sel, int max);

	bool ReadCentrolFreqFromUIAndUpdate();
	void SetCentralFreq(unsigned int freq);
	void SetRxpa(int rxpa);
	void SavePara();
	void LoadPara();
	void SaveLog();
	void Dump();
	void AddDumpData(char * buf, int len);

	void Suggestion();

	void OnClickButtonStart();
	void OnClickCalibration();
	void OnSuggestion();

	void SelectModeSine();
	void SelectModeSnr();
	void SelectDirectionSend();
	void SelectDirectionReceive();
	void SetAutoGain(bool autogain);
	void SetSampleRate(int mHz);
	//void OnCalibrationDone();

	void UpdateData(UPDATE_DATA_TYPE type, double * data);
	void ClearData();

	void ProcessTaskMessage(TASK_TYPE type, void * context);

	void RefreshUI();
	void RefreshStatusBar();
	void ClearUIData();

	void DlgLog(int type, wchar_t * string);

	void SaveLastConfigFile();

	void LoadOpState();
	void SaveOpState();

	friend class IThreadControl;
	friend class CThreadControl;
private:
	HWND hwnd;
	Logger * logger;
	//TaskQueue * taskQueue;

	// state
	enum { TEST_LOOP, TEST_SINE, TEST_SNR } modes[8], mode;
	enum { SEND, RECEIVE } directions[8], direction;
	enum { START, STOP } actionStart;
	bool isAutoCalibrating;
	bool calibrationFirstSample;

	static const int MIN_RX_GAIN = 0x200;
	static const int MAX_RX_GAIN = 0x3E00;
	static const int STEP_RX_GAIN = 0x200;

	static const int MIN_TX_GAIN = 0x80;
	static const int MAX_TX_GAIN = 0x1F80;
	static const int STEP_TX_GAIN = 0x80;

	static const int SLIDER_RANGE = 65535;

	static const wchar_t * LAST_SAVED_CONFIG_FILE;

	SineDataReader * sineDataReader;
	SNRDataReader * OFDMDataReader;

	SineDataProcessor * sineDataProcessor;
	SNRDataProcessor * OFDMDataProcessor;

	IThreadControl * threadControl;

	UMXSendThread * threadSend;
	UMXReceiveThread * threadReceive;

	char * dumpBuf;
	int dumpBufLen;

	// Data
	Average<double> avgSnr;
	Average<double> avgDcI;
	Average<double> avgDcQ;
	Average<double> avgFreqOffset;
	Average<double> avgIQA;
	Average<double> avgIQP;

	bool timerValid;

	// Beacon
	UmxSender * beaconSender;
	HRESULT BeaconSenderInit();
	HRESULT SendBeacon();
	HRESULT BeaconSenderDeinit();
	bool beaconSenderInitialized;

	bool autogains[8], autogain;

	time_t lastTimeBeacon;

	FrequencyOffset * frequencyOffset;
	double latestFrequencyOffset;

	static const wchar_t * DEFAULT_STATUS_EMPTY;
	static const wchar_t * DEFAULT_STATUS_SNR_RX;
	static const wchar_t * DEFAULT_STATUS_SINE_RX;
	static const wchar_t * DEFAULT_STATUS_SNR_TX;
	static const wchar_t * DEFAULT_STATUS_SINE_TX;
	const wchar_t * statusMessage;

	wchar_t exeDir[MAX_PATH];

	static const int FREQ_OFFSET_HIST_LEN = 3;
	double freqOffsetHist[FREQ_OFFSET_HIST_LEN];
	int freqOffsetHistIdx;

	int freqStableCnt;
	bool newFreqOffsetAvailable;
};


class CThreadControl : public IThreadControl
{
public:
	void ThreadStartCallback();
	void ThreadExitCallback();

public:
	CThreadControl(Application * app);

private:
	Application * app;
};
