#include "common.h"

#include <sora.h>

int  TARGET_RADIO = 0; // default radio is zero
bool bUMX = false;
bool flagMonitorMode = false;

static void runCommand(char * c)
{
    PROCESS_INFORMATION info;
    STARTUPINFO stInfo;
    memset(&stInfo, 0, sizeof(STARTUPINFO));
    CreateProcess(NULL, (LPTSTR)(c), NULL, NULL,
            FALSE, CREATE_NO_WINDOW, NULL, NULL, &stInfo, &info);
    WaitForSingleObject(info.hProcess, INFINITE);
    CloseHandle(info.hProcess);
    CloseHandle(info.hThread);
}

void autoGetNewDump()
{
    runCommand("python update.py");
    // system("python update.py");
    // ShellExecute(NULL, "open", "update.py", NULL, NULL, SW_HIDE);
    loadDumpFile("now.dmp");
}

static void dataModelInitialize()
{
    autoGetNewDump();
    clearReceiverState();
}

void pntDecode(HDC hdc);
void pntByte(HDC hdc);
void pntGeneralInfo(HDC hdc);

static void paintersInitialize()
{
    colorInit();
    addPaintFunc(pntClear);   

    addPaintFunc(pntRawData);
    addPaintFunc(pntCarrierSensor);
    addPaintFunc(pntCorrelator);

    addPaintFunc(pntModelSpec);
    addPaintFunc(pntRawSpec);
    addPaintFunc(pntChannelSpec);

    addPaintFunc(pntBeforePilot);
    addPaintFunc(pntAfterPilot);

//    addPaintFunc(pntDecode);
    addPaintFunc(pntByte);
    addPaintFunc(pntOverview);

    addPaintFunc(pntDebugLog);
    addPaintFunc(pntGeneralInfo);
}

// int g_sampleRate = 40; // default is 40MHz
int g_resample44_40 = 0;
int g_decimation    = 2;
int g_samplingrate  = 40;


bool gbWBX = false;

// UMX variables
SORA_RADIO_RX_STREAM	RxStream;	        // RxStream
PVOID					RxBuffer = NULL;	// Mapped Rx Buffer
ULONG					RxBufferSize = 0;

bool                is_whitespace = false;
bool                bGetCmdline   = false;
ULONG				centralfreq = 2422;
ULONG				rxgain = 0x600;
ULONG				rxpa = 0x3000;
ULONG				freqoffset = 0;

char                gcmdline [512];

bool				bMonitFile = false;
char                MonitFile[256];

bool				bEndBatch = false;
char				EndBatchFile[256];

void RadioInfo () {
	printf ( "Target radio %d\n", TARGET_RADIO );
	printf ( "central freq %dMHz\n", centralfreq );
	printf ( "rxpa         0x%x\n", rxpa );
	printf ( "rxgain       0x%x\n", rxgain );
	printf ( "cfo          %d\n", freqoffset );
}

bool is_ws (char ch) {
	return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
}

void configure_rxgain (int radio) {
	// Set the Receiver Gain
	SoraURadioSetRxPA	(radio, rxpa   );  
	SoraURadioSetRxGain (radio, rxgain );   
}

void configure_radio ( int radio) {
	printf ( "configure radio...\n" );
	// Start Radio
	if ( is_whitespace ) {
		SoraURadioSetCentralFreq (radio, centralfreq );
	} else {
		SoraURadioSetCentralFreq (radio, centralfreq * 1000);
	}
		
	configure_rxgain (radio);

	// don't set freqoffset
	RadioInfo ();
}

void usage () {
	printf ( "-h        : display help page\n"
			 "-r rid    : set radio id\n"
		     "-f freq   : set centralfreq\n" 
			 "-s rate   : hardware sampling rate\n"
			 "-rxpa v   : set rxpa \n"
			 "-rxgain v : set rxgain \n"
			 "-cfo    v : set frequency offset\n"
			 "-w        : for whitespace RF board\n"
			 "--monit file  : monitoring a file changes\n"
			 "--bat file    : running a batch file after the playback reachs the end\n"
			 );
}

HRESULT init_radio ( int radio ) {
	HRESULT hr;
	// Map Rx Sample Buffer
	SoraURadioStart (radio);

	hr = SoraURadioMapRxSampleBuf(radio, &RxBuffer, &RxBufferSize);
	if (FAILED(hr)) {
		printf ( "Warning: fail to map rx buffer! Online dump disabled!\n" );
		return hr;
	}   
    
	hr = SoraURadioAllocRxStream(&RxStream, radio, (PUCHAR)RxBuffer, RxBufferSize);
	if (FAILED(hr)) {
		printf ( "Warning: fail to allocate a RX stream! Online dump disabled!\n" );

		hr = SoraURadioUnmapRxSampleBuf(radio, RxBuffer);
		RxBuffer = NULL;
	}

	return hr;
}

void Release_radio ( int radio ) {
	HRESULT hr;
	if ( RxBuffer ) {
		printf ( "Release radio %d\n", radio );
		SoraURadioReleaseRxStream      (&RxStream, radio);
        hr = SoraURadioUnmapRxSampleBuf(radio, RxBuffer);

		RxBuffer = NULL;
	}
}

void init_umx () {
	HRESULT hr;
	printf ( "Initialization UMX....\n" );
	if (!SoraUInitUserExtension("\\\\.\\HWTest")) {
		printf ( "Warning: fail to find the hwtest driver! Online dump disabled!\n" );
		return;
	}

	bUMX = true;

	printf ( "UMX initialized....\n" );
}

void uninit_umx () {
	HRESULT hr;
	
	Release_radio ( TARGET_RADIO );
	
	SoraUCleanUserExtension();
}

void QuitProgram () {
	uninit_umx ();
	PostQuitMessage (0);
}

bool ParseCommandLine ( int argc, char* argv[] )
{
	int i=1;
	bool bCfgRF = false;
	while ( i< argc ) {
		if ( argv[i][0] == '-' 
			 && argv[i][1] == 'h' ) 
		{
			usage ();
			i++;
			return false;
		} else
		if ( argv[i][0] == '-' 
			 && argv[i][1] == 'f' ) 
		{
			if ( ++i < argc ) {
				centralfreq = atoi(argv[i++]);
				bCfgRF = true;
			} else {
				return false;
			}
		} else 
		if ( argv[i][0] == '-' 
			 && argv[i][1] == 's' ) 
		{
			if ( ++i < argc ) {
				g_samplingrate = atoi(argv[i++]);
			} else {
				return false;
			}
		} else
		if ( argv[i][0] == '-' 
			 && argv[i][1] == 'w' ) 
		{
			is_whitespace = true;
			i++;
		} else 
		if ( argv[i][0] == '-' 
			 && argv[i][1] == 'r' ) 
		{
			if ( ++i < argc ) {
				TARGET_RADIO = atoi( argv[i++]);
			} else {
				return false;
			}
				
		} else 
		if ( strncmp (argv[i], "-rxpa", 5 ) == 0 ) 
		{
			if ( ++i < argc ) {
				rxpa = (ULONG) strtol(argv[i++], NULL, 0);
				bCfgRF = true;
			} else {
				return false;
			}
		} else 
		if ( strncmp ( argv[i], "-rxgain", 7 ) == 0 ) 
		{
			if ( ++i < argc ) {
				rxgain = (ULONG) strtol(argv[i++], NULL, 0);
				bCfgRF = true;
			} else {
				return false;
			}
		} else 
		if ( strncmp ( argv[i], "-cfo", 4 ) == 0 ) 
		{
			if ( ++i < argc ) {
				freqoffset = strtol (argv[i++], NULL, 0);
				SoraURadioSetFreqOffset ( TARGET_RADIO, freqoffset );
			} else {
				return false;
			}
		} else 
		if ( strncmp ( argv[i], "-info", 5 ) == 0 ) 
		{
			RadioInfo ();	
			i++;
		} else
		if ( strncmp ( argv[i], "--monit", 7 ) == 0 ) 
		{
			if ( ++i < argc ) {
				strcpy_s ( MonitFile, sizeof(MonitFile), argv[i] );
				bMonitFile = true;
				i ++;
			} else {
				return false;
			}
		} else
		if ( strncmp ( argv[i], "--bat", 5 ) == 0 ) 
		{
			if ( ++i < argc ) {
				strcpy_s ( EndBatchFile, sizeof(EndBatchFile), argv[i] );
				bEndBatch = true;
				i ++;
			} else {
				return false;
			}
		} else
		{
			return false;
		}
	}

	if ( bCfgRF ) {
		configure_radio ( TARGET_RADIO );
	}
	return true;
}

int WINAPI
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
{
    dataModelInitialize();
    paintersInitialize();

	// ParseCommandLine ( lpCmdLine );
	init_umx ();

	if (umx_enabled ()) {
		// SoraURadioStart (TARGET_RADIO);

		HRESULT	hr = init_radio ( TARGET_RADIO );

		Sleep (10);

		configure_radio (TARGET_RADIO);
	}
	
    setMainInstance(hInstance);
    startDemoWindow();

    return 0;
}

int __cdecl main ( int argc, char* argv[] ) {
    printf ( "Software oscolliscope...\n" );

	if ( !ParseCommandLine ( argc, argv ) ) {
		// printf ( "Warning: commandlien error! Some parameters are using defaults!\n" );
		return 0;
	}
	
    HINSTANCE hInst = GetModuleHandle (NULL);
    WinMain ( hInst, NULL, NULL, SW_SHOW );
}
