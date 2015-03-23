#include <stdio.h>
#include <conio.h>

#include <sora.h>
#include <soratime.h>
#include <thread_func.h>
#include <DebugPlotU.h>

#include <sora_radio.h>

TIMESTAMPINFO tsinfo;

int        g_RID = 0;
CSoraRadio g_Radio;

int UMX_main () {
	if ( !g_Radio.CreateRadio (0) ) {
		printf ( "Fail to create a radio!\n" );
		return 0;
	}

	printf ( "Create is created successfully... press any key to exit...\n" );
	getchar ();

	return 0;
}


int _cdecl main(int argc, const char* argv[])
{
	InitializeTimestampInfo(&tsinfo, false);
	DebugPlotInit();

	
	if (!SoraUInitUserExtension("\\\\.\\HWTest")) {
		printf ( "Error: fail to find the hwtest driver!\n" );
		return -1;
	}

	int nRet = UMX_main ();

	SoraUCleanUserExtension();
	DebugPlotDeinit();

	return nRet;
	return 0;
}

