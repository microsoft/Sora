#pragma once
#include "brick.h"
#include "_b_stream_parser.h"

DEFINE_LOCAL_CONTEXT(TStreamParserBPSK_12, CF_VOID);
template<TDEMUX2_ARGS>
class TStreamParserBPSK_12: public TDemux<TDEMUX2_PARAMS>
{
    stream_paser_bpsk_2ss _stream_parser;

public:
    DEFINE_IPORT (UCHAR, 13);
    DEFINE_OPORTS(0, UCHAR, 7); // 6.5 bytes effectively per symbol
    DEFINE_OPORTS(1, UCHAR, 7); // 6.5 bytes effectively per symbol
    
public:
    REFERENCE_LOCAL_CONTEXT(TStreamParserBPSK_12);
    STD_DEMUX2_CONSTRUCTOR(TStreamParserBPSK_12) { }
    STD_TDEMUX2_RESET() { }
   
    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const UCHAR *in  = ipin.peek();
            UCHAR *out1 = opin0().append();
            UCHAR *out2 = opin1().append();

            _stream_parser(in, out1, out2);

            ipin.pop();
            Next0()->Process(opin0());
            Next1()->Process(opin1());
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(TStreamParserQPSK_12, CF_VOID);
template<TDEMUX2_ARGS>
class TStreamParserQPSK_12: public TDemux<TDEMUX2_PARAMS>
{
    stream_paser_qpsk_2ss _stream_parser;

public:
    DEFINE_IPORT (UCHAR, 26);
    DEFINE_OPORTS(0, UCHAR, 13); // 13 bytes per symbol
    DEFINE_OPORTS(1, UCHAR, 13); // 13 bytes per symbol
    
public:
    REFERENCE_LOCAL_CONTEXT(TStreamParserQPSK_12);
    STD_DEMUX2_CONSTRUCTOR(TStreamParserQPSK_12) { }
    STD_TDEMUX2_RESET() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const UCHAR *in  = ipin.peek();
            UCHAR *out1 = opin0().append();
            UCHAR *out2 = opin1().append();

            _stream_parser(in, out1, out2);

            ipin.pop();
            Next0()->Process(opin0());
            Next1()->Process(opin1());
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(TStreamParserQAM16_12, CF_VOID);
template<TDEMUX2_ARGS>
class TStreamParserQAM16_12: public TDemux<TDEMUX2_PARAMS>
{
    stream_paser_16qam_2ss _stream_parser;

public:
    DEFINE_IPORT (UCHAR, 52);
    DEFINE_OPORTS(0, UCHAR, 26); // 26 bytes effectively per symbol
    DEFINE_OPORTS(1, UCHAR, 26); // 26 bytes effectively per symbol
    
public:
    REFERENCE_LOCAL_CONTEXT(TStreamParserQAM16_12);
    STD_DEMUX2_CONSTRUCTOR(TStreamParserQAM16_12) { }
    STD_TDEMUX2_RESET() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const UCHAR *in  = ipin.peek();
            UCHAR *out1 = opin0().append();
            UCHAR *out2 = opin1().append();

            _stream_parser(in, out1, out2);

            ipin.pop();
            Next0()->Process(opin0());
            Next1()->Process(opin1());
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(TStreamParserQAM64_12, CF_VOID);
template<TDEMUX2_ARGS>
class TStreamParserQAM64_12: public TDemux<TDEMUX2_PARAMS>
{
    stream_paser_64qam_2ss _stream_parser;

public:
    DEFINE_IPORT (UCHAR, 78);
    DEFINE_OPORTS(0, UCHAR, 39); // 39 bytes effectively per symbol
    DEFINE_OPORTS(1, UCHAR, 39); // 39 bytes effectively per symbol
    
public:
    REFERENCE_LOCAL_CONTEXT(TStreamParserQAM64_12);
    STD_DEMUX2_CONSTRUCTOR(TStreamParserQAM64_12) { }
    STD_TDEMUX2_RESET() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const UCHAR *in  = ipin.peek();
            UCHAR *out1 = opin0().append();
            UCHAR *out2 = opin1().append();

            _stream_parser(in, out1, out2);

            ipin.pop();
            Next0()->Process(opin0());
            Next1()->Process(opin1());
        }
        return true;
    }
};
