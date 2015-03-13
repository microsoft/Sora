/*++
Copyright (c) Microsoft Corporation

Module Name: sdr_mac_recv.c

Abstract: 
    This file implements the callback routine for recv system thread, 
    which is for transmission finalization, and received packet indication 
    up to NDIS (then protocal stack).

History: 
          3/23/2009: Created by yichen

--*/

#include "miniport.h"
#include "sendrcv.h"
#include "sdr_mac_send_queue.h"
#include "sdr_mac_recv_queue.h"

/*
SdrMacHandleRecvPacket get a packet from a recvslot and change the header 
format of the received packet from DOT11RFC1042ENCAP to ETHERNET. And 
indicate the packet to up layer.

Parameters:
    pMac        : pointer to Mac object
    pAdapter    : pointer to adapter where packets are indicated up to

Return:  

Note: 

History:   4/1/2009 Created by yichen

*/
VOID SdrMacHandleRecvPacket(IN PMAC pMac, IN PMP_ADAPTER pAdapter)
{
    HRESULT             hRes                = E_FAIL;
    NDIS_STATUS         Status;
    PULCB               pULCB               = NULL;
    PMDL                pEthBufferList      = NULL;

    do
    {
        //get a pending received wlan frame
        SafeDequeue(GET_RECEIVE_QUEUE_MANAGER(pMac), RecvWaitList, pULCB, ULCB); 
        if(!pULCB)
        {
            break;
        }
        InterlockedDecrement(&GET_RECEIVE_QUEUE_MANAGER(pMac)->nPendingRXPackets);

        Status = SdrLLRecvPacket(&pAdapter->SdrContext, pULCB);
        if (Status == STATUS_UNSUCCESSFUL) // LL can't handle it, might be lack of resource, drop it.
        {
            InterlockedIncrement(&GET_RECEIVE_QUEUE_MANAGER(pMac)->nFreeRCB);
            SafeEnqueue(GET_RECEIVE_QUEUE_MANAGER(pMac), RecvFreeList, pULCB);
        }
    }while(TRUE);

}


/*
SdrMacSendRecvCompleteThread is thread entry for indicating received packets, 
and finalizing transmitted packets.

Parameters:
    pVoid: thread object pointer.
      
Return:  

Note: 

History:   
    4/1/2009 Created by yichen

IRQL: PASSIVE_LEVEL
*/
VOID
SdrMacSendRecvCompleteThread(IN PVOID pVoid)
{
    //context pointer is specified by INIT_SORA_THREAD
    NTSTATUS Status;
    LARGE_INTEGER       Delay;
    PSDR_CONTEXT        pSdrContext         = SORA_THREAD_CONTEXT_PTR(SDR_CONTEXT, pVoid); 
    PMAC                pMac                = (PMAC)pSdrContext->Mac;
    KAFFINITY           PMask               = SORA_THREAD_AFFINITY(pVoid);
    PRECV_QUEUE_MANAGER pRecvQueueManager   = GET_RECEIVE_QUEUE_MANAGER(pMac);
    
    Delay.QuadPart = -10 * 1000 * 10; //10ms
    KeSetSystemAffinityThread (PMask);
    
    do
    {
        Status = 
            KeWaitForSingleObject(&pRecvQueueManager->hRecvEvent, Executive, 
                KernelMode, FALSE, &Delay);
        if (Status == STATUS_TIMEOUT)
            continue;

        KeResetEvent(&pRecvQueueManager->hRecvEvent);
        
        SdrMacHandleRecvPacket(pMac, (PMP_ADAPTER)pSdrContext->Nic);

        ReleaseCompleteSendQueue( GET_SEND_QUEUE_MANAGER(pMac) );

    }while(!IS_SORA_THREAD_NEED_TERMINATE(pVoid));
 
    DbgPrint("[TxDone] Receive/TxDone Thread Terminated \n");
    SORA_THREAD_STOPPED(pVoid);
    PsTerminateSystemThread(STATUS_SUCCESS);
}