#pragma once

#include "vector128.h"
#include "brick.h"
#include "44MTo40M.hpp"
#include "40MTo44M.hpp"

DEFINE_LOCAL_CONTEXT(TUpsample40MTo44M, CF_VOID);
template<TFILTER_ARGS>
class TUpsample40MTo44M : public TFilter<TFILTER_PARAMS>
{
public:
    REFERENCE_LOCAL_CONTEXT(TUpsample40MTo44M);
    DEFINE_IPORT(COMPLEX16, 160);
    DEFINE_OPORT(COMPLEX16, 176);
    STD_TFILTER_CONSTRUCTOR(TUpsample40MTo44M) { }
    STD_TFILTER_RESET() { }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
            const COMPLEX16 *input = ipin.peek();
            COMPLEX16 *output = opin().append();
            up40to44(input, output);
            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }
};

// Downsample from 44MHz to 40MHz
DEFINE_LOCAL_CONTEXT(TDownSample44_40, CF_VOID);
template<TFILTER_ARGS>
class TDownSample44_40 : public TFilter<TFILTER_PARAMS>
{
    Down44to40 Resampler;
public:
    REFERENCE_LOCAL_CONTEXT(TDownSample44_40);
    DEFINE_IPORT(COMPLEX16, 28);
    DEFINE_OPORT(COMPLEX16, 28);
    STD_TFILTER_CONSTRUCTOR(TDownSample44_40)
    {
    }

    BOOL_FUNC_PROCESS(ipin)
    {
        COMPLEX16 *p40MStream;
        while (ipin.check_read())
        {
            SignalBlock& block44M = *(SignalBlock *)ipin.peek();
            Resampler.Resample(block44M);
            ipin.pop();

            if ((p40MStream = Resampler.GetOutStream(28)) != NULL)
            {
                COMPLEX16 *out = opin().append();
                memcpy(out, p40MStream, sizeof(COMPLEX16) * 28);
                Next()->Process(opin());
            }
        }
        return true;
    }
};
