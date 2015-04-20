#pragma once

/*****************************************************
 TChannelEst --
 TChannelEst takes a sequence of time-domain 11a LTS, 
 estimate channel on the first LTS, and pass through
 the reset LTS down stream
******************************************************/
//
// The LTS sequence as defined in 802.11a
// the sequence is presented in frequency domain
//
static const char LTS_Sequence_11a[64] = {
	0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 
	1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 
	1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1
};

//
// T11aLTS - Processing 11a Long Training Symbols
//
DEFINE_LOCAL_CONTEXT(T11aLTS,
					 CF_VecDC, 
                     CF_Channel_11a, 
                     CF_11aSymState, 
                     CF_CFOffset, 
                     CF_PilotTrack, 
                     CF_PhaseCompensate, 
                     CF_FreqCompensate,
                     CF_Error);

template<TSINK_ARGS>
class T11aLTS: public TSink<TSINK_PARAMS>
{
private:
static const int pre_read   = 16; //16;
static const int skip_cp    = 8;
static const int norm_one	= 1600;
static const int norm_shift = 8;

public:
	DEFINE_IPORT(COMPLEX16, 160 - pre_read); 	

private:
	CTX_VAR_RW (ulong, symbol_type ); 
	// DC
	CTX_VAR_RO (vcs, direct_current );
	// Channel estimation
	CTX_VAR_RW (vcs, ChannelCoeffs, [16] );
	// CFO estimation
	CTX_VAR_RW (FP_RAD, CFO_est );
	// frequency compensation
	CTX_VAR_RW (vcs, FreqCoeffs, [16] );
	// phase compensation
    CTX_VAR_RW (FP_RAD, CFO_comp );
    CTX_VAR_RW (FP_RAD, SFO_comp );
	CTX_VAR_RW (vcs, CompCoeffs, [16] );
	// pilot tracker
    CTX_VAR_RW (FP_RAD, CFO_tracker );
    CTX_VAR_RW (FP_RAD, SFO_tracker );
	// error
	CTX_VAR_RW (ulong, error_code );

protected:
	vcs LTS_seq[16];

	// build up the LTS LUT
	// 1 maps to norm_one
	// 0 maps to - norm_one
	FINL void __init_lut () {
		for (int i=0; i<64; i++) {
			if ( LTS_Sequence_11a[i] ) {
				LTS_seq[i/4][i%4].re = norm_one;
			} else {
				LTS_seq[i/4][i%4].re = -norm_one;
			}

			LTS_seq[i/4][i%4].im = 0;
		}
	}

protected:
	FINL void __init () {
	}

protected:
	FINL
	void _fine_grained_frequency_estimation ( vcs* input )
	{
		// vcs temp[128];
		// coarse frequency compensation to a temp buffer
		// Kun: disabled
		// FrequencyShift<128/vcs::size>(temp, input, 0, CFO_est );

		// Fine grained frequency estimation
		FP_RAD arg = FreqOffsetEstimate<64/vcs::size> (input, input+16);

        _dump_text ( "CFO estimated %d\n", arg );        

        // adjust the CFO estimation
		CFO_est = arg; 
	
		// set the phase compensate parameters
//		CFO_comp = CFO_est * 144;

		CFO_comp = 0;
		SFO_comp = 0;

//		CFO_tracker = CFO_est * 80;
		CFO_tracker = 0;
		SFO_tracker = 0;

		// fine grained frequency compensation
		// setup the frequency compensation ceofficients
		BuildFrequencyShiftCoeffs<64> ((COMPLEX16*) FreqCoeffs, 0, CFO_est );
		_dump_symbol<64>("freq comp coeffs", (COMPLEX16*) FreqCoeffs );
		
		// frequency shift LTS; prepare for channel estimation
		FrequencyShift<64/vcs::size> ( input, input,  FreqCoeffs );
        _dump_symbol<64>("After Freq comp", (COMPLEX16*) input );
        
	}

	FINL
	void _channel_estimation ( vcs* input ) {
		vcs fft_out[16];
		
		// FFT first
		FFT<64>(input, fft_out);
        _dump_symbol<64>("After FFT", (COMPLEX16*) fft_out );
        
		// positive subcarriers
		vi re, im;
		vi rre, rim;
			
		for (int i=0; i<7; i++) {
			vi e = shift_right (SquaredNorm (fft_out[i]), norm_shift); 

			// compute the 1/h
			conj_mul ( re, im, LTS_seq[i], fft_out[i] ); 

			// normalization
			for ( int j=0; j<4; j++ ) {
				if ( e[j] != 0 ) {
					rre[j] = re[j] / e[j];
					rim[j] = im[j] / e[j];					
				} else {
					rre[j] = 0;
					rim[j] = 0;
				}
			}

			pack(ChannelCoeffs[i], rre, rim );
		}

		// negative subcarriers
		for (int i=9; i<16; i++) {
			vi e = shift_right (SquaredNorm (fft_out[i]), norm_shift); 

			// compute the 1/h
			conj_mul ( re, im, LTS_seq[i], fft_out[i] ); 

			// normalization
			for ( int j=0; j<4; j++ ) {
				if ( e[j] != 0 ) {
					rre[j] = re[j] / e[j];
					rim[j] = im[j] / e[j];					
				} else {
					rre[j] = 0;
					rim[j] = 0;
				}	
			}

			pack (ChannelCoeffs[i], rre, rim );
		}
	
	}
	
public:
	REFERENCE_LOCAL_CONTEXT(T11aLTS);

	STD_TSINK_CONSTRUCTOR(T11aLTS)
		BIND_CONTEXT(CF_11aSymState::symbol_type, symbol_type )
		BIND_CONTEXT(CF_VecDC::direct_current, direct_current )		
		BIND_CONTEXT(CF_CFOffset::CFO_est,        CFO_est)		
		BIND_CONTEXT(CF_FreqCompensate::Coeffs,   FreqCoeffs)				
		BIND_CONTEXT(CF_PhaseCompensate::CFO_comp, CFO_comp)
		BIND_CONTEXT(CF_PhaseCompensate::SFO_comp, SFO_comp)		
		BIND_CONTEXT(CF_PhaseCompensate::CompCoeffs,  CompCoeffs)		
		BIND_CONTEXT(CF_PilotTrack::CFO_tracker,    CFO_tracker)
		BIND_CONTEXT(CF_PilotTrack::SFO_tracker,    SFO_tracker)		
		BIND_CONTEXT(CF_Channel_11a::ChannelCoeffs, ChannelCoeffs)
		BIND_CONTEXT(CF_Error::error_code,		  error_code)		
	{ 
		__init (); 
		__init_lut ();
	}

	STD_TSINK_RESET() { 
		__init ();
	}

	STD_TSINK_FLUSH() { }

	BOOL_FUNC_PROCESS (ipin)
	{
		while (ipin.check_read())
		{
			vcs * pvi = (vcs*) ipin.peek ();
			pvi += (skip_cp / vcs::size);

            _dump_text ( "dc <%d,%d>\n", direct_current[0].re, direct_current[0].im );
            _dump_symbol<128> ("LTS", (COMPLEX16*) pvi );

            rep<64/vcs::size>::vshift_right(pvi, 1);
			_fine_grained_frequency_estimation ( pvi );
			_channel_estimation ( pvi );	
            _dump_symbol<64> ("Channel", (COMPLEX16*) ChannelCoeffs );

			ipin.pop ();

			// we have done LTS
			// update the symbol state
			symbol_type = CF_11aSymState::SYMBOL_OFDM_DATA;
		}
		
		return true;
	};
};


DEFINE_LOCAL_CONTEXT(TChannelEst, CF_Channel_11a, CF_Error);
template<TFILTER_ARGS>
class TChannelEst : public TFilter<TFILTER_PARAMS>
{
private:
	static const int norm_one = 1600;
	static const int norm_shift = 8;

protected:
	vcs LTS_seq[16];
	bool est_ctrl;

private:	
	CTX_VAR_RW (vcs, ChannelCoeffs, [16] );

	CTX_VAR_RW (ulong, error_code );
	
	public:
		DEFINE_IPORT(COMPLEX16, 64); 	// 
		DEFINE_OPORT(COMPLEX16, 64); 	// passthru the sample downstream

	protected:
		FINL
		void _channel_est ( vcs* input ) {
			vcs fft_out[16];
			
			// FFT first
			FFT<64>(input, fft_out);
			// positive subcarriers
			vi re, im;
			vi rre, rim;

				
			for (int i=0; i<7; i++) {
				vi e = shift_right (SquaredNorm (fft_out[i]), norm_shift); 

				// compute the 1/h
				conj_mul ( re, im, LTS_seq[i], fft_out[i] ); 

				// normalization
				for ( int j=0; j<4; j++ ) {
					if ( e[j] != 0 ) {
						rre[j] = re[j] / e[j];
						rim[j] = im[j] / e[j];					
					} else {
						rre[j] = 0;
						rim[j] = 0;
					}
				}

				pack(ChannelCoeffs[i], rre, rim );
			}

			// negative subcarriers
			for (int i=9; i<16; i++) {
				vi e = shift_right (SquaredNorm (fft_out[i]), norm_shift); 

				// compute the 1/h
				conj_mul ( re, im, LTS_seq[i], fft_out[i] ); 

				// normalization
				for ( int j=0; j<4; j++ ) {
					if ( e[j] != 0 ) {
						rre[j] = re[j] / e[j];
						rim[j] = im[j] / e[j];					
					} else {
						rre[j] = 0;
						rim[j] = 0;
					}	
				}

				pack (ChannelCoeffs[i], rre, rim );
			}

#if 0
			// debug
			printf ( "LTS " );			
			for ( int i=0; i<64; i++) {
				if ( i%8 == 0 ) printf ("\n");				
				printf ( "<%d, %d> , ", fft_out[i/4][i%4].re, fft_out[i/4][i%4].im);
			}
			printf ( "\n" );

			printf ( "channels: " );			
			for ( int i=0; i<64; i++) {
				if ( i%8 == 0 ) printf ("\n");				
				printf ( "<%d, %d> , ", ChannelCoeffs[i/4][i%4].re, ChannelCoeffs[i/4][i%4].im);
			}
			printf ( "\n" );
#endif			
		}


	protected:
		FINL void __init () {
			est_ctrl = false;
		}

		// build up the LTS LUT
		// 1 maps to norm_one
		// 0 maps to - norm_one
		FINL void __init_lut () {
			for (int i=0; i<64; i++) {
				if ( LTS_Sequence_11a[i] ) {
					LTS_seq[i/4][i%4].re = norm_one;
				} else {
					LTS_seq[i/4][i%4].re = -norm_one;
				}

				LTS_seq[i/4][i%4].im = 0;
			}
		}
		
	public:
		REFERENCE_LOCAL_CONTEXT(TChannelEst );
		
		STD_TFILTER_CONSTRUCTOR(TChannelEst )
			BIND_CONTEXT(CF_Channel_11a::ChannelCoeffs, ChannelCoeffs)
			BIND_CONTEXT(CF_Error::error_code,		  error_code)		
		{ 
			__init ();
			__init_lut ();
		}
	
		STD_TFILTER_RESET() { 
			__init ();
		}
		
		STD_TFILTER_FLUSH() { }
	
		BOOL_FUNC_PROCESS (ipin)
		{
			while (ipin.check_read())
			{
				vcs* pi = (vcs*)ipin.peek ();
				vcs* po = (vcs*) opin().append();
				
				// passthru
				rep<16>::vmemcpy (po, pi);

				// only estimate channel on the first LTS
				// N.B. We need to copy samples downstream first, as
				// FFT operations used in _channel_est will destory
				// the input data
				if ( !est_ctrl ) {
					_channel_est ( pi );
					est_ctrl = true;
				}
				
				Next()->Process(opin());
				ipin.pop();
			}
			return true;
		}

};

/*****************************************************
 TFineCFOEst --
 TFineCFOEst takes two time-domain 11a LTSs, and 
 perform fine grained CFO estimation.
 The original samples are passed through to downstream
 bricks
******************************************************/
DEFINE_LOCAL_CONTEXT(TFineCFOEst, CF_CFOffset, CF_PilotTrack, CF_PhaseCompensate, CF_Error);
template<TFILTER_ARGS>
class TFineCFOEst : public TFilter<TFILTER_PARAMS>
{
private:	
	CTX_VAR_RW (FP_RAD, CFO_est );
	
    CTX_VAR_RW (FP_RAD, CFO_comp );
    CTX_VAR_RW (FP_RAD, SFO_comp );
	CTX_VAR_RW (vcs, CompCoeffs, [16] );

    CTX_VAR_RW (FP_RAD, CFO_tracker );
    CTX_VAR_RW (FP_RAD, SFO_tracker );

	CTX_VAR_RW (ulong, error_code   );
	
public:
	DEFINE_IPORT(COMPLEX16, 128); 	// 
	DEFINE_OPORT(COMPLEX16, 128); 	// passthru the sample downstream

protected:
	FINL
	void _coarse_cfo_comp ( vcs* input ) {
		vcs compCoeffs[32];
		COMPLEX16* pCoeffs = (COMPLEX16*) compCoeffs;
		FP_RAD delta = 0;
		for (int i=0; i<128; i++) {
			pCoeffs[i].re = ucos  (delta);
			pCoeffs[i].im = -usin (delta);
			delta += CFO_est;
		}

		rep<32>::vmul ( input, input, compCoeffs );	
	}

	FINL
	void _build_phase_coeff ( COMPLEX16* pcoeffs, FP_RAD ave ) {
		FP_RAD th = ave;
		int i;
		for (i = 0; i < 64; i++)
		{
			pcoeffs[i].re =  ucos(th);
			pcoeffs[i].im = -usin(th);			
		}
	}
		
	FINL
	void _fine_cfo_est ( vcs* input ) {
		vi re, im;
		int sum_re = 0;
		int sum_im = 0;
			
		for (int i=0; i<16; i++) {
			conj_mul (re, im, input[i+16], input[i] );

			vi sr = hadd (shift_right(re, 6));
			vi si = hadd (shift_right(im, 6));

			sum_re += sr[0]; sum_im += si[0];
		}

		FP_RAD arg = uatan2 (sum_im, sum_re);
		arg >>= 6;

		CFO_est += arg; // adjust the CFO estimation
	
		// set the phase compensate parameters
		CFO_comp = CFO_est * 144;
		SFO_comp = 0;

		CFO_tracker = CFO_est * 80;
		SFO_tracker = 0;
		
		//printf ( "Fine CFO estimation: %lf (%lf) CFO comp %lf CFO tracker % lf\n", 
		//		fprad2rad(CFO_est),
		//		fprad2rad(arg),
		//		fprad2rad(CFO_comp),
		//		fprad2rad(CFO_tracker) );

		_build_phase_coeff ( (COMPLEX16*) CompCoeffs, CFO_comp );
	}
	
protected:
	FINL void __init () {
	}

public:
	REFERENCE_LOCAL_CONTEXT(TFineCFOEst );
	
	STD_TFILTER_CONSTRUCTOR(TFineCFOEst )
		BIND_CONTEXT (CF_CFOffset::CFO_est,              CFO_est)		
		BIND_CONTEXT (CF_PhaseCompensate::CFO_comp,      CFO_comp)
		BIND_CONTEXT (CF_PhaseCompensate::SFO_comp,      SFO_comp)		
		BIND_CONTEXT (CF_PhaseCompensate::CompCoeffs,  CompCoeffs)		
		BIND_CONTEXT (CF_PilotTrack::CFO_tracker,    CFO_tracker)
		BIND_CONTEXT (CF_PilotTrack::SFO_tracker,    SFO_tracker)		
		BIND_CONTEXT(CF_Error::error_code,		      error_code)		
	{ 
		__init ();
	}
	
	STD_TFILTER_RESET() { 
		__init ();
	}
		
	STD_TFILTER_FLUSH() { }
	
	BOOL_FUNC_PROCESS (ipin)
	{
		while (ipin.check_read())
		{
			vcs* pi = (vcs*)ipin.peek ();

			// compensate coarse CFO first 
			_coarse_cfo_comp (pi);
				
			// fine CFO estimation
			_fine_cfo_est ( pi );
			vcs* po = (vcs*) opin().append();

			rep<32>::vmemcpy (po, pi);
			Next()->Process(opin());
			ipin.pop();
		}
		return true;
	}
};

/*****************************************************
 TChannelEqualization --
 TChannelEqualization performs equalization on the 
 incoming data symbol. The equalized results are
 passed downstream.
******************************************************/

DEFINE_LOCAL_CONTEXT(TChannelEqualization, CF_Channel_11a, CF_Error);
template<TFILTER_ARGS>
class TChannelEqualization : public TFilter<TFILTER_PARAMS>
{
static const int norm_shift = 8;
	
private:	
	CTX_VAR_RW (vcs, ChannelCoeffs, [16] );
	CTX_VAR_RW (ulong, error_code );
	
public:
	DEFINE_IPORT(COMPLEX16, 64); 	// 
	DEFINE_OPORT(COMPLEX16, 64); 	// passthru the sample downstream

protected:
	FINL
	void _channel_equalize ( vcs* input, vcs* output ) {
		vi re, im;
		vi rre, rim;

        set_zero(output[7]);
        set_zero(output[8]);
		
		// positive subcarriers				
		for (int i=0; i<7; i++) {
			mul (re, im, input[i], ChannelCoeffs[i] );

			re = shift_right (re, norm_shift);
			im = shift_right (im, norm_shift);

			pack (output[i], re, im );
		}

		// negative subcarriers
		for (int i=9; i<16; i++) {
			mul (re, im, input[i], ChannelCoeffs[i] );

			re = shift_right (re, norm_shift);
			im = shift_right (im, norm_shift);

			pack (output[i], re, im );
		}
	}
		
public:
	REFERENCE_LOCAL_CONTEXT(TChannelEqualization);
	
	STD_TFILTER_CONSTRUCTOR(TChannelEqualization)
		BIND_CONTEXT(CF_Channel_11a::ChannelCoeffs, ChannelCoeffs)
		BIND_CONTEXT(CF_Error::error_code,		  error_code)		
	{ }
	
	STD_TFILTER_RESET() { }
	
	BOOL_FUNC_PROCESS (ipin)
	{
		while (ipin.check_read())
		{
			vcs* pi = (vcs*)ipin.peek();
			vcs* po = (vcs*)opin().append();

//            _dump_symbol<64> ( "before equalization", (COMPLEX16*) pi);
            
			_channel_equalize ( pi, po );

            _dump_symbol<64> ( "After equalization",  (COMPLEX16*) po);

			Next()->Process(opin());
			ipin.pop();
		}
		return true;
	}
};

/*****************************************************
 TFreqCompensation --
 TFreqCompensation compensates the frequency offset of
 the incoming symbol to remove ICI
******************************************************/

DEFINE_LOCAL_CONTEXT(TFreqCompensation, CF_FreqCompensate, CF_Error);
template<TFILTER_ARGS>
class TFreqCompensation : public TFilter<TFILTER_PARAMS>
{
	
private:	
	CTX_VAR_RO (vcs, FreqCoeffs, [16] );
	CTX_VAR_RW (ulong, error_code );

public:
	DEFINE_IPORT(COMPLEX16, 64); 	// 
	DEFINE_OPORT(COMPLEX16, 64); 	// passthru the sample downstream

		
public:
	REFERENCE_LOCAL_CONTEXT(TFreqCompensation);
	
	STD_TFILTER_CONSTRUCTOR(TFreqCompensation)
		BIND_CONTEXT(CF_FreqCompensate::Coeffs, FreqCoeffs)
		BIND_CONTEXT(CF_Error::error_code, error_code)		
	{ }
	
	STD_TFILTER_RESET() { }
	
	BOOL_FUNC_PROCESS (ipin)
	{
		while (ipin.check_read())
		{
			vcs* pi = (vcs*)ipin.peek ();
			vcs* po = (vcs*) opin().append();

            rep<64/vcs::size>::vshift_right(pi, 1);
			FrequencyShift<64/vcs::size> ( po, pi, FreqCoeffs );
//			_dump_symbol<64>("freq comp coeffs", (COMPLEX16*) FreqCoeffs );			
			_dump_symbol<64> ("After freq compensation", (COMPLEX16*) po);

			Next()->Process(opin());
			ipin.pop();
		}
		return true;
	}
};

