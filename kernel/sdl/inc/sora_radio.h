#pragma once

#include <sora.h>
#include <rxstream.h>
#include <vector128.h>
#include <sbuf.h>

#define DUMMY_RADIO 0

static const int sora_max_radios = 4;

template<ULONG len>
class TSequence8 : public TSBuf<len*sizeof(COMPLEX8)>
{
public:
	COMPLEX8* GetBuffer () { return (COMPLEX8*) m_sbuf; }
	operator  COMPLEX8*()  { return (COMPLEX8*) m_sbuf; }	
	COMPLEX8& operator[] (int index) { return ((COMPLEX8*) m_sbuf)[index]; }
};


//
// CSoraSignalBuf : managing the sample buffer
//
class CSoraSignalBuf {
private:
	LPVOID m_pSampleBuf;
	ULONG  m_sampleBufSize;
	ULONG  m_sampleLength; // how many bytes are filled by samples

protected:
	void _release () {
		if ( m_pSampleBuf ) {
			SoraUReleaseBuffer ( m_pSampleBuf );
			m_pSampleBuf = NULL;
		}
	}

public:
	CSoraSignalBuf () {
		m_pSampleBuf = NULL;
		m_sampleBufSize = 0;
		m_sampleLength  = 0;
	}

	~CSoraSignalBuf () {
		_release (); 
	}

	bool Allocate ( ULONG size ) {
		m_pSampleBuf = SoraUAllocBuffer(size);
        if (m_pSampleBuf == NULL) {
			return false;
        }

		m_sampleBufSize = size;
		return true;
	}

	void Release () {
		_release ();
	}

	LPVOID GetBuffer  () { return m_pSampleBuf; }
	ULONG  GetBufSize () { return m_sampleBufSize; }

	template<typename T>
	T* GetNextPointer () { return ((T*)((UCHAR*)m_pSampleBuf + m_sampleLength)); } 

	ULONG  GetSampleLength () { return m_sampleLength; }
	void   SetSampleLength (ULONG len) { m_sampleLength = len; }
	ULONG  AddSampleLength (ULONG len) { m_sampleLength += len; return m_sampleLength; }
	void   Clear () {m_sampleLength = 0;}
	
	operator UCHAR*() { return (UCHAR*) m_pSampleBuf; }

	template<ULONG size>
	CSoraSignalBuf& operator << (TSBuf<size> &sbuf) {
		if ( m_sampleBufSize >= m_sampleLength +  sbuf.GetDataLength()) {
			memcpy ( ((UCHAR*)m_pSampleBuf + m_sampleLength), sbuf.GetBuffer(), sbuf.GetDataLength () );
			m_sampleLength += sbuf.GetDataLength();
		}
		return (*this);
	};
};

class CSoraRadio;
class CSoraSignal {
	ULONG m_sid; // signal id
	int   m_rid; // radio  id

	inline void _release ();

public:
	ULONG& SID () { return m_sid; }

	CSoraSignal () { m_sid = 0; } 
	CSoraSignal (ULONG sid) { m_sid = sid; } 
	~CSoraSignal () { _release (); }

	void Release () { _release (); }

	CSoraSignal& operator= (ULONG sid) { m_sid = sid; return *this; }
	CSoraSignal& operator= (CSoraSignal sig) { m_sid = sig.SID(); return *this; }
	friend class CSoraRadio;
};

class CSoraMIMOSignal {
	CSoraSignal m_signals[sora_max_radios];
	int			m_active;

	void _release () {
		for (int i=0; i<m_active; i++) {
			m_signals[i].Release ();
		}
		m_active = 0;
	}
public:
	CSoraMIMOSignal  () : m_active (0) {}
	~CSoraMIMOSignal () { _release (); }

	int GetSigNum () { return m_active; }
	CSoraSignal& operator[](int idx) {return m_signals[idx]; }
	CSoraSignal& Last() { return m_signals[m_active-1]; }
	void Release () { _release (); }
	CSoraMIMOSignal& operator++() {
		if ( m_active < sora_max_radios ) {
			m_active++;
		}
		return (*this);
	}

	CSoraMIMOSignal& operator++(int) {
		if ( m_active < sora_max_radios ) {
			m_active++;
		}
		return (*this);
	}

	//CSoraMIMOSignal& operator <<(CSoraSignal& sig) {
	//	if ( m_active < sora_max_radios ) {
	//		m_signals[m_active++] = sig;
	//	}
	//	return (*this);
	//}
};

//
// CSoraRadioConfig: all Sora radio-related configuration parameters
//
class CSoraRadioConfig {
public:
	// Freq
	bool  m_bMUnit; // set if the underlying firm interpret cfreq in MHz (the default is KHz)
	ULONG m_cfreq;
	ULONG m_cfo;

	// Gains
	ULONG m_txgain;
	ULONG m_rxgain;
	ULONG m_rxpa;

	CSoraRadioConfig () {
		// default
		m_bMUnit   = false;
		m_cfreq    = 2422 * 1000;
		m_cfo      = 0;

		m_txgain   = 0x1500;

		m_rxgain   = 0x1000;
		m_rxpa     = 0x2000;
	}

	char* usage () {
		return 
			"   -w             -    The firmware interprets freq in MHz unit\n"
			"   -f freq        -    carrier frequency (always in MHz regardless -w setting)\n"
			"   -fo freq       -    freq. offset (always in KHz regardless -w setting)\n"
			"   -tg gain       -    Tx gain (in 1/256dB) \n"
			"   -rg gain       -    Rx gain (in 1/256dB) \n"
			"   -rp gain       -    RxPA setting -- radio specific \n";
	}

	// Return value : 
	//         > 0  : the argument has been processed
	//        == 0  : the argument has not been processed
	//         < 0  : an error in the argument occurs
	int ParseCommandLine ( int& index, int argc, const char* argv[] ) {
		// parse command line of -w -f -tg -rg -rp -fo
		if ( index >= argc ) 
			return -1;

		// -w 
		if ( argv[index][0] == '-' 
				&& argv[index][1] == 'w' ) 
		{
			m_bMUnit = true;
			index ++;
		} 
		else // -fo. N.B. This should go before -f
		if ( argv[index][0] == '-' 
				&& argv[index][1] == 'f' 
				&& argv[index][2] == 'o') 
		{
			if ( ++ index < argc ) {
				char* stopstr;
				m_cfo = strtol ( argv[index++], &stopstr, 0 ); 
			} else {
				return -1;
			}
		} 
		else // -f
		if ( argv[index][0] == '-' 
				&& argv[index][1] == 'f' ) 
		{
			if ( ++ index < argc ) {
				char* stopstr;
				m_cfreq = strtol ( argv[index++], &stopstr, 0 ); 
			} else {
				return -1;
			}
		} 
		else // -tg
		if ( argv[index][0] == '-' 
				&& argv[index][1] == 't' 
				&& argv[index][2] == 'g') 
		{
			if ( ++ index < argc ) {
				char* stopstr;
				m_txgain = strtol ( argv[index++], &stopstr, 0 ); 
			} else {
				return -1;
			}
		} 
		else // -rg
		if ( argv[index][0] == '-' 
				&& argv[index][1] == 'r' 
				&& argv[index][2] == 'g') 
		{
			if ( ++ index < argc ) {
				char* stopstr;
				m_rxgain = strtol ( argv[index++], &stopstr, 0 ); 
			} else {
				return -1;
			}
		} 
		else // -rp
		if ( argv[index][0] == '-' 
				&& argv[index][1] == 'r' 
				&& argv[index][2] == 'p') 
		{
			if ( ++ index < argc ) {
				char* stopstr;
				m_rxpa = strtol ( argv[index++], &stopstr, 0 ); 
			} else {
				return -1;
			}
		} 
		else // unknown
			return 0;
		
		// processed
		return 1;
	}
	
};


//
// CSoraRadio : for managing a single radio object
//
class CSoraRadio {
	friend class CSoraRxStream;

private:
	int		m_rid; // the radio id

	// Freq
	ULONG m_cfreq;
	ULONG m_cfo;

	// Gain
	ULONG m_txgain;
	ULONG m_rxgain;
	ULONG m_rxpa;

	// Rx channel
	PVOID m_pRxBuffer;
	ULONG m_RxBufSize;

	// misc
	bool  m_2xDecimation;
	bool  m_16bit_tx_IQ; 

protected:
	void _init_radio_settings () {
		m_rid = -1;

		m_cfreq    = 2422 * 1000;
		m_cfo      = 0;

		m_txgain   = 0x1500;
		m_rxgain   = 0x1000;
		m_rxpa     = 0x2000;

		m_pRxBuffer = NULL;
		m_RxBufSize = 0;

		m_2xDecimation = true;
		m_16bit_tx_IQ  = false; 
	}

	void _release_radio () {
		if ( m_rid != -1 ) {
			if (m_pRxBuffer) {
				SoraURadioUnmapRxSampleBuf (m_rid, m_pRxBuffer);
			}
		}
	}

	PVOID GetRxBuffer  () { return m_pRxBuffer; }
	ULONG GetRxBufSize () { return m_RxBufSize; }

// methods
public:
	int   GetRadioID () { return m_rid; }

	// setting radio
	ULONG set_txgain ( ULONG gain ) { m_txgain = gain; SoraURadioSetTxGain (m_rid, m_txgain); return m_txgain; }
	ULONG get_txgain () { return m_txgain; }
	
	ULONG set_rxgain ( ULONG gain ) { m_rxgain = gain; SoraURadioSetRxGain (m_rid, m_rxgain); return m_rxgain; }
	ULONG get_rxgain () { return m_rxgain; }
	ULONG set_rxpa   (ULONG pa )    { m_rxpa   = pa;   SoraURadioSetRxPA ( m_rid, m_rxpa );   return m_rxpa;   }
	ULONG get_rxpa   ()   { return m_rxpa; }

	ULONG set_frequence (ULONG freq) { m_cfreq = freq; SoraURadioSetCentralFreq ( m_rid, m_cfreq ); return m_cfreq; }
	ULONG get_frequence () { return m_cfreq; }

	ULONG set_cfo (ULONG offset) { m_cfo = offset; SoraURadioSetFreqOffset ( m_rid, m_cfo ); return m_cfo; }
	ULONG get_cfo () { return m_cfo; }

	ULONG set_tx_16bit () {
		SoraUWriteRadioRegister(m_rid, 0x16, 0x1);
		SoraUWriteRadioRegister(m_rid, 0x16, 0x1);
		return 1;
	}

	ULONG get_tx_16bit () {
		ULONG val;
		SoraUReadRadioRegister(m_rid, 0x16, &val);
		return val;
	}

	bool  set_2xDecimation (bool b) {
		// not implemented due to current limitation...
		m_2xDecimation = b;
	}

	bool set_16bit_tx_IQ (bool b) {
		// not implemented due to current limitation...
		m_16bit_tx_IQ = b;
	}

	// Constructors
	CSoraRadio () {
		_init_radio_settings ();
	}

	~CSoraRadio () {
		ReleaseRadio ();
	}

	// startup a radio
	bool StartRadio () {
		if ( m_rid == -1 ) return false; 

			HRESULT hr = SoraURadioStart ( m_rid );
			return SUCCEEDED (hr);
	}

	bool ConfigureRadio () {
		if ( m_rid == -1 ) return false; 

		// configure radio may be called only when the radio startup
		// to change the radio setting, one should use the set_xxx methods
		HRESULT hr;
		hr = SoraURadioSetCentralFreq ( m_rid, m_cfreq );
		if ( FAILED(hr)) return false;

		hr = SoraURadioSetFreqOffset ( m_rid, m_cfo );
		if ( FAILED(hr)) return false;

		hr = SoraURadioSetTxGain ( m_rid, m_txgain);
		if ( FAILED(hr)) return false;		

		hr = SoraURadioSetRxPA ( m_rid, m_rxpa );
		if ( FAILED(hr)) return false;

		hr = SoraURadioSetRxGain ( m_rid, m_rxgain );
		return SUCCEEDED (hr);
	}

	bool ConfigureRadio ( CSoraRadioConfig& cfg ) {
		if ( m_rid == -1 ) return false; 
		
		// configure radio only when the parameter has changed
		if ( cfg.m_cfreq != m_cfreq ) {
			if ( cfg.m_bMUnit )
				set_frequence ( cfg.m_cfreq );
			else 
				set_frequence ( cfg.m_cfreq * 1000 );
		}

		if ( cfg.m_cfo != m_cfo ) {
			set_cfo ( cfg.m_cfo );
		}

		if ( cfg.m_txgain != m_txgain ) {
			set_txgain (cfg.m_txgain );
		}

		if ( cfg.m_rxgain != m_rxgain ) {
			set_rxgain (cfg.m_rxgain );
		}

		if ( cfg.m_rxpa != m_rxpa ) {
			set_rxpa (cfg.m_rxpa );
		}

		return true;
	}

	bool CreateRadio (int id) {
		if ( m_rid != -1 ) {
			// radio already created
			return false;
		}

		m_rid = id;

		// start radio and configure it
		if ( !StartRadio () ) {
			printf ( "Radio %d: Fail to start radio!\n", id );
			m_rid = -1;
			return false;
		}

		HRESULT hr;
		hr = SoraURadioMapRxSampleBuf( id, &m_pRxBuffer, (PULONG)&m_RxBufSize);
		if ( FAILED (hr) ) {
			m_rid = -1;
			return false;
        }

		// radio is ready for configuration
		ConfigureRadio ();

		return true;
	}

	void ReleaseRadio () {
		if ( m_rid != -1 ) {
			_release_radio ();
			_init_radio_settings ();
		}
	}

	// Transfer
	bool Transfer ( CSoraSignalBuf &sbuf, CSoraSignal &sig ) {
		ULONG sid;
		HRESULT hr = SoraURadioTransferEx ( m_rid, (UCHAR*) sbuf, sbuf.GetSampleLength(), &sid );
		sig = sid;
		sig.m_rid = GetRadioID();

		return SUCCEEDED(hr);
	}

	// Tx
	bool Transmit ( CSoraSignal & sig ) {
		HRESULT hr = SoraURadioTx ( m_rid, sig.SID() );
		return SUCCEEDED (hr);
	}
};

class CSoraMIMORadio {
	CSoraRadio m_radio[sora_max_radios];
	ULONG      m_rids [sora_max_radios];
	int        m_active;

	void _release () {
		for (int i=0; i<m_active; i++) {
			m_radio[i].ReleaseRadio ();
		}
		m_active = 0;
	}
public:
	CSoraMIMORadio () {
		m_active = 0;
	}

	~CSoraMIMORadio () {
		_release ();
	}

	void Release () {
		_release ();
	}

	int GetRadioNum () { return m_active; }
	bool CreateRadios ( int rid[], int nCount ) {
		if ( nCount <0 || nCount > 4) return false;

		m_active = nCount;
		for ( int i=0; i<nCount; i++ ) {
			m_rids[i] = rid[i];
			if ( !m_radio[i].CreateRadio (rid[i]) ) {
				printf ( "Fail to create a radio %d!\n", rid[i]);

				Release ();
				return false;
			}
		} 

		return true;
	}

	bool ConfigureRadios ( CSoraRadioConfig& cfg ) {
		if ( m_active <0 || m_active > 4) 
			return false;

		for ( int i=0; i<m_active; i++ ) {
			if (m_radio[i].GetRadioID () >= 0 ) {
				m_radio[i].ConfigureRadio ( cfg );
			} else {
				return false;
			}
		} 

		return true;
	}

	CSoraRadio& operator[] (int idx) {
		return m_radio[idx];
	}

	// Transfer
	bool Transfer ( CSoraSignalBuf &sbuf, CSoraSignal &sig ) {
		if ( !m_active ) return false;

		return m_radio[0].Transfer ( sbuf, sig );
	}

	// Transmit
	bool Transmit ( CSoraMIMOSignal & sig ) {
		if ( !m_active || sig.GetSigNum () != m_active ) 
			return false;

		ULONG sigs[sora_max_radios];
		for ( int i=0; i<m_active; i++ ) {
			sigs[i] = sig[i].SID();
		}

		HRESULT hr = SoraURadioMimoTx ( m_rids, sigs, m_active );
		return SUCCEEDED(hr);
	}
};

//
// CSoraRxStream : for managing a single Sora rx stream
//
class CSoraRxStream : public SORA_RADIO_RX_STREAM
{
static const ULONG max_skip_count = 137626; 

	bool m_bValid;     // if the RxStream is valid
	FLAG  m_bTouch;	   // if the stream has touched the end
	ULONG m_timestamp; // the timestamp of last read sample
	int   m_rid;       // radio id
	
	void _init () {
		m_bValid = false;
		m_bTouch = 0;
		m_rid    = 0;
	}

	void _release () {
		if ( m_bValid ) {
			SoraURadioReleaseRxStream ( this, m_rid );
		}

		_init ();
	}
public:
	CSoraRxStream () {
		_init (); 
	}

	~CSoraRxStream () {
		_release ();
	}

	// alloc & release
	bool Allocate ( CSoraRadio* pRadio ) {
		HRESULT hr;
		if ( pRadio == NULL ) return false;
		if ( m_bValid ) {
			_release ();
		}

		hr = SoraURadioAllocRxStream( this, pRadio->GetRadioID (), (PUCHAR)pRadio->GetRxBuffer (), pRadio->GetRxBufSize ());
        if (SUCCEEDED(hr)) {
			m_rid = pRadio->GetRadioID();
			m_bValid = true;			
        }

		return m_bValid;
	}

	void Release () {
		_release ();
	}

	// operations
	FLAG IsTouched () { return (m_bTouch); };
	ULONG LastTimestamp () { return (m_timestamp); };

	bool Read ( SignalBlock& blk ) {
		HRESULT hr = SoraRadioReadRxStreamEx( this, &m_bTouch, blk, m_timestamp );
		return SUCCEEDED (hr);
	}

	// skip a number of blocks;
	// return: the actual number of blocks being skipped
	ULONG skip ( ULONG skip_count ) {
		ULONG cnt = 0;
	    PRX_BLOCK pbScanPoint;
		HRESULT hr;

		while (1) {
			pbScanPoint = SoraRadioGetRxStreamPos(this);

			hr = SoraCheckSignalBlock(pbScanPoint, SoraGetStreamVStreamMask(this), 1024, &m_bTouch );
			if (FAILED(hr)) {
				return skip_count;
			}

			m_timestamp = pbScanPoint->u.Desc.TimeStamp;

			// Advance scan ponter
			__SoraRadioAdvanceRxStreamPos(this);
			cnt ++;
			if ( m_bTouch || cnt >= skip_count ) {
				break;
			}
		}

		return cnt;
	}

	// Touch skips all samples until the end of stream reaches
	ULONG Touch () {
		return skip (max_skip_count);
	}
};

class CSoraMIMOStreams {
	CSoraRxStream m_streams[sora_max_radios];
	ULONG         m_scnt;

	void _release () {
		for ( ULONG i=0; i<m_scnt; i++) {
			m_streams[i].Release ();
		}
		m_scnt = 0;
	}
public:
	CSoraMIMOStreams () {
		m_scnt = 0;
	}

	~CSoraMIMOStreams () {
		_release ();
	}

	void Release () {
		_release ();
	}

	bool Allocate ( CSoraMIMORadio* pRadios ) {
		Release ();
		for ( int i=0; i<pRadios->GetRadioNum (); i++) {
			if ( !m_streams[i].Allocate ( &(*pRadios)[i] ) ) {
				Release ();
				return false;
			}
			m_scnt ++;
		}
		
		return true;
	}

	ULONG GetStreamNum () { return m_scnt; }
	
	CSoraRxStream& operator[] (int index ) {
		return m_streams[index];
	}
};

inline void CSoraSignal::_release () {
	if ( m_sid ) {
		// Tx Free is no long associated with a radio object
		// using DUMMY_RADIO for backward compatibility
		SoraURadioTxFree (m_rid, m_sid);
		m_sid = 0;
		m_rid = 0;
	}
}