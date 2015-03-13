#pragma once
#include "sora.h"
#include "const.h"
#include "dspcomm.h"
#include "ieee80211const.h"

FINL 
ulong B11aGetPLCPSignal(uchar bRateCode, ushort usLength)
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

FINL
uchar B11aGetCodeFromKbps ( ulong rate_Kbps ) {
	uchar code = 0;
	switch (rate_Kbps) {
	case  6000:
		code = DOT11A_RATE_6M;
		break;
	case  9000:
		code = DOT11A_RATE_9M;
		break;
	case 12000:
		code = DOT11A_RATE_12M;
		break;
	case 18000:
		code = DOT11A_RATE_18M;
		break;
	case 24000:
		code = DOT11A_RATE_24M;
		break;
	case 36000:
		code = DOT11A_RATE_36M;
		break;
	case 48000:
		code = DOT11A_RATE_48M;
		break;
	case 54000:
		code = DOT11A_RATE_54M;
		break;
	default:
		code = 0;
	}
	return code;
}

//
// 11a - Get number of data bits per symbol
//
FINL
uchar B11aGetNDBPS ( ulong rate_Kbps ) {
	uchar NDBPS = 0;
	switch (rate_Kbps) {
	case  6000:
		NDBPS = 24;
		break;
	case  9000:
		NDBPS = 36;
		break;
	case 12000:
		NDBPS = 48;
		break;
	case 18000:
		NDBPS = 72;
		break;
	case 24000:
		NDBPS = 96;
		break;
	case 36000:
		NDBPS = 144;
		break;
	case 48000:
		NDBPS = 192;
		break;
	case 54000:
		NDBPS = 216;
		break;
	}
	return NDBPS;
}

static const ulong BB11aDataRateLUT[16] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	48000,
	24000,
	12000,
	 6000,
	54000,
	36000,
	18000,
	 9000
};

FINL
ulong BB11aParseDataRate ( uchar uiSignal) {
	return BB11aDataRateLUT[uiSignal & 0xF];
}

FINL
ushort BB11aGetCodingRateFromDataRate ( ulong data_rate_kbps) 
{
	uchar code_rate = CR_12;
	switch (data_rate_kbps) {
	case 48000:
		code_rate = CR_23;
		break;
	case  9000:		
	case 18000:		
	case 36000:
	case 54000:
		code_rate = CR_34;
		break;
	}
	return code_rate;
}

FINL
ulong BB11aGetNCBPS ( ulong data_rate_kbps) 
{
	ulong ret;
	switch (data_rate_kbps) {
    case 6000: case 9000:
        ret = 48; break;
    case 12000: case 18000:
        ret = 96; break;
    case 24000: case 36000:
        ret = 192; break;
    case 48000: case 54000:
        ret = 288; break;
    default:
        NODEFAULT;
	}
	return ret;
}

FINL
int B11aGetSymbolCount(ulong data_rate_kbps, ushort frame_length)
{
    int ntotalbits   = frame_length * 8 + service_length + padding;
    int nsymbolcount = ceil_div(ntotalbits, B11aGetNDBPS(data_rate_kbps));
    return nsymbolcount;
}
