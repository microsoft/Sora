/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name: 
  sendrcvutil.c

Abstract: 
    this file contains utility functions that used to process data packets send by up protocols

Revision History:
    Created by yichen, 7/4/2009

Notes: 

--*/


#include "miniport.h"
#include "sendrcvhelp.h"
#include "bb\bbb.h"
#include "dot11_pkt.h"
#include "CRC32.h"

#pragma NDIS_PAGEABLE_FUNCTION(WlanToEth)
#pragma NDIS_PAGEABLE_FUNCTION(FreeMDLChain)
#pragma NDIS_PAGEABLE_FUNCTION(FreeMDL)

MAC_ADDRESS g_BSSID;

DLCB g_SendSlot;

PMDL
CloneMDL(IN PMDL pMdl)
{
    PMDL    pNewMdl = NULL;
    
  
    if (NULL == pMdl)
    {
        return NULL;
    }

    pNewMdl = IoAllocateMdl((PVOID)((PUCHAR)pMdl->StartVa + pMdl->ByteOffset), pMdl->ByteCount, FALSE, FALSE, NULL);

    if (NULL != pNewMdl)
    {
        MmBuildMdlForNonPagedPool(pNewMdl);
    }

    return pNewMdl;
}


VOID
FreeMDL(IN PMDL pMdl)
{
 
    PAGED_CODE();

    if (NULL != pMdl)
    {
        IoFreeMdl(pMdl);
    }

}



VOID
FreeMemory(IN PUCHAR pVa, IN ULONG Size)
{
  
  
    if (NULL != pVa)
    {
        NdisFreeMemory(pVa, Size, 0);
    }

}


/*
Routine Description:

    CloneMDLChain copies an MDL chain that identical to the given MDL chain 
 
Parameters:  
pMdl: Pointer to mdl descripor


Return Value: 
Pointer to head of copied MDL chain

IRQL:   PASSIVE_LEVEL
*/
PMDL
CloneMDLChain(IN PMDL pMdl)
{
    PMDL    pNewMdlChain       = NULL;
    PMDL    pTempMdl           = NULL;
    PMDL    pNewMdlChainHeader = NULL;

    while (pMdl)
    {
        pTempMdl = CloneMDL(pMdl);

        if (pNewMdlChain == NULL)
        {
            pNewMdlChainHeader = pTempMdl;
            pNewMdlChain       = pTempMdl;
        }
        else
        {
            pNewMdlChain->Next = pTempMdl;
            pNewMdlChain       = pTempMdl;
        }

        pMdl = pMdl->Next;
    }

    if (NULL != pNewMdlChain)
    {
        pNewMdlChain->Next = NULL;
    }

  
    return pNewMdlChainHeader;
}


/*
Routine Description:

FreeMDLChain free the copied MDL chain 
 
Parameters:  
pMdl: Pointer to head of a MDL chain


Return Value: 

IRQL:   PASSIVE_LEVEL
*/
VOID
FreeMDLChain(IN PMDL pMdlChain)
{
    PMDL pTempMdl = NULL;

    PAGED_CODE();

    while(NULL != pMdlChain)
    {
        pTempMdl       = pMdlChain;
        pMdlChain      = pMdlChain->Next;
        pTempMdl->Next = NULL;

        IoFreeMdl(pTempMdl);
    }

}


/*
Routine Description:

PopHeader abstract mac header from a  packet,and add the beginning address of the packet with the size of header 
 
Parameters:  
pMdl:           Pointer to a MDL structure which descript the packet memory
HeaderSize:     Size of the header in the packet
ppHeaderVa:     Point to the beginning address of the header

Return Value:   
S_OK for success, otherwise fail

IRQL:   PASSIVE_LEVEL
*/
HRESULT
PopHeader(IN PMDL pNdisBufferList, IN ULONG HeaderSize, OUT PUCHAR* ppHeaderVa)
{
    HRESULT hRes    = S_OK;
    PUCHAR  pVa     = NULL;
    ULONG   Length  = 0;
    PNDIS_BUFFER    pNdisBuffer;

    PAGED_CODE();

    do 
    {
        pNdisBuffer = pNdisBufferList;

        if (NULL == pNdisBuffer)
        {
            hRes = E_FAIL;
            break;
        }

        // Returns the base system-space virtual address
        NdisQueryBufferSafe(pNdisBuffer, &pVa, &Length, NormalPagePriority);

        if (Length < HeaderSize)
        {
            hRes = E_FAIL;
            *ppHeaderVa = NULL;
            break;
        }
  
        *ppHeaderVa = pVa;

        // We just adjust the header but not free the first buffer
        pNdisBuffer->ByteOffset     += HeaderSize;
        pNdisBuffer->ByteCount      -= HeaderSize;
        (PUCHAR)(pNdisBuffer->MappedSystemVa) = ((PUCHAR)pNdisBuffer->StartVa + pNdisBuffer->ByteOffset);

        // We changed some fields of MDL, so update it
        MmBuildMdlForNonPagedPool(pNdisBuffer);

    } while (FALSE);

    return hRes;
}

/*
Routine Description:

PushHeader  Add a new header to a packet 
 
Parameters:  
pMdl:           Pointer to a MDL structure which descript the packet memory
HeaderSize:     Size of the header in the packet
ppHeaderVa:     Point to the beginning address of the header

Return Value:   
S_OK for success, otherwise fail

IRQL:   PASSIVE_LEVEL

*/
HRESULT
PushHeader(IN OUT PMDL* ppNdisBufferList, IN ULONG HeaderSize, OUT PUCHAR* ppHeaderVa)
{
    HRESULT         hRes            = S_OK;
    NDIS_STATUS     NdisStatus      = NDIS_STATUS_SUCCESS;
    PNDIS_BUFFER    pNewNdisBuffer  = NULL;
    PUCHAR          pHeader         = NULL;

    do 
    {
 
        // We use default tag 'maDN'
        NdisStatus = NdisAllocateMemoryWithTag(&pHeader, HeaderSize, 'maDN');

        if (NDIS_STATUS_SUCCESS != NdisStatus)
        {
            *ppHeaderVa = NULL;
            hRes = E_FAIL;

            break;
        }

        // OK we success allocate the memory for our new header
        // Build a MDL to describe our new header
        
        pNewNdisBuffer = IoAllocateMdl(pHeader, HeaderSize, FALSE, FALSE, NULL);

        if (NULL != pNewNdisBuffer)
        {
            MmBuildMdlForNonPagedPool(pNewNdisBuffer);

            pNewNdisBuffer->Next = *ppNdisBufferList;
            *ppNdisBufferList    = pNewNdisBuffer;

            *ppHeaderVa = (PUCHAR)(pNewNdisBuffer->StartVa) + pNewNdisBuffer->ByteOffset;
        }


    } while (FALSE);


    return hRes;
}




/*
Routine Description:

EthToWlan    Change the header of a packet from ETHERNET to DOT11RFC1042ENCAP
 
Parameters: 
pEthBufferList:     Pointer to a MDL descriptor for the packet
ppWlanBufferList:   Pointer to a NDL descriptor for the packet after the change
pWlanHeaderSize:    Receive the size of the new packet header
ppWlanHeaderVa:     Receive the beginning address of the new packet

Return Value:   
NDIS_STATUS_SUCCESS for success, otherwise fail

IRQL:   PASSIVE_LEVEL

*/

NDIS_STATUS
EthToWlan(IN PMDL pEthBufferList, OUT PMDL* ppWlanBufferList, OUT PULONG pWlanHeaderSize, OUT PUCHAR* ppWlanHeaderVa)
{
    NDIS_STATUS                NdisStatus       = NDIS_STATUS_SUCCESS;
    PNDIS_BUFFER               pWlanBufferList  = NULL;
    PETHERNET_HEADER           pEthHeader       = NULL;
    PDOT11RFC1042ENCAP         pWlanHeader      = NULL;
    
    do 
    {
        if (NULL == pEthBufferList)
        {
            NdisStatus        = NDIS_STATUS_FAILURE;
            *ppWlanBufferList = NULL;
            break;
        }
        
        pWlanBufferList = CloneMDLChain(pEthBufferList);
       
        // Pop ethernet header
        PopHeader((PMDL)pWlanBufferList, sizeof(ETHERNET_HEADER), (PUCHAR *)&pEthHeader);

        if (NULL == pEthHeader)
        {
            NdisStatus        = NDIS_STATUS_FAILURE;

            *ppWlanBufferList = NULL;
            break;
        }

        // Push wlan header
        //pWlanHeader is newly allocate buffer,we have to release it when send finish
        PushHeader((PMDL *)&pWlanBufferList, sizeof(DOT11RFC1042ENCAP), (PUCHAR *)&pWlanHeader);
        
        if (NULL == pWlanHeader)
        {
            NdisStatus        = NDIS_STATUS_FAILURE;
            *ppWlanBufferList = NULL;
            break;
        }

        *pWlanHeaderSize = sizeof(DOT11RFC1042ENCAP);
        *ppWlanHeaderVa  = (PUCHAR)pWlanHeader;
        
        // Version 0.1
        // Eth -> WLAN
        // IBSS mode only
        g_BSSID.Address[0] = 0xB6;
        g_BSSID.Address[1] = 0xDB;
        g_BSSID.Address[2] = 0x71;
        g_BSSID.Address[3] = 0x64;
        g_BSSID.Address[4] = 0x51;
        g_BSSID.Address[5] = 0x75;
        
        pWlanHeader->MacHeader.Address1                   = pEthHeader->destAddr;
        pWlanHeader->MacHeader.Address2                   = pEthHeader->srcAddr;
        pWlanHeader->MacHeader.Address3                   = g_BSSID;//pEthHeader->srcAddr; // BSSID

        *(UINT16*)(&pWlanHeader->MacHeader.FrameControl)  = 0;
        pWlanHeader->MacHeader.FrameControl.Version       = 0;
        pWlanHeader->MacHeader.FrameControl.Type          = FRAME_DATA;
        pWlanHeader->MacHeader.FrameControl.Subtype       = SUBT_DATA;
        pWlanHeader->MacHeader.FrameControl.WEP              = 0;

        //// we support IBSS only in this version
        pWlanHeader->MacHeader.FrameControl.ToDS          = 0;
        pWlanHeader->MacHeader.FrameControl.FromDS        = 0;

        //// sequence # is always 0
        pWlanHeader->MacHeader.SequenceControl.usValue    = 0;

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
ppEthBufferList:      Pointer to a MDL descriptor for the new packet

Return Value:   
STATUS_SUCCESS for success, otherwise fail

IRQL:   PASSIVE_LEVEL
*/

NDIS_STATUS
WlanToEth(IN PMDL pWlanFirstBuffer, OUT PMDL* ppEthBufferList)
{
    NDIS_STATUS                NdisStatus       = NDIS_STATUS_SUCCESS;
    PMDL                       pEthBufferList   = NULL;
    PETHERNET_HEADER           pEthHeader       = NULL;
    PDOT11RFC1042ENCAP         pWlanHeader      = NULL;

    static ETHERNET_HEADER     EthHeader        = {0};
    ULONG                      Length           = 0;

    PAGED_CODE();

    do 
    {
        if (NULL == pWlanFirstBuffer)
        {
            KdPrint(("[W2E2] NULL Header"));
            NdisStatus        = NDIS_STATUS_FAILURE;
            *ppEthBufferList  = NULL;

            DEBUGP(MP_TRACE, ("<--- WlanToEth::NULL POINTER\n"));
            break;
        }

        NdisQueryBufferSafe(pWlanFirstBuffer, &pWlanHeader, &Length,NormalPagePriority);

        KdPrint(("[W2E2] buffer len:%x, wlan header len:%d \n",Length,sizeof(DOT11RFC1042ENCAP)));
        if (Length < sizeof(DOT11RFC1042ENCAP))
        {
            *ppEthBufferList  = NULL;
            NdisStatus        = NDIS_STATUS_FAILURE;
            
            break;
        }

        // Keep a copy of ethernet header
        EthHeader.destAddr    = pWlanHeader->MacHeader.Address1;
        EthHeader.srcAddr     = pWlanHeader->MacHeader.Address2;
        EthHeader.Type        = pWlanHeader->Type;

        // Ajust MDL
        pWlanFirstBuffer->ByteCount      = pWlanFirstBuffer->ByteCount - sizeof(DOT11RFC1042ENCAP) + sizeof(ETHERNET_HEADER);
        pWlanFirstBuffer->ByteOffset     = pWlanFirstBuffer->ByteOffset + sizeof(DOT11RFC1042ENCAP) - sizeof(ETHERNET_HEADER);
        pWlanFirstBuffer->MappedSystemVa = (PUCHAR)(pWlanFirstBuffer->StartVa) + pWlanFirstBuffer->ByteOffset;

        // New ethernet header
        pEthHeader = (PETHERNET_HEADER)(pWlanFirstBuffer->MappedSystemVa);

        // Copy to new ethernet header
        pEthHeader->destAddr  = EthHeader.destAddr;
        pEthHeader->srcAddr   = EthHeader.srcAddr;
        pEthHeader->Type      = EthHeader.Type;

        // OK, we successfully convert wlan packet to ethernet packet
        // Ethernet header + Upper layer header
        *ppEthBufferList      = pWlanFirstBuffer;

    } while (FALSE);

    return NdisStatus;
}
