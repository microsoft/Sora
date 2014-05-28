#pragma once
#include <sora.h>
#include "umxsdr.h"


#define MAX_RX_GAIN 56
#define MIN_RX_GAIN 2

enum ASSO_STATE {
	ASSO_NONE,
	ASSO_AUTH,	
	ASSO_DONE,
};

extern ASSO_STATE assoState;

#define MGMT_PKT_BEACON 0x01
#define MGMT_PKT_AUTH   0x02
#define MGMT_PKT_ASSO   0x04
#define MGMT_PKT_NULL   0x08
#define MGMT_PKT_TEST   0x10


extern ULONG mgmtPkt; // the control packet

// statistics
extern int err_stat[3];

extern ULONG                nDataPacketRcvCnt;
extern ULONG                nDataPacketSndCnt;
extern ULONG                nDataPacketDropCnt;
extern ULONG                nAckRcvCnt;
extern ULONG                nBeaconRcvCnt;
extern ULONG                nInterferenceCnt;

extern ULONG 				energy_max;

extern MAC_ADDRESS			lastMACSrc;
extern MAC_ADDRESS			lastMACDst;
extern MAC_ADDRESS			lastMACBssid;
extern int					lastRate;

// Rx Gain Setting
extern ULONG                RxGain; 
extern ULONG                RxPa;
extern ULONG                RxG;

// maintainence
extern int			  		assocTimer;
extern int 					fKeepAlive;

extern ULONG				RxGainTimer;
extern ULONG				RxBeaconTimer;
extern ULONG				RxBeaconCnt;

extern ULONG 				fAutoCfg;

// operation mode
#define CLIENT_MODE    0
#define ADHOC_MODE 1
extern int OpMode;

// connection maintanence
void AutoConfigure ();

void KeepAlive () ;
void Auth () ;
void Beacon () ;

void Associate () ;
void TestPkt ();

ulong RequestMgmtPkt ( ulong pkttype );

void UpdateEnergy ();

void print_status ();

void ConfigureRxGain ( int RxGain );
bool IncreaseGain ();
bool DecreaseGain ();
bool IncreaseRate ();
bool DecreaseRate ();




void ProcessBeacon (PDOT11_MAC_BEACON beacon);

