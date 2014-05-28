#include "bb/bba.h"
#include "bb/mod.h"
#include "soradsp.h"

//int g11a_iCSPositiveCounter;

// dot11a carrier sense algorithm:
//      For each descriptor, calculate its average energy level
//      and its 16 step self correlation. positive when corrleation
//      is greater then half of energy (16 step periodically signal
//      detected). This is a threshold free algorithm. 
HRESULT BB11ARxCarrierSense(PBB11A_RX_CONTEXT pRxContextA, PSORA_RADIO_RX_STREAM pRxStream)
{
    volatile FLAG *pbWorkIndicator = pRxContextA->ri_pbWorkIndicator;   
    FLAG bTouched;
    int iCorrelation;
    int iEnergy;
    unsigned int uiRxBlockCount = 0;
    HRESULT hr = BB11A_CHANNEL_CLEAN;

    pRxContextA->__uHighEnergyCounter = 0;

    do
    {
        SignalBlock block;
        HRESULT hr1;

        if (pRxContextA->SampleRate == 40)
            hr1 = FetchDMADataTouchDownSampled40(pRxStream, &bTouched, block);
        else if (pRxContextA->SampleRate == 44)
            hr1 = FetchDMADataTouchDownSampled44(pRxStream, &bTouched, block);
        else
        {
            KdPrint(("SampleRate is wrong\n"));
            return E_INVALIDARG;
        }

        if (hr1 != S_OK)
        {
            hr = E_FETCH_SIGNAL_HW_TIMEOUT;
            break;
        }

        // Check whether force stopped
        if (*pbWorkIndicator == 0)
        {
            hr = BB11A_E_FORCE_STOP;
            break;
        }

        // Kun: direct shift right by two hurts the performance
        EstimateDC(pRxContextA, block);
        
        RemoveDC (block, (vcs&)pRxContextA->dcEstimated);

        ShiftRight2(block); // avoid overflow
        iCorrelation = GetAutoCorrelation(block, (vcs*)pRxContextA->histAutoCorrelation);
        iEnergy = GetEnergy(block);

        if (iCorrelation > (iEnergy - (iEnergy >> 3)) && 
                (iCorrelation > (signed)(pRxContextA->uiCSCorrThreshold)))
        {
            pRxContextA->__uHighEnergyCounter++;
            if (pRxContextA->__uHighEnergyCounter >= 3)
            {
//                printf ( " Power detect! DC (%d, %d) AC %d E %d Th %d\n", 
//                    g_rgcDC[0].re, g_rgcDC[0].im, iCorrelation, iEnergy, pRxContextA->uiCSCorrThreshold );
                
                hr = BB11A_OK_POWER_DETECTED;
                break;
            }
        }
        else
            pRxContextA->__uHighEnergyCounter = 0;

        uiRxBlockCount += 1; //two RX blocks fetched in xmmFetchDMADataTouchDownSampled

        if (bTouched && uiRxBlockCount >= pRxContextA->uiCSMinFetchRxBlock &&
            pRxContextA->__uHighEnergyCounter == 0)
        {
            hr = BB11A_CHANNEL_CLEAN;
            break;
        }
        
        if (uiRxBlockCount >= pRxContextA->uiCSMaxFetchRxBlock)
        {
            hr = BB11A_E_PD_LAG;
            break;
        }

    } while(TRUE);

    return hr;
}
