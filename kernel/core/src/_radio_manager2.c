/*++
Copyright (c) Microsoft Corporation

Module Name: Sora Radio Manager v2 for new hardware.

Abstract: The file includes the function implementation of Sora radio manager v2.

History: 
          5/25/2009: Created by senxiang
--*/

#include "sora.h"
#include "__reg_file.h"
#include "__radio_man_internal.h"
#include "__rx_internal.h"
#include "__transfer_obj.h"

#pragma warning(disable:4100 4101)

void __SORA_HW_ENROLL_RX_BUFFER_UNSAFE(PSORA_RADIO pRadio);
HRESULT __SoraHwResetRCBRegsUnsafe(IN SORA_REGS_HANDLE pRegisterManager);
HRESULT __SoraHwResetSysRegsUnsafe(__PSORA_REGISTERS pRegFile);
VOID __SORA_HW_READ_RADIO_ID(OUT PSORA_RADIO pRadio);
VOID SoraCleanupRadio2(IN OUT PSORA_RADIO pRadio);

void SoraInitRegisterManager2(
        IN OUT SORA_REGS_HANDLE pRegisterManager,
        IN __PSORA_REGISTERS    pSoraRegs, 
        IN ULONG                index)
{
    ((__PHW_REGISTER_FILE)pRegisterManager)->pSoraSysRegs = 
        (PVOID)pSoraRegs;
    ((__PHW_REGISTER_FILE)pRegisterManager)->pRadioRegs = 
        (&pSoraRegs->RadioRegs[index]);
}

/* 
__SoraInitRadio2 initialize radio oject with assigned memory resource.
 Parameters:
            pSoraRegs    : Virtual address of mapped physical address.
            nIndex       : radio index.
            pRadio       : point to uninitialized radio object 

 Return:    

 Note:
            All radios are assumed identical currently.
            IRQL: PASSIVE_LEVEL

 History:   3/23/2009 Created by senxiang

*/
void __SoraInitRadio2(
    __PSORA_REGISTERS           pSoraRegs,
    IN ULONG                    nIndex,
    OUT __PSORA_RADIO           pRadio)
{
    
    do 
    {
        pRadio->__radio_no = nIndex;
        
        __MARK_RADIO_FREE(pRadio);
        __MARK_RADIO_USED_BY(pRadio, NULL);
        
        pRadio->__uRxGain       = SORA_RADIO_DEFAULT_RX_GAIN;
        pRadio->__uTxGain       = SORA_RADIO_DEFAULT_TX_GAIN;

        pRadio->__Lock          = 0;
        KeInitializeSpinLock(&pRadio->__HWOpLock);

        SoraInitRegisterManager2(
                    &(pRadio->__ctrl_reg), pSoraRegs, nIndex);
        
        pRadio->__fCanWork = FALSE;
        InitializeListHead(&(pRadio->RadiosList));

        KeInitializeEvent(&pRadio->PnPEvent, NotificationEvent, FALSE);
        SORA_HW_READ_RADIO_POWER_STATUS(pRadio);
        __SORA_HW_READ_RADIO_ID(pRadio);
    }while (FALSE);
    
}

/*
SoraInitRadioManager2 initializes all radios attached to the PCIE board 
with assigned hardware resource, currently only physical memory segment.

Parameters:
        MemStartAddr : Physical memory start address.
        nMemSegLength: the memory segment length. 
        pRadioMgr    : Radio manager object
Return:    
        S_OK if succeeded. Error code E_UNKOWN_HARDWARE, E_NOT_ENOUGH_RESOURCE, 
        if failed.

Note:
        All radios are assumed identical currently.

History:   5/25/2009 Created by senxiang
*/
HRESULT SoraInitRadioManager2(
    IN PHYSICAL_ADDRESS         MemStartAddr,
    IN ULONG                    nMemSegLength,
    OUT __PSORA_RADIO_MANAGER   pRadioMgr
    )
{
    HRESULT hr;
    s_uint32 FirmwareVersion;
    
    RtlZeroMemory(pRadioMgr, sizeof(__SORA_RADIO_MANAGER));
    do{
        ULONG i;
        if (nMemSegLength < sizeof(__SORA_REGISTERS))
        {
            hr = E_UNKOWN_HARDWARE;
            break;
        }
        pRadioMgr->__RegsLength = nMemSegLength; 
        pRadioMgr->__RegsBase   = 
            (__PSORA_REGISTERS) MmMapIoSpace( MemStartAddr, 
                                              nMemSegLength, 
                                              MmNonCached );
        if (NULL == (pRadioMgr->__RegsBase))
        {
            hr = E_NOT_ENOUGH_RESOURCE;
            break;
        }

        // initialize each radio
        for (i = 0; i < MAX_RADIO_NUMBER; i++)
        {
            __SoraInitRadio2( pRadioMgr->__RegsBase, 
                              i, 
                              &(pRadioMgr->__radio_pool[i]));
        }
        
        pRadioMgr->__radio_count = MAX_RADIO_NUMBER;
        
        FirmwareVersion = pRadioMgr->__RegsBase->FirmwareVersion.Version32;
        if (FirmwareVersion < SORA_HW_VERSION)
        {
            hr = E_HW_INCOMPATIBLE;
            break;
        }
        
        hr = SoraInitTxResManager( &pRadioMgr->__TX_RESMGR );
        
        FAILED_BREAK(hr);

        hr = __SoraHwResetSysRegsUnsafe(pRadioMgr->__RegsBase);

    }while(FALSE);
    
	if (FAILED(hr))
	{
		SoraCleanupRadioManager2(pRadioMgr);
	}
    return hr;
}

/* 

__SoraDestroyRadio2 releases all resources owned by the radio.

 Parameters:
            pSoraRadio : point to radio object 

 Return:    
 
 Note: It is called by PCIE Radio Manger driver.

 History:   5/26/2009 Created by senxiang

*/
HRESULT __SoraDestroyRadio2(IN OUT __PSORA_RADIO pSoraRadio)
{
    HRESULT hr = S_OK;
    if (!__IS_RADIO_FREE(pSoraRadio))
    {
        /* notify upper driver to release the radio */
        KeSetEvent(&pSoraRadio->ForceReleaseEvent, 1, FALSE);
        hr = E_RADIO_CANNOT_DESTROY;
    }
    return hr;
}

/* 
SoraCleanupRadioManager2 releases all resources owned by the radio manager.

 Parameters:
            pRadioMgr : point to radio manager object.
 Return:    E_RADIO_CANNOT_DESTROY if some radios are still used by upper driver.
 
 Note:

 History:   5/26/2009 Created by senxiang
*/
HRESULT SoraCleanupRadioManager2(IN OUT __PSORA_RADIO_MANAGER pRadioMgr)
{
    ULONG index = 0;
    KIRQL oldIrql;
    HRESULT hr = S_OK;

    KeAcquireSpinLock(&pRadioMgr->__lock, &oldIrql);
    for (; index < pRadioMgr->__radio_count; index++)
    {
    	SoraCleanupRadio2(&(pRadioMgr->__radio_pool[index]));
        hr = __SoraDestroyRadio2(&(pRadioMgr->__radio_pool[index]));
        FAILED_BREAK(hr);
    }
    if (SUCCEEDED(hr))
    {
        if (pRadioMgr->__RegsBase)
            MmUnmapIoSpace(pRadioMgr->__RegsBase, pRadioMgr->__RegsLength);
        pRadioMgr->__radio_count = 0;
        SoraCleanTxResManager(&pRadioMgr->__TX_RESMGR);
    }
    KeReleaseSpinLock(&pRadioMgr->__lock, oldIrql);
    return hr;
}

#ifndef USER_MODE
HRESULT SoraGetLockEx(IN IN OUT volatile LONG *pLock, int ms);
#endif

/*++
SoraInitSampleBuffer init modulate buffer of Radio. The sample buffer type 
is WriteCombined.
--*/
HRESULT SoraInitModulateBuffer(PTRANSFER_OBJ pTransferObj, ULONG SampleBufferSize)
{
    PHYSICAL_ADDRESS    PhysicalAddress      = {0, 0}; 
    PHYSICAL_ADDRESS    PhysicalAddressLow   = {0, 0};
    PHYSICAL_ADDRESS    PhysicalAddressHigh  = {0x80000000, 0};
    HRESULT hr = E_NOT_ENOUGH_CONTINUOUS_PHY_MEM;

    ALIGN_WITH_RCB_BUFFER_PADDING_ZERO(NULL, SampleBufferSize);

    //ExInitializeFastMutex(&pRadio->__ModSampleBufMutex);
    pTransferObj->__TxBufLock = 0;

    if (!pTransferObj->__TxSampleBufVa)
    {
        pTransferObj->__TxSampleBufVa = 
                (PCOMPLEX8) MmAllocateContiguousMemorySpecifyCache(
                            SampleBufferSize, 
                            PhysicalAddressLow,
                            PhysicalAddressHigh,
                            PhysicalAddress,
                            MmWriteCombined);
        if (pTransferObj->__TxSampleBufVa)
        {
            //ULONG i;
            hr                          = S_OK;
            pTransferObj->__TxSampleBufPa    = MmGetPhysicalAddress(pTransferObj->__TxSampleBufVa);
            ASSERT(pTransferObj->__TxSampleBufPa.u.LowPart % 0x00001000 == 0); //should always 4K aligned
            pTransferObj->__TxSampleBufSize  = SampleBufferSize;
            //for (i = 0; i < SampleBufferSize / 4; i++)
            //{
            //    ((PULONG)pRadio->__TxSampleBufVa)[i] = i;
            //}

        }
    }
    return hr;
}

/*

SoraConfigRadios2 configures each radio for customized physical layer.

Parameters:
            pRadiosHead : radios list head entry; The radios are assigned through 
                          SoraAllocateRadioFromDevice.
            
            nRXBufSize  : Rx buffer size.
            nTXBufNum   : Rx buffer number.

 Return: S_OK if succeeded; error code if failed.
 
 Note: It is called after SoraAllocateRadioFromDevice. SoraConfigRadios initializes
       TX, RX buffer for each radio.
       IRQL <= DISPATCH_LEVEL

 History:   3/31/2009 Created by senxiang
*/
HRESULT 
SORAAPI 
SoraRadioInitialize(
    IN OUT PSORA_RADIO          pRadio, 
    IN PVOID                    pPhyContextExt,
    IN ULONG                    nSampleBuffersize,
    IN ULONG                    nRXBufSize)
{
    HRESULT hRes;
    do
    {
        hRes = SoraInitRxBufferManager(
            SORA_GET_RX_QUEUE(pRadio),
            nRXBufSize,
            1);
    
        FAILED_BREAK(hRes);
/*		
        hRes = SoraInitModulateBuffer(pTransferObj, nSampleBuffersize);
        FAILED_BREAK(hRes);
*/              
        SORA_EXTEND_PHY_CONTEXT(pRadio, pPhyContextExt);
        
        SORA_RESET_RADIO_PHY_RX_INFO_BASE(pRadio);
    }while(FALSE);
    
    if (FAILED(hRes))
    {
        SoraCleanupRxBufferManager(SORA_GET_RX_QUEUE(pRadio));//we only need cleanup Rx buffer
    }
    return hRes;
}

VOID SoraCleanupRadio2(IN OUT PSORA_RADIO pRadio)
{
    SoraCleanupRxBufferManager(SORA_GET_RX_QUEUE(pRadio));
    
    if (pRadio->pTxResMgr) 
    {
        pRadio->pTxResMgr = NULL;
    }

/*	
    if (pRadio->__TxSampleBufVa)
    {
        MmFreeContiguousMemorySpecifyCache(
            pRadio->__TxSampleBufVa, 
            pRadio->__TxSampleBufSize,
            MmWriteCombined);
        pRadio->__TxSampleBufVa    = NULL;
        pRadio->__TxSampleBufSize  = 0;
    }
*/    
}

/*
SoraRadioStart start the radio assigned for the physical layer. 

 Parameters:
            pRadio: radio need to be started
 
 Return: 

 Note: 

 History:   3/31/2009 Created by senxiang


*/
HRESULT 
SORAAPI
SoraRadioStart(
            IN OUT PSORA_RADIO      pRadio, 
            IN ULONG                RXGain,
            IN ULONG                TXGain, 
            IN PSORA_RADIO_CONFIG   pConfig)
{
    HRESULT hRes = S_OK;
    KIRQL OldIrql;
    UNREFERENCED_PARAMETER(pConfig);
    
    pRadio->__uRxGain = RXGain;
    pRadio->__uTxGain = TXGain;
    do
    {
        if (SORA_RADIO_WELL_CONFIGED2(pRadio))
        {
#pragma warning(disable:4311)
            KeAcquireSpinLock(&pRadio->__HWOpLock, &OldIrql);
            __SoraHwResetRCBRegsUnsafe(&pRadio->__ctrl_reg);
            __SORA_HW_ENROLL_RX_BUFFER_UNSAFE(pRadio);
            
            KeReleaseSpinLock(&pRadio->__HWOpLock, OldIrql);

#pragma warning(default:4311)	
            SORA_RESET_RADIO_PHY_RX_INFO_BASE(pRadio);
            
            //hRes = WARPRFConfig(pRadio);
            //FAILED_BREAK(hRes);
            SoraAbsRFStart(pRadio);
            SoraHwSetTXVGA1(pRadio, SORA_RADIO_DEFAULT_TX_GAIN);
            SoraHwSetRXVGA1(pRadio, SORA_RADIO_DEFAULT_RX_GAIN);
            SoraHwSetRXPA(pRadio, SORA_RADIO_DEFAULT_RX_PA);
            pRadio->__fCanWork = TRUE;   
        }
        else
        {
            hRes = E_RADIO_NOT_CONFIGURED;
            break;
        }
    }while (FALSE);

    return hRes;
}

/*
SoraStopRadio stops the radio assigned for the physical layer. 

 Parameters:
            pRadio: radio need to be stoped
 
 Return: SoraStopRadio

 Note: 

 History:   3/31/2009 Created by senxiang

*/
VOID __SoraHwStop(PSORA_RADIO pRadio);
VOID SoraStopRadio2(IN OUT PSORA_RADIO pRadio)
{
    pRadio->__fCanWork = FALSE;   
    SORA_HW_STOP_RX(pRadio);
    _mm_pause(); _mm_pause(); _mm_pause(); _mm_pause(); //wait left symbols written to RX buffer.
//    __SoraHwStop(pRadio);
}

/*
SoraReleaseRadio releases all assigned radios. Upper driver should stop 
MAC before release any radio.

 Parameters:
            pRadiosHead    : free radios double linked list to be released; 
 Return:    

 Note:      
            IRQL: PASSIVE_LEVEL
            If the radio is in dump mode, SoraReleaseRadio will wait till dump exits.
            Upper application driver must release all TX resource occupied by packet 
            and physical frame cache before release radios. Otherwise TX resources will 
            be leaked. 

 History:   3/23/2009 Created by senxiang
*/

VOID SORAAPI SoraReleaseRadios(IN LIST_ENTRY   *pRadiosHead)
{
    LIST_ENTRY  *pEntry = pRadiosHead->Flink;
    
    while(pEntry != pRadiosHead)
    {
        PSORA_RADIO pRadio = CONTAINING_RECORD(pEntry, SORA_RADIO, RadiosList);
        
        SoraGetLock(&pRadio->__Lock); //check if the radio is referenced by dump module.
        SoraStopRadio2(pRadio);
        // SoraCleanupRadio2(pRadio);

        __MARK_RADIO_USED_BY(pRadio, NULL);
        __MARK_RADIO_FREE(pRadio);
        SoraFreeLock(&pRadio->__Lock);
        pEntry = pEntry->Flink;
    }

#ifndef USER_MODE
	{		
		NTSTATUS status;
		UNICODE_STRING DeviceName;
		PFILE_OBJECT FileObject;
		PDEVICE_OBJECT DeviceObject;
		
		RtlInitUnicodeString(&DeviceName, SORA_DEVICE_NAME);
        status = IoGetDeviceObjectPointer(&DeviceName,
										  FILE_ALL_ACCESS,
										  &FileObject,
										  &DeviceObject);
		if (NT_SUCCESS(status)) {
			KEVENT event;
			PIRP irp;
			IO_STATUS_BLOCK iostatus;
			KeInitializeEvent(&event, NotificationEvent, FALSE);			
			irp = IoBuildDeviceIoControlRequest(__IOCTL_RELEASE_RADIOS,
												DeviceObject,
												pRadiosHead,
												sizeof(LIST_ENTRY),
												NULL,
												0,
												FALSE,
												&event, 
												&iostatus);
			if (irp) {
				PIO_STACK_LOCATION stacklocation;
				stacklocation = IoGetNextIrpStackLocation(irp);
				stacklocation->FileObject = FileObject;
				status = IoCallDriver(DeviceObject, irp);
				if (status == STATUS_PENDING) {
					KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
					status = iostatus.Status;
				}
			}
			ObDereferenceObject(FileObject);
		}
	}
#endif

    InitializeListHead(pRadiosHead);
}

/*
SoraAllocateRadio assign free radios to upper driver.

 Parameters:
            pRadioMgr   : radio manager object handle
            ppRadios    : free radios assigned to requester; 
                         pointer array maintained by caller.
            nRadio      : requested radio number.
            userName    : upper driver name 

 Return:    S_OK if succeeded. E_NOT_ENOUGH_FREE_RADIOS if nRadio exceeds
            current free radio number. 

 Note: If S_OK is returned, ppRadios is filled with pointer to free radios.

 History:   3/23/2009 Created by senxiang
*/
HRESULT SoraAllocateRadio2(
        IN PSORA_RADIO_MANAGER      pRadioMgr,
        IN OUT LIST_ENTRY           *pRadios,
        IN ULONG                    nRadio, 
        IN PCWSTR                   userName)
{
    HRESULT     hRes    = S_OK;
    KIRQL       oldIrql;
    ULONG       index   = 0;
    ULONG       count   = 0;
    LIST_ENTRY  *pEntry = NULL;
    PSORA_RADIO pRadio  = NULL;

    InitializeListHead(pRadios);

    KeAcquireSpinLock(&pRadioMgr->__lock, &oldIrql);
    do 
    {
        if (nRadio == 0)
        {
            hRes = E_INVALID_PARAMETER;
            break;
        }

		while (BitScanReverse(&index, nRadio)) {
			if (index > pRadioMgr->__radio_count - 1)
				break;
	        if (SORA_RADIO_IS_ALIVE(&pRadioMgr->__radio_pool[index]) && 
	            __IS_RADIO_FREE(&pRadioMgr->__radio_pool[index])) {	        
				nRadio &= ~(1<<index);	            
	            count++;
	            InsertTailList(pRadios, &(pRadioMgr->__radio_pool[index].RadiosList));
	        }
			else 
				break;
		}
		
        if (nRadio != 0)
        {
            InitializeListHead(pRadios);
            hRes = E_NOT_ENOUGH_FREE_RADIOS;
        }
        else
        {   
            pEntry = pRadios->Flink;
            while (pEntry != pRadios)
            {   
                pRadio = CONTAINING_RECORD(pEntry, SORA_RADIO, RadiosList);
                __MARK_RADIO_USED(pRadio);
                __MARK_RADIO_USED_BY(pRadio, userName);

                pRadio->pTxResMgr = &pRadioMgr->__TX_RESMGR;

                KeInitializeEvent(&pRadio->ForceReleaseEvent, NotificationEvent, FALSE);
                
                pEntry = pEntry->Flink;
            }
        }
    } while (FALSE);
    KeReleaseSpinLock(&pRadioMgr->__lock, oldIrql);
    return hRes;
}

NTSTATUS
SORAAPI
SoraAcquireRadioManager(
    OUT PSORA_RADIO_MANAGER* pRadioManager) {

	NTSTATUS status;
	UNICODE_STRING DeviceName;
	PFILE_OBJECT FileObject = NULL;
	PDEVICE_OBJECT DeviceObject;

	RtlInitUnicodeString(&DeviceName, SORA_DEVICE_NAME);
	status = IoGetDeviceObjectPointer(&DeviceName,
									  FILE_ALL_ACCESS,
									  &FileObject,
									  &DeviceObject);
	if (NT_SUCCESS(status)) {
		KEVENT event;
		PIRP irp;
		IO_STATUS_BLOCK iostatus = { 0 };
		KeInitializeEvent(&event, NotificationEvent, FALSE);			
		irp = IoBuildDeviceIoControlRequest(__IOCTL_ACQUIRE_RADIO_MANAGER,
											DeviceObject,
											NULL,
											0,
											pRadioManager,
											sizeof(PSORA_RADIO_MANAGER*),
											FALSE,
											&event, 
											&iostatus);
		if (irp) {
			PIO_STACK_LOCATION stacklocation;
			stacklocation = IoGetNextIrpStackLocation(irp);
			stacklocation->FileObject = FileObject;
			status = IoCallDriver(DeviceObject, irp);
			if (status == STATUS_PENDING) {
				KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
				status = iostatus.Status;
			}			
		}
		ObDereferenceObject(FileObject);
	}	
	return status;
}

NTSTATUS
SORAAPI	
SoraReleaseRadioManager() {

	NTSTATUS status;
	UNICODE_STRING DeviceName;
	PFILE_OBJECT FileObject = NULL;
	PDEVICE_OBJECT DeviceObject;

	RtlInitUnicodeString(&DeviceName, SORA_DEVICE_NAME);
	status = IoGetDeviceObjectPointer(&DeviceName,
									  FILE_ALL_ACCESS,
									  &FileObject,
									  &DeviceObject);
	if (NT_SUCCESS(status)) {
		KEVENT event;
		PIRP irp;
		IO_STATUS_BLOCK iostatus = { 0 };
		KeInitializeEvent(&event, NotificationEvent, FALSE);			
		irp = IoBuildDeviceIoControlRequest(__IOCTL_RELEASE_RADIO_MANAGER,
											DeviceObject,
											NULL,
											0,
											NULL,
											0,
											FALSE,
											&event, 
											&iostatus);
		if (irp) {
			PIO_STACK_LOCATION stacklocation;
			stacklocation = IoGetNextIrpStackLocation(irp);
			stacklocation->FileObject = FileObject;
			status = IoCallDriver(DeviceObject, irp);
			if (status == STATUS_PENDING) {
				KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
				status = iostatus.Status;
			}
		}
		ObDereferenceObject(FileObject);
	}	
	return status;
}

