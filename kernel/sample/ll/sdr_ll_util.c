/*++
Copyright (c) Microsoft Corporation

Module Name: sdr_ll_util.c

Abstract: 
    sdr_ll_util.c defines functions for Eth2Wlan and Wlan2Eth 

History: 
    22/3/2010: Created by cr
--*/

#include "miniport.h"

PMDL
WlanToEth(IN PMDL pWlanFirstBuffer);

#pragma alloc_text(PAGE, WlanToEth)

/*
Routine Description:

EthToWlan    Change the header of a packet from ETHERNET to DOT11RFC1042ENCAP
 
Parameters: 
    pEthBufferList:     Pointer to a MDL descriptor for the packet
    ppWlanBufferList:   Pointer to a NDL descriptor for the packet after the change
    pWlanHeaderSize:    Receive the size of the new packet header
    ppWlanHeaderVa:     Receive the beginning address of the new packet
    SendSeqNo:          The sequence control value
    ulNBDataOffset:     The offset of NET_BUFFER

Return Value:   
    NDIS_STATUS_SUCCESS for success, otherwise fail

IRQL:   <= DISPATCH_LEVEL

Note:
    EthToWlan occupies additional resources. CleanWLanPacket should be 
    called to cleanup the transformed WLAN packet.
*/
NDIS_STATUS
EthToWlan(
    IN PMDL pEthBufferList, 
    OUT PMDL* ppWlanBufferList, 
    OUT PULONG pWlanHeaderSize, 
    OUT PUCHAR* ppWlanHeaderVa, 
    IN USHORT SendSeqNo, 
    IN ULONG ulNBDataOffset)
{
    NDIS_STATUS             NdisStatus      = NDIS_STATUS_SUCCESS;
    PNDIS_BUFFER            pWlanBufferList = NULL;
    PETHERNET_HEADER        pEthHeader      = NULL;
    PDOT11RFC1042ENCAP      pWlanHeader     = NULL;

    *ppWlanHeaderVa = NULL;
    *ppWlanBufferList = NULL;
    do 
    {
        if (NULL == pEthBufferList)
        {
            NdisStatus        = NDIS_STATUS_FAILURE;
            break;
        }
        
        pWlanBufferList = CloneMDLChain(pEthBufferList);
       
        // Pop ethernet header
        pEthHeader = (PETHERNET_HEADER)PopHeader((PMDL)pWlanBufferList, sizeof(ETHERNET_HEADER), ulNBDataOffset);
        if (NULL == pEthHeader)
        {
            NdisStatus        = NDIS_STATUS_FAILURE;
            break;
        }

        // Push wlan header
        //pWlanHeader is newly allocate buffer,we have to release it when send finish
        pWlanHeader = (PDOT11RFC1042ENCAP)PushHeader((PMDL *)&pWlanBufferList, sizeof(DOT11RFC1042ENCAP));
        if (NULL == pWlanHeader)
        {
            NdisStatus        = NDIS_STATUS_FAILURE;
            FreeMDLChain(pWlanBufferList);
            break;
        }

        *pWlanHeaderSize = sizeof(DOT11RFC1042ENCAP);
        *ppWlanHeaderVa  = (PUCHAR)pWlanHeader;
        
        // Version 0.1
        // Eth -> WLAN
        // IBSS mode only
        pWlanHeader->MacHeader.Address1                   = pEthHeader->destAddr;
        pWlanHeader->MacHeader.Address2                   = pEthHeader->srcAddr;
        pWlanHeader->MacHeader.Address3                   = g_BSSID;//pEthHeader->srcAddr; // BSSID

        *(UINT16*)(&pWlanHeader->MacHeader.FrameControl)    = 0;
        pWlanHeader->MacHeader.FrameControl.Version         = 0;
        pWlanHeader->MacHeader.FrameControl.Type            = FRAME_DATA;
        pWlanHeader->MacHeader.FrameControl.Subtype         = SUBT_DATA;
        pWlanHeader->MacHeader.FrameControl.WEP             = 0;
        pWlanHeader->MacHeader.FrameControl.Retry           = 1; //trick, assume it is re-transmission.

        //// we support IBSS only in this version
        pWlanHeader->MacHeader.FrameControl.ToDS            = 0;
        pWlanHeader->MacHeader.FrameControl.FromDS          = 0;

        //// sequence # is always 0
        pWlanHeader->MacHeader.SequenceControl.usValue    = SendSeqNo;

        //////////////////////////////////////////////////////////////////////////
        //SNAP
        pWlanHeader->SNAP.DSAP                            = 0xAA;
        pWlanHeader->SNAP.SSAP                            = 0xAA;
        pWlanHeader->SNAP.Control                         = 0x03;
        pWlanHeader->SNAP.Encapsulation[0]                = 0x00;
        pWlanHeader->SNAP.Encapsulation[1]                = 0x00;
        pWlanHeader->SNAP.Encapsulation[2]                = 0x00;
        pWlanHeader->Type                                 = pEthHeader->Type;

        ////// OK, we successfully convert a ethernet packet to a wlan packet
        *ppWlanBufferList = pWlanBufferList;

    } while (FALSE);

    return NdisStatus;
}


/*
Routine Description:

EthToWlan    Wlan to ethernet, Change original MDL directly
 
Parameters:  
    pWlanFirstBuffer:     Pointer to a MDL descriptor for the packet

Return Value:   
    MDL object describing the ethernet frame data.

IRQL:   PASSIVE_LEVEL

Note:
    No additional resources are occupied in this routine.
*/
PMDL
WlanToEth(IN PMDL pWlanFirstBuffer)
{
    PMDL                    pEthBufferList   = NULL;
    PETHERNET_HEADER        pEthHeader       = NULL;
    PDOT11RFC1042ENCAP      pWlanHeader      = NULL;
    ETHERNET_HEADER         EthHeader        = {0};

    PAGED_CODE();

    do 
    {
        if (NULL == pWlanFirstBuffer)
        {
            KdPrint(("[W2E2] NULL Header"));
            break;
        }

        pWlanHeader = (PDOT11RFC1042ENCAP)PopHeader(pWlanFirstBuffer, sizeof(DOT11RFC1042ENCAP), 0);
        if (pWlanHeader == NULL)
        {
            break;
        }

        // save destination, source, and type info before pushing a new ethernet header
        EthHeader.destAddr    = pWlanHeader->MacHeader.Address1;
        EthHeader.srcAddr     = pWlanHeader->MacHeader.Address2;
        EthHeader.Type        = pWlanHeader->Type;

        pEthHeader = (PETHERNET_HEADER)PushHeaderInPlace(
                            pWlanFirstBuffer, sizeof(ETHERNET_HEADER), (PUCHAR)pWlanHeader);
        ASSERT(sizeof(DOT11RFC1042ENCAP) > sizeof(ETHERNET_HEADER));
        ASSERT(pEthHeader); //ETHERNET_HEADER is smaller than DOT11RFC1042ENCAP

        pEthHeader->destAddr    = EthHeader.destAddr;
        pEthHeader->srcAddr     = EthHeader.srcAddr;
        pEthHeader->Type        = EthHeader.Type;

        pEthBufferList          = pWlanFirstBuffer;
    } while (FALSE);

    return pEthBufferList;
}

/*
CleanWLanPacket cleans up WLAN packet transformed by EthToWlan

Parameters:
    pPushedWlanHeader: the pushed WLAN header

    pWlanPacketBuffers: pointer to MDL of Wlan packet

Return:
    
*/
void 
CleanWLanPacket(
    IN PUCHAR pPushedWlanHeader, 
    IN ULONG HeaderSize, 
    IN PMDL pWlanPacketBuffers)
{
    if (pPushedWlanHeader)
    {
        NdisFreeMemory(pPushedWlanHeader, HeaderSize, 0);
    }
    if (pWlanPacketBuffers)
    {
        FreeMDLChain(pWlanPacketBuffers);
    }
}


