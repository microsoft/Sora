// we define the standard facade here
#pragma once
#include <brick.h>
#include <sora.h>
#include <dspcomm.h>

// #include "obselete_facade.h" // todo: remove it later

#define E_ERROR_SUCCESS         0x00000000
#define E_ERROR_PARAMETER 		0x80000001
#define E_ERROR_FAILED          0x8000FFFF

// Facade definition on the stock bricks
class CF_Error {
	FACADE_FIELD(ulong, error_code );	  /* error code for the graph */
public:
	CF_Error () { error_code () = E_ERROR_SUCCESS; };
};

class CF_MemSamples {
	FACADE_FIELD (COMPLEX16*, mem_sample_buf);
	FACADE_FIELD (uint,       mem_sample_buf_size);	 // in unit of bytes
	FACADE_FIELD (uint,       mem_sample_start_pos); // in unit of samples
	FACADE_FIELD (uint,       mem_sample_count);     // # of samples in buffer
	FACADE_FIELD (uint,       mem_sample_index);     // index of sample being processed
public:
	FINL void Init ( COMPLEX16* sbuf, uint sbuf_size )
	{
		mem_sample_buf() = sbuf;
		mem_sample_buf_size() = sbuf_size;
		mem_sample_start_pos() = 0;
		mem_sample_count() = sbuf_size / sizeof(COMPLEX16);
		mem_sample_index() = 0;
	}
};

class CF_TxSampleBuffer {
	// INPUT
	FACADE_FIELD(COMPLEX8*, tx_sample_buf );	  	
	FACADE_FIELD(uint,      tx_sample_buf_size ); 
	// OUTPUT	
	FACADE_FIELD(uint,      tx_sample_cnt ); 
};


class CF_RxStream {
	FACADE_FIELD (SORA_RADIO_RX_STREAM*, rxstream_pointer );
	FACADE_FIELD (FLAG,                  rxstream_touched );
};

class CF_VecDC {
	FACADE_FIELD (vcs, direct_current );
public:
	FINL void Reset () {
		set_zero (direct_current());
	}
};

class CF_AvePower {
	FACADE_FIELD (uint,  average_power );
};

class CF_DCRemove {
	FACADE_FIELD (COMPLEX16,  average_DC );
	FACADE_FIELD (uint,  DC_update_lock );
};
// end of facade definition on the stock bricks



