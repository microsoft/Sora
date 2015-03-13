#pragma once

#include "vector128.h"

//
// TDownSample2 
// DownSample the stream by two
//
DEFINE_LOCAL_CONTEXT(TDownSample2, CF_VOID);
template<TFILTER_ARGS>
class TDownSample2 : public TFilter<TFILTER_PARAMS>
{
public:
    static const size_t NSTREAM = T_NEXT::iport_traits::nstream;
    // inport and outport
    DEFINE_IPORT(COMPLEX16, 8, NSTREAM);
    DEFINE_OPORT(COMPLEX16, 4, NSTREAM);

public:
    REFERENCE_LOCAL_CONTEXT(TDownSample2);
    STD_TFILTER_CONSTRUCTOR(TDownSample2)
    {
    }
	STD_TFILTER_RESET() {}
	STD_TFILTER_FLUSH() {}	

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            size_t iss = 0;
            for (iss = 0; iss < NSTREAM; iss++)
            {
                vcs* pi = (vcs*) ipin.peek(iss);

                vcs t1 = permutate<0,2,0,2>(pi[0]);
			    vcs t2 = permutate<0,2,0,2>(pi[1]);
			    vcs& o = cast_ref<vcs> (opin().write (iss));
			    o = (vcs)(interleave_low ((vcui&)t1, (vcui&)t2));
            }	
            ipin.pop();
            opin().append();
            Next()->Process(opin());
        }
        return true;
    }
};
