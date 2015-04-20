#pragma once
#ifndef __cplusplus
#error "Must be compiled under C++"
#endif

#ifndef USER_MODE   // in kernel mode
extern "C" {
#include <Ndis.h>
}
#endif
#include <new>
#include "bb/bbb.h"
#include "sora_fifo.h"

typedef struct __BB11B_RX_FIFOS
{
    // Constants for buffer size
    static const int BB_SIZE = 16;
    static const int LB_SIZE = 1024 / sizeof(COMPLEX16);
    
    // BB11B Private  tempory RX state
    SoraLinearFifo<COMPLEX16, LB_SIZE>      BufDemod;    // Demodulate buffer
    SoraCounterFifo<unsigned char, BB_SIZE> BufFrame;  // Output frame bufffer
#ifdef USER_MODE
    // Buffer for Oscilloscope
    SoraLinearFifo<COMPLEX16, LB_SIZE>      BufDSSymbol;
    SoraCounterFifo<BYTE, 1024>             BufDecodeByte;
#endif

    __BB11B_RX_FIFOS () throw()
        : BufFrame("BufFrame.txt")
        , BufDemod("BufDemod.txt")
#ifdef USER_MODE
        , BufDSSymbol("BufDSSymbol.txt")
        , BufDecodeByte("BufDecodeByte.txt")
#endif
    {
    }

    // Required to make compiler happy
    void operator delete(void *) { }
} BB11B_RX_FIFOS, *PBB11B_RX_FIFOS;

#ifdef USER_MODE
template<int SIZE>
FINL void OutputBitInByteToBitStream(SoraCounterFifo<BYTE, SIZE> *pcb, BYTE b, unsigned int shiftcount)
{
    pcb->Write((BYTE)((b >> shiftcount) & 1));
}

template<int SIZE>
FINL void OutputByteToBitStream(SoraCounterFifo<BYTE, SIZE> *pcb, BYTE b)
{
    int i;
    for (i = 7; i >= 0; i--)
        pcb->Write((BYTE)((b >> i) & 1));
}
#else
#define OutputByteToBitStream(...)
#define OutputBitInByteToBitStream(...)
#endif

FINL HRESULT BB11BFifosInit(PBB11B_RX_CONTEXT pRxContext)
{
    BB11B_RX_FIFOS* rxFifos;
#ifdef USER_MODE
    rxFifos = (BB11B_RX_FIFOS *)malloc(sizeof(BB11B_RX_FIFOS));
    if (!rxFifos) return E_FAIL;
#else
    NDIS_STATUS rc = NdisAllocateMemoryWithTag((void **)&rxFifos, sizeof(BB11B_RX_FIFOS), BBB_TAG);
    if (rc != NDIS_STATUS_SUCCESS) return E_FAIL;
#endif
    new (rxFifos) BB11B_RX_FIFOS();  // Implicit call constructor

    pRxContext->b_rxFifos = rxFifos;
    return S_OK;
}

FINL void BB11BFifosCleanUp(PBB11B_RX_CONTEXT pRxContext)
{
    if (!pRxContext->b_rxFifos) return;
    pRxContext->b_rxFifos->~BB11B_RX_FIFOS();
#ifdef USER_MODE
    free(pRxContext->b_rxFifos);
#else
    NdisFreeMemory ((void *)pRxContext->b_rxFifos, sizeof(BB11B_RX_FIFOS), 0);
#endif
    pRxContext->b_rxFifos = NULL;
}
