#pragma once
#include "sora.h"
#include "const.h"
#include "dspcomm.h"
#include "ieee80211const.h"

FINL
ushort BB11nGetCodingRateFromMcsIndex ( ulong mcs_index) 
{
	uchar code_rate = CR_12;
	switch (mcs_index % 8) {
	case 5:
		code_rate = CR_23;
		break;
	case 2:
	case 4:
    case 6:
		code_rate = CR_34;
		break;
    case 15:
		code_rate = CR_56;
		break;
	}
	return code_rate;
}

FINL
ulong B11nGetNDBPS(ulong mcs_index)
{
    return DOT11N_RATE_PARAMS[mcs_index].ndbps;
}

FINL
int ht_symbol_count(int mcs, int length_bytes)
{
    int ntotalbits   = length_bytes * 8 + service_length + padding;
    int nsymbolcount = ceil_div(ntotalbits, B11nGetNDBPS(mcs));
    return nsymbolcount;
}

FINL
int ht_padding_bytes(int mcs, int length_bytes)
{
    int padding_bytes   = 0;
    int symbol_count    = 0;
    int total_bits      = 0;
    int total_bytes     = 0;
    symbol_count = ht_symbol_count(mcs, length_bytes);
    total_bits = symbol_count * B11nGetNDBPS(mcs);
    total_bytes = total_bits / 8;
    total_bytes += total_bits % 8 > 0 ? 1 : 0;
    padding_bytes = total_bytes - length_bytes - 2;

    return padding_bytes;
}
