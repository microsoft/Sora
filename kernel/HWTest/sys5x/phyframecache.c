#include "Ntifs.h"
#include "miniport.h"

PHY_FRAME_CACHE     g_phycache;
PCOMPLEX8           g_PhyFrameCacheSampleBuffer = NULL;
PHYSICAL_ADDRESS    g_PhyFrameCacheSampleBufferPa;
extern volatile ULONG   fDisplaySample;
CACHE_KEY       g_MacAddr[4];

HRESULT __StartFirstRadio(PMP_ADAPTER Adapter);

volatile UCHAR IsAck = '\0';

ULONG Random(ULONG Range)
{
    ULONG ret = 0;
    LARGE_INTEGER temp = KeQueryPerformanceCounter(NULL);
    ULONG Seed = temp.u.LowPart;

    ret = RtlRandom(&Seed);
    ret %= Range;

    return ret;
}
extern volatile ULONG   fPhyFrameBlockSize;
//HRESULT NICUnitTestTxAck(PMP_ADAPTER Adapter)
//{
//    BOOLEAN fRxRestore = FALSE;
//    HRESULT hr = S_OK;
//    PSORA_RADIO     pRadio = &Adapter->RadioManager.__radio_pool[0];
//    ULONG index = Random(4);
//    PTX_DESC pAckDesc;
//
//    pAckDesc = SoraGetPhyFrameInCache(&g_phycache, g_MacAddr[index]);
//    DbgPrint("[ACK]SoraGetPhyFrameInCache for MAC %d return %08x\n", index, pAckDesc);
//
//    if (pAckDesc != NULL)
//    {
//        if (pRadio->__fRxEnabled)
//        {
//            DbgPrint("[ACK] RXing, stop rx before tx !\n");
//            SORA_HW_STOP_RX(pRadio);
//            fRxRestore = TRUE;
//        }
//        
//        DbgPrint("[ACK] phy frame desc PALow: 0x%08X, PAHi: 0x%08X, __RCBDestAddr: 0x%08X, FrameSize: 0x%08X\n", 
//            pAckDesc->__SourceAddressLo, 
//            pAckDesc->__SourceAddressHi, 
//            pAckDesc->__RCBDestAddr, 
//            pAckDesc->FrameSize);
//        hr = SORA_HW_FAST_TX(pRadio, pAckDesc);
//        DbgPrint("[ACK] RXing, stop rx before tx !\n");
//        IsAck = (UCHAR)index + 1;
//        fPhyFrameBlockSize = 1792 / 112;
//        fDisplaySample = 1;
//        if (fRxRestore)
//        {
//            DbgPrint("[ACK] restore RX after fast tx !\n");
//            SORA_HW_ENABLE_RX(pRadio);
//        }
//
//    }
//    else
//    {
//        hr = E_FAIL;
//    }
//    return hr;
//}
//
//void __FillSampleBuffer(PCOMPLEX8 SampleBuffer, ULONG Size, ULONG Seed)
//{
//    ULONG i;
//    for (i = 0; i < Size / 4; i++)
//    {
//        ((PULONG)SampleBuffer)[i] = i;
//    }
//}
//
//void VerbosePrint()
//{
//        ULONG i = 0;
//        for (i = 0; i < 1792 / 4; i++)
//            DbgPrint("[Verbose] %08x: %08x \n", 
//                (PULONG)g_PhyFrameCacheSampleBuffer + i, 
//                ((PULONG)g_PhyFrameCacheSampleBuffer)[i]);
//}
//
//HRESULT NICUnitTestPhyCache(PMP_ADAPTER Adapter)
//{
//    HRESULT hr;
//    PSORA_RADIO     pRadio = &Adapter->RadioManager.__radio_pool[0];
//    
//    PHYSICAL_ADDRESS    PhysicalAddress      = {0, 0}; 
//    PHYSICAL_ADDRESS    PhysicalAddressLow   = {0, 0};
//    PHYSICAL_ADDRESS    PhysicalAddressHigh  = {0x80000000, 0};
//
//    g_MacAddr[0].QuadKey.u.LowPart = 0x00010203;
//    g_MacAddr[0].QuadKey.u.HighPart = 0x04050000;
//    g_MacAddr[1].QuadKey.u.LowPart = 0x01010203;
//    g_MacAddr[1].QuadKey.u.HighPart = 0x04050000;
//    g_MacAddr[2].QuadKey.u.LowPart = 0x02010203;
//    g_MacAddr[2].QuadKey.u.HighPart = 0x04050000;
//    g_MacAddr[3].QuadKey.u.LowPart = 0x03010203;
//    g_MacAddr[3].QuadKey.u.HighPart = 0x04050000;
//
//    if (g_PhyFrameCacheSampleBuffer == NULL)
//        g_PhyFrameCacheSampleBuffer = 
//            (PCOMPLEX8)MmAllocateContiguousMemorySpecifyCache(
//                    4096, 
//                    PhysicalAddressLow,
//                    PhysicalAddressHigh, 
//                    PhysicalAddress,
//                    MmNonCached
//                    );
//    if (g_PhyFrameCacheSampleBuffer == NULL)
//    {
//        return E_NOT_ENOUGH_CONTINUOUS_PHY_MEM;
//    }
//
//    g_PhyFrameCacheSampleBufferPa = 
//        MmGetPhysicalAddress(g_PhyFrameCacheSampleBuffer);
//    ASSERT(g_PhyFrameCacheSampleBufferPa.u.LowPart % 0x00001000 == 0);
//    if (!pRadio->__fCanWork)
//    {
//        __StartFirstRadio(Adapter);
//    }
//
//    __FillSampleBuffer(g_PhyFrameCacheSampleBuffer, 4096, 0);
//    
//    hr = SoraInsertPhyFrameInCache(
//            &g_phycache, 
//            g_PhyFrameCacheSampleBuffer, 
//            &g_PhyFrameCacheSampleBufferPa, 
//            1792,
//            g_MacAddr[0]
//            );
//    DbgPrint("[ACK]SoraInsertPhyFrameInCache for MAC 0 return %08x\n", hr);
//    
//    hr = SoraInsertPhyFrameInCache(
//            &g_phycache, 
//            g_PhyFrameCacheSampleBuffer, 
//            &g_PhyFrameCacheSampleBufferPa, 
//            1792,
//            g_MacAddr[1]);
//   
//    DbgPrint("[ACK]SoraInsertPhyFrameInCache for MAC 1 return %08x\n", hr);
//    
//    hr = SoraInsertPhyFrameInCache(
//            &g_phycache, 
//            g_PhyFrameCacheSampleBuffer, 
//            &g_PhyFrameCacheSampleBufferPa, 
//            1792,
//            g_MacAddr[2]
//            );
//    DbgPrint("[ACK]SoraInsertPhyFrameInCache for MAC 2 return %08x\n", hr);
//    
//    hr = SoraInsertPhyFrameInCache(
//            &g_phycache, 
//            g_PhyFrameCacheSampleBuffer, 
//            &g_PhyFrameCacheSampleBufferPa, 
//            1792,
//            g_MacAddr[3]
//            );
//    DbgPrint("[ACK]SoraInsertPhyFrameInCache for MAC 3 return %08x\n", hr);
//    
//    //SoraCleanPhyFrameCache(&g_phycache);
//    return hr;
//}