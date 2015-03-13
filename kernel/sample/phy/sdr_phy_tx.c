/*++
Copyright (c) Microsoft Corporation

Module Name: sdr_phy_tx.c

Abstract: This file implements interface functions to modulate data into signal samples.
          
History: 
          3/23/2009: Created by yichen
--*/

#include "const.h"
#include "sdr_phy_precomp.h"

A16 COMPLEX8 g_Output[SAMPLE_BUFFER_SIZE];

CCASSERT(BB11B_MAX_SYMBOL_LENGTH < SAMPLE_BUFFER_SIZE)
/* 
SdrPhyModulate11B modulated a packet into transmittable signal samples.

Parameters:
    pRadiosHead: List head for radio list
    pPacket: Pointer to PACKET_BASE structure which contains data need to modulate
            
Return:  

History:   12/5/2009 Created by yichen

IRQL: PASSIVE_LEVEL
*/
HRESULT SdrPhyModulate11B( 
            IN PPHY         pPhy,
            IN PPACKET_BASE pPacket) 
{
    HRESULT     hRes            = S_OK;
    
    BB11BPMDPacketGenSignal(
                   pPacket,
                   &pPhy->BBContextFor11B.TxVector,
                   (PUCHAR)g_Output,
                   sizeof(g_Output)/sizeof(COMPLEX8)
                   );
     return hRes;
}

HRESULT SdrPhyModulate11A( 
            IN PPHY         pPhy,
            IN PPACKET_BASE pPacket) 
{
    return BB11ATxFrameMod(&(pPhy->BBContextFor11A.TxVector), pPacket);
}

