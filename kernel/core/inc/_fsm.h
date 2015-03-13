/*++
Copyright (c) Microsoft Corporation

Module Name: _fsm.h

Abstract: 
This header file defines structs, macros, interfaces to 
help Sora user easily define their state machines. 

History: 
    
--*/

#if !defined(_FSM_H)
#define _FSM_H

#define __FSM_TERMINATED                 2

typedef LONG SORA_FSM_STATE, *PSORA_FSM_STATE;

#define SORA_BEGIN_DECLARE_FSM_STATES(fsmname)\
    typedef enum __SORA_STATE_INDEX_##fsmname{ \
        __START_##fsmname = -1, 

#define SORA_DECLARE_STATE(State)  State,

#define SORA_END_DECLARE_FSM_STATES(fsmname) \
        __END_##fsmname \
    } ; \
    CCASSERT(__START_##fsmname == -1)

//#define SORA_GET_STATEMACHINE_FROM_CONTEXT(SMContext) \
//    (&(SMContext).__StateMachine)

typedef struct __FSM_BASE *PFSM_BASE;
typedef VOID (*SORA_STATE_HANDLER)(IN OUT PFSM_BASE StateMachine);

typedef struct __FSM_BASE
{
    volatile ULONG              __fTerminate;
    SORA_FSM_STATE              __CurrentState; 
    __THREAD                    __Thread; 
    SORA_STATE_HANDLER          *__pFSMHandlers;
    //SORA_STATE_HANDLER          __FSMHandlers[0]; 
}__FSM_BASE, *__PFSM_BASE;

typedef struct __SDR_CONTEXT
{
    PVOID   Nic;
    PVOID   LinkLayer;
    PVOID   Mac;
    PVOID   Phy;
    ULONG   Reserved1;
    ULONG   Reserved2;
} SDR_CONTEXT, *PSDR_CONTEXT;

#define SORA_DECLARE_FSM_TYPE(type, StateType) \
    typedef struct __STATE_MACHINE_##type \
    { \
        __FSM_BASE                __base; \
        SORA_STATE_HANDLER        __FSMHandlers[(__END_##StateType)];\
    } type, *P##type;

//#define SORA_BEGIN_DECLARE_SM_CONTEXT(contextype, smtype) \
//    typedef struct __##contextype{ \
//        smtype __StateMachine;
//
//#define SORA_END_DECLARE_SM_CONTEXT(contextype) \
//    } contextype, *P##contextype;

#define __FSM_NEED_TERMINATE(p) (p->__fTerminate)

//#define SORA_SM_GOTO_STATE(sm, s) \
//    do { (sm).__base.__CurrentState = s; } while(0);

#define SORA_FSM_ADD_HANDLER(fsm, index, handler) \
    do { (fsm).__FSMHandlers[(index)] = handler; } while (0);

VOID __SORA_FSM_ENGINE(PVOID pVoid);

#if 1
#define SORA_FSM_CONFIG(fsm, SdrContext, affinity) \
    do { \
		(fsm).__base.__Thread.pStartContext = SdrContext; \
        (fsm).__base.__fTerminate  = __FSM_TERMINATED; \
        (fsm).__base.__pFSMHandlers = &(fsm).__FSMHandlers[0]; \
    } while (0); 
#else
#define SORA_FSM_CONFIG(fsm, SdrContext, affinity) \
    do { \
        (fsm).__base.__Thread.DesiredAccess = SYNCHRONIZE | GENERIC_ALL | STANDARD_RIGHTS_ALL; \
        (fsm).__base.__Thread.pStartRoutine = __SORA_FSM_ENGINE; \
        (fsm).__base.__Thread.pStartContext = SdrContext; \
        (fsm).__base.__Thread.Affinity      = affinity; \
        (fsm).__base.__fTerminate  = __FSM_TERMINATED; \
        (fsm).__base.__pFSMHandlers = &(fsm).__FSMHandlers[0];\
    } while (0); 
#endif
/*
SoraFSMGetContext returns the context pointer which is specified when SORA_FSM_CONFIG
*/
__inline PVOID SoraFSMGetContext(PFSM_BASE StateMachine)
{
    return StateMachine->__Thread.pStartContext;
}

__inline VOID SoraFSMGotoState(PFSM_BASE StateMachine, SORA_FSM_STATE s)
{
    StateMachine->__CurrentState = s;
}

__inline VOID SoraFSMSetInitialState(PFSM_BASE StateMachine, SORA_FSM_STATE s)
{
	SoraFSMGotoState (StateMachine, s );
}

NTSTATUS SoraFSMStart( PFSM_BASE StateMachine);

VOID SoraFSMStop( PFSM_BASE StateMachine );

#endif