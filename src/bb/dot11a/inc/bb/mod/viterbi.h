#pragma once

#define BITS_PER_BYTE_SHIFT 3

#include "const.h"
#include "vector128.h"

// We use 3 bit SoftBit
#define SOFT_RANGE	    8

// The path metric range
#define METRIC_SIZE     128
#define METRIC_OFFSET   7

#define NOR_MASK_VITAS  0x7
#define BYTE_MAX_VITAS  6
#define BIT_MAX_VITAS   (BYTE_MAX_VITAS * 8)

// 6 leading bit
#define TB_PREFIX_VITAS  6          // must be 6
#define TB_DEPTH_VITAS   12         // must be multiple of 1
#define TB_OUTPUT_VITAS  (24)       // must be multiple of 8 and factor of 192

extern const vub::data_type ALL_INVERSE_ONE;
extern const vub::data_type ALL_ONE;
extern const vub::data_type ALL_M128;
extern const vub::data_type VIT_MA[8 * SOFT_RANGE];
extern const vub::data_type VIT_MB[8 * SOFT_RANGE];

extern A16 const vub::data_type ALL_INIT;
extern A16 const vub::data_type ALL_INIT0;
extern A16 const vub::data_type INDEXES [4];

/************************************************
Viterbi_asig: decode the signal (PLCP header) 
1/2 coded 24bits
*************************************************/
__forceinline
void Viterbi_asig(vub *pTrellis, char * pbInput, char * pbOutput)
{
    int i, j;
    int ocnt = 0; // Counter for the output bits

    // Vector1 const used
    vub ALLONE    (ALL_ONE);
    vub ALLINVONE (ALL_INVERSE_ONE );
    // __m128i xmm0 = ALLINVONE.raw; 
    // vub ALLINE (ALL_INVERSE_ONE );
    vub ALLINE (ALL_M128);

    A16 unsigned char outchar = 0;    // the output(decoded) char

    int i_trellis = 0;    // index of trellis

    // for trace back  
    A16 vub * pTraceBk;       // trace back pointer in trellis
    unsigned char i_minpos = 0;     // the minimal path position
    unsigned char i_tpos   = 0;


    char * psbit = pbInput;     // pointer to the s-bit stream;

    // temporal variables
    A16 vub rub0, rub1, rub2, rub3;
    A16 vus rus0, rus1, rus2, rus3;
    A16 vus rus4, rus5, rus6, rus7;


    unsigned int i_ma = 0; // index to the Branch Metric LUT table
    unsigned int i_mb = 0;

    pTrellis[0] = ALL_INIT0; 
    pTrellis[1] = ALL_INIT; 
    pTrellis[2] = ALL_INIT; 
    pTrellis[3] = ALL_INIT;

    while (ocnt < 24) {
        // We have to decode 24 bits for Signal (PLCP Header)

        // Compute the bench metric
        i_ma = (unsigned int)(unsigned char)(* psbit    ) << 3;
        i_mb = (unsigned int)(unsigned char)(* (psbit+1)) << 3;

        psbit += 2;

        // Compute the new states

        rub0 = interleave_low (pTrellis[0], pTrellis[0]);
        rub1 = interleave_low (pTrellis[2], pTrellis[2]);

        // branch 0
        rub0 = add ( rub0, VIT_MA[i_ma] );
        rub0 = add ( rub0, VIT_MB[i_mb] );  
        rub0 = and ( rub0, ALLINVONE); // mark the path

        // branch 1
        rub1 = add ( rub1, VIT_MA[i_ma+1] );
        rub1 = add ( rub1, VIT_MB[i_mb+1] ); 
        rub1 = or  ( rub1, ALLONE );

        // store the shortest path, state:[0-15]
        pTrellis[4] = smin (rub0, rub1); 

        rub0 = interleave_high (pTrellis[0], pTrellis[0]);
        rub1 = interleave_high (pTrellis[2], pTrellis[2]);

        // branch 0
        rub0 = add ( rub0, VIT_MA[i_ma+2] );
        rub0 = add ( rub0, VIT_MB[i_mb+2] );  
        rub0 = and ( rub0, ALLINVONE); // mark the path

        // branch 1
        rub1 = add ( rub1, VIT_MA[i_ma+3] );
        rub1 = add ( rub1, VIT_MB[i_mb+3] ); 
        rub1 = or  ( rub1, ALLONE );

        // store the shortest path, state:[16-31]    
        pTrellis[5] = smin (rub0, rub1); 

        rub0 = interleave_low (pTrellis[1], pTrellis[1]);
        rub1 = interleave_low (pTrellis[3], pTrellis[3]);

        // branch 0
        rub0 = add ( rub0, VIT_MA[i_ma+4] );
        rub0 = add ( rub0, VIT_MB[i_mb+4] );  
        rub0 = and ( rub0, ALLINVONE); // mark the path

        // branch 1
        rub1 = add ( rub1, VIT_MA[i_ma+5] );
        rub1 = add ( rub1, VIT_MB[i_mb+5] ); 
        rub1 = or  ( rub1, ALLONE );

        // store the shortest path, state:[32-47]    
        pTrellis[6] = smin (rub0, rub1); 

        rub0 = interleave_high (pTrellis[1], pTrellis[1]);
        rub1 = interleave_high (pTrellis[3], pTrellis[3]);

        // branch 0
        rub0 = add ( rub0, VIT_MA[i_ma+6] );
        rub0 = add ( rub0, VIT_MB[i_mb+6] );  
        rub0 = and ( rub0, ALLINVONE); // mark the path

        // branch 1
        rub1 = add ( rub1, VIT_MA[i_ma+7] );
        rub1 = add ( rub1, VIT_MB[i_mb+7] ); 
        rub1 = or  ( rub1, ALLONE );

        // store the shortest path, state:[48-63]        
        pTrellis[7] = smin (rub0, rub1); 

        // Move to next state
        pTrellis += 4;  
        i_trellis ++;

        // Normalize
        if ((i_trellis & 7) == 0 ) {
            // normalization
            // find the smallest component and extract it from all states
            rub0 = smin (pTrellis[0], pTrellis[1] );
            rub1 = smin (pTrellis[2], pTrellis[3] );
            rub2 = smin (rub0, rub1);

            rub3 = hmin (rub2);

            // make sure to clear the marker bit
            rub3 = and  (rub3, ALLINVONE );

            // normalize
            pTrellis[0] = sub ( pTrellis[0], rub3);
            pTrellis[1] = sub ( pTrellis[1], rub3);
            pTrellis[2] = sub ( pTrellis[2], rub3);
            pTrellis[3] = sub ( pTrellis[3], rub3);        
        }

        // Traceback 
        // We should first skip TB_DEPTH_VITAS bits and 
        // TB_OUTPUT_VITAS is acutal bits we can output
        if ( i_trellis >= TB_DEPTH_VITAS + TB_OUTPUT_VITAS + TB_PREFIX_VITAS ) {
            // track back
            // we need to find the minimal state and index of the state

            // do normalization first
            rub0 = smin (pTrellis[0], pTrellis[1] );
            rub1 = smin (pTrellis[2], pTrellis[3] );
            rub2 = smin (rub0, rub1);

            rub3 = hmin (rub2);
            rub3 = and (rub3, ALLINVONE );

            // normalize
            pTrellis[0] = sub ( pTrellis[0], rub3);
            pTrellis[1] = sub ( pTrellis[1], rub3);
            pTrellis[2] = sub ( pTrellis[2], rub3);
            pTrellis[3] = sub ( pTrellis[3], rub3);        


            // rub3 has the minimal value, we need to find the index
            // the algorithm to find the right index is to embed the index at the least
            // significant bits of state value, then we just find the minimal value

            // ensure to use pminsw - not needed
            rub0 = INDEXES[0];
            rub1 = pTrellis[0];

            rus2 = (vus)interleave_low  ( rub0, rub1 );
            rus3 = (vus)interleave_high ( rub0, rub1 );
            rus4 = smin ( rus2, rus3);

            rub0 = INDEXES[1];
            rub1 = pTrellis[1];
            rus2 = (vus)interleave_low  ( rub0, rub1 );
            rus3 = (vus)interleave_high ( rub0, rub1 );

            rus5 = smin (rus2, rus3);
            rus4 = smin (rus4, rus5);

            rub0 = INDEXES[2];
            rub1 = pTrellis[2];
            rus2 = (vus)interleave_low  ( rub0, rub1 );
            rus3 = (vus)interleave_high ( rub0, rub1 );

            rus6 = smin (rus2, rus3);
            rus4 = smin (rus4, rus6);

            rub0 = INDEXES[3];
            rub1 = pTrellis[3];
            rus2 = (vus)interleave_low  ( rub0, rub1 );
            rus3 = (vus)interleave_high ( rub0, rub1 );

            rus7 = smin (rus2, rus3);
            rus4 = smin (rus4, rus7);

            // now rus4 contains the minimal 8 
            rus0 = hmin (rus4);

            // now the first word contains the index and value
            // index: bit [7:2]; 
            // value: bit [15:8]
            i_minpos = (unsigned char)extract<0>(rus0);

            // now we can trace back ...
            pTraceBk = pTrellis;

            // first part - trace back without output
            i_minpos /= 2; // index 5:0
            for ( i = 0; i < TB_DEPTH_VITAS; i++) {
                pTraceBk -= 4;
                i_minpos = (i_minpos >> 1) & 0x3F;
                i_tpos = ((char*) pTraceBk)[i_minpos];
                i_minpos |= (i_tpos & 1) << 6;  // now i_minpos 6:0 is the new index
            }

            // second part - trace back with output
            pbOutput += (TB_OUTPUT_VITAS >> 3);
            for ( i = 0; i < TB_OUTPUT_VITAS >> 3; i++) {
                for ( j = 0; j < 8; j++ ) {
                    outchar = outchar << 1;
                    outchar |= (i_minpos >> 6) & 1;
                    ocnt ++;

                    // next bit
                    pTraceBk -= 4;
                    i_minpos = (i_minpos >> 1) & 0x3F;
                    i_tpos = ((char*) pTraceBk)[i_minpos] ;
                    i_minpos |= (i_tpos & 1) << 6;  // now i_minpos 6:0 is the new index
                }

                pbOutput --;
                * pbOutput = outchar;
                outchar = 0;
            }
        }
    }
}
