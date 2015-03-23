/*
We use overlapped io instead.
Based on doc, we have to pass overlapped structure for each DeviceIoControl.
For now, all the io that driver supported are all synchronous.
So we can just ignore the check process for overlapped structure.
*/

#include "sora.h"
#include "thread_if.h"
#include "send_packet_data.h"

#define HWT_DEVNAME "\\\\.\\HwTest"

static HANDLE SoraQueuedSendPacket = NULL;
static HANDLE SoraExitSendPacket = NULL;
static HANDLE SoraDevice = INVALID_HANDLE_VALUE;

static LONG LockLong = 0;
static LIST_ENTRY AllocBufferListHead = { 0 };

#define LockList(lock) {							\
													\
	while(1) {										\
		if (*(volatile LONG*)lock) {				\
			_mm_pause();							\
			continue;								\
		}											\
		if (InterlockedCompareExchange(lock,		\
			1,										\
			0))										\
			continue;								\
		break;										\
	}												\
}

#define UnlockList(lock) {							\
													\
	InterlockedCompareExchange(lock,	 			\
		0,											\
		1);											\
}

struct KernelBufferEntry : LIST_ENTRY {

	ALLOC_KERNEL_BUFFER_OUT AllocKBOut;
	ULONG Size;
};



#define is_sora_valid	SoraQueuedSendPacket && SoraExitSendPacket && SoraDevice

#define WAIT_OBJECT_1	WAIT_OBJECT_0+1

BOOLEAN SoraUInitUserExtension(const char * szDevName)
{
	BOOLEAN ret = FALSE;

	if (!AllocBufferListHead.Flink &&
		!AllocBufferListHead.Blink)
		InitializeListHead(&AllocBufferListHead);

    if (SoraQueuedSendPacket == NULL ||
		SoraExitSendPacket == NULL ||
		SoraDevice == INVALID_HANDLE_VALUE) {

		SoraUCleanUserExtension();
		
		SoraQueuedSendPacket =
			CreateSemaphore(NULL, 
				0,
				MAX_QUEUED_SEND_PACKET,
				"SoraQueuedSendPacket");

		SoraExitSendPacket = 
			CreateEvent(NULL,
				TRUE,
				FALSE,
				NULL);
			
        SoraDevice = 
            CreateFileA ( 
                szDevName, GENERIC_READ | GENERIC_WRITE, 
                0,  
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
                NULL );
		
		if (SoraQueuedSendPacket && 
			SoraExitSendPacket &&
			SoraDevice != INVALID_HANDLE_VALUE)
			ret = TRUE;
		else
			SoraUCleanUserExtension();
    }
	else
		ret = TRUE;
	
    return ret;
}

//BOOLEAN SoraUInitUserExtension()
//{
//    if (SoraDevice == NULL)
//    {
//        HANDLE hDev = 
//            CreateFile ( 
//                DEVNAME, GENERIC_READ | GENERIC_WRITE, 
//                0,  
//                NULL,
//                OPEN_EXISTING,
//                FILE_ATTRIBUTE_NORMAL,
//                NULL );
//        SoraDevice = hDev;
//    }
//    return (SoraDevice != NULL);
//}

VOID SoraUCleanUserExtension()
{
	LockList(&LockLong);
	
	LIST_ENTRY* entry;
	entry = AllocBufferListHead.Flink;

	while(entry != &AllocBufferListHead) {
		struct KernelBufferEntry* be;
		be = (struct KernelBufferEntry*)entry;
		
		RELEASE_KERNEL_BUFFER_IN release_kb_in = { 0 };
		DWORD length;
		release_kb_in.Buff = be->AllocKBOut.Buff;

		OVERLAPPED overlap = { 0 };
		DeviceIoControl(SoraDevice, 
						UEXT_CMD(RELEASE_KERNEL_BUFFER), 
						&release_kb_in,
						sizeof(RELEASE_KERNEL_BUFFER_IN),
						NULL,
						0,
						&length,
						&overlap);
		
		LIST_ENTRY* next_entry;
		next_entry = entry->Flink;
		
		RemoveEntryList(entry);
		delete be;

		entry = next_entry;
	}	
	UnlockList(&LockLong);
	
    if (SoraDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(SoraDevice);
		SoraDevice = NULL;
    }

	if (SoraExitSendPacket) {
		CloseHandle(SoraExitSendPacket);
		SoraExitSendPacket = NULL;
	}	

	if (SoraQueuedSendPacket) {
		CloseHandle(SoraQueuedSendPacket);
		SoraQueuedSendPacket = NULL;
	}
}

HRESULT 
SoraURadioMapTxSampleBuf(
    IN ULONG RadioNo, 
    OUT PVOID *ppUserBuffer, 
    OUT PULONG Size)
{
    MAP_MOD_BUF_OUT out = { 0 };
    ULONG nWritten = 0;

	OVERLAPPED overlap = { 0 };
    BOOL ret = DeviceIoControl ( 
                    SoraDevice, 
                    UEXT_CMD(MAP_MOD_BUFFER), 
                    &RadioNo,
                    sizeof(RadioNo),
                    &out,
                    sizeof(out),
                    &nWritten,
                    &overlap); 

    if (nWritten == 0)
    {
       return E_FAIL;
    }
    if (SUCCEEDED(out.hResult))
    {
        *ppUserBuffer   = out.ModBuf;
        *Size           = out.ModBufSize;
    }
    return out.hResult;
}

HRESULT SoraURadioUnmapTxSampleBuf(IN ULONG RadioNo, IN PVOID UserBuffer)
{
    HRESULT hr;
    BOOL ret;
    ULONG nWritten = 0;
    UNMAP_MOD_BUF_IN input = { 0 };
    input.RadioNo   = RadioNo;
    input.ModBuf    = UserBuffer;

	OVERLAPPED overlap = { 0 };
    ret = DeviceIoControl ( 
                    SoraDevice, 
                    UEXT_CMD(UNMAP_MOD_BUFFER), 
                    &input,
                    sizeof(input),
                    &hr,
                    sizeof(hr),
                    &nWritten,
                    &overlap);
    if (nWritten == 0)
    {
       return E_FAIL;
    }

    return hr;
}

HRESULT 
SoraURadioTransfer(
    IN ULONG RadioNo, 
    IN ULONG SampleSize,
    OUT PULONG TxID)
{
    TX_RES_ALLOC_IN Input;
    TX_RES_ALLOC_OUT Output;
    ULONG nWritten;

    Input.RadioNo = RadioNo;
    Input.SampleSize = SampleSize;
	
    OVERLAPPED overlap = { 0 }; 
    DeviceIoControl ( 
        SoraDevice, 
        UEXT_CMD(TX_RES_ALLOC), 
        &Input,
        sizeof(Input),
        &Output,
        sizeof(Output),
        &nWritten,
        &overlap);    
    if (nWritten == 0)
    {
       return E_FAIL;
    }

    *TxID = Output.TxID; 
    return Output.hResult;
}

HRESULT 
SoraURadioTransferContinuous(
	IN PVOID SampleBuffer,
	IN ULONG SampleSize,
	OUT PULONG TxID) {

	TRANSFER_EX_IN transfer_ex_in = { 0 };
	TRANSFER_EX_OUT transfer_ex_out;
	ULONG nWritten;
	transfer_ex_in.Buffer = SampleBuffer;
	transfer_ex_in.Size = SampleSize;
	nWritten = 0;

	OVERLAPPED overlap = { 0 };
	if (!DeviceIoControl( 
		SoraDevice, 
		UEXT_CMD(TRANSFER_CONTINUOUS), 
		&transfer_ex_in,
		sizeof(TRANSFER_EX_IN),
		&transfer_ex_out,
		sizeof(TRANSFER_EX_OUT),
		&nWritten,
		&overlap))
		return GetLastError();

	*TxID = transfer_ex_out.TxID;

	return transfer_ex_out.hResult;
}

HRESULT 
SoraURadioTransferDiscontinuous(
	IN PVOID SampleBuffer,
	IN ULONG SampleSize,
	OUT PULONG TxID) {

	TRANSFER_EX_OUT transfer_ex_out;
	ULONG nWritten;
	nWritten = 0;

	OVERLAPPED overlap = { 0 };
	if (!DeviceIoControl ( 
		SoraDevice, 
		UEXT_CMD_DIRECTIN(TRANSFER_DISCONTINUOUS), 
		SampleBuffer,
		SampleSize,
		&transfer_ex_out,
		sizeof(TRANSFER_EX_OUT),
		&nWritten,
		&overlap))
		return GetLastError();

	*TxID = transfer_ex_out.TxID;

	return transfer_ex_out.hResult;

}

HRESULT 
SoraURadioTransferEx(
    IN ULONG RadioNo,
    IN PVOID SampleBuffer,
    IN ULONG SampleSize,
    OUT PULONG TxID) {

	BOOL isContinuous;
	isContinuous = FALSE;
	
	LockList(&LockLong);
	
	LIST_ENTRY* entry;
	entry = AllocBufferListHead.Flink;
	while(entry != &AllocBufferListHead) {
		struct KernelBufferEntry* be;
		be = (struct KernelBufferEntry*)entry;
		if (((ULONG)SampleBuffer >= (ULONG)be->AllocKBOut.Buff) &&
			((ULONG)SampleBuffer + SampleSize <= (ULONG)be->AllocKBOut.Buff + be->Size)) {
			isContinuous = be->AllocKBOut.is_Continuous;
			break;
		}
		entry = entry->Flink;
	}
	UnlockList(&LockLong);

	if (isContinuous)
		return SoraURadioTransferContinuous(SampleBuffer,
			SampleSize,
			TxID);
	else
		return SoraURadioTransferDiscontinuous(SampleBuffer,	//  we make OS to prepare a physical continuous memory for us, the overhead is memory copy
			SampleSize,											//  it is still ok to force invoke SoraURadioTransferContinuous even if they are not physically continuous
			TxID);
}

HRESULT SoraURadioTx(IN ULONG RadioNo, IN ULONG TxID)
{
    ULONG nWritten = 0;
    HRESULT hr;
    TX_IN Input;
    Input.RadioNo   = RadioNo;
    Input.TxID      = TxID;

	OVERLAPPED overlap = { 0 };
    DeviceIoControl ( SoraDevice, UEXT_CMD(TX_SIGNAL), &Input,
        sizeof(Input), &hr, sizeof(hr), &nWritten, &overlap);    
    if (nWritten == 0)
    {
       return E_FAIL;
    }
    return hr;
}

HRESULT SoraURadioMimoTx(IN ULONG* RadioNo, IN ULONG* TxID, ULONG Count) {

	TIMES_ID_PAIRS times_ip_pairs = { 0 };
	HRESULT hr;
	ULONG nWritten = 0;
	ULONG i;
	for(i=0; i < Count; i++) {
		times_ip_pairs.RadioNo[i] = RadioNo[i];
		times_ip_pairs.PacketID[i] = TxID[i];
	}
	times_ip_pairs.Count = Count;
	times_ip_pairs.Times = 1;

	OVERLAPPED overlap = { 0 };
	DeviceIoControl (
		SoraDevice, 
		UEXT_CMD(MIMO_TX),
		&times_ip_pairs,
		sizeof(TIMES_ID_PAIRS), 
		&hr,
		sizeof(HRESULT), 
		&nWritten,
		&overlap);
	if (nWritten != sizeof(HRESULT))
	{
		return E_FAIL;
	}
	return hr;
}

HRESULT SoraURadioTxFree(IN ULONG RadioNo, IN ULONG TxID)
{
    ULONG nWritten = 0;
    HRESULT hr;
    TX_RES_REL_IN Input;

    Input.RadioNo   = RadioNo;
    Input.TxID      = TxID;

	OVERLAPPED overlap = { 0 };
    DeviceIoControl ( SoraDevice, UEXT_CMD(TX_RES_RELEASE), &Input,
        sizeof(Input), &hr, sizeof(hr), &nWritten, &overlap);    
    if (nWritten == 0)
    {
       return E_FAIL;
    }
    return hr;
}

HRESULT
SoraURadioAllocVStreamMask(
    IN ULONG RadioNo,
    OUT PULONG pVStreamMask)
{
    VSTREAM_MASK_ALLOC_IN Input;
    VSTREAM_MASK_ALLOC_OUT Output;
    ULONG nWritten;

    Input.RadioNo = RadioNo;

	OVERLAPPED overlap = { 0 };
    DeviceIoControl(
        SoraDevice,
        UEXT_CMD(VSTREAM_MASK_ALLOC),
        &Input,
        sizeof(Input),
        &Output,
        sizeof(Output),
        &nWritten,
        &overlap);
    if (nWritten != sizeof(Output))
    {
        // Set to safest mask
        *pVStreamMask = 0;
        return E_FAIL;
    }

    *pVStreamMask = Output.VStreamMask;
    return Output.hResult;
}

HRESULT
SoraURadioReleaseVStreamMask(
    IN ULONG RadioNo,
    IN ULONG VStreamMask)
{
    VSTREAM_MASK_RELEASE_IN Input;
    HRESULT Output;
    ULONG nWritten;

    if (VStreamMask == 0) return S_OK;

    Input.RadioNo = RadioNo;
    Input.VStreamMask = VStreamMask;

	OVERLAPPED overlap = { 0 };
    DeviceIoControl(
        SoraDevice,
        UEXT_CMD(VSTREAM_MASK_RELEASE),
        &Input,
        sizeof(Input),
        &Output,
        sizeof(Output),
        &nWritten,
        &overlap);
    if (nWritten != sizeof(Output)) return E_FAIL;

    return Output;
}

HRESULT 
SoraURadioMapRxSampleBuf(
    ULONG RadioNo, 
    OUT PVOID *ppUserBuffer,
    OUT PULONG Size)
{

    ULONG nWritten = 0;
    MAP_RX_BUF_OUT Output = { 0 };

	OVERLAPPED overlap = { 0 };
    DeviceIoControl ( 
        SoraDevice, 
        UEXT_CMD(MAP_RX_BUFFER), 
        &RadioNo,
        sizeof(RadioNo),
        &Output,
        sizeof(Output),
        &nWritten,
        &overlap);    
    if (nWritten == 0)
    {
        return E_FAIL;
    }

    if (Output.hResult == S_OK)
    {
        *ppUserBuffer   = Output.RxBuf;
        *Size           = Output.RxBufSize;
    }
    else
    {
        *ppUserBuffer   = NULL;
        *Size           = 0;
    }
    return Output.hResult;
}

HRESULT 
SoraURadioUnmapRxSampleBuf(
    IN ULONG RadioNo, 
    IN PVOID UserBuffer)
{
    ULONG nWritten = 0;
    HRESULT hr;
    UNMAP_RX_BUF_IN Input = { 0 };
    Input.RadioNo = RadioNo;
    Input.UserBuffer = UserBuffer;

	OVERLAPPED overlap = { 0 };
    DeviceIoControl ( 
        SoraDevice, 
        UEXT_CMD(UNMAP_RX_BUFFER),
        &Input,
        sizeof(Input),
        &hr,
        sizeof(hr),
        &nWritten,
        &overlap);    
    if (nWritten == 0)
    {
       return E_FAIL;
    }
    return hr;
}

HRESULT __IoAcquireLock(IN ULONG RadioNo, ULONG IoCode)
{
    HRESULT hr = S_OK;
    ULONG nWritten = 0;

	OVERLAPPED overlap = { 0 };
    DeviceIoControl ( 
        SoraDevice, IoCode, &RadioNo, sizeof(RadioNo),
        &hr, sizeof(hr), &nWritten, &overlap);    
    if (nWritten == 0)
    {
       return E_FAIL;
    }
    return hr;
}

HRESULT __SetRadioValue(IN ULONG RadioNo, ULONG Value, ULONG IoCode)
{
    HRESULT hr = E_FAIL;
    ULONG nWritten = 0;
    SET_VALUE_IN In;
    In.RadioNo = RadioNo;
    In.u.uValue = Value;

	OVERLAPPED overlap = { 0 };
    DeviceIoControl ( 
        SoraDevice, IoCode, &In, sizeof(In),
        &hr, sizeof(hr), &nWritten, &overlap);    
    if (nWritten == 0)
    {
       return E_FAIL;
    }
    return hr;
}

HRESULT SoraURadioSetRxPA(IN ULONG RadioNo, IN ULONG RxPa)
{
    return __SetRadioValue(RadioNo, RxPa, UEXT_CMD(SET_RX_PA));
}

HRESULT SoraURadioSetRxGain(IN ULONG RadioNo, IN ULONG RxGain)
{
    return __SetRadioValue(RadioNo, RxGain, UEXT_CMD(SET_RX_GAIN));
}

HRESULT SoraURadioSetTxGain(IN ULONG RadioNo, IN ULONG TxGain)
{
    return __SetRadioValue(RadioNo, TxGain, UEXT_CMD(SET_TX_GAIN));
}

HRESULT SoraURadioSetCentralFreq(IN ULONG RadioNo, IN ULONG KHz)
{
    return __SetRadioValue(RadioNo, KHz, UEXT_CMD(SET_CEN_FREQ));
}

HRESULT SoraURadioSetFreqOffset(IN ULONG RadioNo, IN LONG Hz)
{
    return __SetRadioValue(RadioNo, (ULONG)Hz, UEXT_CMD(SET_FREQ_OFF));
}

HRESULT SoraURadioSetSampleRate(IN ULONG RadioNo, ULONG MHz)
{
    return __SetRadioValue(RadioNo, (ULONG)MHz, UEXT_CMD(SET_SAMPLE_RATE));
}

HRESULT SoraURadioStart(IN ULONG RadioNo)
{
    HRESULT hr = E_FAIL;
    ULONG nWritten = 0;

	OVERLAPPED overlap = { 0 };
	RADIO_START_IN radio_start_in;
	radio_start_in.RadioNo = RadioNo;
    if (!DeviceIoControl ( 
        SoraDevice, UEXT_CMD(RADIO_START), &radio_start_in, sizeof(RADIO_START_IN),
        &hr, sizeof(hr), &nWritten, &overlap))
        return GetLastError();

    return hr;
}


VOID __IoReleaseLock(IN ULONG RadioNo, ULONG IoCode)
{
    ULONG nWritten = 0;

	OVERLAPPED overlap = { 0 };
    DeviceIoControl ( 
        SoraDevice, IoCode, &RadioNo, sizeof(RadioNo),
        NULL, 0, &nWritten, &overlap);    
}

HRESULT SoraUAcquireRxLock(IN ULONG RadioNo)
{
	return S_OK;
    //return __IoAcquireLock(RadioNo, UEXT_CMD(RX_LOCK_ACQUIRE));
}

VOID SoraUReleaseRxLock(IN ULONG RadioNo)
{
	return;
    //__IoReleaseLock(RadioNo, UEXT_CMD(RX_LOCK_RELEASE));
}

HRESULT SoraUAcquireTxLock(IN ULONG RadioNo)
{
	return S_OK;
    //return __IoAcquireLock(RadioNo, UEXT_CMD(TX_LOCK_ACQUIRE));
}

VOID SoraUReleaseTxLock(IN ULONG RadioNo)
{
	return;
    //__IoReleaseLock(RadioNo, UEXT_CMD(TX_LOCK_RELEASE));
}

HRESULT SoraUAcquireTxBufLock(ULONG RadioNo)
{
    return __IoAcquireLock(RadioNo, UEXT_CMD(MOD_BUF_LOCK_ACQUIRE));
}

VOID SoraUReleaseTxBufLock(ULONG RadioNo)
{
    __IoReleaseLock(RadioNo, UEXT_CMD(MOD_BUF_LOCK_RELEASE));
}


__inline HRESULT __WaitNewSignals(
                    IN PRX_BLOCK       pScanPoint, 
                    IN USHORT          uRetries,
                    IN ULONG           VStreamMask,
                    OUT FLAG           *fReachEnd
                    );

HRESULT SoraUWriteRadioRegister(ULONG RadioNo, ULONG Addr, ULONG  Value) {

	DWORD length;
	RADIO_REGISTER_IO regio;
	regio.radio = RadioNo;
	regio.addr = Addr;
	regio.value.in = Value;
	regio.hr = E_FAIL;

	OVERLAPPED overlap = { 0 };
	if (!DeviceIoControl(SoraDevice, 
						UEXT_CMD(WRITE_RADIO_REG), 
						&regio, 
						sizeof(RADIO_REGISTER_IO),
						&regio,
						sizeof(RADIO_REGISTER_IO),
						&length,
						&overlap))
		return GetLastError();

	return regio.hr;
}

HRESULT SoraUReadRadioRegister(ULONG RadioNo, ULONG Addr, ULONG* Value) {

	DWORD length;
	RADIO_REGISTER_IO regio;
	regio.radio = RadioNo;
	regio.addr = Addr;
	regio.value.in = 0;
	regio.hr = E_FAIL;
	
	OVERLAPPED overlap = { 0 };
	if (!DeviceIoControl(SoraDevice, 
						UEXT_CMD(READ_RADIO_REG), 
						&regio, 
						sizeof(RADIO_REGISTER_IO),
						&regio,
						sizeof(RADIO_REGISTER_IO),
						&length,
						&overlap))
		return GetLastError();

	if (Value)
	   *Value = regio.value.out;

	return regio.hr;	
}

HRESULT SoraUIndicateRxPacket(UCHAR* Buffer, ULONG BufferLength) {

	HRESULT hr;
	DWORD length;

	OVERLAPPED overlap = { 0 };
	if (!DeviceIoControl(SoraDevice, 
						UEXT_CMD(INDICATE_PACKET), 
						Buffer,
						BufferLength,
						&hr,
						sizeof(HRESULT),
						&length,
						&overlap))
		return GetLastError();

	return hr;
}

HANDLE SoraUThreadAlloc() {

	return SoraThreadAlloc();
}

VOID SoraUThreadFree(HANDLE Thread) {

	return SoraThreadFree(Thread);
}

BOOLEAN SoraUThreadStart(HANDLE Thread, PSORA_UTHREAD_PROC User_Routine, PVOID User_Context) {

	return SoraThreadStart(Thread, User_Routine, User_Context);
}

BOOLEAN SoraUThreadStartEx(HANDLE Thread, PSORA_UTHREAD_PROC* User_Routine, PVOID* User_Context, ULONG Count, SCHEDULING_TYPE Type) {

	return SoraThreadStartEx(Thread, User_Routine, User_Context, Count, Type);
}

VOID SoraUThreadStop(HANDLE Thread) {

	SoraThreadStop(Thread);
}

PVOID SoraUAllocBuffer(ULONG Size) {

	ALLOC_KERNEL_BUFFER_IN alloc_kb_in;
	ALLOC_KERNEL_BUFFER_OUT alloc_kb_out = { 0 };
	DWORD length;
	alloc_kb_in.Size = Size;
	alloc_kb_in.force_Continuous = 0;

	OVERLAPPED overlap = { 0 };
	if (!DeviceIoControl(SoraDevice, 
						UEXT_CMD(ALLOC_KERNEL_BUFFER), 
						&alloc_kb_in,
						sizeof(ALLOC_KERNEL_BUFFER_IN),
						&alloc_kb_out,
						sizeof(ALLOC_KERNEL_BUFFER_OUT),
						&length,
						&overlap))
		return NULL;
	
	if (alloc_kb_out.Buff) {		
		struct KernelBufferEntry* be;
		be = new struct KernelBufferEntry;
		be->AllocKBOut.Buff = alloc_kb_out.Buff;
		be->AllocKBOut.HiPhyBuff = alloc_kb_out.HiPhyBuff;
		be->AllocKBOut.LoPhyBuff = alloc_kb_out.LoPhyBuff;
		be->AllocKBOut.is_Continuous = alloc_kb_out.is_Continuous;
		be->Size = Size;
		LockList(&LockLong);
		InsertHeadList(&AllocBufferListHead,
			be);
		UnlockList(&LockLong);
	}
	return alloc_kb_out.Buff;
}

KernelBufferEntry* SoraUForceAllocContinuousBufferEx(ULONG Size) {

	ALLOC_KERNEL_BUFFER_IN alloc_kb_in;
	ALLOC_KERNEL_BUFFER_OUT alloc_kb_out = { 0 };
	DWORD length;
	alloc_kb_in.Size = Size;
	alloc_kb_in.force_Continuous = 1;

	OVERLAPPED overlap = { 0 };
	if (!DeviceIoControl(SoraDevice, 
						UEXT_CMD(ALLOC_KERNEL_BUFFER), 
						&alloc_kb_in,
						sizeof(ALLOC_KERNEL_BUFFER_IN),
						&alloc_kb_out,
						sizeof(ALLOC_KERNEL_BUFFER_OUT),
						&length,
						&overlap))
		return NULL;

	if (alloc_kb_out.Buff) {
		struct KernelBufferEntry* be;
		be = new struct KernelBufferEntry;
		be->AllocKBOut.Buff = alloc_kb_out.Buff;
		be->AllocKBOut.HiPhyBuff = alloc_kb_out.HiPhyBuff;
		be->AllocKBOut.LoPhyBuff = alloc_kb_out.LoPhyBuff;
		be->AllocKBOut.is_Continuous = alloc_kb_out.is_Continuous;
		be->Size = Size;		
		LockList(&LockLong);
		InsertHeadList(&AllocBufferListHead,
			be);
		UnlockList(&LockLong);
		return be;
	}
	return NULL;
}

bool SoraUForceAllocContinuousBuffer(ULONG size, void** addr, unsigned long* hi_phy_addr, unsigned long* lo_phy_addr) {

	KernelBufferEntry* kbentry;
	kbentry = SoraUForceAllocContinuousBufferEx(size);
	if (kbentry) {
		if ( addr)
			*addr = kbentry->AllocKBOut.Buff;
		if ( hi_phy_addr)
			*hi_phy_addr = kbentry->AllocKBOut.HiPhyBuff;
		if ( lo_phy_addr)
			*lo_phy_addr = kbentry->AllocKBOut.LoPhyBuff;
		return true;
	}
	return false;
}

PVOID SoraUForceAllocContinuousBuffer(ULONG Size) {

	KernelBufferEntry* be = SoraUForceAllocContinuousBufferEx(Size);
	if (be)
		return be->AllocKBOut.Buff;
	return NULL;
		
}

VOID SoraUReleaseBuffer(PVOID Buff) {

	RELEASE_KERNEL_BUFFER_IN release_kb_in = { 0 };
	DWORD length;
	release_kb_in.Buff = Buff;

	OVERLAPPED overlap = { 0 };
	DeviceIoControl(SoraDevice, 
					UEXT_CMD(RELEASE_KERNEL_BUFFER), 
					&release_kb_in,
					sizeof(RELEASE_KERNEL_BUFFER_IN),
					NULL,
					0,
					&length,
					&overlap);

	LockList(&LockLong);
	LIST_ENTRY* entry;
	entry = AllocBufferListHead.Flink;
	while(entry != &AllocBufferListHead) {
		struct KernelBufferEntry* be;
		be = (struct KernelBufferEntry*)entry;
		if (be->AllocKBOut.Buff != Buff)
			entry = entry->Flink;
		else {
			RemoveEntryList(entry);
			delete be;
			break;
		}
	}
	UnlockList(&LockLong);
}

HRESULT SoraUGetTxPacket(PACKET_HANDLE* Packet, VOID** Addr, UINT* Length, DWORD Timeout) {

	*Packet = NULL;
	*Addr = NULL;
	*Length = NULL;
	if (is_sora_valid) {
		HANDLE handles[2];
		handles[0] = SoraExitSendPacket;
		handles[1] = SoraQueuedSendPacket;
		switch(WaitForMultipleObjects(2,
			handles,
			FALSE,
			Timeout)) {
		case WAIT_OBJECT_0:
			return ERROR_CANCELLED;
			break;			
		case WAIT_OBJECT_1: {
				DWORD length;
				GET_SEND_PACKET_OUT out = { 0 };

				OVERLAPPED overlap = { 0 };
				if (!DeviceIoControl(SoraDevice, 
					UEXT_CMD(GET_SEND_PACKET),
					NULL,
					0,
					&out,
					sizeof(GET_SEND_PACKET_OUT),
					&length,
					&overlap))
					return GetLastError();
			
				if (SUCCEEDED(out.hResult)) {
					*Packet = out.Packet;
					*Addr = out.Addr;
					*Length = out.Size;
				}
				return out.hResult;
			}
			break;
		case WAIT_TIMEOUT:
			return ERROR_TIMEOUT;
			break;
		default:
			return GetLastError();
			break;
		}
	}
	return ERROR_INVALID_HANDLE;
}

HRESULT SoraUCompleteTxPacket(PACKET_HANDLE Packet, HRESULT hResult) {

	if (is_sora_valid) {
		HRESULT hr;
		DWORD length;
		COMPLETE_SEND_PACKET_IN in = { 0 };	
		in.Packet = Packet;
		in.hResult = hResult;

		OVERLAPPED overlap = { 0 };
		if (!DeviceIoControl(SoraDevice, 
							UEXT_CMD(COMPLETE_SEND_PACKET),
							&in,
							sizeof(COMPLETE_SEND_PACKET_IN),
							&hr,
							sizeof(HRESULT),
							&length,
							&overlap))
			return GetLastError();

		return hr;
	}
	return ERROR_INVALID_HANDLE;
}

HRESULT SoraUEnableGetTxPacket() {

	if (is_sora_valid) {
		HRESULT hr;
		DWORD length;
		ENABLE_GET_SEND_PACKET_IN in = { 0 };
		in.Semaph = SoraQueuedSendPacket;

		OVERLAPPED overlap = { 0 };
		if (!DeviceIoControl(SoraDevice,
			UEXT_CMD(ENABLE_GET_SEND_PACKET),
			&in,
			sizeof(ENABLE_GET_SEND_PACKET_IN),
			&hr,
			sizeof(HRESULT),
			&length,
			&overlap))
			return GetLastError();

		if (SUCCEEDED(hr))
			ResetEvent(SoraExitSendPacket);
		
		return hr;
	}
	return ERROR_INVALID_HANDLE;
}

HRESULT SoraUDisableGetTxPacket() {

	if (is_sora_valid) {
		SetEvent(SoraExitSendPacket);
		return S_OK;
	}
	return ERROR_INVALID_HANDLE;
}
