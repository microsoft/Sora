#include "sora.h"

// Note:
// VStreamMask = 0x1 is reserved for kernel mode driver, UMX should not use it
// ref: SORA_RESET_RADIO_PHY_RX_INFO_BASE()
VOID SoraInitVStreamMan(__PRX_QUEUE_MANAGER RxMan)
{
    RxMan->VStreamFreeBitmap = 0xFFFFFFFF; //all free, lowest bit for kernel mode driver.
    KeInitializeSpinLock(&RxMan->VStreamFreeBitmapLock);
}

HRESULT SoraAllocateVStream(PSORA_RADIO radio, PULONG Mask)
{
    KIRQL OldIrql;
    HRESULT hr = E_FAIL;
    
    KeAcquireSpinLock(&radio->__rx_queue.VStreamFreeBitmapLock, &OldIrql);
    if (radio->__rx_queue.VStreamFreeBitmap != 0)
    {
        ULONG FreeBitIndex;
        hr = S_OK;
        BitScanForward(&FreeBitIndex, radio->__rx_queue.VStreamFreeBitmap);
        *Mask = 0x1 << FreeBitIndex;
        radio->__rx_queue.VStreamFreeBitmap &= ~(*Mask);
    }
    KeReleaseSpinLock(&radio->__rx_queue.VStreamFreeBitmapLock, OldIrql);
    return hr;
}

VOID SoraFreeVStream(PSORA_RADIO radio, ULONG Mask)
{
    KIRQL OldIrql;
    KeAcquireSpinLock(&radio->__rx_queue.VStreamFreeBitmapLock, &OldIrql);
    radio->__rx_queue.VStreamFreeBitmap |= Mask;
    KeReleaseSpinLock(&radio->__rx_queue.VStreamFreeBitmapLock, OldIrql);
}