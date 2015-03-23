#pragma once

#include "tpltrick.h"
#include "ieee80211a_cmn.h"
#include "ieee80211facade.hpp"
#include "operator_repeater.h"

//
// Generate interleave template for 802.11
// N_BPSC - bits per subcarrier
// N_CBPS - coded bits per symbol
// N_COL  - interleaver columns
// N_ROT  - interleaver rotion parameter
// I_SS   - the index of the spatial stream on which this interleaver is operating (1, 2, ...)
//
DEFINE_LOCAL_CONTEXT(T11Interleave, CF_VOID);
template<ushort N_CBPS, ushort N_BPSC, ushort N_COL = 16, ushort N_ROT = 11, ushort I_SS = 1>
class T11Interleave {
public:
template<TFILTER_ARGS>
class Filter : public TFilter<TFILTER_PARAMS>
{
private:
	static const ushort entry_bits = 32; 	// ulong is 32 bits
    static const ushort N_S    = max (N_BPSC / 2, 1);
	static const ushort N_b    = ceiling_div<N_CBPS, 8>::value;
	static const ushort N_seg  = ceiling_div<N_CBPS, entry_bits>::value;
protected:
    struct LutInitHelper
    {
        void operator()(ulong (&lut)[N_b][256][N_seg])
        {
	        ushort kbyte, vbyte;
	        ulong vbyte1;

		    memset ( lut, 0, sizeof(lut));
	        for ( kbyte = 0; kbyte < N_b; kbyte++) {
		        for ( vbyte = 0; vbyte < 256; vbyte++) {
			        vbyte1 = vbyte;
			        for (int kbit = 0; kbit < 8; kbit++ ) {
				        int k = kbyte * 8 + kbit;

				        // first permutation
				        int i = N_CBPS / N_COL * (k % N_COL) + k / N_COL;
				
				        // second permutation
				        int j = N_S * (i / N_S) + (i + N_CBPS - N_COL * i / N_CBPS) % N_S;

                        // third permutation
                        int r = (N_CBPS + j - ( ( (I_SS - 1) * 2 ) % 3 + 3 * ((I_SS - 1) / 3) ) * N_ROT * N_BPSC) % N_CBPS;

				        ushort rentry  = r / 32;
				        ushort rbit = r % 32;
				
				        lut[kbyte][vbyte][rentry] |= (vbyte1 & 1) << rbit;
				        vbyte1 >>= 1;
			        }
		        }
	        }
        }
    };
    const static_wrapper<ulong [N_b][256][N_seg], LutInitHelper> m_lut;

public:
	DEFINE_IPORT(uchar, N_b);
    DEFINE_OPORT(uchar, N_b);
	
public:
    REFERENCE_LOCAL_CONTEXT(T11Interleave);
    
    STD_TFILTER_CONSTRUCTOR(Filter) { }
    STD_TFILTER_RESET() { }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const uchar *c = ipin.peek();
            ulong *o = (ulong*) opin().append();

			// perform interleaving
            B11aInterleave(c, o);

            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }

    FINL void B11aInterleave (const uchar * pbInput, ulong * pbOutput)
    {
	    ulong j;
	    rep_memcpy<N_seg>(pbOutput, m_lut[0][pbInput[0]]);

	    for (j = 1; j < N_b; j++)
	    {
            rep_or<N_seg>(pbOutput, pbOutput, m_lut[j][pbInput[j]]);
	    }

        //printf ( "Interleave \n" );
	    //for ( j=0; j<N_b; j++ )
	    //{
	    //	printf ( "%2x -> %2x\n", pbInput[j], ((uchar*) pbOutput)[j] );
	    //}
	    //printf ( "\n" );
    }
}; };


typedef T11Interleave<48  ,1,16,11,1> T11aInterleaveBPSK;
typedef T11Interleave<48*2,2,16,11,1> T11aInterleaveQPSK;
typedef T11Interleave<48*4,4,16,11,1> T11aInterleaveQAM16;
typedef T11Interleave<48*6,6,16,11,1> T11aInterleaveQAM64;

typedef T11Interleave<52,  1,13,11,1> T11nInterleaveBPSK_S1;
typedef T11Interleave<52,  1,13,11,2> T11nInterleaveBPSK_S2;
typedef T11Interleave<52*2,2,13,11,1> T11nInterleaveQPSK_S1;
typedef T11Interleave<52*2,2,13,11,2> T11nInterleaveQPSK_S2;
typedef T11Interleave<52*4,4,13,11,1> T11nInterleaveQAM16_S1;
typedef T11Interleave<52*4,4,13,11,2> T11nInterleaveQAM16_S2;
typedef T11Interleave<52*6,6,13,11,1> T11nInterleaveQAM64_S1;
typedef T11Interleave<52*6,6,13,11,2> T11nInterleaveQAM64_S2;
