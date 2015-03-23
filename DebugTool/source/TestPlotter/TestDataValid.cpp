#include "DebugPlotU.h"

int TestDataValid()
{
	::DebugPlotInit();

	const int SPEC_SIZE = 64;
	int spectrum[SPEC_SIZE];

	int count = 0;

	while(1)
	{
		::Sleep(1);

		for (int i = 0; i < SPEC_SIZE; ++i)
		{
			spectrum[i] = i + rand() % 16;
		}

		::Log("log", "%d", count++);
		::PlotSpectrum("spec", spectrum, SPEC_SIZE);

	}

	::DebugPlotDeinit();

	return 0;
}
