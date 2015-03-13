#pragma once
#include <stdio.h>
#include <stdarg.h>

#define BB_DEBUG 0

#ifdef __cplusplus
// debug
template<int N>
inline 
void _dump_symbol ( char* plable, COMPLEX16* input ) {
#if BB_DEBUG > 2
	printf ("dump::start(%s)", plable);
    for ( size_t i=0; i<N; i++) {
        if ( i%4 == 0 ) printf ("\n(%2d) ", i);				
        printf ( "<%6d, %6d> , ", input[i].re, input[i].im);
    }
	printf ("\ndump::end\n");
#endif	
}
template<int N>
inline 
void _dump_symbol32 ( char* plable, COMPLEX32* input ) {
#if BB_DEBUG > 2
	printf ("dump::start(%s)", plable);
    for ( size_t i=0; i<N; i++) {
        if ( i%4 == 0 ) printf ("\n(%2d) ", i);				
        printf ( "<%6d, %6d> , ", input[i].re, input[i].im);
    }
	printf ("\ndump::end\n");
#endif	
}

inline
void _dump_text (const char *fmt, ...) {
#if BB_DEBUG
	va_list args;
	va_start (args, fmt);
	vprintf (fmt, args);
	va_end (args);
#endif
}
#endif