#pragma once

// standard bricks

#include <stdio.h>
#include <tpltrick.h>
#include <brick.h>
#include "brickutil.h"  // to do: remove it later
#include <dspcomm.h>

#include <stdfacade.h>

#define PORT_0 0
#define PORT_1 1
#define PORT_2 2
#define PORT_3 3
#define PORT_4 4
#define PORT_5 5
#define PORT_6 6
#define PORT_7 7
#define PORT_8 8
#define PORT_9 9
#define PORT_10 10

#include <memsource.hpp>
#include <dc.hpp>
#include <rxstream.hpp>
#include <energy.hpp>
#include <gdemux.hpp>

/*********************************************
 Basic sources and sinks
********************************************/
// TDropAny:: Discard any data type
DEFINE_LOCAL_CONTEXT(TDropAny, CF_VOID);
template< TSINK_ARGS >
class TDropAny : public TSink<TSINK_PARAMS>
{
public:
    DEFINE_IPORT(void, 1);
    STD_TSINK_CONSTRUCTOR(TDropAny) {}
	STD_TSINK_RESET() {}
	STD_TSINK_FLUSH() {}

    BOOL_FUNC_PROCESS(pin) {
        pin.clear ();
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(TDrop, CF_VOID);
template<typename TYPE, size_t NSTREAM>
class TDrop
{
public:
template< TSINK_ARGS >
class Filter: public TSink<TSINK_PARAMS>
{
public:
    DEFINE_IPORT(TYPE, 1, NSTREAM);
    STD_TSINK_CONSTRUCTOR(Filter) {}
    STD_TSINK_RESET() {}
    STD_TSINK_FLUSH() {}

    BOOL_FUNC_PROCESS(pin) {
        pin.clear ();
        return true;
    }
}; };

DEFINE_LOCAL_CONTEXT(TDummySource, CF_VOID);
template < TSOURCE_ARGS > 
class TDummySource : public TSource<TSOURCE_PARAMS>
{
public:
    DEFINE_OPORT(void, 1);
    STD_TSOURCE_CONSTRUCTOR(TDummySource) { }
    REFERENCE_LOCAL_CONTEXT(TDummySource);
    STD_TSOURCE_RESET() {}
    STD_TSOURCE_FLUSH() {}
    FINL bool Process() { return Next()->Process(opin()); }
};

/*********************************************
 Graph Separators
********************************************/
// ThreadSeparator - split the graph into two 
// threads
DEFINE_LOCAL_CONTEXT(TThreadSeparator, CF_VOID);
template<size_t BUF_SIZE = 64>
class TThreadSeparator
{
public:
    REFERENCE_LOCAL_CONTEXT(TThreadSeparator);
    template<TFILTER_ARGS>
    class Filter : public TBrick<T_CTX>, public ISource
    {
        typedef typename T_NEXT::iport_traits::type TYPE;
        static const size_t BURST = T_NEXT::iport_traits::burst;

        struct tagged
        {
            MEM_ALIGN(64) volatile bool valid;
            MEM_ALIGN(64) TYPE data[BURST];
        };

        tagged buf[BUF_SIZE];
        MEM_ALIGN(64) tagged *wptr;
		MEM_ALIGN(64) tagged *rptr;
		
        MEM_ALIGN(64) volatile bool evReset;
        MEM_ALIGN(64) volatile bool evFlush;
        MEM_ALIGN(64) volatile bool evProcessDone;

        T_NEXT* __next_;	// point to next brick
        T_NEXT* Next()       { return __next_; }
        int Seek (int offset) { return 0; } // not supported

    public:
        DEFINE_IPORT(TYPE, BURST);
        DEFINE_OPORT(TYPE, BURST);
        
        Filter(T_CTX & ctx, T_NEXT* n) : TBrick(ctx), __next_(n)
        {
            AddRef(Next());

            size_t i;
            for (i = 0; i < BUF_SIZE; i++)
                buf[i].valid = false;
            wptr = rptr = buf;
            evReset = evFlush = false;
            evProcessDone = true;
        }

        ~Filter()
        {
            IReferenceCounting::Release(Next());
        }

		// Called by the uplink thread
        BOOL_FUNC_PROCESS(ipin)
        {
            while (ipin.check_read())
            {
            	// spin wait if the synchronized buffer is full
                while (wptr->valid) { SoraThreadYield(TRUE); }
				
				// copy a burst of input data into the synchronized buffer
                const TYPE *input = ipin.peek();
//                size_t i;
//                for (i = 0; i < BURST; i++)
//                    wptr->data[i] = input[i];
				memcpy ( wptr->data, input, sizeof(TYPE)*BURST);
                ipin.pop();
				
                wptr->valid = true;
                evProcessDone = false;

				// advance the write pointer
                wptr++;
                if (wptr == buf + BUF_SIZE)
                    wptr = buf;
            }

            return true;
        }

        FINL bool Process() // ISource::Process
        {
            // if the synchronized buffer has no data, 
            // check whether there is reset/flush request
            if (!rptr->valid)
            {
                if (evReset)
                {
                    Next()->Reset();
                    evReset = false;
                }
                if (evFlush)
                {
                    Next()->Flush();
                    evFlush = false;
                }

                evProcessDone = true;
				// no data to process  
                return true;
            }

            // Otherwise, there are data. Pump the data to the output pin
            while (rptr->valid)
            {
                TYPE *output = opin().append();
    //            size_t i;
    //            for (i = 0; i < BURST; i++)
    //                output[i] = rptr->data[i];
			    memcpy ( output, rptr->data, sizeof(TYPE)*BURST);

                rptr->valid = false;
                rptr++;
                if (rptr == buf + BUF_SIZE)
                {
                    rptr = buf;
                    // Periodically yielding in busy processing to prevent OS hanging
                    SoraThreadYield(TRUE);
                }

                bool rc = Next()->Process(opin());
                if (!rc) return rc;
            }
            return TRUE;
        }

        FINL void Reset()
        {
        	// Set the reset event, spin-waiting for 
        	// the downstream to process the event
            evReset = true;
            while (evReset) { SoraThreadYield(TRUE); }
        }

        FINL void Flush()
        {
            // Wait for all data in buf processed by downstreaming bricks
            while (!evProcessDone) { SoraThreadYield(TRUE); }

        	// Set the flush event, spin-waiting for
        	// the downstream to process the event
            evFlush = true;
			while (evFlush) { SoraThreadYield(TRUE); }
        }

        virtual size_t _TraverseGraph(OUT IQuery *bricks[], const char *typeinfo, size_t count, const IQuery *const *begin)
        {
            if (count <= 0) return 0;
            size_t found = 0;
            if (MatchQuery(typeinfo))
        {
                if (Within(begin, bricks)) return 0;
                if (begin == bricks) begin++;
                *bricks++ = this;
                found++;
            }
            found += Next()->_TraverseGraph(bricks, typeinfo, count - found, begin);
            return found;
        }
    };
};

// TNoInline:: make the process function in the downstream block non-inline 
// Use this separator properly to prevent the code from growing too large
DEFINE_LOCAL_CONTEXT(TNoInline, CF_VOID);
template<TFILTER_ARGS>
class TNoInline : public TFilter<TFILTER_PARAMS>
{
public:
    static const size_t NSTREAM = T_NEXT::iport_traits::nstream;
    static const size_t BURST = T_NEXT::iport_traits::burst;
    typedef typename T_NEXT::iport_traits::type TYPE;
    DEFINE_IPORT(TYPE, BURST, NSTREAM);
    DEFINE_OPORT(TYPE, 1); // Downstream brick does not care about the burst size
    REFERENCE_LOCAL_CONTEXT(TNoInline);
    STD_TFILTER_CONSTRUCTOR(TNoInline) { }

    // Force function not inlined, in order to optimize WDK compilation time and running speed.
    template<class T_IPIN> __declspec(noinline) bool Process (T_IPIN & ipin)
    {
        if (ipin.check_read())
        {
            Next()->Process(ipin);
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(TModSink, CF_TxSampleBuffer, CF_Error);
template<TSINK_ARGS>
class TModSink : public TSink<TSINK_PARAMS>
{
	CTX_VAR_RO (COMPLEX8 *, tx_sample_buf );	  	
	CTX_VAR_RO (uint,       tx_sample_buf_size ); 

	CTX_VAR_RW (uint,       tx_sample_cnt );
	CTX_VAR_RW (ulong,      error_code ); 

protected:
	// internal states
	vcb *  m_pIndex;

	FINL void _init () {
		tx_sample_cnt = 0;
		m_pIndex   = NULL;

		if ( tx_sample_buf == NULL ) {
			error_code = E_ERROR_PARAMETER;
			return;
		}
		m_pIndex = (vcb*) tx_sample_buf;
	}

public:
	DEFINE_IPORT(COMPLEX8, 8);
	
public:
    REFERENCE_LOCAL_CONTEXT(TModSink);

    STD_TSINK_CONSTRUCTOR(TModSink)
        BIND_CONTEXT(CF_TxSampleBuffer::tx_sample_buf, tx_sample_buf )
        BIND_CONTEXT(CF_TxSampleBuffer::tx_sample_buf_size, tx_sample_buf_size )
        BIND_CONTEXT(CF_TxSampleBuffer::tx_sample_cnt, tx_sample_cnt )
        // CF_Error
        BIND_CONTEXT(CF_Error::error_code, error_code)
    {
        _init();
    }
	STD_TSINK_RESET()
    {
    	_init ();
    }
	STD_TSINK_FLUSH() {
	}

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            vcb* input = (vcb*)ipin.peek();
			*m_pIndex = *input;
			ipin.pop();
			m_pIndex ++; tx_sample_cnt += 8;
        }
        return true;
    }
};

class ModSinkDesc {
protected:
    // INPUT
    COMPLEX16** ppBuf;
    uint *      pBufSize;
    // OUTPUT	
    uint *      pCount;
    
    ModSinkDesc()
        : ppBuf(NULL)
        , pBufSize(NULL)
        , pCount(NULL)
    { }
public:
    FINL void Init(COMPLEX16*& txbuf, uint& txbufsize, uint& txcount)
    {
        pCount = &txcount;
        ppBuf = &txbuf;
        pBufSize = &txbufsize;
    }
};

// Note: txbuf should live through the TModSink1 object life time
// user may change txbuf during this life time.
DEFINE_LOCAL_CONTEXT(TModSink1, CF_Error);
template<TSINK_ARGS>
class TModSink1 : public TSink<TSINK_PARAMS>, public ModSinkDesc
{
    CTX_VAR_RW (ulong,      error_code ); 
protected:
    // internal states
    vcs *  m_pIndex;

    FINL void _init () {
        m_pIndex   = NULL;

        if ( ppBuf == NULL || *ppBuf == NULL ) {
            error_code = E_ERROR_PARAMETER;
            return;
        }
        m_pIndex = (vcs*) (*ppBuf + *pCount);
    }

public:
    DEFINE_IPORT(COMPLEX16, 4);
    
public:
    REFERENCE_LOCAL_CONTEXT(TModSink1);

    STD_TSINK_CONSTRUCTOR(TModSink1)
        BIND_CONTEXT(CF_Error::error_code, error_code)
    {
        _init();
    }
    STD_TSINK_RESET()
    {
        _init ();
    }
    STD_TSINK_FLUSH() {
    }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            assert(*ppBuf != NULL);
            vcs *input = (vcs *)ipin.peek();
            assert((COMPLEX16 *)m_pIndex < *ppBuf + *pBufSize);
            *m_pIndex = *input;
            ipin.pop();
            m_pIndex ++; *pCount += vcs::size;
        }
        return true;
    }
};


DEFINE_LOCAL_CONTEXT(TPackSample16to8, CF_VOID );
template<TFILTER_ARGS>
class TPackSample16to8 : public TFilter<TFILTER_PARAMS>
{
public:
   DEFINE_IPORT(COMPLEX16, 8);
   DEFINE_OPORT(COMPLEX8,  8);

public:
    REFERENCE_LOCAL_CONTEXT(TPackSample16to8);
    STD_TFILTER_CONSTRUCTOR(TPackSample16to8){}
	STD_TFILTER_RESET(){}
	STD_TFILTER_FLUSH(){}

    BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
            vcs s1, s2;
            vcs *input = (vcs*)ipin.peek();
            s1 = input[0];
            s2 = input[1];
            ipin.pop();

            vcb b = (vcb)saturated_pack((vs&)s1, (vs&)s2);

            vcb* output = (vcb*)opin().append();
			*output = b;
            Next()->Process(opin());
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(TSample8to16, CF_VOID);
template<TFILTER_ARGS>
class TSample8to16 : public TFilter<TFILTER_PARAMS>
{
public: // inport and outport
    DEFINE_IPORT(COMPLEX8, 1);
    DEFINE_OPORT(COMPLEX16, 1);

public:
    REFERENCE_LOCAL_CONTEXT(TSample8to16);
    STD_TFILTER_CONSTRUCTOR(TSample8to16)
    {
    }
	STD_TFILTER_RESET() {}
	STD_TFILTER_FLUSH() {}	

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            COMPLEX8 *input = ipin.peek();
            COMPLEX16* output = opin().push();

			output->re = input->re;
			output->im = input->im;
			
            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(TUnpackBits, CF_VOID );
template<TFILTER_ARGS>
class TUnpackBits : public TFilter<TFILTER_PARAMS>
{
public:
   DEFINE_IPORT(uchar, 1);
   DEFINE_OPORT(uchar, 8);

public:
    REFERENCE_LOCAL_CONTEXT(TUnpackBits);
    STD_TFILTER_CONSTRUCTOR(TUnpackBits){}
	STD_TFILTER_RESET(){}

    BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
            uchar in = *ipin.peek();
            ipin.pop();
            uchar *out = opin().append();

            for (int i = 0; i < 8; i++)
            {
                out[i] = in & 1;
                in >>= 1;
            }
            Next()->Process(opin());
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(TPackBits, CF_VOID );
template<TFILTER_ARGS>
class TPackBits : public TFilter<TFILTER_PARAMS>
{
public:
   DEFINE_IPORT(uchar, 8);
   DEFINE_OPORT(uchar, 1);

public:
    REFERENCE_LOCAL_CONTEXT(TPackBits);
    STD_TFILTER_CONSTRUCTOR(TPackBits){}
	STD_TFILTER_RESET(){}

    BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
            uchar *in = ipin.peek();
            uchar out = 0;

            out = 0;
            for (int i = 0; i < 8; i++)
            {
                out |= (in[i] & 1) << i;
            }
            opin().append(&out);
            Next()->Process(opin());
            ipin.pop();
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(TTeeEx, CF_VOID);
template<TDEMUX2_ARGS>
class TTeeEx : public TDemux<TDEMUX2_PARAMS>
{
public: // inport and outport
    typedef typename T_NEXT0::iport_traits::type TYPE;
    static const size_t BURST = T_NEXT0::iport_traits::burst;
    DEFINE_IPORT (TYPE, BURST);
    DEFINE_OPORTS(0, TYPE, BURST);
    DEFINE_OPORTS(1, TYPE, BURST);

private:
    CCASSERT(BURST == T_NEXT1::iport_traits::burst); // Add this connection requirement, to enable reusing pinqueue

public:
    REFERENCE_LOCAL_CONTEXT(TTeeEx);
    STD_DEMUX2_CONSTRUCTOR(TTeeEx)
    {
    }
    STD_TDEMUX2_RESET() {}

    BOOL_FUNC_PROCESS(ipin)
    {
        if (ipin.check_read())
        {
            const TYPE *input = ipin.peek();
            Next0()->Process(ipin.clone());
            Next1()->Process(ipin);
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(TStreamFork, CF_VOID);
template<size_t NSTREAM>
class TStreamFork
{
public:
template<TDEMUX10_ARGS>
class Filter : public TDemux<TDEMUX10_PARAMS>
{
public: // inport and outport
    typedef typename T_NEXT0::iport_traits::type TYPE;
    static const size_t BURST = T_NEXT0::iport_traits::burst;
    DEFINE_IPORT (TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(0, TYPE, BURST);
    DEFINE_OPORTS(1, TYPE, BURST);
    DEFINE_OPORTS(2, TYPE, BURST);
    DEFINE_OPORTS(3, TYPE, BURST);
    DEFINE_OPORTS(4, TYPE, BURST);
    DEFINE_OPORTS(5, TYPE, BURST);
    DEFINE_OPORTS(6, TYPE, BURST);
    DEFINE_OPORTS(7, TYPE, BURST);
    DEFINE_OPORTS(8, TYPE, BURST);
    DEFINE_OPORTS(9, TYPE, BURST);

public:
    REFERENCE_LOCAL_CONTEXT(Filter);
    STD_DEMUX10_CONSTRUCTOR(Filter) { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            if (NON_DUMMYBRICK(0)) memcpy(opin0().append(), ipin.peek(0), sizeof(TYPE) * BURST);
            if (NON_DUMMYBRICK(1)) memcpy(opin1().append(), ipin.peek(1), sizeof(TYPE) * BURST);
            if (NON_DUMMYBRICK(2)) memcpy(opin2().append(), ipin.peek(2), sizeof(TYPE) * BURST);
            if (NON_DUMMYBRICK(3)) memcpy(opin3().append(), ipin.peek(3), sizeof(TYPE) * BURST);
            if (NON_DUMMYBRICK(4)) memcpy(opin4().append(), ipin.peek(4), sizeof(TYPE) * BURST);
            if (NON_DUMMYBRICK(5)) memcpy(opin5().append(), ipin.peek(5), sizeof(TYPE) * BURST);
            if (NON_DUMMYBRICK(6)) memcpy(opin6().append(), ipin.peek(6), sizeof(TYPE) * BURST);
            if (NON_DUMMYBRICK(7)) memcpy(opin7().append(), ipin.peek(7), sizeof(TYPE) * BURST);
            if (NON_DUMMYBRICK(8)) memcpy(opin8().append(), ipin.peek(8), sizeof(TYPE) * BURST);
            if (NON_DUMMYBRICK(9)) memcpy(opin9().append(), ipin.peek(9), sizeof(TYPE) * BURST);
            ipin.pop();
            if (NON_DUMMYBRICK(0)) Next0()->Process(opin0());
            if (NON_DUMMYBRICK(1)) Next1()->Process(opin1());
            if (NON_DUMMYBRICK(2)) Next2()->Process(opin2());
            if (NON_DUMMYBRICK(3)) Next3()->Process(opin3());
            if (NON_DUMMYBRICK(4)) Next4()->Process(opin4());
            if (NON_DUMMYBRICK(5)) Next5()->Process(opin5());
            if (NON_DUMMYBRICK(6)) Next6()->Process(opin6());
            if (NON_DUMMYBRICK(7)) Next7()->Process(opin7());
            if (NON_DUMMYBRICK(8)) Next8()->Process(opin8());
            if (NON_DUMMYBRICK(9)) Next9()->Process(opin9());
        }
        return true;
    }
}; };

// Note:
// Limitation of usage:
// 1. The upstream blocks (in each branch) connect to this brick directly, they should be 'Process'-ed in round-robin
// 2. The input burst size must be equal to the number of elements each upstream branch generated in one round-robin running
template<size_t NSTREAM, size_t BURST>
class TStreamJoin
{
public:
template<TFILTER_ARGS>
class Filter : public TFilter<TFILTER_PARAMS>
{
public:
    typedef typename T_NEXT::iport_traits::type TYPE;
    DEFINE_IPORT(TYPE, BURST);
    DEFINE_OPORT(TYPE, BURST, NSTREAM);

    STD_TFILTER_CONSTRUCTOR(Filter) { _init(); }
    STD_TFILTER_RESET() { _init(); }

    BOOL_FUNC_PROCESS(ipin)
    {
        CCASSERT(TYPEOF(ipin)::qsize == BURST);
        if (ipin.count() < BURST) return TRUE;

        const TYPE *in = ipin.peek();
        memcpy(opin().write(iss), in, sizeof(TYPE) * BURST);
        iss++;
        ipin.clear();

        if (iss == NSTREAM)
        {
            opin().append();
            Next()->Process(opin());
            _init();
        }
        return TRUE;
    }

private:
    size_t iss;
    CCASSERT(NSTREAM >= 2);

    FINL void _init()
    {
        iss = 0;
    }
}; };

template<size_t NSTREAM, size_t BURST>
class TStreamConcat
{
public:
template<TFILTER_ARGS>
class Filter : public TFilter<TFILTER_PARAMS>
{
public:
    typedef typename T_NEXT::iport_traits::type TYPE;
    DEFINE_IPORT(TYPE, BURST, NSTREAM);
    DEFINE_OPORT(TYPE, BURST * NSTREAM);

    STD_TFILTER_CONSTRUCTOR(Filter) { }
    STD_TFILTER_RESET() { }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
            TYPE *out = opin().append();

            size_t iss;
            for (iss = 0; iss < NSTREAM; iss++)
            {
                const TYPE *in = ipin.peek(iss);
                memcpy(out, in, sizeof(TYPE) * BURST);
                out += BURST;
            }
            ipin.pop();
            Next()->Process(opin());
        }
        return TRUE;
    }

private:
    CCASSERT(NSTREAM >= 2);
}; };
