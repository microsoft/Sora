#pragma once

#define BBDEBUG

#include "sora.h"
#include "dot11_pkt.h"
#include "../timing.h"

#ifdef _DBG_PLOT_
#include "DebugPlotU.h"
#endif

#include <bb_debug.h>

#define BB11A_CHANNEL_CLEAN         ((HRESULT)0x00000200L) // channel free
#define BB11A_OK_POWER_DETECTED     ((HRESULT)0x00000201L) // channel busy
#define BB11A_OK_FRAME              ((HRESULT)0x00000202L) // a good frame is demodulated

#define BB11A_E_PD_LAG              ((HRESULT)0x80006000L)
#define BB11A_E_SYNC_FAIL           ((HRESULT)0x80006001L)
#define BB11A_E_INVALID_SIG         ((HRESULT)0x80006002L)
#define BB11A_E_FRAME_SIZE          ((HRESULT)0x80006003L)
#define BB11A_E_CRC32               ((HRESULT)0x80006004L)
#define BB11A_E_FORCE_STOP          ((HRESULT)0x80006005L)

// Private constants for baseband
#define CHANNEL_FACTOR_SIZE         (64 * 2)
#define DCHIS_LEN                   16
#define FREQ_LUT_SIZE               80
#define COMPLEX_GI                  32
// The number of states in Trellis
#define SPACE_SIZE                  64
// For Sig the maximum input is
#define INPUT_MAX_VITAS             96
#define BITS_PER_BYTE               8
#define COMPLEX_PER_OFDM_SYMBOL     160

typedef struct __BB11A_RX_FIFOS BB11A_RX_FIFOS, *PBB11A_RX_FIFOS;

#pragma warning (push)
#pragma warning (disable:4324)
typedef struct _BB11A_RX_CONTEXT
{
    /* carrier sense configuration */
    unsigned int            SampleRate;
    ULONG                   uiCSCorrThreshold; //Correlation value threshold for high energy
    unsigned int            uiCSMaxFetchRxBlock; // max fetched rx blocks in one round carrier sense
    unsigned int            uiCSMinFetchRxBlock; // min fetched rx blockes in one round carrier sense
    ULONG                   __uHighEnergyCounter; //High energy counter in power detection; 

    ULONG                   __uRxMode; //RX internal status;
    int                     __iSyncPeakPos; //RX correlation peak position;
    unsigned int            __uiSyncMax; //RX sync Max correlation value;
    unsigned short          __usLength;
    unsigned int            __uiSymbolCount;
    
    LARGE_INTEGER           ullSignalFindTimeStamp;

    char                    __bPilotCounter; //Pilot counter in RM_HEADER and RM_DATA mode

    volatile FLAG *         ri_pbWorkIndicator;
    
    char *                  ri_pbFrame;
    unsigned int            ri_uiFrameMaxSize;

    
    // OUT
    unsigned int            ri_uiFrameSize;
    unsigned int            ri_uiDataRate;
    unsigned int            ri_uiFrameType;

#ifdef USER_MODE
    TIMINGINFO              ri_tOfflineTimings[10];
#endif

    volatile FLAG           bRunViterbi;
    volatile FLAG           bViterbiDone;
    volatile BOOL           bCRCCorrect;
    volatile UCHAR          bRate; //Viterbi need it different algorithm.
    volatile unsigned int   uiVitSymbolCount; //viterbi thread input
    volatile unsigned int   uiVitFrameLen; //viterbi thread output length, including CRC32
    char                    *pbVitFrameOutput; //viterbi thread output

    PBB11A_RX_FIFOS         rxFifos;
    SORA_ETHREAD            ViterbiWorkerThread;

    // Private members for baseband
    COMPLEX32               channelFactor[CHANNEL_FACTOR_SIZE];
    __m128i                 histAutoCorrelation[4];
    int                     digitalAGC;
    __m128i                 dcEstimated;
    unsigned int            dcLearningCounter;
    unsigned int            dcHistoryCounter;
    COMPLEX16               dcHistory[DCHIS_LEN];
    COMPLEX32               dcSum;
    A16 short               freqFactorSin[2 * FREQ_LUT_SIZE];
    A16 short               freqFactorCos[2 * FREQ_LUT_SIZE];
    unsigned short          freqEstimated;
    __m128i                 corrRe[16];
    __m128i                 trellisAsig[SPACE_SIZE / 16 * INPUT_MAX_VITAS ];
    __m128i                 trellisData[5000 * BITS_PER_BYTE * SPACE_SIZE / 16];
    char                    VIT_OBUF[5000];
    A16 COMPLEX16           cFFTOut[64];
    A16 unsigned char       bDemapped[288];
    A16 unsigned char       bDeinterleaved[288];

	// added to support 802.11af
	ULONG           ChannelWidth;  // 5/10/20 MHz
	ULONG           ClockFactor;   // 4/2/1
	int             ChannelOffset; // 
} BB11A_RX_CONTEXT, *PBB11A_RX_CONTEXT;
#pragma warning (pop)

FINL void BB11A_VITERBIRUN_SET_EVENT(PBB11A_RX_CONTEXT pRxContextA)
{
    pRxContextA->bRunViterbi = TRUE;
}

FINL void BB11A_VITERBIRUN_CLEAR_EVENT(PBB11A_RX_CONTEXT pRxContextA)
{
    pRxContextA->bRunViterbi = FALSE;
}

FINL BOOL BB11A_VITERBIRUN_WAIT_EVENT(PBB11A_RX_CONTEXT pRxContextA)
{
    return pRxContextA->bRunViterbi;
}

FINL void BB11A_VITERBIDONE_SET_EVENT(PBB11A_RX_CONTEXT pRxContextA)
{
    pRxContextA->bViterbiDone = TRUE;
}

FINL void BB11A_VITERBIDONE_CLEAR_EVENT(PBB11A_RX_CONTEXT pRxContextA)
{
    pRxContextA->bViterbiDone = FALSE;
}

FINL BOOL BB11A_VITERBIDONE_WAIT_EVENT(PBB11A_RX_CONTEXT pRxContextA)
{
    return pRxContextA->bViterbiDone;
}

#pragma warning (push)
#pragma warning (disable:4324)
typedef struct _BB11A_TX_VECTOR
{
    unsigned int    SampleRate;
    unsigned int    ti_uiDataRate;
	unsigned int    ti_uiBufferLength;
    KSPIN_LOCK      txLock;

    // Private members for baseband
    ULONG           ulRandSeed;
    ULONG           ulRadom;        // used by scramble
    unsigned char   bConvEncoderReg;
    char            bEncoded[36];
    char            bInterleaved[36];
    A16 COMPLEX16   cMapped[48];
    A16 COMPLEX16   cPilotAdded[64];
    A16 COMPLEX16   cSymbol[COMPLEX_PER_OFDM_SYMBOL];

    // Temp buffer to upsample 40M->44M, should at least 160/40*44 large
    // Note: cSymbol44M buffer should has at least one more elment to prevent MOVDQU pollutes outside buffer
    A16 COMPLEX16   cSymbol44M[176 + 1];

    char            bFrameScrambled[5000];
    A16 COMPLEX16   cWindow[8];

	// added to support 802.11af
	ULONG           ChannelWidth;  // 5/10/20 MHz
	ULONG           ClockFactor;   // 4/2/1
	int             ChannelOffset; // 

} BB11A_TX_VECTOR, *PBB11A_TX_VECTOR;
#pragma warning (pop)

// Definition of 11a data rate code
#define DOT11A_RATE_6M  0xB // 1-011
#define DOT11A_RATE_9M  0XF // 1-111
#define DOT11A_RATE_12M 0xA // 1-010
#define DOT11A_RATE_18M 0xE // 1-110
#define DOT11A_RATE_24M 0x9 // 1-001
#define DOT11A_RATE_36M 0xD // 1-101
#define DOT11A_RATE_48M 0x8 // 1-000
#define DOT11A_RATE_54M 0xC // 1-100

//
// 802.11a BASEBAND interfaces
//
SORA_EXTERN_C
void 
BB11ARxContextInit(
    PBB11A_RX_CONTEXT pRxContextA, 
    unsigned int SampleRate,
    ULONG rxThreshold,
    ULONG rxMaxBlockCount,
    ULONG rxMinBlockCount, 
    volatile FLAG *WorkIndicator);

SORA_EXTERN_C
void
BB11ATxContextInit(
    PBB11A_TX_VECTOR info,
    unsigned int SampleRate
    );

SORA_EXTERN_C
void 
BB11APrepareRx(
    PBB11A_RX_CONTEXT pRxContextA,
    char* pcFrame,
    unsigned int unFrameMaxSize);

// 802.11a receiver start, call when driver starts, will start background thread
SORA_EXTERN_C BOOLEAN BB11ARxViterbiWorker(PVOID pContext);

// 802.11a receiver reset, call when rx hardware is reset, will reset hardware
// adaptation learning
SORA_EXTERN_C void BB11ARxReset(PBB11A_RX_CONTEXT pRxContextA);

// 802.11a receiver stop, call when driver stops, will stop background thread
SORA_EXTERN_C void BB11ARxContextCleanup(PBB11A_RX_CONTEXT pRxContextA);

// 802.11a carrier sense
SORA_EXTERN_C 
HRESULT 
BB11ARxCarrierSense(
    PBB11A_RX_CONTEXT pRxContextA, 
    PSORA_RADIO_RX_STREAM pRxStream);

// 802.11a frame decoder
SORA_EXTERN_C 
HRESULT 
BB11ARxFrameDemod(
    PBB11A_RX_CONTEXT pRxContextA, 
    PSORA_RADIO_RX_STREAM pRxStream);

// 802.11a tx baseband
SORA_EXTERN_C 
HRESULT 
BB11ATxFrameMod(
    PBB11A_TX_VECTOR info, 
    IN OUT PPACKET_BASE pPacket);

SORA_EXTERN_C
ULONG BB11AModulateACK(unsigned int SampleRate, const PMAC_ADDRESS RecvMacAddress, PVOID PhyACKBuffer);
