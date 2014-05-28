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

ULONG GetNetBufferCount(PNDIS_PACKET_OR_NBL pNBSPacket)
{
    ULONG Count = 0;
    PNET_BUFFER pCurrentNB = NET_BUFFER_LIST_FIRST_NB(pNBSPacket);
    while (pCurrentNB)
    {
        Count++;
        pCurrentNB = NET_BUFFER_NEXT_NB(pCurrentNB);
    }
    return Count;
}

/*
SdrLLSendPacket transforms NDIS packet and enqueue it in MAC send queue.

History:
    3/Nov/2009: Created by senxiang
*/
VOID SdrLLSendPacket(PSDR_CONTEXT SDRContext, PNDIS_PACKET_OR_NBL pNBSPacket)
{
    PMDL                pEthBuffer          = NULL;
    PMDL                pWlanBuffer         = NULL;
    PDLCB               pDLCB               = NULL;
    ULONG               Dot11HeaderSize     = 0;
    PUCHAR              pDot11HeaderVa      = NULL;
    PMAC                pMac                = (PMAC)SDRContext->Mac;
    PLL                 lnk                 = (PLL)SDRContext->LinkLayer;
    PSEND_QUEUE_MANAGER pSendQueueManager   = GET_SEND_QUEUE_MANAGER(pMac);
    PNET_BUFFER         pCurrentNB          = NET_BUFFER_LIST_FIRST_NB(pNBSPacket);
    
    ULONG               NetBufferCount      = GetNetBufferCount(pNBSPacket);
    LIST_ENTRY          DLCBList;

    BOOL                fSufficientDLCBs    = GetNFreeDLCB(&DLCBList, pSendQueueManager, NetBufferCount);
    DbgPrint("[LL_Send] %d net buffer in the NBL\n", NetBufferCount);

    if(pCurrentNB == NULL || !fSufficientDLCBs)
    {
        DbgPrint("[Error] Get NetBuffer failed, PACKET DROPPED\n");
        NicDropPacket((PMP_ADAPTER)SDRContext->Nic, pNBSPacket);
        return;
    }

    NET_BUFFER_LIST_STATUS(pNBSPacket) = NDIS_STATUS_SUCCESS; //init its status to success

    while(pCurrentNB)
    {
        //Get MDL
        ASSERT(!IsListEmpty(&DLCBList));
        pDLCB = CONTAINING_RECORD(RemoveHeadList(&DLCBList), DLCB, List);
        pDLCB->pNdisPktOrNBL = (PVOID)pNBSPacket;
        pDLCB->bLastNB = !(NET_BUFFER_NEXT_NB(pCurrentNB));

        lnk->CurSendSeqNo.SequenceNumber++;

        pEthBuffer = NET_BUFFER_CURRENT_MDL(pCurrentNB);
        if(pEthBuffer == NULL)
        {    
            pDLCB->PacketBase.pMdl = NULL;
            DbgPrint("[Error] Get MDL failed, PACKET DROPPED\n");
        }
        else
        {
            EthToWlan(pEthBuffer,
                &pWlanBuffer,
                &Dot11HeaderSize,
                &pDot11HeaderVa,
                lnk->CurSendSeqNo.usValue,
                NET_BUFFER_CURRENT_MDL_OFFSET(pCurrentNB));
            pDLCB->PacketType = PACKET_NEED_ACK;
            if(pDot11HeaderVa &&
                (ETH_IS_BROADCAST(&((PDOT11RFC1042ENCAP)pDot11HeaderVa)->MacHeader.Address1) 
                || ETH_IS_MULTICAST(&((PDOT11RFC1042ENCAP)pDot11HeaderVa)->MacHeader.Address1)))
            {
                pDLCB->PacketType = PACKET_NOT_NEED_ACK;
            }

            //Config DLCB
            pDLCB->Dot11HeaderSize          = Dot11HeaderSize;
            pDLCB->pDot11HeaderVa           = pDot11HeaderVa;
            pDLCB->PacketBase.pMdl          = pWlanBuffer;
            
            pDLCB->PacketBase.PacketSize    = NET_BUFFER_DATA_LENGTH(pCurrentNB) + sizeof(DOT11RFC1042ENCAP) - sizeof(ETHERNET_HEADER);

			{ // we update the copied mdl length to packet length for baseband 
				ULONG len = pDLCB->PacketBase.PacketSize;
				PMDL mdl = pDLCB->PacketBase.pMdl;
				while(mdl) {
					if (mdl->ByteCount > len) {
						mdl->ByteCount = len;
						break;
					}
					else
					if (mdl->ByteCount < len) {
						len-= mdl->ByteCount;
						mdl = mdl->Next;
						continue;
					}
					else
						break;
				}
			}
			
            pDLCB->PacketBase.Reserved1     = CalcMDLChainCRC32(pWlanBuffer);
            pDLCB->RetryCount               = 0;
            pDLCB->bSendOK                  = FALSE;
        }

        SafeEnqueue(pSendQueueManager, SendSrcWaitList, pDLCB);
        InterlockedIncrement(&pSendQueueManager->nSrcPacket);

        //Notify send thread
        NIC_NOTIFY_SEND_THREAD(pMac);

        //Get next NetBuffer
        pCurrentNB = NET_BUFFER_NEXT_NB(pCurrentNB);
    }

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
    PSEND_QUEUE_MANAGER pSendQueueManager = GET_SEND_QUEUE_MANAGER((PMAC)SDRContext->Mac);
    
    if (DLCB_CONTAIN_VALID_PACKET(pDLCB))
    {
        CleanWLanPacket(
            pDLCB->pDot11HeaderVa, 
            pDLCB->Dot11HeaderSize, 
            pDLCB->PacketBase.pMdl); //cleanup resources in EthToWlan.
    }
    pDLCB->pDot11HeaderVa = NULL;
    pDLCB->PacketBase.pMdl = NULL;
	
    if(pDLCB->pNdisPktOrNBL)
    {
        //One NB fail the whole NBL fail
        if(pDLCB->bSendOK == FALSE)
        {
            NET_BUFFER_LIST_STATUS((PNET_BUFFER_LIST)(pDLCB->pNdisPktOrNBL)) = NDIS_STATUS_FAILURE;
        }

        //If it's the last NB
        if(pDLCB->bLastNB)
        {
            if(NET_BUFFER_LIST_STATUS((PNET_BUFFER_LIST)(pDLCB->pNdisPktOrNBL)) == NDIS_STATUS_SUCCESS)
            {
                NicCompletePacket((PMP_ADAPTER)SDRContext->Nic, (PNET_BUFFER_LIST)(pDLCB->pNdisPktOrNBL));
            }
            else
            {   
                NicDropPacket((PMP_ADAPTER)SDRContext->Nic, (PNET_BUFFER_LIST)(pDLCB->pNdisPktOrNBL));
            }
        }

        //clear
        pDLCB->pNdisPktOrNBL = NULL;
    }

    SoraPacketCleanup (&pDLCB->PacketBase);
    SafeEnqueue(pSendQueueManager, SendFreeList, pDLCB);
    InterlockedIncrement(&pSendQueueManager->nFreeTCB);
}


