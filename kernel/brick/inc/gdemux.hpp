#pragma once
#include "const.h"
#include "vector128.h"
#include "brick.h"
#include "operator_repeater.h"

////////////////////////////////////////////////////
//
// General Demux
//
////////////////////////////////////////////////////
template<size_t NDEMUX, typename TYPE, int BURST, typename Selector>
class TGDemux {
public:
template<TDEMUX10_ARGS>
class Filter : public TDemux<TDEMUX10_PARAMS>
{
public:
    static const size_t NSTREAM = T_NEXT0::iport_traits::nstream;
	DEFINE_IPORT (   TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(0, TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(1, TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(2, TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(3, TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(4, TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(5, TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(6, TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(7, TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(8, TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(9, TYPE, BURST, NSTREAM);
    
public:
    STD_DEMUX10_CONSTRUCTOR(Filter) { }
    STD_TDEMUX10_RESET(){ }
    FINL void Flush()
    {
		int i_port = s(ctx());
		switch (i_port) {
        case PORT_0:  FlushPort(0); break;
		case PORT_1:  FlushPort(1); break;
		case PORT_2:  FlushPort(2); break;
		case PORT_3:  FlushPort(3); break;
		case PORT_4:  FlushPort(4); break;
		case PORT_5:  FlushPort(5); break;
		case PORT_6:  FlushPort(6); break;
		case PORT_7:  FlushPort(7); break;
		case PORT_8:  FlushPort(8); break;
		case PORT_9:  FlushPort(9); break;
        default: NODEFAULT;
        }
    }

	BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
			int i_port = s(ctx());
			switch (i_port) {
			case PORT_0:  CopyPinQueue(opin0(), ipin); Next0()->Process(opin0()); break;
			case PORT_1:  CopyPinQueue(opin1(), ipin); Next1()->Process(opin1()); break;
			case PORT_2:  CopyPinQueue(opin2(), ipin); Next2()->Process(opin2()); break;
			case PORT_3:  CopyPinQueue(opin3(), ipin); Next3()->Process(opin3()); break;
			case PORT_4:  CopyPinQueue(opin4(), ipin); Next4()->Process(opin4()); break;
			case PORT_5:  CopyPinQueue(opin5(), ipin); Next5()->Process(opin5()); break;
			case PORT_6:  CopyPinQueue(opin6(), ipin); Next6()->Process(opin6()); break;
			case PORT_7:  CopyPinQueue(opin7(), ipin); Next7()->Process(opin7()); break;
			case PORT_8:  CopyPinQueue(opin8(), ipin); Next8()->Process(opin8()); break;
			case PORT_9:  CopyPinQueue(opin9(), ipin); Next9()->Process(opin9()); break;
            default: NODEFAULT;
            }
        }
        return true;
    }

    Selector& GetSelector() { return s; }

private:
	Selector s;
			
    template<typename OPIN, typename IPIN>
    FINL void CopyPinQueue(OPIN& opin, IPIN& ipin)
    {
        size_t iss = 0;
        for (iss = 0; iss < NSTREAM; iss++)
        {
            rep_memcpy<BURST, TYPE>(opin.write(iss), ipin.peek(iss));
        }
        opin.append();
		ipin.pop();
    }
}; };

////////////////////////////////////////////////////
//
// Quick Demux: demux without buffer copy
//
////////////////////////////////////////////////////
template<size_t NDEMUX, typename Selector>
class TQDemux {
public:
template<TDEMUX10_ARGS>
class Filter : public TDemux<TDEMUX10_PARAMS>
{
public:
    typedef typename T_NEXT0::iport_traits::type TYPE;
    static const size_t BURST = T_NEXT0::iport_traits::burst;
    static const size_t NSTREAM = T_NEXT0::iport_traits::nstream;
	DEFINE_IPORT (   TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(0, TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(1, TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(2, TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(3, TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(4, TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(5, TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(6, TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(7, TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(8, TYPE, BURST, NSTREAM);
    DEFINE_OPORTS(9, TYPE, BURST, NSTREAM);

private:
    // Add these connection requirements, to enable reusing pinqueue
    CCASSERT(!NON_DUMMYBRICK(1) || BURST == T_NEXT1::iport_traits::burst);
    CCASSERT(!NON_DUMMYBRICK(2) || BURST == T_NEXT2::iport_traits::burst);
    CCASSERT(!NON_DUMMYBRICK(3) || BURST == T_NEXT3::iport_traits::burst);
    CCASSERT(!NON_DUMMYBRICK(4) || BURST == T_NEXT4::iport_traits::burst);
    CCASSERT(!NON_DUMMYBRICK(5) || BURST == T_NEXT5::iport_traits::burst);
    CCASSERT(!NON_DUMMYBRICK(6) || BURST == T_NEXT6::iport_traits::burst);
    CCASSERT(!NON_DUMMYBRICK(7) || BURST == T_NEXT7::iport_traits::burst);
    CCASSERT(!NON_DUMMYBRICK(8) || BURST == T_NEXT8::iport_traits::burst);
    CCASSERT(!NON_DUMMYBRICK(9) || BURST == T_NEXT9::iport_traits::burst);
    
public:
    STD_DEMUX10_CONSTRUCTOR(Filter) { }
    STD_TDEMUX10_RESET(){ }
    FINL void Flush()
    {
		int i_port = s(ctx());
		switch (i_port) {
        case PORT_0:  Next0()->Flush(); break;
		case PORT_1:  Next1()->Flush(); break;
		case PORT_2:  Next2()->Flush(); break;
		case PORT_3:  Next3()->Flush(); break;
		case PORT_4:  Next4()->Flush(); break;
		case PORT_5:  Next5()->Flush(); break;
		case PORT_6:  Next6()->Flush(); break;
		case PORT_7:  Next7()->Flush(); break;
		case PORT_8:  Next8()->Flush(); break;
		case PORT_9:  Next9()->Flush(); break;
        default: NODEFAULT;
        }
    }

	BOOL_FUNC_PROCESS(ipin)
    {
        if (ipin.check_read())
        {
			int i_port = s(ctx());
			switch (i_port) {
			case PORT_0:  Next0()->Process(ipin); break;
			case PORT_1:  Next1()->Process(ipin); break;
			case PORT_2:  Next2()->Process(ipin); break;
			case PORT_3:  Next3()->Process(ipin); break;
			case PORT_4:  Next4()->Process(ipin); break;
			case PORT_5:  Next5()->Process(ipin); break;
			case PORT_6:  Next6()->Process(ipin); break;
			case PORT_7:  Next7()->Process(ipin); break;
			case PORT_8:  Next8()->Process(ipin); break;
			case PORT_9:  Next9()->Process(ipin); break;
            default: NODEFAULT;
            }
        }
        return true;
    }

    Selector& GetSelector() { return s; }
private:
	Selector s;
}; };
