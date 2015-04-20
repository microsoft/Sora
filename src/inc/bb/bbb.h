#pragma once

#include "sora.h"

#pragma warning(disable:4127) // avoid conditional expression is constant error with W4
#include "dot11_plcp.h"

// definition of BB error code
#define BB11B_E_ENERGY              ((HRESULT)0x80050100L) // no energy, no good frame, return immediately
#define BB11B_E_DOWNSAMPLE          ((HRESULT)0x80050101L) // energy high, but currently down sample failed
#define BB11B_E_BARKER              ((HRESULT)0x80050102L) // down sample ok, but currently barker not found
#define BB11B_E_SFD                 ((HRESULT)0x80050103L) // barker ok, but sfd not found

#define BB11B_E_DATA                ((HRESULT)0x80050105L) // SFD found, header correct, data get, 
                                                           // but crc32 incorrect
#define BB11B_E_PD_LAG              ((HRESULT)0x80050106L) // carrier sense is slower than hardware rx
#define BB11B_E_FORCE_STOP          ((HRESULT)0x80050107L) // force stopped

#define BB11B_E_PLCP_HEADER_CRC     ((HRESULT)0x80050210L) // PLCP header crc error
#define BB11B_E_PLCP_HEADER_SIG     ((HRESULT)0x80050211L) // PLCP header signal error
#define BB11B_E_PLCP_HEADER_LEN     ((HRESULT)0x80050211L) // PLCP header length error

#define BB11B_OK_FRAME              ((HRESULT)0x0000007FL) // a good frame demodulated
#define BB11B_NO_ENOUGH_SIGNAL      ((HRESULT)0x00000100L) // no enough signal in signal flow buffer
#define BB11B_BARKER_ALIGNED        ((HRESULT)0x00000101L)
#define BB11B_PREAMBLE_DECODE_END   ((HRESULT)0x00000102L)
#define BB11B_1M_DATARATE_DECODED   ((HRESULT)0x00000103L)
#define BB11B_2M_DATARATE_DECODED   ((HRESULT)0x00000104L)
#define BB11B_5M_DATARATE_DECODED   ((HRESULT)0x00000105L)
#define BB11B_11M_DATARATE_DECODED  ((HRESULT)0x00000106L)
#define BB11B_NO_ENOUGH_SCRAMBLED   ((HRESULT)0x00000107L) // no enough scrambled data for descramble.
#define BB11B_DESCRAMBLE_FINISH     ((HRESULT)0x00000108L)
#define BB11B_OK_PLCP_HEADER        ((HRESULT)0x00000109L)
#define BB11B_OK_POWER_DETECTED     ((HRESULT)0x00000101L)
#define BB11B_CHANNEL_CLEAN         ((HRESULT)0x00000102L)

// Memory tag used when allocating
#define BBB_TAG                     ((ULONG)'BBB')

// Spin-wait time
#define RX_DESC_SPIN_WAIT_MAX       40000

// For modulate buffer size limitation, bigger packet is not supported
#define BB11B_MAX_TRANSMIT_UNIT     1500         

// Definition of 11b data rate code
#define DOT11B_PLCP_DATA_RATE_1M                        0x0A
#define DOT11B_PLCP_DATA_RATE_2M                        0x14
#define DOT11B_PLCP_DATA_RATE_5P5M                      0x37
#define DOT11B_PLCP_DATA_RATE_11M                       0x6E

// Mask for modulation mode
#define DR_ALG                      0x01
#define DR_PREAMBLE                 0x02
#define DR_1M                       0x10
#define DR_2M                       0x20
#define DR_5M                       0x30
#define DR_11M                      0x40

#define UPSAMPLE_FACTOR             4
#define TX_FIR_DEPTH                37

#define HISTORY_LEN                 16
#define HISTORY_MASK                (HISTORY_LEN - 1)

// Constants for representing spd energy level
enum EnergyLevel
{
    EL_NOISE    = 0,
    // Note: below must be powers of 2
    EL_LOW      = 1,
    EL_HIGH     = 2,
};

// Maxium symbol lenght for one packet equals to 
// (MTU + PLCP + crc32) * 8bit * 4UpSample * 11Spread * sizeof(COMPLEX8)
#define BB11B_MAX_SYMBOL_LENGTH         \
    ((BB11B_MAX_TRANSMIT_UNIT +         \
    sizeof(DOT11B_PLCP_LONG_FRAME) +    \
    sizeof(ULONG)) * 8 * 4 * 11 *       \
    sizeof(COMPLEX8))

typedef struct _BB11B_COMMON
{
    BOOLEAN                 b_isEven;               // for decoding half bytes
    int                     b_barkerIndicator;
    int                     b_barkerIndCount;
    char                    b_demodulateRate;
    char                    b_prevDataRate;
    unsigned char           b_scrambleSeed;
    unsigned short          b_SFDReg;
    unsigned int            b_dataByteExpecting;
    unsigned int            b_dataByteDemodulated;
    unsigned int            b_dataByteDescrambled;

    PUCHAR                  b_dataOutputBuffer;     // pointer to decoding output
    DOT11B_PLCP_HEADER      b_PLCPHeader;           // space for PLCP Header Decoding

    unsigned char           b_byteTemp5M;

    unsigned long           b_crc32;
    unsigned long           b_crc32Store;	        // temp store for CRC32 calc

    unsigned int            b_errEnergyLoss;        // energy loss during a package
    unsigned int            b_errFrame;             // CRC32 error
    unsigned int            b_errPLCPHeader;        // PLCP header incorrect
    unsigned int            b_goodFrameCounter;

    LARGE_INTEGER           b_SFDFindTimeStamp;

    unsigned int            b_length;               // total length written or going to write, including CRC32
    unsigned int            b_lengthWritten;        // length already written
    unsigned char           b_dataRate;
    char                    b_isModPBCC;
    char                    b_isLongPreamble;
    PUCHAR                  b_outputPt;
    ULONG                   b_maxOutputSize;
} BB11B_COMMON, *PBB11B_COMMON;

// Pre-declare the baseband internal used data structure to make compiler happy.
typedef struct __BB11B_RX_FIFOS BB11B_RX_FIFOS, *PBB11B_RX_FIFOS;

#pragma warning (push)
#pragma warning (disable:4324)
/* BB11B specific PHY RX context */
typedef struct __BB11B_RX_CONTEXT {
    unsigned int            b_maxDescCount;         // max desc to scan
    int                     b_resetFlag;            // if reset everything
    int                     b_downSampleIndicator;  
    short                   b_energyLeast;
    FLAG *                  b_workIndicator;        // pointer to flag, 0 for force stop, 1 for work
    COMPLEX16               b_cLast;
    int                     b_shiftRight;           // shift bits after downsampling, positive for right shift, negative for left

    A16 COMPLEX16           b_dcOffset;             // estimate DC offset due to the hardware flaw
    __m128i                 DcOffsetSlot;
    A16 USHORT              DownSampleSum[M128_WORD_NUM];
    A16 COMPLEX16           DescBuffer[SORA_RX_SIGNAL_UNIT_COMPLEX16_NUM * SORA_RX_SIGNAL_UNIT_NUM_PER_DESC];
#ifdef USER_MODE
    int                     rgDSIndex[8];           // indice of picked samples (in 28 samples)
#endif
    BB11B_COMMON            BB11bCommon;
    PBB11B_RX_FIFOS         b_rxFifos;
} BB11B_RX_CONTEXT, *PBB11B_RX_CONTEXT;
#pragma warning (pop)

// BB11B specific data structure for Software Packet Detection (carrier sensing)
typedef struct _BB11B_SPD_CONTEXT
{
    unsigned int            b_minDescCount;
    unsigned int            b_maxDescCount;

    unsigned int            b_threshold;
    unsigned int            b_thresholdLH;
    unsigned int            b_thresholdHL;
    unsigned int            b_gainLevel;
    unsigned int            b_gainLevelNext;
    int                     b_resetFlag;
    FLAG *                  b_workIndicator;

    COMPLEX16               b_dcOffset;          // estimate DC offset of hardware and pass it to BBInfo
    char                    b_reestimateOffset;  // set to TRUE to reset DC estimation module

    unsigned int            History[HISTORY_LEN];
    unsigned int            HisPointer;
	ULONG                   b_evalEnergy;       // energy evaluated for AGC

#pragma warning (disable: 4324)
    __m128i                 BlockEnergySum;
    A16 COMPLEX16   SampleBlockBuffer[SORA_RX_SIGNAL_UNIT_SIZE * SORA_RX_SIGNAL_UNIT_NUM_PER_DESC];
    __m128i                 dcOffset;
#pragma warning (default: 4324)
} BB11B_SPD_CONTEXT, *PBB11B_SPD_CONTEXT;

SORA_EXTERN_C
HRESULT BB11BSpd(
    IN OUT PBB11B_SPD_CONTEXT       pRxContext, 
    IN OUT PSORA_RADIO_RX_STREAM    pRxStream
    );

SORA_EXTERN_C
HRESULT BB11BRx(
    IN OUT PBB11B_RX_CONTEXT        pRxContext, 
    IN OUT PSORA_RADIO_RX_STREAM    pRxStream
    );

SORA_EXTERN_C
HRESULT BB11BPMDSpreadFIR4SSE(
    IN PCOMPLEX8                    pcSrc, 
    IN UINT                         uiInputSize, 
    IN PCOMPLEX8                    pcDest, 
    IN PULONG                       puiOutputSize
    );

SORA_EXTERN_C
HRESULT BB11BPMDSpreadFIR4ASM(
    IN PCOMPLEX8                    pcSrc, 
    IN UINT                         uiInputSize, 
    IN PCOMPLEX8                    pcDest, 
    IN PULONG                       puiOutputSize
    );

SORA_EXTERN_C
HRESULT BB11BPMDBufferTx4XWithShortHeader(
    IN PDOT11B_PLCP_TXVECTOR    pTxVector,
    IN PUCHAR                   pbData,
    IN UINT                     dataLength,
    IN PUCHAR                   pOutput, 
    IN PUINT                    pOutputLength
    );

SORA_EXTERN_C
HRESULT BB11BPMDPacketGenSignal(
    IN OUT PPACKET_BASE             pFrame, 
    IN PDOT11B_PLCP_TXVECTOR        pTxVector, 
    IN PUCHAR                       pTempBuffer,
    IN ULONG                        TempBufferLength
    );

SORA_EXTERN_C
void BB11BRxSpdContextInit(
    OUT PBB11B_RX_CONTEXT           pRxContext, 
    OUT PBB11B_SPD_CONTEXT          pSpdContext,
    IN PFLAG                        pfCanWork, 
    IN ULONG                        nRxMaxBlockCount,  
    IN ULONG                        nSPDMaxBlockCount, 
    IN ULONG                        nSPDMinBlockCount, 
    IN ULONG                        nSPDThreashold,
    IN ULONG                        nSPDThreasholdLow,
    IN ULONG                        nSPDThreasholdHigh,
    IN ULONG                        nShiftRight);

SORA_EXTERN_C
void BB11BRxSpdContextCleanUp(PBB11B_RX_CONTEXT pRxContext);

SORA_EXTERN_C
void BB11BTxVectorInit(
    OUT PDOT11B_PLCP_TXVECTOR       pTxVector, 
    IN  UCHAR                       cDataRate,
    IN  UCHAR                       cModSelect,
    IN  UCHAR                       cPreambleType);

SORA_EXTERN_C
FINL void BB11BPrepareRx(PBB11B_RX_CONTEXT pRxContext, PVOID pOutputBuf, ULONG OutputBufSize)
{
    pRxContext->BB11bCommon.b_outputPt       = (PUCHAR)pOutputBuf;
    pRxContext->BB11bCommon.b_maxOutputSize  = OutputBufSize;
}

SORA_EXTERN_C
FINL void BB11BNewFrameReset(PBB11B_RX_CONTEXT pRxContext);

SORA_EXTERN_C
FINL HRESULT BB11BContextIntegrityCheck(PBB11B_RX_CONTEXT pRxContext, unsigned int descCount);

SORA_EXTERN_C
FINL HRESULT BB11BDownSample(IN OUT PBB11B_RX_CONTEXT pRxContext, __m128i *pDownSampleSum);

SORA_EXTERN_C
FINL void BB11BDecodePartial(IN OUT PBB11B_RX_CONTEXT pRxContext);

SORA_EXTERN_C
FINL HRESULT BB11BDescramblePartial(IN OUT PBB11B_RX_CONTEXT pRxContext);

SORA_EXTERN_C
FINL HRESULT BB11BSwitchPLCPHeaderAndData(IN OUT PBB11B_RX_CONTEXT pRxContext);

SORA_EXTERN_C
FINL void BB11BSpdResetHistory(OUT PBB11B_SPD_CONTEXT  pSpdContext);

SORA_EXTERN_C
FINL int BB11BSpdUpdateEngeryHistoryAndCheckThreshold(PBB11B_SPD_CONTEXT pSpdContext, unsigned int t0, unsigned int t1);

BOOLEAN BB11BSpdCheckThreshold(unsigned int history[], unsigned int flag);
