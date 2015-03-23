#pragma once

//
// Library for complex high-precision (double) DSP processing
//

#include <math.h>
#include <string.h>

#define FINL __forceinline

const double PI = 3.141593;

#include "complex.h"

struct COMPLEX {
    double re, im;
	COMPLEX () : re(0), im (0) {};
	COMPLEX ( int r, int i) { re = r; im =i; };
	COMPLEX ( double r, double i) { re = r; im =i; };
	COMPLEX & operator=( const COMPLEX & c ) { re = c.re; im = c.im; return *this; };
	COMPLEX & operator=( const COMPLEX16 & c16 ) { 
		re = c16.re; im = c16.im; 
		return *this; 
	};
	double arg () { if (re == 0 ) {
						if (im >= 0 ) return PI/2;
						else return (-PI/2);
					}

					return atan2 (im, re );
				};
	double norm () { return re * re + im * im; };
	COMPLEX& rotate ( double arg ) {
        double r = re * cos (arg ) - im * sin (arg);
        double i = re * sin (arg) + im * cos (arg);
        re = r; im = i;
		return *this;
	}
	COMPLEX& conjugate () {
		im = -im;
		return *this;
	}
	COMPLEX& scale ( double factor ) {
		re *= factor;
		im *= factor;
		return *this;
	}
	void set_pole ( double mod, double arg ) {
		re = mod * cos (arg);
		im = mod * sin (arg);
	}
	void clear () { re = im = 0; }	
};

// Complex support
inline COMPLEX operator+ ( const COMPLEX & x, const COMPLEX & y ) {
	COMPLEX r;
	r.re = x.re + y.re;
	r.im = x.im + y.im;
	return r;
}

inline COMPLEX operator- ( const COMPLEX & x, const COMPLEX & y ) {
	COMPLEX r;
	r.re = x.re - y.re;
	r.im = x.im - y.im;
	return r;
}

inline COMPLEX operator* ( const COMPLEX & x, const COMPLEX & y ) {
	COMPLEX r;
	r.re = x.re*y.re - x.im * y.im;
	r.im = x.re*y.im + x.im * y.re;
	return r;
}

inline COMPLEX operator* ( const COMPLEX & x, const double & y ) {
	COMPLEX r;
	r.re = x.re*y;
	r.im = x.re*y;
	return r;
}

inline COMPLEX operator/ ( const COMPLEX & x, const double & y ) {
	COMPLEX r;
	r.re = x.re / y;
	r.im = x.im / y;
	return r;
}

inline COMPLEX operator/ ( const COMPLEX & x, const COMPLEX & y ) {
	COMPLEX r;
	double n = y.re*y.re + y.im * y.im;
	r.re = x.re*y.re + x.im * y.im;
	r.im = - x.re*y.im + x.im * y.re;
	
	r = r / n;
	return r;
}

inline COMPLEX operator- ( const COMPLEX & x ) {
	COMPLEX r;
	r.re = - x.re;
	r.im = - x.im;
	return r;
}

// ~ ::= conjugate
inline COMPLEX operator~ (const COMPLEX & x ) {
	COMPLEX r;
	r.re = x.re;
	r.im = - x.im;
	return r;
}

class SampleVec {
public:
    COMPLEX * m_pData;
    int       m_nMemSize;
    int       m_nVecSize;

public:
    SampleVec ( int nSize ) {
        m_pData = new COMPLEX[nSize];
        m_nMemSize = m_nVecSize = nSize;
    }
    
    SampleVec ( int nSize, COMPLEX * pData ) {
        m_pData = new COMPLEX[nSize];
        m_nMemSize = m_nVecSize = nSize;
        memcpy ( m_pData, pData, sizeof(COMPLEX)*nSize );
    }
    
    SampleVec ( const SampleVec& v ) {
        int nSize = v.m_nVecSize;
        m_pData = new COMPLEX[nSize];
        m_nMemSize = m_nVecSize = nSize;
        memcpy ( m_pData, v.m_pData, sizeof(COMPLEX)*nSize );
    }

    ~SampleVec () {
        if ( m_pData ) {
            delete m_pData;
            m_nMemSize = m_nVecSize = 0;    
        }
    }

public:
    COMPLEX * get_vec () { return m_pData; }
    int       get_size () { return m_nVecSize; }
    void      set_size ( int nSize ) {
        if ( nSize == m_nVecSize ) {
            return;
        } else if ( nSize < m_nMemSize ) {
            m_nVecSize = nSize;
        } else {
            delete m_pData;
            m_pData = new COMPLEX[nSize];
            m_nMemSize = m_nVecSize = nSize;
        }
    }
    void clear () {
        memset ( m_pData, 0, sizeof(COMPLEX)*m_nVecSize );
    }

public:    
    FINL COMPLEX& operator[] ( int idx ) {
        idx = idx % m_nVecSize;
        if ( idx < 0 ) idx += m_nVecSize;
        return m_pData[idx];
    }
    FINL operator COMPLEX*() { return m_pData; }

    FINL SampleVec& operator = ( const SampleVec& v ) {
        set_size ( v.m_nVecSize );
        memcpy ( m_pData, v.m_pData, sizeof(COMPLEX)*v.m_nVecSize );
        return *this;
    }
    
};



// DSP support
// Any point DFT - slow version
void DFT  ( int Size, COMPLEX * pInput, COMPLEX * pOutput );
void iDFT ( int Size, COMPLEX * pInput, COMPLEX * pOutput );
void FFT  ( int Size, COMPLEX * pInput, COMPLEX * pOutput );

int LoadDumpFile(const char *fileName, COMPLEX* buf, int maxSize);
void SaveDumpFile ( const char *filename, COMPLEX* buf, int Size );

