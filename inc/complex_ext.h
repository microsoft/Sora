#pragma once

#include "const.h"
#include "complex.h"
#include "tpltrick.h"

namespace complex { namespace details {
    template <class T> struct traits;
    template <> struct traits<COMPLEX8> { 
		typedef COMPLEX8 type; 
		typedef char element_type; 
	    typedef short norm2_type; 
	};
	template <> struct traits<COMPLEX16> { 
		typedef COMPLEX16 type; 
		typedef short element_type; 
	    typedef int   norm2_type; 
	};
	template <> struct traits<COMPLEX32> { 
		typedef COMPLEX32 type; 
		typedef int element_type; 
	    typedef __int64 norm2_type; 
	};
	template <> struct traits<COMPLEX64> { 
		typedef COMPLEX64 type; 
		typedef __int64 element_type; 
	    typedef __int64 norm2_type; 
	};
	template <> struct traits<COMPLEXU8> { 
		typedef COMPLEXU8 type; 
		typedef unsigned char element_type; 
	    typedef unsigned short norm2_type; 
	};
	template <> struct traits<COMPLEXU16> { 
		typedef COMPLEXU16 type; 
		typedef unsigned short element_type; 
	    typedef unsigned int   norm2_type; 	
	};
	template <> struct traits<COMPLEXU32> { 
		typedef COMPLEXU32 type; 
		typedef unsigned int element_type; 
	    typedef unsigned __int64 norm2_type; 
	};
	template <> struct traits<COMPLEXU64> { 
		typedef COMPLEXU64 type; 
		typedef unsigned __int64 element_type; 
	    typedef unsigned __int64 norm2_type; 
	};
	template <> struct traits<COMPLEXF> { 
		typedef COMPLEXF type; 
		typedef float element_type; 
	    typedef float norm2_type; 		
	};
} }

template<typename T_COMPLEX>
struct complex_init : T_COMPLEX
{
    typedef typename complex::details::traits<T_COMPLEX>::element_type element_type;
    complex_init(element_type re, element_type im) { T_COMPLEX::re = re; T_COMPLEX::im = im; }
};

// Use SFINAE principle to constrain operator overload on only complex types
// ref: http://www.wambold.com/Martin/writings/template-parameter-traitss.html
template<typename T>
typename complex::details::traits<T>::type const
FINL operator*(const T& a, const T& b) {
    T ret;
	ret.re = a.re * b.re - a.im * b.im; 
	ret.im = a.im * b.re + a.re * b.im;
	return ret;
}

template<typename T>
typename complex::details::traits<T>::type const
FINL operator*(const T& a, const typename complex::details::traits<T>::element_type& b) {
    T ret;
	ret.re = a.re * b;
	ret.im = a.im * b;
	return ret;
}

template<typename TO, typename T>
FINL
typename constraint_types
    <typename complex::details::traits<TO>::type
    , typename complex::details::traits<T>::type
    >::type1 &
operator+=(TO & a, const T & b)
{ a.re += b.re; a.im += b.im; return a; }

template<typename T>
typename complex::details::traits<T>::type const
FINL operator+(const T& a, const T& b) { T ret; ret.re = a.re + b.re; ret.im = a.im + b.im; return ret; }

template<typename TO, typename T>
FINL
typename constraint_types
    <typename complex::details::traits<TO>::type
    , typename complex::details::traits<T>::type
    >::type1 &
operator-=(TO& a, const T& b)
{ a.re -= b.re; a.im -= b.im; return a; }

template<typename T>
typename complex::details::traits<T>::type const
operator-(const T& a, const T& b) { T ret; ret.re = a.re - b.re; ret.im = a.im - b.im; return ret; }

template<typename T>
typename complex::details::traits<T>::type const
FINL operator-(const T& a) { T ret; ret.re = - a.re; ret.im = - a.im; return ret; }

template<typename T>
typename complex::details::traits<T>::type const
FINL operator>>(const T& a, int shift) { T t; t.re = a.re >> shift; t.im = a.im >> shift; return t; }

template<typename T>
typename complex::details::traits<T>::type const
FINL operator<<(const T& a, int shift) { T t; t.re = a.re << shift; t.im = a.im << shift; return t; }

template<typename TA, typename TB>
FINL
typename constraint_types
    <bool
    , typename complex::details::traits<TA>::type
    , typename complex::details::traits<TB>::type
    >::type1 const
operator==(const TA& a, const TB& b)
{ return a.re == b.re && a.im == b.im; }


template<typename T>
typename complex::details::traits<T>::norm2_type const
FINL SquaredNorm(const T& a) { return (a.re*a.re + a.im*a.im); }

template<typename T>
typename complex::details::traits<T>::norm2_type const
FINL norm2(const T& a) { return SquaredNorm (a); }


FINL int norm1(COMPLEX32 *input)
{
    return (input->re ^ (input->re >> 31)) + (input->im ^ (input->im >> 31));
}

template<typename T>
typename complex::details::traits<T>::type const
FINL conj_im(const T& a) { T ret; ret.re = a.re; ret.im = -a.im; return ret; }

template<typename T>
typename complex::details::traits<T>::type const
FINL conj_re(const T& a) { T ret; ret.re = -a.re; ret.im = a.im; return ret; }
