/*++
Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:
   ioctrl.h

Abstract:
    This module contains device IO control code definition.

Revision History:
    Created by senxiang, 21/Aug/2009

Notes:
    It is the common header file for both user mode utility and 
    kernel mode driver.
--*/
#ifndef _IO_CTRL_H
#define _IO_CTRL_H
#pragma once

#define NIC_DEVICE_NAME                     L"\\Device\\SdrMiniport"
#define NIC_LINK_NAME                       L"\\??\\SdrMiniport"
#define NIC_ANSI_NAME                       "\\\\.\\SdrMiniport"

#define IOCTL_SET_DATA_RATE CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x800,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

#define IOCTL_SET_PREAMBLE_TYPE CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x803,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

#define IOCTL_SET_CHANNEL CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x804,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

#define IOCTL_SET_RX_GAIN CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x805,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

#define IOCTL_SET_RX_DUMP CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x806,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

#define IOCTL_SET_TX_GAIN CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x807,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

#define IOCTL_SET_SPD_MAX_BLOCK CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x808,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

#define IOCTL_GET_INFO CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x809,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

#define IOCTL_SET_SPD_POWER_THRESHOLD CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x80a,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

#define IOCTL_DISPLAY_DEBUG_REGS CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x80b,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

#define IOCTL_SET_SPD_POWER_THRESHOLD_LH CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x80c,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

#define IOCTL_SET_SPD_POWER_THRESHOLD_HL CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x80d,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

#define IOCTL_SET_RX_GAIN_PRESET0 CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x80e,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

#define IOCTL_SET_RX_GAIN_PRESET1 CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x80f,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

#define IOCTL_SET_RX_PA CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x810,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

#define IOCTL_SET_PHY_MODE CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x811,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

#define IOCTL_SET_FREQ_OFFSET CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x812,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

#define IOCTL_UPDATE_MAC_ADDR CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x813,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

#define IOCTL_SET_SAMPLE_RATE CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x814,\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)


typedef struct _STATISTIC
{
    /*Send Queue*/
    ULONG           nFreeTCB;
    ULONG           nSrcPacket;
    ULONG           nSymPacket;
    ULONG           nCompletePacket;

    /*Recv Queue*/
    ULONG           nFreeRCB;
    ULONG           nPendingRXPackets;

    /* RX, TX internal status */
    ULONG64         ullGoodTransmits;
    ULONG64         ullGoodReceives;
    ULONG64         ullTransmitFail;
    ULONG64         ullReceiveErrors; //PHY Errors;
    ULONG64         ullDuplicatedReceives;

    /* RX, SPD internal configuration */
    ULONG           ulSpdEnergy;
    ULONG           ulDataRate;
    ULONG           ulPreambleType;
    ULONG           ulSpdPowerThreshold;
    ULONG           ulSpdPowerThresholdLH;
    ULONG           ulSpdPowerThresholdHL;
    ULONG           ulGainLevel;
    ULONG           ulRxGainPreset0;
    ULONG           ulRxGainPreset1;
    ULONG           ulRxGain;
    ULONG           ulRxPa;
    ULONG           ulTxGain;
    ULONG           ulRxShift;
    ULONG           ulSampleRate;

    BYTE            MACAddr[6];
}STATISTIC, *PSTATISTIC;

enum
{
    DATA_RATE_11000                         = 11000,
    DATA_RATE_5500                          = 5500,
    DATA_RATE_2000                          = 2000,
    DATA_RATE_1000                          = 1000
};

#endif //_IO_CTRL_H
