/*++
Copyright (c) Microsoft Corporation

Module Name: Sora signal cache.

Abstract: This module implements Sora signal cache, using RCB onboard 
          memory to cache pre-computed waveforms, like 802.11 ACKs.
          
--*/

#ifndef _SIGNAL_CACHE
#define _SIGNAL_CACHE

#pragma once
#define __HASH_ENTRY        16

// typedef union PHY_FRAME_KEY
typedef union __CACHE_KEY
{
    LARGE_INTEGER   QuadKey;
    unsigned char   KeyBytes[8];
} CACHE_KEY, *PCACHE_KEY;

typedef struct __KEY_VALUE
{
    CACHE_KEY   __Key;
    PTX_DESC    __Value;
    LIST_ENTRY  __List;
} __KEY_VALUE, *__PKEY_VALUE;


typedef struct __HASH_TBL
{
    LIST_ENTRY     Tbl    [__HASH_ENTRY];
    KSPIN_LOCK     LockTbl[__HASH_ENTRY];
}HASH_TBL, *PHASH_TBL;


typedef struct __SIGNAL_CACHE
{
	HANDLE						__TransferObj;
    PSORA_RADIO                 __pRadio;
    RCB_MEM_POOL                __TxMemPool;
    
    //cache manage
    HASH_TBL                    __HashTbl;
    NPAGED_LOOKASIDE_LIST       __KeyValueList;
} SIGNAL_CACHE, *PSIGNAL_CACHE;

//
// Insert and retrieve signals from a cache
//

PTX_DESC
SORAAPI
SoraGetSignal (
    IN PSIGNAL_CACHE pCache, 
    IN CACHE_KEY     Key);

HRESULT 
SORAAPI
SoraInsertSignal (
    IN PSIGNAL_CACHE     pCache, 
    IN PCOMPLEX8         pSampleBuffer,
    IN PHYSICAL_ADDRESS *pSampleBufferPa,
    IN ULONG             uSampleCount,
    IN CACHE_KEY         Key);

//
// Signal cache initialization and cleanup
//

HRESULT 
SORAAPI
SoraInitSignalCache(
    OUT PSIGNAL_CACHE   pCache,
    IN  HANDLE    		TransferObj,
    IN  PSORA_RADIO     pRadio,
    ULONG               uSize,      // # of samples in one signal
    ULONG               uMaxEntryNum );

VOID 
SORAAPI 
SoraCleanSignalCache (IN PSIGNAL_CACHE pCache);

#endif