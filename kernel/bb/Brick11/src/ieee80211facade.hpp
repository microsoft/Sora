#pragma once

#include "sora.h"
#include "intalg.h"
#include "dspcomm.h"
#include "stdfacade.h"
#include "ieee80211const.h"
#include "soratime.h"

// Error codes
#define E_ERROR_FRAME_OK         0x00000001
#define E_ERROR_DATARATE  		 0x80000002
#define E_ERROR_NOT_SUPPORTED	 0x80000003
#define E_ERROR_SFD_FAIL		 0x80000004
#define E_ERROR_PLCP_HEADER_FAIL 0x80000005
#define E_ERROR_CRC32_FAIL       0x80000006
#define E_ERROR_CS_TIMEOUT       0x80000007
#define E_ERROR_SFD_TIMEOUT      0x80000008
#define E_ERROR_SYNC_TIMEOUT     0x80000009

class CF_11CCA {
public:
	typedef enum { 
		power_clear = 0,
		power_detected 
	} CCAState;
	
	FACADE_FIELD (CCAState, cca_state     );
	FACADE_FIELD (uint,     cca_pwr_threshold ); // for energy-based cca
	FACADE_FIELD (uint,     cca_pwr_reading   );	

public:
    CF_11CCA() {
		cca_pwr_threshold() = 1000*1000;
		cca_pwr_reading()   = 0;
    }
	
	FINL void Reset () {
		cca_state() = power_clear;
	}
	
    FINL void OnPowerDetected() { }
};

class CF_11bRxMRSel {
public:
	typedef enum { 
		rate_sync = 0,
		rate_1m,
		rate_2m,
        rate_5p5m,
		rate_11m
	} RxRateState;
	
	FACADE_FIELD (RxRateState, rxrate_state     );
};

class CF_11RxPLCPSwitch {
public:
	typedef enum {
		plcp_header = 0,
		plcp_data
	} PLCPState;

	FACADE_FIELD (PLCPState, plcp_state );

	FINL void Reset () {
		plcp_state() = plcp_header;
	}
};

class CF_11bRxVector {
	FACADE_FIELD(ushort, frame_length );   /* 1-4095*/
	FACADE_FIELD(ulong,  data_rate_kbps ); /* data rate in kbps */		 
	FACADE_FIELD(ulong,  crc32 );		   /* CRC32 for the frame */
};

class CF_11bTxVector {
	// INPUT
	FACADE_FIELD(ushort, frame_length );  /* 1-4095*/
	FACADE_FIELD(uchar, preamble_type);   /* 0=LONG, 1=SHORT*/
	FACADE_FIELD(uchar, mod_select);      /* 0=CCK, 1=PBCC */    
	FACADE_FIELD(ulong, data_rate_kbps ); /* data rate in kbps */		 
	FACADE_FIELD(ulong, crc32 );		  /* CRC32 for the frame */
};

class CF_TxFrameBuffer {
	FACADE_FIELD(uchar*, mpdu_buf0 );	  	
	FACADE_FIELD(ushort, mpdu_buf_size0 );	  	
	FACADE_FIELD(uchar*, mpdu_buf1 );	  	
	FACADE_FIELD(ushort, mpdu_buf_size1 );	  		
};

class CF_DifferentialMap {
	FACADE_FIELD(uint,  last_phase ); 
	// DBPSK: 0 - 00; pi - 01; 
	// DQPSK: 0 - 00; pi - 11; pi/2 - 10; pi*3/2 - 01
};

class CF_DifferentialDemap {
	FACADE_FIELD(COMPLEX16,  last_symbol );
	// DBPSK: 0 - 00; pi - 01; 
	// DQPSK: 0 - 00; pi - 11; pi/2 - 10; pi*3/2 - 01
};

class CF_RxFrameBuffer {
	FACADE_FIELD(uchar*, rx_frame_buf );	  	
	FACADE_FIELD(uint,   rx_frame_buf_size ); 
public:
	FINL void Init ( uchar* fbuf, uint fbuf_size ) {
		rx_frame_buf() = fbuf;
		rx_frame_buf_size() = fbuf_size;
	}
};

class CF_ScramblerSeed
{
	FACADE_FIELD (UCHAR, sc_seed);
};

class CF_ScramblerControl
{
	FACADE_FIELD (ulong, scramble_ctrl); // 0 - direct pass; 1 - scrambler
public:	
	enum {
		NO_SCRAMBLE = 0, 
		DO_SCRAMBLE ,
		TAIL_SCRAMBLE
	};
};

class CF_Descramber
{
	FACADE_FIELD(UCHAR, byte_reg); // for 802.11b
};

/*****************************************************
	802.11a
*****************************************************/
class CF_11aTxVector {
	// INPUT
	FACADE_FIELD(ushort, frame_length );    // size of the frame payload
	FACADE_FIELD(ulong,  data_rate_kbps );  // data rate in mbps		
	FACADE_FIELD(ulong,  crc32 );		    // CRC32 for the frame 
	FACADE_FIELD(ushort, coding_rate );     // 0 - 1/2; 1 - 2/3; 2 - 3/4
};

class CF_11aSymState {
	FACADE_FIELD(ulong,  symbol_type );
public:
	enum {
		SYMBOL_TRAINING = 0,
		SYMBOL_OFDM_DATA
	};
	
	void Reset () {
		symbol_type() = SYMBOL_TRAINING;
	}
};

class CF_CFOffset
{
    FACADE_FIELD(FP_RAD, CFO_est );
public:
	void Reset () {
		CFO_est() = 0;
	}
};

class CF_Channel_11a
{
    FACADE_FIELD(vcs, ChannelCoeffs, [16]);
public:
    void Reset ()
    {
        memset(ChannelCoeffs(), 0, sizeof(ChannelCoeffs()));
    }
};

class CF_FreqCompensate
{
    FACADE_FIELD(vcs, Coeffs, [16]);
public:
    void Reset ()
    {
        memset(Coeffs(), 0, sizeof(Coeffs()));
    }
};

class CF_PhaseCompensate
{
    FACADE_FIELD(FP_RAD, CFO_comp );
    FACADE_FIELD(FP_RAD, SFO_comp );

	FACADE_FIELD(vcs, CompCoeffs, [16]);	
public:
	CF_PhaseCompensate () { Reset (); }
	void Reset ()
	{
		CFO_comp() = SFO_comp() = 0;
		memset ( CompCoeffs(), 0, sizeof(CompCoeffs()));		
		for ( int i=0; i<16; i++) {
			CompCoeffs()[i][0].re = CompCoeffs()[i][1].re = 
			CompCoeffs()[i][2].re = CompCoeffs()[i][3].re = 0x7fff; 		
		}
	}	
};

class CF_PilotTrack
{
    FACADE_FIELD(FP_RAD, CFO_tracker );
    FACADE_FIELD(FP_RAD, SFO_tracker );
	// the index of the OFDM symbol being processed
	// used by pilot tracking
	FACADE_FIELD(ulong,  symbol_count );   // symbol_counter
public:
	void Reset () {
		CFO_tracker() = SFO_tracker() = 0;
		symbol_count() 		= 127; 		// PLCP Header
	}
};

class CF_11aRxVector {
	// frame properties
	FACADE_FIELD(ushort, frame_length );   // size of the frame payload
	FACADE_FIELD(ushort, total_symbols );  // number of total OFDM symbols 	
	FACADE_FIELD(ushort, remain_symbols ); // number of remaining OFDM symbols 
										   // in frame
	FACADE_FIELD(ulong,  data_rate_kbps ); // data rate in kbps		
    FACADE_FIELD(ushort, code_rate );	   // coding rate: 1/2, 2/3, 3/4 		   

	FACADE_FIELD(ulong,  crc32 );		   // CRC32 for the frame 	

public:
	void Reset () {
		frame_length() 		= 0;
		total_symbols()     = 0;
		remain_symbols() 	= 0;
		data_rate_kbps() 	= 6000;
		code_rate()         = CR_12;	// 1/2 code rate
		crc32() 			= 0;
	}
	
	CF_11aRxVector () {
		Reset ();	
	}
};

// Helper class for performance-counter-based SoraStopwatch, without parameter contructor
struct SoraPerformanceCounter: public SoraStopwatch
{
    SoraPerformanceCounter() : SoraStopwatch(false) { }
};

class CF_TimeStamps {
    FACADE_FIELD(SoraPerformanceCounter, decoding_data_stopwatch);
};

/*****************************************************
	802.11n
*****************************************************/
class CF_11nTxVector {
	FACADE_FIELD(ushort, frame_length );    // size of the frame payload, not including CRC32
	FACADE_FIELD(ulong,  crc32 );		    // CRC32 for the frame
	FACADE_FIELD(ushort, coding_rate );     // 0 - 1/2; 1 - 2/3; 2 - 3/4
    FACADE_FIELD(ushort, mcs_index );       // MCS index
};

class CF_HTRxVector {
    FACADE_FIELD(ushort, ht_frame_length );
    FACADE_FIELD(ulong,  ht_frame_mcs );
};

class CF_11nSymState {
	FACADE_FIELD(ulong,  symbol_type );
public:
	enum {
		SYMBOL_L_STF = 0,
		SYMBOL_L_LTF,
        SYMBOL_SIG, // L_SIG+HT_SIG
        SYMBOL_HT_STF,
        SYMBOL_HT_LTF,
		SYMBOL_OFDM_DATA
	};
	
	void Reset () {
		symbol_type() = SYMBOL_L_LTF;
	}
};

class CF_FreqOffset_11n
{
    FACADE_FIELD(vs, vfo_delta_i);
    FACADE_FIELD(vs, vfo_step_i);
    FACADE_FIELD(vs, vfo_theta_i);
};

typedef vcs Dot11aChannelCoefficient[16];
class CF_Channel_11n
{
    FACADE_FIELD(vcs, dot11a_siso_channel_1, [16]);
    FACADE_FIELD(vcs, dot11a_siso_channel_2, [16]);
};

typedef A16 COMPLEX16 MIMO_2x2_H[2][128];
class CF_ChannelMimo
{
    FACADE_FIELD(MIMO_2x2_H, dot11n_2x2_channel_inv);
    FACADE_FIELD(MIMO_2x2_H, dot11n_2x2_channel);
};
