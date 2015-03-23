/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

DeviceIo.c

Abstract:

IRP handlers

Environment:

Kernel mode

--*/

#include "device.h"
#include "_private_ext_k.h"

/*++
Routine Description:

This event is called when the framework receives IRP_MJ_DEVICE_CONTROL
requests from the system.

Arguments:

Queue - Handle to the framework queue object that is associated
with the I/O request.
Request - Handle to a framework request object.

OutputBufferLength - ReturnLength of the request's output buffer,
if an output buffer is available.
InputBufferLength - ReturnLength of the request's input buffer,
if an input buffer is available.

IoControlCode - the driver-defined or system-defined I/O control code
(IOCTL) that is associated with the request.

Return Value:

VOID

Note: 
        IRQL: <= DISPATCH_LEVEL
--*/

HRESULT PCIEHandleRadiosReq(
            IN PDEVICE_EXTENSION       pDevExt,
            IN PVOID                   pInputBuffer,
            IN SIZE_T                  InputBufferSize,
            IN OUT PVOID               pOutputBuffer,
            IN SIZE_T                  OutputBufferSize)
{
    PCWSTR pUpperDriverName = *(PCWSTR*)pInputBuffer;
    ULONG  nRadios          =  *((ULONG*)pOutputBuffer);
    LIST_ENTRY *pRadiosHead = (LIST_ENTRY*)pOutputBuffer;
    HRESULT hRes            = S_OK;

    ASSERT(OutputBufferSize == sizeof(LIST_ENTRY));
    ASSERT(InputBufferSize == sizeof(PCWSTR));

    hRes = SoraAllocateRadio2(
                &pDevExt->RadioManager,
                pRadiosHead, 
                nRadios,
                pUpperDriverName);
	switch(hRes) {
	case S_OK:		
		pDevExt->RadioMask |= nRadios;
		break;
	default:
		break;
	}
    return hRes;
}

SORA_EXTERN_C 
NTSTATUS __CopyOldRxBuffer(PSORA_RADIO pRadio, PUCHAR pBuffer, ULONG Size, PULONG pWritten);
/*++

Note: 
        IRQL : <= DISPATCH_LEVEL
--*/
HRESULT RadioManagerDumpRX(
            IN PSORA_RADIO pRadio, 
            IN OUT PVOID Buffer, 
            IN ULONG Size, 
            OUT PULONG pnWritten)
{
    ULONG   nRetry = 1024;
    PSORA_RADIO_RX_STREAM pRxStream = SORA_GET_RX_STREAM(pRadio);
    *pnWritten = 0;

    if (__IS_RADIO_FREE(pRadio))
    {
        DbgPrint("[Dump] radio is free, can't be dumpped\n");
        return E_FAIL;
    }

    if (IS_NOT_A16(Buffer))
    {
        DbgPrint("[DUMP] Output buffer is not aligned\n");
        return E_BUFFER_NOT_ALIGNED;
    }
    SoraGetLock(&pRadio->__Lock); //Prevent stop of radio
    if (SORA_RADIO_STARTED(pRadio))
    {
        __CopyOldRxBuffer(pRadio, (PUCHAR)Buffer, Size, pnWritten);
    }
    SoraFreeLock(&pRadio->__Lock);
    return S_OK;
}

void free_map_reg_k(
	PVOID context, 
	struct RESOURCE_OBJECT_DATA* rdata) {

	struct MAP_MEM_K* mregk;
	mregk = (struct MAP_MEM_K*)context;

	if (mregk) {
		unmap_mem_kernel_to_user(mregk->m_mdl, mregk->m_map_mem_u.m_addr);
		ExFreePool(mregk);
		mregk = NULL;
	}		
}

HRESULT 
PCIEMapSoraRegister(
	PDEVICE_EXTENSION pDevExt,
	PVOID pInputBuffer,
	SIZE_T InputBufferSize,
	PVOID pOutputBuffer,
	SIZE_T OutputBufferSize,
	ULONG* RetInfo) {

	if (pOutputBuffer &&
		OutputBufferSize >= sizeof(struct MAP_MEM_U)) {

		struct MAP_MEM_U* mregu;
		struct MAP_MEM_K* mregk;
		mregu = (struct MAP_MEM_U*)pOutputBuffer;
		mregk = map_mem_kernel_to_user_ex(
			pDevExt->RadioManager.__RegsBase,
			pDevExt->RadioManager.__RegsLength);
		if (!mregk)
			goto error_exit;

		mregk->m_map_mem_u.m_hi_phy_addr = 0;
		mregk->m_map_mem_u.m_lo_phy_addr = 0;		
		mregk->m_map_mem_u.m_res_handle_u.m_handle = DoCollectEx(
			pDevExt->ResourceMonitor,
			NULL,
			NULL,
			__IOCTL_MAP_SORA_REGISTER,
			NULL,
			0,
			free_map_reg_k,
			mregk,
			TRUE);

		mregu->m_res_handle_u = mregk->m_map_mem_u.m_res_handle_u;
		mregu->m_addr = mregk->m_map_mem_u.m_addr;
		mregu->m_size = mregk->m_map_mem_u.m_size;
		mregu->m_hi_phy_addr = mregk->m_map_mem_u.m_hi_phy_addr;
		mregu->m_lo_phy_addr = mregk->m_map_mem_u.m_lo_phy_addr;
		
		if ( RetInfo)
			*RetInfo = sizeof(struct MAP_MEM_U);

		return S_OK;
		
error_exit:
	
		free_map_reg_k(
			mregk,
			NULL);
	}

	if ( RetInfo)
		*RetInfo = 0;
	
	return E_FAIL;	
}

HRESULT 
PCIEReleaseResource(
	PDEVICE_EXTENSION pDevExt,
	PVOID pInputBuffer,
	SIZE_T InputBufferSize,
	PVOID pOutputBuffer,
	SIZE_T OutputBufferSize,
	ULONG* RetInfo) {

	if (pInputBuffer &&
		InputBufferSize >= sizeof(RES_HANDLE_U)) {
		FreeResourceObjectData(
			pDevExt->ResourceMonitor,
			*(struct RESOURCE_OBJECT_DATA**)pInputBuffer);
	}
	if ( RetInfo)
		*RetInfo = 0;
	
	return S_OK;
}
	
/*++
PCIEDeviceIoCtrl handles IRP from user-mode application and upper driver.

Note: IRQL: <= DISPATCH_LEVEL unless device's or driver's 
      WDF_OBJECT_ATTRIBUTES struct is set to WdfExecutionLevelPassive.
--*/
VOID PCIEDeviceIoCtrl(
    IN WDFQUEUE      Queue,
    IN WDFREQUEST    Request,
    IN size_t        OutputBufferLength,
    IN size_t        InputBufferLength,
    IN ULONG         IoControlCode
    )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    HRESULT                 hRes   = S_OK;

    PDEVICE_EXTENSION       pDevExt;
    
    PVOID                   pInputBuffer;
    PVOID                   pOutputBuffer;
    size_t                  InputBufferSize;
    size_t                  OutputBufferSize;
    
    ULONG                   RetInfo = 0;    
    //KIRQL                   Irql = KeGetCurrentIrql();
    //
    // Get the device extension.
    //
    pDevExt = PCIEGetDeviceContext(WdfIoQueueGetDevice( Queue ));

    if (InputBufferLength > 0)
    {
        Status = WdfRequestRetrieveInputBuffer(Request, InputBufferLength, &pInputBuffer, &InputBufferSize);
        if( !NT_SUCCESS(Status) )
        {
            WdfRequestComplete(Request, Status);
            return;
        }
    }
    else
    {
        pInputBuffer = NULL;
        InputBufferSize = 0;
    }

    if (OutputBufferLength > 0)
    {
        Status = WdfRequestRetrieveOutputBuffer(Request, OutputBufferLength, &pOutputBuffer, &OutputBufferSize);
        if( !NT_SUCCESS(Status) )
        {
            WdfRequestComplete(Request, Status);
            return;
        }
    }
    else
    {
        pOutputBuffer = NULL;
        OutputBufferSize = 0;
    }

    //
    // Handle this request's specific code.
    //
    switch (IoControlCode)
    {
    case __IOCTL_REQUEST_RADIOS:
        hRes = PCIEHandleRadiosReq(
                    pDevExt, 
                    pInputBuffer, 
                    InputBufferSize, 
                    pOutputBuffer, 
                    OutputBufferSize);
        RetInfo = sizeof(LIST_ENTRY);
        break;
    case IOCTL_DUMP_RADIO_RX:
        {

            ULONG radioIndex = (*(ULONG*)pInputBuffer);
            ULONG nWritten;
            DbgPrint("[DUMP] Output buffer size %08x, address=%08x, radioIndex=%08x\n", 
                OutputBufferSize, pOutputBuffer, radioIndex);
            RadioManagerDumpRX(
                &pDevExt->RadioManager.__radio_pool[radioIndex], 
                pOutputBuffer, 
                OutputBufferSize, 
                &nWritten);
            RetInfo = nWritten;
        }
        break;
	case __IOCTL_RELEASE_RADIOS: {
			LIST_ENTRY* head;
			head = (LIST_ENTRY*)pInputBuffer;
			if (head) {
				LIST_ENTRY* entry;
				for(entry = head->Flink; entry != head; entry = entry->Flink) {
					SORA_RADIO* radio;
					radio = CONTAINING_RECORD(entry, SORA_RADIO, RadiosList);
					pDevExt->RadioMask &= ~(1<<radio->__radio_no);
				}
			}
		}
		break;
	case __IOCTL_MAP_SORA_REGISTER:
		hRes = PCIEMapSoraRegister(
			pDevExt, 
			pInputBuffer,
			InputBufferSize,
			pOutputBuffer,
			OutputBufferSize,
			&RetInfo);
		break;
	case __IOCTL_RELEASE_RESOURCE:
		hRes = PCIEReleaseResource(
			pDevExt, 
			pInputBuffer,
			InputBufferSize,
			pOutputBuffer,
			OutputBufferSize,
			&RetInfo);		
		break;
	case __IOCTL_ACQUIRE_RADIO_MANAGER: {
			if (OutputBufferLength >= sizeof(PSORA_RADIO_MANAGER*)) {
				PSORA_RADIO_MANAGER* sora_radio_manager;				
				sora_radio_manager = (PSORA_RADIO_MANAGER*)pOutputBuffer;
			   *sora_radio_manager = &pDevExt->RadioManager;
			    pDevExt->RadioManagerRef ++;
				RetInfo = sizeof(PSORA_RADIO_MANAGER*);
				Status = STATUS_SUCCESS;
			}
			else {
				RetInfo = 0;
				Status = STATUS_BUFFER_TOO_SMALL;
			}
		}
		break;
	case __IOCTL_RELEASE_RADIO_MANAGER: {
				pDevExt->RadioManagerRef --;
				RetInfo = 0;
				Status = STATUS_SUCCESS;
		}
		break;		
    default:
		Status = STATUS_INVALID_PARAMETER;
        break;
    }
    
    if (Status != STATUS_PENDING)
    {
        WdfRequestCompleteWithInformation(Request, Status, RetInfo);
    }
}
