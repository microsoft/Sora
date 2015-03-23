#include "sora.h"
#include "__tx_res.h"
#include "__transfer_obj.h"

//
//    Initialize a PACKET_BASE
//
VOID SoraPacketInitialize (OUT PPACKET_BASE pPacket)
{
    RtlZeroMemory(pPacket, sizeof(PACKET_BASE));
}

//
//    Cleanup a PACKET_BASE 
//
VOID SORAAPI SoraPacketCleanup (OUT PPACKET_BASE pPacket)
{
    RtlZeroMemory(pPacket, sizeof(PACKET_BASE));
}

/*++
SoraPacketFreeTxResource free TX resource occupied by the packet. 
It should be called after the packet has been consumed.

Parameter: 
            pRadio : The radio object.
            pPacket: the packet.

Note:   
            IRQL <= DISPATCH_LEVEL.

--*/
VOID 
SORAAPI 
SoraPacketFreeTxResource( IN HANDLE TransferObj, 
                          IN PPACKET_BASE pPacket)
{
	PTRANSFER_OBJ pTransferObj = (PTRANSFER_OBJ)TransferObj;

    if (pPacket->pTxDesc)
    {
        if (pPacket->pTxDesc->pRMD)
        {
            SoraFreeRCBMem ( pTransferObj->TransferResMgr, 
                             pPacket->pTxDesc->pRMD);
            
            pPacket->pTxDesc->pRMD = NULL;
        }

        //
        // N.B. for current version - a Tx descriptor always uses
        // a global shared sample buffer.
        //
        pPacket->pTxDesc->pSampleBuffer = NULL;

        SoraFreeTxDesc(pTransferObj->TransferResMgr, pPacket->pTxDesc);        
        pPacket->pTxDesc = NULL;
    }
}

/*++
SoraPacketGetTxResource obtains a Tx descriptor and a pointer to a sample
buffer for modulated packet samples.

Parameters: 
            pRadio : the radio object
            pPacket: the packet

Return: E_NOT_RCB_MEMORY if failed.

Note:   It must be called before SdrPhyModulate.
        IRQL <= DISPATCH_LEVEL. SoraPacketGetTxResource is not thread safe. 
        Modulate thread must be serialized.

--*/
HRESULT 
SORAAPI 
SoraPacketGetTxResource ( IN HANDLE TransferObj, 
                          IN OUT PPACKET_BASE pPacket)
{
    HRESULT hr = E_NOT_RCB_MEMORY;
    PTX_DESC pTxDesc;
	PTRANSFER_OBJ pTransferObj = (PTRANSFER_OBJ)TransferObj;

    pPacket->fStatus     = PACKET_NOT_MOD ;
    
    pTxDesc = SoraAllocateTxDesc(pTransferObj->TransferResMgr);
    
    if (pTxDesc)
    {
	    pPacket->pTxDesc          = pTxDesc;
	    hr = S_OK;    

        if (pTransferObj->__TxSampleBufVa)
        {
            // the Tx descriptor uses the radio sample buffer
            pTxDesc->__SourceAddressLo   = pTransferObj->__TxSampleBufPa.u.LowPart;
            pTxDesc->__SourceAddressHi   = pTransferObj->__TxSampleBufPa.u.HighPart;
            pTxDesc->pSampleBuffer       = pTransferObj->__TxSampleBufVa;
            pTxDesc->SampleBufferSize    = pTransferObj->__TxSampleBufSize;

            pPacket->pTxDesc          = pTxDesc;
            hr = S_OK;
        }
        else
        {
            SoraFreeTxDesc(pTransferObj->TransferResMgr, pTxDesc);
            hr = E_NO_TX_SAMPLE_BUFFER;
        }
    }
    return hr ;
}

//
// SoraPacketSetSignalSize set the size of the packet signal
//
// Note: The packet signal size should be determined after proper modulation
// has been done after a packet gets its Tx resources.
//

#ifdef USER_MODE
//
// Kun: suspicious code - double check later
//
VOID  
SORAAPI
SoraPacketSetSignalLength(
    IN OUT PPACKET_BASE pPacket, 
    IN ULONG            Size)
{
    pPacket->Reserved3 = Size;
}

ULONG
SORAAPI
SoraPacketGetSignalSize(
    IN OUT PPACKET_BASE pPacket)
{
    return pPacket->Reserved3;
}
#else
// Kernel mode
VOID  
SORAAPI
SoraPacketSetSignalLength(
    IN OUT PPACKET_BASE pPacket, 
    IN ULONG            Size)
{
    ASSERT(pPacket->pTxDesc);
    pPacket->pTxDesc->Size = Size;
}

ULONG
SORAAPI
SoraPacketGetSignalSize(
    IN OUT PPACKET_BASE pPacket)
{
    ASSERT(pPacket->pTxDesc);
    return pPacket->pTxDesc->Size;
}
#endif

VOID 
SORAAPI 
SoraPacketSetTimeStamp(
    IN OUT PPACKET_BASE pPacket,
    ULONG               TimeStamp)
{
/*
    PTX_DESC p = pPacket->pTxDesc;

    p->__TimeStamp = TimeStamp;
    p = p->__NextDesc;
    */
    
    pPacket->pTxDesc->__TimeStamp = TimeStamp;

}


#ifdef USER_MODE
VOID
SORAAPI
SoraPacketGetTxSampleBuffer(
    IN  PPACKET_BASE pPacket, 
    OUT PTXSAMPLE  *ppBuffer,
    OUT PULONG      pBufferSize)
{
    *ppBuffer    = (PTXSAMPLE)pPacket->pReserved;
    *pBufferSize = pPacket->Reserved2;
}
#else
VOID
SORAAPI
SoraPacketGetTxSampleBuffer(
    IN PPACKET_BASE pPacket, 
    OUT PTXSAMPLE   *ppBuffer,
    OUT PULONG      pBufferSize)
{
    *ppBuffer = NULL;
    *pBufferSize = 0;
    if (pPacket->pTxDesc)
    {
        *ppBuffer       = (PTXSAMPLE)pPacket->pTxDesc->pSampleBuffer;
        *pBufferSize    = pPacket->pTxDesc->SampleBufferSize;
    }
}

void SORAAPI SoraPacketAssert(IN PPACKET_BASE pPacket, IN HANDLE TransferObj)
{
        if ((pPacket->pTxDesc->__Delimiter != 0xcdcdcdcd) ||
            (pPacket->pTxDesc->__StatusValid != 0x01) ||
            (pPacket->pTxDesc->__MoreDesc != 0) ||
            (pPacket->pTxDesc->__Reserved != 0x0c))
        {
            DbgPrint("[Error] BufPa=%08x, Size=%08x,watchdog=%08x,SV=%01x,MF=%01x,R=%04x\n", 
                pPacket->pTxDesc->__SourceAddressLo,
                pPacket->pTxDesc->Size, 
                pPacket->pTxDesc->__Delimiter, 
                pPacket->pTxDesc->__StatusValid,
                pPacket->pTxDesc->__MoreDesc, 
                pPacket->pTxDesc->__Reserved
                );
			{
				PTRANSFER_OBJ pTransferObj = (PTRANSFER_OBJ)TransferObj;
            	if (pTransferObj)
	                SoraHwPrintDbgRegs(pTransferObj);
            }
        }
}

VOID SORAAPI SoraPacketPrint(IN PPACKET_BASE pPacket)
{
    DbgPrint("[Error] BufPa=%08x,Size=%08x,watchdog=%08x,SV=%01x,MF=%01x,R=%04x\n", 
        pPacket->pTxDesc->__SourceAddressLo,
        pPacket->pTxDesc->Size, 
        pPacket->pTxDesc->__Delimiter, 
        pPacket->pTxDesc->__StatusValid,
        pPacket->pTxDesc->__MoreDesc, 
        pPacket->pTxDesc->__Reserved
        );
}

#endif
