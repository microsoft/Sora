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
#include "specanalyzer.h"


CMonitor monitor;

ISource* CreateGraph () {
	CREATE_BRICK_SINK  (drop,  TDropAny,     monitor );

   	CREATE_BRICK_SINK  (pl1,   TPlotLine<1>::Sink,     monitor );
    pl1->SetName ( "Average power" );
	CREATE_BRICK_FILTER(ave,  TPowerMeter<280*3>::Filter, monitor, pl1 );    
	CREATE_BRICK_FILTER(dc,   TDCRemoveEx<4>::Filter, monitor, ave );    

    
   	CREATE_BRICK_SINK  (ps1,   TPlotSpec<128>::Sink,     monitor );
    ps1->SetName ( "Spectrum" );
	CREATE_BRICK_FILTER(spec,   TSpecMeter<128>::Filter, monitor, ps1 );   

   	CREATE_BRICK_SINK  (pd1,   TPlotDots<28>::Sink,     monitor );
    pd1->SetName ( "Constellation" );


    CREATE_BRICK_DEMUX3(tee, TFourWay<28>::Filter, monitor,
                            dc, spec, pd1 );
    
	CREATE_BRICK_SOURCE(rxsrc, TRxStream,    monitor, tee );	
    

    return rxsrc;
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

