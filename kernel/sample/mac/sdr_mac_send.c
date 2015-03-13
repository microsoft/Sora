/*++
Copyright (c) Microsoft Corporation

Module Name: sdr_mac_send.c

Abstract: This file implements a system thread routine for modulating mac frame.

History: 
          3/23/2009: Created by yichen
--*/

#include "miniport.h"
#include "sendrcv.h"

/*
SdrMacSendThread is system thread callback routine for modulating mac frame and 
transferring physical frame IQ into Radio Control Board.
 
Parameters:
    pVoid: Pointer to Sora thread object.
      
Return:  

Note: 

History:   4/1/2009 Created by yichen

IRQL: PASSIVE_LEVEL
*/

VOID
SdrMacSendThread(IN PVOID pVoid)
{
    NTSTATUS Status;
    HRESULT hRes;
    LARGE_INTEGER Delay;
    PDLCB pTCB                               = NULL;
    //thread context is specified by INIT_SORA_THREAD
    PSDR_CONTEXT        pSdrContext         = SORA_THREAD_CONTEXT_PTR(SDR_CONTEXT, pVoid); 
    PMAC                pMac                = (PMAC)pSdrContext->Mac;
    PPHY                pPhy                = (PPHY)pSdrContext->Phy;
    PSEND_QUEUE_MANAGER pSendQueueManager   = GET_SEND_QUEUE_MANAGER(pMac);
    PSORA_RADIO         pRadio              = NULL;
    KAFFINITY           PMask               = SORA_THREAD_AFFINITY(pVoid);
    
    KeSetSystemAffinityThread (PMask);
    Delay.QuadPart = -10 * 1000 * 10;
    do
    {        
        Status = KeWaitForSingleObject(
            &pSendQueueManager->hSendEvent, Executive, KernelMode, FALSE, &Delay); 
        if(Status == STATUS_TIMEOUT) //nothing to send
        {
            DbgPrint("[MAC_Send] no send event comes, pSendQueueManager->nSrcPacket=%d\n", 
                pSendQueueManager->nSrcPacket);
            if(IS_SORA_THREAD_NEED_TERMINATE(pVoid))
                break;
            else
                continue;
        }

        KeResetEvent(&pSendQueueManager->hSendEvent);
        
        pRadio = RadioInPHY(pPhy, RADIO_SEND);

        do
        {
            SafeDequeue(pSendQueueManager, SendSrcWaitList, pTCB, DLCB);
            if (!pTCB)
            {
                break;
            }
            
            if (!DLCB_CONTAIN_VALID_PACKET(pTCB)) // invalid packet, pass through the pipeline
            {
                SafeEnqueue(pSendQueueManager, SendSymWaitList, pTCB);
                InterlockedIncrement(&pSendQueueManager->nSymPacket);
                InterlockedDecrement(&pSendQueueManager->nSrcPacket);
                continue;
            }

            //Allocate TxSlot for SendSlot
            if (IS_PACKET_NO_RES(&pTCB->PacketBase))
            {
                hRes = SoraPacketGetTxResource(pSendQueueManager->pAdapter->TransferObj, &pTCB->PacketBase);
                //FAILED_BREAK(hRes);
                if (FAILED(hRes))
                {
                    InterlockedIncrement(&pSendQueueManager->nSymPacket);
                    InterlockedDecrement(&pSendQueueManager->nSrcPacket);
                    SafeEnqueue(pSendQueueManager, SendSymWaitList, pTCB); // let it go
                    DbgPrint("[Transfer][Error] insufficient TX resource \n");
                    break;
                }
            }
            else
            {
                KeBugCheck(BUGCODE_ID_DRIVER); //src packet should not own TX resource.
            }
            
            //Modulate SendPacket to TxSymbols
            hRes = (*pPhy->FnPHY_Mod)(pPhy, &(pTCB->PacketBase));

            //Download Symbols to HW
            hRes = SORA_HW_TX_TRANSFER(
                    pSendQueueManager->pAdapter->TransferObj,
                    &pTCB->PacketBase);
	            SoraPacketAssert(&pTCB->PacketBase, pSendQueueManager->pAdapter->TransferObj); //for verification. 

            if (FAILED(hRes))
            {
                SoraPacketPrint(&pTCB->PacketBase);
                SoraPacketFreeTxResource(pSendQueueManager->pAdapter->TransferObj, &pTCB->PacketBase);
                InterlockedIncrement(&pPhy->HwErrorNum);
            }

			{
				LARGE_INTEGER timeout;
				timeout.QuadPart = (LONGLONG)-10*1000*send_timeout;
				KeSetTimer(&pSendQueueManager->CleanQTimer,
					timeout,
					&pSendQueueManager->CleanQDPC);
			}
            
            SafeEnqueue(pSendQueueManager, SendSymWaitList, pTCB);
            InterlockedIncrement(&pSendQueueManager->nSymPacket);
            InterlockedDecrement(&pSendQueueManager->nSrcPacket);

            //both case: let the packet go. 
        } while (TRUE);
        
    }while(!IS_SORA_THREAD_NEED_TERMINATE(pVoid));

    DbgPrint("[Transfer] Mac send thread Terminated \n");
    SORA_THREAD_STOPPED(pVoid);
    PsTerminateSystemThread(STATUS_SUCCESS);
}





