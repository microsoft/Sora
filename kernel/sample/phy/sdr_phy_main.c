/*++
Copyright (c) Microsoft Corporation

Module Name: sdr_phy_main.c

Abstract: 
    This file implements interface functions for initialization 
    and cleanup of PHY of Software Defined Radio (SDR).
          
History: 
    3/23/2009: Created by yichen
    11/23/2009: Code refactory by senxiang

--*/

#include <limits.h>
#include "sdr_phy_precomp.h"

#include "trace.h"
#include "sdr_phy_main.tmh"
#include "thread_if.h"

/* 
SdrInitBBContextFor11B Initialize baseband.

Parameters:
    pAdapter: pointer to adapter which contains required parameter for PHY context.
    pBBContext: Pointer to DOT11B_BB_CONTEXT need to be initialized.
          
Return:  

History:   
    12/5/2009: Created by yichen
    23/11/2009: code refactory by senxiang
IRQL:
*/
VOID
SdrInitBBContextFor11B(
    IN PMP_ADAPTER          pAdapter,
    IN PDOT11B_BB_CONTEXT   pBBContext
    )
{
    do 
    {
        BB11BTxVectorInit(
            &pBBContext->TxVector, 
            (UCHAR)pAdapter->TxParameters[0],  // datarate
            (UCHAR)pAdapter->TxParameters[1],  // Mod select
            (UCHAR)pAdapter->TxParameters[2]); // Preamble type
     
        BB11BRxSpdContextInit(
            &(pBBContext->RxContext), 
            &(pBBContext->SpdContext), 
            (char *)(&(pBBContext->fCanWork)), 
            40000,      // nRxMaxBlockCount
            20000,      // nSPDMaxBlockCount, can be reset by dot11config
            15,         // nSPDMinBlockCount
            4000,       // nSPDThreashold, can be reset by dot11config
            UINT_MAX,
            4000,
            4
            );
        pBBContext->fCanWork      = TRUE;
    } while (FALSE);
}

VOID
SdrInitBBContextFor11A(
    IN PMP_ADAPTER          pAdapter,
    IN PDOT11A_BB_CONTEXT   pBBContext)
{
	pBBContext->fCanWork = TRUE;

    // Init rx
    BB11ARxContextInit(
		&(pBBContext->RxContext),
        pAdapter->TxParameters[3],      // SampleRate
		250000, 
		7000, 
		200, 
		(PFLAG)(&(pBBContext->fCanWork)));

    // Init tx
	BB11ATxContextInit(
        &(pBBContext->TxVector),
        pAdapter->TxParameters[3]);     // Sample rate
}

/* BB Context cleanup */
VOID SdrCleanupBBContext(PPHY pPhy)//PDOT11B_BB_CONTEXT pPhyContextExt)
{
    // only carrier sense context has 
    BB11BRxSpdContextCleanUp(&(pPhy->BBContextFor11B.RxContext));
    BB11ARxContextCleanup(&(pPhy->BBContextFor11A.RxContext));
}


void __RadioLinkList2Array(PPHY pPhy, PLIST_ENTRY pRadiosHead)
{
    ULONG uIndex = 0;
    PLIST_ENTRY pEntry = pRadiosHead->Flink;
    while(pEntry != pRadiosHead)
    {
        pPhy->pRadios[uIndex] = SORA_GET_RADIO_FROM_LIST_ENTRY(pEntry);
        uIndex++;
        pEntry = pEntry->Flink;
    }
}

void PhyMethodBind(PPHY pPhy)
{
    switch (pPhy->PHYMode)
    {
    // set the default value to DOT_11_A
	default:
		DbgPrint("Phy mode is not specified, set to DOT_11_A.");
		pPhy->PHYMode		= DOT_11_A;    
    case DOT_11_A:
        pPhy->FnPHY_Cs		= PhyDot11ACs;
		pPhy->FnPHY_Rx      = PhyDot11ARx;
		pPhy->FnPHY_Mod     = SdrPhyModulate11A;
		pPhy->FnPHY_ModAck  = SdrPhyModulateACK11A;
        break;
    case DOT_11_B:
        pPhy->FnPHY_Cs      = PhyDot11BCs;
		pPhy->FnPHY_Rx      = PhyDot11BRx;
		pPhy->FnPHY_Mod     = SdrPhyModulate11B;
		pPhy->FnPHY_ModAck  = SdrPhyModulateACK11B;
        break;		
    }
}

BOOLEAN viterbi_proc(PVOID pVoid) {

	BB11ARxViterbiWorker(pVoid);

	return *((PBB11A_RX_CONTEXT)pVoid)->ri_pbWorkIndicator;
}

/* 
SdrPhyInitialize gets radios from radio manager driver, 
then config radios required, and start all the radios.

Parameters:
    pPhy: Pointer to PHY structure for which is inited by the routine
    SDRContext: Pointer to SDR_CONTEXT object.
    ulReadioNum: How many radios required

Return:    S_OK for success, E_FAIL for fail 

History:   12/May/2009 Created by yichen

IRQL:      PASSIVE_LEVEL
*/
HRESULT
SdrPhyInitialize(
    IN PPHY                     pPhy,
    IN PSDR_CONTEXT             SDRContext,
    HANDLE						TransferObj,
    IN ULONG                    ulRadioNum
    )
{
    HRESULT      hRes           = S_OK;
    LIST_ENTRY*  pRadiosHead    = &pPhy->RadiosHead;
    
    UINT         uIndex         = 0;
    
    pPhy->HwErrorNum                = 0;
    pPhy->ullReceiveCRC32Error      = 0;
    pPhy->RxGainPreset0             = SORA_RADIO_DEFAULT_RX_GAIN;
    pPhy->RxGainPreset1             = SORA_RADIO_PRESET1_RX_GAIN;

    PhyMethodBind(pPhy);

    do
    {    
    	pPhy->TransferObj = TransferObj;

        hRes = 
            SoraAllocateRadioFromRCBDevice(
                pRadiosHead,
                ulRadioNum,
                NIC_DRIVER_NAME);
        if(FAILED(hRes))
        {
            DbgPrint("[Error]SoraAllocateRadioFromRCBDevice failed\n");
            break;
        }
        __RadioLinkList2Array(pPhy, pRadiosHead);
        SdrInitBBContextFor11B(SDRContext->Nic, &pPhy->BBContextFor11B);
        SdrInitBBContextFor11A(SDRContext->Nic, &pPhy->BBContextFor11A);

		if (!RadioInPHY(pPhy, RADIO_RECV)->__initialized) {
	        hRes = SoraRadioInitialize(
	                    RadioInPHY(pPhy, RADIO_RECV), //same as RADIO_SEND
	                    NULL, //reserved
	                    SAMPLE_BUFFER_SIZE,
	                    RX_BUFFER_SIZE);

	        FAILED_BREAK(hRes);
	        
			RadioInPHY(pPhy, RADIO_RECV)->__initialized = TRUE;
		}
			
        hRes = SoraRadioStart(
                    RadioInPHY(pPhy, RADIO_RECV), 
                    SORA_RADIO_DEFAULT_RX_GAIN,
                    SORA_RADIO_DEFAULT_TX_GAIN, 
                    NULL);
        FAILED_BREAK(hRes);

		

        //You can set TX/RX gain individually by SoraHwSetTXVGA1/SoraHwSetRXVGA1. 
        SoraHwSetTXVGA1(RadioInPHY(pPhy, RADIO_RECV), SORA_RADIO_DEFAULT_TX_GAIN);
        SoraHwSetRXVGA1(RadioInPHY(pPhy, RADIO_RECV), SORA_RADIO_DEFAULT_RX_GAIN);
        SoraHwSetRXPA(RadioInPHY(pPhy, RADIO_RECV), SORA_RADIO_DEFAULT_RX_PA);
        SoraHwSetCentralFreq(RadioInPHY(pPhy, RADIO_RECV), DEFAULT_CENTRAL_FREQ, 0);
        /*hRes = WARPRFSelectChannel(RadioInPHY(pPhy, RADIO_RECV), DEFAULT_CHANNEL);
        FAILED_BREAK(hRes);*/
        if (pPhy->PHYMode == DOT_11_A)
        {
            SoraHwSetSampleClock(RadioInPHY(pPhy, RADIO_RECV), 40 * 1000 * 1000);
        }
        if (pPhy->PHYMode == DOT_11_B)
        {
            SoraHwSetSampleClock(RadioInPHY(pPhy, RADIO_RECV), 44 * 1000 * 1000);
        }

        hRes = SdrPhyInitAckCache(
                    &pPhy->AckCacheMgr, 
                    ((PMP_ADAPTER)(SDRContext->Nic))->Fdo, // System-dependant IoWorkItem
                    pPhy,
                    RadioInPHY(pPhy, RADIO_SEND), 
                    MAX_PHY_ACK_SIZE, 
                    MAX_PHY_ACK_NUM);
        FAILED_BREAK(hRes);

		if (pPhy->PHYMode == DOT_11_A) {
			hRes = NDIS_STATUS_FAILURE;
			pPhy->Thread = SoraThreadAlloc();
			if (pPhy->Thread)
				if (SoraThreadStart(pPhy->Thread, viterbi_proc, &pPhy->BBContextFor11A.RxContext))
					hRes = NDIS_STATUS_SUCCESS;

			if (hRes != NDIS_STATUS_SUCCESS)
				if (pPhy->Thread) {
					SoraThreadFree(pPhy->Thread);
					pPhy->Thread = NULL;
				}				
		}

        pPhy->bRadiosInitOK             = TRUE;
    }while(FALSE);

    if(FAILED(hRes))
    {
        DbgPrint("[Error] PHY startup fail with 0x%08x\n", hRes);
        SdrPhyCleanUp(pPhy);
    }

    return hRes;
}

/* 
SdrPhyCleanUp release and terminate all radios we got 

Parameters:
    pPhy:       Pointer to PHY object.
   
Return:  

History:   12/May/2009 Created by yichen

IRQL:      PASSIVE_LEVEL
*/
VOID
SdrPhyCleanUp(IN PPHY pPhy)
{
    do
    {
        pPhy->BBContextFor11B.fCanWork = FALSE;
		pPhy->BBContextFor11A.fCanWork = FALSE;
        if (pPhy->PHYMode == DOT_11_A)
			if (pPhy->Thread) {
				SoraThreadFree(pPhy->Thread);
				pPhy->Thread = NULL;
			}
        SdrCleanupBBContext(pPhy);
        SdrPhyCleanupAckCache(&pPhy->AckCacheMgr);
        SoraReleaseRadios(&pPhy->RadiosHead);
    } while (FALSE);
    
    pPhy->bRadiosInitOK = FALSE;
}

VOID SdrPhyStopHardware(IN PPHY pPhy)
{
    SoraStopRadio2(RadioInPHY(pPhy, RADIO_RECV));
}


