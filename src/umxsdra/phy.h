#pragma once

#include "umxsdr.h"
#include "uwdm.h"
#include "bb/bba.h"
#include "../dot11a/inc/bb/mod/removedc.h"
#include <crc32.h>
#include <dspcomm.h>
#include "fb11amod_config.hpp"


extern BB11A_RX_CONTEXT		RxContext;
extern SORA_RADIO_RX_STREAM	RxStream;

#pragma warning(disable:4345)

FINL
ULONG CalcMDLCRC32(MDL* mdl) {

	ULONG crc32;
	crc32 = 0xffffffff;
	while(mdl) {		
		PUCHAR p;
		ULONG i;		
		for(p=(PUCHAR)mdl->StartVa+mdl->ByteOffset, i=0; i<mdl->ByteCount; i++)
			CalcCRC32Incremental(p[i], &crc32);	
		mdl = mdl->Next;
	};
	return ~crc32;
}

FINL
void PrepareRxContext()
{
	BB11ARxContextInit(
		&RxContext,
        gSampleRate,
        250000,                                     // rxThreshold
        INPUTBUF_SIZE / sizeof(RX_BLOCK),           // rxMaxBlockCount
        112,                                        // rxMinBlockCount
        (PFLAG)&fWork);

	ResetDC(&RxContext);
}

/*********************************************************************
	ModBuffer11a - modulate a buffer with the given data rate
*********************************************************************/
FINL
bool ModBuffer11a ( UCHAR * whdr, int hdrsize, UCHAR* data, int datasize, 
					ushort rate_kbps,
				    COMPLEX8* sbuf, uint sbuf_size, ULONG* sample_size )
{
    // Generate signals for fixed preamble
	COMPLEX8* pSymBuf = sbuf;

	BB11aModCtx.set_mod_buffer (pSymBuf, sbuf_size );
	pBB11aPreambleSource->Reset ();
	if ( BB11aModCtx.CF_Error::error_code() == E_ERROR_SUCCESS ) {
		pBB11aPreambleSource->Process ();
	}	
	uint TS_len = BB11aModCtx.CF_TxSampleBuffer::tx_sample_cnt();
	pSymBuf += TS_len;

    // Generate signals for PLSC, data and CRC
	BB11aModCtx.init ( whdr, hdrsize, data, datasize, rate_kbps, pSymBuf, sbuf_size-TS_len );
	pBB11aTxSource->Reset ();
	if ( BB11aModCtx.CF_Error::error_code() != BK_ERROR_SUCCESS ) {
		printf ( "Mod graph reset error!\n" );
		return false;
	}	
	// modulation
	while ( pBB11aTxSource->Process () );
	if ( BB11aModCtx.CF_Error::error_code() != BK_ERROR_SUCCESS ) return false;
	*sample_size = (BB11aModCtx.CF_TxSampleBuffer::tx_sample_cnt() + TS_len) * sizeof(COMPLEX8);
	return true;
}
