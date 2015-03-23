#ifndef __TX_RES_H
#define __TX_RES_H

#define TX_RCB_MEM_SHIFT            (28)
#define TX_RCB_MEM_SIZE             (1 << TX_RCB_MEM_SHIFT) //32MB
CCASSERT(TX_RCB_MEM_SIZE == 256 * 1024 * 1024)
#define TX_RCB_MEM_UNIT_SHIFT       (19)
#define TX_RCB_MEM_UNIT_SIZE        (1 << TX_RCB_MEM_UNIT_SHIFT) //512KB
CCASSERT(TX_RCB_MEM_UNIT_SIZE == 512 * 1024)

#define MAX_RCB_MD_NUM              (TX_RCB_MEM_SIZE / TX_RCB_MEM_UNIT_SIZE)  //512
#define MAX_RCB_MEM_UNIT_NUM        MAX_RCB_MD_NUM 

#define HW_RESERVED_UNIT_NUM        1

#define MAX_TRANSFER_SIZE			(((1<<24)-1)&~RCB_BUFFER_ALIGN_MASK)

typedef struct __TX_DESC
{
	union {
		struct {
		    //referenced by hardware and application driver
		    s_uint16                        __CRC;
		    s_uint16                        __TXStatus;
		    s_uint32                        __TimeStamp;

		    s_uint32                        __SourceAddressLo;
		    s_uint32                        __SourceAddressHi;

		    s_uint32                        __RCBDestAddr;

			// Never Use __NextDesc again!!! Pointer size is different from 64bit to 32bit system			
			s_uint32						__NextDesc_NOUSE; // communicate with HW, force to be 4B

#pragma warning (disable: 4201)
		    struct 
		    {
		        s_uint32                    Size            : 24;
		        volatile s_uint32           __FrameCtrlOwn  : 1;
		        s_uint32                    __StatusValid   : 1;
		        s_uint32                    __MoreDesc      : 1;
		        s_uint32                    __Reserved      : 5;
		    } ;
#pragma warning (default: 4201)
		    s_uint32                        __32BPadding;
		    s_uint32                        __Delimiter;
		    
		    //referenced by kernel-mode driver
			union {
				struct __TX_DESC				*__NextDesc;
				unsigned __int64 				 __NextDesc_x64_wrapper;
		    };

			union {
			    PRCB_MD                         pRMD; // point to the RMD 
			    unsigned __int64 				pRMD_x64_wrapper;
			};

			union {
		    	PVOID                           pSampleBuffer;
				unsigned __int64 				pSampleBuffer_x64_wrapper;
			};
		    ULONG                           SampleBufferSize;
		    PHYSICAL_ADDRESS                ThisPa;

#ifdef DEBUG_CHECKSUM
			s_uint16						Checksum;
#endif
		};
		unsigned char align[128];
	};
} TX_DESC, *PTX_DESC;

//TX_DESC can't cross 4K boundary
CCASSERT(0x00001000 % sizeof(TX_DESC) == 0) 

/*
 *   Kun: The descriptor should be descriptive
 */
typedef struct __RCB_MEMORY_DESC
{
    LIST_ENTRY      __List;
    ULONG           Start; // Offset in TX RCB buffer. need by __RCBDestAddr of TX_DESC
    ULONG           Units; // Size of the memory in units - 0xFFFFFFFF means invalid
    ULONG           Bytes; // Size of the memory in bytes - 0xFFFFFFFF means invalid
} RCB_MD, *PRCB_MD;

typedef struct __TX_RESOURCE_MANAGER
{
    RCB_MD                  __DescPool     [MAX_RCB_MD_NUM];
    PRCB_MD                 __SortedMemDesc[MAX_RCB_MD_NUM];
    KSPIN_LOCK              __Lock;
    LIST_ENTRY              __FreeRMDList; // point to list of free RMD

    //
    // A fast-hash link list arry - i-th entry links all 512KB * (i+1) UNIT
    // Each entry is a RMD describing a free RCB memory block
    //
    LIST_ENTRY              __FreeMem      [MAX_RCB_MD_NUM]; 

    PTX_DESC                __pTxDescList; 
    ULONG                   __uTxDescSize; // The size for the DESC array

    PTX_DESC                __pFreeTxDesc;
}TX_RM, *PTX_RM;

//
// TX Descriptor
//

PTX_DESC 
SoraAllocateTxDesc ( 
    IN PTX_RM pTxResMgr );

void 
SoraFreeTxDesc (
    IN PTX_RM   pTxResMgr, 
    IN PTX_DESC pTxDesc );

//
// RCB Memory allocation/deallocation
//

PRCB_MD 
SoraAllocateRCBMem (
            IN PTX_RM       rm, 
            IN ULONG        size);

void 
SoraFreeRCBMem (
        IN PTX_RM  pTxResMgr, 
        IN PRCB_MD pRMD);


//
// RCB Memory Pool
//

HRESULT 
SoraInitRCBMemPool(
        OUT PRCB_MEM_POOL   pRCBMemPool,
        IN  PTX_RM          rm,
        IN  ULONG           uSize,
        IN  ULONG           uCount);

void 
SoraCleanRCBMemPool(
        IN OUT PRCB_MEM_POOL    pRCBMemPool,
        IN PTX_RM               pTxResMgr );

PRCB_MD 
SoraAllocateRCBMemFromPool(
        IN PRCB_MEM_POOL pTxMemPool);

void 
SoraFreeRCBMemToPool(
        IN PRCB_MEM_POOL pTxMemPool,
        IN PRCB_MD       pRMD
        );

//
// Tx Resource Manger initialization and cleanup
//

HRESULT 
SoraInitTxResManager (
    OUT PTX_RM pTxResMgr
);

void 
SoraCleanTxResManager(
    IN PTX_RM pTxResMgr);


#endif
