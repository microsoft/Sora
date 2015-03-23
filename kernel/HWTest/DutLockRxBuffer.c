#include "hwtest_miniport.h"

//HRESULT HwtInitTxBufferMDL(IN  PHWT_ADAPTER  Adapter, PSORA_RADIO pRadio)
//{
//    HRESULT hr = E_FAIL;
//    
//    do
//    {
//        if (!SORA_RADIO_WELL_CONFIGED2(pRadio))
//            break;
//        Adapter->UExtKeObj.TxBufferMDL[0] = IoAllocateMdl(
//                                SoraRadioGetModulateBuffer(pRadio),
//                                SoraRadioGetModulateBufferSize(pRadio),
//                                FALSE, 
//                                TRUE, 
//                                NULL);
//            
//        if (Adapter->UExtKeObj.TxBufferMDL[0] == NULL) 
//        {
//            break;
//        }
//        else
//        {
//            hr = S_OK;
//        }
//        MmBuildMdlForNonPagedPool(Adapter->UExtKeObj.RxBufferMDL[0]);
//    }while(0);
//    return hr;
//}
//
//void HwtFreeTxBufferMDL(IN  PHWT_ADAPTER  Adapter)
//{
//    if (Adapter->UExtKeObj.TxBufferUserSpaceVA[0] != NULL)
//    {
//        MmUnmapLockedPages(Adapter->UExtKeObj.TxBufferUserSpaceVA[0], Adapter->UExtKeObj.TxBufferMDL[0]);
//        Adapter->UExtKeObj.TxBufferUserSpaceVA[0] = NULL;
//    }
//    if (Adapter->UExtKeObj.TxBufferMDL[0] != NULL)  
//    {
//        IoFreeMdl(Adapter->UExtKeObj.TxBufferMDL[0]);
//        Adapter->UExtKeObj.TxBufferMDL[0] = NULL;
//    }
//}
//
//HRESULT HwtInitRxBufferMDL(IN  PHWT_ADAPTER  Adapter, PSORA_RADIO pRadio)
//{
//    HRESULT hr = E_FAIL;
//    do
//    {
//        if (!SORA_RADIO_WELL_CONFIGED2(pRadio))
//            break;
//        Adapter->UExtKeObj.RxBufferMDL[0] = IoAllocateMdl(
//                                SORA_GET_RX_DMA_BUFFER_VADDRESS(&pRadio->__rx_queue_manager), 
//                                SORA_GET_RX_DMA_BUFFER_SIZE(&pRadio->__rx_queue_manager),
//                                FALSE, 
//                                TRUE, 
//                                NULL);
//            
//        if (Adapter->UExtKeObj.RxBufferMDL[0] == NULL) 
//        {
//            break;
//        }
//        else
//        {
//            hr = S_OK;
//        }
//        MmBuildMdlForNonPagedPool(Adapter->UExtKeObj.RxBufferMDL[0]);
//    }while(0);
//    return hr;
//}
//
//void HwtFreeRxBufferMDL(IN  PHWT_ADAPTER  Adapter)
//{
//    if (Adapter->UExtKeObj.RxBufferUserSpaceVA[0] != NULL)
//    {
//        MmUnmapLockedPages(Adapter->UExtKeObj.RxBufferUserSpaceVA, Adapter->UExtKeObj.RxBufferMDL[0]);
//        Adapter->UExtKeObj.RxBufferUserSpaceVA[0] = NULL;
//    }
//    if (Adapter->UExtKeObj.RxBufferMDL[0] != NULL)  
//    {
//        IoFreeMdl(Adapter->UExtKeObj.RxBufferMDL[0]);
//        Adapter->UExtKeObj.RxBufferMDL[0] = NULL;
//    }
//    
//}
//
//HRESULT HwtInitRxBufferMDL(IN  PHWT_ADAPTER  Adapter, PSORA_RADIO pRadio);

//HRESULT 
//DutLockRxBuffer(
//    PHWT_ADAPTER Adapter, 
//    PSORA_RADIO pRadio, 
//    PVOID *ppUserSpaceBuffer, 
//    PULONG BufferSize)
//{
//    HRESULT hr = E_FAIL;
//    PVOID RxBuffer;
//    *ppUserSpaceBuffer = NULL;
//
//    do
//    {
//        hr = HwtInitRxBufferMDL(Adapter, pRadio);
//        FAILED_BREAK(hr);
//
//        *ppUserSpaceBuffer = 
//            MmMapLockedPagesSpecifyCache(
//                Adapter->UExtKeObj.RxBufferMDL[0], 
//                UserMode, 
//                MmCached, 
//                NULL, 
//                FALSE, 
//                NormalPagePriority);
//        
//        if (*ppUserSpaceBuffer)
//        {
//            *BufferSize = SORA_GET_RX_DMA_BUFFER_SIZE(&pRadio->__rx_queue_manager);
//            hr = S_OK;
//        }
//    } while(FALSE);
//
//    return hr;
//}