#include "DebugPlotU.h"

#define BUF_SIZE 512
int dataBuf[BUF_SIZE];

void TestLog()
{
	// prepare data
	for (int i = 0; i < BUF_SIZE; i++)
	{
		dataBuf[i] = 10*i - 20;
	}

	::DebugPlotInit();

	int count = 0;
	while(1)
	{
		//::WaitForViewer(INFINITE);
		::Sleep(0);
		::Log("Log0", "%d\n", count);

		count++;
	}
	
	::DebugPlotDeinit();
}
