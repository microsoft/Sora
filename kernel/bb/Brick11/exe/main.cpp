#pragma warning(disable:4530)

#include <windows.h>

#include <stdio.h>
#include <conio.h>

#include <sora.h>
#include <dspcomm.h>
#include <soratime.h>

#include "DebugPlotU.h"

#define TARGET_RADIO 0

TIMESTAMPINFO tsinfo;

SORA_RADIO_RX_STREAM	RxStream;	// RxStream

PVOID				RxBuffer = NULL;	// Mapped Rx Buffer
ULONG				RxBufferSize = 0;


HANDLE 				hTestThread;

uint 				lagcnt = 0;
uint 				flagmod = true;
	
void usage () {
	printf ( "brkmod11.exe -[h|m|d]\n" );
}

bool ParseCmdLine ( int argc, const char* argv[] ) {
	int i=1;
	if (argc < 2 ) return false;
	
	while ( i< argc ) {
		if ( argv[i][0] == '-'
			&& argv[i][1] == 'h')
		{
			return false;
		} else if ( argv[i][0] == '-'
			&& argv[i][1] == 'm') 
		{
			flagmod = true;
		} else if ( argv[i][0] == '-'
			&& argv[i][1] == 'd') 
		{
			flagmod = false;
		}		
		else {
			return false;
		}
		i++;
	}
	return true;
}

int Test11B_FB_Mod ();
int Test11B_FB_Demod ();

int Test11A_FB_Mod ();
int Test11A_FB_Demod ();


bool running = false;

int __cdecl main(int argc, const char *argv[])
{
	if ( ParseCmdLine (argc, argv) == false )
	{
		usage();
		return 0;
	}

	// init DebugPlot library
	DebugPlotInit();

	InitializeTimestampInfo ( &tsinfo, false );
	tsinfo.use_rdtsc = 0;

	running = true;

	
	if ( flagmod ) {
		Test11A_FB_Mod ();
	} else {
		Test11A_FB_Demod ();
	}
	
	DebugPlotDeinit();
    return 0;
}
