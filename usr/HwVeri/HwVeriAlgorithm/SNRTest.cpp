#include <Windows.h>
#include <stdio.h>
#include <dsptool.h>

/****************************************
Sora Viewer - Yet another gui tool for 
              Baseband Test
              
Should it be a replacement of MatLab?              
****************************************/

#include "const.hpp"
#include "context.hpp"
#include "decoder11a.hpp"
#include "sora-stream.hpp"

#include "configure.hpp"

#include "SNRTest.h"
#include "Message.h"

//static Logger * logger = Logger::GetLogger(L"alg");

//void __cdecl BrickPrint(wchar_t * format, ...)
//{
//	Logger * logger = Logger::GetLogger(L"alg");
//	wchar_t buffer[256];
//
//	va_list ap;
//	va_start(ap, format);
//	_vsnwprintf(buffer, 255, format, ap);
//	va_end(ap);
//
//	logger->Log(0, buffer);
//}

//void _cdecl Dot11ARxApp(ULONG count);
//void _cdecl Dot11ATxApp(ULONG count);
//void _cdecl Dot11ARxAppOffline2(PRX_BLOCK, int);
//void _cdecl Dot11ATxAppOffline(const char* fileName);
//void _cdecl Dot11ATxAppOfflineDump(const char* fileName);
//void _cdecl GenPreamble();


SNRDataProcessor::SNRDataProcessor(HWND hWnd)
	: dumpSampleCount		(0)
	, m_offlineDone			(false)
{

	m_hWnd = hWnd;

	agc = new AGC();

	const int AMP = 1200;

	wndOverview = new CAnimWnd ( );
	wndOverview->Create ( "overview", 0, 0, 320, 320 );
	wndOverview->EraseCanvas ();
	wndOverview->SetScale ( -AMP, AMP );

	seriesOverview = new Series<double>(100);
	drawline = new DrawLine(wndOverview, this->m_hWnd);
	drawline->SetColor(RGB(0,255,0));
	seriesOverview->SetDrawMethod(drawline);

	overviewSnapshotCnt = 200;
	plotOverview = new PlotImpl<double>(seriesOverview, overviewSnapshotCnt);


	wndConstel = new CAnimWnd ( );
	wndConstel->Create ( "constellation", 0, 0, 320, 320 );
	wndConstel->EraseCanvas ();
	wndConstel->SetScale ( -12000, 12000 );

	seriesConstel = new Series<COMPLEX>(300);
	drawdots = new DrawDots(wndConstel, this->m_hWnd);
	drawdots->SetColor(RGB(255,0,0));
	seriesConstel->SetDrawMethod(drawdots);

	constelSnapshotCnt = 600;
	plotConstel = new PlotImpl<COMPLEX>(seriesConstel, constelSnapshotCnt);

	plotPlayer = new PlotPlayer();
	plotPlayer->AddPlot(plotOverview);
	plotPlayer->AddPlot(plotConstel);
	plotPlayer->SetTimeIntervalMS(1);

	dumpBuf = 0;

	autogain = true;

	logger = Logger::GetLogger(L"alg");
}

SNRDataProcessor::~SNRDataProcessor() {

	if (dumpBuf)
	{
		delete [] dumpBuf;
		dumpBuf = 0;
	}

	delete agc;

	plotPlayer->Stop(true);
	delete plotPlayer;
	
	delete plotConstel;
	delete seriesConstel;
	::PostMessage(m_hWnd, WM_DESTORY_PLOT, (WPARAM)wndConstel, 0);
	//::PostMessage(wndConstel->HWnd(), WM_DESTROY, 0, 0);
	//delete wndConstel;

	delete plotOverview;
	delete seriesOverview;
	::PostMessage(m_hWnd, WM_DESTORY_PLOT, (WPARAM)wndOverview, 0);
	//::PostMessage(wndOverview->HWnd(), WM_DESTROY, 0, 0);
	//delete wndOverview;
}

void SNRDataProcessor::Init()
{
	lastAutoGain = false;

	if (dumpBuf == 0)
	{
		dumpBuf = new char[DUMP_BUF_LEN];
		dumpBufValidCnt = 0;	
	}

	sBuffer.ctx_.snrPlot = plotConstel;
	sBuffer.ctx_.hWnd_ = m_hWnd;

	HWND hwndDesktop = GetDesktopWindow();
	RECT rectDesktop;
	GetWindowRect(hwndDesktop, &rectDesktop);

	RECT rect;
	GetWindowRect(m_hWnd, &rect);

	const int WND_LEN = 320;

	RECT rectPlot;
	rectPlot.left = rect.right + 10;
	rectPlot.top = rect.top;

	if (rectPlot.left > rectDesktop.right)
		rectPlot.left = rectDesktop.right - WND_LEN;
	if (rectPlot.top > rectDesktop.bottom)
		rectPlot.top = rectDesktop.bottom - 2*WND_LEN;

	::SetWindowPos(wndOverview->HWnd(), HWND_TOP, rectPlot.left, rectPlot.top, WND_LEN, WND_LEN, 0);

	rectPlot.left = rect.right + 10;
	rectPlot.top = rect.top + WND_LEN + 10;
	if (rectPlot.left > rectDesktop.right)
		rectPlot.left = rectDesktop.right - WND_LEN;
	if (rectPlot.top > rectDesktop.bottom)
		rectPlot.top = rectDesktop.bottom - WND_LEN;

	::SetWindowPos(wndConstel->HWnd(), HWND_TOP, rectPlot.left, rectPlot.top, WND_LEN, WND_LEN, 0);
	wndOverview->EraseCanvas();
	wndOverview->ShowWindow();

	wndConstel->EraseCanvas();
	wndConstel->ShowWindow();

	plotOverview->Clear();
	plotConstel->Clear();

	plotPlayer->Start(false);
}

void SNRDataProcessor::Deinit()
{
	if (dumpBuf)
	{
		delete [] dumpBuf;
		dumpBuf = 0;
	}

	plotPlayer->Stop(false);
	::SendNotifyMessage(m_hWnd, WM_SHOW_WINDOW, (WPARAM)wndOverview->HWnd(), FALSE);
	::SendNotifyMessage(m_hWnd, WM_SHOW_WINDOW, (WPARAM)wndConstel->HWnd(), FALSE);
}

void SNRDataProcessor::ProcessData(PRX_BLOCK buf, int count)
{
	int remainingCount = count*sizeof(RX_BLOCK);
	char * dumpSrcBuf = (char *)buf;

	while(remainingCount > 0)
	{
		int dumpCount = DUMP_BUF_LEN - dumpBufValidCnt;
		dumpCount = min(dumpCount, remainingCount);

		memcpy(dumpBuf + dumpBufValidCnt, dumpSrcBuf, dumpCount);
		dumpBufValidCnt += dumpCount;
		dumpSrcBuf += dumpCount;
		remainingCount -= dumpCount;

		if (dumpBufValidCnt >= DUMP_BUF_LEN)
		{
			::PostMessage(m_hWnd, WM_NEW_DUMP_DATA, DUMP_BUF_LEN, (LPARAM)dumpBuf);
			dumpBuf = new char[DUMP_BUF_LEN];
			dumpBufValidCnt = 0;
		}
	}

	for (int iBuf = 0; iBuf < count; iBuf++)
	{
		for (int idxComplex = 0; idxComplex < 28; idxComplex++)
		{

			PCOMPLEX16 pSample = (PCOMPLEX16)&(buf[iBuf].u.SampleBlock);

			COMPLEX16 sample;

			sample.re = pSample[idxComplex].re;
			sample.im = pSample[idxComplex].im;

			if (dumpSampleCount < DUMP_SAMPLE_MAX)
			{
				dumpRawData[dumpSampleCount].re = sample.re;
				dumpRawData[dumpSampleCount].im = sample.im;

				dumpData[dumpSampleCount].re = sample.re;
				dumpData[dumpSampleCount].im = sample.im;
				dumpSampleCount++;
			}
			else
			{
				int agcCount = min(dumpSampleCount, 128*1024);
				bool agcOK = true;
				if (autogain)
				{
					if (lastAutoGain == false)
					{
						logger->Log(LOG_INFO, 
							L"AGC enabled\n"
							L"AGC module calls SoraURadioSetRxGain and SoraURadioSetRxPa to set gain.\n"
							L"Calls to these two functions are not logged.\n"
							);
					}

					agc->CalcData(dumpData, agcCount);
					
					int curDB = Radio::Current()->GetRxGainDB();
					int db = agc->AdjustByRawData(curDB);
					
					if (db != curDB)
					{
						Logger::GetLogger(L"umx")->Enable(false);
						Radio::Current()->SetRxGainDB(db);
						Logger::GetLogger(L"umx")->Enable(true);
						agcOK = false;
					}
				}
				else
				{
					if (lastAutoGain == true)
					{
						logger->Log(LOG_INFO, L"AGC disabled\n");
					}
					agc->Reset();
				}

				lastAutoGain = autogain;

				if (!agcOK)
				{
					::SendNotifyMessage(m_hWnd, WM_UPDATE_UI, 0, 0);
				}

				//for (int i = dumpSampleCount - constelSnapshotCnt; i < dumpSampleCount; i++)
				//{
				//	double buf[2];
				//	buf[0] = dumpData[i].re * 1000 / 32768;
				//	buf[1] = dumpData[i].im * 1000 / 32768;

				//	plotConstel->PushData(buf, 2);
				//}

				double sum_re = 0.0;
				double sum_im = 0.0;

				for (int i = 0; i < dumpSampleCount; i++)
				{
					double re = dumpData[i].re;
					double im = dumpData[i].im;

					sum_re += re;
					sum_im += im;
				}

				double dcRe = sum_re/dumpSampleCount;
				double dcIm = sum_im/dumpSampleCount;

				// removeDC
				for (int i = 0; i < dumpSampleCount; i++)
				{
					dumpData[i].re -= dcRe;
					dumpData[i].im -= dcIm;
				}

				for (int i = 0; i < dumpSampleCount; i++)
				{
					double re = dumpData[i].re;
					double im = dumpData[i].im;
					double square = re*re + im*im;
					if (square == 0.0)
					{
						energy[i] = 0.0;
					}
					else
					{
						double amp = log(square) - 10.0;
						energy[i] = max(0.0, amp);
					}
				}

				double * dcI = new double[1];
				*dcI = dcRe;
				::SendNotifyMessage(m_hWnd, WM_UPDATE_DATA, DATA_DC_I, (LPARAM)dcI);

				double * dcQ = new double[1];
				*dcQ = dcIm;
				::SendNotifyMessage(m_hWnd, WM_UPDATE_DATA, DATA_DC_Q, (LPARAM)dcQ);

				double avgEnergy = 0;
				int idxEgyPlot = 0;
				int plotDataCntMax = min(EGY_PLOT_COUNT, overviewSnapshotCnt);

				for (int i = 0; i < dumpSampleCount; i++)
				{
					avgEnergy += energy[i];

					if ( ((i % EGY_AVG_COUNT) == 0) && (i != 0) )
					{
						avgEnergy /= EGY_AVG_COUNT;
						egyDataPlot[idxEgyPlot++] = avgEnergy;
						avgEnergy = 0;
						if (idxEgyPlot == plotDataCntMax)
							break;
					}
				}

				plotOverview->PushData(egyDataPlot, idxEgyPlot);

				if (autogain && !agcOK)
				{
					dumpSampleCount = 0;
					return;
				}

				sBuffer.ctx_.Reset ();
				sBuffer.ctx_.reset_ = false;
				bool ret = sBuffer.SetBuffer((COMPLEX *)dumpData, dumpSampleCount);

				while (1) {

					bool ret = sBuffer.Process();
					if (!ret)
						break;

					if (sBuffer.ctx_.reset_ ) {
						//break;
						sBuffer.Reset ();
					} else if ( sBuffer.ctx_.bAbort_) {
						//break;
						sBuffer.ctx_.Reset();
						break;
					}

					if ( sBuffer.ctx_.bPause_ ) {
						sBuffer.ctx_.bPause_ = false;
					}
				}

				dumpSampleCount = 0;
			}

			//PlotData(m_channelEnergy, sample.re, sample.im);
		}
	}
}

void SNRDataProcessor::SetAutoGain(bool autogain)
{
	this->autogain = autogain;
}

void SNRDataReader::StoreFileName(const wchar_t * filename)
{
	int len = wcslen(filename);
	m_fileName = new wchar_t[len+1];
	//wcscpy(m_fileName, filename);
	wcscpy_s(m_fileName, len+1, filename);
}

wchar_t * SNRDataReader::GetFileName()
{
	return m_fileName;
}

void SNRDataReader::ReleaseFileName()
{
	delete [] m_fileName;
}

SNRDataReader::SNRDataReader(const wchar_t * filename) : m_bFirstRead(true)
{
	logger = Logger::GetLogger(L"alg");
	StoreFileName(filename);
}

SNRDataReader::~SNRDataReader()
{
	ReleaseFileName();
}

int SNRDataReader::ReadData(char * buf, int count)
{

	if (m_bFirstRead)
	{
		FILE * fp;
		fp = _wfopen(GetFileName(), L"rb");
		//errno_t ret = _wfopen_s(&fp, GetFileName(), L"rb");
		//fp = _wfopen(GetFileName(), L"rb");
		if (fp == 0)
		{
			logger->Log(LOG_ERROR, L"Failed to open file. errno = %x\r\n", errno);
			return -1;
		}

		int size = fread(buf, sizeof(char), count, fp);
		fclose(fp);

		m_bFirstRead = false;

		return size;
	}
	else
		return 0;
}

void SNRDataReader::Reset()
{
	m_bFirstRead = true;
}

