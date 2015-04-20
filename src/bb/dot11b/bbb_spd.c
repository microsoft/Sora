/*++
Copyright (c) Microsoft Corporation

Module Name: bbb_spd.c

Abstract: 
    Carrier Sense (or Software Power Detection) part of Physical 
    Layer Convergence Procedure Sublayer (PLCP). It is a simple 
    high performance implementation of Carrier Sense.

History: 
          7/7/2009: Modified by senxiang.
--*/

#include <math.h>
#include <limits.h>
#include "bb/bbb.h"
#include "bbb_rx.h"
#include "soradsp.h"

void BB11BRxSpdContextInit(
    OUT PBB11B_RX_CONTEXT           pRxContext, 
    OUT PBB11B_SPD_CONTEXT          pSpdContext,
    IN PFLAG                        pfCanWork, 
    IN ULONG                        nRxMaxBlockCount,  
    IN ULONG                        nSPDMaxBlockCount, 
    IN ULONG                        nSPDMinBlockCount, 
    IN ULONG                        nSPDThreashold,
    IN ULONG                        nSPDThreasholdLH,
    IN ULONG                        nSPDThreasholdHL,
    IN ULONG                        nShiftRight)
{   
    int i;

    BB11BFifosInit(pRxContext);
    BB11BInitRxContext(pRxContext);
    pRxContext->b_maxDescCount          = nRxMaxBlockCount;
    pRxContext->b_workIndicator         = pfCanWork;
    pRxContext->b_resetFlag             = TRUE;
    pRxContext->b_shiftRight            = nShiftRight;

    pSpdContext->b_minDescCount         = nSPDMinBlockCount;
    pSpdContext->b_maxDescCount         = nSPDMaxBlockCount;
    pSpdContext->b_threshold            = nSPDThreashold;
    pSpdContext->b_thresholdLH          = nSPDThreasholdLH;
    pSpdContext->b_thresholdHL          = nSPDThreasholdHL;
    pSpdContext->b_gainLevel            = 1;                // any value of 0/1
    pSpdContext->b_gainLevelNext        = pSpdContext->b_gainLevel;

    pSpdContext->b_workIndicator        = pfCanWork;
    pSpdContext->b_resetFlag            = TRUE;
    pSpdContext->b_reestimateOffset     = TRUE;
    pSpdContext->HisPointer             = 0;
    pSpdContext->b_evalEnergy           = 0;

    for (i = 0; i < HISTORY_LEN; i++) 
         pSpdContext->History[i] = 0; 
}

void BB11BRxSpdContextCleanUp(PBB11B_RX_CONTEXT pRxContext)
{
    BB11BFifosCleanUp(pRxContext);
}

FINL BOOLEAN BB11BSpdCheckThreshold(unsigned int history[], unsigned int flag)
{
    int i, cnt = 0;
    for (i = 0; i < HISTORY_LEN; i++)
    {
        if (!(history[i] & flag)) cnt++;
    }
    if (cnt > HISTORY_LEN * .1) return false;
    else return TRUE;
}

// Store new "sum" into cyclic buffer "History[]", and get the energy sum to "HisSum"
SORA_EXTERN_C
FINL int BB11BSpdUpdateEngeryHistoryAndCheckThreshold(PBB11B_SPD_CONTEXT pSpdContext, unsigned int t0, unsigned int t1)
{
	// Alias
    vui& BlockEnergySum = (vui&)pSpdContext->BlockEnergySum;

    ULONG curEnergy = BlockEnergySum[0];
    if (curEnergy > t1)
        pSpdContext->History[pSpdContext->HisPointer] = EL_HIGH;
    else if (curEnergy > t0)
            pSpdContext->History[pSpdContext->HisPointer] = EL_LOW;
    else
        pSpdContext->History[pSpdContext->HisPointer] = EL_NOISE;

    pSpdContext->HisPointer++;
    pSpdContext->HisPointer &= HISTORY_MASK;

    if (BB11BSpdCheckThreshold(pSpdContext->History, EL_HIGH))
        return EL_HIGH;
    else if (BB11BSpdCheckThreshold(pSpdContext->History, EL_LOW))
        return EL_LOW;
    else
        return EL_NOISE;
}

// Calc DC for complex numbers in 4 SignalBlocks, divided by 4
SORA_EXTERN_C
HRESULT BB11BGetAccurateDCOffset(
    IN PSORA_RADIO_RX_STREAM        pRxStream,
    OUT vcs &                       dcOffset,
    OUT ULONG *                     pDescCount, 
    OUT FLAG *                      touched)
{
    int dcReSum = 0, dcImSum = 0;
    HRESULT hr = S_OK;
    ULONG count;

    SignalBlock block;
    for (count = 0; count < 4; count++)
    {
        hr = SoraRadioReadRxStream(pRxStream, touched, block);
        FAILED_BREAK(hr);

        dcOffset = SoraCalcDC(block);

        dcReSum += dcOffset[0].re;
        dcImSum += dcOffset[0].im;
    }
    
    *pDescCount += count;
    dcOffset[0].re = (short)(dcReSum >> 2);
    dcOffset[0].im = (short)(dcImSum >> 2);
    set_all(dcOffset, dcOffset[0]);
    return hr;
}

SORA_EXTERN_C
FINL void BB11BSpdResetHistory(OUT PBB11B_SPD_CONTEXT  pSpdContext)
{
    int i;
    for (i = 0; i < HISTORY_LEN; i++)
        pSpdContext->History[i] = 0;

    pSpdContext->HisPointer = 0;
}

/*++
BB11BSpd is a simple implementation of software power detection.

Return: E_FETCH_SIGNAL_HW_TIMEOUT, E_FETCH_SIGNAL_FORCE_STOPPED, BB11B_E_PD_LAG,
        BB11B_CHANNEL_CLEAN, BB11B_OK_POWER_DETECTED
--*/
HRESULT BB11BSpd(PBB11B_SPD_CONTEXT pSpdContext, PSORA_RADIO_RX_STREAM pRxStream)
{
	// Alias
    FLAG *b_workIndicator = pSpdContext->b_workIndicator;        // pointer to flag, 0 for force stop, 1 for work
    vcs& DcOffset            = (vcs&)pSpdContext->dcOffset;
    vui& BlockEnergySum      = (vui&)pSpdContext->BlockEnergySum;

    FLAG touched;
    ULONG PeekBlockCount    = 0;
    HRESULT hr              = S_OK;
    int energyLevel;

    if (pSpdContext->b_resetFlag)
    {
        //DbgPrint("[TEMP1] reset\n");
        BB11BSpdResetHistory(pSpdContext);
    }

    SignalBlock block;
    do
    {
        if (pSpdContext->b_reestimateOffset)
        {
            hr = BB11BGetAccurateDCOffset(
                    pRxStream, 
                    DcOffset,
                    &PeekBlockCount, 
                    &touched);
            FAILED_BREAK(hr);

            pSpdContext->b_reestimateOffset = 0;
        }

        while (TRUE)
        {
            hr = SoraRadioReadRxStream(pRxStream, &touched, block);
            FAILED_BREAK(hr);

            // Check whether force stopped
            if (*b_workIndicator == 0)
            {
                hr = BB11B_E_FORCE_STOP;
                break;
            }

            // Estimate and update DC offset
            SoraUpdateDC(block, DcOffset);
            RemoveDC(block, DcOffset);

            BlockEnergySum = SoraGetNorm(block);

            PeekBlockCount++;

            energyLevel = BB11BSpdUpdateEngeryHistoryAndCheckThreshold(
                pSpdContext, 
                pSpdContext->b_threshold,
                pSpdContext->b_gainLevel ? pSpdContext->b_thresholdHL : pSpdContext->b_thresholdLH
                );

            if (energyLevel != EL_NOISE)
            {
                if (pSpdContext->b_gainLevel == 0 && energyLevel == EL_HIGH)
                    pSpdContext->b_gainLevelNext = 1;
                else if (pSpdContext->b_gainLevel == 1 && energyLevel == EL_LOW)
                    pSpdContext->b_gainLevelNext = 0;
                
                pSpdContext->b_evalEnergy = BlockEnergySum[0];
                hr = BB11B_OK_POWER_DETECTED;
                break;
            }

            if (touched && PeekBlockCount > pSpdContext->b_minDescCount)
            {
                hr = BB11B_CHANNEL_CLEAN;
                break;
            }

            if (PeekBlockCount >= pSpdContext->b_maxDescCount)
            {
                hr = BB11B_E_PD_LAG;
                break;
            }
        }
    } while(FALSE);

    pSpdContext->b_dcOffset = DcOffset[0];
    return hr;
}
