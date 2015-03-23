#include "sora.h"

NTSTATUS SoraFSMStart( PFSM_BASE StateMachine )
{
    NTSTATUS Status;
    StateMachine->__fTerminate = FALSE; 
    Status = PsCreateSystemThread ( 
            &(StateMachine->__Thread.hThread), 
            StateMachine->__Thread.DesiredAccess, 
            NULL, NULL, NULL,
            StateMachine->__Thread.pStartRoutine,
            StateMachine); 
    ZwClose(StateMachine->__Thread.hThread);
    return Status;
}

VOID SoraFSMStop( PFSM_BASE StateMachine )
{
    do 
    { 
        if(StateMachine->__fTerminate == __FSM_TERMINATED)
            break;
        if (StateMachine->__fTerminate == FALSE) 
        { 
            StateMachine->__fTerminate = TRUE;  
            while (StateMachine->__fTerminate != __FSM_TERMINATED)
            { 
                _mm_pause(); 
            } 
        }
    } while(FALSE);
}