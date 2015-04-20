/*++
Copyright (c) Microsoft Corporation

Module Name: Sora Library header file for developers and researchers.

Abstract: This header file defines common consts for Sora library.  

--*/

#pragma once

#include <winerror.h>
#include <guiddef.h>

DEFINE_GUID(GUID_PCIE_INTERFACE,
    0x133b9c58, 0x7988, 0x4bf2, 0xb2, 0x34, 0xfa, 0xad, 0x8a, 0x74, 0xc8, 0xfb);

#define SORA_DEVICE_NAME                    L"\\Device\\SoraHWDevice"
#define SORA_DEVICE_SYMBOL_NAME             L"\\DosDevices\\SoraHWDevice"
#define SORA_DEVICE_UNAME                   "\\\\.\\SoraHWDevice"
#define __SORA_TAG                          ((ULONG)'aroS')

#define __CORE_THREAD_TERMINATED            2

#define FAILED_BREAK(hr)                    { if (FAILED(hr)) break; }

#define E_REG_WRITE_FAIL                    ((HRESULT)0x80050001L)
#define E_REG_WRITE_DEP_FAIL                ((HRESULT)0x80050002L)

#define E_TX_TRANSFER_FAIL                  ((HRESULT)0x80050003L)
#define E_NOT_RCB_MEMORY                    ((HRESULT)0x80050004L)
#define E_NOT_ENOUGH_CONTINUOUS_PHY_MEM     ((HRESULT)0x80050005L)
#define E_UNSUPPORTED_RADIO_NUMBER          ((HRESULT)0x80050006L)
#define E_INVALID_PARAMETER                 ((HRESULT)0x80050007L)
#define E_NOT_ENOUGH_FREE_RADIOS            ((HRESULT)0x80050008L)
#define E_DEVICE_NOT_FOUND                  ((HRESULT)0x80050009L)
#define E_NOT_ENOUGH_RESOURCE               ((HRESULT)0x8005000AL)
#define E_RADIO_NOT_CONFIGURED              ((HRESULT)0x8005000BL)
#define E_RADIO_CANNOT_DESTROY              ((HRESULT)0x8005000CL)
#define E_RADIO_NOT_CONFIGED                ((HRESULT)0x8005000DL)
#define E_FETCH_SIGNAL_FORCE_STOPPED        ((HRESULT)0x8005000EL)
#define E_FETCH_SIGNAL_HW_TIMEOUT           ((HRESULT)0x8005000FL)

#define E_UNKOWN_HARDWARE                   ((HRESULT)0x80050010L)

#define E_BUFFER_NOT_ALIGNED                ((HRESULT)0x80050011L)
#define E_TX_TIMEOUT                        ((HRESULT)0x80050012L)
#define E_NO_TX_SAMPLE_BUFFER                ((HRESULT)0x80050013L)

#define E_SIGNAL_EXISTS                  ((HRESULT)0x80050014L)
// #define E_INVALID_PHY_FRAME_SIZE            ((HRESULT)0x80050015L)
#define E_INVALID_SIGNAL_SIZE            ((HRESULT)0x80050015L)

#define E_HW_DDR2_INIT_FAIL                 ((HRESULT)0x80050016L)

#define E_HW_INCOMPATIBLE                   ((HRESULT)0x80050017L)

#define E_TX_TRANSFER_CHECKSUM_FAIL			((HRESULT)0x80050018L)
#define E_TX_TRANSFER_ADDR21_CHECKSUM_FAIL		((HRESULT)0x80050021L)
#define E_TX_TRANSFER_ADDR23_CHECKSUM_FAIL		((HRESULT)0x80050023L)

#define RX_SLOT_MAX_COUNT                   1
#define RX_SLOT_MIN_COUNT                   1

// New default values set by Kun
#define SORA_RADIO_DEFAULT_TX_GAIN          0x1000 // in 1/256db
#define SORA_RADIO_DEFAULT_RX_GAIN          0x000 // in 1/256db
#define SORA_RADIO_DEFAULT_RX_PA			0x000
#define SORA_RADIO_PRESET1_RX_GAIN          0x000 // in 1/256db

/* base band arithemetic macros */
#define MSP                                 0xE4
#define RX_COMPLEX16_INVALID_BITS           2
#define RAW_SHIFT                           RX_COMPLEX16_INVALID_BITS

#define MACRO_COMMA                         ,
#define DUMMY
#define PUBLIC
#define PRIVATE
#define SORAAPI                             __stdcall
#define FINL                                __forceinline
#define SELECTANY                           __declspec(selectany)

#define MEM_ALIGN(n) __declspec(align(n))

#define A16 __declspec(align(16))
#define A32 __declspec(align(32))

// 0[arr] is identical to arr[0] for arrays but will intentionally fail if it's used against a C++ object that overloads operator[]
#define ITEMSOF(arr)                        (sizeof(arr) / sizeof(0[arr]))

#define A16_MASK                            (0x0F)
#define IS_A16(p)                           (!(((UPOINTER)(p)) & A16_MASK))
#define IS_NOT_A16(p)                       (((UPOINTER)(p)) & A16_MASK)

#define FOREACH(pListHead, pEntryName)      \
    for (pEntryName = (pListHead)->Flink; pEntryName != (pListHead); pEntryName = pEntryName->Flink ) 

// CONCATENATE macro return an identifier name combined by 2 parameter
#define CONCATENATE(arg1, arg2)   CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2)  CONCATENATE2(arg1, arg2)
#define CONCATENATE2(arg1, arg2)  arg1##arg2

#define STATIC_ASSERT(x) CCASSERT(x)
#if _MSC_VER < 1600
#define CCASSERT(predicate) _x_CCASSERT_LINE(predicate, __LINE__)
#define _x_CCASSERT_LINE(predicate, line)   \
    typedef char CONCATENATE(constraint_violated_on_line_, line)[2*((predicate)!=0)-1];

#define static_assert(x, msg) CCASSERT((x))
#else
#define CCASSERT(x) static_assert(x, "static assertion error" );
#endif

#ifndef NDEBUG
// __assume(0) is used for suppress warning C4701 when code analyzer assume the default-branch can be reached.
# define NODEFAULT   (assert(0), __assume(0))
#else
// This tells the optimizer that the default arm in a switch statement
// cannot be reached. As so, it does not have to generate
// the extra code to check that the control variable of the switch statement has a value 
// not represented by a case arm. This may make the switch run faster.
// ref: http://msdn.microsoft.com/en-us/library/1b3fsfxw%28VS.80%29.aspx
# define NODEFAULT   __assume(0)
#endif

#ifndef SORA_EXTERN_C
#ifdef __cplusplus
#define SORA_EXTERN_C extern "C"
#else
#define SORA_EXTERN_C
#endif
#endif

enum
{
    CORE_ANY                                = 0x0f,
    CORE_2_AFFINITY                         = 0x02,
    CORE_3_AFFINITY                         = 0x04,
    CORE_4_AFFINITY                         = 0x08,
    CORE_5_AFFINITY                         = 0x10,
    CORE_6_AFFINITY                         = 0x20,
    CORE_7_AFFINITY                         = 0x40,
    CORE_8_AFFINITY                         = 0x80
};
