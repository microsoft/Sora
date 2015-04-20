#pragma once
#include "const.h"
#include "vector128.h"
#include "brick.h"

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
		Selector s = Selector();
			
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
			Selector s = Selector();
			
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

private:
    template<typename OPIN, typename IPIN>
    FINL void CopyPinQueue(OPIN& opin, IPIN& ipin)
    {
        size_t iss = 0;
        for (iss = 0; iss < NSTREAM; iss++)
        {
            repex<TYPE, BURST>::vmemcpy(opin.write(iss), ipin.peek(iss));
        }
        opin.append();
		ipin.pop();
    }
}; };
