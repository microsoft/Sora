/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name: 
   Sendrcv.C

Abstract: 
    NDIS 5.x Miniport driver for 802.11b Sora Sample. This is 
    miniport send/recv packet routine for NDIS 5.x.

Revision History:
    Created by yichen, 7/Apr/2009

Notes: 

--*/

#include "miniport.h"
#include "sendrcv.h"

/*++

Routine Description:

    NDIS Miniport entry point called whenever protocols are done with
    a packet that we had indicated up and they had queued up for returning
    later.

    When we get the return packet , call recv complete routine provided by 
    link layer to free occupied resources.

Arguments:

    MiniportAdapterContext  : pointer to MP_ADAPTER structure
    pNdisPacket             : packet being returned.

Return Value:

    None.

IRQL:   PASSIVE_LEVEL
--*/
VOID
MPReturnNetBufferLists(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNET_BUFFER_LIST NetBufferLists,
    IN ULONG ReturnFlags)
{
    PMP_ADAPTER pAdapter = (PMP_ADAPTER)MiniportAdapterContext;
    SdrLLRecvPacketComplete(&pAdapter->SdrContext, NetBufferLists);
    return;
}

/*
Routine Description:

MPSendPackets is called by NDIS miniport to transmit packets.
 
Arguments:
    MiniportAdapterContext      : pointer to Adapter object
    PacketArray                 : packets send from protocol driver
    NumberOfPackets             : number of packets send from protocol drivers

Return Value:
    None.

IRQL:   <= DISPATCH_LEVEL
*/
VOID 
MPSendNetBufferLists(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNET_BUFFER_LIST pNetBufferList,
    IN NDIS_PORT_NUMBER PortNumber,
    IN ULONG  SendFlags)
{
    PMP_ADAPTER Adapter = (PMP_ADAPTER)MiniportAdapterContext;

    PNET_BUFFER_LIST pCurrentNetBufferList = pNetBufferList;
    PNET_BUFFER_LIST pNextNetBufferList = NULL;
    while(pCurrentNetBufferList)
    {
        pNextNetBufferList = NET_BUFFER_LIST_NEXT_NBL(pCurrentNetBufferList);
        NET_BUFFER_LIST_NEXT_NBL(pCurrentNetBufferList) = NULL;
        
        if(!NIC_IS_DEVICE_CAN_WORK(Adapter))
        {
            DEBUGP(MP_WARNING, ("[MPSend] Adapter can't work, drop the packet\n"));
            NicDropPacket(Adapter, pCurrentNetBufferList);
        }
        else
        {
            SdrLLSendPacket(&Adapter->SdrContext, pCurrentNetBufferList); //hand it over to link layer.
        }

        pCurrentNetBufferList = pNextNetBufferList;
    }

    return;
}

VOID NicDropPacket(IN PMP_ADAPTER Adapter, IN PNDIS_PACKET_OR_NBL pNBSPacket)
{
    NET_BUFFER_LIST_STATUS(pNBSPacket) = NDIS_STATUS_FAILURE;
    NdisMSendNetBufferListsComplete(Adapter->AdapterHandle,
                    pNBSPacket,
                    0);
    InterlockedIncrement64(&Adapter->ullTransmitFail);
}

VOID NicCompletePacket(IN PMP_ADAPTER Adapter, IN PNDIS_PACKET_OR_NBL pNBSPacket)
{
    NET_BUFFER_LIST_STATUS(pNBSPacket) = NDIS_STATUS_SUCCESS;
    NdisMSendNetBufferListsComplete(Adapter->AdapterHandle,
                    pNBSPacket,
                    0);
    InterlockedIncrement64(&Adapter->ullGoodTransmits);
}

/*
NicIndicateRecvPackets indicate received packet (MAC frame) to NDIS 
(then protocol stack)

Parameters:
    pAdapter       : pointer to adapter object created by miniport driver.
    ppNdisPacket   : pointer to an NDIS packet to be indicated up.
      
Return:  

Note: 

History:    Created by yichen, 1/Apr/2009

IRQL: PASSIVE_LEVEL
*/
VOID NicIndicateRecvPackets(IN PMP_ADAPTER pAdapter, IN PPNDIS_PACKETS_OR_NBL ppNBSPackets)
{
    NdisMIndicateReceiveNetBufferLists(pAdapter->AdapterHandle, ppNBSPackets, pAdapter->PortNumber, 1, 0);
    InterlockedIncrement64(&pAdapter->ullGoodReceives);
}


