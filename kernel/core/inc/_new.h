#pragma once

#ifndef _NEW_DELETE_OPERATORS_
#define _NEW_DELETE_OPERATORS_

inline 
PVOID 
operator 
new(
    size_t          iSize,
    POOL_TYPE       poolType) {
    
    PVOID result = ExAllocatePoolWithTag(poolType,iSize,'wNCK');

    if (result)
        RtlZeroMemory(result,iSize);

    return result;
}

inline 
PVOID 
operator 
new(
    size_t          iSize,
    POOL_TYPE       poolType,
    ULONG           tag) {
    
    PVOID result = ExAllocatePoolWithTag(poolType,iSize,tag);

    if (result)
        RtlZeroMemory(result,iSize);

    return result;
}

inline 
void 
__cdecl 
operator 
delete(
    PVOID pVoid) {
    
    if (pVoid)
        ExFreePool(pVoid);
}

#endif


