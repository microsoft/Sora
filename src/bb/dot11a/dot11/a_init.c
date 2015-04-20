#include <new.h>
#include "alinew.h"
#include "bb/bba.h"
#include "../inc/bb/mod.h"

HRESULT BB11AFifosInit(PBB11A_RX_CONTEXT pRxContextA)
{
    // Allocate aligned memory for BB11A_RX_FIFOS
    PBB11A_RX_FIFOS p = aligned_malloc<BB11A_RX_FIFOS>();
    if (!p) return E_FAIL;

    // Placement new operator: call contructor on allocated memory
    new (p) BB11A_RX_FIFOS();
    
    pRxContextA->rxFifos = p;
    return S_OK;
}

void BB11AFifosCleanup(PBB11A_RX_CONTEXT pRxContextA)
{
    if (pRxContextA->rxFifos)
    {
        // Call destructor of class BB11A_RX_FIFOS
        pRxContextA->rxFifos->~BB11A_RX_FIFOS();

        // Free aligned memory
        aligned_free(pRxContextA->rxFifos);
        pRxContextA->rxFifos = NULL;
    }
}

void 
BB11ARxContextInit(
    PBB11A_RX_CONTEXT pRxContextA,
    unsigned int SampleRate,
    ULONG rxThreshold,
    ULONG rxMaxBlockCount,
    ULONG rxMinBlockCount, 
    volatile FLAG *WorkIndicator)
{
    pRxContextA->SampleRate             = SampleRate;
    pRxContextA->uiCSCorrThreshold      = rxThreshold;
	pRxContextA->ri_pbWorkIndicator     = WorkIndicator;
	pRxContextA->uiCSMaxFetchRxBlock    = rxMaxBlockCount;
	pRxContextA->uiCSMinFetchRxBlock    = rxMinBlockCount;
	
    pRxContextA->bRunViterbi            = FALSE;
    pRxContextA->bViterbiDone           = FALSE;
    pRxContextA->bCRCCorrect            = FALSE;
    pRxContextA->bRate                  = 0x0;
    BB11AFifosInit(pRxContextA);

	// 
	pRxContextA->ChannelWidth = 20;
	pRxContextA->ClockFactor  = 1;
	pRxContextA->ChannelOffset = 0;

    ResetDC(pRxContextA);
}

void
BB11ATxContextInit(
    PBB11A_TX_VECTOR info,
    unsigned int SampleRate)
{
    info->ulRandSeed = 1104;
    info->bConvEncoderReg = 0;
    info->ti_uiBufferLength = 0;
    info->SampleRate = SampleRate;

    memset(info->bEncoded,          0, sizeof(info->bEncoded));
    memset(info->bInterleaved,      0, sizeof(info->bInterleaved));
    memset(info->cMapped,           0, sizeof(info->cMapped));
    memset(info->cPilotAdded,       0, sizeof(info->cPilotAdded));
    memset(info->cSymbol,           0, sizeof(info->cSymbol));
    memset(info->bFrameScrambled,   0, sizeof(info->bFrameScrambled));
    memset(info->cWindow,           0, sizeof(info->cWindow));

	// 
	info->ChannelWidth = 20;
	info->ClockFactor  = 1;
	info->ChannelOffset = 0;


    KeInitializeSpinLock(&info->txLock);
}

void BB11ARxContextCleanup(PBB11A_RX_CONTEXT pRxContextA)
{
    BB11AFifosCleanup(pRxContextA);
}
