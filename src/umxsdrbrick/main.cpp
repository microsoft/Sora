#pragma warning(disable:4530)

#include <windows.h>

#include <stdio.h>
#include <conio.h>
#include <soratime.h>

#include "umxsdr.h"
#include "phy.h"
#include "DebugPlotU.h"

// performance monitor
TIMESTAMPINFO        tsinfo;
ULONGLONG            tts1, tts2;

int gDisplayMode    = DISPLAY_PAGE;
int gPHYMode		= PHY_802_11A;	
int gSampleRate		= 40;
int gDataRateK		= 6000; // in Kbps
ULONG gChannelFreq  = 5200;

void Dot11_main ();

bool bWarning = false;
const char sDelim[] = " ,.:-";

void usage () {
	printf ( "umxsdrbrick cmd parameter \n" );
	printf ( "--------------------------------------------------\n" );
	printf ( "   -s  [40|44]  -    sampling rate (MHz) \n" );
	printf ( "   -f  freq     -    central frequency (MHz)\n" );
	printf ( "   -r  rate     -    sending data rate (Kbps or MCS index for 802.11n)\n" );
	printf ( "   -b           -    use 802.11b PHY\n" );
	printf ( "   -n           -    use 802.11n PHY\n" );
	printf ( "   -o  [0|1]    -    0 - status; 1 - logs \n" );
	printf ( "   -h           -    display this page\n" );
    printf ( "   -m  \"str\"    -    specify radio numbers seperated by \"%s\", eg. \"1 0 3 4\"\n", sDelim);
}

bool CheckSampleRate () {
	if ( gSampleRate != 40 &&
 		 gSampleRate != 44 ) {
 		printf("Invalid SampleRate\n");
 		return false;
	}	 

	return true;
}

bool CheckDataRate () {
	if ( gPHYMode == PHY_802_11A ) {
		switch(gDataRateK) {
		case  6000:
		case  9000:
		case 12000:
		case 18000:
		case 24000:
		case 36000:
		case 48000:
		case 54000:
			break;
		default:
			printf( "Warning: data rate (%dKbps) is not supported in 802.11a.\n"
				    "Data rate 6Mbps is used instead.\n\n",
				    gDataRateK );
			gDataRateK = 6000;
			bWarning = true;
		}
	} else if ( gPHYMode == PHY_802_11B ) {
		switch (gDataRateK) {
		case  1000:
		case  2000:
		case  5500:
		case 11000:
			break;
        default:
			printf("Warning: data rate (%dKbps) is not supported in 802.11b.\n",
				gDataRateK );
			gDataRateK = 1000;
			printf( "Data rate 1Mbps is used instead.\n\n" );
			bWarning = true;
		}
	} else if ( gPHYMode == PHY_802_11N ) {
        switch (gDataRateK)
        {
        case 8:
        case 9:
        case 10:
            break;
        default:
			printf("Warning: MCS index (%d) is not supported in 802.11n.\n",
				gDataRateK );
			gDataRateK = 8;
			printf( "MCS index 8 is used instead.\n\n" );
			bWarning = true;
        }
	}
	return true;
}

bool ParseRadioNoString(const char *radioNoString, OUT ULONG radioNo[])
{
    radioNo[0] = atoi(radioNoString);
    size_t len = strcspn(radioNoString, sDelim);
    if (radioNoString[len] == '\0') return true;
    return ParseRadioNoString(radioNoString + len + 1, radioNo + 1);
}

bool CheckRadioNo(ULONG radioNo[], size_t nstream)
{
    bool *exist = new bool[MAX_RADIO_NUMBER];
    memset(exist, 0, sizeof(bool) * MAX_RADIO_NUMBER);
    size_t iss;
    for (iss = 0; iss < nstream; iss++)
    {
        ULONG ra = radioNo[iss];
        if (ra >= MAX_RADIO_NUMBER) return false;
        if (exist[ra]) return false;
        exist[ra] = true;
    }
    return true;
}

bool ParseCmdLine ( int argc, const char* argv[] ) {
	int i=1;
	while ( i< argc ) {
		if ( argv[i][0] == '-' 
			 && argv[i][1] == 's' ) 
		{
			if ( ++i < argc ) {
				gSampleRate = atoi(argv[i++]);
				if (!CheckSampleRate ()) return false;
			} else {
				return false;
			}
		} else 
		if ( argv[i][0] == '-' 
			 && argv[i][1] == 'r' ) 
		{
			if ( ++i < argc ) {
				gDataRateK = atoi(argv[i++]);
			} else {
				return false;
			}
		} else if ( argv[i][0] == '-' 
			 && argv[i][1] == 'f' ) 
		{
			if ( ++i < argc ) {
				gChannelFreq = atoi(argv[i++]);
			} else {
				return false;
			}
		} else if ( argv[i][0] == '-'
			&& argv[i][1] == 'b')
		{
			gPHYMode = PHY_802_11B; 
			i++;
		} else if ( argv[i][0] == '-'
			&& argv[i][1] == 'n')
		{
			gPHYMode = PHY_802_11N;
			i++;
		} else if ( argv[i][0] == '-'
			&& argv[i][1] == 'o')
		{
			if ( ++i < argc ) {
				gDisplayMode = atoi(argv[i++]);
			} else return false;
			if ( gDisplayMode > 1 ) return false;
        } else if ( argv[i][0] == '-'
            && argv[i][1] == 'm')
        {
            if ( ++i < argc ) {
                if (argv[i][0] == '-') return false;
                if (!ParseRadioNoString(argv[i++], RadioNo)) return false;
            } else return false;
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


bool CheckParameters () 
{
	if ( gPHYMode == PHY_802_11B ) {
		// 11b
		if ( gSampleRate == 40 ) {
			printf ( "Warning: sampling rate (%dMHz) is not compatible with 802.11b PHY.\n"
					 "You may use this configurate to communicate to another Sora node with the same setting. "
					 "But it may fail to communicate with a standard 802.11b NIC.\n\n", 
				gSampleRate );
		}

		bWarning = true;
    }
    if (!CheckDataRate()) return false;
    if (!CheckRadioNo(RadioNo, GetNStream()))
    {
        printf("ERROR: Radio numbers are not valid.\n");
        return false;
    }
    return true;
}

int __cdecl main(int argc, const char *argv[])
{
	if ( !ParseCmdLine (argc, argv) )
	{
		usage();
		return 0;
	}

	if ( !CheckParameters () ) {
		return 0;
	}

	// init DebugPlot library
 	DebugPlotInit();

	InitializeTimestampInfo ( &tsinfo, false );
	
	// begin receive or transmit packets
	if (!SoraUInitUserExtension("\\\\.\\HWTest")) {
		printf ( "Error: fail to find the hwtest driver!\n" );
		getchar();
		return -1;
	}

	if ( bWarning ) {
		printf ( "Warnings - press enter to continue...\n" );
		getchar();
	}	
	
	// clear screen	
	system( "cls" );	
	
	// Start the main procedure
	Dot11_main ();

	// clear screen
	// system( "cls" );	
	
	SoraUCleanUserExtension();
	DebugPlotDeinit();

    return 0;
}
