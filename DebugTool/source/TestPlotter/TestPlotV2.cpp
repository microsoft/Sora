#include <assert.h>
#include "DebugPlotU.h"
#include "IChannelBuffer.h"
#include "ChannelBufferImpl.h"
#include "RingBufferWithTimeStamp.h"
#include "FrameWithSizeFilter.h"

#define DATA_SIZE 1024
static int testBuffer[DATA_SIZE];
static COMPLEX16 testBufferDots[DATA_SIZE];

static const int PERIOD = 20;

static const double PI_2 = 3.14 * 2;

static void PrepareData()
{
	for (int i = 0; i < DATA_SIZE; i++)
	{
		testBuffer[i] = (i % 3) * 7;
		testBufferDots[i].re = (short)(100 * sin((double)PI_2 * i / PERIOD));
		testBufferDots[i].im = (short)(100 * cos((double)PI_2 * i / PERIOD));
	}
}

static bool __stdcall ProcessFunction(const char * buffer, int size, void * userData, __int32 userId)
{
	auto writer = (SoraDbgPlot::FrameWithSizeInfoWriter<int> *)userData;
	size_t sizeWritten;
	writer->Write(buffer, size, sizeWritten);
}

static int Test1()
{
	::DebugPlotInit();
	PrepareData();
	//::PlotLine("a test channel", testBuffer, 16);
	for (int i = 0; i < 5; i++)
		::PlotLine("a test channel", testBuffer, 16);

	IChannelBufferReadable * readable = ChannelBuffer::OpenForRead(L"Sora DbgView Global Buffer", 1024, 1024);
	assert(readable);

	RingBufferWithTimeStamp<int> ringBuffer(1024*1024);
	SoraDbgPlot::FrameWithSizeInfoWriter<int> writer(&ringBuffer);

	readable->ReadData(ProcessFunction, &writer);

	for (int i = 0; i < 5; i++)
		::PlotLine("a test channel", testBuffer + 16, 16);

	readable->ReadData(ProcessFunction, &writer);

	::DebugPlotDeinit();

	return 0;
}

static int Test2()
{
	::DebugPlotInit();
	
	PrepareData();
	
	int count = 0;

	while(1)
	{
		::PlotLine("line 1", testBuffer, 16);
		::PlotLine("line 2", testBuffer, 32);
		::PlotSpectrum("spectrum 16", testBuffer, 16);
		::PlotSpectrum("spectrum 32", testBuffer, 32);
		::PlotText("text 1", "%d\n", count);

		for (int i = 0; i < PERIOD; i++)
		{
			testBufferDots[i].re = rand() * rand() % 0x10000;
			testBufferDots[i].im = rand() * rand() % 0x10000;
		}

		::PlotDots("dots 1", testBufferDots, PERIOD);
		::Log("log 1", "current count: %d", count++);

		::Sleep(30);			
	}

	::DebugPlotDeinit();
	
	return 0;
}

static void GenSpectrum(int * ptr, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		int dist = i - size/2;
		ptr[i] = 10000 -dist*dist + rand()%1000;
	}	
}

static int Test3()
{

	const size_t SPECTRUM_SIZE = 511;
	int * spectrumBuf = new int[SPECTRUM_SIZE];
	const size_t BUF_LEN = 127;
	int * OneBuffer1 = new int[BUF_LEN];
	int * ZeroBuffer1 = new int[BUF_LEN];
	int * OneBuffer2 = new int[BUF_LEN];
	int * ZeroBuffer2 = new int[BUF_LEN];
	COMPLEX16 * dotsBuf = new COMPLEX16[BUF_LEN];

	::DebugPlotInit();

	double phase = 0.0;
	unsigned long long count = 0;

	while(1)
	{
		int AMP = (int)(128 * cos(phase));
		for (size_t i = 0; i < BUF_LEN; i++)
		{
			OneBuffer1[i] = AMP;
			ZeroBuffer1[i] = -AMP;
		}

		::PlotLine("test line 1", OneBuffer1, BUF_LEN);
		::PlotLine("test line 1", ZeroBuffer1, BUF_LEN);
		
		AMP = (int)(128 * cos(phase + 3.14));
		for (size_t i = 0; i < BUF_LEN; i++)
		{
			OneBuffer2[i] = AMP;
			ZeroBuffer2[i] = -AMP;
		}

		::PlotLine("test line 2", OneBuffer2, BUF_LEN);
		::PlotLine("test line 2", ZeroBuffer2, BUF_LEN);

		for (size_t i = 0; i < BUF_LEN; i++)
		{
			dotsBuf[i].re = (short)(128 * cos(phase + (double)i*3.14/200));
			dotsBuf[i].im = (short)(128 * sin(phase + (double)i*3.14/200));
		}

		::PlotDots("dots", dotsBuf, BUF_LEN);
		::PlotDots("dots 2", dotsBuf + 2, BUF_LEN / 2);

		::PlotText("text phase", "phase: %f\n", phase);
		::PlotText("text count", "count: %d\n", count);

		::Log("log phase", "phase %f\n", phase);
		::Log("log count", "count %d\n", count);

		GenSpectrum(spectrumBuf, SPECTRUM_SIZE);
		::PlotSpectrum("spectrum 1", spectrumBuf, SPECTRUM_SIZE);
		GenSpectrum(spectrumBuf, 32);
		::PlotSpectrum("spectrum 2", spectrumBuf, 32);
		::Sleep(15);

		phase += 0.05;
		count ++;
	}

	::DebugPlotDeinit();

	delete [] OneBuffer1;
	delete [] ZeroBuffer1;
	delete [] OneBuffer2;
	delete [] ZeroBuffer2;
	delete [] spectrumBuf;
	delete [] dotsBuf;
}


static int Test4()
{
	int buffer[16];
	::DebugPlotInit();

	::PlotText("l1", "%d", 0);
	::PlotLine("l1", buffer, 16);

	::DebugPlotDeinit();

	return 0;
}

int TestPlotV2()
{
	Test3();
	return 0;
}
