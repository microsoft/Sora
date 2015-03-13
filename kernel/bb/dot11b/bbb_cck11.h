/*++
Copyright (c) Microsoft Corporation

Module Name: bbb_cck11.h

Abstract: 11Mbps CCK decoding macro.

History: 
          7/7/2009: Modified by senxiang.
--*/
#pragma once

#include "const.h"

#include "bbb_dqpsk.h"
#define CCK11_DECODER(pbDecodedValue, P0, P1, P2, P3, P4, P5, P6, P7, pLastItem, bEven, lable4, lableend)\
{\
    A32 COMPLEX32 A1, A2, A3, A4; \
    A32 COMPLEX32 B1, B2, B3, B4, B5, B6, B7, B8; \
    A32 COMPLEX32 L1, L2, L3, L4; \
    \
    long Module1Max = 0, Module2Max = 0, Module34Max = 0; \
    long Max1 = 0, Max2 = 0, Max3 = 0, Max4 = 0; \
    unsigned char Value1 = 0, Value2 = 0, Value3 = 0, Value4 = 0; \
    unsigned char Module1DecodedValue = 0, Module2DecodedValue = 0, Module34DecodedValue = 0; \
    \
    long ABS1 = 0, ABS2 = 0; \
    A1.re = P0.re + P1.re;\
    A1.im = P0.im + P1.im;\
    A2.re = P2.re - P3.re;\
    A2.im = P2.im - P3.im;\
    A3.re = P4.re + P5.re;\
    A3.im = P4.im + P5.im;\
    A4.re = P7.re - P6.re;\
    A4.im = P7.im - P6.im;\
    \
    \
    B1.re = A2.re + A1.re; B1.im = A2.im + A1.im;\
    B3.re = A2.re - A1.re; B3.im = A2.im - A1.im;\
    B2.re = A2.re - A1.im; B2.im = A2.im + A1.re;\
    B4.re = A2.re + A1.im; B4.im = A2.im - A1.re;\
    \
    B5.re = A4.re + A3.re; B5.im = A4.im + A3.im;\
    B7.re = A4.re - A3.re; B7.im = A4.im - A3.im;\
    B6.re = A4.re - A3.im; B6.im = A4.im + A3.re;\
    B8.re = A4.re + A3.im; B8.im = A4.im - A3.re;\
    \
    L1.re = B1.re * B5.re + B1.im * B5.im;\
    L1.im = B1.re * B5.im - B1.im * B5.re;\
    L2.re = B2.re * B6.re + B2.im * B6.im;\
    L2.im = B2.re * B6.im - B2.im * B6.re;\
    L3.re = B3.re * B7.re + B3.im * B7.im;\
    L3.im = B3.re * B7.im - B3.im * B7.re;\
    L4.re = B4.re * B8.re + B4.im * B8.im;\
    L4.im = B4.re * B8.im - B4.im * B8.re;\
    \
    if(L1.re < 0) ABS1 = -L1.re; else ABS1 = L1.re;\
    if(L1.im < 0) ABS2 = -L1.im; else ABS2 = L1.im;\
    if (ABS1 > ABS2) {\
        if (L1.re > 0) {\
            Max1 = L1.re; Value1 = 0x00;\
        } else {\
            Max1 = -L1.re; Value1 = 0x40;\
        }\
    } else {\
        if (L1.im > 0) {\
            Max1 = L1.im; Value1 = 0xC0;\
        } else {\
            Max1 = -L1.im; Value1 = 0x80;\
        }\
    }\
    \
    if(L2.re < 0) ABS1 = -L2.re; else ABS1 = L2.re;\
    if(L2.im < 0) ABS2 = -L2.im; else ABS2 = L2.im;\
    if (ABS1 > ABS2) {\
        if (L2.re > 0) {\
            Max2 = L2.re; Value2 = 0x30;\
        } else {\
            Max2 = -L2.re; Value2 = 0x70;\
        }\
    } else {\
        if (L2.im > 0) {\
            Max2 = L2.im; Value2 = 0xF0;\
        } else {\
            Max2 = -L2.im; Value2 = 0xB0;\
        }\
    }\
    \
    if(L3.re < 0) ABS1 = -L3.re; else ABS1 = L3.re;\
    if(L3.im < 0) ABS2 = -L3.im; else ABS2 = L3.im;\
    if (ABS1 > ABS2) {\
        if (L3.re > 0) {\
            Max3 = L3.re; Value3 = 0x10; /*0001*/\
        } else {\
            Max3 = -L3.re; Value3 = 0x50; /*1001*/\
        }\
    } else {\
        if (L3.im > 0) {\
            Max3 = L3.im; Value3 = 0xD0; /*1101*/\
        } else {\
            Max3 = -L3.im; Value3 = 0x90;/*1001*/\
        }\
    }\
    \
    if(L4.re < 0) ABS1 = -L4.re; else ABS1 = L4.re;\
    if(L4.im < 0) ABS2 = -L4.im; else ABS2 = L4.im;\
    if (ABS1 > ABS2) {\
        if (L4.re > 0) {\
            Max4 = L4.re; Value4 = 0x20; /*0010*/\
        } else {\
            Max4 = -L4.re; Value4 = 0x60; /*0110*/\
        }\
    } else {\
        if (L4.im > 0) {\
            Max4 = L4.im; Value4 = 0xE0; /*1110*/\
        } else {\
            Max4 = -L4.im; Value4 = 0xA0; /*1010*/\
        }\
    }\
    \
    \
    if (Max1 > Max2) {\
        Module1Max = Max1; Module1DecodedValue = Value1;\
    } else {\
        Module1Max = Max2; Module1DecodedValue = Value2;\
    }\
    \
    if (Max3 > Max4) {\
        if(Max3 > Module1Max) {\
            Module1Max = Max3; Module1DecodedValue = Value3;\
        }\
    } else {\
        if(Max4 > Module1Max) {\
            Module1Max = Max4; Module1DecodedValue = Value4;\
        }\
    }\
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
    B3.re = A2.re - A1.re; B3.im = A2.im - A1.im;\
    B2.re = A2.re - A1.im; B2.im = A2.im + A1.re;\
    B4.re = A2.re + A1.im; B4.im = A2.im - A1.re;\
    \
    B5.re = A3.re + A4.re; B5.im = A3.im + A4.im;\
    B7.re = A4.re - A3.re; B7.im = A4.im - A3.im;\
    B6.re = A4.re - A3.im; B6.im = A4.im + A3.re;\
    B8.re = A4.re + A3.im; B8.im = A4.im - A3.re;\
    \
    L1.re = B1.re * B5.re + B1.im * B5.im;\
    L1.im = B1.re * B5.im - B1.im * B5.re;\
    L2.re = B2.re * B6.re + B2.im * B6.im;\
    L2.im = B2.re * B6.im - B2.im * B6.re;\
    L3.re = B3.re * B7.re + B3.im * B7.im;\
    L3.im = B3.re * B7.im - B3.im * B7.re;\
    L4.re = B4.re * B8.re + B4.im * B8.im;\
    L4.im = B4.re * B8.im - B4.im * B8.re;\
    \
    if(L1.re < 0) ABS1 = -L1.re; else ABS1 = L1.re;\
    if(L1.im < 0) ABS2 = -L1.im; else ABS2 = L1.im;\
    if (ABS1 > ABS2) {\
        if (L1.re > 0) {\
            Max1 = L1.re; Value1 = 0x00;\
        } else {\
            Max1 = -L1.re; Value1 = 0x40;\
        }\
    } else {\
        if (L1.im > 0) {\
            Max1 = L1.im; Value1 = 0xC0;\
        } else {\
            Max1 = -L1.im; Value1 = 0x80;\
        }\
    }\
    \
    if(L2.re < 0) ABS1 = -L2.re; else ABS1 = L2.re;\
    if(L2.im < 0) ABS2 = -L2.im; else ABS2 = L2.im;\
    if (ABS1 > ABS2) {\
        if (L2.re > 0) {\
            Max2 = L2.re; Value2 = 0x30;\
        } else {\
            Max2 = -L2.re; Value2 = 0x70;\
        }\
    } else {\
        if (L2.im > 0) {\
            Max2 = L2.im; Value2 = 0xF0;\
        } else {\
            Max2 = -L2.im; Value2 = 0xB0;\
        }\
    }\
    \
    if(L3.re < 0) ABS1 = -L3.re; else ABS1 = L3.re;\
    if(L3.im < 0) ABS2 = -L3.im; else ABS2 = L3.im;\
    if (ABS1 > ABS2) {\
        if (L3.re > 0) {\
            Max3 = L3.re; Value3 = 0x10;\
        } else {\
            Max3 = -L3.re; Value3 = 0x50;\
        }\
    } else {\
        if (L3.im > 0) {\
            Max3 = L3.im; Value3 = 0xD0;\
        } else {\
            Max3 = -L3.im; Value3 = 0x90;\
        }\
    }\
    \
    if(L4.re < 0) ABS1 = -L4.re; else ABS1 = L4.re;\
    if(L4.im < 0) ABS2 = -L4.im; else ABS2 = L4.im;\
    if (ABS1 > ABS2) {\
        if (L4.re > 0) {\
            Max4 = L4.re; Value4 = 0x20;\
        } else {\
            Max4 = -L4.re; Value4 = 0x60;\
        }\
    } else {\
        if (L4.im > 0) {\
            Max4 = L4.im; Value4 = 0xE0;\
        } else {\
            Max4 = -L4.im; Value4 = 0xA0;\
        }\
    }\
    \
    if (Max1 > Max2) {\
        Module2Max = Max1; Module2DecodedValue = Value1;\
    } else {\
        Module2Max = Max2; Module2DecodedValue = Value2;\
    }\
    if (Max3 > Max4) {\
        if(Max3 > Module2Max) {\
            Module2Max = Max3; Module2DecodedValue = Value3;\
        }\
    } else {\
        if(Max4 > Module2Max) {\
            Module2Max = Max4; Module2DecodedValue = Value4;\
        }\
    }\
    \
    Module2DecodedValue |= 0x08;\
    \
    if(Module1Max > Module2Max) {\
        goto lable4;\
    }\
    \
    A1.re = P1.re - P0.re;\
    A1.im = P1.im - P0.im;\
    A2.re = -(P2.re + P3.re);\
    A2.im = -(P2.im + P3.im);\
    A3.re = P5.re - P4.re;\
    A3.im = P5.im - P4.im;\
    A4.re = P6.re + P7.re;\
    A4.im = P6.im + P7.im;\
    \
    B1.re = A1.re + A2.re; B1.im = A1.im + A2.im;\
    B3.re = A2.re - A1.re; B3.im = A2.im - A1.im;\
    B2.re = A2.re - A1.im; B2.im = A2.im + A1.re;\
    B4.re = A2.re + A1.im; B4.im = A2.im - A1.re;\
    \
    B5.re = A3.re + A4.re; B5.im = A3.im + A4.im;\
    B7.re = A4.re - A3.re; B7.im = A4.im - A3.im;\
    B6.re = A4.re - A3.im; B6.im = A4.im + A3.re;\
    B8.re = A4.re + A3.im; B8.im = A4.im - A3.re;\
    \
    L1.re = B1.re * B5.re + B1.im * B5.im;\
    L1.im = B1.re * B5.im - B1.im * B5.re;\
    L2.re = B2.re * B6.re + B2.im * B6.im;\
    L2.im = B2.re * B6.im - B2.im * B6.re;\
    L3.re = B3.re * B7.re + B3.im * B7.im;\
    L3.im = B3.re * B7.im - B3.im * B7.re;\
    L4.re = B4.re * B8.re + B4.im * B8.im;\
    L4.im = B4.re * B8.im - B4.im * B8.re;\
    \
    if(L1.re < 0) ABS1 = -L1.re; else ABS1 = L1.re;\
    if(L1.im < 0) ABS2 = -L1.im; else ABS2 = L1.im;\
    if (ABS1 > ABS2) {\
        if (L1.re > 0) {\
            Max1 = L1.re; Value1 = 0x00;\
        } else {\
            Max1 = -L1.re; Value1 = 0x40;\
        }\
    } else {\
        if (L1.im > 0)\
        {\
            Max1 = L1.im; Value1 = 0xC0;\
        } else {\
            Max1 = -L1.im; Value1 = 0x80;\
        }\
    }\
    \
    if(L2.re < 0) ABS1 = -L2.re; else ABS1 = L2.re;\
    if(L2.im < 0) ABS2 = -L2.im; else ABS2 = L2.im;\
    if (ABS1 > ABS2) {\
        if (L2.re > 0) {\
            Max2 = L2.re; Value2 = 0x30;\
        } else {\
            Max2 = -L2.re; Value2 = 0x70;\
        }\
    } else {\
        if (L2.im > 0) {\
            Max2 = L2.im; Value2 = 0xF0;\
        } else {\
            Max2 = -L2.im; Value2 = 0xB0;\
        }\
    }\
    \
    if(L3.re < 0) ABS1 = -L3.re; else ABS1 = L3.re;\
    if(L3.im < 0) ABS2 = -L3.im; else ABS2 = L3.im;\
    if (ABS1 > ABS2) {\
        if (L3.re > 0) {\
            Max3 = L3.re; Value3 = 0x10;\
        } else {\
            Max3 = -L3.re; Value3 = 0x50;\
        }\
    } else {\
        if (L3.im > 0) {\
            Max3 = L3.im; Value3 = 0xD0;\
        } else {\
            Max3 = -L3.im; Value3 = 0x90;\
        }\
    }\
    \
    if(L4.re < 0) ABS1 = -L4.re; else ABS1 = L4.re;\
    if(L4.im < 0) ABS2 = -L4.im; else ABS2 = L4.im;\
    \
    if (ABS1 > ABS2) {\
        if (L4.re > 0) {\
            Max4 = L4.re; Value4 = 0x20;\
        } else {\
            Max4 = -L4.re; Value4 = 0x60;\
        }\
    } else {\
        if (L4.im > 0) {\
            Max4 = L4.im; Value4 = 0xE0;\
        } else {\
            Max4 = -L4.im; Value4 = 0xA0;\
        }\
    }\
    \
    if (Max1 > Max2) {\
        Module34Max = Max1; Module34DecodedValue = Value1;\
    } else {\
        Module34Max = Max2; Module34DecodedValue = Value2;\
    }\
    \
    if (Max3 > Max4) {\
        if(Max3 > Module34Max) {\
            Module34Max = Max3; Module34DecodedValue = Value3;\
        }\
    } else {\
        if(Max4 > Module34Max) {\
            Module34Max = Max4; Module34DecodedValue = Value4;\
        }\
    }\
    \
    Module34DecodedValue |= 0x04;\
    \
    \
    if(Module2Max > Module34Max) {\
        *pbDecodedValue = Module2DecodedValue;\
    } else {\
        *pbDecodedValue = Module34DecodedValue;\
    }\
    \
    goto lableend;\
    \
lable4:\
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
    B3.re = A2.re - A1.re; B3.im = A2.im - A1.im;\
    B2.re = A2.re - A1.im; B2.im = A2.im + A1.re;\
    B4.re = A2.re + A1.im; B4.im = A2.im - A1.re;\
    \
    B5.re = A3.re + A4.re; B5.im = A3.im + A4.im;\
    B7.re = A4.re - A3.re; B7.im = A4.im - A3.im;\
    B6.re = A4.re - A3.im; B6.im = A4.im + A3.re;\
    B8.re = A4.re + A3.im; B8.im = A4.im - A3.re;\
    \
    L1.re = B1.re * B5.re + B1.im * B5.im;\
    L1.im = B1.re * B5.im - B1.im * B5.re;\
    L2.re = B2.re * B6.re + B2.im * B6.im;\
    L2.im = B2.re * B6.im - B2.im * B6.re;\
    L3.re = B3.re * B7.re + B3.im * B7.im;\
    L3.im = B3.re * B7.im - B3.im * B7.re;\
    L4.re = B4.re * B8.re + B4.im * B8.im;\
    L4.im = B4.re * B8.im - B4.im * B8.re;\
    \
    if(L1.re < 0) ABS1 = -L1.re; else ABS1 = L1.re;\
    if(L1.im < 0) ABS2 = -L1.im; else ABS2 = L1.im;\
    \
    if (ABS1 > ABS2) {\
        if (L1.re >0) {\
            Max1 = L1.re; Value1 = 0x00;\
        } else {\
            Max1 = -L1.re; Value1 = 0x40;\
        }\
    } else {\
        if (L1.im > 0) {\
            Max1 = L1.im; Value1 = 0xC0;\
        } else {\
            Max1 = -L1.im; Value1 = 0x80;\
        }\
    }\
    \
    if(L2.re < 0) ABS1 = -L2.re; else ABS1 = L2.re;\
    if(L2.im < 0) ABS2 = -L2.im; else ABS2 = L2.im;\
    \
    if (ABS1 > ABS2) {\
        if (L2.re > 0) {\
            Max2 = L2.re; Value2 = 0x30;\
        } else {\
            Max2 = -L2.re; Value2 = 0x70;\
        }\
    } else {\
        if (L2.im > 0) {\
            Max2 = L2.im; Value2 = 0xF0;\
        } else {\
            Max2 = -L2.im; Value2 = 0xB0;\
        }\
    }\
    \
    if(L3.re < 0) ABS1 = -L3.re; else ABS1 = L3.re;\
    if(L3.im < 0) ABS2 = -L3.im; else ABS2 = L3.im;\
    \
    if (ABS1 > ABS2) {\
        if (L3.re > 0) {\
            Max3 = L3.re; Value3 = 0x10;\
        } else {\
            Max3 = -L3.re;Value3 = 0x50;\
        }\
    } else {\
        if (L3.im > 0) {\
            Max3 = L3.im; Value3 = 0xD0;\
        } else {\
            Max3 = -L3.im; Value3 = 0x90;\
        }\
    }\
    \
    if(L4.re < 0) ABS1 = -L4.re; else ABS1 = L4.re;\
    if(L4.im < 0) ABS2 = -L4.im; else ABS2 = L4.im;\
    \
    if (ABS1 > ABS2) {\
        if (L4.re > 0) {\
            Max4 = L4.re; Value4 = 0x20;\
        } else {\
            Max4 = -L4.re; Value4 = 0x60;\
        }\
    } else {\
        if (L4.im > 0) {\
            Max4 = L4.im; Value4 = 0xE0;\
        } else {\
            Max4 = -L4.im; Value4 = 0xA0;\
        }\
    }\
    \
    if (Max1 > Max2) {\
        Module34Max = Max1; Module34DecodedValue = Value1;\
    } else {\
        Module34Max = Max2; Module34DecodedValue = Value2;\
    }\
    \
    if (Max3 > Max4) {\
        if(Max3 > Module34Max) {\
            Module34Max = Max3; Module34DecodedValue = Value3;\
        }\
    } else {\
        if(Max4 > Module34Max) {\
            Module34Max = Max4; Module34DecodedValue = Value4;\
        }\
    }\
    \
    Module34DecodedValue |= 0x0C;\
    \
    \
    if(Module1Max > Module34Max) {\
        *pbDecodedValue = Module1DecodedValue;\
    } else {\
        *pbDecodedValue = Module34DecodedValue;\
    }\
    \
lableend:\
    DEMAP_BIT_DQPSK((*pbDecodedValue), 0, (*pLastItem), (P7));\
    (*pbDecodedValue) ^= ((bEven << 1) | bEven); \
    bEven ^= 0x1;\
    pLastItem->re = P7.re;\
    pLastItem->im = P7.im;\
}
