/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:
   dot11config.h

Abstract:
    This file declares 802.11 sample configuration utility supported options.

Revision History:
    Created by senxiang, 11/Nov/2009

Notes:
    
--*/
#ifdef __cplusplus
 extern "C" {
#endif 
#include <stdio.h>
#include <windows.h>
#include <winioctl.h> //CTL_CODE
#include <stdlib.h>
#include <strsafe.h>
#include <assert.h>
#include "..\\ioctrl.h"
#ifdef __cplusplus
 } //end extern "C" {
#endif 
#include "args.h"

typedef struct _CMD_INFO
{
    DWORD IoCtrlCode;
    union
    {
        UCHAR Data[12];
        struct Params
        {
            ULONG Parameter;
            ULONG Reserved1;
            ULONG Reserved2;
        };
    };
} CMD_INFO, *PCMD_INFO;

// Note: Do not change the orders of option categories
enum 
{
    ARG_TABLE_BEGIN = -1,

    /* int options */
    DATA_RATE_OPT,
    CHANNEL_OPT,
    FREQ_OFFSET_OPT, 
    PREAMBLE_OPT,
    SPD_MAX_BLOCK, 
    SPD_POWER_THRESHOLD,
    SPD_POWER_THRESHOLD_LH,
    SPD_POWER_THRESHOLD_HL,
    RXGAIN_PRESET0,
    RXGAIN_PRESET1,
    RXGAIN_OPT,
    RXPA_OPT, 
    TXGAIN_OPT,
    RX_SHIFT,
    SAMPLE_RATE_OPT,
    ARG_TABLE_INT_END,

    /* literal options */
    DUMP_OPT = ARG_TABLE_INT_END,
    INFO_OPT,
    READ_REGS_OPT,
    ARG_TABLE_LITERAL_END,

    /* string options */
    UPDATE_MAC_ADDR_OPT = ARG_TABLE_LITERAL_END,
    ARG_TABLE_END
};

typedef BOOLEAN (*VALUECHECK)(ULONG Value);

__inline BOOLEAN DataRateCheck(ULONG DataRate)
{
    return (DataRate == 11000 || 
            DataRate == 5500 ||
            DataRate == 2000 || 
            DataRate == 1000 || 
            DataRate == 6000 ||
            DataRate == 9000 ||
            DataRate == 12000 ||
            DataRate == 18000 ||
            DataRate == 24000 ||
            DataRate == 36000 || 
            DataRate == 48000 ||
            DataRate == 54000 );
}

__inline BOOLEAN PreambleTypeCheck(ULONG Type)
{
    return (Type == 0  || Type == 1);
}

__inline BOOLEAN In5_4GChannel(ULONG ChannelNo)
{
    BOOLEAN InSec1 = (ChannelNo >=36 && ChannelNo <= 140 && ((ChannelNo - 36) % 4 == 0));
    BOOLEAN InSec2 = (ChannelNo >= 149 && ChannelNo <=161 && ((ChannelNo - 149) % 4 == 0));
    BOOLEAN InSec3 = (ChannelNo == 162 || ChannelNo == 163);
    return (InSec1 || InSec2 || InSec3);
}

__inline BOOLEAN CentralFreqCheck(ULONG ChannelNo)
{
    return  ((ChannelNo > 0 && ChannelNo <= 15) || In5_4GChannel(ChannelNo));
}

__inline 
HRESULT 
IntArg2IoCode(
    OUT PCMD_INFO Cmd, 
    PARG_INT IntArg,
    VALUECHECK Checker, 
    DWORD Code)
{
    HRESULT hr = S_OK;
    if (Checker != NULL)
    {
        if (!Checker(IntArg->Values[0]))
            hr = E_FAIL;
    }
    if (SUCCEEDED(hr))
    {
        Cmd->IoCtrlCode = Code;
        Cmd->Parameter = IntArg->Values[0];
    }
    return hr;
}
