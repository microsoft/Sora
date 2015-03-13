#pragma once

#include "vector128.h"
#include "CRC32.h"
#include "bb/mod/afifos.h"

template<int MBitRate> inline bool NeedNewDescriptor(int iTrellis);
template<> inline bool NeedNewDescriptor<6>(int iTrellis) { return iTrellis == 24 || iTrellis == 48; }
template<> inline bool NeedNewDescriptor<9>(int iTrellis) { return iTrellis == 36 || iTrellis == 72|| iTrellis == 108|| iTrellis == 144; }
template<> inline bool NeedNewDescriptor<12>(int iTrellis) { return iTrellis == 48; }
template<> inline bool NeedNewDescriptor<18>(int iTrellis) { return iTrellis == 72 || iTrellis == 144; }
template<> inline bool NeedNewDescriptor<24>(int iTrellis) { return iTrellis == 96; }
template<> inline bool NeedNewDescriptor<36>(int iTrellis) { return iTrellis == 144; }
template<> inline bool NeedNewDescriptor<48>(int iTrellis) { return iTrellis == 192; }
template<> inline bool NeedNewDescriptor<54>(int iTrellis) { return iTrellis == 216; }

inline bool NeedNewDescriptor(int MBitRate, int iTrellis)
{
    bool ret;
    switch(MBitRate)
    {
    case  6: ret = NeedNewDescriptor< 6>(iTrellis); break;
    case  9: ret = NeedNewDescriptor< 9>(iTrellis); break;
    case 12: ret = NeedNewDescriptor<12>(iTrellis); break;
    case 18: ret = NeedNewDescriptor<18>(iTrellis); break;
    case 24: ret = NeedNewDescriptor<24>(iTrellis); break;
    case 36: ret = NeedNewDescriptor<36>(iTrellis); break;
    case 48: ret = NeedNewDescriptor<48>(iTrellis); break;
    case 54: ret = NeedNewDescriptor<54>(iTrellis); break;
    default:
        NODEFAULT;
    }
    return ret;
}

DSP_INLINE void ViterbiAdvance(vub *&pTrellis, const vub pVITMA[], int i_ma, const vub pVITMB[], int i_mb)
{
    const vub ALLONE    (ALL_ONE);
    const vub ALLINVONE (ALL_INVERSE_ONE);
    const vub ALLM128   (ALL_M128);

    // Index to the Branch Metric LUT table
    i_ma <<= 3;
    i_mb <<= 3;

    // temporal variables
    vub rub0, rub1, rub2, rub3;

    // Compute the new states
    rub0 = interleave_low (pTrellis[0], pTrellis[0]);
    rub1 = interleave_low (pTrellis[2], pTrellis[2]);

    // branch 0
    rub0 = add ( rub0, pVITMA[i_ma] );
    rub0 = add ( rub0, pVITMB[i_mb] );
    rub0 = and ( rub0, ALLINVONE); // mark the path

    // branch 1
    rub1 = add ( rub1, pVITMA[i_ma+1] );
    rub1 = add ( rub1, pVITMB[i_mb+1] ); 
    rub1 = or  ( rub1, ALLONE );

    // store the shortest path, state:[0-15]
    pTrellis[4] = smin (rub0, rub1); 

    rub0 = interleave_high (pTrellis[0], pTrellis[0]);
    rub1 = interleave_high (pTrellis[2], pTrellis[2]);

    // branch 0
    rub0 = add ( rub0, pVITMA[i_ma+2] );
    rub0 = add ( rub0, pVITMB[i_mb+2] );  
    rub0 = and ( rub0, ALLINVONE); // mark the path

    // branch 1
    rub1 = add ( rub1, pVITMA[i_ma+3] );
    rub1 = add ( rub1, pVITMB[i_mb+3] ); 
    rub1 = or  ( rub1, ALLONE );

    // store the shortest path, state:[16-31]    
    pTrellis[5] = smin (rub0, rub1); 

    rub0 = interleave_low (pTrellis[1], pTrellis[1]);
    rub1 = interleave_low (pTrellis[3], pTrellis[3]);

    // branch 0
    rub0 = add ( rub0, pVITMA[i_ma+4] );
    rub0 = add ( rub0, pVITMB[i_mb+4] );  
    rub0 = and ( rub0, ALLINVONE); // mark the path

    // branch 1
    rub1 = add ( rub1, pVITMA[i_ma+5] );
    rub1 = add ( rub1, pVITMB[i_mb+5] ); 
    rub1 = or  ( rub1, ALLONE );

    // store the shortest path, state:[32-47]    
    pTrellis[6] = smin (rub0, rub1); 

    rub0 = interleave_high (pTrellis[1], pTrellis[1]);
    rub1 = interleave_high (pTrellis[3], pTrellis[3]);

    // branch 0
    rub0 = add ( rub0, pVITMA[i_ma+6] );
    rub0 = add ( rub0, pVITMB[i_mb+6] );  
    rub0 = and ( rub0, ALLINVONE); // mark the path

    // branch 1
    rub1 = add ( rub1, pVITMA[i_ma+7] );
    rub1 = add ( rub1, pVITMB[i_mb+7] ); 
    rub1 = or  ( rub1, ALLONE );

    // store the shortest path, state:[48-63]        
    pTrellis[7] = smin (rub0, rub1); 

    pTrellis += 4;  
}

DSP_INLINE void ViterbiAdvance(vub *&pTrellis, const vub pVITMA[], int i_ma)
{
    const vub ALLONE    (ALL_ONE);
    const vub ALLINVONE (ALL_INVERSE_ONE);
    const vub ALLM128   (ALL_M128);

    // Index to the Branch Metric LUT table
    i_ma <<= 3;

    // temporal variables
    vub rub0, rub1, rub2, rub3;

    // Compute the new states
    rub0 = interleave_low (pTrellis[0], pTrellis[0]);
    rub1 = interleave_low (pTrellis[2], pTrellis[2]);

    // branch 0
    rub0 = add ( rub0, pVITMA[i_ma] );
    rub0 = and ( rub0, ALLINVONE); // mark the path

    // branch 1
    rub1 = add ( rub1, pVITMA[i_ma+1] );
    rub1 = or  ( rub1, ALLONE );

    // store the shortest path, state:[0-15]
    pTrellis[4] = smin (rub0, rub1); 

    rub0 = interleave_high (pTrellis[0], pTrellis[0]);
    rub1 = interleave_high (pTrellis[2], pTrellis[2]);

    // branch 0
    rub0 = add ( rub0, pVITMA[i_ma+2] );
    rub0 = and ( rub0, ALLINVONE); // mark the path

    // branch 1
    rub1 = add ( rub1, pVITMA[i_ma+3] );
    rub1 = or  ( rub1, ALLONE );

    // store the shortest path, state:[16-31]    
    pTrellis[5] = smin (rub0, rub1); 

    rub0 = interleave_low (pTrellis[1], pTrellis[1]);
    rub1 = interleave_low (pTrellis[3], pTrellis[3]);

    // branch 0
    rub0 = add ( rub0, pVITMA[i_ma+4] );
    rub0 = and ( rub0, ALLINVONE); // mark the path

    // branch 1
    rub1 = add ( rub1, pVITMA[i_ma+5] );
    rub1 = or  ( rub1, ALLONE );

    // store the shortest path, state:[32-47]    
    pTrellis[6] = smin (rub0, rub1); 

    rub0 = interleave_high (pTrellis[1], pTrellis[1]);
    rub1 = interleave_high (pTrellis[3], pTrellis[3]);

    // branch 0
    rub0 = add ( rub0, pVITMA[i_ma+6] );
    rub0 = and ( rub0, ALLINVONE); // mark the path

    // branch 1
    rub1 = add ( rub1, pVITMA[i_ma+7] );
    rub1 = or  ( rub1, ALLONE );

    // store the shortest path, state:[48-63]        
    pTrellis[7] = smin (rub0, rub1);

    pTrellis += 4;  
}

template<int MBitRate, int NOR_MASK_, int TB_DEPTH_, int TB_OUTPUT_, typename VB>
inline void VitDesCRC(PBB11A_RX_CONTEXT pRxContextA, VB& vb)
{
    typedef VB::VB_DCBLOCK VB_DCBLOCK;

    // Thread synchronization flag
    volatile FLAG& fWork = *pRxContextA->ri_pbWorkIndicator;
    
    const int TB_PREFIX = 6;
    unsigned int uiBitsTotal = MBitRate * 4 * pRxContextA->uiVitSymbolCount;
    char *pbVitOutput = pRxContextA->VIT_OBUF;
    char *pbOutput = pRxContextA->pbVitFrameOutput;
    unsigned int uiBytesOutput = 0;

    VB_DCBLOCK * const blksStart   = vb.BlocksBegin();
    VB_DCBLOCK * const blksEnd     = vb.BlocksEnd();
    
    unsigned char bSeed;
    DWORD uiCrc32 = 0xFFFFFFFF;
    DWORD uiCrc32Store;
    unsigned int uiCrc32StorePoint = pRxContextA->uiVitFrameLen + 2 - 4;

    VB_DCBLOCK * blkInputStart = blksStart;
    char *blkInput = (char *)blkInputStart;
    
    int i, j;     

    int ocnt = 0; // Counter for the output bits

    // vector128 constants
    const vub * const pVITMA = (const vub*) VIT_MA; // Branch Metric A
    const vub * const pVITMB = (const vub*) VIT_MB; // Branch Metric B

    const vub ALLINVONE (ALL_INVERSE_ONE);

    unsigned char outchar = 0;    // the output(decoded) char
    unsigned char *pVTOutput;
    
    vub *pTrellis;          // point to trellis
    int i_trellis = 0;      // index of trellis

    // for trace back  
    vub *pTraceBk;          // trace back pointer in trellis
    int i_minpos = 0;       // the minimal path position
    int i_tpos   = 0;

    // temporal variables
    vub rub0, rub1, rub2, rub3;
    vus rus0, rus1, rus2, rus3;
    vus rus4, rus5, rus6, rus7;

    // Initialize Trellis
    pTrellis = (vub *) pRxContextA->trellisData;
    pTrellis[0] = ALL_INIT0;
    pTrellis[1] = ALL_INIT;
    pTrellis[2] = ALL_INIT;
    pTrellis[3] = ALL_INIT;

    assert(&blkInputStart->isValid == blkInput);
    while (blkInputStart->isValid == 0 && fWork)
        _mm_pause();

    if (!fWork) return;
    blkInput += 4; // jump to data
    
    // viterbi decode and descramble here
    // continue to process all input bits
    while (uiBitsTotal)
    {
        if (MBitRate == 6 || MBitRate == 24 || MBitRate == 12)
        {
            ViterbiAdvance(pTrellis, pVITMA, blkInput[0], pVITMB, blkInput[1]);
            blkInput += 2; // jump to data
            i_trellis++;
        }
        else if (MBitRate == 9 || MBitRate == 18 || MBitRate == 36 || MBitRate == 54)
        {
            ViterbiAdvance(pTrellis, pVITMA, blkInput[0], pVITMB, blkInput[1]);
            ViterbiAdvance(pTrellis, pVITMA, blkInput[2]);
            ViterbiAdvance(pTrellis, pVITMB, blkInput[3]);
            blkInput += 4;
            i_trellis += 3;
        }
        else if (MBitRate == 48)
        {
            ViterbiAdvance(pTrellis, pVITMA, blkInput[0], pVITMB, blkInput[1]);
            ViterbiAdvance(pTrellis, pVITMA, blkInput[2]);
            blkInput += 3;
            i_trellis += 2;
        }

        // Dump trellis
        //DUMP_TRELLIS (pTrellis);

        // Normalize
        if ((i_trellis & NOR_MASK_) == 0 )
        {
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
  
            // here I need also check if we need to load a new descriptor
            if (NeedNewDescriptor<MBitRate>(i_trellis))
            {
                // read a new descriptor
                blkInputStart->isValid = 0;
                blkInputStart += 1;
                if ( blkInputStart == blksEnd )
                    blkInputStart = blksStart;
                blkInput = (char *)blkInputStart;

                // wait ready
                assert(&blkInputStart->isValid == blkInput);
                while (blkInputStart->isValid == 0 && fWork)
                    _mm_pause();

                if (!fWork) return;
                blkInput += 4; // jump to data
            }
        }
        
        // Traceback 
        // We should first skip TB_DEPTH_VITAS bits and 
        // TB_OUTPUT_VITAS is acutal bits we can output
        if ( i_trellis >= TB_DEPTH_ + TB_OUTPUT_ + TB_PREFIX )
        {
            // track back
            // we need to find the minimal state and index of the state

            // do normalization first
    
            rub0 = smin (pTrellis[0], pTrellis[1] );
            rub1 = smin (pTrellis[2], pTrellis[3] );
            rub2 = smin (rub0, rub1);

            rub3 = hmin (rub2);
            rub3 = and  (rub3, ALLINVONE );
        
            // normalize
            pTrellis[0] = sub ( pTrellis[0], rub3);
            pTrellis[1] = sub ( pTrellis[1], rub3);
            pTrellis[2] = sub ( pTrellis[2], rub3);
            pTrellis[3] = sub ( pTrellis[3], rub3);        
    
            
            // rub3 has the minimal value, we need to find the index
            // the algorithm to find the right index is to embed the index at the least
            // significant bits of state value, then we just find the minimal value


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
            i_minpos = extract<0>(rus0); 

            // now we can trace back ...
            pTraceBk = pTrellis;

            // first part - trace back without output
            i_minpos = (i_minpos >> 2) & 0x3F; // index 6:0
            for ( i = 0; i < TB_DEPTH_; i++)
            {
                pTraceBk -= 4;
                i_minpos = (i_minpos >> 1) & 0x3F;
                i_tpos = ((char*) pTraceBk)[i_minpos] ;
                i_minpos |= (i_tpos & 1) << 6;  // now i_minpos 6:0 is the new index
            }

            // second part - trace back with output
            pbVitOutput += (TB_OUTPUT_ >> 3);
            pVTOutput = (unsigned char*)pbVitOutput;
            
            for ( i = 0; i < TB_OUTPUT_ >> 3; i++)
            {
                for ( j = 0; j < 8; j++ )
                {
                    outchar = outchar << 1;
                    outchar |= (i_minpos >> 6) & 1;

                    // next bit
                    pTraceBk -= 4;
                    i_minpos = (i_minpos >> 1) & 0x3F;
                    i_tpos = ((char*) pTraceBk)[i_minpos] ;
                    i_minpos |= (i_tpos & 1) << 6;  // now i_minpos 6:0 is the new index
                }

                pVTOutput --;
                * pVTOutput = outchar;
                
                outchar = 0;
            }

            // We need to do descramble and CRC here
            for ( i = 0; i < TB_OUTPUT_ >> 3; i++)
            {
                // first two bytes are seed
                if ( uiBytesOutput < 2)
                {
                    uiBytesOutput += 2;
                    bSeed = (* (pVTOutput+1)) >> 1;
                    pVTOutput += 2;
                    i += 2; // as if we start from the 3rd char
                }
                
                // scramble
                bSeed = SCRAMBLE_11A_LUT[bSeed];
                *pbOutput = (*pVTOutput) ^ bSeed;
                bSeed >>= 1;

                pVTOutput ++;
                uiBytesOutput ++;
            
                // compute crc32
                CalcCRC32Incremental(*pbOutput, &uiCrc32);
                //fprintf(stderr, "CRC %d:\t0x%08X\n", (uiBytesOutput)-3, uiCrc32);

                pbOutput ++;
                
                if ( uiBytesOutput == uiCrc32StorePoint )
                {
                    uiCrc32Store = uiCrc32;
                }
            }
                    
            uiBitsTotal -= TB_OUTPUT_;
            i_trellis   -= TB_OUTPUT_;
        }                
    }

    uiCrc32 = *(DWORD *)
        (pRxContextA->pbVitFrameOutput + (pRxContextA->uiVitFrameLen - 4));

    if (*(DWORD *)(pRxContextA->pbVitFrameOutput
            + (pRxContextA->uiVitFrameLen - 4)) == ~uiCrc32Store)
    {
        pRxContextA->bCRCCorrect = TRUE;
    }
    else
    {
    	// KdPrint(("[%08x!=%08x] ", uiCrc32, ~uiCrc32Store));
        pRxContextA->bCRCCorrect = FALSE;
    }
}
