// Header file for brick frame work

#pragma once

#include <stdarg.h>
#include <typeinfo.h>
#include <new.h>
#include "const.h"
#include "alinew.h"
#include "tpltrick.h"
#include "typeof.h"
#include "variadic_macro.h"
#include "pinqueue.h"

// void base class
class CF_VOID { };
SELECTANY CF_VOID VoidInstance;

// Definition of Bricks
// Interface
//	 	Reset : reset all states
//      Flush : flush all pending data
//		Process: operating on the input pin
//

// return a unique id for a brick instance
// Note: DONNOT use static inline function, which makes the function-scope static variable
//      have multiple instances in multiple compilation units (.cpp/.c)
FINL int GenerateBrickID ()
{
    static volatile int id_cnt = 0;
    return id_cnt++;
}

struct ISource;
class IReferenceCounting
{
private:
    mutable unsigned int cnt;

protected:
    IReferenceCounting() : cnt(0) { }
    IReferenceCounting(const IReferenceCounting&) : cnt(0) { }
    IReferenceCounting& operator=(const IReferenceCounting&) { return *this; }

public:
    // Note: must use virtual, because Release() will delete derived class
    virtual ~IReferenceCounting() { }

    // Note: T should be of type IReferenceCounting or inherited from IReferenceCounting
    // we use template to handle privated inheritance from IReferenceCounting
    template<class T>
    static void AddRef(T *p)
    {
        assert(p);
        ++ ((const IReferenceCounting*)p)->cnt;
    }

    // Note: this class overload delete operator and virtual destructor at the same time,
    // so when delete called on the IReferenceCounting pointer, it will call destructor of
    // derived class, and free memory of derived class. The memory is expected allocated by
    // aligned_malloc function.
    void operator delete(void *p)
    {
        assert(p);
        aligned_free(p);
    }

    // Overload to make compiler happy
    void operator delete(void *p, void *place)
    {
		UNREFERENCED_PARAMETER(p);
		UNREFERENCED_PARAMETER(place);
    }

    static void Release(const IReferenceCounting * const &self)
    {
        assert(self);
        if (-- self->cnt == 0)
        {
            delete(self);
        }
    }

    static void Release(ISource * &self)
    {
        Release((const ISource * &)self);
    }

    static void Release(const ISource * &self)
    {
        if (self == NULL) return;
        Release((const IReferenceCounting * const &)self);
        self = NULL;
    }
};

class IQuery
{
    virtual void Dummy() { } // Necessary for RTTI
public:
    template<class T_INTERFACE>
    T_INTERFACE *QueryInterface()
    {
        return dynamic_cast<T_INTERFACE *>(this);
    }
};

class IQueryable : public IQuery
{
protected:
    bool MatchQuery(const char *name)
    {
        static const char s1[] = "class ", s2[] = "struct ";

        const char *self = typeid(*this).name();
        const size_t x1 = sizeof(s1) - 1
            , x2 = sizeof(s2) - 1
            , xn = strlen(name);

        return (strncmp(self, s1, x1) == 0 && strncmp(self + x1, name, xn) == 0 && (self[x1 + xn] == '<' || self[x1 + xn] == ':'))
            || (strncmp(self, s2, x2) == 0 && strncmp(self + x2, name, xn) == 0 && (self[x2 + xn] == '<' || self[x2 + xn] == ':'));
    }

    bool Within(const IQuery *const *begin, const IQuery *const *end)
    {
        assert(begin <= end);
        const IQuery *const *p;
        for (p = begin; p < end; p++)
        {
            if (*p == this) return true;
    }
        return false;
    }

    // Find brick objects in a brick graph by brick's template name
    // Parameters:
    //  bricks   - (output) found brick objects
    //  typeinfo - brick's template name
    //  count    - the maximal number to find
    //  begin    - define the begin of the range [begin, bricks) for duplication checking
    // Returnes:
    //  number of actual found brick objects
    virtual size_t _TraverseGraph(OUT IQuery *bricks[], const char *typeinfo, size_t count, const IQuery *const *begin) = 0;
public:
    // Find brick objects in a brick graph by brick's template name
    // Parameters:
    //  bricks   - (output) found brick objects
    //  typeinfo - brick's template name
    //  count    - the maximal number to find
    // Returnes:
    //  number of actual found brick objects
    size_t TraverseGraph(OUT IQuery *bricks[], const char *typeinfo, size_t count)
    {
        if (!bricks) return 0;
        return _TraverseGraph(bricks, typeinfo, count, bricks);
    }
};

template<class T_CTX>
class TBrick
{
    // Make compiler happy, calling forbidden
    TBrick& operator=(const TBrick&);
protected:
    int     __id_;
    T_CTX & __ctx_;

	int __brk_marker[4];

public:
    TBrick (T_CTX & ctx ) : __ctx_(ctx), __id_(GenerateBrickID()) { };
    FINL int bid () { return __id_; }
    T_CTX & ctx() { return __ctx_; }
};

//
// The main processing function of a brick
//
// Note: if compilation error C2118 happens at next assertion (CCASSERT), possible reason:
// the related brick class and its upstream brick class has incompatible type
// in iport and oport definitions.
#define BOOL_FUNC_PROCESS(ipin)                                                                                         \
    template<class T_IPIN> FINL bool Process (T_IPIN & ipin) {                                                          \
		CCASSERT(type_convertible<T_IPIN::DataType MACRO_COMMA iport_traits::type>::value                               \
            && T_IPIN::nstream == iport_traits::nstream                                                                 \
            || same_type<iport_traits::type MACRO_COMMA void>::value);                                                  \
		return __ProcessSelf (ipin); }                                                                                  \
	template<class T_IPIN> FINL bool __ProcessSelf  (T_IPIN & ipin) 

// Marco: DEFINE_IPORT(TYPE, BURST, NSTREAM)
//   are used to declare the inport type of a brick
// Parameter:
//   TYPE       - the data type of brick input or output
//   BURST      - the number of data items inputed or outputed at one time
//   NSTREAM    - OPTIONAL, the number of data streams of input or output
//
#define DEFINE_IPORT(...) __DEFINE_IPORT_D1((__VA_NARGS__(__VA_ARGS__), (__VA_ARGS__)))
#define __DEFINE_IPORT_D1(...) __DEFINE_IPORT_D2 __VA_ARGS__
#define __DEFINE_IPORT_D2(N, ...) CONCATENATE(__DEFINE_IPORT_NARGS, N) __VA_ARGS__

// Marco: DEFINE_OPORT(TYPE, BURST, NSTREAM)
//   are used to declare the outport type of a brick with single outport
// Parameter:
//   TYPE       - the data type of brick input or output
//   BURST      - the number of data items inputed or outputed at one time
//   NSTREAM    - OPTIONAL, the number of data streams of input or output
//
#define DEFINE_OPORT(...) __DEFINE_OPORT_D1((__VA_NARGS__(__VA_ARGS__), (__VA_ARGS__)))
#define __DEFINE_OPORT_D1(...) __DEFINE_OPORT_D2 __VA_ARGS__
#define __DEFINE_OPORT_D2(N, ...) CONCATENATE(__DEFINE_OPORT_NARGS, N) __VA_ARGS__

// Marco: DEFINE_OPORTS(INDEX, TYPE, BURST, NSTREAM)
//   are used to declare the outport type of a brick with multiple outports
// Parameter:
//   INDEX      - the index of outport
//   TYPE       - the data type of brick input or output
//   BURST      - the number of data items inputed or outputed at one time
//   NSTREAM    - OPTIONAL, the number of data streams of input or output
//
#define DEFINE_OPORTS(...) __DEFINE_OPORTS_D1((__VA_NARGS__(__VA_ARGS__), (__VA_ARGS__)))
#define __DEFINE_OPORTS_D1(...) __DEFINE_OPORTS_D2 __VA_ARGS__
#define __DEFINE_OPORTS_D2(N, ...) CONCATENATE(__DEFINE_OPORTS_NARGS, N) __VA_ARGS__

#define __DEFINE_IPORT_NARGS2(TYPE, BURST)     typedef iport_traits<TYPE, BURST, 1> iport_traits;
#define __DEFINE_OPORT_NARGS2(TYPE, BURST)                                                                              \
    typedef oport_traits<TYPE, BURST, 1> oport_traits;                                                                  \
    typedef typename DeducedPinQueue<oport_traits, typename T_NEXT::iport_traits>::type opin_type;                      \
    opin_type __opin_;	/* output pin */                                                                                \
    opin_type& opin () { return __opin_; }
#define __DEFINE_OPORTS_NARGS3(INDEX, TYPE, BURST)                                                                      \
    typedef oport_traits<TYPE, BURST, 1> oport_traits##INDEX;                                                           \
    typedef typename DeducedPinQueue<oport_traits##INDEX, typename T_NEXT##INDEX::iport_traits>::type opin_type##INDEX; \
    opin_type##INDEX __opin_##INDEX;	/* output pin */                                                                \
    opin_type##INDEX & opin##INDEX () { return __opin_##INDEX; }

#define __DEFINE_IPORT_NARGS3(TYPE, BURST, NSTREAM)     typedef iport_traits<TYPE, BURST, NSTREAM> iport_traits;
#define __DEFINE_OPORT_NARGS3(TYPE, BURST, NSTREAM)                                                                     \
    typedef oport_traits<TYPE, BURST, NSTREAM> oport_traits;                                                            \
    typedef typename DeducedPinQueue<oport_traits, typename T_NEXT::iport_traits>::type opin_type;                      \
    opin_type __opin_;	/* output pin */                                                                                \
    opin_type& opin () { return __opin_; }
#define __DEFINE_OPORTS_NARGS4(INDEX, TYPE, BURST, NSTREAM)                                                             \
    typedef oport_traits<TYPE, BURST, NSTREAM> oport_traits##INDEX;                                                     \
    typedef typename DeducedPinQueue<oport_traits##INDEX, typename T_NEXT##INDEX::iport_traits>::type opin_type##INDEX; \
    opin_type##INDEX __opin_##INDEX;	/* output pin */                                                                \
    opin_type##INDEX & opin##INDEX () { return __opin_##INDEX; }

// Brick - Sink
#define TSINK_ARGS class T_CTX
#define TSINK_PARAMS T_CTX
#define STD_TSINK_CONSTRUCTOR(name) name (T_CTX& ctx) : TSink(ctx)
#define STD_TSINK_RESET() FINL void Reset()
#define STD_TSINK_FLUSH() FINL void Flush() { \
		__FlushSelf(); } FINL void __FlushSelf()

template<class T_CTX>
class TSink : public TBrick<T_CTX>, public IReferenceCounting, public IControlPoint
{
public:
    TSink (T_CTX& ctx) : TBrick(ctx) { }
    FINL void Reset() { }
    FINL void Flush() { }
    BOOL_FUNC_PROCESS(ipin) { return true; }

    virtual size_t _TraverseGraph(OUT IQuery *bricks[], const char *typeinfo, size_t count, const IQuery *const *begin)
    {
        if (count <= 0) return 0;
        size_t found = 0;
        if (MatchQuery(typeinfo))
    {
            if (Within(begin, bricks)) return 0;
            if (begin == bricks) begin++;
            *bricks++ = this;
            found++;
        }
        return found;
    }
};

// Brick - Filter
#define TFILTER_ARGS class T_CTX, class T_NEXT
#define TFILTER_PARAMS T_CTX, T_NEXT
#define STD_TFILTER_CONSTRUCTOR(name) name ( T_CTX & ctx, T_NEXT* n )               \
    : TFilter (ctx, n)
#define STD_TFILTER_RESET() FINL void Reset() { TFilter::Reset(); opin().clear();   \
	    __ResetSelf(); } FINL void __ResetSelf()
#define STD_TFILTER_FLUSH() FINL void Flush() {                                     \
	__FlushSelf();                                                                  \
    FlushPort();                                                                    \
    } FINL void __FlushSelf()
	

template<class T_CTX, class T_NEXT>
class TFilter : public TBrick<T_CTX>, public IReferenceCounting, public IControlPoint
{
protected:
    T_NEXT* __next_;	// point to next brick
    T_NEXT* Next()    { return __next_; }

public:
    TFilter ( T_CTX & ctx, T_NEXT* n ) : TBrick(ctx), __next_(n)
    {
        AddRef(Next());
    }

    ~TFilter()
    {
        IReferenceCounting::Release(Next());
    }

    FINL void Reset() { Next()->Reset(); }
    FINL void Flush() { Next()->Flush(); }
    BOOL_FUNC_PROCESS(ipin) { return true; }

    virtual size_t _TraverseGraph(OUT IQuery *bricks[], const char *typeinfo, size_t count, const IQuery *const *begin)
    {
        if (count <= 0) return 0;
        size_t found = 0;
        if (MatchQuery(typeinfo))
        {
            if (Within(begin, bricks)) return 0;
            if (begin == bricks) begin++;
            *bricks++ = this;
            found++;
        }
        found += Next()->_TraverseGraph(bricks, typeinfo, count - found, begin);
        return found;
    }
};

// Brick - Source
#define TSOURCE_ARGS class T_CTX, class T_NEXT
#define TSOURCE_PARAMS T_CTX, T_NEXT

#define STD_TSOURCE_CONSTRUCTOR(name) name ( T_CTX & ctx, T_NEXT* n ) : TSource(ctx, n)
#define STD_TSOURCE_RESET() FINL void Reset() { TSource::Reset(); opin().clear();  \
	    __ResetSelf(); } FINL void __ResetSelf()
#define STD_TSOURCE_FLUSH() FINL void Flush() {                                     \
	__FlushSelf();                                                                  \
    FlushPort();                                                                    \
    } FINL void __FlushSelf()

#define SOURCE_CONSTRUCTOR_WITH_TYPE(name, parent) name ( T_CTX & ctx, T_NEXT* n ) : parent(ctx, n)

struct IControlPoint : public IQueryable
{
    virtual void Reset () = 0;
	virtual void Flush () = 0;
};

struct ISource : public IReferenceCounting, public IControlPoint
{
    virtual bool Process () = 0;
    // Seek to an offset based on current processing position in the source stream
    // Parameters:
    //   offset - the seeking offset, START_POS means the beginning, END_POS means the end of the current stream
    // Returns: the offset actual seeked
	virtual int Seek (int offset) = 0;
	const static int START_POS = INT_MIN;
	const static int END_POS   = INT_MAX;
};

template<class T_CTX, class T_NEXT>
class TSource : public TBrick<T_CTX>, public ISource {
protected:	
    T_NEXT* __next_;
    T_NEXT* Next()	  { return __next_; }

public:
    TSource ( T_CTX & ctx, T_NEXT* n ) : TBrick(ctx), __next_(n)
    {
		AddRef(this);
        AddRef(n);
    }

    ~TSource()
    {
        IReferenceCounting::Release(Next());
    }

    FINL void Reset ()
    {
        return Next()->Reset(); 
    }

    FINL void Flush()    { Next()->Flush(); }
    FINL bool Process () { return true; }
    FINL int Seek (int offset) { return 0; }

    virtual size_t _TraverseGraph(OUT IQuery *bricks[], const char *typeinfo, size_t count, const IQuery *const *begin)
    {
        if (count <= 0) return 0;
        size_t found = 0;
        if (MatchQuery(typeinfo))
        {
            if (Within(begin, bricks)) return 0;
            if (begin == bricks) begin++;
            *bricks++ = this;
            found++;
        }
        found += Next()->_TraverseGraph(bricks, typeinfo, count - found, begin);
        return found;
    }
};

#include "demux.h"

#define TYPEDEFINE(TYPE, NAME) struct NAME : public TYPE { NAME(const TYPE& a) : TYPE(a) { } };

// Macros
// Macro to create brick object in function local scope, or file local scope
// Parameters:
//  NAME        - [Output] pointer to created filter object, should be non-conflicted variable name
//  TPL_BRICK   - template name of the brick type
//  T_IN        - type of input TPinQueue
//  CONTEXT     - context variable
//  NEXT_BRICK  - pointer to downstream filter object, should be existing variable name
//  ...         - any other arguments will be passed to brick contructor at the end of contructor argument list
#define CREATE_BRICK_SOURCE(NAME, TPL_BRICK, CONTEXT, NEXT_BRICK)                           \
    typedef TPL_BRICK<TYPEOF(CONTEXT), TYPEOF(*NEXT_BRICK)> __C_##NAME;                     \
    TYPEDEFINE(__C_##NAME, __D_##NAME);                                                     \
    __D_##NAME *NAME = (__D_##NAME *) new (aligned_malloc<__D_##NAME>()) __C_##NAME(CONTEXT, NEXT_BRICK);

#define CREATE_BRICK_FILTER(NAME, TPL_BRICK, CONTEXT, NEXT_BRICK)                           \
    typedef TPL_BRICK<TYPEOF(CONTEXT), TYPEOF(*NEXT_BRICK)> __C_##NAME;                     \
    TYPEDEFINE(__C_##NAME, __D_##NAME);                                                     \
    __D_##NAME *NAME = (__D_##NAME *) new (aligned_malloc<__D_##NAME>()) __C_##NAME(CONTEXT, NEXT_BRICK);

#define CREATE_BRICK_SINK(NAME, TPL_BRICK, CONTEXT)                                         \
    typedef TPL_BRICK<TYPEOF(CONTEXT)> __C_##NAME;                                          \
    TYPEDEFINE(__C_##NAME, __D_##NAME);                                                     \
    __D_##NAME *NAME = (__D_##NAME *) new (aligned_malloc<__D_##NAME>()) __C_##NAME(CONTEXT);

// Local context type is defined by a list of context facade classes
// It is indeed virtual inherited from them
// Note: if the brick component doesn't need any context, please supply CF_VOID as the variadic parameter
#define DEFINE_LOCAL_CONTEXT(TPL_BRICK, ...)                                                \
    struct __declspec(novtable) __LocalContext_##TPL_BRICK : VIRTUAL_SEP(__VA_ARGS__) { };
#define REFERENCE_LOCAL_CONTEXT(TPL_BRICK) typedef struct __LocalContext_##TPL_BRICK __lctx_type;
#define LOCAL_CONTEXT(TPL_BRICK)                                                            \
    __LocalContext_##TPL_BRICK

// Macro to refer member functions in brick's context
// Note: used only in the scope of the brick class
// Note: the useless comma statement is used for type checking purpose only
#define RefCtxFunc(fn)                                                                      \
    (&__lctx_type::fn, __ctx_).fn

#define RaiseEvent(fn)																		\
    (&__lctx_type::fn, __ctx_).fn

// Macro to bind field in local context to class member inside brick class
// in order to optimize access speed, such as prventing access overhead of virtual
// in heritance for context facade and local context
// Note: used only in the initialization list of contructor of birck class
// Note: it including the header comma for better code style
#define BIND_CONTEXT(CONTEXT_FUNC, LOCAL_VARIABLE)                                          \
    , LOCAL_VARIABLE(RefCtxFunc(CONTEXT_FUNC)())

// Disable compilation warning 'not enough actual parameters for macro' CTX_VAR_RW, CTX_VAR_RO, FACADE_FIELD
#pragma warning(disable: 4003)
// Note:
//   DIMENSIONS is optional, used to define array, eg. [2] or [10][101]
#define CTX_VAR_RW(TYPE, NAME, DIMENSIONS) (TYPE) (&NAME) DIMENSIONS;
// Note:
//   DIMENSIONS is optional, used to define array, eg. [2] or [10][101]
#define CTX_VAR_RO(TYPE, NAME, DIMENSIONS) (TYPE) const (&NAME) DIMENSIONS;

#define FlushPort(N) do { if (opin##N().pad()) Next##N()->Process(opin##N()); Next##N()->Flush(); } while (false)

// Note:
//   DIMENSIONS is optional, used to define array, eg. [2] or [10][101]
#define FACADE_FIELD(TYPE, NAME, DIMENSIONS)                                                \
    private: TYPE __##NAME##__ DIMENSIONS;                                                  \
    public:  TYPE (&NAME()) DIMENSIONS { return __##NAME##__; }

class DummyBrick : public TSink<CF_VOID>
{
public:
    DEFINE_IPORT(void, 1);
    DummyBrick() : TSink(VoidInstance) { }
    FINL void Reset() { }
    FINL void Flush() { }

    BOOL_FUNC_PROCESS(pin) {
        pin.clear ();
        return true;
    }
};
SELECTANY DummyBrick DummyBrickInstance;
