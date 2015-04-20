// Fundamental operators defined on SSE vector registers
#pragma once

// WARNING: be careful when including non default header files, in order to maintain the independency
#include <limits.h>
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include "complex.h"

// Note: SSE2 intrinsic functions are assumed to be supported on target platform

// CPU support for SSE4 intrinsic functions is optional, if not, please disable the macro __SSE4__
//#define __SSE4__

// NOTE!!!!
// Set code optimization option "MSC_OPTIMIZATION" to /O2 (maximize speed)
// in the sources file, which is necessary for DSP related inline functions.
// Otherwise inline functions will not inline expanded. The default value
// is /Oxs (minimize size) in free build, and /Od /Oi in check build.
// ref: http://msdn.microsoft.com/en-us/library/ff549305%28VS.85%29.aspx
#define DSP_INLINE		__forceinline
// Note: DSP_INLINE1 is just a hint for compilier, not compulsive
#define DSP_INLINE1		inline

// Note: below declaration are orginal found in <tmmintrin.h> in VS2008 include directory.
// However the header file is missing in WinDDK\7600.16385.0, so copy here to remove the dependency
extern "C" extern __m128i _mm_sign_epi8 (__m128i a, __m128i b);
extern "C" extern __m128i _mm_sign_epi16 (__m128i a, __m128i b);
extern "C" extern __m128i _mm_sign_epi32 (__m128i a, __m128i b);
extern "C" extern __m128i _mm_sign_epi64 (__m128i a, __m128i b);
extern "C" extern __m128i _mm_abs_epi8 (__m128i a);
extern "C" extern __m128i _mm_abs_epi16 (__m128i a);
extern "C" extern __m128i _mm_abs_epi32 (__m128i a);
extern "C" extern __m128i _mm_abs_epi64 (__m128i a);
extern "C" extern __m128i _mm_alignr_epi8 (__m128i a, __m128i b, const int ralign);
extern "C" extern __m128i _mm_mulhrs_epi16(__m128i a, __m128i b);
extern "C" extern __m128i _mm_mullo_epi32(__m128i a, __m128i b);

extern "C" extern __m128i _mm_shuffle_epi8 (__m128i a, __m128i b);
extern "C" extern __m128i _mm_insert_epi32 (__m128i a, int b, const int ndx );
extern "C" extern __m128i _mm_insert_epi16 (__m128i a, int b, const int ndx );

// Note: 
// 1. Below declaration are orginal found in <smmintrin.h> in VS2008 include directory.
//    However the header file is missing in WinDDK\7600.16385.0, so copy here to remove the dependency
// 2. Below are SSE4.1 intrinsics, it may be compiled on other CPU, but runtime will throw
//    an unhandled exception of illegal instruction
extern "C" extern __m128i _mm_min_epi8 (__m128i val1, __m128i val2);
extern "C" extern __m128i _mm_max_epi8 (__m128i val1, __m128i val2);
extern "C" extern __m128i _mm_min_epu16(__m128i val1, __m128i val2);
extern "C" extern __m128i _mm_max_epu16(__m128i val1, __m128i val2);
extern "C" extern __m128i _mm_min_epi32(__m128i val1, __m128i val2);
extern "C" extern __m128i _mm_max_epi32(__m128i val1, __m128i val2);
extern "C" extern __m128i _mm_min_epu32(__m128i val1, __m128i val2);
extern "C" extern __m128i _mm_max_epu32(__m128i val1, __m128i val2);
extern "C" extern __m128i _mm_mul_epi32(__m128i a, __m128i b);
extern "C" extern __m128i _mm_cvtepi16_epi32(__m128i shortValues);
extern "C" extern __m128i _mm_cmpeq_epi64(__m128i val1, __m128i val2);

// Note: below declaration are orginal found in <smmintrin.h> in VS2008 include directory.
extern "C" extern int _mm_extract_epi8 (__m128i src, const int ndx);
extern "C" extern int _mm_extract_epi32(__m128i src, const int ndx);
#if defined(_M_X64)
extern "C" extern __int64 _mm_extract_epi64(__m128i src, const int ndx);
#endif

// Note: below declaration are orginal found in <intrin.h> in VS2008 include directory.
#if defined(_M_X64)
extern "C" extern __m128i _mm_set1_epi64x(__int64 i);
#endif

namespace vector128 { namespace details {
    template<typename T> struct traits { enum { IsVectorType = 0 }; };
} }


//////////////////////////////////////////////////////////////////////////////
// Data structure definition for m128 wrapper types
// Note:
// 1. normally we use template to define similiar structs, but in this case,
// align() parameter is hard to defined by template parameter, so we use macro
// instead.
// 2. the structs all have constructors, so don't define global static objects
// in kernel mode, otherwise, .CRT section will be introduced.
#define PVECTOR_STRUCT(NEWSTRUCT, TRAW, TDATA, ALIGN)		                                        \
struct NEWSTRUCT												                                    \
{																                                    \
    static const size_t size = sizeof(TRAW) / sizeof(TDATA);	                                    \
    typedef __declspec(align(ALIGN)) TDATA data_type[size];		                                    \
    typedef TRAW raw_type;                                                                          \
    typedef TDATA elem_type;                                                                        \
    NEWSTRUCT() { }												                                    \
    explicit NEWSTRUCT(TRAW r) : _raw(r) { }					                                    \
    template<typename TA> __forceinline explicit NEWSTRUCT(const TA& a) : _raw(TRAW(a)) { }         \
    __forceinline NEWSTRUCT(const data_type& pa) : _raw(*(TRAW*)pa) { }		                        \
    __forceinline TDATA& operator[](size_t index) { return _data[index]; }	                        \
    __forceinline const TDATA& operator[](size_t index) const { return _data[index]; }	            \
    __forceinline NEWSTRUCT& operator=(const NEWSTRUCT& a) { _raw = a._raw; return *this; }	        \
    __forceinline NEWSTRUCT& operator=(const data_type& pa) { _raw = *(TRAW*)pa; return *this; }	\
    __forceinline NEWSTRUCT& operator=(const raw_type& r) { _raw = r; return *this; }	            \
    __forceinline operator TRAW&() { return _raw; }                                                 \
    __forceinline operator const TRAW&() const { return _raw; }                                     \
private:                                                                                            \
    union														                                    \
    {															                                    \
        TRAW _raw;												                                    \
        TDATA _data[size];										                                    \
    };															                                    \
};                                                                                                  \
namespace vector128 { namespace details {                                                           \
    template<> struct traits<NEWSTRUCT> {                                                           \
        typedef NEWSTRUCT::raw_type tag;                                                            \
        enum { IsVectorType = 1 };                                                                  \
    };                                                                                              \
} }

PVECTOR_STRUCT(vb,   __m128i, __int8,           16);
PVECTOR_STRUCT(vub,  __m128i, unsigned __int8,  16);
PVECTOR_STRUCT(vs,   __m128i, __int16,          16);
PVECTOR_STRUCT(vus,  __m128i, unsigned __int16, 16);
PVECTOR_STRUCT(vi,   __m128i, __int32,          16);
PVECTOR_STRUCT(vui,  __m128i, unsigned __int32, 16);
PVECTOR_STRUCT(vq,   __m128i, __int64,          16);
PVECTOR_STRUCT(vuq,  __m128i, unsigned __int64, 16);
PVECTOR_STRUCT(vcb,  __m128i, COMPLEX8,         16);
PVECTOR_STRUCT(vcub, __m128i, COMPLEXU8,        16);
PVECTOR_STRUCT(vcs,  __m128i, COMPLEX16,        16);
PVECTOR_STRUCT(vcus, __m128i, COMPLEXU16,       16);
PVECTOR_STRUCT(vci,  __m128i, COMPLEX32,        16);
PVECTOR_STRUCT(vcui, __m128i, COMPLEXU32,       16);
PVECTOR_STRUCT(vcq,  __m128i, COMPLEX64,        16);
PVECTOR_STRUCT(vcuq, __m128i, COMPLEXU64,       16);

#ifdef USER_MODE
PVECTOR_STRUCT(vf,   __m128,  float,            16);
PVECTOR_STRUCT(vcf,  __m128,  COMPLEXF,         16);
#endif

namespace vector128 { namespace details {
    template <class T> struct CrackComplexType { typedef T type; };
    template <> struct CrackComplexType<vcb> { typedef vb type; };
    template <> struct CrackComplexType<vcs> { typedef vs type; };
    template <> struct CrackComplexType<vci> { typedef vi type; };
    template <> struct CrackComplexType<vcq> { typedef vq type; };
#ifdef USER_MODE
    template <> struct CrackComplexType<vcf> { typedef vf type; };
#endif
} }

//////////////////////////////////////////////////////////////////////////////
// Constants

// Note:
//   1. embedded all consts into function so that we have opportunity to optimize its implemtation
//   2. possible impl: RAM, dynamic calc
class vector128_consts
{
public:
    template<typename T>
    DSP_INLINE static const T zero()
    {
        T ret;
        set_zero(ret);
        return ret;
    }

    template<typename T>
    DSP_INLINE static const T __0x0000FFFF0000FFFF0000FFFF0000FFFF()
    {
        const static vub::data_type value =
        {
            0xFF, 0xFF, 0x00, 0x00,
            0xFF, 0xFF, 0x00, 0x00,
            0xFF, 0xFF, 0x00, 0x00,
            0xFF, 0xFF, 0x00, 0x00
        };
        return (T)vub(value);
    }

    template<typename T>
    DSP_INLINE static const T __0x80000001800000018000000180000001()
    {
        const static vus::data_type value =
        {
            0x1, 0x8000,
            0x1, 0x8000,
            0x1, 0x8000,
            0x1, 0x8000
        };
        return (T&)vus(value);
    }

    template<typename T>
    DSP_INLINE static const T __0xFFFF0000FFFF0000FFFF0000FFFF0000()
    {
        vi t;
        set_all_bits(t);
        t = shift_left(t, 16);
        return (T)t;
    }

    template<typename T>
    DSP_INLINE static const T __0xFFFFFFFF00000000FFFFFFFF00000000()
    {
        vq t;
        set_all_bits(t);
        t = shift_left(t, 32);
        return (T)t;
    }

    template<typename T>
    DSP_INLINE static const T __0xFFFFFFFFFFFFFFFF0000000000000000()
    {
        vq t;
        set_all_bits(t);
        // Shift left by 8 bytes
        t = (vq)_mm_slli_si128(t, 8);
        return (T)t;
    }

    template<typename T>
    DSP_INLINE static const T __0xFFFF0000FFFF00000000000000000000()
    {
        vi t;
        set_all_bits(t);
        // Shift left by 8 bytes
        t = (vi)_mm_slli_si128(t, 8);
        t = shift_left(t, 16);
        return (T)t;
    }

    template<typename T>
    DSP_INLINE static const T __0xFFFF0000000000000000000000000000()
    {
        vq t;
        set_all_bits(t);
        // Shift left by 14 bytes
        t = (vq)_mm_slli_si128(t, 14);
        return (T)t;
    }

    template<typename T> DSP_INLINE static const T middle() { }
    template<> DSP_INLINE static const vub middle<vub>()
    {
        const static vub::data_type mid = {
            0x80, 0x80, 0x80, 0x80, 
            0x80, 0x80, 0x80, 0x80,
            0x80, 0x80, 0x80, 0x80, 
            0x80, 0x80, 0x80, 0x80,
        };
        return mid;
    }
    template<> DSP_INLINE static const vs middle<vs>()
    {
        const static vus::data_type mid = {
            0x8000, 0x8000, 0x8000, 0x8000,
            0x8000, 0x8000, 0x8000, 0x8000,
        };
        return (vs)vus(mid);
    }
};

//////////////////////////////////////////////////////////////////////////////
// Private macros

// Dummy declaration
#define DECLARE_PUBLIC_OP(OP) 

// Macro to define operators with 1 operand and 1 result, which are of the same type
// Note: non template version
#define DEFINE_OP_ARITHMETIC1(OP, T, INSTRINSIC)        \
    DSP_INLINE T OP(const T& a) { return (T)INSTRINSIC(a); }

// Macro to define operators with 2 operands and 1 result, which are of the same type
// Using template to define operations are all m128 wrapper types
#define DEFINE_TEMPLATE_OP_ARITHMETIC2(OP, INSTRINSIC)              \
    template<typename T>                                            \
    DSP_INLINE T OP(const T& a, const T& b, typename vector128::details::traits<T>::tag * = 0) {    \
        return (T)INSTRINSIC(a, b);                                 \
    }

// Macro to define operators with 2 operands and 1 result, which are of the same type
// Note: non template version
#define DEFINE_OP_ARITHMETIC2(OP, T, INSTRINSIC)        \
    DSP_INLINE T OP(const T& a, const T& b) { return (T)INSTRINSIC(a, b); }

// Macro to define permutation operators
#define DEFINE_OP_PERMUTATION(OP, T, INSTRINSIC)        \
    template<int n>                                     \
    DSP_INLINE T OP(const T& a) { return (T)INSTRINSIC(a, n); }

// Macro to define 4-element permutation operators
// Note: it is indeed 2 templates, including DEFINE_OP_PERMUTATION
#define DEFINE_OP_PERMUTATION4(OP, T, INSTRINSIC)       \
    template<int a0, int a1, int a2, int a3>            \
    DSP_INLINE T OP(const T& a) { return (T)INSTRINSIC(a, _MM_SHUFFLE(a3, a2, a1, a0)); }   \
    DEFINE_OP_PERMUTATION(OP, T, INSTRINSIC)

// Macro to define shift operatiors
#define DEFINE_OP_SHIFT(OP, T, INSTRINSIC)              \
    DSP_INLINE T OP(const T& a, int nbits) { return (T)INSTRINSIC(a, nbits); }
#define DEFINE_OP_SHIFT_LEFT(OP, T, INSTRINSIC)         \
    DEFINE_OP_SHIFT(OP, T, INSTRINSIC)                  \
    DSP_INLINE T operator <<(const T& a, int nbits) { return OP(a, nbits); }
#define DEFINE_OP_SHIFT_RIGHT(OP, T, INSTRINSIC)        \
    DEFINE_OP_SHIFT(OP, T, INSTRINSIC)                  \
    DSP_INLINE T operator >>(const T& a, int nbits) { return OP(a, nbits); }


// Macro to define 4-way reducing operators
// eg. Get sum of 4 elements at the same index in each operand and comprise the returned vector by them
// Note:
//   1. all operands, returned wrapper and all intermediate wrapper are of the same type T
//   2. REDUCE_OP is the underlying 2-way reducing operator
#define DEFINE_OP_REDUCE4(OP, T, REDUCE_OP)                             \
DSP_INLINE T OP(const T& a0, const T& a1, const T& a2, const T& a3)     \
{                                                                       \
    T temp1, temp2, temp3;                                              \
    temp1 = interleave_low(a0, a2);         /* 2.1 0.1 2.0 0.0*/        \
    temp2 = interleave_high(a0, a2);        /* 2.3 0.3 2.2 0.2*/        \
    temp3 = REDUCE_OP(temp1, temp2);        /* 2.13 0.13 2.02 0.02*/    \
                                                                        \
    temp1 = interleave_low(a1, a3);         /* 3.1 1.1 3.0 1.0*/        \
    temp2 = interleave_high(a1, a3);        /* 3.3 1.3 3.2 1.2*/        \
    temp1 = REDUCE_OP(temp1, temp2);        /* 3.13 1.13 3.02 1.02*/    \
                                                                        \
    temp2 = interleave_low(temp3, temp1);   /* 3.02 2.02 1.02 0.02*/    \
    temp3 = interleave_high(temp3, temp1);  /* 3.13 2.13 1.13 0.13*/    \
    return REDUCE_OP(temp2, temp3);                                     \
}

// Iterate a binary operation (eg. maximal/minimal) on all 4 components in a vector128 type,
// and duplicate the final value to all elements in the returned vector
#define DEFINE_OP_DUPLICATION4_OPERATION(OP, T, REDUCE_OP)								\
DSP_INLINE T OP(const T& a)												\
{																		\
    T t = REDUCE_OP(a, permutate<0xb1>(a));							    \
    return REDUCE_OP(t, permutate<0x4e>(t));							\
}

// Iterate a binary operation (eg. maximal/minimal) on all 16 components in a vector128 type,
// and duplicate the final value to all elements in the returned vector
#define DEFINE_OP_DUPLICATION16_OPERATION(OP, T, OPER)      \
DSP_INLINE T OP(const T& a)                                 \
{                                                           \
    T xmm0, xmm1 = a;                                       \
    xmm0 = (T)permutate<0x4E>((vcs)xmm1);                   \
    xmm0 = OPER(xmm0, xmm1);                                \
    xmm1 = (T)permutate<0xB1>((vcs)xmm0);                   \
    xmm0 = OPER(xmm0, xmm1);                                \
    xmm0 = interleave_low(xmm0, xmm0);                      \
    xmm0 = interleave_low(xmm0, xmm0);                      \
    xmm1 = (T)permutate<0x4E>((vcs)xmm0);                   \
    xmm0 = OPER(xmm0, xmm1);                                \
    xmm1 = (T)permutate<0xB1>((vcs)xmm0);                   \
    xmm0 = OPER(xmm0, xmm1);                                \
    return xmm0;                                            \
}

// Iterate a binary operation (eg. maximal/minimal) on all 8 components in a vector128 type,
// and duplicate the final value to all elements in the returned vector
#define DEFINE_OP_DUPLICATION8_OPERATION(OP, T, OPER)       \
DSP_INLINE T OP(const T& a)                                 \
{                                                           \
    T r1, r2, r0 = a;                                       \
    r1 = (T)permutate<0x4E>((vcs&)r0);                      \
    r2 = OPER(r0, r1);                                      \
    r1 = (T)permutate<0xB1>((vcs&)r2);                      \
    r2 = OPER(r2, r1);                                      \
    r1 = (T)permutate_low<0xB1>(r2);                        \
    r1 = (T)permutate<1, 0, 1, 0>((vcs&)r1);                \
    r2 = OPER(r2, r1);                                      \
    return r2;                                              \
}

// Macro to define min/max for unsigned vector128 from signed one or vice-verse
#define DEFINE_OP_MINMAX_SIGNED_UNSIGNED(OP, T, TA)         \
DSP_INLINE T OP(const T& a, const T& b)                     \
{                                                           \
    const static TA mid = vector128_consts::middle<TA>();   \
    TA aa = xor((TA&)a, mid);                               \
    TA bb = xor((TA&)b, mid);                               \
    return (T)xor(OP(aa, bb), mid);                         \
}

// Micro to define extracting method
#define DEFINE_OP_EXTRACT(T, INTRINSIC)                     \
    template<int index> DSP_INLINE typename T::elem_type extract(const T& a) { return (typename T::elem_type)INTRINSIC(a, index); }

//////////////////////////////////////////////////////////////////////////////
// Public functions defined on vector types

// Clear all bits in a vector
template<typename T>
DSP_INLINE void set_zero(T& a, typename vector128::details::traits<T>::tag * = 0)
{
    __m128 t = _mm_setzero_ps(); a = (typename T::raw_type&)t;
}

template<typename T>
DSP_INLINE bool equal(const T& a, const T& b, typename vector128::details::traits<T>::tag * = 0)
{
    __m128i vcmp = _mm_cmpeq_epi32((__m128i&)a, (__m128i&)b);
    int vmask = _mm_movemask_epi8(vcmp);
    return vmask == 0xffff;
}

// Set all bits in a vector
#pragma warning (push)
#pragma warning (disable:4700)
template<typename T>
DSP_INLINE void set_all_bits(T& a, typename vector128::details::traits<T>::tag * = 0)
{
    __m128i t;
#ifndef NDEBUG
    // Note: prevent runtime error in debug mode: variable is used without initialization
    *reinterpret_cast<__m128*>(&t) = _mm_setzero_ps();
#endif
    t = _mm_cmpeq_epi32(t, t);
    reinterpret_cast<__m128i&>(a) = t;
}
#pragma warning (pop)

// Interleave the elements in lower half of 2 source vectors to a resulting vector
// The first source operand will be interleaved to the even indices, and the second source
// to the odd indices.
DEFINE_OP_ARITHMETIC2(interleave_low, vb, _mm_unpacklo_epi8);
DEFINE_OP_ARITHMETIC2(interleave_low, vs, _mm_unpacklo_epi16);
DEFINE_OP_ARITHMETIC2(interleave_low, vi, _mm_unpacklo_epi32);
DEFINE_OP_ARITHMETIC2(interleave_low, vq, _mm_unpacklo_epi64);
DEFINE_OP_ARITHMETIC2(interleave_low, vcb, _mm_unpacklo_epi16);
DEFINE_OP_ARITHMETIC2(interleave_low, vcs, _mm_unpacklo_epi32);
DEFINE_OP_ARITHMETIC2(interleave_low, vci, _mm_unpacklo_epi64);
DEFINE_OP_ARITHMETIC2(interleave_low, vub, _mm_unpacklo_epi8);
DEFINE_OP_ARITHMETIC2(interleave_low, vus, _mm_unpacklo_epi16);
DEFINE_OP_ARITHMETIC2(interleave_low, vui, _mm_unpacklo_epi32);
DEFINE_OP_ARITHMETIC2(interleave_low, vuq, _mm_unpacklo_epi64);
DEFINE_OP_ARITHMETIC2(interleave_low, vcub, _mm_unpacklo_epi16);
DEFINE_OP_ARITHMETIC2(interleave_low, vcus, _mm_unpacklo_epi32);
DEFINE_OP_ARITHMETIC2(interleave_low, vcui, _mm_unpacklo_epi64);

// Interleave the elements in higher half of 2 source vectors to a resulting vector
// The first source operand will be interleaved to the even indices, and the second source
// to the odd indices.
DEFINE_OP_ARITHMETIC2(interleave_high, vb, _mm_unpackhi_epi8);
DEFINE_OP_ARITHMETIC2(interleave_high, vs, _mm_unpackhi_epi16);
DEFINE_OP_ARITHMETIC2(interleave_high, vi, _mm_unpackhi_epi32);
DEFINE_OP_ARITHMETIC2(interleave_high, vq, _mm_unpackhi_epi64);
DEFINE_OP_ARITHMETIC2(interleave_high, vcb, _mm_unpackhi_epi16);
DEFINE_OP_ARITHMETIC2(interleave_high, vcs, _mm_unpackhi_epi32);
DEFINE_OP_ARITHMETIC2(interleave_high, vci, _mm_unpackhi_epi64);
DEFINE_OP_ARITHMETIC2(interleave_high, vub, _mm_unpackhi_epi8);
DEFINE_OP_ARITHMETIC2(interleave_high, vus, _mm_unpackhi_epi16);
DEFINE_OP_ARITHMETIC2(interleave_high, vui, _mm_unpackhi_epi32);
DEFINE_OP_ARITHMETIC2(interleave_high, vuq, _mm_unpackhi_epi64);
DEFINE_OP_ARITHMETIC2(interleave_high, vcub, _mm_unpackhi_epi16);
DEFINE_OP_ARITHMETIC2(interleave_high, vcus, _mm_unpackhi_epi32);
DEFINE_OP_ARITHMETIC2(interleave_high, vcui, _mm_unpackhi_epi64);

// Compare the elements in 2 vectors in the same type for greater than (>). The resulting vector
// is also in the same type, with elements all-1 for true and 0 for false.
DEFINE_OP_ARITHMETIC2(is_great, vb, _mm_cmpgt_epi8);
DEFINE_OP_ARITHMETIC2(is_great, vs, _mm_cmpgt_epi16);
DEFINE_OP_ARITHMETIC2(is_great, vi, _mm_cmpgt_epi32);
#ifdef USER_MODE
DEFINE_OP_ARITHMETIC2(is_great, vf, _mm_cmpgt_ps);
#endif

// Compare the elements in 2 vectors in the same type for less than (<). The resulting vector
// is also in the same type, with elements all-1 for true and 0 for false.
DEFINE_OP_ARITHMETIC2(is_less, vb, _mm_cmplt_epi8);
DEFINE_OP_ARITHMETIC2(is_less, vs, _mm_cmplt_epi16);
DEFINE_OP_ARITHMETIC2(is_less, vi, _mm_cmplt_epi32);
#ifdef USER_MODE
DEFINE_OP_ARITHMETIC2(is_less, vf, _mm_cmplt_ps);
#endif

// Element-wise polarization on the first operand based on the second operand, ie.
// r[n] := (b[n] < 0) ? -a[n] : ((b[n] == 0) ? 0 : a[n])
DEFINE_OP_ARITHMETIC2(sign, vb, _mm_sign_epi8);
DEFINE_OP_ARITHMETIC2(sign, vs, _mm_sign_epi16);
DEFINE_OP_ARITHMETIC2(sign, vi, _mm_sign_epi32);
DEFINE_OP_ARITHMETIC2(sign, vq, _mm_sign_epi64);
DEFINE_OP_ARITHMETIC2(sign, vcs, _mm_sign_epi16);

// Extract element from a vector types, similar to index operator. The index is 0-based and start from
// the lowest address.
DEFINE_OP_EXTRACT(vb, _mm_extract_epi8);
DEFINE_OP_EXTRACT(vub, _mm_extract_epi8);
DEFINE_OP_EXTRACT(vs, _mm_extract_epi16);
DEFINE_OP_EXTRACT(vus, _mm_extract_epi16);
DEFINE_OP_EXTRACT(vi, _mm_extract_epi32);
DEFINE_OP_EXTRACT(vui, _mm_extract_epi32);
DEFINE_OP_EXTRACT(vq, _mm_extract_epi64);
DEFINE_OP_EXTRACT(vuq, _mm_extract_epi64);

// Extract a vector type from 2 vectors concatenated together
// r := (CONCAT(a, b) >> (nbytes * 8)) & 0xffffffffffffffff
// Note: _mm_alignr_epi8 expects const shift parameter, so template is introduced
template<int nbytes, typename T>
DSP_INLINE T concat_extract(const T& a, const T& b) { return (T)_mm_alignr_epi8(a, b, nbytes); }

// Bitwise OR
DEFINE_TEMPLATE_OP_ARITHMETIC2(or, _mm_or_si128);
// Bitwise XOR
DEFINE_TEMPLATE_OP_ARITHMETIC2(xor, _mm_xor_si128);
// Bitwise AND
DEFINE_TEMPLATE_OP_ARITHMETIC2(and, _mm_and_si128);
// Bitwise ANDNOT
// ANDNOT(a, b) == (NOT(a)) AND b
DEFINE_TEMPLATE_OP_ARITHMETIC2(andnot, _mm_andnot_si128);
// Bitwise NOT
template<typename T>
DSP_INLINE T not(const T& a, typename vector128::details::traits<T>::tag * = 0)
{
    vi all;
    set_all_bits(all);
    return andnot(a, b);
}

// Element-wise add
DEFINE_OP_ARITHMETIC2(add, vb, _mm_add_epi8);
DEFINE_OP_ARITHMETIC2(add, vub, _mm_add_epi8);
DEFINE_OP_ARITHMETIC2(add, vs, _mm_add_epi16);
DEFINE_OP_ARITHMETIC2(add, vus, _mm_add_epi16);
DEFINE_OP_ARITHMETIC2(add, vi, _mm_add_epi32);
DEFINE_OP_ARITHMETIC2(add, vui, _mm_add_epi32);
DEFINE_OP_ARITHMETIC2(add, vq, _mm_add_epi64);
DEFINE_OP_ARITHMETIC2(add, vuq, _mm_add_epi64);
#ifdef USER_MODE
DEFINE_OP_ARITHMETIC2(add, vf, _mm_add_ps);
#endif
DEFINE_OP_ARITHMETIC2(add, vcb, _mm_add_epi8);
DEFINE_OP_ARITHMETIC2(add, vcub, _mm_add_epi8);
DEFINE_OP_ARITHMETIC2(add, vcs, _mm_add_epi16);
DEFINE_OP_ARITHMETIC2(add, vcus, _mm_add_epi16);
DEFINE_OP_ARITHMETIC2(add, vci, _mm_add_epi32);
DEFINE_OP_ARITHMETIC2(add, vcui, _mm_add_epi32);
DEFINE_OP_ARITHMETIC2(add, vcq, _mm_add_epi64);
DEFINE_OP_ARITHMETIC2(add, vcuq, _mm_add_epi64);
#ifdef USER_MODE
DEFINE_OP_ARITHMETIC2(add, vcf, _mm_add_ps);
#endif

// Element-wise Subtract
DEFINE_OP_ARITHMETIC2(sub, vb, _mm_sub_epi8);
DEFINE_OP_ARITHMETIC2(sub, vub, _mm_sub_epi8);
DEFINE_OP_ARITHMETIC2(sub, vs, _mm_sub_epi16);
DEFINE_OP_ARITHMETIC2(sub, vus, _mm_sub_epi16);
DEFINE_OP_ARITHMETIC2(sub, vi, _mm_sub_epi32);
DEFINE_OP_ARITHMETIC2(sub, vui, _mm_sub_epi32);
DEFINE_OP_ARITHMETIC2(sub, vq, _mm_sub_epi64);
DEFINE_OP_ARITHMETIC2(sub, vuq, _mm_sub_epi64);
#ifdef USER_MODE
DEFINE_OP_ARITHMETIC2(sub, vf, _mm_sub_ps);
#endif
DEFINE_OP_ARITHMETIC2(sub, vcb, _mm_sub_epi8);
DEFINE_OP_ARITHMETIC2(sub, vcub, _mm_sub_epi8);
DEFINE_OP_ARITHMETIC2(sub, vcs, _mm_sub_epi16);
DEFINE_OP_ARITHMETIC2(sub, vcus, _mm_sub_epi16);
DEFINE_OP_ARITHMETIC2(sub, vci, _mm_sub_epi32);
DEFINE_OP_ARITHMETIC2(sub, vcui, _mm_sub_epi32);
DEFINE_OP_ARITHMETIC2(sub, vcq, _mm_sub_epi64);
DEFINE_OP_ARITHMETIC2(sub, vcuq, _mm_sub_epi64);
#ifdef USER_MODE
DEFINE_OP_ARITHMETIC2(sub, vcf, _mm_sub_ps);
#endif

// Element-wise saturated add
// Note: there is no _mm_adds_epi/u for 32/64 size elements
DEFINE_OP_ARITHMETIC2(saturated_add, vb, _mm_adds_epi8);
DEFINE_OP_ARITHMETIC2(saturated_add, vub, _mm_adds_epu8);
DEFINE_OP_ARITHMETIC2(saturated_add, vs, _mm_adds_epi16);
DEFINE_OP_ARITHMETIC2(saturated_add, vus, _mm_adds_epu16);
DEFINE_OP_ARITHMETIC2(saturated_add, vcb, _mm_adds_epi8);
DEFINE_OP_ARITHMETIC2(saturated_add, vcub, _mm_adds_epu8);
DEFINE_OP_ARITHMETIC2(saturated_add, vcs, _mm_adds_epi16);
DEFINE_OP_ARITHMETIC2(saturated_add, vcus, _mm_adds_epu16);

// Element-wise saturated subtract
DEFINE_OP_ARITHMETIC2(saturated_sub, vb, _mm_subs_epi8);
DEFINE_OP_ARITHMETIC2(saturated_sub, vub, _mm_subs_epu8);
DEFINE_OP_ARITHMETIC2(saturated_sub, vs, _mm_subs_epi16);
DEFINE_OP_ARITHMETIC2(saturated_sub, vus, _mm_subs_epu16);
DEFINE_OP_ARITHMETIC2(saturated_sub, vcb, _mm_subs_epi8);
DEFINE_OP_ARITHMETIC2(saturated_sub, vcub, _mm_subs_epu8);
DEFINE_OP_ARITHMETIC2(saturated_sub, vcs, _mm_subs_epi16);
DEFINE_OP_ARITHMETIC2(saturated_sub, vcus, _mm_subs_epu16);

// Element-wise average of 2 operands vector
DEFINE_OP_ARITHMETIC2(average, vub, _mm_avg_epu8);
DEFINE_OP_ARITHMETIC2(average, vus, _mm_avg_epu16);
DEFINE_OP_ARITHMETIC2(average, vcub, _mm_avg_epu8);
DEFINE_OP_ARITHMETIC2(average, vcus, _mm_avg_epu16);

// Permutate 4 elements in the lower half vector
// Template parameter n: each 2-bit field (from LSB) selects the contents of one element location
// (from low address) in the destination operand. ie.
// r[0] := a[n(1:0)]
// r[1] := a[n(3:2)]
// r[2] := a[n(5:4)]
// r[3] := a[n(7:6)]
// Template parameter a0 ~ a3: selects the contents of one element location in the destination operand. ie.
// r[0] := a[a0]
// r[1] := a[a1]
// r[2] := a[a2]
// r[3] := a[a3]
DEFINE_OP_PERMUTATION4(permutate_low, vs, _mm_shufflelo_epi16);
DEFINE_OP_PERMUTATION4(permutate_low, vcs, _mm_shufflelo_epi16);
DEFINE_OP_PERMUTATION4(permutate_low, vus, _mm_shufflelo_epi16);
DEFINE_OP_PERMUTATION4(permutate_low, vcus, _mm_shufflelo_epi16);
// Permutate 4 elements in the higher half vector
// The definitions of template parameters are similar to permutate_low
DEFINE_OP_PERMUTATION4(permutate_high, vs, _mm_shufflehi_epi16);
DEFINE_OP_PERMUTATION4(permutate_high, vcs, _mm_shufflehi_epi16);
DEFINE_OP_PERMUTATION4(permutate_high, vus, _mm_shufflehi_epi16);
DEFINE_OP_PERMUTATION4(permutate_high, vcus, _mm_shufflehi_epi16);
// Permutate 4 elements in a vector
// The definitions of template parameters are similar to permutate_low
DEFINE_OP_PERMUTATION4(permutate, vcs, _mm_shuffle_epi32);
DEFINE_OP_PERMUTATION4(permutate, vi, _mm_shuffle_epi32);
DEFINE_OP_PERMUTATION4(permutate, vui, _mm_shuffle_epi32);

// Permutate 2 elements in a vector
// Template parameter a0 ~ a1: selects the contents of one element location in the destination operand. ie.
// r[0] := a[a0]
// r[1] := a[a1]
template<int a0, int a1>
DSP_INLINE vci permutate(vci &a) { vci c; c = (vci&)_mm_shuffle_pd((__m128d&)a, (__m128d&)a, (a0 | (a1 << 1))); return c; }
template<int a0, int a1>
DSP_INLINE vq permutate(vq &a) { vq c; c = (vq&)_mm_shuffle_pd((__m128d&)a, (__m128d&)a, (a0 | (a1 << 1))); return c; }

// Permutate 16 byte-element in a 128-bit vector
// The return value can be expressed by the following equations:
//   r0 = (mask0 & 0x80) ? 0 : SELECT(a, mask0 & 0x0f)
//   r1 = (mask1 & 0x80) ? 0 : SELECT(a, mask1 & 0x0f)
//   ...
//   r15 = (mask15 & 0x80) ? 0 : SELECT(a, mask15 & 0x0f) 
//
// r0-r15 and mask0-mask15 are the sequentially ordered 8-bit components of return value r and parameter mask.
// r0 and mask0 are the least significant 8 bits.
// SELECT(a, n) extracts the nth 8-bit parameter from a. The 0th 8-bit parameter is the least significant 8-bits.
// mask provides the mapping of bytes from parameter a to bytes in the result. If the byte in mask has its highest
// bit set, the corresponding byte in the result will be set to zero.
DEFINE_OP_ARITHMETIC2(permutate16, vb, _mm_shuffle_epi8);
DEFINE_OP_ARITHMETIC2(permutate16, vub, _mm_shuffle_epi8);

// Get sum of 4 elements at the same index in each operand and comprise the returned vector by them
DEFINE_OP_REDUCE4(hadd4, vcs, add);
DEFINE_OP_REDUCE4(hadd4, vi,  add);
__declspec(deprecated("This function is deprecated and will be removed in future version. Consider using 'hadd4' instead."))
DEFINE_OP_REDUCE4(reduce4_add, vcs, add);
__declspec(deprecated("This function is deprecated and will be removed in future version. Consider using 'hadd4' instead."))
DEFINE_OP_REDUCE4(reduce4_add, vi,  add);

// Get saturated sum of 4 elements at the same index in each operand and comprise the returned vector by them
DEFINE_OP_REDUCE4(saturated_hadd4, vcs, saturated_add);
__declspec(deprecated("This function is deprecated and will be removed in future version. Consider using 'saturated_hadd4' instead."))
DEFINE_OP_REDUCE4(reduce4_saturated_add, vcs, saturated_add);
// Note: there is no intrinsic _mm_subs_epu32, so no reduce4_saturated_add on vi

// Compute saturated sum of all 4 components in a vector128 type,
// and duplicate the final value to all elements in the returned vector
DEFINE_OP_DUPLICATION4_OPERATION(saturated_hadd,       vcs, saturated_add);
__declspec(deprecated("This function is deprecated and will be removed in future version. Consider using 'saturated_hadd' instead."))
DEFINE_OP_DUPLICATION4_OPERATION(reduce_saturated_add, vcs, saturated_add);

// Compute sum of all 2 components in a vector128 type,
// and duplicate the final value to all elements in the returned vector
__forceinline vci hadd(const vci& a)
{ 
    vci temp, sum;
    temp = (vci)permutate<2, 3, 0, 1>((vi&)a);
    sum  = add(a, temp);
    return sum;
}
// Compute sum of all 4 components in a vector128 type,
// and duplicate the final value to all elements in the returned vector
DEFINE_OP_DUPLICATION4_OPERATION(hadd,       vcs, add);
DEFINE_OP_DUPLICATION4_OPERATION(hadd,       vi,  add);
DEFINE_OP_DUPLICATION4_OPERATION(hadd,       vui, add);
__declspec(deprecated("This function is deprecated and will be removed in future version. Consider using 'hadd' instead."))
DEFINE_OP_DUPLICATION4_OPERATION(reduce_add, vcs, add);
__declspec(deprecated("This function is deprecated and will be removed in future version. Consider using 'hadd' instead."))
DEFINE_OP_DUPLICATION4_OPERATION(reduce_add, vi,  add);
__declspec(deprecated("This function is deprecated and will be removed in future version. Consider using 'hadd' instead."))
DEFINE_OP_DUPLICATION4_OPERATION(reduce_add, vui, add);

namespace vector128 { namespace details {
// Note: Private functions are defined in embedded namespace

template<typename T>
DSP_INLINE T signmask(const T& a, typename vector128::details::traits<T>::tag * = 0)
{
    return is_less(a, vector128_consts::zero<T>());
}

#ifdef __SSE4__
// Note: _mm_srli_si128 expects const shift parameter, so template is introduced
// Warning: semantics changed from original v_ns2i, v_ni2q: nbytes
template<int nbytes>
DSP_INLINE vs shift_unpack_low(const vb& a) { vs r; r = _mm_cvtepi8_epi16(_mm_srli_si128(a, nbytes)); return r; }
template<int nbytes>
DSP_INLINE vi shift_unpack_low(const vs& a) { vi r; r = _mm_cvtepi16_epi32(_mm_srli_si128(a, nbytes)); return r; }
template<int nbytes>
DSP_INLINE vq shift_unpack_low(const vi& a) { vq r; r = _mm_cvtepi32_epi64(_mm_srli_si128(a, nbytes)); return r; }
#endif

} }

// Assign the same value to all elements in a vector
DSP_INLINE void set_all(vb& x, __int8 a) { x = (vb)_mm_set1_epi8(a); }
DSP_INLINE void set_all(vs& x, __int16 a) { x = (vs)_mm_set1_epi16(a); }
DSP_INLINE void set_all(vi& x, __int32 a) { x = (vi)_mm_set1_epi32(a); }
DSP_INLINE void set_all(vcs& x, COMPLEX16 a) { x = (vcs)_mm_set1_epi32(*reinterpret_cast<__int32*>(&a)); }
#ifdef USER_MODE
DSP_INLINE void set_all(vf& x, float a) { x = (vf)_mm_set1_ps(a); }
#endif
DSP_INLINE void set_all(vq& x, __int64 a)
{
#if (SIZE_MAX == _UI64_MAX)
    // _mm_set1_epi64x only available on AMD64 platform
    x = (vq)_mm_set1_epi64x(a);
#elif (SIZE_MAX == _UI32_MAX)
    // __m64 and _mm_set1_epi64 only available on IX86 platform
    __m64 t;
    t.m64_i64 = a;
    x = (vq)_mm_set1_epi64(t);
    // _mm_empty to suppresss warning C4799
    _mm_empty();
#else
#error The target platform is neither AMD64 or X86, not supported!
#endif
}

// Element-wise arithmetic left shift
DEFINE_OP_SHIFT_LEFT(shift_left, vs, _mm_slli_epi16);
DEFINE_OP_SHIFT_LEFT(shift_left, vi, _mm_slli_epi32);
DEFINE_OP_SHIFT_LEFT(shift_left, vq, _mm_slli_epi64);
DEFINE_OP_SHIFT_LEFT(shift_left, vcs, _mm_slli_epi16);
DEFINE_OP_SHIFT_LEFT(shift_left, vci, _mm_slli_epi32);
DEFINE_OP_SHIFT_LEFT(shift_left, vcq, _mm_slli_epi64);
DEFINE_OP_SHIFT_LEFT(shift_left, vus, _mm_slli_epi16);
DEFINE_OP_SHIFT_LEFT(shift_left, vui, _mm_slli_epi32);
DEFINE_OP_SHIFT_LEFT(shift_left, vuq, _mm_slli_epi64);
DEFINE_OP_SHIFT_LEFT(shift_left, vcus, _mm_slli_epi16);
DEFINE_OP_SHIFT_LEFT(shift_left, vcui, _mm_slli_epi32);
DEFINE_OP_SHIFT_LEFT(shift_left, vcuq, _mm_slli_epi64);

// Element-wise arithmetic right shift
DEFINE_OP_SHIFT_RIGHT(shift_right, vs,   _mm_srai_epi16);
DEFINE_OP_SHIFT_RIGHT(shift_right, vi,   _mm_srai_epi32);
//DEFINE_OP_SHIFT_RIGHT(shift_right, vq,   _mm_srai_epi64); // Not exist instruction
DEFINE_OP_SHIFT_RIGHT(shift_right, vcs,  _mm_srai_epi16);
DEFINE_OP_SHIFT_RIGHT(shift_right, vci,  _mm_srai_epi32);
//DEFINE_OP_SHIFT_RIGHT(shift_right, vcq,  _mm_srai_epi64); // Not exist instruction
DEFINE_OP_SHIFT_RIGHT(shift_right, vus,  _mm_srli_epi16);
DEFINE_OP_SHIFT_RIGHT(shift_right, vui,  _mm_srli_epi32);
DEFINE_OP_SHIFT_RIGHT(shift_right, vuq,  _mm_srli_epi64);
DEFINE_OP_SHIFT_RIGHT(shift_right, vcus, _mm_srli_epi16);
DEFINE_OP_SHIFT_RIGHT(shift_right, vcui, _mm_srli_epi32);
DEFINE_OP_SHIFT_RIGHT(shift_right, vcuq, _mm_srli_epi64);
DSP_INLINE vub shift_right(const vub& a, int nbits)
{
    const static vub::data_type zero_end = {
        0xFE, 0xFE, 0xFE, 0xFE, 
        0xFE, 0xFE, 0xFE, 0xFE,
        0xFE, 0xFE, 0xFE, 0xFE, 
        0xFE, 0xFE, 0xFE, 0xFE,
    };
    vub rub0 = and (a, vub(zero_end));
    return (vub)_mm_srli_epi16(rub0, nbits);
}
//DEFINE_OP_SHIFT(shift_right, vq, _mm_srai_epi64);    // intrinsic N/A

// Shift the whole vector left by specified elements
template<int n, typename T>
DSP_INLINE T shift_element_left (T& a) {
	return (T) _mm_slli_si128 (a, n * sizeof(T::elem_type) );
}

// Shift the whole vector right by specified elements while shifting in zeros
template<typename T, int n>
DSP_INLINE T shift_element_right (T& a) {
	return (T) _mm_srli_si128 (a, n * sizeof(T::elem_type) );
}

// Saturated packs the 2 source vectors into one. The elements in resulting vector has half
// length of the source elements.
DSP_INLINE vs saturated_pack(const vi& a, const vi& b) { return (vs)_mm_packs_epi32(a, b); }
DSP_INLINE vcs saturated_pack(const vci& a, const vci& b) { return (vcs)_mm_packs_epi32(a, b); }
DSP_INLINE vb saturated_pack(const vs& a, const vs& b) { return (vb)_mm_packs_epi16(a, b); }
DSP_INLINE vcb saturated_pack(const vcs& a, const vcs& b) { return (vcb)_mm_packs_epi16(a, b); }

// Pack elements in 2 source vectors into returned vector
// Note:
//    1. each element will be packed to a half-length field, eg. __int32 --> __int16
DSP_INLINE vs pack(const vi& a, const vi& b)
{
    const vi xmm6 = vector128_consts::__0x0000FFFF0000FFFF0000FFFF0000FFFF<vi>();
    vi ta = and(a, xmm6);
    vi tb = and(b, xmm6);
    tb = shift_left(tb, 0x10);
    return (vs)or(ta, tb);
}

// obtain a packed complex vector through two integer vector holds RE and IM values
DSP_INLINE 
void pack(vcs& r, const vi& re, const vi& im)
{	
	r = (vcs) pack(re, im);
}




// Unpack elements in source vector to 2 destination vectors
// Note:
//    1. each element will be unpacked to a double-length field, eg. __int8 --> __int16
//    2. r1 got the unpacked elements in lower half of source vector, r2 got elements in high half
template<typename TO, typename T>
DSP_INLINE void unpack(TO& r1, TO& r2, const T& a)
{
#ifdef __SSE4__
    r1 = (TO)vector128::details::shift_unpack_low<0>(a);
    r2 = (TO)vector128::details::shift_unpack_low<8>(a);
#else
    typedef typename vector128::details::CrackComplexType<T>::type TC;
    TC s = vector128::details::signmask((TC&)a);
    r1 = (TO)interleave_low((TC&)a, s);
    r2 = (TO)interleave_high((TC&)a, s);
#endif
}

// Add pair-wisely of element-wise multiplication product
// ie. (vs, vs) --> vi
//     r0 := (a0 * b0) + (a1 * b1)
//     r1 := (a2 * b2) + (a3 * b3)
//     r2 := (a4 * b4) + (a5 * b5)
//     r3 := (a6 * b6) + (a7 * b7)
DSP_INLINE vi pairwise_muladd(const vs& a, const vs& b) { return (vi)_mm_madd_epi16(a, b); }
DSP_INLINE vi muladd(const vcs& a, const vcs& b) { return (vi)_mm_madd_epi16(a, b); }
DSP_INLINE vq muladd(const vci& a, const vci& b)
{
    vq q0, q1, c;
    q0 = _mm_mul_epi32(a, b);
    q1 = _mm_mul_epi32(shift_right((vuq&)a, 32), shift_right((vuq&)b, 32));
    c  = add(q0, q1);
    return c;
}

// Compute the squared norm of a complex vector
DSP_INLINE vi SquaredNorm(const vcs& a) { return muladd(a, a); }
DSP_INLINE vq SquaredNorm(const vci& a) { return muladd(a, a); }

// Element-wise multiplication, leaving only lower half part product
DEFINE_OP_ARITHMETIC2(mul_low, vs, _mm_mullo_epi16);
DEFINE_OP_ARITHMETIC2(mul_low, vi, _mm_mullo_epi32);

// Element-wise multiplication, leaving only higher half part product
DEFINE_OP_ARITHMETIC2(mul_high, vs, _mm_mulhi_epi16);

// Comprise a real number vector and an imaginary vector into 2 complex vectors.
// r1 got the comprised complex numbers by comprising the low half of real and imaginary vectors,
// and r2 got high half.
DSP_INLINE void comprise(vci& r1, vci& r2, const vi& re, const vi& im)
{
    r1 = (vci)interleave_low(re, im);
    r2 = (vci)interleave_high(re, im);
}

// Negative real part of each complex numbers
DSP_INLINE vcs conjre(const vcs& a)
{ 
    const static vub::data_type value =
    {
        0x00, 0x80, 0x01, 0x00,
        0x00, 0x80, 0x01, 0x00,
        0x00, 0x80, 0x01, 0x00,
        0x00, 0x80, 0x01, 0x00,
    };
    return sign(a, (vcs&)value);
}

// Compute approximate conjugate of each complex numbers in a vector, using xor to implement subtraction.
// This operation is used for better performance than the accurate one.
DSP_INLINE vcs conj(const vcs& a) { return xor(a, vector128_consts::__0xFFFF0000FFFF0000FFFF0000FFFF0000<vcs>()); }

// Compute accurate conjugate of each complex numbers in a vector
DSP_INLINE vcs conj0(const vcs& a)
{
    const static vub::data_type value =
    {
        0x01, 0x00, 0x00, 0x80,
        0x01, 0x00, 0x00, 0x80,
        0x01, 0x00, 0x00, 0x80,
        0x01, 0x00, 0x00, 0x80,
    };
    return sign(a, (vcs&)value);
}

// Swap real and image part of each complex number
DSP_INLINE vcs flip(const vcs& a)
{
    vcs b = permutate_low<0xb1>(a);
    return permutate_high<0xb1>(b);
}
DSP_INLINE vcus flip(const vcus& a)
{
    return (vcus)flip((vcs&)a);
}
DSP_INLINE vci flip(const vci& a)
{
    return (vci)permutate<1, 0, 3, 2>((vi&)a);
}

// Multiply the first source vector by the conjugate of the second source vector
// ie. re + j * im = a * conj(b)
// Returns:
//   1. re got all real parts of products, im got all imaginary parts
//   2. OR, low got all the lower elements of products, high got all the higher
//   3. elements of the resulting product has double-length elements compared to input vector
DSP_INLINE void conj_mul(vi& re, vi& im, const vcs& a, const vcs& b)
{
    vcs vs1 = flip(b);
    vs1 = conjre(vs1);
    re = pairwise_muladd((vs&)a, (vs&)b);
    im = pairwise_muladd((vs&)vs1, (vs&)a);
}
DSP_INLINE void conj_mul(vci& low, vci& high, const vcs& a, const vcs& b)
{
    vi re, im;
    conj_mul(re, im, a, b);
    low   = (vci)interleave_low(re, im);
    high  = (vci)interleave_high(re, im);
}
DSP_INLINE void conj_mul(vq& re, vq& im, const vci& a, const vci& b)
{
    vq temp1, temp2, temp3;
    temp3 = flip(a);
    temp1 = _mm_mul_epi32(temp3, b);
    temp2 = _mm_mul_epi32(shift_right((vuq&)temp3, 32), shift_right((vuq&)b, 32));
    im    = sub(temp1, temp2);
    re    = muladd(a, b);
}
DSP_INLINE void conj_mul(vcq& low, vcq& high, const vci& a, const vci& b)
{
    vq re, im;
    conj_mul(re, im, a, b);
    low   = (vcq)interleave_low(re, im);
    high  = (vcq)interleave_high(re, im);
}

// Multiply the first source vector by the second source vector
// ie. re + j * im = a * b
// Returns:
//   1. re got all real parts of products, im got all imaginary parts
//   2. OR, low got all the lower elements of products, high got all the higher
//   3. elements of the resulting product has double-length elements compared to input vector
DSP_INLINE void mul (vi& re, vi& im, const vcs& a, const vcs& b)
{
	vcs vs1 = conj0(b);
    vcs vs2 = flip(b);
	re = muladd(a, vs1);
	im = muladd(a, vs2);
}
DSP_INLINE void mul(vci& low, vci& high, const vcs& a, const vcs& b)
{ 
    vi re, im;
    mul(re, im, a, b);
    low   = interleave_low(re, im);
    high  = interleave_high(re, im);
}
DSP_INLINE void mul(vq& low, vq& high, const vi& a, const vi& b)
{
    vq q13, q24;
    q13 = _mm_mul_epi32(a, b);
    q24 = _mm_mul_epi32(shift_right((vuq&)a, 32), shift_right((vuq&)b, 32));
    low = interleave_low(q13, q24); 
    high = interleave_high(q13, q24);
}

//
// mul - perform multiple of two complex vectors and return
// the resulting complex vector
//
DSP_INLINE 
vcs mul (const vcs& a, const vcs& b)
{
	vi re, im;
	mul (re, im, a, b );
	
    re = shift_right (re, 15);
    im = shift_right (im, 15);

	return (vcs) pack(re, im);
}

// Multiply the first source by the conjugate of the second source, leaving low part product after right-shifting
// ie. return a * conj(b) >> nbits_right
DSP_INLINE vcs conj_mul_shift(const vcs& a, const vcs& b, int nbits_right)
{
    // b = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    // vcs1 = -I3 Q3 -I2 Q2 -I1 Q1 -I0 Q0
    vcs vcs1 = conjre(a);
    vcs1 = flip(vcs1);
    
    vi vi0 = pairwise_muladd((vs&)a, (vs&)b);       // vi0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    vi vi1 = pairwise_muladd((vs&)vcs1, (vs&)b);    // vi1 = Q3 Q2 Q1 Q0

    // Shift right to normalize
    vi0 = shift_right(vi0, nbits_right);
    vi1 = shift_right(vi1, nbits_right);

    // Q3 I3 Q2 I2 Q1 I1 Q0 I0
    return (vcs)pack(vi0, vi1);
}

// Multiply and leave low part product after right-shifting
// ie. return a * b >> nbits_right
DSP_INLINE vcs mul_shift(const vcs& a, const vcs& b, int nbits_right)
{
    vi vi0 = pairwise_muladd((vs&)a, (vs)conj(b));
    vi vi1 = pairwise_muladd((vs&)a, (vs)flip(b));

    // Shift right to normalize
    vi0 = shift_right(vi0, nbits_right);
    vi1 = shift_right(vi1, nbits_right);

    // Q3 I3 Q2 I2 Q1 I1 Q0 I0
    return (vcs)pack(vi0, vi1);
}

// Multiply leaving only right-shifted low part product
// ie. return a * b >> nbits_right
template<int nbits_right> DSP_INLINE vs mul_shift(const vs& a, const vs& b);
template<>
DSP_INLINE vs mul_shift<15>(const vs& a, const vs& b)
{
    return (vs)_mm_mulhrs_epi16(a, b);
}

// Approximately multiply by imaginary unit
DSP_INLINE vcs mul_j(const vcs& a)
{
    return xor(flip(a), vector128_consts::__0x0000FFFF0000FFFF0000FFFF0000FFFF<vcs>());
}

// Multiply with imaginary unit
DSP_INLINE vcs mul_j0(const vcs& a)
{
    vcs vmask = vector128_consts::__0x0000FFFF0000FFFF0000FFFF0000FFFF<vcs>();
    vcs ret   = xor(flip(a), vmask);
    return sub(ret, vmask);
}

// Approximately compute the element-wise absolute value of a vector, using xor to implement subtraction
// Note: tag is used to allow the function template specialization to only vector128 types
template<typename T>
DSP_INLINE T abs(const T& a, typename vector128::details::traits<T>::tag * = 0)
{
    return xor(a, shift_right(a, sizeof(a[0])*8-1));
}

// Accurate element-wise absolute value of a vector
DEFINE_OP_ARITHMETIC1(abs0, vb, _mm_abs_epi8);
DEFINE_OP_ARITHMETIC1(abs0, vs, _mm_abs_epi16);
DEFINE_OP_ARITHMETIC1(abs0, vi, _mm_abs_epi32);
DEFINE_OP_ARITHMETIC1(abs0, vq, _mm_abs_epi64);

// Compute element-wise minimal
// Note: don't rename to "min", since defined as a macro in <WinDef.h>
DEFINE_OP_ARITHMETIC2(smin, vs, _mm_min_epi16);
DEFINE_OP_ARITHMETIC2(smin, vcs, _mm_min_epi16);
DEFINE_OP_ARITHMETIC2(smin, vub, _mm_min_epu8);
DEFINE_OP_ARITHMETIC2(smin, vcub, _mm_min_epu8);
// Note: below fuctions are disabled since underlying intrinsics are in SSE4, and hard to rewrite with old intrinsics set
//DEFINE_OP_ARITHMETIC2(smin, vci, _mm_min_epi32);
//DEFINE_OP_ARITHMETIC2(smin, vcui, _mm_min_epu32);
//DEFINE_OP_ARITHMETIC2(smin, vui, _mm_min_epu32);
//DEFINE_OP_ARITHMETIC2(smin, vi, _mm_min_epi32);
#ifdef __SSE4__
DEFINE_OP_ARITHMETIC2(smin, vb, _mm_min_epi8);
DEFINE_OP_ARITHMETIC2(smin, vcb, _mm_min_epi8);
DEFINE_OP_ARITHMETIC2(smin, vus, _mm_min_epu16);
DEFINE_OP_ARITHMETIC2(smin, vcus, _mm_min_epu16);
#else
DEFINE_OP_MINMAX_SIGNED_UNSIGNED(smin, vb, vub);
DEFINE_OP_MINMAX_SIGNED_UNSIGNED(smin, vcb, vub);
DEFINE_OP_MINMAX_SIGNED_UNSIGNED(smin, vus, vs);
DEFINE_OP_MINMAX_SIGNED_UNSIGNED(smin, vcus, vs);
#endif

// Compute element-wise maximal
// Note: don't rename to max, since defined as macro in <WinDef.h>
DEFINE_OP_ARITHMETIC2(smax, vs, _mm_max_epi16);
DEFINE_OP_ARITHMETIC2(smax, vcs, _mm_max_epi16);
DEFINE_OP_ARITHMETIC2(smax, vub, _mm_max_epu8);
DEFINE_OP_ARITHMETIC2(smax, vcub, _mm_max_epu8);
// Note: below fuctions are disabled since underlying intrinsics are in SSE4, and hard to rewrite with old intrinsics set
//DEFINE_OP_ARITHMETIC2(smax, vui, _mm_max_epu32);
//DEFINE_OP_ARITHMETIC2(smax, vcui, _mm_max_epu32);
//DEFINE_OP_ARITHMETIC2(smax, vi, _mm_max_epi32);
//DEFINE_OP_ARITHMETIC2(smax, vci, _mm_max_epi32);
#ifdef __SSE4__
DEFINE_OP_ARITHMETIC2(smax, vb, _mm_max_epi8);
DEFINE_OP_ARITHMETIC2(smax, vcb, _mm_max_epi8);
DEFINE_OP_ARITHMETIC2(smax, vus, _mm_max_epu16);
DEFINE_OP_ARITHMETIC2(smax, vcus, _mm_max_epu16);
#else
DEFINE_OP_MINMAX_SIGNED_UNSIGNED(smax, vb, vub);
DEFINE_OP_MINMAX_SIGNED_UNSIGNED(smax, vcb, vub);
DEFINE_OP_MINMAX_SIGNED_UNSIGNED(smax, vus, vs);
DEFINE_OP_MINMAX_SIGNED_UNSIGNED(smax, vcus, vs);
#endif

// Duplicate the minimal element in the source vector to all elements of a vector128 type
DEFINE_OP_DUPLICATION16_OPERATION(hmin, vb, smin);
DEFINE_OP_DUPLICATION16_OPERATION(hmin, vub, smin);
DEFINE_OP_DUPLICATION8_OPERATION(hmin, vs, smin);
DEFINE_OP_DUPLICATION8_OPERATION(hmin, vus, smin);

// Duplicate the maximal element in the source vector to all elements of a vector128 type
DEFINE_OP_DUPLICATION16_OPERATION(hmax, vb, smax);
DEFINE_OP_DUPLICATION16_OPERATION(hmax, vub, smax);
DEFINE_OP_DUPLICATION8_OPERATION(hmax, vs, smax);
DEFINE_OP_DUPLICATION8_OPERATION(hmax, vus, smax);

// Load 128-bit vector from the address p, which is not necessarily 16-byte aligned
template <typename T> 
void DSP_INLINE load (T& r, const void* p) { r = _mm_loadu_si128 ((const __m128i *)p); }


// Stores 128-bit vector to the address p without polluting the caches
template <typename T> void DSP_INLINE store_nt(T *p, const T& a) { _mm_stream_si128((__m128i *)p, a); }

// Stores 128-bit vector to the address p, which not necessarily 16-byte aligned
template <typename T> void DSP_INLINE store(void *p, const T& a) { _mm_storeu_si128((__m128i *)p, a); }

DSP_INLINE int move_mask (const vb& a) { return _mm_movemask_epi8 (a); }
DSP_INLINE int move_mask ( const vs& a ) {
	vb v = saturated_pack (a, a);
	return move_mask (v);
}	
DSP_INLINE int move_mask ( const vs& a, const vs& b ) {
	vb v = saturated_pack (a, b);
	return move_mask (v);
}	
DSP_INLINE int move_mask ( const vi& a ) {
	vs v = saturated_pack (a, a);
	return move_mask ( v );		
}
DSP_INLINE int move_mask ( const vi& a, const vi& b ) {
	vs v = saturated_pack (a, b);
	return move_mask ( v );		
}
DSP_INLINE int move_mask ( const vi& a, const vi& b, const vi& c, const vi& d ) {
	vs v0 = saturated_pack (a, b);
	vs v1 = saturated_pack (c, d);
	return move_mask ( v0, v1 );		
}

// some utilities
template<typename T>
DSP_INLINE
T& cast_ref ( void* pointer ) {
	return *(reinterpret_cast<T*> (pointer));
}
template<typename T>
DSP_INLINE
const T& cast_ref ( const void* pointer ) {
	return *(reinterpret_cast<const T*> (pointer));
}

#define DEFINE_OP_INSERT(OP, T, INSTRINSIC, ET)  \
template<int ndx>								\
DSP_INLINE T OP(const T& a, T::elem_type b)     \
    	{ return (T)INSTRINSIC(a, cast_ref<ET> (&b), ndx); }


// Inserts an element into a vector at the index position ¡°ndx¡±.
// This can be expressed with the following equations:
//   r0 := (ndx == 0) ? b : a0
//   r1 := (ndx == 1) ? b : a1
//   ...
//   r0... and a0... are the sequentially ordered elements of return value r and parameter a. r0 and a0 are the least significant bits.
DEFINE_OP_INSERT(insert, vcs, _mm_insert_epi32, UINT) 
DEFINE_OP_INSERT(insert, vi,  _mm_insert_epi32, int) 
DEFINE_OP_INSERT(insert, vs,  _mm_insert_epi16, short)

// opeartors
#define DEFINE_OPERATOR_OVERLOAD(OP,TYPE,ACTION) \
	__forceinline TYPE operator OP (const TYPE& o1, const TYPE& o2) {\
	return ACTION(o1,o2); }

DEFINE_OPERATOR_OVERLOAD(+, vcs, add)
DEFINE_OPERATOR_OVERLOAD(+, vs, add)	
DEFINE_OPERATOR_OVERLOAD(+, vi, add)

DEFINE_OPERATOR_OVERLOAD(-, vcs, sub)
DEFINE_OPERATOR_OVERLOAD(-, vs, sub)	
DEFINE_OPERATOR_OVERLOAD(-, vi, sub)

DEFINE_OPERATOR_OVERLOAD(*, vcs, mul)
DEFINE_OPERATOR_OVERLOAD(*, vs,  mul_low)	
DEFINE_OPERATOR_OVERLOAD(*, vi,  mul_low)

#define DEFINE_OPERATOR_OVERLOAD2(OP,TYPE,ACTION) \
	__forceinline TYPE& operator OP (TYPE& o1, const TYPE& o2) {\
	o1 = ACTION(o1,o2); return o1; }

DEFINE_OPERATOR_OVERLOAD2(+=, vcs, add)
DEFINE_OPERATOR_OVERLOAD2(+=, vci, add)
DEFINE_OPERATOR_OVERLOAD2(+=, vs, add)	
DEFINE_OPERATOR_OVERLOAD2(+=, vi, add)

DEFINE_OPERATOR_OVERLOAD2(-=, vcs, sub)
DEFINE_OPERATOR_OVERLOAD2(-=, vci, sub)
DEFINE_OPERATOR_OVERLOAD2(-=, vs, sub)	
DEFINE_OPERATOR_OVERLOAD2(-=, vi, sub)

DEFINE_OPERATOR_OVERLOAD2(*=, vcs, mul)
DEFINE_OPERATOR_OVERLOAD2(*=, vs, mul_low)	
DEFINE_OPERATOR_OVERLOAD2(*=, vi, mul_low)

#define DEFINE_OPERATOR_OVERLOAD3(OP,TYPE,ACTION) \
	__forceinline TYPE& operator OP (TYPE& o1, int o2) {\
	o1 = ACTION(o1,o2); return o1; }

DEFINE_OPERATOR_OVERLOAD3(>>=, vcs, shift_right)
DEFINE_OPERATOR_OVERLOAD3(>>=, vs, shift_right)	
DEFINE_OPERATOR_OVERLOAD3(>>=, vi, shift_right)

DEFINE_OPERATOR_OVERLOAD3(<<=, vcs, shift_left)
DEFINE_OPERATOR_OVERLOAD3(<<=, vs,  shift_left)	
DEFINE_OPERATOR_OVERLOAD3(<<=, vi,  shift_left)


template<typename T>
DSP_INLINE 
void vmemcpynt (T * pdst, const T * psrc, int vlen ) {
	while ( vlen>0 ) {
		* pdst = * psrc;
		store_nt<T> (pdst, *psrc);
		pdst++; psrc++; vlen --;
	}
}

template<bool = true> struct true_wrapper;

// Rep utility
template<int N, typename = true_wrapper<> >
class rep {
public:
   // Copy consequent N elements in psrc buffer to pdst buffer
	template<typename T>
	static DSP_INLINE 
	void vmemcpy (T * pdst, const T * psrc ) {
		* pdst = * psrc;
		 pdst ++; psrc ++;
		 rep<N-1>::vmemcpy (pdst, psrc);
	};

	template<typename T>
	static DSP_INLINE 
	void vmemzero (T * pdst ) {
		 set_zero (*pdst);
		 pdst ++; 
		 rep<N-1>::vmemzero (pdst);
	};

	// mul consequent N elements in psrc and in coeff, and store the results to pdst
	template<typename T>	
	static DSP_INLINE 
	void vmul (T* pdst, const T * psrc, const T * pcoeffs ) {
		 *pdst = (*psrc) * (*pcoeffs); 
		 psrc ++; pdst ++; pcoeffs++;
		 rep<N-1>::vmul (pdst, psrc, pcoeffs);
	}

	static DSP_INLINE
	void vor (unsigned long* pdst, const unsigned long* psrc) {
		* pdst |= * psrc;
		 pdst ++; psrc ++;
		 rep<N-1>::vor (pdst, psrc);
	}
	
    // Shift consequent N elements in psrc buffer to the right by "nbits" bits, and store to original buffer separately
	template<typename T>	
	static DSP_INLINE 
	void vshift_right (T * psrc, int nbits ) {
		 *psrc = (*psrc) >> nbits; 
		 psrc ++;
		 rep<N-1>::vshift_right (psrc, nbits);
	}

    // Shift consequent N elements in psrc buffer to the left by "nbits" bits, and store to original buffer separately
	template<typename T>	
	static DSP_INLINE 
	void vshift_left (T * psrc, int nbits ) {
		 *psrc = (*psrc) << nbits; 
		 psrc ++;
		 rep<N-1>::vshift_left (psrc, nbits);
	}

	// Compute the squre norm of consequent N elements in psrc buffer, and store to pdst buffer separately
	static DSP_INLINE
	void vsqrnorm (vi * pdst, const vcs * psrc ) {
		* pdst = SquaredNorm (* psrc);
		 pdst ++; psrc ++;
		 rep<N-1>::vsqrnorm (pdst, psrc);
	};

	// substract consequent N elements in psrc buffer by val, and store to pdst buffer
	template<typename T>	
	static DSP_INLINE 
	void vsub (T* pdst, const T * psrc, const T& val ) {
		 *pdst = (*psrc) - val; 
		 psrc ++; pdst ++;
		 rep<N-1>::vsub (pdst, psrc, val);
	}

    // Compute the sum of consequent N elements in psrc buffer, and store to the variable r
	template<typename T>	
	static DSP_INLINE 
	void vsum (T& r, const T * psrc ) {
		 r += (*psrc); 
		 psrc ++;
		 rep<N-1>::vsum (r, psrc);
	}

	static DSP_INLINE 
	void vdiv (int * pdst, const int* psrc, const int* pdiv ) {
		*pdst = (*psrc) / (*pdiv);
		pdst ++; psrc++; pdiv++;
		rep<N-1>::vdiv (pdst, psrc, pdiv);
	}
};

// Template partial specialization for large N instances,
// use loop-unrolling on a burst of operations instead of recursive unrolling,
// in order to speed up compilation.
const int REP_BURST = 16;
template<int N>
class rep<N, true_wrapper< (N>REP_BURST) > > {
public:
    // Copy consequent N elements in psrc buffer to pdst buffer
	template<typename T>
	static DSP_INLINE 
	void vmemcpy (T * pdst, const T * psrc ) {
        const int imax = N / REP_BURST * REP_BURST;
        int i;
        for (i = 0; i < imax; i += REP_BURST)
            rep<REP_BURST>::vmemcpy(&pdst[i], &psrc[i]);
        rep<N - imax>::vmemcpy(&pdst[i], &psrc[i]);
	};

	template<typename T>
	static DSP_INLINE 
	void vmemzero (T * pdst ) {
        const int imax = N / REP_BURST * REP_BURST;
        int i;
        for (i = 0; i < imax; i += REP_BURST)
            rep<REP_BURST>::vmemzero(&pdst[i]);
        rep<N - imax>::vmemzero(&pdst[i]);
	};

	// mul consequent N elements in psrc and in coeff, and store the results to pdst
	template<typename T>	
	static DSP_INLINE 
	void vmul (T* pdst, const T * psrc, const T * pcoeffs ) {
        const int imax = N / REP_BURST * REP_BURST;
        int i;
        for (i = 0; i < imax; i += REP_BURST)
            rep<REP_BURST>::vmul(&pdst[i], &psrc[i], &pcoeffs[i]);
        rep<N - imax>::vmul(&pdst[i], &psrc[i], &pcoeffs[i]);
	}

	static DSP_INLINE
	void vor (unsigned long* pdst, const unsigned long* psrc) {
        const int imax = N / REP_BURST * REP_BURST;
        int i;
        for (i = 0; i < imax; i += REP_BURST)
            rep<REP_BURST>::vor(&pdst[i], &psrc[i]);
        rep<N - imax>::vor(&pdst[i], &psrc[i]);
	}
	
    // Shift consequent N elements in psrc buffer to the right by "nbits" bits, and store to original buffer separately
	template<typename T>	
	static DSP_INLINE 
	void vshift_right (T * psrc, int nbits ) {
        const int imax = N / REP_BURST * REP_BURST;
        int i;
        for (i = 0; i < imax; i += REP_BURST)
            rep<REP_BURST>::vshift_right(&psrc[i], nbits);
        rep<N - imax>::vshift_right(&psrc[i], nbits);
	}

    // Shift consequent N elements in psrc buffer to the left by "nbits" bits, and store to original buffer separately
	template<typename T>	
	static DSP_INLINE 
	void vshift_left (T * psrc, int nbits ) {
        const int imax = N / REP_BURST * REP_BURST;
        int i;
        for (i = 0; i < imax; i += REP_BURST)
            rep<REP_BURST>::vshift_left(&psrc[i], nbits);
        rep<N - imax>::vshift_left(&psrc[i], nbits);
	}

	// Compute the squre norm of consequent N elements in psrc buffer, and store to pdst buffer separately
	static DSP_INLINE
	void vsqrnorm (vi * pdst, const vcs * psrc ) {
        const int imax = N / REP_BURST * REP_BURST;
        int i;
        for (i = 0; i < imax; i += REP_BURST)
            rep<REP_BURST>::vsqrnorm(&pdst[i], &psrc[i]);
        rep<N - imax>::vsqrnorm(&pdst[i], &psrc[i]);
	};

	// substract consequent N elements in psrc buffer by val, and store to pdst buffer
	template<typename T>	
	static DSP_INLINE 
	void vsub (T* pdst, const T * psrc, const T& val ) {
        const int imax = N / REP_BURST * REP_BURST;
        int i;
        for (i = 0; i < imax; i += REP_BURST)
            rep<REP_BURST>::vsub(&pdst[i], &psrc[i], val);
        rep<N - imax>::vsub(&pdst[i], &psrc[i], val);
	}

    // Compute the sum of consequent N elements in psrc buffer, and store to the variable r
	template<typename T>	
	static DSP_INLINE 
	void vsum (T& r, const T * psrc ) {
        const int imax = N / REP_BURST * REP_BURST;
        int i;
        for (i = 0; i < imax; i += REP_BURST)
            rep<REP_BURST>::vsum(r, &psrc[i]);
        rep<N - imax>::vsum(r, &psrc[i]);
	}

	static DSP_INLINE 
	void vdiv (int * pdst, const int* psrc, const int* pdiv ) {
        const int imax = N / REP_BURST * REP_BURST;
        int i;
        for (i = 0; i < imax; i += REP_BURST)
            rep<REP_BURST>::vdiv(&pdst[i], &psrc[i], &pdiv[i]);
        rep<N - imax>::vdiv(&pdst[i], &psrc[i], &pdiv[i]);
	}
};

template<>
class rep<0> {
public:
	template<typename T>
	static DSP_INLINE 
	void vmemcpy (T * pdst, const T * psrc ) { }

	template<typename T>
	static DSP_INLINE 
	void vmemzero (T * pdst ) { };

	template<typename T>	
	static DSP_INLINE 
	void vmul (T* pdst, const T * psrc, const T * pcoeffs ) { }

	static DSP_INLINE
	void vor (unsigned long* pdst, const unsigned long* psrc) { }

	template<typename T>	
	static DSP_INLINE 
	void vshift_right (T * psrc, int nbits ) { }

	template<typename T>	
	static DSP_INLINE 
	void vshift_left (T * psrc, int nbits ) { }
	
	static DSP_INLINE 
	void vsqrnorm (vi * pdst, const vcs * psrc ) { }

	template<typename T>	
	static DSP_INLINE 
	void vsub (T* pdst, const T * psrc, const T& val ) { }

	template<typename T>	
	static DSP_INLINE 
	void vsum (T& r, const T * psrc ) { }

	static DSP_INLINE 
	void vdiv (int * pdst, const int* psrc, const int* pdiv ) { }
};

template<typename T, size_t N, typename = true_wrapper<> >
struct repex
{
    DSP_INLINE static void vmemcpy(T *dst, const T *src)
    {
        memcpy(dst, src, sizeof(T) * N);
    }
};

template<size_t N>
struct repex<COMPLEX16, N, true_wrapper<N % 4 == 0> >
{
    DSP_INLINE static void vmemcpy(COMPLEX16 *dst, const COMPLEX16 *src)
    {
        rep<N/4>::vmemcpy ((vcs *)dst, (vcs *)src);
    }
};

//////////////////////////////////////////////////////////////////////////////
// Public APIs
DECLARE_PUBLIC_OP(abs);
DECLARE_PUBLIC_OP(abs0);
DECLARE_PUBLIC_OP(add);
DECLARE_PUBLIC_OP(and);
DECLARE_PUBLIC_OP(andnot);
DECLARE_PUBLIC_OP(average);
DECLARE_PUBLIC_OP(comprise);
DECLARE_PUBLIC_OP(conj);
DECLARE_PUBLIC_OP(conj0);
DECLARE_PUBLIC_OP(conj_mul);
DECLARE_PUBLIC_OP(conj_mul_shift);
DECLARE_PUBLIC_OP(conjre);
DECLARE_PUBLIC_OP(extract);
DECLARE_PUBLIC_OP(flip);
DECLARE_PUBLIC_OP(hadd);
DECLARE_PUBLIC_OP(hadd4);
DECLARE_PUBLIC_OP(hmax);
DECLARE_PUBLIC_OP(hmin);
DECLARE_PUBLIC_OP(insert);
DECLARE_PUBLIC_OP(interleave_high);
DECLARE_PUBLIC_OP(interleave_low);
DECLARE_PUBLIC_OP(is_great);
DECLARE_PUBLIC_OP(is_less);
DECLARE_PUBLIC_OP(load);
DECLARE_PUBLIC_OP(move_mask);
DECLARE_PUBLIC_OP(mul_high);
DECLARE_PUBLIC_OP(mul_j);
DECLARE_PUBLIC_OP(mul_low);
DECLARE_PUBLIC_OP(mul_shift);
DECLARE_PUBLIC_OP(or);
DECLARE_PUBLIC_OP(pack);
DECLARE_PUBLIC_OP(pairwise_muladd);
DECLARE_PUBLIC_OP(permutate);
DECLARE_PUBLIC_OP(permutate16);
DECLARE_PUBLIC_OP(permutate_high);
DECLARE_PUBLIC_OP(permutate_low);
DECLARE_PUBLIC_OP(saturated_add);
DECLARE_PUBLIC_OP(saturated_hadd);
DECLARE_PUBLIC_OP(saturated_hadd4);
DECLARE_PUBLIC_OP(saturated_pack);
DECLARE_PUBLIC_OP(saturated_sub);
DECLARE_PUBLIC_OP(set_all);
DECLARE_PUBLIC_OP(set_all_bits);
DECLARE_PUBLIC_OP(set_zero);
DECLARE_PUBLIC_OP(shift_element_left);
DECLARE_PUBLIC_OP(shift_element_right);
DECLARE_PUBLIC_OP(shift_left);
DECLARE_PUBLIC_OP(shift_right);
DECLARE_PUBLIC_OP(sign);
DECLARE_PUBLIC_OP(smax);
DECLARE_PUBLIC_OP(smin);
DECLARE_PUBLIC_OP(SquaredNorm);
DECLARE_PUBLIC_OP(store);
DECLARE_PUBLIC_OP(store_nt);
DECLARE_PUBLIC_OP(sub);
DECLARE_PUBLIC_OP(unpack);
DECLARE_PUBLIC_OP(xor);

//////////////////////////////////////////////////////////////////////////////
// Remove private macros from symbol table
#undef DECLARE_PUBLIC_OP
#undef PVECTOR_STRUCT
#undef DEFINE_OP_ARITHMETIC1
#undef DEFINE_TEMPLATE_OP_ARITHMETIC2
#undef DEFINE_OP_ARITHMETIC2
#undef DEFINE_OP_PERMUTATION
#undef DEFINE_OP_PERMUTATION4
#undef DEFINE_OP_SHIFT
#undef DEFINE_OP_REDUCE4
#undef DEFINE_OP_REDUCE
#undef DEFINE_OP_DUPLICATION16_OPERATION
#undef DEFINE_OP_DUPLICATION8_OPERATION
#undef DEFINE_OP_MINMAX_SIGNED_UNSIGNED
#undef DEFINE_OP_EXTRACT


struct SignalBlock
{
    static const size_t size = 7;
    vcs _data[size];
    vcs& operator[](size_t index) { return _data[index]; }
    const vcs& operator[](size_t index) const { return _data[index]; }

    DSP_INLINE SignalBlock operator>>(int nbits)
    {
        SignalBlock o;
        o[0] = (*this)[0] >> nbits;
        o[1] = (*this)[1] >> nbits;
        o[2] = (*this)[2] >> nbits;
        o[3] = (*this)[3] >> nbits;
        o[4] = (*this)[4] >> nbits;
        o[5] = (*this)[5] >> nbits;
        o[6] = (*this)[6] >> nbits;
        return o;
    }

    DSP_INLINE SignalBlock operator<<(int nbits)
    {
        SignalBlock o;
        o[0] = (*this)[0] << nbits;
        o[1] = (*this)[1] << nbits;
        o[2] = (*this)[2] << nbits;
        o[3] = (*this)[3] << nbits;
        o[4] = (*this)[4] << nbits;
        o[5] = (*this)[5] << nbits;
        o[6] = (*this)[6] << nbits;
        return o;
    }
};
