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
#include "sine.h"

CSineSource sine;

ISource* CreateGraph () {
   	CREATE_BRICK_SINK  (msink, TModSink, sine );
	CREATE_BRICK_FILTER (pack, TPackSample16to8, sine, msink );
	typedef TSine<2000, 0, 8000, 40000> TSine1MHz;
	CREATE_BRICK_SOURCE (ssine, TSine1MHz::Source, sine, pack );

	return ssine;
}


