#pragma once
#include "tpltrick.h"
#include "ieee80211a_cmn.h"
#include "ieee80211facade.hpp"

#include "viterbicore.h"

DEFINE_LOCAL_CONTEXT(T11aViterbiSig, CF_Error);
template<TFILTER_ARGS>
class T11aViterbiSig : public TFilter<TFILTER_PARAMS>
{
static const int state_size = 64;
static const int input_size = 48; // always 48 soft-values

	CTX_VAR_RW (ulong, error_code );
protected:
    vub   trellis[state_size / 16 * input_size];

public:
    DEFINE_IPORT(uchar, 48);
    DEFINE_OPORT(uint,  1);

public:
    REFERENCE_LOCAL_CONTEXT(T11aViterbiSig);
	STD_TFILTER_CONSTRUCTOR(T11aViterbiSig)
		BIND_CONTEXT( CF_Error::error_code, error_code )
    {
    }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const uchar* input = ipin.peek();
            uint output = 0;

            // viterbi decoder, specially for dot11a signal symbol
            Viterbi_sig11 (trellis, (const char *)input, (char *)(&output));
			output >>= 6; // remove the prefix 6 zeros

            ipin.pop();
            *opin().append() = output;
			
            Next()->Process(opin());
        }

        return true;
    }
};

DEFINE_LOCAL_CONTEXT(T11nViterbiSig, CF_Error);
template<TFILTER_ARGS>
class T11nViterbiSig : public TFilter<TFILTER_PARAMS>
{
static const int state_size = 64;
static const int input_size = 48 * 2; // always 48 soft-values

	CTX_VAR_RW (ulong, error_code );
protected:
    vub   trellis[state_size / 16 * input_size];

public:
    DEFINE_IPORT(uchar, 48 * 3);
    DEFINE_OPORT(uchar,  3 * 3);

public:
    REFERENCE_LOCAL_CONTEXT(T11nViterbiSig);
	STD_TFILTER_CONSTRUCTOR(T11nViterbiSig)
		BIND_CONTEXT( CF_Error::error_code, error_code )
    {
    }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const uchar *input = ipin.peek();
            uchar *output = opin().append();

            // viterbi decoder, specially for dot11n L_SIG symbol
            uint lsig = 0;
            Viterbi_sig11 (trellis, (const char *)input, (char *)(&lsig));
			lsig >>= 6; // remove the prefix 6 zeros
            *(uint *)output = lsig;

            // viterbi decoder, specially for dot11n HT_SIG symbol
            unsigned __int64 htsig = 0;
            Viterbi_sig11 (trellis, (const char *)input + 48, (char *)(&htsig), 48);
            htsig >>= 6;
            memcpy(output + 3, (uchar *)&htsig, 6);

            ipin.pop();
			
            Next()->Process(opin());
        }

        return true;
    }
};

// Note:
//  N_INPUT - the input burst bytes. The implementation requires it be integral multiplier of 2/4/3 for there coding rates
DEFINE_LOCAL_CONTEXT(T11aViterbi, CF_11aRxVector, CF_Error);
template<size_t TRELLIS_MAX, size_t N_INPUT, size_t TRELLIS_DEPTH, size_t TRELLIS_LOOKAHEAD = 24>
class T11aViterbi {
public:
template<TFILTER_ARGS>
class Filter : public TFilter<TFILTER_PARAMS>
{
    static const int trellis_prefix 	= 6; 	 // 6 bit zero prefix
private:
	CTX_VAR_RO (ushort, frame_length );
	CTX_VAR_RO (ushort, code_rate );
	CTX_VAR_RW (ulong,  error_code );
	
protected:
	// output bit count
	ulong ob_count;
	TViterbiCore<TRELLIS_MAX> m_viterbi;

	uchar  m_outbuf [TRELLIS_DEPTH/8+1];

	FINL void __init () {
		ob_count = 0;
		m_viterbi.Reset ();
	}
	
public:
	// For 802.11a, the input should be one OFDM symbol worth of s-values
    DEFINE_IPORT(uchar, N_INPUT); 
	// Output decoded byte one-by-one
    DEFINE_OPORT(uchar, 1);

public:
    REFERENCE_LOCAL_CONTEXT(T11aViterbi);
	STD_TFILTER_CONSTRUCTOR(Filter)
		BIND_CONTEXT( CF_11aRxVector::frame_length, frame_length )		
		BIND_CONTEXT( CF_11aRxVector::code_rate, code_rate )		
		BIND_CONTEXT( CF_Error::error_code, error_code )
    {
    	__init ();
    }

	STD_TFILTER_RESET() {
		__init ();
	}

    BOOL_FUNC_PROCESS(ipin)
    {
        // vector128 constants
        static const vub * const pVITMA = (const vub*) VIT_MA; // Branch Metric A
        static const vub * const pVITMB = (const vub*) VIT_MB; // Branch Metric B

        while (ipin.check_read())
        {
			if ( error_code != E_ERROR_SUCCESS ) {
				// frame decode error
				// just clean the input and pending
				ipin.clear ();
				return true;
			}

            const uchar* input = ipin.peek();
            const uchar* input_end = input + N_INPUT;
			while ( input < input_end ) {
    			// advance branch
                if (code_rate == CR_12)
                {
                    assert(N_INPUT % 2 == 0);
    				m_viterbi.BranchACS (pVITMA, input[0], pVITMB, input[1]);
                    input += 2; // jump to data
                }
                else if (code_rate == CR_34)
                {
                    assert(N_INPUT % 4 == 0);
    				m_viterbi.BranchACS (pVITMA, input[0], pVITMB, input[1]);
    				m_viterbi.BranchACS (pVITMA, input[2]);
    				m_viterbi.BranchACS (pVITMB, input[3]);
                    input += 4;
                }
                else if (code_rate == CR_23)
                {
                    assert(N_INPUT % 3 == 0);
    				m_viterbi.BranchACS (pVITMA, input[0], pVITMB, input[1]);
    				m_viterbi.BranchACS (pVITMA, input[2]);
                    input += 3;
                }
				
				uint tr_index = m_viterbi.trellis_index ();

		        if ( (tr_index & 7) == 0 ) {
                //if (m_viterbi.m_pTrellis[0][0] > 190) {
			        m_viterbi.Normalize ();
		        }

				// check trace_back
				uint output_count = 0;
				uint lookahead    = 0;
				const uint tr_index_end = frame_length * 8 + 16 + trellis_prefix;
				if ( tr_index >= tr_index_end) {
					// all bytes have been processed - track back all of them
					output_count = tr_index_end - ob_count
                        - trellis_prefix;
					lookahead    = tr_index - tr_index_end;
				} else if ( tr_index >= ob_count + TRELLIS_DEPTH + TRELLIS_LOOKAHEAD + trellis_prefix)
				{
					// trace back partially

                    // Note: tr_index increase during 'BranchACS', to decode out complete bytes, we may lookahead the
                    // trellis without output a few more than 'TRELLIS_LOOKAHEAD'
                    uint remain = (tr_index - (ob_count + TRELLIS_DEPTH + TRELLIS_LOOKAHEAD + trellis_prefix)) % 8;
					output_count = TRELLIS_DEPTH;
					lookahead    = TRELLIS_LOOKAHEAD + remain;
				} 

				if ( output_count ) {
					m_viterbi.Traceback ( (char*)m_outbuf, output_count, lookahead );
					ob_count += output_count;

					uint ii = 0;
					output_count >>=3;
					while ( ii < output_count ) {				
						uchar* po = opin().append ();
						*po = m_outbuf[ii++];
						//printf ("%d (%x) \n", *po, *po );
						Next()->Process(opin());
					}
					//printf ( "\n" );
				}	
				
			}
            ipin.pop();
        }
        return true;
    }
};
};
