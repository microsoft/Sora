#pragma once

#include "ieee80211const.h"
#include "ieee80211facade.hpp"

inline ushort GeneratorPolynomials0(ulong x, ulong s)
{
    return (x ^ (s >> 4) ^ (s >> 3)^ (s>>1) ^ (s) ) & 0x1;
}

inline ushort GeneratorPolynomials1(ulong x, ulong s)
{
    return (x^(s)^(s>>3)^(s>>4)^(s>>5)) & 0x1;
}

// Convolutional encoder: K=7
// 1/2 rate
DEFINE_LOCAL_CONTEXT(TConvEncode_12, CF_VOID);
template<TFILTER_ARGS>
class TConvEncode_12 : public TFilter<TFILTER_PARAMS>
{
protected:
    uchar m_reg; // the local register

    struct _init_lut
    {
        void operator()(ushort (&lut)[64][256])
        {
		    ulong i,j,s,x;
		    ushort o;
		    for ( i=0; i<256; i++) {
			    for ( j=0; j<64; j++ ) {
				    s = j;
				    x = i;
				    o = 0;
				    for ( int k=0; k<8; k++ ) {
					    ushort o1 = GeneratorPolynomials0(x, s);
					    ushort o2 = GeneratorPolynomials1(x, s);
					    o = (o >> 2) | (o1 << 14) | (o2<<15);

					    s = (s>>1) | ((x & 0x01)<<5);
					    x >>= 1;
				    }
				    lut [j][i] = o;
			    }
		    }
		    /*
		    for ( j=0; j<64; j++ ) {
			    printf ( "\n" );
			    for ( i=0; i<256; i++) {
				    printf ( "%04x ", m_lut[j][i] );
			    }
			    printf ( "\n" );
		    }
 	
		    */
        }
    };
    const static_wrapper<ushort [64][256], _init_lut> m_lut;
    
	void _init () {
		m_reg = 0;
	}
	
public:
	DEFINE_IPORT(uchar, 1);
    DEFINE_OPORT(uchar, 2);
	
public:
    REFERENCE_LOCAL_CONTEXT(TConvEncode_12);
    
    STD_TFILTER_CONSTRUCTOR(TConvEncode_12)
    { 
    	_init ();
	}
    STD_TFILTER_RESET() { _init (); }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            uchar c;
            c = *ipin.peek();
            ipin.pop();

			ushort& code = cast_ref<ushort> (opin().append()); 
			code = m_lut[m_reg][c];
			m_reg = c >> 2;

			//printf ( "%02x -> %04x\n", c, code);
            Next()->Process(opin());
        }
        return true;
    }
};

// Convolutional encoder: K=7
// 2/3 rate
DEFINE_LOCAL_CONTEXT(TConvEncode_23, CF_VOID);
template<TFILTER_ARGS>
class TConvEncode_23 : public TFilter<TFILTER_PARAMS>
{
protected:
    uchar m_reg; // the local register

    struct _init_lut
    {
        void operator()(ushort (&lut)[64][256])
        {
	        ulong i,j,s,x;
	        ushort o, o1, o2;
	        for ( i=0; i<256; i++) {
		        for ( j=0; j<64; j++ ) {
			        s = j;
			        x = i;
			        o = 0;
			        for ( int k=0; k<8; k+=2 ) {
				        o1 = GeneratorPolynomials0(x, s);
				        o2 = GeneratorPolynomials1(x, s);
				        o = (o >> 2) | (o1 << 14) | (o2 << 15);
				        s = (s >> 1) | ((x & 0x01) << 5);
				        x >>= 1;

				        o1 = GeneratorPolynomials0(x, s);
				        o = (o >> 1) | (o1 << 15);
				        s = (s >> 1) | ((x & 0x01) << 5);
				        x >>= 1;
                    }
			        lut [j][i] = o >> 4;
		        }
	        }
	        /*
	        for ( j=0; j<64; j++ ) {
		        printf ( "\n" );
		        for ( i=0; i<256; i++) {
			        printf ( "%04x ", m_lut[j][i] );
		        }
		        printf ( "\n" );
	        }
 	
	        */
        }
    };
    const static_wrapper<ushort [64][256], _init_lut> m_lut;

	void _init () {
		m_reg = 0;
	}

public:
	DEFINE_IPORT(uchar, 2);
    DEFINE_OPORT(uchar, 3);
	
public:
    REFERENCE_LOCAL_CONTEXT(TConvEncode_23);
    
    STD_TFILTER_CONSTRUCTOR(TConvEncode_23)
    { 
    	_init ();
	}
    STD_TFILTER_RESET() { _init (); }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const uchar *c = ipin.peek();
            ushort s0 = m_lut[m_reg][c[0]];
            ushort s1 = m_lut[c[0]>>2][c[1]];
            ipin.pop();

			uchar* o = opin().append();
            o[0] = (uchar)s0;
            o[1] = ((s1 & 0xF) << 4 ) | (s0 >> 8);
            o[2] = s1 >> 4;
			m_reg = c[1] >> 2;

            Next()->Process(opin());
        }
        return true;
    }
};

// Convolutional encoder: K=7
// 3/4 rate
DEFINE_LOCAL_CONTEXT(TConvEncode_34, CF_VOID);
template<TFILTER_ARGS>
class TConvEncode_34 : public TFilter<TFILTER_PARAMS>
{
protected:
    uchar m_reg; // the local register

    struct _init_lut
    {
        void operator()(ushort (&lut)[64][64])
        {
	        ulong i,j,s,x;
	        ushort o, o1, o2;
	        for ( i=0; i<64; i++) {
		        for ( j=0; j<64; j++ ) {
			        s = j;
			        x = i;
			        o = 0;
			        for ( int k=0; k<6; k+=3 ) {
				        o1 = GeneratorPolynomials0(x, s);
				        o2 = GeneratorPolynomials1(x, s);
				        o = (o >> 2) | (o1 << 14) | (o2 << 15);
				        s = (s >> 1) | ((x & 0x01) << 5);
				        x >>= 1;

				        o1 = GeneratorPolynomials0(x, s);
				        o = (o >> 1) | (o1 << 15);
				        s = (s >> 1) | ((x & 0x01) << 5);
				        x >>= 1;

				        o2 = GeneratorPolynomials1(x, s);
				        o = (o >> 1) | (o2 << 15);
				        s = (s >> 1) | ((x & 0x01) << 5);
				        x >>= 1;
                    }
			        lut [j][i] = o >> 8;
		        }
	        }
	        /*
	        for ( j=0; j<64; j++ ) {
		        printf ( "\n" );
		        for ( i=0; i<256; i++) {
			        printf ( "%04x ", m_lut[j][i] );
		        }
		        printf ( "\n" );
	        }
 	
	        */
        }
    };
    const static_wrapper<ushort [64][64], _init_lut> m_lut;

	void _init () {
		m_reg = 0;
	}

public:
	DEFINE_IPORT(uchar, 3);
    DEFINE_OPORT(uchar, 4);
	
public:
    REFERENCE_LOCAL_CONTEXT(TConvEncode_34);
    
    STD_TFILTER_CONSTRUCTOR(TConvEncode_34)
    { 
    	_init ();
	}
    STD_TFILTER_RESET() { _init (); }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const uchar *c = ipin.peek();
			uchar c0 = c[0] & 0x3F;
            uchar c1 = ((c[1] & 0xF ) << 2) | (c[0] >> 6);
            uchar c2 = ((c[2] & 0x3 ) << 4) | (c[1] >> 4);
            uchar c3 = c[2] >> 2;
            ipin.pop();

			uchar* o = opin().append();
			o[0] = (uchar)m_lut[m_reg][c0];
			o[1] = (uchar)m_lut[c0][c1];
			o[2] = (uchar)m_lut[c1][c2];
			o[3] = (uchar)m_lut[c2][c3];
			m_reg = c3;

            Next()->Process(opin());
        }
        return true;
    }
};
