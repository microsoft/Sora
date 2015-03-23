#pragma once

#include "umxsdr.h"
#include "uwdm.h"
#include "bb/bba.h"
#include "../../../dot11a/inc/bb/mod/removedc.h"
#include <crc32.h>
#include <dspcomm.h>

#define _BB20M_ 1

#include "fb11bmod_config.hpp"
#include "fb11bdemod_config.hpp"
#include "fb11amod_config.hpp"


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

/*********************************************************************
	ModBuffer11b - modulate a buffer with the given data rate
*********************************************************************/
FINL
bool ModBuffer11b ( UCHAR * whdr, int hdrsize, UCHAR* data, int datasize, 
					ushort rate_kbps,
				    COMPLEX8* sbuf, uint sbuf_size, ULONG* sample_size )
{
	BB11bModCtx.init ( whdr, hdrsize, data, datasize, rate_kbps, sbuf, sbuf_size );
	pBB11bTxSource->Reset ();
	if ( BB11bModCtx.CF_Error::error_code() != BK_ERROR_SUCCESS ) {
		printf ( "Mod graph reset error!\n" );
		return false;
	}	
	// modulation
	while ( pBB11bTxSource->Process () );
	if ( BB11bModCtx.CF_Error::error_code() != BK_ERROR_SUCCESS ) return false;
	*sample_size = BB11bModCtx.CF_TxSampleBuffer::tx_sample_cnt() * sizeof(COMPLEX8);
	return true;
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

	// dump to a file
//	FILE * f = fopen ( "e:\\preamble5.tx", "wb" );
//	fwrite ( sbuf, 1, *sample_size, f );
//	fclose (f);

	return true;
}
