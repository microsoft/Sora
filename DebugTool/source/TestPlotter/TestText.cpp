#include "DebugPlotU.h"

int TestText()
{
	::DebugPlotInit();

	int count = 0;
	const int TEXT_LENGTH = 1024;
	char buffer[TEXT_LENGTH];

	for (int i = 0; i < TEXT_LENGTH; ++i)
		buffer[i] = 'a';

	while(1)
	{
		//::WaitForViewer(INFINITE);
		::Sleep(0);

		int selectIdx = rand() % (TEXT_LENGTH - 1) + 1;
		buffer[selectIdx] = 0;
		::PlotText("a text", buffer);
		buffer[selectIdx] = 'a';
	}
	
	::DebugPlotDeinit();
}
