#include <windows.h>

#include <stdio.h>
#include <conio.h>
#include <sora.h>
#include <brick.h>
#include <dspcomm.h>
#include <soratime.h>
#include <debugplotu.h>

// some stocked bricks
#include <stdbrick.hpp>

// Sora DSP Library
#include <sdl.hpp>

#include "radioinfo.h"
#include "tx.h"

COFDMTxChain ofdmTx;

ISource* CreateTSGraph () {
    CREATE_BRICK_SINK   ( msink, TModSink, ofdmTx );
    CREATE_BRICK_FILTER ( pack, TPackSample16to8, ofdmTx, msink );
    CREATE_BRICK_SOURCE ( ssrc, TTS11aSrc, ofdmTx, pack ); 

    return ssrc;
}


ISource* CreateModGraph () {
    CREATE_BRICK_SINK	( drop,	TDropAny,  ofdmTx );

    CREATE_BRICK_SINK   ( modsink, 	TModSink, ofdmTx );	
    CREATE_BRICK_FILTER ( pack, TPackSample16to8, ofdmTx, modsink );
    CREATE_BRICK_FILTER ( ifft,	TIFFTx, ofdmTx, pack );
    CREATE_BRICK_FILTER ( addpilot1,T11aAddPilot, ofdmTx, ifft );
    CREATE_BRICK_FILTER ( addpilot, TNoInline, ofdmTx, addpilot1);
    CREATE_BRICK_FILTER ( map_bpsk,		TMap11aBPSK,    ofdmTx, addpilot );
    CREATE_BRICK_FILTER ( inter_bpsk,	T11aInterleaveBPSK::filter,    ofdmTx, map_bpsk );
    CREATE_BRICK_FILTER ( enc6,	TConvEncode_12, ofdmTx, inter_bpsk );
    CREATE_BRICK_FILTER ( sc, T11aSc, ofdmTx, enc6 );
    CREATE_BRICK_SOURCE ( ssrc, TBB11aSrc, ofdmTx, sc ); 

	return ssrc;
}


