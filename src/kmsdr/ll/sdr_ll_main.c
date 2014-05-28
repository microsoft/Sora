/*++
Copyright (c) Microsoft Corporation

Module Name: sdr_ll_main.c

Abstract: 
    sdr_ll_main.c defines initialization routine for the simple link layer.

History: 
    20/Nov/2009: Created by senxiang
--*/

#include "miniport.h"

VOID SdrLLInitialize(PLL lnk)
{
    lnk->CurSendSeqNo.usValue = 0;
}