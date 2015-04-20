#pragma once

#include "ieee80211facade.hpp"
#include <dot11_plcp.h>

//
// Signal Field Deliminator Synchronizer
// 

DEFINE_LOCAL_CONTEXT(TSFDSync, CF_DifferentialDemap, CF_11bRxMRSel, CF_Descramber, CF_Error);
template<TSINK_ARGS>
class TSFDSync : public TSink<TSINK_PARAMS>
{
// hard coded in this version
static const ushort sfd = DOT11B_PLCP_LONG_PREAMBLE_SFD;
static const uint   max_syn_bit = 128+16;
protected:
	// binding variables
	CTX_VAR_RW ( CF_11bRxMRSel::RxRateState, rxrate_state );
	CTX_VAR_RW ( ulong, error_code );
	CTX_VAR_RW ( COMPLEX16, last_symbol );
	CTX_VAR_RW ( UCHAR, byte_reg );

protected:
	bool bit_one_found;
	ushort word;
	int    bit_err_cnt;
	uint   sync_cnt;

	FINL void _init () {
		bit_one_found = false;
		word     	  = 0;
		bit_err_cnt   = 0;
		sync_cnt = 0;
	}

    struct _init_lut
    {
        void operator()(uchar (&lut)[2][128])
        {
		    int i,j;
		    uchar x, s;
		
		    for ( i=0; i<2; i++) {
			    for ( j=0; j<128; j++) {
				    x = (uchar)i;
				    s = (uchar)j;
				    lut [i][j] = (x ^ (s) ^ (s >> 3)) & 0x01;
			    }
		    }
        }
    };
    // bit-wise descrambler lut
    const static_wrapper<uchar [2][128], _init_lut> _descramble_lut;
	
public:
	// inport and outport
	DEFINE_IPORT (COMPLEX16, 1);

public:
    REFERENCE_LOCAL_CONTEXT(TSFDSync);
    STD_TSINK_CONSTRUCTOR(TSFDSync)
        BIND_CONTEXT(CF_11bRxMRSel::rxrate_state, rxrate_state )
        BIND_CONTEXT(CF_DifferentialDemap::last_symbol, last_symbol )
        BIND_CONTEXT(CF_Descramber::byte_reg, byte_reg )
        BIND_CONTEXT(CF_Error::error_code, error_code )
    { 
		_init (); 
	}
	
	STD_TSINK_RESET() {
		_init ();
	}
    STD_TSINK_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
        	const COMPLEX16& s = *ipin.peek();
			ushort bit = demap_dbpsk_bit (last_symbol, s );
			last_symbol = s;
			
			// descramble
			byte_reg &= 0x7f;
			ushort sbit = _descramble_lut[bit&0x01][byte_reg];
			byte_reg = (byte_reg >> 1) | (bit<<6);

			word = (word >> 1 ) | (sbit << 15); 
//			printf ( "sfd_sync bit %d sbit %d word %4x\n", bit, sbit, word );

			sync_cnt ++;

			if ( !bit_one_found ) {
				if ( word == 0xFFFF ) {
					bit_one_found = true; 
				}	
			} else {
				if ( word == sfd ) {
					// SFD find
					// move to plcp
					rxrate_state = CF_11bRxMRSel::rate_1m;

				} else	if ( word != 0xFFFF ) {
					if ( bit_err_cnt++ > 32 ) {
						// Error 
						error_code = E_ERROR_SFD_FAIL;
						ipin.clear();

						return false;
					}	
				} 
			}

			if ( sync_cnt > max_syn_bit ) {
				error_code = E_ERROR_SFD_TIMEOUT;
				ipin.clear();
				return false;
			}
			
			ipin.pop();

			
        }
        return true;
    }

	// demap the symbol and set to the highest bit
	FINL ushort demap_dbpsk_bit (COMPLEX16 & ref, const COMPLEX16 & s )
	{
		return (ushort) ((ulong)(ref.re * s.re + ref.im * s.im) >> 31);
	}
};

