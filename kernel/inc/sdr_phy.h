/*++
Copyright (c) Microsoft Corporation

Module Name: sdr_phy.h

Abstract: 
      This file contains the data structures related to 802.11b sample PHY.
      In some way, we differentiate PHY and Base Band in Sora. 
      In Sora, Base Band is the main function block for digital signal processing.

      PHY has more general functions including some radio resource management,
      caching of pre-modulated frames, etc. 

History: 
          
--*/

#ifndef _DOT11B_PHY_H_
#define _DOT11B_PHY_H_
#pragma once

#include "sora.h"
#include "dot11_plcp.h"
#include "bb/bbb.h"

#include "bb/bba.h"

#include "..\phy\sdr_phy_ack_cache.h"

#define REQUIRED_RADIO_NUM    1
#define NIC_DRIVER_NAME       L"NicMiniport"

#define SAMPLE_BUFFER_SIZE    2*1024*1024
#define RX_BUFFER_SIZE        16*1024*1024

/* Kun review:
 * Default value - Using radio 0 for sending
 */
#define RADIO_SEND      0 
#define RADIO_RECV      0

/*
 * Default value - Using channel 3 for sending
 */
#define DEFAULT_CHANNEL         3
#define DEFAULT_CENTRAL_FREQ    (2422 * 1000) //kHz

/*
 * The following context contains the states specific for 802.11b 
 * baseband processing
 */
typedef struct __DOT11B_BB_CONTEXT
{
    BB11B_RX_CONTEXT        RxContext;  // states for 802.11b baseband demodulation
    DOT11B_PLCP_TXVECTOR    TxVector;   // vector for 802.11b baseband modulation
    BB11B_SPD_CONTEXT       SpdContext; // states for 802.11b baseband frame detection (software carrier sense / power detection (SPD))
    volatile ULONG          fCanWork;   // common flag for termination
} DOT11B_BB_CONTEXT,*PDOT11B_BB_CONTEXT;

typedef struct __DOT11A_BB_CONTEXT
{
    BB11A_RX_CONTEXT        RxContext;
    BB11A_TX_VECTOR         TxVector;
    volatile ULONG          fCanWork;
} DOT11A_BB_CONTEXT, *PDOT11A_BB_CONTEXT;

typedef struct _MP_ADAPTER  *PMP_ADAPTER;

typedef enum __PHY_MODE
{
	DOT_11_A = 0,
	DOT_11_B,
	MOD_MODE_CT,
	MOD_MODE_ERROR = MOD_MODE_CT
}PHY_MODE;

typedef struct _ULCB *PULCB;
typedef HRESULT (*FN_PHY_DOT11_CS)(IN OUT PPHY pPhy, UINT uRadio);
typedef HRESULT (*FN_PHY_DOT11_RX)(IN PPHY pPhy, IN UINT uRadio, IN PULCB pRCB);
typedef HRESULT (*FN_PHY_DOT11_MOD)( IN PPHY pPhy, IN PPACKET_BASE pPacket);
typedef ULONG (*FN_PHY_DOT11_MOD_ACK)(IN PPHY pPhy, MAC_ADDRESS RecvMacAddress, PVOID PhyACKBuffer);

/*
 *  The following structure contains state for a PHY object
 */
typedef struct _PHY{
    DOT11B_BB_CONTEXT           BBContextFor11B; // baseband context for 11b
    DOT11A_BB_CONTEXT           BBContextFor11A; //baseband context for 11a
    PHY_MODE                    PHYMode;
    FN_PHY_DOT11_CS             FnPHY_Cs;
    FN_PHY_DOT11_RX             FnPHY_Rx;
    FN_PHY_DOT11_MOD            FnPHY_Mod;
    FN_PHY_DOT11_MOD_ACK        FnPHY_ModAck;

    // PHY may maintain a list of radios that accosicate with it
    LIST_ENTRY                  RadiosHead;       
    PSORA_RADIO                 pRadios[MAX_RADIO_NUMBER];  // simply a quick access to the radio list
    HANDLE						TransferObj;

    // RxGain preset values, used in AGC
    ULONG                       RxGainPreset0;
    ULONG                       RxGainPreset1;

    // control flag - if the radios are properly initialized
    BOOLEAN                     bRadiosInitOK;

    volatile LONG               HwErrorNum;

    ACK_CACHE_MAN               AckCacheMgr;    //ACK physical frame cache manager

    // Statistic
    ULONG64                     ullReceiveCRC32Error;

	HANDLE						Thread;	
}PHY, *PPHY;

__inline PSORA_RADIO RadioInPHY(PPHY pPhy, ULONG Index)
{
    return pPhy->pRadios[Index];
}

__inline ULONG64 RxCRC32ErrorNumber(PPHY pPhy)
{
    return pPhy->ullReceiveCRC32Error;
}

__inline LARGE_INTEGER DemodStartTimeStampInPHY(PPHY pPhy)
{
    if (pPhy->PHYMode == DOT_11_A)
    {
        return pPhy->BBContextFor11A.RxContext.ullSignalFindTimeStamp;
    }
    else
    {
        return pPhy->BBContextFor11B.RxContext.BB11bCommon.b_SFDFindTimeStamp;
    }
}

__inline char PrevDataRateInPHY(PPHY pPhy)
{
    if (pPhy->PHYMode == DOT_11_A)
        return pPhy->BBContextFor11A.RxContext.bRate;
    else
        return (pPhy->BBContextFor11B.RxContext.BB11bCommon.b_prevDataRate >> 4);
}

#ifdef __cplusplus
extern "C"
{
#endif
HRESULT
SdrPhyInitialize(
    IN PPHY                     pPhy,
    IN PSDR_CONTEXT             SDRContext,
    HANDLE						TransferObj,
    IN ULONG                    ulRadioNum
    );

VOID
SdrPhyCleanUp(
    IN PPHY pPhy
    );

VOID SdrPhyStopHardware(IN PPHY pPhy);

VOID
SdrInitBBContextFor11B(
    IN PMP_ADAPTER          pAdapter,
    IN PDOT11B_BB_CONTEXT   pPhyContext
    );

VOID
SdrInitBBContextFor11A(
    IN PMP_ADAPTER		    pAdapter,
    IN PDOT11A_BB_CONTEXT   pBBContext);

HRESULT
PhyDot11BRx(
    IN      PPHY    pPhy, 
    IN      UINT    uRadio,
    IN      PULCB    pRCB
    );

HRESULT
PhyDot11ARx(
    IN      PPHY        pPhy, 
    IN      UINT        uRadio,
    IN      PULCB        pRCB
	);

HRESULT
PhyDot11BCs(IN PPHY pPhy, UINT uRadio);

HRESULT
PhyDot11ACs(IN PPHY pPhy, UINT uRadio);

HRESULT 
SdrPhyModulate11B( 
    IN PPHY             pPhy,
    IN PPACKET_BASE     pPacket);

HRESULT 
SdrPhyModulate11A( 
    IN PPHY             pPhy,
    IN PPACKET_BASE     pPacket);

ULONG 
SdrPhyModulateACK11B(
    PPHY Phy,
	MAC_ADDRESS RecvMacAddress,
	PVOID PhyACKBuffer);

ULONG 
SdrPhyModulateACK11A(
    PPHY Phy,
	MAC_ADDRESS RecvMacAddress,
	PVOID PhyACKBuffer);
#ifdef __cplusplus
}
#endif

#endif
