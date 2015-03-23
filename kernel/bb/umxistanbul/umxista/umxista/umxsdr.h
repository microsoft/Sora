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

#define IFFT_MAG_SHIFT	0

// #define TARGET_RADIO	0
extern int TARGET_RADIO;

#define _K(N)			(1024 * (N))
#define _M(N)			(1024 * 1024 * (N))


#define INPUTBUF_SIZE		(_M(16))
#define FrameBufferSize		(_M(1))

// global configuration
#define  DISPLAY_PAGE 0
#define  DISPLAY_LINE 1


#define  PHY_802_11B 0
#define  PHY_802_11A 1

extern int      gDisplayMode;
extern int      gPHYMode;    // indicate the PHY mode
extern int 		gSampleRate;
extern int 		gDataRateK;
extern ULONG 	gChannelFreq;
extern int      bWbx;
extern int      gCCAThreshold;
extern int      gDoubleDeci;

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
extern PVOID				SampleBuffer;
extern ULONG				SampleSize; // 4MB

// Rx sample buffer
extern PVOID				RxBuffer;
extern ULONG				RxBufferSize;

// Ack sample buffer
extern PVOID				AckBuffer; 
extern ULONG 				AckSize;
extern TIMESTAMPINFO        tsinfo;
extern ULONGLONG            tts1, tts2;

