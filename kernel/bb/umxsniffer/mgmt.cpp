#include "umxsdr.h"
#include "phy.h"
#include "mac.h"
#include "mgmt.h"
#include "crc32.h"

#include "fb11bdemod_config.hpp"
#include "fb11ademod_config.hpp"

// last packet
MAC_ADDRESS			lastMACSrc   = { 0 };
MAC_ADDRESS			lastMACDst   = { 0 };
MAC_ADDRESS			lastMACBssid = { 0 };
int					lastRate     = 0;

// Assoc Mgmt
// flag for auto configuration rx-gain and association
ULONG 				fAutoCfg = 0;

ASSO_STATE          assoState = ASSO_NONE;
ULONG      		    mgmtPkt; 

// the timer tracking the association process
// if timeout, the connection is reset and the association is started over
int			  		assocTimer  = -1;

// keep alive timer
int 				fKeepAlive  = 0;
ULONG 				energy_max  = 0;

static char* assoMsg[] = { "No association", 
							"Authenticated",
							"Associated" };


// Operation mode 
static char* opModeMsg[] = { "Client Mode", 
							 "Adhoc Mode"
							 };

int OpMode = CLIENT_MODE;

// statistics
int err_stat[3] = {0};

ULONG                nDataPacketRcvCnt = 0;
ULONG                nDataPacketSndCnt = 0;
ULONG                nDataPacketDropCnt = 0;
ULONG                nAckRcvCnt = 0;
ULONG                nBeaconRcvCnt = 0;
ULONG                nInterferenceCnt = 0;
ULONG				 nIdleCnt = 0;

enum RXGAIN_CTRL_STATE {
	RXGAIN_TRACK  = 0,
	RXGAIN_SEARCH = 1,
};

ULONG               RxGain = 44; 
ULONG               RxPa   = 0;  // 2450 specific
ULONG               RxG    = 0;  // 2450 specific
ULONG				RxGainTimer = 0;
int                 RxGainSearch = 1;                
ULONG				RxBeaconTimer = 0;
ULONG				RxBeaconCnt   = 0;

FP_RAD              PhaseOffset    = 0;
LONG                FreqOffset    = 0;

static ULONG 			 lastGoodRxGain = MAX_RX_GAIN;
static RXGAIN_CTRL_STATE rxgState = RXGAIN_SEARCH;
static ULONG  			 goodBeaconCnt = 0;


char line[256] = "                                                                             \r";
#define clear_printf(f,...) { printf (line); printf (f, ## __VA_ARGS__); }
char text[256];
void print_status ( )
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo ( GetStdHandle (STD_OUTPUT_HANDLE), &info );

	COORD cord;
	cord.X = info.srWindow.Left; cord.Y = info.srWindow.Top;
	SetConsoleCursorPosition ( GetStdHandle (STD_OUTPUT_HANDLE), cord );
	
	// A few things done here
	double cfo = 1.0 * PhaseOffset * 20000 / 0x7FFF / 2 / 3.141593;
	if ( cfo > 5 || cfo < -5 ) {
		// configure radio cfo
		FreqOffset += (LONG)(cfo * 1000 / 2 );
		SoraURadioSetFreqOffset ( TARGET_RADIO, FreqOffset );
	}

	// first line - separator
	clear_printf ("+------------------------------------------------------------------------+\n" );	

	sprintf ( text, " Operation mode: %20s | Channel freq %dMHz", opModeMsg[OpMode], gChannelFreq );
	clear_printf ( "%s\n", text );

	sprintf ( text, " Association State: %s", assoMsg[assoState] );
	clear_printf ( "%s\n", text );

	if ( OpMode == CLIENT_MODE ) {	
		sprintf ( text, " SSID: %20s  | Bssid: %02X-%02X-%02X-%02X-%02X-%02X", 
						ssid,
				 		bssid.Address[0], bssid.Address[1], bssid.Address[2],
				 		bssid.Address[3], bssid.Address[4], bssid.Address[5] );

		clear_printf ("%s\n", text );
	} else {
		sprintf ( text, " SSID: %20s  | Bssid: %02X-%02X-%02X-%02X-%02X-%02X", 
						adhoc_ssid,
				 		adhoc_bssid.Address[0], adhoc_bssid.Address[1], adhoc_bssid.Address[2],
				 		adhoc_bssid.Address[3], adhoc_bssid.Address[4], adhoc_bssid.Address[5] );
		clear_printf ("%s\n", text );	
	}
	
	sprintf ( text, " RxGain: %2ddB (%4x, %4x)   | Energy max %10d   | Auto %d", RxGain, RxPa, RxG, energy_max, fAutoCfg);
	clear_printf ("%s\n", text );

	sprintf ( text, " Gain ctrl state %d           | goodBeaconCnt %d  ", rxgState, goodBeaconCnt);
	clear_printf ("%s\n", text );

	sprintf ( text, " Data rate: %4dKbps         | idle idx %d", gDataRateK, nIdleCnt);
	clear_printf ("%s\n", text );
	nIdleCnt = 0;

	sprintf ( text, " CFO to AP %+.2lfKHz          | Freq Offset setting %d  ", cfo, FreqOffset );
	clear_printf ("%s\n", text );

	sprintf ( text, " Frame received %6d | Frame sent %6d | Beacon received %6d ", 
			nDataPacketRcvCnt, nDataPacketSndCnt, nBeaconRcvCnt );
	clear_printf ( "%s\n", text );

	sprintf ( text, " Interference %6d   | Lost pkt %6d   | Ack received %6d ", nInterferenceCnt, nDataPacketDropCnt, nAckRcvCnt );
	clear_printf ( "%s\n", text );

	sprintf ( text, " Last MAC Src %02X-%02X-%02X-%02X-%02X-%02X Dst %02X-%02X-%02X-%02X-%02X-%02X", 
			  lastMACSrc.Address[0], lastMACSrc.Address[1], lastMACSrc.Address[2],
			  lastMACSrc.Address[3], lastMACSrc.Address[4], lastMACSrc.Address[5],  
  			  lastMACDst.Address[0], lastMACDst.Address[1], lastMACDst.Address[2],
			  lastMACDst.Address[3], lastMACDst.Address[4], lastMACDst.Address[5] );
	clear_printf ( "%s\n", text );

	
	sprintf ( text, " Last MAC Bssid %02X-%02X-%02X-%02X-%02X-%02X", 
  			  lastMACBssid.Address[0], lastMACBssid.Address[1], lastMACBssid.Address[2],
			  lastMACBssid.Address[3], lastMACBssid.Address[4], lastMACBssid.Address[5] );
	clear_printf ( "%s\n", text );

	sprintf ( text, " Last received rate %4dKbps", lastRate );
	clear_printf ( "%s\n", text );

    short dc_re = 0, dc_im = 0;
	if ( gPHYMode == PHY_802_11A ) {
        dc_re = BB11aDemodCtx.CF_VecDC::direct_current()[0].re;
        dc_im = BB11aDemodCtx.CF_VecDC::direct_current()[0].im;
	} else if ( gPHYMode == PHY_802_11B ) {
        dc_re = BB11bDemodCtx.direct_current()[0].re;
        dc_im = BB11bDemodCtx.direct_current()[0].im;
	}

	sprintf ( text, " DC <%+06d,%+06d>", dc_re, dc_im);
	clear_printf ( "%s\n", text );

    if  ( gPHYMode == PHY_802_11A ) {
		sprintf ( text, " OFDM errors: sync error %d invalid signal field %d crc32 error %d ", 
			err_stat[0], err_stat[1], err_stat[2]);
		clear_printf ( "%s\n", text );
	}
	
	clear_printf ("+------------------------------------------------------------------------+\n" );	
	clear_printf (" Press 'a'     - toggle auto configuration\n");
	clear_printf (" Press '[' ']' - adjust the sending rate\n");
	clear_printf (" Press '+' '-' - adjust the rx gain\n");
	clear_printf (" Press 'c'     - associate to AP\n");
	clear_printf (" Press 'p'     - toggle Client mode or Adhoc mode\n");
	clear_printf ("\n Press 'X', 'x' or Ctrl-C to exit the program\n");
	clear_printf ("+------------------------------------------------------------------------+\n" );	
	clear_printf ("\n" );	
	
}

FINL
BOOLEAN ModulateBuffer11 ( UCHAR* pbuf, int size, LONG needACK )
{
	struct PACKETxID* ptid;
	HRESULT hr;
	ptid = (struct PACKETxID*) malloc(sizeof(struct PACKETxID));
	memset(ptid, 0, sizeof(struct PACKETxID));
	
	ptid->m_packet = NULL; // no packet is associated
	
	ptid->m_status = PACKET_NOT_MOD;
	ptid->m_needack = needACK;
	
	ulong sample_size;

    if ( gPHYMode == PHY_802_11A ) {
	    hr = ModBuffer11a (pbuf, size, NULL, 0, (ushort)gDataRateK, 
					       (COMPLEX8*)SampleBuffer, SampleSize, &sample_size );
	} else if (gPHYMode == PHY_802_11B) {
	    hr = ModBuffer11b (pbuf, size, NULL, 0, (ushort)gDataRateK, 
					       (COMPLEX8*)SampleBuffer, SampleSize, &sample_size );
    }

	ULONG TxID;			
	hr = SoraURadioTransferEx(TARGET_RADIO, SampleBuffer, sample_size, &TxID);

	if (SUCCEEDED(hr)) {
		ptid->m_txid = TxID;
		ptid->m_status = PACKET_CAN_TX;
		insert_list_tail(&SendListHead, &ptid->m_entry, &SendListLock);
		// printf ( "modulation buffer txid %d\n", TxID );
	} else {
		free ( ptid );
	}
	
	return TRUE;	
}

bool AdjustRxGain11a ( int iG ) {
	int NewGain = RxGain + iG;
	if ( NewGain < MIN_RX_GAIN || NewGain > MAX_RX_GAIN ) return false;
	RxGain = NewGain;
	ConfigureRxGain ( RxGain );
    return true;
}

void ConfigureRxGain ( int RxGain ) {
	if ( RxGain < MIN_RX_GAIN || RxGain > MAX_RX_GAIN ) return; // invalid
	if ( gPHYMode == PHY_802_11A ) {
		int G = (RxGain+1) / 2;
		
		if ( G <= 8 ) {
			RxPa = 0X1000;
			RxG  = G*0x200;
		} else if ( G <= 16 ) {
			RxPa = 0X2000;
			RxG  = (G-8)*0x200;
		} else {
			RxPa = 0X3000;
			RxG  = (G-16)*0x200;
		}
	} else if ( gPHYMode == PHY_802_11B ) {
		// 802.11b - RxG should be less than 0x800
		if ( RxGain < 18 ) {
			RxPa = 0x1000;
			RxG  = (RxGain>8)?8:RxGain;
		} else if ( RxGain < 34 ) {
			RxPa = 0x2000;
			RxG = (RxGain-16>8)?8:(RxGain-16);
		} else {
			RxPa = 0x3000;
			RxG = (RxGain-32>8)?8:(RxGain-32);
		}
		RxG = RxG * 0x100;
	}
	
	SoraURadioSetRxPA	(TARGET_RADIO, RxPa);  // 0dB
	SoraURadioSetRxGain (TARGET_RADIO, RxG);   // 2G dB
}


void SetMaximalGain () {
	if ( gPHYMode == PHY_802_11A ) {
		RxGain = MAX_RX_GAIN;
	} else if ( gPHYMode == PHY_802_11B ) {
		RxGain = 36;
	}
	
	ConfigureRxGain ( RxGain );
}

//
// Increase Rx Gain by one level
//
bool IncreaseGain () {
	uint gain = RxGain;
	if ( gPHYMode == PHY_802_11A ) {
		AdjustRxGain11a ( 2 ); // increase by two dB
	} else {
		// only three level 
		if ( RxGain < 18 ) RxGain = 18;
		else RxGain = 34;
		
		ConfigureRxGain ( RxGain );
	}
	return (gain != RxGain );
}

//
// Decrease Rx Gain by one level
//
bool DecreaseGain () {
	if ( gPHYMode == PHY_802_11A ) {
		return AdjustRxGain11a ( -2 ); // increase by two dB
	} else {
    	uint gain = RxGain;
		// only three level 
		if ( RxGain > 18 ) RxGain = 18;
		else RxGain = 4;
		
		ConfigureRxGain ( RxGain );
    	return (gain != RxGain );
	}
}

void UpdateFreqOffset ()
{
	if ( gPHYMode == PHY_802_11A ) {
		// average the Phase offset
		PhaseOffset = (PhaseOffset >> 1) + (BB11aDemodCtx.CF_CFOffset::CFO_est() >> 1);
	}
}

void UpdateEnergy () {
	if ( gPHYMode == PHY_802_11A ) {
		energy_max = (energy_max - (energy_max >> 2)) + (BB11aDemodCtx.CF_11CCA::cca_pwr_reading() >> 2);
	} else if (gPHYMode == PHY_802_11B){
		energy_max = (energy_max - (energy_max >> 2)) + (BB11bDemodCtx.CF_11CCA::cca_pwr_reading() >> 6);
	}
	//int pdata = (int) energy_max;
	//PlotLine ( "BB:Energy", & pdata, 1 );
}

void GainTracking () {
	if ( gPHYMode == PHY_802_11A ) {
		if ( energy_max > 6000000 ) {
			DecreaseGain ();
		} else if ( energy_max <= 2000000 ) {
			IncreaseGain ();
		}
	}	
}

/*******************************************************
	Management frames
*******************************************************/
ulong RequestMgmtPkt ( ulong pkttype ) {
	mgmtPkt |= pkttype;
	return mgmtPkt;
}

void KeepAlive () {
	WLAN_NULL_FRAME mframe;
	memset ( &mframe, 0, sizeof(mframe));
	
	mframe.wlanheader.FrameControl.Version = 0;
	mframe.wlanheader.FrameControl.Type = FRAME_DATA;
	mframe.wlanheader.FrameControl.Subtype = 0x08; // null
	
	mframe.wlanheader.Address1 = bssid;
	mframe.wlanheader.Address2 = MACAddress;
	mframe.wlanheader.Address3 = bssid;
	
	mframe.wlanheader.FrameControl.ToDS  = 0;	// to DS
	mframe.wlanheader.FrameControl.Retry = 0;
	mframe.wlanheader.SequenceControl.SequenceNumber = SendSeQ++;

	ModulateBuffer11 ((PUCHAR)&mframe, sizeof(mframe), FALSE );
}

void Auth () {
	WLAN_AUTH_FRAME mframe;
	memset ( &mframe, 0, sizeof(mframe));
	
	mframe.wlanheader.FrameControl.Version = 0;
	mframe.wlanheader.FrameControl.Type = FRAME_MAN;
	mframe.wlanheader.FrameControl.Subtype = 0x0B; // AUTH
	
	mframe.wlanheader.Address1 = bssid;
	mframe.wlanheader.Address2 = MACAddress;
	mframe.wlanheader.Address3 = bssid;
	
	mframe.wlanheader.FrameControl.ToDS  = 0;	// to DS
	mframe.wlanheader.FrameControl.Retry = 0;
	mframe.wlanheader.SequenceControl.usValue = SendSeQ << 4;
	SendSeQ ++;
	
	mframe.alg = 0;
	mframe.seq = 1;
	mframe.status = 0;

	ModulateBuffer11 ((PUCHAR)&mframe, sizeof(mframe)-4, FALSE );
}

static UCHAR mgmtpkt_buffer[1500];
static UCHAR support_rate11a[8] = { 0xc, 0x12, 0x18, 0x24, 0x30, 0x48, 0x60, 0x6c };
MAC_ADDRESS bAddr = { 0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF };

void TestPkt () {
}

void Associate11a () {
	PUCHAR pbuf = mgmtpkt_buffer;
	UINT   fsize = 0;
	
	PWLAN_ASSO_FRAME pFrame = (PWLAN_ASSO_FRAME) pbuf;

	memset ( pFrame, 0, sizeof(WLAN_ASSO_FRAME));
	
	pFrame->wlanheader.FrameControl.Version = 0;
	pFrame->wlanheader.FrameControl.Type = FRAME_MAN;
	pFrame->wlanheader.FrameControl.Subtype = 0x0; // Associ
	
	pFrame->wlanheader.Address1 = bssid;
	pFrame->wlanheader.Address2 = MACAddress;
	pFrame->wlanheader.Address3 = bssid;
	
	pFrame->wlanheader.SequenceControl.usValue = SendSeQ << 4;

	pFrame->capInfo  = 0x01;
	pFrame->interval = 1;
	fsize = sizeof (WLAN_ASSO_FRAME);	

	PINFO_ELEMENT pIE = (PINFO_ELEMENT) (pbuf + sizeof(WLAN_ASSO_FRAME));
	pIE->eid  = 0; // ssid;
	pIE->elen = (UCHAR) ssid_len;
	strncpy ( (char*)pIE->data, ssid, ssid_len );
	fsize += pIE->elen + 2;

	pIE = (PINFO_ELEMENT) (pbuf + fsize );
	pIE->eid = 1; // supported rate
	pIE->elen = 8;
	for ( int i=0; i<8; i++ ) {
		pIE->data[i] = support_rate11a[i];
	}
	fsize += pIE->elen + 2;

	ModulateBuffer11 (pbuf, fsize, FALSE );
}

void Associate11b () {
	PUCHAR pbuf = mgmtpkt_buffer;
	UINT   fsize = 0;
	
	PWLAN_ASSO_FRAME pFrame = (PWLAN_ASSO_FRAME) pbuf;

	memset ( pFrame, 0, sizeof(WLAN_ASSO_FRAME));
	
	pFrame->wlanheader.FrameControl.Version = 0;
	pFrame->wlanheader.FrameControl.Type = FRAME_MAN;
	pFrame->wlanheader.FrameControl.Subtype = 0x0; // Associ
	
	pFrame->wlanheader.Address1 = bssid;
	pFrame->wlanheader.Address2 = MACAddress;
	pFrame->wlanheader.Address3 = bssid;
	
	pFrame->wlanheader.SequenceControl.usValue = SendSeQ << 4;

	pFrame->capInfo  = 0x01;
	pFrame->interval = 1;
	fsize = sizeof (WLAN_ASSO_FRAME);
	

	PINFO_ELEMENT pIE = (PINFO_ELEMENT) (pbuf + sizeof(WLAN_ASSO_FRAME));
	pIE->eid  = 0; // ssid;
	pIE->elen = (UCHAR) ssid_len;
	strncpy ( (char*)pIE->data, ssid, ssid_len );
	fsize += pIE->elen + 2;

	pIE = (PINFO_ELEMENT) (pbuf + fsize );
	pIE->eid = 1; // supported rate
	pIE->elen = 4;
	pIE->data[0] = 0x82; // 1M
	pIE->data[1] = 0x84; // 2M
	pIE->data[2] = 0x8B; // 5.5M
	pIE->data[3] = 0x96; // 11M
	fsize += pIE->elen + 2;

	ModulateBuffer11 (pbuf, fsize, FALSE );
}

void Associate () 
{
	if ( gPHYMode == PHY_802_11A ) {
		Associate11a ();
	} else {
		Associate11b ();
	}	

}

void Beacon () {
	PUCHAR pbuf = mgmtpkt_buffer;
	UINT   fsize = 0;
	
	PDOT11_MAC_BEACON pFrame = (PDOT11_MAC_BEACON) pbuf;

	memset ( pFrame, 0, sizeof(WLAN_ASSO_FRAME));
	
	pFrame->MacHeader.FrameControl.Version = 0;
	pFrame->MacHeader.FrameControl.Type    = FRAME_MAN;
	pFrame->MacHeader.FrameControl.Subtype = SUBT_BEACON; // beacon
	
	pFrame->MacHeader.Address1 = bAddr; // Broadcast
	pFrame->MacHeader.Address2 = MACAddress;
	pFrame->MacHeader.Address3 = adhoc_bssid;
	
	pFrame->MacHeader.SequenceControl.SequenceNumber= SendSeQ;

	LARGE_INTEGER cpucnt;
	QueryPerformanceCounter ( &cpucnt );
	pFrame->Timestamp = SoraGetTimeofDay (&tsinfo); 
	pFrame->Interval = 100;
	pFrame->Capability  = 0x02;
	fsize = sizeof (pFrame->MacHeader) + sizeof(pFrame->Timestamp) 
		+ sizeof(pFrame->Interval) + sizeof(pFrame->Capability);
	
	PINFO_ELEMENT pIE = (PINFO_ELEMENT) (pbuf + fsize);
	pIE->eid  = 0; // ssid;
	pIE->elen = (UCHAR) adhoc_ssid_len;
	strncpy ( (char*)pIE->data, adhoc_ssid, adhoc_ssid_len );
	fsize += pIE->elen + 2;

	pIE = (PINFO_ELEMENT) (pbuf + fsize );
	if ( gPHYMode == PHY_802_11A ) {
		pIE->eid = 1; // supported rate
		pIE->elen = 8;
		for ( int i=0; i<8; i++ ) {
			pIE->data[i] = support_rate11a[i];
		}
	}else {
		pIE->eid = 1; // supported rate
		pIE->elen = 4;
		pIE->data[0] = 0x82; // 1M
		pIE->data[1] = 0x84; // 2M
		pIE->data[2] = 0x8B; // 5.5M
		pIE->data[3] = 0x96; // 11M
	}
	fsize += pIE->elen + 2;

	pIE = (PINFO_ELEMENT) (pbuf + fsize );
	pIE->eid = 3; // Direct Sequence Parameter Set
	pIE->elen = 1;
	pIE->data[0] = (char)GetChannelNo ( gChannelFreq );
	fsize += pIE->elen + 2;

	pIE = (PINFO_ELEMENT) (pbuf + fsize );
	pIE->eid = 6; // Direct Sequence Parameter Set
	pIE->elen = 2;
	*((WORD*) &pIE->data[0]) = 0; 
	fsize += pIE->elen + 2;

	ModulateBuffer11 (pbuf, fsize, FALSE );
}


bool IncreaseRate () {
	int dr = gDataRateK;
	if ( gPHYMode == PHY_802_11A ) {
		switch (gDataRateK) {
		case 6000:
			gDataRateK = 9000;
			break;
		case 9000:
			gDataRateK = 12000;
			break;
		case 12000:
			gDataRateK = 18000;
			break;
		case 18000:
			gDataRateK = 24000;
			break;
		case 24000:
			gDataRateK = 36000;
			break;
		case 36000:
			gDataRateK = 48000;
			break;
		case 48000:
			gDataRateK = 54000;
			break;
		default:
            return false;
		}
	}	
	else if (gPHYMode == PHY_802_11B) {
		switch (gDataRateK) {
		case 1000:
			gDataRateK = 2000;
			break;
		case 2000:
			gDataRateK = 5500;
			break;
		case 5500:
			gDataRateK = 11000;
			break;
		default:
            return false;
        }
	}

	return true;
}


bool DecreaseRate () {
	int dr = gDataRateK;
	if ( gPHYMode == PHY_802_11A ) {
		switch (gDataRateK) {
		case 9000:
			gDataRateK = 6000;
			break;
		case 12000:
			gDataRateK = 9000;
			break;
		case 18000:
			gDataRateK = 12000;
			break;
		case 24000:
			gDataRateK = 18000;
			break;
		case 36000:
			gDataRateK = 24000;
			break;
		case 48000:
			gDataRateK = 36000;
			break;
		case 54000:
			gDataRateK = 48000;
			break;
		default:
            return false;
		}
		return (dr != gDataRateK );
	}	
	else if (gPHYMode == PHY_802_11B) {
		switch (gDataRateK) {
		case 2000:
			gDataRateK = 1000;
			break;
		case 5500:
			gDataRateK = 2000;
			break;
		case 11000:
			gDataRateK = 5500;
			break;
		default:
            return false;
        }
	}

    return true;
}

//
// maintain association
//
void Check_Association () {
	if ( nBeaconRcvCnt ) {
		// We have received any beacon
		if ( assoState == ASSO_NONE ) {
			mgmtPkt |= MGMT_PKT_AUTH;
			assocTimer  = 0;
		} else if ( assoState != ASSO_DONE ) // in association
		{
			if ( (assocTimer++) > 50 ) // 5 second
			{
				assoState = ASSO_NONE;
				assocTimer  = -1;
			}

			if ( assoState == ASSO_AUTH ) {
				mgmtPkt |= MGMT_PKT_ASSO;
			}
		} else if ( assoState == ASSO_DONE ) 
		{
			// associated
			if ( (assocTimer++) > 500 ) // 50 second
			{
				// keep alive
				assocTimer = 0;
				fKeepAlive = 1;
			}
		}
	}
}

void Check_RxGain () {
	if ( OpMode == CLIENT_MODE ) {
		if ( rxgState == RXGAIN_SEARCH ) 
		{
			if ( goodBeaconCnt > 3 ) {
				// We have found a good setting
				rxgState = RXGAIN_TRACK;
				RxBeaconTimer = 0;
				RxGainTimer   = 0;
				return;
			}

			if ( RxBeaconTimer ++ > 6 ) {
				// We have missed too much beacon in this configuration
				if  ( !DecreaseGain () ) {
					// if we already reach the minimal gain 
					// start over
					SetMaximalGain ();
				}
				goodBeaconCnt = 0;
				RxBeaconTimer = 0;
			}
		} else if ( rxgState == RXGAIN_TRACK ) {
			// tracking
			if ( RxBeaconTimer ++ > 29 ) 
			{
				// too many beacons missing
				SetMaximalGain ();
				rxgState = RXGAIN_SEARCH;
				
				RxBeaconTimer = 0;
				goodBeaconCnt = 0;
			}

			if ( RxGainTimer ++ > 10 ) {
				GainTracking ();

				RxGainTimer   = 0;
			}
		}
	}	
}

void AutoConfigure () {
	if ( OpMode == CLIENT_MODE ) {
		Check_Association ();
		Check_RxGain ();
	} else {
		// ad hoc mode
		// send beacon
		mgmtPkt |= MGMT_PKT_BEACON;
	}
	
}

void ProcessBeacon (PDOT11_MAC_BEACON beacon) 
{
	nBeaconRcvCnt ++;
#if 0	
	PlotText ("mgmt:ssid", (char*)&beacon->SSID.SSID[0], beacon->SSID.Length);

	int ch[64];
	for ( int i=0; i<31; i++) {
		ch[32+i] = RxContext.channelFactor[i].re * RxContext.channelFactor[i].re + 
			RxContext.channelFactor[i].im * RxContext.channelFactor[i].im;
	}
	
	for ( int i=63; i>=32; i--) {
		ch[i-32] = RxContext.channelFactor[i].re * RxContext.channelFactor[i].re + 
			RxContext.channelFactor[i].im * RxContext.channelFactor[i].im;
	}

	PlotSpectrum ("BB:Channel", ch, 64);
#endif

	if ( OpMode == CLIENT_MODE ) {
		// Update freqOffset
		UpdateFreqOffset ();

		if ( assoState != ASSO_DONE ) {
			// not associated yet
		    if ( 
				beacon->SSID.SSID[0] == 's' &&
				beacon->SSID.SSID[1] == 'd' &&
		        beacon->SSID.SSID[2] == 'r') 
		    {
		        unsigned int i;
			
		        for (i = 0; i < 6; i++) {
		            bssid.Address[i] = beacon->MacHeader.Address3.Address[i];
				}

				ssid_len = beacon->SSID.Length;
				for (i = 0; i < ssid_len; i++ ) {
					ssid[i] = beacon->SSID.SSID[i];
				}
				ssid[ssid_len] = 0;

				UpdateEnergy ();
				
				if ( rxgState == RXGAIN_SEARCH ) {
					goodBeaconCnt ++;
				}
		    }
		} else 
		if ( memcmp (bssid.Address, beacon->MacHeader.Address3.Address,sizeof(bssid.Address) ) == 0 )
		{
			// already associated - check if this is the beacon from the corrent AP
			UpdateEnergy ();

			if ( rxgState == RXGAIN_SEARCH ) {
				goodBeaconCnt ++;
			}

			RxBeaconTimer = 0;
		}
	}	else { 
		// ad hoc mode
		if ( beacon->SSID.Length == adhoc_ssid_len &&
			 memcmp (beacon->SSID.SSID, adhoc_ssid, adhoc_ssid_len ) == 0 )
		{
			UpdateEnergy ();

			if ( rxgState == RXGAIN_SEARCH ) {
				goodBeaconCnt ++;
			}

			RxBeaconTimer = 0;
		}
	}
}


