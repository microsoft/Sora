#include <Windows.h>
#include "ShortSleep.h"

void __stdcall ShortSleep(unsigned int microseconds)
{
	LARGE_INTEGER start, now, freq;
	::QueryPerformanceFrequency(&freq);
	::QueryPerformanceCounter(&start);
	while(true)
	{
		::QueryPerformanceCounter(&now);
		long long milli = (now.QuadPart - start.QuadPart) * 1000000 / freq.QuadPart;
		if (milli >= microseconds)
			break;
	}
}
