#pragma once

#include "umxsdr.h"
#include "uwdm.h"
#include "bb/bba.h"
#include "../dot11a/inc/bb/mod/removedc.h"
#include <crc32.h>
#include <dspcomm.h>

#include "fb11bmod_config.hpp"
#include "fb11amod_config.hpp"
#include "fb11nmod_config.hpp"

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
	return true;
}

/*********************************************************************
	ModBuffer11n - modulate a buffer with the given data rate
*********************************************************************/
FINL
bool ModBuffer11n ( UCHAR * whdr, int hdrsize, UCHAR* data, int datasize, 
					ushort mcs_index,
				    COMPLEX16* sbuf[], uint sbuf_size, ULONG* sample_size )
{
    // Generate signals for L_STF + L_LTF
	BB11nModCtx.init ( whdr, hdrsize, data, datasize, mcs_index, sbuf, sbuf_size );
	pBB11nLPreambleSource->Reset();
	if (BB11nModCtx.CF_Error::error_code() == E_ERROR_SUCCESS) {
		pBB11nLPreambleSource->Process();
	}

    // Generate signals for L_SIG + HT_SIG
    pBB11nSigSource->Reset();
    if (BB11nModCtx.CF_Error::error_code() == E_ERROR_SUCCESS) {
        pBB11nSigSource->Process();
    }

    // Generate signals for HT_STF + HT_LTF
    pBB11nHtPreambleSource->Reset();
    if (BB11nModCtx.CF_Error::error_code() == E_ERROR_SUCCESS) {
        pBB11nHtPreambleSource->Process();
    }

	pBB11nTxSource->Reset ();
	if ( BB11nModCtx.CF_Error::error_code() != BK_ERROR_SUCCESS ) {
		printf ( "Mod graph reset error!\n" );
		return false;
	}	
	// modulation
	pBB11nTxSource->Process();
    pBB11nTxSource->Flush();
	if ( BB11nModCtx.CF_Error::error_code() != BK_ERROR_SUCCESS ) return false;
	*sample_size = BB11nModCtx.GetSinkSampleCount() * sizeof(COMPLEX16);
	return true;
}
