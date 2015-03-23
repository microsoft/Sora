#pragma once

#include <vector>
#include "Logger.h"

class Radio
{
public:
	static void Init();
	static Radio * Current();
	static void SelectRadio(int radioNum);
	static std::vector<int> AvailableRadios();

	static const int MAX_RADIO = 8;
private:
	static Radio * GetRadio(int radioNum);
	static Radio * radios[MAX_RADIO];

public:
	enum UMX_CONFIG {
		FREQ_OFFSET, 
		TX_GAIN, 
		RX_GAIN, 
		RX_PA, 
		CENTRAL_FREQ,
		SAMPLE_RATE,
		CONFIG_COUNT
	};
	void SetConfig(UMX_CONFIG configType, long long value);
	long long GetConfig(UMX_CONFIG configType);
	void SaveConfigFile(const wchar_t * fileName);
	void LoadConfigFile(const wchar_t * fileName);
	static void SaveConfigFileAll(const wchar_t * fileName);
	static void LoadConfigFileAll(const wchar_t * fileName);

	void SoraUmxConfigRadio();
	bool IsInitialized();
	void SetInitialized(bool);

	void SetRxGainDB(int db);
	int GetRxGainDB();

	int GetRadioNum();

	static const int MIN_RX_GAIN = 0x200;
	static const int MAX_RX_GAIN = 0x3E00;
	static const int STEP_RX_GAIN = 0x200;

	static const int MIN_TX_GAIN = 0x80;
	static const int MAX_TX_GAIN = 0x1F80;
	static const int STEP_TX_GAIN = 0x80;

	static const int MIN_RXPA = 0x0;
	static const int MAX_RXPA = 0x3000;
	static const int STEP_RXPA = 0x1000;

	void SetCalibrationLineSlope(double slope);
	void SetCalibrationLineIntercept(double intercept);
	double GetCalibrationLineSlope();
	double GetCalibrationLineIntercept();

	void SetFreqOffset(double hz);

private:
	Radio(int radioNum);
	int radioNum;

	bool initialized;

	long long m_config[CONFIG_COUNT];
	static const wchar_t * m_appName;
	static const wchar_t * m_configString[CONFIG_COUNT];
	static const int m_configDefault[CONFIG_COUNT];
	
	static const wchar_t * LINE_SLOPE_STR;
	static const wchar_t * LINE_INTERCEPT_STR;
	static const double LINE_SLOPE_DEFAULT;
	static const double LINE_INTERCEPT_DEFAULT;

	double calibrationLineSlope;
	double calibrationLineIntercept;
	
	void LoadDefaultConfig();
	void SoraUmxConfigWriteRegister(UMX_CONFIG config);


	Logger * logger;

};

