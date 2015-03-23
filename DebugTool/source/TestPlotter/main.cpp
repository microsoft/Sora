// TestPlotter.cpp : Defines the entry point for the console application.
//

#include <Windows.h>
#include <assert.h>
#include "DebugPlotU.h"
#ifdef DEBUG_USE_VLD
#include "vld.h"
#endif

extern bool Sample();
extern void MT_Test();
extern void TestHashTable();
extern void TestVector();
extern void TestLog();
extern int TestFileLog();
extern int TestChannelBuffer();
extern int TestRingBuffer();
extern int TestRingBufferWithTimeStamp();
extern int TestDataFilter();
extern int TestPlotV2();
extern int TestEvent();
extern int TestTaskQueue();
extern int TestStrategy();
extern int TestTaskQueue2();
extern int TestRecursiveLock();
extern int TestEventLambdaDeleteSelf();
extern int TestPlotV3();
extern int TestPlotAPI();
extern int TraceBufferSample();
extern int TestTut1(bool bBlock, bool bPrint);
extern int TestText();
extern int TestDataValid();
extern int TestSpectrumRange();

int __cdecl main()
{
#if 0
	printf("Please select test\n");
	printf(
		"m: mttest\n"
		"1: tut1\n"
		"a: testAPI\n"
		);

	int c = getchar();

	switch(c)
	{
	case 'm':
		MT_Test();
		break;
	case '1':
		TestTut1(true, true);
		break;
	case '2':
		TestTut1(false, false);
		break;
	case 'a':
		TestPlotAPI();
		break;
	}
#endif

	//TestText();
	//TestLog();
	//TestPlotAPI();
	//TestFileLog();
	//Sample();
	//TestChannelBuffer();
	//TestRingBuffer();
	//TestRingBufferWithTimeStamp();
	//TestDataFilter();
	//TestPlotV3();
	//Sample();
	//TestEvent();
	//TestChannelBuffer();
	//TestTaskQueue();

	//TestStrategy();
	//Sample();
	//TestPlotV3();

	//TestTaskQueue2();

	//TestRecursiveLock();

	//TestEventLambdaDeleteSelf();
	//TestPlotV2();
	//TestPlotV3();

	//TraceBufferSample();

	//TestTut1();
	MT_Test();
	//Sample();

	//TestDataValid();

	//TestSpectrumRange();

	printf("Test completed.\n");
	getchar();

	return 0;
}
