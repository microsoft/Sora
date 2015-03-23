#include "sora.h"

#define LOCK_WAIT_TIMEOUT   (1000 )

#ifndef USER_MODE

typedef enum _RW_TYPE
{
    RdReq   = 0,
    WrReq   = 1
} RW_TYPE;

typedef struct _RW_REQ
{
    LIST_ENTRY          __List;
    volatile BOOLEAN    LockObtained;
    RW_TYPE             Type;
} RW_REQ, *PRW_REQ;

void SoraKInitQueuedRWLock(PQUEUED_RW_LOCK RWLock)
{
    KeInitializeSpinLock(&RWLock->Lock);
    InitializeListHead(&RWLock->WaitingList);
    RWLock->RWSummary = 0;
}

BOOLEAN _SafeRemoveReq(PQUEUED_RW_LOCK RWLock, PRW_REQ Req)
{
    KIRQL Irq;
    BOOLEAN Succ;
    KeAcquireSpinLock(&RWLock->Lock, &Irq);
    if (!IsListEmpty(&Req->__List)) 
    {
        Succ = TRUE;
        RemoveEntryList(&Req->__List);
    }
    else
    {
        Succ = FALSE;
    }
    KeReleaseSpinLock(&RWLock->Lock, Irq);
    return Succ;
}

HRESULT SoraKAcquireRXLock(PQUEUED_RW_LOCK RWLock)
{
    KIRQL Irq;
    RW_REQ RReq;

    KeAcquireSpinLock(&RWLock->Lock, &Irq);
    if (RWLock->RWSummary >= 0 && IsListEmpty(&RWLock->WaitingList)) //free or RXing and no tx waiting
    {
        DbgPrint("[UExtK] SoraKAcquireRXLock: free, obtained\n");
        RReq.LockObtained = TRUE;
        RWLock->RWSummary++;
    }
    else //Txing or Tx request waiting
    {
        DbgPrint("[UExtK] SoraKAcquireRXLock: queue request\n");
        RReq.LockObtained = FALSE;
        RReq.Type = RdReq;
        InsertTailList(&RWLock->WaitingList, &RReq.__List);
    }
    KeReleaseSpinLock(&RWLock->Lock, Irq);

    int i = LOCK_WAIT_TIMEOUT;
    while(!RReq.LockObtained) //wait until wake up
    {
        //LARGE_INTEGER interval;
        //interval.QuadPart = -10 * 1000;
        //KeDelayExecutionThread(KernelMode, FALSE, &interval); //wait 1ms;
        i--;
        if (i < 0)
        {
            
            if (_SafeRemoveReq(RWLock, &RReq) == FALSE) //successful cancelled, otherwise lock obtained.       
                i = 0;
            else
                DbgPrint("[UExtK] SoraKAcquireRXLock: RX lock timeout\n");
            break;
        }
    }
    if (i >= 0)
    {
        DbgPrint("[UExtK]SoraKAcquireRXLock: lock obtained\n");
    }
    DbgPrint("[UExtK] SoraKAcquireRXLock: RW Summary %d\n", RWLock->RWSummary);
    return (i < 0) ? E_LOCK_WAIT_TIMEOUT : S_OK;
    
}

void SoraKReleaseRXLock(PQUEUED_RW_LOCK RWLock)
{
    KIRQL Irq;
    KeAcquireSpinLock(&RWLock->Lock, &Irq);
    if (RWLock->RWSummary > 0)
    {
        RWLock->RWSummary--;
        if (RWLock->RWSummary == 0)
        { //wake up
            DbgPrint("[UExtK] SoraKReleaseRXLock: try to wake up\n");
            if (!IsListEmpty(&RWLock->WaitingList))
            {
                PLIST_ENTRY entry = RemoveHeadList(&RWLock->WaitingList);
                InitializeListHead(entry); //tx req is removed from waiting list;
                PRW_REQ req = CONTAINING_RECORD(entry, RW_REQ, __List);
                req->LockObtained = TRUE;
                RWLock->RWSummary = -1;
                DbgPrint("[UExtK] SoraKReleaseRXLock: wake up tx waiting request, %d\n", req->Type);
            }
        }
    }
    KeReleaseSpinLock(&RWLock->Lock, Irq);
    DbgPrint("[UExtK] SoraKReleaseRXLock: RW Summary %d\n", RWLock->RWSummary);
}

HRESULT SoraKAcquireTXLock(PQUEUED_RW_LOCK RWLock)
{
    KIRQL Irq;
    RW_REQ WReq;

    KeAcquireSpinLock(&RWLock->Lock, &Irq);
    if (RWLock->RWSummary == 0)
    {
        DbgPrint("[UExtK] SoraKAcquireTXLock: free, obtained\n");
        WReq.LockObtained = TRUE;
        RWLock->RWSummary = -1;
    }
    else
    {
        DbgPrint("[UExtK] SoraKAcquireTXLock: queue request\n");
        WReq.LockObtained = FALSE;
        WReq.Type = WrReq;
        InsertTailList(&RWLock->WaitingList, &WReq.__List);
    }
    KeReleaseSpinLock(&RWLock->Lock, Irq);

    int i = LOCK_WAIT_TIMEOUT;
    while(!WReq.LockObtained) //wait until wake up
    {
        ///LARGE_INTEGER interval;
        //interval.QuadPart = -10 * 1000;
        //KeDelayExecutionThread(KernelMode, FALSE, &interval); //wait 1ms;
        i--;
        DbgPrint("[UExtK] wait tx lock i=%d\n", i);
        if (i < 0)
        {
            if (_SafeRemoveReq(RWLock, &WReq) == FALSE) //successful cancelled, otherwise lock obtained.
                i = 0;
            else
                DbgPrint("[UExtK] SoraKAcquireTXLock: TX lock timeout\n");  
            break;
        }
    }
    if (i >= 0)
    {
        DbgPrint("[UExtK] SoraKAcquireTXLock: TX lock obtained\n");
    }
    DbgPrint("[UExtK] SoraKAcquireTXLock: RW Summary %d\n", RWLock->RWSummary);
    return (i < 0) ? E_LOCK_WAIT_TIMEOUT : S_OK;
}

void SoraKReleaseTXLock(PQUEUED_RW_LOCK RWLock)
{
    KIRQL Irq;

    KeAcquireSpinLock(&RWLock->Lock, &Irq);
    if (RWLock->RWSummary == -1)
    {
        RWLock->RWSummary = 0;

        while(!IsListEmpty(&RWLock->WaitingList))
        {
            PLIST_ENTRY entry = RemoveHeadList(&RWLock->WaitingList);
            InitializeListHead(entry); 
            PRW_REQ req = CONTAINING_RECORD(entry, RW_REQ, __List);
            if (req->Type == WrReq)
            {
                if (RWLock->RWSummary == 0)
                {
                    DbgPrint("[UExtK]SoraKReleaseTXLock: wake up TX\n");
                    req->LockObtained = TRUE;
                    RWLock->RWSummary = -1;
                    break;
                }
                else
                {
                    InsertHeadList(&RWLock->WaitingList, entry); //push back
                    break;
                }
            }
            else //read
            {
                if (RWLock->RWSummary >= 0)
                {
                    req->LockObtained = TRUE;
                    DbgPrint("[UExtK]SoraKReleaseTXLock: wake up RX\n");
                    RWLock->RWSummary++;
                }
                else
                {
                    InsertHeadList(&RWLock->WaitingList, entry); //push back
                    break;
                }
            }   
        }
    }
    KeReleaseSpinLock(&RWLock->Lock, Irq);
    DbgPrint("[UExtK] SoraKReleaseTXLock: RW Summary %d\n", RWLock->RWSummary);
}

#endif
