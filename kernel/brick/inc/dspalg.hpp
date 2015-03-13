#pragma once
#include <tpltrick.h>
#include <sequence.h>

template<typename T_TYPE, size_t T_SIZE>
class CMovingWindow {
	T_TYPE m_elements[T_SIZE];
	int    m_index;

	FINL void _increase_index () {
		m_index ++; 
		if (m_index>=T_SIZE) m_index = 0;
	}
	
public:
static const size_t size = T_SIZE;
	CMovingWindow () { Clear (); }
    
	FINL 
	void Clear () {
		memset (m_elements, 0, sizeof(m_elements));
		m_index = 0;
	}

	// basic operation
	FINL
	void Add ( const T_TYPE& datum ) {
		m_elements[m_index] = datum;
		_increase_index ();
	}

	FINL
	T_TYPE& First () {
		return m_elements[m_index];
	}
	
	// operators
/*	FINL
	T_TYPE& operator[] (size_t idx) {
		assert ((idx >= 0) && (idx <T_SIZE));
		return (m_elements[idx]);
	}
	
    FINL 
	const T_TYPE& operator[](size_t idx) const { 
		assert ((idx >= 0) && (idx <T_SIZE));
		return m_elements[idx]; 
	}	            
  */
    FINL   
    operator T_TYPE* () {
        return m_elements;
    }

	FINL
	int& GetIndex () { return m_index; }
	FINL
	CMovingWindow<T_TYPE, T_SIZE>& operator<< (const T_TYPE& datum) {
		Add (datum);
        return *this;
	}
	
};


//
// CAccumulator
//
template<typename T_TYPE, int T_SIZE>
class CAccumulator : public CMovingWindow<T_TYPE, T_SIZE>{
	T_TYPE m_reg; // the accumulator register
public:
	CAccumulator () {
		Clear ();
	}
	FINL 
	void Clear () {
        CMovingWindow<T_TYPE, T_SIZE>::Clear();        		

        memset (&m_reg, 0, sizeof(m_reg));
	}
    
    FINL
    T_TYPE& Register () { return m_reg; }

    FINL 
    void Accumulate ( const T_TYPE & datum ) {
        m_reg = m_reg + datum - First();
        Add ( datum );
    }
    
    FINL
    CAccumulator<T_TYPE, T_SIZE>& operator<< (const T_TYPE& datum ) {
        Accumulate ( datum );
        return (*this);
    }

};


//
// CAutoCorrelator:: fast auto correlation
// N_PERIOD - must be a multiple of 4
//
template<int N_PERIOD, int NORM_SHIFT1 = Log2<N_PERIOD>::value+1>
class CAutoCorrelator {
static const int WND = N_PERIOD / 4;

protected:
	CMovingWindow<vcs, WND> sample_his;

    CAccumulator<int, WND> ac_re_his;
    CAccumulator<int, WND> ac_im_his;
    CAccumulator<int, WND> ac_norm_his;

	void __init () {
        sample_his.Clear ();

        ac_re_his.Clear ();
        ac_im_his.Clear ();
        ac_norm_his.Clear ();
	}

public:
	CAutoCorrelator () { __init (); }
	void Reset () { __init (); }

	vcs* GetSampleWindow ( int& index ) {
		index = sample_his.GetIndex(); 
		return (vcs*) sample_his; 
	}

    FINL
	int AutoCorrelate ( vcs& input, int& corr_re, int& corr_im) {
		vi re, im;
		conj_mul ( re, im, input, sample_his.First() );

		// sum re and im
		//
		vi sum_r = hadd (shift_right(re, NORM_SHIFT1 ));
		vi sum_i = hadd (shift_right(im, NORM_SHIFT1 ));
		
		ac_re_his << sum_r[0];
		ac_im_his << sum_i[0];

		corr_re = ac_re_his.Register(); 
		corr_im	= ac_im_his.Register(); 

		vi re2 = re >> 16; re2 *= re2;
		vi im2 = im >> 16; im2 *= im2;
		vi sum = (hadd (re2 + im2)) >> NORM_SHIFT1;

	
		ac_norm_his << sum[0];
		sample_his << input;

		return ac_norm_his.Register();
	}

};

//
// CEnergyIntegrator:: fast integration of energy 
// N_PERIOD - must be a multiple of 4
//
template<int N_PERIOD, int NORM_SHIFT1 = Log2<N_PERIOD>::value+1>
class CEnergyIntegrator {
static const int WND = N_PERIOD / 4;
protected:
    CAccumulator<int, WND> energy_his;
    CAccumulator<int, WND> energy_sq_his; // square of the energy

	void __init () {
        energy_his.Clear ();
        energy_sq_his.Clear();
	}

public:
	CEnergyIntegrator () { __init (); }
	void Reset () { __init (); }

	FINL
	int Integrate ( vcs& input, int& esquare ) {
		vi e  = SquaredNorm (input); 
		vi e2 = (e >> 16);
		e2 *= e2;
		
		vi sum  = hadd (e >> NORM_SHIFT1 );
		vi sum2 = hadd (e2 >> NORM_SHIFT1);
		
        energy_his << sum[0];
        energy_sq_his << sum2[0];

        esquare = energy_sq_his.Register ();
		return energy_his.Register();
	}
	
};

template<size_t N_SAMPLE>
FINL void BuildFrequencyShiftCoeffs ( COMPLEX16* pCoeffs, FP_RAD ph0, FP_RAD delta ) {
	FP_RAD ph = ph0;
	for (int i=0; i<N_SAMPLE; i++) {
		pCoeffs[i].re = ucos  (ph);
		pCoeffs[i].im = -usin (ph);
		ph += delta;
	}
}

template<size_t LEN>
FINL void FrequencyShift ( vcs* output, vcs* input, FP_RAD ph0, FP_RAD delta ) {
 	vcs coeffs[LEN];
	// build the coefficients
	BuildFrequencyShiftCoeffs<LEN*vcs::size> ( (COMPLEX16*) coeffs, ph0, delta );
	// frequency shift
	rep<LEN>::vmul ( output, input, coeffs ); 
}

template<size_t LEN>
FINL void FrequencyShift ( vcs* output, vcs* input, const vcs* coeffs) {
	// frequency shift
	rep<LEN>::vmul ( output, input, coeffs ); 

}

template<size_t LEN>
FINL FP_RAD FreqOffsetEstimate ( vcs* input1, vcs* input2 ) {
	vi re, im;
	int sum_re = 0;
	int sum_im = 0;
				
	for (int i=0; i<LEN; i++) {
		conj_mul (re, im, input2[i], input1[i] );

		vi sr = hadd (re >> 5);
		vi si = hadd (im >> 5);

		sum_re += sr[0]; sum_im += si[0];
	}
	
	FP_RAD arg = uatan2 (sum_im, sum_re);
	return (arg / (LEN*vcs::size));
}
