/*++
Copyright (c) Microsoft Corporation

Module Name: sdr_phy_cs.c

Abstract: This file implements Physical layer Carrier sense function.
          
History: 
    3/23/2009: Created by yichen
--*/

#include "sdr_phy_precomp.h"

SORA_EXTERN_C
FINL void BB11B_PD_TO_PD(PBB11B_SPD_CONTEXT pSpdContext)
{
    pSpdContext->b_resetFlag = FALSE;
}

SORA_EXTERN_C
FINL void BB11B_PD_TO_RX(PBB11B_SPD_CONTEXT pSpdContext, PBB11B_RX_CONTEXT pRxContext)
{
    pRxContext->b_resetFlag = TRUE;
    pRxContext->b_dcOffset = pSpdContext->b_dcOffset;
}

/* 
__QuickerCarrierSense does quicker software carrier sense.

Parameters:
    pPhy:       pointer to PHY object

Return:  
    BB11B_OK_POWER_DETECTED if channel busy, BB11B_CHANNEL_CLEAN if channel free.

History:   
    moved to PHY from MAC, 2009/11/23

IRQL:      
*/
FINL 
HRESULT __QuickerCarrierSense(
    PBB11B_SPD_CONTEXT pSpdContext, 
    PSORA_RADIO_RX_STREAM pRxStream)
{
    HRESULT hRes;
    KIRQL OldIrql =  KeRaiseIrqlToDpcLevel();    
    hRes = BB11BSpd(pSpdContext, pRxStream);
    //if carrier sense never reaches the radio RX stream end, CS in DPC_LEVEL
    while(hRes == BB11B_E_PD_LAG)
    {
        BB11B_PD_TO_PD(pSpdContext);
        DbgPrint("[MAC_CS][Warning] LAG \n");
        hRes = BB11BSpd(pSpdContext, pRxStream);
    }
    
    KeLowerIrql(OldIrql);
	return hRes;
}

/* 
PhyDot11BCs does software carrier sense.

Parameters:
    pPhy:       pointer to PHY object
    uRadio:     index of radio 

Return:  
    BB11B_OK_POWER_DETECTED if channel busy, BB11B_CHANNEL_CLEAN if channel free.

History:   
    Cleaned by senxiang 2009/11/23

 IRQL:      PASSIVE_LEVEL
*/

HRESULT PhyDot11BCs(IN OUT PPHY pPhy, UINT uRadio)
{
    HRESULT hRes              = E_FAIL;

    PSORA_RADIO pRadio = pPhy->pRadios[uRadio];
    PSORA_RADIO_RX_STREAM pRxStream  = SORA_GET_RX_STREAM(pRadio);

    do 
    {
        hRes = __QuickerCarrierSense(&(pPhy->BBContextFor11B.SpdContext), pRxStream);
        
        if (hRes == BB11B_OK_POWER_DETECTED)
        {
            // Busy channel
            BB11B_PD_TO_RX(&(pPhy->BBContextFor11B.SpdContext), &(pPhy->BBContextFor11B.RxContext));
        }
        else
        { 
            BB11B_PD_TO_PD(&(pPhy->BBContextFor11B.SpdContext));
            if (hRes == E_FETCH_SIGNAL_HW_TIMEOUT)
                InterlockedIncrement(&pPhy->HwErrorNum);
        } 
      
    } while (FALSE);

    return hRes;
}


HRESULT PhyDot11ACs(IN OUT PPHY pPhy, UINT uRadio)
{
	
	HRESULT hRes = BB11A_CHANNEL_CLEAN;
    PSORA_RADIO pRadio = pPhy->pRadios[uRadio];
    KIRQL OldIrql =  KeRaiseIrqlToDpcLevel();

	hRes = BB11ARxCarrierSense(&(pPhy->BBContextFor11A.RxContext), SORA_GET_RX_STREAM(pRadio));
    // After the carrier sense function is called, the return value may indicates the reading pointer
    // in the RX stream does not reach the stream end, ie. it just process the dated samples in
    // the RX stream. Then the priority should remain in the DISPATCH_LEVEL, and continue carrier
    // sense until reaching RX stream end. Then CS and RX can work correctly on the most recent samples
    // in the RX stream.
    while(hRes == BB11A_E_PD_LAG)
    {
        DbgPrint("[MAC_CS][Warning] LAG \n");
        hRes = BB11ARxCarrierSense(&(pPhy->BBContextFor11A.RxContext), SORA_GET_RX_STREAM(pRadio));
    }

    KeLowerIrql(OldIrql);

	
	if(hRes == E_FETCH_SIGNAL_HW_TIMEOUT)
	{
        InterlockedIncrement(&pPhy->HwErrorNum);
	}

	return hRes;
}

