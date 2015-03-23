#pragma once

typedef union {
	void* m_handle;
	unsigned __int64  m_x64_wrapper;
} RES_HANDLE_U;

#define private_ext(cmd) CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0A00+cmd, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef enum private_cmd {

	ACQ_RCB_MEM = 0x0100,
	REL_MAP_MEM,
};

struct MAP_MEM_U {

	RES_HANDLE_U m_res_handle_u;
	union {
		void* m_addr;
		unsigned __int64  m_x64_wrapper;
	};
	unsigned long m_size;
	unsigned long m_hi_phy_addr;
	unsigned long m_lo_phy_addr;
};

struct ACQ_RCB_MEM_U {

	unsigned long m_rcb_addr;
	unsigned long m_rcb_size;	
	struct MAP_MEM_U m_tx_desc;
};

struct ACQ_RCB_MEM_IN {

	unsigned long m_rcb_size;
};

