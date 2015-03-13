#pragma once

DEFINE_LOCAL_CONTEXT(TAvePower, CF_AvePower);
template<TFILTER_ARGS>
class TAvePower : public TFilter<TFILTER_PARAMS>
{
	CTX_VAR_RW (uint, average_power );

public:
	DEFINE_IPORT(COMPLEX16, 4);
	DEFINE_OPORT(COMPLEX16, 4);

protected:
	FINL void __init () {
		average_power = 0;
	}
public:
	REFERENCE_LOCAL_CONTEXT(TAvePower );
	
	STD_TFILTER_CONSTRUCTOR(TAvePower )
		BIND_CONTEXT(CF_AvePower::average_power, average_power)
	{ __init (); }

	STD_TFILTER_RESET() { 
		__init ();
	}
	
	STD_TFILTER_FLUSH() { }

	BOOL_FUNC_PROCESS(ipin)
	{
		while (ipin.check_read())
		{
			vcs * pi = (vcs*) ipin.peek ();

			// computer the power average
			vi power = shift_right(SquaredNorm (*pi), 2);
			uint ave_power = (uint) hadd(power)[0];

			average_power = average_power - (average_power>>3) + (ave_power>>3); 
			
			//
			//  pass to downstream
			COMPLEX16 * po = opin().append();
			rep<1>::vmemcpy((vcs*)po, pi);
			ipin.pop();
			
			Next()->Process(opin());

		}
		return true;
	}
};

