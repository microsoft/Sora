// Header file to help coding with variadic macro
#pragma once

// __VA_NARGS__ macro return the number of arguments contained in __VA_ARGS__
// Note: zero argument is not supported
// ref: http://groups.google.com/group/comp.std.c/browse_thread/thread/77ee8c8f92e4a3fb/346fc464319b1ee5
#define __VA_NARGS__(...) __VA_NARGS_1((__VA_ARGS__, NARGS_SEQ))
#define __VA_NARGS_1(...) __VA_NARGS_2  __VA_ARGS__
#define NARGS_SEQ                                   \
                          63,62,61,60,              \
        59,58,57,56,55,54,53,52,51,50,              \
        49,48,47,46,45,44,43,42,41,40,              \
        39,38,37,36,35,34,33,32,31,30,              \
        29,28,27,26,25,24,23,22,21,20,              \
        19,18,17,16,15,14,13,12,11,10,              \
         9, 8, 7, 6, 5, 4, 3, 2, 1, 0 
#define __VA_NARGS_2( \
         _1, _2, _3, _4, _5, _6, _7, _8, _9,_10,    \
        _11,_12,_13,_14,_15,_16,_17,_18,_19,_20,    \
        _21,_22,_23,_24,_25,_26,_27,_28,_29,_30,    \
        _31,_32,_33,_34,_35,_36,_37,_38,_39,_40,    \
        _41,_42,_43,_44,_45,_46,_47,_48,_49,_50,    \
        _51,_52,_53,_54,_55,_56,_57,_58,_59,_60,    \
        _61,_62,_63, N, ...) N

// COMMA_SEP macro return comma seperated decorated list
// eg:
//   COMMA_SEP(virtual, a, b, c) will be expanded to
//   virtual a, virtual b, virtual c
#define COMMA_SEP(DECORATOR, ...) __COMMA_SEP_D1((__VA_NARGS__(__VA_ARGS__), (DECORATOR, __VA_ARGS__)))
#define __COMMA_SEP_D1(...) __COMMA_SEP_D2 __VA_ARGS__
#define __COMMA_SEP_D2(N, ...) CONCATENATE(COMMA_SEP_, N) __VA_ARGS__

#define COMMA_SEP_1(DECORATOR, X) DECORATOR X
#define COMMA_SEP_2(DECORATOR, X, ...) DECORATOR X, COMMA_SEP_1(DECORATOR, __VA_ARGS__)
#define __COMMA_SEP_2(...) COMMA_SEP_2 __VA_ARGS__
#define COMMA_SEP_3(DECORATOR, X, ...) DECORATOR X, __COMMA_SEP_2((DECORATOR, __VA_ARGS__))
#define __COMMA_SEP_3(...) COMMA_SEP_3 __VA_ARGS__
#define COMMA_SEP_4(DECORATOR, X, ...) DECORATOR X, __COMMA_SEP_3((DECORATOR, __VA_ARGS__))
#define __COMMA_SEP_4(...) COMMA_SEP_4 __VA_ARGS__
#define COMMA_SEP_5(DECORATOR, X, ...) DECORATOR X, __COMMA_SEP_4((DECORATOR, __VA_ARGS__))
#define __COMMA_SEP_5(...) COMMA_SEP_5 __VA_ARGS__
#define COMMA_SEP_6(DECORATOR, X, ...) DECORATOR X, __COMMA_SEP_5((DECORATOR, __VA_ARGS__))
#define __COMMA_SEP_6(...) COMMA_SEP_6 __VA_ARGS__
#define COMMA_SEP_7(DECORATOR, X, ...) DECORATOR X, __COMMA_SEP_6((DECORATOR, __VA_ARGS__))
#define __COMMA_SEP_7(...) COMMA_SEP_7 __VA_ARGS__
#define COMMA_SEP_8(DECORATOR, X, ...) DECORATOR X, __COMMA_SEP_7((DECORATOR, __VA_ARGS__))
#define __COMMA_SEP_8(...) COMMA_SEP_8 __VA_ARGS__
#define COMMA_SEP_9(DECORATOR, X, ...) DECORATOR X, __COMMA_SEP_8((DECORATOR, __VA_ARGS__))
#define __COMMA_SEP_9(...) COMMA_SEP_9 __VA_ARGS__
#define COMMA_SEP_10(DECORATOR, X, ...) DECORATOR X, __COMMA_SEP_10((DECORATOR, __VA_ARGS__))
#define __COMMA_SEP_10(...) COMMA_SEP_10 __VA_ARGS__


// VIRTUAL_SEP macro return "virtual" seperated list
// eg:
//   VIRTUAL_SEP(a, b, c) will be expanded to
//   virtual a, virtual b, virtual c
#define VIRTUAL_SEP(...) COMMA_SEP(virtual, __VA_ARGS__)
