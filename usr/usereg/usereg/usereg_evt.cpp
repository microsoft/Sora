#include "usereg_evt.h"
#include <sora.h>
#include <__reg_file.h>

#define log

transfer_idle_state_obj::transfer_idle_state_obj(transfer_state_obj* state_obj) {

	m_transfer_state_obj = state_obj;
}

inline
EVENTS 
transfer_idle_state_obj::update(REG32VAL val) {

	__REG32_TRANS_CTRL __reg32_trans_ctrl;
	__reg32_trans_ctrl.Value = val;
	if ( __reg32_trans_ctrl.Bits.TransferInit) {
		log("transfer_idle_state_obj init: 1\n");
		m_transfer_state_obj->m_transfer_state_obj = m_transfer_state_obj->m_transfer_init_state_obj;
		return EVENT_TRANSFER_INIT;
	}
	return 0;
}

transfer_init_state_obj::transfer_init_state_obj(transfer_state_obj* state_obj) {

	m_transfer_state_obj = state_obj;
}

inline
EVENTS 
transfer_init_state_obj::update(REG32VAL val) {

	__REG32_TRANS_CTRL __reg32_trans_ctrl;
	__reg32_trans_ctrl.Value = val;
	if ( __reg32_trans_ctrl.Bits.TransferDone) {
		log("transfer_init_state_obj done: 1\n");
		m_transfer_state_obj->m_transfer_state_obj = m_transfer_state_obj->m_transfer_done_state_obj;
		return EVENT_TRANSFER_DONE;
	}
	if (!__reg32_trans_ctrl.Bits.TransferInit) {
		log("transfer_init_state_obj init: 0\n");
		m_transfer_state_obj->m_transfer_state_obj = m_transfer_state_obj->m_transfer_idle_state_obj;
		return EVENT_TRANSFER_DONE;
	}
	return 0;
}

transfer_done_state_obj::transfer_done_state_obj(transfer_state_obj* state_obj) {

	m_transfer_state_obj = state_obj;
}

inline
EVENTS 
transfer_done_state_obj::update(REG32VAL val) {
	
	__REG32_TRANS_CTRL __reg32_trans_ctrl;
	__reg32_trans_ctrl.Value = val;
	if (!__reg32_trans_ctrl.Bits.TransferInit) {
		m_transfer_state_obj->m_transfer_state_obj = m_transfer_state_obj->m_transfer_idle_state_obj;
		log("transfer_done_state_obj init: 0\n");
	}
	return 0;
}

transfer_state_obj::transfer_state_obj() {

	m_transfer_idle_state_obj = new transfer_idle_state_obj(this);
	m_transfer_init_state_obj = new transfer_init_state_obj(this);
	m_transfer_done_state_obj = new transfer_done_state_obj(this);
	m_transfer_state_obj = m_transfer_idle_state_obj;
};

inline
EVENTS
transfer_state_obj::update(REG32VAL val) {

	return m_transfer_state_obj->update(val);
}

tx_idle_state_obj::tx_idle_state_obj(tx_state_obj* state_obj) {

	m_tx_state_obj = state_obj;
}

inline
EVENTS 
tx_idle_state_obj::update(REG32VAL val) {

	__REG32_TX_CTRL __reg32_tx_ctrl;
	__reg32_tx_ctrl.Value = val;
	if ( __reg32_tx_ctrl.Bits.TXInit) {
		log("tx_idle_state_obj init: 1\n");
		m_tx_state_obj->m_tx_state_obj = m_tx_state_obj->m_tx_init_state_obj;
		return EVENT_TX_INIT;
	}
	return 0;
}

tx_init_state_obj::tx_init_state_obj(tx_state_obj* state_obj) {

	m_tx_state_obj = state_obj;
}

inline
EVENTS 
tx_init_state_obj::update(REG32VAL val) {

	__REG32_TX_CTRL __reg32_tx_ctrl;
	__reg32_tx_ctrl.Value = val;
	if ( __reg32_tx_ctrl.Bits.TXDone) {
		log("tx_init_state_obj done: 1\n");
		m_tx_state_obj->m_tx_state_obj = m_tx_state_obj->m_tx_done_state_obj;
		return EVENT_TX_DONE;
	}
	if (!__reg32_tx_ctrl.Bits.TXInit) {
		log("tx_init_state_obj init: 0\n");
		m_tx_state_obj->m_tx_state_obj = m_tx_state_obj->m_tx_idle_state_obj;
		return EVENT_TX_DONE;
	}
	return 0;
}

tx_done_state_obj::tx_done_state_obj(tx_state_obj* state_obj) {

	m_tx_state_obj = state_obj;
}

inline
EVENTS 
tx_done_state_obj::update(REG32VAL val) {
	
	__REG32_TX_CTRL __reg32_tx_ctrl;
	__reg32_tx_ctrl.Value = val;
	if (!__reg32_tx_ctrl.Bits.TXInit) {
		m_tx_state_obj->m_tx_state_obj = m_tx_state_obj->m_tx_idle_state_obj;
		log("tx_done_state_obj init: 0\n");
	}
	return 0;
}

tx_state_obj::tx_state_obj() {

	m_tx_idle_state_obj = new tx_idle_state_obj(this);
	m_tx_init_state_obj = new tx_init_state_obj(this);
	m_tx_done_state_obj = new tx_done_state_obj(this);
	m_tx_state_obj = m_tx_idle_state_obj;
};

inline
EVENTS
tx_state_obj::update(REG32VAL val) {

	return m_tx_state_obj->update(val);
}

rx_idle_state_obj::rx_idle_state_obj(rx_state_obj* state_obj) {

	m_rx_state_obj = state_obj;
}

inline
EVENTS 
rx_idle_state_obj::update(REG32VAL val) {

	__REG32_RX_CTRL __reg32_rx_ctrl;
	__reg32_rx_ctrl.Value = val;
	if ( __reg32_rx_ctrl.Bits.RXEnable) {
		log("rx_idle_state_obj init: 1\n");
		m_rx_state_obj->m_rx_state_obj = m_rx_state_obj->m_rx_init_state_obj;
		return EVENT_RX_INIT;
	}
	return 0;
}

rx_init_state_obj::rx_init_state_obj(rx_state_obj* state_obj) {

	m_rx_state_obj = state_obj;
}

inline
EVENTS 
rx_init_state_obj::update(REG32VAL val) {

	__REG32_RX_CTRL __reg32_rx_ctrl;
	__reg32_rx_ctrl.Value = val;
	if (!__reg32_rx_ctrl.Bits.RXEnable) {
		log("rx_init_state_obj init: 0\n");
		m_rx_state_obj->m_rx_state_obj = m_rx_state_obj->m_rx_idle_state_obj;
		return EVENT_RX_STOP;
	}
	return 0;
}

rx_state_obj::rx_state_obj() {

	m_rx_idle_state_obj = new rx_idle_state_obj(this);
	m_rx_init_state_obj = new rx_init_state_obj(this);
	m_rx_state_obj = m_rx_idle_state_obj;
};

inline
EVENTS
rx_state_obj::update(REG32VAL val) {

	return m_rx_state_obj->update(val);
}
