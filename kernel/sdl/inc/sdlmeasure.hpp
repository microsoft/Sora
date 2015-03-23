#pragma once

#include <sora.h>
#include <brick.h>
#include <dspcomm.h>

#include <sdlplot.hpp>

// Power meter
DEFINE_LOCAL_CONTEXT(TPowerMeter, CF_VOID);
template<size_t N = vcs::size>
class TPowerMeter
{
public:
REFERENCE_LOCAL_CONTEXT(TPowerMeter);

template<TFILTER_ARGS>
class Filter : public TFilter<TFILTER_PARAMS> 
{
public:
    DEFINE_IPORT(COMPLEX16, N);
    DEFINE_OPORT(int, 1);
        
public:
    
    STD_TFILTER_CONSTRUCTOR(Filter) {}
	STD_TFILTER_RESET() {}
	STD_TFILTER_FLUSH() {}
    	
    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            uint ave_power = 0;

            const uint vlen = N / vcs::size;

            // vector part
            vcs * pi = (vcs*) ipin.peek ();
            vi sum, e;
            for (uint i =0; i<vlen; i++) {
                e = SquaredNorm (pi[i]);
                sum = hadd (e) ;
                ave_power += (sum[0] / N);
            }

            
            if ( (N % vcs::size) != 0 ) {
               
                COMPLEX16* pii = (COMPLEX16*) pi;
                for ( uint i=vlen*vcs::size; i<N; i++ )
                {
                    int e = SquaredNorm (pii[i]);
                    ave_power += e / N;
                }
            } 

            ipin.pop();
            *opin().append () = ave_power;
            Next()->Process ( opin());
        }
        return true;
    }
};
};

// Spectrum meter
DEFINE_LOCAL_CONTEXT(TSpecMeter, CF_VOID);
template<size_t N>
class TSpecMeter
{
public:
REFERENCE_LOCAL_CONTEXT(TSpecMeter);

template<TFILTER_ARGS>
class Filter : public TFilter<TFILTER_PARAMS> 
{
    
public:
    DEFINE_IPORT(COMPLEX16, N);
    DEFINE_OPORT(int, N);
        
public:
    
    STD_TFILTER_CONSTRUCTOR(Filter) 
    {}
    
	STD_TFILTER_RESET() {}
	STD_TFILTER_FLUSH() {}
    
	
    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const uint vlen = (N / 4)*4;

            // vector part
            vcs * pi = (vcs*) ipin.peek ();
            vcs temp[vlen];

            FFT<vlen>(pi, temp);

            int* po = opin().append();
            memset (po, 0, N*sizeof(int));

            vi* po1 = (vi*) po;
            
            for ( int i=0;i < vlen / 4; i++ ) {
                po1[i] = SquaredNorm ( temp[i] );
            }
            // clear DC
            po[0] = 0;
            
            ipin.pop();
            Next()->Process ( opin());
        }
        return true;
    }
};
};


// Spectrum meter
DEFINE_LOCAL_CONTEXT(TInspectTube, CF_VOID);
template<class plotter>
class TInspectTube
{
public:
template<TFILTER_ARGS>
class Filter : public TFilter<TFILTER_PARAMS> , public named_plotter
{    
public:
    static const size_t NSTREAM = T_NEXT::iport_traits::nstream;
    static const size_t BURST = T_NEXT::iport_traits::burst;
    typedef typename T_NEXT::iport_traits::type TYPE;
    DEFINE_IPORT(TYPE, BURST, NSTREAM);
    DEFINE_OPORT(TYPE, 1); // Downstream brick does not care about the burst size

	char m_name[MAX_NAME_LEN];

public:
	REFERENCE_LOCAL_CONTEXT(TInspectTube);
        
public:    
    STD_TFILTER_CONSTRUCTOR(Filter) {}
    
	STD_TFILTER_RESET() {}
	STD_TFILTER_FLUSH() {}
    
	plotter m_plt;
    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
			for ( int i=0; i<NSTREAM; i++) {
				const TYPE* pi = ipin.peek (i);
				m_plt ( GetName(), i, pi, BURST); 
			}

            // just forward
            Next()->Process (ipin);
        }
        return true;
    }
};
};

