#pragma once

#if _MSC_VER < 1600
// Note: the implementation of TYPEOF(x) macro is dependent to MSVC compiler,
// and should be replaced by decltype/auto if WDK compiler support C++0x syntax.
namespace typeof { namespace details {

template<int id, class T = void> class id2type;
template<int id> class id2type<id, void>
{
    template<class T> struct __inner;
public:
    typedef __inner<void> inner;
};
template<int id, class T> class id2type : id2type<id>
{
    template<> struct __inner<void> { typedef T type; };
};

template<int id> struct id_cnt { const static int value = id; };

template<class type, int t = 1> struct __type2id
{
    __if_exists(id_cnt<t>) { const static int value = __type2id<type, t + 1>::value; }
    __if_not_exists(id_cnt<t>) { const static int value = t; }
};

template<class type> struct type2id
    : public id_cnt<__type2id<type>::value>
    , id2type<__type2id<type>::value, type>
{ };

template<class T> char (&__specsize(T& t)) [type2id<T>::value];
template<class T> char (&__specsize(const T& t)) [type2id<T>::value];

} }

#define TYPEOF(x) typeof::details::id2type<sizeof(typeof::details::__specsize(x))>::inner::type
#else
#define TYPEOF(x) remove_reference<decltype(x)>::type
#endif
