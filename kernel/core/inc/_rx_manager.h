/*++
Copyright (c) Microsoft Corporation

Module Name: Sora RX DMA buffer manager

Abstract: This header file defines structs, macros, interfaces to manager the RX DMA buffer.

--*/

#ifndef _RX_MANAGER
#define _RX_MANAGER
#pragma

#define __SCANNED_BIT               (1)
#define __VALID_BIT                 (0)

#define SCANNED_FLAG                0x02
#define VALID_FLAG                  0x01

#define SORA_C_RXBUF_SLOT_IS_OLDEST(slot) \
    (((__PRX_DESC)slot)->VStreamBits == 0x0)

#define SORA_C_RXBUF_IS_VALID_EX(slot, VStreamMask) \
    ((((__PRX_DESC)slot)->VStreamBits & (VStreamMask)) != 0x00)
//test if the RXBuf slot contains valid data

#define SORA_C_SIGNAL_BLOCK_IS_SCANNED(slot, VStreamMask) \
    ((((__PRX_DESC)slot)->VStreamBits  & (VStreamMask)) == 0x00)

//#define SORA_C_INVALIDATE_SIGNAL_BLOCK(desc) \
//    do { ((__PRX_DESC)desc)->u.__Flags |= (1 << __SCANNED_BIT); } while(0);

FORCEINLINE
LONG
InterlockedAnd_Inline (
    __inout LONG volatile *Target,
    __in    LONG Set
    )
{
    LONG i;
    LONG j;

    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange(Target,
                                       i & Set,
                                       i);

    } while (i != j);

    return j;
}

#ifdef USER_MODE
#ifdef InterlockedAnd
#undef InterlockedAnd
#endif
#define InterlockedAnd InterlockedAnd_Inline
#endif

#define SORA_C_INVALIDATE_SIGNAL_BLOCK_EX(desc, VStreamMask) \
    do { InterlockedAnd(&((__PRX_DESC)desc)->VStreamBits, (~(VStreamMask))); } while(0);

#define SORA_GET_RX_DMA_BUFFER_PADDRESS(pRXMgr) \
    (ULONG)(((__PRX_QUEUE_MANAGER)(pRXMgr))->__pPhysicalStartAddress.LowPart)

#define SORA_GET_RX_DMA_BUFFER_PADDRESSLO(pRXMgr) \
    (ULONG)(((__PRX_QUEUE_MANAGER)(pRXMgr))->__pPhysicalStartAddress.LowPart)
#define SORA_GET_RX_DMA_BUFFER_PADDRESSHI(pRXMgr) \
    (ULONG)(((__PRX_QUEUE_MANAGER)(pRXMgr))->__pPhysicalStartAddress.HighPart)

#define SORA_GET_RX_DMA_BUFFER_VADDRESS(pRXMgr) \
    (((__PRX_QUEUE_MANAGER)(pRXMgr))->__pVirtualStartAddress)

#define SORA_GET_RX_DMA_BUFFER_SIZE(pRXMgr) \
    (((__PRX_QUEUE_MANAGER)(pRXMgr))->__uSize)

#define SORA_RX_SIGNAL_UNIT_NUM_PER_DESC    7
#define SORA_RX_SIGNAL_UNIT_SIZE            16 //sizeof(__RX_DESC)
#define SORA_RX_SIGNAL_UNIT_COMPLEX16_NUM   (SORA_RX_SIGNAL_UNIT_SIZE / sizeof(COMPLEX16))
//#define SORA_RX_SIGNAL_UNIT_COMPLEX8_NUM    (SORA_RX_SIGNAL_UNIT_SIZE / sizeof(COMPLEX16))
#define SORA_GET_RX_SIGNAL_UNIT(p, i)       ((PUCHAR)(p) + (i) * SORA_RX_SIGNAL_UNIT_SIZE)

#define SORA_RX_DMA_BLOCK_PER_SLOT 7
#define SORA_RX_DMA_BLOCK_SIZE 16 
#define SORA_RX_SAMPLE_PER_DMA_BLOCK  (SORA_RX_DMA_BLOCK_SIZE / sizeof( RXSAMPLE ))


#define SORA_GET_RX_DESC_SIZE(pDesc)     128 //in bytes
// #define SORA_GET_RX_SIGNALS_SIZE(pDesc)     128 //in bytes

#pragma pack(push, 1)

// Here, I will make some major changes to the data structure.
typedef struct ___RX_DESC
{
#pragma warning(disable: 4200 4214 4201)
    struct 
    {
        volatile LONG   VStreamBits;
        UCHAR   __Reserved3[8];
		unsigned __int32			TimeStamp;
    };
    
#pragma warning(default: 4200 4214 4201)
} RX_DESC, *PRX_DESC, __RX_DESC, *__PRX_DESC, **__PPRX_DESC;

typedef union __SORA_RX_DMA_BLOCK {
    RX_DESC   Desc;
    RXSAMPLE  Samples [SORA_RX_SAMPLE_PER_DMA_BLOCK];
} SORA_RX_DMA_BLOCK, * PSORA_RX_DMA_BLOCK;

typedef struct __SORA_SAMPLE_BLOCK
{
    SORA_RX_DMA_BLOCK SampleUnit [SORA_RX_DMA_BLOCK_PER_SLOT];
} SORA_SAMPLE_BLOCK, *PSORA_SAMPLE_BLOCK;

// typedef union __RX_BLOCK
typedef union __RX_SLOT
{
    SORA_RX_DMA_BLOCK blocks[SORA_RX_DMA_BLOCK_PER_SLOT + 1];
    
#pragma warning(disable: 4201)
    struct __DESC_SAMPLE
    {
        RX_DESC              Desc;
        SORA_SAMPLE_BLOCK    SampleBlock;
    } u;
#pragma warning(default: 4201)

}RX_SLOT, *PRX_SLOT;
#pragma pack(pop)

typedef RX_SLOT RX_BLOCK, * PRX_BLOCK;

#define SORA_RX_BLOCK_SIZE 128
#define SORA_RX_SLOT_SIZE 128
CCASSERT(sizeof(RX_BLOCK) == SORA_RX_SLOT_SIZE)


typedef struct ___RX_QUEUE
{
    __PRX_DESC __pRxDesc;
    ULONG      __PhysicalAddress;
    ULONG      __uSize;
} __RX_QUEUE, *__PRX_QUEUE, **__PPRX_QUEUE;

typedef struct ___RX_QUEUE_MANAGER
{
    PUCHAR                  __pVirtualStartAddress;
    PUCHAR                  __pVirtualEndAddress;

    PHYSICAL_ADDRESS        __pPhysicalStartAddress;
    PHYSICAL_ADDRESS        __pPhysicalEndAddress;
    ULONG                   __uSize;

    __RX_QUEUE              __RxQueues[RX_SLOT_MAX_COUNT]; // for future use
    ULONG                   __uCount;
    ULONG                   __uIndex;   // Index of queue if multi-queue is enabled

    ULONG                   VStreamFreeBitmap;
    KSPIN_LOCK              VStreamFreeBitmapLock;
}__RX_QUEUE_MANAGER, *__PRX_QUEUE_MANAGER, **__PPRX_QUEUE_MANAGER;

/*
typedef struct ___RX_QUEUE
{
    PSORA_RX_DMA_BLOCK      __pDMABuf;

    PUCHAR                  __pVirtualStartAddress;
    PUCHAR                  __pVirtualEndAddress;

    PHYSICAL_ADDRESS        __pPhysicalStartAddress;
    PHYSICAL_ADDRESS        __pPhysicalEndAddress;
    ULONG                   __uSize;

    ULONG                   VStreamFreeBitmap;
    KSPIN_LOCK              VStreamFreeBitmapLock;
} __RX_QUEUE, *__PRX_QUEUE, **__PPRX_QUEUE;    
*/
typedef PVOID SORA_RX_MAN_HANDLE;    
typedef struct __SORA_RADIO *PSORA_RADIO;

// APIs to allocate a virtual RX stream
HRESULT SoraAllocateVStream(PSORA_RADIO radio, PULONG Mask);

VOID    SoraFreeVStream(PSORA_RADIO radio, ULONG Mask);

#endif
