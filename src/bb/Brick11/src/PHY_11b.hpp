#pragma once

#include "ieee80211facade.hpp"
#include "dot11_plcp.h"
#include "bb/DataRate.h"

#include <dspcomm.h>
#include "CRC16.h"
#include "CRC32.h"

#pragma pack(push,1)
struct _sfd_plcp {
	uchar  sync_data [DOT11B_PLCP_LONG_PREAMBLE_SYNC_LENGTH];
	ushort sfd;
	DOT11B_PLCP_HEADER plcp;
};
#pragma pack(pop)

DEFINE_LOCAL_CONTEXT(TBB11bSrc, CF_11bTxVector, CF_TxFrameBuffer, CF_Error );
template<TSOURCE_ARGS>
class TBB11bSrc : public TSource<TSOURCE_PARAMS>
{
private:
	// Context binding
	CTX_VAR_RO (ushort, frame_length );  /* 1-4095*/
	CTX_VAR_RO (uchar, preamble_type);   /* 0=LONG, 1=SHORT*/
	CTX_VAR_RO (uchar, mod_select);      /* 0=CCK, 1=PBCC */    
	CTX_VAR_RO (ulong, data_rate_kbps ); /* data rate in kbps */		 
	CTX_VAR_RO (ulong, crc32 );		     /* update CRC32 in the brick */

	// 
	CTX_VAR_RO (uchar*, mpdu_buf0 );	  	
	CTX_VAR_RO (uchar*, mpdu_buf1 );	  	
	CTX_VAR_RO (ushort, mpdu_buf_size0 );	  	
	CTX_VAR_RO (ushort, mpdu_buf_size1 );	  	

	// Read write field
	CTX_VAR_RW (ulong, error_code );     /* error code for the modulation  */	
	
protected:	
	// local states
	uchar  rate_code;	/* the data rate code */
	uint   sync_len;
    struct _sfd_plcp m_header;
	
	void _init () {
		memset(&m_header, 0, sizeof(m_header));

		Preprocess (); // check all parameters and prepare the headers
	}

public:
	// outport
	DEFINE_OPORT (UCHAR, 1);

public:
	// Brick interface
    REFERENCE_LOCAL_CONTEXT(TBB11bSrc);

    STD_TSOURCE_CONSTRUCTOR(TBB11bSrc)
		BIND_CONTEXT (CF_11bTxVector::frame_length,    frame_length )
		BIND_CONTEXT (CF_11bTxVector::preamble_type,   preamble_type)
		BIND_CONTEXT (CF_11bTxVector::mod_select,      mod_select   )
		BIND_CONTEXT (CF_11bTxVector::data_rate_kbps,  data_rate_kbps )
		BIND_CONTEXT (CF_11bTxVector::crc32,		   crc32 ) 	
		// CF_TxFrameBuffer
		BIND_CONTEXT (CF_TxFrameBuffer::mpdu_buf0, 	   mpdu_buf0 )
		BIND_CONTEXT (CF_TxFrameBuffer::mpdu_buf1, 	   mpdu_buf1 )
		BIND_CONTEXT (CF_TxFrameBuffer::mpdu_buf_size0, mpdu_buf_size0 )
		BIND_CONTEXT (CF_TxFrameBuffer::mpdu_buf_size1, mpdu_buf_size1 )		
		// CF_Error
		BIND_CONTEXT (CF_Error::error_code,	   error_code ) 	
    {
    	_init ();
    }
	STD_TSOURCE_RESET() {
		_init ();
	}
	STD_TSOURCE_FLUSH() { }
	
public:		
	FINL USHORT GetPPDULength (OUT PUINT ext)
    {
        UINT ret;
        UINT size = frame_length + 4; //plus CRC32

        *ext = 0;
        switch (rate_code)
        {
        case DOT11B_PLCP_DATA_RATE_1M:
            return (USHORT)(size << 3);
        case DOT11B_PLCP_DATA_RATE_2M:
            return (USHORT)(size << 2);
        case DOT11B_PLCP_DATA_RATE_5P5M:
            return (USHORT)(((size << 4) - 1) / 11 + 1);
        case DOT11B_PLCP_DATA_RATE_11M:
            ret = ((size << 3) - 1) / 11 + 1;
            if (ret * 11 - (size << 3) >= 8)
                (*ext)++;
            return (USHORT)(ret);
        default:
            NODEFAULT;
        }
    }

	FINL
	bool PreparePLCPHeader () {
		// fill PLCP Header	
		rate_code =  Dot11BDataRate_Kbps2Code(data_rate_kbps);
		if (  rate_code == 0 ) {
			error_code = E_ERROR_DATARATE;
			return false;
		}

		if ( preamble_type == 1 ) {
			// short preamble is not supported yet
			error_code = E_ERROR_NOT_SUPPORTED;
			return false;
		}

		// plcp header
		UINT ext = 0;
		m_header.plcp.Signal = rate_code;
		m_header.plcp.Length = GetPPDULength (&ext);
		m_header.plcp.Service.Bits.LengthExt = (UCHAR) ext;
		m_header.plcp.CRC = CalcCRC16((PUCHAR)&m_header.plcp.Signal, 4);

		// sfd
		m_header.sfd 	  =	DOT11B_PLCP_LONG_PREAMBLE_SFD;

		// sync bits
		sync_len = DOT11B_PLCP_LONG_PREAMBLE_SYNC_LENGTH;
		memset ( m_header.sync_data, 0xFF, sync_len );

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

		if ( !PreparePLCPHeader () ) {
			return false;
		}

		return true;
	}
	
    bool Process ()
    {
		// Drive the encoding pipeline
#if 1
		uchar * pc0 = (PUCHAR)&m_header; 
		uchar * pc1 = mpdu_buf0;
		uchar * pc2 = mpdu_buf1;
		uchar * pc3 = (uchar*) &crc32;
		
		int count = 0;
		int totalbyte = sizeof(m_header) + frame_length + 4;
		int block0_size, block1_size;
		
		block0_size = sizeof(m_header) + mpdu_buf_size0;
		block1_size = block0_size + mpdu_buf_size1;
		
		while (count < totalbyte ) {
			if (count < sizeof(m_header) ) {
	            *opin().append() = *pc0; pc0 ++;				
			} else if ( count < block0_size )
			{
			    *opin().append() = *pc1; pc1 ++;
			} else if ( count < block1_size ) {
			    *opin().append() = *pc2; pc2 ++;
			} else {
			    *opin().append() = *pc3; pc3 ++; // CRC32
			}
			
			count ++;
			Next()->Process(opin());
		}
#else		
		uchar * pc;
        for (pc = (PUCHAR)&m_header; pc < (PUCHAR)& m_header + sizeof(m_header); pc++)
        {
            *opin().append() = *pc;
            Next()->Process(opin());
        }

		pc = mpdu_buf;
		uint ppdu_size = frame_length  + 4 /*pad*/;
		for ( uint j=0; j< ppdu_size; j++ ) {
            // Process data + CRC32
            *opin().append() = *pc;
			pc ++;
            Next()->Process(opin());
    	}
#endif
        return false;
    }
};



/*************************************************
TBB11bMRSelect - modulation multi-rate selector
*************************************************/

DEFINE_LOCAL_CONTEXT(TBB11bMRSelect, CF_11bTxVector);
template<TDEMUX4_ARGS>
class TBB11bMRSelect : public TDemux<TDEMUX4_PARAMS>
{
	CTX_VAR_RO(uchar, preamble_type); 
	CTX_VAR_RO(ulong, data_rate_kbps);
	
	int  m_preamble_cnt;
    FINL void _init()
    {
		if ( preamble_type == 1 ) // short preamble
		{
			m_preamble_cnt = DOT11B_PLCP_SHORT_PREAMBLE_SYNC_LENGTH + 2 /*sfd*/+ 6/*plcp*/;
		} else {
			m_preamble_cnt = DOT11B_PLCP_LONG_PREAMBLE_SYNC_LENGTH + 2 /*sfd*/+ 6/*plcp*/;
		}
    }
public:
	DEFINE_IPORT(uchar, 1);
    DEFINE_OPORTS(0, uchar, 1);
    DEFINE_OPORTS(1, uchar, 1);
    DEFINE_OPORTS(2, uchar, 1);
    DEFINE_OPORTS(3, uchar, 1);

public:
    REFERENCE_LOCAL_CONTEXT(TBB11bMRSelect);
    	
    STD_DEMUX4_CONSTRUCTOR(TBB11bMRSelect)
        BIND_CONTEXT(CF_11bTxVector::preamble_type,  preamble_type)
        BIND_CONTEXT(CF_11bTxVector::data_rate_kbps, data_rate_kbps)
    {
        _init();
    }

    STD_TDEMUX4_RESET()
    {
        _init();
    }

    FINL void Flush()
    {
        if (m_preamble_cnt || (data_rate_kbps == 1000))
        {
            FlushPort(0);
        }
        else if (data_rate_kbps == 2000)
        {
            FlushPort(1);
        }
        else if (data_rate_kbps == 5500)
        {
            FlushPort(2);
        }
        else
        {
            assert(data_rate_kbps == 11000);
            FlushPort(3);
        }
    }

	BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
			uchar b = *ipin.peek();
			ipin.pop();
			if ( m_preamble_cnt || (data_rate_kbps == 1000)) {
				*opin0().append() = b;
				Next0()->Process (opin0());
				m_preamble_cnt --;
            }
            else if (data_rate_kbps == 2000)
            {
				*opin1().append() = b;
				Next1()->Process (opin1());
			}
            else if (data_rate_kbps == 5500)
            {
				*opin2().append() = b;
				Next2()->Process (opin2());
            }
            else
            {
                assert(data_rate_kbps == 11000);
				*opin3().append() = b;
				Next3()->Process (opin3());
            }
        }
        return true;
    }
};



/*************************************************
TBB11bRxSwitch - switcher between CCA and Demod
*************************************************/
DEFINE_LOCAL_CONTEXT(TBB11bRxSwitch, CF_11CCA );
template<TDEMUX2_ARGS>
class TBB11bRxSwitch : public TDemux<TDEMUX2_PARAMS>
{
	CTX_VAR_RO(CF_11CCA::CCAState, cca_state ); 
	
public:
	DEFINE_IPORT(COMPLEX16, 4);
//	DEFINE_IPORT(vcs, 1);

	
    DEFINE_OPORTS(0, COMPLEX16, 4);
    DEFINE_OPORTS(1, COMPLEX16, 4);

public:
    REFERENCE_LOCAL_CONTEXT(TBB11bRxSwitch);
    	
    STD_DEMUX2_CONSTRUCTOR(TBB11bRxSwitch)
        BIND_CONTEXT(CF_11CCA::cca_state,  cca_state)
    {
    }

    // Note: not use STD_TDEMUX2_RESET to improve downstreaming symbol timing performance
    FINL void Reset()
    {
        TDemux::Reset();
    }

    FINL void Flush()
    {
        if (cca_state == CF_11CCA::power_clear)
        {
            FlushPort(0);
        }
        else 
        {
            FlushPort(1);
        }
    }

	BOOL_FUNC_PROCESS(ipin)
    {
    	// Marker::TBB11bRxSwitch
        while(ipin.check_read())
        {
			vcs* pi = (vcs*) ipin.peek();

			if (cca_state == CF_11CCA::power_clear) {
				vcs* po = (vcs*) opin0().append();
				*po = *pi;
				Next0()->Process (opin0());
			} else {
				vcs* po = (vcs*) opin1().append();
				*po = *pi;
				Next1()->Process (opin1());
			}
			ipin.pop();
        }
        return true;
    }
};

/*************************************************
TBB11bRxRateSel - multiple decoding rate selector
*************************************************/
DEFINE_LOCAL_CONTEXT(TBB11bRxRateSel, CF_11bRxMRSel );
template<TDEMUX5_ARGS>
class TBB11bRxRateSel : public TDemux<TDEMUX5_PARAMS>
{
	CTX_VAR_RO(CF_11bRxMRSel::RxRateState, rxrate_state ); 
	
public:
	DEFINE_IPORT(COMPLEX16, 1);
	
    DEFINE_OPORTS(0, COMPLEX16, 1); // frame sync
    DEFINE_OPORTS(1, COMPLEX16, 1); // 1M - bpsk
    DEFINE_OPORTS(2, COMPLEX16, 1); // 2M - qpsk
    DEFINE_OPORTS(3, COMPLEX16, 1); // 5.5M - cck5p5
    DEFINE_OPORTS(4, COMPLEX16, 1); // 11M - cck11

public:
    REFERENCE_LOCAL_CONTEXT(TBB11bRxRateSel);
    	
    STD_DEMUX5_CONSTRUCTOR(TBB11bRxRateSel)
        BIND_CONTEXT(CF_11bRxMRSel::rxrate_state,  rxrate_state)
    {}

    STD_TDEMUX5_RESET() { }

    FINL void Flush()
    {
    	switch (rxrate_state) {
		case CF_11bRxMRSel::rate_sync:
            opin0().pad();
            Next0()->Process(opin0());
			break;
		case CF_11bRxMRSel::rate_1m:
			opin1().pad();
            Next1()->Process(opin1());
			break;
		case CF_11bRxMRSel::rate_2m:
			opin2().pad();
			Next2()->Process(opin2());
			break;
		case CF_11bRxMRSel::rate_5p5m:
			opin3().pad();
			Next3()->Process(opin3());
			break;
		case CF_11bRxMRSel::rate_11m:
			opin4().pad();
			Next4()->Process(opin4());
			break;
        }
    }

	BOOL_FUNC_PROCESS (ipin)
    {
        while(ipin.check_read())
        {
			const COMPLEX16* pi = ipin.peek();
			switch (rxrate_state) {
			case CF_11bRxMRSel::rate_sync:
			    *opin0().append() = *pi;
			    Next0()->Process(opin0());
				break;
			case CF_11bRxMRSel::rate_1m:
			    *opin1().append() = *pi;
			    Next1()->Process(opin1());
				break;
			case CF_11bRxMRSel::rate_2m:
			    *opin2().append() = *pi;
				Next2()->Process(opin2());
				break;
			case CF_11bRxMRSel::rate_5p5m:
			    *opin3().append() = *pi;
				Next3()->Process(opin3());
				break;
            case CF_11bRxMRSel::rate_11m:
			    *opin4().append() = *pi;
				Next4()->Process(opin4());
				break;
			}
			ipin.pop();
        }
        return true;
    }
};

/*********************************************************
TBB11bPlcpSwitch - switcher between plcp header and data
**********************************************************/
DEFINE_LOCAL_CONTEXT(TBB11bPlcpSwitch, CF_11RxPLCPSwitch );
template<TDEMUX2_ARGS>
class TBB11bPlcpSwitch : public TDemux<TDEMUX2_PARAMS>
{
	CTX_VAR_RO(CF_11RxPLCPSwitch::PLCPState, plcp_state ); 
	
public:
	DEFINE_IPORT(uchar, 1);
	
    DEFINE_OPORTS(0, uchar, 1);
    DEFINE_OPORTS(1, uchar, 1);

public:
    REFERENCE_LOCAL_CONTEXT(TBB11bPlcpSwitch);
    	
    STD_DEMUX2_CONSTRUCTOR(TBB11bPlcpSwitch)
        BIND_CONTEXT(CF_11RxPLCPSwitch::plcp_state, plcp_state)
    {}

    STD_TDEMUX2_RESET() { }

    FINL void Flush()
    {
        if (plcp_state == CF_11RxPLCPSwitch::plcp_header)
        {
			FlushPort (0);
        }
        else if (plcp_state == CF_11RxPLCPSwitch::plcp_data)
        {
			FlushPort (1);
        }
    }

	BOOL_FUNC_PROCESS (ipin)
    {
        while(ipin.check_read())
        {
			const uchar& pi = *ipin.peek();

			if (plcp_state == CF_11RxPLCPSwitch::plcp_header)
			{
				*opin0().append() = pi;
				Next0()->Process(opin0());
			}
			else if (plcp_state == CF_11RxPLCPSwitch::plcp_data)
			{
				*opin1().append() = pi;
				Next1()->Process(opin1());
			} else {
				// just drop it
				
			}

			ipin.pop();
        }
        return true;
    }
};

/*********************************************************
	TBB11bPlcpParser - 11b PLCP header parser
**********************************************************/
DEFINE_LOCAL_CONTEXT(TBB11bPlcpParser, CF_11bRxVector, CF_Error, CF_11bRxMRSel, CF_11RxPLCPSwitch);
template<TSINK_ARGS>
class TBB11bPlcpParser : public TSink<TSINK_PARAMS>
{
	//
	// context binding
	//
	CTX_VAR_RW(ushort, frame_length   ); 
	CTX_VAR_RW(ulong,  data_rate_kbps ); 
	
	CTX_VAR_RW(ulong,  error_code ); 
	// control switch states
	CTX_VAR_RW(CF_11RxPLCPSwitch::PLCPState,  plcp_state ); 
	CTX_VAR_RW(CF_11bRxMRSel::RxRateState,    rxrate_state );

public:
	DEFINE_IPORT(UCHAR, sizeof(DOT11B_PLCP_HEADER));
	
public:
    REFERENCE_LOCAL_CONTEXT(TBB11bPlcpParser);

    STD_TSINK_CONSTRUCTOR(TBB11bPlcpParser)
        BIND_CONTEXT(CF_11bRxVector::frame_length,   frame_length )
        BIND_CONTEXT(CF_11bRxVector::data_rate_kbps, data_rate_kbps )
        BIND_CONTEXT(CF_Error::error_code, error_code)
        BIND_CONTEXT(CF_11RxPLCPSwitch::plcp_state, plcp_state)
        BIND_CONTEXT(CF_11bRxMRSel::rxrate_state, rxrate_state)
    {}
	
	STD_TSINK_RESET() {}
	STD_TSINK_FLUSH() {}

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
        	const DOT11B_PLCP_HEADER* phdr = reinterpret_cast<const DOT11B_PLCP_HEADER*> (ipin.peek());

			ushort CRC16 = 	CalcCRC16((PUCHAR)phdr, 4);
			if ( CRC16 != phdr->CRC ) {
				error_code = E_ERROR_PLCP_HEADER_FAIL;

				// that is it - just wait the source the stop the stream
			} else {
				// a valid plcp header
				data_rate_kbps = GetDataRateKbps ( phdr->Signal );
				frame_length   = GetFrameLength  ( phdr->Signal,
												   phdr->Length, 
												   phdr->Service.bValue );

//				printf ( "PLCP Header found! frame-len %d, data rate %d (kbps)\n",
//							frame_length, data_rate_kbps );

				// control
				switch (data_rate_kbps ) {
				case 1000:
					// dbpsk 
					rxrate_state = CF_11bRxMRSel::rate_1m;
					break;
				case 2000:
					// dqpsk 
					rxrate_state = CF_11bRxMRSel::rate_2m;
					break;
                case 5500:
                    // cck5
                    rxrate_state = CF_11bRxMRSel::rate_5p5m;
                    break;
                case 11000:
                    // cck11
                    rxrate_state = CF_11bRxMRSel::rate_11m;
                    break;
				}

				plcp_state = CF_11RxPLCPSwitch::plcp_data;
			}

			ipin.pop();	
        }

			
        return true;
    }


	FINL ulong GetDataRateKbps (uchar rate_code )
	{
		ulong rate;
		switch (rate_code)
		{
		case DOT11B_PLCP_DATA_RATE_1M:
			rate = 1000;
			break;
		case DOT11B_PLCP_DATA_RATE_2M:
			rate = 2000;
			break;
		case DOT11B_PLCP_DATA_RATE_5P5M:
			rate = 5500;
			break;
		case DOT11B_PLCP_DATA_RATE_11M:
			rate = 11000;
			break;
		default:
			rate = 0;
		}
		return rate;
	}
	
	FINL ushort GetFrameLength ( uchar signal, ushort len, uchar service )
	{
		switch (signal)
		{
		case DOT11B_PLCP_DATA_RATE_1M:
			len = len >> 3;
			break;
		case DOT11B_PLCP_DATA_RATE_2M:
			len = len >> 2;
			break;
		case DOT11B_PLCP_DATA_RATE_5P5M:
			len = ((len * 11) >> 4) - (service >> 7) - ((service >> 3) & 0x1);
			break;
		case DOT11B_PLCP_DATA_RATE_11M:
			len = ((len * 11) >> 3) - (service >> 7) - ((service >> 3) & 0x1);
			break;
		default:
			len = 0;
		}
		return len;
	}
};


DEFINE_LOCAL_CONTEXT(TBB11bFrameSink, CF_RxFrameBuffer, CF_Error, CF_11bRxVector);
template<TSINK_ARGS>
class TBB11bFrameSink : public TSink<TSINK_PARAMS>
{
	CTX_VAR_RO (uchar*, rx_frame_buf );	  	
	CTX_VAR_RO (uint,   rx_frame_buf_size ); 

	CTX_VAR_RO (ushort, frame_length );	 // expected frame length in plcp 	
	CTX_VAR_RW (ulong, frame_crc32 )
		
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
    REFERENCE_LOCAL_CONTEXT(TBB11bFrameSink);

    STD_TSINK_CONSTRUCTOR(TBB11bFrameSink)
        BIND_CONTEXT(CF_RxFrameBuffer::rx_frame_buf, rx_frame_buf )
        BIND_CONTEXT(CF_RxFrameBuffer::rx_frame_buf_size, rx_frame_buf_size )
        BIND_CONTEXT(CF_11bRxVector::frame_length, frame_length )
        BIND_CONTEXT(CF_11bRxVector::crc32, frame_crc32 )
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
        while (ipin.check_read())
        {
        	uchar b = *ipin.peek();
			ipin.pop();
			
			if ( byte_count < (ulong)(frame_length - 4) ) {
				byte_count ++;				
				*byte_pointer = b; byte_pointer ++; 	
				
				// compute crc
				CalcCRC32Incremental (b, &crc32 );
			} else	if ( byte_count < (ulong)(frame_length) ) {
				byte_count ++;				
				*byte_pointer = b; byte_pointer ++; 	

				// speculating ACK ---
				// This is a walk-around for the RAB firmware 1.5 which adding
				// additional delay in TX path.
				//
				if ( byte_count == frame_length - 1) {
					// only compare the first three bytes
					uint* pcrc = (uint*) (byte_pointer - 3);
					frame_crc32 = *pcrc;
					
					if ( (~crc32 & 0x00FFFFFF) == ((*pcrc) & 0x00FFFFFF) ) {
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


