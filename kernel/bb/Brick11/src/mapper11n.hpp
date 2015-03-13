#pragma once
#include "tpltrick.h"
#include "brick.h"
#include "complex_ext.h"
#include "mapper11a.hpp"

DEFINE_LOCAL_CONTEXT(TSigMap11n, CF_VOID);
template<TFILTER_ARGS>
class TSigMap11n: public TFilter<TFILTER_PARAMS>
{
public:
    DEFINE_IPORT (UCHAR, 6 * 3);
    DEFINE_OPORT (COMPLEX16, 48 * 3); // 3 symbols

public:
    REFERENCE_LOCAL_CONTEXT(TSigMap11n);
    STD_TFILTER_CONSTRUCTOR(TSigMap11n)
    {
        opin().zerobuf();
    }

    STD_TFILTER_RESET() { }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const UCHAR *p  = ipin.peek();
            COMPLEX16 *op = opin().append();

            int i;
            for (i = 0; i < 6; i++)
            {
                mapper.MapBPSK(p[i], op);
                op += 8;
            }

            for (i = 6; i < 6 * 3; i++)
            {
                mapper.MapQBPSK(p[i], op);
                op += 8;
            }

            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }

private:
    TMapperCore<30339> mapper;
};
