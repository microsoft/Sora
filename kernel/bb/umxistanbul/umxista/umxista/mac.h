#pragma once
#include <sora.h>
#include "dot11_pkt.h"
#include "_user_mode_ext.h"

#include "phy.h"
#include "_spinlock.h"

#define PACKET_MAX_RETRY	15

#define ACK_MIN_TIMEOUT		11 // micro-second
#define ACK_MAX_TIMEOUT		60

#define ETH_IS_MULTICAST(Address) \
    (BOOLEAN)(((PUCHAR)(Address))[0] & ((UCHAR)0x01))

#define ETH_IS_BROADCAST(Address) \
    ((((PUCHAR)(Address))[0] == ((UCHAR)0xff)) && (((PUCHAR)(Address))[1] == ((UCHAR)0xff)) && (((PUCHAR)(Address))[2] == ((UCHAR)0xff)) && (((PUCHAR)(Address))[3] == ((UCHAR)0xff)) && (((PUCHAR)(Address))[4] == ((UCHAR)0xff)) && (((PUCHAR)(Address))[5] == ((UCHAR)0xff)))


extern ULONG macVQueue;
extern ULONG macDrop;

// MAC send queue entry
struct MAC2TxID {
	MAC2TxID* m_next;
	MAC_ADDRESS m_address;
	ULONG       m_dataRate;
	ULONG       m_txid;
};

struct PACKETxID {
	LIST_ENTRY m_entry;
	PACKET_HANDLE m_packet;

	ULONG m_txid;
	ULONG m_size;	// the size of the modulated signal in bytes

	ULONG m_txid1;  // first  retran rate
	ULONG m_size1;	// the size of the modulated signal in bytes

	ULONG m_txid2;	// second retran rate
	ULONG m_size2;	// the size of the modulated signal in bytes

	LONG m_status;
	LONG m_needack;
	LONG m_retry;
};

// chain a TxID at the tail
FINL 
MAC2TxID* MAC2TxID_Chain( MAC2TxID** root, MAC_ADDRESS address, ULONG txid, ULONG dataRate) 
{
	MAC2TxID* obj;
	obj = (MAC2TxID*)malloc(sizeof(MAC2TxID));
	obj->m_next = *root;
	obj->m_address = address;
	obj->m_dataRate = dataRate;
	obj->m_txid = txid;
	*root = obj;
	return *root;
}

// find a TxID in list
FINL 
MAC2TxID* MAC2TxID_Find( MAC2TxID* root, MAC_ADDRESS address, ULONG dataRate) 
{
	while(root) {
		if ( memcmp(root->m_address.Address, address.Address, sizeof(MAC_ADDRESS)) || 
			(dataRate != root->m_dataRate) ) {
			root = root->m_next;
			continue;
		}

		return root;
	}
	return NULL;
}

// MAC state machine
typedef BOOLEAN MAC_STATE_HANDLER(void*); 

enum MAC_STATE {
	MAC_STATE_RX 	= 0,
	MAC_STATE_TX 	= 1,
	MAC_STATE_WAITACK = 2,
};

void GetRetranRates ( int rate, int& rrate1, int& rrate2 );

extern MAC_STATE 			current_state;

// MAC status
extern char                 ssid  [64];
extern UINT                 ssid_len;
extern BSSID				bssid;

extern BSSID adhoc_bssid;
extern char  adhoc_ssid[];
extern UINT  adhoc_ssid_len;

// MAC frame sequence number
extern USHORT				SendSeQ; 

// MAC address of the node
extern MAC_ADDRESS			MACAddress;

extern MAC2TxID*			Root;
extern struct PACKETxID*	LastPACKETxID;


#define insert_list_tail(list, entry, lock) {				\
	acquire_lock (lock);									\
	InsertTailList(list, entry);							\
	release_lock (lock); 									\
}

#define remove_list_head(list, entry, lock) {				\
	acquire_lock (lock);									\
	entry = RemoveHeadList(list);							\
	release_lock (lock); 									\
}

extern SPINLOCK 		SendListLock;
extern LIST_ENTRY		SendListHead;

// thread proc
void __cdecl Dot11aSendThread (void*);
BOOLEAN Dot11aRecvProc (void* args); 

BOOLEAN Dot11bSendProc (void*);
BOOLEAN Dot11bRecvProc (void* args); 

extern const ULONG RateMap11a[]; 	

#pragma pack(push,1)
typedef struct tagWLAN_AUTH_FRAME {
	WLAN_DATA_ADDRESS3_HEADER wlanheader;
	USHORT                    alg;
	USHORT                    seq;
	USHORT                    status;
	ULONG                     FCS;
} WLAN_AUTH_FRAME, * PWLAN_AUTH_FRAME;

typedef struct tagWLAN_NULL_FRAME {
	WLAN_DATA_ADDRESS3_HEADER wlanheader;
	ULONG                     FCS;
} WLAN_NULL_FRAME, * PWLAN_NULL_FRAME;

typedef struct tagWLAN_ASSO_FRAME {
	WLAN_DATA_ADDRESS3_HEADER wlanheader;
	USHORT                    capInfo;
	USHORT                    interval; // listen interval
} WLAN_ASSO_FRAME, * PWLAN_ASSO_FRAME;

typedef struct tagInfoElement {
	UCHAR  				  eid;
	UCHAR                 elen;
	UCHAR                 data [1];
} INFO_ELEMENT, * PINFO_ELEMENT;

#pragma pack(pop)

