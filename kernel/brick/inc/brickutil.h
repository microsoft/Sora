#pragma once
#include <stdio.h>
#include "sora.h"
#include "dspcomm.h"

/******************************************************************
int LoadSoraDumpFile ( 
	const char* fname,	// name to the dump file
	COMPLEX16 * pbuf,	// memory buffer to hold the samples
	int size			// size of the memory
);

Remarks:
	This function loads a Sora dump file, and returns 
	the number of I/Q samples loaded. 
	If pbuf is NULL, the function only returns the number of samples 
	in the file.
	When return negative, an error occurs.
******************************************************************/
FINL
int LoadSoraDumpFile ( const char* fname, COMPLEX16 * pbuf, int size )
{
    const int sora_samples_per_block = vcs::size * 7;
    const int sora_sample_size = sizeof(COMPLEX16);
    
    if ( fname == NULL ) 
		return BK_ERROR_INVALID_PARAM;

#pragma warning (push)
#pragma warning (disable:4996)
	FILE *fin = fopen(fname, "rb");
#pragma warning (pop)
	if (!fin) return BK_ERROR_FILE_NOT_FOUND;
		
	fseek (fin, 0, SEEK_END);
	int fsize = ftell (fin );

	int nExpSamples = size / sora_sample_size;

    // If the output buffer is not specified, just return the expected buffer size
	if ( pbuf == NULL ) {
		fclose (fin);
        return ceil_div(fsize, sizeof(RX_BLOCK)) * sora_samples_per_block;
	}

	COMPLEX16* pLoad = pbuf;
	
	fseek ( fin, 0, SEEK_SET);
	while (!feof(fin) && nExpSamples > 0 ) {
		fseek ( fin, 16, SEEK_CUR );
		int inc = fread ( pLoad, sizeof(COMPLEX16), min(nExpSamples, sora_samples_per_block), fin);
		pLoad += inc;
		nExpSamples -= inc;
	}
		
	fclose(fin);
	return (pLoad - pbuf);
}
