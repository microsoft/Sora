#pragma once
#include "tpltrick.h"
#include "unroll.h"
#include "vector128.h"

namespace operator_repeater { namespace details {
    // Helper class to define various operator repeaters
    // Use normal loop and unroll loop to operate FUNC repeatedly
    template<size_t N, typename FUNC, const int REP_BURST = 16>
    struct _Rep_Base
    {
        FINL static void run(FUNC& func)
        {
            const int imax = N / REP_BURST * REP_BURST;
            for (int i = 0; i < imax; i += REP_BURST)
                unroll_for<0, REP_BURST, 1>(func);
            unroll_for<0, N - imax, 1>(func);
        }
    };
} }

// Define operator repeaters for differnt operators on arrays
template<size_t N, typename T>
FINL void rep_memcpynt(T* dst, const T* src)
{
    class op {
        T* dst; const T* src;
    public:
        FINL op(T* dst, const T* src)
            : dst(dst), src(src)
        { }

        FINL void operator()(size_t /*index*/)
        {
            store_nt(dst++, *src++);
        }
    };
	op op1(dst, src);
    operator_repeater::details::_Rep_Base<N, op>::run(op1);
}

template<size_t N, typename T>
FINL void rep_memzero(T* dst)
{
    class op {
        T* dst; const T* src;
    public:
        FINL op(T* dst)
            : dst(dst)
        { }

        FINL void operator()(size_t /*index*/)
        {
            set_zero(*dst++);
        }
    };
	op op1(dst);
    operator_repeater::details::_Rep_Base<N, op>::run(op1);
}

template<size_t N, typename T>
FINL void rep_mul(T* dst, const T* src0, const T* src1)
{
    class op {
        T* dst; const T* src0; const T* src1;
    public:
        FINL op(T* dst, const T* src0, const T* src1)
            : dst(dst), src0(src0), src1(src1)
        { }

        FINL void operator()(size_t /*index*/)
        {
            *dst++ = (*src0++) * (*src1++);
        }
    };
	op op1(dst, src0, src1);
    operator_repeater::details::_Rep_Base<N, op>::run(op1);
}

template<size_t N, typename T, typename T_SCALE>
FINL void rep_mul_scale(T* dst, const T* src, const T_SCALE& scale)
{
    class op {
        T* dst; const T* src; const T_SCALE& scale;
    public:
        FINL op(T* dst, const T* src, const T_SCALE& scale)
            : dst(dst), src(src), scale(scale)
        { }

        FINL void operator()(size_t /*index*/)
        {
            *dst++ = mul(*src++, scale);
        }
    };
	op op1(dst, src, scale);
    operator_repeater::details::_Rep_Base<N, op>::run(op1);
}

template<size_t N, typename T>
FINL void rep_or(T* dst, const T* src0, const T* src1)
{
    class op {
        T* dst; const T* src0; const T* src1;
    public:
        FINL op(T* dst, const T* src0, const T* src1)
            : dst(dst), src0(src0), src1(src1)
        { }

        FINL void operator()(size_t /*index*/)
        {
            *dst++ = (*src0++) | (*src1++);
        }
    };
	op op1(dst, src0, src1);
    operator_repeater::details::_Rep_Base<N, op>::run(op1);
}

template<size_t N, typename T>
FINL void rep_shift_right(T* dst, const T* src, const int nbits)
{
    class op {
        T* dst; const T* src; const int nbits;
    public:
        FINL op(T* dst, const T* src, const int nbits)
            : dst(dst), src(src), nbits(nbits)
        { }

        FINL void operator()(size_t /*index*/)
        {
            *dst++ = (*src++) >> nbits;
        }
    };
	op op1(dst, src, nbits);
    operator_repeater::details::_Rep_Base<N, op>::run(op1);
}

template<size_t N, typename T>
FINL void rep_shift_left(T* dst, const T* src, const int nbits)
{
    class op {
        T* dst; const T* src; const int nbits;
    public:
        FINL op(T* dst, const T* src, const int nbits)
            : dst(dst), src(src), nbits(nbits)
        { }

        FINL void operator()(size_t /*index*/)
        {
            *dst++ = (*src++) << nbits;
        }
    };
	op op1(dst, src, nbits);
    operator_repeater::details::_Rep_Base<N, op>::run(op1);
}

template<size_t N, typename T>
FINL void rep_sub(T* dst, const T* src, const T& val)
{
    class op {
        T* dst; const T* src; const T& val;
    public:
        FINL op(T* dst, const T* src, const T& val)
            : dst(dst), src(src), val(val)
        { }

        FINL void operator()(size_t /*index*/)
        {
            *dst++ = (*src++) - val;
        }
    };
	op op1(dst, src, val);
    operator_repeater::details::_Rep_Base<N, op>::run(op1);
}

template<size_t N, typename T>
FINL void rep_saturated_sub(T* dst, const T* src, const T* oprand )
{
    class op {
        T* dst; const T* src; const T* oprand;
    public:
        FINL op(T* dst, const T* src, const T* oprand)
            : dst(dst), src(src), val(oprand)
        { }

        FINL void operator()(size_t /*index*/)
        {
            *dst++ = saturated_sub ((*src++),  (*oprand++));
        }
    };
	op op1(dst, src, oprand);
    operator_repeater::details::_Rep_Base<N, op>::run(op1);
}

template<size_t N>
FINL void rep_sqrnorm(vi *dst, const vcs *src)
{
    class op {
        vi *dst; const vcs *src;
    public:
        FINL op(vi *dst, const vcs *src)
            : dst(dst), src(src)
        { }

        FINL void operator()(size_t /*index*/)
        {
            *dst++ = SquaredNorm(*src++);
        }
    };
	op op1(dst, src);
    operator_repeater::details::_Rep_Base<N, op>::run(op1);
}

template<size_t N, typename T>
FINL void rep_sum(T& result, const T* src)
{
    class op {
        T& result; const T* src;
    public:
        FINL op(T& result, const T* src)
            : result(result), src(src)
        { }

        FINL void operator()(size_t /*index*/)
        {
            result += *src++;
        }
    };
	op op1(result, src);
    operator_repeater::details::_Rep_Base<N, op>::run(op1);
}

template<size_t N, typename T>
FINL void rep_div(T* dst, const T* src0, const T* src1)
{
    class op {
        T* dst; const T* src0; const T* src1;
    public:
        FINL op(T* dst, const T* src0, const T* src1)
            : dst(dst), src0(src0), src1(src1)
        { }

        FINL void operator()(size_t /*index*/)
        {
            *dst++ = (*src0++) / (*src1++);
        }
    };
	op op1(dst, src0, src1);
    operator_repeater::details::_Rep_Base<N, op>::run(op1);
}

template<size_t N, typename T, typename T_SCALE>
FINL void rep_div_scale(T* dst, const T* src, const T_SCALE& scale)
{
    class op {
        T* dst; const T* src; const T_SCALE& scale;
    public:
        FINL op(T* dst, const T* src, const T_SCALE& scale)
            : dst(dst), src(src), scale(scale)
        { }

        FINL void operator()(size_t /*index*/)
        {
            *dst++ = div(*src++, scale);
        }
    };
	op op1(dst, src, scale);
    operator_repeater::details::_Rep_Base<N, op>::run(op1);
}

namespace operator_repeater { namespace details {
    // Helper class to define rep_memcpy function, in order to take advantage of SSE optimization for copying COMPLEX16 array 
    template<size_t N, typename T> // General function
    struct helper_memcpy
    {
        FINL static void run(T* dst, const T* src)
        {
            class op {
                T* dst; const T* src;
            public:
                FINL op(T* dst, const T* src)
                    : dst(dst), src(src)
                { }

                FINL void operator()(size_t /*index*/)
                {
                    *dst++ = *src++;
                }
            };
			op op1(dst, src);
            operator_repeater::details::_Rep_Base<N, op>::run(op1);
        }
    };
    template<size_t N>
    struct helper_memcpy<N, COMPLEX16> // Partial specialization for COMPLEX16 array
    {
        FINL static void run(COMPLEX16* dst, const COMPLEX16* src)
        {
            assert((uintptr_t)dst % 16 == 0); // must be 16 bytes aligned
            assert((uintptr_t)src % 16 == 0); // must be 16 bytes aligned
            helper_memcpy<N/4, vcs>::run((vcs *)dst, (const vcs *)src);

            // Tailing part
            const size_t copied = N / 4 * 4;
            helper_memcpy<N%4, COMPLEX16>::run(dst + copied, src + copied);
        }
    };
    template<>
    struct helper_memcpy<0, COMPLEX16> // Recursive terminal
    {
        FINL static void run(COMPLEX16* dst, const COMPLEX16* src) { }
    };
} }

template<size_t N, typename T>
FINL void rep_memcpy(T* dst, const T* src)
{
    operator_repeater::details::helper_memcpy<N, T>::run(dst, src);
}
