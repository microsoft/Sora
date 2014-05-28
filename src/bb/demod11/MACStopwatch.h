#pragma once

#include <stdio.h>
#include <math.h>
#include "const.h"
#include "soradsp.h"
#include "bb/bba.h"

class MACStopwatch
{
    int         iNoiseCounter;
    int         iFrameCounter;
    int         iGoodFrameCounter;
    int         idxLastSample;
    TIMINGINFO  ti;
    double dReq, dCost, dRatio;

    double dNoiseAverage, dFrameAverage;
    double dNoiseMax, dFrameMax;
    double dNosieSD, dFrameSD;
    int dNoiseAboveCounter, dFrameAboveCounter;
    int dNoiseAbove0_8Counter, dFrameAbove0_8Counter;
public:
    MACStopwatch()
        : iNoiseCounter       (0)
        , iFrameCounter       (0)
        , iGoodFrameCounter   (0)
        , dNoiseAverage (0), dFrameAverage (0)
        , dNoiseMax (0), dFrameMax (0)
        , dNosieSD (0), dFrameSD (0)
        , dNoiseAboveCounter (0), dFrameAboveCounter (0)
        , dNoiseAbove0_8Counter (0), dFrameAbove0_8Counter (0)
        , idxLastSample(0)
    {
    }

    FINL void EnterCS(int idxSample)
    {
        idxLastSample = idxSample;
        TimerStart(&ti);
    }

    FINL void LeaveCS(int idxSample)
    {
        TimerStop(&ti);

        // Possible the scan pointer wraps to the buffer header
        size_t nSample = idxSample - idxLastSample;

        dReq = nSample  / 40.0;
        dCost = TimerRead(&ti) * 1000;
        dRatio = dCost / dReq;

        printf("[noise]  req: %9.3fus  cost: %9.3fus  "
            "noise@desc#%d-desc#%d\n",
            dReq,
            dCost,
            idxLastSample / 4 / SORA_RX_SIGNAL_UNIT_NUM_PER_DESC,
            idxSample / 4 / SORA_RX_SIGNAL_UNIT_NUM_PER_DESC);

        iNoiseCounter++;
        dNoiseAverage += dRatio;
        dNosieSD += dRatio * dRatio;
        dNoiseMax = (dNoiseMax >= dRatio)?dNoiseMax : dRatio;
        if (dRatio >= 0.8)
        {
            dNoiseAbove0_8Counter++;
            if (dRatio >= 1.0)
            {
                dNoiseAboveCounter++;
            }
        }
    }
    FINL void EnterRX(int idxSample)
    {
        idxLastSample = idxSample;
        TimerStart(&ti);
    }

    // Parameters:
    //  finished - demodulate and decode all the frame
    //  good     - demodulate and decode a CRC correct frame
    FINL void LeaveRX(int idxSample, bool finished, bool good)
    {
        TimerStop(&ti);

        // Possible the scan pointer wraps to the buffer header
        int nSample = idxSample - idxLastSample;

        dReq = nSample / 40;
        dCost = TimerRead(&ti) * 1000;
        dRatio = dCost / dReq;
        
        if (finished)
        {
            iFrameCounter++;
            printf("[frame]  req: %9.3fus  cost: %9.3fus  "
                "frame#%3d@desc#%d-desc#%d\n",
                dReq,
                dCost,
                iFrameCounter,
                idxLastSample / 4 / SORA_RX_SIGNAL_UNIT_NUM_PER_DESC,
                idxSample / 4 / SORA_RX_SIGNAL_UNIT_NUM_PER_DESC);

            dFrameAverage += dRatio;
            dFrameSD += dRatio * dRatio;
            dFrameMax = (dFrameMax >= dRatio)?dFrameMax : dRatio;
            if (dRatio >= 0.8)
            {
                dFrameAbove0_8Counter++;
                if (dRatio >= 1.0)
                {
                    dFrameAboveCounter++;
                }
            }
        }
                      
        if (!good)
        {
            printf("         err\n");
        }
        else
        {
            iGoodFrameCounter++;
        }
    }

    void OutputStats()
    {
        printf("[summary] positive: %5d  good: %5d        bad: %5d\n", 
            iFrameCounter, iGoodFrameCounter, iFrameCounter - iGoodFrameCounter);

        dNoiseAverage /= iNoiseCounter;
        dFrameAverage /= iFrameCounter;
        dNosieSD = sqrt(dNosieSD / iNoiseCounter - dNoiseAverage * dNoiseAverage);
        dFrameSD = sqrt(dFrameSD / iFrameCounter - dFrameAverage * dFrameAverage);

        printf("\nDetail statistics for noise:\n"
            "total: \t\t%d\n"
            "average: \t%.2f\n"
            "max: \t\t%.2f\n"
            "std: \t\t%.2f\n"
            "#(cost >= req):     %d (%3.2f%%)\n"
            "#(cost >= req*0.8): %d (%3.2f%%)\n",
            iNoiseCounter,
            dNoiseAverage,
            dNoiseMax,
            dNosieSD,
            dNoiseAboveCounter, dNoiseAboveCounter / (double)iNoiseCounter * 100,
            dNoiseAbove0_8Counter, dNoiseAbove0_8Counter / (double)iNoiseCounter * 100);

        printf("\nDetail statistics for frame:\n"
            "total: \t\t%d\n"
            "average: \t%.2f\n"
            "max: \t\t%.2f\n"
            "std: \t\t%.2f\n"
            "#(cost >= req):     %d (%3.2f%%)\n"
            "#(cost >= req*0.8): %d (%3.2f%%)\n",
            iFrameCounter,
            dFrameAverage,
            dFrameMax,
            dFrameSD,
            dFrameAboveCounter, dFrameAboveCounter / (double)iFrameCounter * 100,
            dFrameAbove0_8Counter, dFrameAbove0_8Counter / (double)iFrameCounter * 100);
    }
};
