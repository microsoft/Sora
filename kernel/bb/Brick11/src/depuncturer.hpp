#pragma once
#include "demapper11a.hpp"

static const int demapper_normal   = 4;

DEFINE_LOCAL_CONTEXT(TDepuncture_34, CF_VOID);
template<TFILTER_ARGS>
class TDepuncture_34: public TFilter<TFILTER_PARAMS>
{
public:
    DEFINE_IPORT(uchar, 4);
    DEFINE_OPORT(uchar, 6);
	
public:
    REFERENCE_LOCAL_CONTEXT(TDepuncture_34);
    STD_TFILTER_CONSTRUCTOR(TDepuncture_34) { }
    STD_TFILTER_RESET() { }

    void depuncture(const uchar* input, uchar* output)
    {
        output[0] = input[0];
        output[1] = input[1];
        output[2] = input[2];
        output[3] = demapper_normal;
        output[4] = demapper_normal;
        output[5] = input[3];
    }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            uchar *in  = (uchar*) ipin.peek();
            uchar *out = (uchar*) opin().append();

            depuncture(in, out);

            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(TDepuncture_23, CF_VOID);
template<size_t N_SBIT>
class TDepuncture_23 {
public:
template<TFILTER_ARGS>
class Filter: public TFilter<TFILTER_PARAMS>
{
static const int IN_BURST  = N_SBIT;
static const int OUT_BURST = N_SBIT*4/3;
public:
    DEFINE_IPORT(uchar, IN_BURST );
    DEFINE_OPORT(uchar, OUT_BURST);
	
public:
    REFERENCE_LOCAL_CONTEXT(TDepuncture_23);
    STD_TFILTER_CONSTRUCTOR(Filter) { }
    STD_TFILTER_RESET() { }

	FINL
    void depuncture(const uchar* input, uchar* output)
    {
  		int i=0; int j=0;
		for ( int j=0; j<OUT_BURST; j+= 4 ) {
			output[j]   = input[i];
			output[j+1] = input[i+1];
			output[j+2] = input[i+2];
			output[j+3] = demapper_normal;			
			i+=3;
		}
    }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            uchar *in  = (uchar*) ipin.peek();
            uchar *out = (uchar*) opin().append();

            depuncture(in, out);

            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }
};
};
