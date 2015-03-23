#ifndef __RX_INTERNAL_H
#define __RX_INTERNAL_H

__inline void SORA_EXTEND_PHY_CONTEXT(PSORA_RADIO pRadio, PVOID ext)
{
    pRadio->__pContextExt = ext; 
}

#define SORA_BEGIN_CUSTOMIZE_PHY_RX(pRadiosHead, pOutputBufferVa, pOutputBufferLen) \
    HRESULT SoraPhyRX( \
                IN  LIST_ENTRY *pRadiosHead, \
                OUT PUCHAR      pOutputBufferVa, \
                IN OUT ULONG    *pOutputBufferLen) \
    { \
        {


#define SORA_END_CUSTOMIZE_PHY_RX \
        } \
    }

#define SORA_BEGIN_CUSTOMIZE_PHY_PREPARE_TX(pRadiosHead, pPacket) \
    HRESULT SdrPhyModulate( \
            IN  LIST_ENTRY      *pRadiosHead, \
            IN  PPACKET_BASE    pPacket) \
    { \
        { 


#define SORA_END_CUSTOMIZE_PHY_PREPARE_TX \
        } \
    } 

// Note:
// VStreamMask = 0x1 is reserved for kernel mode driver, UMX should not use it
// ref: SoraInitVStreamMan()
#define SORA_RESET_RADIO_PHY_RX_INFO_BASE(pRadio) \
    do \
    { \
        (pRadio)->__RxStream.__VStreamMask = 0x1;         \
        (pRadio)->__RxStream.__pStartPt = \
            (pRadio)->__rx_queue.__pVirtualStartAddress; \
        (pRadio)->__RxStream.__nRxBufSize = \
            (pRadio)->__rx_queue.__uSize; \
        (pRadio)->__RxStream.__pScanPt = \
            (pRadio)->__RxStream.__pStartPt; \
        (pRadio)->__RxStream.__pEndPt = \
            (pRadio)->__rx_queue.__pVirtualStartAddress + \
            (pRadio)->__rx_queue.__uSize; \
    } while(0);

#define __ZERO_RX_MANAGER(pRxMgr) \
    do \
    { \
        (pRxMgr)->__pVirtualStartAddress            = NULL; \
        (pRxMgr)->__pVirtualEndAddress              = NULL; \
        (pRxMgr)->__uSize                           = 0;    \
        (pRxMgr)->__pPhysicalStartAddress.QuadPart  = 0;    \
        (pRxMgr)->__pPhysicalEndAddress.QuadPart    = 0;    \
        (pRxMgr)->__uCount                          = 0;    \
    } while (0);

#define __IS_RX_MANAGER_INIT(pRxMgr) \
    (NULL != ((__PRX_QUEUE_MANAGER)(pRxMgr))->__pVirtualStartAddress)

HRESULT SoraInitRxBufferManager(
        IN SORA_RX_MAN_HANDLE   rxMgr,
        IN ULONG                rxBufSize,
        IN ULONG                rxBufNum);

VOID SoraCleanupRxBufferManager(IN SORA_RX_MAN_HANDLE rxMgr);

#endif