#pragma once

DEFINE_LOCAL_CONTEXT(TTee, CF_VOID);
template<size_t N>
class TTee {
public:
template<TDEMUX2_ARGS>
class Filter : public TDemux<TDEMUX2_PARAMS>
{
public:
	DEFINE_IPORT (COMPLEX16, N);
    DEFINE_OPORTS(0, COMPLEX16, N);
    DEFINE_OPORTS(1, COMPLEX16, N);
    
public:
    STD_DEMUX2_CONSTRUCTOR(Filter) { }

    STD_TDEMUX2_RESET(){ }

    FINL void Flush()  { }

	BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
#if 0        
            const int vlen = N / vcs::size;
            COMPLEX16 * pii = ipin.peek();
			vcs* pi = (vcs*) pii;

            // port 0
            COMPLEX16 * poo0 = opin0().append();
            vcs* po0 = (vcs*) poo0;
            
            rep<vlen>::vmemcpy (po0, pi);
            if ( N % vcs::size != 0 ) {
                for (int i=vlen*vcs::size; i<N; i++ ) {
                    poo0[i] = pii[i];
                }
            }
            Next0()->Process (opin0());
			
            // port 1
            COMPLEX16 * poo1 = opin1().append();
            vcs* po1 = (vcs*) poo1;
            
            rep<vlen>::vmemcpy (po1, pi);
            if ( N % vcs::size != 0 ) {
                for (int i=vlen*vcs::size; i<N; i++ ) {
                    poo1[i] = pii[i];
                }
            }
            Next1()->Process (opin1());
#endif
            COMPLEX16 * pi = ipin.peek();
            COMPLEX16 * po = opin0().append();
            rep<N>::vmemcpy ( po, pi );
            Next0()->Process ( opin0());

            po = opin1().append();
            rep<N>::vmemcpy ( po, pi );
            Next1()->Process ( opin1());

			ipin.pop();
        }
        return true;
    }
};
};


DEFINE_LOCAL_CONTEXT(TFourWay, CF_VOID);
template<size_t N>
class TFourWay {
public:
template<TDEMUX3_ARGS>
class Filter : public TDemux<TDEMUX3_PARAMS>
{
public:
	DEFINE_IPORT (COMPLEX16, N);
    DEFINE_OPORTS(0, COMPLEX16, N);
    DEFINE_OPORTS(1, COMPLEX16, N);
    DEFINE_OPORTS(2, COMPLEX16, N);    
    
public:
    STD_DEMUX3_CONSTRUCTOR(Filter) { }

    STD_TDEMUX3_RESET(){ }

    FINL void Flush()  { }

    
	BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
            COMPLEX16 * pi = ipin.peek();
            COMPLEX16 * po;

            // port 0
            po = opin0().append();
            rep<N>::vmemcpy (po, pi);
            Next0()->Process (opin0());

            // port 1
            po = opin1().append();
            rep<N>::vmemcpy (po, pi);
            Next1()->Process (opin1());

            // port 2
            po = opin2().append();
            rep<N>::vmemcpy (po, pi);
            Next2()->Process (opin2());
            
			ipin.pop();
        }
        return true;
    }
};
};

