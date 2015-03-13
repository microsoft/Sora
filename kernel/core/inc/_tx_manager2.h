/*++
Copyright (c) Microsoft Corporation

Module Name: _tx_manager2.h

Abstract: This header file provides Sora RCB memory pool for PHY frame cache. 

--*/

#ifndef _TX_MANAGER2_
#define _TX_MANAGER2_

typedef struct __TX_RESOURCE_MANAGER *PTX_RM;
typedef struct __TX_DESC *PTX_DESC;
typedef struct __RCB_MEMORY_DESC *PRCB_MD;

typedef struct __RCB_MEM_POOL
{
    NPAGED_LOOKASIDE_LIST       __SmallRCBMDL;
    PRCB_MD                     __pTotalRCBMD;
    ULONG                       __nSize;
    LIST_ENTRY                  __FreeRCBMDL;
    KSPIN_LOCK                  __PoolLock;
}RCB_MEM_POOL, *PRCB_MEM_POOL;

#define RCB_BUFFER_ALIGN_MASK       0x0000007F //128 -byte align
#define RCB_BUFFER_ALIGN            0x00000080

#define ALIGN_WITH_RCB_BUFFER_PADDING_ZERO(SampleBuffer, SampleBufSize) \
{\
        if (((SampleBufSize) & RCB_BUFFER_ALIGN_MASK) != 0)\
        {\
        	ULONG OldSampleBufSize = SampleBufSize;\
            (SampleBufSize)= ((SampleBufSize) & (~RCB_BUFFER_ALIGN_MASK)) + RCB_BUFFER_ALIGN;\
        	if (SampleBuffer) \
				RtlZeroMemory((unsigned char*)SampleBuffer+OldSampleBufSize, SampleBufSize-OldSampleBufSize); \
        }\
}

#endif //_TX_MANAGER2_
