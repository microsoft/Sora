#include <Windows.h>
#include <CommCtrl.h>
#include <time.h>
#include <CommDlg.h>
#include <shellapi.h>

#include "HwApp.h"
#include "LoggerImpl.h"
#include "Message.h"
#include "HwVeriTask.h"
#include "Radio.h"
#include "UmxCommonOps.h"

#include "resource.h"

const wchar_t * Application::LAST_SAVED_CONFIG_FILE = L"SavedConfig";
const wchar_t * Application::DEFAULT_STATUS_EMPTY =  L"Idle.";
const wchar_t * Application::DEFAULT_STATUS_SNR_RX = L"No valid frame detected.";
const wchar_t * Application::DEFAULT_STATUS_SINE_RX = L"No valid package detected.";
const wchar_t * Application::DEFAULT_STATUS_SNR_TX = L"Sending 16QAM wave.";
const wchar_t * Application::DEFAULT_STATUS_SINE_TX = L"Sending 1MHz sine wave.";

extern HWND g_hDlg;

CThreadControl::CThreadControl(Application * app)
{
	this->app = app;
}

void CThreadControl::ThreadStartCallback()
{
	if (app->direction == Application::RECEIVE)
		switch (app->mode)
		{
		case Application::TEST_SINE:
			app->sineDataProcessor->Init();
			break;
		case Application::TEST_SNR:
			app->OFDMDataProcessor->Init();
			break;
		}
	app->actionStart = Application::STOP;
	//app->ReconfigUI();
	::SendNotifyMessage(app->hwnd, WM_RECONFIG_UI, 0, 0);
}

void CThreadControl::ThreadExitCallback()
{
	if (app->direction == Application::RECEIVE)
		switch (app->mode)
		{
		case Application::TEST_SINE:
			app->sineDataProcessor->Deinit();
			break;
		case Application::TEST_SNR:
			app->OFDMDataProcessor->Deinit();
			break;
		}
	app->actionStart = Application::START;
	//app->ReconfigUI();
	::SendNotifyMessage(app->hwnd, WM_RECONFIG_UI, 0, 0);
	//::SendNotifyMessageA(app->hwnd, WM_UPDATA_STATUS_BAR, 0, 0);
}


Application::Application()
{
	logger = Logger::GetLogger(L"App");
	//taskQueue = new TaskQueue();
}

Application::~Application()
{
	//delete taskQueue;
}

void Application::Init(HWND hwnd)
{
	Radio::Init();

	if (Radio::AvailableRadios().size() == 0)
	{
		ReconfigUIDisableAll();
		::MessageBoxA(g_hDlg, "Error: no radio discovered", "", 0);
		exit(-1);
	}

	GetCurrentDirectory(MAX_PATH, exeDir);

	this->hwnd = hwnd;
	for (int i = 0; i < Radio::MAX_RADIO; ++i)
	{
		this->modes[i] = TEST_SINE;
		this->mode = TEST_SINE;
		this->directions[i] = SEND;
		this->direction = SEND;
		this->autogains[i] = true;
		this->autogain = true;
	}
	this->actionStart = START;

	ReconfigUI();

	wchar_t path[MAX_PATH];
	path[0] = '\0';
	wcscat_s(path, exeDir);
	wcscat_s(path, L"\\");
	wcscat_s(path, LAST_SAVED_CONFIG_FILE);
	Radio::LoadConfigFileAll(path);

	UpdateUIPara();

	path[0] = '\0';

	wcscat_s(path, exeDir);
	wcscat_s(path, L"\\data\\ofdm.bin");

	sineDataReader = new SineDataReader();
	OFDMDataReader = new SNRDataReader(path);
	sineDataProcessor = new SineDataProcessor(hwnd, Radio::Current());
	OFDMDataProcessor = new SNRDataProcessor(hwnd);
	threadControl = new CThreadControl(this);

	threadSend = new UMXSendThread(Radio::Current(), threadControl);
	threadReceive = new UMXReceiveThread(Radio::Current(), threadControl);
	beaconSender = new UmxSender(Radio::Current());

	dumpBuf = 0;

	timerValid = false;
	beaconSenderInitialized = false;
	//BeaconSenderInit();
	autogain = true;

	frequencyOffset = new FrequencyOffset(Radio::Current());
	isAutoCalibrating = false;

	statusMessage = 0;

	for (int i = 0; i < FREQ_OFFSET_HIST_LEN; i++)
	{
		freqOffsetHist[i] = 0.0;
	}
	
	freqOffsetHistIdx = 0;

	//::SoraUmxInit(radio);
	::SetTimer(hwnd, 1, 1000, 0);
}

void Application::LoadOpState()
{
	mode = modes[Radio::Current()->GetRadioNum()];
	direction = directions[Radio::Current()->GetRadioNum()];
	autogain = autogains[Radio::Current()->GetRadioNum()];
	actionStart = START;
}

void Application::SaveOpState()
{
	modes[Radio::Current()->GetRadioNum()] = mode;
	directions[Radio::Current()->GetRadioNum()] = direction;
	autogains[Radio::Current()->GetRadioNum()] = autogain;
}

void Application::SaveLastConfigFile()
{
	wchar_t path[MAX_PATH];
	path[0] = '\0';
	wcscat_s(path, exeDir);
	wcscat_s(path, L"\\");
	wcscat_s(path, LAST_SAVED_CONFIG_FILE);
	Radio::SaveConfigFileAll(path);
}

void Application::Deinit()
{
	::KillTimer(hwnd, 1);

	threadSend->Stop(true);
	threadReceive->Stop(true);

	delete threadSend;
	delete threadReceive;

	//delete frequencyOffset;
	delete sineDataReader;
	delete OFDMDataReader;
	delete sineDataProcessor;
	delete OFDMDataProcessor;

	delete threadControl;
	
	delete frequencyOffset;

	//BeaconSenderDeinit();
	if (beaconSender)
	{
		delete beaconSender;
		beaconSender = 0;
	}


	SaveLastConfigFile();

	::SoraUmxDeinit(Radio::Current());
}

void Application::ReconfigUI()
{
	ReconfigUIDisableAll();
	EnableUIParaSetting();
	EnableUIByState();
	//RefreshStatusBar();
}

void Application::ReconfigUIDisableAll()
{
	::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_CENTRALFREQ), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_RADIO_LOOP), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_SUGGESTION), FALSE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_START), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_RADIO_SINE), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_RADIO_SNR), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_RADIO_SEND), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_RADIO_RECEIVE), FALSE);

	// Para Settings
	::EnableWindow(GetDlgItem(hwnd, IDC_COMBO_SAMPLE_RATE), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_SAMPLE_RATE), FALSE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_SLIDER_GAIN), FALSE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_GAIN), FALSE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_GAIN), FALSE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_GAIN_DB), FALSE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_CFREQ), FALSE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_COMBO_CFREQ), FALSE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_CFREQ), FALSE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_CFREQ_DB), FALSE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_RXPA), FALSE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_COMBO_RXPA), FALSE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_SAVE_PARA), FALSE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_LOAD_PARA), FALSE);

	::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_FREQ_OFFSET), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_FREQ_OFFSET), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_CALIBRATION), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_IQA), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_IQA), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_IQP), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_IQP), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_SNR), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_SNR), FALSE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_LOG), FALSE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_SAVE_LOG), FALSE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_CLR_LOG), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_DUMP), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_DC), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_DC_I), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_Q), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_DC_I), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_DC_Q), FALSE);
	::EnableWindow(GetDlgItem(hwnd, IDC_CHECK_AUTOGAIN), FALSE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_EDIT3), FALSE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_STATUS), FALSE);
}

void Application::EnableUIByState()
{
	switch (mode)
	{
	case TEST_SINE:
		return EnableUISineTest();
	case TEST_SNR:
		return EnableUISnrTest();
	}
	return;
}

void Application::EnableUISineTest()
{
	::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_START), TRUE);

	if (actionStart == START)
	{
		EnableUIModeSelection();
		EnableUIDirectionSelection();
		::SetWindowText(GetDlgItem(hwnd, IDC_BUTTON_START), L"start");
	}
	else
	{
		::SetWindowText(GetDlgItem(hwnd, IDC_BUTTON_START), L"stop");
	}

	switch(direction)
	{
	case SEND:
		::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_RXPA), FALSE);
		::EnableWindow(GetDlgItem(hwnd, IDC_COMBO_RXPA), FALSE);
		::SetWindowText(GetDlgItem(hwnd, IDC_STATIC_GAIN), L"txgain");

		if (actionStart == START)
			::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_CENTRALFREQ), TRUE);


		break;
	case RECEIVE:
		::SetWindowText(GetDlgItem(hwnd, IDC_STATIC_GAIN), L"rxgain");
		::EnableWindow(GetDlgItem(hwnd, IDC_CHECK_AUTOGAIN), TRUE);

		if (autogain)
		{
			::EnableWindow(GetDlgItem(hwnd, IDC_SLIDER_GAIN), FALSE);
			::EnableWindow(GetDlgItem(hwnd, IDC_COMBO_RXPA), FALSE);
		}

		if (actionStart == STOP)
		{
			::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_SUGGESTION), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_FREQ_OFFSET), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_FREQ_OFFSET), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_CALIBRATION), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_IQA), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_IQA), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_IQP), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_IQP), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_DUMP), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_DC), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_DC_I), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_Q), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_DC_I), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_DC_Q), TRUE);
		}
		else
		{
			::EnableWindow(GetDlgItem(hwnd, IDC_COMBO_SAMPLE_RATE), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_SAMPLE_RATE), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_CENTRALFREQ), TRUE);
		}

		break;
	}
}

void Application::OnSuggestion()
{
	wchar_t path[MAX_PATH];
	path[0] = '\0';

	if (mode == TEST_SINE)
	{
		wcscat_s(path, exeDir);
		wcscat_s(path, L"\\data\\SineTest.mht");
	}
	else
	{
		wcscat_s(path, exeDir);
		wcscat_s(path, L"\\data\\SnrTest.mht");
	}

	ShellExecute(NULL, L"open", path, NULL, NULL, SW_SHOWNORMAL);
}

void Application::EnableUISnrTest()
{
	::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_START), TRUE);

	if (actionStart == START)
	{
		EnableUIModeSelection();
		EnableUIDirectionSelection();
		::SetWindowText(GetDlgItem(hwnd, IDC_BUTTON_START), L"start");
	}
	else
	{
		::SetWindowText(GetDlgItem(hwnd, IDC_BUTTON_START), L"stop");
	}

	switch(direction)
	{
	case SEND:
		::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_RXPA), FALSE);
		::EnableWindow(GetDlgItem(hwnd, IDC_COMBO_RXPA), FALSE);
		::SetWindowText(GetDlgItem(hwnd, IDC_STATIC_GAIN), L"txgain");
		
		if (actionStart == START)
			::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_CENTRALFREQ), TRUE);

		break;
	case RECEIVE:
		::SetWindowText(GetDlgItem(hwnd, IDC_STATIC_GAIN), L"rxgain");
		::EnableWindow(GetDlgItem(hwnd, IDC_CHECK_AUTOGAIN), TRUE);

		if (autogain)
		{
			::EnableWindow(GetDlgItem(hwnd, IDC_SLIDER_GAIN), FALSE);
			::EnableWindow(GetDlgItem(hwnd, IDC_COMBO_RXPA), FALSE);
		}

		if (actionStart == STOP)
		{
			::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_SUGGESTION), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_SNR), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_SNR), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_DC), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_DC_I), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_Q), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_DC_I), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_DC_Q), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_FREQ_OFFSET), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_FREQ_OFFSET), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_DUMP), TRUE);
		}
		else
		{
			::EnableWindow(GetDlgItem(hwnd, IDC_COMBO_SAMPLE_RATE), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_SAMPLE_RATE), TRUE);
			::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_CENTRALFREQ), TRUE);
		}
		break;
	}	
}

void Application::EnableUIModeSelection()
{
	::EnableWindow(GetDlgItem(hwnd, IDC_RADIO_SINE), TRUE);
	::EnableWindow(GetDlgItem(hwnd, IDC_RADIO_SNR), TRUE);		
}

void Application::EnableUIDirectionSelection()
{
	::EnableWindow(GetDlgItem(hwnd, IDC_RADIO_SEND), TRUE);
	::EnableWindow(GetDlgItem(hwnd, IDC_RADIO_RECEIVE), TRUE);
}

void Application::EnableUIParaSetting()
{
	::EnableWindow(GetDlgItem(hwnd, IDC_SLIDER_GAIN), TRUE);
	::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_GAIN), TRUE);
	::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_GAIN_DB), TRUE);
	::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_GAIN), TRUE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_GAIN_DB), TRUE);
	::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_CFREQ), TRUE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_CENTRALFREQ), TRUE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_EDIT_CFREQ), TRUE);
	//::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_CFREQ_DB), TRUE);
	::EnableWindow(GetDlgItem(hwnd, IDC_STATIC_RXPA), TRUE);
	::EnableWindow(GetDlgItem(hwnd, IDC_COMBO_RXPA), TRUE);
	::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_SAVE_PARA), TRUE);
	::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_LOAD_PARA), TRUE);	
}

void Application::OnClickButtonStart()
{
	//ReconfigUI();
	//TaskVoidContext * context = new TaskVoidContext();
	//context->hwnd = hwnd;
	//context->state = TASK_NEW;
	//context->type = TASK_VOID;
	//TaskVoid * taskVoid = new TaskVoid(context);
	//taskQueue->AddTask(taskVoid);

	ReconfigUI();

	if (actionStart == START)
	{
		if (ReadCentrolFreqFromUIAndUpdate() == false)
			return;
	}
	else
	{
		//timerValid = false;
		//::KillTimer(hwnd, 1);
		ClearData();
		ClearUIData();
	}

	switch (mode)
	{
	case TEST_LOOP:		// not implemented
		return;
	case TEST_SINE:
		switch (direction)
		{
		case SEND:
			switch(actionStart)
			{
			case START:

				::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_START), FALSE);
				::EnableWindow(GetDlgItem(hwnd, IDC_COMBO_RADIO_SEL), FALSE);
				((SineDataReader *)sineDataReader)->Reset();

				threadSend->SetReader(sineDataReader);
				threadSend->Start(false);

				//m_pUmx->StartSend(m_pSineDataReader, m_pThreadControl, false);
				break;
			case STOP:
				::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_START), FALSE);
				::EnableWindow(GetDlgItem(hwnd, IDC_COMBO_RADIO_SEL), TRUE);
				//m_pUmx->StopSend(false);
				threadSend->Stop(false);
				break;
			}
			break;
		case RECEIVE:
			switch(actionStart)
			{
			case START:
				::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_START), FALSE);
				::EnableWindow(GetDlgItem(hwnd, IDC_COMBO_RADIO_SEL), FALSE);
				threadReceive->SetProcessor(sineDataProcessor);
				threadReceive->Start(false);
				time(&lastTimeBeacon);
				break;
			case STOP:
				::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_START), FALSE);
				::EnableWindow(GetDlgItem(hwnd, IDC_COMBO_RADIO_SEL), TRUE);		
				::SetWindowText(
					GetDlgItem(hwnd, IDC_BUTTON_CALIBRATION),
					L"calibration");
				isAutoCalibrating = false;
				threadReceive->Stop(false);

				break;
			}
			break;
		}
		return;
	case TEST_SNR:
		switch (direction)
		{
		case SEND:
			switch(actionStart)
			{
			case START:
				::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_START), FALSE);
				::EnableWindow(GetDlgItem(hwnd, IDC_COMBO_RADIO_SEL), FALSE);
				((SNRDataReader *)OFDMDataReader)->Reset();

				threadSend->SetReader(OFDMDataReader);
				threadSend->Start(false);

				break;
			case STOP:
				::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_START), FALSE);
				::EnableWindow(GetDlgItem(hwnd, IDC_COMBO_RADIO_SEL), TRUE);
				threadSend->Stop(false);
				break;
			}
			break;
		case RECEIVE:
			switch(actionStart)
			{
			case START:
				::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_START), FALSE);
				::EnableWindow(GetDlgItem(hwnd, IDC_COMBO_RADIO_SEL), FALSE);
				threadReceive->SetProcessor(OFDMDataProcessor);
				threadReceive->Start(false);
				time(&lastTimeBeacon);
				break;

			case STOP:
				::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_START), FALSE);
				::EnableWindow(GetDlgItem(hwnd, IDC_COMBO_RADIO_SEL), TRUE);
				threadReceive->Stop(false);
				break;
			}
			break;
		}

		return;
	}

}

bool Application::ReadCentrolFreqFromUIAndUpdate()
{
	const int BUF_CNT = 64;
	wchar_t freqBuffer[BUF_CNT];
	::GetWindowText(GetDlgItem(hwnd, IDC_EDIT_CENTRALFREQ) , freqBuffer, BUF_CNT);

	for (int i = 0; i < BUF_CNT; ++i)
	{
		wchar_t ch = freqBuffer[i];
		if (ch == L'\0')
			break;
		if ( (ch > L'9') || (ch < L'0') )
		{
			::MessageBox(hwnd, L"Invalid central frequency", L"", 0);
			return false;
		}
	}

	int freq = _wtoi(freqBuffer);
	long long freqKiloHz = (long long)freq * 1000;
	if ((freqKiloHz <= 0) || (freqKiloHz > UINT_MAX))
	{
		::MessageBox(hwnd, L"Invalid central frequency", L"", 0);
		return false;
	}
	else
	{
		this->SetCentralFreq((unsigned int)freqKiloHz);
		return true;
	}
}

void Application::OnClickCalibration()
{
	if (!isAutoCalibrating)
	{
		frequencyOffset->Reset();
		isAutoCalibrating = true;
		calibrationFirstSample = true;
		freqStableCnt = 0;
		::SetWindowText(
			GetDlgItem(hwnd, IDC_BUTTON_CALIBRATION),
			L"stop calibration");
		logger->Log(LOG_INFO, L"Start calibration...\n");
	}
	else
	{
		isAutoCalibrating = false;
		::SetWindowText(
			GetDlgItem(hwnd, IDC_BUTTON_CALIBRATION),
			L"calibration");
	}
	return;
}

void Application::SaveLog()
{
	wchar_t inputFileNameBuf[256];
    inputFileNameBuf[0] = '\0';
    OPENFILENAME openFileName;
    memset(&openFileName, 0, sizeof(OPENFILENAME));
    openFileName.lStructSize = sizeof(OPENFILENAME);
    openFileName.lpstrFilter = L"Log file(*.txt)\0*.txt\0All file(*.*)\0*.*\0\0";
    openFileName.lpstrFile = inputFileNameBuf;
    openFileName.nMaxFile = 256;
    openFileName.lpstrTitle = L"Save Log...";
    openFileName.lpstrDefExt = L"txt";
    openFileName.Flags |= OFN_HIDEREADONLY;
	openFileName.hwndOwner = hwnd;

    if (!GetSaveFileName(&openFileName))
        return;

	FILE * fp;
	//fp = _wfopen(inputFileNameBuf, L"w");
	fp = _wfopen(inputFileNameBuf, L"w");
	if (fp == 0)
	{
		logger->Log(LOG_ERROR, L"Failed to open file. errno = %x.\n", errno);
		return;
	}

	long len = ::SendMessage(GetDlgItem(hwnd, IDC_RICHEDIT21_LOG), WM_GETTEXTLENGTH, NULL, NULL);
	wchar_t * pBuf = new wchar_t[len+1];
	::SendMessage(GetDlgItem(hwnd, IDC_RICHEDIT21_LOG), WM_GETTEXT, len, (LPARAM)pBuf);
	pBuf[len] = '\0';
	fwprintf(fp, L"%s", pBuf);
	delete pBuf;
	fclose(fp);
}

void Application::SelectModeSine()
{
	mode = TEST_SINE;
	ReconfigUI();
	//logger->Log(LOG_DEFAULT, L"Test mode set to sine test.\n");
}

void Application::SelectModeSnr()
{
	mode = TEST_SNR;
	ReconfigUI();
	//logger->Log(LOG_DEFAULT, L"Test mode set to snr test.\n");
}

void Application::SelectDirectionSend()
{
	direction = SEND;
	ReconfigUI();
	UpdateUIPara();
	//logger->Log(LOG_DEFAULT, L"Direction set to send.\n");
}

void Application::SelectDirectionReceive()
{
	direction = RECEIVE;
	ReconfigUI();
	UpdateUIPara();
	//logger->Log(LOG_DEFAULT, L"Direction set to receive.\n");
}


void Application::SetGain(int sel, int max)
{
	Radio::UMX_CONFIG config;
	int minGain, maxGain, stepGain;

	if (direction == SEND)	// txgin
	{
		config = Radio::TX_GAIN;
		minGain = MIN_TX_GAIN;
		maxGain = MAX_TX_GAIN;
		stepGain = STEP_TX_GAIN;
	}
	else	// rxgain
	{
		config = Radio::RX_GAIN;
		minGain = MIN_RX_GAIN;
		maxGain = MAX_RX_GAIN;
		stepGain = STEP_RX_GAIN;
	}

	int value = minGain + (maxGain-minGain)*sel/max;
	value = (value/stepGain)*stepGain;

	double db = (double)value/256;

	wchar_t buffer[64];
	_snwprintf_s(buffer, 64, _TRUNCATE, L"%.1f", db);
	::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_GAIN), buffer);

	if ( (direction == SEND) && (actionStart ==  STOP) )
	{
		threadSend->Suspend();
		::Sleep(0);
	}

	Radio::Current()->SetConfig(config, value);

	if ( (direction == SEND) && (actionStart ==  STOP) )
	{
		threadSend->Resume();
	}
}

void Application::OnGainSliderChange(int sel, int max)
{
	Radio::UMX_CONFIG config;
	int minGain, maxGain, stepGain;

	if (direction == SEND)	// txgin
	{
		config = Radio::TX_GAIN;
		minGain = MIN_TX_GAIN;
		maxGain = MAX_TX_GAIN;
		stepGain = STEP_TX_GAIN;
	}
	else					// rxgain
	{
		config = Radio::RX_GAIN;
		minGain = MIN_RX_GAIN;
		maxGain = MAX_RX_GAIN;
		stepGain = STEP_RX_GAIN;
	}

	int value = minGain + (maxGain-minGain)*sel/max;
	value = (value/stepGain)*stepGain;

	double db = (double)value/256;

	wchar_t buffer[64];
	_snwprintf_s(buffer, 64, _TRUNCATE, L"%.1f", db);
	::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_GAIN), buffer);
}

void Application::SetCentralFreq(unsigned int freq)
{
	if ( (direction == SEND) && (actionStart ==  STOP) )
	{
		threadSend->Suspend();
		::Sleep(0);
	}

	Radio::Current()->SetConfig(Radio::CENTRAL_FREQ, freq);
	
	if ( (direction == SEND) && (actionStart ==  STOP) )
	{
		threadSend->Resume();
	}
}

void Application::SetRxpa(int rxpa)
{
	Radio::Current()->SetConfig(Radio::RX_PA, rxpa);
}

void Application::SavePara()
{
	wchar_t inputFileNameBuf[256];
    inputFileNameBuf[0] = '\0';
    OPENFILENAME openFileName;
    memset(&openFileName, 0, sizeof(OPENFILENAME));
    openFileName.lStructSize = sizeof(OPENFILENAME);
    openFileName.lpstrFilter = L"Config file(*.ini)\0*.ini\0All file(*.*)\0*.*\0\0";
    openFileName.lpstrFile = inputFileNameBuf;
    openFileName.nMaxFile = 256;
    openFileName.lpstrTitle = L"Save Config File...";
    openFileName.lpstrDefExt = L"ini";
    openFileName.Flags |= OFN_HIDEREADONLY;
	openFileName.hwndOwner = hwnd;

    if (!GetSaveFileName(&openFileName))
        return;

	Radio::Current()->SaveConfigFile(inputFileNameBuf);
}

void Application::LoadPara()
{
	wchar_t inputFileNameBuf[256];
    inputFileNameBuf[0] = '\0';
    OPENFILENAME openFileName;
    memset(&openFileName, 0, sizeof(OPENFILENAME));
    openFileName.lStructSize = sizeof(OPENFILENAME);
    openFileName.lpstrFilter = L"Config file(*.ini)\0*.ini\0All file(*.*)\0*.*\0\0";
    openFileName.lpstrFile = inputFileNameBuf;
    openFileName.nMaxFile = 256;
    openFileName.lpstrTitle = L"Load Config File...";
    openFileName.lpstrDefExt = L"ini";
    openFileName.Flags |= OFN_HIDEREADONLY;
	openFileName.hwndOwner = hwnd;

    if (!GetOpenFileName(&openFileName))
        return;

	if ( (direction == SEND) && (actionStart ==  STOP) )
	{
		threadSend->Suspend();
		::Sleep(0);
	}

	Radio::Current()->LoadConfigFile(inputFileNameBuf);
	
	if ( (direction == SEND) && (actionStart ==  STOP) )
	{
		threadSend->Resume();
	}

	UpdateUIPara();
}

void Application::UpdateUIPara()
{
	// update gain
	int gain;

	int minGain, maxGain, stepGain;

	if (mode == TEST_SINE)
		CheckRadioButton(g_hDlg, IDC_RADIO_LOOP, IDC_RADIO_SNR, IDC_RADIO_SINE);
	else
		CheckRadioButton(g_hDlg, IDC_RADIO_LOOP, IDC_RADIO_SNR, IDC_RADIO_SNR);

	if (direction == SEND)	// txgin
	{
		gain = (int)Radio::Current()->GetConfig(Radio::TX_GAIN);
		minGain = MIN_TX_GAIN;
		maxGain = MAX_TX_GAIN;
		stepGain = STEP_TX_GAIN;
		CheckRadioButton(g_hDlg, IDC_RADIO_SEND, IDC_RADIO_RECEIVE, IDC_RADIO_SEND);
	}
	else					// rxgain
	{
		gain = (int)Radio::Current()->GetConfig(Radio::RX_GAIN);
		minGain = MIN_RX_GAIN;
		maxGain = MAX_RX_GAIN;
		stepGain = STEP_RX_GAIN;
		CheckRadioButton(g_hDlg, IDC_RADIO_SEND, IDC_RADIO_RECEIVE, IDC_RADIO_RECEIVE);
	}

	int pos = (gain - minGain) * SLIDER_RANGE / (maxGain-minGain);
	::SendMessage(GetDlgItem(hwnd, IDC_SLIDER_GAIN), TBM_SETPOS, TRUE, pos);
	
	double db = (double)gain/256;

	wchar_t buffer[64];
	_snwprintf_s(buffer, 64, _TRUNCATE, L"%.1f", db);
	::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_GAIN), buffer);

	// update rxpa

	int select;
	switch(Radio::Current()->GetConfig(Radio::RX_PA))
	{
	case 0x1000:
		select = 0;
		break;
	case 0x2000:
		select = 1;
		break;
	case 0x3000:
		select = 2;
		break;
	default:
		select = 0;
	}

	::SendMessage(GetDlgItem(hwnd, IDC_COMBO_RXPA), CB_SETCURSEL, select, 0);

	// update sample rate
	int selectSampleRate;
	switch(Radio::Current()->GetConfig(Radio::SAMPLE_RATE))
	{
	case 40:
		selectSampleRate = 0;
		break;
	case 44:
		selectSampleRate = 1;
		break;
	default:
		selectSampleRate = 0;
	}

	::SendMessage(GetDlgItem(hwnd, IDC_COMBO_SAMPLE_RATE), CB_SETCURSEL, selectSampleRate, 0);

	// update central frequency
	unsigned int cfreq = (unsigned int)Radio::Current()->GetConfig(Radio::CENTRAL_FREQ);
	
	wchar_t buf[64];
	_snwprintf_s(buf, 64, _TRUNCATE, L"%d", cfreq/1000);
	::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_CENTRALFREQ), buf);

	if (autogain)
	{
		CheckDlgButton(g_hDlg,IDC_CHECK_AUTOGAIN, 1);
	}
	else
	{
		CheckDlgButton(g_hDlg,IDC_CHECK_AUTOGAIN, 0);
	}
}

void Application::Dump()
{
	if (dumpBuf == 0)
	{
		logger->Log(LOG_ERROR, L"No dump data available.\n");
		return;
	}

	wchar_t inputFileNameBuf[256];
    inputFileNameBuf[0] = '\0';
    OPENFILENAME openFileName;
    memset(&openFileName, 0, sizeof(OPENFILENAME));
    openFileName.lStructSize = sizeof(OPENFILENAME);
    openFileName.lpstrFilter = L"Dump file(*.dmp)\0*.dmp\0All file(*.*)\0*.*\0\0";
    openFileName.lpstrFile = inputFileNameBuf;
    openFileName.nMaxFile = 256;
    openFileName.lpstrTitle = L"Make a Dump File...";
    openFileName.lpstrDefExt = L"dmp";
    openFileName.Flags |= OFN_HIDEREADONLY;
	openFileName.hwndOwner = hwnd;

    if (!GetSaveFileName(&openFileName))
        return;

	FILE * fp;
	fp = _wfopen(inputFileNameBuf, L"wb");
	if (fp != 0)
	{
		int len = fwrite(dumpBuf, sizeof(char), dumpBufLen, fp);
		fclose(fp);
		logger->Log(LOG_INFO, L"Dump %d bytes.\r\n", len);
	}
	else
	{
		logger->Log(LOG_ERROR, L"Failed to open file. errno = %x\r\n", errno);		
	}

	return;	
}

void Application::AddDumpData(char * buf, int len)
{
	if (dumpBuf)
	{
		delete dumpBuf;
	}

	dumpBuf = buf;
	dumpBufLen = len;
}

void Application::Suggestion()
{
	
}

void Application::EnableStartButton(bool enable)
{
	::EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_START), enable);
}

void Application::ProcessTaskMessage(TASK_TYPE type, void * context)
{
	switch(type)
	{
	case TASK_VOID:
		TaskVoidContext * aContext = (TaskVoidContext *)context;
		if (aContext->state == TASK_STARTED)
		{
			actionStart = STOP;
			ReconfigUI();
		}
		else if (aContext->state == TASK_DONE_SUCCESS)
		{
			actionStart = START;
			ReconfigUI();
			delete aContext->task;
			delete aContext;
		}
		break;
	}
}

void Application::UpdateData(UPDATE_DATA_TYPE type, double * data)
{
	double dataValue = *data;
	delete [] data;

	wchar_t buffer[64];

	_snwprintf_s(buffer, 64, _TRUNCATE, L"%f", dataValue);	

	switch(type)
	{
	case DATA_SNR:
		avgSnr.Add(dataValue);
		break;
	case DATA_DC_I:
		avgDcI.Add(dataValue);
		break;
	case DATA_DC_Q:
		avgDcQ.Add(dataValue);
		break;
	case DATA_FREQ_OFFSET:
		avgFreqOffset.Add(dataValue);
		latestFrequencyOffset = dataValue;
		newFreqOffsetAvailable = true;
		freqOffsetHist[freqOffsetHistIdx] = dataValue;
		freqOffsetHistIdx = (freqOffsetHistIdx+1) % FREQ_OFFSET_HIST_LEN;

		break;
	case DATA_IQ_IMBALANCE_AMP:
		avgIQA.Add(dataValue);

		break;
	case DATA_IQ_IMBALANCE_PHASE:
		avgIQP.Add(dataValue);

		break;
	}

}

void Application::OnTimer()
{
	//if (!timerValid)
	//	return;

	if (actionStart == STOP)
		RefreshUI();

	RefreshStatusBar();

	if (isAutoCalibrating)
	{
		bool freqOffsetStable = true;

		if (!newFreqOffsetAvailable)
		{
			freqOffsetStable = false;
		}
		else
		{
		}

		if (freqOffsetStable)
		{
			freqStableCnt = 0;

			if (calibrationFirstSample)
			{
				frequencyOffset->SetNextX();
				calibrationFirstSample = false;
			}
			else
			{
				frequencyOffset->SetNextY(latestFrequencyOffset);

				if (!frequencyOffset->SampleFinished())
				{
					frequencyOffset->SetNextX();
				}
				else
				{
					frequencyOffset->Calibration();
					Radio::Current()->SetFreqOffset(-latestFrequencyOffset);
					//frequencyOffset->SetFreqOffset(-latestFrequencyOffset);
					isAutoCalibrating = false;
					::SetWindowText(
						GetDlgItem(hwnd, IDC_BUTTON_CALIBRATION),
						L"calibration");
				}
			}
		}
		else
		{
			freqStableCnt++;
			if (freqStableCnt > 3)
			{
				logger->Log(LOG_ERROR, L"Incorrect signal detected. Calibration stopped.\n");
				isAutoCalibrating = false;
				::SetWindowText(
					GetDlgItem(hwnd, IDC_BUTTON_CALIBRATION),
					L"calibration");			
			}
		}
	}

	if ( (direction == RECEIVE) && (actionStart == STOP) && !isAutoCalibrating)
	{
		time_t curTime;

		time(&curTime);

		const int TIME_30_MINUTE = 30 * 60;

		if ( difftime(curTime, lastTimeBeacon) > TIME_30_MINUTE)
		{
			threadReceive->Suspend();
			::Sleep(100);
			SendBeacon();
			threadReceive->Resume();
			lastTimeBeacon = curTime;
		}
	}

	newFreqOffsetAvailable = false;
}

void Application::RefreshStatusBar()
{
	const wchar_t * statusMessageDefault = DEFAULT_STATUS_EMPTY;

	if (statusMessage)
	{
		::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_STATUS), statusMessage);
		delete statusMessage;
		statusMessage = 0;
	}
	else {
		if (actionStart == STOP)
		{
			if ( (mode == TEST_SINE) && (direction == RECEIVE) )
			{
				statusMessageDefault = DEFAULT_STATUS_SINE_RX;
			}
			else if ( (mode == TEST_SNR) && (direction == RECEIVE) )
			{
				statusMessageDefault = DEFAULT_STATUS_SNR_RX;
			}
			else if ( (mode == TEST_SINE) && (direction == SEND) )
			{
				statusMessageDefault = DEFAULT_STATUS_SINE_TX;
			}
			else if ( (mode == TEST_SNR) && (direction == SEND) )
			{
				statusMessageDefault = DEFAULT_STATUS_SNR_TX;
			}
		}
		else
		{
			statusMessageDefault = DEFAULT_STATUS_EMPTY;
		}

		::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_STATUS), statusMessageDefault);
	}
}

//TODO
void Application::RefreshUI()
{
	wchar_t buffer[64];
	buffer[0] = '\0';

	//if (avgSnr->IsFull())
	if (avgSnr.GetCount() > 0)
	{
		_snwprintf_s(buffer, 64, _TRUNCATE, L"%f", avgSnr.GetAverage());
		::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_SNR), buffer);
		avgSnr.Clear();
	}
	else
	{
		::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_SNR), L"");		
	}


	//if (avgDcI.IsFull())
	if (avgDcI.GetCount() > 0)
	{
		_snwprintf_s(buffer, 64, _TRUNCATE, L"%f", avgDcI.GetAverage());
		::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_DC_I), buffer);
		avgDcI.Clear();
	}
	else
	{
		::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_DC_I), L"");		
	}

	//if (avgDcQ.IsFull())
	if (avgDcQ.GetCount() > 0)
	{
		_snwprintf_s(buffer, 64, _TRUNCATE, L"%f", avgDcQ.GetAverage());
		::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_DC_Q), buffer);
		avgDcQ.Clear();
	}
	else
	{
		::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_DC_Q), L"");		
	}

	//if (avgFreqOffset.IsFull())
	if (avgFreqOffset.GetCount() > 0)
	{
		_snwprintf_s(buffer, 64, _TRUNCATE, L"%f", avgFreqOffset.GetAverage());
		::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_FREQ_OFFSET), buffer);
		avgFreqOffset.Clear();
	}
	else
	{
		::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_FREQ_OFFSET), L"");		
	}

	//if (avgIQA.IsFull())
	if (avgIQA.GetCount() > 0)
	{
		_snwprintf_s(buffer, 64, _TRUNCATE, L"%f", avgIQA.GetAverage());
		::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_IQA), buffer);
		avgIQA.Clear();
	}
	else
	{
		::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_IQA), L"");		
	}

	//if (avgIQP.IsFull())
	if (avgIQP.GetCount() > 0)
	{
		_snwprintf_s(buffer, 64, _TRUNCATE, L"%f", avgIQP.GetAverage());
		::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_IQP), buffer);
		avgIQP.Clear();
	}
	else
	{
		::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_IQP), L"");		
	}
}

void Application::ClearUIData()
{
	::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_SNR), L"");
	::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_DC_I), L"");	
	::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_DC_Q), L"");	
	::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_FREQ_OFFSET), L"");
	::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_IQA), L"");
	::SetWindowText(GetDlgItem(hwnd, IDC_EDIT_IQP), L"");
}

void Application::ClearData()
{
	avgSnr.Clear();
	avgDcI.Clear();
	avgDcQ.Clear();
	avgFreqOffset.Clear();
	avgIQA.Clear();
	avgIQP.Clear();
}

void Application::DlgLog(int type, wchar_t * string)
{
	if (type == LOG_STATUS)
	{
		if (statusMessage)
		{
			delete statusMessage;
		}
		statusMessage = string;
	}
	else
	{
		long nBegin, nEnd;

		HWND hRichEdit = GetDlgItem(hwnd, IDC_RICHEDIT21_LOG);

		CHARRANGE selection;

		// Set Selection
		selection.cpMin = -1;
		selection.cpMax = -1;
		SendMessage(hRichEdit, EM_EXSETSEL, 0, (LPARAM)&selection);

		// Get selection
		SendMessage(hRichEdit, EM_EXGETSEL, 0, (LPARAM)&selection);
		nBegin = selection.cpMin;
		nEnd = selection.cpMax;

		// replace selection
		SendMessage(hRichEdit, EM_REPLACESEL, FALSE, (LPARAM)string);

		// select the text just appended
		selection.cpMin = nBegin;
		selection.cpMax = -1;

		SendMessage(hRichEdit, EM_EXSETSEL, 0, (LPARAM)&selection);

		delete [] string;

		CHARFORMAT2 cf;
		cf.cbSize = sizeof(cf);
		//outputWindow->GetDefaultCharFormat(cf);
		::SendNotifyMessage(hRichEdit, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

		cf.dwEffects &= ~CFE_AUTOCOLOR;
		cf.dwMask |= CFM_COLOR;

		switch(type)
		{
		case LOG_DEFAULT:
			cf.crTextColor = RGB(0, 0, 0);
			break;
		case LOG_INFO:
			cf.crTextColor = RGB(0, 0, 0);
			break;
		case LOG_SUCCESS:
			cf.crTextColor = RGB(0, 100, 0);
			break;
		case LOG_ERROR:
			cf.crTextColor = RGB(200, 0, 0);
			break;
		case LOG_FUNC_CALL:
			cf.crTextColor = RGB(0, 0, 150);
			break;
		default:
			cf.crTextColor = RGB(0, 0, 0);
		}

		cf.cbSize = sizeof(cf);
		::SendNotifyMessage(hRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		//outputWindow->SetSelectionCharFormat(cf);

		selection.cpMin = -1;
		selection.cpMax = -1;
		SendMessage(hRichEdit, EM_EXSETSEL, 0, (LPARAM)&selection);

		SendMessage(hRichEdit, WM_VSCROLL, SB_BOTTOM, 0);
	}
}

HRESULT Application::BeaconSenderInit()
{
	HRESULT hr = -1;

	hr = ::SoraUmxInit(Radio::Current());
	if (FAILED(hr))
	{
		hr = -1;
		goto EXIT;
	}

	//beaconSender = new UmxSender(radio);

	static const int SIZE_1M = 1024*1024;
	char * buf = new char[SIZE_1M];
	FILE * fp;

	wchar_t path[MAX_PATH];
	path[0] = '\0';

	wcscat_s(path, exeDir);
	wcscat_s(path, L"\\data\\ofdm.bin");
	fp = _wfopen(path, L"rb");
	//fp = _wfopen(L"data/ofdm.bin", L"rb");
	if (fp == 0)
	{
		logger->Log(LOG_ERROR, L"Failed to open file in beacon initialization. errno = %x.\n", errno);
		return -1;
	}

	int size = fread(buf, sizeof(char), SIZE_1M, fp);
	fclose(fp);

	hr = beaconSender->Init();
	
	if (FAILED(hr))
	{
		logger->Log(LOG_ERROR, L"Beacon initialization failed.\n");
		goto EXIT;
	}
	else
	{
		logger->Log(LOG_SUCCESS, L"Beacon initialization succeeded.\n");
	}

	hr = beaconSender->Transfer(buf, size, 20);
	if (SUCCEEDED(hr))
	{
		beaconSenderInitialized = true;
		logger->Log(LOG_SUCCESS, L"Beacon transfer succeeded.\n");
	}
	else
	{
		logger->Log(LOG_ERROR, L"Beacon transfer failed.\n");
	}

EXIT:
	if (buf)
		delete buf;
	return hr;
}

HRESULT Application::SendBeacon()
{
	if (!beaconSenderInitialized)
	{
		BeaconSenderInit();
	}

	HRESULT hr = -1;

	if (beaconSenderInitialized)
	{
		//int txgain = Radio::MIN_TX_GAIN + Radio::STEP_TX_GAIN;
		//radio->SetConfig(Radio::TX_GAIN, txgain);
		hr = beaconSender->Transmit(true);
		logger->Log(LOG_SUCCESS, L"Beacon is sent.\n");
		BeaconSenderDeinit();
		beaconSenderInitialized = false;
	}
	else
	{
		logger->Log(LOG_ERROR, L"Beacon is not sent.\n");
	}
	return hr;
}

HRESULT Application::BeaconSenderDeinit()
{
	HRESULT hr;
	hr = beaconSender->Deinit();
	//if (beaconSender)
	//{
	//	delete beaconSender;
	//	beaconSender = 0;
	//}
	return hr;
}

//void Application::OnCalibrationDone()
//{
//	::SetWindowText(
//		GetDlgItem(hwnd, IDC_BUTTON_CALIBRATION),
//		L"auto calibration");
//	isAutoCalibrating = false;	
//}

void Application::SetAutoGain(bool autogain)
{
	this->autogain = autogain;

	if ( direction == RECEIVE )
	{
		sineDataProcessor->SetAutoGain(autogain);
		OFDMDataProcessor->SetAutoGain(autogain);		
	}

	ReconfigUI();
}

void Application::SetSampleRate(int mHz)
{
	if (mHz != 40 && mHz != 44)
	{
		logger->Log(LOG_ERROR, L"Sampling rate should be 40 or 44 MHz.\n");
	}
	Radio::Current()->SetConfig(Radio::SAMPLE_RATE, mHz);
}
