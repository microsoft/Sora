/*++
Copyright (c) Microsoft Corporation

Module Name: sdr_mac_main.c

Abstract: This file implements interface functions for initialization and cleanup of Mac
          it also contain functions for start up of system threads of which are state machine,
          sending thread, receiving thread

History: 
          3/23/2009: Created by yichen
--*/

#include "miniport.h"
#include "trace.h"
#include "sdr_mac_main.tmh"
#include "thread_if.h"

// Allocate a buffer for sample dump
NDIS_STATUS AllocateDumpBuffer(PMAC pMac)
{
    NDIS_STATUS DumpStatus = NDIS_STATUS_SUCCESS;
    //allocate mem for dump
    MP_ALLOCATE_MEMORY(pMac->pDumpBuffer, RX_BUFFER_SIZE, DumpStatus);
    if(DumpStatus != NDIS_STATUS_SUCCESS)
    {
        DbgPrint("[Error] allocate dump buffer fails\n");
    }

    return DumpStatus;
}

//dump buffer release
void ReleaseDumpBuffer(PMAC pMac)
{
     if(pMac->pDumpBuffer)
     {
        NdisFreeMemory(pMac->pDumpBuffer, RX_BUFFER_SIZE, 0);
        pMac->pDumpBuffer = NULL;
     }

     pMac->fDumpMode = 0;
}

VOID
SdrMacInitStateMachine(IN PMAC pMac, IN PSDR_CONTEXT SDRContext)
{
    // Associate the real state handlers to the FSM
    SORA_FSM_ADD_HANDLER(pMac->StateMachine, Dot11_MAC_CS, SdrMacCs);
    SORA_FSM_ADD_HANDLER(pMac->StateMachine, Dot11_MAC_TX, SdrMacTx);
    SORA_FSM_ADD_HANDLER(pMac->StateMachine, Dot11_MAC_RX, SdrMacRx);
	
    SORA_FSM_CONFIG (pMac->StateMachine, SDRContext, 0);

    // Set the initial start state    
    SoraFSMSetInitialState ((PFSM_BASE)&pMac->StateMachine, Dot11_MAC_CS);
    
}

/*
SdrMacStartThreads initializes and starts up three system threads for Mac.
There are sending thread, receiving thread.

Parameters:
    pMac : pointer to Mac object
    Context: pointer to context object that these threads run in.

Return:  

Note: 

History:   4/1/2009 Created by yichen

IRQL: PASSIVE_LEVEL
*/
VOID
SdrMacStartThreads(IN PMAC pMac, PSDR_CONTEXT Context)
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    
    INIT_SORA_THREAD(
        pMac->SendThread,
        SdrMacSendThread, //entry point
        Context, //thread context
        SYNCHRONIZE | GENERIC_ALL | STANDARD_RIGHTS_ALL, 
        KeQueryActiveProcessors()
        );
    
    INIT_SORA_THREAD(
        pMac->ReceiveThread,
        SdrMacSendRecvCompleteThread,
        Context, 
        SYNCHRONIZE | GENERIC_ALL | STANDARD_RIGHTS_ALL,
        KeQueryActiveProcessors()
        );

    START_SORA_THREAD(pMac->SendThread);

    START_SORA_THREAD(pMac->ReceiveThread);
}

/*
SdrMacInitialize initializes MAC object, including send, receive queues, 
and serveral MAC system threads.

Parameters:
    pMac      : pointer to Mac object.
    pAdapter  : pointer to adapter object created in miniport driver

Return:  
    S_OK for success, otherwise error code.

Note: 

History:   
    4/1/2009 Created by yichen

IRQL: PASSIVE_LEVEL
*/

BOOLEAN mac_proc(PVOID pVoid) {
	
    __PFSM_BASE pSMBasic = (__PFSM_BASE)pVoid;

	(pSMBasic->__pFSMHandlers[pSMBasic->__CurrentState])((PFSM_BASE)pVoid);

	{
		PSDR_CONTEXT pSDRContext;
		PPHY pPhy;                   
		pSDRContext = (PSDR_CONTEXT)SoraFSMGetContext(pSMBasic); 
		pPhy = (PPHY)pSDRContext->Phy;
		return pPhy->BBContextFor11A.fCanWork && pPhy->BBContextFor11B.fCanWork;
	}
}

HRESULT
SdrMacInitialize(IN PMAC pMac, IN PSDR_CONTEXT SDRContext)
{
    HRESULT  hRes     = S_OK;
    
    do 
    {   
        pMac->fTxNeedACK            = FALSE;
        
        // Randomly pick up a receive sequence
        pMac->CurRecvSeqNo          = 0xcdcd; 

        hRes = InitSendQueueManager(&(pMac->SendQueueManager), SDRContext->Nic); //send queue is NDIS dependant
        FAILED_BREAK(hRes);

        hRes = InitializeRecvQueueManager(&(pMac->RecvQueueManager));
        FAILED_BREAK(hRes);

        // Allocate an additional buffer for dumping samples
        AllocateDumpBuffer(pMac);
        
        SdrMacInitStateMachine(pMac, SDRContext);

        // Set flag to indicate MAC is ready
        pMac->bMacInitOK = TRUE;
        
        // start auxiliary threads
        SdrMacStartThreads(pMac, SDRContext);

        // finally, we start the MAC FSM

		hRes = NDIS_STATUS_FAILURE;
		pMac->Thread = SoraThreadAlloc();
		if (pMac->Thread)
			if (SoraThreadStart(pMac->Thread, mac_proc, (PFSM_BASE)&pMac->StateMachine))
				hRes = NDIS_STATUS_SUCCESS;

		if (hRes != NDIS_STATUS_SUCCESS)
			if (pMac->Thread) {
				SoraThreadFree(pMac->Thread);
				pMac->Thread = NULL;
			}
        
        if (hRes != NDIS_STATUS_SUCCESS)
        {
            DbgPrint("[Error] MAC State Machine is not started \n");
        }

    } while (FALSE);

    return hRes;
}


/*
SdrMacCleanUp is a cleanup interface for Mac , it's called when Mac need to be destroyed.
it stops three core threads of which are started by SdrMacInitialize, and release resources allocated for
ack cache queue, send queue and receive queue

 Parameters:
            pMac : pointer to Mac object.
        
 Return:  

 Note: 

 History:   4/1/2009 Created by yichen

 IRQL: PASSIVE_LEVEL
*/
VOID
SdrMacCleanUp(IN PMAC pMac)
{   
    do 
    {
        if(pMac == NULL)
            break;

        if(pMac->bMacInitOK)
        {
            //pMac->pPhy->BBContext.fCanWork = FALSE; //force any loop in PHY to break.
            DbgPrint("[Exit] Try to stop state machine\n");

			if (pMac->Thread) {
				SoraThreadFree(pMac->Thread);
				pMac->Thread = NULL;
			}

            DbgPrint("[Exit] Try to stop send thread \n");
            STOP_SORA_THREAD(pMac->SendThread);
            DbgPrint("[Exit] Try to stop receive/TxDone thread \n");
            STOP_SORA_THREAD(pMac->ReceiveThread);
            DbgPrint("[Exit] All thread stopped \n");
        }

        CleanSendQueueManager(&pMac->SendQueueManager);
        CleanupRecvQueueManager(&(pMac->RecvQueueManager));

        ReleaseDumpBuffer(pMac);

    } while (FALSE);

}

