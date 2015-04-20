#pragma once

// dspcomm.h - defines common basic types here

#include <const.h>
#include <soratypes.h>
#include <complex.h>
#include <complex_ext.h>
#include <vector128.h>
#include <fft.h>

typedef unsigned char	uchar;
typedef unsigned short	ushort;
typedef unsigned long	ulong;
typedef unsigned int    uint;

typedef COMPLEX8        TXSAMPLE;
typedef COMPLEX16       RXSAMPLE;

#include <intalg.h>
#include <dspalg.hpp>

// Global error codes
#define BK_ERROR_SUCCESS         0x00000000
#define BK_ERROR_FAILED          0x8000FFFF
#define BK_ERROR_HARDWARE_FAILED 0x8000FFFE
#define BK_ERROR_TIMESTAMP_DROP  0x8000FFFD // multiple radio cannot sync timestamps
#define BK_ERROR_TIMESTAMP_DROPS 0x8000FFFC // multiple radio cannot sync timestamps

#define BK_ERROR_INVALID_PARAM  (-1) 		// 0xFFFFFFFF
#define BK_ERROR_FILE_NOT_FOUND (-2)  		// 0xFFFFFFFE
#define BK_ERROR_INVALID_DUMP_FILE (-3) 	// 0xFFFFFFFD
#define BK_ERROR_YIELD          (-4)        // 0xFFFFFFFC
