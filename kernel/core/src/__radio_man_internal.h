#ifndef __RADIO_MAN_INTERNAL_H
#define __RADIO_MAN_INTERNAL_H
#include "__tx_res.h"

#define __MARK_RADIO_FREE(radio) \
    do { InterlockedExchange(&((radio)->__status.__fFree), 1); } while(0);
#define __MARK_RADIO_USED(radio) \
    do { InterlockedExchange(&((radio)->__status.__fFree), 0); } while(0);
#define __IS_RADIO_FREE(radio)  ((radio)->__status.__fFree)

#define __MARK_RADIO_USED_BY(radio, drivername) \
    do { (radio)->__status.__upperDriverName = drivername; } while(0);

typedef struct ___SORA_RADIO_MANAGER
{
    __SORA_RADIO        __radio_pool[MAX_RADIO_NUMBER];
    ULONG               __radio_count;

    __PSORA_REGISTERS   __RegsBase;
    ULONG               __RegsLength;
    KSPIN_LOCK          __lock; //allocation and free lock
    KSPIN_LOCK          __SysRegsLock;
    TX_RM               __TX_RESMGR;
}SORA_RADIO_MANAGER, __SORA_RADIO_MANAGER, *__PSORA_RADIO_MANAGER, *PSORA_RADIO_MANAGER;

SORA_EXTERN_C void SoraGetLock(IN OUT volatile LONG *pLock);
SORA_EXTERN_C void SoraFreeLock(OUT PLONG pLock);

__inline void __SetRadioAlive(IN OUT PSORA_RADIO pRadio)
{
    pRadio->__status.__Flags |= __RADIO_STATUS_ALIVE;
}

__inline void __SetRadioDead(IN OUT PSORA_RADIO pRadio)
{
    pRadio->__status.__Flags &= ~__RADIO_STATUS_ALIVE;
}

SORA_EXTERN_C
HRESULT SoraAllocateRadio2(
        IN PSORA_RADIO_MANAGER    pRadioMgr,
        IN OUT LIST_ENTRY           *pRadios,
        IN ULONG                    nRadio, 
        IN PCWSTR                   userName);


#endif 