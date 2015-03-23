#pragma once
#include "const.h"

// template trick for static_assert
// will be obsolated with VC 10

// compile time assert
template<bool B> struct _tagAssert {};
template<> struct _tagAssert<true> {static void assert() {}; };
#define STATIC_ASSERT_TYPE(T1,T2) { T2* __p = NULL; T1* __q = __p; }

// template numerics
// Greatest Common Division
template<int u, int v> struct gcd { enum {value = gcd<v, u % v>::value }; };
template<int u> struct gcd <u, 0> { enum {value = u }; };
template<int u, int v> struct lcm { enum {value = u * v / gcd<u,v>::value}; };


