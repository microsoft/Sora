#pragma once
#include <stdio.h>
#include <Windows.h>

class Monitor
{
    // Time related variables
    static const unsigned   interval = 1; // in seconds
    LONGLONG            intervalFactor;
    LARGE_INTEGER	    freq;
	LARGE_INTEGER	    prevTime, curTime;

    // Statistic variables
    ULONG				goodFrame;
    ULONG				badFrame;
    ULONGLONG           throughput;

public:
    Monitor()
        : goodFrame(0)
        , badFrame(0)
        , throughput(0)
    {
        // Init time
	    QueryPerformanceFrequency(&freq);
	    QueryPerformanceCounter(&prevTime);
        intervalFactor = interval * freq.QuadPart;
    }

    ULONG IncGoodCounter()
    {
        ++goodFrame;
        //printf("Good frame %d.\n", goodFrame);
        return goodFrame;
    }

    ULONG IncBadCounter()
    {
        ++badFrame;
		//printf("Bad frame %d.\n", badFrame);
        return badFrame;
    }

    ULONGLONG IncThroughput(ULONGLONG incBytes)
    {
        //printf("incBytes = %d\n", incBytes);
        return throughput += incBytes;
    }

    void Query(bool send)
    {
        // Print out statistics on fixed interval
		QueryPerformanceCounter(&curTime);
        if (curTime.QuadPart - prevTime.QuadPart > intervalFactor)
		{
            if (send)
            {
			    printf("[%8.2lfs-%8.2lfs] send %3d frames  throughput %.3f Mbps\n"
                    , (double)prevTime.QuadPart / freq.QuadPart
                    , (double)curTime.QuadPart / freq.QuadPart
                    , goodFrame
                    , (double)throughput * 8 / 1024 / 1024 / interval); // Throughput is normalized to 1 sec
            }
            else
            {
			    printf("[%8.2lfs-%8.2lfs] recv %3d good frames %3d bad frames  throughput %.3f Mbps\n"
                    , (double)prevTime.QuadPart / freq.QuadPart
                    , (double)curTime.QuadPart / freq.QuadPart
                    , goodFrame, badFrame
                    , (double)throughput * 8 / 1024 / 1024 / interval); // Throughput is normalized to 1 sec
            }

			goodFrame = badFrame = 0;
            throughput = 0;

            // Note: cannot use curTime directly, because of accumulated error
            prevTime.QuadPart += intervalFactor;
		}
    }
};
