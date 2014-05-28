/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name: dot11_pkt.h .
  

Abstracts:
    this file defines packet header and frame format.
    it contains structure definitions for:
    
    Mac Address
    Ethernet packet header
    Wlan packet header

Revision History:
    Created by yichen, 2009/Apr/8
    Code refactory by senxiang 2009/Sep/1

Notes: 

--*/

#pragma once

//
// This file we define the useful header structure definitions
//

#pragma pack(push, 1)

typedef unsigned char  UINT8 ;
typedef unsigned short UINT16;

#define MAC_ADDRESS_LENGTH 6
typedef struct _MAC_ADDRESS 
{
    UCHAR Address[MAC_ADDRESS_LENGTH];
}MAC_ADDRESS, *PMAC_ADDRESS;

typedef struct _MAC_ADDRESS BSSID;

typedef struct tagEthernetHdr {
    MAC_ADDRESS   destAddr;
    MAC_ADDRESS   srcAddr;
    UINT16        Type;
}ETHERNET_HEADER, * PETHERNET_HEADER;

/* Values for frame type */
#define FRAME_MAN               0       /* management */
#define FRAME_CTRL              1       /* Control frame */
#define FRAME_DATA              2       /* Data frame */

#define SUBT_DATA               0       /* Just data */
#define SUBT_ASSO               1

#define SUBT_PROB_REQUEST       4
#define SUBT_PROB_RESPONSE      5
#define SUBT_BEACON             8       /* beacon management packet subtype */
#define SUBT_DEASSO             0xa
#define SUBT_AUTH             0xb
#define SUBT_DEAUTH             0xc

#define SUBT_ACK                13      /* ACK frame */



typedef struct {
    USHORT  Version : 2;                // Protocol Version
    USHORT  Type    : 2;
    USHORT  Subtype : 4;
    USHORT  ToDS    : 1;
    USHORT  FromDS  : 1;
    USHORT  MoreFrag: 1;
    USHORT  Retry   : 1;
    USHORT  PwrMgt  : 1;
    USHORT  MoreData: 1;
    USHORT  WEP     : 1;
    USHORT  Order   : 1;
} WLAN_FRAME_CTRL, * PWLAN_FRAME_CTRL;

typedef union tagSeqControl {
    struct {
        USHORT  FragmentNumber: 4;
        USHORT  SequenceNumber: 12;
    };
    USHORT usValue;
} WLAN_SEQUENCE_CONTROL, * PWLAN_SEQUENCE_CONTROL;


// For data frames, short header should be used
// when either FromDS=0 or ToDS=0
typedef struct tag80211Address3 {
    WLAN_FRAME_CTRL        FrameControl;
    USHORT                 DurationID;
    MAC_ADDRESS             Address1;
    MAC_ADDRESS             Address2;
    MAC_ADDRESS             Address3;
    WLAN_SEQUENCE_CONTROL  SequenceControl;
} WLAN_DATA_ADDRESS3_HEADER, * PWLAN_DATA_ADDRESS3_HEADER;

typedef struct _DOT11_MAC_ACK_FRAME
{
    WLAN_FRAME_CTRL        FrameControl;
    USHORT                 Duration;
    MAC_ADDRESS            RecvAddress;
    ULONG                  FCS;
}DOT11_MAC_ACK_FRAME, *PDOT11_MAC_ACK_FRAME;

typedef struct _DOT11_MAC_BEACON
{
    WLAN_DATA_ADDRESS3_HEADER   MacHeader;
    unsigned __int64            Timestamp;
    unsigned __int16            Interval;
    unsigned __int16            Capability;
    struct {
        unsigned __int8         ElementID;
        unsigned __int8         Length;
        unsigned char           SSID[1];
    }SSID;
    //......
}DOT11_MAC_BEACON, *PDOT11_MAC_BEACON;

typedef struct tagDOT11RFC1042ENCAP{
    WLAN_DATA_ADDRESS3_HEADER  MacHeader;
    struct  
    {
        UCHAR                  DSAP;                // 0xaa
        UCHAR                  SSAP;                // 0xaa
        UCHAR                  Control;             // 0x03
        UCHAR                  Encapsulation[3];    // 0x00-0x00-0x00
    }SNAP;
    UINT16                     Type;                // Copied from ethernet header
}DOT11RFC1042ENCAP, *PDOT11RFC1042ENCAP;

#define INPUT_MAX 4096

#pragma pack(pop)
