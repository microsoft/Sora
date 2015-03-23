#pragma once

#include "ieee80211a_cmn.h"
#include "ieee80211facade.hpp"
#include <intalg.h>

//
// Pilot sequence as defined in 802.11a
//
const char PilotSgn[128] = {	
         0,  0,  0, -1, -1, -1,  0, -1, 
        -1, -1, -1,  0,  0, -1,  0, -1, 
        -1,  0,  0, -1,  0,  0, -1,  0, 
         0,  0,  0,  0,  0, -1,  0,  0, 
         0, -1,  0,  0, -1, -1,  0,  0, 
         0, -1,  0, -1, -1, -1,  0, -1, 
         0, -1, -1,  0, -1, -1,  0,  0, 
         0,  0,  0, -1, -1,  0,  0, -1, 
        -1,  0, -1,  0, -1,  0,  0, -1, 
        -1, -1,  0,  0, -1, -1, -1, -1, 
         0, -1, -1,  0, -1,  0,  0,  0, 
         0, -1,  0, -1,  0, -1,  0, -1, 
        -1, -1, -1, -1,  0, -1,  0,  0, 
        -1,  0, -1,  0,  0,  0, -1, -1, 
         0, -1, -1, -1,  0,  0,  0, -1, 
        -1, -1, -1, -1, -1, -1,  0, 
        0, // # 127: reserved for PLCP signal
};

DEFINE_LOCAL_CONTEXT(T11aAddPilot, CF_VOID);
template<short BPSK_MOD = 10720>
class T11aAddPilot
{
public:
template<TFILTER_ARGS>
class Filter : public TFilter<TFILTER_PARAMS>
{
private:
    uchar m_PilotIndex;

    void _init () {
        m_PilotIndex = 127;
    }
public:
    DEFINE_IPORT(COMPLEX16, 48);
    DEFINE_OPORT(COMPLEX16, 64);
    
public:
    REFERENCE_LOCAL_CONTEXT(Filter);
    STD_TFILTER_CONSTRUCTOR(Filter)
    { 
        // The output symbols contain data subcarrriers, pilot subcarriers, and zero subcarriers.
        // Here we prepare the output pin queue with zero buffer, to make symbol composition easier.
        opin().zerobuf();

        _init ();
    }
    STD_TFILTER_RESET() { _init ();  }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const COMPLEX16 *in  = ipin.peek();
            COMPLEX16 *out = opin().append();

            add_pilot (out, in);
            m_PilotIndex++;
            if (m_PilotIndex >= 127)
                m_PilotIndex = 0;

            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }
	
	FINL
	void add_pilot (COMPLEX16* out, const COMPLEX16* in ) {
		ulong i;
		for (i = 64 - 26; i < 64; i++)
		{
			if (i == 64 - 7 || i == 64 - 21)
				continue;
			out[i] = *in; in ++;
		}
		
		for (i = 1; i <= 26; i++)
		{
			if (i == 7 || i == 21)	continue;
			out[i] = *in; in ++;
		}
		
		if (!PilotSgn[m_PilotIndex])
		{
			out[7].re     =  BPSK_MOD; out[7].im = 0;
			out[21].re    = -BPSK_MOD; out[21].im = 0;
			out[64-7].re  =  BPSK_MOD; out[64-7].im = 0;
			out[64-21].re =  BPSK_MOD; out[64-21].im = 0;
		}
		else
		{		  
			out[7].re     = - BPSK_MOD; out[7].im = 0;
			out[21].re    =   BPSK_MOD; out[21].im = 0;
			out[64-7].re  = - BPSK_MOD; out[64-7].im = 0;
			out[64-21].re = - BPSK_MOD; out[64-21].im = 0;
		}

        //printf ( "add pilot\n" );
        //for ( int i=0; i< 16; i++ ) {
        //	for (int j=0; j<4; j++ ) {
        //		printf ( "<%d, %d> ", out[i*4+j].re, out[i*4+j].im );
        //	}
        //	printf ( "\n" );
        //}
    }
}; };

// pilot freq compensation on 64 complexes
DEFINE_LOCAL_CONTEXT(TPilotTrack, CF_PhaseCompensate, CF_PilotTrack, CF_11aRxVector);
template<TFILTER_ARGS>
class TPilotTrack : public TFilter<TFILTER_PARAMS>
{
private:
    CTX_VAR_RW (FP_RAD, CFO_tracker);
    CTX_VAR_RW (FP_RAD, SFO_tracker);
    
    CTX_VAR_RW (FP_RAD, CFO_comp );
    CTX_VAR_RW (FP_RAD, SFO_comp );
    CTX_VAR_RW (vcs, CompCoeffs, [16] );

    CTX_VAR_RW (ulong,	symbol_count);

protected:	
    FINL 
    void _rotate ( vcs * dst, vcs * src, vcs * coeffs ) {
        rep_mul<7> (dst, src, coeffs );
        rep_mul<7> (dst+9, src+9, coeffs+9 );
    }

    FINL 
    void _build_coeff ( COMPLEX16* pcoeffs, FP_RAD ave, FP_RAD delta ) {
        FP_RAD th = ave - delta * 26;
        int i;
        for (i = 64-26; i < 64; i++)
        {
            pcoeffs[i].re = ucos(th);
            pcoeffs[i].im = -usin(th);			

            th += delta;
        }
        
        th += delta; // 0 subcarrier, dc
        
        // subcarrier 1 - 26
        for (i = 1; i <= 26; i++)
        {
            pcoeffs[i].re = ucos(th);
            pcoeffs[i].im = -usin(th);			

            th += delta;
        }

        
    }
    
    FINL
    void _pilot_track ( vcs * input, vcs * output )
    {

        COMPLEX16* pc = (COMPLEX16*) input;
        //
        FP_RAD th1 = uatan2 (  pc[64-21].im, pc[64-21].re );
        FP_RAD th2 = uatan2 (  pc[64-7].im,  pc[64-7].re  );		
        FP_RAD th3 = uatan2 (  pc[7].im,  	 pc[7].re     );
        FP_RAD th4 = uatan2 ( -pc[21].im,   -pc[21].re    );
    
        if (PilotSgn[symbol_count]) {
            th1 += FP_PI;
            th2 += FP_PI;
            th3 += FP_PI;
            th4 += FP_PI;
        }	

        symbol_count++;
        if (symbol_count >= 127) symbol_count = 0; 
        
        _dump_text ( "Pilots: <%d,%d> %d <%d,%d> %d <%d,%d> %d <%d,%d> %d \n\n", 
                    pc[64-21].re, pc[64-21].im, th1, 
                    pc[64-7].re, pc[64-7].im, th2, 
                    pc[7].re,    pc[7].im, th3, 
                    pc[21].re,   pc[21].im, th4 );
		

        // subcarrier rotation = const_rotate + i * delta_rotate
        // estimate the const part
        FP_RAD avgTheta = (th1 + th2 + th3 + th4) / 4;

        // estimate the delta part
        FP_RAD delTheta = ((th3 - th1) / (21 + 7) 
                + (th4 - th2) / (21 + 7)) >> 1;

        // pilot rotate
        vcs rotate_coeffs[16];
        _build_coeff ( (COMPLEX16*) rotate_coeffs, avgTheta, delTheta );
        _rotate ( output, input, rotate_coeffs );

        // debug
        _dump_symbol<64>( "after pilot", (COMPLEX16*) output);
		
        
        // update tracker
        CFO_tracker += avgTheta >> 2;
        SFO_tracker += delTheta >> 2;

        
        CFO_comp += avgTheta + (CFO_tracker );
        SFO_comp += delTheta + (SFO_tracker );

        
        // Debug
        _dump_text ( "Tracker:: avg %lf del %lf CFO %lf SFO %lf\nFreqComp:: CFO %lf SFO %lf\n", 
         			fprad2rad (avgTheta),
         			fprad2rad (delTheta),	    			
         			fprad2rad (CFO_tracker),
         			fprad2rad (SFO_tracker), 
         			fprad2rad (CFO_comp),
         			fprad2rad (SFO_comp)
        );
        
        _build_coeff ( (COMPLEX16*) CompCoeffs, CFO_comp, SFO_comp );
    }

public:
    DEFINE_IPORT(COMPLEX16, 64);
    DEFINE_OPORT(COMPLEX16, 64);
    
public:
    REFERENCE_LOCAL_CONTEXT(TPilotTrack);
    STD_TFILTER_CONSTRUCTOR(TPilotTrack)
        BIND_CONTEXT (CF_PilotTrack::CFO_tracker, CFO_tracker)
        BIND_CONTEXT (CF_PilotTrack::SFO_tracker, SFO_tracker)		
        BIND_CONTEXT (CF_PilotTrack::symbol_count, symbol_count)		
        BIND_CONTEXT (CF_PhaseCompensate::CFO_comp,      CFO_comp)
        BIND_CONTEXT (CF_PhaseCompensate::SFO_comp,      SFO_comp)		
        BIND_CONTEXT (CF_PhaseCompensate::CompCoeffs,  CompCoeffs)		
    { }
    
    STD_TFILTER_RESET() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            vcs *input  = (vcs*)ipin.peek();
            vcs *output = (vcs*)opin().append();

//            _dump_symbol<64>("before pilot\n", (COMPLEX16*) input);

            _pilot_track ( input, output );
            
            ipin.pop();
            bool rc = Next()->Process(opin());
            if (!rc) return false;
        }
        return true;
    }
};

