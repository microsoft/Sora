#pragma once

#include "_radio_manager.h"

typedef struct _TRANSFER_OBJ_ {

	PSORA_RADIO_MANAGER		RadioManager;
	PTX_RM					TransferResMgr;
	KSPIN_LOCK				TransferLock;
	__PSORA_RADIO_REGS		TransferReg;
	PVOID					SoraSysReg;
	PSORA_RADIO				SoraHWOpRadio;

	PTXSAMPLE						__TxSampleBufVa;	
	PHYSICAL_ADDRESS				__TxSampleBufPa;
	ULONG							__TxSampleBufSize;
	LONG						   	__TxBufLock;    // Lock to access the Tx sample buffer	 
	
} TRANSFER_OBJ, *PTRANSFER_OBJ;

SORA_EXTERN_C
NTSTATUS
SORAAPI
SoraAcquireRadioManager(
    OUT PSORA_RADIO_MANAGER* pRadioManager);

SORA_EXTERN_C
NTSTATUS
SORAAPI	
SoraReleaseRadioManager();

