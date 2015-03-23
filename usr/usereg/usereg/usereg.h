#pragma once

#include <Windows.h>

typedef unsigned __int32	REG32ADDR;
typedef unsigned __int32	REG32VAL;
typedef LARGE_INTEGER		TRANSID;
typedef unsigned __int32	EVENTS;

#define EVENT_TRANSFER_INIT			(1 << 0x00000000)
#define EVENT_TRANSFER_DONE			(1 << 0x00000001)
#define EVENT_TX_INIT				(1 << 0x00000002)
#define EVENT_TX_DONE				(1 << 0x00000003)
#define EVENT_RX_INIT				(1 << 0x00000004)
#define EVENT_RX_STOP				(1 << 0x00000005)
#define EVENT_ALL					EVENT_TRANSFER_INIT | EVENT_TRANSFER_DONE | EVENT_TX_INIT | EVENT_TX_DONE | EVENT_RX_INIT | EVENT_RX_STOP

typedef void __stdcall EVENT_HANDLER(EVENTS, void*);

class CSoraPHYMem {

public:
	__declspec(dllexport) static CSoraPHYMem* AllocSoraPHYMem();
	__declspec(dllexport) static void FreeSoraPHYMem(CSoraPHYMem*);
	virtual bool Alloc(ULONG) = 0;
	virtual void Free() = 0;
	virtual void* GetAddr() = 0;
	virtual unsigned long GetSize() = 0;
	virtual unsigned long GetHiPhyAddr() = 0;
	virtual unsigned long GetLoPhyAddr() = 0;
};

class CSoraRCBMem {

public:
	__declspec(dllexport) static void FreeSoraRCBMem(CSoraRCBMem*);
	virtual bool Acquire(ULONG) = 0;
	virtual void Release() = 0;
	virtual unsigned long GetAddr() = 0;
	virtual unsigned long GetSize() = 0;
	virtual void* GetTxDesc() = 0;
	virtual HRESULT Transfer(CSoraPHYMem*, ULONG, ULONG) = 0;

	virtual HRESULT TransferAsync(CSoraPHYMem*, ULONG, ULONG) = 0;
	virtual HRESULT WaitTransferDone(ULONG) = 0;
	virtual void TransferDone() = 0;
};

class CSoraRadio {

public:
	__declspec(dllexport) static void FreeSoraRadio(CSoraRadio*);
	virtual void* GetMappingAddr() = 0;	
	virtual REG32VAL GetRadioStatus() = 0;
	virtual REG32VAL GetRadioID() = 0;
	virtual REG32VAL GetRoundTripLatency() = 0;
	virtual REG32VAL GetTransferSrcAddrL() = 0;
	virtual REG32VAL GetTransferSrcAddrH() = 0;
	virtual REG32VAL GetTransferDuration() = 0;
	virtual REG32VAL GetTransferControl() = 0;
	virtual REG32VAL GetTransferReset() = 0;
	virtual REG32VAL GetTransferMask() = 0;
	virtual REG32VAL GetTransferErrorCount() = 0;
	virtual REG32VAL GetTransferChecksum() = 0;
	virtual REG32VAL GetTransferCountChecksum() = 0;	
	virtual REG32VAL GetTXAddr() = 0;
	virtual REG32VAL GetTXSize() = 0;
	virtual REG32VAL GetTXControl() = 0;
	virtual REG32VAL GetTXMask() = 0;	
	virtual REG32VAL GetTXErrorCount() = 0;
	virtual REG32VAL GetTXChecksum() = 0;
	virtual REG32VAL GetTXCountChecksum() = 0;
	virtual REG32VAL GetRXBufAddrL() = 0;
	virtual REG32VAL GetRXBufAddrH() = 0;
	virtual REG32VAL GetRXBufSize() = 0;
	virtual REG32VAL GetRXControl() = 0;
	virtual REG32VAL GetTXTiming() = 0;

	virtual void SetTransferSrcAddrL(REG32VAL) = 0;
	virtual void SetTransferSrcAddrH(REG32VAL) = 0;
	virtual void SetTransferControl(REG32VAL) = 0;
	virtual void SetTXAddr(REG32VAL) = 0;
	virtual void SetTXSize(REG32VAL) = 0;
	virtual void SetTXControl(REG32VAL) = 0;
	virtual void SetTXMask(REG32VAL) = 0;
	virtual void SetRXBufAddrL(REG32VAL) = 0;
	virtual void SetRXBufAddrH(REG32VAL) = 0;
	virtual void SetRXBufSize(REG32VAL) = 0;
	virtual void SetRXControl(REG32VAL) = 0;
	virtual void SetTXTiming(REG32VAL) = 0;

	virtual HRESULT Start() = 0;
	virtual HRESULT TransferEx(VOID*, ULONG, TRANSID*) = 0;
	virtual HRESULT Tx(TRANSID) = 0;
	virtual HRESULT TxFree(TRANSID) = 0;	
	virtual HRESULT MapRxSampleBuf(VOID**, ULONG*) = 0;
	virtual HRESULT UnmapRxSampleBuf(VOID**) = 0;
	virtual HRESULT GetRxPA(REG32VAL*) = 0;
	virtual HRESULT GetRxGain(REG32VAL*) = 0;
	virtual HRESULT GetTxGain(REG32VAL*) = 0;
	virtual HRESULT GetCentralFreq(REG32VAL*) = 0;
	virtual HRESULT GetFreqOffset(REG32VAL*) = 0;
	virtual HRESULT GetSampleRate(REG32VAL*) = 0;
	virtual HRESULT GetClock(REG32VAL*) = 0;
	virtual HRESULT SetRxPA(REG32VAL) = 0;
	virtual HRESULT SetRxGain(REG32VAL) = 0;
	virtual HRESULT SetTxGain(REG32VAL) = 0;
	virtual HRESULT SetCentralFreq(REG32VAL) = 0;
	virtual HRESULT SetFreqOffset(REG32VAL) = 0;
	virtual HRESULT SetSampleRate(REG32VAL) = 0;
	virtual HRESULT WriteRadioRegister(REG32ADDR, REG32VAL) = 0;
	virtual HRESULT ReadRadioRegister(REG32ADDR, REG32VAL*) = 0;	

	virtual HRESULT TxAsync(TRANSID) = 0;
	virtual HRESULT WaitTxDone(ULONG) = 0;
	virtual void TxDone() = 0;

	virtual bool EnableEvent(EVENTS, EVENT_HANDLER*, void*) = 0;
	virtual void DisableEvent(EVENTS) = 0;
};

class CSoraRegister {

public:
	__declspec(dllexport) static CSoraRegister* AllocSoraRegister();
	__declspec(dllexport) static void FreeSoraRegister(CSoraRegister*);
	virtual bool Map() = 0;
	virtual void Unmap() = 0;	
	virtual CSoraRadio* AllocSoraRadio(int) = 0;
	virtual void* GetMappingAddr() = 0;
	virtual REG32VAL GetFirmwareVersion() = 0;
	virtual REG32VAL GetHWStatus() = 0;
	virtual REG32VAL GetLinkStatus() = 0;
	virtual REG32VAL GetLinkControl() = 0;
	virtual REG32VAL GetHWControl() = 0;
	virtual REG32VAL GetPCIeTXBurstSize() = 0;
	virtual REG32VAL GetPCIeRXBlockSize() = 0;	

	virtual CSoraRCBMem* AllocSoraRCBMem() = 0;
};