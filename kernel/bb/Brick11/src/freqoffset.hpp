#pragma once

#include <sora.h>
#include <dspcomm.h>
#include "ieee80211facade.hpp"
#include "operator_repeater.h"


/***********************************************
 TPhaseCompensate -- 
 TPhaseCompensate compensates phase distortion
 of the incoming symobl
************************************************/
DEFINE_LOCAL_CONTEXT(TPhaseCompensate, CF_PhaseCompensate, CF_Error);
template<TFILTER_ARGS>
class TPhaseCompensate : public TFilter<TFILTER_PARAMS>
{
static const int sample_per_sym = 80;
private:
    CTX_VAR_RW (FP_RAD, CFO_comp );
    CTX_VAR_RW (FP_RAD, SFO_comp );
	CTX_VAR_RW (vcs, CompCoeffs, [16] );

	CTX_VAR_RW (ulong,  error_code );

protected:
	FINL
	void _phase_compensate ( vcs* input, vcs* output ) {
        rep_mul<16>(output, input, CompCoeffs);
	}
	
public:
	DEFINE_IPORT(COMPLEX16, 64);
    DEFINE_OPORT(COMPLEX16, 64);

	
public:
	REFERENCE_LOCAL_CONTEXT(TPhaseCompensate );
	
	STD_TFILTER_CONSTRUCTOR(TPhaseCompensate )
		BIND_CONTEXT (CF_PhaseCompensate::CFO_comp,      CFO_comp)
		BIND_CONTEXT (CF_PhaseCompensate::SFO_comp,      SFO_comp)		
		BIND_CONTEXT (CF_PhaseCompensate::CompCoeffs,  CompCoeffs)		
		BIND_CONTEXT(CF_Error::error_code, 		  error_code)		
	{ }

	STD_TFILTER_RESET() { }
	
	BOOL_FUNC_PROCESS (ipin)
	{
		while (ipin.check_read())
		{
			vcs* pi = (vcs*)(ipin.peek ());
			vcs* po = (vcs*)(opin().append());
			
			_phase_compensate (pi, po);

//            _dump_symbol<64> ( "After phase compensate", (COMPLEX16*) po);

			ipin.pop();
			Next()->Process (opin());
		}
		return true;
	}
};


