/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name: sdr_mac.h
  

Abstracts:
    This file defines structures and interfaces that used to manage Mac Object
    It defines data structure for Mac object
    It also defines interfaces for:
    
    Init and Clean up Mac
    Start core threads for Mac

Revision History:
    Created by yichen, 8/Arp/2009

Notes: 

--*/

#ifndef _DOT11B_MAC_H_
#define _DOT11B_MAC_H_
#include "sora.h"

#include "dot11_pkt.h"

#include "sdr_mac_send_queue.h"
#include "sdr_mac_recv_queue.h"

#define TX_RETRY_TIMEOUT    16

#define ACK_MIN_TIMEOUT     11        //micro-second
#define ACK_MAX_TIMEOUT     60
typedef struct _MP_ADAPTER *PMP_ADAPTER;

// Declare MAC FSM states for 802.11
SORA_BEGIN_DECLARE_FSM_STATES(Dot11)
    SORA_DECLARE_STATE(Dot11_MAC_CS) 
    SORA_DECLARE_STATE(Dot11_MAC_TX)
    SORA_DECLARE_STATE(Dot11_MAC_RX)
SORA_END_DECLARE_FSM_STATES(Dot11)

// Declare a FSM structure type for 802.11
SORA_DECLARE_FSM_TYPE(DOT11FSM, Dot11) 

/*
 *  The following structure contains state for a MAC object
 */
typedef struct _MAC
{
    // MAC state machine
    DOT11FSM                    StateMachine;

    // Sending queue
    SEND_QUEUE_MANAGER          SendQueueManager;  
    
    // Receiving queue
    RECV_QUEUE_MANAGER          RecvQueueManager;  
   
/*
 *  In this sample code, we use two additional threads for
 *  1) asynchronous modulation; and 2) indication of demodulated frames 
 *  and packet finalization.
 */

    SORA_THREAD                 SendThread;
    SORA_THREAD                 ReceiveThread;
    
    // Dot11 specific MAC variables	
    volatile BOOLEAN            bMacInitOK;  // is MAC properly initialized?

    ULONG                       fTxNeedACK;  // Set if an incoming ACK is expected

    // Sequence control
    USHORT                      CurRecvSeqNo; //Recv seq #

    // Monitoring support
    ULONG                       fDumpMode;    // is MAC in monitor mode
    PUCHAR                      pDumpBuffer;  // pointer to the buffer storing I/Q samples in monitor mode

	HANDLE						Thread;
}MAC, *PMAC;

// Macro defines for short-cuts
#define IS_MAC_EXPECT_ACK(pMac) (pMac->fTxNeedACK)
#define MAC_EXPECT_ACK(pMac)    do { pMac->fTxNeedACK = TRUE; } while(FALSE);
#define MAC_DISLIKE_ACK(pMac)    do { pMac->fTxNeedACK = FALSE; } while(FALSE);

// Send RecvThread a signal to release its wait state
#define SDR_MAC_INDICATE_RECV_PACKET(_pMacManager)                                  \
    KeSetEvent(&(_pMacManager)->RecvQueueManager.hRecvEvent,IO_NO_INCREMENT,FALSE)

// Send RecvThread a signal to release its wait state
#define SDR_MAC_INDICATE_PACKET_SENT_COMPLETE(_pMacManager)                         \
    KeSetEvent(&(_pMacManager)->RecvQueueManager.hRecvEvent,IO_NO_INCREMENT,FALSE)

//////////////////////////////////////////////////////////////////////////

// The single entry function to start mac
#ifdef __cplusplus
extern "C"
{
#endif
HRESULT
SdrMacInitialize(IN PMAC pMac, PSDR_CONTEXT SDRContext);

// Initialize the mac state-machine
VOID
SdrMacInitStateMachine(IN PMAC pMac, IN PSDR_CONTEXT SDRContext);

// Cleanup (destroy) mac
VOID
SdrMacCleanUp(IN PMAC pMac);

// Start the auxiliary threads (send/recv threads)
VOID
SdrMacStartThreads(IN PMAC, IN PSDR_CONTEXT);


/*
 * Dot11 MAC FSM handlers
 */

// carrier sensing
VOID
SdrMacCs(IN PFSM_BASE StateMachine);

// Transmitting
VOID
SdrMacTx(IN PFSM_BASE StateMachine);

// Receiving
VOID
SdrMacRx(IN PFSM_BASE StateMachine);

/*
 * Auxiliary thread entries 
 */
VOID
SdrMacSendThread(IN PVOID pVoid);

VOID
SdrMacSendRecvCompleteThread(IN PVOID pVoid);

#ifdef __cplusplus
}
#endif

#endif
