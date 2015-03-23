#pragma once

#include <sora.h>
#include <soratypes.h>
#include <complex.h>
#include <complex_ext.h>
#include <operator_repeater.h>
#include <vector128.h>
#include <const.h>

//
// Matrix Inverse: invert 2 2x2 matrix in-a-row. 
// Alg.           
// inv (M) = 1/det ( v22, -v12, -v21, v22)
// det = v11*v22 - v12*v21
//
FINL
void inverse ( const vcf& v11, const vcf& v12, const vcf& v21, const vcf& v22, 
			   vcf& ov11, vcf& ov12, vcf& ov21, vcf& ov22 )
{
	vcf ad = mul ( v11, v22 );
	vcf bc = mul ( v12, v21 );

	vcf det = sub (ad, bc);
	vf  n   = SquaredNorm ( det );

	// 
	vcf det_star = conj ( det );
	vcf r11 = mul ( v22, det_star );
	vcf r12 = mul ( neg (v12), det_star );
	vcf r21 = mul ( neg (v21), det_star );
	vcf r22 = mul ( v11, det_star );

	ov11 = (vcf)div ( (vf)r11, (vf) n );
	ov12 = (vcf)div ( (vf)r12, (vf) n );
	ov21 = (vcf)div ( (vf)r21, (vf) n );
	ov22 = (vcf)div ( (vf)r22, (vf) n );
}


FINL vcf& random_elem ( vcf& a) {
	for ( int i=0; i<vcf::size; i++ ) {
		a[i].re = 1.0f * rand() / RAND_MAX - 0.5f;
		a[i].im = 1.0f * rand() / RAND_MAX - 0.5f;
	}
	return a;	
}

//
// CSoraMatrix
//
template<typename T, int M, int N>
class CSoraMatrix {
public:
static const ULONG d_row = M;
static const ULONG d_col = N;

	T m_array[M][N]; // M row, N column

	typedef T DATA_TYPE;
	CSoraMatrix () {}
	CSoraMatrix (T* pArray) {
		memcpy ( m_array, pArray, M*N*sizeof(T));
	}

	ULONG GetM () { return d_row; }
	ULONG GetN () { return d_col; }

	CSoraMatrix (CSoraMatrix<T,M,N>& a) {
		memcpy ( m_array, a.m_array, M*N*sizeof(T));
	}

	T* operator[] (int index ) { return m_array[index]; }
	const T* operator[] (int index ) const { return m_array[index]; }

	CSoraMatrix& operator=( const CSoraMatrix<T,M,N>& a ) {
		memcpy ( m_array, a.m_array, M*N*sizeof(T));
		return (*this);
	}

	// generate a random matrix
	void Random () {
		for ( int i=0; i<d_row; i++) {
			for ( int j=0; j<d_col; j++) {
				random_elem (m_array[i][j]);
			}
		}
	}

    template<typename T_SCALE>
    CSoraMatrix& operator *=(const T_SCALE& scale)
    {
        rep_mul_scale<M*N>((T *)m_array, (const T *)m_array, scale);
        return *this;
    }

    template<typename T_SCALE>
    CSoraMatrix& operator /=(const T_SCALE& scale)
    {
        rep_div_scale<M*N>((T *)m_array, (const T *)m_array, scale);
        return *this;
    }
};

typedef CSoraMatrix<vcf,2,2> CMatCF2x2;
typedef CSoraMatrix<vcf,3,3> CMatCF3x3;
typedef CSoraMatrix<vcf,4,4> CMatCF4x4;

template<class RMAT, class MAT0, class MAT1>
inline RMAT& mul ( RMAT& r, MAT0& a, const MAT1& b ) {
	if ( RMAT::d_row != MAT0::d_row || RMAT::d_col != MAT1::d_col || MAT0::d_col != MAT1::d_row ) {
		printf ( "matrix multiple failes. Dimension mismached!\n" );
		return r;
	}

	RMAT::DATA_TYPE temp;
	for ( ULONG i=0; i<r.GetM(); i++) {
		for ( ULONG j=0; j<r.GetN(); j++) {
			set_zero (r[i][j]);
			for (ULONG k=0; k<a.GetN(); k++) {
				temp = mul (a[i][k], b[k][j] );
				r[i][j] = add (r[i][j], temp );
			}
		}
	}

	return r;
}

//
// Compute the inverse without normalization
// For small dimension matrix, compute inverse with adjugate matrices 
// 2x2
FINL
void inverse (CMatCF2x2& r, vf & det_norm, const CMatCF2x2& a) {
	vcf ad = a[0][0] * a[1][1];
	vcf bc = a[0][1] * a[1][0];

	vcf det  = ad - bc;
	det_norm = SquaredNorm ( det );

	vcf det_star = conj ( det );

	r[0][0] =   a[1][1] * det_star;
	r[0][1] = - a[0][1] * det_star;
	r[1][0] = - a[1][0] * det_star;
	r[1][1] =   a[0][0] * det_star; 
}

//3x3
FINL 
void inverse (CMatCF3x3& r, vf & det_norm, const CMatCF3x3& a) {
	vcf A00 = a[1][1]*a[2][2] - a[1][2]*a[2][1];
	vcf A01 = a[1][0]*a[2][2] - a[1][2]*a[2][0];
	vcf A02 = a[1][0]*a[2][1] - a[1][1]*a[2][0];

	vcf A10 = a[0][1]*a[2][2] - a[0][2]*a[2][1];
	vcf A11 = a[0][0]*a[2][2] - a[0][2]*a[2][0];
	vcf A12 = a[0][0]*a[2][1] - a[0][1]*a[2][0];

	vcf t0011 = a[0][0]*a[1][1];
	vcf t0112 = a[0][1]*a[1][2];
	vcf t0210 = a[0][2]*a[1][0];
	vcf t0012 = a[0][0]*a[1][2];
	vcf t0110 = a[0][1]*a[1][0];
	vcf t0211 = a[0][2]*a[1][1];

	vcf A20 = t0112 - t0211;
	vcf A21 = t0012 - t0210;
	vcf A22 = t0011 - t0110;

	vcf det = t0011 *a[2][2] + t0112*a[2][0] + t0210 *a[2][1] - 
		      t0012 *a[2][1] - t0110*a[2][2] - t0211*a[2][0];

	det_norm = SquaredNorm (det);
	vcf det_star = conj ( det );

	r[0][0] =   A00 * det_star;
	r[0][1] = - A10 * det_star;
	r[0][2] =   A20 * det_star;

	r[1][0] = - A01 * det_star;
	r[1][1] =   A11 * det_star;
	r[1][2] = - A21 * det_star;

	r[2][0] =   A02 * det_star;
	r[2][1] = - A12 * det_star;
	r[2][2] =   A22 * det_star;
}


// N.B. using inline instead of forceinline to avoid C1001 error
//
// 4x4
inline
void __adj_matrix_0 (vcf* fa, vcf* fb, const CMatCF4x4& a ) {
	fa[0] = a[0][0] * a[1][1] - a[0][1]*a[1][0];
	fa[1] = a[0][0] * a[1][2] - a[0][2]*a[1][0];
	fa[2] = a[0][0] * a[1][3] - a[0][3]*a[1][0];

	fa[3] = a[0][1] * a[1][2] - a[0][2]*a[1][1];
	fa[4] = a[0][1] * a[1][3] - a[0][3]*a[1][1];

	fa[5] = a[0][2] * a[1][3] - a[0][3]*a[1][2];

	fb[0] = a[2][0] * a[3][1] - a[2][1]*a[3][0];
	fb[1] = a[2][0] * a[3][2] - a[2][2]*a[3][0];
	fb[2] = a[2][0] * a[3][3] - a[2][3]*a[3][0];

	fb[3] = a[2][1] * a[3][2] - a[2][2]*a[3][1];
	fb[4] = a[2][1] * a[3][3] - a[2][3]*a[3][1];

	fb[5] = a[2][2] * a[3][3] - a[2][3]*a[3][2];
}

inline
void __det_matrix ( vcf* fa, vcf* fb, vcf & det ) {
	det = fa[0]*fb[5] - fa[1]*fb[4] + fa[2]*fb[3] + fa[3]*fb[2] - fa[4]*fb[1] + fa[5]*fb[0];
}

inline
void __inv_row0 ( vcf* t, vcf* fa, vcf* fb, const CMatCF4x4& a) {
	t[0] = (  a[1][1] * fb[5] - a[1][2]*fb[4] + a[1][3]*fb[3] ) ;
	t[1] = (- a[1][0] * fb[5] + a[1][2]*fb[2] - a[1][3]*fb[1] ) ;
	t[2] = (  a[1][0] * fb[4] - a[1][1]*fb[2] + a[1][3]*fb[0] ) ;
	t[3] = (- a[1][0] * fb[3] + a[1][1]*fb[1] - a[1][2]*fb[0] ) ;
}

inline
void __inv_row1 ( vcf* t, vcf* fa, vcf* fb, const CMatCF4x4& a) {
	t[0] = (- a[0][1] * fb[5] + a[0][2]*fb[4] - a[0][3]*fb[3] ) ;
	t[1] = (  a[0][0] * fb[5] - a[0][2]*fb[2] + a[0][3]*fb[1] ) ;
	t[2] = (- a[0][0] * fb[4] + a[0][1]*fb[2] - a[0][3]*fb[0] ) ;
	t[3] = (  a[0][0] * fb[3] - a[0][1]*fb[1] + a[0][2]*fb[0] ) ;
}

inline
void __inv_row2 ( vcf* t, vcf* fa, vcf* fb, const CMatCF4x4& a) {
	t[0] = (  a[3][1] * fa[5] - a[3][2]*fa[4] + a[3][3]*fa[3] );
	t[1] = (- a[3][0] * fa[5] + a[3][2]*fa[2] - a[3][3]*fa[1] );
	t[2] = (  a[3][0] * fa[4] - a[3][1]*fa[2] + a[3][3]*fa[0] );
	t[3] = (- a[3][0] * fa[3] + a[3][1]*fa[1] - a[3][2]*fa[0] );
}

inline
void __inv_row3 ( vcf* t, vcf* fa, vcf* fb, const CMatCF4x4& a) {
	t[0] = (- a[2][1] * fa[5] + a[2][2]*fa[4] - a[2][3]*fa[3] ) ;
	t[1] = (  a[2][0] * fa[5] - a[2][2]*fa[2] + a[2][3]*fa[1] ) ;
	t[2] = (- a[2][0] * fa[4] + a[2][1]*fa[2] - a[2][3]*fa[0] ) ;
	t[3] = (  a[2][0] * fa[3] - a[2][1]*fa[1] + a[2][2]*fa[0] ) ;
}

inline
void inverse (CMatCF4x4& r, vf & det_norm, const CMatCF4x4& a) {
	vcf fa[6];
	vcf fb[6];
	vcf t [4];
	
	__adj_matrix_0 ( fa, fb, a);

	vcf det;
	__det_matrix (fa, fb, det );

	det_norm = SquaredNorm (det);
	vcf det_star = conj (det);
	
	__inv_row0 (t, fa, fb, a);

	for ( int i=0; i<4; i++) {
		r[i][0] = t[i] * det_star;
	}
	
	__inv_row1 (t, fa, fb, a);	

	for ( int i=0; i<4; i++) {
		r[i][1] = t[i] * det_star;
	}

	__inv_row2 (t, fa, fb, a);	

	for ( int i=0; i<4; i++) {
		r[i][2] = t[i]* det_star;
	}

	__inv_row3 (t, fa, fb, a);	

	for ( int i=0; i<4; i++) {
		r[i][3] = t[i] * det_star;
	}
}

//
// Compute the inverse with normalization
template<int M>
FINL
CSoraMatrix<vcf,M,M>& inverse (CSoraMatrix<vcf,M,M>& r, const CSoraMatrix<vcf,M,M>& a) {
	vf n;
	inverse ( r, n, a );
	// N.B. below division is dot-div
    return r /= n;
}

//
// Compute the inverse with scaling factor
template<int M>
FINL
CSoraMatrix<vcf,M,M>& inverse_scale (CSoraMatrix<vcf,M,M>& r, const CSoraMatrix<vcf,M,M>& a, float scale) {
	vf n;
	inverse ( r, n, a );
    n = div(n, scale);
	// N.B. below division is dot-div
    return r /= n;
}

//
// Before is just for reference
//
__forceinline vci v_convert2ci_lo(vcq &a, vcq &b){ vci c; c = (vci&)_mm_shuffle_ps((__m128&)a, (__m128&)b, _MM_SHUFFLE(2, 0, 2, 0)); return c; }

FINL
void Inv_ref ( vcs& vh11, vcs& vh12, vcs& vh21, vcs& vh22, vcs& vinvh11, vcs& vinvh12, vcs& vinvh21, vcs& vinvh22) {

	vci vtemp[4];
    vci vstar[2];
    vq  vstarsqr[2];

	vci vcih1, vcih2;

	vcs vNegMask;
	set_all_bits(vNegMask);

    mul(vtemp[0], vtemp[1], vh11, vh22);
    mul(vtemp[2], vtemp[3], vh12, vh21);

    vstar[0] = sub(vtemp[0], vtemp[2]);
    vstar[1] = sub(vtemp[1], vtemp[3]);

    vstarsqr[0] = SquaredNorm(vstar[0]);
    vstarsqr[1] = SquaredNorm(vstar[1]);

    // Add 1 to avoid divided by 0 exception
    vstarsqr[0] = sub(vstarsqr[0], (vq&)vNegMask);
    vstarsqr[1] = sub(vstarsqr[1], (vq&)vNegMask);

    vcq &vres1 = (vcq&)vtemp[0];
    vcq &vres2 = (vcq&)vtemp[1];
    vcq &vres3 = (vcq&)vtemp[2];
    vcq &vres4 = (vcq&)vtemp[3];

    // - invh11
    unpack(vcih1, vcih2, vh22);

    conj_mul(vres1, vres2, vcih1, vstar[0]);
    conj_mul(vres3, vres4, vcih2, vstar[1]);

    vres1 = shift_left(vres1, 16);
    vres2 = shift_left(vres2, 16);
    vres3 = shift_left(vres3, 16);
    vres4 = shift_left(vres4, 16);

    vres1[0].re /= vstarsqr[0][0]; vres1[0].im /= vstarsqr[0][0];
    vres2[0].re /= vstarsqr[0][1]; vres2[0].im /= vstarsqr[0][1];
    vres3[0].re /= vstarsqr[1][0]; vres3[0].im /= vstarsqr[1][0];
    vres4[0].re /= vstarsqr[1][1]; vres4[0].im /= vstarsqr[1][1];

    ((vci&)vres1) = v_convert2ci_lo(vres1, vres2);
    ((vci&)vres3) = v_convert2ci_lo(vres3, vres4);

    vcs &vout11 = (vcs&)vres2;
    vout11       = saturated_pack((vci&)vres1, (vci&)vres3);
    vinvh11      = vout11;

                // - invh12
                unpack(vcih1, vcih2, vh12);

                conj_mul(vres1, vres2, vcih1, vstar[0]);
                conj_mul(vres3, vres4, vcih2, vstar[1]);

                vres1 = shift_left(vres1, 16);
                vres2 = shift_left(vres2, 16);
                vres3 = shift_left(vres3, 16);
                vres4 = shift_left(vres4, 16);

                vres1[0].re /= vstarsqr[0][0]; vres1[0].im /= vstarsqr[0][0];
                vres2[0].re /= vstarsqr[0][1]; vres2[0].im /= vstarsqr[0][1];
                vres3[0].re /= vstarsqr[1][0]; vres3[0].im /= vstarsqr[1][0];
                vres4[0].re /= vstarsqr[1][1]; vres4[0].im /= vstarsqr[1][1];

                ((vci&)vres1) = v_convert2ci_lo(vres1, vres2);
                ((vci&)vres3) = v_convert2ci_lo(vres3, vres4);

                vcs &vout12 = (vcs&)vres2;
                vout12       = saturated_pack((vci&)vres1, (vci&)vres3);
                vout12       = xor(vout12, vNegMask);
                vout12       = sub((vcs&)vout12, (vcs&)vNegMask);
                vinvh12      = vout12;

                // - invh21
                unpack(vcih1, vcih2, vh21);

                conj_mul(vres1, vres2, vcih1, vstar[0]);
                conj_mul(vres3, vres4, vcih2, vstar[1]);

                vres1 = shift_left(vres1, 16);
                vres2 = shift_left(vres2, 16);
                vres3 = shift_left(vres3, 16);
                vres4 = shift_left(vres4, 16);

                vres1[0].re /= vstarsqr[0][0]; vres1[0].im /= vstarsqr[0][0];
                vres2[0].re /= vstarsqr[0][1]; vres2[0].im /= vstarsqr[0][1];
                vres3[0].re /= vstarsqr[1][0]; vres3[0].im /= vstarsqr[1][0];
                vres4[0].re /= vstarsqr[1][1]; vres4[0].im /= vstarsqr[1][1];

                ((vci&)vres1) = v_convert2ci_lo(vres1, vres2);
                ((vci&)vres3) = v_convert2ci_lo(vres3, vres4);

                vcs &vout21 = (vcs&)vres2;
                vout21       = saturated_pack((vci&)vres1, (vci&)vres3);
                vout21       = xor(vout21, vNegMask);
                vout21       = sub((vcs&)vout21, (vcs&)vNegMask);
                vinvh21      = vout21;

                // - invh22
                unpack(vcih1, vcih2, vh11);

                conj_mul(vres1, vres2, vcih1, vstar[0]);
                conj_mul(vres3, vres4, vcih2, vstar[1]);

                vres1 = shift_left(vres1, 16);
                vres2 = shift_left(vres2, 16);
                vres3 = shift_left(vres3, 16);
                vres4 = shift_left(vres4, 16);

                vres1[0].re /= vstarsqr[0][0]; vres1[0].im /= vstarsqr[0][0];
                vres2[0].re /= vstarsqr[0][1]; vres2[0].im /= vstarsqr[0][1];
                vres3[0].re /= vstarsqr[1][0]; vres3[0].im /= vstarsqr[1][0];
                vres4[0].re /= vstarsqr[1][1]; vres4[0].im /= vstarsqr[1][1];

                ((vci&)vres1) = v_convert2ci_lo(vres1, vres2);
                ((vci&)vres3) = v_convert2ci_lo(vres3, vres4);

                vcs &vout22 = (vcs&)vres2;
                vout22       = saturated_pack((vci&)vres1, (vci&)vres3);
                vinvh22      = vout22;
}