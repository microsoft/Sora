
#include <windows.h>
#include <process.h>
#include <math.h>
#include <memory>
#include "DebugPlotU.h"

using namespace std;

static void PrepareData(COMPLEX16 * buf, int size, int egyPeriod)
{
	for (int i = 0; i < size; i++)
	{
		if ((i/egyPeriod)%2)
		{
			const double PI_2 = 3.14 * 2;
			const int AMP = 20000;
			const int PERIOD = 100;
			COMPLEX16 complex;
			complex.re = short(AMP * sin(PI_2 * i / PERIOD));
			complex.im = short(AMP * cos(PI_2 * i / PERIOD));
			buf[i] = complex;
		}
		else
		{
			buf[i].re = 0;
			buf[i].im = 0;
		}
	}
}

#define MAX_NAME 260

volatile unsigned long g_threadId = 0;

void __cdecl ThreadPlot(PVOID param)
{
	unsigned long tid = ::InterlockedIncrement(&g_threadId);
	//int tid = 0;

	const bool bUseSameName = true;

	char * nameRe = new char[MAX_NAME];

	if (bUseSameName)
		sprintf_s(nameRe, MAX_NAME, "re", tid);
	else
		sprintf_s(nameRe, MAX_NAME, "re(%d)", tid);

	char * nameIm = new char[MAX_NAME];

	if (bUseSameName)
		sprintf_s(nameIm, MAX_NAME, "im", tid);
	else
		sprintf_s(nameIm, MAX_NAME, "im(%d)", tid);

	char * nameEnergy = new char[MAX_NAME];
	
	if (bUseSameName)
		sprintf_s(nameEnergy, MAX_NAME, "energy", tid);
	else
		sprintf_s(nameEnergy, MAX_NAME, "energy(%d)", tid);

	char * nameDots = new char[MAX_NAME];
	if (bUseSameName)
		sprintf_s(nameDots, MAX_NAME, "dots", tid);
	else
		sprintf_s(nameDots, MAX_NAME, "dots(%d)", tid);

	char * nameSpectrum = new char[MAX_NAME];
	if (bUseSameName)
		sprintf_s(nameSpectrum, MAX_NAME, "spectrum", tid);
	else
		sprintf_s(nameSpectrum, MAX_NAME, "spectrum(%d)", tid);

	char * nameText1 = new char[MAX_NAME];
	if (bUseSameName)
		sprintf_s(nameText1, MAX_NAME, "text1", tid);
	else
		sprintf_s(nameText1, MAX_NAME, "text1(%d)", tid);

	char * nameText2 = new char[MAX_NAME];
	if (bUseSameName)
		sprintf_s(nameText2, MAX_NAME, "text2", tid);
	else
		sprintf_s(nameText2, MAX_NAME, "text2(%d)", tid);

	char * nameLog = new char[MAX_NAME];
	if (bUseSameName)
		sprintf_s(nameLog, MAX_NAME, "log", tid);
	else
		sprintf_s(nameLog, MAX_NAME, "log(%d)", tid);

	DWORD startTime = ::GetTickCount();

	int time = 0;
	const int PERIOD = 256;
	const int AMP = 32768;
	const float PI_2 = 3.14159f * 2;


	const int PROCESS_SIZE = 64;
	COMPLEX16 readBuf[PROCESS_SIZE];
	int reBuf[PROCESS_SIZE];
	int imBuf[PROCESS_SIZE];
	int energyBuf[PROCESS_SIZE];

	const int SPECTRUM_SIZE = 64;
	int spectrumBuf[SPECTRUM_SIZE];

	PrepareData(readBuf, PROCESS_SIZE, 16);		// generate a sine wave data and write to a 4k buffer

	int count = 0;
	const int PRINT_TIME = 128;

	while(1)
	{
		//DWORD curTime = ::GetTickCount();
		//if (curTime - startTime > 1000*20)	// 1min
		//	break;

		count++;
		if ( (count % PRINT_TIME) == 0)
		{
			printf("still alive: %d", count);
		}

		::Sleep(15);

		double avgEnergy = 0;

		for (int i = 0; i < PROCESS_SIZE; i++)
		{
			int re = readBuf[i].re = short(AMP * sin(PI_2 * time / PERIOD));
			int im = readBuf[i].im = short(AMP * cos(PI_2 * time / PERIOD));
			reBuf[i] = readBuf[i].re;
			imBuf[i] = readBuf[i].im;

			double energyF = log(double(re*re + im*im)) - 10;
			energyF = max(energyF, 0.0);
			avgEnergy += energyF;
			energyBuf[i] = (int)energyF;

			time++;
		}
		avgEnergy /= PROCESS_SIZE;

		for (int i = 0; i < SPECTRUM_SIZE; i++)
		{
			int dist = i - SPECTRUM_SIZE/2;
			spectrumBuf[i] = 10000 -dist*dist + rand()%1000;
		}

		::PlotLine(nameRe, reBuf, PROCESS_SIZE);
		::PlotLine(nameIm, imBuf, PROCESS_SIZE);
		::PlotLine(nameEnergy, energyBuf, PROCESS_SIZE);
		::PlotDots(nameDots, readBuf, PROCESS_SIZE);
		::PlotSpectrum(nameSpectrum, spectrumBuf, SPECTRUM_SIZE);
		::PlotText(nameText1, "%f", avgEnergy);
		::PlotText(nameText2, "%d", time);
		::Log(nameLog, "%d", time);
	}

	delete [] nameRe;
	delete [] nameIm;
	delete [] nameEnergy;
	delete [] nameDots;
	delete [] nameSpectrum;
	delete [] nameText1;
	delete [] nameText2;

	_endthread();

}


#define MAX_THREAD 8

void MT_Test()
{
	::DebugPlotInit();

	HANDLE hThread[MAX_THREAD];
	for (int i = 0; i < MAX_THREAD; i++)
	{
		hThread[i] = (HANDLE) ::_beginthread(ThreadPlot, 0, 0);
	}

	printf("Waiting for threads\n");
	::WaitForMultipleObjects(MAX_THREAD, hThread, TRUE, INFINITE);
	printf("All threads have ended\n");

	::DebugPlotDeinit();
}
