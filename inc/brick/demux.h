// Header file for brick frame work. This heder define demux template class and related macros
// Note:
//   Mux output pin is up to 10

#pragma once
#include "brick.h"
#include "tpltrick.h"

// Use macro to determine default next brick, in order to prevent runtime check overload
#define NON_DUMMYBRICK(INDEX) (!same_type<T_NEXT##INDEX*, DummyBrick*>::value)

// Note: it is assumed that the maximal output port is 10
template<class T_CTX
    , class T_NEXT0 = DummyBrick
    , class T_NEXT1 = DummyBrick
    , class T_NEXT2 = DummyBrick
    , class T_NEXT3 = DummyBrick
    , class T_NEXT4 = DummyBrick
    , class T_NEXT5 = DummyBrick
    , class T_NEXT6 = DummyBrick
    , class T_NEXT7 = DummyBrick
    , class T_NEXT8 = DummyBrick
    , class T_NEXT9 = DummyBrick
    >
class TDemux : public TBrick<T_CTX>, public IReferenceCounting, public IControlPoint
{
protected:
    T_NEXT0*  __next_0;
    T_NEXT1*  __next_1;
    T_NEXT2*  __next_2;
    T_NEXT3*  __next_3;
    T_NEXT4*  __next_4;
    T_NEXT5*  __next_5;
    T_NEXT6*  __next_6;
    T_NEXT7*  __next_7;
    T_NEXT8*  __next_8;
    T_NEXT9*  __next_9;
    T_NEXT0*  Next0() { return __next_0; }
    T_NEXT1*  Next1() { return __next_1; }
    T_NEXT2*  Next2() { return __next_2; }
    T_NEXT3*  Next3() { return __next_3; }
    T_NEXT4*  Next4() { return __next_4; }
    T_NEXT5*  Next5() { return __next_5; }
    T_NEXT6*  Next6() { return __next_6; }
    T_NEXT7*  Next7() { return __next_7; }
    T_NEXT8*  Next8() { return __next_8; }
    T_NEXT9*  Next9() { return __next_9; }

public:
    TDemux(T_CTX& ctx
        , T_NEXT0 * n0 = NULL 
        , T_NEXT1 * n1 = NULL 
        , T_NEXT2 * n2 = &DummyBrickInstance 
        , T_NEXT3 * n3 = &DummyBrickInstance 
        , T_NEXT4 * n4 = &DummyBrickInstance 
        , T_NEXT5 * n5 = &DummyBrickInstance 
        , T_NEXT6 * n6 = &DummyBrickInstance 
        , T_NEXT7 * n7 = &DummyBrickInstance 
        , T_NEXT8 * n8 = &DummyBrickInstance 
        , T_NEXT9 * n9 = &DummyBrickInstance
        ) : TBrick(ctx)
        , __next_0 (n0)
        , __next_1 (n1)
        , __next_2 (n2)
        , __next_3 (n3)
        , __next_4 (n4)
        , __next_5 (n5)
        , __next_6 (n6)
        , __next_7 (n7)
        , __next_8 (n8)
        , __next_9 (n9)
    {
        if (NON_DUMMYBRICK(0)) AddRef(Next0());
        if (NON_DUMMYBRICK(1)) AddRef(Next1());
        if (NON_DUMMYBRICK(2)) AddRef(Next2());
        if (NON_DUMMYBRICK(3)) AddRef(Next3());
        if (NON_DUMMYBRICK(4)) AddRef(Next4());
        if (NON_DUMMYBRICK(5)) AddRef(Next5());
        if (NON_DUMMYBRICK(6)) AddRef(Next6());
        if (NON_DUMMYBRICK(7)) AddRef(Next7());
        if (NON_DUMMYBRICK(8)) AddRef(Next8());
        if (NON_DUMMYBRICK(9)) AddRef(Next9());
    }

    ~TDemux()
    {
        if (NON_DUMMYBRICK(0)) IReferenceCounting::Release(Next0());
        if (NON_DUMMYBRICK(1)) IReferenceCounting::Release(Next1());
        if (NON_DUMMYBRICK(2)) IReferenceCounting::Release(Next2());
        if (NON_DUMMYBRICK(3)) IReferenceCounting::Release(Next3());
        if (NON_DUMMYBRICK(4)) IReferenceCounting::Release(Next4());
        if (NON_DUMMYBRICK(5)) IReferenceCounting::Release(Next5());
        if (NON_DUMMYBRICK(6)) IReferenceCounting::Release(Next6());
        if (NON_DUMMYBRICK(7)) IReferenceCounting::Release(Next7());
        if (NON_DUMMYBRICK(8)) IReferenceCounting::Release(Next8());
        if (NON_DUMMYBRICK(9)) IReferenceCounting::Release(Next9());
    }

    virtual size_t _TraverseGraph(OUT IQuery *bricks[], const char *typeinfo, size_t count, const IQuery *const *begin)
    {
        if (count <= 0) return 0;
        size_t found = 0;
        if (MatchQuery(typeinfo))
        {
            if (Within(begin, bricks)) return 0;
            if (begin == bricks) begin++;
            *bricks = this;
            found++;
        }
        if (NON_DUMMYBRICK(0)) { found += Next0()->_TraverseGraph(bricks + found, typeinfo, count - found, begin); }
        if (NON_DUMMYBRICK(1)) { found += Next1()->_TraverseGraph(bricks + found, typeinfo, count - found, begin); }
        if (NON_DUMMYBRICK(2)) { found += Next2()->_TraverseGraph(bricks + found, typeinfo, count - found, begin); }
        if (NON_DUMMYBRICK(3)) { found += Next3()->_TraverseGraph(bricks + found, typeinfo, count - found, begin); }
        if (NON_DUMMYBRICK(4)) { found += Next4()->_TraverseGraph(bricks + found, typeinfo, count - found, begin); }
        if (NON_DUMMYBRICK(5)) { found += Next5()->_TraverseGraph(bricks + found, typeinfo, count - found, begin); }
        if (NON_DUMMYBRICK(6)) { found += Next6()->_TraverseGraph(bricks + found, typeinfo, count - found, begin); }
        if (NON_DUMMYBRICK(7)) { found += Next7()->_TraverseGraph(bricks + found, typeinfo, count - found, begin); }
        if (NON_DUMMYBRICK(8)) { found += Next8()->_TraverseGraph(bricks + found, typeinfo, count - found, begin); }
        if (NON_DUMMYBRICK(9)) { found += Next9()->_TraverseGraph(bricks + found, typeinfo, count - found, begin); }
        return found;
    }

    FINL void Reset ()
    {
        if (NON_DUMMYBRICK(0)) Next0()->Reset();
        if (NON_DUMMYBRICK(1)) Next1()->Reset();
        if (NON_DUMMYBRICK(2)) Next2()->Reset();
        if (NON_DUMMYBRICK(3)) Next3()->Reset();
        if (NON_DUMMYBRICK(4)) Next4()->Reset();
        if (NON_DUMMYBRICK(5)) Next5()->Reset();
        if (NON_DUMMYBRICK(6)) Next6()->Reset();
        if (NON_DUMMYBRICK(7)) Next7()->Reset();
        if (NON_DUMMYBRICK(8)) Next8()->Reset();
        if (NON_DUMMYBRICK(9)) Next9()->Reset();
    }

    FINL void Flush()
    {
        if (NON_DUMMYBRICK(0)) Next0()->Flush();
        if (NON_DUMMYBRICK(1)) Next1()->Flush();
        if (NON_DUMMYBRICK(2)) Next2()->Flush();
        if (NON_DUMMYBRICK(3)) Next3()->Flush();
        if (NON_DUMMYBRICK(4)) Next4()->Flush();
        if (NON_DUMMYBRICK(5)) Next5()->Flush();
        if (NON_DUMMYBRICK(6)) Next6()->Flush();
        if (NON_DUMMYBRICK(7)) Next7()->Flush();
        if (NON_DUMMYBRICK(8)) Next8()->Flush();
        if (NON_DUMMYBRICK(9)) Next9()->Flush();
    }

    BOOL_FUNC_PROCESS(ipin) { return true;}; 
}; 

#define TDEMUX2_ARGS class T_CTX, class T_NEXT0, class T_NEXT1

#define TDEMUX2_PARAMS \
    T_CTX, T_NEXT0, T_NEXT1

#define STD_DEMUX2_CONSTRUCTOR(name) name ( T_CTX &ctx, T_NEXT0 * n0, T_NEXT1 * n1 )    \
    : TDemux ( ctx, n0, n1 ) 

#define STD_TDEMUX2_RESET() FINL void Reset() { TDemux::Reset();                        \
        opin0().clear();                                                                \
        opin1().clear();                                                                \
        __ResetSelf(); } FINL void __ResetSelf()

#define CREATE_BRICK_DEMUX2(NAME, TPL_DEMUX, CONTEXT, NEXT_FILTER0, NEXT_FILTER1) \
typedef TPL_DEMUX< TYPEOF(CONTEXT) \
                , TYPEOF(*NEXT_FILTER0) \
                , TYPEOF(*NEXT_FILTER1) \
         > __C_##NAME; \
TYPEDEFINE(__C_##NAME, __D_##NAME);                     \
__D_##NAME *NAME = (__D_##NAME *) new (aligned_malloc<__D_##NAME>()) __C_##NAME(CONTEXT \
                , NEXT_FILTER0 \
                , NEXT_FILTER1 \
                );

#define TDEMUX3_ARGS class T_CTX, class T_NEXT0, class T_NEXT1, class T_NEXT2

#define TDEMUX3_PARAMS \
    T_CTX, T_NEXT0, T_NEXT1, T_NEXT2

#define STD_DEMUX3_CONSTRUCTOR(name) name ( T_CTX &ctx, T_NEXT0 * n0, T_NEXT1 * n1, T_NEXT2 * n2 ) \
    : TDemux ( ctx, n0, n1, n2 ) 

#define STD_TDEMUX3_RESET() FINL void Reset() { TDemux::Reset();                        \
        opin0().clear();                                                                \
        opin1().clear();                                                                \
        opin2().clear();                                                                \
        __ResetSelf(); } FINL void __ResetSelf()

#define CREATE_BRICK_DEMUX3(NAME, TPL_DEMUX, CONTEXT, NEXT_FILTER0, NEXT_FILTER1, NEXT_FILTER2) \
typedef TPL_DEMUX< TYPEOF(CONTEXT) \
                , TYPEOF(*NEXT_FILTER0) \
                , TYPEOF(*NEXT_FILTER1) \
                , TYPEOF(*NEXT_FILTER2) \
         > __C_##NAME; \
TYPEDEFINE(__C_##NAME, __D_##NAME);                     \
__D_##NAME *NAME = (__D_##NAME *) new (aligned_malloc<__D_##NAME>()) __C_##NAME(CONTEXT \
                , NEXT_FILTER0 \
                , NEXT_FILTER1 \
                , NEXT_FILTER2 \
                );

#define TDEMUX4_ARGS class T_CTX, class T_NEXT0, class T_NEXT1, class T_NEXT2, class T_NEXT3

#define TDEMUX4_PARAMS \
    T_CTX, T_NEXT0, T_NEXT1, T_NEXT2, T_NEXT3

#define STD_DEMUX4_CONSTRUCTOR(name) name ( T_CTX &ctx, T_NEXT0 * n0, T_NEXT1 * n1, T_NEXT2 * n2, T_NEXT3 * n3 ) \
    : TDemux ( ctx, n0, n1, n2, n3 ) 

#define STD_TDEMUX4_RESET() FINL void Reset() { TDemux::Reset();                        \
        opin0().clear();                                                                \
        opin1().clear();                                                                \
        opin2().clear();                                                                \
        opin3().clear();                                                                \
        __ResetSelf(); } FINL void __ResetSelf()

#define CREATE_BRICK_DEMUX4(NAME, TPL_DEMUX, CONTEXT, NEXT_FILTER0, NEXT_FILTER1, NEXT_FILTER2, NEXT_FILTER3) \
typedef TPL_DEMUX< TYPEOF(CONTEXT) \
                , TYPEOF(*NEXT_FILTER0) \
                , TYPEOF(*NEXT_FILTER1) \
                , TYPEOF(*NEXT_FILTER2) \
                , TYPEOF(*NEXT_FILTER3) \
         > __C_##NAME; \
TYPEDEFINE(__C_##NAME, __D_##NAME);                     \
__D_##NAME *NAME = (__D_##NAME *) new (aligned_malloc<__D_##NAME>()) __C_##NAME(CONTEXT \
                , NEXT_FILTER0 \
                , NEXT_FILTER1 \
                , NEXT_FILTER2 \
                , NEXT_FILTER3 \
                );

#define TDEMUX5_ARGS class T_CTX, class T_NEXT0, class T_NEXT1, class T_NEXT2, class T_NEXT3, class T_NEXT4

#define TDEMUX5_PARAMS \
    T_CTX, T_NEXT0, T_NEXT1, T_NEXT2, T_NEXT3, T_NEXT4

#define STD_DEMUX5_CONSTRUCTOR(name) name ( T_CTX &ctx, T_NEXT0 * n0, T_NEXT1 * n1, T_NEXT2 * n2, T_NEXT3 * n3, T_NEXT4 * n4 ) \
    : TDemux ( ctx, n0, n1, n2, n3, n4 ) 

#define STD_TDEMUX5_RESET() FINL void Reset() { TDemux::Reset();                       \
        opin0().clear();                                                                \
        opin1().clear();                                                                \
        opin2().clear();                                                                \
        opin3().clear();                                                                \
        opin4().clear();                                                                \
        __ResetSelf(); } FINL void __ResetSelf()

#define CREATE_BRICK_DEMUX5(NAME, TPL_DEMUX, CONTEXT, NEXT_FILTER0, NEXT_FILTER1, NEXT_FILTER2, NEXT_FILTER3, NEXT_FILTER4) \
typedef TPL_DEMUX< TYPEOF(CONTEXT) \
                , TYPEOF(*NEXT_FILTER0) \
                , TYPEOF(*NEXT_FILTER1) \
                , TYPEOF(*NEXT_FILTER2) \
                , TYPEOF(*NEXT_FILTER3) \
                , TYPEOF(*NEXT_FILTER4) \
         > __C_##NAME; \
TYPEDEFINE(__C_##NAME, __D_##NAME);                     \
__D_##NAME *NAME = (__D_##NAME *) new (aligned_malloc<__D_##NAME>()) __C_##NAME(CONTEXT \
                , NEXT_FILTER0 \
                , NEXT_FILTER1 \
                , NEXT_FILTER2 \
                , NEXT_FILTER3 \
                , NEXT_FILTER4 \
                );

#define TDEMUX6_ARGS class T_CTX, class T_NEXT0, class T_NEXT1, class T_NEXT2, class T_NEXT3, class T_NEXT4, class T_NEXT5

#define TDEMUX6_PARAMS \
    T_CTX, T_NEXT0, T_NEXT1, T_NEXT2, T_NEXT3, T_NEXT4, T_NEXT5

#define STD_DEMUX6_CONSTRUCTOR(name) name ( T_CTX &ctx, T_NEXT0 * n0, T_NEXT1 * n1, T_NEXT2 * n2, T_NEXT3 * n3, T_NEXT4 * n4, T_NEXT5 * n5 ) \
    : TDemux ( ctx, n0, n1, n2, n3, n4, n5 ) 

#define STD_TDEMUX6_RESET() FINL void Reset() { TDemux::Reset();                       \
        opin0().clear();                                                                \
        opin1().clear();                                                                \
        opin2().clear();                                                                \
        opin3().clear();                                                                \
        opin4().clear();                                                                \
        opin5().clear();                                                                \
        __ResetSelf(); } FINL void __ResetSelf()

#define CREATE_BRICK_DEMUX6(NAME, TPL_DEMUX, CONTEXT, NEXT_FILTER0, NEXT_FILTER1, NEXT_FILTER2, NEXT_FILTER3, NEXT_FILTER4, NEXT_FILTER5) \
typedef TPL_DEMUX< TYPEOF(CONTEXT) \
                , TYPEOF(*NEXT_FILTER0) \
                , TYPEOF(*NEXT_FILTER1) \
                , TYPEOF(*NEXT_FILTER2) \
                , TYPEOF(*NEXT_FILTER3) \
                , TYPEOF(*NEXT_FILTER4) \
                , TYPEOF(*NEXT_FILTER5) \
         > __C_##NAME; \
TYPEDEFINE(__C_##NAME, __D_##NAME);                     \
__D_##NAME *NAME = (__D_##NAME *) new (aligned_malloc<__D_##NAME>()) __C_##NAME(CONTEXT \
                , NEXT_FILTER0 \
                , NEXT_FILTER1 \
                , NEXT_FILTER2 \
                , NEXT_FILTER3 \
                , NEXT_FILTER4 \
                , NEXT_FILTER5 \
                );

#define TDEMUX7_ARGS class T_CTX, class T_NEXT0, class T_NEXT1, class T_NEXT2, class T_NEXT3, class T_NEXT4, class T_NEXT5, class T_NEXT6

#define TDEMUX7_PARAMS \
    T_CTX, T_NEXT0, T_NEXT1, T_NEXT2, T_NEXT3, T_NEXT4, T_NEXT5, T_NEXT6

#define STD_DEMUX7_CONSTRUCTOR(name) name ( T_CTX &ctx, T_NEXT0 * n0, T_NEXT1 * n1, T_NEXT2 * n2, T_NEXT3 * n3, T_NEXT4 * n4, T_NEXT5 * n5, T_NEXT6 * n6 ) \
    : TDemux ( ctx, n0, n1, n2, n3, n4, n5, n6 ) 

#define STD_TDEMUX7_RESET() FINL void Reset() { TDemux::Reset();                       \
        opin0().clear();                                                                \
        opin1().clear();                                                                \
        opin2().clear();                                                                \
        opin3().clear();                                                                \
        opin4().clear();                                                                \
        opin5().clear();                                                                \
        opin6().clear();                                                                \
        __ResetSelf(); } FINL void __ResetSelf()

#define CREATE_BRICK_DEMUX7(NAME, TPL_DEMUX, CONTEXT, NEXT_FILTER0, NEXT_FILTER1, NEXT_FILTER2, NEXT_FILTER3, NEXT_FILTER4, NEXT_FILTER5, NEXT_FILTER6) \
typedef TPL_DEMUX< TYPEOF(CONTEXT) \
                , TYPEOF(*NEXT_FILTER0) \
                , TYPEOF(*NEXT_FILTER1) \
                , TYPEOF(*NEXT_FILTER2) \
                , TYPEOF(*NEXT_FILTER3) \
                , TYPEOF(*NEXT_FILTER4) \
                , TYPEOF(*NEXT_FILTER5) \
                , TYPEOF(*NEXT_FILTER6) \
         > __C_##NAME; \
TYPEDEFINE(__C_##NAME, __D_##NAME);                     \
__D_##NAME *NAME = (__D_##NAME *) new (aligned_malloc<__D_##NAME>()) __C_##NAME(CONTEXT \
                , NEXT_FILTER0 \
                , NEXT_FILTER1 \
                , NEXT_FILTER2 \
                , NEXT_FILTER3 \
                , NEXT_FILTER4 \
                , NEXT_FILTER5 \
                , NEXT_FILTER6 \
                );

#define TDEMUX8_ARGS class T_CTX, class T_NEXT0, class T_NEXT1, class T_NEXT2, class T_NEXT3, class T_NEXT4, class T_NEXT5, class T_NEXT6, class T_NEXT7

#define TDEMUX8_PARAMS \
    T_CTX, T_NEXT0, T_NEXT1, T_NEXT2, T_NEXT3, T_NEXT4, T_NEXT5, T_NEXT6, T_NEXT7

#define STD_DEMUX8_CONSTRUCTOR(name) name ( T_CTX &ctx, T_NEXT0 * n0, T_NEXT1 * n1, T_NEXT2 * n2, T_NEXT3 * n3, T_NEXT4 * n4, T_NEXT5 * n5, T_NEXT6 * n6, T_NEXT7 * n7 ) \
    : TDemux ( ctx, n0, n1, n2, n3, n4, n5, n6, n7 ) 

#define STD_TDEMUX8_RESET() FINL void Reset() { TDemux::Reset();                       \
        opin0().clear();                                                                \
        opin1().clear();                                                                \
        opin2().clear();                                                                \
        opin3().clear();                                                                \
        opin4().clear();                                                                \
        opin5().clear();                                                                \
        opin6().clear();                                                                \
        opin7().clear();                                                                \
        __ResetSelf(); } FINL void __ResetSelf()

#define CREATE_BRICK_DEMUX8(NAME, TPL_DEMUX, CONTEXT, NEXT_FILTER0, NEXT_FILTER1, NEXT_FILTER2, NEXT_FILTER3, NEXT_FILTER4, NEXT_FILTER5, NEXT_FILTER6, NEXT_FILTER7) \
typedef TPL_DEMUX< TYPEOF(CONTEXT) \
                , TYPEOF(*NEXT_FILTER0) \
                , TYPEOF(*NEXT_FILTER1) \
                , TYPEOF(*NEXT_FILTER2) \
                , TYPEOF(*NEXT_FILTER3) \
                , TYPEOF(*NEXT_FILTER4) \
                , TYPEOF(*NEXT_FILTER5) \
                , TYPEOF(*NEXT_FILTER6) \
                , TYPEOF(*NEXT_FILTER7) \
         > __C_##NAME; \
TYPEDEFINE(__C_##NAME, __D_##NAME);                     \
__D_##NAME *NAME = (__D_##NAME *) new (aligned_malloc<__D_##NAME>()) __C_##NAME(CONTEXT \
                , NEXT_FILTER0 \
                , NEXT_FILTER1 \
                , NEXT_FILTER2 \
                , NEXT_FILTER3 \
                , NEXT_FILTER4 \
                , NEXT_FILTER5 \
                , NEXT_FILTER6 \
                , NEXT_FILTER7 \
                );

#define TDEMUX9_ARGS class T_CTX, class T_NEXT0, class T_NEXT1, class T_NEXT2, class T_NEXT3, class T_NEXT4, class T_NEXT5, class T_NEXT6, class T_NEXT7, class T_NEXT8

#define TDEMUX9_PARAMS \
    T_CTX, T_NEXT0, T_NEXT1, T_NEXT2, T_NEXT3, T_NEXT4, T_NEXT5, T_NEXT6, T_NEXT7, T_NEXT8

#define STD_DEMUX9_CONSTRUCTOR(name) name ( T_CTX &ctx, T_NEXT0 * n0, T_NEXT1 * n1, T_NEXT2 * n2, T_NEXT3 * n3, T_NEXT4 * n4, T_NEXT5 * n5, T_NEXT6 * n6, T_NEXT7 * n7, T_NEXT8 * n8 ) \
    : TDemux ( ctx, n0, n1, n2, n3, n4, n5, n6, n7, n8 ) 

#define STD_TDEMUX9_RESET() FINL void Reset() { TDemux::Reset();                       \
        opin0().clear();                                                                \
        opin1().clear();                                                                \
        opin2().clear();                                                                \
        opin3().clear();                                                                \
        opin4().clear();                                                                \
        opin5().clear();                                                                \
        opin6().clear();                                                                \
        opin7().clear();                                                                \
        opin8().clear();                                                                \
        __ResetSelf(); } FINL void __ResetSelf()

#define CREATE_BRICK_DEMUX9(NAME, TPL_DEMUX, CONTEXT, NEXT_FILTER0, NEXT_FILTER1, NEXT_FILTER2, NEXT_FILTER3, NEXT_FILTER4, NEXT_FILTER5, NEXT_FILTER6, NEXT_FILTER7, NEXT_FILTER8) \
typedef TPL_DEMUX< TYPEOF(CONTEXT) \
                , TYPEOF(*NEXT_FILTER0) \
                , TYPEOF(*NEXT_FILTER1) \
                , TYPEOF(*NEXT_FILTER2) \
                , TYPEOF(*NEXT_FILTER3) \
                , TYPEOF(*NEXT_FILTER4) \
                , TYPEOF(*NEXT_FILTER5) \
                , TYPEOF(*NEXT_FILTER6) \
                , TYPEOF(*NEXT_FILTER7) \
                , TYPEOF(*NEXT_FILTER8) \
         > __C_##NAME; \
TYPEDEFINE(__C_##NAME, __D_##NAME);                     \
__D_##NAME *NAME = (__D_##NAME *) new (aligned_malloc<__D_##NAME>()) __C_##NAME(CONTEXT \
                , NEXT_FILTER0 \
                , NEXT_FILTER1 \
                , NEXT_FILTER2 \
                , NEXT_FILTER3 \
                , NEXT_FILTER4 \
                , NEXT_FILTER5 \
                , NEXT_FILTER6 \
                , NEXT_FILTER7 \
                , NEXT_FILTER8 \
                );

#define TDEMUX10_ARGS class T_CTX, class T_NEXT0, class T_NEXT1, class T_NEXT2 = DummyBrick, class T_NEXT3 = DummyBrick, class T_NEXT4 = DummyBrick, class T_NEXT5 = DummyBrick, class T_NEXT6 = DummyBrick, class T_NEXT7 = DummyBrick, class T_NEXT8 = DummyBrick, class T_NEXT9 = DummyBrick

#define TDEMUX10_PARAMS \
    T_CTX, T_NEXT0, T_NEXT1, T_NEXT2, T_NEXT3, T_NEXT4, T_NEXT5, T_NEXT6, T_NEXT7, T_NEXT8, T_NEXT9

#define STD_DEMUX10_CONSTRUCTOR(name) name ( T_CTX &ctx, T_NEXT0 * n0, T_NEXT1 * n1, T_NEXT2 * n2 = &DummyBrickInstance, T_NEXT3 * n3 = &DummyBrickInstance, T_NEXT4 * n4 = &DummyBrickInstance, T_NEXT5 * n5 = &DummyBrickInstance, T_NEXT6 * n6 = &DummyBrickInstance, T_NEXT7 * n7 = &DummyBrickInstance, T_NEXT8 * n8 = &DummyBrickInstance, T_NEXT9 * n9 = &DummyBrickInstance ) \
    : TDemux ( ctx, n0, n1, n2, n3, n4, n5, n6, n7, n8, n9 ) 

#define STD_TDEMUX10_RESET() FINL void Reset() { TDemux::Reset();                       \
        opin0().clear();                                                                \
        opin1().clear();                                                                \
        opin2().clear();                                                                \
        opin3().clear();                                                                \
        opin4().clear();                                                                \
        opin5().clear();                                                                \
        opin6().clear();                                                                \
        opin7().clear();                                                                \
        opin8().clear();                                                                \
        opin9().clear();                                                                \
        __ResetSelf(); } FINL void __ResetSelf()

#define CREATE_BRICK_DEMUX10(NAME, TPL_DEMUX, CONTEXT, NEXT_FILTER0, NEXT_FILTER1, NEXT_FILTER2, NEXT_FILTER3, NEXT_FILTER4, NEXT_FILTER5, NEXT_FILTER6, NEXT_FILTER7, NEXT_FILTER8, NEXT_FILTER9) \
typedef TPL_DEMUX< TYPEOF(CONTEXT) \
                , TYPEOF(*NEXT_FILTER0) \
                , TYPEOF(*NEXT_FILTER1) \
                , TYPEOF(*NEXT_FILTER2) \
                , TYPEOF(*NEXT_FILTER3) \
                , TYPEOF(*NEXT_FILTER4) \
                , TYPEOF(*NEXT_FILTER5) \
                , TYPEOF(*NEXT_FILTER6) \
                , TYPEOF(*NEXT_FILTER7) \
                , TYPEOF(*NEXT_FILTER8) \
                , TYPEOF(*NEXT_FILTER9) \
         > __C_##NAME; \
TYPEDEFINE(__C_##NAME, __D_##NAME);                     \
__D_##NAME *NAME = (__D_##NAME *) new (aligned_malloc<__D_##NAME>()) __C_##NAME(CONTEXT \
                , NEXT_FILTER0 \
                , NEXT_FILTER1 \
                , NEXT_FILTER2 \
                , NEXT_FILTER3 \
                , NEXT_FILTER4 \
                , NEXT_FILTER5 \
                , NEXT_FILTER6 \
                , NEXT_FILTER7 \
                , NEXT_FILTER8 \
                , NEXT_FILTER9 \
                );
