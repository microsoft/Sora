#include <stdarg.h>
#include "complex.h"
#include "SineWaveTest.h"
#include "Message.h"
#include <SetupAPI.h>
#include <Windows.h>
#include <Windef.h>
#include "FFTAllIn1.h"

#define PI 3.141593

//static double g_centralFreq;
//static HANDLE g_hMutex = 0;
//static const int SEND_FREQ = (1000*1000);



#define ALLIGN16(p)		((unsigned int)(p) + 16 - (unsigned int)(p)%16)

SineDataProcessor::SineDataProcessor(HWND hWnd, Radio * radio)
{
	this->m_hWnd = hWnd;

	dumpSampleCount = 0;

	//sumIQA = 0;
	//sumIQP = 0;
	//avgCount = 0;

	//rwMutex = CreateMutex( 
 //       NULL,              // default security attributes
 //       FALSE,             // initially not owned
 //       NULL);             // unnamed mutex

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
	wndConstel->SetScale ( -AMP, AMP );

	seriesConstel = new Series<COMPLEX>(100);
	drawdots = new DrawDots(wndConstel, this->m_hWnd);
	drawdots->SetColor(RGB(255,0,0));
	seriesConstel->SetDrawMethod(drawdots);	//TODO

	int constelSnapshotCnt = 150;
	plotConstel = new PlotImpl<COMPLEX>(seriesConstel, constelSnapshotCnt);

	plotPlayer = new PlotPlayer();
	plotPlayer->AddPlot(plotOverview);
	plotPlayer->AddPlot(plotConstel);
	plotPlayer->SetTimeIntervalMS(1);

	dumpBuf = 0;

	autogain = true;

	fftInput = new char[sizeof(COMPLEX16)*FFT_LEN+FFT_ALIGN+1];//COMPLEX16[FFT_LEN + FFT_ALIGN];
	fftOutput = new char[sizeof(COMPLEX16)*FFT_LEN+FFT_ALIGN+1];//COMPLEX16[FFT_LEN + FFT_ALIGN];
	fftInputStart = (COMPLEX16 *)ALLIGN16(fftInput);
	fftOutputStart = (COMPLEX16 *)ALLIGN16(fftOutput);

	logger = Logger::GetLogger(L"alg");
}

SineDataProcessor::~SineDataProcessor()
{
	//if (rwMutex)
	//	ReleaseMutex(rwMutex);

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

	delete plotOverview;
	delete seriesOverview;
	::PostMessage(m_hWnd, WM_DESTORY_PLOT, (WPARAM)wndOverview, 0);

	delete fftInput;
	delete fftOutput;
}

void SineDataProcessor::Init()
{

	statusTotalPkgNum = 0;
	status40MPkgNum = 0;
	status44MPkgNum = 0;
	statusValidPkgNum = 0;

	sampleRateNotMatchMessagePrinted = false;

	lastAutoGain = false;

	if (dumpBuf == 0)
	{
		dumpBuf = new char[DUMP_BUF_LEN];
		dumpBufValidCnt = 0;	
	}

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

void SineDataProcessor::Deinit()
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

void SineDataProcessor::ProcessData(PRX_BLOCK buf, int count)
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

				removeDCOffset(dumpData, dumpSampleCount);

				calEnergy(dumpData, egyData, dumpSampleCount);

				int pkg_num = searchPkg(egyData, pkg_start, pkg_end);

				// Plot
				double maxAmp = 0;
				double sumAmp = 0;
				for (int i = 0; i < dumpSampleCount; i++)
				{
					double re = abs(dumpData[i].re);
					double im = abs(dumpData[i].im);
					maxAmp = max(maxAmp, max(re, im));
					sumAmp += re*re + im*im;
				}

				double avgAmp = sumAmp / dumpSampleCount;
				int magPlot = (int)(sqrt(avgAmp) * 2);

				int drawStart, drawEnd;
				if (pkg_num > 0)
				{
					drawStart = pkg_start[0];
					drawEnd = pkg_end[0];
				}
				else
				{
					drawStart = 0;
					drawEnd = 512;
				}

				for (int i = drawStart; i < drawEnd; i++)
				{
					int I = dumpRawData[i].re * 1000 / 32768;
					int Q = dumpRawData[i].im * 1000 / 32768;

					double buf[2];
					buf[0] = I;
					buf[1] = Q;
					plotConstel->PushData(buf, 2);
				}

				double avgEnergy = 0;
				int idxEgyPlot = 0;
				double maxAvgEnergy = 0;

				int egyStart = 0;
				int egyEnd = dumpSampleCount;

				if (pkg_num >= 3)
				{
					egyStart = pkg_end[0] + 8192;
					egyStart = min(egyStart, 65536);
					for (int i = 1; i < pkg_num - 1; i++)
					{
						egyEnd = pkg_end[i] + 8192;
						if ( (egyEnd - egyStart) / EGY_AVG_COUNT > overviewSnapshotCnt )
						{
							//logger->Log(LOG_INFO, L"plot 1: %d\n", (egyEnd - egyStart) / EGY_AVG_COUNT);
							break;
						}
					}
				}

				//logger->Log(LOG_INFO, L"plot 2: %d, pkg_num: %d\n", (egyEnd - egyStart) / EGY_AVG_COUNT, pkg_num);

				//for (int i = 0; i < dumpSampleCount; i++)
				int plotDataCntMax = min(EGY_PLOT_COUNT, overviewSnapshotCnt + 64);
				int periodPoint = egyStart % EGY_AVG_COUNT;
				for (int i = egyStart; i < egyEnd; i++)
				{
					avgEnergy += egyDataRaw[i];
					if ( (i != egyStart ) && ((i % EGY_AVG_COUNT) == periodPoint) )
					{
						avgEnergy /= EGY_AVG_COUNT;
						egyDataPlot[idxEgyPlot++] = avgEnergy;
						if (maxAvgEnergy < avgEnergy)
							maxAvgEnergy = avgEnergy;

						avgEnergy = 0;

						if (idxEgyPlot == plotDataCntMax )
							break;
					}
				}

				//logger->Log(LOG_INFO, L"plot count: %d\n", idxEgyPlot);

				plotOverview->PushData(egyDataPlot, idxEgyPlot);

				if (autogain && !agcOK)
				{
					dumpSampleCount = 0;
					return;
				}

				if (pkg_total_num > 0)
				{
					const int TEST_SATURATION_CNT = pkg_end[0] - pkg_start[0];
					int saturationCount = 0;
					const double THRESHOLD = log((double)32700*32700) - 10;
					for (int i = pkg_start[0]; i < pkg_end[0]; i++)
					{
						
						if ( egyDataRaw[i] > THRESHOLD )
						{
							saturationCount++;
						}
					}

					if (saturationCount > (TEST_SATURATION_CNT/16))
					{
						saturation = true;
					}
					else
					{
						saturation = false;
					}


					if (!saturation)
						logger->Log(LOG_STATUS, L"Valid package detected.");
					else
						logger->Log(LOG_STATUS, L"Saturation detected.");

					double centralFrequency2 = 0.0;
					for (int i = 0; i < pkg_total_num; i++)
					{
						centralFrequency2 += CentralFreq(i);
					}

					centralFrequency2 /= pkg_total_num;

					int total = (*pkg_end - *pkg_start + 1) / 40 - 5;
					for (int i = 0; i < pkg_total_num; i++)
					{
						detectPeriod(dumpData, i, total, 1);
						detectPeriod(dumpData, i, total, 0);
						detectPhaseDiff(dumpData, i, total);
					}

					//double centralFreq = 0.0;
					double imbAmp = 0.0;
					double imbPhase = 0.0;

					int validCount = 0;

					for (int i = 0; i < pkg_total_num; i++)
					{
						if (pkgValid[i])
						{
							//centralFreq += *(centralFrequency+i);		
							imbAmp += A_re[i]/A_im[i];
							imbPhase += phaseDiff[i];
							validCount++;
						}
					}


					if (validCount > 0)
					{
						//centralFreq /= validCount;
						imbAmp /= validCount;
						imbPhase /= validCount;

						//double freqOffset = centralFreq - SEND_FREQ;

						double sendFreq = (double)1000*1000*Radio::Current()->GetConfig(Radio::SAMPLE_RATE)/40;

						UpdateUI(centralFrequency2, centralFrequency2 - sendFreq, imbAmp, imbPhase);

						//SetCentralFreq(centralFreq);
					}
				}
				else
				{
				}

				dumpSampleCount = 0;

				// draw a boundary
				//PlotData(m_channelEnergy, -1000);

				return;
			}
		}
	}	
}

void SineDataProcessor::removeDCOffset(Complex * Data, int SampleCount)
{
	const int DC_TRAIN_LEN = 100000;
	Complex sum = { 0, 0 };

	int count = DC_TRAIN_LEN;
	if (count > SampleCount)
		count = SampleCount;

	for (int i = 0; i < count; i++)
		cadd(sum, Data[i]);

	Complex dcOffset;
	cdiv(dcOffset, sum, count);


	double * dcI = new double[1];
	*dcI = dcOffset.re;
	::SendNotifyMessage(m_hWnd, WM_UPDATE_DATA, DATA_DC_I, (LPARAM)dcI);



	//wchar_t *str;

	//avgDCI.Add(dcOffset.re);

	//if (avgDCI.GetCount() > 20)
	//{
	//	double * dcI = new double[1];
	//	*dcI = avgDCI.GetAverage();
	//	::SendNotifyMessage(m_hWnd, WM_UPDATE_DATA, DATA_DC_I, (LPARAM)dcI);
	//	avgDCI.Clear();
	//}


	double * dcQ = new double[1];
	*dcQ = dcOffset.im;
	::SendNotifyMessage(m_hWnd, WM_UPDATE_DATA, DATA_DC_Q, (LPARAM)dcQ);

	//avgDCQ.Add(dcOffset.im);
	//if (avgDCQ.GetCount() > 20)
	//{
	//	double * dcQ = new double[1];
	//	*dcQ = avgDCQ.GetAverage();
	//	::SendNotifyMessage(m_hWnd, WM_UPDATE_DATA, DATA_DC_Q, (LPARAM)dcQ);
	//	avgDCQ.Clear();
	//}

	for (int i = 0; i < SampleCount; i++)
		csub(Data[i], dcOffset);
}

// re_flag == 1: search in real part of dumpData
// re_flag == 0; search in image part of dumpData
int SineDataProcessor::detectOnePeriod(Complex* dumpData, int pkg_end, int start_peak, int re_flag, double *max, double *min)
{
	int i, pos;
	int max_i;
	int positive_flag;

	for (i=start_peak; i<pkg_end; i++)
	{
		if (re_flag == 1)
		{
			if (dumpData[i].re > 0)
				break;
		}
		else
		{
			if (dumpData[i].im > 0)
				break;
		}
	}	

	if (i == pkg_end)
	{
		return -1;
	}

	// detect the next negtive trough
	for (; i<pkg_end; i++)
	{
		if (re_flag == 1)
		{
			if (dumpData[i].re < 0)
				break;
		}
		else
		{
			if (dumpData[i].im < 0)
				break;
		}
	}

	if (i == pkg_end)
	{
		return -1;
	}

	pos = i;

	// detect the positive peak before next negtive trough
	*max = 0;
	max_i = pos;
	positive_flag = 0;
	for (i=pos; i<pkg_end; i++)
	{
		if (re_flag == 1)
		{
			if (dumpData[i].re > *max)
			{
				*max = dumpData[i].re;
				max_i = i;
				positive_flag = 1;
			}

			if ( (positive_flag == 1) && (dumpData[i].re < 0) )
				break;
		}
		else
		{
			if (dumpData[i].im > *max)
			{
				*max = dumpData[i].im;
				max_i = i;
				positive_flag = 1;
			}

			if ( (positive_flag == 1) && (dumpData[i].im < 0) )
				break;
		}
	}
	pos = i;

	// detect minimon value
	*min = 0;
	for (i=pos; i<pkg_end; i++)
	{
		if (re_flag == 1)
		{
			if (dumpData[i].re < *min)
				*min = dumpData[i].re;

			if ( dumpData[i].re > 0 )
				break;
		}
		else
		{
			if (dumpData[i].im < *min)
				*min = dumpData[i].im;

			if ( dumpData[i].im > 0 )
				break;
		}
	}

	return (max_i); // return the next peak of start_peak
}

void SineDataProcessor::detectPeriod(Complex* dumpData, int pkg_num, int total, int re_flag)
{
	int i, pos1, pos2;
	//int *p = (int *)malloc(total*sizeof(int));
	int p;
	//double *max = (double *)malloc(sizeof(double));
	//double *min = (double *)malloc(sizeof(double));
	double max, min;
	double max_avg;

	int validCount = 0;

	// got the total periods
	p = 0;
	max_avg = 0;
	pos1 = detectOnePeriod(dumpData, *(pkg_end+pkg_num), *(pkg_start+pkg_num), re_flag, &max, &min);

	if (pos1 != -1)
		for (i=0; i<total; i++)
		{
			pos2 = detectOnePeriod(dumpData, *(pkg_end+pkg_num), pos1, re_flag, &max, &min);
			if (pos2 == -1)
				break;

			p += pos2 - pos1;
			pos1 = pos2;
			max_avg += (max - min) / 2;

			validCount++;
		}

	// average peak
	if (re_flag)
	{
		if (validCount > 0)
		{
			*(period_re+pkg_num) = (double)p / total;
			*(A_re+pkg_num) = max_avg / total;
			return;// (*(period_re+pkg_num));
		}
		else
		{
			pkgValid[pkg_num] = FALSE;
			return;
		}
	}
	else
	{
		if (validCount > 0)
		{
			*(period_im+pkg_num) = (double)p / total;
			*(A_im+pkg_num) = max_avg / total;
			return;// (*(period_im+pkg_num));
		}
		else
		{
			pkgValid[pkg_num] = FALSE;
			return;
		}
	}   
}

double SineDataProcessor::detectPhaseDiff(Complex* dumpData, int pkg_num, int total)
{
	int i, pos1, pos2;
	double p;
	//double *max = (double *)malloc(sizeof(double));
	//double *min = (double *)malloc(sizeof(double));

	double max, min;

	if (!pkgValid[pkg_num])
	{
		return 0.0;
	}

	// got the total periods
	p = 0;
	pos1 = pos2 = *(pkg_start+pkg_num);
	for (i=0; i<total; i++)
	{
		pos1 = detectOnePeriod(dumpData, *(pkg_end+pkg_num), pos1, 1, &max, &min);
		pos2 = detectOnePeriod(dumpData, *(pkg_end+pkg_num), pos2, 0, &max, &min);
		p += pos2 - pos1;
	}

	p = (p / total);
	for (i=0; p-*(period_re+pkg_num) > 0; i++)
		p -= *(period_re+pkg_num);
	for (i=0; p<0; i++)
		p += *(period_re+pkg_num);
	//p = (p > 0) ? p : (*(period_re+pkg_num) + p) ;
	p = (p - *(period_re+pkg_num)/4) * 2* PI / *(period_re+pkg_num);

	*(phaseDiff+pkg_num) = p;

	return (p);
}

void SineDataProcessor::calEnergy(Complex* input, double *output, int sampleNum)
{
	int i;
	int l;
	double l_avg;

	l_avg = 0;
	for(i=0; i<sampleNum; i++)
	{
		double square = input[i].re * input[i].re + input[i].im * input[i].im;
		l = (int)log(square);

		if (square == 0.0)
		{
			egyDataRaw[i] = 0.0;
		}
		else
		{
			egyDataRaw[i] = log(square) - 10.0;
			egyDataRaw[i] = max(0.0, egyDataRaw[i]);
		}

		if (l<0) 
			l = 0;
		output[i] = l;
		l_avg += l;
	}
	l_avg /= sampleNum;

	for(i=0; i<sampleNum; i++)
	{
		output[i] -= l_avg;
		//output[i] -= 11;
		if (output[i] < 0)
			output[i] = 0;
	}
}

int SineDataProcessor::searchPkg(double* input, int *pkg_start, int *pkg_end)
{

	int i, j;
	int length = 100;
	int num_zero; // zero number within length
	int pos; // record position of i

	for (int i = 0; i < PKG_MAX; i++)
	{
		pkgValid[i] = TRUE;
	}

	// initial check
	// check first length values
	num_zero = 0;
	for (i=0; i<length; i++)
	{
		if (input[i] == 0)
			num_zero ++;
	}
	pos = i;

	pkg_total_num = 0;

	//logger->Log(0, L"search PKG\n");

	while(1)
	{
		// check for first 100 zeros
		for (i=pos, j=pos-99; num_zero!=length && i<dumpSampleCount; i++, j++)
		{
			if (input[i] == 0)
				num_zero ++;
			if (input[j-1] == 0)
				num_zero --;
		}
		if (i == dumpSampleCount)
			break;
			//return(pkg_total_num);
		pos = i;

		// search for the beginning of the first complete package
		// check for first 100 non-zeros
		for (i=pos, j=pos-99; num_zero!=0 && i<dumpSampleCount; i++, j++)
		{
			if (input[i] == 0)
				num_zero ++;
			if (input[j-1] == 0)
				num_zero --;
		}
		if (i == dumpSampleCount)
			break;
			//return(pkg_total_num);
		pos = i;
		//*(pkg_start+pkg_total_num) = j;
		*(pkg_start+pkg_total_num) = i;

		// search for the end of the first complete package
		// check for the first 100 zeros
		for (i=pos, j=pos-99; num_zero!=length && i<dumpSampleCount; i++, j++)
		{
			if (input[i] == 0)
				num_zero ++;
			if (input[j-1] == 0)
				num_zero --;
		}
		if (i == dumpSampleCount)
			break;
			//return(pkg_total_num);
		pos = i;
		//*(pkg_end+pkg_total_num) = j;
		*(pkg_end+pkg_total_num) = j - 100;

		// detect one complete package
		int pkg_len = pkg_end[pkg_total_num] - pkg_start[pkg_total_num];

		//Print(ILogger::PNT_NORMAL, L"(%d)\r\n"), pkg_len);
		
		statusTotalPkgNum++;
		if ( (pkg_len > 16100) && (pkg_len < 16250) )
		{
			pkg_total_num++;
			statusValidPkgNum++;
		}
		else if ( (pkg_len > 14680) && (pkg_len < 14720) )
		{
			status40MPkgNum++;
		}
		else if ( (pkg_len > 17800) && (pkg_len < 17840) )
		{
			status44MPkgNum++;
		}
		else
		{
			//logger->Log(0, L"3 bad pkg %d\n", pkg_len);
		}

		if (statusTotalPkgNum == STATUS_TOTAL_PKG_NUM)
		{
			double ratio40M = (double)status40MPkgNum/statusTotalPkgNum;
			double ratio44M = (double)status44MPkgNum/statusTotalPkgNum;
			
			if (!sampleRateNotMatchMessagePrinted)
			{
				sampleRateNotMatchMessagePrinted = true;
				if (ratio40M > 0.5)
				{
					logger->Log(LOG_ERROR, 
						L"Sampling rates do not match!\n"
						L"It seems that the sampling rate of sender RF board is 44M and "
						L"the sampling rate of receiver RF board is 40M."
						L"The tool could not work with this configuration.\n"
						);
				}

				if (ratio44M > 0.5)
				{
					logger->Log(LOG_ERROR, 
						L"Sampling rates do not match!\n"
						L"It seems that the sampling rate of sender RF board is 40M and "
						L"the sampling rate of receiver RF board is 44M."
						L"The tool could not work with this configuration.\n"
						);			
				}
			}

			statusTotalPkgNum = 0;
			status40MPkgNum = 0;
			status44MPkgNum = 0;
			statusValidPkgNum = 0;
		}
	}

	return(pkg_total_num);
}

double SineDataProcessor::CentralFreq(int pkgNum)
{
	int pkgStart = this->pkg_start[pkgNum];
	int pkgEnd = this->pkg_end[pkgNum];
	int pkgLen = pkgEnd - pkgStart + 1;

	int fftLen = FFT_LEN;

	for (int i = 0; i < fftLen; i++)
	{
		if (i < pkgLen)
		{
			double re = this->dumpData[pkgStart+i].re;
			re = min(re, 32767);
			re = max(re, -32768);
			double im = this->dumpData[pkgStart+i].im;
			im = min(im, 32767);
			im = max(im, -32768);
			fftInputStart[i].re = (short)re;
			fftInputStart[i].im = (short)im;
		}
		else
		{
			fftInputStart[i].re = 0;
			fftInputStart[i].im = 0;
		}
	}

	if ( ( ((unsigned long)fftInputStart & 0x0f) == 0) &&
		( ((unsigned long)fftOutputStart & 0x0f) == 0) )
	{
		if (fftLen == 32768)
			FFT32768(fftInputStart, fftOutputStart);
		else if (fftLen == 16384)
			FFT16384(fftInputStart, fftOutputStart);
	}
	else
	{
		logger->Log(0, L"%x %x\n", fftInputStart, fftOutputStart);
	}

	double maxI = 0;
	double maxDist2 = 0;

	for (int i = 0; i < fftLen; i++)
	{
		double re = fftOutputStart[i].re;
		double im = fftOutputStart[i].im;
		double dist2 = re*re + im*im;
		if (dist2 > maxDist2)
		{
			maxDist2 = dist2;
			maxI = i;
		}
	}

	int sampleFreq = (int)Radio::Current()->GetConfig(Radio::SAMPLE_RATE) * 1000 * 1000;
	double centralFrequency = (double)sampleFreq * maxI / fftLen;

	return centralFrequency;
}

//void SineDataProcessor::fft(Complex * input, Complex * output, int pkg_start, int pkg_end)
//{	
//	int sampleNum, k, nfft;
//	sampleNum = pkg_end - pkg_start + 1;
//	k = (int)ceil(log((double)sampleNum) / log((double)2));
//	nfft = (int)pow(2.0, k);
//
//	static double pr[DUMP_SAMPLE_MAX];
//	static double pi[DUMP_SAMPLE_MAX];
//	static double fr[DUMP_SAMPLE_MAX];
//	static double fi[DUMP_SAMPLE_MAX];
//
//	for (int i=0; i<nfft; i++)
//	{
//		if (i<sampleNum)
//		{
//			pr[i] = input[i+pkg_start].re;
//			pi[i] = input[i+pkg_start].im;
//		}
//		else
//		{
//			pr[i] = 0;
//			pi[i] = 0;
//		}
//	}
//
//	kkfft(pr, pi, nfft, k, fr, fi, 1, 0);
//
//	for (int i=0; i<nfft; i++)	
//	{
//		output[i+pkg_start].re = fr[i];
//		output[i+pkg_start].im = fi[i];
//	}
//}
//
//void SineDataProcessor::kkfft(double pr[], double pi[], int n, int k, double fr[], double fi[], int l, int il)
//{
//	int it,m,is,i,j,nv,l0;
//	double p,q,s,vr,vi,poddr,poddi;
//
//	for (it=0; it<=n-1; it++)
//	{
//		m = it;
//		is = 0;
//		for(i=0; i<=k-1; i++)
//		{ 
//			j = m/2; 
//			is = 2*is+(m-2*j); 
//			m = j;
//		}
//		fr[it] = pr[is]; 
//		fi[it] = pi[is];
//	}
//	//----------------------------
//	pr[0] = 1.0; 
//	pi[0] = 0.0;
//	p = 6.283185306/(1.0*n);
//	pr[1] = cos(p); 
//	pi[1] = -sin(p);
//
//	if (l!=0) 
//		pi[1]=-pi[1];
//
//	for (i=2; i<=n-1; i++)
//	{ 
//		p = pr[i-1]*pr[1]; 
//		q = pi[i-1]*pi[1];
//		s = (pr[i-1]+pi[i-1])*(pr[1]+pi[1]);
//		pr[i] = p-q; 
//		pi[i] = s-p-q;
//	}
//
//	for (it=0; it<=n-2; it=it+2)
//	{ 
//		vr = fr[it]; 
//		vi = fi[it];
//		fr[it] = vr+fr[it+1]; 
//		fi[it] = vi+fi[it+1];
//		fr[it+1] = vr-fr[it+1]; 
//		fi[it+1] = vi-fi[it+1];
//	}
//	m = n/2; 
//	nv = 2;
//
//	for (l0=k-2; l0>=0; l0--)
//	{ 
//		m = m/2; 
//		nv = 2*nv;
//		for(it=0; it<=(m-1)*nv; it=it+nv)
//			for (j=0; j<=(nv/2)-1; j++)
//			{ 
//				p = pr[m*j]*fr[it+j+nv/2];
//				q = pi[m*j]*fi[it+j+nv/2];
//				s = pr[m*j]+pi[m*j];
//				s = s*(fr[it+j+nv/2]+fi[it+j+nv/2]);
//				poddr = p-q; 
//				poddi = s-p-q;
//				fr[it+j+nv/2] = fr[it+j]-poddr;
//				fi[it+j+nv/2] = fi[it+j]-poddi;
//				fr[it+j] = fr[it+j]+poddr;
//				fi[it+j] = fi[it+j]+poddi;
//			}
//	}
//
//	if(l!=0)
//		for(i=0; i<=n-1; i++)
//		{ 
//			fr[i] = fr[i]/(1.0*n);
//			fi[i] = fi[i]/(1.0*n);
//		}
//
//		if(il!=0)
//			for(i=0; i<=n-1; i++)
//			{ 
//				pr[i] = sqrt(fr[i]*fr[i]+fi[i]*fi[i]);
//				if(fabs(fr[i])<0.000001*fabs(fi[i]))
//				{ 
//					if ((fi[i]*fr[i])>0) 
//						pi[i] = 90.0;
//					else 
//						pi[i] = -90.0;
//				}
//				else
//					pi[i] = atan(fi[i]/fr[i])*360.0/6.283185306;
//			}
//			return;
//} 
//
//
//void SineDataProcessor::centralFreq(Complex * fftData, int * fftEgyData, double *centralFrequency, double * fftPeakMirrorRate, int sampleFreq, int pkg_num)
//{
//	int i;
//	int sampleNum, k, nfft;
//	int max_egy, max_egy_mirror;
//	int distance;
//	// old
//	sampleNum = *(pkg_end+pkg_num) - *(pkg_start+pkg_num) + 1;
//	k = (int)ceil(log((double)sampleNum) / log((double)2));
//	nfft = (int)pow(2.0, k);	
//	//
//
//	max_egy = 0;
//	*(max_i+pkg_num) = 0;
//	//
//
//	for (i=*(pkg_start+pkg_num); i<*(pkg_start+pkg_num)+nfft; i++)
//	{
//		fftEgyData[i] = (int)(sqrt(pow(fftData[i].re,2) + pow(fftData[i].im,2)));	//TODO
//		if (fftEgyData[i] > max_egy)
//		{
//			max_egy = fftEgyData[i];
//			*(max_i+pkg_num) = i;
//		}
//	}
//
//	distance = nfft - (*(max_i+pkg_num) - *(pkg_start+pkg_num));
//
//	max_egy_mirror = 0;
//	for (i=*(pkg_start+pkg_num)+distance-50; i<*(pkg_start+pkg_num)+distance+50; i++)
//	{
//		if (fftEgyData[i] > max_egy_mirror)
//		{
//			max_egy_mirror = fftEgyData[i];
//			*(max_i_mirror+pkg_num) = i;
//		}
//	}
//
//	*(centralFrequency+pkg_num) = (float)sampleFreq * distance / nfft;
//	*(fftPeakMirrorRate+pkg_num) = 10 * log( (double)max_egy / max_egy_mirror );
//}

void SineDataProcessor::UpdateUI(double centralFrequency, double freqOffset, double imbAmp, double imbPhase)
{
	if (!saturation)
	{
		double * freqOffsetSend = new double[1];
		*freqOffsetSend = freqOffset;
		::SendNotifyMessage(m_hWnd, WM_UPDATE_DATA, DATA_FREQ_OFFSET, (LPARAM)freqOffsetSend);
	}

	double * imbAmpSend = new double[1];
	*imbAmpSend = imbAmp;
	::SendNotifyMessage(m_hWnd, WM_UPDATE_DATA, DATA_IQ_IMBALANCE_AMP, (LPARAM)imbAmpSend);


	double * imbPhaseSend = new double[1];
	*imbPhaseSend = imbPhase;
	::SendNotifyMessage(m_hWnd, WM_UPDATE_DATA, DATA_IQ_IMBALANCE_PHASE, (LPARAM)imbPhaseSend);

	//sumIQA += imbAmp;
	//sumIQP += imbPhase;

	//avgCount++;

	//if (avgCount == MAX_COUNT)
	//{

	//	double * imbAmpSend = new double[1];
	//	*imbAmpSend = sumIQA/avgCount;
	//	::SendNotifyMessage(m_hWnd, WM_UPDATE_DATA, DATA_IQ_IMBALANCE_AMP, (LPARAM)imbAmpSend);

	//	double * imbPhaseSend = new double[1];
	//	*imbPhaseSend = sumIQP/avgCount;
	//	::SendNotifyMessage(m_hWnd, WM_UPDATE_DATA, DATA_IQ_IMBALANCE_PHASE, (LPARAM)imbPhaseSend);

	//	sumIQA = 0;
	//	sumIQP = 0;
	//	avgCount = 0;
	//}
}

void SineDataReader::Reset()
{
	m_bFirstRead = true;
}

SineDataReader::SineDataReader()
{
	m_bFirstRead = true;
}

int SineDataReader::ReadData(char * buffer, int len)
{
	const int SIZE_1M = 1024*1024;
	len = min(SIZE_1M, len);

	int repeatLen = 4096 * 8; // bytes

	int sfreq = 40*1000*1000;
	int freq = 1000*1000;

	if (m_bFirstRead)
		m_bFirstRead = false;
	else
		return 0;

	for (int i = 0; i < len; i+=2)
	{
		if ( (i/repeatLen) % 2 )
		{
			((char *)buffer)[i] = 0;
			((char *)buffer)[i+1] = 0;
		}
		else
		{
			int A = 127;
			int I = (int)floor( A * cos(2 * PI * freq * (i/2) / sfreq) );
			int Q = (int)floor( A * sin(2 * PI * freq * (i/2) / sfreq) );
			((char *)buffer)[i] = (char)I;
			((char *)buffer)[i+1] = (char)Q;
		}
	}

	return len;
}

//double SineDataProcessor::GetCentralFreq()
//{
//	double cfreq;
//
//	DWORD dwWaitResult;
//	dwWaitResult = WaitForSingleObject( 
//		rwMutex,    // handle to mutex
//		INFINITE);  // no time-out interval
//
//	cfreq = this->cfreq;
//
//	ReleaseMutex(rwMutex);
//
//	return cfreq;
//}
//
//void SineDataProcessor::SetCentralFreq(double cfreq)
//{
//	DWORD dwWaitResult;
//	dwWaitResult = WaitForSingleObject( 
//		rwMutex,    // handle to mutex
//		INFINITE);  // no time-out interval
//
//	this->cfreq = cfreq;
//
//	ReleaseMutex(rwMutex);	
//}

void SineDataProcessor::SetAutoGain(bool autogain)
{
	this->autogain = autogain;
}
