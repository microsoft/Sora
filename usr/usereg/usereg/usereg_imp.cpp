#include "usereg_imp.h"
#include "usereg_evt.h"
#include "__tx_res.h"
#include <process.h>

#define remove_head_list(list)	(IsListEmpty(list) ? NULL : RemoveHeadList(list))

unsigned 
CSoraRadioImp::Dispatch(void* context) {

	CSoraRadioImp* rad;
	rad = (CSoraRadioImp*)context;
	while(1) {
		WaitForSingleObject(rad->m_event_ctrl, INFINITE);
		WaitForSingleObject(rad->m_event_sync, INFINITE);
		EVENT_DESCRIPTOR* descriptor;
		descriptor = (EVENT_DESCRIPTOR*)remove_head_list(&rad->m_event_list);
		ReleaseMutex(rad->m_event_sync);
		if (descriptor) {
			if (descriptor->m_evts) {
				if (descriptor->m_handler)
					descriptor->m_handler(descriptor->m_evts, descriptor->m_context);
				delete descriptor;
				continue;
			}
			delete descriptor;
			break;
		}
	}
	return 0;
}

unsigned 
CSoraRadioImp::Monitor(void* context) {

	CSoraRadioImp* rad;
	rad = (CSoraRadioImp*)context;
	__SORA_RADIO_REGS* reg;
	reg = (__SORA_RADIO_REGS*)rad->GetMappingAddr();
	EVENTS evts;
	transfer_state_obj transfer;
	tx_state_obj tx;
	rx_state_obj rx;
	__REG32_TRANS_CTRL __reg32_trans_ctrl;
	__reg32_trans_ctrl.Value = 0;
	__reg32_trans_ctrl.Bits.TransferDone = 1;
	__REG32_TX_CTRL __reg32_tx_ctrl;
	__reg32_tx_ctrl.Value = 0;
	__reg32_tx_ctrl.Bits.TXDone = 1;
	while(rad->m_event_table.m_evts) {
		evts = transfer.update(reg->TransferControl.Value);
		if (evts)
			rad->FireEvent(evts);
		evts = tx.update(reg->TXControl.Value);
		if (evts)
			rad->FireEvent(evts);
		evts = rx.update(reg->RXControl.Value);
		if (evts)
			rad->FireEvent(evts);
	}
	return 0;
}

CSoraPHYMemImp::CSoraPHYMemImp() {

	memset(&m_mphymu, 0, sizeof(MAP_MEM_U));
}

CSoraPHYMemImp::~CSoraPHYMemImp() {

	Free();
}

inline
bool 
CSoraPHYMemImp::Alloc(ULONG size) {

	extern 
	bool 
	SoraUForceAllocContinuousBuffer(
		unsigned long size, 
		void** addr, 
		unsigned long* hi_phy_addr, 
		unsigned long* lo_phy_addr);
	
	Free();
	if (SoraUForceAllocContinuousBuffer(size, &m_mphymu.m_addr, &m_mphymu.m_hi_phy_addr, &m_mphymu.m_lo_phy_addr)) {
		m_mphymu.m_size = size;
		return true;
	}
	Free();
	return false;
}

inline
void 
CSoraPHYMemImp::Free() {

	if (m_mphymu.m_addr)
		SoraUReleaseBuffer(m_mphymu.m_addr);
	memset(&m_mphymu, 0, sizeof(MAP_MEM_U));
}

inline
void* 
CSoraPHYMemImp::GetAddr() {

	return m_mphymu.m_addr;
}

inline
unsigned long 
CSoraPHYMemImp::GetSize() {

	return m_mphymu.m_size;
}

inline
unsigned long 
CSoraPHYMemImp::GetHiPhyAddr() {

	return m_mphymu.m_hi_phy_addr;
}

inline
unsigned long 
CSoraPHYMemImp::GetLoPhyAddr() {

	return m_mphymu.m_lo_phy_addr;
}

CSoraRCBMemImp::CSoraRCBMemImp(__SORA_REGISTERS* addr) {

	m_addr = addr;
	m_device = INVALID_HANDLE_VALUE;	
	memset(&m_arcbmu, 0, sizeof(ACQ_RCB_MEM_U));
}

CSoraRCBMemImp::~CSoraRCBMemImp() {

	Release();
}

inline
bool
CSoraRCBMemImp::Acquire(ULONG size) {

	Release();
	m_device = CreateFile(
		"\\\\.\\HwTest", 
		GENERIC_READ|GENERIC_WRITE, 
		0,
		NULL, 
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (m_device != INVALID_HANDLE_VALUE) {
		DWORD r;
		r = 0;		
		ACQ_RCB_MEM_IN arcbmi;
		arcbmi.m_rcb_size = size;
		BOOL b;
		b = DeviceIoControl(
			m_device,
			private_ext(ACQ_RCB_MEM),
			&arcbmi, 
			sizeof(ACQ_RCB_MEM_IN),
			&m_arcbmu,
			sizeof(ACQ_RCB_MEM_U),
			&r, 
			NULL);
		if (b &&
			m_arcbmu.m_rcb_addr &&
			m_arcbmu.m_rcb_size &&
			m_arcbmu.m_tx_desc.m_res_handle_u.m_handle &&
			m_arcbmu.m_tx_desc.m_addr &&
			m_arcbmu.m_tx_desc.m_size)
			return true;
	}
	Release();
	return false;
}

inline
void 
CSoraRCBMemImp::Release() {

	if (m_device != INVALID_HANDLE_VALUE) {
		if (m_arcbmu.m_rcb_addr) {
			HRESULT hr;
			hr = SoraURadioTxFree(0, m_arcbmu.m_rcb_addr);
		}
		if (m_arcbmu.m_tx_desc.m_res_handle_u.m_handle) {
			DWORD r;
			r = 0;
			BOOL b;
			b = DeviceIoControl(
				m_device,
				private_ext(REL_MAP_MEM),
				&m_arcbmu.m_tx_desc.m_res_handle_u, 
				sizeof(RES_HANDLE_U),
				NULL,
				0,
				&r,
				NULL);
		}
		CloseHandle(m_device);
	}
	m_device = INVALID_HANDLE_VALUE;
	memset(&m_arcbmu, 0, sizeof(ACQ_RCB_MEM_U));
}

inline
unsigned long 
CSoraRCBMemImp::GetAddr() {

	return m_arcbmu.m_rcb_addr;
}

inline
unsigned long 
CSoraRCBMemImp::GetSize() {

	return m_arcbmu.m_rcb_size;
}

inline
void*
CSoraRCBMemImp::GetTxDesc() {

	return m_arcbmu.m_tx_desc.m_addr;
}

inline 
HRESULT 
CSoraRCBMemImp::Transfer(CSoraPHYMem* spm, ULONG offset, ULONG size) {

	if (SUCCEEDED(TransferAsync(spm, offset, size))) 
		if (SUCCEEDED(WaitTransferDone(100000))) {
			TransferDone();
			return S_OK;
		}
	TransferDone();
	return E_FAIL;
}

inline
HRESULT 
CSoraRCBMemImp::TransferAsync(CSoraPHYMem* spm, ULONG offset, ULONG size) {

	TransferDone();
	
	if (spm) {
		CSoraPHYMemImp* imp;
		imp = (CSoraPHYMemImp*)spm;
		if (size &&
			size <= imp->GetSize() &&
			size <= m_arcbmu.m_rcb_size - offset) {
			TX_DESC* tx_desc;
			tx_desc = (TX_DESC*)m_arcbmu.m_tx_desc.m_addr;
			tx_desc->__RCBDestAddr = m_arcbmu.m_rcb_addr + offset;
			tx_desc->Size = size;
			tx_desc->__FrameCtrlOwn = 1;			
			tx_desc->__SourceAddressHi = imp->m_mphymu.m_hi_phy_addr;
			tx_desc->__SourceAddressLo = imp->m_mphymu.m_lo_phy_addr;

			__REG32_TRANS_CTRL __reg32_trans_ctrl;
			__reg32_trans_ctrl.Value = 0;
			__reg32_trans_ctrl.Bits.TransferInit = 1;
			m_addr->RadioRegs->TransferSrcAddrH.Value = m_arcbmu.m_tx_desc.m_hi_phy_addr;
			m_addr->RadioRegs->TransferSrcAddrL.Value = m_arcbmu.m_tx_desc.m_lo_phy_addr;
			m_addr->RadioRegs->TransferControl.Value = __reg32_trans_ctrl.Value;

			return S_OK;
		}
	}
	return E_FAIL;
}

inline
HRESULT 
CSoraRCBMemImp::WaitTransferDone(ULONG timeout) {

	for(; timeout; timeout--) {	
		_mm_pause();
		if (m_addr->RadioRegs->TransferControl.Bits.TransferDone)
			return S_OK;
	}
	return E_FAIL;

}

inline
void 
CSoraRCBMemImp::TransferDone() {

	if (m_addr->RadioRegs->TransferControl.Bits.TransferInit)
		m_addr->RadioRegs->TransferControl.Value = 0;
}

CSoraRadioImp::CSoraRadioImp(int i, __SORA_RADIO_REGS* addr) {

	m_id = i;
	m_addr = addr;
	memset(&m_event_table, 0, sizeof(EVENT_TABLE));
	m_event_sync = CreateMutex(NULL, FALSE, NULL);
	m_event_ctrl = CreateSemaphore(NULL, 0, 256, NULL);
	InitializeListHead(&m_event_list);
	m_dispatch = NULL;
	m_monitor = NULL;
}

CSoraRadioImp::~CSoraRadioImp() {

	DisableEvent(EVENT_ALL);
	while(1) {
		EVENT_DESCRIPTOR* descriptor;
		descriptor = (EVENT_DESCRIPTOR*)remove_head_list(&m_event_list);
		if (descriptor) {
			delete descriptor;
			continue;
		}
		break;
	}
	CloseHandle(m_event_sync);
	CloseHandle(m_event_ctrl);	
}

inline
void* 
CSoraRadioImp::GetMappingAddr() {

	return m_addr;
}

inline
REG32VAL 
CSoraRadioImp::GetRadioStatus() {

	return *(REG32VAL*)&m_addr->RadioStatus;
}

inline
REG32VAL
CSoraRadioImp::GetRadioID() {

	return *(REG32VAL*)&m_addr->RadioID;
}

inline
REG32VAL 
CSoraRadioImp::GetRoundTripLatency() {

	return *(REG32VAL*)&m_addr->RoundTripLatency;
}

inline
REG32VAL 
CSoraRadioImp::GetTransferSrcAddrL() {

	return *(REG32VAL*)&m_addr->TransferSrcAddrL;
}

inline
REG32VAL 
CSoraRadioImp::GetTransferSrcAddrH() {

	return *(REG32VAL*)&m_addr->TransferSrcAddrH;
}

inline
REG32VAL 
CSoraRadioImp::GetTransferDuration() {

	return *(REG32VAL*)&m_addr->TransferDuration;
}

inline
REG32VAL 
CSoraRadioImp::GetTransferControl() {

	return *(REG32VAL*)&m_addr->TransferControl;
}

inline
REG32VAL 
CSoraRadioImp::GetTransferReset() {

	return *(REG32VAL*)&m_addr->TransferReset;
}

inline
REG32VAL 
CSoraRadioImp::GetTransferMask() {

	return *(REG32VAL*)&m_addr->TransferMask;
}

inline
REG32VAL 
CSoraRadioImp::GetTransferErrorCount() {

	return *(REG32VAL*)&m_addr->TransferErrorCount;
}

inline
REG32VAL 
CSoraRadioImp::GetTransferChecksum() {

	return *(REG32VAL*)&m_addr->TransferChecksum;
}

inline
REG32VAL
CSoraRadioImp::GetTransferCountChecksum() {

	return *(REG32VAL*)&m_addr->TransferCountChecksum;
}

inline
REG32VAL 
CSoraRadioImp::GetTXAddr() {

	return *(REG32VAL*)&m_addr->TXAddr;
}

inline
REG32VAL 
CSoraRadioImp::GetTXSize() {

	return *(REG32VAL*)&m_addr->TXSize;
}

inline
REG32VAL 
CSoraRadioImp::GetTXControl() {

	return *(REG32VAL*)&m_addr->TXControl;
}

inline
REG32VAL 
CSoraRadioImp::GetTXMask() {

	return m_addr->TXControl.Bits.TXMask;
}

inline
REG32VAL 
CSoraRadioImp::GetTXErrorCount() {

	return *(REG32VAL*)&m_addr->TXErrorCount;
}

inline
REG32VAL 
CSoraRadioImp::GetTXChecksum() {

	return *(REG32VAL*)&m_addr->TXChecksum;
}

inline
REG32VAL 
CSoraRadioImp::GetTXCountChecksum() {

	return *(REG32VAL*)&m_addr->TXCountChecksum;
}

inline
REG32VAL 
CSoraRadioImp::GetRXBufAddrL() {

	return *(REG32VAL*)&m_addr->RXBufAddrL;
}

inline
REG32VAL 
CSoraRadioImp::GetRXBufAddrH() {

	return *(REG32VAL*)&m_addr->RXBufAddrH;
}

inline
REG32VAL 
CSoraRadioImp::GetRXBufSize() {

	return *(REG32VAL*)&m_addr->RXBufSize;
}

inline
REG32VAL
CSoraRadioImp::GetRXControl() {

	return *(REG32VAL*)&m_addr->RXControl;
}

inline
REG32VAL
CSoraRadioImp::GetTXTiming() {

	return *(REG32VAL*)&m_addr->TXTiming;
}

inline
void 
CSoraRadioImp::SetTransferSrcAddrL(REG32VAL val) {

	m_addr->TransferSrcAddrL.Value = val;
}

inline
void
CSoraRadioImp::SetTransferSrcAddrH(REG32VAL val) {

	m_addr->TransferSrcAddrH.Value = val;
}

inline
void
CSoraRadioImp::SetTransferControl(REG32VAL val) {

	m_addr->TransferControl.Value = val;
}

inline
void
CSoraRadioImp::SetTXAddr(REG32VAL val) {

	m_addr->TXAddr.Value = val;
}

inline
void
CSoraRadioImp::SetTXSize(REG32VAL val) {

	m_addr->TXSize.Value = val;
}

inline
void
CSoraRadioImp::SetTXControl(REG32VAL val) {

	m_addr->TXControl.Value = val;
}

inline
void 
CSoraRadioImp::SetTXMask(REG32VAL val) {

	m_addr->TXControl.Bits.TXMask = val;
}

inline
void
CSoraRadioImp::SetRXBufAddrL(REG32VAL val) {

	m_addr->RXBufAddrL.Value = val;
}

inline
void
CSoraRadioImp::SetRXBufAddrH(REG32VAL val) {

	m_addr->RXBufAddrH.Value = val;
}

inline
void
CSoraRadioImp::SetRXBufSize(REG32VAL val) {

	m_addr->RXBufSize.Value = val;
}

inline 
void
CSoraRadioImp::SetRXControl(REG32VAL val) {

	m_addr->RXControl.Value = val;
}

inline 
void
CSoraRadioImp::SetTXTiming(REG32VAL val) {

	m_addr->TXTiming.Value = val;
}

inline 
HRESULT 
CSoraRadioImp::Start() {

	return SoraURadioStart(m_id);
}

inline
HRESULT 
CSoraRadioImp::TransferEx(PVOID buf, ULONG len, TRANSID* tid) {

	tid->HighPart = len;
	return SoraURadioTransferEx(m_id, buf, len, &tid->LowPart);
}

inline
HRESULT 
CSoraRadioImp::Tx(TRANSID tid) {

	if (SUCCEEDED(TxAsync(tid)))
		if (SUCCEEDED(WaitTxDone(100000))) {
			TxDone();
			return S_OK;
		}
	TxDone();
	return E_FAIL;
}

inline
HRESULT 
CSoraRadioImp::TxFree(TRANSID tid) {

	return SoraURadioTxFree(m_id, tid.LowPart);
}

inline
HRESULT 
CSoraRadioImp::MapRxSampleBuf(VOID** buf, ULONG* len) {

	return SoraURadioMapRxSampleBuf(m_id, buf, len);
}

inline
HRESULT 
CSoraRadioImp::UnmapRxSampleBuf(VOID** buf) {

	return SoraURadioUnmapRxSampleBuf(m_id, buf);
}

inline
HRESULT 
CSoraRadioImp::GetRxPA(REG32VAL* val) {

	return ReadRadioRegister(0x11, val);
}

inline
HRESULT 
CSoraRadioImp::GetRxGain(REG32VAL* val) {

	return ReadRadioRegister(0x12, val);
}

inline
HRESULT 
CSoraRadioImp::GetTxGain(REG32VAL* val) {

	return ReadRadioRegister(0x0C, val);
}

inline
HRESULT 
CSoraRadioImp::GetCentralFreq(REG32VAL* val) {

	return ReadRadioRegister(0x07, val);
}

inline
HRESULT 
CSoraRadioImp::GetFreqOffset(REG32VAL* val) {

	return ReadRadioRegister(0x09, val);
}

inline
HRESULT 
CSoraRadioImp::GetSampleRate(REG32VAL* val) {

	return ReadRadioRegister(0x06, val);
}

inline
HRESULT 
CSoraRadioImp::GetClock(REG32VAL* val) {

	return ReadRadioRegister(0x15, val);
}

inline
HRESULT
CSoraRadioImp::SetRxPA(REG32VAL val) {

	return WriteRadioRegister(0x11, val);
}

inline
HRESULT 
CSoraRadioImp::SetRxGain(REG32VAL val) {

	return WriteRadioRegister(0x12, val);
}

inline
HRESULT 
CSoraRadioImp::SetTxGain(REG32VAL val) {

	return WriteRadioRegister(0x0C, val);
}

inline
HRESULT 
CSoraRadioImp::SetCentralFreq(REG32VAL val) {

	return WriteRadioRegister(0x07, val);
}

inline
HRESULT 
CSoraRadioImp::SetFreqOffset(REG32VAL val) {

	return WriteRadioRegister(0x09, val);
}

inline
HRESULT 
CSoraRadioImp::SetSampleRate(REG32VAL val) {

	return WriteRadioRegister(0x06, val);
}

inline
HRESULT
CSoraRadioImp::WriteRadioRegister(REG32ADDR addr, REG32VAL val) {

	__REG32_RF_CMD_ADDR __reg32_rf_cmd_addr;
	__reg32_rf_cmd_addr.Value = 0;
	__reg32_rf_cmd_addr.Bits.Addr = addr;
	__reg32_rf_cmd_addr.Bits.Cmd = 0;

	m_addr->RadioChannelRegs.RFRegValueIn.Value = val;
	m_addr->RadioChannelRegs.RFRegOpInst.Value = __reg32_rf_cmd_addr.Value;

	return S_OK;
}

inline
HRESULT 
CSoraRadioImp::ReadRadioRegister(REG32ADDR addr, REG32VAL* val) {

	__REG32_RF_CMD_ADDR __reg32_rf_cmd_addr;
	__reg32_rf_cmd_addr.Value = 0;
	__reg32_rf_cmd_addr.Bits.Addr = addr;
	__reg32_rf_cmd_addr.Bits.Cmd = 1;

	m_addr->RadioChannelRegs.RFRegOpInst.Value = __reg32_rf_cmd_addr.Value;

	for(int timeout=100; timeout; timeout--) {
		_mm_pause();
		if (m_addr->RadioChannelRegs.RFRegOpInst.Bits.RdDone) {
			*val = m_addr->RadioChannelRegs.RFRegValueOut.Value;
			return S_OK;
		}		
	}
	return E_FAIL;
}

inline
HRESULT
CSoraRadioImp::TxAsync(TRANSID tid) {

	TxDone();

	__REG32_TX_CTRL __reg32_tx_ctrl;
	__reg32_tx_ctrl.Value = 0;
	__reg32_tx_ctrl.Bits.TXInit = 1;
	__reg32_tx_ctrl.Bits.TXMask = m_addr->TXControl.Bits.TXMask;

	m_addr->TXAddr.Value = tid.LowPart;
	m_addr->TXSize.Value = tid.HighPart;
	m_addr->TXControl.Value = __reg32_tx_ctrl.Value;

	return S_OK;
}

inline 
HRESULT 
CSoraRadioImp::WaitTxDone(ULONG timeout) {

	for(; timeout; timeout--) {	
		_mm_pause();
		if (m_addr->TXControl.Bits.TXDone)
			return S_OK;
	}
	return E_FAIL;
}

inline
void
CSoraRadioImp::TxDone() {

	if (m_addr->TXControl.Bits.TXInit)
		m_addr->TXControl.Bits.TXInit = 0;

	if (m_addr->TXControl.Bits.TXDone)
		m_addr->TXControl.Bits.TXDone = 0;
}

inline
bool 
CSoraRadioImp::EnableEvent(EVENTS evts, EVENT_HANDLER* handler, void* context) {

	if (handler) {
		evts &= EVENT_ALL;
		while(evts) {
			unsigned long i;
			if (BitScanReverse(&i, evts)) {					
				m_event_table.m_descriptor[i].m_evts = (1 << i);
				m_event_table.m_descriptor[i].m_context = context;
				m_event_table.m_descriptor[i].m_handler = handler;				
				m_event_table.m_evts |= (1 << i);
				evts &= ~(1 << i);
			}
		}
	}
	if (m_event_table.m_event_transfer_init.m_handler ||
		m_event_table.m_event_transfer_done.m_handler ||
		m_event_table.m_event_tx_init.m_handler ||
		m_event_table.m_event_tx_done.m_handler ||
		m_event_table.m_event_rx_init.m_handler ||
		m_event_table.m_event_rx_stop.m_handler) {
		if (!m_monitor) {
			m_monitor = (HANDLE)_beginthreadex(
				NULL, 
				0, 
				Monitor, 
				this, 
				0, 
				NULL);
			if (!m_monitor)
				goto error_exit;
		}
		if (!m_dispatch) {
			m_dispatch = (HANDLE)_beginthreadex(
				NULL, 
				0, 
				Dispatch, 
				this, 
				0, 
				NULL);
			if (!m_dispatch)
				goto error_exit;
		}
		return true;
	}
error_exit:
	DisableEvent(EVENT_ALL);
	return false;
}

inline
void 
CSoraRadioImp::DisableEvent(EVENTS evts) {

	evts &= EVENT_ALL;
	while (evts) {
		unsigned long i;
		if (BitScanReverse(&i, evts)) {
			m_event_table.m_descriptor[i].m_evts = (1 << i);
			m_event_table.m_descriptor[i].m_handler = NULL;
			m_event_table.m_descriptor[i].m_context = NULL;
			m_event_table.m_evts &= ~(1 << i);
			evts &= ~(1 << i);
		}
	}
	if (m_event_table.m_event_transfer_init.m_handler ||
		m_event_table.m_event_transfer_done.m_handler ||
		m_event_table.m_event_tx_init.m_handler ||
		m_event_table.m_event_tx_done.m_handler ||
		m_event_table.m_event_rx_init.m_handler ||
		m_event_table.m_event_rx_stop.m_handler)
		return;
	if (m_monitor) {
		WaitForSingleObject(m_monitor, INFINITE);
		CloseHandle(m_monitor);
		m_monitor = NULL;
	}
	if (m_dispatch) {
		EVENT_DESCRIPTOR* descriptor;
		descriptor = new EVENT_DESCRIPTOR;
		descriptor->m_evts = 0;
		descriptor->m_handler = NULL;
		descriptor->m_context = NULL;
		WaitForSingleObject(m_event_sync, INFINITE);
		if (!ReleaseSemaphore(m_event_ctrl, 1, NULL)) {
			EVENT_DESCRIPTOR* descriptor;
			descriptor = (EVENT_DESCRIPTOR*)remove_head_list(&m_event_list);
			if (descriptor)
				delete descriptor;
		}
		InsertHeadList(&m_event_list, descriptor);
		ReleaseMutex(m_event_sync);
		WaitForSingleObject(m_dispatch, INFINITE);
		CloseHandle(m_dispatch);
		m_dispatch = NULL;
	}
	while(1)
		if (WaitForSingleObject(m_event_ctrl, 0) != WAIT_OBJECT_0)
			break;
	while(1) {
		EVENT_DESCRIPTOR* descriptor;
		descriptor = (EVENT_DESCRIPTOR*)remove_head_list(&m_event_list);
		if (descriptor) {
			delete descriptor;
			continue;
		}
		break;
	}
}

inline
void 
CSoraRadioImp::FireEvent(EVENTS evts) {

	unsigned long i;
	if (BitScanReverse(&i, evts))
		if (m_event_table.m_descriptor[i].m_handler) {
			EVENT_DESCRIPTOR* descriptor;
			descriptor = new EVENT_DESCRIPTOR;
			descriptor->m_evts = m_event_table.m_descriptor[i].m_evts;
			descriptor->m_handler = m_event_table.m_descriptor[i].m_handler;
			descriptor->m_context = m_event_table.m_descriptor[i].m_context;
			WaitForSingleObject(m_event_sync, INFINITE);
			if (!ReleaseSemaphore(m_event_ctrl, 1, NULL)) {
				EVENT_DESCRIPTOR* descriptor;
				descriptor = (EVENT_DESCRIPTOR*)remove_head_list(&m_event_list);
				if (descriptor)
					delete descriptor;
			}
			InsertTailList(&m_event_list, descriptor);
			ReleaseMutex(m_event_sync);
		}
}

CSoraRegisterImp::CSoraRegisterImp() {

	m_device = INVALID_HANDLE_VALUE;
	memset(&m_mreguex, 0, sizeof(MAP_MEM_U_EX));
}

CSoraRegisterImp::~CSoraRegisterImp() {

	Unmap();
}

inline
bool 
CSoraRegisterImp::Map() {
	
	Unmap();
	m_device = CreateFile(
		SORA_DEVICE_UNAME, 
		GENERIC_READ|GENERIC_WRITE, 
		0,
		NULL, 
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (m_device != INVALID_HANDLE_VALUE) {
		DWORD r;
		r = 0;
		BOOL b;
		b = DeviceIoControl(
			m_device,
			__IOCTL_MAP_SORA_REGISTER,
			NULL, 
			0,
			&m_mreguex,
			sizeof(MAP_MEM_U_EX),
			&r, 
			NULL);
		if (b &&
			m_mreguex.m_res_handle_u.m_handle &&
			m_mreguex.m_addr &&
			m_mreguex.m_size)
			return true;
	}
	Unmap();
	return false;
}

inline
void
CSoraRegisterImp::Unmap(
	) {

	if (m_device != INVALID_HANDLE_VALUE) {
		if (m_mreguex.m_res_handle_u.m_handle) {
			DWORD r;
			r = 0;
			BOOL b;
			b = DeviceIoControl(
				m_device,
				__IOCTL_RELEASE_RESOURCE,
				&m_mreguex.m_res_handle_u, 
				sizeof(RES_HANDLE_U),
				NULL, 
				0,
				&r, 
				NULL);
		}		
		CloseHandle(m_device);
	}
	m_device = INVALID_HANDLE_VALUE;
	memset(&m_mreguex, 0, sizeof(MAP_MEM_U_EX));
}

inline
CSoraRadio* 
CSoraRegisterImp::AllocSoraRadio(int i) {

	switch(i) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		return new CSoraRadioImp(i, &m_mreguex.m_addr->RadioRegs[i]);
		break;
	}
	return NULL;
}

inline
void* 
CSoraRegisterImp::GetMappingAddr() {

	return m_mreguex.m_addr;
}

inline
REG32VAL 
CSoraRegisterImp::GetFirmwareVersion() {

	return *(REG32VAL*)&m_mreguex.m_addr->FirmwareVersion;
}

inline
REG32VAL
CSoraRegisterImp::GetHWStatus() {

	return *(REG32VAL*)&m_mreguex.m_addr->HWStatus;
}

inline
REG32VAL
CSoraRegisterImp::GetLinkStatus() {

	return *(REG32VAL*)&m_mreguex.m_addr->LinkStatus;
}

inline
REG32VAL
CSoraRegisterImp::GetLinkControl() {

	return *(REG32VAL*)&m_mreguex.m_addr->LinkControl;
}

inline
REG32VAL 
CSoraRegisterImp::GetHWControl() {

	return *(REG32VAL*)&m_mreguex.m_addr->HWControl;
}

inline
REG32VAL 
CSoraRegisterImp::GetPCIeTXBurstSize() {

	return *(REG32VAL*)&m_mreguex.m_addr->PCIeTXBurstSize;
}

inline
REG32VAL 
CSoraRegisterImp::GetPCIeRXBlockSize() {

	return *(REG32VAL*)&m_mreguex.m_addr->PCIeRxBlockSize;
}

inline
CSoraRCBMem*
CSoraRegisterImp::AllocSoraRCBMem() {

	return new CSoraRCBMemImp(m_mreguex.m_addr);
}
