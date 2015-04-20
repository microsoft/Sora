#pragma once
#include "brick.h"

DEFINE_LOCAL_CONTEXT(TCSD, CF_VOID);
template<int ncsd>
class TCSD
{
public:
template<TFILTER_ARGS>
class Filter: public TFilter<TFILTER_PARAMS>
{
public:
    DEFINE_IPORT(COMPLEX16, 128);
    DEFINE_OPORT(COMPLEX16, 128);
    
public:
    REFERENCE_LOCAL_CONTEXT(Filter);
    STD_TFILTER_CONSTRUCTOR(Filter) { }
    STD_TFILTER_RESET() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const COMPLEX16 *ip  = ipin.peek();
            COMPLEX16 *op = opin().append();

            // produce a new symbol
            csd(op, ip, ncsd);

            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }

private:
    __forceinline static void csd(COMPLEX16 *output, const COMPLEX16 *input, int vncs)
    {
        vcs *vinput = (vcs *)input;
        vcs *voutput = (vcs *)output;
        const int vnsubcarrier = 128 / vcs::size;
        int i = 0;
        for (; i < vnsubcarrier - vncs; i++)
        {
            voutput[i + vncs] = vinput[i];
        }
        for (int j = 0; i < vnsubcarrier; i++, j++)
        {
            voutput[j] = vinput[i];
        }
    }
}; };
