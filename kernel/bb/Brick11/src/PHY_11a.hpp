#pragma once
#include <dspcomm.h>

#include "ieee80211facade.hpp"
#include "ieee80211a_cmn.h"
#include "thread_if.h"

//
//  11a Src -> Scramble -> Encode -> Interleave -> Mapper -> AddPilot -> IFFT -> AddGI -> AddTS -> TModSink
//
DEFINE_LOCAL_CONTEXT(TBB11aSrc, CF_11aTxVector, CF_TxFrameBuffer, CF_Error, CF_ScramblerControl );
template<TSOURCE_ARGS>
class TBB11aSrc : public TSource<TSOURCE_PARAMS>
{
public:
static const ulong plcp_signal_size = 3;
static const ulong service_size     = 2;
static const ulong fcs_size         = 4;
static const ulong tail_size        = 1;

private:
	// Context binding
	// CF_11aTxVector
	CTX_VAR_RO (ushort, frame_length );    // payload size
	CTX_VAR_RW (ulong,  data_rate_kbps );  // data rate in kbps		
	CTX_VAR_RO (ulong,  crc32 );	       // update CRC32 in the brick 
	
	CTX_VAR_RW (ulong, scramble_ctrl );
	
	// CF_TxFrameBuffer
	CTX_VAR_RO (uchar*, mpdu_buf0 );	  	
	CTX_VAR_RO (uchar*, mpdu_buf1 );	  	
	CTX_VAR_RO (ushort, mpdu_buf_size0 );	  	
	CTX_VAR_RO (ushort, mpdu_buf_size1 );	  	

	// CF_Error
	CTX_VAR_RW (ulong, error_code );     /* error code for the modulation  */	
	
protected:	
	// local states
	ulong  plcp_signal; // the plcp signal head of the frame
	uchar  service[service_size]; 	
	
	void _init () {
		plcp_signal = 0;
		memset ( service, 0, sizeof(service));

		Preprocess (); // check all parameters and prepare the headers
	}

public:
	// outport
	DEFINE_OPORT (UCHAR, 1);

public:
	// Brick interface
    REFERENCE_LOCAL_CONTEXT(TBB11aSrc);

    STD_TSOURCE_CONSTRUCTOR(TBB11aSrc)
		BIND_CONTEXT (CF_11aTxVector::frame_length,    frame_length )
		BIND_CONTEXT (CF_11aTxVector::data_rate_kbps,  data_rate_kbps )
		BIND_CONTEXT (CF_11aTxVector::crc32,		   crc32 ) 	
		// CF_TxFrameBuffer
		BIND_CONTEXT (CF_TxFrameBuffer::mpdu_buf0, 	    mpdu_buf0 )
		BIND_CONTEXT (CF_TxFrameBuffer::mpdu_buf1, 	    mpdu_buf1 )
		BIND_CONTEXT (CF_TxFrameBuffer::mpdu_buf_size0, mpdu_buf_size0 )
		BIND_CONTEXT (CF_TxFrameBuffer::mpdu_buf_size1, mpdu_buf_size1 )		
		// CF_Error
		BIND_CONTEXT (CF_Error::error_code,	   error_code ) 
		// CF_ScramblerControl
		BIND_CONTEXT (CF_ScramblerControl::scramble_ctrl, scramble_ctrl )
    {
    	_init ();
    }
	STD_TSOURCE_RESET() {
		_init ();
	}
	STD_TSOURCE_FLUSH() { }
	
public:		

	FINL
	bool PreparePLCPSignal () {
		// fill PLCP Header	
		uchar rate_code = B11aGetCodeFromKbps (data_rate_kbps);
		if ( rate_code == 0 ) return FALSE;
		ushort frame_length_plus_fcs = frame_length + 4;
		plcp_signal = B11aGetPLCPSignal (rate_code, frame_length_plus_fcs );

		return true;		
	}

	bool Preprocess () {
		error_code = E_ERROR_SUCCESS;
		
		if ( mpdu_buf0 == NULL 
			 || (mpdu_buf_size1 > 0 && mpdu_buf1 == NULL ) 
			 || (mpdu_buf_size0 + mpdu_buf_size1 != frame_length )) {
			error_code = E_ERROR_PARAMETER;
			return false; // done
		}	

		if ( !PreparePLCPSignal () ) {
			error_code = E_ERROR_PARAMETER;
			return false;
		}

		return true;
	}

	ulong GetPadingByte (ulong rate) 
	{
		//
		// NB: the standard states the padding bits are at least 6.
		//     in practical, it is always a multiple of 8 (using byte)
		//

		int Ndbps = B11aGetNDBPS (rate);

        // Note: Ndbps for rate 9000 is not integral multiple of 8 bits (1 byte)
        // If we padding economical, the convolutional encoder is not easy to write.
        // So we pad more to make life easy.
        if (rate == 9000) Ndbps *= 2;

		// Calculate the number of appended zeroes to satisfy DBPS in one OFDM symbol
		ulong dbytes   = service_size + (frame_length + fcs_size) + tail_size; 
		ulong pad_bits = ceil_pad(dbytes * 8, Ndbps);
		ulong pad      = ceil_div(pad_bits, 8);
		return pad;
	}
	
    bool Process ()
    {
		// Drive the encoding pipeline

		// save the data rate
		ulong rate = data_rate_kbps;
		
		uchar * pc0 = (PUCHAR)&plcp_signal; 
		uchar * pc1 = service;
		uchar * pc2 = mpdu_buf0;
		uchar * pc3 = mpdu_buf1;
		uchar * pc4 = (uchar*) &crc32;

		ulong Npad = GetPadingByte ( data_rate_kbps );
		
		//printf ( "frame_len %d, Npad %d\n" , frame_length, Npad );
		//printf ( "crc32 %08x\n", crc32 );
		
		int count = 0;
		
		int totalbytes  = plcp_signal_size + service_size + frame_length + fcs_size ;
		int tailedbytes = totalbytes + tail_size; 
		int paddedbytes = tailedbytes + Npad;
		
		int block0_size, block1_size;
		block0_size = plcp_signal_size + service_size + mpdu_buf_size0;
		block1_size = block0_size + mpdu_buf_size1;

		// plcp signal does not scrambled
		// always modulated with 6Mbps
		scramble_ctrl = CF_ScramblerControl::NO_SCRAMBLE;
		data_rate_kbps = 6000; // plcp_signal
		while (count < plcp_signal_size ) {
	        *opin().append() = *pc0; pc0 ++;	
			count ++;
			Next()->Process(opin());
		}


		// scramble all data payload
		// restore the specified rate
		scramble_ctrl = CF_ScramblerControl::DO_SCRAMBLE;
		data_rate_kbps = rate; // plcp_signal
		
		while (count < paddedbytes ) {
			if ( count < plcp_signal_size + service_size ) {
				*opin().append() = *pc1; pc1 ++;	// service bits
			} else if ( count < block0_size ) {
			    *opin().append() = *pc2; pc2 ++;	// data block 0 
			} else if ( count < block1_size ) {
			    *opin().append() = *pc3; pc3 ++;	// data block 1 
			} else if ( count < totalbytes ) {
			    *opin().append() = *pc4; pc4 ++; 	// CRC32
			} else if ( count < tailedbytes ) {
				// tail
				scramble_ctrl = CF_ScramblerControl::TAIL_SCRAMBLE;
        		*opin().append() = 0;

			} else {
				// padding
				scramble_ctrl = CF_ScramblerControl::DO_SCRAMBLE;
				*opin().append() = 0;
			}
			
			count ++;
			Next()->Process(opin());
		}
		
    	Next()->Flush();
        return false;
    }
};

/*************************************************
TBB11aMRSelect - modulation multi-rate selector
*************************************************/
DEFINE_LOCAL_CONTEXT(TBB11aMRSelect, CF_11aTxVector);
template<TDEMUX8_ARGS>
class TBB11aMRSelect : public TDemux<TDEMUX8_PARAMS>
{
	CTX_VAR_RO(ulong, data_rate_kbps);
	
public:
	DEFINE_IPORT (uchar, 1);
    DEFINE_OPORTS(0, uchar, 1);
    DEFINE_OPORTS(1, uchar, 1);
    DEFINE_OPORTS(2, uchar, 1);
    DEFINE_OPORTS(3, uchar, 1);
    DEFINE_OPORTS(4, uchar, 1);
    DEFINE_OPORTS(5, uchar, 1);
    DEFINE_OPORTS(6, uchar, 1);
    DEFINE_OPORTS(7, uchar, 1);

public:
    REFERENCE_LOCAL_CONTEXT(TBB11aMRSelect);
    	
    STD_DEMUX8_CONSTRUCTOR(TBB11aMRSelect)
        BIND_CONTEXT(CF_11aTxVector::data_rate_kbps, data_rate_kbps)
    { }

    STD_TDEMUX8_RESET()
    { }

    FINL void Flush()
  {
    if ( data_rate_kbps == 6000) {
      FlushPort(0);
    }
    else if (data_rate_kbps == 9000) {
      FlushPort(1);
    }
    else if (data_rate_kbps == 12000) {
      FlushPort(2);
    }
    else if (data_rate_kbps == 18000) {
      FlushPort(3);
    }
    else if (data_rate_kbps == 24000) {
      FlushPort(4);
    }
    else if (data_rate_kbps == 36000) {
      FlushPort(5);
    }
    else if (data_rate_kbps == 48000) {
      FlushPort(6);
    }
    else if (data_rate_kbps == 54000) {
      FlushPort(7);
    }
  }

	BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
			uchar b = *ipin.peek();
			ipin.pop();
			if ( data_rate_kbps == 6000) {
				*opin0().append() = b;
				Next0()->Process (opin0());
            }
            else if (data_rate_kbps == 9000) {
				*opin1().append() = b;
				Next1()->Process (opin1());
            }
            else if (data_rate_kbps == 12000) {
				*opin2().append() = b;
				Next2()->Process (opin2());
			}
            else if (data_rate_kbps == 18000) {
				*opin3().append() = b;
				Next3()->Process (opin3());
            }
            else if (data_rate_kbps == 24000) {
				*opin4().append() = b;
				Next4()->Process (opin4());
            }
            else if (data_rate_kbps == 36000) {
				*opin5().append() = b;
				Next5()->Process (opin5());
            }
            else if (data_rate_kbps == 48000) {
				*opin6().append() = b;
				Next6()->Process (opin6());
            }
            else if (data_rate_kbps == 54000) {
				*opin7().append() = b;
				Next7()->Process (opin7());
            }
        }
        return true;
    }
};

#if 0
/*****************************************************
TBB11aSymSel - Training symbol / OFDM symbol selector
******************************************************/
DEFINE_LOCAL_CONTEXT(TBB11aSymSel, CF_11aSymState);
template<TDEMUX2_ARGS>
class TBB11aSymSel : public TDemux<TDEMUX2_PARAMS>
{
private:
	CTX_VAR_RO(ulong, symbol_type);
	
public:
	DEFINE_IPORT (COMPLEX16, 4);
    DEFINE_OPORTS(0, COMPLEX16, 4);
    DEFINE_OPORTS(1, COMPLEX16, 4);
    
public:
    REFERENCE_LOCAL_CONTEXT(TBB11aSymSel);
    	
    STD_DEMUX2_CONSTRUCTOR(TBB11aSymSel)
        BIND_CONTEXT(CF_11aSymState::symbol_type, symbol_type)
    { }

    STD_TDEMUX2_RESET()
    { }

    FINL void Flush()
    { }

	BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
			vcs& pi = cast_ref<vcs> (ipin.peek());
			ipin.pop();
			if ( symbol_type == CF_11aSymState::SYMBOL_TRAINING ) {
				vcs &po = cast_ref<vcs> (opin0().append());
				po = pi;
				Next0()->Process (opin0());
            } else {
				vcs &po = cast_ref<vcs> (opin1().append());
				po = pi;

				Next1()->Process (opin1());
            }
            
        }
        return true;
    }
};
#endif

//
// T11aDataSymbol - skip CP here
//
DEFINE_LOCAL_CONTEXT(T11aDataSymbol, CF_11aRxVector, CF_Error);
template<TFILTER_ARGS>
class T11aDataSymbol : public TFilter<TFILTER_PARAMS>
{
static const int skip_cp   = 8;

private:	
	CTX_VAR_RW (ushort, remain_symbols );	
	CTX_VAR_RW (ulong, error_code );
	
	vcs m_fft_out[16];

	public:
		DEFINE_IPORT(COMPLEX16, 80); 	// 
		DEFINE_OPORT(COMPLEX16, 64); 	// passthru the sample downstream

	public:
		REFERENCE_LOCAL_CONTEXT(T11aDataSymbol );
		
		STD_TFILTER_CONSTRUCTOR(T11aDataSymbol )
			BIND_CONTEXT(CF_11aRxVector::remain_symbols, remain_symbols)					
			BIND_CONTEXT(CF_Error::error_code,		  error_code)		
		{ }
	
		STD_TFILTER_RESET() { }
		
		STD_TFILTER_FLUSH() { }
	
		BOOL_FUNC_PROCESS (ipin)
		{
			while (ipin.check_read())
			{
				vcs* pi = (vcs*)ipin.peek ();
				pi += skip_cp/4; // skip CP

				vcs* po = (vcs*) opin().append();
				rep_memcpy<16> (po, pi);

				_dump_symbol<64> ( "OFDM symbol", (COMPLEX16*) po );
				
				Next()->Process(opin());
				ipin.pop();


                remain_symbols --;
				if ( remain_symbols == 0 ) {
                    //
					// this is compatible for multi-thread pipeline processing.
					// a second thread maybe still processing the data, while
					// the first thread has done all symbols.
					// If the second thread is done (or the brick is running
					// in a single thread), the blocking flushing will return
					//
                    
                    Next()->Flush();

                    // The viterbi subgraph is expected to processed all data, and get CRC result
                    // If not, logic error!!!
                    if (error_code == E_ERROR_SUCCESS)
                    {
                        assert(0);
                        error_code = E_ERROR_FAILED;
                        return false;
                    }
				}
			}
			return true;
		}

};

//
// TLTSymbol - consume 160 samples
//
DEFINE_LOCAL_CONTEXT(T11aLTSymbol, CF_11aSymState);
template<TFILTER_ARGS>
class T11aLTSymbol: public TFilter<TFILTER_PARAMS>
{

private:
static const int pre_read   = 12; //16;
static const int skip_cp    = 12;
static const int lts_number = 2; 
static const int skip_tail = 160 - skip_cp - 64 * lts_number - pre_read;

private:
	CTX_VAR_RW (ulong, symbol_type ); 

protected:
	int m_skip_cp_count;
	int m_skip_tail_count;
	int m_lts_count;
	int m_wr_count;
	vcs* p_write; 
	
	FINL void __init () {
		m_skip_cp_count = 0;
		m_skip_tail_count = 0;
		m_lts_count = lts_number;
		
		m_wr_count = 0;
		p_write = NULL;
	}
	
public:
	DEFINE_IPORT(COMPLEX16, 4); 	
	DEFINE_OPORT(COMPLEX16, 64); 	

	REFERENCE_LOCAL_CONTEXT(T11aLTSymbol);

	STD_TFILTER_CONSTRUCTOR(T11aLTSymbol)
		BIND_CONTEXT(CF_11aSymState::symbol_type, symbol_type )
	{ __init (); }

	STD_TFILTER_RESET() { 
		__init ();
	}

	STD_TFILTER_FLUSH() { }

	BOOL_FUNC_PROCESS (ipin)
	{
		while (ipin.check_read())
		{
			vcs& pi = cast_ref<vcs>(ipin.peek());

			if ( m_skip_cp_count < skip_cp ) {
				m_skip_cp_count += 4;

			} else if ( m_lts_count > 0 ) {
				if ( m_wr_count == 0 ) {
					p_write = (vcs*)opin().append();
				} 
				
				*p_write = pi; p_write ++;
				m_wr_count += 4;
				
				if ( m_wr_count == 64 ) {
					Next()->Process (opin());
					m_wr_count = 0;
					m_lts_count --;
				}

			} else if ( m_skip_tail_count < skip_tail ){
				m_skip_tail_count += 4;
				if ( m_skip_tail_count >= skip_tail ) {
					symbol_type = CF_11aSymState::SYMBOL_OFDM_DATA;
				}
			}
			
			ipin.pop ();
		}
		
		return true;
	};
};

DEFINE_LOCAL_CONTEXT(T11aPLCPParser, CF_11aRxVector, CF_11RxPLCPSwitch, CF_Error, CF_TimeStamps);
template<TSINK_ARGS>
class T11aPLCPParser : public TSink<TSINK_PARAMS>
{
private:
	CTX_VAR_RW (ushort, frame_length   ); // size of the frame payload
	CTX_VAR_RW (ushort, total_symbols  ); // total OFDM symbols	
	CTX_VAR_RW (ushort, remain_symbols ); 
	CTX_VAR_RW (ulong,  data_rate_kbps ); // data rate in kbps		
	CTX_VAR_RW (ushort, code_rate );
	CTX_VAR_RW (CF_11RxPLCPSwitch::PLCPState,  plcp_state );
	CTX_VAR_RW (ulong,  error_code );
    CTX_VAR_RW (SoraPerformanceCounter, decoding_data_stopwatch);

public:
    DEFINE_IPORT(uint, 1);
	
public:
	REFERENCE_LOCAL_CONTEXT(T11aPLCPParser);
    STD_TSINK_CONSTRUCTOR(T11aPLCPParser)
		BIND_CONTEXT (CF_11aRxVector::frame_length, frame_length)
		BIND_CONTEXT (CF_11aRxVector::total_symbols, total_symbols)	
		BIND_CONTEXT (CF_11aRxVector::data_rate_kbps, data_rate_kbps)
		BIND_CONTEXT (CF_11aRxVector::code_rate, code_rate)
		BIND_CONTEXT (CF_11aRxVector::remain_symbols, remain_symbols)
		BIND_CONTEXT (CF_11RxPLCPSwitch::plcp_state, plcp_state)
		BIND_CONTEXT (CF_Error::error_code, error_code)
        BIND_CONTEXT (CF_TimeStamps::decoding_data_stopwatch, decoding_data_stopwatch)
    { }

	FINL 
	bool _parse_plcp ( uint uiSignal ) {
		uint uiParity;
		
	    uiSignal &= 0xFFFFFF;
    	if (uiSignal & 0xFC0010) // all these bits should be always zero
        	return false;
    
    	uiParity = (uiSignal >> 16) ^ (uiSignal);
    	uiParity = (uiParity >> 8) ^ (uiParity);
    	uiParity = (uiParity >> 4) ^ (uiParity);
	    uiParity = (uiParity >> 2) ^ (uiParity);
	    uiParity = (uiParity >> 1) ^ (uiParity);
	    if (uiParity & 0x1)
			return false;

    	data_rate_kbps = BB11aParseDataRate ( uiSignal & 0xF );
		if ( data_rate_kbps == 0 )
			return false;

		code_rate = BB11aGetCodingRateFromDataRate (data_rate_kbps);

    	frame_length = (uiSignal >> 5) & 0xFFF;
        if (frame_length > 2500) // MTU = 2500
            return false;
		
        int Nsym  = B11aGetSymbolCount(data_rate_kbps, frame_length);
		total_symbols  = (ushort)Nsym + 1; // count this plcp symbol
		remain_symbols = total_symbols;
        assert((short)remain_symbols > 0);

        return true;
	}
	
    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            uint uiSignal = *ipin.peek();
            ipin.pop();

			if ( _parse_plcp (uiSignal) ) {
                decoding_data_stopwatch.Restart();
                _dump_text ( "PCLP: data rate %d length %d symbols %d\n", 
                    data_rate_kbps, frame_length, remain_symbols );
					
				// now proceed to data
				plcp_state = CF_11RxPLCPSwitch::plcp_data;
				
			} else {
				error_code = E_ERROR_PLCP_HEADER_FAIL;
			}
			
        }
        return true;
    }
};


DEFINE_LOCAL_CONTEXT(TBB11aFrameSink, CF_RxFrameBuffer, CF_Error, CF_11aRxVector);
template<TSINK_ARGS>
class TBB11aFrameSink : public TSink<TSINK_PARAMS>
{
	CTX_VAR_RO (uchar*, rx_frame_buf );	  	
	CTX_VAR_RO (uint,   rx_frame_buf_size ); 

	CTX_VAR_RO (ushort, frame_length );	 // expected frame length in plcp 	
	CTX_VAR_RW (ulong,  frame_crc32 )
		
	CTX_VAR_RW (ulong,  error_code ); 

protected:
	// internal states
	uchar * byte_pointer;
	ulong   byte_count;
	ulong   crc32;
	
	FINL void _init () {
		crc32 = 0xFFFFFFFF;
		byte_count    = 0;
		byte_pointer  = NULL;

		if ( rx_frame_buf == NULL ) {
			error_code = E_ERROR_PARAMETER;
			return;
		}
		byte_pointer = rx_frame_buf;
	}

public:
	DEFINE_IPORT(uchar, 1);
	
public:
    REFERENCE_LOCAL_CONTEXT(TBB11aFrameSink);

    STD_TSINK_CONSTRUCTOR(TBB11aFrameSink)
        BIND_CONTEXT(CF_RxFrameBuffer::rx_frame_buf, rx_frame_buf )
        BIND_CONTEXT(CF_RxFrameBuffer::rx_frame_buf_size, rx_frame_buf_size )
        BIND_CONTEXT(CF_11aRxVector::frame_length, frame_length )
        BIND_CONTEXT(CF_11aRxVector::crc32, frame_crc32 )
        // CF_Error
        BIND_CONTEXT(CF_Error::error_code, error_code)
    {
        _init();
    }
	STD_TSINK_RESET()
    {
    	_init ();
    }
	STD_TSINK_FLUSH() {}

    BOOL_FUNC_PROCESS (ipin)
    {
        assert(frame_length <= 2500); // MTU = 2500
        while (ipin.check_read())
        {
        	uchar b = *ipin.peek();
			ipin.pop();

            _dump_text("decoded %02X\n", b); 
			if ( byte_count < (ulong)(frame_length - 4) ) {
				byte_count ++;				
				*byte_pointer = b; byte_pointer ++; 	
				
				// compute crc
				CalcCRC32Incremental (b, &crc32 );
			} else	if ( byte_count < (ulong)(frame_length) ) {
//				error_code = E_ERROR_FRAME_OK;
//			    return false;

				byte_count ++;				
				*byte_pointer = b; byte_pointer ++; 	

				if ( byte_count == frame_length ) {
					// only compare the first three bytes
					uint* pcrc = (uint*) (byte_pointer - 4);
					frame_crc32 = *pcrc;

                    _dump_text ( "CRC32 computed %08x - in frame %08x\n", (~crc32 ), frame_crc32 );

					if ( (~crc32) == (*pcrc) ) {
						error_code = E_ERROR_FRAME_OK;
					} else {
						error_code = E_ERROR_CRC32_FAIL;
					}

					return false;
				}	
			}
			
        }
		
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(TBB11aRxRateSel, CF_11RxPLCPSwitch, CF_11aRxVector );
template<TDEMUX5_ARGS>
class TBB11aRxRateSel : public TDemux<TDEMUX5_PARAMS>
{
    CTX_VAR_RO (CF_11RxPLCPSwitch::PLCPState, plcp_state );
    CTX_VAR_RO (ulong,  data_rate_kbps );  // data rate in kbps		
public:
    static const size_t BURST = T_NEXT0::iport_traits::burst;
	DEFINE_IPORT (   COMPLEX16, BURST);
    DEFINE_OPORTS(0, COMPLEX16, BURST); // plsc header
    DEFINE_OPORTS(1, COMPLEX16, BURST); // BPSK
    DEFINE_OPORTS(2, COMPLEX16, BURST); // QPSK
    DEFINE_OPORTS(3, COMPLEX16, BURST); // 16QAM
    DEFINE_OPORTS(4, COMPLEX16, BURST); // 64QAM

private:
    // Add these connection requirements, to enable reusing pinqueue
    CCASSERT(!NON_DUMMYBRICK(1) || BURST == T_NEXT1::iport_traits::burst);
    CCASSERT(!NON_DUMMYBRICK(2) || BURST == T_NEXT2::iport_traits::burst);
    CCASSERT(!NON_DUMMYBRICK(3) || BURST == T_NEXT3::iport_traits::burst);
    CCASSERT(!NON_DUMMYBRICK(4) || BURST == T_NEXT4::iport_traits::burst);

public:
    REFERENCE_LOCAL_CONTEXT(TBB11aRxRateSel);
    	
    STD_DEMUX5_CONSTRUCTOR(TBB11aRxRateSel)
        BIND_CONTEXT(CF_11RxPLCPSwitch::plcp_state,  plcp_state)
        BIND_CONTEXT(CF_11aRxVector::data_rate_kbps,  data_rate_kbps)
    {}

    void Reset()
    {
        Next0()->Reset();

        // No need to reset all path, just reset the path we used in this frame
		switch (data_rate_kbps) {
		case 6000:
		case 9000:
            Next1()->Reset();
			break;
		case 12000:
		case 18000:
            Next2()->Reset();
			break;
		case 24000:
		case 36000:
            Next3()->Reset();
			break;
		case 48000:
		case 54000:
            Next4()->Reset();
			break;
		}
    }

    void Flush()
    {
        if (plcp_state == CF_11RxPLCPSwitch::plcp_header) {
            Next0()->Flush();
        } else {
			switch (data_rate_kbps) {
			case 6000: 
			case 9000:
                Next1()->Flush();
				break;
			case 12000:
			case 18000:
                Next2()->Flush();
				break;
			case 24000:
			case 36000:
                Next3()->Flush();
				break;
			case 48000:
			case 54000:
                Next4()->Flush();
				break;
			}
        }
    }

	BOOL_FUNC_PROCESS (ipin)
    {
        if (ipin.check_read())
        {
            if (plcp_state == CF_11RxPLCPSwitch::plcp_header) {
			    Next0()->Process(ipin);
            } else {
			    switch (data_rate_kbps) {
			    case 6000:
			    case 9000:
			        Next1()->Process(ipin);
				    break;
			    case 12000:
			    case 18000:
			        Next2()->Process(ipin);
				    break;
			    case 24000:
			    case 36000:
			        Next3()->Process(ipin);
				    break;
			    case 48000:
			    case 54000:
			        Next4()->Process(ipin);
				    break;
			    }
            }
        }
        return true;
    }
};

// TFrameError:: Discard any data type and set error_code
DEFINE_LOCAL_CONTEXT(TFrameError, CF_Error);
template< TSINK_ARGS >
class TFrameError : public TSink<TSINK_PARAMS>
{
	CTX_VAR_RW (ulong, error_code);
public:
    DEFINE_IPORT(void, 1);

	REFERENCE_LOCAL_CONTEXT(TFrameError)
		
    STD_TSINK_CONSTRUCTOR(TFrameError) 
		BIND_CONTEXT (CF_Error::error_code, error_code)
	{}
	
	STD_TSINK_RESET() {}
	STD_TSINK_FLUSH() {}

    BOOL_FUNC_PROCESS(pin) {
        pin.clear ();
		error_code = E_ERROR_NOT_SUPPORTED;
		
        return true;
    }
};
