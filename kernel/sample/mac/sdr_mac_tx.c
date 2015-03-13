/*++
Copyright (c) Microsoft Corporation

Module Name: sdr_mac_tx.c

Abstract: 
    This file implements state handler for 802.11 TX state.

History: 
    3/25/2009: Created by yichen
--*/

#include "miniport.h"

/*
SdrMacTx get a modulated packet from send queue, and 
transmits its IQ data through radio.

Parameters:
    StateMachine: pointer to state machine object.

Return:  

Note: 

History: 4/1/2009 Created by yichen

IRQL: PASSIVE_LEVEL
*/
VOID
SdrMacTx(IN PFSM_BASE StateMachine)
{
    HRESULT                 hRes                = S_OK;
    PSDR_CONTEXT            pSDRContext         = SoraFSMGetContext(StateMachine);
    PMP_ADAPTER             pAdapter            = (PMP_ADAPTER)pSDRContext->Nic;
    PMAC                    pMac                = (PMAC)pSDRContext->Mac;
    PPHY                    pPhy                = (PPHY)pSDRContext->Phy;
    PSEND_QUEUE_MANAGER     pSendQueueManager   = GET_SEND_QUEUE_MANAGER(pMac);
    PSORA_RADIO             pRadio              = RadioInPHY(pPhy, RADIO_SEND);
    PDLCB                    pTCB                = NULL;

    do 
    {
        SafeDequeue(pSendQueueManager, SendSymWaitList, pTCB, DLCB);
        if (!pTCB)
        {
            break;
        }

        if (pTCB->PacketBase.fStatus == PACKET_TF_FAIL ||
            !DLCB_CONTAIN_VALID_PACKET(pTCB)) //The packet can't be TX out, so complete it.
        {
            DbgPrint("[TX][Error] I can't tx it out because transfer fail, make it TXDone to complete\n");
            pTCB->bSendOK = FALSE;
            SoraPacketSetTXDone(&pTCB->PacketBase);
            InterlockedIncrement(&pSendQueueManager->nCompletePacket);
            InterlockedDecrement(&pSendQueueManager->nSymPacket);
            SafeEnqueue(pSendQueueManager, SendCompleteList, pTCB);
            SDR_MAC_INDICATE_PACKET_SENT_COMPLETE(pMac); //indicate to complete NDIS_PACKET
            break;
        }

        pMac->fTxNeedACK = (pTCB->PacketType == PACKET_NEED_ACK); //Notify other state handler, an ACK is expected.
        pTCB->RetryCount++;

        hRes = SORA_HW_BEGIN_TX(pRadio, &pTCB->PacketBase);
        if (FAILED(hRes))
        {
            DbgPrint("[TX][Error] TX hardware error , ret=%08x\n", hRes);
            SoraHwPrintDbgRegs(pAdapter->TransferObj);
            InterlockedIncrement(&pPhy->HwErrorNum);
        }
		else {
			LARGE_INTEGER timeout;
			timeout.QuadPart = (LONGLONG)-10*1000*send_timeout;
			KeSetTimer(&pSendQueueManager->CleanQTimer,
				timeout,
				&pSendQueueManager->CleanQDPC);
		}
         
        if ( !IS_MAC_EXPECT_ACK(pMac) || pTCB->RetryCount > TX_RETRY_TIMEOUT)
        {
            pTCB->bSendOK = (pTCB->RetryCount <= TX_RETRY_TIMEOUT);
            //if retry is not so big, assume it is sent out successufully.

            SoraPacketFreeTxResource(pSendQueueManager->pAdapter->TransferObj, &pTCB->PacketBase); 
            SoraPacketSetTXDone(&pTCB->PacketBase);
            InterlockedIncrement(&pSendQueueManager->nCompletePacket);
            InterlockedDecrement(&pSendQueueManager->nSymPacket);
            SafeEnqueue(pSendQueueManager, SendCompleteList, pTCB);
            //MarkModulatedSlotAsTxDone(pSendQueueManager); //dequeue the packet from send queue
            SDR_MAC_INDICATE_PACKET_SENT_COMPLETE(pMac); //indicate to complete NDIS_PACKET
            MAC_DISLIKE_ACK(pMac);  // we don't need ACK any more.
        }
        else
        {
            SafeJumpQueue(pSendQueueManager, SendSymWaitList, pTCB); 
            //wait for ack to retry or complete
        }
    } while (FALSE);

    SoraFSMGotoState(StateMachine, Dot11_MAC_CS);

    return;

}


