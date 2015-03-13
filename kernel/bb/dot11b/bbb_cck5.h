/*++
Copyright (c) Microsoft Corporation

Module Name: bbb_cck5.h

Abstract: 5.5Mbps CCK decoding macro.

History: 
          7/7/2009: Modified by senxiang.
--*/

#pragma once

#include "const.h"
#include "bbb_dqpsk.h"

#define CCK5P5_EVEN_DECODER(pbDecodedValue, P0, P1, P2, P3, P4, P5, P6, P7, pLastItem)\
{\
    A32 COMPLEX32 A1, A2, A3, A4;\
    A32 COMPLEX32 B1, B5;\
    A32 COMPLEX32 L1;\
    \
    long Max1 = 0, Max2 = 0;\
    unsigned char Value1 = 0, Value2 = 0;\
    \
    A1.re = P0.im + P1.re;\
    A1.im = P1.im - P0.re;\
    A2.re = P2.im - P3.re;\
    A2.im = -(P2.re + P3.im);\
    A3.re = P4.im + P5.re;\
    A3.im = P5.im - P4.re;\
    A4.re = P7.re - P6.im;\
    A4.im = P6.re + P7.im;\
    \
    B1.re = A1.re + A2.re; B1.im = A1.im + A2.im;\
    B5.re = A3.re + A4.re; B5.im = A3.im + A4.im;\
    \
    B1.im = -B1.im;\
    \
    L1.re = B1.re * B5.re - B1.im * B5.im;\
    L1.im = B1.im * B5.re + B1.re * B5.im;\
    \
    if (L1.re > 0) {\
        Max1 = L1.re; Value1 = 0x00;\
    } else {\
        Max1 = -L1.re; Value1 = 0x08;\
    }\
    A1.re = P1.re - P0.im;\
    A1.im = P0.re + P1.im;\
    A2.re = -(P2.im + P3.re);\
    A2.im = P2.re - P3.im;\
    A3.re = P5.re - P4.im;\
    A3.im = P4.re + P5.im;\
    A4.re = P6.im + P7.re;\
    A4.im = P7.im - P6.re;\
    \
    B1.re = A1.re + A2.re; B1.im = A1.im + A2.im;\
    B5.re = A3.re + A4.re; B5.im = A3.im + A4.im;\
    \
    B1.im = -B1.im;\
    \
    L1.re = B1.re * B5.re - B1.im * B5.im;\
    L1.im = B1.im * B5.re + B1.re * B5.im;\
    \
    \
    if (L1.re > 0) {\
        Max2 = L1.re; Value2 = 0x04;\
    } else {\
        Max2 = -L1.re; Value2 = 0x0c;\
    }\
    \
    if(Max1 > Max2) {\
        *pbDecodedValue = Value1;\
    } else {\
        *pbDecodedValue = Value2;\
    }\
    DEMAP_BIT_DQPSK((*pbDecodedValue), 0, (*pLastItem), (P7));\
    pLastItem->re = P7.re;\
    pLastItem->im = P7.im;\
}\

#define CCK5P5_ODD_DECODER(pbDecodedValue, P0, P1, P2, P3, P4, P5, P6, P7, pLastItem)\
{\
    A32 COMPLEX32 A1, A2, A3, A4;\
    A32 COMPLEX32 B1, B5;\
    A32 COMPLEX32 L1;\
    \
    long Max1 = 0, Max2 = 0;\
    unsigned char Value1 = 0, Value2 = 0;\
    \
    A1.re = P0.im + P1.re;\
    A1.im = P1.im - P0.re;\
    A2.re = P2.im - P3.re;\
    A2.im = -(P2.re + P3.im);\
    A3.re = P4.im + P5.re;\
    A3.im = P5.im - P4.re;\
    A4.re = P7.re - P6.im;\
    A4.im = P6.re + P7.im;\
    \
    B1.re = A1.re + A2.re; B1.im = A1.im + A2.im;\
    B5.re = A3.re + A4.re; B5.im = A3.im + A4.im;\
    \
    B1.im = -B1.im;\
    \
    L1.re = B1.re * B5.re - B1.im * B5.im;\
    L1.im = B1.im * B5.re + B1.re * B5.im;\
    \
    if (L1.re > 0) {\
        Max1 = L1.re; Value1 = 0x00;\
    } else {\
        Max1 = -L1.re; Value1 = 0x80;\
    }\
    \
    A1.re = P1.re - P0.im;\
    A1.im = P0.re + P1.im;\
    A2.re = -(P2.im + P3.re);\
    A2.im = P2.re - P3.im;\
    A3.re = P5.re - P4.im;\
    A3.im = P4.re + P5.im;\
    A4.re = P6.im + P7.re;\
    A4.im = P7.im - P6.re;\
    \
    B1.re = A1.re + A2.re; B1.im = A1.im + A2.im;\
    B5.re = A3.re + A4.re; B5.im = A3.im + A4.im;\
    \
    B1.im = -B1.im;\
    \
    L1.re = B1.re * B5.re - B1.im * B5.im;\
    L1.im = B1.im * B5.re + B1.re * B5.im;\
    \
    \
    if (L1.re > 0) {\
        Max2 = L1.re; Value2 = 0x40;\
    } else {\
        Max2 = -L1.re; Value2 = 0xc0;\
    }\
    \
    if(Max1 > Max2) {\
        *pbDecodedValue |= Value1;\
    } else {\
        *pbDecodedValue |= Value2;\
    }\
    DEMAP_BIT_DQPSK((*pbDecodedValue), 4, (*pLastItem), (P7));\
    *pbDecodedValue ^= 0x30; \
    pLastItem->re = P7.re;\
    pLastItem->im = P7.im;\
}

