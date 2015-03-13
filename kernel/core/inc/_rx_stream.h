/*++
Copyright (c) Microsoft Corporation

Module Name: _rx_stream.h

Abstract: This header file defines the signal sample stream object.
        
--*/

#pragma once 

#ifndef INLINE_ASM
#define INLINE_ASM
#endif

#include <emmintrin.h>

#pragma warning (disable: 4201)

#define SORA_MAX_RX_SPIN_WAIT   1024

typedef struct __SORA_RADIO_RX_STREAM
{
    /* initial configuration */
    PUCHAR              __pStartPt;     
    ULONG               __nRxBufSize;
    PUCHAR              __pEndPt;
    PUCHAR              __pScanPt;
    ULONG               __VStreamMask;
} SORA_RADIO_RX_STREAM, *PSORA_RADIO_RX_STREAM, **PPSORA_RADIO_RX_STREAM;

__inline ULONG SoraGetStreamVStreamMask(PSORA_RADIO_RX_STREAM pRxStream)
{
    return pRxStream->__VStreamMask;
}
__inline void SoraSetStreamVStreamMask(PSORA_RADIO_RX_STREAM pRxStream, ULONG Mask)
{
    pRxStream->__VStreamMask = Mask;
}

/*
 Generated RX stream object from pure software UCHAR buffer
 Note: only used in offline test
 */
VOID
SORAAPI 
SoraGenRadioRxStreamOffline(
    OUT PSORA_RADIO_RX_STREAM   pRxStream, 
    IN PUCHAR                   pInput, 
    IN ULONG                    Size);

#ifdef USER_MODE
/*
 Function: SoraGenRadioRxStream
 Description: Generate a SORA_RADIO_RX_STREAM from a mapped RX buffer location
 */
HRESULT
SORAAPI 
SoraURadioAllocRxStream(
    OUT PSORA_RADIO_RX_STREAM   pRxStream, 
    IN ULONG                    RadioNo,
    IN PUCHAR                   pInput, 
    IN ULONG                    Size
    );

void
SORAAPI
SoraURadioReleaseRxStream(
    IN PSORA_RADIO_RX_STREAM    pRxStream,
    IN ULONG                    RadioNo
    );
#endif

// Get RxStream's buffer size in RX_BLOCK
__inline size_t SoraRadioGetRxStreamSize(PSORA_RADIO_RX_STREAM pRxStream)
{
    return pRxStream->__nRxBufSize / SORA_RX_BLOCK_SIZE;
}

__inline PRX_BLOCK 
SORAAPI 
SoraRadioGetRxStreamPos(PSORA_RADIO_RX_STREAM pRxStream)
{
    return (PRX_BLOCK)pRxStream->__pScanPt; 
}

__inline void 
SORAAPI SoraRadioSetRxStreamPos( PSORA_RADIO_RX_STREAM pRxStream, 
                                      PRX_BLOCK pt)
{
    pRxStream->__pScanPt = (PUCHAR)pt;
}

// Get the index of COMPLEX16 of current scan point in the Rx stream
__inline size_t
SORAAPI 
SoraRadioGetRxStreamIndex(PSORA_RADIO_RX_STREAM pRxStream)
{
    return (pRxStream->__pScanPt - pRxStream->__pStartPt)  / SORA_RX_BLOCK_SIZE * 28;
}

__inline
PRX_BLOCK 
SORAAPI
__SoraRadioIncRxStreamPointer(IN PSORA_RADIO_RX_STREAM pRxStream, PRX_BLOCK pointer)
{
    PRX_BLOCK ret = pointer + 1;

    ASSERT(ret <= (PRX_BLOCK)pRxStream->__pEndPt); //ScanPoint always at the board of signal block.
    if (ret == (PRX_BLOCK)pRxStream->__pEndPt)
    {
        ret = (PRX_BLOCK)pRxStream->__pStartPt;
    }
    return ret;
}

__inline
PRX_BLOCK 
SORAAPI
__SoraRadioAdvanceRxStreamPos(IN PSORA_RADIO_RX_STREAM pRxStream)
{
    PRX_BLOCK ret = __SoraRadioIncRxStreamPointer(pRxStream, SoraRadioGetRxStreamPos(pRxStream));
    SoraRadioSetRxStreamPos(pRxStream, ret);
    return ret;
}

__inline
PRX_BLOCK 
SORAAPI
__SoraRadioGetRxStreamPrevPos(IN PSORA_RADIO_RX_STREAM pRxStream)
{
    PRX_BLOCK ret = SoraRadioGetRxStreamPos(pRxStream);
    if (ret == (PRX_BLOCK)pRxStream->__pStartPt)
        ret = (PRX_BLOCK)pRxStream->__pEndPt;

    ret--;
    ASSERT(ret < (PRX_BLOCK)pRxStream->__pEndPt); //ScanPoint always at the board of signal block.
    ASSERT(ret >= (PRX_BLOCK)pRxStream->__pStartPt); //ScanPoint always at the board of signal block.
    return ret;
}

void 
__SnapShotSignalBlockWithDesc (IN PRX_BLOCK pScanPoint, IN PVOID Buffer);

void 
__DumpSignalBlockWithDesc(IN PRX_BLOCK pScanPoint, IN PVOID Buffer);


// Obsoleted 
HRESULT 
SORAAPI
SoraCheckSignalBlock(
    IN  PRX_BLOCK       pScanPoint, 
    IN  ULONG           VStreamMask,
    IN  USHORT          uRetries, 
    OUT FLAG           *fReachEnd );

void 
SORAAPI 
__FetchSignalBlock(IN PRX_BLOCK pScanPoint);

