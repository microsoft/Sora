#pragma once

#include "const.h"
#include "dspcomm.h"
#include "operator_repeater.h"

SELECTANY extern const int Barker11[] = { 1, -1, 1, 1, -1, 1, 1, 1, -1, -1, -1 };

DEFINE_LOCAL_CONTEXT(TBB11bDBPSKSpread, CF_DifferentialMap);
template<TFILTER_ARGS>
class TBB11bDBPSKSpread : public TFilter<TFILTER_PARAMS>
{
	// normalize to 1 and -1, let pulse shaper decide the scale
	static const char ONE  =  1; 
	static const char MONE = -1;

private:
	// context binding
	CTX_VAR_RW (uint, m_reg);

private:
    struct _init_coding_lut
    {
        void operator()(uchar (&lut)[256][2])
        {
		    // coding lut
		    int i,k;

		    // encoder table		
		    for ( i=0; i<256; i++ ) {
			    uchar x = (uchar) i;
			
			    uchar phase = 0;
			    lut[i][0] = 0;
			    for ( k=0; k<8; k++) {
				    phase = (x & 0x01) ? (phase ^ 0x01):phase;				
				    lut[i][0] |= (phase << k);
				    x >>= 1;
			    }
			
			    lut[i][1] = lut[i][0] ^ 0xFF;
		    }
        }
    };

    struct _init_spreading_lut
    {
        void operator()(COMPLEX8 (&lut)[256][88])
        {
		    // coding lut
		    int i,j,k;

		    // spreading lut
		    // 0 - 0 rad
		    // 1 - pi rad
		    for ( i = 0; i<256; i++) {
			    uchar x = (uchar) i;
			    for ( j=0; j<8; j++ ) {
				    int c = (x & 0x01)?-1:1; 
				    x >>= 1;
				    for (k=0; k<11; k++) {
					    lut[i][j*11+k].re = c*Barker11[k]*ONE;
					    lut[i][j*11+k].im = 0;
				    }
			    }
		    }
        }
    };

	// the spreading lut spreads 8 bit
    const static_wrapper<A16 uchar [256][ 2], _init_coding_lut> m_Coding_LUT;
    const static_wrapper<A16 COMPLEX8 [256][88], _init_spreading_lut> m_Spreading_LUT;
	
public:
	// inport
	DEFINE_IPORT(uchar, 1);
	DEFINE_OPORT(COMPLEX8, 88);
	
public:
    REFERENCE_LOCAL_CONTEXT(TBB11bDBPSKSpread);
    
    STD_TFILTER_CONSTRUCTOR(TBB11bDBPSKSpread)
		BIND_CONTEXT(CF_DifferentialMap::last_phase, m_reg)
    {
    }
	STD_TFILTER_RESET() {}
	STD_TFILTER_FLUSH() {}

    BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
            uchar b = *ipin.peek();
            ipin.pop();

			// m_reg must be 00 or 10;
			uchar code = m_Coding_LUT[b][m_reg & 0x01];

            m_reg = code >> 7;
            m_reg = (m_reg << 1) | m_reg; // compatible to dqpsk
            
			// spreaded waveform
			vcb * pSpreadSig = (vcb*) &m_Spreading_LUT[code][0];
			vcb * pbuf = (vcb*) opin().append ();

			rep_memcpy<11>(pbuf, pSpreadSig );
            Next()->Process(opin());
		}
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(TBB11bDQPSKSpread, CF_DifferentialMap);
template<TFILTER_ARGS>
class TBB11bDQPSKSpread : public TFilter<TFILTER_PARAMS>
{
	// normalize to 1 and -1, let pulse shaper decide the scale
	static const char ONE  =  1; 
	static const char MONE = -1;

private:
	// context binding
	CTX_VAR_RW (uint, m_reg);

private:
    struct _init_coding_lut
    {
        void operator()(uchar (&lut)[256][4])
        {
		    // coding lut
		    int i,j,k;
		    // DQPSK encode
		    // 00 - 0 rad
		    // 01 - - pi / 2 rad
		    // 10 - pi / 2 rad
		    // 11 - pi rad
		    const uchar rotate[4][4] = { {0, 1, 2, 3}, {1, 3, 0, 2}, {2, 0, 3, 1}, {3, 2, 1, 0} }; 

		    // encoder table		
		    uchar phase = 0;

		    for ( i=0; i<256; i++ ) {
			    for ( j = 0; j<4; j++ ) {
				    uchar x = (uchar) i;
				    lut[i][j] = 0;
				
				    phase = (uchar) j; // initial phase
				    for ( k=0; k<8; k+=2) {
					    phase = rotate[phase][x & 0x3];
					    lut[i][j] |= (phase << (k));
					    x = x>>2;
				    }
    		    }
		    }
        }
    };

    struct _init_spreading_lut
    {
        void operator()(COMPLEX8 (&lut)[256][44])
        {
		    // spreading lut
		    int i,j,k;
		    for ( i = 0; i<256; i++) {
			    uchar x = (uchar) i;
			    for ( j=0; j<4; j++ ) {
				    char re, im;
				
				    switch (x & 0x03) {
				    case 0: re = ONE;    im = 0;    break;
				    case 1: re = 0;      im = MONE; break;
				    case 2: re = 0;      im = ONE;  break;
				    case 3: re = MONE;   im = 0;    break;
                    default:
                        NODEFAULT;
				    }
				
				    x >>= 2;
				    for (k=0; k<11; k++) {
					    lut[i][j*11+k].re = re*Barker11[k];
					    lut[i][j*11+k].im = im*Barker11[k];
				    }
			    }
		    }		    
        }
    };

	// the spreading lut spreads 8 bit
    const static_wrapper<A16 uchar [256][ 4], _init_coding_lut> m_Coding_LUT;
    const static_wrapper<A16 COMPLEX8 [256][44], _init_spreading_lut> m_Spreading_LUT;
	
public:
	// inport
	DEFINE_IPORT(uchar, 1);
	DEFINE_OPORT(COMPLEX8, 44);
	
public:
    REFERENCE_LOCAL_CONTEXT(TBB11bDQPSKSpread);
    
    STD_TFILTER_CONSTRUCTOR(TBB11bDQPSKSpread)
		BIND_CONTEXT(CF_DifferentialMap::last_phase, m_reg)
    {
    }
	STD_TFILTER_RESET() {}
	STD_TFILTER_FLUSH() {}
	
    BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
            uchar b = *ipin.peek();
            ipin.pop();

			uchar code = m_Coding_LUT[b][m_reg & 0x03];
			m_reg = code >> 6;

			// spreaded waveform
			COMPLEX8 * pSig = &m_Spreading_LUT[code][0];
			COMPLEX8 * pOut = opin().append();
			rep_memcpy<44>(pOut, pSig );
            Next()->Process(opin());
		}
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(TBB11bDespread, CF_VOID);
template<TFILTER_ARGS>
class TBB11bDespread : public TFilter<TFILTER_PARAMS>
{
public: // inport and outport
    DEFINE_IPORT(COMPLEX16, 11);
    DEFINE_OPORT(COMPLEX16, 1);

public:
    REFERENCE_LOCAL_CONTEXT(TBB11bDespread);
    STD_TFILTER_CONSTRUCTOR(TBB11bDespread) {}
	STD_TFILTER_RESET() {}
	STD_TFILTER_FLUSH() {}

    BOOL_FUNC_PROCESS(ipin)
    {
        COMPLEX16 output;
        while (ipin.check_read())
        {
            const COMPLEX16 *input = ipin.peek();
			QuickBarkerDespread (input, output );
			// 	BarkerDespread (input, output );
            ipin.pop();
			
            // Push to output pin
            *opin().append() = output;
            Next()->Process(opin());
        }
        return true;
    }
public:

	FINL void BarkerDespread ( COMPLEX16 * pInput, COMPLEX16& res ) {
		// simply add-up
		COMPLEX32 sum;
		sum.re = pInput[0].re; sum.im = pInput[0].im;
		
		sum -= pInput[1]; sum += pInput[2];
		sum += pInput[3]; sum -= pInput[4]; sum += pInput[5];
		sum += pInput[6]; sum += pInput[7]; sum -= pInput[8];
		sum -= pInput[9]; sum -= pInput[10];

		res.re = (short) (sum.re >> 4 );
		res.im = (short) (sum.im >> 4 );		
//		printf ( "despread ::=> %d, %d\n", res.re, res.im );

	}
	
	// Barker11[] = { 1, -1, 1, 1, -- -1, 1, 1, 1, -- -1, -1, -1 };
	FINL void QuickBarkerDespread ( const COMPLEX16 * pInput, COMPLEX16& res ) {
		// simply add-up
 static const vcs::data_type sign0 = {{0,0} , {-1,-1}, {0,0}, {0,0} };
 static const vcs::data_type sign1 = {{-1,-1} , {0,0}, {0,0}, {0,0} };
		
		vcs sum, v;
		load (sum, pInput); 
		sum = sub (xor(sum, (vcs)sign0), (vcs)sign0);
		sum = shift_right (sum, 4);
		
		load (v, &pInput[4] );
		v = sub (xor(v, (vcs)sign1), (vcs)sign1);

		v = shift_right(v, 4);
		
		sum = add (sum, v); 
		
		load (v, &pInput[7]);
		v = shift_element_right<vcs,1>(v);
		v = shift_right (v, 4);
		sum = sub (sum, v);

		sum = hadd (sum);
		
		res = sum[0];
		
//		printf ( "despread0 ::=> %d, %d => %I64d\n", res.re, res.im, t1-t0 );
		
	}

};

/**************************************************
	TDBPSKDemap
**************************************************/

DEFINE_LOCAL_CONTEXT(TDBPSKDemap, CF_DifferentialDemap);
template<TFILTER_ARGS>
class TDBPSKDemap : public TFilter<TFILTER_PARAMS>
{
private:
	// context binding
	CTX_VAR_RW (COMPLEX16, last_symbol );

public:
	// inport and outport
    DEFINE_IPORT(COMPLEX16, 8);
    DEFINE_OPORT(UCHAR, 1);
		
public:
    REFERENCE_LOCAL_CONTEXT(TDBPSKDemap);

    STD_TFILTER_CONSTRUCTOR(TDBPSKDemap)
        BIND_CONTEXT(CF_DifferentialDemap::last_symbol, last_symbol)
    {}
	STD_TFILTER_RESET() {}
	STD_TFILTER_FLUSH() {}

    BOOL_FUNC_PROCESS (ipin)
    {
    // Marker::DBPSK
        while(ipin.check_read())
        {
            const COMPLEX16 *input = ipin.peek();
//	        UCHAR output = QuickDemapDBPSK (input, last_symbol );

			UCHAR output = DemapDBPSK (input, last_symbol );


            ipin.pop();
            *opin().append() = output;
            Next()->Process(opin());
        }
        return true;
    }
	
public:
	FINL void demap_dbpsk_bit (UCHAR& r, int pos, const COMPLEX16 & ref, const COMPLEX16 & s )
	{
		r |= (UCHAR) ((ulong)(ref.re * s.re + ref.im * s.im) >> 31) << pos;
	}
	
	FINL UCHAR DemapDBPSK (const COMPLEX16 *input, COMPLEX16& ref)
	{
    	UCHAR ret = 0;
	    demap_dbpsk_bit(ret, 0,   (ref), input[0]);
	    demap_dbpsk_bit(ret, 1, input[0], input[1]);
	    demap_dbpsk_bit(ret, 2, input[1], input[2]);
	    demap_dbpsk_bit(ret, 3, input[2], input[3]);
	    demap_dbpsk_bit(ret, 4, input[3], input[4]);
	    demap_dbpsk_bit(ret, 5, input[4], input[5]);
	    demap_dbpsk_bit(ret, 6, input[5], input[6]);
	    demap_dbpsk_bit(ret, 7, input[6], input[7]);
	    ref = input[7];
	    return ret;
	}
	
	FINL UCHAR QuickDemapDBPSK (COMPLEX16 *input, COMPLEX16& ref)
	{
		vcs* pi = (vcs*) input;
		vcs vref = shift_element_left<vcs,1> (*pi);
		vref = insert<0> ( vref, ref);

		vi r  = pairwise_muladd ((vs) pi[0],(vs) vref);
	
		load (vref, input+3); 
		vi r1 = pairwise_muladd ((vs) pi[1], (vs) vref);

		int ret = move_mask ( r, r1 );

	    ref = input[7];
	    return (uchar)(ret&0x00FF);
	}

};

/**************************************************
	TDQPSKDemap
**************************************************/

DEFINE_LOCAL_CONTEXT(TDQPSKDemap, CF_DifferentialDemap);
template<TFILTER_ARGS>
class TDQPSKDemap : public TFilter<TFILTER_PARAMS>
{
private:
	// context binding
	CTX_VAR_RW (COMPLEX16, last_symbol );

public:
	// inport and outport
    DEFINE_IPORT(COMPLEX16, 4);
    DEFINE_OPORT(UCHAR, 1);
		
public:
    REFERENCE_LOCAL_CONTEXT(TDQPSKDemap);

    STD_TFILTER_CONSTRUCTOR(TDQPSKDemap)
        BIND_CONTEXT(CF_DifferentialDemap::last_symbol, last_symbol)
    {}
	STD_TFILTER_RESET() {}
	STD_TFILTER_FLUSH() {}

    BOOL_FUNC_PROCESS (ipin)
    {
        while(ipin.check_read())
        {
            const COMPLEX16 *input = ipin.peek();
            UCHAR output = DemapDQPSK (input, last_symbol );
            ipin.pop();
            *opin().append() = output;
            Next()->Process(opin());
        }
        return true;
    }

public:
	FINL void demap_dqpsk_bits (UCHAR& r, int pos, const COMPLEX16 & ref, const COMPLEX16 & s )
	{
		long re = (ref.re*s.re + ref.im * s.im );
		long im = (ref.re*s.im - ref.im * s.re );
		
		r |= (UCHAR) ((ulong)(re+im) >> 31) << pos;
		r |= (UCHAR) ((ulong)(re-im) >> 31) << (pos+1);
	}
	
	FINL UCHAR DemapDQPSK (const COMPLEX16 *input, COMPLEX16& ref)
	{
    	UCHAR ret = 0;
	    demap_dqpsk_bits(ret, 0,   (ref),  input[0]);
	    demap_dqpsk_bits(ret, 2, input[0], input[1]);
	    demap_dqpsk_bits(ret, 4, input[1], input[2]);
	    demap_dqpsk_bits(ret, 6, input[2], input[3]);
	    ref = input[3];
	    return ret;
	}
};



