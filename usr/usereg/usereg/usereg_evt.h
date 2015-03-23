#pragma once

#include "usereg.h"

class state_obj;
class transfer_idle_state_obj;
class transfer_init_state_obj;
class transfer_done_state_obj;
class transfer_state_obj;
class tx_idle_state_obj;
class tx_init_state_obj;
class tx_done_state_obj;
class tx_state_obj;
class rx_idle_state_obj;
class rx_init_state_obj;
class rx_state_obj;

class state_obj {

public:	
	virtual EVENTS update(REG32VAL) = 0;
};

class transfer_idle_state_obj : public state_obj {

public:	
	transfer_idle_state_obj(transfer_state_obj*);
	virtual EVENTS update(REG32VAL val);

protected:
	transfer_state_obj* m_transfer_state_obj;
};

class transfer_init_state_obj : public state_obj {

public:
	transfer_init_state_obj(transfer_state_obj*);
	virtual EVENTS update(REG32VAL val);

protected:
	transfer_state_obj* m_transfer_state_obj;
};

class transfer_done_state_obj : public state_obj {

public:
	transfer_done_state_obj(transfer_state_obj*);
	virtual EVENTS update(REG32VAL val);

protected:
	transfer_state_obj* m_transfer_state_obj;
};

class transfer_state_obj : public state_obj {

	friend class transfer_idle_state_obj;
	friend class transfer_init_state_obj;
	friend class transfer_done_state_obj;

public:
	transfer_state_obj();
	virtual EVENTS update(REG32VAL val);

protected:	
	transfer_idle_state_obj* m_transfer_idle_state_obj;
	transfer_init_state_obj* m_transfer_init_state_obj;
	transfer_done_state_obj* m_transfer_done_state_obj;
	state_obj* m_transfer_state_obj;
};

class tx_idle_state_obj : public state_obj {

public:
	tx_idle_state_obj(tx_state_obj*);
	virtual EVENTS update(REG32VAL val);

protected:
	tx_state_obj* m_tx_state_obj;
};

class tx_init_state_obj : public state_obj {

public:
	tx_init_state_obj(tx_state_obj*);
	virtual EVENTS update(REG32VAL val);

protected:
	tx_state_obj* m_tx_state_obj;
};

class tx_done_state_obj : public state_obj {

public:
	tx_done_state_obj(tx_state_obj*);
	virtual EVENTS update(REG32VAL val);

protected:
	tx_state_obj* m_tx_state_obj;
};

class tx_state_obj : public state_obj {

	friend class tx_idle_state_obj;
	friend class tx_init_state_obj;
	friend class tx_done_state_obj;

public:
	tx_state_obj();
	virtual EVENTS update(REG32VAL val);

protected:	
	tx_idle_state_obj* m_tx_idle_state_obj;
	tx_init_state_obj* m_tx_init_state_obj;
	tx_done_state_obj* m_tx_done_state_obj;
	state_obj* m_tx_state_obj;
};

class rx_idle_state_obj : public state_obj {

public:
	rx_idle_state_obj(rx_state_obj*);
	virtual EVENTS update(REG32VAL val);

protected:
	rx_state_obj* m_rx_state_obj;
};

class rx_init_state_obj : public state_obj {

public:
	rx_init_state_obj(rx_state_obj*);
	virtual EVENTS update(REG32VAL val);

protected:
	rx_state_obj* m_rx_state_obj;
};

class rx_state_obj : public state_obj {

	friend class rx_idle_state_obj;
	friend class rx_init_state_obj;

public:
	rx_state_obj();
	virtual EVENTS update(REG32VAL val);

protected:	
	rx_idle_state_obj* m_rx_idle_state_obj;
	rx_init_state_obj* m_rx_init_state_obj;
	state_obj* m_rx_state_obj;
};
