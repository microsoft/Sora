#pragma once

#include "const.h"

class DemapperCore
{
	const static uchar m_bpsk_lut  [256];
	const static uchar m_qam64_lut2[256];
	const static uchar m_qam64_lut3[256];
    const static uchar m_qam16_lut2[256];

	FINL static uchar bpsk_demap   ( uchar re ) { return m_bpsk_lut[re]; }
    FINL static uchar qam16_demap2 ( uchar re ) { return m_qam16_lut2[re]; }
    FINL static uchar qam64_demap2 ( uchar re ) { return m_qam64_lut2[re]; }
    FINL static uchar qam64_demap3 ( uchar re ) { return m_qam64_lut3[re]; }

public:
    FINL static void DemapBPSK(const COMPLEX16& input, uchar* output) {
	    *output = bpsk_demap((uchar)(input.re));
    }

    FINL static void DemapQPSK(const COMPLEX16& input, uchar* output) {
        output[0] = bpsk_demap((unsigned char)(input.re));
        output[1] = bpsk_demap((unsigned char)(input.im));
    }
    FINL static void DemapQAM16(const COMPLEX16& input, uchar* output) {
        unsigned char re, im;
        re = (unsigned char)(input.re);
        im = (unsigned char)(input.im);
        output[0] = bpsk_demap(re);
        output[1] = qam16_demap2(re);
        output[2] = bpsk_demap(im);
        output[3] = qam16_demap2(im);
    }

    FINL static void DemapQAM64(const COMPLEX16& input, uchar* output) {
        unsigned char re, im;
        re = (unsigned char)(input.re);
        im = (unsigned char)(input.im);
        output[0] = bpsk_demap(re);
        output[1] = qam64_demap2(re);
        output[2] = qam64_demap3(re);
        output[3] = bpsk_demap(im);
        output[4] = qam64_demap2(im);
        output[5] = qam64_demap3(im);
    }

    template<ushort N_BPSCS> FINL static void Demap(const COMPLEX16& input, uchar* output);

    template<> static void Demap<1>(const COMPLEX16& input, uchar* output) { DemapBPSK(input, output); }
    template<> static void Demap<2>(const COMPLEX16& input, uchar* output) { DemapQPSK(input, output); }
    template<> static void Demap<4>(const COMPLEX16& input, uchar* output) { DemapQAM16(input, output); }
    template<> static void Demap<6>(const COMPLEX16& input, uchar* output) { DemapQAM64(input, output); }
};

SELECTANY const uchar DemapperCore::m_bpsk_lut[256] = {
	4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 
	5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 
};

SELECTANY const uchar DemapperCore::m_qam16_lut2[256] = {
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 
	3, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 
	3, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
};

SELECTANY const uchar DemapperCore::m_qam64_lut2[256] = {
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 5, 5, 4, 3, 3, 
	2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 
	2, 3, 3, 4, 5, 5, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
};

SELECTANY const uchar DemapperCore::m_qam64_lut3[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 
	4, 5, 5, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 5, 4, 4, 3, 3, 2, 
	2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 
	2, 2, 3, 3, 4, 4, 5, 6, 6, 6, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 5, 5, 
	4, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
};

SELECTANY DemapperCore Demapper;

SELECTANY extern const vcs::data_type gDemapMin = {
    { -128, -128 }, { -128, -128 }, { -128, -128 }, { -128, -128 },
};
SELECTANY extern const vcs::data_type gDemapMax = {
    { 127, 127 }, { 127, 127 }, { 127, 127 }, { 127, 127 },
};

template<int N>
FINL void demap_limit ( const COMPLEX16 * input, COMPLEX16 * output ) {
	const vcs * pi = (const vcs *) input;
    vcs * po = (vcs *) output;
	rep_shift_right<N/4>(po, pi, 4);

	for (int i = 0; i < N/4; i++)
	{
		po[i] = smin(smax(po[i], gDemapMin), gDemapMax);
	}
}
