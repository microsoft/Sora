#pragma once

#include <dsplib.h>

#include "demap.hpp"
#include "interleaver.hpp"
#include "viterbi.hpp"

#include "simplewnd.h"
#include "Plot.h"
#include "Logger.h"

extern void __cdecl BrickPrint(wchar_t * format,...);

template<class T_CTX, class T_IN, class T_OUT, class T_NEXT>
class THDRParser : public TFilter<T_CTX, T_IN, T_OUT, T_NEXT>
{
public:
	static const int INBITS   = 1;

public:
	
	THDRParser (T_NEXT *n) : TFilter<T_CTX, T_IN, T_OUT, T_NEXT> (n)
	{

	}
	
	FINL void Reset (T_CTX & ctx ) {
		next_->Reset ( ctx );
	}

	FINL int MapRate ( int sig ) {
		int ret;
		switch (sig) {
		case 11:
			ret = 6;
			break;
		case 15:
			ret = 9;
			break;
		case 10:
			ret = 12;
			break;	
		case 14:
			ret = 18;
			break;	
		case 9:
			ret = 24;
			break;	
		case 13:
			ret = 36;
			break;	
		case 8:
			ret = 48;
			break;	
		case 12:
			ret = 54;
			break;			
		default:
			ret = -1;
		}

		return ret;
	}

	FINL int GetSymCount ( int rate, int size ) {
		int bitSym;
		switch (rate) {
		case 6:
			bitSym =  24;
			break;
		case 9:
			bitSym =  36;
			break;
		case 12:
			bitSym =  48;
			break;
		case 18:
			bitSym = 72;
			break;
		case 24:
			bitSym = 96;
			break;
		case 36:
			bitSym = 144;
			break;
		case 48:
			bitSym = 192;
			break;
		case 54:
			bitSym = 216;
			break;
		default:
			bitSym = 24;
		};

		//int icnt = size * 8 / bitSym;
		//if ( size * 8 % bitSym > 0 ) icnt ++;

		// use padding
		int icnt = (((size << 3) + (22 - 1 + bitSym)) / bitSym);

		return icnt;
		
	}
	
	FINL bool Process ( T_IN & ipin, T_CTX & ctx )
	{
		STATIC_ASSERT ( ipin.rsize == INBITS );
		
		while (ipin.check_read ()) {
			uint * p_i = ipin.Peek ();

			// decode here
			uint hdr = * p_i;
			uint uiParity;

			hdr &= 0xFFFFFF;

			if (hdr & 0xFC0010) // all these bits should be always zero
			{

				Printf ( "Tail or reserved bit are non-zeor! (%x)\n", 
					 hdr & 0xFC0010);

				ctx.bHdrFlag_ = false;	
			} else {
				// parity check
				uiParity = (hdr >> 16) ^ (hdr);
				uiParity = (uiParity >> 8) ^ (uiParity);
				uiParity = (uiParity >> 4) ^ (uiParity);
				uiParity = (uiParity >> 2) ^ (uiParity);
				uiParity = (uiParity >> 1) ^ (uiParity);
				
				if (uiParity & 0x1) {
					Printf ( "Parity check failed!\n" );
					
					ctx.bHdrFlag_ = false;	
				} else {
					ctx.bHdrFlag_ = true;	

					ctx.datarate_ = MapRate ( hdr & 0xF );
					ctx.framelen_ = (hdr >> 5) & 0xFFF;
					ctx.symCount_ = GetSymCount ( ctx.datarate_, 
												  ctx.framelen_ );

					if (ctx.symCount_ != 18)
					{
						//BrickPrint(L"Error, Symbol Count: %d\r\n", ctx.symCount_);
						ctx.Reset();
					}
					Printf ( "Rate %d\nLength %d\n\n", 
						ctx.datarate_, ctx.framelen_ );	 
				}
			}
			
			ipin.Pop (); // pop data

			if ( ctx.bHdrFlag_ ) {
				ctx.HeaderDone ();
			} else {
				ctx.Reset ();
			}
		//	ctx.Abort ();
		}

		return true;
	}
};


template<class T_CTX, class T_IN, class T_OUT, class T_NEXT>
class THDRViterbi : public TFilter<T_CTX, T_IN, T_OUT, T_NEXT>
{
public:
	static const int INBITS   = 48;
	static const int OUTBITS  = 1;

	unsigned int uiHdr_;

public:
	
	THDRViterbi (T_NEXT *n) : TFilter<T_CTX, T_IN, T_OUT, T_NEXT> (n)
	{

	}
	
	FINL void Reset (T_CTX & ctx ) {
		next_->Reset ( ctx );
	}

	FINL bool Process ( T_IN & ipin, T_CTX & ctx )
	{
	    STATIC_ASSERT ( ipin.rsize == INBITS );
		STATIC_ASSERT ( opin_.wsize == OUTBITS );
		
		while (ipin.check_read ()) {
			uchar * p_i = ipin.Peek ();

			// decode here
			uiHdr_ = 0;
			ViterbiHeader (p_i, (uchar*) &uiHdr_ );

			Printf ( "decoded hdr : %X\n", uiHdr_ );
			//BrickPrint(L"decoded hdr : %X\r\n", uiHdr_);

			//if (uiHdr_ != 0x1989)
			//{
			//	getchar();
			//}

			ipin.Pop (); // pop data

			uint* p_o = opin_.Push ();
			* p_o = uiHdr_;
			next_->Process ( opin_, ctx );
		}

		return true;
	}
};


template<class T_CTX, class T_IN, class T_OUT, class T_NEXT>
class THDRSym : public TFilter<T_CTX, T_IN, T_OUT, T_NEXT>
{
public:
	static const int FFTLen   = 64;
	static const int CIOffset = 8;
	static const int mapMod   = 10000;
	
	static const int HDRSize  = 48; // 48 bits for a header
	
	static const int dispW    = 300;
	static const int dispH    = 300;

	int       sidx_;		
	SampleVec sym_;
	unsigned char hdrSoftBits_[HDRSize];
	unsigned char dehdrBits_  [HDRSize];

public:
	void assert () {
		STATIC_ASSERT ( sizeof(T_CTX) == sizeof(COMPLEX) );
		STATIC_ASSERT ( opin_.wsize == FFTLen );
	}
	
	THDRSym (T_NEXT *n) : sym_ (FFTLen),
						  TFilter<T_CTX, T_IN, T_OUT, T_NEXT> (n)
	{
		InitDisplay ();
		InitBrick ();	
	}

	FINL void InitDisplay () {
		
	}

	FINL void InitBrick () {
		sidx_ = 0;
	}
	
	FINL void Reset (T_CTX & ctx ) {
		InitBrick();
		next_->Reset ( ctx );
	}


	FINL void ChannelComp ( T_CTX & ctx )
	{
		for ( int i=0; i < FFTLen; i++ ) {
			sym_[i] = sym_[i] * ctx.ch_[i];
		}
	}


    FINL void PilotTrackCorrection ( T_CTX & ctx ) {
		double th1 = atan2 ( sym_[43].im,  sym_[43].re  ); // -21
		double th2 = atan2 ( sym_[57].im, sym_[57].re );
		double th3 = atan2 ( sym_[7].im, sym_[7].re );
		double th4 = atan2 ( -sym_[21].im, -sym_[21].re ); // 21

		double cf = (th1 + th2 + th3 + th4 ) /4;

		
		double d1 = th3 - th1;
		double d2 = th4 - th2;

		double df = (d1+d2) / 28 / 2; 

		ctx.accum_r_ += cf;
		ctx.accum_sr_ += df;
		

        // We can estimate the sampling frequency offset here
        //
        double dT = df / 2 / 3.14159265359 ;  // us
        
        Printf ( "Sampling freq offset %lf KHz (40MHz sampling)\n", 
			 	 - dT / 144 * 20 * 1024  );
        
		for (int i = 64 - 26; i < 64; i++)
	    {
	       	ctx.ch_[i] = ctx.ch_[i].rotate ( (-cf + (64-i)*df)/2 ); 
            sym_[i].rotate ((-cf + (64-i)*df));
		}

	    for (int i = 1; i <= 26; i++)
	    {
           	ctx.ch_[i] = ctx.ch_[i].rotate ( (-cf - i*df)/2); 
            sym_[i].rotate ((-cf - i*df));
	    }

	}

	FINL bool Process ( T_IN & ipin, T_CTX & ctx )
	{
        STATIC_ASSERT ( ipin.rsize == FFTLen );
		
		while (ipin.check_read ()) {
			COMPLEX * p_i = ipin.Peek ();

			// Get the symbol signal

			// compensate freqoffset
			double darg = ctx.freqOffset_;
			double arg = darg;
			for ( int i=1; i < FFTLen; i++ ) {
				p_i[i].rotate (-arg);
				arg += darg;
			}
		
			
			FFT ( FFTLen, p_i, (COMPLEX*) sym_ );
			ChannelComp ( ctx );
            PilotTrackCorrection ( ctx );

			ipin.Pop (); // pop data
			
			// demap - header, always bpsk
			BPSKDemap ( (COMPLEX*) sym_, hdrSoftBits_ ); 

			DeinterleaveBPSK ( hdrSoftBits_, dehdrBits_ );

			for ( int i=0; i < HDRSize; i++ ) {
				Printf ( "%d", dehdrBits_[i] );
			}
			Printf ( "\n" );

			uchar* p_o = opin_.Push ();
			memcpy ( p_o, dehdrBits_, HDRSize );
			next_->Process ( opin_, ctx );

            ctx.Pause ();
			
		}

		return true;
	}
};


template<class T_CTX, class T_IN, class T_OUT, class T_NEXT>
class TSNREst : public TFilter<T_CTX, T_IN, T_OUT, T_NEXT>
{
public:

static const int FFTLen = 64;
	
#define Mod 	 10000
#define bpskMod  1
#define qpskMod  0.707106781 // 1 / sqrt(2)
#define qam16Mod 0.316227766 // 1 / sqrt(10)
#define qam64Mod 0.15430335  // 1 / sqrt(42)

static const int DataSize  = 48*4;

SampleVec sym_;
unsigned char dataSoftBits_[DataSize];
public:
	TSNREst (T_NEXT *n) : sym_(FFTLen), TFilter<T_CTX, T_IN, T_OUT, T_NEXT> (n)
	{
		STATIC_ASSERT ( sizeof(T_IN::DataType) == sizeof(COMPLEX) );
		STATIC_ASSERT ( opin_.wsize == FFTLen );
	}

	FINL bool Process ( T_IN & ipin, T_CTX & ctx ) {
		while ( ipin.check_read () ) {

			double d;

			if ( ctx.datarate_ < 12 )
			{
				Printf("datarate bpsk\n");
				d = bpskMod * Mod;
			}
			else if ( ctx.datarate_ < 24 )
			{
				Printf("datarate qpsk\n");
				d = qpskMod * Mod;
			}
			else if ( ctx.datarate_ < 48 )
			{
				Printf("datarate 16qam\n");
				d = qam16Mod * Mod;
			}
			else
			{
				Printf("datarate 64qam\n");
				d = qam64Mod * Mod;
			}

			Printf("d: %f\n", d);

			COMPLEX * p_i = ipin.Peek ();

			//Printf("in TSNREst\n");
			//for (int i = 0; i < 64; i++)
			//{
			//	Printf("(%f %f)", p_i[i].re, p_i[i].im);
			//}
			//Printf("\n");

			//for (int i = 0; i < 64; i++)
			//{
			//	ctx.PlotData(p_i[i].re / 12, p_i[i].im / 12);
			//}

			PlotImpl<COMPLEX> * plot = ctx.snrPlot;
			plot->PushData( (double*) p_i, FFTLen * 2 );

			RX_SAMPLE& samples = ctx.samples_;

			demap16QAM_11a(samples, (COMPLEX*) p_i, dataSoftBits_ );
	
			if (samples.IsFull())
			{

				Printf("Calculating SNR\n");
				COMPLEX centers[16];
				double avgRadiusRe[16];
				double avgRadiusIm[16];

				for (int i = 0; i < 16; i++)
				{
					COMPLEX sumSample;
					sumSample.re = 0.0;
					sumSample.im = 0.0;

					int sampleCountInSpot = samples.spots[i].sampleNum;
					for (int j = 0; j < sampleCountInSpot; j++)
					{
						COMPLEX sample = samples.spots[i].samples[j];
						sumSample = sumSample + sample;
					}
					
					centers[i] = sumSample / sampleCountInSpot;
				}				

				for (int i = 0; i < 16; i++)
				{
					double sumRadiusRe = 0;
					double sumRadiusIm = 0;

					int sampleCountInSpot = samples.spots[i].sampleNum;
					for (int j = 0; j < sampleCountInSpot; j++)
					{
						COMPLEX sample = samples.spots[i].samples[j];
						sumRadiusRe += abs(sample.re - centers[i].re);
						sumRadiusIm += abs(sample.im - centers[i].im);
					}
					
					avgRadiusRe[i] = sumRadiusRe / sampleCountInSpot;
					avgRadiusIm[i] = sumRadiusIm / sampleCountInSpot;
				}

				double squareS = 0.0;
				double squareN = 0.0;
				
				for (int i = 0; i < 16; i++)
				{
					squareS += 	centers[i].re * centers[i].re +
								centers[i].im * centers[i].im;
					squareN +=	avgRadiusRe[i] * avgRadiusRe[i] +
								avgRadiusIm[i] * avgRadiusIm[i];
				}

				squareS /= 16;
				squareN /= 16;

				Printf("squareS: %f\n", squareS);
				Printf("squareN: %f\n", squareN);

				double snr = 10 * log10(squareS / squareN);

				Printf("SNR: %f\n", snr);
				//BrickPrint(L"-SNR: %f\r\n", snr);

				ctx.UpdateSNR(snr);
				ctx.UpdateFreqOffset();

				Logger::GetLogger(L"alg")->Log(LOG_STATUS, L"Valid frame detected.");

				//ctx.ShowPerformance();

			}

			ipin.Pop ();

			ctx.Pause();
		}
		return TRUE;
	}

};

extern char pilotSeq[127];

template<class T_CTX, class T_IN, class T_OUT, class T_NEXT>
class TDataSym : public TFilter<T_CTX, T_IN, T_OUT, T_NEXT>
{
public:
	static const int FFTLen   = 64;
	static const int CIOffset = 8;
	static const int mapMod   = 10000;

	static const int dispW    = 300;
	static const int dispH    = 300;

	int       sidx_;		
	SampleVec sym_;

	SampleVec pilot_sym_;
	
	// display
	//CAnimWnd * pwnd_;

public:
	
	TDataSym (T_NEXT *n) : sym_ (FFTLen), pilot_sym_(FFTLen),
						  TFilter<T_CTX, T_IN, T_OUT, T_NEXT> (n)
	{
	    STATIC_ASSERT ( sizeof(T_IN::DataType) == sizeof(COMPLEX) );
		
	    STATIC_ASSERT ( sizeof(T_OUT::DataType) == sizeof(COMPLEX) );
	    STATIC_ASSERT ( opin_.wsize == FFTLen );

		InitDisplay ();
		InitBrick ();
	}

	FINL void InitDisplay () {
		
		//pwnd_ = new CAnimWnd ( );
		//pwnd_->Create ( "Data Symbol", 640, 220, dispW, dispH );
		//pwnd_->EraseCanvas ();
		//pwnd_->SetScale ( -12000, 12000 );
		//pwnd_->ShowWindow ();

		//praw_ = new CAnimWnd ();
		//praw_->Create ( "Raw Pilot", 960, 220, dispW, dispH );
		//praw_->EraseCanvas ();
		//praw_->SetScale ( -12000, 12000 );
		//praw_->ShowWindow ();*/
		
	}

	FINL void InitBrick () {
		sidx_ = 0;
	}
	
	FINL void Reset (T_CTX & ctx ) {
		InitBrick ();
		next_->Reset ( ctx );
	}

	FINL double GetPhaseRotate ( COMPLEX & c1, COMPLEX & c2 )
	{
		double a1 = atan2(c1.im, c1.re); 
		double a2 = atan2(c2.im, c2.re);
		double diff = a2 - a1;
		if ( diff > PI ) diff -= 2*PI;
		else if ( diff < -PI) diff += 2*PI;

		// a quick fix to pilot polarity change, page 33
		if ( diff > PI / 2 ) diff -= PI;
		else if ( diff < - PI/2 ) diff += PI;
		return diff;
	}
	
	FINL void TrackChannel ( T_CTX & ctx ) {
		double th1 = atan2 ( sym_[43].im,  sym_[43].re  ); // -21
		double th2 = atan2 ( sym_[57].im, sym_[57].re );
		double th3 = atan2 ( sym_[7].im, sym_[7].re );
		double th4 = atan2 ( -sym_[21].im, -sym_[21].re ); // 21

		Printf ( "Pilot theta %lf %lf %lf %lf\n", 
			     th1, th2, th3, th4 );

		//
		// Handling the negative polarity in polits
		//
		if ( pilotSeq[ctx.symIdx_ % 127 ] ) {
			Printf ( "Turning PI\n" );
			th1 = (th1 > 0 )? th1-PI: th1+PI;
			th2 = (th2 > 0 )? th2-PI: th2+PI;
			th3 = (th3 > 0 )? th3-PI: th3+PI;
			th4 = (th4 > 0 )? th4-PI: th4+PI;
		}
		
		Printf ( "After correction - Pilot theta %lf %lf %lf %lf\n", 
			     th1, th2, th3, th4 );

		double cf = (th1 + th2 + th3 + th4 ) /4;

		
		double d1 = th3 - th1;
		double d2 = th4 - th2;

		double df = (d1+d2) / 28 / 2; 

		Printf ( "Track const %lf delta theta %lf\n", cf, df );
		
		ctx.accum_r_ += cf;
		ctx.accum_sr_ += df;
		
		Printf ( "Accumulated rotation: cfo %lf sfo %lf \n", 
				 ctx.accum_r_,
				 ctx.accum_sr_ );

		for (int i = 64 - 26; i < 64; i++)
	    {
	       	ctx.ch_[i] = ctx.ch_[i].rotate ( (-cf + (64-i)*df)/2 ); 
            sym_[i].rotate ((-cf + (64-i)*df));
		}

	    for (int i = 1; i <= 26; i++)
	    {
           	ctx.ch_[i] = ctx.ch_[i].rotate ( (-cf - i*df)/2); 
            sym_[i].rotate ((-cf - i*df));
	    }

	}

	FINL void ChannelComp ( T_CTX & ctx )
	{
		for ( int i=0; i < FFTLen; i++ ) {
			sym_[i] = sym_[i] * ctx.ch_[i];
		}
	}
	
	FINL bool Process ( T_IN & ipin, T_CTX & ctx )
	{
		// check_input
        STATIC_ASSERT ( ipin.rsize == FFTLen );
		
		while (ipin.check_read ()) {
			COMPLEX * p_i = ipin.Peek ();

			double darg = ctx.freqOffset_;
			double arg = darg;
			for ( int i=1; i < FFTLen; i++ ) {
				p_i[i].rotate (-arg);
				arg += darg;
			}
		

			// Get the symbol signal
			FFT ( FFTLen, p_i, (COMPLEX*) sym_ );

Printf ( "before compensation\n" );
Printf ( "pilot (%lf %lf) (%lf %lf)\n", sym_[-21].re, sym_[-21].im, 
										sym_[-7].re, sym_[-7].im );

Printf ( "pilot (%lf %lf) (%lf %lf)\n", sym_[7].re, sym_[7].im, 
	                                   sym_[21].re, sym_[21].im );			

			ChannelComp ( ctx );
			pilot_sym_.clear ();
			pilot_sym_[7]  = sym_[7];
			pilot_sym_[21] = sym_[21];
			pilot_sym_[43] = sym_[43];
			pilot_sym_[57] = sym_[57];

			Printf ( "after compensation\n" );

			Printf ( "pilot (%lf %lf) (%lf %lf)\n", sym_[-21].re, sym_[-21].im, 
				sym_[-7].re, sym_[-7].im );

			Printf ( "pilot (%lf %lf) (%lf %lf)\n", sym_[7].re, sym_[7].im, 
				sym_[21].re, sym_[21].im );

			Printf ( "channel (%lf %lf) (%lf %lf)\n", 
				ctx.ch_[57].re, ctx.ch_[57].im, ctx.ch_[43].re, ctx.ch_[43].im );

			Printf ( "channel (%lf %lf) (%lf %lf)\n", ctx.ch_[7].re, ctx.ch_[7].im, 
				ctx.ch_[21].re, ctx.ch_[21].im );


			TrackChannel ( ctx );
			
			ipin.Pop (); // pop data

			//for (int i = 64 - 26; i < 64; i++)
			//{
			//	if (i == 64 - 21 || i == 64 - 7)
			//		continue;
			//	ctx.PlotData(sym_[i].re / 12, sym_[i].im / 12);
			//}
			//for (int i = 1; i <= 26; i++)
			//{
			//	if (i == 7 || i == 21)
			//		continue;
			//	ctx.PlotData(sym_[i].re / 12, sym_[i].im / 12);
			//}

			//if ( pwnd_ ) {
			//	pwnd_->EraseCanvas ();
			//	pwnd_->PlotDots ( (COMPLEX*) sym_, FFTLen, RGB(200,0,0) );
			//	pwnd_->UpdateCanvas ();

			//	//praw_->EraseCanvas ();
			//	//praw_->PlotDots ( (COMPLEX*) pilot_sym_, FFTLen, RGB(200,0,0) );
			//	//praw_->UpdateCanvas ();
			//}



			//for (int i = 0; i < FFTLen; i++)
			//{
			//	plot->PushData((double *)(sym_.m_pData + i), 2);
			//}

			//CAnimWnd * window = ctx.snrwnd;

			//window->EraseCanvas ();
			//window->PlotDots ( (COMPLEX*) sym_, FFTLen, RGB(200,0,0) );
			//window->UpdateCanvas ();

			if (ctx.symCount_ == 18)
			{			
				// call next
				COMPLEX * p_o = opin_.Push ();
				memcpy ( p_o, (COMPLEX*) sym_, FFTLen * sizeof(COMPLEX));
				next_->Process ( opin_, ctx );
			}
			
		}

		return true;
	}
};



template<class T_CTX, class T_IN, 
		 class T_OUT, class T_NEXT,
		 class T_OUT1, class T_NEXT1>
class TSymbol : public TMux <T_CTX, T_IN, 
							 T_OUT, T_NEXT,
							 T_OUT1, T_NEXT1>
{
public:
	const static int SymLen = 80;
	const static int FFTLen = 64;
	
	static const int CIOffset = 8;

	int       sidx_;		
	SampleVec sym_;

public:
	TSymbol (T_NEXT *n, T_NEXT1 * n1) : sym_ (SymLen),
						  TMux <T_CTX, T_IN, 
						        T_OUT, T_NEXT,
						        T_OUT1, T_NEXT1> (n, n1)
	{
		sidx_ = 0;
	}
	
	FINL void Reset (T_CTX & ctx ) {
		sidx_ = 0;
		next_0->Reset ( ctx );
		next_1->Reset ( ctx );
	}

	
	FINL bool Process ( T_IN & ipin, T_CTX & ctx )
	{
		while (ipin.check_read ()) {
			COMPLEX * p_i = ipin.Peek ();
			for ( int i=0; i< ipin.rsize; i++ )
			{
				sym_[sidx_++] = p_i[i];

				if ( sidx_ == SymLen ) {

					//BrickPrint(L"\nSymbol collected! (%d - %d) \n", ctx.symIdx_, ctx.symCount_ );
					
					Printf ( "\nSymbol collected! (%d - %d) \n", ctx.symIdx_, ctx.symCount_ );
					//BrickPrint(L"\nSymbol collected! (%d - %d) \n", ctx.symIdx_, ctx.symCount_ );
					COMPLEX * p_o;

					if (ctx.State() == STATE_HDR ) {
						p_o = opin_0.Push ();

						for ( int j=0; j<FFTLen; j++ ) {
							// strip CP
							p_o[j] = sym_[j+CIOffset];
						}
						
						next_0->Process ( opin_0, ctx );					
						
					} else {
						p_o = opin_1.Push ();

						for ( int j=0; j<FFTLen; j++ ) {
							// strip CP
							p_o[j] = sym_[j+CIOffset];
						}

						next_1->Process ( opin_1, ctx );
				    }
					
					ctx.symIdx_ ++;
					if ( ctx.symIdx_ > ctx.symCount_ ) {
						Printf ( "Frame finished!\n\n" );
						//BrickPrint(L"Frame finished! %d\r\n", ctx.symCount_);
						ctx.Reset ();
					} else {

				//		ctx.Pause ();		
					}

					sidx_ = 0;
				}
			}

			ipin.Pop ();

		}

		return true;
	}
};

