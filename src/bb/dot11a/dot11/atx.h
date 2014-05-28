#pragma once

#include "bb/bba.h"
#include "bb/mod.h"

#ifdef USER_MODE
#define BBRAND(pseed) rand() /* pseed no use */
#else
#define BBRAND(pseed) RtlRandomEx(pseed)
#endif

#define SCRAMBLED_MAX 5000
#define ENCODED_MAX (SCRAMBLED_MAX * 2)

#define COMPLEX_PER_SYMBOL 160

#define SYMBOL_COUNT_PREAMBLE 4
#define SYMBOL_COUNT_SIGNAL 1

#define DBPS_6M     24
#define DBPS_9M     36
#define DBPS_12M    48
#define DBPS_18M    72
#define DBPS_24M    96
#define DBPS_36M    144
#define DBPS_48M    192
#define DBPS_54M    216

const static int DBPS_ALL[] =
{
    DBPS_6M,
    DBPS_9M,
    DBPS_12M,
    DBPS_18M,
    DBPS_24M,
    DBPS_36M,
    DBPS_48M,
    DBPS_54M
};

#define CBPS_6M     48
#define CBPS_9M     48
#define CBPS_12M    96
#define CBPS_18M    96
#define CBPS_24M    192
#define CBPS_36M    192
#define CBPS_48M    288
#define CBPS_54M    288

#define BPSC_6M     1
#define BPSC_9M     1
#define BPSC_12M    2
#define BPSC_18M    2
#define BPSC_24M    4
#define BPSC_36M    4
#define BPSC_48M    6
#define BPSC_54M    6

#define COMPLEX_COUNT_PREAMBLE (SYMBOL_COUNT_PREAMBLE * COMPLEX_PER_SYMBOL)
#define COMPLEX_COUNT_SIGNAL (SYMBOL_COUNT_SIGNAL * COMPLEX_PER_SYMBOL)

#define SERVICE_LEN_IN_BYTES 2
#define SERVICE_LEN_IN_BITS 16
#define PADDING_LEN_IN_BITS 6

#define SIGNAL_RATE_6M  DOT11A_RATE_6M
#define SIGNAL_RATE_9M  DOT11A_RATE_9M
#define SIGNAL_RATE_12M DOT11A_RATE_12M
#define SIGNAL_RATE_18M DOT11A_RATE_18M
#define SIGNAL_RATE_24M DOT11A_RATE_24M
#define SIGNAL_RATE_36M DOT11A_RATE_36M
#define SIGNAL_RATE_48M DOT11A_RATE_48M
#define SIGNAL_RATE_54M DOT11A_RATE_54M

#define SIGNAL_LENGTH_OFFSET 5
#define SIGNAL_PARITY_OFFSET 17

__forceinline
unsigned int GetSignal(unsigned char bRateCode, unsigned short usLength)
{
    unsigned int uiRet;
    unsigned int uiParity;

    uiRet = bRateCode;
    uiRet |= ((unsigned int)(usLength)) << 5;

    uiParity = uiRet ^ (uiRet >> 16);
    uiParity ^= uiParity >> 8;
    uiParity ^= uiParity >> 4;
    uiParity ^= uiParity >> 2;
    uiParity ^= uiParity >> 1;
    uiParity &= 0x1;

    uiRet |= uiParity << 17;

    return uiRet;
}

__forceinline
unsigned int CopyPreamble16_NT(PBB11A_TX_VECTOR info, COMPLEX8 * pcOutput, COMPLEX16 * pcLast)
{
    const COMPLEX16 * pcInput = PREAMBLE40_11A();
    const COMPLEX16 * pcTemp;

    unsigned int sizeOutput = UpsampleAndCopyNT(info, pcOutput, pcInput, PREAMBLE40_11A_LUT_SIZE);

    pcTemp = (PREAMBLE40_11A() + (PREAMBLE40_11A_LUT_SIZE - 128));

    pcLast[0].re = pcTemp[0].re - (pcTemp[0].re >> 2);
    pcLast[0].im = pcTemp[0].im - (pcTemp[0].im >> 2);
    pcLast[1].re = pcTemp[1].re >> 1;
    pcLast[1].im = pcTemp[1].im >> 1;
    pcLast[2].re = pcTemp[2].re >> 2;
    pcLast[2].im = pcTemp[2].im >> 2;
    pcLast[3].re = 0;
    pcLast[3].im = 0;
    
    return sizeOutput;
}

__forceinline
unsigned int GenerateSignal(PBB11A_TX_VECTOR info, COMPLEX8 * pcOutput, COMPLEX16 * pcLast, 
        unsigned char bRate, unsigned short usLength)
{
    char rgbSignal[4];

    *(unsigned int *)(rgbSignal) = GetSignal(bRate, usLength);

    /*
    KdPrint(("Rate code: %02x\n", bRate));
    KdPrint(("Len: %d\n", usLength)); */

    // KdPrint(("Frame signal: %06x\n", (*(unsigned int *)(rgbSignal)) & 0xFFFFFF));

    return Generate6MSymbol(info, pcOutput, pcLast, rgbSignal, 0);
}
