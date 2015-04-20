/*++
Copyright (c) Microsoft Corporation

Module Name: sdr_ll_recv.c

Abstract: 
    sdr_ll_recv.c defines functions for link layer, which only transforms mac frame type 
    in up-link and cleans resources when receiving is completed.

History: 
    20/Nov/2009: Created by senxiang
--*/
#include "miniport.h"


/* PULCB can be a void attachment, error handling can be done in MAC */
NTSTATUS
SdrLLRecvPacket(PSDR_CONTEXT SDRContext, PULCB pULCB)
{
    PMDL                pEthBufferList      = NULL;
    PNDIS_PACKET        pRecvPacket         = NULL;
    NTSTATUS            Status;

    pEthBufferList = WlanToEth(pULCB->pMdl); //transform it to ethernet frame
    if(NULL != pEthBufferList)
    {
        NdisAllocatePacket(
            &Status, 
            &pRecvPacket, 
            ((PMP_ADAPTER)(SDRContext->Nic))->RecvPacketOrNetBufferListPoolHandle);
        if(Status == NDIS_STATUS_SUCCESS)
        {
            NdisReinitializePacket(pRecvPacket);
            *((PULCB *)pRecvPacket->MiniportReserved) = pULCB; //for SdrLLRecvPacketComplete cleanup
            NdisChainBufferAtBack(pRecvPacket, pEthBufferList);
            NicIndicateRecvPackets((PMP_ADAPTER)(SDRContext->Nic), &pRecvPacket);
        }
        else
        {
            Status = STATUS_UNSUCCESSFUL;
            DbgPrint("[Error] NdisAllocatePacket return failure\n");
        }
    }
    else //Error in WlanToEth,we have to return recv_pack
    {
        Status = STATUS_UNSUCCESSFUL;
        DbgPrint("[Error] WlanToEth fail\n");
    }

    return Status;
}

VOID
SdrLLRecvPacketComplete(PSDR_CONTEXT SDRContext, PNDIS_PACKET_OR_NBL pNBSPacket)
{
    //clean resources appended to pNBSPacket
    PULCB           pULCB;
    PMAC            pMac        = (PMAC)SDRContext->Mac;
    
    pULCB = *((PULCB*)pNBSPacket->MiniportReserved);
    SafeEnqueue(GET_RECEIVE_QUEUE_MANAGER(pMac), RecvFreeList, pULCB);
    InterlockedIncrement(&GET_RECEIVE_QUEUE_MANAGER(pMac)->nFreeRCB);
    NdisFreePacket(pNBSPacket);
}

