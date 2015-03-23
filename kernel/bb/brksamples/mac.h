#pragma once
#include <sora.h>
#include "rxgraph.hpp"


#define PACKET_MAX_RETRY	16

#define ACK_MIN_TIMEOUT		11 // micro-second
#define ACK_MAX_TIMEOUT		60

#define ETH_IS_MULTICAST(Address) \
    (BOOLEAN)(((PUCHAR)(Address))[0] & ((UCHAR)0x01))

#define ETH_IS_BROADCAST(Address) \
    ((((PUCHAR)(Address))[0] == ((UCHAR)0xff)) && (((PUCHAR)(Address))[1] == ((UCHAR)0xff)) && (((PUCHAR)(Address))[2] == ((UCHAR)0xff)) && (((PUCHAR)(Address))[3] == ((UCHAR)0xff)) && (((PUCHAR)(Address))[4] == ((UCHAR)0xff)) && (((PUCHAR)(Address))[5] == ((UCHAR)0xff)))

// MAC state machine
typedef BOOLEAN MAC_STATE_HANDLER(void*); 

enum MAC_STATE {
	MAC_STATE_CS 	= 0,
	MAC_STATE_RX 	= 1,
	MAC_STATE_TX 	= 2,
};

extern MAC_STATE 			current_state;


extern ISource* pCSGraph;
extern ISource* pRxGraph;

// thread proc
BOOLEAN RxProc ( void * args ) ;
void CreateBasebandGraph  ();
void ReleaseBasebandGraph ();



