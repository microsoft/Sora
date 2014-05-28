// Some helper functions to print vector types
#pragma once
#include <stdio.h>
#include "vector128.h"

#ifndef USER_MODE
#define printf(...) 
#endif

#define BB_DBG 0

inline void dumpBlock ( SignalBlock* block ) {
    short * pBlock = (short*) block;
    printf ( "Dump block\n" );
    for ( int i=0; i < 7; i++ ) {
        for ( int j=0; j <4; j++ ) {
            printf ( "{%d, %d} ", pBlock[i*8+j*2], pBlock[i*8+2*j+1] );
        }
        printf ( "\n" );
    }
}

inline void dumpVector16 ( vcs * vec ) {
    short * pBlock = (short*) vec;
    printf ( "Dump vector 16\n" );
    for ( int i=0; i < 4; i++ ) {
        printf ( "{%d, %d} ", pBlock[i*2], pBlock[i*2+1] );
    }
    printf ( "\n" );
}

inline void dumpSymbol16 ( void * sym ) {
    short * pBlock = (short*) sym;
    printf ( "Dump 64 symbol vector\n" );
    int cnt = 0;
    int br  = 0;
    for ( int j=0; j <64; j ++ ) {
        cnt ++;
        br = 0;
        if ( cnt % 4 == 0 ) {
            printf ( "\n" ); br = 1;
        } 
        
        if ( j == 7 || j==21 || j==43 || j== 57 ) {   
            if ( !br ) {
                printf ( "\n" );
            }
            printf ( "P - {%d, %d}\n", pBlock[j*2], pBlock[j*2+1] );
        } else {
            printf ( "{%d, %d} ", pBlock[j*2], pBlock[j*2+1] );
        }
        
    }
    printf ( "\n" );
    printf ( "\n" );
}

inline void dumpSymbol32 ( void * sym ) {
    int * pBlock = (int*) sym;
    printf ( "Dump 64 symbol vector\n" );
    int cnt = 0;
    int br  = 0;
    for ( int j=0; j <64; j ++ ) {
        cnt ++;
        br = 0;
        if ( cnt % 4 == 0 ) {
            printf ( "\n" ); br = 1;
        } 
        
        if ( j == 7 || j==21 || j==43 || j== 57 ) {   
            if ( !br ) {
                printf ( "\n" );
            }
            printf ( "P - {%d, %d}\n", pBlock[j*2], pBlock[j*2+1] );
        } else {
            printf ( "{%d, %d} ", pBlock[j*2], pBlock[j*2+1] );
        }
        
    }
    printf ( "\n" );
    printf ( "\n" );
}
