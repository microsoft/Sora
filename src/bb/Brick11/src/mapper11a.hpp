#pragma once

#include "func.h"
#include "tpltrick.h"
#include "ieee80211const.h"
#include "ieee80211facade.hpp"

static const short bpsk_mod_11a = 10720;
static const short qpsk_mod_11a = (short)(bpsk_mod_11a / 1.414); // divided by sqrt(2)
static const short qam16_mod_11a = (short)(bpsk_mod_11a / 3.162); // divided by sqrt(10)
static const short qam64_mod_11a = (short)(bpsk_mod_11a / 6.481); // divided by sqrt(42)

// Init 4/16/64QAM mapping table
// M        - half of N_BPSC (coded bits per subcarrier)
// LUT_BITS - log2 of the size of mapping table
template<int M, int LUT_BITS>
void InitQamMapLut(COMPLEX16 (&lut)[intpow<2, LUT_BITS>::value][LUT_BITS/M/2], short kmod)
{
    static const int MASK = intpow<2, M>::value - 1;
    static const int LUT_SIZE = intpow<2, LUT_BITS>::value;

	ushort i, x, g, rg, b;
    short l;
	for (i=0; i<LUT_SIZE; i++) {
		x = i;
		for (int k = 0; k < LUT_BITS/M/2; k++) {
            rg = x & MASK;
            g = (ushort)BitReverseN(rg, M);
            b = (ushort)GrayToBinary(g);
            l = b * 2 - MASK;
            lut[i][k].re = l * kmod;
            x >>= M;

            rg = x & MASK;
            g = (ushort)BitReverseN(rg, M);
            b = (ushort)GrayToBinary(g);
            l = b * 2 - MASK;
            lut[i][k].im = l * kmod;
            x >>= M;
		}
	}

	//for (i=0; i<LUT_SIZE; i++) {
	//	printf ( "lut %d ", i );
	//	for (int j=0; j<4; j++ ) {
	//		printf ( "<%d,%d> ", lut[i][j].re, lut[i][j].im );
	//	}
	//	printf ( "\n" );
	//}
}

template<short MOD = bpsk_mod_11a>
class TMapperCore
{
public:
    TMapperCore() { }
    FINL void MapBPSK(uchar c, COMPLEX16 out[8])
    {
        COMPLEX16 *po = m_lut_i[c];
        rep<2>::vmemcpy((vcs*)out, (vcs*)po);  
    }
    FINL void MapQBPSK(uchar c, COMPLEX16 out[8])
    {
        COMPLEX16 *po = m_lut_q[c];
        rep<2>::vmemcpy((vcs*)out, (vcs*)po);  
    }

private:
	// BPSK mapper table
	// 0 -> - mod
	// 1 ->   mod
    struct _init_lut_i
    {
        void operator()(COMPLEX16 (&lut)[256][8])
        {
		    ushort i, x;
		    for (i=0; i<256; i++) {
			    x = i;
			    for (int k=0; k<8; k++) {
					lut[i][k].re = (x & 0x01) ? MOD : - MOD;
				    lut[i][k].im = 0;
				    x >>= 1;
			    }
		    }
        }
	};
	// QBPSK mapper table
	// 0 -> - mod * i
	// 1 ->   mod * i
    struct _init_lut_q
    {
        void operator()(COMPLEX16 (&lut)[256][8])
        {
		    ushort i, x;
		    for (i=0; i<256; i++) {
			    x = i;
			    for (int k=0; k<8; k++) {
				    lut[i][k].re = 0;
					lut[i][k].im = (x & 0x01) ? MOD : - MOD;
				    x >>= 1;
			    }
		    }
        }
	};
    const static_wrapper<COMPLEX16 [256][8], _init_lut_i> m_lut_i;
    const static_wrapper<COMPLEX16 [256][8], _init_lut_q> m_lut_q;
};

DEFINE_LOCAL_CONTEXT(TMap11aBPSK, CF_VOID);
template<short MOD = bpsk_mod_11a>
class TMap11aBPSK
{
public:
template<TFILTER_ARGS>
class Filter : public TFilter<TFILTER_PARAMS>
{
public:
    DEFINE_IPORT(uchar, 1);		 
    DEFINE_OPORT(COMPLEX16, 8); 
	
public:
    REFERENCE_LOCAL_CONTEXT(Filter);
    STD_TFILTER_CONSTRUCTOR(Filter) { 
	}
	
    STD_TFILTER_RESET() { }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            uchar c = *ipin.peek();
            ipin.pop();
			
            COMPLEX16 * po = opin().append();

            mapper.MapBPSK(c, po);
            Next()->Process(opin());
        }
        return true;
    }

private:
    TMapperCore<MOD> mapper;
}; };

DEFINE_LOCAL_CONTEXT(TMap11aQPSK, CF_VOID);
template<short MOD = qpsk_mod_11a>
class TMap11aQPSK
{
public:
template<TFILTER_ARGS>
class Filter : public TFilter<TFILTER_PARAMS>
{
protected:

	// BPSK mapper table
	// LSB->MSB
	// 00 -> <- mod, -mod>
	// 01 -> <- mod, mod>
	// 10 -> < mod, -mod>
	// 11 -> < mod, mod>
    struct _init_lut
    {
        void operator()(COMPLEX16 (&lut)[256][4])
        {
    		InitQamMapLut<1, 8>(lut, MOD);
        }
    };
    const static_wrapper<COMPLEX16 [256][4], _init_lut> m_lut;

public:
    DEFINE_IPORT(uchar, 1);		 
    DEFINE_OPORT(COMPLEX16, 4); 
	
public:
    REFERENCE_LOCAL_CONTEXT(Filter);
    STD_TFILTER_CONSTRUCTOR(Filter) {
	}
	
    STD_TFILTER_RESET() { }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            uchar c = *ipin.peek();
            ipin.pop();
			
            vcs * po = (vcs*) opin().append();
			*po = m_lut[c];

            Next()->Process(opin());
        }
        return true;
    }
}; };

DEFINE_LOCAL_CONTEXT(TMap11aQAM16, CF_VOID);
template<short MOD = qam16_mod_11a>
class TMap11aQAM16
{
public:
template<TFILTER_ARGS>
class Filter : public TFilter<TFILTER_PARAMS>
{
protected:

    struct _init_lut
    {
        void operator()(COMPLEX16 (&lut)[256][2])
        {
    		InitQamMapLut<2, 8>(lut, MOD);
        }
    };
    const static_wrapper<COMPLEX16 [256][2], _init_lut> m_lut;

public:
    DEFINE_IPORT(uchar, 1);		 
    DEFINE_OPORT(COMPLEX16, 2); 
	
public:
    REFERENCE_LOCAL_CONTEXT(Filter);
    STD_TFILTER_CONSTRUCTOR(Filter) {
	}
	
    STD_TFILTER_RESET() { }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            uchar c = *ipin.peek();
            ipin.pop();
			
            COMPLEX16 * po = opin().append();
			po[0] = m_lut[c][0];
            po[1] = m_lut[c][1];

            Next()->Process(opin());
        }
        return true;
    }
}; };

DEFINE_LOCAL_CONTEXT(TMap11aQAM64, CF_VOID);
template<short MOD = qam64_mod_11a>
class TMap11aQAM64
{
public:
template<TFILTER_ARGS>
class Filter : public TFilter<TFILTER_PARAMS>
{
public:
protected:
    struct _init_lut
    {
        void operator()(COMPLEX16 (&lut)[4096][2])
        {
    		InitQamMapLut<3, 12>(lut, MOD);
        }
    };
    const static_wrapper<COMPLEX16 [4096][2], _init_lut> m_lut;

public:
    DEFINE_IPORT(uchar, 3);		 
    DEFINE_OPORT(COMPLEX16, 4); 
	
public:
    REFERENCE_LOCAL_CONTEXT(Filter);
    STD_TFILTER_CONSTRUCTOR(Filter) {
	}
	
    STD_TFILTER_RESET() { }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const uchar *c = ipin.peek();
			const ushort c0 = c[0] | ((c[1] & 0xF) << 8);
            const ushort c1 = (c[1] >> 4) | (c[2] << 4);
            ipin.pop();

            COMPLEX16 *po = opin().append();
			po[0] = m_lut[c0][0];
			po[1] = m_lut[c0][1];
			po[2] = m_lut[c1][0];
			po[3] = m_lut[c1][1];
            Next()->Process(opin());
        }
        return true;
    }
}; };
