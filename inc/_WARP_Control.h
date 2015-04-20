/*++
Copyright (c) Microsoft Corporation

Module Name: _WARP_Control.h

Abstract: 
    This header file defines macros and functions to operate WARP radio frontend.
    We provide this file only for easy WARP radio frontend control.For other type 
    of radio front end, users should write their own, but can reference this file.
--*/

#ifndef _WARP_CONTROL_H
#define _WARP_CONTROL_H

#pragma once 

//extern __REG_CONFIGURATION_ENTRY __gc_RadioRegMaximSPITemplate[3];
//extern __REG_CONFIGURATION_ENTRY __gc_RadioRegDACSPITemplate[3];

//is not threadsafe
/*++
0xFFFFFFFF return value indicates failure.
--*/

//HRESULT WARPRFConfig(PSORA_RADIO pRadio);
//HRESULT WARPRFSelectChannel(PSORA_RADIO pRadio, s_uint32 ChannelNumber);
//
//HRESULT 
//WARPRFWriteDACSPI(
//    PSORA_RADIO pRadio, 
//    s_uint32 address, 
//    s_uint32 value);
//
//HRESULT 
//WARPRFWriteMaximSPI(
//    PSORA_RADIO pRadio, 
//    s_uint32 address, 
//    s_uint32 value);
//
//s_uint32 WARPRFReadDACSPI(PSORA_RADIO pRadio, s_uint32 address);

//VOID WARPRFWriteMaximGain(PSORA_RADIO pRadio, ULONG uGain);
#endif