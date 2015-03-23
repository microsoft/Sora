#include <Windows.h>
#include <wchar.h>
#include "sora.h"
#include "Radio.h"

#include "Logger.h"

//#define SORA_RADIO_DEFAULT_TARGET_RADIO			0
//#define SORA_RADIO_DEFAULT_TX_GAIN				0x1F80				//0x1F80 in 1/256db
//#define SORA_RADIO_DEFAULT_RX_GAIN				0x2000				//0x2000 in 1/256db
//#define SORA_RADIO_DEFAULT_RX_PA				0x2000
//#define SORA_RADIO_PRESET1_RX_GAIN				0x2800				//0x2400 in 1/256db
//#define SORA_RADIO_DEFAULT_CENTRAL_FREQUENCY	(2422 * 1000)		// unit: kHz. central frequency: 2422MHz
//#define SORA_RADIO_DEFAULT_FREQUENCY_OFFSET		0					// unit: Hz
//#define SORA_RADIO_DEFAULT_SAMPLE_RATE			40					// sample rate: 40MHz

static Radio * gCurrentRadio = 0;

Radio * Radio::radios[] = {0};

const wchar_t * Radio::m_appName = L"Parameter";

const wchar_t * Radio::m_configString[] = {
	L"freqOffset", L"txgain", L"rxgain", L"rxpa", L"centralFreq", L"sampleRate"
};

const int Radio::m_configDefault[] = {
	0x0, 0xf00, 0x2000, 0x1000, 2422, 40
};

const wchar_t * Radio::LINE_SLOPE_STR = L"freqOffsetLineSlope";
const wchar_t * Radio::LINE_INTERCEPT_STR = L"freqOffsetLineIntercept";
const double Radio::LINE_SLOPE_DEFAULT = -0.894;
const double Radio::LINE_INTERCEPT_DEFAULT = 0.0;

static bool gUmxInited = false;

void Radio::Init()
{
	if (gUmxInited)
		return;

	gUmxInited = true;

	Logger * logger = Logger::GetLogger(L"umx");
	BOOLEAN succ = SoraUInitUserExtension("\\\\.\\HWTest");
	logger->Log(LOG_FUNC_CALL, L"SoraUInitUserExtension called\r\n");

	ULONG res;
	//wchar_t buf[32];
	for (int i = 0; i < MAX_RADIO; ++i)
	{
		HRESULT ret = SoraURadioStart(i);
		if (ret != S_OK)
			continue;
		
		ret = SoraUReadRadioRegister(i, 0x1, &res);
		//swprintf(buf, L"radio: %d, 0x%x %x\n", i, ret, res);
		//Logger::GetLogger(L"radio")->Log(LOG_DEFAULT, buf);

		if ((ret == S_OK) && (res == 0x1))
		{
			radios[i] = new Radio(i);
		}
		else
		{
			radios[i] = 0;
		}
	}	
}

Radio * Radio::GetRadio(int radioNum)
{
	if (radioNum < 0 || radioNum >= MAX_RADIO)
		return 0;

	if (radios[radioNum] == 0)
	{
		radios[radioNum] = new Radio(radioNum);
	}

	return radios[radioNum];
}

Radio * Radio::Current()
{
	return gCurrentRadio;
}

void Radio::SelectRadio(int radioNum)
{
	gCurrentRadio = radios[radioNum];
}

std::vector<int> Radio::AvailableRadios()
{
	std::vector<int> vecRadioId;

	for (int i = 0; i < MAX_RADIO; ++i)
	{
		if (radios[i] != 0)
			vecRadioId.push_back(i);
	}

	return vecRadioId;
}

Radio::Radio(int radioNum)
{
	this->radioNum = radioNum;
	initialized = false;

	logger = Logger::GetLogger(L"umx");

	LoadDefaultConfig();
}

void Radio::SetConfig(UMX_CONFIG configType, long long value)
{
	if (configType == SAMPLE_RATE)
	{
		if (value != 40 && value != 44)
		{
			logger->Log(LOG_ERROR, L"Sampling rate should be 40 or 44 MHz\n");
		}
	}

	switch(configType)
	{
	case TX_GAIN:
		value = min(value, MAX_TX_GAIN);
		value = max(value, MIN_TX_GAIN);
		value = value - value % STEP_TX_GAIN;
		break;
	case RX_GAIN:
		value = min(value, MAX_RX_GAIN);
		value = max(value, MIN_RX_GAIN);
		value = value - value % STEP_RX_GAIN;
		break;
	case RX_PA:
		value = min(value, MAX_RXPA);
		value = max(value, MIN_RXPA);
		value = value - value % STEP_RXPA;
		break;
	}

	m_config[configType] = value;

	switch(configType)
	{
	case FREQ_OFFSET:
		logger->Log(LOG_INFO, L"frequency offset set to %lld\r\n", value);
		break;
	case TX_GAIN:
		logger->Log(LOG_INFO, L"txgain set to 0x%llx\r\n", value);
		break;
	case RX_GAIN:
		logger->Log(LOG_INFO, L"rxgain set to 0x%llx\r\n", value);
		break;
	case RX_PA:
		logger->Log(LOG_INFO, L"rxpa set to 0x%llx\r\n", value);
		break;
	case CENTRAL_FREQ:
		logger->Log(LOG_INFO, L"central frequency set to %lld\r\n", value);
		break;
	case SAMPLE_RATE:
		logger->Log(LOG_INFO, L"sampling rate set to %lld MHz\r\n", value);
		break;
	}

	if (this->IsInitialized())
		SoraUmxConfigWriteRegister(configType);
}

long long Radio::GetConfig(UMX_CONFIG configType)
{
	return m_config[configType];
}


void Radio::SaveConfigFile(const wchar_t * fileName)
{
	for (int i = 0; i < CONFIG_COUNT; i++)
	{
		wchar_t buffer[64];
		//_snwprintf(buffer, 63, L"%I64d", m_config[i]);
		if ( (i >=1) && (i <= 3) )
			_snwprintf_s(buffer, 64, L"0x%I64x", m_config[i]);
		else if (i == 4)
			_snwprintf_s(buffer, 64, L"%I64d", m_config[i] / 1000);
		else
			_snwprintf_s(buffer, 64, L"%I64d", m_config[i]);

		WritePrivateProfileString(m_appName, m_configString[i], buffer, fileName);
	}

	wchar_t slopeStr[32];
	_snwprintf_s(slopeStr, 32, L"%f", this->calibrationLineSlope);
	WritePrivateProfileString(L"Calibration", LINE_SLOPE_STR, slopeStr, fileName);

	wchar_t interceptStr[32];
	_snwprintf_s(interceptStr, 32, L"%f", this->calibrationLineIntercept);
	WritePrivateProfileString(L"Calibration", LINE_INTERCEPT_STR, interceptStr, fileName);

	logger->Log(LOG_INFO, L"Save radio configuration to %s.\n", fileName);
}

void Radio::SaveConfigFileAll(const wchar_t * fileNamePrefix)
{
	size_t len = wcslen(fileNamePrefix);
	std::auto_ptr<wchar_t> ptrfileName(new wchar_t[len+16]);
	wchar_t * fileName = ptrfileName.get();

	for (int i = 0; i < MAX_RADIO; ++i)
	{
		Radio * radio = radios[i];
		if (radio != 0)
		{
			swprintf(fileName, L"%s%d.ini", fileNamePrefix, i);
			radio->SaveConfigFile(fileName);
		}
	}
}

void Radio::LoadConfigFileAll(const wchar_t * fileNamePrefix)
{
	size_t len = wcslen(fileNamePrefix);
	std::auto_ptr<wchar_t> ptrfileName(new wchar_t[len+16]);

	wchar_t * fileName = ptrfileName.get();

	for (int i = 0; i < MAX_RADIO; ++i)
	{
		Radio * radio = radios[i];
		if (radio != 0)
		{
			swprintf(fileName, L"%s%d.ini", fileNamePrefix, i);
			radio->LoadConfigFile(fileName);
		}
	}
}

void Radio::LoadConfigFile(const wchar_t * fileName)
{
	logger->Log(LOG_DEFAULT, L"Load radio configuration from %s.\n", fileName);
	for (int i = 0; i < CONFIG_COUNT; i++)
	{
		int value = GetPrivateProfileInt(m_appName, m_configString[i], m_configDefault[i], fileName);
		if (i == 4)
		{
			value *= 1000;
			m_config[i] = (unsigned int)value;
		}
		else
			m_config[i] = value;

		if ( (i >=1) && (i <= 3) )
			logger->Log(LOG_INFO, L"%s 0x%I64x\r\n", m_configString[i], m_config[i]);
		else if (i == 4)
			logger->Log(LOG_INFO, L"%s %I64d\r\n", m_configString[i], m_config[i]/1000);
		else
			logger->Log(LOG_INFO, L"%s %I64d\r\n", m_configString[i], m_config[i]);
	}

	double slope;
	wchar_t slopeStr[32];
	GetPrivateProfileString(L"Calibration", LINE_SLOPE_STR, L"-0.894", slopeStr, 32, fileName);
	swscanf_s(slopeStr, L"%lf", &slope);
	this->calibrationLineSlope = slope;

	logger->Log(LOG_INFO, L"%s %lf\r\n", LINE_SLOPE_STR, this->calibrationLineSlope);

	double intercept;
	wchar_t interceptStr[32];
	GetPrivateProfileString(L"Calibration", LINE_INTERCEPT_STR, L"0.0", interceptStr, 32, fileName);
	swscanf_s(interceptStr, L"%lf", &intercept);
	this->calibrationLineIntercept = intercept;

	logger->Log(LOG_INFO, L"%s %lf\r\n", LINE_INTERCEPT_STR, this->calibrationLineIntercept);

	if (IsInitialized())
	{
		for (int i = 0; i < CONFIG_COUNT; i++)
		{
			SoraUmxConfigWriteRegister((Radio::UMX_CONFIG)i);
		}
	}
}

void Radio::SoraUmxConfigRadio()
{
	wchar_t buf[32];
	HRESULT hResult = SoraURadioStart(radioNum);
	wsprintf(buf, L"SoraURadioStart(%d) called, 0x%x\n", radioNum, hResult);
	logger->Log(LOG_FUNC_CALL, buf);

	::Sleep(200);

	if (hResult == S_OK)
	{
		logger->Log(LOG_SUCCESS, L"success\n");
	}
	else
	{
		logger->Log(LOG_ERROR, L"Failed. ret = %x\n", hResult);
	}

	for (int i = 0; i < CONFIG_COUNT; i++)
	{
		SoraUmxConfigWriteRegister((UMX_CONFIG)i);
	}
	return;
}

void Radio::SoraUmxConfigWriteRegister(UMX_CONFIG config)
{
	HRESULT ret = S_OK;

	switch(config)
	{
	case FREQ_OFFSET:
		ret = SoraURadioSetFreqOffset(radioNum, (LONG)m_config[config]);
		logger->Log(LOG_FUNC_CALL, L"SoraURadioSetFreqOffset called for radio %d. freqOffset set to %lld\r\n", radioNum, m_config[config]);
		break;
	case TX_GAIN:
		ret = SoraURadioSetTxGain(radioNum, (ULONG)m_config[config]);
		logger->Log(LOG_FUNC_CALL, L"SoraURadioSetTxGain called for radio %d. txgain set to 0x%llx\r\n", radioNum, m_config[config]);
		break;
	case RX_GAIN:
		ret = SoraURadioSetRxGain(radioNum, (ULONG)m_config[config]);
		logger->Log(LOG_FUNC_CALL, L"SoraURadioSetRxGain called for radio %d. rxgain set to 0x%llx\r\n", radioNum, m_config[config]);
		break;
	case RX_PA:
		ret = SoraURadioSetRxPA(radioNum, (ULONG)m_config[config]);
		logger->Log(LOG_FUNC_CALL, L"SoraURadioSetRxPA called for radio %d. rxpa set to 0x%llx\r\n", radioNum, m_config[config]);
		break;
	case CENTRAL_FREQ:
		ret = SoraURadioSetCentralFreq(radioNum, (ULONG)m_config[config]);
		logger->Log(LOG_FUNC_CALL, L"SoraURadioSetCentralFreq called for radio %d. centralFreq set to %lld\r\n", radioNum, m_config[config]);
		break;
	case SAMPLE_RATE:
		return;
		;
	}

	if (FAILED(ret))
	{
		logger->Log(LOG_ERROR, L"Failed. ret = %x\n", ret);
	}
	else
	{
		logger->Log(LOG_SUCCESS, L"success\n");
	}
}

void Radio::LoadDefaultConfig()
{
	for (int i = 0; i < CONFIG_COUNT; i++)
	{
		m_config[i] = m_configDefault[i];
	}

	calibrationLineSlope = LINE_SLOPE_DEFAULT;
	calibrationLineIntercept = LINE_INTERCEPT_DEFAULT;
}

bool Radio::IsInitialized()
{
	return initialized;
}

void Radio::SetInitialized(bool value)
{
	initialized = value;
}

int Radio::GetRadioNum()
{
	return radioNum;
}

void Radio::SetRxGainDB(int db)
{
	int db2 = db / 2;	// in units of 2 dbs

	int	rxPa = (db2 / 8 + 1) * 0x1000;
	int	rxGain = db2 % 8 * Radio::STEP_RX_GAIN;	

	if ( rxPa > 0x3000 )
	{
		rxGain += (rxPa - 0x3000) / 0x1000 * 8 * Radio::STEP_RX_GAIN;
		rxPa = 0x3000;
	}

	if ( (rxGain < Radio::MIN_RX_GAIN) && (rxPa > 0x1000) )
	{
		rxGain += 8 * Radio::STEP_RX_GAIN;	// 16db
		rxPa -= 0x1000;						// 16db
	}

	if ( (rxGain > Radio::MAX_RX_GAIN) && (rxPa < 0x3000) )
	{
		rxGain -= 8 * Radio::STEP_RX_GAIN;	// 16db
		rxPa += 0x1000;						// 16db
	}

	rxGain = min(MAX_RX_GAIN, rxGain);

	this->SetConfig(Radio::RX_PA, rxPa);
	this->SetConfig(Radio::RX_GAIN, rxGain);
}


int Radio::GetRxGainDB()
{
	int rxPa = (int)this->GetConfig(Radio::RX_PA);
	int rxGain = (int)this->GetConfig(Radio::RX_GAIN);

	int db2 = (rxPa / 0x1000 - 1) * 8 + rxGain / Radio::STEP_RX_GAIN;	// in units of 2 dbs

	return 2 * db2;
}

void Radio::SetCalibrationLineSlope(double slope)
{
	this->calibrationLineSlope = slope;
}

void Radio::SetCalibrationLineIntercept(double intercept)
{
	this->calibrationLineIntercept = intercept;
}

double Radio::GetCalibrationLineSlope()
{
	return this->calibrationLineSlope;
}

double Radio::GetCalibrationLineIntercept()
{
	return this->calibrationLineIntercept;
}

void Radio::SetFreqOffset(double hz)
{
	HRESULT ret;
	double slope = this->calibrationLineSlope;
	double intercept = this->calibrationLineIntercept;

	int freqOffset = (int)GetConfig(Radio::FREQ_OFFSET);
	if (slope != 0)
		SetConfig(Radio::FREQ_OFFSET, (long long)(freqOffset + (double)hz/slope));
	else
		logger->Log(LOG_ERROR, L"Slope of calibration line equals zero.\n");
}
