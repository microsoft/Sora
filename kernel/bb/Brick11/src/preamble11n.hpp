#pragma once
#include "brick.h"
#include "_b_lstf.h"
#include "_b_lltf.h"
#include "_b_htstf.h"
#include "_b_htltf.h"

// TRICK: this is multiple output ports source brick, which mixes the ISource interface with TDemux template
DEFINE_LOCAL_CONTEXT(LSrc, CF_VOID);
template<TDEMUX2_ARGS>
class LSrc: public TDemux<TDEMUX2_PARAMS>
{
public:
    DEFINE_IPORT(void, 1);
    DEFINE_OPORTS(0, COMPLEX16, 320 * 2);
    DEFINE_OPORTS(1, COMPLEX16, 320 * 2);
	
public:
    REFERENCE_LOCAL_CONTEXT(LSrc);
    STD_DEMUX2_CONSTRUCTOR(LSrc) { }
    STD_TDEMUX2_RESET() {}
    FINL void Flush() { TDemux::Reset(); }
    BOOL_FUNC_PROCESS(ipin)
    {
       	COMPLEX16* op0 = opin0().append();
       	COMPLEX16* op1 = opin1().append();
        _lstf.get_stf_1(op0);
        op0 += 2 * 160;
        _lltf.get_ltf_1(op0);
		Next0()->Process(opin0());

        _lstf.get_stf_2(op1);
        op1 += 2 * 160;
        _lltf.get_ltf_2(op1);
		Next1()->Process(opin1());
    	return FALSE;		
    }

private:
    L_STF _lstf;
    L_LTF _lltf;
};

// TRICK: this is multiple output ports source brick, which mixes the ISource interface with TDemux template
DEFINE_LOCAL_CONTEXT(HTSrc, CF_VOID);
template<TDEMUX2_ARGS>
class HTSrc: public TDemux<TDEMUX2_PARAMS>
{
public:
    DEFINE_IPORT(void, 1);
    DEFINE_OPORTS(0, COMPLEX16, 160 * 3);
    DEFINE_OPORTS(1, COMPLEX16, 160 * 3);
	
public:
    REFERENCE_LOCAL_CONTEXT(HTSrc);
    STD_DEMUX2_CONSTRUCTOR(HTSrc) { }
    STD_TDEMUX2_RESET() { }
    FINL void Flush() { TDemux::Reset(); }
    BOOL_FUNC_PROCESS(ipin)
    {
       	COMPLEX16* op1 = opin0().append();
       	COMPLEX16* op2 = opin1().append();

        _htstf.get_stf_1(op1);
        op1 += 160;
        _htltf.get_ltf_11(op1);
        op1 += 160;
        _htltf.get_ltf_12(op1);
		Next0()->Process(opin0());

        _htstf.get_stf_2(op2);
        op2 += 160;
        _htltf.get_ltf_21(op2);
        op2 += 160;
        _htltf.get_ltf_22(op2);
		Next1()->Process(opin1());
    	return FALSE;		
    }

private:
    HT_STF _htstf;
    HT_LTF _htltf;
};
