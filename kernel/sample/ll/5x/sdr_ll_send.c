/*++
Copyright (c) Microsoft Corporation

Module Name: sdr_ll_send.c

Abstract: 
    sdr_ll_send.c defines functions for link layer, which only transforms mac frame type 
    in down-link and cleans resources when transmission is completed.

History: 
    20/Nov/2009: Created by senxiang
--*/

#include "miniport.h"


/*
SdrLLSendPacket transforms NDIS packet and enqueue it in MAC send queue.

History:
    3/Nov/2009: Created by senxiang
*/
VOID SdrLLSendPacket(PSDR_CONTEXT SDRContext, PNDIS_PACKET_OR_NBL pNBSPacket)
{
    PMDL                pEthBuffer          = NULL;
    PMDL                pWlanBuffer         = NULL;
    PUCHAR              pAddress            = NULL;
    PDLCB               pDLCB                = NULL;
    ULONG               Dot11HeaderSize     = 0;
    PUCHAR              pDot11HeaderVa      = 0;
    UINT                uCurrentLength      = 0;
    UINT                uTotalLength        = 0;
    PMAC                pMac                = (PMAC)SDRContext->Mac;
    PLL                 lnk                 = (PLL)SDRContext->LinkLayer;
    PSEND_QUEUE_MANAGER pSendQueueManager   = GET_SEND_QUEUE_MANAGER(pMac);

    do
    {
        NdisGetFirstBufferFromPacketSafe(pNBSPacket,
            &pEthBuffer,
            (PVOID *)&pAddress,
            &uCurrentLength,
            &uTotalLength,
            NormalPagePriority);
        if(NULL == pAddress)
        {
            DbgPrint("[Error] NdisGetFirstBufferFromPacketSafe failed, PACKET DROPPED\n");
            NicDropPacket((PMP_ADAPTER)SDRContext->Nic, pNBSPacket);
            break;
        }


        EthToWlan(pEthBuffer, &pWlanBuffer, &Dot11HeaderSize, &pDot11HeaderVa, lnk->CurSendSeqNo.usValue, 0);
        lnk->CurSendSeqNo.SequenceNumber++;

        if(NULL == pWlanBuffer)
        {
            DbgPrint("[Error] EthToWlan failed, PACKET DROPPED\n");
            NicDropPacket((PMP_ADAPTER)SDRContext->Nic, pNBSPacket);
            break;
        }

        SafeDequeue(pSendQueueManager, SendFreeList, pDLCB, DLCB);
        if(!pDLCB)
        {
            DbgPrint("[LL][Warning] send queue is full, drop the packet \n");
            CleanWLanPacket(pDot11HeaderVa, Dot11HeaderSize, pWlanBuffer);
            NicDropPacket((PMP_ADAPTER)SDRContext->Nic, pNBSPacket);
            break;
        }

        pDLCB->PacketType = PACKET_NEED_ACK;
        if(ETH_IS_BROADCAST(pAddress) || ETH_IS_MULTICAST(pAddress))
        {
            pDLCB->PacketType = PACKET_NOT_NEED_ACK;
        }

        pDLCB->Dot11HeaderSize         = Dot11HeaderSize;
        pDLCB->pDot11HeaderVa          = pDot11HeaderVa;
        pDLCB->PacketBase.pMdl         = pWlanBuffer;
        pDLCB->pNdisPktOrNBL           = (PVOID)pNBSPacket;
        pDLCB->PacketBase.PacketSize   = uTotalLength - sizeof(ETHERNET_HEADER) + sizeof(DOT11RFC1042ENCAP);
        pDLCB->PacketBase.Reserved1    = CalcMDLChainCRC32(pWlanBuffer);
        pDLCB->RetryCount              = 0;
        pDLCB->bSendOK                 = FALSE;

        SafeEnqueue(pSendQueueManager, SendSrcWaitList, pDLCB);
        InterlockedIncrement(&pSendQueueManager->nSrcPacket);
        InterlockedDecrement(&pSendQueueManager->nFreeTCB);
        NIC_NOTIFY_SEND_THREAD(pMac);
    }while(FALSE);
}

/*
SdrLLSendPacketComplete cleans resouces occupied by SdrLLSendPacket. 

History:
    3/Nov/2009: Created by senxiang

Note:
    Different SdrLLSendPacket implementation needs different complete handler.
*/
VOID 
SdrLLSendPacketComplete(
    PSDR_CONTEXT SDRContext, 
    PDLCB pDLCB)
{
    PSEND_QUEUE_MANAGER pSendQueueManager   = GET_SEND_QUEUE_MANAGER((PMAC)SDRContext->Mac);
    
    CleanWLanPacket(
        pDLCB->pDot11HeaderVa, 
        pDLCB->Dot11HeaderSize, 
        pDLCB->PacketBase.pMdl); //cleanup resources in EthToWlan.
    pDLCB->pDot11HeaderVa = NULL;
    pDLCB->PacketBase.pMdl = NULL;
    
    if(pDLCB->pNdisPktOrNBL)
    {
        if(pDLCB->bSendOK)
        {
            NicCompletePacket((PMP_ADAPTER)SDRContext->Nic, (PNDIS_PACKET)(pDLCB->pNdisPktOrNBL));
        }
        else
        {   
            NicDropPacket((PMP_ADAPTER)SDRContext->Nic, (PNDIS_PACKET)(pDLCB->pNdisPktOrNBL));
        }
        pDLCB->pNdisPktOrNBL = NULL;
    }

    SoraPacketCleanup (&pDLCB->PacketBase);
    SafeEnqueue(pSendQueueManager, SendFreeList, pDLCB);
    InterlockedIncrement(&pSendQueueManager->nFreeTCB);
}
