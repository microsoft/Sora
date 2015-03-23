#include "umxsdr.h"
#include "phy.h"
#include "mac.h"
#include "mgmt.h"
#include "fb11ademod_config.hpp"
#include "diagnostics.h"

SoraStopwatch swatch  (false);


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
MAC_STATE 			current_state = MAC_STATE_RX;

MAC2TxID*			Root = NULL;
struct PACKETxID*	LastPACKETxID;

LONG				SendListLock = 0;
LIST_ENTRY			SendListHead;

USHORT				SendSeQ = 0;

PVOID				AckBuffer = NULL; 
ULONG 				AckSize = _M(1); // 1MB

UCHAR				FrameBuffer[FrameBufferSize];

const int           nDIFS = 12;
const int           nACKTimeout = 60;
const int           nBackoffWnd = 24;
uint                nBackoffCounter = 0;


// Handler pointer
BOOLEAN MAC11a_Receive        (void*);

static MAC_STATE_HANDLER* STATE_11A[] = {
	MAC11a_Receive,
	MAC11a_Receive,
};

// 802.11b Brick Handler pointer
BOOLEAN MAC11b_Receive (void*);

static MAC_STATE_HANDLER* STATE_11B[] = {
	MAC11b_Receive,
	MAC11b_Receive,
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
	if ( gPHYMode == PHY_802_11A ) {
	    bRet = ModBuffer11a ((uchar*)&AckFrame, sizeof(AckFrame)-4, NULL, 0,
								    (ushort) gDataRateK, 
								    (COMPLEX8*)AckBuffer, AckSize, sample_size);
		PlotText ( "ack event", "ACK crc %8X", BB11aModCtx.CF_11aTxVector::crc32() );
    } else {
	    // printf ( "modulate 11b ack size %d\n", sizeof(AckFrame)-4 );
	    bRet = ModBuffer11b ((uchar*)&AckFrame, sizeof(AckFrame)-4, NULL, 0,
								    (ushort) gDataRateK, 
								    (COMPLEX8*)AckBuffer, AckSize, sample_size);
		PlotText ( "ack event", "ACK crc %8X", BB11bModCtx.CF_11bTxVector::crc32() );
    }

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
			PlotText ( "mac event", "missing ack cache......................" );

		}
	}else {
		hr = SoraURadioTx(TARGET_RADIO, find->m_txid);
	}

	PlotText ( "mac event", "Send ACK to (%02x:%02x:%02x:%02x:%02x:%02x)\n", 
				pWlanHeader->MacHeader.Address2.Address[0], 
				pWlanHeader->MacHeader.Address2.Address[1],
				pWlanHeader->MacHeader.Address2.Address[2], 
				pWlanHeader->MacHeader.Address2.Address[3],
				pWlanHeader->MacHeader.Address2.Address[4], 
				pWlanHeader->MacHeader.Address2.Address[5] );
	return hr;
}

BOOLEAN ViterbiThread ( void * ) {
	pBB11aViterbi->Process ();
    return TRUE;
}

FINL ULONG CalcDecodeTime (LONGLONG SFDFindTimeStamp)
{
    LARGE_INTEGER   Freq, End;    
    ULONG           DecodeTime;
    
    QueryPerformanceFrequency(&Freq);
    QueryPerformanceCounter(&End);

	DecodeTime = (ULONG)((End.QuadPart - SFDFindTimeStamp) * 1000 * 1000 / Freq.QuadPart);

    return DecodeTime;
}

BOOLEAN MAC11a_Receive (void*) {

	uint nWaitCounter;
	if ( current_state == MAC_STATE_RX) {
		nWaitCounter = nDIFS + nBackoffCounter;
	} else { 
		// we are waiting for an ACK
		nWaitCounter = nACKTimeout;
	}

	while (1) {
		pBB11aRxSource->Process ();
		ulong err = BB11aDemodCtx.CF_Error::error_code();
	
		if ( err != E_ERROR_SUCCESS ) {
			// Any event happened
            if ( err == E_ERROR_FRAME_OK ) {
				// One frame is received
				//ProcessDot11Frame(TARGET_RADIO, FrameBuffer, BB11aDemodCtx.CF_11aRxVector::frame_length());
                nDataPacketRcvCnt ++;
			    TraceOutput ( "mac event: receive a packet - size (%u) rate (%u) expect (%u) samplerate (%u)",
				    BB11aDemodCtx.CF_11aRxVector::frame_length(),
			        (gPHYMode == PHY_802_11A  ? BB11aDemodCtx.GetDataRate() : BB11bDemodCtx.GetDataRate()),
                    BB11aDemodCtx.CF_11aRxVector::total_symbols() * 4 + 16, // 16 == LTS + STS
                    gSampleRate
			    );

            } else if ( err == E_ERROR_CRC32_FAIL) {
                TraceOutput ( "mac event: crc error" );
                err_stat[2] ++;
            } else if ( err == E_ERROR_PLCP_HEADER_FAIL) {
                TraceOutput ( "mac event: PLCP header fail" );
                err_stat[1] ++;
			} else if ( err == E_ERROR_CS_TIMEOUT ) {
				// Channel clean
                BB11aDemodCtx.ResetCarrierSense();
		        pBB11aCarrierSense->Reset ();

				if ( nWaitCounter > 0) {
					nWaitCounter --; // just backoff
					continue;
				} else {
					current_state = MAC_STATE_RX;
                    break;
				}

	        } else {
                TraceOutput ( "mac event: other error" );
            }

            // Flush the brick graph, make related brick threads safe to reset
            pBB11aRxSource->Flush();

            //
			// Reset context
			BB11aDemodCtx.Reset ();
			pBB11aRxSource->Reset ();

            if (   err == E_ERROR_FRAME_OK
                || err == E_ERROR_CRC32_FAIL
                || err == E_ERROR_PLCP_HEADER_FAIL)
            {
				// seek to the rx stream end
				int nMove = pBB11aRxSource->Seek(ISource::END_POS);

				// to do - if HEADER fails, we need to advance an entire frame.
				//
				// We have received anything
				// 
				if ( current_state == MAC_STATE_WAITACK ) {
					current_state = MAC_STATE_RX;

					nBackoffCounter = nBackoffCounter << 1; // a simple way for BEB

					break;
				} else {
					// we have paused during backoff, continue
					
					if ( nWaitCounter < (uint) nBackoffCounter ) 
						nBackoffCounter = nWaitCounter;

					nWaitCounter = nDIFS + nBackoffCounter;

					continue;
				}
            } else {
				PlotText ( "mac event", "error not fail %X", err );
			}

	    } else {
			nIdleCnt ++;
		}
	}

    return TRUE;
}

FINL
void MACCompletePacket ( PACKET_HANDLE pkt, HRESULT status )
{
	if ( pkt ) {
		SoraUCompleteTxPacket(pkt, status ); 
	}
}


int nMove, nMove1, nMove2;

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
	return STATE_11A[current_state](args);
}

BOOLEAN MAC11b_Receive (void*) {

    uint nWaitCounter;
	if ( current_state == MAC_STATE_RX) {
		nWaitCounter = nDIFS + nBackoffCounter;
	} else { 
		// we are waiting for an ACK
		nWaitCounter = nACKTimeout;
	}
	
	while (1) {
		pBB11bRxSource->Process ();
		ulong err = BB11bDemodCtx.CF_Error::error_code();

		if ( err != E_ERROR_SUCCESS ) {
			// Any event happened
			if (  err == E_ERROR_FRAME_OK ) {
				// One frame is received
				ProcessDot11Frame(TARGET_RADIO, FrameBuffer, BB11bDemodCtx.CF_11bRxVector::frame_length());
			} else if ( err == E_ERROR_CS_TIMEOUT || err == E_ERROR_SYNC_TIMEOUT ) {
				// Channel clean
			    BB11bDemodCtx.reset ();
			    pBB11bRxSource->Reset ();

                if ( nWaitCounter > 0) {
					nWaitCounter --; // just backoff
					continue;
				} else {
					current_state = MAC_STATE_RX;
                    break;
				}
			}

			//
			// Reset context
			BB11bDemodCtx.reset ();
			pBB11bRxSource->Reset ();

            if (   err == E_ERROR_FRAME_OK
                || err == E_ERROR_CRC32_FAIL
                || err == E_ERROR_PLCP_HEADER_FAIL)
            {
				// seek to the rx stream end
				int nMove = pBB11bRxSource->Seek(ISource::END_POS);

				// to do - if HEADER fails, we need to advance an entire frame.
				//
				// We have received anything
				// 
				if ( current_state == MAC_STATE_WAITACK ) {
					current_state = MAC_STATE_RX;

					nBackoffCounter = nBackoffCounter << 1; // a simple way for BEB

					break;
				} else {
					// we have paused during backoff, continue
					
					if ( nWaitCounter < (uint) nBackoffCounter ) 
						nBackoffCounter = nWaitCounter;

					nWaitCounter = nDIFS + nBackoffCounter;

					continue;
				}
            } else {
				PlotText ( "mac event", "error not fail %X", err );
			}
        } else {
			nIdleCnt ++;
		}
	}	
    return TRUE;
}

BOOLEAN Dot11bRecvProc (void* args) {
	return STATE_11B[current_state](args);
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
		if ( (memcmp( MACAddress.Address, 
					 pWlanHeader->MacHeader.Address1.Address, 
					 MAC_ADDRESS_LENGTH) == 0) )
		{
			DataFrameAck11(pWlanHeader, frameBufSize);

			PlotText ( "mac event", "receive a packet - seq (%d) size (%d) rate (%u).................................",
				pWlanHeader->MacHeader.SequenceControl.SequenceNumber,
				frameBufSize,
			    (gPHYMode == PHY_802_11A  ? BB11aDemodCtx.GetDataRate() : BB11bDemodCtx.GetDataRate())
			);

			fTargetFrame = 1;
		}
		
		if ( fTargetFrame ||
			 ETH_IS_BROADCAST (pWlanHeader->MacHeader.Address1.Address) )
		{

			if ( gPHYMode == PHY_802_11A ) {
				lastRate = (int) BB11aDemodCtx.GetDataRate ();
			} else {
				lastRate = (int) BB11bDemodCtx.GetDataRate ();
			}
			
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

				PlotText ( "mac event", "Indicate packet seq (%d)", 
					CurRecvSeqNo );

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
