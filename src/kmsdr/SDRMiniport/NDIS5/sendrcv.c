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
MPSendPackets(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets)
{    
    UINT                PacketCount;
    PNDIS_PACKET        pNdisPacket = NULL;
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    PMP_ADAPTER         Adapter = (PMP_ADAPTER) MiniportAdapterContext;
    
    for(PacketCount = 0; PacketCount < NumberOfPackets; PacketCount++)
    {
        pNdisPacket = PacketArray[PacketCount];
        
        if(!NIC_IS_DEVICE_CAN_WORK(Adapter))
        {
            DEBUGP(MP_WARNING, ("[MPSend] Adapter can't work, drop the packet\n"));
            NicDropPacket(Adapter,pNdisPacket);
        }
        else
        {
            SdrLLSendPacket(&Adapter->SdrContext, pNdisPacket); //hand it over to link layer.
        }
    }
    
    return;
}

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
MPReturnPacket(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN PNDIS_PACKET pNdisPacket)
{
    PMP_ADAPTER     pAdapter    = (PMP_ADAPTER)MiniportAdapterContext;
    
    SdrLLRecvPacketComplete(&pAdapter->SdrContext, pNdisPacket);
    return;
}

VOID NicDropPacket(IN PMP_ADAPTER Adapter, IN PNDIS_PACKET_OR_NBL pNBSPacket)
{
   NDIS_SET_PACKET_STATUS(pNBSPacket,NDIS_STATUS_FAILURE);
   NdisMSendComplete(Adapter->AdapterHandle,
                     pNBSPacket,
                     NDIS_STATUS_FAILURE);
   InterlockedIncrement64(&Adapter->ullTransmitFail);
}

VOID NicCompletePacket(IN PMP_ADAPTER Adapter, IN PNDIS_PACKET_OR_NBL pNBSPacket)
{
   NDIS_SET_PACKET_STATUS(pNBSPacket,NDIS_STATUS_SUCCESS);
   NdisMSendComplete(Adapter->AdapterHandle,
                     pNBSPacket,
                     NDIS_STATUS_SUCCESS);
   InterlockedIncrement64(&Adapter->ullGoodTransmits);
}

/*
NicIndicateRecvPackets indicate received packet (MAC frame) to NDIS 
(then protocol stack)

Parameters:
    pAdapter       : pointer to adapter object created by miniport driver.
    ppNBSPackets   : pointer to an NDIS packet to be indicated up.
      
Return:  

Note: 

History:    Created by yichen, 1/Apr/2009

IRQL: PASSIVE_LEVEL
*/
VOID NicIndicateRecvPackets(IN PMP_ADAPTER pAdapter,IN PPNDIS_PACKETS_OR_NBL ppNBSPackets)
{
    NDIS_SET_PACKET_HEADER_SIZE(*ppNBSPackets, ETH_HEADER_SIZE);
    NDIS_SET_PACKET_STATUS(*ppNBSPackets, NDIS_STATUS_SUCCESS);
    NdisMIndicateReceivePacket(pAdapter->AdapterHandle, ppNBSPackets, 1); //only 1 packet indicated
    InterlockedIncrement64(&pAdapter->ullGoodReceives);
}
