/*++
Copyright (c) Microsoft Corporation

Module Name: 
    sora_utility.h

Abstract: 
    sora_utility.h defines common helper routine for MDL 
    operation, CRC caculation, etc.

History: 
    9/Dec/2009: Created by senxiang
--*/
#pragma once

#ifndef USER_MODE
#include <ntddk.h>
#include <wdm.h>

#pragma message("Sora_utility.h, disable C4201, 4214, 4189")
#pragma warning(disable:4201 4214 4189)
#include <ndis.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MDL manipulation */
ULONG CalcMDLChainCRC32(PMDL pMdl);

PMDL CloneMDL(IN PMDL pMdl);
PMDL CloneMDLChain(IN PMDL pMdl);

VOID FreeMDL(IN PMDL pMdl);
VOID FreeMDLChain(IN PMDL pMdlChain);

PUCHAR 
PopHeader(
    IN PMDL pFirstBuffer, 
    IN ULONG HeaderSize,
    IN ULONG ulNBDataOffset);
PUCHAR 
PushHeaderInPlace(
    PMDL pFirstBuffer, 
    ULONG HeaderSize, 
    PUCHAR OlderHeader);
PUCHAR 
PushHeader(
    IN OUT PMDL* ppFirstMdl, 
    IN ULONG HeaderSize);

#ifdef __cplusplus
}
#endif

#endif

