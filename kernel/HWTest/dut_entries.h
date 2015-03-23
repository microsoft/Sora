#ifndef _DUT_ENTRIES_H
#define _DUT_ENTRIES_H

#include "__transfer_obj.h"

HRESULT DutReadRegisterByOffset(PVOID SoraSysReg, ULONG Offset, PULONG Value);
HRESULT DutWriteRegisterByOffset(PVOID SoraSysReg, ULONG Offset, ULONG Value);
HRESULT DutStartRadio(PSORA_RADIO Radio);
HRESULT DutStopRadio(PSORA_RADIO Radio);

typedef struct _HWT_ADAPTER *PHWT_ADAPTER;

HRESULT 
DutQueueTransferSigFile(
    PHWT_ADAPTER Adapter, 
    PTRANSFER_OBJ pTransferObj, 
    PCWSTR wszSigFile);

HRESULT DutSetValue(PSORA_RADIO Radio, ULONG Gain, ULONG code);


//HRESULT 
//DutLockRxBuffer(
//    PHWT_ADAPTER Adapter, 
//    PSORA_RADIO pRadio, 
//    PVOID *ppUserSpaceBuffer,
//    PULONG BufferSize);

#endif