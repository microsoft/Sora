#ifndef SEN_HLP
#define SEN_HLP

#include "mp_5x.h"
typedef struct _DLCB
{
    LIST_ENTRY      TCBList;
    ULONG           Dot11HeaderSize;
    PUCHAR          pDot11HeaderVa;
    
    PNDIS_PACKET    pNdisPacket;
    PACKET_BASE     PacketBase;
}DLCB, *PDLCB;

NDIS_STATUS
WlanToEth(IN PMDL pWlanFirstBuffer, OUT PMDL* ppEthBufferList);
VOID
FreeMDLChain(IN PMDL pMdlChain);

VOID
FreeMDL(IN PMDL pMdl);

VOID NicSendPackets(PHWT_ADAPTER Adapter,PNDIS_PACKET pNdisPacket);

VOID
FreeMemory(IN PUCHAR pVa, IN ULONG Size);

#endif