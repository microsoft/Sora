#pragma once

#include <alinew.h>

// All about static buffers

// TSBuf - static buffer
template<ULONG SIZE>
class TSBuf {
protected:
	UCHAR m_sbuf[SIZE];
	ULONG m_length;		// how many validate data are stored in the buffer
public:
	TSBuf () { m_length = 0; }
	ULONG  GetDataLength()            { return m_length; }
	ULONG  SetDataLength(ULONG len )  { m_length = len; return m_length;    }
	ULONG  AddDataLength(ULONG delta) { m_length += delta; return m_length; }
	ULONG  GetSize ()      { return SIZE;   }
	UCHAR* GetBuffer ()    { return m_sbuf; }
	void   Clear ()        { m_length = 0; }
	operator UCHAR*()      { return m_sbuf; }

	template<typename T>
	T* GetNextPointer () { return ((T*)&m_sbuf[m_length]); } 

	// append operator
	template<ULONG S>
	TSBuf& operator<< (const TSBuf<S>& sbuf) {
		if ( m_length + S < SIZE ) {
			memcpy ( &m_sbuf[m_length], sbuf.GetBuffer(), sbuf.GetDataLength () );
			m_length += S;
		}
		return (*this);
	}

public:
	// simple operations
	void zero () { memset ( m_sbuf, 0, SIZE); }
};

// TDBuf - dynamic buffer
class TDBuf {
protected:
	UCHAR* m_pdbuf;
	ULONG  m_bufsize;
	ULONG  m_length;		// how many validate data are stored in the buffer
public:
	TDBuf () { 
		m_pdbuf = NULL;
		m_bufsize = 0;
		m_length  = 0; 
	}

	TDBuf ( ULONG size ) {
		m_pdbuf = (UCHAR*) aligned_malloc (size, 16);
		m_bufsize = size;
		m_length  = 0;
	}

	void Release () {
		if ( m_pdbuf ) {
			aligned_free ( m_pdbuf );
			m_pdbuf = NULL;
			m_bufsize = 0;
			m_length  = 0; 
		}
	}

	bool Allocate ( ULONG size ) {
		Release ();
		assert (m_pdbuf == NULL );

		m_pdbuf = (UCHAR*) aligned_malloc (size, 16);
		if ( m_pdbuf == NULL ) 
			return false;

		m_bufsize = size;
		m_length  = 0;
		return true;
	}

	ULONG  GetDataLength()            { return m_length; }
	ULONG  SetDataLength(ULONG len )  { m_length = len; return m_length;    }
	ULONG  AddDataLength(ULONG delta) { m_length += delta; return m_length; }
	ULONG  GetSize ()      { return m_bufsize;   }
	UCHAR* GetBuffer ()    { return m_pdbuf; }
	void   Clear ()        { m_length = 0; }
	operator UCHAR*()      { return m_pdbuf; }

	template<typename T>
	T* GetNextPointer () { return ((T*) m_pdbuf + m_length); } 

	// append operator
	template<ULONG S>
	TDBuf& operator<< (const TSBuf<S>& s) {
		if ( m_length + S < SIZE ) {
			memcpy ( &m_sbuf[m_length], sbuf.GetBuffer(), sbuf.GetDataLength () );
			m_length += S;
		}
		return (*this);
	}

	TDBuf& operator<< (TDBuf& dbuf) {
		if ( m_length + dbuf.GetDataLength() < m_bufsize ) {
			memcpy ( m_pdbuf + m_length, dbuf.GetBuffer(), dbuf.GetDataLength () );
			m_length += dbuf.GetDataLength ();
		}
		return (*this);
	}
public:
	// simple operations
	void zero () { 
		if ( m_pdbuf ) {
			memset ( m_pdbuf, 0, m_bufsize ); 
		}
	}
};