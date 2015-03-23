#pragma once
#include "brick.h"

DEFINE_LOCAL_CONTEXT(TAddGI, CF_VOID);
template<TFILTER_ARGS>
class TAddGI: public TFilter<TFILTER_PARAMS>
{
public:
    DEFINE_IPORT(COMPLEX16, 128);
    DEFINE_OPORT(COMPLEX16, 160);

public:
    REFERENCE_LOCAL_CONTEXT(TAddGI);
    STD_TFILTER_CONSTRUCTOR(TAddGI) { }
    STD_TFILTER_RESET() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const COMPLEX16 *ip  = ipin.peek();
            COMPLEX16 *op = opin().append();

            AddGI(ip, op);
            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }

private:
    __forceinline void AddGI(const COMPLEX16 *in, COMPLEX16 *out)
    {
        const int vnsubcarrier = 128 / vcs::size;
        const int vncp         = 32 / vcs::size;

        const vcs *vin = (const vcs *)in;
        vcs *vout  = (vcs *)out;
        rep_memcpy<vncp>(vout, vin + vnsubcarrier - vncp);
        rep_memcpy<vnsubcarrier>(vout + vncp, vin);
    }
};
