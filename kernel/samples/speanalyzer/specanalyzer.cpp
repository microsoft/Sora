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
	CREATE_BRICK_FILTER(lpass,  THanningWin<16>::Filter, monitor, spec );   

	uint cutoff[2] = {10, 0};
	lpass->config ( TYPE_LOWPASS, cutoff, 40);

   	CREATE_BRICK_SINK  (pd1,   TPlotDots<28>::Sink,     monitor );
    pd1->SetName ( "Constellation" );


    CREATE_BRICK_DEMUX3(tee, TFourWay<28>::Filter, monitor,
                            dc, lpass, pd1 );
    
	CREATE_BRICK_SOURCE(rxsrc, TRxStream,    monitor, tee );	
    

    return rxsrc;
}

