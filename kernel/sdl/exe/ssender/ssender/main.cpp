#include <stdio.h>
#include <conio.h>

#include <sora.h>
#include <sdl.hpp>
#include <DebugPlotU.h>

// Graph for 11a
#include <graph11a.h>


TIMESTAMPINFO tsinfo;

int        g_RID = 0;
CSoraRadioConfig g_RadioCfg;
CSoraRadio		 g_Radio;

// Tx graph
CTxGraph11a      g_11aTx;

//
int delay = 100;

char * default_msg = "HELLO WORLD!";
char   file_msg[2000];

long   msg_len = 12;
char * msg = default_msg;

int UMX_main () {
	if ( !g_Radio.CreateRadio (g_RID) ) {
		printf ( "Fail to create a radio!\n" );
		return 0;
	}

	g_Radio.ConfigureRadio ( g_RadioCfg );

	CSoraSignalBuf txbuf;
	if ( !txbuf.Allocate ( 2*1024*1024 ) ) {
		printf ( "Fail to allocate a tx buffer!\n" );
		return 0;
	}

	// Tx Graph
	if ( !g_11aTx.CreateGraph () ) {
		printf ( "Fail to create Tx graph!\n" );
		return 0;
	}

	// Modulate
	ULONG slen;
	if ( g_11aTx.Modulate ( (UCHAR*) msg, msg_len, NULL, 0, 6000, (COMPLEX8*)txbuf.GetBuffer (), txbuf.GetBufSize (), &slen ) ) {
		txbuf.SetSampleLength (slen);
	}

	// Transfer
	CSoraSignal txsig;
	g_Radio.Transfer ( txbuf, txsig );

	// Tx
	printf ( "Create a radio at %d... start sending..\n", g_RID );
	printf ( "Press x to exit...\n" );
	int input;
	while (1) {
		g_Radio.Transmit (txsig);
		input = SoraGetConsoleKey ();
		if ( input == 'x' ) break;
		Sleep (delay);
	}		

	return 0;
}

bool ParseCommandLine ( int argc, const char* argv[] ) {
	if ( argc < 2 ) // No command line
		return false;

	int index = 1;
	while ( index < argc ) {
		// process radio configuration
		int ret = g_RadioCfg.ParseCommandLine (index, argc, argv );
		if ( ret > 0 ) continue;
		else if ( ret < 0 ) return false;
		
		// addition command line
		if ( argv[index][0] == '-' 
				&& argv[index][1] == 'r' ) 
		{
			if ( ++ index < argc ) {
				char* stopstr;
				g_RID = strtol ( argv[index++], &stopstr, 0 ); 
				if ( g_RID < 0 || g_RID > 3 )
					return false;
			} else {
				return false;
			}
		} 
		else // -h
		if ( argv[index][0] == '-' 
				&& argv[index][1] == 'h' ) {
			return false;
		}
		else // -d 
		if ( argv[index][0] == '-' 
				&& argv[index][1] == 'd' ) 
		{
			if ( ++ index < argc ) {
				char* stopstr;
				delay = strtol ( argv[index++], &stopstr, 0 ); 
			} else {
				return false;
			}
		} 
		else // -m
		if ( argv[index][0] == '-' 
				&& argv[index][1] == 'm' ) 
		{
			if ( ++ index < argc ) {
				FILE* fin = fopen ( argv[index], "rb" );
				if ( fin == NULL ) return false;

				long len;
				fseek ( fin, 0, SEEK_END );
				len = ftell (fin);

				if ( len > 1500 ) {
					len = 1500;
				}
				fseek ( fin, 0, SEEK_SET );
				msg_len = fread ( file_msg, 1, len, fin );
				fclose ( fin);
				index ++;
			} else {
				return false;
			}
		} 
		else // unknown
		{
			return false;
		}
	}
	return true;
}

void Usage () {
	printf ( "ssender usage\n" );
	printf ("   -r rid         -    The Radio ID\n" );
	printf ( "%s", g_RadioCfg.usage ());	
	printf ("   -d delay       -    Interval between two transmissions (ms)\n" );
	printf ("   -m file        -    A text message to send!\n" );
	printf ("   -h             -    Display this page\n" );
}

int _cdecl main(int argc, const char* argv[])
{
	if ( !ParseCommandLine ( argc, argv )) {
		Usage ();
		return 0;
	}

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

