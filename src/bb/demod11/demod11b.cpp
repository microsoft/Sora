// Copyright (c) Microsoft Corporation
//
// Module Name: bbb_cck5.h
//
// Abstract: Offline demodulation test for 802.11b, input data is from dump file

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <windows.h>
#include "func.h"
#include "bb/bbb.h"
#include "bb_test.h"

#define _M(N) (1024 * 1024 * (N))
#define _K(N) (1024 * (N))

#define INPUTBUF_SIZE (_M(16))
#define OUTPUTBUF_SIZE (_K(4))

SORA_RADIO_RX_STREAM            RxStream;
BB11B_RX_CONTEXT                RxContext;
BB11B_SPD_CONTEXT               SpdContext;
A16 UCHAR                       InputBuf[INPUTBUF_SIZE];
UCHAR                           OutputBuf[OUTPUTBUF_SIZE];

// Process one dumped symbol file as 802.11b baseband input.
#pragma warning (push)
#pragma warning (disable : 28110) // Suppress driver PREFast warning since running in user mode.
int ProcessSymbolFile(PDOT11_DEMOD_ARGS pArgs)
{
    char bCanWork = 1;
    int rc;
    int goodFrame = 0;
    int badFrame = 0;
    int cntDesc;
    double timeRequired, timeUsed;
    PRX_BLOCK before, after;
    LARGE_INTEGER start, end, freq, totalStart, totalEnd;

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&totalStart);

    if (LoadDumpFile(pArgs->pcInFileName, InputBuf, INPUTBUF_SIZE) < 0)
    {
        puts("Failed to load input file."); 
        return -1;
    }

    FILE *pcOutFile = fopen_try(pArgs->pcOutFileName, "wb");

    SoraGenRadioRxStreamOffline(&RxStream, InputBuf, INPUTBUF_SIZE);

    // Advance to user specified descriptor counter
    SoraRadioSetRxStreamPos(&RxStream, SoraRadioGetRxStreamPos(&RxStream) +
        pArgs->nStartDesc * SORA_GET_RX_DESC_SIZE(RxStream.__pScanPt));

    BB11BRxSpdContextInit(
        &RxContext,
        &SpdContext,
        &bCanWork, 
        0xFFFFFFF,                          // nRxMaxBlockCount
        INPUTBUF_SIZE / sizeof(RX_BLOCK),   // nSPDMaxBlockCount
        15,                                 // nSPDMinBlockCount
        pArgs->nPowerThreshold,             // nSPDThreashold
        pArgs->nPowerThresholdLH,
        pArgs->nPowerThresholdHL,
        pArgs->nShiftRight
        );

    BB11BPrepareRx(&RxContext, OutputBuf, OUTPUTBUF_SIZE);

    while (1)
    {
        SpdContext.b_resetFlag = 1;

        while (1)
        {
            before = SoraRadioGetRxStreamPos(&RxStream);
            
            // Power dectection
            QueryPerformanceCounter(&start);
            rc = BB11BSpd(&SpdContext, &RxStream);
            QueryPerformanceCounter(&end);

            after = SoraRadioGetRxStreamPos(&RxStream);
            if (after == (PRX_BLOCK)RxStream.__pStartPt)
            {
                // Raido scan point is wrapped to the buffer start, which is expected in
                // driver mode. In demod11b test, we just has one buffer input data, so
                // wrap the "after" back.
                assert(rc == BB11B_E_PD_LAG || rc == E_FETCH_SIGNAL_HW_TIMEOUT);
                after = (PRX_BLOCK)RxStream.__pEndPt;
            }

            // Calc time
            timeRequired = (after - before) * 4 * 7 / 44.0 / 1000;
            timeUsed = ((double)(end.QuadPart - start.QuadPart) / freq.QuadPart) * 1000;
            cntDesc = ((PUCHAR)before - InputBuf) / 128;
            printf("[PD] from: %7d  req: %9fms  cost: %9fms  gl: %d\n", cntDesc, timeRequired, timeUsed, SpdContext.b_gainLevelNext);

            if (rc == BB11B_CHANNEL_CLEAN || rc == E_FETCH_SIGNAL_HW_TIMEOUT || rc == BB11B_E_PD_LAG)
                goto out;
            if (rc == BB11B_OK_POWER_DETECTED)
                break;

            SpdContext.b_resetFlag = 0;
        }

        RxContext.b_dcOffset = SpdContext.b_dcOffset;
        RxContext.b_resetFlag = 1;

        do
        {
            before = SoraRadioGetRxStreamPos(&RxStream);

            // Demodulate
            QueryPerformanceCounter(&start);
            rc = BB11BRx(&RxContext, &RxStream);
            QueryPerformanceCounter(&end);

            after = SoraRadioGetRxStreamPos(&RxStream);
            if (after == (PRX_BLOCK)RxStream.__pStartPt)
            {
                // Raido scan point is wrapped to the buffer start, which is expected in
                // driver mode. In demod11b test, we just has one buffer input data, so
                // wrap the "after" back.
                assert(rc == E_FETCH_SIGNAL_HW_TIMEOUT);
                after = (PRX_BLOCK)RxStream.__pEndPt;
            }

            // Calc time
            timeRequired = (after - before) * 4 * 7 / 44.0 / 1000;
            timeUsed = ((double)(end.QuadPart - start.QuadPart) / freq.QuadPart) * 1000;
            cntDesc = ((PUCHAR)before - InputBuf) / 128;
            printf("[RX] from: %7d  req: %9fms  cost: %9fms  ", cntDesc, timeRequired, timeUsed);

            if (rc == BB11B_OK_FRAME) 
            { 
                puts("good frame.");
                if (pcOutFile) fwrite(OutputBuf, RxContext.BB11bCommon.b_length - 4, 1, pcOutFile);
                goodFrame++;
            }
            else if (rc == BB11B_E_DATA)
            { 
                puts("bad frame."); 
                if (pcOutFile) fwrite(OutputBuf, RxContext.BB11bCommon.b_length - 4, 1, pcOutFile);
                badFrame++; 
            }
            else
            {
                puts("");

                if (rc == E_FETCH_SIGNAL_HW_TIMEOUT)
                {
                    printf("no more signals.\n");
                    goto out;
                }
                else if (rc == E_FETCH_SIGNAL_FORCE_STOPPED)
                {
                    printf("force stopped.\n");
                    goto out;
                }
                else if (rc == S_OK)
                {
                    printf("unknown exit reason.\n");
                    goto out;
                }
            }

            RxContext.b_resetFlag = 0;
        } while(rc != BB11B_E_ENERGY && rc != BB11B_E_DATA
            && rc != BB11B_OK_FRAME && rc != BB11B_E_SFD);
    }

out:
    QueryPerformanceCounter(&totalEnd);
    BB11BRxSpdContextCleanUp(&RxContext);
    fclose_try(pcOutFile);

    timeUsed = ((double)(totalEnd.QuadPart - totalStart.QuadPart) / freq.QuadPart) * 1000;
    printf ("%d good frame(s), %d bad frame(s).\n", goodFrame, badFrame);
    printf("total time used %9f ms\n", timeUsed);
    return 0;
}
#pragma warning (pop)

int Demod11B(PDOT11_DEMOD_ARGS pArgs)
{
    // Set thread affinity and priority
    DWORD_PTR dwProcessAffinityMask, dwSystemAffinityMask;
    if (!GetProcessAffinityMask(GetCurrentProcess(), &dwProcessAffinityMask, &dwSystemAffinityMask))
    {
        printf ("GetProcessAffinityMask() failed.\n");
        return 1;
    }
    if (dwProcessAffinityMask & 0x02)
    {
        if (!SetThreadAffinityMask(GetCurrentThread(), 0x02))
        {
            printf ("SetThreadAffinityMask() failed.\n");
            return 1;
        }
    }
    if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
    {
        printf ("SetPriorityClass() failed.\n");
        return 1;
    }
    
    // Start processing
    return ProcessSymbolFile(pArgs);
}
