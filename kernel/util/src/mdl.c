/*++
Copyright (c) Microsoft Corporation

Module Name: 
    mdl.c

Abstract: 
    mdl.c defines MDL manipulation routines.

History: 
    9/Dec/2009: Created by senxiang
--*/
#include "sora.h"
#include "sora_utility.h"
#include "CRC32.h"

#ifndef USER_MODE

#define MP_QUERY_MDL_SAFE(_Mdl, _VirtualAddress, _Length, _Priority)   \
{                                                                           \
    if (ARGUMENT_PRESENT(_VirtualAddress))                                  \
    {                                                                       \
        *(PVOID *)(_VirtualAddress) = MmGetSystemAddressForMdlSafe(_Mdl, _Priority); \
    }                                                                       \
    *(_Length) = MmGetMdlByteCount(_Mdl);                                   \
}


/*
CalcMDLChainCRC32 calculate CRC32 for content in given MDL
 
Parameters:  
    pMdl: Pointer to mdl descripor

Return Value: 
    CRC32 value for content bytes in given MDL.

IRQL: <= DPC_LEVEL
*/
ULONG CalcMDLChainCRC32(PMDL pMdl)
{
    
    ULONG CRC32 = 0xFFFFFFFF;
    ULONG Index = 0;
    PUCHAR pByte = NULL;
     
    for (; pMdl != NULL; pMdl = pMdl->Next)
    {
        pByte = (PUCHAR)pMdl->StartVa + pMdl->ByteOffset;
        for (Index = 0; Index < pMdl->ByteCount; Index++)
        {
            CRC32 = ((CRC32 ) >> 8 ) ^ gc_CRC32LUT[( pByte[Index] ) ^ (( CRC32 ) & 0x000000FF )];
        }
    }

    return ~CRC32;    
}

PMDL CloneMDL(IN PMDL pMdl)
{
    PMDL    pNewMdl = NULL;
    ULONG nLength;
    PVOID pSystemAddress;
  
    if (NULL == pMdl)
    {
        return NULL;
    }

    pSystemAddress = MmGetSystemAddressForMdlSafe(pMdl, NormalPagePriority);
    if (!pSystemAddress)
    {
        return NULL;
    }
    nLength = MmGetMdlByteCount(pMdl);
    pNewMdl = IoAllocateMdl(pSystemAddress, nLength, FALSE, FALSE, NULL);

    if (NULL != pNewMdl)
    {
        MmBuildMdlForNonPagedPool(pNewMdl);
    }

    return pNewMdl;
}

VOID FreeMDL(IN PMDL pMdl)
{ 
    if (NULL != pMdl)
    {
        IoFreeMdl(pMdl);
    }
}

/*
Routine Description:

    CloneMDLChain copies an MDL chain that identical to the given MDL chain 
 
Parameters:  
    pMdl: Pointer to mdl descripor


Return Value: 
    Pointer to head of copied MDL chain

*/
PMDL CloneMDLChain(IN PMDL pMdl)
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

IRQL:   <= DPC_LEVEL
*/
VOID FreeMDLChain(IN PMDL pMdlChain)
{
    PMDL pTempMdl = NULL;

    while(NULL != pMdlChain)
    {
        pTempMdl       = pMdlChain;
        pMdlChain      = pMdlChain->Next;
        pTempMdl->Next = NULL;

        IoFreeMdl(pTempMdl);
    }

}

/*
PopHeader pops header with specified size from first buffer.
 
Parameters:  
    pMdl:           Pointer to a MDL structure which describes a packet memory
    HeaderSize:     Size of the header in the packet

Return Value:   
    Old header pointer. NULL if fails.

IRQL:   <= DISPATCH_LEVEL
*/
PUCHAR PopHeader(IN PMDL pFirstBuffer, IN ULONG HeaderSize, IN ULONG ulNBDataOffset)
{
    HRESULT hRes        = S_OK;
    ULONG   Length      = 0;
    PUCHAR pOldHeaderVa = NULL;

    do 
    {
        if (NULL == pFirstBuffer)
        {
            break;
        }
        // Returns the base system-space virtual address
        MP_QUERY_MDL_SAFE(pFirstBuffer, &pOldHeaderVa, &Length, NormalPagePriority);
        if ((Length - ulNBDataOffset) < HeaderSize)
        {
            pOldHeaderVa = NULL;
            break;
        }

        // We just adjust the header but not free the first buffer
        pFirstBuffer->ByteOffset     += (HeaderSize + ulNBDataOffset);
        pFirstBuffer->ByteCount      -= (HeaderSize + ulNBDataOffset);
        pFirstBuffer->MappedSystemVa = ((PUCHAR)pFirstBuffer->StartVa + pFirstBuffer->ByteOffset);

        // We changed some fields of MDL, so update it
        MmBuildMdlForNonPagedPool(pFirstBuffer);

    } while (FALSE);

    return pOldHeaderVa + ulNBDataOffset;
}

/*
PushHeaderInPlace push a new MAC header in the existing memory 
that is big enough to accommodate the header.

Parameters:
    pFirstBuffer: MDL decribing the packet without old header.
    HeaderSize: New MAC header size.
    OlderHeader: Old header pointer return by PopHeader.
    
Return:
    New header pointer. If fails, return NULL.

Note:
    It can only be called when popped header size if bigger than pushed 
    header size.

IRQL: <= DPC_LEVEL.
*/
PUCHAR PushHeaderInPlace(PMDL pFirstBuffer, ULONG HeaderSize, PUCHAR OlderHeader)
{
    PUCHAR  pBuffer = NULL;
    ULONG   Length  = 0;
    PUCHAR  NewHeader = NULL;

    MP_QUERY_MDL_SAFE(pFirstBuffer, &pBuffer, &Length, NormalPagePriority);
    do
    {
        if (!pBuffer) break;

        if (pBuffer - OlderHeader > (LONG) HeaderSize) //big enough 
        {
            pFirstBuffer->ByteCount      = pFirstBuffer->ByteCount  + HeaderSize;
            pFirstBuffer->ByteOffset     = pFirstBuffer->ByteOffset - HeaderSize;
            pFirstBuffer->MappedSystemVa = (PUCHAR)(pFirstBuffer->MappedSystemVa) + pFirstBuffer->ByteOffset;
            MmBuildMdlForNonPagedPool(pFirstBuffer);
            NewHeader = (PUCHAR)pFirstBuffer->MappedSystemVa;
        }

    } while(FALSE);
    return NewHeader;
}

/*
PushHeader add a new header to a packet 

Parameters:  
    pFirstMdl:  Pointer to a MDL structure which describes the 
                packet first buffer without header. After new 
                header pushed, it decribes the packet with new header.
    HeaderSize: New header size.

Return Value:   
    New header pointer. NULL if fails.

IRQL:   PASSIVE_LEVEL

*/
PUCHAR
PushHeader(
    IN OUT PMDL* ppFirstMdl, 
    IN ULONG HeaderSize)
{
    PNDIS_BUFFER    pNewNdisBuffer  = NULL;
    PUCHAR          pHeader         = NULL;

    do 
    {
        // We use default tag 'maDN'
        pHeader = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, HeaderSize, 'maDN');
        if(pHeader == NULL)
        {
            break;
        }

        // OK we success allocate the memory for our new header
        // Build a MDL to describe our new header
        
        pNewNdisBuffer = IoAllocateMdl(pHeader, HeaderSize, FALSE, FALSE, NULL);

        if (NULL != pNewNdisBuffer)
        {
            MmBuildMdlForNonPagedPool(pNewNdisBuffer);
            pNewNdisBuffer->Next    = *ppFirstMdl; //chain it
            *ppFirstMdl             = pNewNdisBuffer;
            //pHeader = (PUCHAR)(pNewNdisBuffer->StartVa) + pNewNdisBuffer->ByteOffset;
        }
        else
        {
            NdisFreeMemory(pHeader, HeaderSize, 0);
            pHeader = NULL;
        }

    } while (FALSE);

    return pHeader;
}

#endif
