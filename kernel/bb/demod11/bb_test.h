#pragma once

#include <stdio.h>
#include "bb/bba.h"
#include "dot11_plcp.h"
#include "atx.h"
#include "CRC32.h"
#include "func.h"

typedef struct _DOT11_DEMOD_ARGS
{
	int nPowerThreshold;
	int nPowerThresholdLH;
	int nPowerThresholdHL;
	int nShiftRight;
	int nStartDesc;
	const char* pcInFileName;
    const char* pcOutFileName;
    unsigned int SampleRate;
}DOT11_DEMOD_ARGS, *PDOT11_DEMOD_ARGS;

typedef struct _DOT11_MOD_ARGS
{
    int nBitRate;
    unsigned int SampleRate;
    const char* pcInFileName;
    const char* pcOutFileName;
}DOT11_MOD_ARGS, *PDOT11_MOD_ARGS;

#define DEFAULT_THRESHOLD 4000
#define DEFAULT_THRESHOLD_LH UINT_MAX
#define DEFAULT_THRESHOLD_HL 4000
#define DEFAULT_SHIFT_RIGHT 4
#define DEFAULT_START_DESC 0
#define DEFAULT_BITRATE 6000
#define DEFAULT_SAMPLERATE 44

inline int PreparePacket(const char* pcFile, PMDL pMdl, PPACKET_BASE pPacket, PUCHAR bufPacket, PUCHAR bufSymbol, ULONG sizeSymbol)
{
    int nLen = LoadDumpFile(pcFile, bufPacket, INPUT_MAX);
    if (nLen < 0)
    {
        puts("Failed to load input file.");
        return -1;
    }

    ULONG& crcStore = *(PULONG)(bufPacket + nLen - 4);
    ULONG crcCalc = CalcCRC32(bufPacket, nLen-  4);

    if (crcStore != crcCalc)
    {
        printf("[WARNING] CRC32 %d in input file is not correct. Use calculated CRC32 0x%X instead.\n", crcStore, crcCalc);
        crcStore = crcCalc;
    }

    SoraPacketInitializeByBuffer(pPacket, pMdl, bufPacket, nLen - 4, crcStore, bufSymbol, sizeSymbol);

    return 0;
}
