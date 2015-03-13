/*++
Copyright (c) Microsoft Corporation

Module Name: _ref_conf.h

Abstract: This header file defines structs for batch register configuration. 

--*/


#ifndef _REG_CONF_H
#define _REG_CONF_H

typedef struct __PHY_CHANNEL_SELECTOR
{
    ULONG       ChannelNumber;
    ULONG       Reg1, Reg2, Reg3; // To change the channel, we have to write 3 registers, see MAXIM manual
}__PHY_CHANNEL_SELECTOR, *__PPHY_CHANNEL_SELECTOR;

extern const __PHY_CHANNEL_SELECTOR    __gc_Phy_2dot4GHz_ChannelSelectors[];
extern const __PHY_CHANNEL_SELECTOR    __gc_Phy_5GHz_ChannelSelectors[];

typedef struct ___REG_CONFIGURATION_ENTRY{
    ULONG        RegAddress;
    ULONG        RegValue;

    ULONG        fNeedConfirm;
    ULONG        MaxConfirmTimes;

    ULONG        fDependent;
    ULONG        DependentRegAddress;
    ULONG        DependentRegBitMask;
    ULONG        DependentRegValue;
    LONG         DependentTimeOuts;
}__REG_CONFIGURATION_ENTRY, *__PREG_CONFIGURATION_ENTRY;


#endif