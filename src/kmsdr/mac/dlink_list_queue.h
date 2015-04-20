/*++
Copyright (c) Microsoft Corporation

Module Name: dlink_list_queue.h

Abstract: Thread-Safe queue implementation with double linked list.

History: 
          10/15/2009: Created by senxiang
--*/
#ifndef _DLINK_LIST_QUEUE_H
#define _DLINK_LIST_QUEUE_H

#define SafeEnqueue(QueueMan, Queue, pTCB) \
{ \
    NdisInterlockedInsertTailList(&(QueueMan)->Queue, &(pTCB)->List, &(QueueMan)->QueueLock);\
} 

#define SafeDequeue(QueueMan, Queue, pTCB, type) \
{ \
    PLIST_ENTRY pFreeEntry;\
    pFreeEntry = NdisInterlockedRemoveHeadList(&(QueueMan)->Queue, &(QueueMan)->QueueLock);\
    pTCB = CONTAINING_RECORD(pFreeEntry, type, List);\
}

//Insert into queue head
#define SafeJumpQueue(QueueMan, Queue, pTCB) \
{ \
    NdisInterlockedInsertHeadList(&(QueueMan)->Queue, &(pTCB)->List, &(QueueMan)->QueueLock);\
}

#endif