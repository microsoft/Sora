#pragma once

#include <math.h>
#include <tpltrick.h>
#include "ieee80211const.h"
#include "ieee80211facade.hpp"
#include "demapper.h"

DEFINE_LOCAL_CONTEXT(T11aDemap, CF_VOID);
template<ushort N_BPSC>
class T11aDemap
{
public:
template<TFILTER_ARGS>
class Filter : public TFilter<TFILTER_PARAMS>
{
static const int NbPC = N_BPSC;	// number of bits per subcarrier
static const int NbPS = NbPC * 48;  // number of bits per symbol

    A16 COMPLEX16 limited[64];
	FINL void demap ( const COMPLEX16 * input, uchar* output ) {
	    int i;
	    
	    for (i = 64 - 26; i < 64; i++)
	    {
	        if (i == 64 - 21 || i == 64 - 7) continue;
            DemapperCore::Demap<N_BPSC>(input[i], output);
	        output += N_BPSC;
	    }

	    for (i = 1; i <= 26; i++)
	    {
	        if (i == 7 || i == 21) continue;
            DemapperCore::Demap<N_BPSC>(input[i], output);
	        output += N_BPSC;
	    }
	}
	
public:
    DEFINE_IPORT(COMPLEX16, 64);
    DEFINE_OPORT(uchar,   NbPS); // each bit is one 8-bit soft-value

public:
    REFERENCE_LOCAL_CONTEXT(T11aDemap);

    STD_TFILTER_CONSTRUCTOR(Filter)
    { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const COMPLEX16 *input = ipin.peek();
            uchar*    output = opin().append();

			demap_limit<64>(input, limited);
            demap(limited, output);

#if 0
            //debug
            printf ( "demapper\n" );
            for (int i=0; i<48; i++ ) {
	            printf ( "%d ", output[i] );
            }
            printf ( "\n" );
#endif
            ipin.pop();

			Next()->Process(opin());
            
        }
        return true;
    }
}; };

typedef T11aDemap<1> T11aDemapBPSK;
typedef T11aDemap<2> T11aDemapQPSK;
typedef T11aDemap<4> T11aDemapQAM16;
typedef T11aDemap<6> T11aDemapQAM64;
