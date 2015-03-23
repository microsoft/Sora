#pragma once

#include "dsplib.h"
#include "const.hpp"
#include <cmath>

//
// Bricks start from here
template<class T_CTX, class T_IN>
class TDiscard : public TSink<T_CTX, T_IN>
{
public:
	int total_;
	TDiscard () { 
		total_ = 0;
	}
	
	FINL void Reset ( T_CTX & ctx )
	{
	}
	
	FINL bool Process ( T_IN & ipin, T_CTX & ctx ) { 
		while ( ipin.check_read () ) {
			COMPLEX * p_i = ipin.Peek ();
			ipin.Pop ();
			total_ += ipin.rsize;
			
		}		
		return true;
	};
};

extern int LTPositive[64];
template<class T_CTX, class T_IN, class T_OUT, class T_NEXT>
class TFreqSync : public TFilter<T_CTX, T_IN, T_OUT, T_NEXT>
{
public:
	static const int dispW = 300;
	static const int dispH = 200;
	static const int LTLeading = 16;
	static const int CIOffset = 8;
	static const int FFTSize  = 64;
	static const int LTLen    = 80*2;
		
	int sidx_;
	SampleVec swnd_;
	
	SampleVec lt1_;
	SampleVec lt2_;

	// display
	double dispVal_[64];
	
public:
	TFreqSync (T_NEXT *n) : swnd_ (LTLen), 
							lt1_(FFTSize), 
							lt2_(FFTSize), 
							TFilter<T_CTX, T_IN, T_OUT, T_NEXT> (n)
	{
		InitDisplay ();
		InitBrick ();
	}

	void InitDisplay () {
		
		/*pwnd_ = new CAnimWnd ( );
		pwnd_->Create ( "Channel Model", 5, 220, dispW, dispH );
		pwnd_->EraseCanvas ();
		pwnd_->ShowWindow ();*/
		
	}
	
	void InitBrick () {
		sidx_ = 0;

		memset ( dispVal_, 0, sizeof(dispVal_ ));
	}
	
	void Reset (T_CTX & ctx ) {
		InitBrick();
		next_->Reset ( ctx );
	}


	FINL void ChannelEstimate ( T_CTX & ctx ) {
		// compensation of freqoff
		double darg = ctx.freqOffset_;
		double arg = darg;
		
		for ( int i=CIOffset+1; i < CIOffset + FFTSize*2; i++ ) {
			swnd_[i].rotate (-arg);
			arg += darg;
		}
		
		// channel estimation
		COMPLEX * pwnd = swnd_.get_vec ();
		FFT (64, pwnd+CIOffset,         (COMPLEX*) lt1_ );
		FFT (64, pwnd+CIOffset+FFTSize, (COMPLEX*) lt2_ );

		COMPLEX one  ( 10000,0);
		COMPLEX mone (-10000,0);


	    for (int i = 64 - 26; i < 64; i++)
	    {
	        if (LTPositive[i])
                ctx.ch_[i] = one / (lt1_[i]); 
	        else
				ctx.ch_[i] = mone / (lt1_[i]); 

			
			dispVal_[i] = ctx.ch_[i].norm ();
		}

	    for (int i = 1; i <= 26; i++)
	    {
	        if (LTPositive[i])
	            ctx.ch_[i] = one / (lt1_[i]); 
	        else
	           	ctx.ch_[i] = mone / (lt1_[i]); 
			
			dispVal_[i] = ctx.ch_[i].norm ();
	    }

	}
	
	FINL bool Process ( T_IN & ipin, T_CTX & ctx )
	{
		while (ipin.check_read ()) {
			COMPLEX * p_i = ipin.Peek ();
			for ( int i=0; i< ipin.rsize; i++ )
			{
				if ( ctx.State() == STATE_FSYNC ) {
					
					if ( sidx_ >= LTLen - LTLeading ) {
						swnd_[sidx_] = p_i[i];

						double sarg = 0;
                        COMPLEX fdiff;
                        fdiff.clear ();
                        
						for (int j=0; j<FFTSize; j++) {
							double re = (swnd_[CIOffset+j+FFTSize].re * swnd_[CIOffset+j].re) +
										(swnd_[CIOffset+j+FFTSize].im * swnd_[CIOffset+j].im);
							double im = (swnd_[CIOffset+j+FFTSize].im * swnd_[CIOffset+j].re) -
										(swnd_[CIOffset+j+FFTSize].re * swnd_[CIOffset+j].im);
			
						    fdiff.re += re;
                            fdiff.im += im;
						}
                        
                    	double arg = atan2 ( fdiff.im, fdiff.re );
						//sarg = arg / 64;
						sarg = arg / 64;

						ctx.SetFreqOffset ( sarg );
						
						// Channel estiation 
						ChannelEstimate ( ctx );
						
						ctx.FreqSyncDone ();
						
						ctx.Pause ();
					} else {
						swnd_[sidx_++] = p_i[i];
					}
				
				}

				if (ctx.State() == STATE_HDR || ctx.State () == STATE_DATA ) 
				{
					COMPLEX * p_o = opin_.Push ();
					* p_o = * p_i;
					next_->Process ( opin_, ctx );
				}
			}

			ipin.Pop ();

		}

		return true;
	}
};


template<class T_CTX, class T_IN, class T_OUT, class T_NEXT>
class TCoarseFOE : public TFilter<T_CTX, T_IN, T_OUT, T_NEXT>
{
public:
	const static int foeLen = 16;
	
	int    sidx_;
	SampleVec swnd_;
	
public:
	TCoarseFOE (T_NEXT *n) : swnd_ (2*foeLen), TFilter<T_CTX, T_IN, T_OUT, T_NEXT> (n)
	{
		sidx_ = 0;
	}
	
	void Reset (T_CTX & ctx ) {
		sidx_ = 0;
		next_->Reset ( ctx );
	}
	
	FINL bool Process ( T_IN & ipin, T_CTX & ctx )
	{
		while (ipin.check_read ()) {
			COMPLEX * p_i = ipin.Peek ();
			for ( int i=0; i< ipin.rsize; i++ )
			{
//				if ( !ctx.bCFOE_ && ctx.bCS_ ) {
//
{
 //					ctx.bCFOE_ = true;
					
					swnd_[sidx_] = p_i[0];

					double sarg = 0;
					for (int j=0; j<foeLen; j++) {
						double im = (swnd_[sidx_-j].im - swnd_[sidx_-j-foeLen].im);
						double re = (swnd_[sidx_-j].re - swnd_[sidx_-j-foeLen].re);
						double arg = atan2 ( im, re );
						sarg += arg;
					}

//					ctx.dfoe_ = sarg / foeLen / foeLen;
//					Printf ( "Coarse FOE rand per sample: %.8f\n", ctx.dfoe_ );
//				} else {
//					swnd_[sidx_++] = p_i[i];
//					sidx_ %= 2*foeLen;
				}

//				if ( ctx.bCFOE_ ) {
//					COMPLEX * p_o = opin_.Push ();
//					* p_o = * p_i;
//					next_->Process ( opin_, ctx );
//				}
			}

			ipin.Pop ();

		}

		return true;
	}
};


extern int SHORT_PRE[16][2];
#define TEMPL_ENERGY (54508358.0)
template<class T_CTX, class T_IN, class T_OUT, class T_NEXT>
class TTimeSync : public TFilter<T_CTX, T_IN, T_OUT, T_NEXT>
{
public:
	static const int corrLen 	= 16;
	static const int engThresh 	= 192000000;
	static const int dispLen    = 200;
	static const int dispW = 500;
	static const int dispH = 200;

	// sample count
	int 	nSample_;
	
	double  engSum_;
	double  corrSum_;
	double	lastCorrSum_;
	
	COMPLEX sampleHis_[corrLen];
	double  engHis_[corrLen];
	double  corrHis_[corrLen];
	
	int     hisIdx_;

	
	int     lastPeak_;

	double     dispCorr_[dispLen];
	int        dispIdx_;
	double     dispEng_[dispLen];
	int peek16Count;
public:

	TTimeSync (T_NEXT *n) : TFilter<T_CTX, T_IN, T_OUT, T_NEXT> (n)
	{
		nSample_ = 0;

		InitDisplay ();
		InitBrick ();		
	}

	void Reset (T_CTX & ctx ) {
		InitBrick ();
		next_->Reset ( ctx );
	}

	void InitDisplay () {
		
	}
	
	void InitBrick () {
		lastPeak_ = -1;
		peek16Count = 0;
		
		hisIdx_   = 0;
		engSum_   = 0;
		corrSum_  = 0;
		lastCorrSum_ = 0;

		memset ( sampleHis_, 0, sizeof(sampleHis_));
		memset ( engHis_, 0, sizeof(engHis_));
		memset ( corrHis_, 0, sizeof(corrHis_));

		// clear display buffer
		memset ( dispCorr_, 0, sizeof(dispCorr_));
		memset ( dispEng_, 0, sizeof(dispEng_));
		dispIdx_ = 0;
	}

	FINL bool is_peak ( double & corr )
	{
		double sum = 0;
		for (int i=0; i<16; i++) {
			if (corr < corrHis_[(hisIdx_ + i) % corrLen ] * 0.9999)
			{
				return false;
			}
			sum += corrHis_[(hisIdx_ + i) % corrLen ];
		}
		sum /= corrLen;
		if ( corr < sum * 1.3 )
		{
			return false;
		}
		//return ( corr > sum * 2 );
		return true;
	}
	
	FINL bool Process ( T_IN & ipin, T_CTX & ctx )
	{
		while (ipin.check_read () ) {
			// read one sample each time
			COMPLEX * p = ipin.Peek ();
			for ( int i=0; i < ipin.rsize; i++ ) {
				nSample_ ++;

				double eng = p[i].norm ();
				engSum_   += eng - engHis_[hisIdx_];
				engHis_[hisIdx_] = eng;

				sampleHis_[hisIdx_] = p[i];
				
				// correlation
				int idx = hisIdx_;
				COMPLEX csum;
				csum.clear ();

				for ( int j=0; j<corrLen ; j++ ) {
					idx = idx % corrLen;
					COMPLEX s (SHORT_PRE[j][0], SHORT_PRE[j][1]);
					csum  = csum + sampleHis_[idx] * s;

					idx ++;
				}

				double corrSum = csum.norm();
				corrHis_[hisIdx_++] = corrSum;
				hisIdx_ %= corrLen;
				 
			
				// display
				dispCorr_[dispIdx_] =  corrSum / corrLen / TEMPL_ENERGY ;
				dispEng_[dispIdx_++] = engSum_ / corrLen;
				dispIdx_ %= dispLen;

				if ( ctx.State() == STATE_TSYNC ) {
					if ( is_peak ( corrSum ) ) {
						// This is a peak
						// Found a peak
						if ( (nSample_ - lastPeak_ == 16) && (corrSum > lastCorrSum_ / 2) )
						{
							peek16Count++;
						}
						lastPeak_ = nSample_;
						lastCorrSum_ = corrSum;
	//					Printf ( "Found peak %d\n", nSample_ );
					} else {

						if ( (nSample_ - lastPeak_ == 16) && (peek16Count > 6) )
						{
							Printf("Time Sync Done! Last peak at %d, this sample is %d\n", lastPeak_, nSample_);
							Printf("dist from framestart %d\n", nSample_ - ctx.frameStart_);
								ctx.TimeSyncDone ( nSample_ );
						} else {
							if ( nSample_ - ctx.frameStart_ > 800 ) {
								
								// still no peak found, sync fail
								Printf ( "Time sync failed!\n" );
								//BrickPrint(L"Time sync failed!\r\n");
								ctx.Reset ();
								
							}
						}
						
					}
				 } else if ( ctx.State() == STATE_FSYNC ||
				 			 ctx.State() == STATE_HDR   ||
				 			 ctx.State() == STATE_DATA ) {
				
					COMPLEX * p_o = opin_.Push ();
					* p_o = p[i];
					next_->Process ( opin_, ctx );
				}
			
				ipin.Pop ();
			}
		}
		
		return true;
	}
};

template<class T_CTX, class T_IN, class T_OUT, class T_NEXT>
class TCarrierSense : public TFilter<T_CTX, T_IN, T_OUT, T_NEXT>
{
public:
	static const int autoCorrLen 	= 48;
	static const int noiseEstLen    = 100; // long enough
	static const int engThresh 		= 24000;
	static const int dispLen      	= 200;
	static const int dispW = 500;
	static const int dispH = 200;
	
//	double  acorrSum_;

	COMPLEX acorr;
	COMPLEX ssample;
	double  engSum_;
	
	COMPLEX sampleHis_ [autoCorrLen];
	COMPLEX acorrHis_  [autoCorrLen];
	COMPLEX ssampleHis_[autoCorrLen];
	double  engHis_    [autoCorrLen];

	double  noiseHis_  [noiseEstLen];
	int     nfIdx_;
	
	int     hisIdx_;
	int     nCount_;

	double     dispAcorr_[dispLen];
	int        dispIdx_;
	double     dispEng_[dispLen];
public:

	TCarrierSense (T_NEXT *n) : TFilter<T_CTX, T_IN, T_OUT, T_NEXT> (n)
	{
	
		nCount_   = 0;
	
		InitDisplay ();
		InitBrick ();		
	}

	void Reset (T_CTX & ctx ) {
		InitBrick ();		
		next_->Reset ( ctx );
	}

	void InitDisplay () {
		
	}
	
	void InitBrick () {
		hisIdx_   = 0;
		engSum_   = 0;

		nfIdx_ = 0;
		
		acorr.clear ();
		ssample.clear ();
		
		memset ( sampleHis_, 0, sizeof(sampleHis_));
		memset ( ssampleHis_,0, sizeof(ssampleHis_));
		memset ( acorrHis_,  0, sizeof(acorrHis_));
		memset ( engHis_,    0, sizeof(engHis_));

		memset ( noiseHis_, 0, sizeof (noiseHis_));
	
		// clear display buffer
		memset ( dispAcorr_, 0, sizeof(dispAcorr_));
		memset ( dispEng_,   0, sizeof(dispEng_));
		dispIdx_ = 0;
	}
	
	FINL bool Process ( T_IN & ipin, T_CTX & ctx )
	{
		while (ipin.check_read () ) {
			// read one sample each time
			COMPLEX * p = ipin.Peek ();
			for ( int i=0; i < ipin.rsize; i++ ) {

				double eng = p[i].norm ();
				engSum_ = engSum_ + eng - engHis_[hisIdx_];

				noiseHis_[nfIdx_++] = eng;
				nfIdx_ = nfIdx_ % (noiseEstLen);
				
				COMPLEX np;
				
				np.re = p[i].re * sampleHis_[hisIdx_].re +
					    p[i].im * sampleHis_[hisIdx_].im;

				np.im = p[i].im * sampleHis_[hisIdx_].re -
					    p[i].re * sampleHis_[hisIdx_].im;
				
				acorr = acorr + np;
				acorr = acorr - acorrHis_[hisIdx_];

				COMPLEX ss;
				ss.re = p[i].norm();
				ss.im = 2*p[i].re*p[i].im;

				ssample = ssample + ss;
				ssample = ssample - ssampleHis_[hisIdx_];
							
				// update
				acorrHis_  [hisIdx_] = np;
				ssampleHis_[hisIdx_] = ss;
				engHis_    [hisIdx_] = eng;

				sampleHis_ [hisIdx_++] = p[i];

				hisIdx_ %= autoCorrLen;


				double aa = acorr.norm()  / ssample.norm();

				// display
				dispAcorr_[dispIdx_]   = aa * engSum_;
				dispEng_  [dispIdx_++] = engSum_;
				dispIdx_ %= dispLen;

				nCount_ ++;

	//			Printf ( "sample %d : energy %lf\n", nCount_, eng);				
				if ( nCount_ > autoCorrLen ) {
					// you must have enough in the his

					if ( (aa > 6.0 / 9 ) && 
						 engSum_ > engThresh ) {
						// frame detected
						
						// show noise flow
						int i = nfIdx_;
						
						double nf = 0;
						for ( int c=0; c < 10; c++) {
							nf += noiseHis_[i++];
							i = i % noiseEstLen;
						}

						nf = nf / 10;

						if ( ctx.State() == STATE_CS  ) {
							ctx.CarrierSense ( nCount_, 
											   aa,
											   (engSum_ / 48), 
											   nf );
							
							ctx.Pause ();
						}
					} 

				}

				// push to next
				
				COMPLEX * p_o = opin_.Push ();
				* p_o = p[i];
				next_->Process ( opin_, ctx );
				
			}
			ipin.Pop ();
		}


		
		return true;
	}
};


template<class T_CTX, class T_IN, class T_OUT, class T_NEXT>
class TDecimation : public TFilter<T_CTX, T_IN, T_OUT, T_NEXT>
{
public:
	TDecimation (T_NEXT *n) : TFilter<T_CTX, T_IN, T_OUT, T_NEXT> (n)
	{
	}

	void Reset (T_CTX & ctx ) {
		next_->Reset ( ctx );
	}
	
	FINL bool Process ( T_IN & ipin, T_CTX & ctx )
	{
		while (ipin.check_read ()) {
			COMPLEX * p_i = ipin.Peek ();
			COMPLEX * p_o = opin_.Push ();
			* p_o = * p_i;
			
			ipin.Pop ();

			next_->Process ( opin_, ctx );
		}

		return true;
	}
};


template<class T_CTX, class T_OUT, class T_NEXT>
class TFileSource : public TSource<T_CTX, T_OUT, T_NEXT>
{
public:
	static const int maxsize = 16*1024*1024;
	
	COMPLEX * buf_;
	int       nsample_;
	int       idx_;
	
	TFileSource (T_NEXT * n) : TSource<T_CTX, T_OUT, T_NEXT> (n) 
	{
	 	buf_ = NULL; nsample_ = 0;
	};

	~TFileSource () {
		if (buf_ ) delete [] buf_;
	}

	void Reset () {

		Printf ( "File source reset! idx %d samples %d\n", idx_, nsample_ );
		ctx_.reset_ = false;
		next_->Reset ( ctx_ );
	}
	
	FINL bool LoadFile (char * fname) {
		int msize = maxsize;
		if ( buf_ ) delete buf_;
		buf_ = new COMPLEX [msize];
		nsample_ = LoadDumpFile ( fname, buf_, msize); 
		idx_ = 0;

		Printf ( "Total loaded samples (%d)\n", nsample_ );
		return (nsample_ > 0 );
	}


	FINL bool Process () {

		if ( (idx_ + (int)opin_.wsize) <= nsample_ ) {
			COMPLEX* p = opin_.Push ();
			for ( int j=0; j < opin_.wsize; j++) {
				p[j] = buf_[idx_++];
			}
			next_->Process ( opin_, ctx_ );
		
			return true;
		}
		return false;
	}
};


template<class T_CTX, class T_OUT, class T_NEXT>
class TBufferSource : public TSource<T_CTX, T_OUT, T_NEXT>
{
public:
//	static const int maxsize = 16*1024*1024;
	
	COMPLEX * buf_;
	int       nsample_;
	int       idx_;
	
	TBufferSource (T_NEXT * n) : TSource<T_CTX, T_OUT, T_NEXT> (n) 
	{
	 	buf_ = NULL; nsample_ = 0;
	};

	~TBufferSource () {
//		if (buf_ ) delete [] buf_;
	}

	void Reset () {

		Printf ( "File source reset! idx %d samples %d\n", idx_, nsample_ );
		ctx_.reset_ = false;
		next_->Reset ( ctx_ );
	}
	
	FINL bool SetBuffer (COMPLEX * buf, int size) {
//		int msize = maxsize;
//		if ( buf_ ) delete buf_;
//		buf_ = new COMPLEX [msize];
//		nsample_ = LoadDumpFile ( fname, buf_, msize); 
		buf_ = buf;
		idx_ = 0;
		nsample_ = size;
		
		
		Printf ( "Total loaded samples (%d)\n", nsample_ );
		return (nsample_ > 0 );
	}


	FINL bool Process () {

		if ( (idx_ + (int)opin_.wsize) <= nsample_ ) {
			COMPLEX* p = opin_.Push ();
			for ( int j=0; j < opin_.wsize; j++) {
				p[j] = buf_[idx_++];
			}
			next_->Process ( opin_, ctx_ );
		
			return true;
		}
		return false;
	}
};


