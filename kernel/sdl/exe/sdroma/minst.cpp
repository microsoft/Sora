#include "common.h"

static HINSTANCE hMainInstance = 0;
static const char * PROGRAM_NAME = "Sora Oscillometer";
static const int WIN_WIDTH  = 830;
static const int WIN_HEIGHT = 650;

static const char * INFO_WND_NAME = "Sora Info";
static const int INFO_WND_HEIGHT = 800;
static const int INFO_WND_WIDTH  = 600;


void setMainInstance(HINSTANCE hi)
{
    hMainInstance = hi;
}

HINSTANCE getMainInstance()
{
    return hMainInstance;
}

const char * getClassName()
{
    return PROGRAM_NAME;
}

static char title[200];
const char * getWinTitle()
{
	if ( flagMonitorMode ) {
		if ( g_resample44_40 )
			sprintf ( title, "%s - %d-%d/%d @ radio %d - playback speed %d (monitor mode)", PROGRAM_NAME, g_samplingrate, g_samplingrate*40/44, g_decimation, TARGET_RADIO, getSpeedLevel() );
		else
			sprintf ( title, "%s - %d/%d @ radio %d - playback speed %d (monitor mode)", PROGRAM_NAME, g_samplingrate, g_decimation, TARGET_RADIO, getSpeedLevel() );
	}
	else { 
		if ( g_resample44_40 )
			sprintf ( title, "%s - %d-%d/%d @ radio %d - playback speed %d", PROGRAM_NAME, g_samplingrate, g_samplingrate*40/44, g_decimation, TARGET_RADIO, getSpeedLevel() );
		else
			sprintf ( title, "%s - %d/%d @ radio %d - playback speed %d", PROGRAM_NAME, g_samplingrate, g_decimation, TARGET_RADIO, getSpeedLevel() );
	}
    return title;
}

int getWinHeight()
{
    return WIN_HEIGHT;
}

int getWinWidth()
{
    return WIN_WIDTH;
}
