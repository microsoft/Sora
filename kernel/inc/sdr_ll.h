/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name: sdr_ll.h

Abstracts:
    sdr_ll.h defines interface for upper miniport and lower mac.

Revision History:
    Created by senxiang, 9/Dec/2009

Notes: 

--*/

#ifndef _SDR_LL_H
#define _SDR_LL_H

#if	defined NDIS620_MINIPORT
#define PNDIS_PACKET_OR_NBL PNET_BUFFER_LIST
#define PPNDIS_PACKETS_OR_NBL PNET_BUFFER_LIST
#else
#define PNDIS_PACKET_OR_NBL PNDIS_PACKET
#define PPNDIS_PACKETS_OR_NBL PPNDIS_PACKET
#endif

typedef struct _LINK_LAYER
{
    WLAN_SEQUENCE_CONTROL       CurSendSeqNo; //Send seq #, used to do Ethernet/WLAN packet transformation
} LL, *PLL;

#ifdef __cplusplus
extern "C"
{
#endif

VOID SdrLLInitialize(PLL lnk);
VOID SdrLLSendPacket(PSDR_CONTEXT SDRContext, PNDIS_PACKET_OR_NBL pNBSPacket);

VOID 
SdrLLSendPacketComplete(
    PSDR_CONTEXT SDRContext, 
    PDLCB pDLCB);

NTSTATUS
SdrLLRecvPacket(PSDR_CONTEXT SDRContext, PULCB pULCB);

VOID
SdrLLRecvPacketComplete(PSDR_CONTEXT SDRContext, PNDIS_PACKET_OR_NBL pNBSPacket);

#ifdef __cplusplus
}
#endif

#endif