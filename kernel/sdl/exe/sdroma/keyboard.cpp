#include "common.h"

static char inputFileNameBuf[2048];

static bool OpenNewFile()
{
    inputFileNameBuf[0] = '\0';
    OPENFILENAME openFileName;
    memset(&openFileName, 0, sizeof(OPENFILENAME));
    openFileName.lStructSize = sizeof(OPENFILENAME);
    openFileName.lpstrFilter = "Dump file(*.dmp)\0*.dmp\0All file(*.*)\0*.*\0\0";
    openFileName.lpstrFile = inputFileNameBuf;
    openFileName.nMaxFile = 1024;
    openFileName.lpstrTitle = "Open Dumped File...";
    openFileName.lpstrDefExt = "dmp";
    openFileName.Flags |= OFN_HIDEREADONLY;

    if (!GetOpenFileName(&openFileName))
        return false;

    loadDumpFile(inputFileNameBuf);
    return true;
}

static bool WriteToFile()
{
    inputFileNameBuf[0] = '\0';
    OPENFILENAME openFileName;
    memset(&openFileName, 0, sizeof(OPENFILENAME));
    openFileName.lStructSize = sizeof(OPENFILENAME);
    openFileName.lpstrFilter = "Dump file(*.dmp)\0*.dmp\0All file(*.*)\0*.*\0\0";
    openFileName.lpstrFile = inputFileNameBuf;
    openFileName.nMaxFile = 1024;
    openFileName.lpstrTitle = "Write Dumped File...";
    openFileName.lpstrDefExt = "dmp";
    openFileName.Flags |= OFN_HIDEREADONLY;

    if (!GetSaveFileName(&openFileName))
        return false;

	printf ( "write to file %s\n", inputFileNameBuf );
    writeDumpFile(inputFileNameBuf);

    return true;
}

void autoGetNewDump();

extern HWND g_hwndMain;
extern int g_sampleRate;

void RotateSampleRate () {

	g_resample44_40 = 1 - g_resample44_40;

	SetWindowText ( gHwnd, getWinTitle () );
}

void RotateDecimation () {
	if ( g_decimation < 8 ) {
		g_decimation = g_decimation << 1;
	} else {
		g_decimation = 1;
	}

	SetWindowText ( gHwnd, getWinTitle () );
}


int   gargc = 0;
char* gargv[128];

void ParseCommand ( char * line ) {
	gargc = 1;
	char * pcmd = strtok (line, " " );
	while (pcmd) {
		gargv[gargc++] = pcmd;
		pcmd = strtok (NULL, " ");
	}

	if ( !ParseCommandLine ( gargc, gargv ) ) {
		printf ( "Warning: command line error! Some parameters are unchanged!\n" );
	}
}

void GetCommand () {
	printf ( "cmd:>" );
	fgets (gcmdline, 512, stdin );	
	gcmdline[511] = 0;
	ParseCommand ( gcmdline );
}

void SwitchRadio (int radio ) {
	if (radio == TARGET_RADIO) {
		return;
	}

	if ( umx_enabled () ) {
		Release_radio (TARGET_RADIO );

		TARGET_RADIO = radio;
		init_radio    (TARGET_RADIO );
		configure_radio (TARGET_RADIO );

		SetWindowText ( gHwnd, getWinTitle () );
	}
}

void IncreaseRxgain () {
	rxgain += 0x200;
	configure_rxgain (TARGET_RADIO);

	printf ( "==>rxgain %0x\n", rxgain );
}

void DecreaseRxgain () {
	if (rxgain >= 0x200 ) {
		rxgain -= 0x200;
		configure_rxgain (TARGET_RADIO);
	}

	printf ( "==>rxgain %0x\n", rxgain );
}

void IncreaseRxPa () {
	rxpa += 0x1000;
	configure_rxgain (TARGET_RADIO);

	printf ( "==>rxgain %0x\n", rxpa );
}

void DecreaseRxPa () {
	if (rxpa >= 0x1000 ) {
		rxpa -= 0x1000;
		configure_rxgain (TARGET_RADIO);
	}

	printf ( "==>rxgain %0x\n", rxpa );
}

extern void QuitProgram ();


void keyusage () {
	printf ( "\nKeyboard command:\n" );
	printf ( "o      : open a dump file\n" );
	printf ( "w      : save current dump to a file\n" );
	printf ( "s      : toggle resampling 44-40\n" );
	printf ( "e      : toggle decimation rate\n" );
	printf ( "d      : get a dump from radio\n" );
	printf ( "c      : get a command line\n" );
	printf ( "x      : quite the program\n" );
	printf ( "0~3    : change radio\n" );
	printf ( "+      : increase the rxgain\n" );
	printf ( "-      : decrease the rxgain\n" );
	printf ( ">      : decrease the rxpa\n" );
	printf ( "<      : decrease the rxpa\n" );
	printf ( "space  : toggle playback\n" );
	printf ( "->     : speedup playback\n" );
	printf ( "<-     : slowdown playback\n" );
	printf ( "up arrow   : zoom out overview\n" );
	printf ( "down arrow : zoom in  overview\n" );
	printf ( "enter      : center to current playback position\n" );
	printf ( "h/F1       : this page\n" );
}

void keyControl(WPARAM key)
{
    switch (key)
    {
        case VK_SPACE:
            playOrPause();
            break;
        case VK_RIGHT:
            speedUp();
            break;
        case VK_LEFT:
            speedDown();
            break;
        case 'o': case 'O':
            // open new file
			printf ( "open file\n" );
            OpenNewFile();
            break;
		case 'w': case 'W':
			WriteToFile ();
			break;
		case 'c': case 'C': 
			GetCommand ();
			break;
		case 'd': case 'D': 
			OnlineDump ();
			break;
		case 'x': case 'X': 
			QuitProgram ();
			break;
		case 's': case 'S':
			RotateSampleRate ();
			break;		
		case 'e': case 'E':
			RotateDecimation ();
			break;
		case '0':
			SwitchRadio (0);
			break;
		case '1':
		case '2':
		case '3':
			SwitchRadio (1+key-'1');
			break;
		case VK_OEM_PLUS: case '=':
			IncreaseRxgain ();
			break;
		case VK_OEM_MINUS: case '_':
			DecreaseRxgain ();
			break;
		case '>': case VK_OEM_PERIOD:
			IncreaseRxPa ();
			break;
		case '<': case VK_OEM_COMMA:
			DecreaseRxPa ();
			break;
        case VK_F12:
            nextScheme();
            break;
        case VK_PRIOR:
            overviewPageUp();
            break;
        case VK_NEXT:
            overviewPageDown();
            break;
        case VK_DOWN:
            overviewZoomIn();
            break;
        case VK_UP:
            overviewZoomOut();
            break;
        case VK_RETURN:
            overviewCenter();
            break;
		case VK_F5:
			flagMonitorMode = !flagMonitorMode;
			SetWindowText ( gHwnd, getWinTitle () );
			break;

		case VK_F1: case 'h' : case 'H':
			keyusage ();
			break;
    }
}
