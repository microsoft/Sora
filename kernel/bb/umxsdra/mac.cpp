#include "umxsdr.h"
#include "phy.h"
#include "mac.h"
#include "mgmt.h"

bool ProcessDot11Frame(ULONG RadioNo, UCHAR *frameBuf, ULONG frameBufSize );

// MAC states
#pragma warning(disable:4345)

BSSID				bssid = {0xB6, 0xDB, 0x71, 0x64, 0x51, 0x75};
BSSID		        adhoc_bssid = {0xB6, 0xDB, 0x71, 0x64, 0x51, 0x75};

char                adhoc_ssid[] = "sdradhoc";
UINT                adhoc_ssid_len = 8;

char                ssid  [64];
UINT                ssid_len = 0;

MAC_ADDRESS			MACAddress = { 0 };
MAC_STATE 			current_state = MAC_STATE_CS;

MAC2TxID*			Root = NULL;
struct PACKETxID*	LastPACKETxID;

LONG				SendListLock = 0;
LIST_ENTRY			SendListHead;

USHORT				SendSeQ = 0;

PVOID				AckBuffer = NULL; 
ULONG 				AckSize = _M(1); // 1MB

UCHAR				FrameBuffer[FrameBufferSize];

// Handler pointer
BOOLEAN MAC11a_CarrierSense   (void*);
BOOLEAN MAC11a_Send           (void*);
BOOLEAN MAC11a_Receive        (void*);

static MAC_STATE_HANDLER* STATE[] = {
	MAC11a_Receive,
	MAC11a_Send,
	MAC11a_CarrierSense
};

const ULONG RateMap11a[] = { 
	48, // DOT11A_RATE_48M	000
	24, // DOT11A_RATE_24M	001
	12, // DOT11A_RATE_12M	010
	6,  // DOT11A_RATE_6M	011
	54, // DOT11A_RATE_54M	100
	36, // DOT11A_RATE_36M	101
	18, // DOT11A_RATE_18M	110
	9   // DOT11A_RATE_9M	111
};

FINL
HRESULT ModulateACK11 ( MAC_ADDRESS Address, PVOID AckBuffer, ULONG AckSize, ULONG* sample_size) 
{
	DOT11_MAC_ACK_FRAME AckFrame	= {0};
	AckFrame.FrameControl.Subtype	= SUBT_ACK;
	AckFrame.FrameControl.Type		= FRAME_CTRL;
	AckFrame.RecvAddress			= Address;
	AckFrame.Duration				= 0;

    bool bRet;
	bRet = ModBuffer11a ((uchar*)&AckFrame, sizeof(AckFrame)-4, NULL, 0,
								(ushort) gDataRateK, 
								(COMPLEX8*)AckBuffer, AckSize, sample_size);

	if ( bRet ) return S_OK;
	else return E_FAIL;
}

FINL
HRESULT DataFrameAck11(PDOT11RFC1042ENCAP pWlanHeader, ULONG frameBufSize) {
	HRESULT hr;
	hr = E_FAIL;
	MAC2TxID* find;
	ULONG sample_size;
	
	find = MAC2TxID_Find(Root, pWlanHeader->MacHeader.Address2);	
	if (!find) {
		hr = ModulateACK11 (pWlanHeader->MacHeader.Address2, AckBuffer, AckSize, &sample_size);
		
		if (SUCCEEDED(hr)) {
			ULONG TxID;
			hr = SoraURadioTransferEx(TARGET_RADIO, AckBuffer, sample_size, &TxID);
			if (SUCCEEDED(hr)) {
				find = MAC2TxID_Chain(&Root, pWlanHeader->MacHeader.Address2, TxID);
			}	
			
			/*printf ( "ack modulated %d - %d (%02x:%02x:%02x:%02x:%02x:%02x)\n", 
				sample_size,
				TxID,
				pWlanHeader->MacHeader.Address2.Address[0], 
				pWlanHeader->MacHeader.Address2.Address[1],
				pWlanHeader->MacHeader.Address2.Address[2], 
				pWlanHeader->MacHeader.Address2.Address[3],
				pWlanHeader->MacHeader.Address2.Address[4], 
				pWlanHeader->MacHeader.Address2.Address[5]	);
			*/
		}
	}else {
		hr = SoraURadioTx(TARGET_RADIO, find->m_txid);
	}

	return hr;
}

BOOLEAN MAC11a_CarrierSense (void*) {
	HRESULT hr;

	hr = BB11ARxCarrierSense(&RxContext, &RxStream);

	switch (hr) {
	case BB11A_CHANNEL_CLEAN:
		current_state = MAC_STATE_TX;
		break;
	case E_FETCH_SIGNAL_HW_TIMEOUT:
		current_state = MAC_STATE_CS;
		break;
	case BB11A_E_PD_LAG:
		current_state = MAC_STATE_CS;
		break;
	case BB11A_OK_POWER_DETECTED:
		current_state = MAC_STATE_RX;
		break;
	}
	
	return TRUE;
}

BOOLEAN MAC11a_Receive (void*) {
	HRESULT hr;
	
	hr = BB11ARxFrameDemod(&RxContext, &RxStream);

	if (hr == BB11A_OK_FRAME) {
		ProcessDot11Frame(TARGET_RADIO, FrameBuffer, RxContext.ri_uiFrameSize );
	} else {
		switch (hr) {
		case BB11A_E_SYNC_FAIL:
			err_stat[0] ++;
			break;
		case BB11A_E_INVALID_SIG:
			err_stat[1] ++;
			break;
		case BB11A_E_CRC32:
			err_stat[2] ++;
			break;
		}
		
	}

	current_state = MAC_STATE_CS;
	return TRUE;
}

FINL
void MACCompletePacket ( PACKET_HANDLE pkt, HRESULT status )
{
	if ( pkt ) {
		SoraUCompleteTxPacket(pkt, status ); 
	}

}

BOOLEAN MAC11a_Send (void*) {
	struct PACKETxID* ptid = NULL;

	if (LastPACKETxID) {
		ptid = LastPACKETxID;
		LastPACKETxID = NULL;
	}
	else {
		LIST_ENTRY* entry;
		remove_list_head( 	&SendListHead, 
							entry, 
							&SendListLock);
		if (entry != &SendListHead)
			ptid = (struct PACKETxID*)entry;
	}
	
	if (ptid) {
		switch(ptid->m_status) {
		case PACKET_CAN_TX: {
				if (ptid->m_retry > PACKET_MAX_RETRY) {
					nDataPacketDropCnt  ++;
					MACCompletePacket (ptid->m_packet, STATUS_UNSUCCESSFUL);
					break;
				}
				
				HRESULT hr;
				hr = SoraURadioTx(TARGET_RADIO, ptid->m_txid);
				
				ptid->m_retry++;
				
				if (hr != S_OK) {
					split_printf("SoraURadioTx hr: [0x%08x]\n", hr);
					current_state = MAC_STATE_CS;
					
					MACCompletePacket (ptid->m_packet, STATUS_UNSUCCESSFUL);

					// HW Error - No need retransmit again 
					LastPACKETxID = NULL;
				} else {
					if (ptid->m_needack) {
						LastPACKETxID = ptid;
						ptid = NULL;	// do not free the frame
					}
					else {
						nDataPacketSndCnt ++;
						MACCompletePacket ( ptid->m_packet, STATUS_SUCCESS );
					}
				}
			}
			break;
		default:
			MACCompletePacket (ptid->m_packet, STATUS_UNSUCCESSFUL);
			nDataPacketDropCnt ++;
			break;
		}
		
		if (ptid) {
			switch(ptid->m_status) {
			case PACKET_CAN_TX:
				SoraURadioTxFree(TARGET_RADIO, ptid->m_txid);
				break;
			default:
				break;
			}
			free(ptid);
		}			
	}	
	
	current_state = MAC_STATE_CS;
	return TRUE;
}

//
// Send thread
// It gets a packet from NDIS network stack, modulates it and
// transfer the modulated signal into RCB.
//
UCHAR databuf[10*1024];


bool ProcessMgmtPktSendRequest ( int mgmtMask )
{
	if ( mgmtMask & MGMT_PKT_BEACON ) {
		Beacon ();
	}
	
	if ( mgmtMask & MGMT_PKT_AUTH ) {
		Auth ();
	} 
	
	if ( mgmtMask & MGMT_PKT_ASSO) {
		Associate ();
	} 
	
	if ( mgmtMask & MGMT_PKT_TEST) {
		TestPkt ();
	} 
	return true;
}

bool Encapsulate80211Frame ( struct PACKETxID* ptid, 
							 DOT11RFC1042ENCAP& wheader, 
							 UCHAR * addr, int len ) 
{
	bool bRet = true;
	
//	DOT11RFC1042ENCAP wheader = { 0 };
	PETHERNET_HEADER  eheader;
	eheader = (PETHERNET_HEADER)addr;

	// Three address format with ap mode
	if ( OpMode == CLIENT_MODE ) {
		wheader.MacHeader.Address1 = bssid;
		wheader.MacHeader.Address2 = eheader->srcAddr;
		wheader.MacHeader.Address3 = eheader->destAddr;
		wheader.MacHeader.FrameControl.Type    = FRAME_DATA;
		wheader.MacHeader.FrameControl.Subtype = SUBT_DATA;
		wheader.MacHeader.FrameControl.ToDS  = 1;	// to DS
		wheader.MacHeader.FrameControl.Retry = 1;
	} else {
		wheader.MacHeader.Address1 = eheader->destAddr;
		wheader.MacHeader.Address2 = eheader->srcAddr;
		wheader.MacHeader.Address3 = adhoc_bssid;
		wheader.MacHeader.FrameControl.Type = FRAME_DATA;
		wheader.MacHeader.FrameControl.Subtype = SUBT_DATA;
		wheader.MacHeader.FrameControl.Retry = 1;
	}
	//		wheader.MacHeader.DurationID = 20 + (8*(len + 4)*1000 / gDataRateK); 
	
	// Kun: SequenceControl includes fragment seq
	//
	wheader.MacHeader.SequenceControl.usValue = (SendSeQ << 4);
	wheader.SNAP.DSAP = 0xAA;
	wheader.SNAP.SSAP = 0xAA;
	wheader.SNAP.Control = 0x03;
	wheader.Type = eheader->Type;
	
	SendSeQ++;
		
	ptid->m_status = PACKET_NOT_MOD;
	ptid->m_needack = !(ETH_IS_BROADCAST(eheader->destAddr.Address) || 
						ETH_IS_MULTICAST(eheader->destAddr.Address));
	
	return bRet;
}

BOOLEAN Dot11aSendProc (void*) {

	PACKET_HANDLE packet;
	PUCHAR addr;
	UINT len;
	HRESULT hr;
	bool bRet;
	ulong sample_size;
	DOT11RFC1042ENCAP wheader = { 0 };

	if ( mgmtPkt ) {
		ULONG mp = 0;
		mp = InterlockedExchange ( (LONG*)&mgmtPkt, 0 );
		return ProcessMgmtPktSendRequest(mp);
	}
	
	if ( fKeepAlive == 1 ) {
		KeepAlive ();
		fKeepAlive = 0;
	}

	hr = SoraUGetTxPacket(&packet, (VOID**)&addr, &len, 10);
	if (hr == S_OK) {

		struct PACKETxID* ptid;
		ptid = (struct PACKETxID*)malloc(sizeof(struct PACKETxID));
		memset(ptid, 0, sizeof(struct PACKETxID));
		ptid->m_packet = packet;

		bRet = Encapsulate80211Frame (ptid, wheader, addr, len );		

		bRet = ModBuffer11a ( (UCHAR*)&wheader, sizeof(DOT11RFC1042ENCAP), 
				addr + sizeof(ETHERNET_HEADER), len - sizeof(ETHERNET_HEADER), 
				(ushort) gDataRateK, 
				(COMPLEX8*)SampleBuffer, SampleSize, &sample_size );

		if ( bRet ) {
			// Frame is modulated
			ULONG TxID;
			hr = SoraURadioTransferEx(TARGET_RADIO, SampleBuffer, sample_size , &TxID);
			if (SUCCEEDED(hr)) {
				ptid->m_txid = TxID;
				ptid->m_status = PACKET_CAN_TX;

				insert_list_tail(&SendListHead, 
					&ptid->m_entry, 
					&SendListLock);
			}
			else {
				printf("SoraURadioTransferEx hr: [0x%08x]\n", hr);
				
				ptid->m_status = PACKET_TF_FAIL;
				debug_printf("Queued Send Packet PACKET_TF_FAIL: [%08x]\n", 
					ptid->m_packet);
				insert_list_tail(   &SendListHead, 
	 								&ptid->m_entry, 
	 								&SendListLock  );
			}
		}
		else {
			printf("Modulation failure\n");

			ptid->m_status = PACKET_NOT_MOD;
			debug_printf("Queued Send Packet PACKET_NOT_MOD: [%08x]\n", 
				ptid->m_packet);
			insert_list_tail(&SendListHead,
				&ptid->m_entry,
				&SendListLock);
		}

	}
	else
	if (hr == ERROR_CANCELLED)
		return FALSE;
	
	return TRUE;
}

BOOLEAN Dot11aRecvProc (void* args) {
	return STATE[current_state](args);
}

bool ProcessDot11Frame(ULONG RadioNo, UCHAR *frameBuf, ULONG frameBufSize )
{
	
	PDOT11RFC1042ENCAP pWlanHeader = (PDOT11RFC1042ENCAP)frameBuf;
		
	if (pWlanHeader->MacHeader.FrameControl.Type    == FRAME_DATA && 
		pWlanHeader->MacHeader.FrameControl.Subtype == SUBT_DATA ) 
	{
		static USHORT CurRecvSeqNo = 0xffff;

		int fTargetFrame = 0;
		// Try to send some ACK
		if ( memcmp( MACAddress.Address, 
					 pWlanHeader->MacHeader.Address1.Address, 
					 MAC_ADDRESS_LENGTH) == 0) {

			DataFrameAck11(pWlanHeader, frameBufSize);
			
			fTargetFrame = 1;
		}
		
		if ( fTargetFrame ||
			 ETH_IS_BROADCAST (pWlanHeader->MacHeader.Address1.Address) )
		{
			lastRate = RateMap11a[RxContext.bRate &7]*1000 ;
			
			nDataPacketRcvCnt ++;
			
	        if (CurRecvSeqNo  != pWlanHeader->MacHeader.SequenceControl.SequenceNumber 
				&& pWlanHeader->MacHeader.SequenceControl.FragmentNumber == 0 ) {			
				if ( frameBufSize < sizeof(DOT11RFC1042ENCAP)) {
					return S_OK;
				}
				
				CurRecvSeqNo   = pWlanHeader->MacHeader.SequenceControl.SequenceNumber;
				MAC_ADDRESS			destAddr;
				MAC_ADDRESS			srcAddr;
				UINT16				Type;
				ULONG				Offset;
				PETHERNET_HEADER	pEthernetHeader;

				/* ad hoc mode
				destAddr = pWlanHeader->MacHeader.Address1;
				srcAddr  = pWlanHeader->MacHeader.Address2;


				lastMACDst   = pWlanHeader->MacHeader.Address1;
				lastMACSrc   = pWlanHeader->MacHeader.Address2;
				lastMACBssid = pWlanHeader->MacHeader.Address3;
				*/

				/* AP mode */
				if ( OpMode == CLIENT_MODE ) {
					destAddr = pWlanHeader->MacHeader.Address1;
					srcAddr  = pWlanHeader->MacHeader.Address3;
					Type     = pWlanHeader->Type;

					lastMACDst   = pWlanHeader->MacHeader.Address1;
					lastMACSrc   = pWlanHeader->MacHeader.Address3;
					lastMACBssid = pWlanHeader->MacHeader.Address2;
				} else {
					destAddr = pWlanHeader->MacHeader.Address1;
					srcAddr  = pWlanHeader->MacHeader.Address2;
					Type     = pWlanHeader->Type;					
					
					lastMACDst	 = pWlanHeader->MacHeader.Address1;
					lastMACSrc	 = pWlanHeader->MacHeader.Address2;
					lastMACBssid = pWlanHeader->MacHeader.Address3;

				}
		

				Offset   = sizeof(DOT11RFC1042ENCAP) - sizeof(ETHERNET_HEADER);
				pEthernetHeader = (PETHERNET_HEADER)(frameBuf + Offset);
				pEthernetHeader->destAddr = destAddr;
				pEthernetHeader->srcAddr = srcAddr;
				pEthernetHeader->Type = Type;

				
				HRESULT hr;
				hr = SoraUIndicateRxPacket(frameBuf + Offset, frameBufSize - Offset - 4 ); // remove FCS

				UpdateEnergy ();
				return SUCCEEDED(hr);
			}
		}
		else {
			nInterferenceCnt ++;
		}
	}
	else
	if (pWlanHeader->MacHeader.FrameControl.Type == FRAME_CTRL &&
		pWlanHeader->MacHeader.FrameControl.Subtype == SUBT_ACK) {
		
		if (LastPACKETxID) {
			if (memcmp(MACAddress.Address, 
				pWlanHeader->MacHeader.Address1.Address,
				MAC_ADDRESS_LENGTH))
				return true;

			struct PACKETxID* ptid;
			ptid = LastPACKETxID;
			LastPACKETxID = NULL;

			nAckRcvCnt ++;
			
			if ( ptid->m_packet ) {
				SoraUCompleteTxPacket(ptid->m_packet, 
					STATUS_SUCCESS);
			}
			
			debug_printf("Complete Send Packet: [%08x], TxID: [%d], TX succeed\n", 
				ptid->m_packet, 
				ptid->m_txid);
			
			switch(ptid->m_status) {
			case PACKET_CAN_TX:
				SoraURadioTxFree(TARGET_RADIO, ptid->m_txid);
				break;
			default:
				break;
			}
			free(ptid);
		}
	}
	else
	if (pWlanHeader->MacHeader.FrameControl.Type    == FRAME_MAN &&
		pWlanHeader->MacHeader.FrameControl.Subtype == SUBT_BEACON)
	{
		ProcessBeacon ((PDOT11_MAC_BEACON)pWlanHeader);
	}
	else 
	if (pWlanHeader->MacHeader.FrameControl.Type    == FRAME_MAN )
	{
		if ( memcmp( MACAddress.Address, pWlanHeader->MacHeader.Address1.Address, MAC_ADDRESS_LENGTH) == 0 )
		{
			if ( pWlanHeader->MacHeader.FrameControl.Subtype == SUBT_AUTH ) // auth
			{
				DataFrameAck11 ( pWlanHeader, frameBufSize );

				if ( assoState == ASSO_NONE ) {
					assoState = ASSO_AUTH;
				}
			} else if ( pWlanHeader->MacHeader.FrameControl.Subtype == SUBT_ASSO ) // asso
			{
				DataFrameAck11 ( pWlanHeader, frameBufSize );

				if ( assoState == ASSO_AUTH || 
					 assoState == ASSO_NONE 
				) {
					assoState = ASSO_DONE;
				}
			} else if ( pWlanHeader->MacHeader.FrameControl.Subtype == SUBT_DEASSO || // deassociate
					    pWlanHeader->MacHeader.FrameControl.Subtype == SUBT_DEAUTH )  // deauth
			{
				assoState = ASSO_NONE;  
			}
		}	
	}

	return true;
}
