/*++
Copyright (c) Microsoft Corporation

Module Name: Sora Radio Manager.

Abstract: The file includes the function implementation of Sora radio manager.

History: 
          3/12/2009: Created by senxiang
--*/

#include "sora.h"
#include "__reg_file.h"
#include "__transfer_obj.h"

#pragma alloc_text (PAGE, SoraAllocateRadioFromDevice)
#pragma warning(disable:4100 4101)

/*
SoraAllocateRadioFromDevice send IRP to PCIE device to request free radios.

 Parameters:
            pcieDevice : name of Sora PCIE device.
            ppRadios   : free radios assigned to requester; 
                         pointer array maintained by caller.
            nRadio     : requested radio number.
            userName   : requester name, it is used by PCIE to track radio usage. 
                         Caller should not modify or destroy before release the radios.
 Return:    S_OK if succeeded. E_NOT_ENOUGH_FREE_RADIOS if nRadio exceeds
            current free radio number. 

 Note: If S_OK is returned, ppRadios is filled with pointer to free radios. This 
 function is only called by upper miniport driver.

 History:   3/23/2009 Created by senxiang
*/
HRESULT 
SORAAPI
SoraAllocateRadioFromDevice(
    IN PCWSTR           pcieDeviceName,
    IN OUT LIST_ENTRY   *pRadios,
    IN ULONG            nRadio, 
    IN PCWSTR           userName)
{
    HRESULT                 hRes    = S_OK;
    NTSTATUS                status  = STATUS_SUCCESS;
    
    PFILE_OBJECT            pBuddyFileObject = NULL;
    PDEVICE_OBJECT          pBuddyDeviceObject;
    UNICODE_STRING          buddyDeviceName;
    IO_STATUS_BLOCK         IoStatus;
    KEVENT                  Event;
    PIRP                    pIrp             = NULL;
    PIO_STACK_LOCATION      pStackLocation   = NULL;
    
    PAGED_CODE();

    do
    {
        if (0 == nRadio)
        {
            hRes = E_INVALID_PARAMETER;
            break;
        }

        RtlInitUnicodeString(&buddyDeviceName, pcieDeviceName);
        status = IoGetDeviceObjectPointer(
                        &buddyDeviceName,
                        FILE_ALL_ACCESS,
                        &pBuddyFileObject,
                        &pBuddyDeviceObject);
        if (!NT_SUCCESS(status))
        {
            hRes = E_DEVICE_NOT_FOUND;
            break;
        }
        KeInitializeEvent(&Event, NotificationEvent, FALSE);
        //It's a problem here,this (ULONG *)pRadios = nRadio statement change pRadios to nRadio
        //that means pRadios point to adress nRadio(0x1)
        //(ULONG *)pRadios = nRadio; /* Trick, use output buffer to input radio number */
        //changed by YiChen 21-04-2009
        *((ULONG *)pRadios) = nRadio;
        pIrp = IoBuildDeviceIoControlRequest(
                    __IOCTL_REQUEST_RADIOS,
                    pBuddyDeviceObject,
                    (PVOID) &userName,
                    sizeof(PCWSTR),
                    pRadios,
                    sizeof(LIST_ENTRY),
                    FALSE,
                    &Event,
                    &IoStatus);
        if (NULL == pIrp)
        {
            hRes = E_NOT_ENOUGH_RESOURCE;
            break;
        }

        pStackLocation = IoGetNextIrpStackLocation(pIrp);

        pStackLocation->FileObject = pBuddyFileObject;
        
        status = IoCallDriver(pBuddyDeviceObject, pIrp);

        if (status == STATUS_PENDING)
        {
            KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL); 

#ifdef USER_MODE 
//USER_MODE Sora library only provide limited module. 
//SoraAllocateRadioFromDevice NEVER called in User mode.
            IoStatus.Status = 0;
#endif
            status = IoStatus.Status;

            if (!NT_SUCCESS(status))
            {
                hRes = E_NOT_ENOUGH_RESOURCE;
            }

            if (IsListEmpty(pRadios))
            {
                hRes = E_NOT_ENOUGH_FREE_RADIOS;
            }
            /* DDK: file obj dereference indirectly deref the device object */
        }
    } while(FALSE);

	if (pBuddyFileObject) {
		ObDereferenceObject(pBuddyFileObject);
		pBuddyFileObject = NULL;
	}

    return hRes;
}

SORA_EXTERN_C void SoraGetLock(IN OUT volatile LONG *pLock);
void SoraGetLock(IN OUT volatile LONG *pLock) // Flag lock
{
    LONG Origin;
    do{
        while ( *pLock == 1 ) 
        {
            _mm_pause();
        }
        Origin = InterlockedExchange(pLock, 1);
        if (Origin == 0)
            break;
    } while(TRUE);
}

HRESULT SoraGetLockEx(IN IN OUT volatile LONG *pLock, int ms)
{
    LONG Origin;
    HRESULT hr = E_FAIL;
    LARGE_INTEGER interval;
    interval.QuadPart = -10 * 1000;
    
    do{
        
        if (*pLock == 0)
        {
            Origin = InterlockedExchange(pLock, 1);
            if (Origin == 0)
            {
                hr = S_OK;
                break;
            }
        }
        
        DbgPrint("[UExtK] mod buffer lock waiting\n, *pLock = %d, i=%d", *pLock, ms);
        ms--;
        
        //KeDelayExecutionThread(KernelMode, FALSE, &interval); //wait 1ms;
        if (ms >= 0)
            _mm_pause();
        else
            break;
    } while(TRUE);

    return hr;
}

SORA_EXTERN_C void SoraFreeLock(OUT PLONG pLock);

void SoraFreeLock(OUT PLONG pLock) // Flag lock
{
    *pLock = 0;
}



