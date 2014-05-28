/*++
Copyright (c) Microsoft Corporation

Module Name: soratypes.h

Abstract: common type definition.
--*/

#pragma once

#include <emmintrin.h>
#include <windef.h>
#include "const.h"
#include "complex.h"

#ifdef _M_X64
typedef unsigned __int64    UPOINTER;
#else
typedef unsigned __int32    UPOINTER;
#endif 

typedef __int32             s_int32;
typedef unsigned __int32    s_uint32;
typedef __int16             s_int16;
typedef unsigned __int16    s_uint16;

typedef __int32             RADIO_MASK;
typedef char                FLAG, *PFLAG;

typedef unsigned char	uchar;
typedef unsigned short  ushort;
typedef unsigned long	ulong;
typedef unsigned int    uint;

typedef COMPLEX16 SAMPLE, *PSAMPLE, DC_OFFSET, *PDC_OFFSET;

// Sample type
typedef COMPLEX16 RXSAMPLE, *PRXSAMPLE;
typedef COMPLEX8  TXSAMPLE, *PTXSAMPLE;

#define M128_BYTE_NUM                       (sizeof(__m128)/sizeof(BYTE))
#define M128_WORD_NUM                       (sizeof(__m128)/sizeof(WORD))
#define M128_DWORD_NUM                      (sizeof(__m128)/sizeof(DWORD))
#define M128_COMPLEX16_NUM                  (sizeof(__m128)/sizeof(COMPLEX16))

CCASSERT(M128_BYTE_NUM == 16);
CCASSERT(M128_WORD_NUM == 8);
CCASSERT(M128_DWORD_NUM == 4);
CCASSERT(M128_COMPLEX16_NUM == 4);
