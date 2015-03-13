/*++
Copyright (c) Microsoft Corporation

Module Name: Sora hardware registers manager

Abstract: This header file defines structs, macros, interfaces to access the on-board 
          control registers easily. 

--*/

#ifndef _SORA_REGMAN_
#define _SORA_REGMAN_
#pragma once

#include "_WARP_Regs.h"

#define MAX_RADIO_NUMBER                    8

typedef PVOID SORA_REGS_HANDLE;

typedef struct __SORA_RADIO_REGS *__PSORA_RADIO_REGS;

typedef struct ___HW_REGISTER_FILE{
    PVOID                   pSoraSysRegs;
    __PSORA_RADIO_REGS      pRadioRegs;

}__HW_REGISTER_FILE, *__PHW_REGISTER_FILE;

#endif 