#pragma once

#include "complex.h"
#include "bbb_tx.h"

#define CCK5_SPREAD_LENGTH          16

typedef struct _LUT_ELEMENT_DBPSK_UCHAR_SPREADED_COMPLEX{
    COMPLEX8 Values[DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR]; // one bit is spreaded into 11 bits, we process a UCHAR(8 bits) a time, produce 88 complexes
    UCHAR        bNewRef;   // new ref value after processing this UCHAR
}LUT_ELEMENT_DBPSK_UCHAR_SPREADED_COMPLEX, *PLUT_ELEMENT_DBPSK_UCHAR_SPREADED_COMPLEX;

typedef struct _LUT_ELEMENT_DQPSK_UCHAR_SPREADED_COMPLEX{
    COMPLEX8 Values[DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR]; // two bits are spreaded into 11 bits, we process a UCHAR(8 bits) a time, produce 44 complexes
    UCHAR        bNewRef;   // new ref value after processing this UCHAR
}LUT_ELEMENT_DQPSK_UCHAR_SPREADED_COMPLEX, *PLUT_ELEMENT_DQPSK_UCHAR_SPREADED_COMPLEX;

typedef struct _LUT_ELEMENT_CCK11_UCHAR_SPREADED_COMPLEX
{
    COMPLEX8     Values[CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR]; // 8 bits are spreaded into 8 complexes, we process a UCHAR(8bits) a time, produce 8 complexes
    UCHAR        bNewRef; // new ref for next step
}LUT_ELEMENT_CCK11_UCHAR_SPREADED_COMPLEX, *PLUT_ELEMENT_CCK11_UCHAR_SPREADED_COMPLEX;

typedef struct _LUT_ELEMENT_CCK5_UCHAR_SPREADED_COMPLEX
{
    COMPLEX8  Values[CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR]; // 4 bits are spreaded into 8 complexes, we process a UCHAR(8bits) a time, produce 16 complexes
    UCHAR         bNewRef; // new ref for next step
}LUT_ELEMENT_CCK5_UCHAR_SPREADED_COMPLEX, *PLUT_ELEMENT_CCK5_UCHAR_SPREADED_COMPLEX;

extern const unsigned char gc_ScramblerLUT[256][128];
extern const LUT_ELEMENT_DBPSK_UCHAR_SPREADED_COMPLEX gc_DBPSKUCHARSpreadedComplexLUT[256][2];
extern const LUT_ELEMENT_DQPSK_UCHAR_SPREADED_COMPLEX gc_DQPSKUCHARSpreadedComplexLUT[256][4];
extern const LUT_ELEMENT_CCK11_UCHAR_SPREADED_COMPLEX gc_CCK11UCHARSpreadedComplexLUT[256][4][2];
extern const LUT_ELEMENT_CCK5_UCHAR_SPREADED_COMPLEX gc_CCK5UCHARSpreadedComplexLUT[256][4];
extern const unsigned char gc_ScramblerLUT[256][128];
