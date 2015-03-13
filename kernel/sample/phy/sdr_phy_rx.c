/*++
Copyright (c) Microsoft Corporation

Module Name: sdr_phy_rx.c

Abstract: 
    This file implements RX routine for physical layer.
          
History: 
    23/Mar/2009: Created by yichen
--*/

#include "sdr_phy_precomp.h"

SORA_EXTERN_C
FINL void BB11B_RX_TO_RX(PBB11B_RX_CONTEXT pRxContext)
{
    pRxContext->b_resetFlag = FALSE;
}

SORA_EXTERN_C
FINL void BB11B_RX_TO_PD(PBB11B_SPD_CONTEXT pSpdContext)
{
    pSpdContext->b_resetFlag = TRUE;
}

// Private function: set gain to preset value indicated by gain level
SORA_EXTERN_C
FINL void __ApplyPresetGain(PPHY pPhy, UINT newGainLevel)
{
    ULONG newGain = newGainLevel? pPhy->RxGainPreset0 : pPhy->RxGainPreset1;

    DbgPrint("[MAC_CS] AGC set RxGain: 0x%X\n", newGain);
    pPhy->BBContextFor11B.SpdContext.b_gainLevel = newGainLevel;

    SoraHwSetRXVGA1(RadioInPHY(pPhy, RADIO_RECV), newGain);
}

// Note: energy is in Norm1 system
SORA_EXTERN_C
FINL int __GetDagcShiftRightBits(ULONG energy)
{
    const int DAGC_ENERGY_REFERENCE_FACTOR = 14;

    // Find the position of the left most 1 in energy (int32)
    int n = FindLeftMostSetPosition(energy);

    n -= DAGC_ENERGY_REFERENCE_FACTOR;
    // Never shift bits more than 15, since signal is represend in COMPLEX16
    if (n > 15) n = 15;
    if (n < -15) n = -15;
    return n;
}

/* 
PhyDot11BRx demodulates RX sample stream from radio and saves byte stream 
into ULCB (receive control block).

Parameters:
    pPhy:       Pointer to PHY
    uRadio:     Radio num from which we get sample stream
    pRCB:       free ULCB to accommodate byte stream

Return:  
    BB11B_OK_FRAME if a good frame is decoded. Otherwise error number.

History:   
    12/May/2009 Created by yichen
    23/Nov/2009 code refactory by senxiang

IRQL:
*/
HRESULT
PhyDot11BRx(
    IN      PPHY        pPhy, 
    IN      UINT        uRadio,
    IN      PULCB        pRCB
    )
{
    HRESULT     hRes        = E_FAIL;
    PSORA_RADIO pRadio      = pPhy->pRadios[uRadio];
    ULONG       fContinue;

    if(pPhy->BBContextFor11B.fCanWork)
    {
        pPhy->BBContextFor11B.RxContext.b_shiftRight = __GetDagcShiftRightBits(pPhy->BBContextFor11B.SpdContext.b_evalEnergy);

        BB11BPrepareRx(
            &pPhy->BBContextFor11B.RxContext,
            pRCB->pVirtualAddress, pRCB->BufSize);//PHY context is registered when SoraRadioInitialize called

        do 
        {
            fContinue = 0;
            hRes = BB11BRx(&(pPhy->BBContextFor11B.RxContext), SORA_GET_RX_STREAM(pRadio));
            //DbgPrint("[RX] BB11BRx:%08X\n", hRes);
            switch (hRes)
            {
            case BB11B_OK_FRAME:
                // Front-end AGC feedback logic
                // Note: only do AGC after receiving a good frame, not do after bad frame since it is possible
                //   not for local MAC address
                // TODO: do AGC only after receiving a good frame and for local MAC address
                if (pPhy->BBContextFor11B.SpdContext.b_gainLevelNext != pPhy->BBContextFor11B.SpdContext.b_gainLevel)
                {
                    unsigned int newGainLevel = pPhy->BBContextFor11B.SpdContext.b_gainLevelNext;
                    __ApplyPresetGain(pPhy, newGainLevel);
                }

                BB11B_RX_TO_PD(&(pPhy->BBContextFor11B.SpdContext));
                pRCB->PacketLength = (USHORT)(pPhy->BBContextFor11B.RxContext.BB11bCommon.b_length);
                pRCB->CRC32        = *((PULONG)(pRCB->pVirtualAddress + pRCB->PacketLength - 4));
                break;
            case BB11B_E_SFD:
            case BB11B_E_ENERGY:
            case BB11B_E_DATA:
                BB11B_RX_TO_PD(&(pPhy->BBContextFor11B.SpdContext));
                if(hRes == BB11B_E_DATA)
                   InterlockedIncrement64(&pPhy->ullReceiveCRC32Error);

                DbgPrint("[RX][Warning] BB11BRx fail:%08X\n", hRes);
                break;
            case E_FETCH_SIGNAL_HW_TIMEOUT:
                BB11B_RX_TO_PD(&(pPhy->BBContextFor11B.SpdContext));
                DbgPrint("[RX][Error] BB11BRx fail:%08X\n", hRes);
                break;
            default:
                BB11B_RX_TO_RX(&(pPhy->BBContextFor11B.RxContext));
                DbgPrint("[RX][Warning] BB11BRx Continuous Error : %08X\n",hRes);
                fContinue = 1;
            }
            if (!fContinue)
                break;
        } while(pPhy->BBContextFor11B.fCanWork);
    }

    return hRes;
}


HRESULT
PhyDot11ARx(
    IN      PPHY        pPhy, 
    IN      UINT        uRadio,
    IN      PULCB       pRCB
    )
{
    HRESULT     hRes        = E_FAIL;
    PSORA_RADIO pRadio      = pPhy->pRadios[uRadio];
	PSORA_RADIO_RX_STREAM pRxStream = SORA_GET_RX_STREAM(pRadio);

    if(pPhy->BBContextFor11A.fCanWork)
    {
		BB11APrepareRx(
			&(pPhy->BBContextFor11A.RxContext),
			(char*)(pRCB->pVirtualAddress),
			(unsigned int)(pRCB->BufSize));

		//Rx
		hRes = BB11ARxFrameDemod(&(pPhy->BBContextFor11A.RxContext), pRxStream);

        switch(hRes)
        {
        case BB11A_OK_FRAME:
			//BB11ARxReset(&(pPhy->BBContextFor11A.RxContext));
            pRCB->PacketLength = (USHORT)(pPhy->BBContextFor11A.RxContext.ri_uiFrameSize);
            pRCB->CRC32        = *((PULONG)(pRCB->pVirtualAddress + pRCB->PacketLength - 4));
            DbgPrint("[RX] BB11ARx succeed, packet length=%d\n", pRCB->PacketLength);
            break;
        
        default:
			//BB11ARxReset(&(pPhy->BBContextFor11A.RxContext));
            DbgPrint("[RX][Warning] BB11ARx fail:%08X\n", hRes);
			break;
        }
        
	}

    return hRes;
}

