/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:
   main.c

Abstract:
    This file implements commandline frontend of 802.11 sample configuration utility.

Revision History:
    Created by senxiang, 11/Nov/2009

Notes:
    
--*/
#include "dot11config.h"
#include "args.h"

void AboutMe()
{
    printf("Sora 802.11b sample configuration utility.\n\n");
}

PARG_BASE *
SupportedOptions()
{
    PARG_BASE   *ArgTable = malloc(sizeof(PARG_BASE) * ARG_TABLE_END);

    int i;
    for (i = 0; i < ARG_TABLE_INT_END; i++)
        ArgTable[i] = (PARG_BASE)malloc(sizeof(ARG_INT));

    for (i = ARG_TABLE_INT_END; i < ARG_TABLE_LITERAL_END; i++)
        ArgTable[i] = (PARG_BASE)malloc(sizeof(ARG_LITERAL));

    for (i = ARG_TABLE_LITERAL_END; i < ARG_TABLE_END; i++)
        ArgTable[i] = (PARG_BASE)malloc(sizeof(ARG_STR));

    ArgLiteralCtor((PARG_LITERAL)ArgTable[DUMP_OPT], "d", "dump", NULL, "Dump RX signals into file which will be saved at c:\\ with timestamp in file name", 0, 1);
    ArgLiteralCtor((PARG_LITERAL)ArgTable[INFO_OPT], "i", "info", NULL, "Show 802.11 adapter running status", 0, 1);
    ArgLiteralCtor((PARG_LITERAL)ArgTable[READ_REGS_OPT], "g", "regs", NULL, "Display Debug registers", 0, 1);
    
    ArgStrCtor((PARG_STR)ArgTable[UPDATE_MAC_ADDR_OPT], "a", "setmacaddr", NULL, "Set MAC address for sdr", 0, 1);

    ArgIntCtor((PARG_INT)ArgTable[RXGAIN_PRESET0], NULL, "rxgain_preset0", "[gain value]", "Set radio RX gain preset0 to gain value in 1/256 db, i.e., 0x2400", 0, 1);
    ArgIntCtor((PARG_INT)ArgTable[RXGAIN_PRESET1], NULL, "rxgain_preset1", "[gain value]", "Set radio RX gain preset1 to gain value in 1/256 db, i.e., 0x2400", 0, 1);
    ArgIntCtor((PARG_INT)ArgTable[RXGAIN_OPT], "R", "rxgain", "[gain value]", "Set radio RX gain to gain value in 1/256 db, i.e., 0x2400", 0, 1);
    ArgIntCtor((PARG_INT)ArgTable[RXPA_OPT], NULL, "rxpa", "[power level]", "Set radio RX PA, can only be 0, 0x1000, 0x2000, 0x3000", 0, 1);
    ArgIntCtor((PARG_INT)ArgTable[TXGAIN_OPT], "T", "txgain", "[gain value]", "Set radio TX gain to gain value in 1/256 db, i.e., 0x1F80", 0, 1);
    ArgIntCtor((PARG_INT)ArgTable[DATA_RATE_OPT], "r", "datarate", "[Kbps]", "Transmission data rate, i.e., 11000 for 11Mbps, 5500 for 5.5Mbps, etc", 0, 1);
    ArgIntCtor((PARG_INT)ArgTable[CHANNEL_OPT], "c", "channel", "[channel NO.]", "Channel 1:2412MHz, 2:2417MHz, ..., 15: 2482MHz, 36:5180MHz, 40:5200MHz ...", 0, 1);
    ArgIntCtor((PARG_INT)ArgTable[FREQ_OFFSET_OPT], "f", "freqoffset", "[Hz]", "Set frequency offset, can be negative", 0, 1);
    ArgIntCtor((PARG_INT)ArgTable[PREAMBLE_OPT], NULL, "preamble", "[0/1]", "0/1 for long/short preamble", 0, 1);
    ArgIntCtor((PARG_INT)ArgTable[SPD_MAX_BLOCK], "s", "spdmax", "[blocks]", "Set 802.11b power detection block timeout", 0, 1);
    ArgIntCtor((PARG_INT)ArgTable[SPD_POWER_THRESHOLD], "t", "spdthd", "[energy]", "Set energy threshold for 802.11b power detection", 0, 1);
    ArgIntCtor((PARG_INT)ArgTable[SPD_POWER_THRESHOLD_LH], NULL, "spdthd_lh", "[energy]", "Set energy threshold for 802.11b power detection", 0, 1);
    ArgIntCtor((PARG_INT)ArgTable[SPD_POWER_THRESHOLD_HL], NULL, "spdthd_hl", "[energy]", "Set energy threshold for 802.11b power detection", 0, 1);
    ArgIntCtor((PARG_INT)ArgTable[RX_SHIFT], "S", "shift", "[bits]", "Set shift bits after downsampling", 0, 1);
    ArgIntCtor((PARG_INT)ArgTable[SAMPLE_RATE_OPT], "p", "samplerate", "[MHz]", "Set sample rate of the radio PCB", 0, 1);
    
    return ArgTable;
}

void SupportedOptionsDtor(PARG_BASE *Options)
{
    int i;
    for (i = ARG_TABLE_BEGIN + 1; i < ARG_TABLE_END; i++)
    {
        free(Options[i]);
    }
    free(Options);
}

HRESULT ParseMACAddr(
    OUT PCMD_INFO Cmd,
    const char *szAddr
)
{
    unsigned short AddrBytes[6];
    int cItem;
    int i;

    cItem = sscanf(szAddr, "%hx-%hx-%hx-%hx-%hx-%hx",
        &AddrBytes[0], &AddrBytes[1], &AddrBytes[2],
        &AddrBytes[3], &AddrBytes[4], &AddrBytes[5]);
    if (cItem != 6)
    {
        cItem = sscanf(szAddr, "%hx:%hx:%hx:%hx:%hx:%hx",
            &AddrBytes[0], &AddrBytes[1], &AddrBytes[2],
            &AddrBytes[3], &AddrBytes[4], &AddrBytes[5]);
        if (cItem != 6)
        {
            return E_FAIL;
        }
    }

    printf("MacAddr=[%02X-%02X-%02X-%02X-%02X-%02X]\n",
        AddrBytes[0], AddrBytes[1], AddrBytes[2],
        AddrBytes[3], AddrBytes[4], AddrBytes[5]);    

    for (i = 0; i < 6; i++)
    {
        Cmd->Data[i] = (UCHAR)AddrBytes[i];
    }
    
    Cmd->IoCtrlCode     = IOCTL_UPDATE_MAC_ADDR;
    return S_OK;
}

HRESULT GetCmd(
    OUT PCMD_INFO Cmd, 
    int      argc, 
    char     **argv)
{
    HRESULT hr;
    PARG_BASE       *Options;
    int i;
    Options = SupportedOptions();

    hr = ArgParse(Options, ARG_TABLE_END, argc, argv);
    if (SUCCEEDED(hr))
    {
        hr = S_OK;        
        if (((PARG_LITERAL)Options[READ_REGS_OPT])->Count)
        {
            Cmd->IoCtrlCode     = IOCTL_DISPLAY_DEBUG_REGS;  
        }
        if (((PARG_LITERAL)Options[DUMP_OPT])->Count)
        {
            Cmd->IoCtrlCode     = IOCTL_SET_RX_DUMP;
        }
        if (((PARG_LITERAL)Options[INFO_OPT])->Count)
        {
            Cmd->IoCtrlCode     = IOCTL_GET_INFO;
        }
        if (((PARG_STR)Options[UPDATE_MAC_ADDR_OPT])->Count)
        {
            hr = ParseMACAddr(Cmd, ((PARG_STR)Options[UPDATE_MAC_ADDR_OPT])->Values[0]);
        }
        if (((PARG_INT)Options[PREAMBLE_OPT])->Count)
        {
            hr = IntArg2IoCode(Cmd, (PARG_INT)Options[PREAMBLE_OPT], PreambleTypeCheck, IOCTL_SET_PREAMBLE_TYPE);
        }
        if (((PARG_INT)Options[CHANNEL_OPT])->Count)
        {
            hr = IntArg2IoCode(Cmd, (PARG_INT)Options[CHANNEL_OPT], CentralFreqCheck, IOCTL_SET_CHANNEL);
        }
        if (((PARG_INT)Options[FREQ_OFFSET_OPT])->Count)
        {
            hr = IntArg2IoCode(Cmd, (PARG_INT)Options[FREQ_OFFSET_OPT], NULL, IOCTL_SET_FREQ_OFFSET);
        }
        if (((PARG_INT)Options[RXGAIN_PRESET0])->Count)
        {
            hr = IntArg2IoCode(Cmd, (PARG_INT)Options[RXGAIN_PRESET0], NULL, IOCTL_SET_RX_GAIN_PRESET0);
        }
        if (((PARG_INT)Options[RXGAIN_PRESET1])->Count)
        {
            hr = IntArg2IoCode(Cmd, (PARG_INT)Options[RXGAIN_PRESET1], NULL, IOCTL_SET_RX_GAIN_PRESET1);
        }
        if (((PARG_INT)Options[RXGAIN_OPT])->Count)
        {
            hr = IntArg2IoCode(Cmd, (PARG_INT)Options[RXGAIN_OPT], NULL, IOCTL_SET_RX_GAIN);
        }
        if (((PARG_INT)Options[RXPA_OPT])->Count)
        {
            hr = IntArg2IoCode(Cmd, (PARG_INT)Options[RXPA_OPT], NULL, IOCTL_SET_RX_PA);
        }
        if (((PARG_INT)Options[TXGAIN_OPT])->Count)
        {
            hr = IntArg2IoCode(Cmd, (PARG_INT)Options[TXGAIN_OPT], NULL, IOCTL_SET_TX_GAIN);
        }
        if (((PARG_INT)Options[DATA_RATE_OPT])->Count)
        {
            hr = IntArg2IoCode(Cmd, (PARG_INT)Options[DATA_RATE_OPT], DataRateCheck, IOCTL_SET_DATA_RATE);
        }
        if (((PARG_INT)Options[SPD_MAX_BLOCK])->Count)
        {
            hr = IntArg2IoCode(Cmd, (PARG_INT)Options[SPD_MAX_BLOCK], NULL, IOCTL_SET_SPD_MAX_BLOCK);
        }
        if (((PARG_INT)Options[SPD_POWER_THRESHOLD])->Count)
        {
            hr = IntArg2IoCode(Cmd, (PARG_INT)Options[SPD_POWER_THRESHOLD], NULL, IOCTL_SET_SPD_POWER_THRESHOLD);
        }
        if (((PARG_INT)Options[SPD_POWER_THRESHOLD_LH])->Count)
        {
            hr = IntArg2IoCode(Cmd, (PARG_INT)Options[SPD_POWER_THRESHOLD_LH], NULL, IOCTL_SET_SPD_POWER_THRESHOLD_LH);
        }
        if (((PARG_INT)Options[SPD_POWER_THRESHOLD_HL])->Count)
        {
            hr = IntArg2IoCode(Cmd, (PARG_INT)Options[SPD_POWER_THRESHOLD_HL], NULL, IOCTL_SET_SPD_POWER_THRESHOLD_HL);
        }
        if (((PARG_INT)Options[SAMPLE_RATE_OPT])->Count)
        {
            hr = IntArg2IoCode(Cmd, (PARG_INT)Options[SAMPLE_RATE_OPT], NULL, IOCTL_SET_SAMPLE_RATE);
        }
    }

    if (FAILED(hr))
    {
        AboutMe();
        ArgsHelp(Options, ARG_TABLE_END);
    }
    SupportedOptionsDtor(Options);
    return hr;
}

HRESULT 
Dot11ConfigGetCmd(
    OUT PCMD_INFO Cmd, 
    int argc, 
    char **argv)
{
    Cmd->IoCtrlCode = 0;
    Cmd->Parameter  = 0;
    
    return GetCmd(Cmd, argc, argv);
}

ULONG _ConfigDevice(HANDLE hDevice, PCMD_INFO Cmd, PVOID output, ULONG size)
{
    DWORD dwOutput = 0;
    BOOL bRet = 
            DeviceIoControl(
                hDevice, 
                Cmd->IoCtrlCode,
                &Cmd->Parameter, 
                sizeof(CMD_INFO) - sizeof(Cmd->IoCtrlCode), 
                output, 
                size, 
                &dwOutput, 
                NULL);
    printf("IO Code = 0x%08x, Input=%u\n", Cmd->IoCtrlCode, Cmd->Parameter);
    if (bRet)
    {
        printf("Configuration Done\n");
    }
    else
    {
        printf("Configuration Fails\n");
    }
    return dwOutput;
}

void PrintInfo(PSTATISTIC Info) 
{
    printf("TX Queue Status:\n");
    printf("Free Transmission Capability: \t%u\n", Info->nFreeTCB);
    printf("Source Packets in Queue: \t%u\n", Info->nSrcPacket);
    printf("Symbolized Packets in queue: \t%u\n", Info->nSymPacket);
    printf("Completed Packets in queue: \t%u\n\n", Info->nCompletePacket);

    printf("RX Queue Status:\n");
    printf("Free Receive Capability:\t%u\n", Info->nFreeRCB);
    printf("Pending Received Packets: \t%u\n", Info->nPendingRXPackets);

    printf("\nRX/TX statistics:\n");
    printf("Packets up to TCP/IP: \t%lld\n", Info->ullGoodReceives);
    printf("Duplicated packets received: \t%lld\n", Info->ullDuplicatedReceives);

    printf("\nRX, SPD internal configuration:\n");
    printf("SPD energy: \t%u\n", Info->ulSpdEnergy);
    printf("DataRate: \t%u\n", Info->ulDataRate);
    printf("Preamble Type: \t%u\n", Info->ulPreambleType);
    printf("Spd Power Threshold: \t%u\n", Info->ulSpdPowerThreshold);
    printf("Spd Power ThresholdLH: \t%u\n", Info->ulSpdPowerThresholdLH);
    printf("Spd Power ThresholdHL: \t%u\n", Info->ulSpdPowerThresholdHL);
    printf("GainLevel: \t%u\n", Info->ulGainLevel);
    printf("Rx Gain Preset0: \t0x%X\n", Info->ulRxGainPreset0);
    printf("Rx Gain Preset1: \t0x%X\n", Info->ulRxGainPreset1);
    printf("Rx Gain: \t0x%X\n", Info->ulRxGain);
    printf("Rx Pa: \t0x%X\n", Info->ulRxPa);
    printf("Tx Gain: \t0x%X\n", Info->ulTxGain);
    printf("Rx Shift: \t%d\n", Info->ulRxShift);
    printf("Sample Rate: \t%d\n", Info->ulSampleRate);

    printf("MAC Addr: \t[%02X-%02X-%02X-%02X-%02X-%02X]\n", Info->MACAddr[0],
        Info->MACAddr[1],
        Info->MACAddr[2],
        Info->MACAddr[3],
        Info->MACAddr[4],
        Info->MACAddr[5]);
}

void ConfigDevice(LPCSTR szDeviceName, PCMD_INFO Cmd)
{
    HANDLE hDevice = NULL;
    STATISTIC Info;
    int i;

    hDevice = 
        CreateFile(
            szDeviceName,
            GENERIC_READ | GENERIC_WRITE,
            0,        // share mode none
            NULL,    // no security
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL );        // no template
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        AboutMe();
        printf("%s is not ready\n", szDeviceName);
        return;
    }
    if (Cmd->IoCtrlCode == IOCTL_GET_INFO)
    {
        ULONG ret = _ConfigDevice(hDevice, Cmd, &Info, sizeof(STATISTIC));
        printf("IOCTL_GET_INFO\n");
        if (ret != 0)
        {
            PrintInfo(&Info);
        }
    }
    else
    {
        _ConfigDevice(hDevice, Cmd, NULL, 0);
    }
    
    if (hDevice)
        CloseHandle(hDevice);

    return;
}

int __cdecl
main(int argc,char** argv)
{
#ifdef UNIT_TEST
    void UnitTest();
    UnitTest();
    return;
#else
    CMD_INFO Cmd;
    HRESULT hr;
    hr = Dot11ConfigGetCmd(&Cmd, argc - 1, argv + 1);
    if (SUCCEEDED(hr))
    {
        ConfigDevice(NIC_ANSI_NAME, &Cmd);
    }
    return 0;
#endif
}

