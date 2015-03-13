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
    PMDL                pEthMDL         = NULL;
    PNET_BUFFER_LIST    pNetBufferList  = NULL;
    NTSTATUS            Status;
    SIZE_T              DataLength      = 0;
    PMDL                pMDLTmp         = NULL;

    pEthMDL = WlanToEth(pULCB->pMdl); //transform it to ethernet frame
    if(NULL != pEthMDL)
    {
        //GetMDLsDataLength
        DataLength = 0;
        pMDLTmp = pEthMDL;
        while(pMDLTmp)
        {
            DataLength += MmGetMdlByteCount(pMDLTmp);
            pMDLTmp = pMDLTmp->Next;
        }

        pNetBufferList = NdisAllocateNetBufferAndNetBufferList(
            ((PMP_ADAPTER)(SDRContext->Nic))->RecvPacketOrNetBufferListPoolHandle,
            0,
            0,
            pEthMDL,
            0,
            DataLength);
        if(pNetBufferList)
        {
            pNetBufferList->MiniportReserved[0] = (PVOID)pULCB; //for SdrLLRecvPacketComplete cleanup
            NicIndicateRecvPackets((PMP_ADAPTER)(SDRContext->Nic), pNetBufferList);
            Status = STATUS_SUCCESS;
        }
        else
        {
            Status = STATUS_UNSUCCESSFUL;
            DbgPrint("[Error] NdisAllocateNetBufferList return failure\n");
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
    //clean resources appended to pNdisPacket
    PULCB   pULCB;
    PMAC    pMac = (PMAC)SDRContext->Mac;

    pULCB = (PULCB)pNBSPacket->MiniportReserved[0];
    SafeEnqueue(GET_RECEIVE_QUEUE_MANAGER(pMac), RecvFreeList, pULCB);
    InterlockedIncrement(&GET_RECEIVE_QUEUE_MANAGER(pMac)->nFreeRCB);

    //Free
//  NdisFreeNetBuffer(NET_BUFFER_LIST_FIRST_NB(pNBSPacket));//suspect
    NdisFreeNetBufferList(pNBSPacket);
}


