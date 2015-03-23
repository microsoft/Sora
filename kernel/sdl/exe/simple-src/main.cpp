#pragma warning(disable:4530)

#include <windows.h>

#include <stdio.h>
#include <conio.h>
#include <sora.h>
#include <brick.h>
#include <dspcomm.h>
#include <soratime.h>
#include <debugplotu.h>

#include "radioinfo.h"
#include "sine.h"


// performance monitor
TIMESTAMPINFO        tsinfo;
ULONGLONG            tts1, tts2;

SORA_RADIO_RX_STREAM	RxStream;	    // RxStream
PVOID				RxBuffer = NULL;	// Mapped Rx Buffer
ULONG				RxBufferSize = 0;

PVOID				SampleBuffer = NULL;	// Mapped Tx Buffer	
ULONG				SampleSize = 4*1024*1024; 

void usage () {
	printf ( "Sine cmd parameter \n" );
	printf ( "--------------------------------------------------\n" );
	printf ( "   -f  freq     -    central frequency (MHz)\n" );
	printf ( "   -h           -    display this page\n" );
}

void ConfigureRadio ();
void process_kb ();


bool ParseCmdLine ( int argc, const char* argv[] ) {
	int i=1;
	while ( i< argc ) {
		if ( argv[i][0] == '-' 
			 && argv[i][1] == 'f' ) 
		{
			if ( ++i < argc ) {
				gRadioInfo.CentralFreq () = atoi(argv[i++]);
			} else {
				return false;
			}
		} else if ( argv[i][0] == '-'
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

extern CSineSource sine;

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

    // init sora timestamp library
	InitializeTimestampInfo ( &tsinfo, false );

    do {
    	// begin receive or transmit packets
    	if (!SoraUInitUserExtension("\\\\.\\HWTest")) {
    		printf ( "Error: fail to find the hwtest driver!\n" );
    		break;
    	}

        // start UMX
        // Map Rx Sample Buffer
        hr = SoraURadioMapRxSampleBuf(TARGET_RADIO, &RxBuffer, &RxBufferSize);
        if (FAILED(hr)) {
            printf ( "Fail to map rx buffer!\n" );
       		break;
        }   
    
        // Alloc Tx Sample Buffer
        SampleBuffer = SoraUAllocBuffer(SampleSize);
        if (!SampleBuffer) {
            printf ( "Fail to allocate Tx buffer!\n" );
            break;
        }   
    
        hr = SoraURadioAllocRxStream(&RxStream, TARGET_RADIO, (PUCHAR)RxBuffer, RxBufferSize);
        if (FAILED(hr)) {
            printf ( "Fail to allocate a RX stream!\n" );
            break;
        }

        printf ( "Configure radio...\n" );
        
        // configure radio parameters properly
        ConfigureRadio ();

		if ( sine.Create ( SampleBuffer, SampleSize ) ) {
			printf ("Sine source starts...\n" );
			sine.Start ();
		}
      
        // enter the message loop
        process_kb (); 
        
    }    while (false);

    
    sine.Stop ();
        
    SoraURadioReleaseRxStream(&RxStream, TARGET_RADIO);
    
    if (SampleBuffer) {
        SoraUReleaseBuffer(SampleBuffer);
        SampleBuffer = NULL;
    }
    
    if (RxBuffer) {
        hr = SoraURadioUnmapRxSampleBuf(TARGET_RADIO, RxBuffer);
    }
    
	SoraUCleanUserExtension();
	DebugPlotDeinit();

    return 0;
}

void ConfigureRadio () {
    
	// Start Radio
	SoraURadioStart (TARGET_RADIO);

	Sleep (10);
	
	SoraURadioSetCentralFreq (TARGET_RADIO, gRadioInfo.CentralFreq() * 1000);
		
	// Set Tx Gain to a fixed value
	SoraURadioSetTxGain ( TARGET_RADIO, gRadioInfo.TxGain () );
    
	// Set the Receiver Gain
	SoraURadioSetRxPA	(TARGET_RADIO, gRadioInfo.RxPA()   );  
	SoraURadioSetRxGain (TARGET_RADIO, gRadioInfo.RxGain() );   

}


// Process KB events
void process_kb () {
    printf ( "Press 'x' to exit...\n" );
    
	while(1) {
		if ( _kbhit () ) {
			switch(_getch()) {
			case '\3':  // _getch() will capture Ctrl+C as '\3'
			case 'x':
			case 'X':
				return;
            }    
		} else {
			Sleep (1000);
		}
    }        
}


