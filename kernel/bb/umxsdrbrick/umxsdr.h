#pragma once
#include <sora.h>
#include "alinew.h"	// aligned new header file

#include "errmsg.h"
#include <brick.h>
#include <dspcomm.h>
#include <soratime.h>

#include "stdbrick.hpp"


// involve dbgplot API
#include "DebugPlotU.h"
#include "mac.h"

#define IFFT_MAG_SHIFT	0

#define _K(N)			(1024 * (N))
#define _M(N)			(1024 * 1024 * (N))


#define INPUTBUF_SIZE		(_M(16))
#define FrameBufferSize		(_M(1))

// global configuration
#define  DISPLAY_PAGE 0
#define  DISPLAY_LINE 1


#define  PHY_802_11B 0
#define  PHY_802_11A 1
#define  PHY_802_11N 2

extern int      gDisplayMode;
extern int      gPHYMode;    // indicate the PHY mode
extern int 		gSampleRate;
extern int 		gDataRateK;
extern ULONG 	gChannelFreq;

FINL size_t GetNStream()
{
    size_t nstream;
    if (gPHYMode == PHY_802_11A || gPHYMode == PHY_802_11B) {
        nstream = 1;
    } else if (gPHYMode == PHY_802_11N) {
        nstream = 2;
    } else {
        assert(0);
    }
    return nstream;
}

FINL
ULONG GetChannelNo ( ULONG freq ) {
	if ( freq < 5000 ) {
		return ((freq - 2412) / 5 + 1  );
	} else {
		return ((freq - 5180) / 5 + 36 );
	}
}


// debug flag
extern ULONG 				debug;

// flag control viterbi thread
extern FLAG					fWork;

// MPDU buffer
extern UCHAR				FrameBuffer[FrameBufferSize];

// Modulation sample buffer
extern PVOID				SampleBuffer[MAX_RADIO_NUMBER];
extern ULONG				SampleSize; // 4MB

// Rx sample buffer
extern PVOID				RxBuffer[MAX_RADIO_NUMBER];
extern ULONG				RxBufferSize;

// Ack sample buffer
extern PVOID				AckBuffer[MAX_RADIO_NUMBER]; 
extern ULONG 				AckSize;
extern TIMESTAMPINFO        tsinfo;
extern ULONGLONG            tts1, tts2;

SELECTANY ULONG RadioNo[MAX_RADIO_NUMBER] = { 0, 1, 2, 3, 4, 5, 6, 7 }; // RadioNo used by 802.11n
SELECTANY ULONG TARGET_RADIO = RadioNo[0]; // RadioNo used by 802.11a/b, WARNING: don't change because of imp of TransferBuffers()

FINL void SaveDataToFile(const char *fname, void *p, size_t size)
{
    FILE *f = fopen(fname, "wb");
    fwrite(p, size, 1, f);
    fclose(f);
}

FINL HRESULT TransferBuffers(OUT PACKETxID *ptid, PVOID *bufs, size_t count, ULONG size)
{
    assert(ptid);
    HRESULT hr = S_OK;
    size_t iss;
    for (iss = 0; iss < count; iss++) {
        hr = SoraURadioTransferEx(ptid->m_tid.m_radiono[iss], bufs[iss], size, &ptid->m_tid.m_txid[iss]);
        ptid->m_tid.m_radiono[iss] = RadioNo[iss];
        if (FAILED(hr)) return hr;
    }
    ptid->m_tid.m_count = count;
	ptid->m_status = PACKET_CAN_TX;
    return hr;
}

FINL void FreeTxResource(TxIDs *ptid)
{
    assert(ptid);
    size_t iss;
    for (iss = 0; iss < ptid->m_count; iss++) {
        SoraURadioTxFree(ptid->m_radiono[iss], ptid->m_txid[iss]);
    }
}
