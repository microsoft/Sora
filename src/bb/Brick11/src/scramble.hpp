#pragma once
#include "ieee80211facade.hpp"

// Scrambler 741
// Self-synchronized scrambler g = z^-7 + z^-4 + 1
// 
DEFINE_LOCAL_CONTEXT(TSc741, CF_ScramblerSeed);
template<TFILTER_ARGS>
class TSc741 : public TFilter<TFILTER_PARAMS>
{
private:
	// context binding
	CTX_VAR_RO(UCHAR, m_seed);

protected:
	// internal states
    UCHAR m_Reg;

    struct _init_lut
    {
        void operator()(uchar (&lut)[256][128])
        {
		    int i,j,k;
		    uchar x, s, o;
		
		    for ( i=0; i<256; i++) {
			    for ( j=0; j<128; j++) {
				    x = (uchar)i;
				    s = (uchar)j;
				    o = 0;
				    for ( k=0; k<8; k++) {
					    uchar o1 = (x ^ (s) ^ (s >> 3)) & 0x01;
					    s = (s >> 1) | (o1 << 6);
					    o = (o >> 1) | (o1 << 7);

					    x = x >> 1;
				    }
				    lut [i][j] = o;
			    }
		    }
        }
    };
	// scrambler LUT
    const static_wrapper<uchar [256][128], _init_lut> m_lut;

	FINL void _init () {
		m_Reg = m_seed;
	}

public:
	// define pin traits
	// input
	DEFINE_IPORT (uchar, 1);
	// output
    DEFINE_OPORT (uchar, 1);

public:
	// Brick interface
    REFERENCE_LOCAL_CONTEXT(TSc741);
    STD_TFILTER_CONSTRUCTOR(TSc741)
        BIND_CONTEXT(CF_ScramblerSeed::sc_seed, m_seed)
    {
    	_init ();
    }
	STD_TFILTER_RESET()
    {
		_init ();
    }
	STD_TFILTER_FLUSH() {}

	BOOL_FUNC_PROCESS (ipin)
    {
        while(ipin.check_read())
        {
            uchar b = *ipin.peek();
            ipin.pop();
//		    printf ( "transferred byte %02x\n", b );

			// lookup table to scramble
            uchar code = m_lut[b][m_Reg];
            m_Reg = code >> 1;

            *opin().append() = code;
            Next()->Process(opin());
        }
        return true;
    }
};

// Descrambler 741
// Self-synchronized descrambler g = z^-7 + z^-4 + 1
// 
DEFINE_LOCAL_CONTEXT(TDesc741, CF_Descramber);
template<TFILTER_ARGS>
class TDesc741 : public TFilter<TFILTER_PARAMS>
{
private:
	// context binding
	CTX_VAR_RW (UCHAR, byte_reg);

protected:
    struct _init_lut
    {
        void operator()(uchar (&lut)[256][128])
        {
		    int i,j,k;
		    uchar x, s, o;
		
		    for ( i=0; i<256; i++) {
			    for ( j=0; j<128; j++) {
				    x = (uchar)i;
				    s = (uchar)j;
				    o = 0;
				    for ( k=0; k<8; k++) {
					    uchar o1 = (x ^ (s) ^ (s >> 3)) & 0x01;
					    s = (s >> 1) | ((x & 0x01) << 6);
					    o = (o >> 1) | (o1 << 7);

					    x = x >> 1;
				    }
				    lut [i][j] = o;
			    }
		    }
        }
    };
	// scrambler LUT
    const static_wrapper<uchar [256][128], _init_lut> m_lut;

public:
	// define pin traits
	// input
	DEFINE_IPORT (uchar, 1);
	// output
    DEFINE_OPORT (uchar, 1);

public:
	// Brick interface
    REFERENCE_LOCAL_CONTEXT(TDesc741);
    STD_TFILTER_CONSTRUCTOR(TDesc741)
        BIND_CONTEXT(CF_Descramber::byte_reg, byte_reg)
    {
    }
	STD_TFILTER_RESET() {}

	STD_TFILTER_FLUSH() {}

    BOOL_FUNC_PROCESS (ipin)
    {
        while(ipin.check_read())
        {
            uchar b = *ipin.peek();
            ipin.pop();

            uchar outb = m_lut[b][byte_reg];
            byte_reg = b >> 1;

            *opin().append() = outb;
            Next()->Process(opin());
        }
        return true;
    }
};

/*******************************************/
// 11a Scrambler 
// scrambler g = z^-7 + z^-4 + 1
// 
//

DEFINE_LOCAL_CONTEXT(T11aSc, CF_ScramblerSeed, CF_ScramblerControl);
template<TFILTER_ARGS>
class T11aSc : public TFilter<TFILTER_PARAMS>
{
private:
	// context binding
	CTX_VAR_RO(UCHAR, sc_seed);
	CTX_VAR_RO(ulong, scramble_ctrl);

protected:
	// internal states
    UCHAR m_Reg;

	// N.B. In the lookup table, the least significant bit represents x^-7.
	// This placement is to favor the scramble operation.
    struct _init_lut
    {
        void operator()(uchar (&lut)[128])
        {
		    int i, k;
		    uchar x;
		    // state i
		    for ( i=0; i<128; i++) {
			    x = (uchar)i<<1;
			
			    for ( k=0; k<8; k++) {
				    uchar o1 = ((x >> 1) ^ (x >> 4)) & 0x01;
				    x = (x >> 1) | (o1 << 7);
			    }
			
			    lut [i] = x;
		    }
        }
    };
	// scrambler LUT
    const static_wrapper<uchar [128], _init_lut> m_lut;

	FINL void _init () {
		m_Reg = sc_seed;
	}

public:
	// define pin traits
	// input
	DEFINE_IPORT (uchar, 1);
	// output
    DEFINE_OPORT (uchar, 1);

public:
	// Brick interface
    REFERENCE_LOCAL_CONTEXT(T11aSc);
    STD_TFILTER_CONSTRUCTOR(T11aSc)
        BIND_CONTEXT(CF_ScramblerSeed::sc_seed, sc_seed)
        BIND_CONTEXT(CF_ScramblerControl::scramble_ctrl, scramble_ctrl)
    {
    	_init ();
    }
	STD_TFILTER_RESET()
    {
		_init ();
    }
	STD_TFILTER_FLUSH() {}

	BOOL_FUNC_PROCESS (ipin)
    {
        while(ipin.check_read())
        {
            uchar b = *ipin.peek();
            ipin.pop();
			
			uchar code;
			switch ( scramble_ctrl ) {
			case CF_ScramblerControl::DO_SCRAMBLE:
				m_Reg = m_lut[m_Reg>>1];
				code = b ^ m_Reg;
				break;
			case CF_ScramblerControl::NO_SCRAMBLE: 
				code = b;
				break;
			default:
				m_Reg = m_lut[m_Reg>>1];
				code = (b ^ m_Reg) & 0xC0;
			}
			
//			printf ( "input %x output %x\n", b, code );
			
            *opin().append() = code;
            Next()->Process(opin());
        }
        return true;
    }
};


//
// 11a Descrambler
//
DEFINE_LOCAL_CONTEXT(T11aDesc, CF_VOID);
template<TFILTER_ARGS>
class T11aDesc : public TFilter<TFILTER_PARAMS>
{
private:
	ulong m_desc_count;
    uchar m_desc_reg;

protected:
    struct _LUT
    {
        void operator()(uchar (&lut)[128])
        {
		    int i, k;
		    uchar x;
		    // state i
		    for ( i=0; i<128; i++) {
			    x = (uchar)i<<1;
			
			    for ( k=0; k<8; k++) {
				    uchar o1 = ((x >> 1) ^ (x >> 4)) & 0x01;
				    x = (x >> 1) | (o1 << 7);
			    }
			
			    lut [i] = x;
		    }
        }
    };
	// scrambler LUT
    const static_wrapper<uchar [128], _LUT> m_lut;


	FINL
	void __init () {
		m_desc_count = 0;
		m_desc_reg   = 0;
	}

	
	
public:
    DEFINE_IPORT(uchar, 1);
    DEFINE_OPORT(uchar, 1);
	
public:
    REFERENCE_LOCAL_CONTEXT(T11aDesc);
    STD_TFILTER_CONSTRUCTOR(T11aDesc)
    {
    	__init ();    
    }

    STD_TFILTER_RESET()
    {
    	__init ();
    }

    BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
            uchar b = *ipin.peek();
            ipin.pop();

            // first two bytes are seed
            m_desc_count ++;
            if (m_desc_count == 1)
            {
                return true;
            }
            else if (m_desc_count == 2)
            {
                m_desc_reg = b >> 1;
                return true;
            }

            // scramble
            m_desc_reg = m_lut[m_desc_reg];
            uchar o = b ^ m_desc_reg;
            m_desc_reg >>= 1;

//printf ( "desc %d (%x)\n", o, o );

            *opin().append() = o;
            Next()->Process(opin());
        }
		
        return true;
    }
};
