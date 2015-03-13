/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:
   sendrcv.h

Abstract:
    This module contains function prototypes for interaction with NDIS 
    in Sending and Receiving Packets

Revision History:
    Created by yichen, 23/Apr/2009
    Modified by senxiang, 8/Dec/2009
Notes:
--*/

#ifndef _SENDRCV_H
#define _SENDRCV_H

#define send_timeout	2000 // ms

#ifdef __cplusplus
extern "C"
{
#endif
VOID NicIndicateRecvPackets(IN PMP_ADAPTER pAdapter,IN PPNDIS_PACKETS_OR_NBL ppNBSPackets);

VOID NicDropPacket(IN PMP_ADAPTER Adapter, IN PNDIS_PACKET_OR_NBL pNBSPacket);

VOID NicCompletePacket(IN PMP_ADAPTER Adapter, IN PNDIS_PACKET_OR_NBL pNBSPacket);
#ifdef __cplusplus
}
#endif

#define NIC_NOTIFY_SEND_THREAD(PMAC)\
{\
    KeSetEvent(&(PMAC)->SendQueueManager.hSendEvent,IO_NO_INCREMENT,FALSE);\
}

#endif
