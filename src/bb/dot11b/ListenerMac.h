#pragma once

#include "bb/bbb.h"
#include <soradsp.h>
#include "brick.h"
#include "vector128.h"
#include "stdfacade.h"

struct SignalBlockWithTouch
{
    SignalBlock block;
    FLAG        touch;
};

class CF_MacState
{
public:
    enum MAC_STATE
    {
        MAC_CS,
        MAC_RX,
    };

    FACADE_FIELD(MAC_STATE, MacState);

    FINL void Reset()
    {
        MacState() = CF_MacState::MAC_CS;
    }

    FINL void MoveState_CS_RX()
    {
        MacState() = CF_MacState::MAC_RX;
    }

    FINL void MoveState_RX_CS(bool finished, bool good)
    {
        UNREFERENCED_PARAMETER((finished, good));
        MacState() = CF_MacState::MAC_CS;
    }
};


DEFINE_LOCAL_CONTEXT(TListenerMac, CF_MacState);
template<TDEMUX2_ARGS>
class TListenerMac : public TDemux<TDEMUX2_PARAMS>
{
    // Alias to context members
    CF_MacState::MAC_STATE  &m_MacState;

    FLAG touch;

    FINL void _init()
    {
        RefCtxFunc(CF_MacState::Reset)();
    }

public:
    REFERENCE_LOCAL_CONTEXT(TListenerMac);
    DEFINE_IPORT(SignalBlockWithTouch, 1);
    DEFINE_OPORTS(0, SignalBlock, 1);
    DEFINE_OPORTS(1, SignalBlock, 1);
    STD_DEMUX2_CONSTRUCTOR(TListenerMac)
        BIND_CONTEXT(MacState, m_MacState)
    {
        _init();
    }

    FINL void Reset()
    {
        TDemux::Reset();
        _init();
    }

    FINL void Flush()
    {
        if (m_MacState == CF_MacState::MAC_CS)
        {
            opin0().pad();
            Next0()->Process(opin0());
        }
        else if (m_MacState == CF_MacState::MAC_RX)
        {
            opin1().pad();
            Next1()->Process(opin1());
        }
    }

	BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
            SignalBlockWithTouch blockt = *ipin.peek();
            ipin.pop();

            if (m_MacState == CF_MacState::MAC_CS)
            {
                touch = blockt.touch;

                opin0().append(&blockt.block);
                Next0()->Process(opin0());
            }
            else if (m_MacState == CF_MacState::MAC_RX)
            {
                opin1().append(&blockt.block);
                Next1()->Process(opin1());
            }
        }
        return true;
    }
};
