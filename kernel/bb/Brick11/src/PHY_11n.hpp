#pragma once
#include <dspcomm.h>
#include "ieee80211a_cmn.h"
#include "ieee80211n_cmn.h"
#include "ieee80211facade.hpp"
#include "_b_lsig.h"
#include "_b_htsig.h"
#include "CRC8.h"

//
//  11a Src -> Scramble -> Encode -> Interleave -> Mapper -> AddPilot -> IFFT -> AddGI -> AddTS -> TModSink
//
DEFINE_LOCAL_CONTEXT(TBB11nSrc, CF_11nTxVector, CF_TxFrameBuffer, CF_Error, CF_ScramblerControl );
template<TSOURCE_ARGS>
class TBB11nSrc : public TSource<TSOURCE_PARAMS>
{
public:
    static const ulong service_size     = 2;
    static const ulong fcs_size         = 4;
    static const ulong tail_size        = 1;

private:
    // Context binding
    // CF_11nTxVector
    CTX_VAR_RO (ushort, frame_length );    // payload size
    CTX_VAR_RO (ushort, mcs_index);

    CTX_VAR_RO (ulong,  crc32 );

    CTX_VAR_RW (ulong,  scramble_ctrl );

    // CF_TxFrameBuffer
    CTX_VAR_RO (uchar*, mpdu_buf0 );	  	
    CTX_VAR_RO (uchar*, mpdu_buf1 );	  	
    CTX_VAR_RO (ushort, mpdu_buf_size0 );	  	
    CTX_VAR_RO (ushort, mpdu_buf_size1 );

    // CF_Error
    CTX_VAR_RW (ulong,  error_code );     /* error code for the modulation  */	

protected:
    // local states
    uchar  service[service_size];

    void _init () {
        memset ( service, 0, sizeof(service));

        Preprocess (); // check all parameters and prepare the headers
    }

public:
    // outport
    DEFINE_OPORT (uchar, 1);

public:
    // Brick interface
    REFERENCE_LOCAL_CONTEXT(TBB11nSrc);

    STD_TSOURCE_CONSTRUCTOR(TBB11nSrc)
        BIND_CONTEXT (CF_11nTxVector::frame_length,     frame_length )
        BIND_CONTEXT (CF_11nTxVector::crc32,		    crc32 ) 	
        BIND_CONTEXT (CF_11nTxVector::mcs_index,        mcs_index )
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
    bool Process ()
    {
        // Drive the encoding pipeline
        uchar * pc1 = service;
        uchar * pc2 = mpdu_buf0;
        uchar * pc3 = mpdu_buf1;
        uchar * pc4 = (uchar*) &crc32;

        ulong Npad = ht_padding_bytes ( mcs_index, frame_length + fcs_size ) - tail_size;

        //printf ( "frame_len %d, Npad %d\n" , frame_length, Npad );
        //printf ( "crc32 %08x\n", crc32 );

        int count = 0;

        int totalbytes  = service_size + frame_length + fcs_size ;
        int tailedbytes = totalbytes + tail_size; 
        int paddedbytes = tailedbytes + Npad;

        int block0_size, block1_size;
        block0_size = service_size + mpdu_buf_size0;
        block1_size = block0_size + mpdu_buf_size1;

        // scramble all data payload
        // restore the specified rate
        scramble_ctrl = CF_ScramblerControl::DO_SCRAMBLE;

        while (count < paddedbytes ) {
            if ( count < service_size ) {
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

private:
    bool Preprocess () {
        error_code = E_ERROR_SUCCESS;

        if ( mpdu_buf0 == NULL 
            || (mpdu_buf_size1 > 0 && mpdu_buf1 == NULL ) 
            || (mpdu_buf_size0 + mpdu_buf_size1 != frame_length )) {
                error_code = E_ERROR_PARAMETER;
                return false; // done
        }	

        return true;
    }
};

/*************************************************
TBB11nMRSelect - modulation multi-rate selector
*************************************************/
DEFINE_LOCAL_CONTEXT(TBB11nMRSelect, CF_11nTxVector);
template<TDEMUX8_ARGS>
class TBB11nMRSelect : public TDemux<TDEMUX8_PARAMS>
{
    CTX_VAR_RO(ushort, mcs_index);

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
    REFERENCE_LOCAL_CONTEXT(TBB11nMRSelect);

    STD_DEMUX8_CONSTRUCTOR(TBB11nMRSelect)
        BIND_CONTEXT(CF_11nTxVector::mcs_index, mcs_index)
    { }

    STD_TDEMUX8_RESET()
    { }

    FINL void Flush()
    {
        switch(mcs_index)
        {
        case  8: FlushPort(0); break;
        case  9: FlushPort(1); break;
        case 10: FlushPort(2); break;
        case 11: FlushPort(3); break;
        case 12: FlushPort(4); break;
        case 13: FlushPort(5); break;
        case 14: FlushPort(6); break;
        case 15: FlushPort(7); break;
        default: NODEFAULT;
        }
    }

    BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
            uchar b = *ipin.peek();
            ipin.pop();

            switch(mcs_index)
            {
            case  8: *opin0().append() = b; Next0()->Process (opin0()); break;
            case  9: *opin1().append() = b; Next1()->Process (opin1()); break;
            case 10: *opin2().append() = b; Next2()->Process (opin2()); break;
            case 11: *opin3().append() = b; Next3()->Process (opin3()); break;
            case 12: *opin4().append() = b; Next4()->Process (opin4()); break;
            case 13: *opin5().append() = b; Next5()->Process (opin5()); break;
            case 14: *opin6().append() = b; Next6()->Process (opin6()); break;
            case 15: *opin7().append() = b; Next7()->Process (opin7()); break;
            default: NODEFAULT;
            }
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(TBB11nSigSrc, CF_11nTxVector);
template<TSOURCE_ARGS>
class TBB11nSigSrc : public TSource<TSOURCE_PARAMS>
{
    static const size_t SIG_SIZE = L_SIG::SIZE + HT_SIG::SIZE;

    // Context binding
    // CF_11nTxVector
    CTX_VAR_RO (ushort, frame_length );    // payload size
    CTX_VAR_RO (ushort, mcs_index);
    CTX_VAR_RO (ulong,  crc32 );


public:
    // outport
    DEFINE_OPORT (UCHAR, SIG_SIZE);

public:
    // Brick interface
    REFERENCE_LOCAL_CONTEXT(TBB11nSigSrc);

    STD_TSOURCE_CONSTRUCTOR(TBB11nSigSrc)
        BIND_CONTEXT (CF_11nTxVector::frame_length,     frame_length )
        BIND_CONTEXT (CF_11nTxVector::crc32,		    crc32 ) 	
        BIND_CONTEXT (CF_11nTxVector::mcs_index,        mcs_index )
    {
    }
    STD_TSOURCE_RESET() {
    }
    STD_TSOURCE_FLUSH() { }

    bool Process()
    {
        UCHAR *sig = opin().append();
		
        ushort frame_length_plus_fcs = frame_length + 4;
        int Nsym = ht_symbol_count(mcs_index, frame_length_plus_fcs) + 5; // 2 for HT_SIG, 1 for HT_STF, 2 for HT_LTF
        const int DOT11A_NDBPS_6M = 24;
        int lsig_frame_length = (Nsym * DOT11A_NDBPS_6M - service_length - padding) / 8;
        assert(Nsym == B11aGetSymbolCount(6000, (ushort)lsig_frame_length)); // L_SIG and HT_SIG should show equal symbol count

        // L_SIG
        _lsig.clear();
        // indicate as legacy 6Mbps frame
        // frame length should be 1/2 of original frame since we use two streams
        _lsig.update(DOT11A_RATE_6M, lsig_frame_length);

        sig[0] = _lsig.cdata[0];
        sig[1] = _lsig.cdata[1];
        sig[2] = _lsig.cdata[2];

        // HT_SIG
        _htsig.clear();
        _htsig.update(mcs_index, frame_length + 4);
        memcpy(&sig[3], _htsig.cdata, HT_SIG::SIZE);

        Next()->Process(opin());
        return true;
    }

private:
    L_SIG _lsig;
    HT_SIG _htsig;
};

//
// T11nDataSymbol - skip CP here
//
DEFINE_LOCAL_CONTEXT(T11nDataSymbol, CF_11aRxVector, CF_Error);
template<TFILTER_ARGS>
class T11nDataSymbol : public TFilter<TFILTER_PARAMS>
{
    CTX_VAR_RW (ushort, remain_symbols );	
    CTX_VAR_RW (ulong, error_code );

public:
    static const size_t NSTREAM = 2;
    DEFINE_IPORT(COMPLEX16, 80, NSTREAM); 	// 
    DEFINE_OPORT(COMPLEX16, 64, NSTREAM); 	// passthru the sample downstream

public:
    REFERENCE_LOCAL_CONTEXT(T11nDataSymbol );

    STD_TFILTER_CONSTRUCTOR(T11nDataSymbol )
        BIND_CONTEXT(CF_11aRxVector::remain_symbols, remain_symbols)
        BIND_CONTEXT(CF_Error::error_code,		  error_code)
    { }

    STD_TFILTER_RESET() { }

    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS (ipin)
    {
        while (ipin.check_read())
        {
            for (size_t iss = 0; iss < NSTREAM; iss++)
            {
                vcs* pi = (vcs*)ipin.peek(iss);
                pi += skip_cp/4; // skip CP

                vcs* po = (vcs*) opin().write(iss);
                rep_memcpy<16> (po, pi);

                _dump_symbol<64> ( "OFDM symbol", (COMPLEX16*) po );

            }
            ipin.pop();
            opin().append();
            Next()->Process(opin());

            remain_symbols --;
            if ( remain_symbols == 0 ) {
                //					printf ( "remaining symbol %d\n", remain_symbols );

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
private:
    static const int skip_cp   = 16;
    vcs m_fft_out[16];
};

DEFINE_LOCAL_CONTEXT(TMrcCombine, CF_VOID);
template<TFILTER_ARGS>
class TMrcCombine : public TFilter<TFILTER_PARAMS>
{
public:
    static const size_t NSTREAM = 2;
    static const size_t BURST = 64;
    DEFINE_IPORT(COMPLEX16, BURST, NSTREAM);
    DEFINE_OPORT   (COMPLEX16, BURST);

public:
    REFERENCE_LOCAL_CONTEXT(TMrcCombine);

    STD_TFILTER_CONSTRUCTOR(TMrcCombine) { }

    STD_TFILTER_RESET() { }

    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS (ipin)
    {
        while (ipin.check_read())
        {
            vcs *ip1 = (vcs *)ipin.peek(0);
            vcs *ip2 = (vcs *)ipin.peek(1);
            vcs *op  = (vcs *)opin().append();
            for (int i = 0; i < BURST / vcs::size; i++)
            {
                vcs r = add(ip1[i], ip2[i]);
                op[i] = shift_right(r, 1);
            }
            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(T11nSigParser, CF_11aRxVector, CF_HTRxVector, CF_11nSymState, CF_Error);
template<TSINK_ARGS>
class T11nSigParser : public TSink<TSINK_PARAMS>
{
private:
    CTX_VAR_RW (ushort, frame_length   ); // size of the frame payload
    CTX_VAR_RW (ushort, total_symbols  ); // total OFDM symbols	
    CTX_VAR_RW (ushort, remain_symbols ); 
    CTX_VAR_RW (ulong,  data_rate_kbps ); // data rate in kbps		
    CTX_VAR_RW (ushort, code_rate );
    CTX_VAR_RW (ulong,  symbol_type);
    CTX_VAR_RW (ulong,  error_code );
    CTX_VAR_RW (ushort, ht_frame_length );
    CTX_VAR_RW (ulong,  ht_frame_mcs );

public:
    DEFINE_IPORT(uchar, 9);

public:
    REFERENCE_LOCAL_CONTEXT(T11nSigParser);
    STD_TSINK_CONSTRUCTOR(T11nSigParser)
        BIND_CONTEXT (CF_11aRxVector::frame_length, frame_length)
        BIND_CONTEXT (CF_11aRxVector::total_symbols, total_symbols)	
        BIND_CONTEXT (CF_11aRxVector::data_rate_kbps, data_rate_kbps)
        BIND_CONTEXT (CF_11aRxVector::code_rate, code_rate)
        BIND_CONTEXT (CF_11aRxVector::remain_symbols, remain_symbols)
        BIND_CONTEXT (CF_HTRxVector::ht_frame_length, ht_frame_length )
        BIND_CONTEXT (CF_HTRxVector::ht_frame_mcs,  ht_frame_mcs )
        BIND_CONTEXT (CF_11nSymState::symbol_type, symbol_type)
        BIND_CONTEXT (CF_Error::error_code, error_code)
    { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const uchar *input = ipin.peek();
            uint uiSignal = *(const uint *)input;
            ipin.pop();

            if ( !_parse_plcp (uiSignal) || !_parse_htsig (input + 3)) {
                error_code = E_ERROR_PLCP_HEADER_FAIL;
                ipin.clear();
                return false;
            }

            // Note: frame_length will be used by viterbi decoder, so overwrite.
            frame_length = ht_frame_length;

            _dump_text ( "PCLP: data rate %d length %d symbols %d\n", 
                data_rate_kbps, frame_length, remain_symbols );

            symbol_type = CF_11nSymState::SYMBOL_HT_STF;
        }
        return true;
    }

private:
    FINL bool _parse_plcp ( uint uiSignal) {
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

        frame_length = ((uiSignal >> 5) & 0xFFF) * 2;
        if (frame_length > 1500) // MTU = 1500
            return false;

        return true;
    }

    FINL bool _parse_htsig(const uchar *ip)
    {
        UCHAR crc8 = CalcCRC8(ip, 4, 2);

        if (crc8 != ((ip[4] >> 2) | (ip[5] << 6)))
        {
            ht_frame_mcs    = 0;
            ht_frame_length = 0;
            _dump_text(" ht-sig error: crc8 check failed %X.\n", crc8);
            return false;
        }

        ht_frame_mcs    = (ip[0] & 0x7F);
        if (ht_frame_mcs < 8 || ht_frame_mcs >= 11)
        {
            _dump_text(" ht-sig error: MCS index not supported %d.\n", ht_frame_mcs);
            return false;
        }

        ht_frame_length = *((unsigned short*)(ip + 1));
        _dump_text(" ht-sig : mcs %X, length %d B.\n", ht_frame_mcs, ht_frame_length);
        if (ht_frame_length > 1500) // MTU = 1500
            return false;

        code_rate = BB11nGetCodingRateFromMcsIndex (ht_frame_mcs);
        total_symbols  = (ushort)ht_symbol_count(ht_frame_mcs, ht_frame_length) + 4; // 1 for this symbol, 1 for HT_STF, 2 for HT_LTF
        remain_symbols = total_symbols;
        assert((short)remain_symbols > 0);
        return true;
    }
};
