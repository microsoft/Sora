#pragma once
#include "const.h"

template<class T, size_t N>
size_t find(const T(&arr)[N], const T& value)
{
    size_t i;
    for (i = 0; i < N; i++)
    {
        if (arr[i] == value) return i;
    }
    // This is an wierd value for size_t, only for error return value
    return -1;
}


// template numerics
// Greatest Common Division
template<int u, int v> struct gcd        { enum { value = gcd<v, u % v>::value }; };
template<int u>        struct gcd <u, 0> { enum { value = u }; };
template<int u, int v> struct lcm        { enum { value = u * v / gcd<u,v>::value }; };

template<int x, unsigned int p> struct intpow      { enum { value = x * intpow<x,p-1>::value }; };
template<int x>                 struct intpow<x,0> { enum { value = 1 }; };

template<class T> struct remove_pointer { };
template<class T> struct remove_pointer<T*> { typedef T type; };
template<class T> struct remove_pointer<T * const> { typedef T type; };

// ceil of x/y
template<int x, int y> struct ceiling_div {	enum { value = (x + y -1) / y};};


template<class T> struct remove_reference { typedef T type; };
template<class T> struct remove_reference<T&> { typedef T type; };

// Template to test one type can be converted to another type
template<class FROM, class TO>
class type_convertible
{
    typedef char char1;
    typedef char (&char2)[2];

    static FROM& create();
    static char1 fit(const TO&);
    template<class T> static char2 fit(const T&);
public:
    const static bool value = sizeof(fit(create())) == sizeof(char1);
};
// No type cannot be converted to void type
template<class FROM>
class type_convertible<FROM, void>
{
public:
    const static bool value = false;
};
// void type cannot be converted to other type
template<class TO>
class type_convertible<void, TO>
{
public:
    const static bool value = false;
};

template<class T1, class T2>
struct same_type
{
    const static bool value = type_convertible<T1, T2>::value && type_convertible<T2, T1>::value;
};
// void type == void type
template<>
struct same_type<void, void>
{
    const static bool value = true;
};
// Types except void cannot be same to void type
template<class T1>
struct same_type<T1, void>
{
    const static bool value = false;
};
template<class T2>
struct same_type<void, T2>
{
    const static bool value = false;
};

#define STATIC_ASSERT_TYPE(T1,T2) CCASSERT((type_convertible<T1, T2>::value));

// Used in SFINAE technique: if all the types in template arguments exist, this type will be instanticated
template <class type1, class type2, class type3 = void> struct constraint_types
{
    typedef type1 type1;
    typedef type2 type2;
    typedef type3 type3;
};

struct DummyInitializer
{
    void operator()(...) { }
};

// Assumption: a static_wrapper type is identified by the type T and INITIALIZER.
// We assume the case that two different templates sharing the same T and INITIALIZER
// cannot happen.
// Note: if use default INITIALIZER, no initialization.
template<typename T, typename INITIALIZER = DummyInitializer>
class static_wrapper
{
    A16 static T value;
public:
    static_wrapper(INITIALIZER init_func = INITIALIZER())
    {
        init_func(value);
    }

    __forceinline operator T&() const
    {
        return value;
    }

    __forceinline T& get() const
    {
        return value;
    }
};
template<typename T, typename INITIALIZER>
T static_wrapper<T, INITIALIZER>::value;

template<int N>
class Log2
{
public:
    enum { value = Log2<N/2>::value + 1 };
};

template<> 
class Log2<1>
{
public:
    enum { value = 0 };
};

// IsPower2 statically tests if N is a power of two
template<int N>
struct IsPower2 { enum { IsFalse = (N & 1) + IsPower2<N/2>::IsFalse }; };
template<>
struct IsPower2<1> { enum { IsFalse = 0 }; };

