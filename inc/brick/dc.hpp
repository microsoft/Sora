#pragma once

//
// TDCRemove: substract a common DC from the sample stream
//
DEFINE_LOCAL_CONTEXT(TDCRemove, CF_VecDC);
template<TFILTER_ARGS>
class TDCRemove : public TFilter<TFILTER_PARAMS>
{
	CTX_VAR_RO (vcs, DC );

public:
	DEFINE_IPORT(COMPLEX16, 28);
	DEFINE_OPORT(COMPLEX16, 4);

public:
	REFERENCE_LOCAL_CONTEXT(TDCRemove );
	
	STD_TFILTER_CONSTRUCTOR(TDCRemove )
		BIND_CONTEXT(CF_VecDC::direct_current, DC)
	{ }

	STD_TFILTER_RESET() {}
	
	STD_TFILTER_FLUSH() {}

	BOOL_FUNC_PROCESS(ipin)
	{
		// Marker::TDCRemove
		while (ipin.check_read())
		{
			const vcs * pi = reinterpret_cast<const vcs*> (ipin.peek());
			for ( int i=0; i<7; i++ ) {
				vcs& out = cast_ref<vcs>(opin().append());
				out = sub (pi[i], DC);
				Next()->Process(opin());
			}
			ipin.pop();
			
			
		}
		return true;
	}
};

DEFINE_LOCAL_CONTEXT(TDCRemoveEx, CF_VecDC);
template<int N_IN> 
class TDCRemoveEx {
public:
template<TFILTER_ARGS>
class Filter : public TFilter<TFILTER_PARAMS>
{
	CTX_VAR_RO (vcs, DC );

public:
	DEFINE_IPORT(COMPLEX16, N_IN);
	DEFINE_OPORT(COMPLEX16, 4);

public:
	REFERENCE_LOCAL_CONTEXT(TDCRemoveEx );
	
	STD_TFILTER_CONSTRUCTOR(Filter )
		BIND_CONTEXT(CF_VecDC::direct_current, DC)
	{ }

	STD_TFILTER_RESET() {}
	
	STD_TFILTER_FLUSH() {}

	BOOL_FUNC_PROCESS(ipin)
	{
		// Marker::TDCRemove
		while (ipin.check_read())
		{
			const vcs * pi = reinterpret_cast<const vcs*> (ipin.peek());
			vcs * po = reinterpret_cast<vcs*> (opin().append());
			rep<N_IN/4>::vsub (po, pi, DC );
			ipin.pop();			

			Next()->Process(opin());

		}
		return true;
	}
};
};


//  --->TDCRemove--->TDCEstimator--->
//          ^           |
//          |___________|
//
// From this structure, you can see that TDCEstimator is not estimating the real DC but Residual DC (sum_dc).
// Let DC is the estimation, and DC_actual is the true DC. Then
//      DC_actual = DC + sum_dc.
// The Residual DC is dampped with 1/4 in feedback loop. That's how the equation comes.
//      DC = DC + sum_dc / 4
//
DEFINE_LOCAL_CONTEXT(TDCEstimator, CF_VecDC);
template<TFILTER_ARGS>
class TDCEstimator : public TFilter<TFILTER_PARAMS>
{
static const uint dc_update_interval = 8; // 32 samples; roughly 1us
private:
	CTX_VAR_RW (vcs, DC );

protected:
	uint 		update_cnt; 
	vcs     	sum_dc;
public:
	DEFINE_IPORT(COMPLEX16, 4);
	DEFINE_OPORT(COMPLEX16, 4);

protected:
	FINL void __init () {
		update_cnt = dc_update_interval;
		set_zero (sum_dc );
	}
	
public:
	REFERENCE_LOCAL_CONTEXT(TDCEstimator );
	
	STD_TFILTER_CONSTRUCTOR(TDCEstimator )
		BIND_CONTEXT(CF_VecDC::direct_current, DC)
	{ __init (); }

	STD_TFILTER_RESET() { 
		__init ();
	}
	
	STD_TFILTER_FLUSH() {}

	BOOL_FUNC_PROCESS(ipin)
	{
		while (ipin.check_read())
		{
			const vcs& pi = cast_ref<const vcs>(ipin.peek());

			// average over 4 samples
			sum_dc = add (sum_dc, hadd (shift_right(pi,5)));

			if ( update_cnt == 0) {
				//
				// update the dc estimation
				// the delta is averaged over 8 vector samples and 
				// damped further by a factor of 1/4 
				//
				DC = add ( DC, shift_right(sum_dc, 2));

				update_cnt = dc_update_interval;
				set_zero (sum_dc);
			} 

			update_cnt--;

			vcs& po = cast_ref<vcs> (opin().append());
			po = pi;
						
			ipin.pop();
			
			Next()->Process(opin());
		}
		return true;
	}
};

