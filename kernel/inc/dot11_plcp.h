/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name: dot11_plcp.h

Abstracts:
    this file defines 802.11 physical layer PLCP header format and TX vector.

Revision History:

Notes: 

--*/

#ifndef _PLCP11B_H
#define _PLCP11B_H


#pragma warning( disable : 4200 )
#pragma warning( disable : 4214 )

#define DOT11B_PLCP_LONG_PREAMBLE_SYNC_VALUE            0xFF
#define DOT11B_PLCP_SHORT_PREAMBLE_SYNC_VALUE           0x00

#define DOT11B_PLCP_LONG_PREAMBLE_SYNC_LENGTH           16
#define DOT11B_PLCP_SHORT_PREAMBLE_SYNC_LENGTH          7

#define DOT11B_PLCP_HEADER_LENGTH                       6
#define DOT11B_PLCP_LONG_PREAMBLE_LENGTH                18
#define DOT11B_PLCP_SHORT_PREAMBLE_LENGTH               9

#define DOT11B_PLCP_LONG_PREAMBLE_SFD                   0xF3A0
#define DOT11B_PLCP_SCRAMBLED_LONG_PREAM_SFD            0x0949
#define DOT11B_PLCP_SHORT_PREAMBLE_SFD                  0x05CF
#define DOT11B_PLCP_SCRAMBLED_SHORT_PREAM_SFD           0x8C7F
#define DOT11B_PLCP_SFD_LENGTH_IN_BITS                  16

#define DOT11B_PLCP_LONG_TX_SCRAMBLER_REGISTER          0x6C
#define DOT11B_PLCP_SHORT_TX_SCRAMBLER_REGISTER         0x1B

#define DOT11B_PLCP_LONG_LENGTH                         24
#define DOT11B_PLCP_SHORT_LENGTH                        15

#define DOT11B_PLCP_EVEN_SYMBOL                         0
#define DOT11B_PLCP_ODD_SYMBOL                          1

#define DOT11B_PLCP_IS_LONG_PREAMBLE                    0
#define DOT11B_PLCP_IS_SHORT_PREAMBLE                   1
#define DOT11B_PLCP_IS_NOT_PREAMBLE                     2

#define DOT11B_PLCP_IS_CCK                              0
#define DOT11B_PLCP_IS_PBCC                             1

typedef struct _DOT11B_PLCP_TXVECTOR{
    UCHAR     DateRate;         /* Data rate for modulation */
    UCHAR     PreambleType;     /* 0=LONG, 1=SHORT*/
    UCHAR     ModSelect;        /* 0=CCK, 1=PBCC */
}DOT11B_PLCP_TXVECTOR, *PDOT11B_PLCP_TXVECTOR;

#pragma pack(push, 1)

/************************************************************************
* Long PLCP Preamble
* 128bits SYNC & 16bits SFD
************************************************************************/
typedef struct _DOT11B_PLCP_LONG_PREAMBLE
{
    UCHAR         Sync[DOT11B_PLCP_LONG_PREAMBLE_SYNC_LENGTH];   //128bits
    USHORT        SFD;        //16bits
}DOT11B_PLCP_LONG_PREAMBLE, *PDOT11B_PLCP_LONG_PREAMBLE;


typedef union _DOT11B_PLCP_HEADER_SERVICE
{
    struct _SERVICE_IN_BIT{
        UCHAR Reserved0 : 1;
        UCHAR Reserved1 : 1;
        UCHAR LockedBit : 1; // 0=not, 1=locked
        UCHAR ModSelect : 1; // 0=CCK, 1=PBCC
        UCHAR Reserved4 : 1;
        UCHAR Reserved5 : 1;
        UCHAR Reserved6 : 1;
        UCHAR LengthExt : 1;
    }Bits;
    UCHAR bValue;
}DOT11B_PLCP_HEADER_SERVICE, *PDOT11B_PLCP_HEADER_SERVICE;


/************************************************************************
* Short PLCP Preamble
* 56bits SYNC & 16bits SFD
************************************************************************/
typedef struct _DOT11B_PLCP_SHORT_PREAMBLE
{
    UCHAR         Sync[DOT11B_PLCP_SHORT_PREAMBLE_SYNC_LENGTH];     //56bits
    USHORT        SFD;          //16bits
}DOT11B_PLCP_SHORT_PREAMBLE, *PDOT11B_PLCP_SHORT_PREAMBLE;


/************************************************************************
* PLCP Header
* 8bits Signal & 8bits Service & 16bits length & 16bits CRC16 CCITT
************************************************************************/
typedef struct _DOT11B_PLCP_HEADER
{
    UCHAR                                           Signal;
    DOT11B_PLCP_HEADER_SERVICE                      Service;
    union
    {
        USHORT                                      Length;
        UCHAR                                       LengthBytes[2];
    };
    USHORT                                          CRC;
}DOT11B_PLCP_HEADER, *PDOT11B_PLCP_HEADER;


/************************************************************************
* Dot11 PLCP long frame
************************************************************************/
typedef struct _DOT11B_PLCP_LONG_FRAME
{
    DOT11B_PLCP_LONG_PREAMBLE      Preamble;
    DOT11B_PLCP_HEADER             Header;
    UCHAR                          PSDU[0];
}DOT11B_PLCP_LONG_FRAME, *PDOT11B_PLCP_LONG_FRAME;


/************************************************************************
* Dot11 PLCP short frame
************************************************************************/
typedef struct _DOT11B_PLCP_SHORT_FRAME
{
    DOT11B_PLCP_SHORT_PREAMBLE       Preamble;
    DOT11B_PLCP_HEADER               Header;
    UCHAR                            PSDU[0];
}DOT11B_PLCP_SHORT_FRAME, *PDOT11B_PLCP_SHORT_FRAME;

#pragma pack(pop)

#endif // _PLCP11B_H