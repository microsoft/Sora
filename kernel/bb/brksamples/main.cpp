#pragma warning(disable:4530)

#include <windows.h>

#include <stdio.h>
#include <conio.h>

#include "DebugPlotU.h"

#include "mac.h"

#define TARGET_RADIO 0

TIMESTAMPINFO tsinfo;

SORA_RADIO_RX_STREAM	RxStream;	// RxStream

PVOID				RxBuffer = NULL;	// Mapped Rx Buffer
ULONG				RxBufferSize = 0;


HANDLE 				hRxThread;

uint 				lagcnt = 0;
	
void usage () {
	printf ( "brktest.exe \n" );
}

bool ParseCmdLine ( int argc, const char* argv[] ) {
	int i=1;
	while ( i< argc ) {
		if ( argv[i][0] == '-'
			&& argv[i][1] == 'h')
		{
			return false;
		}
		else {
			return false;
		}
	}
	return true;
}

HANDLE StartThread ( PSORA_UTHREAD_PROC proc, PVOID pParam ) 
{
	HANDLE hThread = SoraUThreadAlloc();
	if (!hThread) {
		return NULL;
	}
	
	if (!SoraUThreadStart(hThread, proc, pParam)) {
		SoraUThreadStop(hThread);
		SoraUThreadFree(hThread);

		return NULL;
	}
	return hThread;
}

void ConfigureRadio () 
{

	// Start Radio
	SoraURadioStart (TARGET_RADIO);

	Sleep (10);
	
	SoraURadioSetCentralFreq (TARGET_RADIO, 2422 * 1000);
		
	// Set Tx Gain to a fixed value
	SoraURadioSetTxGain ( TARGET_RADIO, 0x1800 ); // 21 dBm
	
	// Set the Receiver Gain
	SoraURadioSetRxPA	(TARGET_RADIO, 0x2000);  // 16dB
	SoraURadioSetRxGain (TARGET_RADIO, 0xa00);  // 2G dB

}

void print_status () {
//	printf ( "lag count %d\n", lagcnt );
}

void process_kb () {
	while(1)
		if ( _kbhit () ) {
			switch(_getch()) {
			case '\3':  // _getch() will capture Ctrl+C as '\3'
			case 'x':
			case 'X':
				return;
			default:
				break;
			}
		} else {
			Sleep (100);
			print_status ();
		}
}


int __cdecl main(int argc, const char *argv[])
{
	HRESULT hr;
	
	if ( ParseCmdLine (argc, argv) == false )
	{
		usage();
		return 0;
	}

	// init DebugPlot library
	DebugPlotInit();

	InitializeTimestampInfo ( &tsinfo );

	// begin receive or transmit packets
	if (!SoraUInitUserExtension("\\\\.\\HWTest")) {
		return -1;
	}

	// Map Rx Sample Buffer
	hr = SoraURadioMapRxSampleBuf(TARGET_RADIO, &RxBuffer, &RxBufferSize);
	if (FAILED(hr)) {
		printf ( "Fail to map rx buffer!\n" );
		return 0;
	}	
		

	hr = SoraURadioAllocRxStream(&RxStream, TARGET_RADIO, (PUCHAR)RxBuffer, RxBufferSize);
	if (FAILED(hr)) {
		printf ( "Fail to allocate a RX stream!\n" );
		goto error_exit;
	}
		
	// configure radio parameters properly
	ConfigureRadio ();

	CreateBasebandGraph ();
	
	// Start all threads
	hRxThread = StartThread ( RxProc, NULL);
	if (hRxThread == NULL ) {
		printf("failed to start tx thread\n");
		goto error_exit;
	}
		
	// enter the message loop
	process_kb ();
			
error_exit:	
	ReleaseBasebandGraph (); 
	
		
	SoraURadioReleaseRxStream(&RxStream, TARGET_RADIO);
		
	if (RxBuffer) {
		hr = SoraURadioUnmapRxSampleBuf(TARGET_RADIO, RxBuffer);
	}

	SoraUCleanUserExtension();
	DebugPlotDeinit();

    return 0;
}
