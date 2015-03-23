#include "sora.h"
#include "__reg_file.h"
#include "__tx_res.h"
#include "__radio_man_internal.h"
#include "__transfer_obj.h"

#ifdef DEBUG_CHECKSUM
s_uint16
SORAAPI
_CalculateChecksum16(
	PVOID buffer, 
	ULONG size) {

	s_uint16* pdata;
	s_uint16 checksum;
	ULONG i;
	pdata = (s_uint16*)buffer;
	for(i=0, checksum=0; i<size/sizeof(s_uint16); i++)
		checksum += pdata[i];
	return checksum;
}
#endif

#ifdef DEBUG_CHECKSUM
#define get_checksum(x) 	(x & 0xffff)
#define get_count(x)		(x >> 16)
#endif

#ifdef DEBUG_CHECKSUM
HRESULT
SORAAPI
_CheckTXError(
	PSORA_RADIO pRadio,
	s_uint16 Checksum) {

	extern HRESULT _SoraRadioReadRFReg(PSORA_RADIO pRadio, s_uint32 addr, OUT s_uint32 *pValue);
	s_uint32 addr_23_CountChecksum;
	s_uint32 addr_21_CountChecksum;
	HRESULT hr;
	hr = E_TX_TRANSFER_CHECKSUM_FAIL;

	DbgPrint("TX Checksum: [0x%x], TX Count: [%d](16-bits)\n",  
		get_checksum(READ_REGISTER_ULONG((PULONG)&((pRadio->__ctrl_reg.pRadioRegs->TXCountChecksum)))),
		get_count(READ_REGISTER_ULONG((PULONG)&((pRadio->__ctrl_reg.pRadioRegs->TXCountChecksum)))));

	if (SUCCEEDED(_SoraRadioReadRFReg(pRadio,
		0x0023,
		&addr_23_CountChecksum))) {
		DbgPrint("addr 23 Checksum: [0x%x], addr 23 Count: [%d](16-bits)\n", 
			get_checksum(addr_23_CountChecksum), 
			get_count(addr_23_CountChecksum));
		if (get_checksum(addr_23_CountChecksum) != Checksum)
			hr = E_TX_TRANSFER_ADDR23_CHECKSUM_FAIL;
	}
	else
		DbgPrint("Failed to get addr 23 CountChecksum\n");

	if (SUCCEEDED(_SoraRadioReadRFReg(pRadio,
		0x0021,
		&addr_21_CountChecksum))) {
		DbgPrint("addr 21 Checksum: [0x%x], addr 21 Count: [%d](16-bits)\n", 
			get_checksum(addr_21_CountChecksum), 
			get_count(addr_21_CountChecksum));
		if (get_checksum(addr_21_CountChecksum) != Checksum)
			hr = E_TX_TRANSFER_ADDR21_CHECKSUM_FAIL;
	}
	else
		DbgPrint("Failed to get addr 21 CountChecksum\n");
	
	return hr;
}
#endif

//
// Hardware operations
//

VOID 
_SoraRadioWriteRFReg ( PSORA_RADIO pRadio, 
                       s_uint32 addr, 
                       s_uint32 value);

void 
__SoraHwPrintDbgRegsUnsafe (PTRANSFER_OBJ pTransferObj);

/*++
SORA_HW_ENABLE_RX enable radio to start RX and write samples into RX buffer from previous write point. 
--*/
VOID 
SORAAPI 
SORA_HW_ENABLE_RX(PSORA_RADIO pRadio)
{
    __REG32_RX_CTRL RXControl;
    KIRQL OldIrql;

    RXControl.Value         = 0;
    RXControl.Bits.RXEnable = 1;
    
    KeAcquireSpinLock(&pRadio->__HWOpLock, &OldIrql);
    WRITE_REGISTER_ULONG(
        (PULONG)&pRadio->__ctrl_reg.pRadioRegs->RXControl,
        RXControl.Value);
    KeReleaseSpinLock(&pRadio->__HWOpLock, OldIrql);

    pRadio->__fRxEnabled = TRUE;
}

/*++
SORA_HW_STOP_RX will stop radio signal RX. 
--*/
VOID 
SORAAPI 
SORA_HW_STOP_RX(PSORA_RADIO pRadio)
{
    __REG32_RX_CTRL RXControl;
    KIRQL OldIrql;

    RXControl.Value         = 0;
    RXControl.Bits.RXEnable = 0;
    
    KeAcquireSpinLock(&pRadio->__HWOpLock, &OldIrql);
    WRITE_REGISTER_ULONG(
        (PULONG)&pRadio->__ctrl_reg.pRadioRegs->RXControl,
        RXControl.Value);
    KeReleaseSpinLock(&pRadio->__HWOpLock, OldIrql);

    pRadio->__fRxEnabled = FALSE;
}

#ifdef DEBUG_CHECKSUM
VOID __SoraHwTransferUnsafeNoWait(
    IN PTRANSFER_OBJ pTransferObj,
    IN PTX_DESC     pTxDesc,
    IN s_uint16    Checksum)
#else
VOID __SoraHwTransferUnsafeNoWait(
    IN PTRANSFER_OBJ pTransferObj,
    IN PTX_DESC     pTxDesc)
#endif
{
    HRESULT hr = S_OK;
    __REG32_TRANS_CTRL TransCtrl;
    __PSORA_RADIO_REGS pRegs = pTransferObj->TransferReg;
    
    pTxDesc->__FrameCtrlOwn = 1; //software own the buffer
    // make sure modulation buffer is flushed into memory.
    _mm_mfence(); 
    TransCtrl.Value = 0;
    TransCtrl.Bits.TransferInit  = 1;	

#ifdef DEBUG_CHECKSUM
    WRITE_REGISTER_ULONG(
        (PULONG)&pRegs->TransferChecksum, Checksum);
#endif

    WRITE_REGISTER_ULONG(
        (PULONG)&pRegs->TransferSrcAddrL, pTxDesc->ThisPa.u.LowPart);
    WRITE_REGISTER_ULONG(
        (PULONG)&pRegs->TransferSrcAddrH, pTxDesc->ThisPa.u.HighPart);
    WRITE_REGISTER_ULONG(
        (PULONG)&pRegs->TransferControl, TransCtrl.Value);
}

#ifdef DEBUG_CHECKSUM
VOID __SoraHwTxUnsafeNoWait ( 
    PSORA_RADIO pRadio, 
    PTX_DESC    pTxDesc,
    s_uint16	Checksum)
#else
VOID __SoraHwTxUnsafeNoWait ( 
    PSORA_RADIO pRadio, 
    PTX_DESC    pTxDesc)
#endif
{
    __REG32_TX_CTRL TXControl;

#ifdef DEBUG_CHECKSUM
    WRITE_REGISTER_ULONG(
        (PULONG)&pRadio->__ctrl_reg.pRadioRegs->TXChecksum, 
        Checksum);
#endif

    WRITE_REGISTER_ULONG(
        (PULONG)&pRadio->__ctrl_reg.pRadioRegs->TXAddr, 
        pTxDesc->__RCBDestAddr);
    WRITE_REGISTER_ULONG(
        (PULONG)&pRadio->__ctrl_reg.pRadioRegs->TXSize, 
        pTxDesc->Size);
    
    TXControl.Value = 0;
    TXControl.Bits.TXInit = 1;
    WRITE_REGISTER_ULONG(
        (PULONG)&pRadio->__ctrl_reg.pRadioRegs->TXControl,
        TXControl.Value); 
}

HRESULT __SoraHwWaitTransferDone (PTX_DESC pTxDesc )
{
    HRESULT hr = S_OK;
    ULONG   uTemp  = 10000000;	// for MAX data transfer (24bit, 2^24 = 16M) it takes more time than previous value 100000

    // wait until hw owns the buffer
    while(pTxDesc->__FrameCtrlOwn == 1) 
    //hw will set it if TXDone. Tx Desc is in MmWriteCombined mem, so not cached.
    {
        uTemp--;
        if(uTemp < 1) {
            hr = E_TX_TRANSFER_FAIL;
            break;
        }
        _mm_pause();
    }
    return hr;
}

#ifdef DEBUG_CHECKSUM
HRESULT 
__SORA_HW_TX_TRANSFER_UNSAFE(
    IN HANDLE TransferObj,
    IN PTX_DESC    pTxDesc, 
    IN s_uint16    Checksum)
#else
HRESULT 
__SORA_HW_TX_TRANSFER_UNSAFE(
    IN HANDLE TransferObj,
    IN PTX_DESC    pTxDesc)
#endif
{
    PTRANSFER_OBJ pTransferObj = (PTRANSFER_OBJ)TransferObj;
	
    HRESULT hr = S_OK;
    __REG32_TRANS_CTRL TransCtrl;
    __PSORA_RADIO_REGS pRegs = pTransferObj->TransferReg;

#ifdef DEBUG_CHECKSUM	
	ULONG TransferErrorCount;
	TransferErrorCount = READ_REGISTER_ULONG((PULONG)&((pRegs->TransferErrorCount)));
#endif

#ifdef DEBUG_CHECKSUM	
    __SoraHwTransferUnsafeNoWait(pTransferObj, pTxDesc, Checksum);
#else
	__SoraHwTransferUnsafeNoWait(pTransferObj, pTxDesc);
#endif

    hr = __SoraHwWaitTransferDone(pTxDesc);

    TransCtrl.Value = 0;
    WRITE_REGISTER_ULONG((PULONG)&pRegs->TransferControl, TransCtrl.Value); //clear init bit

#ifdef DEBUG_CHECKSUM
	if (SUCCEEDED(hr)) {
		if (TransferErrorCount != READ_REGISTER_ULONG((PULONG)&((pRegs->TransferErrorCount))))
			hr = E_TX_TRANSFER_CHECKSUM_FAIL;
	}
	else
        __SoraHwPrintDbgRegsUnsafe(pTransferObj);
#else
	if (FAILED(hr))
		__SoraHwPrintDbgRegsUnsafe(pTransferObj);
#endif

    return hr;
    
}

//HRESULT 
//SORA_HW_TX_TRANSFER_PHY_FRAME(
//    IN PSORA_RADIO      pRadio,
//    IN PTX_DESC  pPhyFrameDesc)
//{
//    KIRQL               OldIrql;
//    HRESULT             hr = E_NO_FREE_TX_SLOT;
//    if (!pPhyFrameDesc->pRMD)
//    {
//        PRCB_MD        pTxRCBMem;
//        pTxRCBMem = SoraAllocateRCBMem(pRadio->pTxResMgr, pPhyFrameDesc->FrameSize);
//        if (!pTxRCBMem)
//        {
//            return hr;
//        }
//        pPhyFrameDesc->pRMD    = pTxRCBMem;
//    }
//
//    pPhyFrameDesc->__RCBDestAddr   = pPhyFrameDesc->pRMD->Start;
//    KeAcquireSpinLock(&pRadio->__HWOpLock, &OldIrql);
//    hr = __SORA_HW_TX_TRANSFER_UNSAFE(pRadio, pPhyFrameDesc);
//    KeReleaseSpinLock(&pRadio->__HWOpLock, OldIrql);
//    return hr;
//}

/*++
__SORA_HW_TX_TRANSFER_SIGNAL_EX transfer modulated samples into 
RCB signal cache buffer, which is managed by a pool.

Parameters:
            pRadio:     destination radio where samples are transferred to.
            pTxMemPool: a TX RCB buffer pool 
            pTxDesc:    pointer to Tx Descriptor 

Return: E_NO_FREE_TX_SLOT if no RCB buffer, E_TX_TRANSFER_FAIL if hardware error.

Note:  Tx Desc should refer to a modulated signal. Radio must be started. 
       The signal should be able to fit a memory block in pool.
       Otherwise, it will cause bug check error.

--*/
HRESULT 
__SORA_HW_TX_TRANSFER_SIGNAL_EX(
    IN HANDLE	TransferObj,
    IN PRCB_MEM_POOL    pTxMemPool,
    IN PTX_DESC         pTxDesc)
{
	PTRANSFER_OBJ pTransferObj = (PTRANSFER_OBJ)TransferObj;
    KIRQL               OldIrql;
    HRESULT             hr = E_NOT_RCB_MEMORY;
    if (pTxMemPool->__nSize < pTxDesc->Size)
    {
        KeBugCheck(BUGCODE_ID_DRIVER);
    }
    
    if (!pTxDesc->pRMD)
    {
        PRCB_MD  pRMD;
        pRMD = SoraAllocateRCBMemFromPool(pTxMemPool);
        if (!pRMD)
        {
            return hr;
        }
        
        pTxDesc->pRMD = pRMD;
    }

    pTxDesc->__RCBDestAddr = pTxDesc->pRMD->Start;

#ifdef DEBUG_CHECKSUM	
	pTxDesc->Checksum = _CalculateChecksum16(pTxDesc->pSampleBuffer, pTxDesc->Size);	
#endif

	if (((__PSORA_REGISTERS)pTransferObj->SoraSysReg)->FirmwareVersion.Version32 >= 0x02000000)
	    KeAcquireSpinLock(&pTransferObj->TransferLock, &OldIrql);
	else
		KeAcquireSpinLock(&pTransferObj->SoraHWOpRadio->__HWOpLock, &OldIrql);

#ifdef DEBUG_CHECKSUM
    hr = __SORA_HW_TX_TRANSFER_UNSAFE(pTransferObj, pTxDesc, pTxDesc->Checksum);
#else
	hr = __SORA_HW_TX_TRANSFER_UNSAFE(pTransferObj, pTxDesc);
#endif

	if (((__PSORA_REGISTERS)pTransferObj->SoraSysReg)->FirmwareVersion.Version32 >= 0x02000000)
	    KeReleaseSpinLock(&pTransferObj->TransferLock, OldIrql);
	else
    	KeReleaseSpinLock(&pTransferObj->SoraHWOpRadio->__HWOpLock, OldIrql);
	
    return hr;
}

HRESULT __SoraHwWaitTxDone (PSORA_RADIO pRadio);

/*++
SORA_HW_TX_TRANSFER transfer a signal to RCB memory.

Parameters:
    pRadio:     destination radio where samples are transferred to.
    pPacket:    pointer to source packet. The packet must own TX resource by 
                SoraPacketGetTxResource, modulated and with frame size set by 
                __SoraPacketSetSignalSize.

Return: E_NOT_RCB_MEMORY if no RCB buffer, E_TX_TRANSFER_FAIL if hardware error. 

Note:       The packet must be modulated first. And radio must be started.
            If succeeded, packet status will be PACKET_CAN_TX. 

--*/
HRESULT 
SORA_HW_TX_TRANSFER(
    IN HANDLE TransferObj,
    IN PPACKET_BASE pPacket)
{
	PTRANSFER_OBJ pTransferObj = (PTRANSFER_OBJ)TransferObj;
    KIRQL    OldIrql;
    PRCB_MD  pRMD;
    HRESULT  hr = E_NOT_RCB_MEMORY;

    if (pPacket->pTxDesc->Size & RCB_BUFFER_ALIGN_MASK)
    {
		InterlockedExchange(&pPacket->fStatus, PACKET_TF_FAIL);
        hr = E_INVALID_SIGNAL_SIZE;
        return hr;
    }
	
    pRMD = SoraAllocateRCBMem (pTransferObj->TransferResMgr, 
                               pPacket->pTxDesc->Size);	
    if (pRMD)
    {   
        pPacket->pTxDesc->__RCBDestAddr   = pRMD->Start;
        pPacket->pTxDesc->pRMD            = pRMD;
		
#ifdef DEBUG_CHECKSUM		
		pPacket->pTxDesc->Checksum = _CalculateChecksum16(pPacket->pTxDesc->pSampleBuffer, 
			pPacket->pTxDesc->Size);
#endif

        // Locked
		if (((__PSORA_REGISTERS)pTransferObj->SoraSysReg)->FirmwareVersion.Version32 >= 0x02000000)
			KeAcquireSpinLock(&pTransferObj->TransferLock, &OldIrql);
		else
			KeAcquireSpinLock(&pTransferObj->SoraHWOpRadio->__HWOpLock, &OldIrql);		

#ifdef DEBUG_CHECKSUM
        hr = __SORA_HW_TX_TRANSFER_UNSAFE(pTransferObj, pPacket->pTxDesc, pPacket->pTxDesc->Checksum);
#else
		hr = __SORA_HW_TX_TRANSFER_UNSAFE(pTransferObj, pPacket->pTxDesc);
#endif
		
		if (((__PSORA_REGISTERS)pTransferObj->SoraSysReg)->FirmwareVersion.Version32 >= 0x02000000)
			KeReleaseSpinLock(&pTransferObj->TransferLock, OldIrql);
		else
			KeReleaseSpinLock(&pTransferObj->SoraHWOpRadio->__HWOpLock, OldIrql);

        
        if (SUCCEEDED(hr))
        {	        
            pPacket->pTxDesc->pSampleBuffer      = NULL;
            pPacket->pTxDesc->SampleBufferSize   = 0;
            InterlockedExchange(&pPacket->fStatus, PACKET_CAN_TX);			
        }
        else
        { 
            // Transfer fails; release RCB TX memory
            SoraFreeRCBMem(pTransferObj->TransferResMgr, pPacket->pTxDesc->pRMD);
            pPacket->pTxDesc->__RCBDestAddr   = 0xcdcdcdcd;
            pPacket->pTxDesc->pRMD    = NULL;
            InterlockedExchange(&pPacket->fStatus, PACKET_TF_FAIL);
        }
    }
    
    return hr;
}

HRESULT
SORAAPI
SORA_HW_FAST_TX_TRANSFER(HANDLE TransferObj, PTX_DESC pTxDesc) {

	PTRANSFER_OBJ pTransferObj = (PTRANSFER_OBJ)TransferObj;
	KIRQL irql;
	HRESULT hr;
	
#ifdef DEBUG_CHECKSUM
	pTxDesc->Checksum = _CalculateChecksum16(pTxDesc->pSampleBuffer, 
		pTxDesc->Size);
#endif

    // Locked
	if (((__PSORA_REGISTERS)pTransferObj->SoraSysReg)->FirmwareVersion.Version32 >= 0x02000000)
	    KeAcquireSpinLock(&pTransferObj->TransferLock, &irql);
	else
		KeAcquireSpinLock(&pTransferObj->SoraHWOpRadio->__HWOpLock, &irql);

#ifdef DEBUG_CHECKSUM
    hr = __SORA_HW_TX_TRANSFER_UNSAFE(pTransferObj, pTxDesc, pTxDesc->Checksum);
#else
	hr = __SORA_HW_TX_TRANSFER_UNSAFE(pTransferObj, pTxDesc);
#endif

	if (((__PSORA_REGISTERS)pTransferObj->SoraSysReg)->FirmwareVersion.Version32 >= 0x02000000)
	    KeReleaseSpinLock(&pTransferObj->TransferLock, irql);
	else
    	KeReleaseSpinLock(&pTransferObj->SoraHWOpRadio->__HWOpLock, irql);


	return hr;
}


//
// Spinlock to wait for Tx finish
//
HRESULT 
__SoraHwWaitTxDone(PSORA_RADIO pRadio)
{
    HRESULT    hr = S_OK;
    __REG32_TX_CTRL TXControl;
    int TimeOut = 1000000;
    do
    {
        TimeOut--;
        if (TimeOut < 1)
        {
            hr = E_TX_TIMEOUT;
            break;
        }
        _mm_pause();
        
        TXControl.Value = 
            READ_REGISTER_ULONG((PULONG)&((pRadio->__ctrl_reg.pRadioRegs->TXControl)));
    } while (!TXControl.Bits.TXDone); // wait finish or timeout
    return hr;
}

#ifdef DEBUG_CHECKSUM
HRESULT __SoraHwTxUnsafe(
    PSORA_RADIO pRadio, 
    PTX_DESC    pTxDesc,
    s_uint16	Checksum)
#else
HRESULT __SoraHwTxUnsafe(
    PSORA_RADIO pRadio, 
    PTX_DESC    pTxDesc)
#endif
{
    
    HRESULT    hr = S_OK;
    __REG32_TX_CTRL TXControl;
	__PSORA_RADIO_REGS pRegs = pRadio->__ctrl_reg.pRadioRegs;

#ifdef DEBUG_CHECKSUM
	ULONG TXErrorCount;
	TXErrorCount = READ_REGISTER_ULONG((PULONG)&pRegs->TXErrorCount);
#endif

#ifdef DEBUG_CHECKSUM
    __SoraHwTxUnsafeNoWait(pRadio, pTxDesc, Checksum);
#else
	__SoraHwTxUnsafeNoWait(pRadio, pTxDesc);
#endif
    
    hr = __SoraHwWaitTxDone(pRadio);

    TXControl.Value = 0;
    TXControl.Bits.TXInit   = 0;
    WRITE_REGISTER_ULONG(
        (PULONG)&pRadio->__ctrl_reg.pRadioRegs->TXControl,
        TXControl.Value);

#ifdef DEBUG_CHECKSUM
	if (SUCCEEDED(hr))
		if (TXErrorCount != READ_REGISTER_ULONG((PULONG)&pRegs->TXErrorCount))
			hr = _CheckTXError(pRadio, pTxDesc->Checksum);
#endif
	
    return hr;
}

/*++
SORA_HW_BEGIN_TX send out packet samples through radio.

Parameters:
    pRadio:     radio object
    pPacket:    packet to be TX out.

Return: E_TX_TIMEOUT if fail. if succeeded, packet status will be PACKET_TX_PEND, 
        which means if your protocol(i.e, MAC) need ACK, we don't know if the 
        packet is successfully TX out.

Note:   SoraRadioSpecifyCurTxPacket must be called before SORA_HW_BEGIN_TX.

--*/
HRESULT SORAAPI SORA_HW_BEGIN_TX(PSORA_RADIO pRadio, PPACKET_BASE pPacket)
{
    HRESULT    hr = S_OK;
    KIRQL      OldIrql;

    ASSERT(pPacket->pTxDesc);
    //Samples are downloaded to RCB memory
    ASSERT(pPacket->pTxDesc->pRMD); 

    KeAcquireSpinLock(&pRadio->__HWOpLock, &OldIrql);
#ifdef DEBUG_CHECKSUM	
    hr = __SoraHwTxUnsafe(pRadio, pPacket->pTxDesc, pPacket->pTxDesc->Checksum);
#else
	hr = __SoraHwTxUnsafe(pRadio, pPacket->pTxDesc);
#endif
    KeReleaseSpinLock(&pRadio->__HWOpLock, OldIrql);

    if (SUCCEEDED(hr))
    {
        InterlockedExchange(&pPacket->fStatus, PACKET_TX_PEND);
    }
    
    return hr;
}

/*++
SORA_HW_FAST_TX send out a physical frame through radio.

Parameters:
            pRadio:         radio object
            pTxDesc:  pointer to physical frame descriptor.

Return: E_TX_TIMEOUT if fail. 

Note:  pTxDesc must already be transferred into signal cache.

--*/
HRESULT SORAAPI SORA_HW_FAST_TX(
    PSORA_RADIO pRadio, 
    PTX_DESC pTxDesc)
{
    HRESULT    hr;
    KIRQL      OldIrql;
    ASSERT(pTxDesc);
    //samples downloaded to RCB SDRAM
    ASSERT(pTxDesc->pRMD); 

    KeAcquireSpinLock(&pRadio->__HWOpLock, &OldIrql);
#ifdef DEBUG_CHECKSUM	
    hr = __SoraHwTxUnsafe(pRadio, pTxDesc, pTxDesc->Checksum);
#else
	hr = __SoraHwTxUnsafe(pRadio, pTxDesc);
#endif
    KeReleaseSpinLock(&pRadio->__HWOpLock, OldIrql);
    return hr;
}

#pragma region "New hardware register's default value"

/*++
__SORA_HW_ENROLL_RX_BUFFER_UNSAFE notify hardware the physical address and 
size of RX DMA buffer.
--*/
void __SORA_HW_ENROLL_RX_BUFFER_UNSAFE(PSORA_RADIO pRadio)
{
    ASSERT(SORA_RADIO_WELL_CONFIGED2(pRadio));
    WRITE_REGISTER_ULONG(
        (PULONG)&pRadio->__ctrl_reg.pRadioRegs->RXBufAddrL,
        SORA_GET_RX_DMA_BUFFER_PADDRESSLO(SORA_GET_RX_QUEUE(pRadio)));
    WRITE_REGISTER_ULONG(
        (PULONG)&pRadio->__ctrl_reg.pRadioRegs->RXBufAddrH,
        SORA_GET_RX_DMA_BUFFER_PADDRESSHI(SORA_GET_RX_QUEUE(pRadio)));
    
    WRITE_REGISTER_ULONG(
        (PULONG)&pRadio->__ctrl_reg.pRadioRegs->RXBufSize,
        SORA_GET_RX_DMA_BUFFER_SIZE(SORA_GET_RX_QUEUE(pRadio)));
}

static  __REG_CONFIGURATION_ENTRY __gc_RadioRegDefaultValue[] = 
{
#pragma region "DMA related common radio registers"
    //TransferControl to 0
    { 0x00, 0x00000000, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000}, 
    //TransferSrcAddrL to 0
    { 0x04, 0x00000000, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000}, 
    //TransferSrcAddrH to 0
    { 0x08, 0x00000000, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000}, 
    //RXControl to 0
    { 0x0c, 0x00000000, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
    //RXBufAddrL to 0
    { 0x10, 0x00000000, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
    //RXBufAddrH to 0 
    { 0x14, 0x00000000, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
    //RXBufSize to 0
    { 0x18, 0x00000000, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
    //TransferMask to 0
    { 0x1c, 0x00000000, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
#pragma region
#pragma region "common radio registers"
    //TXControl to 0
    { 0x80, 0x00000000, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
    //TXAddr to 0
    { 0x84, 0x00000000, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
    //TXMask to 0
    { 0x88, 0x00000000, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
    //TXSize to 0
    { 0x8C, 0x00000000, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
#pragma region 
    //
};

#pragma endregion 

HRESULT __SORA_HW_STATUS_FENCE_UNSAFE(__PSORA_REGISTERS pRegFile)
{
    HRESULT hRes = S_OK;
    __REG32_HWSTATUS    HWStatus;
    int timeout;    
    for (timeout = 1024; timeout > 0; timeout--) 
    {
        HWStatus.Value = READ_REGISTER_ULONG(
        (PULONG)(&(pRegFile->HWStatus)));

        if (HWStatus.Value != 0x03)
        {
            DbgPrint("[Error] HW not ready, value=%08x\n", HWStatus.Value);
            hRes = E_HW_DDR2_INIT_FAIL;
        }
        else
        {
            DbgPrint("HW ready, value=%08x\n", HWStatus.Value);
            hRes = S_OK; 
            break;
        }
        
    }
    return hRes;
}

HRESULT __SoraHwResetSysRegsUnsafe(__PSORA_REGISTERS pRegFile)
{   
    //WRITE_REGISTER_ULONG((PULONG)(&pRegFile->HWControl), 0x00);
    //WRITE_REGISTER_ULONG((PULONG)(&pRegFile->PCIeTXBurstSize), 0x020);
    //WRITE_REGISTER_ULONG((PULONG)(&pRegFile->PCIeRxBlockSize), 0x020);
    return __SORA_HW_STATUS_FENCE_UNSAFE(pRegFile);
}
/*++
__SoraHwResetRCBRegsUnsafe set Radio Control board's registers to default value.
--*/
HRESULT __SoraHwResetRCBRegsUnsafe(
        IN SORA_REGS_HANDLE        pRegisterManager)
{
    HRESULT hRes = S_OK;
    do
    {
        hRes = __ConfigRegistersUnsafe(
                (__PHW_REGISTER_FILE)pRegisterManager, 
                __gc_RadioRegDefaultValue, //TBD: move it to const static data section.
                sizeof(__gc_RadioRegDefaultValue) / sizeof(__REG_CONFIGURATION_ENTRY));
        FAILED_BREAK(hRes);
        
    }while(FALSE);
    return hRes;
}

VOID __SoraHwStop(PSORA_RADIO pRadio)
{
    KIRQL OldIrql;
    KeAcquireSpinLock(&pRadio->__HWOpLock, &OldIrql);
    WRITE_REGISTER_ULONG(
            (PULONG)(&((__PSORA_REGISTERS)pRadio->__ctrl_reg.pSoraSysRegs)->HWControl), 0x01);
    KeReleaseSpinLock(&pRadio->__HWOpLock, OldIrql);

}
HRESULT SORAAPI SoraHwHeavyRestart(PSORA_RADIO pRadio)
{
    HRESULT hRes;
    KIRQL OldIrql;
    do
    {
        KeAcquireSpinLock(&pRadio->__HWOpLock, &OldIrql);
        WRITE_REGISTER_ULONG(
            (PULONG)(&((__PSORA_REGISTERS)pRadio->__ctrl_reg.pSoraSysRegs)->HWControl), 0x00);
        hRes = __SoraHwResetSysRegsUnsafe((__PSORA_REGISTERS)pRadio->__ctrl_reg.pSoraSysRegs);
        KeReleaseSpinLock(&pRadio->__HWOpLock, OldIrql);

        FAILED_BREAK(hRes);
        hRes = SoraRadioStart(
            pRadio, 
            pRadio->__uRxGain, 
            pRadio->__uTxGain, 
            NULL); 
            
        FAILED_BREAK(hRes);

    } while(FALSE);
    return hRes;
}

VOID __SORA_HW_READ_RADIO_ID(OUT PSORA_RADIO pRadio)
{
    KIRQL OldIrql;
    KeAcquireSpinLock(&pRadio->__HWOpLock, &OldIrql);
    pRadio->RadioID = 
        READ_REGISTER_ULONG((PULONG)&pRadio->__ctrl_reg.pRadioRegs->RadioID);   
    KeReleaseSpinLock(&pRadio->__HWOpLock, OldIrql);
}

void SORA_HW_READ_RADIO_POWER_STATUS(OUT PSORA_RADIO pRadio)
{
    ULONG    oldFlag = pRadio->__status.__Flags;
    __REG32_RADIO_STATUS RadioStatus;
    KIRQL OldIrql;

    KeAcquireSpinLock(&pRadio->__HWOpLock, &OldIrql);
    RadioStatus.Value = 
        READ_REGISTER_ULONG((PULONG)&pRadio->__ctrl_reg.pRadioRegs->RadioStatus);
    KeReleaseSpinLock(&pRadio->__HWOpLock, OldIrql);

    if ((oldFlag & __RADIO_STATUS_ALIVE) != RadioStatus.Bits.RadioAlive) //Alive status change
    {
        KeSetEvent(&pRadio->PnPEvent, 1, FALSE);
        if (RadioStatus.Bits.RadioAlive)
        {
            __SetRadioAlive(pRadio);
        }
        else
        {
            __SetRadioDead(pRadio);
        }   
    }
}

void SoraHwPrintDbgRegs(HANDLE TransferObj)
{
	PTRANSFER_OBJ pTransferObj = (PTRANSFER_OBJ)TransferObj;
    KIRQL OldIrql;
	if (((__PSORA_REGISTERS)pTransferObj->SoraSysReg)->FirmwareVersion.Version32 >= 0x02000000)
	    KeAcquireSpinLock(&pTransferObj->TransferLock, &OldIrql);
	else
		KeAcquireSpinLock(&pTransferObj->SoraHWOpRadio->__HWOpLock, &OldIrql);

    __SoraHwPrintDbgRegsUnsafe(pTransferObj);

	if (((__PSORA_REGISTERS)pTransferObj->SoraSysReg)->FirmwareVersion.Version32 >= 0x02000000)
	    KeReleaseSpinLock(&pTransferObj->TransferLock, OldIrql);
	else
    	KeReleaseSpinLock(&pTransferObj->SoraHWOpRadio->__HWOpLock, OldIrql);
}

void __SoraHwPrintDbgRegsUnsafe(PTRANSFER_OBJ pTransferObj)
{
    PUCHAR Regs = (PUCHAR)pTransferObj->SoraSysReg;
    ULONG Offset;

    for (Offset = 0x80; Offset  < 0xFF; Offset += 4)
    {
        ULONG value;
        value = READ_REGISTER_ULONG((PULONG)(Regs + Offset));
        DbgPrint("[Error] %02x: %08x\n", Offset, value);
    }
    for (Offset = 0xF00; Offset < 0xF28; Offset += 4)
    {
        ULONG value;
        value = READ_REGISTER_ULONG((PULONG)(Regs + Offset));
        DbgPrint("[Error] %02x: %08x\n", Offset, value);
    }
}

void __SoraHwPrintRadioRegsUnsafe(PSORA_RADIO pRadio)
{
    PUCHAR Regs = (PUCHAR)pRadio->__ctrl_reg.pRadioRegs;
    ULONG Offset;
    for (Offset = 0x00; Offset  < 0x20; Offset += 4)
    {
        ULONG value;
        value = READ_REGISTER_ULONG((PULONG)(Regs + Offset));
        DbgPrint("[Error] %02x: %08x\n", Offset, value);
    }

    for (Offset = 0x70; Offset  < 0x100; Offset += 4)
    {
        ULONG value;
        value = READ_REGISTER_ULONG((PULONG)(Regs + Offset));
        DbgPrint("[Error] %02x: %08x\n", Offset, value);
    }
    
}

void SoraHwPrintRadioRegs(PSORA_RADIO pRadio)
{
    KIRQL OldIrql;
    KeAcquireSpinLock(&pRadio->__HWOpLock, &OldIrql);
    __SoraHwPrintRadioRegsUnsafe(pRadio);
    KeReleaseSpinLock(&pRadio->__HWOpLock, OldIrql);
}

#define __BASE_CENTRAL_FREQ     (2412 * 1000) // kHz
VOID SoraHwSetCentralFreq( 
    PSORA_RADIO pRadio, 
    ULONG CoarseKHZ, 
    LONG  FineHz)
{
    //UNREFERENCED_PARAMETER(HzFinerGrain);
    //
    //if ((kHzCoare - __BASE_CENTRAL_FREQ) % 5 == 0)
    //{
    //    ULONG ChannelNum = ((kHzCoare - __BASE_CENTRAL_FREQ) / 5000 + 1);
    //    //DbgPrint("[TEMP1] ChannelNum =%d\n", ChannelNum);
    //    if (ChannelNum >= 1 && ChannelNum <=14)
    //        WARPRFSelectChannel(pRadio, ChannelNum);
    //}
    
    _SoraRadioWriteRFReg(pRadio, 0x7, CoarseKHZ);
    _SoraRadioWriteRFReg(pRadio, 0x8, FineHz);
}

VOID SoraHwSetFreqCompensation(PSORA_RADIO pRadio, LONG valueHz)
{
    _SoraRadioWriteRFReg(pRadio, 0x9, valueHz);
}

VOID SoraHwSetSampleClock(PSORA_RADIO pRadio, ULONG valueHz)
{
    _SoraRadioWriteRFReg(pRadio, 0x6, valueHz);
}

void SoraHwSetTxBufferRegs(PSORA_RADIO pRadio, PPACKET_BASE pPacket) {

	WRITE_REGISTER_ULONG(
		(PULONG)&pRadio->__ctrl_reg.pRadioRegs->TXAddr, 
		pPacket->pTxDesc->__RCBDestAddr);
	WRITE_REGISTER_ULONG(
		(PULONG)&pRadio->__ctrl_reg.pRadioRegs->TXSize, 
		pPacket->pTxDesc->Size);
}

HRESULT SoraHwSyncTx(PSORA_RADIO pRadio, ULONG mask) {

    __REG32_TX_CTRL TXControl;
	HRESULT hr;

    TXControl.Value = 0;
    TXControl.Bits.TXInit = 1;
	TXControl.Bits.TXMask = mask;
    WRITE_REGISTER_ULONG(
        (PULONG)&pRadio->__ctrl_reg.pRadioRegs->TXControl,
        TXControl.Value); 

	hr = __SoraHwWaitTxDone(pRadio);

	TXControl.Value = 0;
	TXControl.Bits.TXInit = 0;
	WRITE_REGISTER_ULONG(
		(PULONG)&pRadio->__ctrl_reg.pRadioRegs->TXControl,
		TXControl.Value);

	return hr;
}

