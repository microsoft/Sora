#pragma once

#include "usereg.h"
#include <sora.h>
#include <_private_ext_u.h>
#include <__reg_file.h>
#include <_radio_manager.h>

struct MAP_MEM_U_EX {

	union {
		MAP_MEM_U m_mmemu;
		struct {
			RES_HANDLE_U m_res_handle_u;
			union {
				__SORA_REGISTERS* m_addr;
				unsigned __int64  m_x64_wrapper;
			};
			ULONG m_size;
			unsigned long m_hi_phy_addr;
			unsigned long m_lo_phy_addr;
		};
	};
};

struct EVENT_DESCRIPTOR : public LIST_ENTRY {

	EVENTS m_evts;
	EVENT_HANDLER* m_handler;
	void* m_context;
};

struct EVENT_TABLE {

	EVENTS m_evts;
	union {
		EVENT_DESCRIPTOR m_descriptor[1];
		struct {
			EVENT_DESCRIPTOR m_event_transfer_init;
			EVENT_DESCRIPTOR m_event_transfer_done;
			EVENT_DESCRIPTOR m_event_tx_init;
			EVENT_DESCRIPTOR m_event_tx_done;
			EVENT_DESCRIPTOR m_event_rx_init;
			EVENT_DESCRIPTOR m_event_rx_stop;
		};
	};
};

class CSoraPHYMemImp : public CSoraPHYMem {

	friend class CSoraRCBMemImp;

public:
	CSoraPHYMemImp();
	virtual ~CSoraPHYMemImp();
	virtual bool Alloc(ULONG);
	virtual void Free();
	virtual void* GetAddr();
	virtual unsigned long GetSize();
	virtual unsigned long GetHiPhyAddr();
	virtual unsigned long GetLoPhyAddr();

protected:
	MAP_MEM_U m_mphymu;
};

class CSoraRCBMemImp : public CSoraRCBMem {

public:
	CSoraRCBMemImp(__SORA_REGISTERS*);
	virtual ~CSoraRCBMemImp();
	virtual bool Acquire(ULONG);
	virtual void Release();
	virtual unsigned long GetAddr();
	virtual unsigned long GetSize();
	virtual void* GetTxDesc();
	virtual HRESULT Transfer(CSoraPHYMem*, ULONG, ULONG);

	virtual HRESULT TransferAsync(CSoraPHYMem*, ULONG, ULONG);
	virtual HRESULT WaitTransferDone(ULONG);
	virtual void TransferDone();

protected:
	__SORA_REGISTERS* m_addr;
	HANDLE m_device;	
	ACQ_RCB_MEM_U m_arcbmu;
};

class CSoraRadioImp : public CSoraRadio {

	static unsigned Dispatch(void*);
	static unsigned Monitor(void*);

public:
	CSoraRadioImp(int, __SORA_RADIO_REGS*);
	virtual ~CSoraRadioImp();
	virtual void* GetMappingAddr();
	virtual REG32VAL GetRadioStatus();
	virtual REG32VAL GetRadioID();
	virtual REG32VAL GetRoundTripLatency();
	virtual REG32VAL GetTransferSrcAddrL();
	virtual REG32VAL GetTransferSrcAddrH();
	virtual REG32VAL GetTransferDuration();
	virtual REG32VAL GetTransferControl();
	virtual REG32VAL GetTransferReset();
	virtual REG32VAL GetTransferMask();
	virtual REG32VAL GetTransferErrorCount();
	virtual REG32VAL GetTransferChecksum();
	virtual REG32VAL GetTransferCountChecksum();	
	virtual REG32VAL GetTXAddr();
	virtual REG32VAL GetTXSize();
	virtual REG32VAL GetTXControl();
	virtual REG32VAL GetTXMask();	
	virtual REG32VAL GetTXErrorCount();
	virtual REG32VAL GetTXChecksum();
	virtual REG32VAL GetTXCountChecksum();
	virtual REG32VAL GetRXBufAddrL();
	virtual REG32VAL GetRXBufAddrH();
	virtual REG32VAL GetRXBufSize();
	virtual REG32VAL GetRXControl();
	virtual REG32VAL GetTXTiming();

	virtual void SetTransferSrcAddrL(REG32VAL);
	virtual void SetTransferSrcAddrH(REG32VAL);
	virtual void SetTransferControl(REG32VAL);
	virtual void SetTXAddr(REG32VAL);
	virtual void SetTXSize(REG32VAL);
	virtual void SetTXControl(REG32VAL);
	virtual void SetTXMask(REG32VAL);
	virtual void SetRXBufAddrL(REG32VAL);
	virtual void SetRXBufAddrH(REG32VAL);
	virtual void SetRXBufSize(REG32VAL);
	virtual void SetRXControl(REG32VAL);
	virtual void SetTXTiming(REG32VAL);

	virtual HRESULT Start();
	virtual HRESULT TransferEx(VOID*, ULONG, TRANSID*);
	virtual HRESULT Tx(TRANSID);
	virtual HRESULT TxFree(TRANSID);	
	virtual HRESULT MapRxSampleBuf(VOID**, ULONG*);
	virtual HRESULT UnmapRxSampleBuf(VOID**);
	virtual HRESULT GetRxPA(REG32VAL*);
	virtual HRESULT GetRxGain(REG32VAL*);
	virtual HRESULT GetTxGain(REG32VAL*);
	virtual HRESULT GetCentralFreq(REG32VAL*);
	virtual HRESULT GetFreqOffset(REG32VAL*);
	virtual HRESULT GetSampleRate(REG32VAL*);
	virtual HRESULT GetClock(REG32VAL*);
	virtual HRESULT SetRxPA(REG32VAL);
	virtual HRESULT SetRxGain(REG32VAL);
	virtual HRESULT SetTxGain(REG32VAL);
	virtual HRESULT SetCentralFreq(REG32VAL);
	virtual HRESULT SetFreqOffset(REG32VAL);
	virtual HRESULT SetSampleRate(REG32VAL);

	virtual HRESULT WriteRadioRegister(REG32ADDR, REG32VAL);
	virtual HRESULT ReadRadioRegister(REG32ADDR, REG32VAL*);

	virtual HRESULT TxAsync(TRANSID);
	virtual HRESULT WaitTxDone(ULONG);
	virtual void TxDone();

	virtual bool EnableEvent(EVENTS, EVENT_HANDLER*, void*);
	virtual void DisableEvent(EVENTS);
	virtual void FireEvent(EVENTS);

protected:
	int m_id;
	__SORA_RADIO_REGS* m_addr;
	EVENT_TABLE m_event_table;
	HANDLE m_event_sync;
	HANDLE m_event_ctrl;
	LIST_ENTRY m_event_list;
	HANDLE m_dispatch;
	HANDLE m_monitor;
};

class CSoraRegisterImp : public CSoraRegister {

public:
	CSoraRegisterImp();
	virtual ~CSoraRegisterImp();
	virtual bool Map();
	virtual void Unmap();
	virtual CSoraRadio* AllocSoraRadio(int i);
	virtual void* GetMappingAddr();
	virtual REG32VAL GetFirmwareVersion();
	virtual REG32VAL GetHWStatus();
	virtual REG32VAL GetLinkStatus();
	virtual REG32VAL GetLinkControl();
	virtual REG32VAL GetHWControl();
	virtual REG32VAL GetPCIeTXBurstSize();
	virtual REG32VAL GetPCIeRXBlockSize();

	virtual CSoraRCBMem* AllocSoraRCBMem();

protected:
	HANDLE m_device;
	MAP_MEM_U_EX m_mreguex;
};