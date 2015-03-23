#pragma once

SELECTANY class CRadioInfo {
private:
	ulong m_centralfreq;
	ulong m_rxgain;
	ulong m_txgain;
	ulong m_rxpa;
	
public:
    ulong& CentralFreq () { return m_centralfreq; }
	ulong& RxGain () { return m_rxgain; }
	ulong& TxGain () { return m_txgain; }	
	ulong& RxPA () {return m_rxpa;}

	CRadioInfo () {
		m_centralfreq = 5200; // Channel 40
		m_txgain = 0x1500;
		
		m_rxgain = 0x1000;
		m_rxpa   = 0x2000;
	}
} gRadioInfo;

