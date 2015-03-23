#include "hwtest_miniport.h"
#include "dut_entries.h"
#include "hwtest_ioctrl.h"
#include "_private_ext_k.h"

#define TAG 'bcr '

#pragma warning(disable:4995)

#define NTSTRSAFE_LIB
#include <ntstrsafe.h>

#define E_CMD_SYNTAX                        (0x80050200L)
#define E_NO_HARDWARE                       (0x80050201L)
#define E_OUT_BOUND                         (0x80050202L)

#pragma NDIS_PAGEABLE_FUNCTION(NICRegisterDevice)
#pragma NDIS_PAGEABLE_FUNCTION(NICDeregisterDevice)
#pragma NDIS_PAGEABLE_FUNCTION(NICDispatch)


PDEVICE_OBJECT     ControlDeviceObject  = NULL;  // Device for IOCTLs
NDIS_HANDLE        NdisDeviceHandle     = NULL; // From NdisMRegisterDevice
extern NDIS_HANDLE NdisWrapperHandle;
extern SIGNAL_CACHE     g_phycache;

extern volatile ULONG fDumpMode;

extern void free_get_send_packet(PVOID context, struct RESOURCE_OBJECT_DATA* rdata);

extern PHWT_ADAPTER GetMPAdapter(PDEVICE_OBJECT device);

#define Lock(lock_long) {							\
													\
	while(1) {										\
		if (*(volatile LONG*)lock_long)	{			\
			_mm_pause();							\
			continue;								\
		}											\
		if (InterlockedCompareExchange(lock_long,	\
			1,										\
			0))										\
			continue;								\
		break;										\
	}												\
}

#define Unlock(lock_long) {							\
													\
	InterlockedCompareExchange(lock_long, 			\
		0,											\
		1);											\
}


HRESULT NICTestFirstRadioTxModulate(PHWT_ADAPTER Adapter);

void NicCopyInfo(PHWT_DETAIL_INFO out, PHWT_ADAPTER Adapter)
{
    out->ullBlocksInTime = Adapter->Infos.ullBlocksInTime;
    out->ullBlocksLag = Adapter->Infos.ullBlocksLag;

    out->Count = SerialTXCacheInfo(out->CachedSignals, HWT_MAX_CACHED_SIGS, Adapter);
    DEBUGP(MP_INFO, ("[HWT] NicCopyInfo: cached %d entries\n", out->Count));
}

typedef BOOL phyaddr_segment_callback(PVOID vaddr, PHYSICAL_ADDRESS phyaddr, ULONG offset, ULONG size, void* context);

struct phyaddr_transfer_to_rcb_info {

	PTRANSFER_OBJ transfer_obj;
	PTXCB txcb;
	unsigned char* wrapper_page_vaddr;
	PHYSICAL_ADDRESS wrapper_page_paddr;
	volatile LONG* wrapper_page_lock;	
};

__forceinline
BOOL do_transfer(unsigned char* vaddr, PHYSICAL_ADDRESS paddr, ULONG size, void* context) {

	struct phyaddr_transfer_to_rcb_info* info;
#ifdef DEBUG_CHECKSUM
	s_uint16 checksum; 
#endif	
	info = (struct phyaddr_transfer_to_rcb_info*)context;
#ifdef DEBUG_CHECKSUM
	checksum = info->txcb->Base.pTxDesc->Checksum;
#endif
	
	info->txcb->Base.pTxDesc->__SourceAddressHi = paddr.HighPart;
	info->txcb->Base.pTxDesc->__SourceAddressLo = paddr.LowPart;
	info->txcb->Base.pTxDesc->pSampleBuffer = vaddr;
	SoraPacketSetSignalLength(&info->txcb->Base, 
		size);

	if (SORA_HW_FAST_TX_TRANSFER(info->transfer_obj,
		info->txcb->Base.pTxDesc) != S_OK)
		return FALSE;
	
#ifdef DEBUG_CHECKSUM
		info->txcb->Base.pTxDesc->Checksum += checksum;
#endif
	return TRUE;
}

BOOL phyaddr_transfer_to_rcb(unsigned char* vaddr, PHYSICAL_ADDRESS paddr, ULONG offset, ULONG size, void* context) {

	struct phyaddr_transfer_to_rcb_info* info;
	ULONG align_size;
	info = (struct phyaddr_transfer_to_rcb_info*)context;

	if (offset) {
		ULONG other_size;	
		ULONG align_other_size;
		BOOL result;

		other_size = PAGE_SIZE - offset > size ? size : PAGE_SIZE - offset;
		
		Lock(info->wrapper_page_lock);
		{
			RtlCopyMemory(info->wrapper_page_vaddr,
				vaddr, 
				other_size);
			align_other_size = other_size;
			ALIGN_WITH_RCB_BUFFER_PADDING_ZERO(info->wrapper_page_vaddr,
				align_other_size);
			result = do_transfer(info->wrapper_page_vaddr, 
				info->wrapper_page_paddr,
				align_other_size,
				context);
		}		
		Unlock(info->wrapper_page_lock);

		if (!result)
			return FALSE;
			
		info->txcb->Base.pTxDesc->__RCBDestAddr += other_size;
		vaddr += other_size;
		paddr.QuadPart += other_size;
		size -= other_size;
	}
	while(size >= MAX_TRANSFER_SIZE) {
		if (!do_transfer(vaddr, paddr, MAX_TRANSFER_SIZE, context))
			return FALSE;
		
		info->txcb->Base.pTxDesc->__RCBDestAddr += MAX_TRANSFER_SIZE;
		vaddr += MAX_TRANSFER_SIZE;
		paddr.QuadPart += MAX_TRANSFER_SIZE;
		size -= MAX_TRANSFER_SIZE;
	}
	align_size = size;
	ALIGN_WITH_RCB_BUFFER_PADDING_ZERO(NULL, 
		align_size);
	if (align_size > size)
		align_size -= RCB_BUFFER_ALIGN;
	if (align_size) {
		if (!do_transfer(vaddr, paddr, align_size, context))
			return FALSE;

		info->txcb->Base.pTxDesc->__RCBDestAddr += align_size;
		vaddr += align_size;
		paddr.QuadPart += align_size;
		size -= align_size;
	}
	if (size) {
		ULONG other_size;
		ULONG align_other_size;
		BOOL result;

		other_size = size;
		
		Lock(info->wrapper_page_lock);
		{
			RtlCopyMemory(info->wrapper_page_vaddr,
				vaddr, 
				other_size);
			align_other_size = other_size;
			ALIGN_WITH_RCB_BUFFER_PADDING_ZERO(info->wrapper_page_vaddr,
				align_other_size);
			result = do_transfer(info->wrapper_page_vaddr, 
				info->wrapper_page_paddr,
				align_other_size,
				context);
		}
		Unlock(info->wrapper_page_lock);

		if (!result)
			return FALSE;
			
		info->txcb->Base.pTxDesc->__RCBDestAddr += other_size;
		vaddr += other_size;
		paddr.QuadPart += other_size;
		size -= other_size;
	}
	return TRUE;
}

BOOL vaddr_to_phyaddr(unsigned char* vaddr, ULONG count, ULONG offset, phyaddr_segment_callback* callback, void* context) {

	while(count) {
		PHYSICAL_ADDRESS current_paddr;
		unsigned char* current_vaddr;
		ULONG current_size;
		current_paddr = MmGetPhysicalAddress(vaddr);
		current_vaddr = vaddr;
		current_size = PAGE_SIZE - offset;
		if (current_size > count)
			current_size = count;
		vaddr += current_size;
		count -= current_size;
		if (offset) {	// the source transfer address must be page aligned (4K), if offset is not 0, it is not page aligned, we need special handler
			if (callback)
				if (callback(current_vaddr, current_paddr, offset, current_size, context)) {
					offset = 0;
					continue;
				}
				else
					return FALSE;
			offset = 0;
			continue;
		}
		while (count) { // check if next page is physically continous
			PHYSICAL_ADDRESS next_paddr;
			ULONG next_size;
			next_paddr = MmGetPhysicalAddress(vaddr);
			if (next_paddr.QuadPart != current_paddr.QuadPart + current_size)
				break;
			next_size = PAGE_SIZE;
			if (next_size > count)
				next_size = count;
			vaddr += next_size;
			count -= next_size;
			current_size += next_size;
		}
		if (callback)
			if (callback(current_vaddr, current_paddr, offset, current_size, context))
				continue;
			else
				return FALSE;
	}
	return TRUE;
}

BOOL mdl_to_phyaddr(PMDL mdl, phyaddr_segment_callback* callback, void* context) {

	if (mdl) {
		unsigned char* vaddr;
		ULONG count;
		ULONG offset;
		ULONG result;
		vaddr = (unsigned char*)MmGetSystemAddressForMdlSafe(mdl, NormalPagePriority);
		count = MmGetMdlByteCount(mdl);
		offset = MmGetMdlByteOffset(mdl);
		return vaddr_to_phyaddr(vaddr,
			count, 
			offset,
			callback,
			context);
	}
	return FALSE;
}

ULONG vaddr_to_offset(PVOID vaddr) {

	PMDL mdl;
	ULONG offset;
	mdl = IoAllocateMdl(vaddr, 
		PAGE_SIZE,
		FALSE,
		FALSE, 
		NULL);
	offset = MmGetMdlByteOffset(mdl);
	IoFreeMdl(mdl);
	return offset;
}

void free_acq_rcb_mem_k(
	PVOID context, 
	struct RESOURCE_OBJECT_DATA* rdata) {

	struct ACQ_RCB_MEM_K* acq_rcb_mem_k;
	acq_rcb_mem_k = (struct ACQ_RCB_MEM_K*)context;

	if (acq_rcb_mem_k) {
		unmap_mem_kernel_to_user(acq_rcb_mem_k->m_mdl, acq_rcb_mem_k->m_acq_rcb_mem_u.m_tx_desc.m_addr);
		ExFreePool(acq_rcb_mem_k);
		acq_rcb_mem_k = NULL;
	}		
}

NTSTATUS
DutDevCtrl(
	IN PTRANSFER_OBJ pTransferObj,
    IN PSORA_RADIO Radios2[MAX_RADIO_NUMBER], 
    IN ULONG code, 
    IN PVOID Input, 
    IN ULONG InputSize, 
    IN PDEV_RET Output, 
    IN ULONG OutputSize, 
    OUT PULONG nWritten, 
    PIRP Irp)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PHWT_ADAPTER Adapter = (PHWT_ADAPTER) GlobalData.AdapterList.Flink;

    UNREFERENCED_PARAMETER(Input);
    UNREFERENCED_PARAMETER(InputSize);
    UNREFERENCED_PARAMETER(Output);
    UNREFERENCED_PARAMETER(OutputSize);
    
    switch (code)
    {
    case DUT_IO_CODE(CMD_RD_REG):
        if (InputSize == sizeof(ULONG) && OutputSize == sizeof(DEV_RET) + sizeof(ULONG))
        {
        	Output->hResult = DutReadRegisterByOffset(pTransferObj->SoraSysReg, *(PULONG)Input, (PULONG)(Output->data));
			*nWritten = sizeof(ULONG) + sizeof(DEV_RET);
			Status = STATUS_SUCCESS;
        }
        break;
    case DUT_IO_CODE(CMD_WR_REG):
        if (InputSize == sizeof(OFFSET_VALUE) && OutputSize == sizeof(DEV_RET))
        {
            Output->hResult = DutWriteRegisterByOffset(pTransferObj->SoraSysReg, ((POFFSET_VALUE)Input)->Offset, ((POFFSET_VALUE)Input)->Value);
			*nWritten = sizeof(DEV_RET);
			Status = STATUS_SUCCESS;
        }
        break;
    case DUT_IO_CODE(CMD_START_RADIO):
        if (InputSize == sizeof(START_RADIO) && OutputSize == sizeof(DEV_RET))
        {
        	PSTART_RADIO start_radio = (PSTART_RADIO)Input;
			if (start_radio->RadioNo < MAX_RADIO_NUMBER) {
				if (Radios2[start_radio->RadioNo])
					Status = DutStartRadio(Radios2[start_radio->RadioNo]);
				else {
					LIST_ENTRY radio_list;
					InitializeListHead(&radio_list);					
					do {
						Status = SoraAllocateRadioFromRCBDevice(
							&radio_list,
							1 << start_radio->RadioNo,
							HWTEST_USERNAME);						
						if (!IsListEmpty(&radio_list)) {
							PSORA_RADIO radio = CONTAINING_RECORD(radio_list.Flink, SORA_RADIO, RadiosList);
							if (!radio->__initialized) {
								Status = SoraRadioInitialize(
											radio,
											NULL, //reserved
											MODULATE_BUFFER_SIZE,
											RX_BUFFER_SIZE);
								if (Status != S_OK)
									break;

								radio->__initialized = TRUE;
							}							
							Status = DutStartRadio(radio);
							if (Status != S_OK)
								break;
							
							Radios2[start_radio->RadioNo] = radio;
						}
					} while(0);
					
					if (Status != S_OK) {
						if (!IsListEmpty(&radio_list))
							SoraReleaseRadios(&radio_list);						
						Radios2[start_radio->RadioNo] = NULL;
					}					
				}
				Output->hResult = Status;
			}
			else 
				Output->hResult = E_INVALID_PARAMETER;

            *nWritten = sizeof(DEV_RET);
            Status = STATUS_SUCCESS;
        }
        break;
    case DUT_IO_CODE(CMD_STOP_RADIO):
        if (InputSize == sizeof(STOP_RADIO) && OutputSize == sizeof(DEV_RET))
        {
        	PSTOP_RADIO stop_radio = (PSTOP_RADIO)Input;
			if (stop_radio->RadioNo < MAX_RADIO_NUMBER) {
				if (Radios2[stop_radio->RadioNo]) {
					LIST_ENTRY radio_list;
					InitializeListHead(&radio_list);					
					Status = DutStopRadio(Radios2[stop_radio->RadioNo]);
					InitializeListHead(&Radios2[stop_radio->RadioNo]->RadiosList);
					InsertTailList(&radio_list, &Radios2[stop_radio->RadioNo]->RadiosList);
					SoraReleaseRadios(&radio_list);
					Radios2[stop_radio->RadioNo] = NULL;
					Output->hResult = Status;
				}
				else 
					Output->hResult = E_RADIO_NOT_CONFIGURED;
			}
			else 
				Output->hResult = E_INVALID_PARAMETER;
			
            *nWritten = sizeof(DEV_RET);
            Status = STATUS_SUCCESS;
        }
        break;
    case DUT_IO_CODE(CMD_STOP_TX):
        if (OutputSize == sizeof(DEV_RET))
        {
            ((PHWT_ADAPTER) GlobalData.AdapterList.Flink)->UExtKeObj.TxQueue.fStopTX = 1;
            Output->hResult = S_OK;
            *nWritten = sizeof(DEV_RET);
            Status = STATUS_SUCCESS;
        }
        break;
    case DUT_IO_CODE(CMD_INFO):
        if (OutputSize == sizeof(DEV_RET) + sizeof(HWT_DETAIL_INFO))
        {
			PHWT_DETAIL_INFO hwt_detail_info = (PHWT_DETAIL_INFO)Output->data;
            Output->hResult = S_OK;
            NicCopyInfo((PHWT_DETAIL_INFO)Output->data, (PHWT_ADAPTER) GlobalData.AdapterList.Flink);
			{				
				ULONG i;
				hwt_detail_info->RadioMask = 0;
				for(i=0; i < MAX_RADIO_NUMBER; i++)
					if (Radios2[i])
						hwt_detail_info->RadioMask |= 1 << i;
            }			
            *nWritten = sizeof(DEV_RET) + sizeof(HWT_DETAIL_INFO);
            Status = STATUS_SUCCESS;
        }
    case DUT_IO_CODE(CMD_TRANSFER):
        if (OutputSize == sizeof(DEV_RET) && InputSize <= SYM_FILE_MAX_PATH_LENGTH * sizeof(WCHAR))
        {
            Output->hResult = DutQueueTransferSigFile(
                (PHWT_ADAPTER) GlobalData.AdapterList.Flink, &Adapter->TransferObj, (PCWSTR)Input);
            *nWritten = sizeof(DEV_RET);
            Status = STATUS_SUCCESS;
        }
        break;
    case DUT_IO_CODE(CMD_TX):
        if (OutputSize == sizeof(DEV_RET) && InputSize == sizeof(TIMES_ID_PAIR))
        {
        	PTIMES_ID_PAIR times_id_pair = (PTIMES_ID_PAIR)Input;
			if (times_id_pair->RadioNo < MAX_RADIO_NUMBER)
				if (Radios2[times_id_pair->RadioNo]) {
					Output->hResult = SoraKTxSignalByID(
						&((PHWT_ADAPTER) GlobalData.AdapterList.Flink)->UExtKeObj.TxQueue, 
						Radios2[times_id_pair->RadioNo], 
						((PTIMES_ID_PAIR)Input)->PacketID, 
						((PTIMES_ID_PAIR)Input)->Times);
				}
				else
					Output->hResult = E_RADIO_NOT_CONFIGURED;
			else
				Output->hResult = E_INVALID_PARAMETER;
			
            *nWritten = sizeof(DEV_RET);
            Status = STATUS_SUCCESS;
        }
        break;
	case UEXT_CMD(MIMO_TX):
	case DUT_IO_CODE(CMD_MIMO_TX): {
			if (Input && 
				InputSize >= sizeof(TIMES_ID_PAIRS) &&
				Output &&
				OutputSize >= sizeof(DEV_RET)) 
			{
				PTIMES_ID_PAIRS times_id_pairs = (PTIMES_ID_PAIRS)Input;
				Output->hResult = SoraKMimoTxSignalByID(
					&((PHWT_ADAPTER) GlobalData.AdapterList.Flink)->UExtKeObj.TxQueue, 
					Radios2,
					times_id_pairs->RadioNo,
					times_id_pairs->PacketID,
					times_id_pairs->Count,
					times_id_pairs->Times);
				
	            *nWritten = sizeof(DEV_RET);
				Status = STATUS_SUCCESS;
			}
		}
		break;	
    case DUT_IO_CODE(CMD_TX_DONE):
        if (OutputSize == sizeof(DEV_RET) && InputSize == sizeof(ULONG))
        {
            Output->hResult = SoraKTxDoneByID(
                &((PHWT_ADAPTER) GlobalData.AdapterList.Flink)->UExtKeObj.TxQueue, 
                pTransferObj, *(PULONG)Input);
            *nWritten = sizeof(DEV_RET);
            Status = STATUS_SUCCESS;
        }
        break;
    case DUT_IO_CODE(CMD_TX_GAIN):
    case DUT_IO_CODE(CMD_RX_GAIN):
    case DUT_IO_CODE(CMD_CENTRAL_FREQ):
    case DUT_IO_CODE(CMD_FREQ_OFFSET):
    case DUT_IO_CODE(CMD_RX_PA):
    case DUT_IO_CODE(CMD_SAMPLE_RATE):
        if (OutputSize == sizeof(DEV_RET) && InputSize == sizeof(RADIO_PARAMETER))
        {
        	PRADIO_PARAMETER radio_parameter = (PRADIO_PARAMETER)Input;
			if (radio_parameter->RadioNo < MAX_RADIO_NUMBER)
				if (Radios2[radio_parameter->RadioNo])
		            Output->hResult = DutSetValue(Radios2[radio_parameter->RadioNo], radio_parameter->Value, code);
				else 
					Output->hResult = E_RADIO_NOT_CONFIGURED;
			else
				Output->hResult = E_INVALID_PARAMETER;
			
            *nWritten = sizeof(DEV_RET);
            Status = STATUS_SUCCESS;
        }
        break;
    case DUT_IO_CODE(CMD_DMA):
        //if (OutputSize == sizeof(DEV_RET) + sizeof(HWT_DMA_RET)) //va + length
        //{
        //    Output->hResult = DutLockRxBuffer(
        //        (PHWT_ADAPTER) GlobalData.AdapterList.Flink, 
        //        Radio, 
        //        &(((PHWT_DMA_RET)(Output->data))->UserVA), 
        //        &(((PHWT_DMA_RET)(Output->data))->Size)
        //        );
        //    *nWritten = sizeof(DEV_RET) + sizeof(HWT_DMA_RET);
        //    Status = STATUS_SUCCESS;
        //}
        break;
	case DUT_IO_CODE(CMD_FW_VERSION): {
			if (OutputSize >= sizeof(DEV_RET) + sizeof(HWT_FW_VERSION)) {
				HWT_FW_VERSION* fw_version;
				fw_version = (HWT_FW_VERSION*)Output->data;
				fw_version->m_fw_version = Adapter->SoraRegs->FirmwareVersion.Version32;
				*nWritten = sizeof(DEV_RET) + sizeof(HWT_FW_VERSION);
				Status = STATUS_SUCCESS;
			}
		}
		break;
	case UEXT_CMD(TRANSFER_CONTINUOUS): {
			if (Input &&
				InputSize >= sizeof(TRANSFER_EX_IN) &&
				Output &&
				OutputSize >= sizeof(TRANSFER_EX_OUT)) {
				TRANSFER_EX_IN* transfer_ex_in;
				TRANSFER_EX_OUT* transfer_ex_out;
				transfer_ex_in = (TRANSFER_EX_IN*)Input;
				transfer_ex_out = (TRANSFER_EX_OUT*)Output;
				if (transfer_ex_in->Size) {
					ULONG align_size;
					PTXCB txcb;
					align_size = transfer_ex_in->Size;
					ALIGN_WITH_RCB_BUFFER_PADDING_ZERO(NULL, 
						align_size);
					txcb = SoraKAllocTXCB(pTransferObj,
						&Adapter->UExtKeObj.TxQueue,
						align_size+RCB_BUFFER_ALIGN);
					if (txcb) {	
						struct phyaddr_transfer_to_rcb_info info;
						info.transfer_obj = pTransferObj;
						info.txcb = txcb;
						info.wrapper_page_vaddr = Adapter->WrapperPageVAddr;
						info.wrapper_page_paddr = Adapter->WrapperPagePAddr;
						info.wrapper_page_lock = &Adapter->WrapperPageLock;
#ifdef DEBUG_CHECKSUM
						txcb->Base.pTxDesc->Checksum = 0;
#endif
						if (vaddr_to_phyaddr((unsigned char*)transfer_ex_in->Buffer,
							transfer_ex_in->Size,
							vaddr_to_offset(transfer_ex_in->Buffer),
							phyaddr_transfer_to_rcb, 
							&info)) {
							txcb->Base.pTxDesc->__RCBDestAddr = txcb->Base.pTxDesc->pRMD->Start;
							SoraPacketSetSignalLength(&txcb->Base, 
								align_size);
							
							txcb->PacketID = txcb->Base.pTxDesc->__RCBDestAddr;
							wcscpy(txcb->SymFileName, 
								L"Application modulated symbols");
							InterlockedExchange(&txcb->Base.fStatus, PACKET_CAN_TX);
							NdisInterlockedInsertTailList(&Adapter->UExtKeObj.TxQueue.CanTxQueue, 
								&txcb->TXCBList, 
								&Adapter->UExtKeObj.TxQueue.QueueLock);

							transfer_ex_out->hResult = S_OK;
							transfer_ex_out->TxID = txcb->PacketID;

							*nWritten = sizeof(TRANSFER_EX_OUT);
							Status = STATUS_SUCCESS;

							DoCollect(Adapter->UExtKeObj.Monitor, 
								&Adapter->UExtKeObj, 
								&Adapter->TransferObj,
								NULL, 
								UEXT_CMD(TX_RES_ALLOC), 
								transfer_ex_out, 
								sizeof(TRANSFER_EX_OUT));

							break;
						}						
						SoraPacketFreeTxResource(&Adapter->TransferObj, 
							&txcb->Base);
						NdisFreeToNPagedLookasideList(&Adapter->UExtKeObj.TxQueue.TxCBLookaside, 
							txcb);

						transfer_ex_out->hResult = E_TX_TRANSFER_FAIL;
						transfer_ex_out->TxID = 0;
						
						*nWritten = sizeof(TRANSFER_EX_OUT);
						Status = STATUS_SUCCESS;

						break;						
					}					
					transfer_ex_out->hResult = E_NOT_ENOUGH_RESOURCE;
					transfer_ex_out->TxID = 0;
					
					*nWritten = sizeof(TRANSFER_EX_OUT);
					Status = STATUS_SUCCESS;

					break;
				}
				transfer_ex_out->hResult = E_INVALID_PARAMETER;
				transfer_ex_out->TxID = 0;
				
				*nWritten = sizeof(TRANSFER_EX_OUT);
				Status = STATUS_SUCCESS;
				
				break;
			}
			Status = STATUS_INVALID_PARAMETER;
		}
		break;				
	case UEXT_CMD_DIRECTIN(TRANSFER_DISCONTINUOUS): {
			if (InputSize &&
				Output &&
				OutputSize >= sizeof(TRANSFER_EX_OUT)) {
				TRANSFER_EX_OUT* transfer_ex_out;				
				transfer_ex_out = (TRANSFER_EX_OUT*)Output;	 
				{
					ULONG align_size;
					PTXCB txcb;
					align_size = InputSize;
					ALIGN_WITH_RCB_BUFFER_PADDING_ZERO(NULL, 
						align_size);
					txcb = SoraKAllocTXCB(&Adapter->TransferObj,
						&Adapter->UExtKeObj.TxQueue,
						align_size+RCB_BUFFER_ALIGN);
					if (txcb) {
						struct phyaddr_transfer_to_rcb_info info;
						info.transfer_obj = pTransferObj;
						info.txcb = txcb;
						info.wrapper_page_vaddr = Adapter->WrapperPageVAddr;
						info.wrapper_page_paddr = Adapter->WrapperPagePAddr;
						info.wrapper_page_lock = &Adapter->WrapperPageLock;
#ifdef DEBUG_CHECKSUM
						txcb->Base.pTxDesc->Checksum = 0;
#endif						
						if (Irp->MdlAddress) {
							BOOL result;
							PMDL mdl;
							result = FALSE;							
							mdl = Irp->MdlAddress;
							while(mdl) {
								result = mdl_to_phyaddr(mdl,
									phyaddr_transfer_to_rcb, 
									&info);
								if (result) {
									mdl = mdl->Next;
									continue;
								}
								break;
							}
							if (result) {
								txcb->Base.pTxDesc->__RCBDestAddr = txcb->Base.pTxDesc->pRMD->Start;
								SoraPacketSetSignalLength(&txcb->Base, 
									align_size);
								
								txcb->PacketID = txcb->Base.pTxDesc->__RCBDestAddr;
								wcscpy(txcb->SymFileName, 
									L"Application modulated symbols");
								InterlockedExchange(&txcb->Base.fStatus, PACKET_CAN_TX);
								NdisInterlockedInsertTailList(&Adapter->UExtKeObj.TxQueue.CanTxQueue, 
									&txcb->TXCBList, 
									&Adapter->UExtKeObj.TxQueue.QueueLock);
								
								transfer_ex_out->hResult = S_OK;
								transfer_ex_out->TxID = txcb->PacketID;
								
								*nWritten = sizeof(TRANSFER_EX_OUT);
								Status = STATUS_SUCCESS;

								DoCollect(Adapter->UExtKeObj.Monitor, 
									&Adapter->UExtKeObj, 
									&Adapter->TransferObj,
									NULL, 
									UEXT_CMD(TX_RES_ALLOC), 
									transfer_ex_out, 
									sizeof(TRANSFER_EX_OUT));
								
								break;
							}
							SoraPacketFreeTxResource(&Adapter->TransferObj, 
								&txcb->Base);
							NdisFreeToNPagedLookasideList(&Adapter->UExtKeObj.TxQueue.TxCBLookaside, 
								txcb);								

							transfer_ex_out->hResult = E_TX_TRANSFER_FAIL;
							transfer_ex_out->TxID = txcb->PacketID;
							
							*nWritten = sizeof(TRANSFER_EX_OUT);
							Status = STATUS_SUCCESS;
							
							break;
						}
						else 
						if (Irp->AssociatedIrp.SystemBuffer) {
							if (vaddr_to_phyaddr((unsigned char*)Irp->AssociatedIrp.SystemBuffer,
								InputSize,
								vaddr_to_offset(Irp->AssociatedIrp.SystemBuffer),
								phyaddr_transfer_to_rcb, 
								&info)) {								
								txcb->Base.pTxDesc->__RCBDestAddr = txcb->Base.pTxDesc->pRMD->Start;
								SoraPacketSetSignalLength(&txcb->Base, 
									align_size);
								
								txcb->PacketID = txcb->Base.pTxDesc->__RCBDestAddr;
								wcscpy(txcb->SymFileName, 
									L"Application modulated symbols");
								InterlockedExchange(&txcb->Base.fStatus, PACKET_CAN_TX);
								NdisInterlockedInsertTailList(&Adapter->UExtKeObj.TxQueue.CanTxQueue, 
									&txcb->TXCBList, 
									&Adapter->UExtKeObj.TxQueue.QueueLock);
								
								transfer_ex_out->hResult = S_OK;
								transfer_ex_out->TxID = txcb->PacketID;

								DoCollect(Adapter->UExtKeObj.Monitor, 
									&Adapter->UExtKeObj, 
									&Adapter->TransferObj,
									NULL, 
									UEXT_CMD(TX_RES_ALLOC), 
									transfer_ex_out, 
									sizeof(TRANSFER_EX_OUT));
								
								*nWritten = sizeof(TRANSFER_EX_OUT);
								Status = STATUS_SUCCESS;
								
								break;
							}
							SoraPacketFreeTxResource(&Adapter->TransferObj, 
								&txcb->Base);
							NdisFreeToNPagedLookasideList(&Adapter->UExtKeObj.TxQueue.TxCBLookaside, 
								txcb);

							transfer_ex_out->hResult = E_TX_TRANSFER_FAIL;
							transfer_ex_out->TxID = 0;
							
							*nWritten = sizeof(TRANSFER_EX_OUT);
							Status = STATUS_SUCCESS;
							
							break;
						}
						else {
							SoraPacketFreeTxResource(&Adapter->TransferObj, 
								&txcb->Base);
							NdisFreeToNPagedLookasideList(&Adapter->UExtKeObj.TxQueue.TxCBLookaside, 
								txcb);

							transfer_ex_out->hResult = E_INVALID_PARAMETER;
							transfer_ex_out->TxID = 0;
							
							*nWritten = sizeof(TRANSFER_EX_OUT);
							Status = STATUS_SUCCESS;
							
							break;
						}							
					}
					transfer_ex_out->hResult = E_NOT_ENOUGH_RESOURCE;
					transfer_ex_out->TxID = 0;
					
					*nWritten = sizeof(TRANSFER_EX_OUT);
					Status = STATUS_SUCCESS;
					
					break;					
				}
			}
			Status = STATUS_INVALID_PARAMETER;
			break;			
		}
	case UEXT_CMD(INDICATE_PACKET): {		
			if (Input &&
				InputSize &&
				Output &&
				OutputSize >= sizeof(DEV_RET)) {
				extern NTSTATUS MPIndicatePacket(PHWT_ADAPTER Adapter, PVOID Input, ULONG InputSize, PDEV_RET Output, ULONG OutputSize, PIRP Irp);
				Status = MPIndicatePacket(Adapter, Input, InputSize, Output, OutputSize, Irp);
				if (Status != STATUS_PENDING) {
					*nWritten = sizeof(DEV_RET);
					Status = STATUS_SUCCESS;
				}
			}
		}
		break;
	case UEXT_CMD(GET_SEND_PACKET): {
			if (Output &&
				OutputSize >= sizeof(GET_SEND_PACKET_OUT)) {
				extern NTSTATUS MPGetSendPacket(PHWT_ADAPTER Adapter, PVOID Input, ULONG InputSize, PVOID Output, ULONG OutputSize, PIRP Irp);
				Status = MPGetSendPacket(Adapter, Input, InputSize, Output, OutputSize, Irp);
				if (Status != STATUS_PENDING) {
					if (Output->hResult == ERROR_SUCCESS)
						DoCollectEx(Adapter->UExtKeObj.Monitor,
							&Adapter->UExtKeObj,
							NULL,
							code, 
							Output,
							sizeof(GET_SEND_PACKET_OUT),
							free_get_send_packet,
							Adapter, 
							FALSE);					
					*nWritten = sizeof(GET_SEND_PACKET_OUT);
					Status = STATUS_SUCCESS;
				}
			}
		}
		break;
	case UEXT_CMD(COMPLETE_SEND_PACKET): {
			if (Input &&
				InputSize >= sizeof(COMPLETE_SEND_PACKET_IN) &&
				Output &&
				OutputSize >= sizeof(DEV_RET)) {
				struct RESOURCE_OBJECT_DATA* rdata;
				KIRQL irql;
				KeAcquireSpinLock(&Adapter->UExtKeObj.Monitor->m_monitor_lock, &irql);
				rdata = Get_COMPLETE_SEND_PACKET(Adapter->UExtKeObj.Monitor,
					Input,
					sizeof(COMPLETE_SEND_PACKET_IN));
				if (rdata)
					RemoveEntryList(&rdata->m_resource_entry);				
				KeReleaseSpinLock(&Adapter->UExtKeObj.Monitor->m_monitor_lock, irql);
				
				if (rdata) {
					extern NTSTATUS MPCompleteSendPacket(PHWT_ADAPTER Adapter, PVOID Input, ULONG InputSize, PDEV_RET Output, ULONG OutputSize, PIRP Irp);
					MPCompleteSendPacket(Adapter, Input, InputSize, Output, OutputSize, Irp);
					ExFreePool(rdata);
				}
				else
					Output->hResult = ERROR_INVALID_HANDLE;				
				
				*nWritten = sizeof(DEV_RET);
				Status = STATUS_SUCCESS;					
			}
		}
		break;
	case UEXT_CMD(ENABLE_GET_SEND_PACKET): {
			if (Input &&
				InputSize >= sizeof(ENABLE_GET_SEND_PACKET_IN) &&
				Output &&
				OutputSize >= sizeof(DEV_RET)) {
				PENABLE_GET_SEND_PACKET_IN enable_in;
				enable_in = (PENABLE_GET_SEND_PACKET_IN)Input;
				if (enable_in->Semaph) {
					PKSEMAPHORE semaph;
					Output->hResult = ObReferenceObjectByHandle(enable_in->Semaph,
						SEMAPHORE_ALL_ACCESS,
						*ExSemaphoreObjectType,
						UserMode,
						&semaph,
						NULL);
					if (NT_SUCCESS(Output->hResult)) {
						KIRQL irql;
						KeAcquireSpinLock(&Adapter->UExtKeObj.SendPacketLock,
							&irql);
						if (Adapter->UExtKeObj.SendPacketControl)
							ObDereferenceObject(Adapter->UExtKeObj.SendPacketControl);
						Adapter->UExtKeObj.SendPacketControl = semaph;
						KeReleaseSpinLock(&Adapter->UExtKeObj.SendPacketLock, 
							irql);
					}
				}
				*nWritten = sizeof(DEV_RET);
				Status = STATUS_SUCCESS;
			}
		}
		break;
	case private_ext(ACQ_RCB_MEM): {
			if (Input &&
				InputSize >= sizeof(struct ACQ_RCB_MEM_IN) &&
				Output &&
				OutputSize >= sizeof(struct ACQ_RCB_MEM_U)) {
				struct ACQ_RCB_MEM_IN* alloc_rcb_in;
				struct ACQ_RCB_MEM_U* alloc_rcb_u;
				struct ACQ_RCB_MEM_K* alloc_rcb_k;
				PTXCB txcb;
				TRANSFER_EX_OUT transfer_ex_out;
				alloc_rcb_in = (struct ACQ_RCB_MEM_IN*)Input;
				alloc_rcb_u = (struct ACQ_RCB_MEM_U*)Output;				
				txcb = NULL;
				alloc_rcb_k = NULL;
				do {
					if (!alloc_rcb_in->m_rcb_size)
						break;
					
					alloc_rcb_k = (struct ACQ_RCB_MEM_K*)ExAllocatePoolWithTag(
							PagedPool,
							sizeof(struct ACQ_RCB_MEM_K),
							'bcra');
					if (!alloc_rcb_k)
						break;

					RtlZeroMemory(
						alloc_rcb_k,
						sizeof(struct ACQ_RCB_MEM_K));

					txcb = SoraKAllocTXCB(
						pTransferObj,
						&Adapter->UExtKeObj.TxQueue,
						alloc_rcb_in->m_rcb_size);
					if (!txcb)
						break;

					txcb->Base.pTxDesc->__RCBDestAddr = txcb->Base.pTxDesc->pRMD->Start;
					SoraPacketSetSignalLength(&txcb->Base, txcb->Base.pTxDesc->pRMD->Units * TX_RCB_MEM_UNIT_SIZE);
					txcb->PacketID = txcb->Base.pTxDesc->__RCBDestAddr;
					wcscpy(txcb->SymFileName, L"Allocated RCB Buffer");
					InterlockedExchange(&txcb->Base.fStatus, PACKET_CAN_TX);
					NdisInterlockedInsertTailList(&Adapter->UExtKeObj.TxQueue.CanTxQueue, &txcb->TXCBList, &Adapter->UExtKeObj.TxQueue.QueueLock);

					transfer_ex_out.hResult = S_OK;
					transfer_ex_out.TxID = txcb->PacketID;
					DoCollect(
						Adapter->UExtKeObj.Monitor, 
						&Adapter->UExtKeObj, 
						&Adapter->TransferObj,
						NULL, 
						UEXT_CMD(TX_RES_ALLOC), 
						&transfer_ex_out, 
						sizeof(TRANSFER_EX_OUT));
					
					alloc_rcb_k->m_acq_rcb_mem_u.m_rcb_addr = txcb->Base.pTxDesc->pRMD->Start;
					alloc_rcb_k->m_acq_rcb_mem_u.m_rcb_size = txcb->Base.pTxDesc->pRMD->Units * TX_RCB_MEM_UNIT_SIZE;

					if (!map_mem_kernel_to_user(
						txcb->Base.pTxDesc,
						sizeof(TX_DESC),
						&alloc_rcb_k->m_mdl, 
						&alloc_rcb_k->m_acq_rcb_mem_u.m_tx_desc.m_addr))
						break;
					
					alloc_rcb_k->m_acq_rcb_mem_u.m_tx_desc.m_size = sizeof(TX_DESC);
					alloc_rcb_k->m_acq_rcb_mem_u.m_tx_desc.m_hi_phy_addr = txcb->Base.pTxDesc->ThisPa.u.HighPart;
					alloc_rcb_k->m_acq_rcb_mem_u.m_tx_desc.m_lo_phy_addr = txcb->Base.pTxDesc->ThisPa.u.LowPart;
					alloc_rcb_k->m_acq_rcb_mem_u.m_tx_desc.m_res_handle_u.m_handle = DoCollectEx(
						Adapter->UExtKeObj.Monitor,
						NULL,
						NULL,
						private_ext(ACQ_RCB_MEM),
						NULL,
						0,
						free_acq_rcb_mem_k,
						alloc_rcb_k,
						TRUE);
					if (!alloc_rcb_k->m_acq_rcb_mem_u.m_tx_desc.m_res_handle_u.m_handle)
						break;

					alloc_rcb_u->m_rcb_addr = alloc_rcb_k->m_acq_rcb_mem_u.m_rcb_addr;
					alloc_rcb_u->m_rcb_size = alloc_rcb_k->m_acq_rcb_mem_u.m_rcb_size;
					alloc_rcb_u->m_tx_desc.m_res_handle_u = alloc_rcb_k->m_acq_rcb_mem_u.m_tx_desc.m_res_handle_u;
					alloc_rcb_u->m_tx_desc.m_addr = alloc_rcb_k->m_acq_rcb_mem_u.m_tx_desc.m_addr;
					alloc_rcb_u->m_tx_desc.m_size = alloc_rcb_k->m_acq_rcb_mem_u.m_tx_desc.m_size;
					alloc_rcb_u->m_tx_desc.m_hi_phy_addr = alloc_rcb_k->m_acq_rcb_mem_u.m_tx_desc.m_hi_phy_addr;
					alloc_rcb_u->m_tx_desc.m_lo_phy_addr = alloc_rcb_k->m_acq_rcb_mem_u.m_tx_desc.m_lo_phy_addr;
					
					*nWritten = sizeof(struct ACQ_RCB_MEM_U);
					Status = STATUS_SUCCESS;
					
				} while(0);
				
				if (Status != STATUS_SUCCESS) {
					if (txcb) {
						if (SoraKTxDoneByID(&Adapter->UExtKeObj.TxQueue, pTransferObj, txcb->PacketID) != S_OK) {
							SoraPacketFreeTxResource(pTransferObj, &txcb->Base);
							NdisFreeToNPagedLookasideList(&Adapter->UExtKeObj.TxQueue.TxCBLookaside, txcb);
						}
						txcb = NULL;
					}
					if (alloc_rcb_k) {
						if (alloc_rcb_k->m_acq_rcb_mem_u.m_tx_desc.m_res_handle_u.m_handle)
							FreeResourceObjectData(
								Adapter->UExtKeObj.Monitor, 
								(struct RESOURCE_OBJECT_DATA*)alloc_rcb_k->m_acq_rcb_mem_u.m_tx_desc.m_res_handle_u.m_handle);
						else
							free_acq_rcb_mem_k(alloc_rcb_k, NULL);
						alloc_rcb_k = NULL;	
					}
				}
			}
		}
		break;
	case private_ext(REL_MAP_MEM): {
			if (Input &&
				InputSize >= sizeof(RES_HANDLE_U)) {
				FreeResourceObjectData(
					Adapter->UExtKeObj.Monitor,
					*(struct RESOURCE_OBJECT_DATA**)Input);
				
				*nWritten = 0;
				Status = STATUS_SUCCESS;
			}
		}
		break;
    default:
        *nWritten = SoraKHandleUExtReq(
            &Adapter->UExtKeObj,
            &Adapter->TransferObj,
            Adapter->Radios2,
            code, Input, InputSize, Output, OutputSize);
        DbgPrint("[UExtK] SoraKHandleUExtReq return %d\n", *nWritten);
        Status = STATUS_SUCCESS;
        break;
   	}
    return Status;
}

NTSTATUS NICDispatch(
            IN PDEVICE_OBJECT           DeviceObject,
            IN PIRP                     Irp
            )
{
    PIO_STACK_LOCATION  irpStack    = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS            Status      = STATUS_SUCCESS;
    HRESULT             hr          = S_OK;
    
    ULONG               inlen, outlen, nWritten, code;
    PVOID               buffer;
    PHWT_ADAPTER         pAdapter = NULL;

    PAGED_CODE();

    pAdapter = (PHWT_ADAPTER) GlobalData.AdapterList.Flink;

    switch (irpStack->MajorFunction)
    {
    case IRP_MJ_CREATE: {		
			PHWT_ADAPTER Adapter;
			Adapter = GetMPAdapter(DeviceObject);
			if (Adapter) {
				if (Adapter->Removed) {					
					Irp->IoStatus.Status = STATUS_DEVICE_REMOVED;
					Irp->IoStatus.Information = 0;
					IoCompleteRequest(Irp, 
						IO_NO_INCREMENT);
					return STATUS_DEVICE_REMOVED;
				}
				SoraKHandleUExtReq(&Adapter->UExtKeObj,
					&Adapter->TransferObj,
					Adapter->Radios2,
					UEXT_CMD(REDIRECT_IRP_MJ_CREATE), 
					NULL, 
					0, 
					NULL, 
					0);
				InterlockedIncrement(&Adapter->Reference);
				KeResetEvent(&Adapter->Exit);
			}
		}
        break;
    case IRP_MJ_CLEANUP:
        break;
    case IRP_MJ_CLOSE: {		
		PHWT_ADAPTER Adapter;
		Adapter = GetMPAdapter(DeviceObject);
		if (Adapter)
			SoraKHandleUExtReq(&Adapter->UExtKeObj,
				&Adapter->TransferObj,
				Adapter->Radios2,
				UEXT_CMD(REDIRECT_IRP_MJ_CLOSE), 
				NULL, 
				0, 
				NULL, 
				0);
			switch(InterlockedDecrement(&Adapter->Reference)) {
			case 0:
				CleanQueuedSendPacket(Adapter);
				KeSetEvent(&Adapter->Exit,
					IO_NO_INCREMENT,
					FALSE);
				break;
			default:
				break;				
			}
    	}
        break;
    case IRP_MJ_DEVICE_CONTROL: 
        code   = irpStack->Parameters.DeviceIoControl.IoControlCode;
        buffer = Irp->AssociatedIrp.SystemBuffer;
        inlen = irpStack->Parameters.DeviceIoControl.InputBufferLength;
        outlen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
        Status = DutDevCtrl(&pAdapter->TransferObj, pAdapter->Radios2, code, buffer, inlen, buffer, outlen, &nWritten, Irp);
		if (Status != STATUS_PENDING)
	        Irp->IoStatus.Information   = nWritten;         
        break;
    }
	
    if (Status != STATUS_PENDING) {
	    Irp->IoStatus.Status = Status;
	    IoCompleteRequest(Irp, 
			IO_NO_INCREMENT);
    }
    return Status;
}


