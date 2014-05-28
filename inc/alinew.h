#pragma once

#include <assert.h>
#include <stdlib.h>
#include <new.h>

#ifdef USER_MODE
#include <Windows.h>
#else
#ifdef __cplusplus
extern "C"
{
#endif

// Need to be compatiable with sora.h
#define WIN9X_COMPAT_SPINLOCK
#include <ntifs.h>

#include <ndis.h>
#ifdef __cplusplus
}
#endif
#endif

__forceinline void *aligned_malloc(size_t size, size_t align)
{
    assert((align & (align - 1)) == 0); // align must to be a power of two
    size_t mask = align - 1;
    size_t all = size + align + sizeof(size_t) * 2;

    char *ps;
#ifdef USER_MODE
    ps = (char *)malloc(all);
    if (!ps) return ps;
#else
    const ULONG tag = 'SORA';
    NDIS_STATUS rc = NdisAllocateMemoryWithTag((PVOID *)&ps, all, tag);
    if (rc != NDIS_STATUS_SUCCESS) return NULL;
#endif

    char *pt = ps + sizeof(size_t) * 2;
    char *pa = pt + align - ((size_t)pt & mask);    // pa point to first aligned address after pt

    ((size_t *)pa)[-1] = pa - ps; // (pa-ps) bytes are padded before ps, store the number just before pa
    ((size_t *)pa)[-2] = all; // store the total allocated memory size before previous numer

    return pa;
}

__forceinline void aligned_free(const void *pa)
{
    if (!pa) return;

    size_t padding  = ((size_t *)pa)[-1];
    char *ps = (char *)pa - padding;
    
#ifdef USER_MODE
    free(ps);
#else
    size_t all      = ((size_t *)pa)[-2];
    NdisFreeMemory(ps, all, 0);
#endif
}

// template version of aligned_malloc
// syntax suger for specified type T
template<class T>
T *aligned_malloc()
{
    return (T *)aligned_malloc(sizeof(T), __alignof(T));
}

// aligned_new is the combination of aligned memory allocation and calling constructor of type T
// Parameters:
//   Same as the constructor of specified type T
#define aligned_new(T, ...) new (aligned_malloc<T>()) T(__VA_ARGS__)

// Parameter t can point to a derived class of T if the T has a virtual destructor
template<class T>
__forceinline void aligned_delete(T *t)
{
    t->~T();
    aligned_free(t);
}

template<class T>
struct aligned_deleter
{ 
    __forceinline void operator()(T *t)
    { 
        aligned_delete(t);
    } 
};
